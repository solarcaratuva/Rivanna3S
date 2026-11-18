/* ===================== CAN BENCH TARGET -- NUCLEO-G474RE ===================
 * A CAN test receiver that is not a car board.
 *
 * This branch exists to be branched from. It holds the NucleoBoard target and
 * nothing else, so you can cut your own branch off it, merge your CAN work in,
 * flash a Nucleo and measure what the bus actually does before any of it goes
 * near the car. Keep your changes on your branch; this one stays minimal.
 *
 * A host sends sequence-numbered frames over a USB-CAN adapter (see the
 * usbcantesting repo) and this firmware counts what arrives and what the
 * sequence proves never did. Nothing else runs: no precharge, no ADCs, no
 * relay outputs, no fault handler. The only things exercised are CAN,
 * CanInterface and the log UART, which is the point -- on a car board the
 * measurement keeps being spoiled by the board's other work.
 *
 * Why a separate target rather than reusing a car board: the car boards log on
 * PC_12/PD_2 or PC_10/PC_11, and neither is wired to anything on a Nucleo.
 * See Inc/pindef.h for the pins and the wiring.
 *
 * Reading the numbers:
 *   missing       = frames the sequence proves never arrived
 *   fifo          = how many of those were hardware RX FIFO0 overruns, meaning
 *                   this board was too slow to drain a 3-deep FIFO
 *   the remainder = bus level: arbitration loss with retransmission disabled,
 *                   or bit errors
 *
 * The (+N) figures count from the first stress frame, which drops the handful
 * of frames the board loses at boot before its RX thread is first scheduled.
 * Those are the numbers to read.
 *
 * Compare `got` against the sent count the host prints. The host may not be
 * able to saturate a 250 kbit/s bus, so read the rate it reports rather than
 * assuming it hit the requested period.
 *
 *   python3 compile.py            # builds every target, NucleoBoard included
 *   python3 upload.py nucleo      # flashes it over the Nucleo's ST-Link
 *   python3 monitor.py            # the STRESS lines, 921600 baud
 * ========================================================================= */

#include "CanInterface.h"
#include "Clock.h"
#include "log.h"
#include "pindef.h"
#include "thread.h"

#define STRESS_CAN_ID    0x7FE
/* Every frame is recorded, not just the sequence-numbered ones, so one run can
   answer "what DLCs arrived", "what ID range arrived" and "how many in total"
   as well as "how many went missing". The host resets the board over SWD before
   each test, which zeroes all of this -- there is no command channel to get
   wrong. */
#define ECHO_ID_FIRST    0x7E0  ///< Frames in this range are echoed back,
#define ECHO_ID_LAST     0x7EF  ///< on ID + ECHO_ID_OFFSET.
#define ECHO_ID_OFFSET   0x010
#define STRESS_FILL_BYTE 0xA5
#define REPORT_PERIOD_MS 1000

/* Bench bitrate. 250000 is the car's rate; drop it to trade bus bandwidth for
   signal margin while bringing the wiring up. Keep the host adapter in step. */
#define BENCH_BITRATE 250000

CanInterface main_can(CAN_TX, CAN_RX, CAN_STANDBY, BENCH_BITRATE, CanNetwork::Main);

/* Second instance on FDCAN1, off by default. See Inc/pindef.h for the wiring
   and why it is opt in. Set to 1 once a second transceiver is on the bus. */
#define DUAL_INSTANCE 0

/* Load thread, off by default. Emulates a thread that burns CPU, as an SD write
   or a polled UART transfer does on the car. Every application thread there runs
   at priority 1, and FreeRTOS only preempts from an ISR for a strictly higher
   priority, so the receive thread cannot interrupt one of these and waits for
   it to block or for the next 1 ms tick.

   LOAD_BURN_MS     CPU time burned per period; 0 leaves the thread out.
   LOAD_PERIOD_MS   Period. BURN >= PERIOD is a thread that never blocks.
   LOAD_PRIORITY    1 contends with the receive thread; 0 lets it outrank the
                    load, which tests raising its priority without touching
                    the shared Thread wrapper. */
#define LOAD_BURN_MS   0
#define LOAD_PERIOD_MS 20
#define LOAD_PRIORITY  1

#if LOAD_BURN_MS > 0
static void load_task(void *)
{
    while (true) {
        uint32_t start = Clock::get_current_time();
        while (Clock::get_current_time() - start < LOAD_BURN_MS) {
            __asm volatile("nop");
        }

        if (LOAD_BURN_MS >= LOAD_PERIOD_MS) {
            taskYIELD();
        } else {
            vTaskDelay(pdMS_TO_TICKS(LOAD_PERIOD_MS - LOAD_BURN_MS));
        }
    }
}
#endif

#if DUAL_INSTANCE
#define AUX_TX_ID 0x7FC
CanInterface aux_can(AUX_CAN_TX, AUX_CAN_RX, AUX_CAN_STANDBY, BENCH_BITRATE,
                     CanNetwork::Motor);
Thread aux_tx_thread;

static volatile uint32_t aux_rec     = 0;
static volatile uint32_t aux_tx_ok   = 0;
static volatile uint32_t aux_tx_fail = 0;

/* Counted separately from the first instance on purpose. If the ISR routes a
   frame to the wrong CAN object, these two counts diverge from each other in a
   way nothing else on the bench would reveal. */
void aux_record(const SerializedCanMessage &msg)
{
    (void)msg;
    ++aux_rec;
}

void aux_transmitter()
{
    Clock tx_clock;
    uint32_t seq = 0;
    while (true) {
        SerializedCanMessage msg{};
        msg.id  = AUX_TX_ID;
        msg.len = 4;
        msg.data[0] = static_cast<uint8_t>(seq);
        msg.data[1] = static_cast<uint8_t>(seq >> 8);
        msg.data[2] = static_cast<uint8_t>(seq >> 16);
        msg.data[3] = static_cast<uint8_t>(seq >> 24);
        if (aux_can.write(&msg) == 0) ++aux_tx_ok; else ++aux_tx_fail;
        ++seq;
        tx_clock.sleep_since(100);
    }
}
#endif /* DUAL_INSTANCE */

Thread stress_report_thread;
Thread stress_tx_thread;

/* Transmit side, for bringing the bench up rather than for the test itself.
 * With it running, the board's own log says whether the peripheral accepted
 * each frame, and the adapter listening says whether anything reached the wire.
 * Those two together separate "the MCU cannot send" from "the transceiver is
 * not driving the bus". Set STRESS_TX_PERIOD_MS to 0 to leave the bus alone. */
/* 1 ms between attempts. At 50 kbit/s a frame is about 2.2 ms, so the bus is
   driven essentially all the time -- which is what makes CANH/CANL readable on
   a DC meter. At the default 100 ms the bus is busy ~1% of the time and both
   lines average to the recessive bias whether the driver works or not. */
#define STRESS_TX_PERIOD_MS 100
#define STRESS_TX_ID        0x7FD

static volatile uint32_t stress_tx_ok   = 0;
static volatile uint32_t stress_tx_fail = 0;

void stress_transmitter()
{
    Clock tx_clock;
    uint32_t seq = 0;

    while (true) {
        SerializedCanMessage msg{};
        msg.id  = STRESS_TX_ID;
        /* Every sixteenth frame is zero length. The fdcanusb cannot send a
           DLC 0 frame -- its `can send` requires a payload argument -- so the
           board transmitting one is the only way this bench covers zero
           length frames at all. They are legal CAN, and code that assumes a
           payload tends to drop them or read a stale buffer. */
        msg.len = (seq % 16 == 0) ? 0 : 8;
        msg.data[0] = static_cast<uint8_t>(seq);
        msg.data[1] = static_cast<uint8_t>(seq >> 8);
        msg.data[2] = static_cast<uint8_t>(seq >> 16);
        msg.data[3] = static_cast<uint8_t>(seq >> 24);
        for (int i = 4; i < 8; ++i) {
            msg.data[i] = STRESS_FILL_BYTE;
        }

        if (main_can.write(&msg) == 0) {
            ++stress_tx_ok;
        } else {
            ++stress_tx_fail;
        }
        ++seq;
        tx_clock.sleep_since(STRESS_TX_PERIOD_MS);
    }
}

static volatile uint32_t stress_rx_count    = 0;  // frames delivered to the callback
static volatile uint32_t stress_rx_missing  = 0;  // frames the sequence says never arrived
static volatile uint32_t stress_rx_gaps     = 0;  // how many separate gaps those were
static volatile uint32_t stress_rx_bad      = 0;  // wrong length or wrong fill pattern
static volatile uint32_t stress_rx_restarts = 0;  // sequence went backwards (sender restarted)
static volatile uint32_t stress_first_seq   = 0;
static volatile uint32_t stress_last_seq    = 0;
static volatile bool     stress_have_seq    = false;

// Driver counters as they stood when the first stress frame arrived, so boot
// time losses can be subtracted out.
static volatile uint32_t stress_fifo_base  = 0;
static volatile bool     stress_based      = false;

static volatile uint32_t rec_total    = 0;   ///< Every frame the callback saw
static volatile uint32_t rec_dlc[9]   = {0}; ///< How many arrived at each DLC
static volatile uint32_t rec_min_id   = 0xFFFF;
static volatile uint32_t rec_max_id   = 0;
static volatile uint32_t rec_echoed   = 0;
static volatile uint32_t rec_echo_fail = 0;

/**
 * @brief Record every received frame, and echo the ones the host asks to have
 *        echoed.
 *
 * The DLC histogram and ID range let a single run verify frame-shape handling
 * without logging per frame, which the log UART could not keep up with anyway.
 *
 * Echo covers the other direction: the host sends a frame with a known length
 * and payload, the board sends it straight back, and the host compares. That
 * exercises serialize/deserialize and the transmit path on content the host
 * chose, rather than only on frames the board generated. The range is narrow on
 * purpose so a flood never triggers it and doubles the bus load.
 */
void record_frame(const SerializedCanMessage &msg)
{
    ++rec_total;
    if (msg.len <= 8) {
        ++rec_dlc[msg.len];
    }
    if (msg.id < rec_min_id) rec_min_id = msg.id;
    if (msg.id > rec_max_id) rec_max_id = msg.id;

    if (msg.id >= ECHO_ID_FIRST && msg.id <= ECHO_ID_LAST) {
        SerializedCanMessage reply = msg;
        reply.id = msg.id + ECHO_ID_OFFSET;
        if (main_can.write(&reply) == 0) {
            ++rec_echoed;
        } else {
            ++rec_echo_fail;
        }
    }
}


/**
 * @brief Count a stress frame and work out whether any were skipped.
 *
 * CAN preserves ordering per ID, so a sequence that is not last+1 means the
 * frames in between are gone rather than reordered. A sequence that goes
 * backwards means the sender restarted, counted separately so restarting
 * the host does not read as millions of lost frames.
 */
void handle_stress_frame(const SerializedCanMessage &msg)
{
    ++stress_rx_count;

    if (!stress_based) {
        stress_fifo_base = main_can.get_rx_lost_count();
        stress_based     = true;
    }

    if (msg.len != 8) {
        ++stress_rx_bad;
        return;
    }
    for (int i = 4; i < 8; ++i) {
        if (msg.data[i] != STRESS_FILL_BYTE) {
            ++stress_rx_bad;
            break;
        }
    }

    const uint32_t seq = static_cast<uint32_t>(msg.data[0])
                       | (static_cast<uint32_t>(msg.data[1]) << 8)
                       | (static_cast<uint32_t>(msg.data[2]) << 16)
                       | (static_cast<uint32_t>(msg.data[3]) << 24);

    if (!stress_have_seq) {
        stress_first_seq = seq;
        stress_have_seq  = true;
    } else if (seq < stress_last_seq) {
        ++stress_rx_restarts;
        stress_first_seq = seq;
    } else if (seq != stress_last_seq + 1) {
        ++stress_rx_gaps;
        stress_rx_missing += (seq - stress_last_seq - 1);
    }

    stress_last_seq = seq;
}

/**
 * @brief Print the counters once a second at warn level.
 *
 * Once a second, not per frame, and not from the interrupt: at 921600 baud the
 * log UART saturates around a thousand lines a second and its write timeout
 * drops lines after that, so a per-frame log would measure the logger instead
 * of the CAN path. Split over two lines because the log buffer is 256 bytes and
 * the file-and-line prefix already uses a good part of it.
 */
void stress_report()
{
    Clock report_clock;

    while (true) {
        const uint32_t fifo = main_can.get_rx_lost_count();
        // Frames the sender must have put on the bus over the observed span.
        const uint32_t span = stress_have_seq
                            ? (stress_last_seq - stress_first_seq + 1) : 0;

        log_warn("STRESS got=%lu span=%lu missing=%lu gaps=%lu bad=%lu restarts=%lu",
                 static_cast<unsigned long>(stress_rx_count),
                 static_cast<unsigned long>(span),
                 static_cast<unsigned long>(stress_rx_missing),
                 static_cast<unsigned long>(stress_rx_gaps),
                 static_cast<unsigned long>(stress_rx_bad),
                 static_cast<unsigned long>(stress_rx_restarts));

        log_warn("STRESS seq=%lu..%lu fifo=%lu(+%lu)",
                 static_cast<unsigned long>(stress_first_seq),
                 static_cast<unsigned long>(stress_last_seq),
                 static_cast<unsigned long>(fifo),
                 static_cast<unsigned long>(stress_based ? fifo - stress_fifo_base : 0));

        log_warn("STRESS tx_ok=%lu tx_fail=%lu on 0x%03X",
                 static_cast<unsigned long>(stress_tx_ok),
                 static_cast<unsigned long>(stress_tx_fail),
                 STRESS_TX_ID);

        log_warn("STRESS rec=%lu dlc=%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu id=%03lX..%03lX echo=%lu/%lu",
                 static_cast<unsigned long>(rec_total),
                 static_cast<unsigned long>(rec_dlc[0]), static_cast<unsigned long>(rec_dlc[1]),
                 static_cast<unsigned long>(rec_dlc[2]), static_cast<unsigned long>(rec_dlc[3]),
                 static_cast<unsigned long>(rec_dlc[4]), static_cast<unsigned long>(rec_dlc[5]),
                 static_cast<unsigned long>(rec_dlc[6]), static_cast<unsigned long>(rec_dlc[7]),
                 static_cast<unsigned long>(rec_dlc[8]),
                 static_cast<unsigned long>(rec_min_id > rec_max_id ? 0 : rec_min_id),
                 static_cast<unsigned long>(rec_max_id),
                 static_cast<unsigned long>(rec_echoed),
                 static_cast<unsigned long>(rec_echo_fail));

#if DUAL_INSTANCE
        log_warn("STRESS2 dual=1 got=%lu fifo=%lu tx_ok=%lu tx_fail=%lu",
                 static_cast<unsigned long>(aux_rec),
                 static_cast<unsigned long>(aux_can.get_rx_lost_count()),
                 static_cast<unsigned long>(aux_tx_ok),
                 static_cast<unsigned long>(aux_tx_fail));
#else
        log_warn("STRESS2 dual=0");
#endif

        report_clock.sleep_since(REPORT_PERIOD_MS);
    }
}

void app_main()
{
    // Warn level, not debug: CanInterface::handle() logs every received frame at
    // debug, which saturates the log UART long before the bus saturates.
    log_configure(WARN_LVL, LOG_TX, LOG_RX, 921600);
    log_warn("Nucleo stress receiver up: FDCAN2 on PB_5/PB_6 at %lu bit/s, listening on 0x%03X",
             static_cast<unsigned long>(BENCH_BITRATE), STRESS_CAN_ID);

    main_can.register_callback(STRESS_CAN_ID, handle_stress_frame);
    main_can.register_always_callback(record_frame);
#if DUAL_INSTANCE
    aux_can.register_always_callback(aux_record);
    aux_tx_thread.start(aux_transmitter);
#endif
    stress_report_thread.start(stress_report);
#if LOAD_BURN_MS > 0
    xTaskCreate(load_task, "LOAD", 256, nullptr, LOAD_PRIORITY, nullptr);
    log_warn("Load thread: burn %d ms every %d ms at priority %d",
             LOAD_BURN_MS, LOAD_PERIOD_MS, LOAD_PRIORITY);
#endif
#if STRESS_TX_PERIOD_MS > 0
    stress_tx_thread.start(stress_transmitter);
#endif

    Clock::sleep_forever();
}

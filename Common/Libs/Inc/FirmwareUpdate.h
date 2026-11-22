#pragma once

#ifdef CAN_FIRMWARE_UPDATE
#include <atomic>
#include "CanInterface.h"
#include "FiniteQueue.h"
#include "FirmwareFlash.h"
#include "nodes.h"
#include "thread.h"

// One host and one target per bus, one outstanding 64-byte block. UpdateData is
// raw bytes: do not use the generated uint64_t UpdateData packer.
// UpdateControl preserves its DBC bytes [0..4], followed by this v1 extension:
// [5] 0x10 | operation, [6] source board, [7] session, [8..11] size/offset/error
// (little endian). CRC fields use CRC16/CCITT-FALSE in the low 16 bits.
// A Block control carries its offset and padded-block CRC; the following raw
// UpdateData carries 64 bytes. BlockAck carries the next written offset.
// Legacy 5-byte control messages are ignored, before any erase can occur.
// Testing extension: operations 10/11 request/acknowledge activation of the
// last verified transfer using the same source, session, size and CRC.
// Operations 12/13 query/report the physical bank currently executing (1/2).
class FirmwareUpdate
{
public:
    FirmwareUpdate(CanInterface &can, Node current_board);
    bool start();
    bool begin(uint8_t target, uint32_t size, uint16_t crc);
    bool wait_ready();
    bool write_block(const uint8_t *data);
    bool finish(uint16_t &crc);
    void cancel();
    const char *error() const;

private:
    enum class Operation : uint8_t { Setup = 1, SetupAck, Ready, Block, BlockAck, Done, Result, Abort, Error,
                                     Activate, Activating, QueryBoot, BootStatus };
    enum class Command : uint8_t { Can, Begin, Block, Finish, Cancel };
    enum class Role : uint8_t { Idle, Host, Target };
    enum class Phase : uint8_t { Setup, Erasing, Ready, Block, Finish };
    enum class Reply : uint8_t { SetupAck, Ready, BlockAck, Done, Failed, Cancelled };
    enum class Error : uint8_t { None, Busy, Destination, FlashConfig, TooLarge, Erase, Program,
                                 Protocol, BlockCrc, Timeout, Can, Queue, FinalCrc, Cancelled, Activation };
    struct Event { Command command; SerializedCanMessage frame; };
    struct Result { Reply reply; Error error; uint16_t crc; };

    CanInterface &can_;
    const uint8_t board_;
    FiniteQueue events_{4, sizeof(Event)};
    FiniteQueue results_{2, sizeof(Result)};
    Thread worker_;
    FirmwareFlash flash_;
    std::atomic<bool> overflow_{false};
    static FirmwareUpdate *instance_;
    Role role_ = Role::Idle;
    Phase phase_ = Phase::Setup;
    uint8_t target_ = 0;
    uint8_t host_ = 0;
    uint8_t session_ = 0;
    uint8_t next_session_ = 0;
    uint32_t size_ = 0;
    uint32_t offset_ = 0;
    uint32_t deadline_ = 0;
    uint16_t expected_crc_ = 0;
    uint16_t block_crc_ = 0;
    bool image_verified_ = false;
    Error last_error_ = Error::None; // Only accessed by the UART caller.

    static void receive(const SerializedCanMessage &frame);
    void run();
    void handle_command(const Event &event);
    void handle_can(const SerializedCanMessage &frame);
    bool send(const SerializedCanMessage &frame);
    SerializedCanMessage control(Operation operation, uint32_t value = 0, uint16_t crc = 0) const;
    bool send_control(Operation operation, uint32_t value = 0, uint16_t crc = 0);
    void respond(Reply reply, Error error = Error::None, uint16_t crc = 0);
    bool wait(Reply reply, uint32_t timeout_ms, uint16_t *crc = nullptr);
    bool submit(Command command, const SerializedCanMessage &frame);
    void fail(Error error, bool notify_peer = true);
};
#endif

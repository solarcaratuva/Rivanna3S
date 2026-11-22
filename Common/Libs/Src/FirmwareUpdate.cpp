#include "FirmwareUpdate.h"

#ifdef CAN_FIRMWARE_UPDATE
#include "FirmwareUploader.h"
#include "rivanna3_s.h"
#include <cstring>

namespace {
constexpr uint8_t PROTOCOL_VERSION = 0x10;
constexpr uint8_t CONTROL_SIZE = 12;
constexpr uint32_t CAN_SEND_TIMEOUT_MS = 100;
constexpr uint32_t RESPONSE_TIMEOUT_MS = 2000;
constexpr uint32_t ERASE_TIMEOUT_MS = 15000;
constexpr uint32_t VERIFY_TIMEOUT_MS = 10000; // Full-bank CRC at the G4's 16 MHz debug clock.
constexpr uint32_t IDLE_TIMEOUT_MS = 10000;
constexpr uint16_t WORKER_STACK_WORDS = 768;
// Flags from UpdateControl's DBC, indexed by the v1 operation.
constexpr uint8_t CONTROL_FLAGS[] = {0, 0x10, 0x20, 0x40, 0, 0, 0x80, 0x80, 0, 0, 0, 0, 0, 0};

uint32_t read_u32(const uint8_t *data)
{
    return uint32_t(data[0]) | (uint32_t(data[1]) << 8) |
           (uint32_t(data[2]) << 16) | (uint32_t(data[3]) << 24);
}

void write_u32(uint8_t *data, uint32_t value)
{
    for (unsigned i = 0; i < 4; ++i) data[i] = value >> (8 * i);
}
}

FirmwareUpdate *FirmwareUpdate::instance_ = nullptr;

FirmwareUpdate::FirmwareUpdate(CanInterface &can, Node current_board)
    : can_(can), board_(static_cast<uint8_t>(current_board))
{
}

bool FirmwareUpdate::start()
{
    if (instance_ || board_ >= NUM_NODES) return false;
    instance_ = this;
    next_session_ = Clock::get_current_time();
    if (can_.register_callback(RIVANNA3_S_UPDATE_CONTROL_FRAME_ID, receive) != 0 ||
        can_.register_callback(RIVANNA3_S_UPDATE_DATA_FRAME_ID, receive) != 0) return false;
    return worker_.start([]{ instance_->run(); }, WORKER_STACK_WORDS) == pdPASS;
}

void FirmwareUpdate::receive(const SerializedCanMessage &frame)
{
    Event event{Command::Can, frame};
    // CAN dispatch must not wait for an erase or a flash write.
    if (instance_->events_.append_to_back(&event, 0) != 0) instance_->overflow_.store(true);
}

bool FirmwareUpdate::submit(Command command, const SerializedCanMessage &frame)
{
    Event event{command, frame};
    if (events_.append_to_back(&event, CAN_SEND_TIMEOUT_MS) == 0) return true;
    last_error_ = Error::Queue;
    return false;
}

bool FirmwareUpdate::begin(uint8_t target, uint32_t size, uint16_t crc)
{
    Result stale;
    while (results_.get(&stale, 0) == 0) {}
    SerializedCanMessage frame{};
    frame.data[0] = target;
    write_u32(frame.data + 1, crc);
    write_u32(frame.data + 8, size);
    return submit(Command::Begin, frame) && wait(Reply::SetupAck, RESPONSE_TIMEOUT_MS + CAN_SEND_TIMEOUT_MS);
}

bool FirmwareUpdate::wait_ready()
{
    return wait(Reply::Ready, ERASE_TIMEOUT_MS + RESPONSE_TIMEOUT_MS);
}

bool FirmwareUpdate::write_block(const uint8_t *data)
{
    SerializedCanMessage frame{};
    frame.id = RIVANNA3_S_UPDATE_DATA_FRAME_ID;
    frame.len = FW_BLOCK_SIZE;
    std::memcpy(frame.data, data, FW_BLOCK_SIZE);
    return submit(Command::Block, frame) && wait(Reply::BlockAck, RESPONSE_TIMEOUT_MS + 2 * CAN_SEND_TIMEOUT_MS);
}

bool FirmwareUpdate::finish(uint16_t &crc)
{
    return submit(Command::Finish, {}) && wait(Reply::Done, VERIFY_TIMEOUT_MS + CAN_SEND_TIMEOUT_MS, &crc);
}

void FirmwareUpdate::cancel()
{
    // Cancel only a locally initiated host transfer, never an incoming update.
    const Error saved_error = last_error_;
    if (submit(Command::Cancel, {})) wait(Reply::Cancelled, RESPONSE_TIMEOUT_MS);
    last_error_ = saved_error;
}

bool FirmwareUpdate::wait(Reply reply, uint32_t timeout_ms, uint16_t *crc)
{
    const uint32_t started = Clock::get_current_time();
    Result result;
    while (uint32_t(Clock::get_current_time() - started) < timeout_ms) {
        if (results_.get(&result, 10) != 0) continue;
        if (result.reply == Reply::Failed && reply != Reply::Cancelled) {
            last_error_ = result.error;
            return false;
        }
        if (result.reply == reply) {
            last_error_ = Error::None;
            if (crc) *crc = result.crc;
            return true;
        }
    }
    last_error_ = Error::Timeout;
    return false;
}

const char *FirmwareUpdate::error() const
{
    static const char *const names[] = {nullptr, "update busy", "invalid destination", "unsupported flash layout",
        "image exceeds inactive bank", "flash erase failed", "flash write failed", "invalid CAN update sequence",
        "CAN block CRC mismatch", "CAN update timed out", "CAN send failed", "update queue overflow",
        "target flash CRC mismatch", "update cancelled", "image activation rejected or failed"};
    return names[static_cast<unsigned>(last_error_)];
}

SerializedCanMessage FirmwareUpdate::control(Operation operation, uint32_t value, uint16_t crc) const
{
    SerializedCanMessage frame{};
    frame.id = RIVANNA3_S_UPDATE_CONTROL_FRAME_ID;
    frame.len = CONTROL_SIZE;
    frame.data[0] = target_ | CONTROL_FLAGS[static_cast<unsigned>(operation)];
    write_u32(frame.data + 1, crc);
    frame.data[5] = PROTOCOL_VERSION | static_cast<uint8_t>(operation);
    frame.data[6] = board_;
    frame.data[7] = session_;
    write_u32(frame.data + 8, value);
    return frame;
}

bool FirmwareUpdate::send(const SerializedCanMessage &frame)
{
    const uint32_t started = Clock::get_current_time();
    while (uint32_t(Clock::get_current_time() - started) < CAN_SEND_TIMEOUT_MS) {
        SerializedCanMessage message = frame;
        const int status = can_.write(&message);
        if (status == 0) return true;
        if (status != 3) return false; // Only a full TX FIFO is retryable.
        Clock::sleep_for(1);
    }
    return false;
}

bool FirmwareUpdate::send_control(Operation operation, uint32_t value, uint16_t crc)
{
    return send(control(operation, value, crc));
}

void FirmwareUpdate::respond(Reply reply, Error error, uint16_t crc)
{
    Result result{reply, error, crc};
    if (results_.append_to_back(&result, 0) != 0) overflow_.store(true);
}

void FirmwareUpdate::fail(Error error, bool notify_peer)
{
    image_verified_ = false;
    flash_.lock();
    if (role_ == Role::Host) {
        if (notify_peer) send_control(Operation::Abort);
        Result stale;
        while (results_.get(&stale, 0) == 0) {}
        respond(Reply::Failed, error);
    } else if (role_ == Role::Target && notify_peer) {
        send_control(Operation::Error, static_cast<uint32_t>(error));
    }
    role_ = Role::Idle;
}

void FirmwareUpdate::run()
{
    Event event;
    while (true) {
        const bool received = events_.get(&event, 10) == 0;
        if (overflow_.exchange(false)) fail(Error::Queue);
        if (role_ != Role::Idle && int32_t(Clock::get_current_time() - deadline_) >= 0) fail(Error::Timeout);
        if (!received) continue;
        if (event.command == Command::Can) handle_can(event.frame);
        else handle_command(event);
    }
}

void FirmwareUpdate::handle_command(const Event &event)
{
    if (event.command == Command::Cancel) {
        if (role_ == Role::Host) {
            send_control(Operation::Abort);
            role_ = Role::Idle;
        }
        respond(Reply::Cancelled);
        return;
    }
    if (event.command == Command::Begin) {
        if (role_ != Role::Idle) {
            respond(Reply::Failed, Error::Busy);
            return;
        }
        target_ = event.frame.data[0];
        if (target_ >= NUM_NODES || target_ == board_) {
            respond(Reply::Failed, Error::Destination);
            return;
        }
        role_ = Role::Host;
        phase_ = Phase::Setup;
        host_ = board_;
        if (++next_session_ == 0) ++next_session_;
        session_ = next_session_;
        size_ = read_u32(event.frame.data + 8);
        expected_crc_ = read_u32(event.frame.data + 1);
        offset_ = 0;
        deadline_ = Clock::get_current_time() + RESPONSE_TIMEOUT_MS;
        if (!send_control(Operation::Setup, size_, expected_crc_)) fail(Error::Can);
        return;
    }
    if (role_ != Role::Host || phase_ != Phase::Ready) {
        respond(Reply::Failed, Error::Protocol);
        return;
    }
    if (event.command == Command::Block && offset_ < size_) {
        phase_ = Phase::Block;
        deadline_ = Clock::get_current_time() + RESPONSE_TIMEOUT_MS;
        const uint16_t crc = FirmwareUploader::crc16_hqx(event.frame.data, FW_BLOCK_SIZE);
        if (!send_control(Operation::Block, offset_, crc) || !send(event.frame)) fail(Error::Can);
    } else if (event.command == Command::Finish && offset_ == size_) {
        phase_ = Phase::Finish;
        deadline_ = Clock::get_current_time() + VERIFY_TIMEOUT_MS;
        if (!send_control(Operation::Done, offset_, expected_crc_)) fail(Error::Can);
    } else {
        fail(Error::Protocol);
    }
}

void FirmwareUpdate::handle_can(const SerializedCanMessage &frame)
{
    if (frame.id == RIVANNA3_S_UPDATE_DATA_FRAME_ID) {
        if (role_ != Role::Target) return;
        if (phase_ != Phase::Block || frame.len != FW_BLOCK_SIZE) {
            fail(Error::Protocol);
            return;
        }
        if (FirmwareUploader::crc16_hqx(frame.data, FW_BLOCK_SIZE) != block_crc_) {
            fail(Error::BlockCrc);
            return;
        }
        const uint32_t remaining = size_ - offset_;
        const uint16_t length = remaining < FW_BLOCK_SIZE ? remaining : FW_BLOCK_SIZE;
        if (!flash_.write(frame.data, length)) {
            fail(Error::Program);
            return;
        }
        offset_ += length;
        phase_ = Phase::Ready;
        deadline_ = Clock::get_current_time() + IDLE_TIMEOUT_MS;
        if (!send_control(Operation::BlockAck, offset_)) fail(Error::Can);
        return;
    }
    if (frame.len != CONTROL_SIZE || (frame.data[5] & 0xf0) != PROTOCOL_VERSION) return;
    const unsigned operation = frame.data[5] & 0x0f;
    const uint8_t target = frame.data[0] & 0x0f;
    const uint8_t source = frame.data[6];
    const uint8_t session = frame.data[7];
    if (!operation || operation >= sizeof(CONTROL_FLAGS) || target >= NUM_NODES || source >= NUM_NODES ||
        source == board_ || !session || (frame.data[0] & 0xf0) != CONTROL_FLAGS[operation]) return;
    const auto op = static_cast<Operation>(operation);
    const uint32_t value = read_u32(frame.data + 8);
    const uint32_t crc = read_u32(frame.data + 1);
    if (crc > UINT16_MAX) return;

    if (op == Operation::QueryBoot && target == board_) {
        SerializedCanMessage status = frame;
        status.data[5] = PROTOCOL_VERSION | static_cast<uint8_t>(Operation::BootStatus);
        status.data[6] = board_;
        write_u32(status.data + 1, 0);
        write_u32(status.data + 8, FirmwareFlash::active_bank());
        send(status);
        return;
    }
    if (op == Operation::Activate && target == board_) {
        if (role_ != Role::Idle || !image_verified_ || source != host_ || session != session_ ||
            value != size_ || crc != expected_crc_) return;
        image_verified_ = false; // A replay cannot activate a second time.
        if (!flash_.valid_image() || flash_.crc() != expected_crc_) {
            send_control(Operation::Error, static_cast<uint32_t>(Error::Activation));
            return;
        }
        if (!send_control(Operation::Activating, size_, expected_crc_) || !can_.flush(CAN_SEND_TIMEOUT_MS)) return;
        if (!flash_.activate()) send_control(Operation::Error, static_cast<uint32_t>(Error::Activation));
        return;
    }

    if (op == Operation::Setup && target == board_) {
        if (role_ != Role::Idle) {
            SerializedCanMessage busy = control(Operation::Error, static_cast<uint32_t>(Error::Busy));
            busy.data[0] = target;
            busy.data[7] = session;
            send(busy);
            return;
        }
        role_ = Role::Target;
        image_verified_ = false;
        target_ = target;
        host_ = source;
        session_ = session;
        size_ = value;
        expected_crc_ = crc;
        offset_ = 0;
        if (!flash_.configure(size_)) {
            fail(flash_.capacity() ? Error::TooLarge : Error::FlashConfig);
            return;
        }
        if (!send_control(Operation::SetupAck)) {
            fail(Error::Can);
            return;
        }
        if (!flash_.erase()) {
            fail(Error::Erase);
            return;
        }
        phase_ = Phase::Ready;
        deadline_ = Clock::get_current_time() + IDLE_TIMEOUT_MS;
        if (!send_control(Operation::Ready, flash_.capacity())) fail(Error::Can);
        return;
    }
    if (role_ == Role::Idle || target != target_ || session != session_ ||
        source != (role_ == Role::Host ? target_ : host_)) return;
    if (op == Operation::Abort) {
        fail(Error::Cancelled, false);
        return;
    }
    if (op == Operation::Error) {
        const auto error = value && value <= static_cast<unsigned>(Error::Activation) ?
                           static_cast<Error>(value) : Error::Protocol;
        fail(error, false);
        return;
    }
    if (role_ == Role::Host) {
        if (op == Operation::SetupAck && phase_ == Phase::Setup) {
            phase_ = Phase::Erasing;
            deadline_ = Clock::get_current_time() + ERASE_TIMEOUT_MS;
            respond(Reply::SetupAck);
        } else if (op == Operation::Ready && phase_ == Phase::Erasing && value >= size_) {
            phase_ = Phase::Ready;
            deadline_ = Clock::get_current_time() + IDLE_TIMEOUT_MS;
            respond(Reply::Ready);
        } else if (op == Operation::BlockAck && phase_ == Phase::Block &&
                   value == offset_ + (size_ - offset_ < FW_BLOCK_SIZE ? size_ - offset_ : FW_BLOCK_SIZE)) {
            offset_ = value;
            phase_ = Phase::Ready;
            deadline_ = Clock::get_current_time() + IDLE_TIMEOUT_MS;
            respond(Reply::BlockAck);
        } else if (op == Operation::Result && phase_ == Phase::Finish && value == size_) {
            if (crc != expected_crc_) fail(Error::FinalCrc, false);
            else {
                role_ = Role::Idle;
                respond(Reply::Done, Error::None, crc);
            }
        } else {
            fail(Error::Protocol);
        }
    } else if (op == Operation::Block && phase_ == Phase::Ready && value == offset_ && offset_ < size_) {
        block_crc_ = crc;
        phase_ = Phase::Block;
        deadline_ = Clock::get_current_time() + RESPONSE_TIMEOUT_MS;
    } else if (op == Operation::Done && phase_ == Phase::Ready && value == size_ && offset_ == size_ &&
               crc == expected_crc_) {
        const uint16_t actual_crc = flash_.crc();
        flash_.lock();
        image_verified_ = actual_crc == expected_crc_;
        send_control(Operation::Result, offset_, actual_crc);
        role_ = Role::Idle;
    } else {
        fail(Error::Protocol);
    }
}
#endif

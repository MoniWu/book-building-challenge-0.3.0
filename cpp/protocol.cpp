#include "protocol.hpp"

#include <algorithm>
#include <cstring>

namespace otx {

namespace {

constexpr size_t kCommonHeaderSize = 25;

template <typename T>
T read_le(const uint8_t* data) {
  T value{};
  std::memcpy(&value, data, sizeof(T));
  return value;
}

std::string trim_symbol(const uint8_t* data, size_t len) {
  std::string value(reinterpret_cast<const char*>(data), len);
  const size_t end = value.find('\0');
  if (end != std::string::npos) {
    value.resize(end);
  }
  while (!value.empty() && value.back() == ' ') {
    value.pop_back();
  }
  return value;
}

uint16_t fallback_msg_size(uint8_t msg_type) {
  switch (msg_type) {
    case 1: return 47;  // NewOrder
    case 2: return 33;  // CancelOrder
    case 4: return 61;  // Trade
  }
  return 0;
}

bool parse_message(const uint8_t* data, size_t max_len, MdMessage& message) {
  message = {};
  if (max_len < kCommonHeaderSize) {
    return false;
  }

  uint16_t declared_len = read_le<uint16_t>(data);
  uint16_t expected = fallback_msg_size(data[2]);
  if (expected == 0 || declared_len != expected || declared_len > max_len) {
    return false;
  }

  message.type = static_cast<MessageType>(data[2]);
  message.event_ts_ns = read_le<uint64_t>(data + 9);
  message.symbol = trim_symbol(data + 17, 8);

  switch (message.type) {
    case MessageType::kNewOrder: {
      // offset 25: order_id(8), price(8), quantity(4), side(1), order_type(1)
      message.order_id = read_le<uint64_t>(data + 25);
      message.price = read_le<int64_t>(data + 33);
      message.quantity = read_le<uint32_t>(data + 41);
      message.side = static_cast<Side>(data[45]);
      message.order_type = data[46];
      break;
    }
    case MessageType::kCancelOrder: {
      // offset 25: order_id(8)
      message.order_id = read_le<uint64_t>(data + 25);
      break;
    }
    case MessageType::kTrade: {
      // offset 25: bid_order_id(8), ask_order_id(8), price(8), quantity(4), execution_id(8)
      message.bid_order_id = read_le<uint64_t>(data + 25);
      message.ask_order_id = read_le<uint64_t>(data + 33);
      message.price = read_le<int64_t>(data + 41);
      message.quantity = read_le<uint32_t>(data + 49);
      message.execution_id = read_le<uint64_t>(data + 53);
      break;
    }
    default:
      return false;
  }

  return true;
}

}  // namespace

bool decode_udp_payload(const std::vector<uint8_t>& frame, const uint8_t*& payload, size_t& payload_len) {
  payload = nullptr;
  payload_len = 0;
  if (frame.size() < 42) {
    return false;
  }

  const size_t ip_offset = 14;
  const uint8_t version_ihl = frame[ip_offset];
  const uint8_t version = version_ihl >> 4;
  const size_t ihl = static_cast<size_t>(version_ihl & 0x0F) * 4;
  if (version != 4 || ihl < 20 || frame.size() < ip_offset + ihl + 8) {
    return false;
  }
  if (frame[ip_offset + 9] != 17) {
    return false;
  }

  const size_t udp_offset = ip_offset + ihl;
  const uint16_t udp_length = (static_cast<uint16_t>(frame[udp_offset + 4]) << 8) | frame[udp_offset + 5];
  if (udp_length < 8 || udp_offset + udp_length > frame.size()) {
    return false;
  }

  payload = frame.data() + udp_offset + 8;
  payload_len = udp_length - 8;
  return payload_len > 0;
}

bool parse_md_packet(const uint8_t* payload, size_t payload_len, DecodedPacket& packet) {
  packet = {};
  if (payload_len < kCommonHeaderSize) {
    return false;
  }

  packet.header.channel_id = read_le<uint16_t>(payload + 3);
  packet.header.sequence = read_le<uint32_t>(payload + 5);

  if (!parse_message(payload, payload_len, packet.message)) {
    return false;
  }
  packet.has_message = true;
  return packet.has_message;
}

}  // namespace otx

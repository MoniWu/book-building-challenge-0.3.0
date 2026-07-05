#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace otx {

enum class MessageType : uint8_t {
  kUnknown = 0,
  kNewOrder = 1,
  kCancelOrder = 2,
  kTrade = 4,
};

enum class Side : uint8_t {
  kUnknown = 0,
  kBuy = 1,
  kSell = 2,
};

struct MdMessageHeader {
  uint16_t channel_id = 0;
  uint32_t sequence = 0;
};

struct MdMessage {
  MessageType type = MessageType::kUnknown;
  uint64_t event_ts_ns = 0;
  std::string symbol;
  uint64_t order_id = 0;       // Add/Cancel
  uint64_t bid_order_id = 0;  // Trade: buy-side order
  uint64_t ask_order_id = 0;  // Trade: sell-side order
  int64_t price = 0;
  uint32_t quantity = 0;
  Side side = Side::kUnknown;
  uint8_t order_type = 0;
  uint64_t execution_id = 0;
};

struct DecodedPacket {
  MdMessageHeader header;
  MdMessage message;
  bool has_message = false;
};

bool decode_udp_payload(const std::vector<uint8_t>& frame, const uint8_t*& payload, size_t& payload_len);
bool parse_md_packet(const uint8_t* payload, size_t payload_len, DecodedPacket& packet);

}  // namespace otx

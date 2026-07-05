#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "protocol.hpp"

namespace otx {

struct OrderState {
  uint64_t order_id = 0;
  std::string symbol;
  Side side = Side::kUnknown;
  int64_t price = 0;
  uint32_t qty = 0;
};

struct SymbolBook {
  std::string symbol;
  std::unordered_map<uint64_t, OrderState> orders;
  uint64_t cum_volume = 0;
  uint64_t cum_turnover = 0;
  int64_t high_price = 0;
  int64_t low_price = 0;
  int64_t last_price = 0;
};

using PriceLevel = std::pair<int64_t, std::pair<uint64_t, uint64_t>>;

class BookBuilder {
 public:
  using SnapshotCallback = std::function<void(
      uint64_t ts_ns, const SymbolBook& book,
      const std::vector<PriceLevel>& bids,
      const std::vector<PriceLevel>& asks)>;

  void handle_packet(const DecodedPacket& packet);

  void set_snapshot_callback(SnapshotCallback callback);

 private:
  // Returns true if the message materially changed book or cumulative state.
  bool apply_message(const MdMessage& message);
  void emit_snapshot(uint64_t ts_ns, const SymbolBook& book);

  // Per-channel deduplication state: same (channel_id, sequence) is applied once.
  struct ChannelState {
    uint32_t last_sequence = 0;
    bool seen = false;
  };
  std::unordered_map<uint16_t, ChannelState> channel_states_;
  std::unordered_map<std::string, SymbolBook> books_;
  SnapshotCallback snapshot_callback_;
};

}  // namespace otx
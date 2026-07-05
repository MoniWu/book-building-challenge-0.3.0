#include "book.hpp"

#include <algorithm>
#include <map>

namespace otx {

void BookBuilder::set_snapshot_callback(SnapshotCallback callback) {
  snapshot_callback_ = std::move(callback);
}

void BookBuilder::handle_packet(const DecodedPacket& packet) {
  auto& state = channel_states_[packet.header.channel_id];

  if (state.seen) {
    if (packet.header.sequence <= state.last_sequence) {
      return;
    }
  }
  state.seen = true;
  state.last_sequence = packet.header.sequence;

  if (packet.has_message) {
    const auto& message = packet.message;
    if (!apply_message(message)) return;
    auto book_it = books_.find(message.symbol);
    if (book_it != books_.end()) {
      emit_snapshot(message.event_ts_ns, book_it->second);
    }
  }
}

bool BookBuilder::apply_message(const MdMessage& message) {
  auto& book = books_[message.symbol];
  if (!message.symbol.empty()) {
    book.symbol = message.symbol;
  }

  switch (message.type) {
    case MessageType::kNewOrder: {
      OrderState order;
      order.order_id = message.order_id;
      order.symbol = book.symbol;
      order.side = message.side;
      order.price = message.price;
      order.qty = message.quantity;
      book.orders[order.order_id] = std::move(order);
      return true;
    }
    case MessageType::kCancelOrder: {
      return book.orders.erase(message.order_id) > 0;
    }
    case MessageType::kTrade: {
      auto bid_it = book.orders.find(message.bid_order_id);
      auto ask_it = book.orders.find(message.ask_order_id);

      // At least one order should exist in the book
      if (bid_it == book.orders.end() && ask_it == book.orders.end()) {
        return false;
      }

      // Update cumulative statistics
      book.cum_volume += message.quantity;
      book.cum_turnover += static_cast<uint64_t>(message.price) * message.quantity;
      if (book.high_price == 0 || message.price > book.high_price) {
        book.high_price = message.price;
      }
      if (book.low_price == 0 || message.price < book.low_price) {
        book.low_price = message.price;
      }
      book.last_price = message.price;

      // Reduce quantity or remove bid order
      if (bid_it != book.orders.end()) {
        if (message.quantity >= bid_it->second.qty) {
          book.orders.erase(bid_it);
        } else {
          bid_it->second.qty -= message.quantity;
        }
      }

      // Reduce quantity or remove ask order
      if (ask_it != book.orders.end()) {
        if (message.quantity >= ask_it->second.qty) {
          book.orders.erase(ask_it);
        } else {
          ask_it->second.qty -= message.quantity;
        }
      }

      return true;
    }
    case MessageType::kUnknown:
      return false;
  }
  return false;
}

void BookBuilder::emit_snapshot(uint64_t ts_ns, const SymbolBook& book) {
  if (!snapshot_callback_) {
    return;
  }
  std::map<int64_t, std::pair<uint64_t, uint64_t>, std::greater<int64_t>> bids;
  std::map<int64_t, std::pair<uint64_t, uint64_t>> asks;

  for (const auto& order_entry : book.orders) {
    const auto& order = order_entry.second;
    if (order.side == Side::kBuy) {
      auto& level = bids[order.price];
      level.first += order.qty;
      level.second += 1;
    } else {
      auto& level = asks[order.price];
      level.first += order.qty;
      level.second += 1;
    }
  }

  std::vector<PriceLevel> bid_vec;
  std::vector<PriceLevel> ask_vec;
  for (const auto& level_entry : bids) {
    bid_vec.push_back(level_entry);
    if (bid_vec.size() >= 10) break;
  }
  for (const auto& level_entry : asks) {
    ask_vec.push_back(level_entry);
    if (ask_vec.size() >= 10) break;
  }

  snapshot_callback_(ts_ns, book, bid_vec, ask_vec);
}

}  // namespace otx

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#include "book.hpp"
#include "protocol.hpp"
#include "snapshot_writer.hpp"

namespace {

// Build a valid NewOrder message (47 bytes: 25 common + 22 type-specific)
std::vector<uint8_t> make_add_message(uint16_t channel_id, uint32_t sequence,
                                       uint64_t event_ts_ns, const char* symbol,
                                       uint64_t order_id, int64_t price, uint32_t qty,
                                       uint8_t side, uint8_t order_type) {
  std::vector<uint8_t> msg(47, 0);
  auto write16 = [&](size_t offset, uint16_t v) { std::memcpy(msg.data() + offset, &v, 2); };
  auto write32 = [&](size_t offset, uint32_t v) { std::memcpy(msg.data() + offset, &v, 4); };
  auto write64 = [&](size_t offset, uint64_t v) { std::memcpy(msg.data() + offset, &v, 8); };
  auto write64s = [&](size_t offset, int64_t v) { std::memcpy(msg.data() + offset, &v, 8); };

  write16(0, 47);     // msg_len
  msg[2] = 1;         // msg_type = NewOrder
  write16(3, channel_id);
  write32(5, sequence);
  write64(9, event_ts_ns);
  std::memcpy(msg.data() + 17, symbol, std::strlen(symbol));  // symbol at offset 17
  write64(25, order_id);
  write64s(33, price);
  write32(41, qty);
  msg[45] = side;
  msg[46] = order_type;
  return msg;
}

// Build a UDP payload containing one message
std::vector<uint8_t> make_payload() {
  return make_add_message(101, 7, 1500000000ULL, "ALFA", 42, 123450, 900, 1, 1);
}

size_t g_row_count = 0;

void test_callback(uint64_t /*ts_ns*/, const otx::SymbolBook& /*book*/,
                   const std::vector<otx::PriceLevel>& /*bids*/,
                   const std::vector<otx::PriceLevel>& /*asks*/) {
  ++g_row_count;
}

}  // namespace

int main() {
  auto payload = make_payload();

  otx::DecodedPacket packet;
  assert(otx::parse_md_packet(payload.data(), payload.size(), packet));
  assert(packet.header.channel_id == 101);
  assert(packet.header.sequence == 7);
  assert(packet.has_message);
  assert(packet.message.symbol == "ALFA");
  assert(packet.message.order_id == 42);
  assert(packet.message.quantity == 900);
  assert(packet.message.type == otx::MessageType::kNewOrder);

  otx::BookBuilder builder;
  builder.set_snapshot_callback(test_callback);
  builder.handle_packet(packet);
  assert(g_row_count > 0);

  // Test SnapshotWriter produces output
  otx::SnapshotWriter writer("test_output.csv");
  assert(writer.good());

  return 0;
}

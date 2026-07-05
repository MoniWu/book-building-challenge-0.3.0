# Market Data Specification

## Scope

This document describes the market-data capture format and the required output schema for the replay exercise.

The provided `input.pcap` contains UDP traffic from a simulated market. Some channels are captured from redundant A/B lines. Per-channel sequencing exists, but records may arrive with duplicate logical messages and limited capture-time reordering.

## Packet Envelope

The PCAP file uses classic little-endian PCAP, not PCAPNG.

Each captured record contains an Ethernet frame with:

- Ethernet II
- IPv4
- UDP
- a market-data payload

## Message Framing

Each UDP payload contains exactly one market-data message. The first two bytes declare the message's total size (`msg_len`). All multi-byte integers are little-endian.

The `(channel_id, sequence)` pair uniquely identifies a logical message. The same logical message may appear on the A line, the B line, or both (see "A/B Lines"). Consumers should deduplicate by this key.

### Common Header Fields

Every message begins with these 25 bytes:

| offset | size | type | field |
| --- | --- | --- | --- |
| 0 | 2 | `uint16_le` | `msg_len` — total message size in bytes |
| 2 | 1 | `uint8` | `msg_type` |
| 3 | 2 | `uint16_le` | `channel_id` |
| 5 | 4 | `uint32_le` | `sequence` — per-channel, monotonically increasing, shared between A and B lines |
| 9 | 8 | `uint64_le` | `event_ts_ns` |
| 17 | 8 | `char[8]` | `symbol` — ASCII, NUL-padded |

Field conventions:

- `side`: `1 = buy`, `2 = sell`
- `order_type`: `1 = limit`, `2 = IOC`
- prices are integer price units with 4 decimal places implied
- quantities are integer units

### Message Types

**`1 = NewOrder`** — 47 bytes

A resting order becomes visible in the book. For partially matched aggressive limit orders, only the remaining resting quantity is published.

| offset | size | type | field |
| --- | --- | --- | --- |
| 0 | 2 | `uint16_le` | `msg_len` |
| 2 | 1 | `uint8` | `msg_type` = 1 |
| 3 | 2 | `uint16_le` | `channel_id` |
| 5 | 4 | `uint32_le` | `sequence` |
| 9 | 8 | `uint64_le` | `event_ts_ns` |
| 17 | 8 | `char[8]` | `symbol` |
| 25 | 8 | `uint64_le` | `order_id` |
| 33 | 8 | `int64_le` | `price` |
| 41 | 4 | `uint32_le` | `quantity` |
| 45 | 1 | `uint8` | `side` |
| 46 | 1 | `uint8` | `order_type` |

**`2 = CancelOrder`** — 33 bytes

Removes the full remaining quantity of a resting order.

| offset | size | type | field |
| --- | --- | --- | --- |
| 0 | 2 | `uint16_le` | `msg_len` |
| 2 | 1 | `uint8` | `msg_type` = 2 |
| 3 | 2 | `uint16_le` | `channel_id` |
| 5 | 4 | `uint32_le` | `sequence` |
| 9 | 8 | `uint64_le` | `event_ts_ns` |
| 17 | 8 | `char[8]` | `symbol` |
| 25 | 8 | `uint64_le` | `order_id` |

**`4 = Trade`** — 61 bytes

A fill against a resting visible order. `bid_order_id` is the buy-side order, `ask_order_id` is the sell-side order. Trade price is the resting order's price.

| offset | size | type | field |
| --- | --- | --- | --- |
| 0 | 2 | `uint16_le` | `msg_len` |
| 2 | 1 | `uint8` | `msg_type` = 4 |
| 3 | 2 | `uint16_le` | `channel_id` |
| 5 | 4 | `uint32_le` | `sequence` |
| 9 | 8 | `uint64_le` | `event_ts_ns` |
| 17 | 8 | `char[8]` | `symbol` |
| 25 | 8 | `uint64_le` | `bid_order_id` — buy-side order |
| 33 | 8 | `uint64_le` | `ask_order_id` — sell-side order |
| 41 | 8 | `int64_le` | `price` |
| 49 | 4 | `uint32_le` | `quantity` |
| 53 | 8 | `uint64_le` | `execution_id` |

## Output CSV Schema

Your program must emit a CSV with this header:

```text
time,ts_ns,symbol,ask1_price,ask1_qty,ask1_orders,ask2_price,ask2_qty,ask2_orders,...,ask10_price,ask10_qty,ask10_orders,bid1_price,bid1_qty,bid1_orders,...,bid10_price,bid10_qty,bid10_orders,cum_volume,cum_turnover,high_price,low_price,last_price
```

Rules:

- `time` is a human-readable timestamp in `yyyymmdd HH:MM:SS.mmm` format (UTC)
- Emit one row after each visible market-data update that materially changes the book or cumulative trade statistics
- `ts_ns` is the triggering message's `event_ts_ns`
- `symbol` is the ASCII symbol
- `ask{N}_price` / `bid{N}_price` is the aggregate price at ask/bid level N (1 = best)
- `ask{N}_qty` / `bid{N}_qty` is the aggregate visible quantity at that level
- `ask{N}_orders` / `bid{N}_orders` is the number of visible resting orders at that level
- Empty fields indicate no order exists at that level
- `cum_volume` is the cumulative trade volume since session start
- `cum_turnover` is the cumulative trade turnover (sum of price x qty) since session start
- `high_price` is the highest trade price since session start (empty if no trades yet)
- `low_price` is the lowest trade price since session start (empty if no trades yet)
- `last_price` is the most recent trade price (empty if no trades yet)

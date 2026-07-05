# Knowledge Primer: Order Books and Market Data

If you are new to financial markets, this document explains the core concepts you need for this exercise. If anything remains unclear, you are encouraged to search online and learn more about the relevant market data concepts.

## What Is an Order Book?

An order book is a list of buy and sell orders for a financial instrument (a "symbol"), organized by price.

Imagine a marketplace where people want to trade apples:

- **Buyers** say "I will pay up to $1.00 per apple for 100 apples." These are called **bids**.
- **Sellers** say "I will sell at $1.05 per apple for 50 apples." These are called **asks** (or offers).

The order book keeps these lists sorted:

- **Bids** are sorted from highest price to lowest — the best bid is the highest price anyone is willing to pay.
- **Asks** are sorted from lowest price to highest — the best ask is the lowest price anyone is willing to sell at.

The gap between the best bid and best ask is the **spread**.

A snapshot of an order book might look like this (top 3 levels):

| Side | Price | Total Qty | Order Count |
| --- | --- | --- | --- |
| Ask (sell) | 105.00 | 50 | 1 |
| Ask (sell) | 105.50 | 200 | 2 |
| Ask (sell) | 106.00 | 75 | 1 |
| **---** | | | |
| Bid (buy) | 104.50 | 100 | 1 |
| Bid (buy) | 104.00 | 300 | 3 |
| Bid (buy) | 103.50 | 150 | 2 |

## How Does a Trade Happen?

A **trade** occurs when a new order matches against a resting order on the opposite side:

- A **buy order** that arrives at 105.00 (or higher) will match against the best ask (105.00). The trade executes at 105.00 — the resting order's price.
- A **sell order** that arrives at 104.50 (or lower) will match against the best bid (104.50).

When a trade happens, the quantity of the resting order is reduced. If it goes to zero, the order is removed from the book.

**Price-time priority** determines who gets filled first: among orders at the same price, the one that arrived first gets matched first.

## Order Types

This exercise uses two order types:

### Limit Orders

A limit order states a specific price. If it cannot match immediately against the opposite side, it **rests** in the order book and becomes visible to other participants.

Example: "Buy 100 shares at $104.00." If the lowest ask is $105.00, there is no match, so this order sits in the book as a bid at $104.00.

If a limit order matches partially (e.g., only 30 out of 100 shares can fill), the remaining 70 become a resting order in the book. The Add message in the feed will show only this resting portion.

### IOC Orders

IOC stands for "Immediate or Cancel." An IOC order tries to match immediately against available resting orders. Any portion that cannot fill right away is **canceled** — it never rests in the book.

IOC orders do not appear in Add messages (they never become visible resting orders), but they can generate Trade messages when they match.

## What Is a Snapshot?

A **snapshot** is the state of the order book at a specific moment in time. Instead of describing every individual order event that happened, a snapshot answers: "What does the book look like right now?"

For this exercise, each output row is a snapshot for one symbol at one timestamp. It contains:

- The top 10 ask price levels and top 10 bid price levels.
- The total visible quantity and number of resting orders at each level.
- Cumulative trade statistics since the start of the session, such as total volume, turnover, high price, low price, and last traded price.

Snapshots are derived from the sequence of market data messages. To produce a correct snapshot, your program needs to replay all relevant NewOrder, CancelOrder, and Trade messages up to that snapshot timestamp, then aggregate the visible resting orders by price level.

Think of the message feed as a video and snapshots as still frames taken from that video. If you miss or process a frame incorrectly, later snapshots may also be wrong because the book state carries forward over time.

The provided `ground_truth.csv` is the correct output for this task. Your reconstructed snapshots should match it as closely as possible.

## A/B Lines (Redundant Feeds)

In real market data systems, the same data is often sent over two redundant network paths — called **A line** and **B line**. If one line drops packets, the other may still deliver them.

Key facts for this exercise:

- **Same sequence space**: Both A and B carry the same logical channel. A message with `sequence = 100` on the A line and `sequence = 100` on the B line is the same logical message. You should process it only once.
- **Not identical coverage**: One line may carry packets the other missed. Your consumer should merge both lines, not pick one.
- **Line identity**: You can tell which line a packet arrived on by inspecting the UDP destination address and port in the PCAP frame. The `channel_id` in the message header (see Specification) also identifies the logical channel.

Think of A/B lines as two copies of the same newspaper delivered to your doorstep — you only need to read one copy, but if one goes missing, the other has you covered.

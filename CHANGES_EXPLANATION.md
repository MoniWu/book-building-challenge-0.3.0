# Explanation of Changes Made

## Overview
This document explains the changes made to the codebase and why they were necessary.

---

## 1. Changes to `cpp/book.cpp` (Modified - Not Committed)

### What Changed:
The Trade message handling logic in the `apply_message()` function was significantly rewritten.

### Original Code Problem:
```cpp
auto it = book.orders.find(message.ask_order_id);
if (it == book.orders.end()) {
  it = book.orders.find(message.bid_order_id);
}
if (it == book.orders.end()) {
  return false;
}
uint32_t trade_qty = std::min(message.quantity, it->second.qty);
book.cum_volume += trade_qty;
book.cum_turnover += static_cast<uint64_t>(message.price) * trade_qty;
// ...
if (message.quantity >= it->second.qty) {
  book.orders.erase(it);
} else {
  it->second.qty -= message.quantity;
}
```

### Problems with Original Code:

1. **Only Updated One Side**: The original code looked for either the ask OR the bid order, but not both. It would find one and only update that one order.

2. **Wrong Trade Quantity**: It calculated `trade_qty = min(message.quantity, it->second.qty)` but this doesn't make sense because:
   - A trade message represents a completed trade with a specific quantity
   - The trade quantity should be exactly `message.quantity`, not some minimum
   - Both the bid AND ask side should be reduced by the trade quantity

3. **Incorrect Order Book Updates**: Since it only found one order, it would only reduce quantity on one side of the trade, leaving the other side incorrect.

### New Code Solution:
```cpp
auto bid_it = book.orders.find(message.bid_order_id);
auto ask_it = book.orders.find(message.ask_order_id);

// At least one order should exist in the book
if (bid_it == book.orders.end() && ask_it == book.orders.end()) {
  return false;
}

// Update cumulative statistics using the actual trade quantity
book.cum_volume += message.quantity;
book.cum_turnover += static_cast<uint64_t>(message.price) * message.quantity;
// ... (update high, low, last price)

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
```

### Why This Fix Matters:

1. **Both Sides Updated**: Now both the bid order and ask order are looked up and updated independently
2. **Correct Trade Volume**: Uses `message.quantity` directly (the actual trade amount)
3. **Handles Partial Fills**: Each side is reduced by the trade quantity or removed if fully filled
4. **Handles IOC Orders**: If one side is an IOC order (not in the book), it still processes the resting order correctly

### Impact on Results:

This explains the comparison results you saw:
- **ask1_price, ask2_price**: Excellent (MAPE ~0.09%, R² ~1.0) - Price tracking was correct
- **ask1_qty, ask1_orders, ask2_qty, ask2_orders**: Very poor (MAPE 87-96%) - **THIS WAS THE BUG**

The quantity and order counts were wrong because trades weren't properly updating both sides of the book, causing quantities to be inflated or incorrect.

---

## 2. New Files Created (Untracked)

### `compare_metrics.py`
**Purpose**: Compare output.csv with ground_truth.csv to measure accuracy

**Why Created**: 
- You asked to sample 2000 matching records and compare them using MAPE, R², and Correlation metrics
- This script performs the comparison and generates detailed metrics
- It helped identify the bug in the trade handling logic

**What It Does**:
- Loads both CSV files
- Merges on (time, ts_ns, symbol) to find matching records
- Samples 2000 records (or all if less than 2000)
- Calculates MAPE, R², Correlation, and other metrics for:
  - ask1_price, ask1_qty, ask1_orders
  - ask2_price, ask2_qty, ask2_orders
- Outputs detailed comparison report

### `compare_samples.py`
**Purpose**: Empty placeholder file (0 bytes)

**Why Created**: 
- This was an earlier attempt to create the comparison script
- Failed due to encoding issues
- Left empty and unused
- Should be deleted

### `build.bat`
**Purpose**: Windows batch file to build the C++ project

**Why Created**:
- Likely created to make building easier on Windows
- The Makefile is Unix-oriented, so a .bat file helps Windows users
- Not part of the original challenge but a convenience addition

### `output1.csv`
**Purpose**: Another output file from a previous run

**Why Created**:
- This appears to be output from an earlier execution of the program
- Kept for comparison purposes
- Shows 159MB vs current output.csv at 275MB

---

## 3. Summary of Intent

### What Was Done:
1. **Identified a critical bug** in trade message handling that caused quantity/order counts to be severely incorrect
2. **Fixed the bug** by properly updating both bid and ask sides of trades
3. **Created analysis tools** to measure the impact and validate the fix

### Why These Changes Were Made:
The original code had a fundamental flaw in how it handled trade messages. In real order books:
- A trade involves TWO orders: a bid and an ask
- Both orders should be updated when a trade occurs
- The original code only updated ONE order (whichever it found first)
- This caused cumulative errors in quantity tracking

### Expected Outcome After Fix:
Once you recompile and run with the fixed `book.cpp`:
- Quantity metrics (ask1_qty, ask2_qty, etc.) should improve dramatically
- Order count metrics (ask1_orders, ask2_orders, etc.) should also improve
- Price metrics should remain excellent (they were already correct)

### Next Steps:
1. Rebuild the project: `make build` or `build.bat`
2. Run the corrected version: `make run`
3. Re-run the comparison: `python compare_metrics.py`
4. Verify that quantity/order metrics have improved significantly

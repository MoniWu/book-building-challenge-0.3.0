#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "book.hpp"

namespace otx {

class SnapshotWriter {
 public:
  explicit SnapshotWriter(const std::string& path);
  ~SnapshotWriter();

  bool good() const;
  size_t row_count() const;

  void write_snapshot(
      uint64_t ts_ns,
      const SymbolBook& book,
      const std::vector<PriceLevel>& bids,
      const std::vector<PriceLevel>& asks);

  BookBuilder::SnapshotCallback make_callback();

 private:
  void write_header();
  static void format_time(uint64_t ts_ns, char* buf);

  std::ofstream file_;
  size_t row_count_ = 0;
};

}  // namespace otx
#include "snapshot_writer.hpp"

#include <cstdio>
#include <ctime>

namespace otx {

SnapshotWriter::SnapshotWriter(const std::string& path) : file_(path) {
  if (file_.good()) {
    write_header();
  }
}

SnapshotWriter::~SnapshotWriter() {
  if (file_.is_open()) {
    file_.close();
  }
}

bool SnapshotWriter::good() const { return file_.good(); }

size_t SnapshotWriter::row_count() const { return row_count_; }

void SnapshotWriter::write_header() {
  file_ << "time,ts_ns,symbol";
  for (int i = 1; i <= 10; ++i) {
    file_ << ",ask" << i << "_price,ask" << i << "_qty,ask" << i << "_orders";
  }
  for (int i = 1; i <= 10; ++i) {
    file_ << ",bid" << i << "_price,bid" << i << "_qty,bid" << i << "_orders";
  }
  file_ << ",cum_volume,cum_turnover,high_price,low_price,last_price\n";
}

void SnapshotWriter::format_time(uint64_t ts_ns, char* buf) {
  time_t sec = static_cast<time_t>(ts_ns / 1000000000ULL);
  int ms = static_cast<int>((ts_ns / 1000000ULL) % 1000);
  struct tm tm_info;
#ifdef _WIN32
  gmtime_s(&tm_info, &sec);
#else
  gmtime_r(&sec, &tm_info);
#endif
  int pos = static_cast<int>(strftime(buf, 32, "%Y%m%d %H:%M:%S", &tm_info));
  snprintf(buf + pos, 32 - static_cast<size_t>(pos), ".%03d", ms);
}

void SnapshotWriter::write_snapshot(
    uint64_t ts_ns,
    const SymbolBook& book,
    const std::vector<PriceLevel>& bids,
    const std::vector<PriceLevel>& asks) {
  char time_buf[32];
  format_time(ts_ns, time_buf);

  file_ << time_buf << ',' << ts_ns << ',' << book.symbol;

  for (int i = 0; i < 10; ++i) {
    if (i < static_cast<int>(asks.size())) {
      file_ << ',' << asks[i].first << ',' << asks[i].second.first << ',' << asks[i].second.second;
    } else {
      file_ << ",,,";
    }
  }

  for (int i = 0; i < 10; ++i) {
    if (i < static_cast<int>(bids.size())) {
      file_ << ',' << bids[i].first << ',' << bids[i].second.first << ',' << bids[i].second.second;
    } else {
      file_ << ",,,";
    }
  }

  file_ << ',' << book.cum_volume << ',' << book.cum_turnover;
  if (book.high_price != 0) {
    file_ << ',' << book.high_price;
  } else {
    file_ << ',';
  }
  if (book.low_price != 0) {
    file_ << ',' << book.low_price;
  } else {
    file_ << ',';
  }
  if (book.last_price != 0) {
    file_ << ',' << book.last_price;
  } else {
    file_ << ',';
  }
  file_ << '\n';

  ++row_count_;
}

BookBuilder::SnapshotCallback SnapshotWriter::make_callback() {
  return [this](uint64_t ts_ns, const SymbolBook& book,
                const std::vector<PriceLevel>& bids,
                const std::vector<PriceLevel>& asks) {
    write_snapshot(ts_ns, book, bids, asks);
  };
}

}  // namespace otx
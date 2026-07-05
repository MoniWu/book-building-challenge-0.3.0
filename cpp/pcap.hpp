#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace otx {

struct PcapRecord {
  uint64_t capture_ts_ns = 0;
  uint32_t orig_len = 0;
  std::vector<uint8_t> data;
};

class PcapReader {
 public:
  explicit PcapReader(const std::string& path);

  bool good() const;
  bool next(PcapRecord& record);

 private:
  std::ifstream file_;
};

}  // namespace otx

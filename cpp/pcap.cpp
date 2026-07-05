#include "pcap.hpp"

#include <array>
#include <cstring>

namespace otx {

namespace {

template <typename T>
bool read_value(std::ifstream& file, T& value) {
  file.read(reinterpret_cast<char*>(&value), sizeof(T));
  return file.good();
}

}  // namespace

PcapReader::PcapReader(const std::string& path) : file_(path, std::ios::binary) {
  if (!file_.good()) {
    return;
  }

  std::array<uint8_t, 24> global_header{};
  file_.read(reinterpret_cast<char*>(global_header.data()), global_header.size());
  if (!file_.good()) {
    file_.setstate(std::ios::failbit);
    return;
  }

  const uint32_t magic = *reinterpret_cast<const uint32_t*>(global_header.data());
  if (magic != 0xA1B2C3D4u) {
    file_.setstate(std::ios::failbit);
  }
}

bool PcapReader::good() const { return file_.good(); }

bool PcapReader::next(PcapRecord& record) {
  uint32_t ts_sec = 0;
  uint32_t ts_usec = 0;
  uint32_t incl_len = 0;
  uint32_t orig_len = 0;

  if (!read_value(file_, ts_sec)) {
    return false;
  }
  if (!read_value(file_, ts_usec) || !read_value(file_, incl_len) || !read_value(file_, orig_len)) {
    return false;
  }

  record.capture_ts_ns = static_cast<uint64_t>(ts_sec) * 1000000000ULL +
                         static_cast<uint64_t>(ts_usec) * 1000ULL;
  record.orig_len = orig_len;
  record.data.resize(incl_len);
  file_.read(reinterpret_cast<char*>(record.data.data()), static_cast<std::streamsize>(incl_len));
  return file_.good();
}

}  // namespace otx

#include <iostream>
#include <iostream>
#include <string>

#include "book.hpp"
#include "pcap.hpp"
#include "protocol.hpp"
#include "snapshot_writer.hpp"

namespace {

struct Options {
  std::string input_path;
  std::string output_path;
};

bool parse_args(int argc, char** argv, Options& options) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--input" && i + 1 < argc) {
      options.input_path = argv[++i];
    } else if (arg == "--output" && i + 1 < argc) {
      options.output_path = argv[++i];
    } else {
      return false;
    }
  }
  return !options.input_path.empty() && !options.output_path.empty();
}

}  // namespace

int main(int argc, char** argv) {
  Options options;
  if (!parse_args(argc, argv, options)) {
    std::cerr << "usage: build-book --input input.pcap --output output.csv\n";
    return 1;
  }

  otx::PcapReader reader(options.input_path);
  if (!reader.good()) {
    std::cerr << "failed to open pcap: " << options.input_path << '\n';
    return 1;
  }

  otx::SnapshotWriter writer(options.output_path);
  if (!writer.good()) {
    std::cerr << "failed to open output: " << options.output_path << '\n';
    return 1;
  }

  otx::BookBuilder builder;
  builder.set_snapshot_callback(writer.make_callback());

  otx::PcapRecord record;
  size_t packet_count = 0;
  size_t decoded_count = 0;

  while (reader.next(record)) {
    ++packet_count;

    const uint8_t* payload = nullptr;
    size_t payload_len = 0;
    if (!otx::decode_udp_payload(record.data, payload, payload_len)) {
      continue;
    }

    otx::DecodedPacket packet;
    if (!otx::parse_md_packet(payload, payload_len, packet)) {
      continue;
    }

    ++decoded_count;
    builder.handle_packet(packet);
  }

  std::cerr << "processed " << packet_count << " pcap records, decoded " << decoded_count
            << " market-data packets, wrote " << writer.row_count() << " rows\n";
  return 0;
}
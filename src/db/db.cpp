#include <fstream>

#include "db/db.hpp"
#include "format/format.hpp"
#include "gen/pb/block.pb.h"

namespace bcx {
  size_t LenBytes::size() const {
    return len.size();
  }

  size_t LenBytes::size_bytes() const {
    return len.size_bytes();
  }

  ds::String LenBytes::operator[](size_t i) const {
    return {bytes.data() + len.offset(i), len.size(i)};
  }

  void LenBytes::push_back(const std::string &str) {
    len.push_back(str.size());
    bytes.insert(bytes.end(), c2b(str.data()), c2b(str.data()) + str.size());
  }

  void LenBytes::truncate(size_t n) {
    len.truncate(n);
    bytes.resize(size_bytes());
  }
}  // namespace bcx

namespace bcx::db {
  constexpr auto kBlockCache = "block.cache";

  static LenBytes block_bytes;
  static size_t block_count;
  static size_t tx_count;
  static size_t account_count;
  static size_t peer_count;
  static std::ofstream appender;

  void truncate(size_t n) {
    block_bytes.truncate(n);
    std::ofstream(kBlockCache, std::ios::binary | std::ios::trunc)
        .write(b2c(block_bytes.bytes.data()), block_bytes.bytes.size());
  }

  void load() {
    auto load_start_time = std::chrono::system_clock::now();

    block_bytes.bytes = format::readBytes(kBlockCache);
    format::splitPb(block_bytes.len, block_bytes.bytes);
    auto extra = block_bytes.bytes.size() - block_bytes.size_bytes();
    if (extra) {
      logger::warn("Block cache corrupted, truncating {} bytes", extra);
      truncate(block_bytes.size());
    }
    iroha::protocol::Block block;
    for (auto i = 0u; i < block_bytes.size(); ++i) {
      if (!block.ParseFromString(block_bytes[i].str())) {
        logger::warn("Cached block {} corrupted, truncating {} blocks",
                     i + 1,
                     block_bytes.size() - i);
        truncate(i);
        break;
      }
      addBlock(block);
    }
    appender.open(kBlockCache, std::ios::binary | std::ios::app);

    auto load_duration = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now() - load_start_time);
    logger::info("Loaded {} blocks with {} transactions in {} sec",
                 blockCount(),
                 txCount(),
                 load_duration.count());
  }

  void addBlock(const iroha::protocol::Block &block) {
    auto height = format::blockHeight(block);
    if (height != block_count + 1) {
      fatal("Expected block {} got {}", block_count + 1, height);
    }
    if (height > block_bytes.size()) {
      auto bytes = block.SerializeAsString();
      block_bytes.push_back(bytes);
      appender.write(bytes.data(), bytes.size());
      appender.flush();
    }
    ++block_count;
    for (auto &tx_wrap : block.block_v1().payload().transactions()) {
      ++tx_count;
      for (auto &cmd : tx_wrap.payload().reduced_payload().commands()) {
        using iroha::protocol::Command;
        switch (cmd.command_case()) {
          case Command::kCreateAccount: {
            ++account_count;
            break;
          }
          case Command::kAddPeer: {
            ++peer_count;
            break;
          }
          default:
            break;
        }
      }
    }
  }

  size_t blockCount() {
    return block_count;
  }

  size_t txCount() {
    return tx_count;
  }

  size_t accountCount() {
    return account_count;
  }

  size_t peerCount() {
    return peer_count;
  }
}  // namespace bcx::db

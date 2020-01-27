#include "db/db.hpp"
#include "format/format.hpp"
#include "gen/pb/block.pb.h"

namespace bcx::db {
  static size_t block_count;
  static size_t tx_count;
  static size_t account_count;
  static size_t peer_count;

  void load() {
    auto load_start_time = std::chrono::system_clock::now();

    auto load_duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - load_start_time);
    logger::info("loaded {} blocks with {} transactions in {} sec", blockCount(), txCount(), load_duration.count());
  }

  void addBlock(const iroha::protocol::Block &block) {
    auto height = format::blockHeight(block);
    if (height != block_count + 1) {
      fatal("Expected block {} got {}", block_count + 1, height);
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

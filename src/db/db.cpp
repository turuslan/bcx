#include <fstream>

#include "db/db.hpp"
#include "format/format.hpp"
#include "gen/pb/block.pb.h"

namespace bcx::db {
  constexpr auto kBlockCache = "block.cache";

  static ds::Strings block_bytes;
  static size_t block_count;
  DEFINE_STATIC(block_hash);
  static size_t tx_count;
  DEFINE_STATIC(tx_hash);
  static size_t account_count;
  DEFINE_STATIC(account_id);
  static size_t peer_count;
  DEFINE_STATIC(peer_address);
  DEFINE_STATIC(peer_pub);
  static size_t role_count;
  DEFINE_STATIC(role_name);
  static size_t domain_count;
  DEFINE_STATIC(domain_id);
  DEFINE_STATIC(domain_role);
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
      auto block_span = block_bytes[i];
      if (!block.ParseFromArray(block_span.data(), block_span.size())) {
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
    auto &block_payload = block.block_v1().payload();
    block_hash.push_back(format::sha256(block_payload.SerializeAsString()));
    for (auto &tx_wrap : block_payload.transactions()) {
      ++tx_count;
      auto &tx_payload = tx_wrap.payload().reduced_payload();
      tx_hash.push_back(format::sha256(tx_payload.SerializeAsString()));
      for (auto &cmd : tx_payload.commands()) {
        using iroha::protocol::Command;
        switch (cmd.command_case()) {
          case Command::kCreateAccount: {
            auto &account = cmd.create_account();
            account_id.push_back(account.account_name() + "@" + account.domain_id());
            ++account_count;
            break;
          }
          case Command::kAddPeer: {
            ++peer_count;
            auto &peer = cmd.add_peer().peer();
            peer_address.push_back(peer.address());
            peer_pub.push_back(*format::unhex<EDKey>(peer.peer_key()));
            break;
          }
          case Command::kCreateRole: {
            ++role_count;
            auto role = cmd.create_role();
            role_name.push_back(role.role_name());
            break;
          }
          case Command::kCreateDomain: {
            ++domain_count;
            auto &domain = cmd.create_domain();
            domain_id.push_back(domain.domain_id());
            domain_role.push_back(*role_name.find(domain.default_role()));
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

  size_t roleCount() {
    return role_count;
  }

  size_t domainCount() {
    return domain_count;
  }
}  // namespace bcx::db

#include <fstream>

#include "db/db.hpp"
#include "format/format.hpp"
#include "gen/pb/block.pb.h"

namespace bcx::db {
  static_assert(iroha::protocol::Block::kBlockV1FieldNumber == 1);
  static_assert(iroha::protocol::Block_v1::kPayloadFieldNumber == 1);
  static_assert(iroha::protocol::Block_v1_Payload::kTransactionsFieldNumber == 1);
  static_assert(iroha::protocol::Transaction::kPayloadFieldNumber == 1);
  static_assert(iroha::protocol::Transaction_Payload::kReducedPayloadFieldNumber == 1);
  static_assert(iroha::protocol::Transaction_Payload_ReducedPayload::kCommandsFieldNumber == 1);

  constexpr auto kBlockCache = "block.cache";

  DEFINE_STATIC(block_bytes);
  static size_t block_count;
  DEFINE_STATIC(block_hash);
  DEFINE_STATIC(block_time);
  DEFINE_STATIC(block_tx_count);
  static size_t tx_count;
  DEFINE_STATIC(tx_hash);
  DEFINE_STATIC(tx_time);
  DEFINE_STATIC(tx_creator);
  DEFINE_STATIC(tx_pubs);
  DEFINE_STATIC(tx_cmds);
  static size_t account_count;
  DEFINE_STATIC(account_id);
  DEFINE_STATIC(account_quorum);
  DEFINE_STATIC(account_roles);
  DEFINE_STATIC(account_grant);
  static size_t peer_count;
  DEFINE_STATIC(peer_address);
  DEFINE_STATIC(peer_pub);
  static size_t role_count;
  DEFINE_STATIC(role_name);
  DEFINE_STATIC(role_perms);
  static size_t domain_count;
  DEFINE_STATIC(domain_id);
  DEFINE_STATIC(domain_role);
  DEFINE_STATIC(domain_tx_count);
  DEFINE_STATIC(all_pub);
  static std::ofstream appender;

  void truncate(size_t n) {
    block_bytes.truncate(n);
    std::ofstream(kBlockCache, std::ios::binary | std::ios::trunc)
        .write(b2c(block_bytes.bytes.data()), block_bytes.bytes.size());
  }

  void load() {
    auto load_start_time = std::chrono::system_clock::now();

    block_bytes.bytes = format::readBytes(kBlockCache);
    format::splitPb(block_bytes.len, bv2sv(block_bytes.bytes), iroha::protocol::Block::kBlockV1);
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

  auto txCreator(const iroha::protocol::Transaction_Payload_ReducedPayload &payload) {
    return *account_id.find(payload.creator_account_id());
  }

  auto txCreatorDomain(const iroha::protocol::Transaction_Payload_ReducedPayload &payload) {
    return *domain_id.find(format::domainOf(payload.creator_account_id()));
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
    auto block_i = block_count++;
    auto &block_payload = block.block_v1().payload();
    block_hash.push_back(format::blockHash(block));
    block_time.push_back(block_payload.created_time());
    block_tx_count.push_back(block_payload.transactions_size());
    format::getTxCmd(tx_cmds, block_bytes[block_i]);
    for (auto &tx_wrap : block_payload.transactions()) {
      auto tx_i = tx_count++;
      auto &tx_payload = tx_wrap.payload().reduced_payload();
      tx_hash.push_back(format::sha256(tx_payload.SerializeAsString()));
      tx_time.push_back(tx_payload.created_time());
      for (auto &sig : tx_wrap.signatures()) {
        auto pub = *format::unhex<EDKey>(sig.public_key());
        auto pub_i = all_pub.find(pub);
        if (!pub_i) {
          pub_i = all_pub.size();
          all_pub.push_back(pub);
        }
        tx_pubs.add(tx_i, *pub_i);
      }
      for (auto &cmd : tx_payload.commands()) {
        using iroha::protocol::Command;
        switch (cmd.command_case()) {
          case Command::kCreateAccount: {
            auto &account = cmd.create_account();
            auto account_i = account_count++;
            auto domain = account.domain_id();
            account_id.push_back(account.account_name() + "@" + domain);
            account_quorum.push_back(1);
            account_roles.add(account_i, domain_role[*domain_id.find(domain)]);
            break;
          }
          case Command::kAppendRole: {
            auto &append = cmd.append_role();
            account_roles.add(*account_id.find(append.account_id()), *role_name.find(append.role_name()));
            break;
          }
          case Command::kSetAccountQuorum: {
            auto &set = cmd.set_account_quorum();
            account_quorum[*account_id.find(set.account_id())] = set.quorum();
            break;
          }
          case Command::kGrantPermission: {
            auto &grant = cmd.grant_permission();
            auto to = *account_id.find(grant.account_id());
            auto by = txCreator(tx_payload);
            auto p = account_grant.find(GrantBimap::key_type{by, to});
            if (p == account_grant.end()) {
              p = account_grant.insert({by, to}).first;
            }
            p->info.set(grant.permission());
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
            RolePerms perms;
            for (auto &perm : role.permissions()) {
              perms.set(perm);
            }
            role_perms.push_back(perms);
            break;
          }
          case Command::kCreateDomain: {
            ++domain_count;
            auto &domain = cmd.create_domain();
            domain_id.push_back(domain.domain_id());
            domain_role.push_back(*role_name.find(domain.default_role()));
            domain_tx_count.push_back(0);
            break;
          }
          default:
            break;
        }
      }
      tx_creator.push_back(txCreator(tx_payload));
      ++domain_tx_count[txCreatorDomain(tx_payload)];
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

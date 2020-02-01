#ifndef BCX_DB_DB_HPP
#define BCX_DB_DB_HPP

#include "ds/ds.hpp"

namespace bcx::db {
  extern std::vector<Sha256> block_hash;
  extern ds::Indirect<false, std::vector<Sha256>>::Hashed tx_hash;
  extern ds::Indirect<true, ds::Strings>::Hashed account_id;
  extern ds::Strings peer_address;
  extern ds::Indirect<false, std::vector<EDKey>>::Hashed peer_pub;
  extern ds::Indirect<true, ds::Strings>::Hashed role_name;
  extern ds::Indirect<true, ds::Strings>::Hashed domain_id;
  extern std::vector<size_t> domain_role;

  void load();
  void addBlock(const iroha::protocol::Block &block);

  size_t blockCount();
  size_t txCount();
  size_t accountCount();
  size_t peerCount();
  size_t roleCount();
  size_t domainCount();
}  // namespace bcx::db

#endif  // BCX_DB_DB_HPP

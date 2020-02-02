#ifndef BCX_DB_DB_HPP
#define BCX_DB_DB_HPP

#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/range/iterator_range_core.hpp>

#include "ds/ds.hpp"

namespace bcx::db {
  using GrantBimap = boost::bimap<boost::bimaps::multiset_of<size_t>, boost::bimaps::multiset_of<size_t>, boost::bimaps::with_info<GrantPerms>>;
  extern std::vector<Sha256> block_hash;
  extern std::vector<uint64_t> block_time;
  extern ds::Len block_tx_count;
  extern ds::Indirect<false, std::vector<Sha256>>::Hashed tx_hash;
  extern std::vector<uint64_t> tx_time;
  extern std::vector<size_t> tx_creator;
  extern ds::Linked<size_t>::Vector tx_pubs;
  extern ds::Indirect<true, ds::Strings>::Hashed account_id;
  extern std::vector<size_t> account_quorum;
  extern ds::Linked<size_t>::Vector account_roles;
  extern GrantBimap account_grant;
  extern ds::Strings peer_address;
  extern ds::Indirect<false, std::vector<EDKey>>::Hashed peer_pub;
  extern ds::Indirect<true, ds::Strings>::Hashed role_name;
  extern std::vector<RolePerms> role_perms;
  extern ds::Indirect<true, ds::Strings>::Hashed domain_id;
  extern std::vector<size_t> domain_role;
  extern ds::Indirect<false, std::vector<EDKey>>::Hashed all_pub;

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

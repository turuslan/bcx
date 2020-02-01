#ifndef BCX_DB_DB_HPP
#define BCX_DB_DB_HPP

#include "ds/ds.hpp"

namespace bcx::db {
  void load();
  void addBlock(const iroha::protocol::Block &block);

  size_t blockCount();
  size_t txCount();
  size_t accountCount();
  size_t peerCount();
}  // namespace bcx::db

#endif  // BCX_DB_DB_HPP

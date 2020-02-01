#ifndef BCX_DB_DB_HPP
#define BCX_DB_DB_HPP

#include "ds/ds.hpp"

namespace bcx {
  struct LenBytes {
    size_t size() const;
    size_t size_bytes() const;
    ds::String operator[](size_t i) const;
    void push_back(const std::string &str);
    void truncate(size_t n);

    std::vector<Byte> bytes;
    ds::Len len;
  };
}  // namespace bcx

namespace bcx::db {
  void load();
  void addBlock(const iroha::protocol::Block &block);

  size_t blockCount();
  size_t txCount();
  size_t accountCount();
  size_t peerCount();
}  // namespace bcx::db

#endif  // BCX_DB_DB_HPP

#ifndef BCX_DB_DB_HPP
#define BCX_DB_DB_HPP

#include <vector>

#include "types.hpp"

namespace bcx {
  class LenIndex {
   public:
    LenIndex();
    size_t count() const;
    size_t size_bytes() const;
    size_t size(size_t i) const;
    size_t offset(size_t i) const;
    void push_back(size_t n);
    void truncate(size_t n);

   private:
    std::vector<size_t> offset_;
  };

  struct LenBytes {
    void push_back(const std::string &str);
    void truncate(size_t n);
    std::string str(size_t i);

    std::vector<Byte> bytes;
    LenIndex len;
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

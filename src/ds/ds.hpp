#ifndef BCX_DS_DS_HPP
#define BCX_DS_DS_HPP

#include <vector>

#include "types.hpp"

namespace bcx::ds {
  class Len {
   public:
    size_t size() const;
    size_t size_bytes() const;
    size_t size(size_t i) const;
    size_t offset(size_t i) const;
    void push_back(size_t n);
    void truncate(size_t n);

   private:
    std::vector<size_t> offset_{0};
  };

  class String {
   public:
    String(const Byte *ptr, size_t size);
    std::string str() const;

   private:
    const Byte *ptr_;
    size_t size_;
  };
}  // namespace bcx::ds

#endif  // BCX_DS_DS_HPP

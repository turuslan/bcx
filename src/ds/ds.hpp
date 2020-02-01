#ifndef BCX_DS_DS_HPP
#define BCX_DS_DS_HPP

#include <string_view>
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

  struct Strings {
    using value_type = std::string_view;

    size_t size() const;
    size_t size_bytes() const;
    std::string_view operator[](size_t i) const;
    void push_back(const std::string_view &str);
    void truncate(size_t n);

    std::vector<Byte> bytes;
    Len len;
  };
}  // namespace bcx::ds

#endif  // BCX_DS_DS_HPP

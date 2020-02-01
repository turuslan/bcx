#ifndef BCX_TYPES_HPP
#define BCX_TYPES_HPP

#include <array>

#define DEFINE_STATIC(x) decltype(x) x

namespace iroha::protocol {
  class Block;
}  // namespace iroha::protocol

namespace bcx {
  using Byte = uint8_t;
  using EDKey = std::array<Byte, 32>;
  using Sha256 = std::array<Byte, 32>;

  inline auto b2c(Byte *ptr) {
    return reinterpret_cast<char *>(ptr);
  }

  inline auto b2c(const Byte *ptr) {
    return reinterpret_cast<const char *>(ptr);
  }

  inline auto c2b(char *ptr) {
    return reinterpret_cast<Byte *>(ptr);
  }

  inline auto c2b(const char *ptr) {
    return reinterpret_cast<const Byte *>(ptr);
  }
}  // namespace bcx

#endif  // BCX_TYPES_HPP

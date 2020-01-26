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
}  // namespace bcx

#endif  // BCX_TYPES_HPP

#ifndef BCX_TYPES_HPP
#define BCX_TYPES_HPP

#include <array>
#include <bitset>

#define DEFINE_STATIC(x) decltype(x) x

namespace iroha::protocol {
  class Block;
}  // namespace iroha::protocol

namespace bcx {
  using Byte = uint8_t;
  using EDKey = std::array<Byte, 32>;
  using Sha256 = std::array<Byte, 32>;

  constexpr size_t kRolePermsBits = 48;
  using RolePerms = std::bitset<kRolePermsBits>;
  constexpr size_t kGrantPermsBits = 5;
  using GrantPerms = std::bitset<kGrantPermsBits>;

  struct Grant {
    size_t by, to, perm;
  };

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

  inline auto bv2sv(const std::vector<Byte> &bytes) {
    return std::string_view{b2c(bytes.data()), bytes.size()};
  }
}  // namespace bcx

#endif  // BCX_TYPES_HPP

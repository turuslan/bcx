#ifndef BCX_FORMAT_FORMAT_HPP
#define BCX_FORMAT_FORMAT_HPP

#include <spdlog/spdlog.h>
#include <optional>

#include "types.hpp"

namespace bcx {
  namespace ds {
    struct Len;
  }  // namespace ds

  namespace format {
    bool unhex(const std::string &str, Byte *out);

    template <typename C>
    std::optional<C> unhex(const std::string &str) {
      C c;
      if (str.size() != std::size(c) * 2 || !unhex(str, std::data(c))) {
        return std::nullopt;
      }
      return c;
    }

    std::string hex(const Byte *begin, size_t size);

    template <typename C>
    std::string hex(const C &bytes) {
      return hex(std::data(bytes), std::size(bytes));
    }

    Sha256 sha256(const std::string &bytes);

    std::string domainOf(const std::string &account);

    size_t blockHeight(const iroha::protocol::Block &block);

    std::vector<Byte> readBytes(const std::string &path);

    std::string readText(const std::string &path);

    void splitPb(ds::Len &len, const std::vector<Byte> &bytes);

    std::string rolePermName(size_t i);

    std::vector<std::string> rolePermNames(const RolePerms &perms);

    std::string grantPermName(size_t i);

    std::string timeToIso(uint64_t time);

    std::optional<uint64_t> isoToTime(const std::string &str);
  }  // namespace format

  namespace logger = spdlog;

  template <typename... T>
  void fatal(T &&... args) {
    logger::error(std::forward<T>(args)...);
    exit(-1);
  }

  bool setLogLevel(const std::string &str);

  struct Config {
    struct Iroha {
      std::string host;
      std::string account;
      EDKey private_key;
    };

    void load();
    bool disable_sync() const;

    std::optional<Iroha> iroha;
  };

  extern Config config;
}  // namespace bcx

#endif  // BCX_FORMAT_FORMAT_HPP

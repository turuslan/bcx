#ifndef BCX_FORMAT_FORMAT_HPP
#define BCX_FORMAT_FORMAT_HPP

#include <spdlog/spdlog.h>
#include <optional>

#include "types.hpp"

namespace bcx {
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
  }  // namespace format

  namespace logger = spdlog;

  template <typename ...T>
  void fatal(T &&...args) {
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

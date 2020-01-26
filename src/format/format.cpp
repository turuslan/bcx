#include <ed25519/ed25519/sha256.h>
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdlib>

#include "gen/pb/block.pb.h"
#include "format/format.hpp"

namespace bcx {
  namespace format {
    bool unhex(const std::string &str, Byte *out) {
      try {
        boost::algorithm::unhex(str.begin(), str.end(), out);
        return true;
      } catch (const boost::algorithm::hex_decode_error &) {
        return false;
      }
    }

    std::string hex(const Byte *begin, size_t size) {
      std::string res(size * 2, '\0');
      boost::algorithm::hex_lower(begin, begin + size, res.begin());
      return res;
    }

    Sha256 sha256(const std::string &bytes) {
      Sha256 res;
      ::sha256(
          res.data(), reinterpret_cast<const Byte *>(bytes.data()), bytes.size());
      return res;
    }

    size_t blockHeight(const iroha::protocol::Block &block) {
      return block.block_v1().payload().height();
    }
  }  // namespace format

  const std::map<std::string, logger::level::level_enum> kLogLevels{
      {"error", logger::level::err},
      {"warn", logger::level::warn},
      {"info", logger::level::info},
      {"debug", logger::level::debug},
  };

  bool setLogLevel(const std::string &str) {
    auto value = kLogLevels.find(boost::algorithm::to_lower_copy(str));
    if (value == kLogLevels.end()) {
      return false;
    }
    logger::set_level(value->second);
    return true;
  }

  inline std::string getenv(const char *name, const char *fallback) {
    auto value = std::getenv(name);
    return value ? value : fallback;
  }

  inline std::string getenv(const char *name) {
    auto value = std::getenv(name);
    if (!value) {
      fatal("Environment {} is required", name);
    }
    return value;
  }

  void Config::load() {
    auto log_level = getenv("LOG_LEVEL", "info");
    if (!setLogLevel(log_level)) {
      fatal("Unknown LOG_LEVEL={}", log_level);
    }

    if (getenv("DISABLE_SYNC", "0") != "1") {
      auto iroha_account_key = format::unhex<EDKey>(getenv("IROHA_ACCOUNT_KEY"));
      if (!iroha_account_key) {
        fatal("Invalid IROHA_ACCOUNT_KEY format", log_level);
      }
      iroha = Iroha{
          getenv("IROHA_HOST"),
          getenv("IROHA_ACCOUNT"),
          *iroha_account_key,
      };
    }
  }

  bool Config::disable_sync() const {
    return !iroha;
  }

  DEFINE_STATIC(config);
}  // namespace bcx

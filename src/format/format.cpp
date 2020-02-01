#include <ed25519/ed25519/sha256.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/util/time_util.h>
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <fstream>

#include "db/db.hpp"
#include "ds/ds.hpp"
#include "format/format.hpp"
#include "gen/pb/block.pb.h"

static_assert(bcx::kRolePermsBits == iroha::protocol::RolePermission_ARRAYSIZE);

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
      ::sha256(res.data(), c2b(bytes.data()), bytes.size());
      return res;
    }

    size_t blockHeight(const iroha::protocol::Block &block) {
      return block.block_v1().payload().height();
    }

    template <typename C>
    C read(const std::string &path) {
      std::ifstream file(path, std::ios::binary | std::ios::ate);
      C res;
      if (file.good()) {
        res.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(b2c(std::data(res)), std::size(res));
      }
      return res;
    }

    std::vector<Byte> readBytes(const std::string &path) {
      return read<std::vector<Byte>>(path);
    }

    void splitPb(ds::Len &len, const std::vector<Byte> &bytes) {
      len = ds::Len{};
      google::protobuf::io::CodedInputStream stream{
          bytes.data(), static_cast<int>(bytes.size())};
      while (true) {
        if (!stream.ReadTag()) {
          break;
        }
        int skip;
        if (!stream.ReadVarintSizeAsInt(&skip)) {
          break;
        }
        if (!stream.Skip(skip)) {
          break;
        }
        len.push_back(stream.CurrentPosition() - len.size_bytes());
      }
    }

    std::string rolePermName(size_t i) {
      return iroha::protocol::RolePermission_Name(i);
    }

    std::vector<std::string> rolePermNames(const RolePerms &perms) {
      std::vector<std::string> names;
      for (auto i = 0; i < perms.size(); ++i) {
        if (perms.test(i)) {
          names.push_back(rolePermName(i));
        }
      }
      return names;
    }

    using google::protobuf::util::TimeUtil;

    std::string timeToIso(uint64_t time) {
      return TimeUtil::ToString(TimeUtil::MillisecondsToTimestamp(time));
    }

    std::optional<uint64_t> isoToTime(const std::string &str) {
      google::protobuf::Timestamp ts;
      if (TimeUtil::FromString(str, &ts)) {
        return TimeUtil::TimestampToMilliseconds(ts);
      }
      return std::nullopt;
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
      auto iroha_account_key =
          format::unhex<EDKey>(getenv("IROHA_ACCOUNT_KEY"));
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

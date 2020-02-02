#include <ed25519/ed25519/sha256.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/util/json_util.h>
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
static_assert(bcx::kGrantPermsBits == iroha::protocol::GrantablePermission_ARRAYSIZE);

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

    std::string domainOf(const std::string &account) {
      return account.substr(account.find_first_of('@') + 1);
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
        file.read(reinterpret_cast<char *>(std::data(res)), std::size(res));
      }
      return res;
    }

    std::vector<Byte> readBytes(const std::string &path) {
      return read<std::vector<Byte>>(path);
    }

    std::string readText(const std::string &path) {
      return read<std::string>(path);
    }

    struct Pb {
      Pb(std::string_view bytes) : stream{c2b(bytes.data()), static_cast<int>(bytes.size())} {}

      bool next(int field, int &size) {
        auto tag = stream.ReadTag();
        if (!tag || (tag >> 3) != field || !stream.ReadVarintSizeAsInt(&size)) {
          return false;
        }
        if (tag & 2) {
          if (!stream.Skip(size)) {
            return false;
          }
        } else {
          size = 0;
        }
        return true;
      }

      bool next(int field) {
        int size;
        return next(field, size);
      }

      bool next(int field, std::string_view &bytes) {
        int size;
        const void *data;
        stream.GetDirectBufferPointer(&data, &size);
        auto offset = stream.CurrentPosition();
        if (!next(field, size)) {
          return false;
        }
        bytes = {static_cast<const char *>(data) + stream.CurrentPosition() - offset - size, static_cast<size_t>(size)};
        return true;
      }

      google::protobuf::io::CodedInputStream stream;
    };

    void splitPb(ds::Len &len, std::string_view bytes, int field) {
      len = ds::Len{};
      Pb pb{bytes};
      while (pb.next(field)) {
        len.push_back(pb.stream.CurrentPosition() - len.size_bytes());
      }
    }

    std::string rolePermName(size_t i) {
      return iroha::protocol::RolePermission_Name(i);
    }

    std::vector<std::string> rolePermNames(const RolePerms &perms) {
      std::vector<std::string> names;
      for (auto i = 0u; i < perms.size(); ++i) {
        if (perms.test(i)) {
          names.push_back(rolePermName(i));
        }
      }
      return names;
    }

    std::string grantPermName(size_t i) {
      return iroha::protocol::GrantablePermission_Name(i);
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

    void getTxCmd(std::vector<std::pair<size_t, size_t>> &result, std::string_view block) {
      std::string_view block_v1, block_payload, tx, tx_payload, tx_reduced_payload, cmd;
      Pb{block}.next(iroha::protocol::Block::kBlockV1FieldNumber, block_v1);
      Pb{block_v1}.next(iroha::protocol::Block_v1::kPayloadFieldNumber, block_payload);
      Pb pb_block{block_payload};
      while (pb_block.next(iroha::protocol::Block_v1_Payload::kTransactionsFieldNumber, tx)) {
        Pb{tx}.next(iroha::protocol::Transaction::kPayloadFieldNumber, tx_payload);
        Pb{tx}.next(iroha::protocol::Transaction_Payload::kReducedPayloadFieldNumber, tx_reduced_payload);
        Pb pb_tx{tx_reduced_payload};
        std::string_view tx_cmd;
        while (pb_tx.next(iroha::protocol::Transaction_Payload_ReducedPayload::kCommandsFieldNumber, cmd)) {
          if (!tx_cmd.data()) {
            tx_cmd = cmd;
          } else {
            tx_cmd = {tx_cmd.data(), tx_cmd.size() + cmd.size()};
          }
        }
        result.push_back({tx_cmd.data() - block.data(), tx_cmd.size()});
      }
    }

    std::string txCmdJson(std::string_view bytes) {
      std::string result;
      result.push_back('[');

      Pb pb{bytes};
      std::string_view cmd_bytes;
      iroha::protocol::Command cmd;
      while (pb.next(iroha::protocol::Transaction_Payload_ReducedPayload::kCommandsFieldNumber, cmd_bytes)) {
        cmd.ParseFromArray(cmd_bytes.data(), cmd_bytes.size());
        google::protobuf::util::MessageToJsonString(cmd, &result);
        result.push_back(',');
      }

      result.pop_back();
      result.push_back(']');
      return result;
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

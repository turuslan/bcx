#include "gql/impl.hpp"

std::chrono::milliseconds truncNow(std::chrono::seconds step) {
  auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  auto tm = *localtime(&time);
  if (step == std::chrono::minutes(1)) {
    tm.tm_sec = 0;
  } else if (step == std::chrono::hours(1)) {
    tm.tm_sec = 0;
    tm.tm_min = 0;
  }
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::from_time_t(mktime(&tm)).time_since_epoch());
}

std::vector<int> countPerTime(const std::vector<uint64_t> times, std::chrono::seconds step, size_t steps) {
  std::vector<int> buckets(steps, 0);
  auto cut = truncNow(step);
  auto bucket = buckets.rbegin();
  auto time = times.rbegin();
  auto stop = false;
  while (true) {
    while (true) {
      stop = time == times.rend() || bucket == buckets.rend();
      if (stop) {
        break;
      }
      if (*time >= cut.count()) {
        break;
      } else {
        ++bucket;
        cut -= step;
      }
    }
    if (stop) {
      break;
    }
    ++(*bucket);
    ++time;
  }
  return buckets;
}

namespace graphql::bcx {
  FieldResult<IntType> Query::getBlockCount(FieldParams&& params) const {
    return db::blockCount();
  }

  FieldResult<IntType> Query::getTransactionCount(FieldParams&& params) const {
    return db::txCount();
  }

  FieldResult<IntType> Query::getAccountCount(FieldParams&& params) const {
    return db::accountCount();
  }

  FieldResult<IntType> Query::getPeerCount(FieldParams&& params) const {
    return db::peerCount();
  }

  FieldResult<IntType> Query::getRoleCount(FieldParams&& params) const {
    return db::roleCount();
  }

  FieldResult<IntType> Query::getDomainCount(FieldParams&& params) const {
    return db::domainCount();
  }

  FieldResult<std::vector<IntType>> Query::getTransactionCountPerMinute(FieldParams&& params, IntType&& countArg) const {
    return countPerTime(db::tx_time, std::chrono::minutes(1), countArg);
  }

  FieldResult<std::vector<IntType>> Query::getTransactionCountPerHour(FieldParams&& params, IntType&& countArg) const {
    return countPerTime(db::tx_time, std::chrono::hours(1), countArg);
  }

  FieldResult<std::vector<IntType>> Query::getBlockCountPerMinute(FieldParams&& params, IntType&& countArg) const {
    return countPerTime(db::block_time, std::chrono::minutes(1), countArg);
  }

  FieldResult<std::vector<IntType>> Query::getBlockCountPerHour(FieldParams&& params, IntType&& countArg) const {
    return countPerTime(db::block_time, std::chrono::hours(1), countArg);
  }
}  // namespace graphql::bcx

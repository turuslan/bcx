#include "gql/impl.hpp"

namespace graphql::bcx {
  auto txBlock(size_t i) {
    return db::block_tx_count.index(i);
  }

  FieldResult<StringType> Transaction::getHash(FieldParams&& params) const {
    return format::hex(db::tx_hash[i]);
  }

  FieldResult<std::shared_ptr<object::Account>> Transaction::getCreatedBy(FieldParams&& params) const {
    return std::make_shared<Account>(db::tx_creator[i]);
  }

  FieldResult<StringType> Transaction::getTime(FieldParams&& params) const {
    return format::timeToIso(db::tx_time[i]);
  }

  FieldResult<IntType> Transaction::getBlockHeight(FieldParams&& params) const {
    return txBlock(i) + 1;
  }

  FieldResult<std::vector<StringType>> Transaction::getSignatories(FieldParams&& params) const {
    std::vector<std::string> items;
    for (auto j : db::tx_pubs.range(i)) {
      items.push_back(format::hex(db::all_pub[j]));
    }
    return items;
  }

  FieldResult<StringType> Transaction::getCommandsJson(FieldParams&& params) const {
    auto cmd = db::tx_cmds[i];
    return format::txCmdJson({db::block_bytes[txBlock(i)].data() + cmd.first, cmd.second});
  }

  FieldResult<std::shared_ptr<object::Transaction>> Query::getTransactionByHash(FieldParams&& params, StringType&& hashArg) const {
    auto hash = format::unhex<Sha256>(hashArg);
    if (!hash) {
      return nullptr;
    }
    auto i = db::tx_hash.find(*hash);
    return i ? std::make_shared<Transaction>(*i) : nullptr;
  }

  FieldResult<std::shared_ptr<object::TransactionList>> Query::getTransactionList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg, std::optional<StringType>&& timeAfterArg, std::optional<StringType>&& timeBeforeArg, std::optional<StringType>&& creatorIdArg) const {
    // TODO: creatorIdArg
    std::vector<size_t> iv;
    auto time_after = timeAfterArg ? format::isoToTime(*timeAfterArg) : std::nullopt;
    auto time_before = timeBeforeArg ? format::isoToTime(*timeBeforeArg) : std::nullopt;
    auto after = afterArg ? *afterArg : -1;
    auto i = after + 1;
    if (time_after) {
      i = std::max(i, static_cast<int>(std::lower_bound(db::tx_time.begin() + i, db::tx_time.end(), *time_after) - db::tx_time.begin()));
    }
    while (i >= 0 && i < db::txCount() && iv.size() < countArg && (!time_before || db::tx_time[i] < *time_before)) {
      iv.push_back(i);
      ++i;
    }
    return TransactionList::make(std::move(iv), after);
  }
}  // namespace graphql::bcx

#include "gql/impl.hpp"

namespace graphql::bcx {
  FieldResult<IntType> Block::getHeight(FieldParams&& params) const {
    return i + 1;
  }

  FieldResult<StringType> Block::getHash(FieldParams&& params) const {
    return format::hex(db::block_hash[i]);
  }

  FieldResult<IntType> Block::getTransactionCount(FieldParams&& params) const {
    return db::block_tx_count.size(i);
  }

  FieldResult<StringType> Block::getTime(FieldParams&& params) const {
    return format::timeToIso(db::block_time[i]);
  }

  FieldResult<std::vector<std::shared_ptr<object::Transaction>>> Block::getTransactions(FieldParams&& params) const {
    std::vector<std::shared_ptr<object::Transaction>> items;
    auto end = db::block_tx_count.offset(i + 1);
    for (auto j = db::block_tx_count.offset(i); j < end; ++j) {
      items.push_back(std::make_shared<Transaction>(j));
    }
    return items;
  }

  FieldResult<StringType> Block::getPreviousBlockHash(FieldParams&& params) const {
    return i == 0 ? "-" : format::hex(db::block_hash[i - 1]);
  }

  FieldResult<std::shared_ptr<object::Block>> Query::getBlockByHeight(FieldParams&& params, IntType&& heightArg) const {
    return heightArg <= 0 || heightArg > db::blockCount() ? nullptr : std::make_shared<Block>(heightArg - 1);
  }

  FieldResult<std::shared_ptr<object::BlockList>> Query::getBlockList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg, std::optional<BooleanType>&& reverseArg, std::optional<StringType>&& timeAfterArg, std::optional<StringType>&& timeBeforeArg) const {
    std::vector<size_t> iv;
    auto time_after = timeAfterArg ? format::isoToTime(*timeAfterArg) : std::nullopt;
    auto time_before = timeBeforeArg ? format::isoToTime(*timeBeforeArg) : std::nullopt;
    auto blocks_total = static_cast<int>(db::blockCount());
    auto reverse = reverseArg && *reverseArg;
    auto step = reverse ? -1 : 1;
    auto after = afterArg ? *afterArg : reverse ? blocks_total : -1;
    auto i = after + step;
    if (reverse) {
      if (time_before) {
        i = std::min(i, static_cast<int>(std::lower_bound(db::block_time.begin(), db::block_time.begin() + i, *time_before) - db::block_time.begin()) - 1);
      }
    } else {
      if (time_after) {
        i = std::max(i, static_cast<int>(std::lower_bound(db::block_time.begin() + i, db::block_time.end(), *time_after) - db::block_time.begin()));
      }
    }
    while (i >= 0 && i < blocks_total && iv.size() < countArg && (reverse ? !time_after || db::block_time[i] >= *time_after : !time_before || db::block_time[i] < *time_before)) {
      iv.push_back(i);
      i += step;
    }
    return BlockList::make(std::move(iv), after);
  }
}  // namespace graphql::bcx

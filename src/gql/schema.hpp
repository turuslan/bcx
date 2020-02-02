#ifndef BCX_GQL_SCHEMA_HPP
#define BCX_GQL_SCHEMA_HPP

#include <string>

#include "gen/gql/BcxSchema.h"
#include "types.hpp"

namespace bcx {
  std::string gql(const std::string &body);
}  // namespace bcx

namespace graphql::bcx {
  using service::FieldResult;
  using service::FieldParams;
  using response::IntType;
  using response::StringType;
  using response::BooleanType;

  template <typename ListBase, typename ItemBase, typename Item>
  struct List : ListBase {
    static_assert(std::is_base_of_v<ItemBase, Item>);

    inline List(std::vector<size_t> &&iv, size_t after) : iv{std::move(iv)}, after{after} {}

    FieldResult<std::vector<std::shared_ptr<ItemBase>>> getItems(FieldParams&& params) const override {
      std::vector<std::shared_ptr<ItemBase>> items;
      for (auto i : iv) {
        items.push_back(std::make_shared<Item>(i));
      }
      return items;
    }

    FieldResult<IntType> getNextAfter(FieldParams&& params) const override {
      return after;
    }

    static auto make(std::vector<size_t> &&iv, size_t after) {
      if (!iv.empty()) {
        after = iv.back();
      }
      return std::make_shared<List>(std::move(iv), after);
    }

    std::vector<size_t> iv;
    size_t after;
  };

  class Block;
  class Transaction;
  class Account;
  class PermissionGranted;
  class Peer;
  class Role;
  class Domain;
  class CountPerDomain;
  class Query;

  using BlockList = List<object::BlockList, object::Block, Block>;
  using TransactionList = List<object::TransactionList, object::Transaction, Transaction>;
  using AccountList = List<object::AccountList, object::Account, Account>;
  using PeerList = List<object::PeerList, object::Peer, Peer>;
  using RoleList = List<object::RoleList, object::Role, Role>;
  using DomainList = List<object::DomainList, object::Domain, Domain>;

  using ::bcx::Grant;

  struct Block : object::Block {
    inline Block(size_t i) : i{i} {}

    FieldResult<IntType> getHeight(FieldParams&& params) const override;
    FieldResult<StringType> getHash(FieldParams&& params) const override;
    FieldResult<IntType> getTransactionCount(FieldParams&& params) const override;
    FieldResult<StringType> getTime(FieldParams&& params) const override;
    FieldResult<std::vector<std::shared_ptr<object::Transaction>>> getTransactions(FieldParams&& params) const override;
    FieldResult<StringType> getPreviousBlockHash(FieldParams&& params) const override;

    size_t i;
  };

  struct Transaction : object::Transaction {
    inline Transaction(size_t i) : i{i} {}

    FieldResult<StringType> getHash(FieldParams&& params) const override;
    FieldResult<std::shared_ptr<object::Account>> getCreatedBy(FieldParams&& params) const override;
    FieldResult<StringType> getTime(FieldParams&& params) const override;
    FieldResult<IntType> getBlockHeight(FieldParams&& params) const override;
    FieldResult<std::vector<StringType>> getSignatories(FieldParams&& params) const override;
    FieldResult<StringType> getCommandsJson(FieldParams&& params) const override;

    size_t i;
  };

  struct Account : object::Account {
    inline Account(size_t i) : i{i} {}

    FieldResult<StringType> getId(FieldParams&& params) const override;
    FieldResult<IntType> getQuorum(FieldParams&& params) const override;
    FieldResult<std::vector<std::shared_ptr<object::Role>>> getRoles(FieldParams&& params) const override;
    FieldResult<std::vector<StringType>> getPermissions(FieldParams&& params) const override;
    FieldResult<std::vector<std::shared_ptr<object::PermissionGranted>>> getPermissionsGrantedBy(FieldParams&& params) const override;
    FieldResult<std::vector<std::shared_ptr<object::PermissionGranted>>> getPermissionsGrantedTo(FieldParams&& params) const override;

    size_t i;
  };

  struct PermissionGranted : object::PermissionGranted {
    inline PermissionGranted(Grant grant) : grant{grant} {}

    FieldResult<StringType> getBy(FieldParams&& params) const override;
    FieldResult<StringType> getTo(FieldParams&& params) const override;
    FieldResult<StringType> getPermission(FieldParams&& params) const override;

    Grant grant;
  };

  struct Peer : object::Peer {
    inline Peer(size_t i) : i{i} {}

    FieldResult<StringType> getAddress(FieldParams&& params) const override;
    FieldResult<StringType> getPublicKey(FieldParams&& params) const override;

    size_t i;
  };

  struct Role : object::Role {
    inline Role(size_t i) : i{i} {}

    FieldResult<StringType> getName(FieldParams&& params) const override;
    FieldResult<std::vector<StringType>> getPermissions(FieldParams&& params) const override;

    size_t i;
  };

  struct Domain : object::Domain {
    inline Domain(size_t i) : i{i} {}

    FieldResult<StringType> getId(FieldParams&& params) const override;
    FieldResult<std::shared_ptr<object::Role>> getDefaultRole(FieldParams&& params) const override;

    size_t i;
  };

  struct CountPerDomain : object::CountPerDomain {
    inline CountPerDomain(size_t i) : i{i} {}

    FieldResult<StringType> getDomain(FieldParams&& params) const override;
    FieldResult<IntType> getCount(FieldParams&& params) const override;

    size_t i;
  };

  struct Query : object::Query {
    FieldResult<IntType> getBlockCount(FieldParams&& params) const override;
    FieldResult<IntType> getTransactionCount(FieldParams&& params) const override;
    FieldResult<IntType> getAccountCount(FieldParams&& params) const override;
    FieldResult<IntType> getPeerCount(FieldParams&& params) const override;
    FieldResult<IntType> getRoleCount(FieldParams&& params) const override;
    FieldResult<IntType> getDomainCount(FieldParams&& params) const override;
    FieldResult<std::shared_ptr<object::Block>> getBlockByHeight(FieldParams&& params, IntType&& heightArg) const override;
    FieldResult<std::shared_ptr<object::Transaction>> getTransactionByHash(FieldParams&& params, StringType&& hashArg) const override;
    FieldResult<std::shared_ptr<object::Account>> getAccountById(FieldParams&& params, StringType&& idArg) const override;
    FieldResult<std::shared_ptr<object::Peer>> getPeerByPublicKey(FieldParams&& params, StringType&& publicKeyArg) const override;
    FieldResult<std::shared_ptr<object::Role>> getRoleByName(FieldParams&& params, StringType&& nameArg) const override;
    FieldResult<std::shared_ptr<object::Domain>> getDomainById(FieldParams&& params, StringType&& idArg) const override;
    FieldResult<std::shared_ptr<object::BlockList>> getBlockList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg, std::optional<BooleanType>&& reverseArg, std::optional<StringType>&& timeAfterArg, std::optional<StringType>&& timeBeforeArg) const override;
    FieldResult<std::shared_ptr<object::TransactionList>> getTransactionList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg, std::optional<StringType>&& timeAfterArg, std::optional<StringType>&& timeBeforeArg, std::optional<StringType>&& creatorIdArg) const override;
    FieldResult<std::shared_ptr<object::AccountList>> getAccountList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg, std::optional<StringType>&& idArg) const override;
    FieldResult<std::shared_ptr<object::PeerList>> getPeerList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg) const override;
    FieldResult<std::shared_ptr<object::RoleList>> getRoleList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg) const override;
    FieldResult<std::shared_ptr<object::DomainList>> getDomainList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg) const override;
    FieldResult<std::vector<IntType>> getTransactionCountPerMinute(FieldParams&& params, IntType&& countArg) const override;
    FieldResult<std::vector<IntType>> getTransactionCountPerHour(FieldParams&& params, IntType&& countArg) const override;
    FieldResult<std::vector<IntType>> getBlockCountPerMinute(FieldParams&& params, IntType&& countArg) const override;
    FieldResult<std::vector<IntType>> getBlockCountPerHour(FieldParams&& params, IntType&& countArg) const override;
    FieldResult<std::vector<std::shared_ptr<object::CountPerDomain>>> getTransactionCountPerDomain(FieldParams&& params) const override;
  };
}  // namespace graphql::bcx

#endif  // BCX_GQL_SCHEMA_HPP

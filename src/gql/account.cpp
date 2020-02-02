#include "gql/impl.hpp"

namespace graphql::bcx {
  template <bool swap, typename C>
  auto permissionsGranted(size_t i, const C &map) {
    std::vector<std::shared_ptr<object::PermissionGranted>> items;
    for (auto &x : boost::make_iterator_range(map.equal_range(i))) {
      auto by = swap ? x.second : x.first;
      auto to = swap ? x.first : x.second;
      for (auto j = 0u; j < x.info.size(); ++j) {
        if (x.info.test(j)) {
          items.push_back(std::make_shared<PermissionGranted>(Grant{by, to, j}));
        }
      }
    }
    return items;
  }

  FieldResult<StringType> Account::getId(FieldParams&& params) const {
    return std::string{db::account_id[i]};
  }

  FieldResult<IntType> Account::getQuorum(FieldParams&& params) const {
    return db::account_quorum[i];
  }

  FieldResult<std::vector<std::shared_ptr<object::Role>>> Account::getRoles(FieldParams&& params) const {
    std::vector<std::shared_ptr<object::Role>> items;
    for (auto j : db::account_roles.range(i)) {
      items.push_back(std::make_shared<Role>(j));
    }
    return items;
  }

  FieldResult<std::vector<StringType>> Account::getPermissions(FieldParams&& params) const {
    RolePerms perms;
    for (auto j : db::account_roles.range(i)) {
      perms |= db::role_perms[j];
    }
    return format::rolePermNames(perms);
  }

  FieldResult<std::vector<std::shared_ptr<object::PermissionGranted>>> Account::getPermissionsGrantedBy(FieldParams&& params) const {
    return permissionsGranted<false>(i, db::account_grant.left);
  }

  FieldResult<std::vector<std::shared_ptr<object::PermissionGranted>>> Account::getPermissionsGrantedTo(FieldParams&& params) const {
    return permissionsGranted<true>(i, db::account_grant.right);
  }

  FieldResult<std::shared_ptr<object::Account>> Query::getAccountById(FieldParams&& params, StringType&& idArg) const {
    auto i = db::account_id.find(idArg);
    return i ? std::make_shared<Account>(*i) : nullptr;
  }

  FieldResult<std::shared_ptr<object::AccountList>> Query::getAccountList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg, std::optional<StringType>&& idArg) const {
    // TODO: idArg
    std::vector<size_t> iv;
    auto after = afterArg ? *afterArg : -1;
    for (auto i = after + 1; i >= 0 && i < db::accountCount() && iv.size() < countArg; ++i) {
      iv.push_back(i);
    }
    return AccountList::make(std::move(iv), after);
  }
}  // namespace graphql::bcx

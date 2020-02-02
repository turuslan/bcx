#include "gql/impl.hpp"

namespace graphql::bcx {
  FieldResult<StringType> PermissionGranted::getBy(FieldParams&& params) const {
    return std::string{db::account_id[grant.by]};
  }

  FieldResult<StringType> PermissionGranted::getTo(FieldParams&& params) const {
    return std::string{db::account_id[grant.to]};
  }

  FieldResult<StringType> PermissionGranted::getPermission(FieldParams&& params) const {
    return format::grantPermName(grant.perm);
  }

  FieldResult<StringType> Role::getName(FieldParams&& params) const {
    return std::string{db::role_name[i]};
  }

  FieldResult<std::vector<StringType>> Role::getPermissions(FieldParams&& params) const {
    return format::rolePermNames(db::role_perms[i]);
  }

  FieldResult<std::shared_ptr<object::Role>> Query::getRoleByName(FieldParams&& params, StringType&& nameArg) const {
    auto i = db::role_name.find(nameArg);
    return i ? std::make_shared<Role>(*i) : nullptr;
  }

  FieldResult<std::shared_ptr<object::RoleList>> Query::getRoleList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg) const {
    std::vector<size_t> iv;
    auto after = afterArg ? *afterArg : -1;
    for (auto i = after + 1; i >= 0 && i < db::roleCount() && iv.size() < countArg; ++i) {
      iv.push_back(i);
    }
    return RoleList::make(std::move(iv), after);
  }
}  // namespace graphql::bcx

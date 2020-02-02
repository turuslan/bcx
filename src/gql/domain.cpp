#include "gql/impl.hpp"

namespace graphql::bcx {
  FieldResult<StringType> Domain::getId(FieldParams&& params) const {
    return std::string{db::domain_id[i]};
  }

  FieldResult<std::shared_ptr<object::Role>> Domain::getDefaultRole(FieldParams&& params) const {
    return std::make_shared<Role>(db::domain_role[i]);
  }

  FieldResult<std::shared_ptr<object::Domain>> Query::getDomainById(FieldParams&& params, StringType&& idArg) const {
    auto i = db::domain_id.find(idArg);
    return i ? std::make_shared<Domain>(*i) : nullptr;
  }

  FieldResult<std::shared_ptr<object::DomainList>> Query::getDomainList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg) const {
    std::vector<size_t> iv;
    auto after = afterArg ? *afterArg : -1;
    for (auto i = after + 1; i >= 0 && i < db::domainCount() && iv.size() < countArg; ++i) {
      iv.push_back(i);
    }
    return DomainList::make(std::move(iv), after);
  }

  FieldResult<StringType> CountPerDomain::getDomain(FieldParams&& params) const {
    return std::string{db::domain_id[i]};
  }

  FieldResult<IntType> CountPerDomain::getCount(FieldParams&& params) const {
    return db::domain_tx_count[i];
  }

  FieldResult<std::vector<std::shared_ptr<object::CountPerDomain>>> Query::getTransactionCountPerDomain(FieldParams&& params) const {
    std::vector<std::shared_ptr<object::CountPerDomain>> result;
    for (auto i = 0u; i < db::domainCount(); ++i) {
      result.push_back(std::make_shared<CountPerDomain>(i));
    }
    return result;
  }
}  // namespace graphql::bcx

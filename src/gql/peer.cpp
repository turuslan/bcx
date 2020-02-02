#include "gql/impl.hpp"

namespace graphql::bcx {
  FieldResult<StringType> Peer::getAddress(FieldParams&& params) const {
    return std::string{db::peer_address[i]};
  }

  FieldResult<StringType> Peer::getPublicKey(FieldParams&& params) const {
    return format::hex(db::peer_pub[i]);
  }

  FieldResult<std::shared_ptr<object::Peer>> Query::getPeerByPublicKey(FieldParams&& params, StringType&& publicKeyArg) const {
    auto pub = format::unhex<Sha256>(publicKeyArg);
    if (!pub) {
      return nullptr;
    }
    auto i = db::peer_pub.find(*pub);
    return i ? std::make_shared<Peer>(*i) : nullptr;
  }

  FieldResult<std::shared_ptr<object::PeerList>> Query::getPeerList(FieldParams&& params, std::optional<IntType>&& afterArg, IntType&& countArg) const {
    std::vector<size_t> iv;
    auto after = afterArg ? *afterArg : -1;
    for (auto i = after + 1; i >= 0 && i < db::peerCount() && iv.size() < countArg; ++i) {
      iv.push_back(i);
    }
    return PeerList::make(std::move(iv), after);
  }
}  // namespace graphql::bcx

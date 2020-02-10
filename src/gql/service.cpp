#include <graphqlservice/JSONResponse.h>
#include <rapidjson/document.h>

#include "gql/schema.hpp"
#include "gql/service.hpp"

namespace bcx {
  static auto service = std::make_shared<graphql::service::Request>(graphql::service::TypeMap{{"query", std::make_shared<graphql::bcx::Query>()}});

  std::string gql(const std::string &body) {
    using graphql::response::Value;
    rapidjson::Document jdoc;
    jdoc.Parse(body);
    auto jbody = jdoc.GetObject();
    Value vars{graphql::response::Type::Map};
    if (jbody.HasMember("variables")) {
      auto &jvars = jbody["variables"];
      if (jvars.IsObject()) {
        for (auto &pair : jvars.GetObject()) {
          auto &jval = pair.value;
          Value val;
          if (jval.IsString()) {
            val = Value(jval.GetString());
          } else if (jval.IsInt()) {
            val = Value(jval.GetInt());
          } else if (jval.IsBool()) {
            val = Value(jval.GetBool());
          }
          vars.emplace_back(pair.name.GetString(), std::move(val));
        }
      }
    }
    auto query = graphql::peg::parseString(jbody["query"].GetString());
    return graphql::response::toJSON(service->resolve(nullptr, *query.root, "", std::move(vars)).get());
  }
}  // namespace bcx

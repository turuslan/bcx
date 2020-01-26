#include <belle.hh>

#include "format/format.hpp"
#include "server/server.hpp"

namespace bcx {
  Server::Server() : app(std::make_shared<OB::Belle::Server>()) {
    using OB::Belle::Method;
    constexpr auto kContentType = boost::beast::http::field::content_type;

    app->on_http("/health", Method::get, [](auto &ctx) {
      ctx.res.set(kContentType, "application/json");
      ctx.res.body() = "{\"status\":\"UP\"}";
    });

    app->on_http("/logLevel", Method::post, [](auto &ctx) {
      auto query = ctx.req.params();
      auto param = query.find("level");
      if (param == query.end() || !setLogLevel(param->second)) {
        ctx.res.result(OB::Belle::Status::bad_request);
        return;
      }
    });
  }

  boost::asio::io_context &Server::io() {
    return app->io();
  }

  void Server::run() {
    logger::info("Server is running on localhost:{}", app->port());
    app->listen();
  }
}  // namespace bcx

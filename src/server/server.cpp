#include <belle.hh>

#include "db/db.hpp"
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

    app->on_http("/metrics", Method::get, [](auto &ctx) {
      std::stringstream s;
      auto counter = [&s](const std::string &type, size_t count) {
        auto key = "explorer_" + type + "_total";
        s << "#HELP " << key << " " << type << " count\n";
        s << "#TYPE " << key << " counter\n";
        s << key << " " << count << "\n";
        s << "\n";
      };
      counter("blocks", db::blockCount());
      counter("transactions", db::txCount());
      counter("accounts", db::accountCount());
      counter("peers", db::peerCount());
      ctx.res.set(kContentType, "text/plain");
      ctx.res.body() = s.str();
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

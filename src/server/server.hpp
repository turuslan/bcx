#ifndef BCX_SERVER_SERVER_HPP
#define BCX_SERVER_SERVER_HPP

#include <memory>

namespace boost::asio {
  class io_context;
}  // namespace boost::asio

namespace OB::Belle {
  class Server;
}  // namespace OB::Belle

namespace bcx {
  struct Server {
    Server();
    boost::asio::io_context &io();
    void run();

    std::shared_ptr<OB::Belle::Server> app;
  };
}  // namespace bcx

#endif  // BCX_SERVER_SERVER_HPP

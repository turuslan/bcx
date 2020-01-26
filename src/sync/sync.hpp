#ifndef BCX_SYNC_SYNC_HPP
#define BCX_SYNC_SYNC_HPP

namespace boost::asio {
  class io_context;
}  // namespace boost::asio

namespace bcx {
  void runSync(boost::asio::io_context &io);
}  // namespace bcx

#endif  // BCX_SYNC_SYNC_HPP

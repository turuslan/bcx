#include <csignal>
#include <thread>

#include "db/db.hpp"
#include "format/format.hpp"
#include "server/server.hpp"
#include "sync/sync.hpp"

void signalExit(int) {
  exit(0);
}

int main() {
  signal(SIGINT, signalExit);
  signal(SIGTERM, signalExit);

  bcx::config.load();
  bcx::db::load();
  bcx::Server server;
  auto &io = server.io();
  std::thread sync;
  if (!bcx::config.disable_sync()) {
    sync = std::thread{[&]() { bcx::runSync(server.io()); }};
  }
  server.run();
  if (sync.joinable()) {
    sync.join();
  }
  return 0;
}

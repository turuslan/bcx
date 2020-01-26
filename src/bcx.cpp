#include "format/format.hpp"
#include "server/server.hpp"

int main() {
  bcx::config.load();
  bcx::Server server;
  server.run();
  return 0;
}

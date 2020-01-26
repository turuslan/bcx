#include "format/format.hpp"

int main() {
  bcx::config.load();
  bcx::logger::info("hi");
  return 0;
}

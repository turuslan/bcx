
add_library(format
  format.cpp
  )
target_link_libraries(format
  gen-pb
  iroha::ed25519
  spdlog::spdlog
  )

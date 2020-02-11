
add_library(format
  format.cpp
  )
target_link_libraries(format
  Boost::filesystem
  gen-pb
  iroha::ed25519
  spdlog::spdlog
  )

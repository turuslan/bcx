
add_library(server
  server.cpp
  )
target_compile_definitions(server PRIVATE
  OB_BELLE_CONFIG_SSL_OFF
  OB_BELLE_CONFIG_CLIENT_OFF
  )
target_link_libraries(server
  Boost::system
  gql
  )

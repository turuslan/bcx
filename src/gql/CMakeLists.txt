
add_library(gql
  account.cpp
  block.cpp
  domain.cpp
  peer.cpp
  query.cpp
  role.cpp
  service.cpp
  transaction.cpp
  )
target_compile_definitions(gql PRIVATE
  RAPIDJSON_HAS_STDSTRING
  RAPIDJSON_HAS_CXX11_RANGE_FOR
  )
target_include_directories(gql PRIVATE
  ${RAPIDJSON_INCLUDE_DIRS}
  )
target_link_libraries(gql
  cppgraphqlgen::graphqljson
  gen-gql
  )

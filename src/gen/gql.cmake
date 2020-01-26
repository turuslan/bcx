
set(GQL_IN ${PROJECT_SOURCE_DIR}/schema/gql.gql)
set(GQL_OUT_1 ${CMAKE_CURRENT_SOURCE_DIR}/gql)
set(GQL_OUT_2 ${GQL_OUT_1}/Bcx)
set(GQL_OUT_3 ${GQL_OUT_2}Schema)
file(MAKE_DIRECTORY ${GQL_OUT_1})

add_custom_command(
  OUTPUT ${GQL_OUT_3}.h ${GQL_OUT_3}.cpp
  COMMAND cppgraphqlgen::schemagen --no-stubs ${GQL_IN} ${GQL_OUT_2} bcx
  DEPENDS cppgraphqlgen::schemagen ${GQL_IN}
  )

add_library(gen-gql
  ${GQL_OUT_3}.cpp
  )
target_link_libraries(gen-gql
  cppgraphqlgen::graphqlservice
  )

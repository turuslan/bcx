cmake_minimum_required(VERSION 3.15)

project(bcx)

set(CMAKE_CXX_STANDARD 17)

include(cmake/dependencies.cmake)

include_directories(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(src)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/graphiql.html ${CMAKE_CURRENT_BINARY_DIR}/graphiql.html COPYONLY)


add_subdirectory(format)
add_subdirectory(gen)
add_subdirectory(server)
add_subdirectory(sync)

add_executable(bcx
  bcx.cpp
  )
target_link_libraries(bcx
  format
  server
  sync
  )

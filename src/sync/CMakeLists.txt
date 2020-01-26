
add_library(sync
  sync.cpp
  )
target_link_libraries(sync
  gRPC::grpc++
  )

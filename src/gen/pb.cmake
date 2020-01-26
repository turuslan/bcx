
set(PB_IN_DIR ${PROJECT_SOURCE_DIR}/schema/pb)
set(PB_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pb)
file(MAKE_DIRECTORY ${PB_OUT_DIR})

function(compile_proto_to_cpp PROTO)
  string(REGEX REPLACE "\\.proto$" ".pb.h" GEN_PB_HEADER ${PROTO})
  string(REGEX REPLACE "\\.proto$" ".pb.cc" GEN_PB ${PROTO})
  get_target_property(Protobuf_INCLUDE_DIR protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
  add_custom_command(
    OUTPUT ${PB_OUT_DIR}/${GEN_PB_HEADER} ${PB_OUT_DIR}/${GEN_PB}
    COMMAND protobuf::protoc -I${Protobuf_INCLUDE_DIR} -I${PB_IN_DIR} ${ARGN} --cpp_out=${PB_OUT_DIR} ${PROTO}
    DEPENDS protobuf::protoc ${PB_IN_DIR}/${PROTO}
    )
endfunction()

function(compile_proto_only_grpc_to_cpp PROTO)
  string(REGEX REPLACE "\\.proto$" ".grpc.pb.h" GEN_GRPC_PB_HEADER ${PROTO})
  string(REGEX REPLACE "\\.proto$" ".grpc.pb.cc" GEN_GRPC_PB ${PROTO})
  get_target_property(Protobuf_INCLUDE_DIR protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
  add_custom_command(
    OUTPUT ${PB_OUT_DIR}/${GEN_GRPC_PB_HEADER} ${PB_OUT_DIR}/${GEN_GRPC_PB}
    COMMAND protobuf::protoc -I${Protobuf_INCLUDE_DIR} -I${PB_IN_DIR} ${ARGN} --grpc_out=${PB_OUT_DIR} --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin> ${PROTO}
    DEPENDS gRPC::grpc_cpp_plugin ${PB_IN_DIR}/${PROTO}
    )
endfunction()

compile_proto_to_cpp(block.proto)
compile_proto_to_cpp(commands.proto)
compile_proto_to_cpp(endpoint.proto)
compile_proto_to_cpp(primitive.proto)
compile_proto_to_cpp(qry_responses.proto)
compile_proto_to_cpp(queries.proto)
compile_proto_to_cpp(transaction.proto)
compile_proto_only_grpc_to_cpp(endpoint.proto)

add_library(gen-pb
  ${PB_OUT_DIR}/block.pb.cc
  ${PB_OUT_DIR}/commands.pb.cc
  ${PB_OUT_DIR}/primitive.pb.cc
  ${PB_OUT_DIR}/transaction.pb.cc
  ${PB_OUT_DIR}/endpoint.grpc.pb.cc
  ${PB_OUT_DIR}/endpoint.pb.cc
  ${PB_OUT_DIR}/queries.pb.cc
  ${PB_OUT_DIR}/qry_responses.pb.cc
  )
target_link_libraries(gen-pb
  protobuf::libprotobuf
  )

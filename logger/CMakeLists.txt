
get_filename_component(logger_proto "${CMAKE_SOURCE_DIR}/deploy/protos/logger.proto" ABSOLUTE)
get_filename_component(logger_proto_path "${logger_proto}" PATH)
set(logger_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/logger.pb.cc")
set(logger_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/logger.pb.h")
set(logger_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/logger.grpc.pb.cc")
set(logger_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/logger.grpc.pb.h")

add_custom_command(
        OUTPUT "${logger_proto_srcs}" "${logger_proto_hdrs}" "${logger_grpc_srcs}" "${logger_grpc_hdrs}"
        COMMAND ${PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${logger_proto_path}"
        --plugin=protoc-gen-grpc=${GRPC_PROTO_PLUGIN}
        "${logger_proto}"
        DEPENDS "${logger_proto}")
add_custom_target(logger_proto_gen ALL DEPENDS ${logger_proto} ${logger_proto_srcs} ${logger_proto_headers} ${logger_grpc_srcs} ${logger_grpc_hdrs})

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_library(klogger Logger.cpp ${logger_proto_srcs} ${logger_grpc_srcs})
target_link_libraries(klogger g3logger)

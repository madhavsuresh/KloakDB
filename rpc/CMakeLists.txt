link_directories(/usr/local/lib /home/madhav/DB/grpc/libs/opt/)
set(CMAKE_BUILD_TYPE Debug)
LIST(APPEND CMAKE_PROGRAM_PATH ${CMAKE_SOURCE_DIR}/lib/bin)
find_program(PROTOC protoc ${CMAKE_SOURCE_DIR}/lib/bin)
find_program(GRPC_PROTO_PLUGIN grpc_cpp_plugin ${CMAKE_SOURCE_DIR}/lib/bin)

find_library(GRPC++ grpc++)
find_library(GRPC grpc)
find_library(GFLAGS gflags)
find_library(DL dl)
find_library(PROTOBUF protobuf)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

get_filename_component(vdb_proto "protos/vaultdb.proto" ABSOLUTE)
get_filename_component(vdb_proto_path "${vdb_proto}" PATH)

set(vdb_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/vaultdb.pb.cc")
set(vdb_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/vaultdb.pb.h")
set(vdb_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/vaultdb.grpc.pb.cc")
set(vdb_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/vaultdb.grpc.pb.h")


#add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vaultdb.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/vaultdb.grpc.pb.h
#		   PRE_BUILD
#		   COMMAND protoc -I ${CMAKE_CURRENT_SOURCE_DIR} --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin ${CMAKE_CURRENT_SOURCE_DIR}/vaultdb.proto)
#
#add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vaultdb.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/vaultdb.pb.h
#		PRE_BUILD
#		COMMAND protoc -I ${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=. ${CMAKE_CURRENT_SOURCE_DIR}/vaultdb.proto)

#add_library(HBGeneralize HBGeneralize.cpp ../postgres_client.cpp vaultdb.grpc.pb.cc vaultdb.pb.cc ../TableStatistics.cpp)

add_custom_command(
		OUTPUT "${vdb_proto_srcs}" "${vdb_proto_hdrs}" "${vdb_grpc_srcs}" "${vdb_grpc_hdrs}"
		COMMAND ${PROTOC}
		ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
		--cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
		-I "${vdb_proto_path}"
		--plugin=protoc-gen-grpc=${GRPC_PROTO_PLUGIN}
		"${vdb_proto}"
		DEPENDS "${vdb_proto}")
add_custom_target(rpc_proto_gen ALL DEPENDS ${vdb_proto} ${vdb_proto_srcs} ${vdb_proto_headers} ${vdb_grpc_srcs} ${vdb_grpc_hdrs})


add_library(krpc ${vdb_proto_srcs} ${vdb_grpc_srcs} HonestBrokerImpl.cpp HonestBrokerPrivate.cpp HonestBrokerClient.cpp
	DataOwnerPrivate.cpp DataOwnerImpl.cpp InfoPrivate.cpp DataOwnerClient.cpp Repartition.cpp)

target_link_libraries(krpc grpc grpc++ protobuf dl klogger)

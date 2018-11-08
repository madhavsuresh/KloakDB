link_directories(/usr/local/lib /home/madhav/DB/grpc/libs/opt/)
set(CMAKE_BUILD_TYPE Debug)
LIST(APPEND CMAKE_PROGRAM_PATH /opt/grpc/bin)

find_library(GRPC++ grpc++)
find_library(GRPC grpc)
find_library(GFLAGS gflags)
find_library(DL dl)
find_library(PROTOBUF protobuf)
 
add_custom_command(OUTPUT vaultdb.grpc.pb.cc
		   PRE_BUILD
		   COMMAND protoc -I ${CMAKE_CURRENT_SOURCE_DIR} --grpc_out=. --plugin=protoc-gen-grpc=/opt/grpc/bin/grpc_cpp_plugin ${CMAKE_CURRENT_SOURCE_DIR}/vaultdb.proto)

add_custom_command(OUTPUT vaultdb.pb.cc
		PRE_BUILD
		COMMAND protoc -I ${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=. ${CMAKE_CURRENT_SOURCE_DIR}/vaultdb.proto)

#add_library(HBGeneralize HBGeneralize.cpp ../postgres_client.cpp vaultdb.grpc.pb.cc vaultdb.pb.cc ../TableStatistics.cpp)

add_library(krpc vaultdb.pb.cc vaultdb.grpc.pb.cc HonestBrokerImpl.cpp HonestBrokerPrivate.cpp HonestBrokerClient.cpp
	DataOwnerPrivate.cpp DataOwnerImpl.cpp InfoPrivate.cpp DataOwnerClient.cpp)

target_link_libraries(krpc grpc grpc++ protobuf dl klogger)

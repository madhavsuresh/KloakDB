link_directories(/usr/local/lib /home/madhav/DB/grpc/libs/opt/)
set(CMAKE_BUILD_TYPE Debug)

find_library(GRPC++ grpc++)
find_library(GRPC grpc)
find_library(GFLAGS gflags)
find_library(DL dl)
find_library(PROTOBUF protobuf)
 
add_custom_command(OUTPUT vaultdb.grpc.pb.cc
		   PRE_BUILD
		   COMMAND protoc -I ${CMAKE_CURRENT_SOURCE_DIR} --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ${CMAKE_CURRENT_SOURCE_DIR}/vaultdb.proto)

add_custom_command(OUTPUT vaultdb.pb.cc
		PRE_BUILD
		COMMAND protoc -I ${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=. ${CMAKE_CURRENT_SOURCE_DIR}/vaultdb.proto)

add_library(HBGeneralize HBGeneralize.cpp ../postgres_client.cpp vaultdb.grpc.pb.cc vaultdb.pb.cc ../TableStatistics.cpp)

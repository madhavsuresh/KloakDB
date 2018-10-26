cmake_minimum_required(VERSION 3.10)
# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif ()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif ()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
        ${CMAKE_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif ()

SET( CMAKE_EXPORT_COMPILE_COMMANDS ON )
IF( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
    EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
	${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
	)
ENDIF()
set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_CXX_FLAGS "-fno-omit-frame-pointer -g")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=address")


#find_library(GLOG glog)
link_directories(/usr/local/lib /root/DB/grpc/libs/opt/)
include_directories("/root/DB/linux-sgx/sgxsdk/include")
link_directories(/root/DB/linux-sgx/sgxsdk/lib64)
find_library(SGXCRYPTO sgx_tcrypto)


add_executable(postgres_test postgres_client/postgres_client_test.cpp postgres_client/postgres_client.cpp)
target_link_libraries(postgres_test gtest_main pqxx glog profiler tcmalloc)
add_test(NAME postgres_test COMMAND postgres_test)

add_executable(aggregate_test aggregate_test.cpp Aggregate.cpp postgres_client.cpp)
target_link_libraries(aggregate_test gtest_main pq pqxx glog profiler tcmalloc g3logger)
add_test(NAME aggregate_test COMMAND aggregate_test)

add_executable(filter_test filter_test.cpp Filter.cpp postgres_client.cpp Expressions.cpp)
target_link_libraries(filter_test gtest_main pq pqxx glog profiler tcmalloc)
add_test(NAME filter_test COMMAND filter_test)

add_executable(join_test operators/join_test.cpp operators/HashJoin.cpp postgres_client/postgres_client.cpp operators/Expressions.cpp)
target_link_libraries(join_test gtest_main pqxx glog profiler tcmalloc)
add_test(NAME join_test COMMAND join_test)

add_executable(sort_test operators/sort_test.cpp operators/Sort.cpp postgres_client/postgres_client.cpp operators/Expressions.cpp)
target_link_libraries(sort_test gtest_main pqxx glog profiler tcmalloc)
add_test(NAME sort_test COMMAND sort_test)

set(REPART_SRCS
        rpc/Repartition.cpp)

add_executable(vaultdb VaultDB.cpp postgres_client/postgres_client.cpp rpc/vaultdb.pb.cc rpc/vaultdb.grpc.pb.cc rpc/HonestBrokerImpl.cpp rpc/HonestBrokerPrivate.cpp rpc/HonestBrokerClient.cpp rpc/HonestBrokerClient.h rpc/DataOwnerPrivate.cpp rpc/DataOwnerPrivate.h rpc/DataOwnerImpl.cpp rpc/DataOwnerImpl.h rpc/InfoPrivate.cpp rpc/InfoPrivate.h rpc/DataOwnerClient.cpp rpc/DataOwnerClient.h rpc/Repartition.cpp)

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
        /lib
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        /usr/lib/x86_64-linux-gnu
        /usr/lib/gcc/x86_64-redhat-linux/7/32)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})

find_library(GRPC_LIB NAMES libgrpc.a)
message(STATUS ${GRPC_LIB})
find_library(GRPCxx_LIB NAMES libgrpc++.a)
find_library(GFLAGS_LIB NAMES libgflags.a)
message(STATUS ${GFLAGS_LIB})

find_library(STD_LIB NAMES /usr/lib/gcc/x86_64-redhat-linux/7/32/libstdc++.a)
message(STATUS ${STD_LIB})

find_library(PROTOBUF_LIB NAMES libprotobuf.a)
find_library(PQXX_LIB NAMES libpqxx.a)
find_library(DL_LIB NAMES libdl.a)

find_library(PTHREAD_LIB pthread)
find_library(G3LOGGER_LIB NAMES libg3logger.a)
add_executable(vaultdb VaultDB.cpp postgres_client.cpp rpc/vaultdb.pb.cc rpc/vaultdb.grpc.pb.cc rpc/HonestBrokerImpl.cpp rpc/HonestBrokerPrivate.cpp rpc/HonestBrokerClient.cpp rpc/HonestBrokerClient.h rpc/DataOwnerPrivate.cpp rpc/DataOwnerPrivate.h rpc/DataOwnerImpl.cpp rpc/DataOwnerImpl.h rpc/InfoPrivate.cpp rpc/InfoPrivate.h rpc/DataOwnerClient.cpp rpc/DataOwnerClient.h Repartition.cpp Sort.cpp Filter.cpp Expressions.cpp Aggregate.cpp)

#target_link_libraries(vaultdb grpc grpc++ gflags protobuf pqxx dl pthread g3logger "-static")
target_link_libraries(vaultdb ${GRPC_LIB} ${GFLAGS_LIB} ${PQXX_LIB} ${GRPCxx_LIB} ${PROTOBUF_LIB} ${PTHREAD_LIB} ${G3LOGGER_LIB} -static )

add_subdirectory(rpc)

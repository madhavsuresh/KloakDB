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
set(CMAKE_CXX_FLAGS "-fno-omit-frame-pointer -fsanitize=address -g")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=address")


#find_library(GLOG glog)
link_directories(/usr/local/lib /opt/grpc/lib)
include_directories("/opt/grpc/include")
#/home/madhav/DB/linux-sgx/sgxsdk/include
link_directories(/home/madhav/DB/linux-sgx/sgxsdk/lib64)
find_library(SGXCRYPTO sgx_tcrypto)


add_executable(postgres_test postgres_client_test.cpp postgres_client.cpp)
target_link_libraries(postgres_test gtest_main pq pqxx glog profiler tcmalloc)
add_test(NAME postgres_test COMMAND postgres_test)

add_executable(aggregate_test aggregate_test.cpp Aggregate.cpp postgres_client.cpp)
target_link_libraries(aggregate_test gtest_main pq pqxx glog profiler tcmalloc g3logger)
add_test(NAME aggregate_test COMMAND aggregate_test)

add_executable(filter_test filter_test.cpp Filter.cpp postgres_client.cpp Expressions.cpp)
target_link_libraries(filter_test gtest_main pq pqxx glog profiler tcmalloc)
add_test(NAME filter_test COMMAND filter_test)

add_executable(sort_test sort_test.cpp Sort.cpp postgres_client.cpp Expressions.cpp)
target_link_libraries(sort_test gtest_main pq pqxx glog profiler tcmalloc g3logger)
add_test(NAME sort_test COMMAND sort_test)

set(REPART_SRCS
    Repartition.cpp)

add_executable(vaultdb VaultDB.cpp postgres_client.cpp rpc/vaultdb.pb.cc rpc/vaultdb.grpc.pb.cc rpc/HonestBrokerImpl.cpp rpc/HonestBrokerPrivate.cpp rpc/HonestBrokerClient.cpp rpc/HonestBrokerClient.h rpc/DataOwnerPrivate.cpp rpc/DataOwnerPrivate.h rpc/DataOwnerImpl.cpp rpc/DataOwnerImpl.h rpc/InfoPrivate.cpp rpc/InfoPrivate.h rpc/DataOwnerClient.cpp rpc/DataOwnerClient.h Repartition.cpp Sort.cpp Filter.cpp Expressions.cpp Aggregate.cpp)

target_link_libraries(vaultdb grpc grpc++ gflags protobuf pqxx dl pthread asan g3logger)

add_subdirectory(rpc)

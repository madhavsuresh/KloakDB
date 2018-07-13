cmake_minimum_required(VERSION 3.10)
SET( CMAKE_EXPORT_COMPILE_COMMANDS ON )
IF( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
    EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
	${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
	)
ENDIF()
set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_FLAGS "-g")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=address")


include_directories("/home/madhav/DB/linux-sgx/sgxsdk/include")
link_directories(/home/madhav/DB/linux-sgx/sgxsdk/lib64)
find_library(SGXCRYPTO sgx_tcrypto)
set(RUNNER_SRCS
	postgres_client.cpp
	Filter.cpp
	runner.cpp
	Expressions.cpp
	Repartition.cpp
    vaultdb_generated.h Aggregate.cpp)

add_executable(runner ${RUNNER_SRCS})

set(REPART_SRCS
    Repartition.cpp)
target_link_libraries(runner pq pqxx sgx_tcrypto)

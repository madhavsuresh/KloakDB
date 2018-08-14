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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g")
#set(CMAKE_CXX_FLAGS "-g")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=address")


#find_library(GLOG glog)
include_directories("/home/madhav/DB/linux-sgx/sgxsdk/include")
link_directories(/home/madhav/DB/linux-sgx/sgxsdk/lib64)
find_library(SGXCRYPTO sgx_tcrypto)
set(RUNNER_SRCS
	postgres_client.cpp
	runner.cpp
	Expressions.cpp
	Repartition.cpp
    vaultdb_generated.h Aggregate.cpp Aggregate.h Sort.cpp)

add_executable(runner ${RUNNER_SRCS})

add_executable(postgres_test postgres_client_test.cpp postgres_client.cpp)
target_link_libraries(postgres_test gtest_main pq pqxx glog profiler tcmalloc)
add_test(NAME postgres_test COMMAND postgres_test)


add_executable(filter_test filter_test.cpp Filter.cpp postgres_client.cpp Expressions.cpp)
target_link_libraries(filter_test gtest_main pq pqxx glog profiler tcmalloc)
add_test(NAME filter_test COMMAND filter_test)

set(REPART_SRCS
    Repartition.cpp)
target_link_libraries(runner pq pqxx sgx_tcrypto profiler tcmalloc)

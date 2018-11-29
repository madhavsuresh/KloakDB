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
include(CTest)
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
#set(CMAKE_CXX_FLAGS "-fno-omit-frame-pointer -fsanitize=address -g")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=address")


#find_library(GLOG glog)
link_directories(${CMAKE_SOURCE_DIR}/lib/lib/ ${CMAKE_SOURCE_DIR}/lib/sgxsdk/lib64 /usr/local/lib /opt/grpc/lib)
include_directories(${CMAKE_SOURCE_DIR}/lib/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/sgxsdk/include)

include_directories(${CMAKE_SOURCE_DIR})
#/home/madhav/DB/linux-sgx/sgxsdk/include
find_library(SGXCRYPTO sgx_tcrypto)


set(REPART_SRCS
    Repartition.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/rpc)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/logger)

add_executable(vaultdb VaultDB.cpp test/experiments/exp5.cpp test/experiments/exp3.cpp test/experiments/exp4.cpp)

target_link_libraries(vaultdb asan gflags dl pthread krpc g3logger pqxxcompat dataformat operators klogger vaultdb_sgx_app)

add_subdirectory(rpc)
add_subdirectory(data)
add_subdirectory(operators)
add_subdirectory(test)
add_subdirectory(logger)
add_subdirectory(generalize)
add_subdirectory(sgx)

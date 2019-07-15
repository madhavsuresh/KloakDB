cmake_minimum_required (VERSION 2.8.11)
set(CMAKE_BUILD_TYPE Debug)
find_library(TCRYPTO_LIB sgx_tcrypto ${CMAKE_SOURCE_DIR}/lib/sgxsdk/lib64)
#project (emp-sh2pc)
#set(NAME "emp-sh2pc")

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake PATHS /home/madhav/DB/)
include(${CMAKE_FOLDER}/cmake/common.cmake)
include(${CMAKE_FOLDER}/cmake/source_of_randomness.cmake)
include(${CMAKE_FOLDER}/cmake/threading.cmake)
list(APPEND CMAKE_PREFIX_PATH "/home/madhav/DB")

find_package(emp-ot REQUIRED)
include_directories(${EMP-OT_INCLUDE_DIRS})

# Installation
install(FILES cmake/emp-sh2pc-config.cmake DESTINATION cmake/)
install(DIRECTORY emp-sh2pc DESTINATION include/)

# Test cases
macro (add_test _name)
	add_test_with_lib(${_name} ${EMP-OT_LIBRARIES})
endmacro()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O0")


add_executable(left_deep_lib test/left_deep.cpp)
target_link_libraries(left_deep_lib pqxxcompat operators dataformat pqxx pq gflags klogger krpc ${EMP-OT_LIBRARIES} ${TCRYPTO_LIB})


set(CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(RUNNER_SRCS
	postgres_client.cpp
	Filter.cpp
	runner.cpp
	Expressions.cpp
	Repartition.cpp)

add_executable(runner ${RUNNER_SRCS})

set(REPART_SRCS
    Repartition.cpp)
target_link_libraries(runner pq pqxx)

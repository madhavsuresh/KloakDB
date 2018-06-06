set(CMAKE_CXX_STANDARD 14)
set(RUNNER_SRCS
	postgres_client.cpp
	Filter.cpp
	runner.cpp
	Expressions.cpp)

add_executable(runner ${RUNNER_SRCS})

target_link_libraries(runner pq pqxx)

set(JOIN_SRCS
    SimpleJoin.cpp
    test_simplejoin.cpp)

add_executable(join ${JOIN_SRCS})

target_link_libraries(join gtest)

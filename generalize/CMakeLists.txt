add_executable(gen_test gen_test.cpp)
target_link_libraries(gen_test gtest_main profiler tcmalloc operators dataformat pqxxcompat)
add_test(gen_test2 gen_test)

add_executable(postgres_test postgres_client_test.cpp)
target_link_libraries(postgres_test gtest_main profiler tcmalloc dataformat pqxxcompat)
add_test(NAME postgres_test COMMAND postgres_test)

add_executable(health_lnk_schema_test health_lnk_schema_test.cpp)
target_link_libraries(health_lnk_schema_test gtest_main profiler tcmalloc dataformat pqxxcompat)
add_test(NAME health_lnk_schema_test COMMAND health_lnk_schema_test)

add_executable(vdb3 vdb3.cpp)
target_link_libraries(vdb3 gtest_main profiler tcmalloc dataformat pqxxcompat)
add_test(NAME vdb3 COMMAND vdb3)

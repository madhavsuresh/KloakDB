find_library(TCRYPTO_LIB sgx_tcrypto ${CMAKE_SOURCE_DIR}/lib/sgxsdk/lib64)
add_executable(aggregate_test aggregate_test.cpp)
target_link_libraries(aggregate_test gtest_main profiler tcmalloc operators dataformat pqxxcompat vaultdb_sgx_app ${TCRYPTO_LIB})
add_test(NAME aggregate_test COMMAND aggregate_test)

add_executable(filter_test filter_test.cpp)
target_link_libraries(filter_test gtest_main profiler tcmalloc operators dataformat pqxxcompat vaultdb_sgx_app klogger)
add_test(NAME filter_test COMMAND filter_test)

add_executable(sort_test sort_test.cpp)
target_link_libraries(sort_test gtest_main profiler tcmalloc operators dataformat pqxxcompat vaultdb_sgx_app)
add_test(NAME sort_test COMMAND sort_test)

add_executable(join_test join_test.cpp)
target_link_libraries(join_test gtest_main profiler tcmalloc operators dataformat pqxxcompat vaultdb_sgx_app klogger)
add_test(NAME join_test COMMAND join_test)

add_executable(repart_test repart_test.cpp)
target_link_libraries(repart_test gtest_main profiler tcmalloc operators dataformat pqxxcompat vaultdb_sgx_app klogger krpc)
add_test(NAME repart_test COMMAND repart_test)

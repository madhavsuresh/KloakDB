add_executable(aspirin_profile_test aspirin_profile_test.cpp)
target_link_libraries(aspirin_profile_test gtest_main profiler operators dataformat pqxxcompat)
add_test(NAME aspirin_profile_test COMMAND aspirin_profile_test)

add_executable(comorbidity_test comorbidity_test.cpp)
target_link_libraries(comorbidity_test gtest_main profiler g3logger operators dataformat pqxxcompat)
add_test(NAME comorbidity_test COMMAND comorbidity_test)

add_executable(dosage_study_test dosage_study_test.cpp)
target_link_libraries(dosage_study_test gtest_main profiler operators dataformat pqxxcompat)
add_test(NAME dosage_study_test COMMAND dosage_study_test)

add_executable(exp2_kanonjoins exp2_kanonjoins.cpp)
# TODO(madhavsuresh) why does the order of this only affect this target, and no other targets?
target_link_libraries(exp2_kanonjoins gtest_main profiler operators dataformat pqxxcompat)
add_test(NAME exp2_kanonjoins COMMAND exp2_kanonjoins)


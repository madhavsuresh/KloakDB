find_program(SGX_EDGER8R sgx_edger8r ${CMAKE_SOURCE_DIR}/lib/sgxsdk/bin/x64)

get_filename_component(vdb_edl "VaultDB.edl" ABSOLUTE)
get_filename_component(vdb_edl_path "${vdb_edl}" PATH)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/VaultDB_u.c"
	COMMAND ${SGX_EDGER8R} 
	ARGS --untrusted ${vdb_edl} --search-path ${CMAKE_SOURCE_DIR}/sgx --search-path ${CMAKE_SOURCE_DIR}/lib/sgxsdk/include)

#We use the "app" nomenclature that is used within the SGX development examples
# App is for all code outside the enclave. 
message("${CMAKE_CURRENT_BINARY_DIR}")
add_library(vaultdb_sgx_app VaultDBSGXApp.cpp "${CMAKE_CURRENT_BINARY_DIR}/VaultDB_u.c")
enable_language(ASM)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

if(NOT "$ENV{SGX_MODE}" EQUAL "HW")
	message("SOFTWARE SIM MODE")
  target_link_libraries(vaultdb_sgx_app sgx_urts_sim sgx_uae_service_sim)
else()
	message("HARDWARE MODE")
  target_link_libraries(vaultdb_sgx_app sgx_urts sgx_uae_service)
endif()

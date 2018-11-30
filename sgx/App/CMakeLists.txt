SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Wno-attributes -O0 -g -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type -Waddress -Wsequence-point -Wformat-security -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wno-attributes -O0 -g -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type -Waddress -Wsequence-point -Wformat-security -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls -DDEBUG -UNDEBUG -UEDEBUG" )
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Wno-attributes -O0 -g -DDEBUG -UNDEBUG  -UEDEBUG" )
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -DDEBUG -UNDEBUG -UEDEBUG" )
message("${vdb_edl}")
add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/VaultDB_u.c"
	COMMAND ${SGX_EDGER8R} 
	ARGS --untrusted ${vdb_edl} --search-path ${CMAKE_SOURCE_DIR}/sgx --search-path ${CMAKE_SOURCE_DIR}/lib/sgxsdk/include
	DEPENDS ${vdb_edl})

#We use the "app" nomenclature that is used within the SGX development examples
# App is for all code outside the enclave. 
add_library(vaultdb_sgx_app VaultDBSGXApp.cpp "${CMAKE_CURRENT_BINARY_DIR}/VaultDB_u.c")
enable_language(ASM)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

if(NOT "$ENV{SGX_MODE}" EQUAL "SW")
	message("HARDWARE MODE")
  target_link_libraries(vaultdb_sgx_app sgx_urts sgx_uae_service)
else()
	message("SOFTWARE SIM MODE")
  target_link_libraries(vaultdb_sgx_app sgx_urts_sim sgx_uae_service_sim)
endif()
target_link_libraries(vaultdb_sgx_app gflags)


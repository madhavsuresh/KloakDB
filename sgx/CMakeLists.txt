find_program(SGX_EDGER8R sgx_edger8r ${CMAKE_SOURCE_DIR}/lib/sgxsdk/bin/x64)
find_program(SGX_ENCLAVE_SIGNER sgx_sign ${CMAKE_SOURCE_DIR}/lib/sgxsdk/bin/x64)


get_filename_component(vdb_edl "VaultDB.edl" ABSOLUTE)
get_filename_component(vdb_edl_path "${vdb_edl}" PATH)


#Generate Trusted and Untrusted files
add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/VaultDB_u.c"
	COMMAND ${SGX_EDGER8R} 
	ARGS --untrusted ${vdb_edl} --search-path ${CMAKE_SOURCE_DIR}/sgx --search-path ${CMAKE_SOURCE_DIR}/lib/sgxsdk/include)

add_custom_command(
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/VaultDB_t.c"
		COMMAND ${SGX_EDGER8R}
		ARGS --trusted ${vdb_edl} --search-path ${CMAKE_SOURCE_DIR}/sgx --search-path ${CMAKE_SOURCE_DIR}/lib/sgxsdk/include)

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

find_library(TRTS_LIB sgx_trts ${CMAKE_SOURCE_DIR}/lib/sgxsdk/lib64)
find_library(TRTS_SIM_LIB sgx_trts_sim ${CMAKE_SOURCE_DIR}/lib/sgxsdk/lib64)
find_library(TSTDC_LIB sgx_tstdc ${CMAKE_SOURCE_DIR}/lib/sgxsdk/lib64)
find_library(TSTDCXX_LIB sgx_tcxx ${CMAKE_SOURCE_DIR}/lib/sgxsdk/lib64)
find_library(TKEY_EXCHANGE_LIB sgx_tkey_exchange ${CMAKE_SOURCE_DIR}/lib/sgxsdk/lib64)
find_library(TCRYPTO_LIB sgx_tcrypto ${CMAKE_SOURCE_DIR}/lib/sgxsdk/lib64)
find_library(SERVICE_LIB sgx_tservice ${CMAKE_SOURCE_DIR}/lib/sgxsdk/lib64)
find_library(SERVICE_SIM_LIB sgx_tservice_sim ${CMAKE_SOURCE_DIR}/lib/sgxsdk/lib64)
find_library(VAULTDB_DATAFORMAT dataformat)
find_library(VAULTDB_OPERATORS operators)

include_directories("${CMAKE_SOURCE_DIR}/lib/sgxsdk/include/libcxx" "${CMAKE_SOURCE_DIR}/lib/sgxsdk/include/tlibc")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
if(NOT "$ENV{SGX_MODE}" EQUAL "HW")
    set(Trts_Library_Path "${TRTS_SIM_LIB}")
	set(Service_Library_Path "${SERVICE_SIM_LIB}")
else()
	set(Trts_Library_Path "${TRTS_LIB}")
	set(Service_Library_Path "${SERVICE_LIB}")
endif()

set(ENCLAVE_LINK_FLAGS "-Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined -Wl,-pie,-eenclave_entry -Wl,--export-dynamic -Wl,--defsym,__ImageBase=0 -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/Enclave_debug.lds")
add_library(vaultdb_trusted SHARED VaultDB_t.c VaultDBEnclave.cpp)
target_link_libraries(vaultdb_trusted -Wl,--whole-archive "${Trts_Library_Path}" -Wl,--no-whole-archive -Wl,--start-group "${TSTDC_LIB}" "${TSTDCXX_LIB}"
		"${TKEY_EXCHANGE_LIB}" "${TCRYPTO_LIB}" "${Service_Library_Path}" -Wl,--end-group)
#target_link_libraries(vaultdb_trusted operators dataformat)
set_property(TARGET vaultdb_trusted PROPERTY POSITION_INDEPENDENT_CODE OFF)
set_target_properties(vaultdb_trusted PROPERTIES LINK_FLAGS ${ENCLAVE_LINK_FLAGS})

add_custom_command(
		COMMAND ${SGX_ENCLAVE_SIGNER} sign -key "${CMAKE_CURRENT_SOURCE_DIR}/Enclave_private.pem" -enclave $<TARGET_FILE:vaultdb_trusted> -out libvaultdb_trusted_signed.so -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
		DEPENDS vaultdb_trusted ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libvaultdb_trusted_signed.so)

add_custom_target(vaultdb_trusted_signed_target ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libvaultdb_trusted_signed.so)


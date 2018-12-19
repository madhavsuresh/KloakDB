find_program(SGX_EDGER8R sgx_edger8r ${CMAKE_SOURCE_DIR}/lib/sgxsdk/bin/x64)
find_program(SGX_ENCLAVE_SIGNER sgx_sign ${CMAKE_SOURCE_DIR}/lib/sgxsdk/bin/x64)


get_filename_component(vdb_edl "VaultDB.edl" ABSOLUTE)
get_filename_component(vdb_edl_path "${vdb_edl}" PATH)

set(SGX_MODE "SW")


#Generate Trusted and Untrusted files

add_subdirectory(App)
add_subdirectory(Enclave)


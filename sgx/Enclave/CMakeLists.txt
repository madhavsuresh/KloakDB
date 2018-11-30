SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpie -fstack-protector -fvisibility=hidden -nostdinc -fPIC -Wno-attributes -O0 -g -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type -Waddress -Wsequence-point -Wformat-security -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpie -fstack-protector -fvisibility=hidden -nostdinc -nostdinc++ -m64 -O0 -g -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type -Waddress -Wsequence-point -Wformat-security -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls -DDEBUG -UNDEBUG -UEDEBUG" )

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_custom_command(
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/VaultDB_t.c"
		COMMAND ${SGX_EDGER8R}
		ARGS --trusted ${vdb_edl} --search-path ${CMAKE_SOURCE_DIR}/sgx --search-path ${CMAKE_SOURCE_DIR}/lib/sgxsdk/include
        DEPENDS ${vdb_edl})

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
if("${SGX_MODE}" MATCHES "HW")
	message("HARDWARE SGX MODE")
	set(Trts_Library_Path "${TRTS_LIB}")
	set(Service_Library_Path "${SERVICE_LIB}")
else()
    message("SOFTWARE SIM MODE")
    set(Trts_Library_Path "${TRTS_SIM_LIB}")
	set(Service_Library_Path "${SERVICE_SIM_LIB}")
endif()


#recompile operator code for SGX compatability
SET(VAULTDB_SOURCES
		${CMAKE_SOURCE_DIR}/operators/Aggregate.cpp
		${CMAKE_SOURCE_DIR}/operators/HashJoin.cpp
		${CMAKE_SOURCE_DIR}/operators/Sort.cpp
		${CMAKE_SOURCE_DIR}/operators/Filter.cpp
		${CMAKE_SOURCE_DIR}/operators/Expressions.cpp
		${CMAKE_SOURCE_DIR}/data/postgres_client.cpp
		#${CMAKE_SOURCE_DIR}/operators/Generalize.cpp
		)

set(ENCLAVE_LINK_FLAGS "-Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined -Wl,-pie,-eenclave_entry -Wl,--export-dynamic -Wl,--defsym,__ImageBase=0 -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/Enclave_debug.lds")
add_library(vaultdb_trusted SHARED VaultDB_t.c VaultDBEnclave.cpp ${VAULTDB_SOURCES})
target_link_libraries(vaultdb_trusted -Wl,--whole-archive "${Trts_Library_Path}" -Wl,--no-whole-archive -Wl,--start-group "${TSTDC_LIB}" "${TSTDCXX_LIB}"
		"${TKEY_EXCHANGE_LIB}" "${TCRYPTO_LIB}" "${Service_Library_Path}" -Wl,--end-group)
set_property(TARGET vaultdb_trusted PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(vaultdb_trusted PROPERTIES LINK_FLAGS ${ENCLAVE_LINK_FLAGS})

add_custom_command(
		COMMAND ${SGX_ENCLAVE_SIGNER} sign -key "${CMAKE_CURRENT_SOURCE_DIR}/Enclave_private.pem" -enclave $<TARGET_FILE:vaultdb_trusted> -out libvaultdb_trusted_signed.so -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
		DEPENDS vaultdb_trusted ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libvaultdb_trusted_signed.so)

add_custom_target(vaultdb_trusted_signed_target ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libvaultdb_trusted_signed.so )


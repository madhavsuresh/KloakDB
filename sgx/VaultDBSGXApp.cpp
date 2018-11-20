#include <data/postgres_client.h>
#include <gflags/gflags.h>
#include "sgx_urts.h"
#include "VaultDB_u.h"
#include "data/postgres_client.h"

DEFINE_string(enclave_path, "sgx/enclave.signed.so", "path for built enclave");
/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
sgx_enclave_id_t initialize_enclave(void)
{
  sgx_launch_token_t token = {0};
  sgx_status_t ret = SGX_ERROR_UNEXPECTED;
  int updated = 0;
  sgx_enclave_id_t eid;

  /* Call sgx_create_enclave to initialize an enclave instance */
  /* Debug Support: set 2nd parameter to 1 */
  ret = sgx_create_enclave(FLAGS_enclave_path.c_str(), SGX_DEBUG_FLAG, &token, &updated, &eid, NULL);
  if (ret != SGX_SUCCESS) {
    throw;
  }
  return eid;
}

table_t * get_table_sgx(sgx_enclave_id_t eid, table_manager_t tm, int table_id) {
  ecall_get_table_t(eid, &tm, )

}

table_t * hash_join_sgx(table_t * left, table_t * right, join_def_t def) {

  sgx_enclave_id_t eid = initialize_enclave();
  table_manager_t tm;
  int left_table_id, right_table_id, output_table_id;
  //ecall_load_table_enclave()
  ecall_load_table(eid, &tm, left, sizeof(table_t), &right_table_id);
  ecall_load_table(eid, &tm, right, sizeof(table_t), &left_table_id);
  ecall_hash_join(eid, left_table_id, right_table_id, &def, sizeof(join_def_t), &output_table_id);
  ecall_get_table(eid, &tm, output_table_id);
  sgx_destroy_enclave(eid);
}


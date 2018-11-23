#include "VaultDB_u.h"
#include "VaultDBSGXApp.h"
#include "sgx_urts.h"
#include <gflags/gflags.h>

DEFINE_string(enclave_path, "./sgx/Enclave/libvaultdb_trusted_signed.so", "path for built enclave");
/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
sgx_enclave_id_t initialize_enclave(void) {
  sgx_launch_token_t token = {0};
  sgx_status_t ret = SGX_ERROR_UNEXPECTED;
  int updated = 0;
  sgx_enclave_id_t eid;

  /* Call sgx_create_enclave to initialize an enclave instance */
  /* Debug Support: set 2nd parameter to 1 */
  ret = sgx_create_enclave(FLAGS_enclave_path.c_str(), SGX_DEBUG_FLAG, &token,
                           &updated, &eid, NULL);
  if (ret != SGX_SUCCESS) {
    throw;
  }
  return eid;
}

table_t *get_table_sgx(sgx_enclave_id_t eid, table_manager_t tm, int table_id) {
  auto *tmp = (table_t *)malloc(sizeof(table_t));
  ecall_get_table_t(eid, &tm, table_id, tmp, sizeof(table_t));
  table_t *t = allocate_table(tmp->num_tuple_pages);
  memcpy(t, tmp, sizeof(table_t));
  for (uint64_t i = 0; i < t->num_tuple_pages; i++) {
    t->tuple_pages[i] = (tuple_page_t *)malloc(PAGE_SIZE);
    ecall_get_tuple_page(eid, &tm, table_id, (int) i, t->tuple_pages[i], PAGE_SIZE);
  }
  return t;
}

int load_table_into_sgx(sgx_enclave_id_t eid, table_manager_t *tm, table_t * t) {

  int table_id;
  ecall_load_table(eid, tm, t, sizeof(table_t), &table_id);
  for (uint64_t i = 0; i < t->num_tuple_pages; i++) {
    ecall_load_tuple_page(eid, tm, table_id, i, t->tuple_pages[i], PAGE_SIZE);
  }
  return table_id;
}


table_t *hash_join_sgx(table_t *left, table_t *right, join_def_t def) {

  sgx_enclave_id_t eid = initialize_enclave();
  table_manager_t tm;
  memset(&tm, '\0', sizeof(table_manager_t));
  int left_table_id, right_table_id, output_table_id;

  left_table_id = load_table_into_sgx(eid, &tm, left);
  right_table_id = load_table_into_sgx(eid, &tm, right);
  ecall_hash_join(eid, &tm, left_table_id, right_table_id, &def,
                  sizeof(join_def_t), &output_table_id);
  return get_table_sgx(eid, tm, output_table_id);
}

table_t *aggregate_sgx(table_t *t, groupby_def_t *gb) {
  sgx_enclave_id_t eid = initialize_enclave();
  table_manager_t tm;
  memset(&tm, '\0', sizeof(table_manager_t));
  int table_id, output_table_id;
  table_id = load_table_into_sgx(eid, &tm, t);
  ecall_aggregate(eid, &tm, table_id, gb, sizeof(groupby_def_t), &output_table_id);
  return get_table_sgx(eid, tm, output_table_id);
}

table_t *filter_sgx(table_t *t, expr_t *ex) {
  sgx_enclave_id_t eid = initialize_enclave();
  table_manager_t tm;
  memset(&tm, '\0', sizeof(table_manager_t));
  int table_id, output_table_id;
  table_id = load_table_into_sgx(eid, &tm, t);
  ecall_filter(eid, &tm, table_id, ex, sizeof(expr_t), &output_table_id);
  return get_table_sgx(eid, tm, output_table_id);
}

table_t *sort_sgx(table_t *t, sort_t *s) {
  sgx_enclave_id_t eid = initialize_enclave();
  table_manager_t tm;
  memset(&tm, '\0', sizeof(table_manager_t));
  int table_id, output_table_id;
  table_id = load_table_into_sgx(eid, &tm, t);
  ecall_filter(eid, &tm, table_id, s, sizeof(sort_t), &output_table_id);
  return get_table_sgx(eid, tm, output_table_id);
}

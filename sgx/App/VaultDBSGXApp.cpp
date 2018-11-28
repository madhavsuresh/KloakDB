#include "VaultDB_u.h"
#include "VaultDBSGXApp.h"
#include "sgx_urts.h"
#include <gflags/gflags.h>
#include "logger/LoggerDefs.h"
#include "logger/Logger.h"

DEFINE_string(enclave_path, "./sgx/Enclave/libvaultdb_trusted_signed.so", "path for built enclave");

sgx_enclave_id_t global_eid = 0;

sgx_enclave_id_t get_enclave() {
  return global_eid;
}

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
        {
                SGX_ERROR_UNEXPECTED,
                "Unexpected error occurred.",
                NULL
        },
        {
                SGX_ERROR_INVALID_PARAMETER,
                "Invalid parameter.",
                NULL
        },
        {
                SGX_ERROR_OUT_OF_MEMORY,
                "Out of memory.",
                NULL
        },
        {
                SGX_ERROR_ENCLAVE_LOST,
                "Power transition occurred.",
                "Please refer to the sample \"PowerTransition\" for details."
        },
        {
                SGX_ERROR_INVALID_ENCLAVE,
                "Invalid enclave image.",
                NULL
        },
        {
                SGX_ERROR_INVALID_ENCLAVE_ID,
                "Invalid enclave identification.",
                NULL
        },
        {
                SGX_ERROR_INVALID_SIGNATURE,
                "Invalid enclave signature.",
                NULL
        },
        {
                SGX_ERROR_OUT_OF_EPC,
                "Out of EPC memory.",
                NULL
        },
        {
                SGX_ERROR_NO_DEVICE,
                "Invalid SGX device.",
                "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
        },
        {
                SGX_ERROR_MEMORY_MAP_CONFLICT,
                "Memory map conflicted.",
                NULL
        },
        {
                SGX_ERROR_INVALID_METADATA,
                "Invalid enclave metadata.",
                NULL
        },
        {
                SGX_ERROR_DEVICE_BUSY,
                "SGX device was busy.",
                NULL
        },
        {
                SGX_ERROR_INVALID_VERSION,
                "Enclave version was invalid.",
                NULL
        },
        {
                SGX_ERROR_INVALID_ATTRIBUTE,
                "Enclave was not authorized.",
                NULL
        },
        {
                SGX_ERROR_ENCLAVE_FILE_ACCESS,
                "Can't open enclave file.",
                NULL
        },
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
  size_t idx = 0;
  size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

  for (idx = 0; idx < ttl; idx++) {
    if(ret == sgx_errlist[idx].err) {
      if(NULL != sgx_errlist[idx].sug)
        LOGF(SGX_FATAL, "Info: %s\n", sgx_errlist[idx].sug);
      LOGF(SGX_FATAL, "Error: %s\n", sgx_errlist[idx].msg);
      break;
    }
  }

  if (idx == ttl)
    LOGF(SGX_FATAL, "Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}
/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
sgx_enclave_id_t initialize_enclave(void) {
  sgx_launch_token_t token = {0};
  sgx_status_t ret = SGX_ERROR_UNEXPECTED;
  int updated = 0;

  /* Call sgx_create_enclave to initialize an enclave instance */
  /* Debug Support: set 2nd parameter to 1 */

  LOG(SGX) << "Creating Enclave: " << FLAGS_enclave_path;
  START_TIMER(sgx_create);
  global_eid = sgx_create_enclave(FLAGS_enclave_path.c_str(), SGX_DEBUG_FLAG, &token,
                           &updated, &global_eid, NULL);
  END_AND_LOG_SGX_TIMER(sgx_create);
  if (ret != SGX_SUCCESS) {
    LOG(SGX_FATAL) << "Failed to Create Enclave at " << FLAGS_enclave_path;
    throw;
  }
  return global_eid;
}

table_t *get_table_sgx(sgx_enclave_id_t eid, table_manager_t tm, int table_id) {
  START_TIMER(get_table);
  auto *tmp = (table_t *)malloc(sizeof(table_t));
  LOG(SGX) << "ecall_get_table";
  ecall_get_table_t(eid, &tm, table_id, tmp, sizeof(table_t));
  table_t *t = allocate_table(tmp->num_tuple_pages);
  memcpy(t, tmp, sizeof(table_t));
  for (uint64_t i = 0; i < t->num_tuple_pages; i++) {
    t->tuple_pages[i] = (tuple_page_t *)malloc(PAGE_SIZE);
    ecall_get_tuple_page(eid, &tm, table_id, (int) i, t->tuple_pages[i], PAGE_SIZE);
  }
  free(tmp);
  END_AND_LOG_SGX_TIMER(get_table);
  return t;
}

int load_table_into_sgx(sgx_enclave_id_t eid, table_manager_t *tm, table_t * t) {

  START_TIMER(load_table);
  int table_id;
  LOG(SGX) << "ecall_load_table";
  ecall_load_table(eid, tm, t, sizeof(table_t), &table_id);
  for (uint64_t i = 0; i < t->num_tuple_pages; i++) {
    ecall_load_tuple_page(eid, tm, table_id, i, t->tuple_pages[i], PAGE_SIZE);
  }
  END_AND_LOG_SGX_TIMER(load_table);
  return table_id;
}


table_t *hash_join_sgx(table_t *left, table_t *right, join_def_t def) {

  START_TIMER(hash_join_op);
  sgx_enclave_id_t eid = get_enclave();
  table_manager_t tm;
  memset(&tm, '\0', sizeof(table_manager_t));
  int left_table_id, right_table_id, output_table_id;

  left_table_id = load_table_into_sgx(eid, &tm, left);
  right_table_id = load_table_into_sgx(eid, &tm, right);
  START_TIMER(hash_join_inner);
  ecall_hash_join(eid, &tm, left_table_id, right_table_id, &def,
                  sizeof(join_def_t), &output_table_id);
  ecall_free_table(eid, &tm, left_table_id);
  ecall_free_table(eid, &tm, right_table_id);
  END_AND_LOG_SGX_TIMER(hash_join_inner);
  auto out_t = get_table_sgx(eid, tm, output_table_id);
  END_AND_LOG_SGX_TIMER(hash_join_op);
  ecall_free_table(eid, &tm, output_table_id);
  return out_t;
}

table_t *aggregate_sgx(table_t *t, groupby_def_t *gb) {
  START_TIMER(aggregate_op);
  sgx_enclave_id_t eid = get_enclave();
  table_manager_t tm;
  memset(&tm, '\0', sizeof(table_manager_t));
  int table_id, output_table_id;
  table_id = load_table_into_sgx(eid, &tm, t);
  START_TIMER(aggregate_inner);
  ecall_aggregate(eid, &tm, table_id, gb, sizeof(groupby_def_t), &output_table_id);
  ecall_free_table(eid, &tm, table_id);
  END_AND_LOG_SGX_TIMER(aggregate_inner);
  auto out_t = get_table_sgx(eid, tm, output_table_id);
  END_AND_LOG_SGX_TIMER(aggregate_op);
  ecall_free_table(eid, &tm, output_table_id);
  return out_t;
}

table_t *filter_sgx(table_t *t, expr_t *ex) {
  START_TIMER(filter_op);
  sgx_enclave_id_t eid = get_enclave();
  table_manager_t tm;
  memset(&tm, '\0', sizeof(table_manager_t));
  int table_id, output_table_id;
  table_id = load_table_into_sgx(eid, &tm, t);
  START_TIMER(filter_inner);
  ecall_filter(eid, &tm, table_id, ex, sizeof(expr_t), &output_table_id);
  END_AND_LOG_SGX_TIMER(filter_inner);
  auto out_t = get_table_sgx(eid, tm, output_table_id);
  END_AND_LOG_SGX_TIMER(filter_op);
  ecall_free_table(eid, &tm, output_table_id);
  return out_t;
}

table_t *sort_sgx(table_t *t, sort_t *s) {
  START_TIMER(sort_op);
  sgx_enclave_id_t eid = get_enclave();
  table_manager_t tm;
  memset(&tm, '\0', sizeof(table_manager_t));
  int table_id, output_table_id;
  table_id = load_table_into_sgx(eid, &tm, t);
  START_TIMER(sort_inner);
  ecall_filter(eid, &tm, table_id, s, sizeof(sort_t), &output_table_id);
  END_AND_LOG_SGX_TIMER(sort_inner);
  auto out_t =  get_table_sgx(eid, tm, output_table_id);
  END_AND_LOG_SGX_TIMER(sort_op);
  ecall_free_table(eid, &tm, output_table_id);
  return out_t;
}

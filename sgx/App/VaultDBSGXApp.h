#include <sgx_eid.h>
#include "data/postgres_client.h"

sgx_enclave_id_t initialize_enclave(void);
table_t *hash_join_sgx(table_t *left, table_t *right, join_def_t def);
table_t *filter_sgx(table_t *t, expr_t *ex);

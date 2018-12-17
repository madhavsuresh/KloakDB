#include <sgx_eid.h>
#include <operators/Generalize.h>
#include "data/postgres_client.h"
#include "operators/Sort.h"

sgx_enclave_id_t get_enclave();
sgx_enclave_id_t initialize_enclave(void);
table_t *hash_join_sgx(table_t *left, table_t *right, join_def_t def);
table_t *filter_sgx(table_t *t, expr_t *ex);
table_t *sort_sgx(table_t *t, sort_t *s);
table_t *aggregate_sgx(table_t *t, groupby_def_t *gb);
table_t *generalize_table_fast_sgx(
        std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
        table_map_host_table_pairs,
        int num_hosts, int k);

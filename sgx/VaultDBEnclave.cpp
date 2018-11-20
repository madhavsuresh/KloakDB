#include "sgx_trts.h"
#include "VaultDB_t.h"


void ecall_load_table_enclave(void *table, size_t len) {

}

void ecall_load_table(void *table_manager, void *table_t, size_t table_t_len, int *output_table_id) {

}

void ecall_hash_join(void* table_manager, int left_table_id, int right_table_id, void* join_def, size_t jd_len, int* output_table_id) {

}
void ecall_get_table_t(void* table_manger, int table_id, void* t, size_t len) {

}
void ecall_get_tuple_page(void* table_manager, int table_id, int page_no, void* tuple_page, size_t len) {

}


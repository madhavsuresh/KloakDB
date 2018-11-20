#include "sgx_trts.h"
#include "VaultDB_t.h"
#include "data/postgres_client.h"
#include "operators/HashJoin.h"


void ecall_load_table_enclave(void *table, size_t len) {

}

void ecall_load_table(void *table_manager, void *table, size_t table_t_len, int *output_table_id) {
  table_manager_t *tm = (table_manager_t*) table_manager;
  table_t *tmp_t = (table_t*) table;
  table_t * curr_table;// = allocate_table(tmp_t->num_tuple_pages);
  memcpy(curr_table, table, table_t_len);
  tuple_page_t * scratch = (tuple_page_t *) malloc(PAGE_SIZE);

  for (int i = 0; i < curr_table->num_tuple_pages; i++) {
    curr_table->tuple_pages[i] = (tuple_page_t *) malloc(PAGE_SIZE);
    if (curr_table->tuple_pages[i] == NULL) {
      throw;
    }
    ocall_copy_page(curr_table->tuple_pages[i], PAGE_SIZE, i);
  }
  int tid; // = insert_into_table_manager(tm,curr_table);
  join_def_t def;
  hash_join(curr_table,curr_table, def);
  *output_table_id = tid;
}

void ecall_hash_join(void* table_manager, int left_table_id, int right_table_id, void* join_def, size_t jd_len, int* output_table_id) {

}
void ecall_get_table_t(void* table_manger, int table_id, void* t, size_t len) {

}
void ecall_get_tuple_page(void* table_manager, int table_id, int page_no, void* tuple_page, size_t len) {

}


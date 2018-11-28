#include "sgx_trts.h"
#include "VaultDB_t.h"
#include "data/postgres_client.h"
#include "operators/HashJoin.h"
#include "operators/Aggregate.h"
#include "operators/Filter.h"
#include "operators/Sort.h"


void ecall_load_table_enclave(void *table, size_t len) {

}

void ecall_load_table(void *table_manager, void *table, size_t table_t_len, int *output_table_id) {
  auto*tm = (table_manager_t*) table_manager;
  auto *tmp_t = (table_t*) table;
  table_t * curr_table = allocate_table(tmp_t->num_tuple_pages);
  memcpy(curr_table, table, table_t_len);
  auto tid = insert_into_table_manager(tm,curr_table);
  *output_table_id = tid;
}

void ecall_load_tuple_page(void* table_manager, int table_id, int page_no, void* tuple_page, size_t len) {
  auto *tm = (table_manager_t *) table_manager;
  table_t *t = get_table_table_manager(tm, table_id);
  t->tuple_pages[page_no] = (tuple_page_t *) malloc(len);
  memcpy(t->tuple_pages[page_no], tuple_page, len);
}

void ecall_free_table(void *table_manager, int table_id) {
  auto *tm = (table_manager_t *) table_manager;
  table_t* t = get_table_table_manager(tm, table_id);
  free_table(t);
}

void ecall_hash_join(void* table_manager, int left_table_id, int right_table_id, void* join_def, size_t jd_len, int* output_table_id) {
  auto *tm = (table_manager_t*) table_manager;
  table_t* left = get_table_table_manager(tm, left_table_id);
  table_t* right = get_table_table_manager(tm, right_table_id);
  join_def_t jd;
  memcpy(&jd, join_def, jd_len);
  table_t *output = hash_join(left, right, jd);
  auto tid = insert_into_table_manager(tm, output);
  *output_table_id = tid;
}
void ecall_get_table_t(void* table_manager, int table_id, void* t, size_t len) {
  auto *tm = (table_manager_t*) table_manager;
  table_t* table = get_table_table_manager(tm, table_id);
  memcpy(t, table, len);
  t = table;
  len = sizeof(table_t);
}
void ecall_get_tuple_page(void* table_manager, int table_id, int page_no, void* tuple_page, size_t len) {
  table_manager_t *tm = (table_manager_t*) table_manager;
  table_t* table = get_table_table_manager(tm, table_id);
  memcpy(tuple_page, table->tuple_pages[page_no], len);
}
void ecall_sort(void *table_manager, int table_id, void *sort_def, size_t sd_len, int *output_table_id) {
  auto *tm = (table_manager_t*) table_manager;
  table_t* t = get_table_table_manager(tm, table_id);
  sort_t * s = (sort_t*) sort_def;
  table_t * output = sort(t, s);
  auto tid = insert_into_table_manager(tm, output);
  *output_table_id = tid;
}

void ecall_filter(void *table_manager, int table_id,  void *expr_buf, size_t eb_len, int *output_table_id) {
  auto *tm = (table_manager_t*) table_manager;
  table_t* t = get_table_table_manager(tm, table_id);
  expr_t * ex = (expr_t *) expr_buf;
  table_t * output = filter(t, ex);
  auto tid = insert_into_table_manager(tm, output);
  *output_table_id = tid;
}

void ecall_aggregate(void *table_manager, int table_id,  void *gb_def, size_t gbd_len,  int *output_table_id) {
  auto *tm = (table_manager_t*) table_manager;
  table_t* t = get_table_table_manager(tm, table_id);
  groupby_def_t *gb = (groupby_def_t*)gb_def;
  table_t * output = aggregate(t, gb);
  auto tid = insert_into_table_manager(tm, output);
  *output_table_id = tid;
}


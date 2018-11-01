#include <sys/types.h>
#include "sgx_trts.h"
#include "sgx_lfence.h"
#include "../Enclave.h"
#include "Enclave_t.h"
#include "postgres_client.h"
#include "HashJoin.h"
#include "Sort.h"
#include <chrono>


void many_hashes(table_t * curr_table) {
	join_def_t jd;
	jd.l_col = 1;
	jd.r_col = 1;
	jd.project_len = 1;
	jd.project_list[0].col_no = 1;
	jd.project_list[0].side = LEFT_RELATION;

	table_t * output = hash_join(curr_table, curr_table, jd);
	printf("output size: %d\n", output->num_tuples);
	jd.l_col = 1;
	jd.r_col = 0;
	jd.project_len = 1;
	jd.project_list[0].col_no = 1;
	jd.project_list[0].side = LEFT_RELATION;
	table_t * output2 = hash_join(curr_table, output, jd);

	table_t * output3 = hash_join(curr_table, output2, jd);
	sort_t s;
	s.ascending = true;
	s.colno = 0;
	printf("*output size 2: %d, pages: %d\n*", output2->num_tuples, output2->num_tuple_pages);
	printf("*output size 3: %d, pages: %d\n*", output3->num_tuples, output3->num_tuple_pages);
}

void ecall_load_table_enclave(void *ptr, size_t len) {
  //TODO(madhavsuresh): variable length tables are ANNOYING
    table_t *tmp_table = (table_t *) ptr;
    table_t * curr_table = allocate_table(tmp_table->num_tuple_pages);
	memcpy(curr_table, ptr, len);
	//table_t* curr_table = (table_t *) ptr;
	printf("num_tuples: %d\n", curr_table->num_tuples);
	tuple_page_t * scratch = (tuple_page_t *) malloc(PAGE_SIZE);
	for (int i = 0; i < curr_table->num_tuple_pages; i++) {
		curr_table->tuple_pages[i] = (tuple_page_t *) malloc(PAGE_SIZE);
		if (curr_table->tuple_pages[i] == NULL) {
			printf("FAILED MALLOC %d, [%d]\n", i, curr_table->num_tuple_pages);
			printf("size of tuple: %d, total bytes: %d", curr_table->size_of_tuple, curr_table->num_tuple_pages*PAGE_SIZE);
			return;
		}
		ocall_copy_page(curr_table->tuple_pages[i], PAGE_SIZE, i);
	}
	many_hashes(curr_table);
}


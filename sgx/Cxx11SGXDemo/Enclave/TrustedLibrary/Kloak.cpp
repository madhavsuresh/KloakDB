#include <sys/types.h>
#include "sgx_trts.h"
#include "sgx_lfence.h"
#include "../Enclave.h"
#include "Enclave_t.h"
#include "postgres_client.h"


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
		printf("%p\n", curr_table->tuple_pages[i]);

		if (curr_table->tuple_pages[i] == NULL) {
			printf("FAILED MALLOC %d, [%d]\n", i, curr_table->num_tuple_pages);
			return;
		} else {
			printf("tuple_page [%d]", i);

		}
		ocall_copy_page(curr_table->tuple_pages[i], PAGE_SIZE, i);
	}

	for (int i = 0; i < curr_table->num_tuples; i++) {
		std::string tup = tuple_string(get_tuple(i, curr_table));
		printf("%s", tup.c_str());
	}
}

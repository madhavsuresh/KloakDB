#include <sys/types.h>
#include "sgx_trts.h"
#include "sgx_lfence.h"
#include "../Enclave.h"
#include "Enclave_t.h"
#include "postgres_client.h"


void ecall_load_table_enclave(void *ptr, size_t len) {
	table_t* curr_table = (table_t *) ptr;
	printf("num_tuples: %d\n", curr_table->num_tuples);
	for (int i = 0; i < curr_table->num_tuple_pages; i++) {
		curr_table->tuple_pages[i] = (tuple_page_t *) malloc(PAGE_SIZE);
		ocall_copy_page(curr_table->tuple_pages[i], PAGE_SIZE, i);
	}

	for (int i = 0; i < curr_table->num_tuples; i++) {
		std::string tup = tuple_string(get_tuple(i, curr_table));
		printf("%s\n", tup.c_str());
	}
}

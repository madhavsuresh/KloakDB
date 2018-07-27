#include "vaultdb_generated.h"
#include <pqxx/pqxx>
	//OID constants taken from postgres/catalog/pg_type.h, not included in Ubuntu 16.04 postgres package. These are global constants set in postgres
#define VARCHAROID	1043
#define INT4OID		23
#define TIMESTAMPOID	1114

#define FIELD_NAME_LEN 16
#define FIXEDCHAR_LEN 16
#define MAX_FIELDS 10
#define PAGE_SIZE 32768

// invariant that every table has the same size tuple

enum FIELD_TYPE {
    FIXEDCHAR, INT, UNSUPPORTED
};

typedef struct field_desc {
    char field_name[FIELD_NAME_LEN];
    uint32_t col_no;
    FIELD_TYPE type;
    char unused[8];
} field_desc_t;

//schema is a fixed size of 328 bytes
typedef struct schema {
    uint32_t num_fields;
    uint32_t unused;
    field_desc_t fields[MAX_FIELDS];
} schema_t;

typedef struct field_int {
    uint64_t val;
    uint64_t genval;
} field_int_t;

typedef struct field_fixed_char {
    char val[FIXEDCHAR_LEN];
} field_fixed_char_t;

union field_union {
    field_int_t int_field;
    field_fixed_char_t fixed_char_field;
};

typedef struct field {
    field_union f;
    FIELD_TYPE type;
} field_t;

typedef struct tuple {
    bool is_dummy;
    int num_fields;
    field_t field_list[];
} tuple_t;

typedef struct tuple_page {
    int page_no;
    int num_tuples;
    tuple_t tuple_list[];
} tuple_page_t;


typedef struct table {
    int num_tuple_pages;
    int num_tuples;
    schema_t schema;
    tuple_page_t *tuple_pages[];
} table_t;

typedef struct table_builder {
    uint64_t curr_tuple;
    uint32_t num_pages_allocated;
    uint32_t curr_page;
    uint64_t expected_pages;
    uint64_t expected_tuples;
    uint32_t num_columns;
    table_t *table;
    const pqxx::result res;
} table_builder_t;

//TODO(madhavsuresh): might need to worry about blocks


flatbuffers::DetachedBuffer postgres_query_writer(std::string query_string, std::string dbname);

pqxx::result query(std::string query_string, std::string dbname);

schema_t *get_schema_from_query(pqxx::result res, table_t *t);

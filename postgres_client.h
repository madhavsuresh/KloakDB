#ifndef PROJECT_POSTGRES_CLIENT_H
#define PROJECT_POSTGRES_CLIENT_H
#include <pqxx/pqxx>
// OID constants taken from postgres/catalog/pg_type.h, not included in
// Ubuntu 16.04 postgres package. These are global constants set in postgres
#define VARCHAROID 1043
#define INT8OID 20
#define INT4OID 23
#define TIMESTAMPOID 1114

#define FIELD_LEN 16
#define FIELD_NAME_LEN FIELD_LEN
#define FIXEDCHAR_LEN 16
#define MAX_FIELDS 10
#define PAGE_SIZE 32768 // 32k sized pages. This could be much bigger.
#define LEFT_RELATION 0
#define RIGHT_RELATION 1

// invariant that every table has the same size tuple

//TODO(madhavsuresh): change to intfield
enum FIELD_TYPE { UNSUPPORTED, FIXEDCHAR, INT };

enum FILTER_EXPR { UNSUPPORTED_EXPR, EQ_EXPR };

typedef struct field_desc {
  char field_name[FIELD_NAME_LEN];
  uint32_t col_no;
  FIELD_TYPE type;
  char unused[8];
} field_desc_t;

// schema is a fixed size of 328 bytes
typedef struct schema {
  uint32_t num_fields;
  uint32_t unused;
  field_desc_t fields[MAX_FIELDS];
} schema_t;

typedef struct join_col_ID {
    int32_t side; //Either 0, or 1. 0-> left table, 1-> right table
    int32_t col_no;
} join_colID_t;

typedef struct join_def {
    int32_t l_col; // Column to join on in left relation.
    int32_t r_col; // Column to join on in right relation.
    join_colID_t project_list[MAX_FIELDS]; // ordered list of columns to project after join.
    int32_t project_len; // Number of fields in project list
} join_def_t;

typedef struct field_int {
  int64_t val;
  int64_t genval;
} field_int_t;

typedef struct field_fixed_char {
  char val[FIXEDCHAR_LEN];
} field_fixed_char_t;

union field_union {
  field_int_t int_field;
  field_fixed_char_t fixed_char_field;
};

// FIXME(madhav): if this causes performance problems
// TODO(madhav): deal with sizeof more elegantly
typedef struct field {
  field_union f;
  FIELD_TYPE type;
} field_t;

typedef struct expr {
  FILTER_EXPR expr_type;
  field_t field_val;
  int colno;
} expr_t;

typedef struct tuple {
  // IF ANY FIELDS GET ADDED
  // THE SIZE FUNCTION MUST BE
  // MODIFIED
  bool is_dummy;
  uint64_t num_fields;
  field_t field_list[];
} tuple_t;

typedef struct tuple_page {
  uint64_t page_no;
  tuple_t tuple_list[];
} tuple_page_t;

typedef struct table {
  uint32_t num_tuple_pages;
  uint32_t num_tuples;
  uint32_t size_of_tuple;
  schema_t schema;
  tuple_page_t *tuple_pages[];
} table_t;

typedef struct table_builder {
  uint32_t num_pages_allocated;
  uint32_t curr_page;
  uint64_t curr_tuple;
  uint64_t num_tuples_per_page;
  uint64_t expected_pages;
  uint64_t expected_tuples;
  uint32_t size_of_tuple;
  uint32_t num_columns;
  table_t *table;
  pqxx::result res;
} table_builder_t;

// TODO(madhavsuresh): might need to worry about blocks

pqxx::result query(std::string query_string, std::string dbname);

schema_t get_schema_from_query(table_builder_t *tb, pqxx::result res);
table_builder_t *table_builder(std::string query_string, std::string dbname);
tuple_t *get_tuple_from_page(int tuple_number, tuple_page_t *tp,
                             table_t *table);
tuple_t *get_tuple(int tuple_number, table_t *table);
int get_int_field(tuple_t *tup, int field_no);
table_t *get_table(std::string query_string, std::string dbname);
void free_table(table_t *table);
expr_t make_int_expr(FILTER_EXPR type, uint64_t field_val, int colno);
void print_tuple(tuple_t *t);
bool check_add_tuple_page(table_builder_t *tb);
tuple_page_t *add_tuple_page(table_builder_t *tb);
void init_table_builder(int expected_tuples, int num_columns, schema_t *schema,
                        table_builder_t *tb);
void copy_tuple_to_position(table_t *t, int pos, tuple_t *tup);
table_t *copy_table_by_index(table_t *t, std::vector<int> index_list);
table_t *allocate_table(int num_tuple_pages);
void init_table_builder_from_pq(pqxx::result res, table_builder_t *tb);
void append_tuple(table_builder_t *tb, tuple_t *tup);
table_t *coalesce_tables(std::vector<table_t *> tables);
void print_tuple_log(int i, tuple_t *t);
std::string tuple_string(tuple_t * t);;
#endif // PROJECT_POSTGRES_CLIENT_H

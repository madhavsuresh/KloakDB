#ifndef PROJECT_POSTGRES_CLIENT_H
#define PROJECT_POSTGRES_CLIENT_H
// OID constants taken from postgres/catalog/pg_type.h, not included in
// Ubuntu 16.04 postgres package. These are global constants set in postgres
#include <cstdint>
#include <string>
#include <vector>

#define VARCHAROID 1043
#define INT8OID 20
#define INT4OID 23
#define TIMESTAMPOID 1114
#define FLOAT4OID 700
#define FLOAT8OID 701

#define FIELD_LEN 20
#define FIELD_NAME_LEN FIELD_LEN
#define FIXEDCHAR_LEN 16
#define MAX_FIELDS 20
#define PAGE_SIZE 32768 // 32k sized pages. This could be much bigger.
#define LEFT_RELATION 0
#define RIGHT_RELATION 1

// invariant that every table has the same size tuple

// TODO(madhavsuresh): change to intfield
enum FIELD_TYPE { UNSUPPORTED, FIXEDCHAR, INT, TIMESTAMP, DOUBLE };

enum FILTER_EXPR { UNSUPPORTED_EXPR, EQ_EXPR, LIKE_EXPR };

typedef struct field_desc {
  char field_name[FIELD_NAME_LEN];
  int32_t col_no;
  FIELD_TYPE type;
  char unused[8];
} field_desc_t;

// schema is a fixed size of 328 bytes
typedef struct schema {
  int32_t num_fields;
  uint32_t unused;
  field_desc_t fields[MAX_FIELDS];
} schema_t;

typedef struct join_col_ID {
  int32_t side; // Either 0, or 1. 0-> left table, 1-> right table
  int32_t col_no;
} join_colID_t;

typedef struct join_def {
  int32_t l_col;                         // Column to join on in left relation.
  int32_t r_col;                         // Column to join on in right relation.
  join_colID_t project_list[MAX_FIELDS]; // ordered list of columns to project
                                         // after join.
  int32_t project_len;                   // Number of fields in project list
} join_def_t;

enum GROUPBY_TYPE { GROUPBY_UNSUPPORTED, MINX, COUNT, AVG };

typedef struct groupby_def {
  GROUPBY_TYPE type;
  uint8_t colno;
  int32_t num_cols;
  uint8_t gb_colnos[MAX_FIELDS];
} groupby_def_t;

typedef struct field_int {
  int64_t val;
  int64_t genval;
} field_int_t;

typedef struct field_double {
  double val;
  double genval;
} field_double_t;

typedef struct field_ts {
  time_t val;
  time_t genval;
} field_ts_t;

typedef struct field_fixed_char {
  char val[FIXEDCHAR_LEN];
} field_fixed_char_t;

union field_union {
  field_int_t int_field;
  field_fixed_char_t fixed_char_field;
  field_ts_t ts_field;
  field_double_t double_field;
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
  uint64_t num_tuples;
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
} table_builder_t;

// TODO(madhavsuresh): might need to worry about blocks

tuple_t *get_tuple_from_page(int tuple_number, tuple_page_t *tp,
                             table_t *table);
tuple_t *get_tuple(int tuple_number, table_t *table);
int64_t get_int_field(tuple_t *tup, int field_no);
void free_table(table_t *table);
expr_t make_int_expr(FILTER_EXPR type, uint64_t field_val, int colno);
bool check_add_tuple_page(table_builder_t *tb);
void add_tuple_page(table_builder_t *tb);
void init_table_builder(uint64_t expected_tuples, int num_columns,
                        schema_t *schema, table_builder_t *tb);
void copy_tuple_to_position(table_t *t, int pos, tuple_t *tup);
table_t *copy_table_by_index(table_t *t, std::vector<int> index_list);
table_t *allocate_table(int num_tuple_pages);
void append_tuple(table_builder_t *tb, tuple_t *tup);
table_t *coalesce_tables(std::vector<table_t *> tables);
std::string tuple_string(tuple_t *t);
int colno_from_name(table_t *t, std::string colname);
double get_num_field(table_t *t, int tuple_no, int colno);
bool compare_tuple_cols_val(tuple_t *t1, tuple_t *t2, int t1_col, int t2_col);
#endif // PROJECT_POSTGRES_CLIENT_H

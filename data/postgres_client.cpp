#include "postgres_client.h"
#include <cstring>
#include <iostream>

uint64_t tuples_per_page(uint64_t tuple_size) {
  return (PAGE_SIZE - sizeof(uint64_t)) / tuple_size;
}

int colno_from_name(table_t *t, std::string colname) {
  for (int i = 0; i < t->schema.num_fields; i++) {
    if (strncmp(colname.c_str(), t->schema.fields[i].field_name,
                FIELD_NAME_LEN) == 0) {
      return i;
    }
  }
  throw std::invalid_argument("Column does not exist");
}

std::string tuple_string(tuple_t *t) {
  std::string output = "";
  for (int i = 0; i < t->num_fields; i++) {
    switch (t->field_list[i].type) {
    case FIXEDCHAR: {
      output += std::string(t->field_list[i].f.fixed_char_field.val);
      break;
    }
    case INT: {
      output += std::to_string(t->field_list[i].f.int_field.val);
      //output += ",[gv]:(" + std::to_string(t->field_list[i].f.int_field.genval) + ")";
      break;
    }
    case TIMESTAMP: {
      time_t time = t->field_list[i].f.int_field.val;
      //TODO(madhavsuresh): HIGH PRIORITY FIX THIS TIME BUG
      char *timetext; // = asctime(gmtime(&time));
      timetext[24] = '\0';
      output += std::string(timetext);
      break;
    }
    case DOUBLE_V: {
      output += std::to_string(t->field_list[i].f.double_field.val);
      break;
    }
    default: {
      throw std::invalid_argument("Cannot print this tuple");
    }
    }
    output += "| ";
  }
    output += "(" + std::to_string(t->is_dummy) + ")";
  return output;
}

expr_t make_string_expr(FILTER_EXPR type, std::string field_val, int colno) {
  expr_t expr;
  expr.field_val.type = FIXEDCHAR;
  strncpy(expr.field_val.f.fixed_char_field.val, field_val.c_str(), FIXEDCHAR_LEN);
  expr.colno = colno;
  expr.expr_type = type;
  return expr;
}

expr_t make_int_expr(FILTER_EXPR type, uint64_t field_val, int colno) {
  expr_t expr;
  expr.field_val.type = INT;
  expr.field_val.f.int_field.val = field_val;
  expr.colno = colno;
  expr.expr_type = type;
  return expr;
}

void free_table(table_t *t) {
  // TODO(madhavsuresh): there is a memory leak here!
  for (int i = 0; i < t->num_tuple_pages; i++) {
    free(t->tuple_pages[i]);
  }
  free(t);
}

tuple_page_t *allocate_tuple_page(table_builder_t *tb) {
  tb->table->tuple_pages[tb->curr_page] = (tuple_page_t *)malloc(PAGE_SIZE);
  memset(tb->table->tuple_pages[tb->curr_page], '\0', PAGE_SIZE);
  tb->table->tuple_pages[tb->curr_page]->page_no = tb->curr_page;
  tb->num_pages_allocated++;
  tb->table->num_tuple_pages++;
  return tb->table->tuple_pages[tb->curr_page];
}

void initialize_tuple_page(table_builder_t *tb) {
  tb->curr_page = 0;
  allocate_tuple_page(tb);
}

void add_tuple_page(table_builder_t *tb) {
  tb->curr_page++;
  if (tb->curr_page > MAX_NUM_PAGES) {
    throw;
  }
  allocate_tuple_page(tb);
}

tuple_t *get_tuple_from_page(int page_tuple_num, tuple_page_t *tp,
                             table_t *table) {
  return (tuple_t *)(((char *)tp->tuple_list) +
                     (table->size_of_tuple * page_tuple_num));
}

int64_t get_int_field(tuple_t *tup, int field_no) {
  return tup->field_list[field_no].f.int_field.val;
}

tuple_page_t *get_page(int page_num, table_t *table) {
  return table->tuple_pages[page_num];
}

int get_page_num_for_tuple(int tuple_number, table_t *table) {
  int num_tuples_per_page = (int)tuples_per_page(table->size_of_tuple);
  int page_num = (tuple_number) / (num_tuples_per_page);
  return page_num;
}

tuple_t *get_tuple(int tuple_number, table_t *table) {
  int num_tuples_per_page = (int)tuples_per_page(table->size_of_tuple);
  int page_num = get_page_num_for_tuple(tuple_number, table);
  tuple_page_t *tp = get_page(page_num, table);
  int page_tuple_num = tuple_number % num_tuples_per_page;
  return (tuple_t *)(((char *)tp->tuple_list) +
                     (table->size_of_tuple * page_tuple_num));
}

void copy_tuple_to_position(table_t *t, int pos, tuple_t *tup) {
  tuple_t *curr_tup = get_tuple(pos, t);
  // TODO(madhavsuresh): are tuple sizes invariant
  memcpy(curr_tup, tup, t->size_of_tuple);
}

bool check_add_tuple_page(table_builder_t *tb) {

  if (0 == tb->curr_tuple % tb->num_tuples_per_page && tb->curr_tuple > 0) {
    return true;
  }
  return false;
}

table_t *allocate_table(uint64_t num_tuple_pages) {
  table_t *ret = (table_t *)malloc(sizeof(table_t) +
                                   num_tuple_pages * sizeof(tuple_page_t *));
  if (ret == nullptr) {
    throw std::invalid_argument("Malloc failed");
  }
  return ret;
}

bool compare_tuple_cols_val(tuple_t *t1, tuple_t *t2, int t1_col, int t2_col) {
  if (t1->field_list[t1_col].type != t2->field_list[t2_col].type) {
    throw;
  }
  switch (t1->field_list[t1_col].type) {
  case UNSUPPORTED: {
    throw;
  }
  case FIXEDCHAR: {
    if (strncmp(t1->field_list[t1_col].f.fixed_char_field.val,
                t2->field_list[t2_col].f.fixed_char_field.val,
                FIXEDCHAR_LEN) == 0) {
      return true;
    } else {
      return false;
    }
  }
  case INT: {
    if (t1->field_list[t1_col].f.int_field.val ==
        t2->field_list[t2_col].f.int_field.val) {
      return true;
    } else {
      return false;
    }
  }
  case TIMESTAMP: {
    if (t1->field_list[t1_col].f.ts_field.val ==
        t2->field_list[t2_col].f.ts_field.val) {
      return true;
    } else {
      return false;
    }
  }
  case DOUBLE_V: {
    if (t1->field_list[t1_col].f.double_field.val ==
        t2->field_list[t2_col].f.double_field.val) {
      return true;
    } else {
      return false;
    }
  }
  }
}

void init_table_builder(uint64_t expected_tuples, int num_columns,
                        schema_t *schema, table_builder_t *tb) {

  tb->expected_tuples = expected_tuples;
  tb->num_columns = num_columns;
  tb->num_pages_allocated = 0;
  // DLOG_IF(INFO, tb->num_columns > MAX_FIELDS) << "Max fields exceeded num
  // columns:" << tb->num_columns;
  tb->size_of_tuple = sizeof(tuple) + tb->num_columns * (sizeof(field_t));

  uint64_t total_size = tb->expected_tuples * tb->size_of_tuple;
  tb->expected_pages =
      tb->expected_tuples / tuples_per_page(tb->size_of_tuple) + 1;
  if (tb->expected_pages > MAX_NUM_PAGES) {
    tb->expected_pages = MAX_NUM_PAGES;
  }
  // total_size / PAGE_SIZE + 2;
  // TODO(madhavsuresh): this needs to be abstracted out. this is terrible.
  tb->table = allocate_table(tb->expected_pages); //(table_t *)
                                                  // malloc(sizeof(table_t) +
                                                  // sizeof(tuple_page_t *) *
                                                  // tb->expected_pages);
  memset(tb->table, '\0',
         sizeof(table_t) + sizeof(tuple_page_t *) * tb->expected_pages);
  // Copy schema to new table
  memcpy(&tb->table->schema, schema, sizeof(schema_t));

  tb->table->size_of_tuple = tb->size_of_tuple;
  tb->num_tuples_per_page = tuples_per_page(tb->table->size_of_tuple);
  tb->curr_tuple = 0;
  tb->curr_page = 0;
  // Initialize first page regardless
  initialize_tuple_page(tb);
}

table_t *coalesce_tables(std::vector<table_t *> tables) {
  table_builder_t tb;
  int num_columns = tables[0]->schema.num_fields;
  uint64_t size = 0;
  for (auto t : tables) {
    size += t->num_tuples;
    if (t->schema.num_fields != num_columns) {
      // LOG(FATAL) << "The number of fields across the schemas is not
      // consistent";
      throw;
    }
  }
  init_table_builder(size, num_columns, &tables[0]->schema, &tb);
  for (auto t : tables) {
    for (int i = 0; i < t->num_tuples; i++) {
      tuple_t *tup = get_tuple(i, t);
      append_tuple(&tb, tup);
    }
    free_table(t);
  }
  return tb.table;
}

void append_tuple(table_builder_t *tb, tuple_t *tup) {
  if (check_add_tuple_page(tb)) {
    add_tuple_page(tb);
  }
  tb->table->num_tuples++;
  copy_tuple_to_position(tb->table, tb->curr_tuple, tup);
  tb->curr_tuple++;
}

table_t *copy_table_by_index(table_t *t, std::vector<int> index_list) {
  int expected_tuples = index_list.size();
  int num_columns = t->schema.num_fields;
  table_builder_t tb;

  init_table_builder(expected_tuples, num_columns, &t->schema, &tb);

  for (auto i : index_list) {
    tuple_t *tup = get_tuple(i, t);
    append_tuple(&tb, tup);
  }
  return tb.table;
}

double get_num_field(table_t *t, int tuple_no, int colno) {
  switch (t->schema.fields[colno].type) {
  case UNSUPPORTED: {
    throw std::invalid_argument("Cannot convert unsupported column");
  }
  case FIXEDCHAR: {
    throw std::invalid_argument("Cannot convert fixedchar column");
  }
  case INT: {
    return (double)get_tuple(tuple_no, t)->field_list[colno].f.int_field.val;
  }
  case TIMESTAMP: {
    return (double)get_tuple(tuple_no, t)->field_list[colno].f.ts_field.val;
  }
  case DOUBLE_V: {
    return (double)get_tuple(tuple_no, t)->field_list[colno].f.double_field.val;
  }
  }
  throw;
}

uint32_t insert_into_table_manager(table_manager_t *tm, table_t *t) {
  for (uint32_t i = 0; i < TABLE_MANAGER_MAX_TABLES; i++) {
    if (!tm->allocated_map[i]) {
      tm->table_list[i] = t;
      tm->allocated_map[i] = true;
      return i;
    }
  }
  throw;
}

table_t *get_table_table_manager(table_manager_t *tm, uint32_t pos) {
  if (!tm->allocated_map[pos]) {
    throw;
  }
  return tm->table_list[pos];
}

void free_table_table_manager(table_manager_t *tm, uint32_t pos) {
  if (!tm->allocated_map[pos]) {
    throw;
  }
  free_table(tm->table_list[pos]);
  tm->allocated_map[pos] = false;
}

#include "postgres_client.h"
#include <cstring>
#include <g3log/g3log.hpp>
#include <glog/logging.h>
#include <iostream>

uint64_t tuples_per_page(uint64_t page_size, uint64_t tuple_size) {
  return (PAGE_SIZE - sizeof(uint64_t)) / tuple_size;
}

void print_tuple_log(int ii, tuple_t *t) {
  std::string output;
  if (t->is_dummy) {
    return;
  }
  output+= std::to_string(ii) + "{";
  for (int i = 0; i < t->num_fields; i++) {
    switch (t->field_list[i].type) {
    case FIXEDCHAR: {
      output += std::to_string(i) + ":|" + std::string(t->field_list[i].f.fixed_char_field.val) + "|";
      break;
    }
    case INT: {
      output += std::to_string(i) + ":|" + std::to_string(t->field_list[i].f.int_field.val) + "|";
      //snprintf(output+offset, 8, "%d:|%d|",i, t->field_list[i].f.int_field.val);
      //offset+=8;
      break;
    }
    case UNSUPPORTED: {
      throw;
    }
    }
  }
  LOGF(INFO, "%s}", output.c_str());
}


std::string tuple_string(tuple_t * t) {
  std::string output = "";
  for (int i = 0; i < t->num_fields; i++) {
    switch (t->field_list[i].type) {
      case FIXEDCHAR: {
        output += std::string(t->field_list[i].f.fixed_char_field.val);
        printf("%s", t->field_list[i].f.fixed_char_field.val);
        break;
      }
      case INT : {
        output += std::to_string(t->field_list[i].f.int_field.val);
        break;
      }
      case UNSUPPORTED : {
        throw;
      }
    }
    output += "| ";
  }
  return output;
}

void print_tuple(tuple_t * t) {
  std::cout << tuple_string(t);
}

expr_t make_int_expr(FILTER_EXPR type, uint64_t field_val, int colno) {
  expr_t expr;
  expr.field_val.type = INT;
  expr.field_val.f.int_field.val = field_val;
  expr.colno = colno;
  expr.expr_type = type;
  return expr;
}

pqxx::result query(std::string query_string, std::string dbname) {
  try {
    pqxx::connection c(dbname);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  pqxx::connection c(dbname);
  pqxx::work txn(c);
  pqxx::result res;
  try {
    res = txn.exec(query_string);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  txn.commit();
  txn.commit();
  return res;
}

FIELD_TYPE get_OID_field_type(pqxx::oid oid) {
  switch (oid) {
  case VARCHAROID:
    return FIXEDCHAR;
  case INT4OID:
  case INT8OID:
    return INT;
  default:
    throw;
    // return UNSUPPORTED;
  }
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

tuple_page_t *initialize_tuple_page(table_builder_t *tb) {
  tb->curr_page = 0;
  allocate_tuple_page(tb);
}

tuple_page_t *add_tuple_page(table_builder_t *tb) {
  tb->curr_page++;
  allocate_tuple_page(tb);
}

tuple_t *get_tuple_from_page(int page_tuple_num, tuple_page_t *tp,
                             table_t *table) {
  return (tuple_t *)(((char *)tp->tuple_list) +
                     (table->size_of_tuple * page_tuple_num));
}

int get_int_field(tuple_t *tup, int field_no) {
  return tup->field_list[field_no].f.int_field.val;
}

tuple_t *get_tuple(int tuple_number, table_t *table) {
  int num_tuples_per_page =
      (int)tuples_per_page(PAGE_SIZE, table->size_of_tuple);
  int page_num = (tuple_number) / (num_tuples_per_page);
  tuple_page_t *tp = table->tuple_pages[page_num];
  int page_tuple_num = tuple_number % num_tuples_per_page;
  return (tuple_t *)(((char *)tp->tuple_list) +
                     (table->size_of_tuple * page_tuple_num));
}

void copy_tuple_to_position(table_t *t, int pos, tuple_t *tup) {
  tuple_t *curr_tup = get_tuple(pos, t);
  // TODO(madhavsuresh): are tuple sizes invariant
  memcpy(curr_tup, tup, t->size_of_tuple);
}

void build_tuple_from_pq(pqxx::tuple tup, tuple_t *tuple, schema_t *s) {
  int field_counter = 0;
  tuple->num_fields = s->num_fields;
  for (auto field : tup) {
    switch (s->fields[field_counter].type) {
    case FIXEDCHAR:
      strncpy(tuple->field_list[field_counter].f.fixed_char_field.val,
              field.c_str(), FIXEDCHAR_LEN);
      tuple->field_list[field_counter].type = FIXEDCHAR;
      break;
    case INT:
      tuple->field_list[field_counter].f.int_field.val = field.as<int>();
      tuple->field_list[field_counter].type = INT;
      break;
    case UNSUPPORTED:
      throw;
    }
    field_counter++;
  }
}

bool check_add_tuple_page(table_builder_t *tb) {

  if (0 == tb->curr_tuple % tb->num_tuples_per_page && tb->curr_tuple > 0) {
    return true;
  }
  return false;
}

void write_table_from_postgres(pqxx::result res, table_builder_t *tb) {
  // TODO(madhavsuresh): would prefer this to be on the stack
  for (auto psql_row : res) {
    // Don't want to jump on the first tuple
    if (check_add_tuple_page(tb)) {
      add_tuple_page(tb);
    }
    tb->table->num_tuples++;
    // build_tuple_from_pq adds the tuple to the
    build_tuple_from_pq(psql_row, get_tuple(tb->curr_tuple, tb->table),
                        &tb->table->schema);
    tb->curr_tuple++;
    fflush(stdin);
  }
}

table_t *allocate_table(int num_tuple_pages) {
  return (table_t *)malloc(sizeof(table_t) +
                           num_tuple_pages * sizeof(tuple_page_t *));
}

void init_table_builder(int expected_tuples, int num_columns, schema_t *schema,
                        table_builder_t *tb) {

  tb->expected_tuples = expected_tuples;
  tb->num_columns = num_columns;
  // DLOG_IF(INFO, tb->num_columns > MAX_FIELDS) << "Max fields exceeded num
  // columns:" << tb->num_columns;
  tb->size_of_tuple = sizeof(tuple) + tb->num_columns * (sizeof(field_t));

  uint64_t total_size = tb->expected_tuples * tb->size_of_tuple;
  tb->expected_pages =
      tb->expected_tuples / tuples_per_page(PAGE_SIZE, tb->size_of_tuple) + 1;
  // total_size / PAGE_SIZE + 2;
  // TODO(madhavsuresh): this needs to be abstracted out. this is terrible.
  tb->table = allocate_table(tb->expected_pages); //(table_t *)
                                                  //malloc(sizeof(table_t) +
                                                  //sizeof(tuple_page_t *) *
                                                  //tb->expected_pages);
  memset(tb->table, '\0',
         sizeof(table_t) + sizeof(tuple_page_t *) * tb->expected_pages);
  // Copy schema to new table
  memcpy(&tb->table->schema, schema, sizeof(schema_t));

  tb->table->size_of_tuple = tb->size_of_tuple;
  tb->num_tuples_per_page =
      tuples_per_page(PAGE_SIZE, tb->table->size_of_tuple);
  tb->curr_tuple = 0;
  tb->curr_page = 0;
  // Initialize first page regardless
  initialize_tuple_page(tb);
}

table_t *coalesce_tables(std::vector<table_t *> tables) {
  table_builder_t tb;
  int num_columns = tables[0]->schema.num_fields;
  int size = 0;
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

void init_table_builder_from_pq(pqxx::result res, table_builder_t *tb) {
  schema_t schema = get_schema_from_query(tb, res);
  init_table_builder(res.capacity(), res.columns(), &schema, tb);
}

table_t *get_table(std::string query_string, std::string dbname) {
  table_t *t;
  table_builder_t *tb = table_builder(query_string, dbname);
  t = tb->table;
  free(tb);
  return t;
}

table_builder_t *table_builder(std::string query_string, std::string dbname) {
  auto *tb = (table_builder_t *)malloc(sizeof(table_builder_t));
  memset(tb, '\0', sizeof(table_builder_t));
  pqxx::result res = query(query_string, dbname);
  init_table_builder_from_pq(res, tb);
  write_table_from_postgres(res, tb);
  // Everything should be zero-indexed
  return tb;
}

schema_t get_schema_from_query(table_builder_t *tb, pqxx::result res) {
  schema_t schema;
  schema.num_fields = res.columns();
  for (int i = 0; i < schema.num_fields; i++) {
    strncpy(schema.fields[i].field_name, res.column_name(i), FIELD_NAME_LEN);
    schema.fields[i].col_no = (uint32_t)i;
    schema.fields[i].type = get_OID_field_type(res.column_type(i));
  }
  // DLOG(INFO) << "Completed Schema with columns: " << tb->num_columns;
  return schema;
}

//
// Created by madhav on 10/30/18.
//
#include "pqxx_compat.h"
#include "postgres_client.h"
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <logger/LoggerDefs.h>
int tuple_size_breach;

FIELD_TYPE get_OID_field_type(pqxx::oid oid) {
  switch (oid) {
  case VARCHAROID:
    return FIXEDCHAR;
  case NUMERIC:
  case INT4OID:
  case INT8OID:
    return INT;
  case FLOAT4OID:
  case FLOAT8OID:
    return DOUBLE;
  case TIMESTAMPOID:
    return TIMESTAMP;
  default:
    throw std::invalid_argument("Unsupported column type");
    // return UNSUPPORTED;
  }
}

table_t *get_table(std::string query_string, std::string dbname) {
  table_t *t;
  table_builder_t *tb = table_builder_init(query_string, dbname);
  t = tb->table;
  free(tb);
  return t;
}

void build_tuple_from_pq(pqxx::row tup, tuple_t *tuple, schema_t *s, table_builder_t * tb) {
  int field_counter = 0;
  tuple->num_fields = s->num_fields;
  for (auto field : tup) {
    // TODO(madhavsuresh): do better than this!
    // TODO(madhavsuresh): field length is not correct
    if (field.is_null()) {
      field_counter++;
      continue;
    }

    switch (s->fields[field_counter].type) {
    case FIXEDCHAR:
      if (field.size() > FIXEDCHAR_LEN) {
        tuple_size_breach++;
      }
      strncpy(tuple->field_list[field_counter].f.fixed_char_field.val,
              field.c_str(), FIXEDCHAR_LEN);
      tuple->field_list[field_counter].type = FIXEDCHAR;
      break;
    case INT:
      tuple->field_list[field_counter].f.int_field.val = field.as<int>();
      tuple->field_list[field_counter].f.int_field.genval =
          tuple->field_list[field_counter].f.int_field.val;
      tuple->field_list[field_counter].type = INT;
      break;
    case TIMESTAMP: {
      std::tm t = {};
      std::istringstream ss(field.c_str());
      if (ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S")) {
        tuple->field_list[field_counter].f.ts_field.val =
            std::mktime(&t) - timezone;
        tuple->field_list[field_counter].f.ts_field.genval =
            tuple->field_list[field_counter].f.ts_field.val;
        tuple->field_list[field_counter].type = TIMESTAMP;
      } else {
        printf("\n**\n%s\n**\n", field.c_str());
        throw std::invalid_argument("Parsing timezone failed");
      }
      break;
    }
    case DOUBLE: {
      tuple->field_list[field_counter].f.double_field.val = field.as<double>();
      tuple->field_list[field_counter].f.double_field.genval =
          tuple->field_list[field_counter].f.double_field.val;
      tuple->field_list[field_counter].type = DOUBLE;
      break;
    }
    case UNSUPPORTED: {
      throw std::invalid_argument("Type not supported in tuple builder");
    }
    default: {
      throw std::invalid_argument("Type not supported in tuple builder");
    }
    }
    field_counter++;
  }
}

void write_table_from_postgres(pqxx::result res, table_builder_t *tb) {
  // TODO(madhavsuresh): would prefer this to be on the stack
  tuple_size_breach = 0;
  for (auto psql_row : res) {
    // Don't want to jump on the first tuple
    if (check_add_tuple_page(tb)) {
      add_tuple_page(tb);
    }
    tb->table->num_tuples++;
    // build_tuple_from_pq adds the tuple to the
    build_tuple_from_pq(psql_row, get_tuple(tb->curr_tuple, tb->table),
                        &tb->table->schema, tb);
    tb->curr_tuple++;
    fflush(stdin);
  }
  LOG(PQXX) << "TUPLE SIZE BREACHED: " << tuple_size_breach;
}

table_builder_t *table_builder_init(std::string query_string,
                                    std::string dbname) {
  auto *tb = (table_builder_t *)malloc(sizeof(table_builder_t));
  memset(tb, '\0', sizeof(table_builder_t));
  pqxx::result res = query(query_string, dbname);
  init_table_builder_from_pq(res, tb);
  write_table_from_postgres(res, tb);
  // Everything should be zero-indexed
  return tb;
}

void init_table_builder_from_pq(pqxx::result res, table_builder_t *tb) {
  schema_t schema = get_schema_from_query(tb, res);
  init_table_builder(res.capacity(), res.columns(), &schema, tb);
}

schema_t get_schema_from_query(table_builder_t *tb, pqxx::result res) {
  schema_t schema;
  schema.num_fields = res.columns();
  if (schema.num_fields > MAX_FIELDS) {
    throw std::range_error("KloakDB does not support the number of columns in "
                           "your schema. See the MAX_FIELDS constant.");
  }
  for (int i = 0; i < schema.num_fields; i++) {
    strncpy(schema.fields[i].field_name, res.column_name(i), FIELD_NAME_LEN);
    schema.fields[i].col_no = (uint32_t)i;
    schema.fields[i].type = get_OID_field_type(res.column_type(i));
  }
  // DLOG(INFO) << "Completed Schema with columns: " << tb->num_columns;
  return schema;
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
  return res;
}

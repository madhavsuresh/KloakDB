//
//
//
#include "Aggregate.h"
#include "Expressions.h"
#include <map>
#include <cstring>
#include "postgres_client.h"


schema_t agg_schema(FIELD_TYPE type, table_t * t) {
  schema_t agg_schema;
  agg_schema.num_fields = 2; // Fixed two width groupby for now
  agg_schema.fields[0].type = type;
  agg_schema.fields[0].col_no = 0;
  strncpy(agg_schema.fields[0].field_name, t->schema.fields[0].field_name,
          FIELD_NAME_LEN);
  agg_schema.fields[1].type = INT;
  agg_schema.fields[1].col_no = 1;
  strncpy(agg_schema.fields[1].field_name, "count\0", FIELD_NAME_LEN);
  return agg_schema;
}

table_t *aggregate_count(table_t *t, uint32_t colno) {
  std::map<std::string, int> agg_map;
  // This will always be the same.
  if (t->num_tuples == 0) {
    throw;
  }

  FIELD_TYPE type = t->schema.fields[colno].type;
  for (int i = 0; i < t->num_tuples; i++) {
    tuple_t *tup = get_tuple(i, t);
    std::string key;
    switch (type) {
    case INT: {
      key = std::to_string(tup->field_list[colno].f.int_field.val);
      break;
    }
    case FIXEDCHAR: {
      key = std::string(tup->field_list[colno].f.fixed_char_field.val);
      break;
    }
    case UNSUPPORTED:
      throw;
    }
    if (agg_map[key] == 0) {
      agg_map[key] = 1;
    } else {
      agg_map[key]++;
    }
  }
  agg_map.size();

  schema_t schema = agg_schema(type, t);
  free_table(t);
  table_builder_t tb;
  init_table_builder(agg_map.size(), 2 /*num_columns*/, &schema, &tb);
  tuple_t * tup = (tuple_t *) malloc(tb.size_of_tuple);
  tup->is_dummy = false;
  for (const auto &agg_pair : agg_map) {
    tup->field_list[0].type = schema.fields[0].type;
    switch (tup->field_list[0].type) {
      case INT: {
        tup->field_list[0].f.int_field.val = stoi(agg_pair.first);
        break;
      }
      case FIXEDCHAR: {
        strncpy(tup->field_list[0].f.fixed_char_field.val,
                agg_pair.first.c_str(), FIXEDCHAR_LEN);
        break;
      }
      default: {
        throw;
      }
    }
    tup->field_list[1].type = INT;
    tup->field_list[1].f.int_field.val = agg_pair.second;
    tup->num_fields = 2;
    append_tuple(&tb, tup);
  }
  return tb.table;
}


table_t *aggregate(table_t *t, groupby_def_t *def) {
  switch (def->type) {
  case COUNT: {
    return aggregate_count(t, def->colno);
    break;
  }
  case MINX: {
    printf("UNIMPLEMENTED");
  }
  case GROUPBY_UNSUPPORTED: {
    printf("UNSUPPORTED");
  }
  }
  return nullptr;
}

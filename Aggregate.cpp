//
//
//
#include "Aggregate.h"
#include "Expressions.h"
#include <map>
#include <cstring>
#include "postgres_client.h"


schema_t agg_schema(int32_t colno, table_t * t) {
  schema_t agg_schema;
  FIELD_TYPE type = t->schema.fields[colno].type;
  agg_schema.num_fields = 2; // Fixed two width groupby for now
  agg_schema.fields[0].type = type;
  agg_schema.fields[0].col_no = 0;
  strncpy(agg_schema.fields[0].field_name, t->schema.fields[colno].field_name,
          FIELD_NAME_LEN);
  agg_schema.fields[1].type = INT;
  agg_schema.fields[1].col_no = 1;
  strncpy(agg_schema.fields[1].field_name, "count\0", FIELD_NAME_LEN);
  return agg_schema;
}

table_t *aggregate_count(table_t *t, uint32_t colno) {
  //TODO(madhavsuresh): add unconditional writes
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

  schema_t schema = agg_schema(colno, t);
  //TODO(madhavsuresh): remove all frees inside of operators
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

schema_t agg_schema_avg(groupby_def_t *def, table_t * t) {
  schema_t agg_schema;
  agg_schema.num_fields = def->num_cols + 1; // Fixed two width groupby for now
  for (int i = 0; i < def->num_cols; i ++) {
   agg_schema.fields[i].type = t->schema.fields[def->gb_colnos[i]].type;
   agg_schema.fields[i].col_no = i;
    strncpy(agg_schema.fields[i].field_name, t->schema.fields[def->gb_colnos[i]].field_name,
            FIELD_NAME_LEN);
  }
  agg_schema.fields[def->num_cols].type = DOUBLE;
  agg_schema.fields[def->num_cols].col_no = def->num_cols;
  strncpy(agg_schema.fields[def->num_cols].field_name, "avg\0", FIELD_NAME_LEN);
  return agg_schema;
}

table_t  *aggregate_avg(table_t * t, groupby_def_t *def) {
  std::map<std::string, std::vector<int>> agg_map;
  for (int i = 0; i < t->num_tuples; i++) {
    tuple_t *tup = get_tuple(i, t);
    std::string key = "";
    for (int j = 0; j < def->num_cols; j++) {
      int colno = def->gb_colnos[j];
      //TODO(madhavsuresh): this is ugly, assume that $, is not present in any of the fixedchar fields
      // this should be enforced when reading the data from postgres
      switch (t->schema.fields[colno].type) {
        case INT: {
          key += std::to_string(tup->field_list[colno].f.int_field.val) + "$,";
          break;
        }
        case DOUBLE: {
          key += std::to_string(tup->field_list[colno].f.double_field.val) + "$,";
          break;
        }
        case TIMESTAMP : {
          key += std::to_string(tup->field_list[colno].f.ts_field.val) + "$,";
          break;
        }
        case FIXEDCHAR : {
          key += std::string(tup->field_list[colno].f.fixed_char_field.val) + "$,";
          break;
        }
        case UNSUPPORTED: {
          throw;
        }
        default: {
          throw;
        }

      }
    }
      agg_map[key].push_back(i);
  }

  schema_t s = agg_schema_avg(def, t);
  table_builder_t tb;
  init_table_builder(agg_map.size(), s.num_fields /*num_columns*/, &s, &tb);
  tuple_t *tup = (tuple_t *)malloc(tb.size_of_tuple);
  tup->is_dummy = false;
  for (const auto &agg_pair : agg_map) {
    double avg_total = 0;
    for (auto tup_no : agg_pair.second) {
      avg_total += get_num_field(t, tup_no, def->colno);
    }
    avg_total = avg_total / agg_pair.second.size();

    tup->field_list[s.num_fields-1].type = DOUBLE;
    tup->field_list[s.num_fields-1].f.double_field.val = avg_total;

    tuple_t *to_copy = get_tuple(agg_pair.second[0], t);
    for (int gb = 0; gb < def->num_cols; gb++) {
      tup->field_list[gb].type = s.fields[gb].type;
      switch (s.fields[gb].type) {
      case UNSUPPORTED: {
        throw;
      }
      case FIXEDCHAR: {
        strncpy(tup->field_list[gb].f.fixed_char_field.val,
                to_copy->field_list[def->gb_colnos[gb]].f.fixed_char_field.val,
                FIXEDCHAR_LEN);
        break;
      }
      case INT: {
        tup->field_list[gb].f.int_field =
            to_copy->field_list[def->gb_colnos[gb]].f.int_field;
        break;
      }
      case TIMESTAMP: {
        tup->field_list[gb].f.ts_field =
            to_copy->field_list[def->gb_colnos[gb]].f.ts_field;
        break;
      }
      case DOUBLE: {
        tup->field_list[gb].f.double_field =
            to_copy->field_list[def->gb_colnos[gb]].f.double_field;
        break;
      }
      }
    }
    tup->num_fields = s.num_fields;
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
  case AVG : {
    return aggregate_avg(t, def);
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

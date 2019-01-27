#include "Aggregate.h"
#include "Expressions.h"
#include "sgx_tcrypto.h"
#include <cstring>
#include <map>
#include <unordered_map>
#include <math.h>
#include <sgx_tcrypto.h>
using namespace std;

schema_t agg_schema(int32_t colno, table_t *t) {
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
  // TODO(madhavsuresh): add unconditional writes
  std::unordered_map<std::string, int> agg_map;
  // This will always be the same.
  if (t->num_tuples == 0) {
    schema_t schema = agg_schema(colno, t);
    table_builder_t tb;
    init_table_builder(1, 2 /*num_columns*/, &schema, &tb);
    return tb.table;
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
    default:
      throw std::invalid_argument("Aggregate types not supported");
    }
    if (agg_map[key] == 0) {
      agg_map[key] = 1;
    } else {
      agg_map[key]++;
    }
  }

  schema_t schema = agg_schema(colno, t);
  // TODO(madhavsuresh): remove all frees inside of operators
  table_builder_t tb;
  init_table_builder(agg_map.size(), 2 /*num_columns*/, &schema, &tb);
  auto *tup = (tuple_t *)malloc(tb.size_of_tuple);
  tup->is_dummy = false;
  for (const auto &agg_pair : agg_map) {
    tup->field_list[0].type = schema.fields[0].type;
    switch (tup->field_list[0].type) {
    case INT: {
      tup->field_list[0].f.int_field.val = stoi(agg_pair.first);
      break;
    }
    case FIXEDCHAR: {
      strncpy(tup->field_list[0].f.fixed_char_field.val, agg_pair.first.c_str(),
              FIXEDCHAR_LEN);
      break;
    }
    default: { throw; }
    }
    tup->field_list[1].type = INT;
    tup->field_list[1].f.int_field.val = agg_pair.second;
    tup->field_list[1].f.int_field.genval = agg_pair.second;
    tup->num_fields = 2;
    append_tuple(&tb, tup);
  }
  free(tup);
  return tb.table;
}

schema_t agg_schema_avg_partial(groupby_def_t *def, table_t *t) {
  schema_t agg_schema;
  agg_schema.num_fields = def->num_cols + 2; // Fixed two width groupby for now
  for (int i = 0; i < def->num_cols; i++) {
    agg_schema.fields[i].type = t->schema.fields[def->gb_colnos[i]].type;
    agg_schema.fields[i].col_no = i;
    strncpy(agg_schema.fields[i].field_name,
            t->schema.fields[def->gb_colnos[i]].field_name, FIELD_NAME_LEN);
  }
  agg_schema.fields[def->num_cols].type = INT;
  agg_schema.fields[def->num_cols].col_no = def->num_cols;
  strncpy(agg_schema.fields[def->num_cols].field_name, "sum\0", FIELD_NAME_LEN);

  agg_schema.fields[def->num_cols+1].type = INT;
  agg_schema.fields[def->num_cols +1].col_no = def->num_cols + 1;
  strncpy(agg_schema.fields[def->num_cols+1].field_name, "count\0", FIELD_NAME_LEN);
  return agg_schema;
}


schema_t agg_schema_avg(groupby_def_t *def, table_t *t) {
  schema_t agg_schema;
  agg_schema.num_fields = def->num_cols + 1; // Fixed two width groupby for now
  for (int i = 0; i < def->num_cols; i++) {
    agg_schema.fields[i].type = t->schema.fields[def->gb_colnos[i]].type;
    agg_schema.fields[i].col_no = i;
    strncpy(agg_schema.fields[i].field_name,
            t->schema.fields[def->gb_colnos[i]].field_name, FIELD_NAME_LEN);
  }
  agg_schema.fields[def->num_cols].type = DOUBLE;
  agg_schema.fields[def->num_cols].col_no = def->num_cols;
  strncpy(agg_schema.fields[def->num_cols].field_name, "avg\0", FIELD_NAME_LEN);
  return agg_schema;
}

int64_t get_gen(tuple_t * t, int colno) {
  switch(t->field_list[colno].type) {
    case FIXEDCHAR: {
      return t->field_list[colno].f.fixed_char_field.genval;
    }
    case INT : {
      return t->field_list[colno].f.int_field.genval;
    }
  }
  throw std::invalid_argument("Cannot get_gen on this type");
}


int64_t hash_field(groupby_def_t *def,tuple_t *t,
                 table_t *table) {
  sgx_sha256_hash_t hash;
  uint8_t f[MAX_FIELDS*FIXEDCHAR_LEN];
  uint32_t ptr = 0;
  for (int i = 0; i < def->num_cols; i++) {
    int input_col = def->gb_colnos[i];
    switch(t->field_list[input_col].type){
      case FIXEDCHAR : {
        if (input_col != def->kanon_col) {
          memcpy(&f[ptr], t->field_list[input_col].f.fixed_char_field.val, FIXEDCHAR_LEN);
          ptr += FIXEDCHAR_LEN;
        } else {
          memcpy(&f[ptr], &(t->field_list[input_col].f.fixed_char_field.val), sizeof(uint64_t));
          ptr += sizeof(uint64_t);
        }
        break;
      }
      case INT: {
        if (input_col != def->kanon_col) {
          memcpy(&f[ptr], &(t->field_list[input_col].f.int_field.val), sizeof(uint64_t));
        } else {
          memcpy(&f[ptr], &(t->field_list[input_col].f.int_field.val), sizeof(uint64_t));
        }
        ptr += sizeof(uint64_t);
        break;
      }
      default :{
        throw;
      }
    }
  }
  sgx_sha256_msg(f, ptr, &hash);
  union {
      uint64_t u;
      unsigned char u8[sizeof(uint64_t)];
  } out;
  out.u8[0] = hash[0];
  out.u8[1] = hash[1];
  out.u8[2] = hash[2];
  out.u8[3] = hash[3];
  out.u8[4] = hash[4];
  out.u8[5] = hash[5];
  out.u8[6] = hash[6];
  out.u8[7] = hash[7];
  return out.u;
}


void copy_tuple_gb(groupby_def_t *def, tuple_t * to_copy, tuple_t * copy_into, table_builder_t *tb) {

  for (int gb = 0; gb < def->num_cols; gb++) {
    copy_into->field_list[gb].type = tb->table->schema.fields[gb].type;
    switch (tb->table->schema.fields[gb].type) {
      case UNSUPPORTED: {
        throw;
      }
      case FIXEDCHAR: {
        strncpy(copy_into->field_list[gb].f.fixed_char_field.val,
                to_copy->field_list[def->gb_colnos[gb]].f.fixed_char_field.val,
                FIXEDCHAR_LEN);
        break;
      }
      case INT: {
        copy_into->field_list[gb].f.int_field =
                to_copy->field_list[def->gb_colnos[gb]].f.int_field;
        break;
      }
      case TIMESTAMP: {
        copy_into->field_list[gb].f.ts_field =
                to_copy->field_list[def->gb_colnos[gb]].f.ts_field;
        break;
      }
      case DOUBLE: {
        copy_into->field_list[gb].f.double_field =
                to_copy->field_list[def->gb_colnos[gb]].f.double_field;
        break;
      }
    }
  }
  copy_into->num_fields = tb->table->schema.num_fields;
  for (int i = def->num_cols; i < tb->table->schema.num_fields; i++) {
    copy_into->field_list[i].type = INT;
    copy_into->field_list[i].f.int_field.val = 0;
  }
}

unordered_map<int64_t, pair<int,int>> oblivious_partial_aggregate(
    unordered_map<int64_t, int> gb_to_tuple,
    vector<int> gen_tuple_indices, groupby_def_t *gb, table_t * t) {
  unordered_map<int64_t, pair<int, int>> hash_to_count_and_total;
  for (auto &i : gen_tuple_indices) {
    tuple_t * tup = get_tuple(i, t);
    int64_t hashed = hash_field(gb, tup, t);
    for (auto &h : gb_to_tuple) {
	auto first = hash_to_count_and_total[hashed].first;
	auto second = hash_to_count_and_total[hashed].second;
      if (h.first == hashed && !tup->is_dummy) {
        hash_to_count_and_total[hashed].first = first+1;
        if (gb->type != COUNT) {
          hash_to_count_and_total[hashed].second = second + tup->field_list[gb->colno].f.int_field.val;
        }
      } else {
        hash_to_count_and_total[hashed].first = first;
        if (gb->type != COUNT) {
          hash_to_count_and_total[hashed].second = second;
        }
      }
    }
  }
  return hash_to_count_and_total;
}

void merge_partials(vector<unordered_map<int64_t, pair<int, int>>> partials,
                    groupby_def_t *def, table_builder_t tb,
                    unordered_map<int64_t, int> gb_to_tuple) {

  unordered_map<int64_t, vector<pair<int, int>>> grouped_partials;
  for (auto &p : partials) {
    for (auto &h : p) {
      grouped_partials[h.first].emplace_back(h.second);
    }
  }
  for (auto &g: grouped_partials) {
    tuple_t * tup = get_tuple(gb_to_tuple[g.first], tb.table);
    int count = 0;
    int sum = 0;
    for (auto &cs : g.second) {
      count += cs.first;
      sum += cs.second;
    }
    tup->field_list[colno_from_name(tb.table, "count")].f.int_field.val = count;
    tup->field_list[colno_from_name(tb.table, "sum")].f.int_field.val = sum;
  }
}

table_t *kpartial_aggregate_avg(table_t * t, groupby_def_t *def) {
  //TODO(madhavsuresh): if kanon-col == groupby-col , then map
  std::unordered_map<int, std::vector<int>> agg_map;
  std::unordered_map<int,vector<int>> gen_values_to_index;
  std::unordered_map<int64_t, int> gb_to_tuple;

  table_builder_t tb;
  schema_t s;
  if (def->type == AVG) {
    s = agg_schema_avg_partial(def, t);
  } else if (def->type == COUNT) {
    s = agg_schema_avg_partial(def, t);
  }

  init_table_builder(t->num_tuples, s.num_fields, &s, &tb);
  auto *copy_into = (tuple_t *)malloc(tb.size_of_tuple);
  copy_into->is_dummy = false;
  int curr_tuple = 0;
  for (int i = 0; i < t->num_tuples; i++) {
    tuple_t * input_tup = get_tuple(i, t);
    int64_t gen_val = get_gen(input_tup, def->kanon_col);
    int64_t hashed = hash_field(def, input_tup, t);
    if (gb_to_tuple.find(hashed) == gb_to_tuple.end()) {
      copy_tuple_gb(def, input_tup,copy_into,&tb);
      gb_to_tuple[hashed] = curr_tuple;
      append_tuple(&tb, copy_into);
      curr_tuple++;
    }
    gen_values_to_index[gen_val].emplace_back(i);
  }

  vector<unordered_map<int64_t, pair<int,int>>> partials; //first in pair is count, second is field value sum
  partials.reserve(gen_values_to_index.size());
  for (auto &g: gen_values_to_index) {
    partials.emplace_back(oblivious_partial_aggregate(gb_to_tuple, g.second, def, t));
  }
  merge_partials(partials, def, tb, gb_to_tuple);
  return tb.table;
}

// In lieu of optimizing this, we will assumee that the group by columns
// are integers, and are in the range 0-16
table_t *aggregate_avg(table_t *t, groupby_def_t *def) {
  std::unordered_map<int, std::vector<int>> agg_map;
  for (int i = 0; i < t->num_tuples; i++) {
    tuple_t *tup = get_tuple(i, t);
    int key = 0;
    for (int j = 0; j < def->num_cols; j++) {
      int colno = def->gb_colnos[j];
      switch (t->schema.fields[colno].type) {
      case INT: {
        key += tup->field_list[colno].f.int_field.val * (int)pow(10,j +1);
        break;
      }
      case DOUBLE: {
        throw;
      }
      case TIMESTAMP: {
          throw;
      }
      case FIXEDCHAR: {
          throw;
      }
      case UNSUPPORTED: {
        throw;
      }
      default: { throw; }
      }
    }
    agg_map[key].push_back(i);
  }

  schema_t s = agg_schema_avg_partial(def, t);
  table_builder_t tb;
  init_table_builder(agg_map.size(), s.num_fields /*num_columns*/, &s, &tb);
  tuple_t *tup = (tuple_t *)malloc(tb.size_of_tuple);
  tup->is_dummy = false;
  double num_dummy = 0;
  for (const auto &agg_pair : agg_map) {
    int avg_total = 0;
    for (auto tup_no : agg_pair.second) {
      if (!get_tuple(tup_no, t)->is_dummy) {
        avg_total += (int)get_num_field(t, tup_no, def->colno);
      }
    }
    tuple_t *to_copy = get_tuple(agg_pair.second[0], t);
    copy_tuple_gb(def, to_copy,tup, &tb);
    tup->field_list[colno_from_name(tb.table, "count")].f.int_field.val = agg_pair.second.size();
    tup->field_list[colno_from_name(tb.table, "sum")].f.int_field.val = avg_total;
    append_tuple(&tb, tup);
  }
  free(tup);
  return tb.table;
}

table_t *merge_partial_aggregate_tables(vector<table_t*> tables) {

}

table_t *aggregate(table_t *t, groupby_def_t *def) {
  if (def->secure) {
        return kpartial_aggregate_avg(t, def);
  } else {

    switch (def->type) {
      case COUNT: {
        return aggregate_count(t, def->colno);
        break;
      }
      case AVG: {
        return aggregate_avg(t, def);
      }
      case MINX: {
        throw;
        // printf("UNIMPLEMENTED");
      }
      case GROUPBY_UNSUPPORTED: {
        throw;
        // printf("UNSUPPORTED");
      }
    }
  }
  return nullptr;
}

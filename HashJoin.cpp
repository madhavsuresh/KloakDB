//
// Created by root on 10/4/18.
//
#include "HashJoin.h"
#include "Expressions.h"
#include "postgres_client.h"
#include <iostream>
#include <unordered_map>
#include <cstring>

schema_t build_join_schema(table_t *left_table, table_t *right_table, join_def_t def) {
  table_t * tables[2];
  tables[0] = left_table;
  tables[1] = right_table;
  schema_t s;
  s.num_fields = def.project_len;

  for (int i = 0; i < def.project_len; i++) {
    join_colID_t id = def.project_list[i];
    field_desc_t fd = tables[id.side]->schema.fields[id.col_no];
    s.fields[i].col_no = i;
    s.fields[i].type = fd.type;
    strncpy(s.fields[i].field_name, fd.field_name, FIELD_NAME_LEN);
  }

  return s;
}

void merge_tuple(tuple_t * to_fill, tuple_t * left_tup, tuple_t * right_tup, join_def_t def) {
  // Reusing allocated tuple for copying
  tuple_t * tuples[2] = {left_tup, right_tup};
  memset(to_fill, '\0', def.project_len*sizeof(field_t) + sizeof(tuple_t));
  to_fill->num_fields = def.project_len;

  for (int i = 0; i < def.project_len; i ++) {
    tuple_t * copyFromTup = tuples[def.project_list[i].side];
    to_fill->field_list[i].type = copyFromTup->field_list[def.project_list[i].col_no].type;
    switch (to_fill->field_list[i].type) {
      case FIXEDCHAR : {
        memcpy(to_fill->field_list[i].f.fixed_char_field.val, copyFromTup->field_list[def.project_list[i].col_no].f.fixed_char_field.val, FIXEDCHAR_LEN);
        break;
      }
      case INT : {
        to_fill->field_list[i].f.int_field.val = copyFromTup->field_list[def.project_list[i].col_no].f.int_field.val;
        to_fill->field_list[i].f.int_field.genval = copyFromTup->field_list[def.project_list[i].col_no].f.int_field.genval;
        break;
      }
    }
  }
}


table_t *hash_join(table_t *left_table, table_t *right_table, join_def_t def) {

  schema_t schema = build_join_schema(left_table, right_table, def);
    std::unordered_multimap<std::string, int> hashmap;
    for (int i = 0; i < left_table->num_tuples; i++) {
        // get the i'th tuple
        tuple_t *tuple = get_tuple(i, left_table);

        // hash row # -> store value at specified column
        // hashmap[tup1->field_list[l_col]] = i;
        switch (tuple->field_list[def.l_col].type) {
            case FIXEDCHAR:
                //TODO(madhavsuresh): this needs to be the generalized value.
                hashmap.insert(std::make_pair(tuple->field_list[def.l_col].f.fixed_char_field.val, i));
                break;
                case INT:
                    // cast int to string to store in hashmap
                    hashmap.insert(std::make_pair(std::to_string(tuple->field_list[def.l_col].f.int_field.val), i));
                    break;
                    case UNSUPPORTED:
                        throw;
        }

    }

    // hash map should be built
    table_builder_t tb;

    init_table_builder(left_table->num_tuples * right_table->num_tuples, def.project_len, &schema, &tb);
    tuple_t * to_fill = (tuple_t *) malloc(tb.size_of_tuple);
    //iterate over other table checking hash map
    for (int i = 0; i < right_table->num_tuples; i++) {
        tuple_t *tup2 = get_tuple(i, right_table);

        switch (tup2->field_list[def.r_col].type) {
          case FIXEDCHAR: {
            auto range = hashmap.equal_range(std::string(tup2->field_list[def.r_col].f.fixed_char_field.val));
            for (auto it = range.first; it != range.second; it++) {
                merge_tuple(to_fill, get_tuple(it->second, left_table), tup2, def);
                append_tuple(&tb, to_fill);
            }
            break;
          }
          case INT : {
            auto range = hashmap.equal_range(std::to_string(tup2->field_list[def.r_col].f.int_field.val));
              for (auto it = range.first; it != range.second; it++) {
                  merge_tuple(to_fill, get_tuple(it->second, left_table), tup2, def);
                  append_tuple(&tb, to_fill);
              }
            break;
          }
        }
    }
    free(to_fill);
    return tb.table;
}

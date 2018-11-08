//
// Created by root on 10/4/18.
//
#include "postgres_client.h"

#ifndef PROJECT_HASHJOIN_H
#define PROJECT_HASHJOIN_H

#endif // PROJECT_HASHJOIN_H
table_t *hash_join(table_t *left_table, table_t *right_table, join_def_t def);
void merge_tuple(tuple_t *to_fill, tuple_t *left_tup, tuple_t *right_tup,
                 join_def_t def);
schema_t build_join_schema(table_t *left_table, table_t *right_table,
                           join_def_t def);

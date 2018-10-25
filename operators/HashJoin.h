//
// Created by root on 10/4/18.
//
#include "../postgres_client/postgres_client.h"


#ifndef PROJECT_HASHJOIN_H
#define PROJECT_HASHJOIN_H

#endif //PROJECT_HASHJOIN_H
table_t *HashJoin(table_t * left_table, table_t * right_table, uint16_t l_col, uint16_t r_col);
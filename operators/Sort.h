//
// Created by madhav on 8/17/18.
//

#ifndef PROJECT_SORT_H
#define PROJECT_SORT_H
#include "../data/postgres_client.h"
#include <cstdint>

typedef struct sortdef {
  int32_t colno;
  bool ascending;
  bool sorting_dummies;
} sort_t;

table_t *sort(table_t *t, sort_t *s);
void swap_tuples(int t1, int t2, table_t *t, bool to_swap);
#endif // PROJECT_SORT_H

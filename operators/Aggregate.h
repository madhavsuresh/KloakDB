//
// Created by madhav on 7/13/18.
//

#ifndef PROJECT_AGGREGATE_H
#define PROJECT_AGGREGATE_H
#include "../data/postgres_client.h"
#include <cstdint>

table_t *aggregate(table_t *t, groupby_def_t *def);
#endif // PROJECT_AGGREGATE_H

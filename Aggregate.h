//
// Created by madhav on 7/13/18.
//

#ifndef PROJECT_AGGREGATE_H
#define PROJECT_AGGREGATE_H
#include <cstdint>
#include "postgres_client.h"

enum GROUPBY_TYPE { GROUPBY_UNSUPPORTED, MINX, COUNT };

typedef struct groupby_def {
  GROUPBY_TYPE type;
  uint8_t colno;
} groupby_def_t;

table_t *aggregate(table_t *t, groupby_def_t *def);
#endif // PROJECT_AGGREGATE_H

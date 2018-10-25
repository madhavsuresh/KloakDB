//
// Created by madhav on 7/13/18.
//

#ifndef PROJECT_AGGREGATE_H
#define PROJECT_AGGREGATE_H
#include <cstdint>
uint8_t *aggregate(uint8_t *table_buf, uint8_t *groupby_defs_buf);

enum GROUPBY_TYPE { GROUPBY_UNSUPPORTED, MINX, COUNT };

typedef struct groupby_def {
  GROUPBY_TYPE type;
  uint8_t colno;
} groupby_def_t;
#endif // PROJECT_AGGREGATE_H

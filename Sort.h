//
// Created by madhav on 8/17/18.
//

#ifndef PROJECT_SORT_H
#define PROJECT_SORT_H
#include <cstdint>
#include "postgres_client.h"

typedef struct sort {
    uint32_t colno;
    FIELD_TYPE type;
    bool ascending;
} sort_t;

table_t * sort(table_t * t, sort_t * s);
#endif //PROJECT_SORT_H



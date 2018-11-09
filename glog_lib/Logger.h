//
// Created by madhav on 10/30/18.
//

#ifndef PROJECT_LOGGER_H
#define PROJECT_LOGGER_H

#include "../operators/postgres_client.h"
void print_tuple_log(int ii, tuple_t *t);
void print_tuple(tuple_t * t);
#endif //PROJECT_LOGGER_H

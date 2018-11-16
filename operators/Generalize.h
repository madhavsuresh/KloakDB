//
// Created by madhav on 11/14/18.
//

#ifndef PROJECT_GENERALIZE_H
#define PROJECT_GENERALIZE_H

#include <vector>
#include "data/postgres_client.h"

typedef int cf_hash;
typedef uint64_t cf_gen;
typedef int hostnum;
typedef int tup_count;
typedef int numhosts;
typedef double score;

table_t * generalize_table(std::vector<std::pair<hostnum, table_t *>> host_table_pairs,
                           int num_hosts, int k);

table_t * generalize_zip(table_t * t, table_t * gen_map_table, int col_no);
#endif //PROJECT_GENERALIZE_H

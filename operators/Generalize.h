//
// Created by madhav on 11/14/18.
//

#ifndef PROJECT_GENERALIZE_H
#define PROJECT_GENERALIZE_H

#include <vector>
#include "data/postgres_client.h"
#include <unordered_map>

typedef int cf_hash;
typedef uint64_t cf_gen;
typedef int hostnum;
typedef int tup_count;
typedef int numhosts;
typedef double score;
typedef std::string table_name;
typedef std::unordered_map<cf_hash,
        std::vector<std::tuple<hostnum, tup_count, cf_hash>>>
        gen_map_t;
typedef std::vector<std::tuple<hostnum, tup_count, cf_hash>> eq_class_t;
using namespace std;

table_t * generalize_table(std::vector<std::pair<hostnum, table_t *>> host_table_pairs,
                           int num_hosts, int k);
table_t *generalize_table(
        std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
        table_map_host_table_pairs,
        int num_hosts, int k);

table_t * generalize_zip(table_t * t, table_t * gen_map_table, int col_no);
#endif //PROJECT_GENERALIZE_H

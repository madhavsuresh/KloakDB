//
// Created by madhav on 10/30/18.
//

#ifndef PROJECT_PQXX_COMPAT_H
#define PROJECT_PQXX_COMPAT_H
#include "postgres_client.h"
#include <pqxx/pqxx>

table_builder_t *table_builder_init(std::string query_string,
                                    std::string dbname);
table_t *get_table(std::string query_string, std::string dbname);
pqxx::result query(std::string query_string, std::string dbname);
schema_t get_schema_from_query(table_builder_t *tb, pqxx::result res);
void init_table_builder_from_pq(pqxx::result res, table_builder_t *tb);

#endif // PROJECT_PQXX_COMPAT_H

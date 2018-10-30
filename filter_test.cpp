//
// Created by madhav on 8/11/18.
//

#include <gtest/gtest.h>
#include <pqxx/result>

#include "Filter.h"
#include "filter_test.h"
#include "postgres_client.h"
#include "Logger.h"
#include "pqxx_compat.h"

class filter_test : public ::testing::Test {
public:
  std::string dbname;

protected:
  void SetUp() override { dbname = "dbname=test"; };
  void TearDown() override{

  };
};

TEST_F(filter_test, simple_filter) {
  std::string query1("create table filter_test (a INT, b INT)");
  pqxx::result res2;
  res2 = query(query1, dbname);
  query1 = "INSERT INTO filter_test (a,b) VALUES (1,2), (1,3), (2,3)";
  res2 = query(query1, dbname);
  query1 = "SELECT * FROM filter_test;";
  table_builder_t *tb = table_builder_init(query1, dbname);
  expr_t expr = make_int_expr(EQ_EXPR, 1 /* field_val */, 0 /* colno */);
  table_t *t = filter(tb->table, &expr);
  for (int i = 0; i < t->num_tuples; i++) {
    auto tup = get_tuple(i, t);
    if (!tup->is_dummy) {
      print_tuple(tup);
      printf("\n");
    }
  }
  std::string query_destroy("DROP TABLE filter_test");
  res2 = query(query_destroy, dbname);
}

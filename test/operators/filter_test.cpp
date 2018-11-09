//
// Created by madhav on 8/11/18.
//

#include <gtest/gtest.h>

#include "data/postgres_client.h"
#include "data/pqxx_compat.h"
#include "operators/Filter.h"

class filter_test : public ::testing::Test {
public:
  std::string dbname;

protected:
  void SetUp() override { dbname = "dbname=test"; };
  void TearDown() override{

  };
};

TEST_F(filter_test, simple_filter) {
  // TODO(madhavsuresh): write more extensive filter tests
  std::string query1("create table filter_test (a INT, b INT)");
  query(query1, dbname);
  query1 = "INSERT INTO filter_test (a,b) VALUES (1,2), (1,3), (2,3)";
  query(query1, dbname);
  query1 = "SELECT * FROM filter_test;";
  table_builder_t *tb = table_builder_init(query1, dbname);
  expr_t expr = make_int_expr(EQ_EXPR, 1 /* field_val */, 0 /* colno */);
  table_t *t = filter(tb->table, &expr);
  std::string query_destroy("DROP TABLE filter_test");
  query(query_destroy, dbname);
  free_table(t);
  free(tb);
}

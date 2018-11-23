//
// Created by madhav on 8/11/18.
//

#include <gtest/gtest.h>

#include "data/postgres_client.h"
#include "data/pqxx_compat.h"
#include "operators/Filter.h"
#include "sgx/App/VaultDBSGXApp.h"

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
  query("DROP TABLE IF EXISTS filter_test", dbname);
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

TEST_F(filter_test, sgx) {
  // TODO(madhavsuresh): write more extensive filter tests
  query("DROP TABLE IF EXISTS filter_test", dbname);
  std::string query1("create table filter_test (a INT, b INT)");
  query(query1, dbname);
  query1 = "INSERT INTO filter_test (a,b) VALUES (1,2), (1,3), (2,3)";
  query(query1, dbname);
  query1 = "SELECT * FROM filter_test;";
  table_builder_t *tb = table_builder_init(query1, dbname);
  expr_t expr = make_int_expr(EQ_EXPR, 1 /* field_val */, 0 /* colno */);
  table_t *t = filter_sgx(tb->table, &expr);
  std::string query_destroy("DROP TABLE filter_test");
  query(query_destroy, dbname);
  free_table(t);
  free(tb);
}

TEST_F(filter_test, string_test) {
  query("DROP TABLE IF EXISTS filter_test", dbname);
  query("CREATE TABLE filter_test (a INT, b VARCHAR)", dbname);
  query("INSERT INTO filter_test (a,b) VALUES (1, 'a'), (2, 'b')", dbname);
  table_t * t = get_table("SELECT * FROM filter_test", dbname);
  expr_t expr = make_string_expr(EQ_EXPR, "a", 1);
  table_t *out = filter(t, &expr);
  ASSERT_EQ(out->num_tuples, 2);
  ASSERT_EQ(get_tuple(0,out)->is_dummy, false);
  ASSERT_EQ(get_tuple(1,out)->is_dummy, true);

}

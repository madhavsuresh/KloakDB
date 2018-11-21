//
// Created by madhav on 10/25/18.
//
#include "operators/Aggregate.h"
#include "data/postgres_client.h"
#include "data/pqxx_compat.h"
#include <gtest/gtest.h>
#include "sgx/App/VaultDBSGXApp.h"

class aggregate_test : public ::testing::Test {
public:
  std::string dbname;

protected:
  void SetUp() override {
    dbname = "dbname=test";
    query("DROP TABLE IF EXISTS simple_agg", dbname);
    std::string query1("create table simple_agg (a INT, b INT)");
    query(query1, dbname);
  };
  void TearDown() override {
    std::string query1("DROP TABLE simple_agg");
    query(query1, dbname);
  };
};

TEST_F(aggregate_test, simple_aggregate) {
  std::string query1 =
      "INSERT INTO simple_agg (a,b) VALUES (1,7), (2,7), (3,4)";
  query(query1, dbname);
  query1 = "SELECT * FROM simple_agg";
  table_t *t = get_table(query1, dbname);
  schema_t before = t->schema;
  groupby_def_t gbd;
  gbd.type = COUNT;
  gbd.colno = 1;
  table_t *t2 = aggregate(t, &gbd);
  schema_t after = t2->schema;
  ASSERT_EQ(after.num_fields, 2);
  ASSERT_EQ(after.fields[0].type, before.fields[gbd.colno].type);
  ASSERT_EQ(after.fields[1].type, INT);
  ASSERT_EQ(2, t2->num_tuples);
  tuple_t *tup = get_tuple(0, t2);
  ASSERT_EQ(tup->field_list[0].f.int_field.val, 4);
  ASSERT_EQ(tup->field_list[1].f.int_field.val, 1);
  ASSERT_FALSE(tup->is_dummy);
  ASSERT_EQ(tup->num_fields, 2);

  tup = get_tuple(1, t2);
  ASSERT_EQ(tup->field_list[0].f.int_field.val, 7);
  ASSERT_EQ(tup->field_list[1].f.int_field.val, 2);
  ASSERT_STREQ(t2->schema.fields[0].field_name, "b");
  ASSERT_STREQ(t2->schema.fields[1].field_name, "count");
  ASSERT_FALSE(tup->is_dummy);
  ASSERT_EQ(tup->num_fields, 2);
  query("DELETE FROM simple_agg", dbname);
  free_table(t);
  free_table(t2);
}

TEST_F(aggregate_test, avg_aggregate) {
  std::string query1 =
      "INSERT INTO simple_agg (a,b) VALUES (1,7), (2,7), (3,4)";
  query(query1, dbname);
  query1 = "SELECT * FROM simple_agg";
  table_t *t = get_table(query1, dbname);
  groupby_def_t gbd;
  gbd.type = AVG;
  gbd.colno = 0;
  gbd.num_cols = 1;
  gbd.gb_colnos[0] = 1;
  table_t *t2 = aggregate(t, &gbd);
  for (int i = 0; i < t2->num_tuples; i++) {
    std::cout << tuple_string(get_tuple(i, t2)) << std::endl;
  }
  free_table(t);
  free_table(t2);
}

TEST_F(aggregate_test, sgx_avg_aggregate) {
  std::string query1 =
      "INSERT INTO simple_agg (a,b) VALUES (1,7), (2,7), (3,4)";
  query(query1, dbname);
  query1 = "SELECT * FROM simple_agg";
  table_t *t = get_table(query1, dbname);
  groupby_def_t gbd;
  gbd.type = AVG;
  gbd.colno = 0;
  gbd.num_cols = 1;
  gbd.gb_colnos[0] = 1;
  table_t *t2 = aggregate_sgx(t, &gbd);
  for (int i = 0; i < t2->num_tuples; i++) {
    std::cout << tuple_string(get_tuple(i, t2)) << std::endl;
  }
  free_table(t);
  free_table(t2);
}

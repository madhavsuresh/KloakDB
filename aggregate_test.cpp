//
// Created by madhav on 10/25/18.
//
#include <gtest/gtest.h>
#include "postgres_client.h"
#include "pqxx_compat.h"
#include "Aggregate.h"

class aggregate_test : public ::testing::Test {
public:
    std::string dbname;
protected:
    void SetUp() override {
      dbname = "dbname=test";
      std::string query1("create table simple_agg (a INT, b INT)");
      query(query1, dbname);
    };
    void TearDown() override {
      std::string query1("DROP TABLE simple_agg");
      query(query1, dbname);
    };
};

TEST_F(aggregate_test, simple_aggregate) {
  std::string query1 = "INSERT INTO simple_agg (a,b) VALUES (1,7), (2,7), (3,4)";
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
  tuple_t * tup = get_tuple(0, t2);
  ASSERT_EQ(tup->field_list[0].f.int_field.val, 4);
  ASSERT_EQ(tup->field_list[1].f.int_field.val, 1);
  ASSERT_FALSE(tup->is_dummy);
  ASSERT_EQ(tup->num_fields, 2);

  tup = get_tuple(1, t2);
  ASSERT_EQ(tup->field_list[0].f.int_field.val, 7);
  ASSERT_EQ(tup->field_list[1].f.int_field.val, 2);
  ASSERT_FALSE(tup->is_dummy);
  ASSERT_EQ(tup->num_fields, 2);
}
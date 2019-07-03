//
// Created by madhav on 7/27/18.
//
#include "data/postgres_client.h"
#include "data/pqxx_compat.h"
#include "operators/HashJoin.h"
#include "operators/Aggregate.h"
#include "operators/Sort.h"
#include <gtest/gtest.h>
#include <malloc.h>

class tpch_test : public ::testing::Test {
public:
  std::string dbname;
  std::string test_year;
  std::string site1;
  std::string site2;

protected:
  void SetUp() override {
    // This test assumes query works... also existence of "t_random_500"
    // This test can just create the database we want
    // TODO(madhavsuresh): should create new database every time
    // TODO(madhavsuresh): these test are very brittle.
    dbname = "dbname=tpch";
    test_year = "2006";
    site1 = "4";
    site2 = "7";
  }

  void TearDown() override{};
};

TEST_F(tpch_test, customer) {
  table_t *t = get_table("SELECT * FROM customer", dbname);
  ASSERT_EQ(t->schema.num_fields, 8);
  free_table(t);
}

TEST_F(tpch_test, orders) {
  table_t *t = get_table("SELECT * FROM orders", dbname);
  ASSERT_EQ(t->schema.num_fields, 9);
  free_table(t);
}

TEST_F(tpch_test, lineitem) {
  table_t *t = get_table("SELECT * FROM lineitem", dbname);
  ASSERT_EQ(t->schema.num_fields, 16);
  free_table(t);
}

TEST_F(tpch_test, query3) {
  table_t *lineitem = get_table("SELECT l_orderkey, l_extendedprice *(1 - l_discount) as revenue FROM lineitem where l_shipdate > '1995-03-22'", dbname);
  table_t *customer = get_table("SELECT c_custkey FROM customer WHERE c_mktsegment='BUILDING'", dbname);

  table_t *orders = get_table("SELECT o_orderdate, o_shippriority, o_custkey, o_orderkey from orders WHERE o_orderdate <'1995-03-22'",dbname);

  join_def_t jd;
  jd.l_col = 2;
  jd.r_col = 0;
  jd.project_len=3;
  jd.project_list[0].side = LEFT_RELATION;
  jd.project_list[0].col_no = colno_from_name(orders, "o_orderkey");;
  jd.project_list[1].side = LEFT_RELATION;
  jd.project_list[1].col_no = colno_from_name(orders, "o_orderdate");
  jd.project_list[2].side = LEFT_RELATION;
  jd.project_list[2].col_no = colno_from_name(orders, "o_shippriority");
  table_t *oc = hash_join(orders, customer, jd);

  join_def_t jd1;
  jd1.l_col = colno_from_name(oc, "o_orderkey");;
  jd1.r_col = colno_from_name(lineitem, "l_orderkey");
  jd1.project_len=4;
  jd1.project_list[0].side = RIGHT_RELATION;
  jd1.project_list[0].col_no = colno_from_name(lineitem, "l_orderkey");
  jd1.project_list[1].side = RIGHT_RELATION;
  jd1.project_list[1].col_no = colno_from_name(lineitem, "revenue");
  jd1.project_list[2].side = LEFT_RELATION;
  jd1.project_list[2].col_no = colno_from_name(oc, "o_orderdate");
  jd1.project_list[3].side = LEFT_RELATION;
  jd1.project_list[3].col_no = colno_from_name(oc, "o_shippriority");
  table_t *ocl = hash_join(oc, lineitem, jd1);
  groupby_def_t def;
  def.secure = true;
  def.type=AVG;
  def.kanon_col = 0;
  def.colno=colno_from_name(ocl, "revenue");
  def.num_cols = 3;
  def.gb_colnos[0] = colno_from_name(ocl, "l_orderkey");
  def.gb_colnos[1] = colno_from_name(ocl, "o_orderdate");
  def.gb_colnos[2] = colno_from_name(ocl, "o_shippriority");

  table_t * agg_out = aggregate(ocl, &def);
  sort_t sort_def = {.colno=colno_from_name(agg_out,"sum"), .ascending=false};

  table_t * sorted = sort(agg_out, &sort_def);
  printf("num tuples :%lu\n", agg_out->num_tuples);
}


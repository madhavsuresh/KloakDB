//
// Created by madhav on 7/27/18.
//
#include "data/postgres_client.h"
#include "data/pqxx_compat.h"
#include "operators/Aggregate.h"
#include "operators/HashJoin.h"
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
  table_t *lineitem =
      get_table("SELECT l_orderkey, l_extendedprice *(1 - l_discount) as "
                "revenue FROM lineitem where l_shipdate > '1995-03-22'",
                dbname);
  table_t *customer = get_table(
      "SELECT c_custkey FROM customer WHERE c_mktsegment='BUILDING'", dbname);

  table_t *orders =
      get_table("SELECT o_orderdate, o_shippriority, o_custkey, o_orderkey "
                "from orders WHERE o_orderdate <'1995-03-22'",
                dbname);

  join_def_t jd;
  jd.l_col = 2;
  jd.r_col = 0;
  jd.project_len = 3;
  jd.project_list[0].side = LEFT_RELATION;
  jd.project_list[0].col_no = colno_from_name(orders, "o_orderkey");
  ;
  jd.project_list[1].side = LEFT_RELATION;
  jd.project_list[1].col_no = colno_from_name(orders, "o_orderdate");
  jd.project_list[2].side = LEFT_RELATION;
  jd.project_list[2].col_no = colno_from_name(orders, "o_shippriority");
  table_t *oc = hash_join(orders, customer, jd);

  join_def_t jd1;
  jd1.l_col = colno_from_name(oc, "o_orderkey");
  ;
  jd1.r_col = colno_from_name(lineitem, "l_orderkey");
  jd1.project_len = 4;
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
  def.type = AVG;
  def.kanon_col = 0;
  def.colno = colno_from_name(ocl, "revenue");
  def.num_cols = 3;
  def.gb_colnos[0] = colno_from_name(ocl, "l_orderkey");
  def.gb_colnos[1] = colno_from_name(ocl, "o_orderdate");
  def.gb_colnos[2] = colno_from_name(ocl, "o_shippriority");

  table_t *agg_out = aggregate(ocl, &def);
  sort_t sort_def = {.colno = colno_from_name(agg_out, "sum"),
                     .ascending = false};

  table_t *sorted = sort(agg_out, &sort_def);
  printf("num tuples :%lu\n", agg_out->num_tuples);
}

TEST_F(tpch_test, query5) {
  table_t *customer =
      get_table("SELECT c_custkey, c_nationkey FROM customer", dbname);
  table_t *orders =
      get_table("SELECT o_custkey, o_orderkey FROM orders WHERE o_orderdate "
                ">='1993-01-01' AND o_orderdate < '1994-01-01'",
                dbname);
  table_t *lineitem =
      get_table("SELECT l_orderkey, l_suppkey, l_extendedprice*(1-l_discount) "
                "as revenue FROM lineitem",
                dbname);
  table_t *supplier =
      get_table("SELECT s_suppkey, s_nationkey FROM supplier", dbname);
  table_t *nation =
      get_table("SELECT n_name, n_regionkey, n_nationkey FROM nation", dbname);
  table_t *region = get_table(
      "SELECT r_regionkey FROM region WHERE r_name= 'AFRICA'", dbname);

  join_def_t jd;
  jd.l_col = colno_from_name(nation, "n_regionkey");
  jd.r_col = colno_from_name(region, "r_regionkey");
  jd.project_len = 2;
  jd.project_list[0].side = LEFT_RELATION;
  jd.project_list[0].col_no = colno_from_name(nation, "n_nationkey");
  jd.project_list[1].side = LEFT_RELATION;
  jd.project_list[1].col_no = colno_from_name(nation, "n_name");
  table_t *nr = hash_join(nation, region, jd);
  bzero(&jd, sizeof(jd));

  jd.l_col = colno_from_name(customer, "c_nationkey");
  jd.r_col = colno_from_name(nr, "n_nationkey");
  jd.project_len = 2;
  jd.project_list[0].side = RIGHT_RELATION;
  jd.project_list[0].col_no = colno_from_name(nr, "n_name");
  jd.project_list[1].side = LEFT_RELATION;
  jd.project_list[1].col_no = colno_from_name(customer, "c_custkey");
  table_t *cnr = hash_join(customer, nr, jd);
  bzero(&jd, sizeof(jd));

  jd.l_col = colno_from_name(orders, "o_custkey");
  jd.r_col = colno_from_name(cnr, "c_custkey");
  jd.project_len = 2;
  jd.project_list[0].side = RIGHT_RELATION;
  jd.project_list[0].col_no = colno_from_name(cnr, "n_name");
  jd.project_list[1].side = LEFT_RELATION;
  jd.project_list[1].col_no = colno_from_name(orders, "o_orderkey");
  table_t *ocnr = hash_join(orders, cnr, jd);
  bzero(&jd, sizeof(jd));

  jd.l_col = colno_from_name(lineitem, "l_orderkey");
  jd.r_col = colno_from_name(ocnr, "o_orderkey");
  jd.project_len = 3;
  jd.project_list[0].side = RIGHT_RELATION;
  jd.project_list[0].col_no = colno_from_name(ocnr, "n_name");
  jd.project_list[1].side = LEFT_RELATION;
  jd.project_list[1].col_no = colno_from_name(lineitem, "l_suppkey");
  jd.project_list[2].side = LEFT_RELATION;
  jd.project_list[2].col_no = colno_from_name(lineitem, "revenue");
  table_t *locnr = hash_join(lineitem, ocnr, jd);
  bzero(&jd, sizeof(jd));

  jd.l_col = colno_from_name(supplier, "s_suppkey");
  jd.r_col = colno_from_name(locnr, "l_suppkey");
  jd.project_len = 2;
  jd.project_list[0].side = RIGHT_RELATION;
  jd.project_list[0].col_no = colno_from_name(locnr, "n_name");
  jd.project_list[1].side = RIGHT_RELATION;
  jd.project_list[1].col_no = colno_from_name(locnr, "revenue");
  table_t *slocnr = hash_join(supplier, locnr, jd);
  bzero(&jd, sizeof(jd));

  groupby_def_t def;
  def.secure = true;
  def.type = AVG;
  def.kanon_col = 0;
  def.colno = colno_from_name(slocnr, "revenue");
  def.num_cols = 1;
  def.gb_colnos[0] = colno_from_name(slocnr, "n_name");
  table_t *agg_out = aggregate(slocnr, &def);
  sort_t sort_def = {.colno = colno_from_name(agg_out, "sum"),
                     .ascending = false};
  table_t *sorted = sort(agg_out, &sort_def);
}

TEST_F(tpch_test, query10) {
  table_t *lineitem =
      get_table("SELECT l_orderkey, l_extendedprice *(1 - l_discount) as "
                "revenue FROM lineitem where l_returnflag = 'R'",
                dbname);
  table_t *orders =
      get_table("SELECT o_custkey, o_orderkey from orders WHERE o_orderdate "
                "<'1995-10-01' AND o_orderdate >= '1993-07-01'",
                dbname);
  table_t *customer = get_table("SELECT c_custkey, c_name, c_acctbal, \
	  c_phone, c_address, c_comment, c_nationkey FROM customer;",
                                dbname);
  table_t *nation = get_table("SELECT n_nationkey, n_name FROM nation", dbname);

  join_def_t jd;
  jd.l_col = colno_from_name(lineitem, "l_orderkey");
  jd.r_col = colno_from_name(orders, "o_orderkey");
  jd.project_len = 2;
  jd.project_list[0].side = RIGHT_RELATION;
  jd.project_list[0].col_no = colno_from_name(orders, "o_custkey");
  jd.project_list[1].side = LEFT_RELATION;
  jd.project_list[1].col_no = colno_from_name(lineitem, "revenue");
  table_t *lo = hash_join(lineitem, orders, jd);
  bzero(&jd, sizeof(join_def_t));

  jd.l_col = colno_from_name(lo, "o_custkey");
  jd.r_col = colno_from_name(customer, "c_custkey");
  jd.project_len = 8;
  jd.project_list[0] = {.side = RIGHT_RELATION,
                        .col_no = colno_from_name(customer, "c_custkey")};
  jd.project_list[1].side = RIGHT_RELATION;
  jd.project_list[1].col_no = colno_from_name(customer, "c_name");
  jd.project_list[2].side = RIGHT_RELATION;
  jd.project_list[2].col_no = colno_from_name(customer, "c_acctbal");
  jd.project_list[3].side = RIGHT_RELATION;
  jd.project_list[3].col_no = colno_from_name(customer, "c_phone");
  jd.project_list[4].side = RIGHT_RELATION;
  jd.project_list[4].col_no = colno_from_name(customer, "c_address");
  jd.project_list[5].side = RIGHT_RELATION;
  jd.project_list[5].col_no = colno_from_name(customer, "c_comment");
  jd.project_list[6].side = LEFT_RELATION;
  jd.project_list[6].col_no = colno_from_name(lo, "revenue");
  jd.project_list[7].side = RIGHT_RELATION;
  jd.project_list[7].col_no = colno_from_name(customer, "c_nationkey");
  table_t *clo = hash_join(lo, customer, jd);
  bzero(&jd, sizeof(join_def_t));

  jd.l_col = colno_from_name(clo, "c_nationkey");
  jd.r_col = colno_from_name(nation, "n_nationkey");

  jd.project_len = 8;
  jd.project_list[0].side = LEFT_RELATION;
  jd.project_list[0].col_no = colno_from_name(clo, "c_custkey");
  jd.project_list[1].side = LEFT_RELATION;
  jd.project_list[1].col_no = colno_from_name(clo, "c_name");
  jd.project_list[2].side = LEFT_RELATION;
  jd.project_list[2].col_no = colno_from_name(clo, "c_acctbal");
  jd.project_list[3].side = LEFT_RELATION;
  jd.project_list[3].col_no = colno_from_name(clo, "c_phone");
  jd.project_list[4].side = LEFT_RELATION;
  jd.project_list[4].col_no = colno_from_name(clo, "c_address");
  jd.project_list[5].side = LEFT_RELATION;
  jd.project_list[5].col_no = colno_from_name(clo, "c_comment");
  jd.project_list[6].side = LEFT_RELATION;
  jd.project_list[6].col_no = colno_from_name(clo, "revenue");
  jd.project_list[7].side = RIGHT_RELATION;
  jd.project_list[7].col_no = colno_from_name(nation, "n_name");
  table_t *nclo = hash_join(clo, nation, jd);
  bzero(&jd, sizeof(join_def_t));

  groupby_def_t def;
  bzero(&def, sizeof(groupby_def_t));
  def.secure = true;
  def.type = AVG;
  def.kanon_col = 0;
  def.colno = colno_from_name(nclo, "revenue");
  def.num_cols = 7;
  def.gb_colnos[0] = colno_from_name(nclo, "c_custkey");
  def.gb_colnos[1] = colno_from_name(nclo, "c_name");
  def.gb_colnos[2] = colno_from_name(nclo, "c_acctbal");
  def.gb_colnos[3] = colno_from_name(nclo, "c_phone");
  def.gb_colnos[4] = colno_from_name(nclo, "n_name");
  def.gb_colnos[5] = colno_from_name(nclo, "c_address");
  def.gb_colnos[6] = colno_from_name(nclo, "c_comment");
  table_t *agg_out = aggregate(nclo, &def);
}

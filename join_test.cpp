//
// Created by root on 10/4/18.
//
#include <gtest/gtest.h>
#include <pqxx/result>

#include "join_test.h"
#include "postgres_client.h"
#include "HashJoin.h"

class join_test : public ::testing::Test {
public:
    std::string dbname;
protected:
    void SetUp() override {
        dbname = "dbname=test";
    };
    void TearDown() override{
    };
};

TEST_F(join_test, first_join_test){
    // std::string create_table("create table join_test (a INT, b INT)");
    std::string create_left_table("create table if not exists join_test_left as select a, floor(random()*10 + 1)::int from generate_series(1,20) a");
    pqxx::result res_l;
    res_l = query(create_left_table, dbname);

    std::string create_right_table("create table if not exists join_test_right as select a, floor(random()*10 + 1)::int from generate_series(1,20) a");
    pqxx::result res_r;
    res_r = query(create_right_table, dbname);

    std::string get_table_left("SELECT * FROM join_test_left;");

    std::string get_table_right("SELECT * FROM join_test_right;");

    table_builder_t *tbl = table_builder(get_table_left, dbname);
    table_builder_t *tbr = table_builder(get_table_right, dbname);

    // TODO: this isn't right!!! need to figure out how to make a better expression!
    expr_t expr = make_int_expr(EQ_EXPR, 1 /* field_val */, 0 /* colno */);
    table_t * t = HashJoin(tbl->table,tbr->table,0,1);


}

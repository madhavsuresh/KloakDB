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
        std::string query1("create table simple_join (a INT, b INT)");
        query(query1, dbname);
        query1 = "INSERT INTO simple_join (a,b) VALUES (1,7), (2,7), (3,4)";
        query(query1, dbname);
    };
    void TearDown() override{
        std::string query1("DROP table simple_join");
        query(query1, dbname);
    };
};

TEST_F(join_test, merge_tuple_simple) {
    std::string query1 = "SELECT * FROM simple_join";
    table_t *t = get_table(query1, dbname);
    tuple_t * to_fill = (tuple_t*) malloc(sizeof(tuple_t) + 3*sizeof(field_desc_t));
    join_def_t jd;
    jd.l_col = 1;
    jd.r_col = 1;
    jd.project_len = 2;
    jd.project_list[0].side = 0;
    jd.project_list[0].col_no = 0;
    jd.project_list[1].side = 1;
    jd.project_list[1].col_no = 0;
    merge_tuple(to_fill, get_tuple(1,t), get_tuple(0,t), jd);
    ASSERT_EQ(2, to_fill->field_list[0].f.int_field.val);
    ASSERT_EQ(1, to_fill->field_list[1].f.int_field.val);
    free_table(t);
    free(to_fill);
}

TEST_F(join_test, merge_tuple_simple_2) {
    std::string query1 = "SELECT * FROM simple_join";
    table_t *t = get_table(query1, dbname);
    tuple_t * to_fill = (tuple_t*) malloc(sizeof(tuple_t) + 3*sizeof(field_desc_t));
    join_def_t jd;
    jd.l_col = 1;
    jd.r_col = 0;
    jd.project_len = 2;
    jd.project_list[0].side = 0;
    jd.project_list[0].col_no = 0;
    jd.project_list[1].side = 1;
    jd.project_list[1].col_no = 1;
    merge_tuple(to_fill, get_tuple(1,t), get_tuple(2,t), jd);
    ASSERT_EQ(2, to_fill->field_list[0].f.int_field.val);
    ASSERT_EQ(4, to_fill->field_list[1].f.int_field.val);
    free_table(t);
    free(to_fill);
}

TEST_F(join_test, build_merged_schema) {
    std::string query1 = "SELECT * FROM simple_join";
    table_t *t = get_table(query1, dbname);
    join_def_t jd;
    jd.l_col = 1;
    jd.r_col = 0;
    jd.project_len = 2;
    jd.project_list[0].side = 0;
    jd.project_list[0].col_no = 0;
    jd.project_list[1].side = 1;
    jd.project_list[1].col_no = 1;
    schema_t s = build_join_schema(t,t,jd);
    ASSERT_STREQ(s.fields[0].field_name, "a");
    ASSERT_STREQ(s.fields[1].field_name, "b");
    ASSERT_EQ(s.fields[0].type, INT);
    ASSERT_EQ(s.fields[1].type, INT);
    free_table(t);
}

TEST_F(join_test, first_join_test){
    // std::string create_table("create table join_test (a INT, b INT)");
    std::string query1("create table full_join_simple (a INT, b INT)");
    query(query1, dbname);
    query1 = "INSERT INTO full_join_simple (a,b) VALUES (1,7), (7,8), (3,4)";
    query(query1, dbname);
    query1 = "SELECT * FROM full_join_simple";
    table_t *t = get_table(query1, dbname);

    join_def_t jd;
    jd.l_col = 1;
    jd.r_col = 0;
    jd.project_len = 2;
    jd.project_list[0].side = LEFT_RELATION;
    jd.project_list[0].col_no = 0;
    jd.project_list[1].side = RIGHT_RELATION;
    jd.project_list[1].col_no = 1;
    table_t * output = hash_join(t,t,jd);
    ASSERT_EQ(output->num_tuples, 1);
    ASSERT_EQ(get_tuple(0,output)->field_list[0].f.int_field.val, 1);
    ASSERT_EQ(get_tuple(0,output)->field_list[1].f.int_field.val, 8);
    query1 = "DROP table full_join_simple";
    query(query1, dbname);
    free_table(t);
    free_table(output);
}

TEST_F(join_test, second_join_test){
    // std::string create_table("create table join_test (a INT, b INT)");
    std::string query1("create table full_join_simple (a INT, b INT)");
    query(query1, dbname);
    query1 = "INSERT INTO full_join_simple (a,b) VALUES (1,7), (7,8), (3,4), (7,9)";
    query(query1, dbname);
    query1 = "SELECT * FROM full_join_simple";
    table_t *t = get_table(query1, dbname);

    join_def_t jd;
    jd.l_col = 1;
    jd.r_col = 0;
    jd.project_len = 2;
    jd.project_list[0].side = LEFT_RELATION;
    jd.project_list[0].col_no = 0;
    jd.project_list[1].side = RIGHT_RELATION;
    jd.project_list[1].col_no = 1;
    table_t * output = hash_join(t,t,jd);
    ASSERT_EQ(output->num_tuples, 2);
    ASSERT_EQ(get_tuple(0,output)->field_list[0].f.int_field.val, 1);
    ASSERT_EQ(get_tuple(0,output)->field_list[1].f.int_field.val, 8);
    ASSERT_EQ(get_tuple(1,output)->field_list[0].f.int_field.val, 1);
    ASSERT_EQ(get_tuple(1,output)->field_list[1].f.int_field.val, 9);
    query1 = "DROP table full_join_simple";
    query(query1, dbname);
    free_table(t);
    free_table(output);
}

TEST_F(join_test, third_join_test){
    // std::string create_table("create table join_test (a INT, b INT)");
    std::string query1("create table full_join_simple (a INT, b INT)");
    query(query1, dbname);
    query1 = "INSERT INTO full_join_simple (a,b) VALUES (1,7), (7,8), (3,4), (7,9), (4,5)";
    query(query1, dbname);
    query1 = "SELECT * FROM full_join_simple";
    table_t *t = get_table(query1, dbname);

    join_def_t jd;
    jd.l_col = 1;
    jd.r_col = 0;
    jd.project_len = 2;
    jd.project_list[0].side = LEFT_RELATION;
    jd.project_list[0].col_no = 0;
    jd.project_list[1].side = RIGHT_RELATION;
    jd.project_list[1].col_no = 1;
    table_t * output = hash_join(t,t,jd);
    ASSERT_EQ(output->num_tuples, 3);
    ASSERT_EQ(get_tuple(0,output)->field_list[0].f.int_field.val, 1);
    ASSERT_EQ(get_tuple(0,output)->field_list[1].f.int_field.val, 8);
    ASSERT_EQ(get_tuple(1,output)->field_list[0].f.int_field.val, 1);
    ASSERT_EQ(get_tuple(1,output)->field_list[1].f.int_field.val, 9);
    ASSERT_EQ(get_tuple(2,output)->field_list[0].f.int_field.val, 3);
    ASSERT_EQ(get_tuple(2,output)->field_list[1].f.int_field.val, 5);
    query1 = "DROP table full_join_simple";
    query(query1, dbname);
    free_table(t);
    free_table(output);
}

TEST_F(join_test, cross_product){
    // std::string create_table("create table join_test (a INT, b INT)");
    std::string query1("create table full_join_simple (a INT, b INT)");
    query(query1, dbname);
    query1 = "INSERT INTO full_join_simple (a,b) VALUES (1,2), (1,3), (1,4)";
    query(query1, dbname);
    query1 = "SELECT * FROM full_join_simple";
    table_t *t = get_table(query1, dbname);

    join_def_t jd;
    jd.l_col = 0;
    jd.r_col = 0;
    jd.project_len = 4;
    jd.project_list[0].side = LEFT_RELATION;
    jd.project_list[0].col_no = 0;
    jd.project_list[1].side = LEFT_RELATION;
    jd.project_list[1].col_no = 1;
    jd.project_list[2].side = RIGHT_RELATION;
    jd.project_list[2].col_no = 0;
    jd.project_list[3].side = RIGHT_RELATION;
    jd.project_list[3].col_no = 1;
    table_t * output = hash_join(t,t,jd);
    ASSERT_EQ(output->num_tuples, 9);

    printf("\n");
    std::string str_output;
    for (int i = 0; i < output->num_tuples; i++) {
        str_output += tuple_string(get_tuple(i, output)) + ",";
    }

    ASSERT_STREQ(str_output.c_str(),
        "1| 4| 1| 2| ,1| 3| 1| 2| ,1| 2| 1| 2| ,1| 4| 1| 3| ,1| 3| 1| 3| ,1| 2| 1| 3| ,1| 4| 1| 4| ,1| 3| 1| 4| ,1| 2| 1| 4| ,");
    query1 = "DROP table full_join_simple";
    query(query1, dbname);
    free_table(t);
    free_table(output);
}

TEST_F(join_test, large_join) {
    std::string dbname;
    dbname = "dbname=test";
    char buf[128];
    sprintf(buf,
            "create table if not exists test_large_join as select s, "
            "floor(random() * 100 +1)::int from generate_series(1,%d) s;",
            2048);
    std::string query_create(buf);
    pqxx::result res2;
    res2 = query(query_create, dbname);
    std::string query_string = "SELECT * FROM test_large_join";
    table_t *t = get_table(query_string, dbname);

    join_def_t jd;
    jd.l_col = 1;
    jd.r_col = 1;
    jd.project_len = 1;
    jd.project_list[0].side = LEFT_RELATION;
    jd.project_list[0].col_no = 0;
    table_t * output = hash_join(t,t,jd);
    printf("Size of output: %d", output->num_tuples);

    query_string = "DROP table test_large_join";
    query(query_string, dbname);
}

//
// Created by madhav on 7/27/18.
//
#include "postgres_client_test.h"
#include "postgres_client.h"
#include <malloc.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

class postgres_client_test : public ::testing::Test {
public:
    std::string dbname;

protected:
    void SetUp() override {
        // This test assumes query works... also existence of "t_random_500"
        // This test can just create the database we want
        dbname = "dbname=test";
    }

    void TearDown() override {
    };

};


TEST_F(postgres_client_test, get_schema) {
    table_t t;
    table_builder_t tb;
    tb.table = &t;
    std::string query_create("create table tz (s INT, floor INT)");
    query(query_create, dbname);
    std::string select("SELECT * FROM t;");
    pqxx::result res = query(select, dbname);
    tb.expected_tuples = res.capacity();
    tb.num_columns = res.columns();
    schema_t *s = get_schema_from_query(&tb, res);
    ASSERT_EQ(s->num_fields, 2);
    ASSERT_STRCASEEQ(s->fields[0].field_name, "s");
    ASSERT_EQ(s->fields[0].col_no, 0);
    ASSERT_EQ(s->fields[0].type, INT);
    ASSERT_STRCASEEQ(s->fields[1].field_name, "floor");
    ASSERT_EQ(s->fields[1].col_no, 1);
    ASSERT_EQ(s->fields[1].type, INT);
    //ASSERT_LE(malloc_usable_size(t.schema), 328);
    //ASSERT_EQ(t.schema.num_fields, s->num_fields);
    free(s);
    std::string query_destroy("DROP TABLE tz");
    res = query(query_destroy, dbname);
}

TEST_F(postgres_client_test, build_table) {
    std::string query_create("create table test_random as select s, floor(random() * 100 +1)::int from generate_series(1,3) s;");
    pqxx::result res2;
    res2 = query(query_create, dbname);
    std::string query_string = "SELECT * FROM test_random";
    pqxx::result t_random = query(query_string, dbname);
    table_builder_t * tb = table_builder(query_string, dbname);
    check_int_table(tb->table);
    free_table(tb->table);
    free(tb);

    std::string query_destroy("DROP TABLE test_random");
    res2 = query(query_destroy, dbname);
}

// TODO(madhavsuresh): write tests that pull out all values

void check_int_table(table_t * table) {
    uint32_t num_tuples = table->num_tuples;
    printf("\n");
    for (int i = 0; i < num_tuples; i++) {
        tuple_t *t = get_tuple(i, table);
        print_tuple(t);
        printf("\n");
    }
}
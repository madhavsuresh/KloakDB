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
    std::string select("SELECT * FROM tz;");
    pqxx::result res = query(select, dbname);
    tb.expected_tuples = res.capacity();
    tb.num_columns = res.columns();
    schema_t s = get_schema_from_query(&tb, res);
    ASSERT_EQ(s.num_fields, 2);
    ASSERT_STRCASEEQ(s.fields[0].field_name, "s");
    ASSERT_EQ(s.fields[0].col_no, 0);
    ASSERT_EQ(s.fields[0].type, INT);
    ASSERT_STRCASEEQ(s.fields[1].field_name, "floor");
    ASSERT_EQ(s.fields[1].col_no, 1);
    ASSERT_EQ(s.fields[1].type, INT);
    //ASSERT_LE(malloc_usable_size(t.schema), 328);
    //ASSERT_EQ(t.schema.num_fields, s->num_fields);
    std::string query_destroy("DROP TABLE tz");
    res = query(query_destroy, dbname);
}

TEST_F(postgres_client_test, get_tuple) {
    std::string query_create("create table if not exists test_random_get_tuple as select s, floor(random() * 100 +1)::int from generate_series(1,30000) s;");
    pqxx::result res2;
    res2 = query(query_create, dbname);
    std::string query_string = "SELECT * FROM test_random_get_tuple";
    auto *tb = (table_builder_t *) malloc(sizeof(table_builder_t));
    bzero(tb, sizeof(table_builder_t));

    pqxx::result res = query(query_string, dbname);
    init_table_builder_from_pq(res, tb);
    tuple_t * t = get_tuple(0, tb->table);
    char*  b = (char *) tb->table->tuple_pages[0];
    ASSERT_EQ((char *)t,b + sizeof(uint64_t));

}

TEST_F(postgres_client_test, build_table) {
    std::string query_create("create table if not exists test_random as select s, floor(random() * 100 +1)::int from generate_series(1,30000) s;");
    pqxx::result res2;
    res2 = query(query_create, dbname);
    std::string query_string = "SELECT * FROM test_random";
    pqxx::result t_random = query(query_string, dbname);
    table_builder_t * tb = table_builder(query_string, dbname);
    printf("\n num tuples per page: %d", tb->num_tuples_per_page);
    check_int_table(tb->table);
    free_table(tb->table);
    free(tb);

    std::string query_destroy("DROP TABLE test_random");
    res2 = query(query_destroy, dbname);
}

TEST_F(postgres_client_test, test_size) {
    printf("tuple_t %d", (int)sizeof(tuple_t));
    printf("field_t %d", + (int)sizeof(field_t));
    printf("field_union %d", + (int)sizeof(field_union));
}

// TODO(madhavsuresh): write tests that pull out all values
// TODO(madhavsuresh): test different width

void check_int_table(table_t * table) {
    uint32_t num_tuples = table->num_tuples;
    printf("\n");
    for (int i = 0; i < num_tuples; i++) {
        tuple_t *t = get_tuple(i, table);
        print_tuple(t);
        printf("\n");
    }
}

TEST_F(postgres_client_test, coalesce_tables) {
    std::string query1("create table coalesce_test (a INT, b INT)");
    query(query1, dbname);
    query1 = "INSERT INTO coalesce_test (a,b) VALUES (7,6), (8,3), (9,1)";
    query(query1, dbname);

    std::vector<table_t *> tables;

    query1 = "SELECT * FROM coalesce_test;";
    table_t *t = get_table(query1, dbname);
    ASSERT_EQ(t->num_tuples, 3);
    tables.push_back(t);
    t = get_table(query1, dbname);
    ASSERT_EQ(t->num_tuples, 3);
    tables.push_back(t);

    t = coalesce_tables(tables);
    ASSERT_EQ(t->num_tuples, 6);
    free_table(tables[0]);
    free_table(tables[1]);
    std::string query_destroy("DROP TABLE coalesce_test");
    query(query_destroy, dbname);
}

TEST_F(postgres_client_test, coalesce_big) {
    std::string query_create("create table if not exists test_random as select s, floor(random() * 100 +1)::int from generate_series(1,3000) s;");
    query(query_create, dbname);
    std::vector<table_t *> tables;
    std::string query1 = "SELECT * FROM test_random";
    table_t *t = get_table(query1, dbname);
    ASSERT_EQ(t->num_tuples, 3000);
    tables.push_back(t);
    t = get_table(query1, dbname);
    ASSERT_EQ(t->num_tuples, 3000);
    tables.push_back(t);
    t = coalesce_tables(tables);
    ASSERT_EQ(t->num_tuples, 6000);
    free_table(tables[0]);
    free_table(tables[1]);
    free_table(t);

    std::string query_destroy("DROP TABLE test_random");
    query(query_destroy, dbname);
}

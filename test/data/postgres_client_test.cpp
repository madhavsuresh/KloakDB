//
// Created by madhav on 7/27/18.
//
#include "postgres_client_test.h"
#include "data/postgres_client.h"
#include "data/pqxx_compat.h"
#include <gtest/gtest.h>
#include <malloc.h>

class postgres_client_test : public ::testing::Test {
public:
  std::string dbname;

protected:
  void SetUp() override {
    // This test assumes query works... also existence of "t_random_500"
    // This test can just create the database we want
    dbname = "dbname=test";
  }

  void TearDown() override{};
};

TEST_F(postgres_client_test, get_schema) {
  table_t t;
  table_builder_t tb;
  tb.table = &t;
  query("DROP TABLE if EXISTS tz;", dbname);
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
  // ASSERT_LE(malloc_usable_size(t.schema), 328);
  // ASSERT_EQ(t.schema.num_fields, s->num_fields);
  std::string query_destroy("DROP TABLE tz");
  res = query(query_destroy, dbname);
}

TEST_F(postgres_client_test, get_tuple) {
  query("DROP table if EXISTS test_random_get_tuple", dbname);
  std::string query_create("create table if not exists test_random_get_tuple "
                           "as select s, floor(random() * 100 +1)::int from "
                           "generate_series(1,30000) s;");
  pqxx::result res2;
  res2 = query(query_create, dbname);
  std::string query_string = "SELECT * FROM test_random_get_tuple";
  auto *tb = (table_builder_t *)malloc(sizeof(table_builder_t));
  bzero(tb, sizeof(table_builder_t));

  pqxx::result res = query(query_string, dbname);
  init_table_builder_from_pq(res, tb);
  tuple_t *t = get_tuple(0, tb->table);
  char *b = (char *)tb->table->tuple_pages[0];
  ASSERT_EQ((char *)t, b + sizeof(uint64_t));
}

TEST_F(postgres_client_test, build_table) {
  //TODO(madhavsuresh): this test doesn't do anything
  query("DROP table if EXISTS test_random", dbname);
  std::string query_create("create table if not exists test_random as select "
                           "s, floor(random() * 100 +1)::int from "
                           "generate_series(1,30000) s;");
  pqxx::result res2;
  res2 = query(query_create, dbname);
  std::string query_string = "SELECT * FROM test_random";
  pqxx::result t_random = query(query_string, dbname);
  table_builder_t *tb = table_builder_init(query_string, dbname);
  printf("\n num tuples per page: %d", tb->num_tuples_per_page);
  check_int_table(tb->table);
  free_table(tb->table);
  free(tb);

  std::string query_destroy("DROP TABLE test_random");
  res2 = query(query_destroy, dbname);
}

// TODO(madhavsuresh): write tests that pull out all values
// TODO(madhavsuresh): test different width

void check_int_table(table_t *table) {
  uint32_t num_tuples = table->num_tuples;
  for (int i = 0; i < num_tuples; i++) {
    tuple_t *t = get_tuple(i, table);
  }
}

TEST_F(postgres_client_test, coalesce_tables) {
  query("DROP table if EXISTS coalesce_test", dbname);
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
  query("DROP table if EXISTS test_random", dbname);
  std::string query_create("create table if not exists test_random as select "
                           "s, floor(random() * 100 +1)::int from "
                           "generate_series(1,3000) s;");
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

TEST_F(postgres_client_test, column_too_large) {
  query("DROP table if EXISTS big_column", dbname);
  std::string query_create("create table big_column (a INT, b VARCHAR)");
  query(query_create, dbname);

  query_create = "INSERT INTO big_column (a,b) VALUES (1, '1234567890123456');";
  query(query_create, dbname);

  try {
    table_t *t = get_table("SELECT * FROM big_column", dbname);
  } catch (std::invalid_argument const &err) {
    ASSERT_STREQ(err.what(), "Unsupported length column");
  } catch (...) {
    FAIL() << "Expected unsupported length column";
  }

  std::string query_destroy("DROP TABLE big_column");
  query(query_destroy, dbname);
}

TEST_F(postgres_client_test, timestamp) {
  query("DROP table if EXISTS timestamp", dbname);
  query("CREATE TABLE timestamp (a INT, b TIMESTAMP);", dbname);
  query("INSERT INTO timestamp (a,b) VALUES (1,'01/01/06');", dbname);
  table_t *t = get_table("SELECT * FROM timestamp;", dbname);
  ASSERT_EQ(t->schema.fields[0].type, INT);
  ASSERT_EQ(t->schema.fields[1].type, TIMESTAMP);
  ASSERT_STREQ(tuple_string(get_tuple(0, t)).c_str(),
               "1| Sun Jan  1 00:00:00 2006| ");
  query("DROP TABLE timestamp", dbname);
}

TEST_F(postgres_client_test, real) {
  query("DROP table if EXISTS real", dbname);
  query("CREATE TABLE real (a REAL, b REAL);", dbname);
  query("INSERT INTO real (a,b) VALUES (1,1.1);", dbname);
  table_t *t = get_table("SELECT * FROM real;", dbname);
  ASSERT_EQ(t->schema.fields[0].type, DOUBLE);
  ASSERT_EQ(t->schema.fields[1].type, DOUBLE);
  ASSERT_EQ(get_tuple(0, t)->field_list[0].f.double_field.val, 1);
  ASSERT_EQ(get_tuple(0, t)->field_list[1].f.double_field.val, 1.1);
  query("DROP TABLE real", dbname);
}

TEST_F(postgres_client_test, colname) {
  query("DROP table if EXISTS real", dbname);
  query("CREATE TABLE real (a REAL, b REAL);", dbname);
  query("INSERT INTO real (a,b) VALUES (1,1.1);", dbname);
  table_t *t = get_table("SELECT * FROM real;", dbname);
  ASSERT_EQ(colno_from_name(t, "a"), 0);
  ASSERT_EQ(colno_from_name(t, "b"), 1);
  query("DROP TABLE real", dbname);
}

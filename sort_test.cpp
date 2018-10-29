#include <cstdio>
#include <gtest/gtest.h>
#include <pqxx/result>

#include "Filter.h"
#include "Sort.h"
#include "filter_test.h"
#include "postgres_client.h"
#include "postgres_client_test.h"

class sort_test : public ::testing::Test {
public:
protected:
  std::string dbname;
  void SetUp() override { dbname = "dbname=test"; };
  void TearDown() override{

  };
};

TEST_F(sort_test, simple_sort) {
  std::string query1("create table sort_test (a INT, b INT)");
  pqxx::result res2;
  res2 = query(query1, dbname);
  query1 = "INSERT INTO sort_test (a,b) VALUES (7,6), (8,3), (9,1)";
  res2 = query(query1, dbname);
  query1 = "SELECT * FROM sort_test;";
  table_builder_t *tb = table_builder(query1, dbname);
  sort_t sortex = {.colno = 1, .ascending = true};
  for (int i = 0; i < tb->table->num_tuples; i++) {
    auto tup = get_tuple(i, tb->table);
    if (!tup->is_dummy) {
      print_tuple(tup);
      printf("\n");
    }
  }
  table_t *t = sort(tb->table, &sortex);
  for (int i = 0; i < t->num_tuples; i++) {
    auto tup = get_tuple(i, t);
    if (!tup->is_dummy) {
      print_tuple(tup);
      printf("\n");
    }
  }
  std::string query_destroy("DROP TABLE sort_test");
  res2 = query(query_destroy, dbname);
  free_table(tb->table);
  free(tb);
}


// Unit test to ensure swapping two tuples is correct and doesn't loose information
TEST_F(sort_test, swap_unit){
  std::string query1("DROP TABLE if exists swap_test ");
  pqxx::result res2;
  res2 = query(query1, dbname);
  query1 = ("create table if not exists swap_test (a INT, b INT)");
  res2 = query(query1, dbname);
  query1 = "INSERT INTO swap_test (a,b) VALUES (7,6), (8,3)";
  res2 = query(query1, dbname);
  query1 = "SELECT * FROM swap_test;";
  table_builder_t *tb = table_builder(query1, dbname);

  tuple_t *old_t0_ptr = get_tuple(0,tb->table);
  tuple_t *old_t1_ptr = get_tuple(1,tb->table);

  // important step to set one to be true, default is false
  old_t1_ptr->is_dummy = true;
  tuple_t *old_t0 = (tuple_t *)malloc(tb->table->size_of_tuple);
  memcpy(old_t0, old_t0_ptr, tb->table->size_of_tuple);

  tuple_t *old_t1 = (tuple_t *)malloc(tb->table->size_of_tuple);
  memcpy(old_t1, old_t1_ptr, tb->table->size_of_tuple);

  swap_tuples(0,1,tb->table,true);

  tuple_t *new_t0 = get_tuple(0,tb->table);
  tuple_t *new_t1 = get_tuple(1,tb->table);

  for (int i = 0; i < tb->num_columns; i++)
  {
    EXPECT_EQ(new_t0->field_list[i].f.int_field.val, old_t1->field_list[i].f.int_field.val);
    EXPECT_EQ(new_t1->field_list[i].f.int_field.val, old_t0->field_list[i].f.int_field.val); // make sure all swapped values in all cols are same
  }

  EXPECT_EQ(new_t0->num_fields, old_t1->num_fields);
  EXPECT_EQ(new_t1->num_fields, old_t0->num_fields); // make sure num_fields isn't changed

  EXPECT_EQ(new_t0->is_dummy, old_t1->is_dummy);
  EXPECT_EQ(new_t1->is_dummy, old_t0->is_dummy);

  free(old_t0);
  free(old_t1);
  std::string query_destroy("DROP TABLE swap_test");
  res2 = query(query_destroy, dbname);
  free_table(tb->table);
  free(tb);
}



TEST_F(sort_test, string_type) {
  std::string query1("create table sort_test (a varchar(16), b INT)");
  pqxx::result res2;
  res2 = query(query1, dbname);
  query1 =
      "INSERT INTO sort_test (a,b) VALUES ('hello',6), ('world',3), ('foo',1)";
  res2 = query(query1, dbname);
  query1 = "SELECT * FROM sort_test;";
  table_builder_t *tb = table_builder(query1, dbname);
  sort_t sortex = {.colno = 1, .ascending = true};
  for (int i = 0; i < tb->table->num_tuples; i++) {
    auto tup = get_tuple(i, tb->table);
    if (!tup->is_dummy) {
      print_tuple(tup);
      printf("\n");
    }
  }
  table_t *t = sort(tb->table, &sortex);
  for (int i = 0; i < t->num_tuples; i++) {
    auto tup = get_tuple(i, t);
    if (!tup->is_dummy) {
      print_tuple(tup);
      printf("\n");
    }
  }
  std::string query_destroy("DROP TABLE sort_test");
  res2 = query(query_destroy, dbname);
  free_table(t);
  free(tb);
}

void large_random_sort(int power_of_two) {
  std::string dbname;
  dbname = "dbname=test";
  char buf[128];
  sprintf(buf,
          "create table if not exists test_random_sort as select s, "
          "floor(random() * 100 +1)::int from generate_series(1,%d) s;",
          power_of_two);
  std::string query_create(buf);
  pqxx::result res2;
  res2 = query(query_create, dbname);
  std::string query_string = "SELECT * FROM test_random_sort";
  pqxx::result t_random = query(query_string, dbname);
  table_builder_t *tb = table_builder(query_string, dbname);
  sort_t sortex = {.colno = 1, .ascending = true};
  uint64_t max_val = 0;
  table_t *t = sort(tb->table, &sortex);
  for (int i = 0; i < t->num_tuples; i++) {
    auto tup = get_tuple(i, t);
    if (!tup->is_dummy) {
      ASSERT_GE(tup->field_list[1].f.int_field.val, max_val);
      max_val = tup->field_list[1].f.int_field.val;
    }
  }

  free_table(tb->table);
  free(tb);
  std::string query_destroy("DROP TABLE test_random_sort");
  res2 = query(query_destroy, dbname);
}

void sort_swap_dummy_regression(int power_of_two) {
  std::string dbname;
  dbname = "dbname=test";
  char buf[128];
  sprintf(buf,
          "create table if not exists test_random_sort as select s, "
          "floor(random() * 100 +1)::int from generate_series(1,%d) s;",
          power_of_two);
  std::string query_create(buf);
  pqxx::result res2;
  res2 = query(query_create, dbname);
  std::string query_string = "SELECT * FROM test_random_sort";
  pqxx::result t_random = query(query_string, dbname);
  table_builder_t *tb = table_builder(query_string, dbname);

  //sort on column 1
  sort_t sortex = {.colno = 1, .ascending = true};

  uint64_t max_val = 0;
  table_t *t = sort(tb->table, &sortex);
  for (int i = 0; i < t->num_tuples; i++) {
    auto tup = get_tuple(i, t);
    if (!tup->is_dummy) {
      ASSERT_GE(tup->field_list[1].f.int_field.val, max_val);
      max_val = tup->field_list[1].f.int_field.val;
    }
  }

  free_table(tb->table);
  free(tb);
  std::string query_destroy("DROP TABLE test_random_sort");
  res2 = query(query_destroy, dbname);
}


TEST_F(sort_test, large_random_sort_512) { large_random_sort(512); }
/*

TEST_F(sort_test, large_random_sort_256) { large_random_sort(256); }

TEST_F(sort_test, large_random_sort_600) { large_random_sort(600); }

TEST_F(sort_test, large_random_sort_90000) { large_random_sort(90000); }

TEST_F(sort_test, large_random_sort_500000) { large_random_sort(500000); }

TEST_F(sort_test, large_random_sort_1000000) { large_random_sort(1000000); }

TEST_F(sort_test, large_random_sort_4000000) { large_random_sort(4000000); }
*/
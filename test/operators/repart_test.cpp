//
// Created by madhav on 11/29/18.
//
#include <gtest/gtest.h>
#include <data/pqxx_compat.h>
#include "rpc/Repartition.h"

class repart_test : public ::testing::Test {
public:
    std::string dbname;
    std::string xdbname;
    void SetUp() override {
      xdbname = "repart_name";
      std::string command = "createdb " + xdbname;
      system(command.c_str());
      dbname = "dbname=" + xdbname;
    }

    void TearDown() override{
      std::string command = "dropdb " + xdbname;
      system(command.c_str());
    };

};

TEST_F(repart_test, multi_column) {

  ::vaultdb::ControlFlowColumn cf;
  cf.add_cf_name_strings("a");
  cf.add_cf_name_strings("b");
  query("CREATE TABLE test (a INT, b INT, c INT)", dbname);
  query("INSERT INTO test (a,b,c) VALUES (1,2,3), (1,2,4), (1,2,5), (1,4,5), (1,4,6)", dbname);
  table_t * t = get_table("SELECT * FROM test", dbname);

  ASSERT_EQ(hash_to_host(cf, 4, get_tuple(0, t), t), hash_to_host(cf, 4, get_tuple(1, t),t));
  ASSERT_EQ(hash_to_host(cf, 4, get_tuple(1, t), t), hash_to_host(cf, 4, get_tuple(2, t),t));
  ASSERT_NE(hash_to_host(cf, 4, get_tuple(1, t), t), hash_to_host(cf, 4, get_tuple(3, t),t));
  ASSERT_EQ(hash_to_host(cf, 4, get_tuple(3, t), t), hash_to_host(cf, 4, get_tuple(4, t),t));
}

TEST_F(repart_test, string_multi_column) {

  ::vaultdb::ControlFlowColumn cf;
  cf.add_cf_name_strings("a");
  cf.add_cf_name_strings("b");
  query("CREATE TABLE test (a VARCHAR, b VARCHAR, c VARCHAR)", dbname);
  query("INSERT INTO test (a,b,c) VALUES ('1a','2b','3c'), ('1a','2b','4c'), ('1a','2c','5d'), ('1a','4z','5e'), ('1a','4z','6hh')", dbname);
  table_t * t = get_table("SELECT * FROM test", dbname);

  ASSERT_EQ(hash_to_host(cf, 4, get_tuple(0, t), t), hash_to_host(cf, 4, get_tuple(1, t),t));
  ASSERT_NE(hash_to_host(cf, 4, get_tuple(1, t), t), hash_to_host(cf, 4, get_tuple(2, t),t));
  ASSERT_NE(hash_to_host(cf, 4, get_tuple(1, t), t), hash_to_host(cf, 4, get_tuple(3, t),t));
  ASSERT_EQ(hash_to_host(cf, 4, get_tuple(3, t), t), hash_to_host(cf, 4, get_tuple(4, t),t));
}

TEST_F(repart_test, mixed_column) {

  ::vaultdb::ControlFlowColumn cf;
  cf.add_cf_name_strings("a");
  cf.add_cf_name_strings("b");
  cf.add_cf_name_strings("d");
  query("CREATE TABLE test (a VARCHAR, b VARCHAR, c VARCHAR, d INT)", dbname);
  query("INSERT INTO test (a,b,c,d) VALUES ('1a','2b','3c', 1), ('1a','2b','4c', 1), ('1a','2b','5d', 2), ('1a','4z','5e', 10), ('1a','4z','6hh', 10)", dbname);
  table_t * t = get_table("SELECT * FROM test", dbname);

  ASSERT_EQ(hash_to_host(cf, 4, get_tuple(0, t), t), hash_to_host(cf, 4, get_tuple(1, t),t));
  ASSERT_NE(hash_to_host(cf, 4, get_tuple(1, t), t), hash_to_host(cf, 4, get_tuple(2, t),t));
  ASSERT_NE(hash_to_host(cf, 4, get_tuple(1, t), t), hash_to_host(cf, 4, get_tuple(3, t),t));
  ASSERT_EQ(hash_to_host(cf, 4, get_tuple(3, t), t), hash_to_host(cf, 4, get_tuple(4, t),t));
}

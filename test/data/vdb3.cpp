//
// Created by madhav on 7/27/18.
//
#include "data/postgres_client.h"
#include "data/pqxx_compat.h"
#include <gtest/gtest.h>
#include <malloc.h>

class vdb3: public ::testing::Test {
public:
  std::string dbname;
  std::string test_year;
  std::string site1;
  std::string site2;

protected:
  void SetUp() override {
    dbname = "dbname=healthlnk";
  }

  void TearDown() override{};
};

TEST_F(vdb3, demographics) {
  table_t *t = get_table("SELECT * FROM demographics", dbname);
  free_table(t);
}

TEST_F(vdb3, vitals) {
  table_t *t = get_table("SELECT * FROM vitals", dbname);
  free_table(t);
}

TEST_F(vdb3, diagnoses) {
  table_t *t = get_table("SELECT * FROM diagnoses", dbname);
  free_table(t);
}

TEST_F(vdb3, medications) {
  table_t *t = get_table("SELECT * FROM medications", dbname);
  free_table(t);
}


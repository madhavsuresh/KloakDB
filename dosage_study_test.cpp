//
// Created by madhav on 11/6/18.
//
#include "Aggregate.h"
#include "HashJoin.h"
#include "Logger.h"
#include "Sort.h"
#include "pqxx_compat.h"
#include <chrono>
#include <gtest/gtest.h>

class dosage_study_test : public ::testing::Test {
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
    dbname = "dbname=smcql_testDB";
    test_year = "2006";
    site1 = "4";
    site2 = "7";
  }

  void TearDown() override{};
};
TEST_F(dosage_study_test, full_dosage_study_test) {
  // ICD9 for internal bleeding: 997.49

  auto b4_readstart = std::chrono::high_resolution_clock::now();
  table_t *diagnoses =
      get_table("SELECT * from diagnoses where icd9='997.49'", dbname);
  table_t *medications = get_table(
      "SELECT * from medications where medication='aspirin' and dosage='325mg'",
      dbname);
  join_def_t join_def;
  join_def.l_col = colno_from_name(diagnoses, "patient_id");
  join_def.r_col = colno_from_name(medications, "patient_id");
  join_def.project_len = 1;
  join_def.project_list[0].side = LEFT_RELATION;
  join_def.project_list[0].col_no = colno_from_name(diagnoses, "patient_id");
  table_t *output = hash_join(diagnoses, medications, join_def);
  auto start = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = start - b4_readstart;
  std::cout << "Read Elapsed time: " << elapsed.count() << " s\n";
}

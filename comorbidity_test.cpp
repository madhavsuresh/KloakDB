//
// Created by madhav on 11/6/18.
//
#include "Aggregate.h"
#include "HashJoin.h"
#include "Logger.h"
#include "pqxx_compat.h"
#include "Sort.h"
#include <chrono>
#include <gtest/gtest.h>

class comorbidity_test : public ::testing::Test {
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
TEST_F(comorbidity_test, full_comorbidity_test) {

  auto b4_readstart = std::chrono::high_resolution_clock::now();
  table_t *cdiff_cohort_scan = get_table(
      "SELECT * from cdiff_cohort_diagnoses", dbname);
  auto start = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = start - b4_readstart;
  std::cout << "Read Elapsed time: " << elapsed.count() << " s\n";
  groupby_def_t gbd;
  gbd.type =  COUNT;
  gbd.colno = colno_from_name(cdiff_cohort_scan, "major_icd9");
  table_t *agg_output = aggregate(cdiff_cohort_scan, &gbd);
  auto end_aggregate = std::chrono::high_resolution_clock::now();
  printf("\n");
   elapsed = end_aggregate - start;
  std::cout << "Aggregate Elapsed time: " << elapsed.count() << " s\n";
  sort_t sort_def;
  sort_def.colno = colno_from_name(agg_output, "count");
  sort_def.ascending = false;
  table_t *sort_output = sort(agg_output, &sort_def);
  auto end_sort = std::chrono::high_resolution_clock::now();
   elapsed = end_sort - end_aggregate;
  std::cout << "Sort Elapsed time: " << elapsed.count() << " s\n";
  elapsed = end_sort - b4_readstart;
  std::cout << "Total Elapsed Time:" << elapsed.count() << "s\n";
}

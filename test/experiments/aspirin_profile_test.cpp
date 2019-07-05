//
// Created by madhav on 11/6/18.
//
#include "data/pqxx_compat.h"
#include "operators/Aggregate.h"
#include "operators/HashJoin.h"
#include <chrono>
#include <gtest/gtest.h>

class aspirin_profile_test : public ::testing::Test {
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
    dbname = "dbname=healthlnk";
    test_year = "2006";
    site1 = "4";
    site2 = "7";
  }

  void TearDown() override{};
};
TEST_F(aspirin_profile_test, full_aspirin_tests) {

  table_t *medications_scan =
      get_table("SELECT patient_id, medication from medications where "
                "medication = 'aspirin'",
                dbname);
  table_t *demographics_scan = get_table(
      "SELECT DISTINCT patient_id, gender, race from demographics", dbname);
  table_t *diagnoses_scan = get_table(
      "SELECT icd9, patient_id from diagnoses where diag_src = 'hd'", dbname);
  table_t *vitals_scan =
      get_table("SELECT patient_id, pulse from vitals", dbname);
  auto start = std::chrono::high_resolution_clock::now();

  join_def_t vd_join;
  vd_join.l_col = colno_from_name(vitals_scan, "patient_id");
  vd_join.r_col = colno_from_name(diagnoses_scan, "patient_id");
  vd_join.project_len = 2;
  vd_join.project_list[0].side = LEFT_RELATION;
  vd_join.project_list[0].col_no = colno_from_name(vitals_scan, "patient_id");
  vd_join.project_list[1].side = LEFT_RELATION;
  vd_join.project_list[1].col_no = colno_from_name(vitals_scan, "pulse");
  table_t *vitals_diagnosis_join =
      hash_join(vitals_scan, diagnoses_scan, vd_join);
  auto end_first_join = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end_first_join - start;
  std::cout << "First Join Elapsed time: " << elapsed.count() << " s\n";
  free_table(diagnoses_scan);
  free_table(vitals_scan);

  join_def_t vdijm_join;
  vdijm_join.l_col = colno_from_name(vitals_diagnosis_join, "patient_id");
  vdijm_join.r_col = colno_from_name(medications_scan, "patient_id");
  vdijm_join.project_len = 2;
  vdijm_join.project_list[0].side = LEFT_RELATION;
  vdijm_join.project_list[0].col_no =
      colno_from_name(vitals_diagnosis_join, "patient_id");
  vdijm_join.project_list[1].side = LEFT_RELATION;
  vdijm_join.project_list[1].col_no =
      colno_from_name(vitals_diagnosis_join, "pulse");
  table_t *vitals_diagnosis_join_medication_join =
      hash_join(vitals_diagnosis_join, medications_scan, vdijm_join);
  free_table(vitals_diagnosis_join);
  free_table(medications_scan);
  auto end_second_join = std::chrono::high_resolution_clock::now();
  elapsed = end_second_join - end_first_join;
  std::cout << "Second Join Elapsed time: " << elapsed.count() << " s\n";

  join_def_t vdijmjde_join;
  vdijmjde_join.l_col =
      colno_from_name(vitals_diagnosis_join_medication_join, "patient_id");
  vdijmjde_join.r_col = colno_from_name(demographics_scan, "patient_id");
  vdijmjde_join.project_len = 3;
  /*
  vdijmjde_join.project_list[0].side  = LEFT_RELATION;
  vdijmjde_join.project_list[0].col_no =
  colno_from_name(vitals_diagnosis_join_medication_join, "patient_id");
   */
  vdijmjde_join.project_list[0].side = LEFT_RELATION;
  vdijmjde_join.project_list[0].col_no =
      colno_from_name(vitals_diagnosis_join_medication_join, "pulse");
  vdijmjde_join.project_list[1].side = RIGHT_RELATION;
  vdijmjde_join.project_list[1].col_no =
      colno_from_name(demographics_scan, "gender");
  vdijmjde_join.project_list[2].side = RIGHT_RELATION;
  vdijmjde_join.project_list[2].col_no =
      colno_from_name(demographics_scan, "race");
  printf("before last join\n");
  table_t *vitals_diagnosis_join_medication_join_demographics_join = hash_join(
      vitals_diagnosis_join_medication_join, demographics_scan, vdijmjde_join);
  auto end_third_join = std::chrono::high_resolution_clock::now();
  elapsed = end_third_join - end_second_join;
  std::cout << "Third Join Elapsed time: " << elapsed.count() << " s\n";
  free_table(vitals_diagnosis_join_medication_join);
  free_table(demographics_scan);

  groupby_def_t gbd;
  gbd.type = AVG;
  gbd.num_cols = 2;
  gbd.colno = colno_from_name(
      vitals_diagnosis_join_medication_join_demographics_join, "pulse");
  gbd.gb_colnos[0] = colno_from_name(
      vitals_diagnosis_join_medication_join_demographics_join, "gender");
  gbd.gb_colnos[1] = colno_from_name(
      vitals_diagnosis_join_medication_join_demographics_join, "race");
  table_t *final_avg =
      aggregate(vitals_diagnosis_join_medication_join_demographics_join, &gbd);
  auto finish = std::chrono::high_resolution_clock::now();
  elapsed = finish - end_third_join;
  std::cout << "Aggregate Elapsed time: " << elapsed.count() << " s\n";
  elapsed = finish - start;
  std::cout << "Total Join Elapsed time: " << elapsed.count() << " s\n";

  free_table(vitals_diagnosis_join_medication_join_demographics_join);

  free_table(final_avg);
}

//
// Created by madhav on 11/6/18.
//
#include <gtest/gtest.h>
#include "pqxx_compat.h"
#include "HashJoin.h"

class aspirin_profile_test :public ::testing::Test {
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
    TEST_F(aspirin_profile_test , full_aspirin_tests) {

      table_t * medications_scan = get_table("SELECT * from medications where medication = 'aspirin'", dbname);
      table_t * demographics_scan = get_table("SELECT * from demographics", dbname);
      table_t * diagnoses_scan = get_table("SELECT * from diagnoses where diag_src = 'hd'", dbname);
      table_t * vitals_scan = get_table("SELECT * from vitals", dbname);

      join_def_t vd_join;
      vd_join.l_col = colno_from_name(vitals_scan, "patient_id");
      vd_join.r_col = colno_from_name(diagnoses_scan, "patient_id");
      vd_join.project_len = 2;
      vd_join.project_list[0].side = LEFT_RELATION;
      vd_join.project_list[0].col_no = colno_from_name(vitals_scan, "patient_id");
      vd_join.project_list[1].side = LEFT_RELATION;
      vd_join.project_list[1].col_no = colno_from_name(vitals_scan, "pulse");
      table_t * vitals_diagnosis_join = hash_join(vitals_scan, diagnoses_scan, vd_join);

      join_def_t vdijm_join;
      vdijm_join.project_len = 2;
      vdijm_join.project_list[0].side = LEFT_RELATION;
      vdijm_join.project_list[0].col_no = colno_from_name(vitals_diagnosis_join, "patient_id");
      vdijm_join.project_list[1].side = LEFT_RELATION;
      vdijm_join.project_list[1].col_no = colno_from_name(vitals_diagnosis_join, "pulse");
      table_t * vitals_diagnosis_join_medication_join = hash_join(vitals_diagnosis_join, medications_scan, vdijm_join);

      join_def_t vdijmjde_join;
      vdijmjde_join.project_len = 4;
      vdijmjde_join.project_list[0].side  = LEFT_RELATION;
      vdijmjde_join.project_list[0].col_no = colno_from_name(vitals_diagnosis_join_medication_join, "patient_id");
      vdijmjde_join.project_list[1].side  = LEFT_RELATION;
      vdijmjde_join.project_list[1].col_no = colno_from_name(vitals_diagnosis_join_medication_join, "pulse");
      vdijmjde_join.project_list[2].side  = RIGHT_RELATION;
      vdijmjde_join.project_list[2].col_no = colno_from_name(demographics_scan, "gender");
      vdijmjde_join.project_list[2].side  = RIGHT_RELATION;
      vdijmjde_join.project_list[2].col_no = colno_from_name(demographics_scan, "race");
      table_t * vitals_diagnosis_join_medication_join_demographics_join = hash_join(vitals_diagnosis_join_medication_join, demographics_scan, vdijmjde_join);
    }



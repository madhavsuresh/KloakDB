//
// Created by madhav on 7/27/18.
//
#include "postgres_client_test.h"
#include "postgres_client.h"
#include "pqxx_compat.h"
#include "Logger.h"
#include <gtest/gtest.h>
#include <malloc.h>

class health_lnk_schema_test : public ::testing::Test {
public:
  std::string dbname;

protected:
  void SetUp() override {
    // This test assumes query works... also existence of "t_random_500"
    // This test can just create the database we want
    // TODO(madhavsuresh): should create new database every time
    // TODO(madhavsuresh): these test are very brittle.
    dbname = "dbname=test";
  }

  void TearDown() override{};
};

TEST_F(health_lnk_schema_test, demographics) {
  std::string query_create("CREATE TABLE demographics (\n"
                           "       patient_id integer,\n"
                           "       birth_year integer,\n"
                           "       gender integer,\n"
                           "       race integer,\n"
                           "       ethnicity integer,\n"
                           "       insurance integer,\n"
                           "       zip integer);");
  query(query_create, dbname);
  table_t *t = get_table("SELECT * FROM demographics", dbname);
  ASSERT_EQ(t->schema.num_fields, 7);
  ASSERT_EQ(t->schema.fields[0].type, INT);
  ASSERT_STREQ(t->schema.fields[0].field_name, "patient_id");
  ASSERT_EQ(t->schema.fields[1].type, INT);
  ASSERT_STREQ(t->schema.fields[1].field_name, "birth_year");
  ASSERT_EQ(t->schema.fields[2].type, INT);
  ASSERT_STREQ(t->schema.fields[2].field_name, "gender");
  ASSERT_EQ(t->schema.fields[3].type, INT);
  ASSERT_STREQ(t->schema.fields[3].field_name, "race");
  ASSERT_EQ(t->schema.fields[4].type, INT);
  ASSERT_STREQ(t->schema.fields[4].field_name, "ethnicity");
  ASSERT_EQ(t->schema.fields[5].type, INT);
  ASSERT_STREQ(t->schema.fields[5].field_name, "insurance");
  ASSERT_EQ(t->schema.fields[6].type, INT);
  ASSERT_STREQ(t->schema.fields[6].field_name, "zip");
  std::string query_destroy("DROP TABLE demographics");
  query(query_destroy, dbname);
}

TEST_F(health_lnk_schema_test, remote_diagnoses) {
  std::string query_create("CREATE TABLE remote_diagnoses (\n"
                           "        patient_id integer NOT NULL,\n"
                           "    site integer NOT NULL,\n"
                           "    year integer NOT NULL,\n"
                           "    month integer NOT NULL,\n"
                           "    visit_no integer NOT NULL,\n"
                           "    type_ integer NOT NULL,\n"
                           "    encounter_id integer NOT NULL,\n"
                           "    diag_src character varying NOT NULL,\n"
                           "    icd9 character varying NOT NULL,\n"
                           "    primary_ integer NOT NULL,\n"
                           "    timestamp_ timestamp without time zone,\n"
                           "    clean_icd9 character varying,\n"
                           "    major_icd9 character varying\n"
                           ");");
  query(query_create, dbname);
  table_t *t = get_table("SELECT * FROM remote_diagnoses", dbname);
  ASSERT_EQ(t->schema.num_fields,13);
  ASSERT_EQ(t->schema.fields[0].type, INT);
  ASSERT_STREQ(t->schema.fields[0].field_name, "patient_id");
  ASSERT_EQ(t->schema.fields[1].type, INT);
  ASSERT_STREQ(t->schema.fields[1].field_name, "site");
  ASSERT_EQ(t->schema.fields[2].type, INT);
  ASSERT_STREQ(t->schema.fields[2].field_name, "year");
  ASSERT_EQ(t->schema.fields[3].type, INT);
  ASSERT_STREQ(t->schema.fields[3].field_name, "month");
  ASSERT_EQ(t->schema.fields[4].type, INT);
  ASSERT_STREQ(t->schema.fields[4].field_name, "visit_no");
  ASSERT_EQ(t->schema.fields[5].type, INT);
  ASSERT_STREQ(t->schema.fields[5].field_name, "type_");
  ASSERT_EQ(t->schema.fields[6].type, INT);
  ASSERT_STREQ(t->schema.fields[6].field_name, "encounter_id");
  ASSERT_EQ(t->schema.fields[7].type, FIXEDCHAR);
  ASSERT_STREQ(t->schema.fields[7].field_name, "diag_src");
  ASSERT_EQ(t->schema.fields[8].type, FIXEDCHAR);
  ASSERT_STREQ(t->schema.fields[8].field_name, "icd9");
  ASSERT_EQ(t->schema.fields[9].type, INT);
  ASSERT_STREQ(t->schema.fields[9].field_name, "primary_");
  ASSERT_EQ(t->schema.fields[10].type, TIMESTAMP);
  ASSERT_STREQ(t->schema.fields[10].field_name, "timestamp_");
  ASSERT_EQ(t->schema.fields[11].type, FIXEDCHAR);
  ASSERT_STREQ(t->schema.fields[11].field_name, "clean_icd9");
  ASSERT_EQ(t->schema.fields[12].type, FIXEDCHAR);
  ASSERT_STREQ(t->schema.fields[12].field_name, "major_icd9");

  std::string query_destroy("DROP TABLE remote_diagnoses");
  query(query_destroy, dbname);
}


TEST_F(health_lnk_schema_test, diagnoses) {
  std::string query_create("CREATE TABLE diagnoses (\n"
                           "    patient_id integer NOT NULL,\n"
                           "    site integer NOT NULL,\n"
                           "    year integer NOT NULL,\n"
                           "    month integer NOT NULL,\n"
                           "    visit_no integer NOT NULL,\n"
                           "    type_ integer NOT NULL,\n"
                           "    encounter_id integer NOT NULL,\n"
                           "    diag_src character varying NOT NULL,\n"
                           "    icd9 character varying NOT NULL,\n"
                           "    primary_ integer NOT NULL,\n"
                           "    timestamp_ timestamp without time zone,  \n"
                           "    clean_icd9 character varying,\n"
                           "    major_icd9 character varying\n"
                           ");");
  query(query_create, dbname);
  table_t *t = get_table("SELECT * FROM diagnoses", dbname);
  ASSERT_EQ(t->schema.num_fields,13);
  ASSERT_EQ(t->schema.fields[0].type, INT);
  ASSERT_STREQ(t->schema.fields[0].field_name, "patient_id");
  ASSERT_EQ(t->schema.fields[1].type, INT);
  ASSERT_STREQ(t->schema.fields[1].field_name, "site");
  ASSERT_EQ(t->schema.fields[2].type, INT);
  ASSERT_STREQ(t->schema.fields[2].field_name, "year");
  ASSERT_EQ(t->schema.fields[3].type, INT);
  ASSERT_STREQ(t->schema.fields[3].field_name, "month");
  ASSERT_EQ(t->schema.fields[4].type, INT);
  ASSERT_STREQ(t->schema.fields[4].field_name, "visit_no");
  ASSERT_EQ(t->schema.fields[5].type, INT);
  ASSERT_STREQ(t->schema.fields[5].field_name, "type_");
  ASSERT_EQ(t->schema.fields[6].type, INT);
  ASSERT_STREQ(t->schema.fields[6].field_name, "encounter_id");
  ASSERT_EQ(t->schema.fields[7].type, FIXEDCHAR);
  ASSERT_STREQ(t->schema.fields[7].field_name, "diag_src");
  ASSERT_EQ(t->schema.fields[8].type, FIXEDCHAR);
  ASSERT_STREQ(t->schema.fields[8].field_name, "icd9");
  ASSERT_EQ(t->schema.fields[9].type, INT);
  ASSERT_STREQ(t->schema.fields[9].field_name, "primary_");
  ASSERT_EQ(t->schema.fields[10].type, TIMESTAMP);
  ASSERT_STREQ(t->schema.fields[10].field_name, "timestamp_");
  ASSERT_EQ(t->schema.fields[11].type, FIXEDCHAR);
  ASSERT_STREQ(t->schema.fields[11].field_name, "clean_icd9");
  ASSERT_EQ(t->schema.fields[12].type, FIXEDCHAR);
  ASSERT_STREQ(t->schema.fields[12].field_name, "major_icd9");

  std::string query_destroy("DROP TABLE diagnoses");
  query(query_destroy, dbname);
}

TEST_F(health_lnk_schema_test, vitals) {
  std::string qr( "CREATE TABLE vitals (\n"
                 "        patient_id integer,\n"
                 "        height_timestamp timestamp,\n"
                 "        height_visit_no integer,\n"
                 "        height real,\n"
                 "        height_units character varying,\n"
                 "        weight_timestamp timestamp,\n"
                 "        weight_visit_no integer,\n"
                 "        weight real,\n"
                 "        weight_units character varying,\n"
                 "        bmi_timestamp timestamp,\n"
                 "        bmi_visit_no integer,\n"
                 "        bmi real,\n"
                 "        bmi_units character varying,\n"
                 "        pulse integer,\n"
                 "        systolic integer,\n"
                 "        diastolic integer ,\n"
                 "        bp_method character varying);");
  query(qr, dbname);
  table_t *t = get_table("SELECT * FROM vitals", dbname);
  ASSERT_STREQ(t->schema.fields[0].field_name,"patient_id");
  ASSERT_EQ(t->schema.fields[0].type, INT);
  ASSERT_STREQ(t->schema.fields[1].field_name,"height_timestamp");
  ASSERT_EQ(t->schema.fields[1].type, TIMESTAMP);
  ASSERT_STREQ(t->schema.fields[2].field_name,"height_visit_no");
  ASSERT_EQ(t->schema.fields[2].type, INT);
ASSERT_STREQ(t->schema.fields[3].field_name,"height");
ASSERT_EQ(t->schema.fields[3].type, DOUBLE);
    ASSERT_STREQ(t->schema.fields[4].field_name,"height_units");
    ASSERT_EQ(t->schema.fields[4].type, FIXEDCHAR);
    ASSERT_STREQ(t->schema.fields[5].field_name,"weight_timestamp");
    ASSERT_EQ(t->schema.fields[5].type, TIMESTAMP);
    ASSERT_STREQ(t->schema.fields[6].field_name,"weight_visit_no");
    ASSERT_EQ(t->schema.fields[6].type, INT);
    ASSERT_STREQ(t->schema.fields[7].field_name,"weight");
    ASSERT_EQ(t->schema.fields[7].type, DOUBLE);
    ASSERT_STREQ(t->schema.fields[8].field_name,"weight_units");
    ASSERT_EQ(t->schema.fields[8].type, FIXEDCHAR);
    ASSERT_STREQ(t->schema.fields[9].field_name,"bmi_timestamp");
    ASSERT_EQ(t->schema.fields[9].type, TIMESTAMP);
    ASSERT_STREQ(t->schema.fields[10].field_name,"bmi_visit_no");
    ASSERT_EQ(t->schema.fields[10].type, INT);
    ASSERT_STREQ(t->schema.fields[11].field_name,"bmi");
    ASSERT_EQ(t->schema.fields[11].type, DOUBLE);
    ASSERT_STREQ(t->schema.fields[12].field_name,"bmi_units");
    ASSERT_EQ(t->schema.fields[12].type, FIXEDCHAR);
    ASSERT_STREQ(t->schema.fields[13].field_name,"pulse");
    ASSERT_EQ(t->schema.fields[13].type, INT);
    ASSERT_STREQ(t->schema.fields[14].field_name,"systolic");
    ASSERT_EQ(t->schema.fields[14].type, INT);
    ASSERT_STREQ(t->schema.fields[15].field_name,"diastolic");
    ASSERT_EQ(t->schema.fields[16].type, FIXEDCHAR);
    ASSERT_STREQ(t->schema.fields[16].field_name,"bp_method");
  query("DROP TABLE vitals;", dbname);
}

TEST_F(health_lnk_schema_test, labs) {
    std::string query_create("CREATE TABLE labs (\n"
                             "        patient_id integer,\n"
                             "        timestamp_ timestamp,\n"
                             "        test_name character varying,\n"
                             "        value_ character varying,\n"
                             "        unit character varying,\n"
                             "        value_low real,\n"
                             "        value_high real);");
    query(query_create, dbname);
    table_t *t = get_table("SELECT * FROM labs", dbname);
    ASSERT_STREQ(t->schema.fields[0].field_name,"patient_id");
    ASSERT_EQ(t->schema.fields[0].type, INT);
    ASSERT_STREQ(t->schema.fields[1].field_name,"timestamp_");
    ASSERT_EQ(t->schema.fields[1].type, TIMESTAMP);
    ASSERT_STREQ(t->schema.fields[2].field_name,"test_name");
    ASSERT_EQ(t->schema.fields[2].type, FIXEDCHAR);
    ASSERT_STREQ(t->schema.fields[3].field_name,"value_");
    ASSERT_EQ(t->schema.fields[3].type, FIXEDCHAR);
    ASSERT_STREQ(t->schema.fields[4].field_name,"unit");
    ASSERT_EQ(t->schema.fields[4].type, FIXEDCHAR);
    ASSERT_STREQ(t->schema.fields[5].field_name,"value_low");
    ASSERT_EQ(t->schema.fields[5].type, DOUBLE);
    ASSERT_STREQ(t->schema.fields[6].field_name,"value_high");
    ASSERT_EQ(t->schema.fields[6].type, DOUBLE);
    query("DROP TABLE labs;", dbname);
}

TEST_F(health_lnk_schema_test, medications) {
    std::string query_create("CREATE TABLE medications (\n"
                             "    patient_id integer NOT NULL,\n"
                             "    site integer NOT NULL,\n"
                             "    year integer NOT NULL,\n"
                             "    month integer NOT NULL,\n"
                             "    medication character varying NOT NULL,\n"
                             "    dosage character varying NOT NULL,\n"
                             "    route character varying,\n"
                             "    timestamp_ timestamp without time zone);");
    query(query_create, dbname);
    table_t *t = get_table("SELECT * FROM medications", dbname);
    ASSERT_STREQ(t->schema.fields[0].field_name,"patient_id");
    ASSERT_EQ(t->schema.fields[0].type, INT);
    ASSERT_STREQ(t->schema.fields[1].field_name,"site");
    ASSERT_EQ(t->schema.fields[1].type, INT);
    ASSERT_STREQ(t->schema.fields[2].field_name,"year");
    ASSERT_EQ(t->schema.fields[2].type, INT);
    ASSERT_STREQ(t->schema.fields[3].field_name,"month");
    ASSERT_EQ(t->schema.fields[3].type, INT);
    ASSERT_STREQ(t->schema.fields[4].field_name,"medication");
    ASSERT_EQ(t->schema.fields[4].type, FIXEDCHAR);
    ASSERT_STREQ(t->schema.fields[5].field_name,"dosage");
    ASSERT_EQ(t->schema.fields[5].type, FIXEDCHAR);
    ASSERT_STREQ(t->schema.fields[6].field_name,"route");
    ASSERT_EQ(t->schema.fields[6].type, FIXEDCHAR);
    ASSERT_STREQ(t->schema.fields[7].field_name,"timestamp_");
    ASSERT_EQ(t->schema.fields[7].type, TIMESTAMP);
    query("DROP TABLE medications;", dbname);
}

TEST_F(health_lnk_schema_test, site) {
    std::string query_create("CREATE TABLE site (id integer);");
    query(query_create, dbname);
    table_t *t = get_table("SELECT * FROM site", dbname);
    ASSERT_STREQ(t->schema.fields[0].field_name,"id");
    ASSERT_EQ(t->schema.fields[0].type, INT);
    query("DROP TABLE site;", dbname);
}

TEST_F(health_lnk_schema_test, cdiff_cohort) {

    query("DROP table IF EXISTS diagnoses", dbname);
    std::string query_create("CREATE TABLE diagnoses (\n"
                             "    patient_id integer NOT NULL,\n"
                             "    site integer NOT NULL,\n"
                             "    year integer NOT NULL,\n"
                             "    month integer NOT NULL,\n"
                             "    visit_no integer NOT NULL,\n"
                             "    type_ integer NOT NULL,\n"
                             "    encounter_id integer NOT NULL,\n"
                             "    diag_src character varying NOT NULL,\n"
                             "    icd9 character varying NOT NULL,\n"
                             "    primary_ integer NOT NULL,\n"
                             "    timestamp_ timestamp without time zone,  \n"
                             "    clean_icd9 character varying,\n"
                             "    major_icd9 character varying\n"
                             ");");
    query(query_create, dbname);
    query("DROP TABLE IF EXISTS cdiff_cohort", dbname);
    query("CREATE TABLE cdiff_cohort AS (SELECT DISTINCT patient_id FROM diagnoses "
          "WHERE icd9 = '008.45' AND year = :test_year AND "
          "(site=:site1 OR site=:site2));", dbname);
}
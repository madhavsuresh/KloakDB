//
// Created by madhav on 11/29/18.
//
#include "distributed_aspirin_profile.h"
#include "logger/LoggerDefs.h"

void aspirin_profile_encrypt(HonestBrokerPrivate *p, std::string database,
                             std::string diagnoses_table,
                             std::string vitals_table,
                             std::string medications_table,
                             std::string demographics_table, std::string year,
                             bool sgx) {

  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }
  LOG(EXP7_ASP) << "STARTING ASPIRIN PROFILE ENCRYPTED";
  vector<std::future<vector<tableid_ptr>>> semi_joined_tables;
  semi_joined_tables.emplace_back(std::async(
          std::launch::async, &HonestBrokerPrivate::ClusterDBMSQuery, p,
          "dbname=" + database,
          "SELECT gender,race, sum(pulse) AS sum, count(*) AS count FROM  "
          "dem_ex_local de, hd_cohort_local di, vit_ex_local v, meds_ex_local m "
          "WHERE m.medication ILIKE '%aspirin%'  AND de.patient_id = di.patient_id "
          "AND di.patient_id = v.patient_id AND m.patient_id = di.patient_id GROUP "
          "BY gender, race;"));
  START_TIMER(aspirin_profile_full);
  START_TIMER(postgres_read);
  p->SetControlFlowColName("patient_id");
  unordered_map<table_name, to_gen_t> gen_in;
  auto diagnoses_scan = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT patient_id from " + diagnoses_table);
  auto vitals_scan = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT patient_id, pulse from " + vitals_table);
  auto meds_scan = p->ClusterDBMSQuery("dbname=" + database, "SELECT patient_id, medication from " + medications_table + " WHERE medication ILIKE '%ASPIRIN%'");

  auto demographics_single_scan = p->DBMSQuery(0,
                                        "dbname=" + database, "SELECT DISTINCT patient_id, gender, race from " +
                                                              demographics_table);
  vector<tableid_ptr> demographics_scan;
  demographics_scan.emplace_back(demographics_single_scan);

  END_AND_LOG_EXP7_ASP_STAT_TIMER(postgres_read, "encrypted");

  START_TIMER(repartition);
  auto diagnoses_repart = p->RepartitionJustHash(diagnoses_scan);
  auto vitals_repart = p->RepartitionJustHash(vitals_scan);
  auto meds_repart = p->RepartitionJustHash(meds_scan);
  auto demographics_repart = p->RepartitionJustHash(demographics_scan);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(repartition, "encrypted");

  // join def vitals-diagnoses
  START_TIMER(join_one);
  JoinDef jd_vd;
  jd_vd.set_l_col_name("patient_id");
  jd_vd.set_r_col_name("patient_id");
  jd_vd.set_project_len(2);
  // vitals-diagnoses-join project 1
  auto vdjp1 = jd_vd.add_project_list();
  vdjp1->set_side(JoinColID_RelationSide_LEFT);
  vdjp1->set_colname("patient_id");
  auto vdjp2 = jd_vd.add_project_list();
  vdjp2->set_side(JoinColID_RelationSide_LEFT);
  vdjp2->set_colname("pulse");
  auto to_join1 = zip_join_tables(vitals_repart, diagnoses_repart);
  auto out_vd_join = p->Join(to_join1, jd_vd, sgx );
  END_AND_LOG_EXP7_ASP_STAT_TIMER(join_one, "encrypted");

  // join def first join "plus medications"
  // join between output of vitals/diagnonses join and medications
  START_TIMER(join_two);
  JoinDef jd_pm2;
  jd_pm2.set_l_col_name("patient_id");
  jd_pm2.set_r_col_name("patient_id");
  jd_pm2.set_project_len(2);
  // plus medications project 1
  auto pmp1 = jd_pm2.add_project_list();
  pmp1->set_side(JoinColID_RelationSide_LEFT);
  pmp1->set_colname("patient_id");
  auto pmp2 = jd_pm2.add_project_list();
  pmp2->set_side(JoinColID_RelationSide_LEFT);
  pmp2->set_colname("pulse");

  auto to_join2 = zip_join_tables(out_vd_join, meds_repart);
  auto out_pm_join = p->Join(to_join2, jd_pm2, sgx);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(join_two, "encrypted");

  // p->FreeTables(meds_repart);

  // join def second join "plus demographics"
  START_TIMER(join_three);
  JoinDef jd_pd3;
  jd_pd3.set_l_col_name("patient_id");
  jd_pd3.set_r_col_name("patient_id");
  jd_pd3.set_project_len(3);
  auto pdp1 = jd_pd3.add_project_list();
  pdp1->set_side(JoinColID_RelationSide_RIGHT);
  pdp1->set_colname("pulse");
  auto pdp2 = jd_pd3.add_project_list();
  pdp2->set_side(JoinColID_RelationSide_LEFT);
  pdp2->set_colname("gender");
  auto pdp3 = jd_pd3.add_project_list();
  pdp3->set_side(JoinColID_RelationSide_LEFT);
  pdp3->set_colname("race");
  auto to_join3 = zip_join_tables(demographics_repart, out_pm_join);
  auto out_pd_join = p->Join(to_join3, jd_pd3, sgx);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(join_three, "encrypted");

  GroupByDef gbd;
  START_TIMER(aggregate);
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_col_name("pulse");
  gbd.add_gb_col_names("gender");
  gbd.add_gb_col_names("race");
  gbd.set_secure(false);
  LOG(INFO) << "GB LEN" << gbd.gb_col_names_size();
  vector<string> cfids;
  cfids.emplace_back("gender");
  cfids.emplace_back("race");
 auto final_avg = p->Aggregate(out_pd_join, gbd, sgx);
  vector<tableid_ptr> semi_joined_out;
  for (auto &s: semi_joined_tables) {
    semi_joined_out = s.get();
  }
  END_AND_LOG_EXP7_ASP_STAT_TIMER(aggregate, "encrypted");
  END_AND_LOG_EXP7_ASP_STAT_TIMER(aspirin_profile_full, "encrypted");
  LOG(EXP7_ASP) << "ENDING ASPIRIN PROFILE ENCRYPTED";
}

void aspirin_profile_obli(HonestBrokerPrivate *p, std::string database,
                          std::string diagnoses_table, std::string vitals_table,
                          std::string medications_table,
                          std::string demographics_table, bool sgx) {

  START_TIMER(aspirin_profile_full);
  START_TIMER(postgres_read);
  p->SetControlFlowColName("patient_id");
  unordered_map<table_name, to_gen_t> gen_in;
  auto diagnoses_scan = p->ClusterDBMSQuery(
          "dbname=" + database, "SELECT patient_id from " + diagnoses_table);
  p->MakeObli(diagnoses_scan, "patient_id");
  auto vitals_scan = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT patient_id, pulse from " + vitals_table);
  p->MakeObli(vitals_scan, "patient_id");

  auto meds_scan = p->ClusterDBMSQuery("dbname=" + database,
                                       "SELECT patient_id, medication from " +
                                           medications_table);
  p->MakeObli(meds_scan, "patient_id");

  auto demographics_scan = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT DISTINCT patient_id, gender, race from " + demographics_table);
  p->MakeObli(demographics_scan, "patient_id");

  END_AND_LOG_EXP7_ASP_STAT_TIMER(postgres_read, "full");

  START_TIMER(repartition);
  auto diagnoses_repart = p->RepartitionJustHash(diagnoses_scan);
  auto vitals_repart = p->RepartitionJustHash(vitals_scan);
  auto meds_repart = p->RepartitionJustHash(meds_scan);
  auto demographics_repart = p->RepartitionJustHash(demographics_scan);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(repartition, "full");

  // join def vitals-diagnoses
  START_TIMER(join_one);
  JoinDef jd_vd;
  jd_vd.set_l_col_name("patient_id");
  jd_vd.set_r_col_name("patient_id");
  jd_vd.set_project_len(1);
  // vitals-diagnoses-join project 1
  auto vdjp1 = jd_vd.add_project_list();
  vdjp1->set_side(JoinColID_RelationSide_LEFT);
  vdjp1->set_colname("patient_id");
  //auto vdjp2 = jd_vd.add_project_list();
  //vdjp2->set_side(JoinColID_RelationSide_LEFT);
  //vdjp2->set_colname("pulse");
  auto to_join1 = zip_join_tables(vitals_repart, diagnoses_repart);
  auto out_vd_join = p->Join(to_join1, jd_vd, sgx );
  END_AND_LOG_EXP7_ASP_STAT_TIMER(join_one, "full");

  // join def first join "plus medications"
  // join between output of vitals/diagnonses join and medications
  START_TIMER(join_two);
  JoinDef jd_pm2;
  jd_pm2.set_l_col_name("patient_id");
  jd_pm2.set_r_col_name("patient_id");
  jd_pm2.set_project_len(1);
  // plus medications project 1
  auto pmp1 = jd_pm2.add_project_list();
  pmp1->set_side(JoinColID_RelationSide_LEFT);
  pmp1->set_colname("patient_id");
  //auto pmp2 = jd_pm2.add_project_list();
  //pmp2->set_side(JoinColID_RelationSide_LEFT);
  //pmp2->set_colname("pulse");
  auto to_join2 = zip_join_tables(demographics_repart, out_vd_join);
  auto out_pm_join = p->Join(to_join2, jd_pm2, sgx);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(join_two, "full");

  // p->FreeTables(meds_repart);

  /*
  // join def second join "plus demographics"
  START_TIMER(join_three);
  JoinDef jd_pd3;
  jd_pd3.set_l_col_name("patient_id");
  jd_pd3.set_r_col_name("patient_id");
  jd_pd3.set_project_len(3);
  auto pdp1 = jd_pd3.add_project_list();
  pdp1->set_side(JoinColID_RelationSide_LEFT);
  pdp1->set_colname("pulse");
  auto pdp2 = jd_pd3.add_project_list();
  pdp2->set_side(JoinColID_RelationSide_RIGHT);
  pdp2->set_colname("gender");
  auto pdp3 = jd_pd3.add_project_list();
  pdp3->set_side(JoinColID_RelationSide_RIGHT);
  pdp3->set_colname("race");
  auto to_join3 = zip_join_tables(out_pm_join, demographics_repart);
  auto out_pd_join = p->Join(to_join3, jd_pd3, sgx);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(join_three, "full");

  START_TIMER(repartition_two);
  vector<string> cfnames;
  cfnames.emplace_back("gender");
  cfnames.emplace_back("race");
  p->ResetControlFlowCols();
  p->SetControlFlowColNames(cfnames);
  auto out_repart_2 = p->Repartition(out_pd_join);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(repartition_two, "full");

  GroupByDef gbd;
  START_TIMER(aggregate);
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_col_name("pulse");
  gbd.add_gb_col_names("gender");
  gbd.add_gb_col_names("race");
  LOG(INFO) << "GB LEN" << gbd.gb_col_names_size();
  vector<string> cfids;
  cfids.emplace_back("gender");
  cfids.emplace_back("race");
  auto final_avg = p->Aggregate(out_repart_2, gbd, sgx);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(aggregate, "full");
   */
  END_AND_LOG_EXP7_ASP_STAT_TIMER(aspirin_profile_full, "full");

}

void aspirin_profile_gen(HonestBrokerPrivate *p, std::string database,
                     std::string diagnoses_table, std::string vitals_table,
                     std::string medications_table,
                     std::string demographics_table, std::string year, bool sgx,
                     int gen_level) {

  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }

  vector<std::future<vector<tableid_ptr>>> semi_joined_tables;
  semi_joined_tables.emplace_back(std::async(
      std::launch::async, &HonestBrokerPrivate::ClusterDBMSQuery, p,
      "dbname=" + database,
      "SELECT gender,race, sum(pulse) AS sum, count(*) AS count FROM  "
      "dem_ex_local de, hd_cohort_local di, vit_ex_local v, meds_ex_local m "
      "WHERE m.medication ILIKE '%aspirin%'  AND de.patient_id = di.patient_id "
      "AND di.patient_id = v.patient_id AND m.patient_id = di.patient_id GROUP "
      "BY gender, race;"));

  START_TIMER(aspirin_profile_full);
  START_TIMER(postgres_read);
  p->SetControlFlowColName("patient_id");
  unordered_map<table_name, to_gen_t> gen_in;
  auto diagnoses_scan = p->ClusterDBMSQuery(
          "dbname=" + database, "SELECT patient_id from " + diagnoses_table);
  to_gen_t diag_gen;
  diag_gen.column = "patient_id";
  diag_gen.dbname = "healthlnk";
  diag_gen.scan_tables.insert(diag_gen.scan_tables.end(),
                              diagnoses_scan.begin(), diagnoses_scan.end());
  gen_in[diagnoses_table] = diag_gen;
  auto vitals_scan = p->ClusterDBMSQuery(
          "dbname=" + database, "SELECT patient_id, pulse from " + vitals_table);
  to_gen_t vitals_gen;
  vitals_gen.column = "patient_id";
  vitals_gen.dbname = "healthlnk";
  vitals_gen.scan_tables.insert(vitals_gen.scan_tables.end(),
                                vitals_scan.begin(), vitals_scan.end());
  gen_in[vitals_table] = vitals_gen;
  auto meds_scan = p->ClusterDBMSQuery(
          "dbname=" + database, "SELECT patient_id, medication from " +
                                medications_table);

  to_gen_t meds_gen;
  meds_gen.column = "patient_id";
  meds_gen.dbname = "healthlnk";
  meds_gen.scan_tables.insert(meds_gen.scan_tables.end(), meds_scan.begin(),
                              meds_scan.end());
  gen_in[medications_table] = meds_gen;

  auto demographics_scan = p->DBMSQuery(0,
                                        "dbname=" + database, "SELECT DISTINCT patient_id, gender, race from " +
                                                              demographics_table);

  to_gen_t dem_gen;
  dem_gen.column = "patient_id";
  dem_gen.dbname = "healthlnk";
  dem_gen.scan_tables.emplace_back(demographics_scan);
  gen_in[demographics_table] = dem_gen;

  END_AND_LOG_EXP7_ASP_STAT_TIMER(postgres_read, "k5");
  START_TIMER(generalize);
  auto gen_zipped_map = p->Generalize(gen_in, 5);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(generalize, "k5");

  START_TIMER(medications_filter);
  ::vaultdb::Expr expr_med;
  expr_med.set_colname("medication");
  expr_med.set_type(Expr_ExprType_LIKE_EXPR);
  auto fieldMed = expr_med.mutable_desc();
  fieldMed->set_field_type(FieldDesc_FieldType_FIXEDCHAR);
  expr_med.set_charfield("ASPIRIN");
  auto filtered_meds =
          p->Filter(gen_zipped_map[medications_table], expr_med, false);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(medications_filter, "k5");

  START_TIMER(repartition);
  auto diagnoses_repart = p->Repartition(gen_zipped_map[diagnoses_table]);
  auto vitals_repart = p->Repartition(gen_zipped_map[vitals_table]);
  auto meds_repart = p->Repartition(filtered_meds);
  auto demographics_repart = p->Repartition(gen_zipped_map[demographics_table]);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(repartition, "k5");

  START_TIMER(join_one);
  JoinDef jd_vd;
  jd_vd.set_l_col_name("patient_id");
  jd_vd.set_r_col_name("patient_id");
  jd_vd.set_project_len(2);
  // vitals-diagnoses-join project 1
  auto vdjp1 = jd_vd.add_project_list();
  vdjp1->set_side(JoinColID_RelationSide_LEFT);
  vdjp1->set_colname("patient_id");
  auto vdjp2 = jd_vd.add_project_list();
  vdjp2->set_side(JoinColID_RelationSide_LEFT);
  vdjp2->set_colname("pulse");
  auto to_join1 = zip_join_tables(vitals_repart, diagnoses_repart);
  auto out_vd_join = p->Join(to_join1, jd_vd, sgx);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(join_one, "k5");
  START_TIMER(join_two);
  JoinDef jd_pm2;
  jd_pm2.set_l_col_name("patient_id");
  jd_pm2.set_r_col_name("patient_id");
  jd_pm2.set_project_len(2);
  // plus medications project 1
  auto pmp1 = jd_pm2.add_project_list();
  pmp1->set_side(JoinColID_RelationSide_LEFT);
  pmp1->set_colname("patient_id");
  auto pmp2 = jd_pm2.add_project_list();
  pmp2->set_side(JoinColID_RelationSide_LEFT);
  pmp2->set_colname("pulse");

  auto to_join2 = zip_join_tables(out_vd_join, meds_repart);
  auto out_pm_join = p->Join(to_join2, jd_pm2, sgx);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(join_two, "k5");
  START_TIMER(join_three);
  JoinDef jd_pd3;
  jd_pd3.set_l_col_name("patient_id");
  jd_pd3.set_r_col_name("patient_id");
  jd_pd3.set_project_len(4);
  auto pdp1 = jd_pd3.add_project_list();
  pdp1->set_side(JoinColID_RelationSide_RIGHT);
  pdp1->set_colname("pulse");
  auto pdp2 = jd_pd3.add_project_list();
  pdp2->set_side(JoinColID_RelationSide_LEFT);
  pdp2->set_colname("gender");
  auto pdp3 = jd_pd3.add_project_list();
  pdp3->set_side(JoinColID_RelationSide_LEFT);
  pdp3->set_colname("race");
  auto pdp4 = jd_pd3.add_project_list();
  pdp4->set_side(JoinColID_RelationSide_LEFT);
  pdp4->set_colname("patient_id");
  auto to_join3 = zip_join_tables(demographics_repart,out_pm_join);
  LOG(EXP7_ASP) << "Join Input Table 3:";
  auto out_pd_join = p->Join(to_join3, jd_pd3, sgx);
  END_AND_LOG_EXP7_ASP_STAT_TIMER(join_three, "k5");
  LOG(EXP7_ASP) << "End Join Input Table 3:";

  GroupByDef gbd;
  START_TIMER(aggregate);
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_secure(true);
  gbd.set_col_name("pulse");
  gbd.add_gb_col_names("gender");
  gbd.add_gb_col_names("race");
  gbd.set_kanon_col_name("patient_id");
  LOG(INFO) << "GB LEN" << gbd.gb_col_names_size();
  vector<string> cfids;
  cfids.emplace_back("gender");
  cfids.emplace_back("race");
  auto final_avg = p->Aggregate(out_pd_join, gbd, sgx);
  vector<tableid_ptr> semi_joined_out;
  for (auto &s: semi_joined_tables) {
    semi_joined_out = s.get();
  }
  END_AND_LOG_EXP7_ASP_STAT_TIMER(aggregate, "k5");
  END_AND_LOG_EXP7_ASP_STAT_TIMER(aspirin_profile_full, "k5");
  LOG(EXP7_ASP) << "ENDING ASPIRIN PROFILE ENCRYPTED";
  END_AND_LOG_EXP7_ASP_STAT_TIMER(aspirin_profile_full, "k5");
}


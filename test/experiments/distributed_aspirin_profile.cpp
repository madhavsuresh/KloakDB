//
// Created by madhav on 11/29/18.
//
#include "distributed_aspirin_profile.h"

void aspirin_profile(HonestBrokerPrivate *p, std::string database,
                     std::string diagnoses_table, std::string vitals_table,
                     std::string medications_table,
                     std::string demographics_table, std::string year,
                     bool sgx) {

  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }
  p->SetControlFlowColName("patient_id");
  auto diagnoses_scan = p->ClusterDBMSQuery("dbname=" + database,
                                            "SELECT icd9, patient_id from " +
                                                diagnoses_table + year_append);
  auto vitals_scan = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT patient_id, pulse from " + vitals_table +
                                year_append + " AND pulse IS NOT NULL");
  auto diagnoses_repart = p->Repartition(diagnoses_scan);
  auto vitals_repart = p->Repartition(vitals_scan);

  // join def vitals-diagnoses
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
  auto out_vd_join = p->Join(to_join1, jd_vd, false /* in_sgx */);
  /*
  p->FreeTables(vitals_repart);
  p->FreeTables(diagnoses_repart);
   */

  // join def first join "plus medications"
  // join between output of vitals/diagnonses join and medications
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
  auto meds_scan = p->ClusterDBMSQuery(
          "dbname=" + database, "SELECT patient_id, medication from " +
  medications_table);

  auto meds_repart = p->Repartition(meds_scan);
  auto to_join2 = zip_join_tables(out_vd_join, meds_repart);
  auto out_pm_join = p->Join(to_join2, jd_pm2, false );

  //p->FreeTables(meds_repart);


  auto demographics_scan = p->ClusterDBMSQuery(
          "dbname=" + database, "SELECT DISTINCT patient_id, gender, race from "
  + demographics_table); auto demographics_repart =
  p->Repartition(demographics_scan);
  // join def second join "plus demographics"
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
  auto out_pd_join = p->Join(to_join3, jd_pd3, false);

  /*
  GroupByDef gbd;
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_col_name("pulse");
  gbd.add_gb_col_names("gender");
  gbd.add_gb_col_names("race");
  vector<string> cfids;
  cfids.emplace_back("gender");
  cfids.emplace_back("race");
  p->SetControlFlowColName("patient_id");
  auto final_avg = p->Aggregate(out_pd_join, gbd, false);
  */
}

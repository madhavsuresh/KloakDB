//
// Created by madhav on 12/1/18.
//
#include "distributed_dosage.h"
#include <VaultDB.h>

void dosage_encrypted(HonestBrokerPrivate *p, std::string dbname,
                      std::string diag, std::string meds, std::string year) {
  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }

  LOG(EXP7_DOS) << "STARTING DOSAGE STUDY ENCRYPTED";
  START_TIMER(dosage_study_encrypted);
  auto diag_scan = p->ClusterDBMSQuery("dbname=" + dbname,
                                       "SELECT * from " + diag + year_append + " AND icd9 LIKE '997%'");
  auto med_scan = p->ClusterDBMSQuery("dbname=" + dbname,
                                      "SELECT * from " + meds + year_append + " AND medication LIKE 'ASPIRIN%' AND dosage = '325 MG'");
  // auto to_join = zip_join_tables(diag_scan, med_scan);
  p->SetControlFlowColName("patient_id");
  auto diag_repart = p->RepartitionJustHash(diag_scan);
  auto med_repart = p->RepartitionJustHash(med_scan);
  auto to_join = zip_join_tables(diag_repart, med_repart);


  JoinDef jd;
  jd.set_l_col_name("patient_id");
  jd.set_r_col_name("patient_id");
  jd.set_project_len(1);
  auto join_project = jd.add_project_list();
  join_project->set_colname("patient_id");
  join_project->set_col_no(JoinColID_RelationSide_LEFT);

  auto output_join = p->Join(to_join, jd, true /* in_sgx */);
  END_AND_LOG_EXP7_DOS_STAT_TIMER(dosage_study_encrypted, "debug");
  LOG(EXP7_DOS) << "ENDING DOSAGE STUDY ENCRYPTED";
}

void dosage_obliv(HonestBrokerPrivate *p, std::string dbname,
                      std::string diag, std::string meds, std::string year) {
  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }

  auto diag_scan = p->ClusterDBMSQuery("dbname=" + dbname,
                                       "SELECT * from " + diag + year_append + " AND icd9 LIKE '997%'");
  auto med_scan = p->ClusterDBMSQuery("dbname=" + dbname,
                                      "SELECT * from " + meds + year_append + " AND medication LIKE 'ASPIRIN%' AND dosage = '325 MG'");
  // auto to_join = zip_join_tables(diag_scan, med_scan);
  p->SetControlFlowColName("patient_id");
  auto obli_diag = p->MakeObli(diag_scan, "patient_id");
  auto obli_med = p->MakeObli(med_scan, "patient_id");
  auto diag_repart = p->RepartitionJustHash(obli_diag);
  auto med_repart = p->RepartitionJustHash(obli_med);

  auto to_join = zip_join_tables(diag_repart, med_repart);


  JoinDef jd;
  jd.set_l_col_name("patient_id");
  jd.set_r_col_name("patient_id");
  jd.set_project_len(1);
  auto join_project = jd.add_project_list();
  join_project->set_colname("patient_id");
  join_project->set_col_no(JoinColID_RelationSide_LEFT);

  auto output_join = p->Join(to_join, jd, true /* in_sgx */);
}


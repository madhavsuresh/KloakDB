//
// Created by madhav on 12/1/18.
//
#include "distributed_dosage.h"
#include <VaultDB.h>
#include <gflags/gflags.h>

DEFINE_int32(num_pids, 100, "number of pids in query");

void dosage_encrypted(HonestBrokerPrivate *p, std::string dbname,
                      std::string diag, std::string meds, std::string year) {
  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }

  LOG(EXP7_DOS) << "STARTING DOSAGE STUDY ENCRYPTED";
  START_TIMER(dosage_study_encrypted);
  auto diag_scan =
      p->ClusterDBMSQuery("dbname=" + dbname,
                          "SELECT * from " + diag); // + year_append +
                                                    //" AND icd9 LIKE '997%'");
  auto med_scan = p->ClusterDBMSQuery(
      "dbname=" + dbname,
      "SELECT * from " + meds + year_append +
          " AND medication LIKE 'ASPIRIN%' AND dosage = '325 MG'");
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
  END_AND_LOG_EXP7_DOS_STAT_TIMER(dosage_study_encrypted, FLAGS_num_pids);
  LOG(EXP7_DOS) << "ENDING DOSAGE STUDY ENCRYPTED";
}

void dosage_obliv(HonestBrokerPrivate *p, std::string dbname, std::string diag,
                  std::string meds, std::string year) {
  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }

  LOG(EXP7_DOS) << "STARTING DOSAGE STUDY ENCRYPTED";
  START_TIMER(dosage_study_obli);
  auto diag_scan =
      p->ClusterDBMSQuery("dbname=" + dbname, "SELECT patient_id from " + diag);
  auto med_scan = p->ClusterDBMSQuery(
      "dbname=" + dbname,
      "SELECT medication, dosage, patient_id from " + meds + year_append);
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
  END_AND_LOG_EXP7_DOS_STAT_TIMER(dosage_study_obli, FLAGS_num_pids);
  LOG(EXP7_DOS) << "ENDING DOSAGE STUDY ENCRYPTED";
}

void dosage_k(HonestBrokerPrivate *p, std::string dbname, std::string diag,
              std::string meds, std::string year, int gen_level) {
  unordered_map<table_name, to_gen_t> gen_in;
  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }

  LOG(EXP7_DOS) << "STARTING DOSAGE STUDY K-ANONYMOUS";
  vector<std::future<vector<tableid_ptr>>> semi_joined_tables;
  semi_joined_tables.emplace_back(std::async(
      std::launch::async, &HonestBrokerPrivate::ClusterDBMSQuery, p,
      "dbname=" + dbname,
      "SELECT distinct(m.patient_id) from meds_ex_local m, hd_cohort_local h "
      "where h.patient_id=m.patient_id and m.medication iLIKE '%ASPIRIN%' and "
      "m.dosage ILIKE '%325MG%'"));
  START_TIMER(dosage_study_k);
  auto diag_scan = p->ClusterDBMSQuery("dbname=" + dbname,
                                       "SELECT patient_id from " +
                                           diag); // + year_append + "AND icd9
                                                  // LIKE '997%'"); //;+ " AND
                                                  // icd9 LIKE '997%'");
  auto med_scan =
      p->ClusterDBMSQuery("dbname=" + dbname,
                          "SELECT medication, dosage, patient_id from " + meds +
                              year_append); // AND medication LIKE 'ASPIRIN%'
                                            // AND dosage = '325 MG'");
  // auto to_join = zip_join_tables(diag_scan, med_scan);
  ::vaultdb::Expr expr_med;
  expr_med.set_colname("medication");
  expr_med.set_type(Expr_ExprType_LIKE_EXPR);
  auto fieldMed = expr_med.mutable_desc();
  fieldMed->set_field_type(FieldDesc_FieldType_FIXEDCHAR);
  expr_med.set_charfield("ASPIRIN");

  ::vaultdb::Expr expr_dosage;
  expr_dosage.set_colname("dosage");
  expr_dosage.set_type(Expr_ExprType_LIKE_EXPR);
  auto fieldDosage = expr_dosage.mutable_desc();
  fieldDosage->set_field_type(FieldDesc_FieldType_FIXEDCHAR);
  expr_dosage.set_charfield("325 MG");

  to_gen_t meds_gen;
  meds_gen.column = "patient_id";
  meds_gen.dbname = "healthlnk";
  meds_gen.scan_tables.insert(meds_gen.scan_tables.end(), med_scan.begin(),
                              med_scan.end());
  gen_in[meds] = meds_gen;

  to_gen_t diag_gen;
  diag_gen.column = "patient_id";
  diag_gen.dbname = "healthlnk";
  diag_gen.scan_tables.insert(diag_gen.scan_tables.end(), diag_scan.begin(),
                              diag_scan.end());
  gen_in[diag] = diag_gen;

  p->SetControlFlowColName("patient_id");
  START_TIMER(gen_dos_timer);
  auto gen_zipped_map = p->Generalize(gen_in, gen_level);
  END_AND_LOG_EXP7_DOS_STAT_TIMER(gen_dos_timer, FLAGS_num_pids);
  LOG(EXP7_DOS) << "MEDICATIONS FILTER";
  auto filtered_meds = p->Filter(gen_zipped_map[meds], expr_med, false);
  LOG(EXP7_DOS) << "DOSAGE FILTER";
  auto filtered_dosage = p->Filter(filtered_meds, expr_dosage, false);
  LOG(EXP7_DOS) << "DIAG FILTER";
  // auto filtered_diag =  p->Filter(gen_zipped_map["diagnoses"], expr_icd9,
  // false);
  auto med_repart = p->RepartitionJustHash(filtered_dosage);
  auto diag_repart = p->RepartitionJustHash(gen_zipped_map[diag]);
  auto to_join = zip_join_tables(diag_repart, med_repart);

  JoinDef jd;
  jd.set_l_col_name("patient_id");
  jd.set_r_col_name("patient_id");
  jd.set_project_len(1);
  auto join_project = jd.add_project_list();
  join_project->set_colname("patient_id");
  join_project->set_col_no(JoinColID_RelationSide_LEFT);

  auto output_join = p->Join(to_join, jd, true /* in_sgx */);
  vector<tableid_ptr> semi_joined_out;
  for (auto &s : semi_joined_tables) {
    semi_joined_out = s.get();
  }
  END_AND_LOG_EXP7_DOS_STAT_TIMER(dosage_study_k, FLAGS_num_pids);
  LOG(EXP7_DOS) << "ENDING DOSAGE STUDY K-ANON";
}

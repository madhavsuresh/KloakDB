//
// Created by madhav on 11/30/18.
//
#include "distributed_comorb.h"


void comorbidity_encrypted(HonestBrokerPrivate *p, std::string dbname, std::string year) {
  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }
  LOG(EXP7_COM) << "STARTING COMORBIDITY QUERY";
  START_TIMER(comorbidity_encrypted_full);
  p->SetControlFlowColName("major_icd9");
  auto cdiff_cohort_scan = p->ClusterDBMSQuery(
          "dbname=" + dbname, "SELECT major_icd9 from cdiff_cohort_diagnoses" + year_append);
  auto cdiff_repart = p->RepartitionJustHash(cdiff_cohort_scan);
  GroupByDef gbd;
  gbd.set_col_name("major_icd9");
  gbd.set_secure(false);
  gbd.add_gb_col_names("major_icd9");
  gbd.set_kanon_col_name("major_icd9");
  gbd.set_type(GroupByDef_GroupByType_COUNT);
  auto agg_out = p->Aggregate(cdiff_repart, gbd, true);

  p->SetControlFlowColName("count");
  //auto cnt_repartition = p->Repartition(agg_out);
  SortDef sort;
  sort.set_colname("count");
  sort.set_ascending(false);
  auto sorted = p->Sort(agg_out, sort, true);
  END_AND_LOG_EXP7_COM_STAT_TIMER(comorbidity_encrypted_full, "insgx");
  LOG(EXP7_COM) << "ENDING COMORBIDITY QUERY";
}

void comorbidity_oliv(HonestBrokerPrivate *p, std::string dbname, std::string year) {
  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }
  LOG(EXP7_COM) << "STARTING COMORBIDITY QUERY OBLIV";
  START_TIMER(comorbidity_obliv_full);
  p->SetControlFlowColName("major_icd9");
  auto cdiff_cohort_scan = p->ClusterDBMSQuery(
          "dbname=" + dbname, "SELECT major_icd9 from cdiff_cohort_diagnoses" + year_append);
  auto obliv = p->MakeObli(cdiff_cohort_scan, "major_icd9");
  auto cdiff_repart = p->RepartitionJustHash(obliv);
  GroupByDef gbd;
  gbd.set_col_name("major_icd9");
  gbd.add_gb_col_names("major_icd9");
  gbd.set_kanon_col_name("major_icd9");
  gbd.set_type(GroupByDef_GroupByType_COUNT);
  gbd.set_secure(true);
  auto agg_out = p->Aggregate(cdiff_repart, gbd, true);

  p->SetControlFlowColName("count");
  //auto cnt_repartition = p->Repartition(agg_out);
  SortDef sort;
  sort.set_colname("count");
  sort.set_ascending(false);
  auto sorted = p->Sort(agg_out, sort, true);
  END_AND_LOG_EXP7_COM_STAT_TIMER(comorbidity_obliv_full, "insgx");
  LOG(EXP7_COM) << "ENDING COMORBIDITY QUERY";
}

void comorbidity_keq5(HonestBrokerPrivate *p, std::string dbname, std::string year) {
  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }
  START_TIMER(comorbidity_kanon_full);
  p->SetControlFlowColName("major_icd9");
  p->SetControlFlowNotAnon(false);
  auto cdiff_cohort_scan = p->ClusterDBMSQuery(
          "dbname=" + dbname, "SELECT major_icd9 from cdiff_cohort_diagnoses" + year_append);
  unordered_map<string, to_gen_t> gen_in;
  to_gen_t tg;
  tg.column = "major_icd9";
  tg.dbname = "healthlnk";
  tg.scan_tables.insert(tg.scan_tables.end(), cdiff_cohort_scan.begin(),
                        cdiff_cohort_scan.end());
  gen_in["cdiff_cohort_diagnoses"] = tg;
  START_TIMER(generalize);
  auto gen_out = p->Generalize(gen_in, 5);
  END_AND_LOG_EXEC_TIMER(generalize);
  START_TIMER(repartition);
  auto cdiff_cohort_repart = p->Repartition(gen_out["cdiff_cohort_diagnoses"]);
  END_AND_LOG_EXEC_TIMER(repartition);

  GroupByDef gbd;
  gbd.set_secure(true);
  gbd.set_col_name("major_icd9");
  gbd.add_gb_col_names("major_icd9");
  gbd.set_kanon_col_name("major_icd9");
  gbd.set_type(GroupByDef_GroupByType_COUNT);
  START_TIMER(aggregate);
  auto agg_out = p->Aggregate(cdiff_cohort_repart, gbd, true);
  END_AND_LOG_EXEC_TIMER(aggregate);

  p->SetControlFlowColName("count");
  START_TIMER(repartition2);
  auto cnt_repartition = p->Repartition(agg_out);
  END_AND_LOG_EXEC_TIMER(repartition2);
  SortDef sort;
  sort.set_colname("count");
  sort.set_ascending(false);
  START_TIMER(sort);
  auto sorted = p->Sort(cnt_repartition, sort, true);
  END_AND_LOG_EXEC_TIMER(sort);
  END_AND_LOG_EXP7_COM_STAT_TIMER(comorbidity_kanon_full, "insgx");
}

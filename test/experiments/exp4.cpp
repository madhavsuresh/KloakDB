//
// Created by madhav on 11/27/18.
//

#include <logger/LoggerDefs.h>
#include <rpc/HonestBrokerPrivate.h>

void comorbidity(HonestBrokerPrivate *p, std::string dbname, int gen_level) {
  auto cdiff_cohort_scan = p->ClusterDBMSQuery(
      "dbname=" + dbname, "SELECT * from cdiff_cohort_diagnoses");
  unordered_map<string, to_gen_t> gen_in;
  to_gen_t tg;
  tg.column = "major_icd9";
  tg.dbname = "vaultdb_";
  gen_in["cdiff_cohort_diagnoses"] = tg;
  tg.scan_tables.insert(tg.scan_tables.end(), cdiff_cohort_scan.begin(),
                        cdiff_cohort_scan.end());
  p->SetControlFlowColName("major_icd9");
  START_TIMER(generalize);
   auto gen_out = p->Generalize("cdiff_cohort_diagnoses","major_icd9", "vaultdb_", cdiff_cohort_scan, 5);
  END_AND_LOG_EXEC_TIMER(generalize);
  START_TIMER(repartition);
  auto cdiff_cohort_repart = p->Repartition(gen_out);
  END_AND_LOG_EXEC_TIMER(repartition);

  GroupByDef gbd;
  gbd.set_col_name("major_icd9");
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
}

void exp4(HonestBrokerPrivate *p) { comorbidity(p, "vaultdb_", 5); }
//
// Created by madhav on 12/23/18.
//

#include "dist_gen_test.h"
#include <VaultDB.h>
#include <logger/LoggerDefs.h>

void gen_test_rand_table(HonestBrokerPrivate *p, std::string dbname,
                         std::string table1, std::string table2, int k,
                         bool in_sgx, std::string tag) {
  START_TIMER(end_to_end);
  std::string column = "b";
  auto gen_scan1 =
      p->ClusterDBMSQuery("dbname=" + dbname, "SELECT * FROM " + table1);
  auto gen_scan2 =
      p->ClusterDBMSQuery("dbname=" + dbname, "SELECT * FROM " + table2);

  unordered_map<table_name, to_gen_t> gen_in;
  to_gen_t scan1;
  scan1.column = column;
  scan1.dbname = dbname;
  scan1.scan_tables.insert(scan1.scan_tables.end(), gen_scan1.begin(),
                           gen_scan1.end());
  gen_in[table1] = scan1;

  to_gen_t scan2;
  scan2.column = column;
  scan2.dbname = dbname;
  scan2.scan_tables.insert(scan2.scan_tables.end(), gen_scan2.begin(),
                           gen_scan2.end());
  gen_in[table2] = scan2;
  p->SetControlFlowColName(column);
  START_TIMER(generalize);
  auto gen_zipped_map = p->Generalize(gen_in, k);
  END_AND_LOG_EXP8_GEN_STAT_TIMER(generalize, tag);

  START_TIMER(repartition);
  auto table1_repart = p->Repartition(gen_zipped_map[table1]);
  auto table2_repart = p->Repartition(gen_zipped_map[table2]);
  END_AND_LOG_EXP8_GEN_STAT_TIMER(repartition, tag);
  JoinDef jd;
  jd.set_l_col_name(column);
  jd.set_r_col_name(column);
  jd.set_project_len(1);
  auto join_project = jd.add_project_list();
  join_project->set_colname(column);
  join_project->set_col_no(JoinColID_RelationSide_LEFT);

  auto to_join = zip_join_tables(table1_repart, table2_repart);
  START_TIMER(join);
  auto output_join = p->Join(to_join, jd, in_sgx);
  END_AND_LOG_EXP8_GEN_STAT_TIMER(join, tag);
  END_AND_LOG_EXP8_GEN_STAT_TIMER(end_to_end, tag);
}
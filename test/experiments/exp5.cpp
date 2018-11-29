//
// Created by madhav on 11/26/18.
//

#include "VaultDB.h"
#include <logger/LoggerDefs.h>
#include <rpc/HonestBrokerPrivate.h>

void exp5(HonestBrokerPrivate *p, int gen_level, bool sgx) {
  auto scan = p->ClusterDBMSQuery("dbname=vaultdb_",
                                  "SELECT * FROM left_deep_joins_512");
  unordered_map<table_name, to_gen_t> gen_in;
  to_gen_t to_gen_ld;
  to_gen_ld.column = "b";
  to_gen_ld.dbname = "vaultdb_";
  to_gen_ld.scan_tables.insert(to_gen_ld.scan_tables.end(), scan.begin(),
                               scan.end());
  gen_in["left_deep_joins_1024"] = to_gen_ld;
  vector<tableid_ptr> into_repart;
  if (gen_level > 0) {/* if 0, we are running obliviously */
  LOG(EXEC) << "======Start Generalize====";
  START_TIMER(generalize);
  auto gen_zipped_map = p->Generalize(gen_in, gen_level);
  END_AND_LOG_EXEC_TIMER(generalize);
  LOG(EXEC) << "======End Generalize====";
  auto gen_zipped = gen_zipped_map["left_deep_joins_1024"];
  into_repart = gen_zipped;
} else if (gen_level == -1) {
    into_repart = scan;
} else if (gen_level == 0) {
    auto obli = p->MakeObli(scan, "b");
    into_repart = obli;
  }

  p->SetControlFlowColName("b");
  LOG(EXEC) << "======Start Repartition==== GEN_LEVEL:[" << gen_level << "]";
  START_TIMER(repartition);
  auto repart = p->Repartition(into_repart);
  END_AND_LOG_EXP5_STAT_TIMER(repartition, gen_level);
  LOG(EXEC) << "======End Repartition====";
  auto to_join1 = zip_join_tables(repart, repart);
  JoinDef jd;
  jd.set_l_col_name("b");
  jd.set_r_col_name("b");
  jd.set_project_len(1);
  auto p1 = jd.add_project_list();
  p1->set_colname("b");
  p1->set_side(JoinColID_RelationSide_LEFT);
  LOG(EXEC) << "======Start Join 1====";
  START_TIMER(join1);
  auto out1 = p->Join(to_join1, jd, sgx);
  END_AND_LOG_EXP5_STAT_TIMER(join1, gen_level);
  LOG(EXEC) << "======END Join 1====";
  auto to_join2 = zip_join_tables(repart, out1);
  LOG(EXEC) << "======Start Join 2====";
  START_TIMER(join2);
  auto out2 = p->Join(to_join2, jd, sgx);
  END_AND_LOG_EXP5_STAT_TIMER(join2, gen_level);
  LOG(EXEC) << "======END Join 2====";
  auto to_join3 = zip_join_tables(repart, out2);
  LOG(EXEC) << "======Start Join 3====";
  START_TIMER(join3);
  auto out3 = p->Join(to_join3, jd, sgx);
  END_AND_LOG_EXP5_STAT_TIMER(join3, gen_level);
  LOG(EXEC) << "======END Join 3====";
  auto to_join4 = zip_join_tables(repart, out3);
  if (gen_level != 0) {
    LOG(EXEC) << "======Start Join 4====";
    START_TIMER(join4);
    auto out4 = p->Join(to_join4, jd, sgx);
    END_AND_LOG_EXP5_STAT_TIMER(join4, gen_level);
    LOG(EXEC) << "======END Join 4====";
  }
}

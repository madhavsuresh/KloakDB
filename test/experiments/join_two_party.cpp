//
// Created by madhav on 11/26/18.
//

#include "VaultDB.h"
#include <logger/LoggerDefs.h>
#include <rpc/HonestBrokerPrivate.h>

void join_two_party(HonestBrokerPrivate *p, int gen_level, bool sgx) {
  string lda = "left_deep_joins_4096";
  string ldb = "left_deep_joins_512_b";
  auto scan_a = p->ClusterDBMSQuery("dbname=vaultdb_", "SELECT * FROM " + lda);
  auto scan_b = p->ClusterDBMSQuery("dbname=vaultdb_", "SELECT * FROM " + ldb);

  unordered_map<table_name, to_gen_t> gen_in;
  to_gen_t to_gen_a;
  to_gen_a.column = "b";
  to_gen_a.dbname = "vaultdb_";
  to_gen_a.scan_tables.insert(to_gen_a.scan_tables.end(), scan_a.begin(),
                              scan_a.end());
  gen_in[lda] = to_gen_a;
  to_gen_t to_gen_b;
  to_gen_b.column = "b";
  to_gen_b.dbname = "vaultdb_";
  to_gen_b.scan_tables.insert(to_gen_b.scan_tables.end(), scan_b.begin(),
                              scan_b.end());
  gen_in[ldb] = to_gen_b;

  LOG(EXEC) << "======Start Generalize====";
  START_TIMER(generalize);
  auto gen_zipped_map = p->Generalize(gen_in, gen_level);
  END_AND_LOG_EXEC_TIMER(generalize);
  LOG(EXEC) << "======End Generalize====";

  START_TIMER(repartition);
  auto repart_a = p->Repartition(gen_zipped_map[lda]);
  auto repart_b = p->Repartition(gen_zipped_map[ldb]);
  LOG(EXEC) << "======End Repartition====";
  auto to_join1 = zip_join_tables(repart_a, repart_b);
  JoinDef jd;
  jd.set_l_col_name("b");
  jd.set_r_col_name("b");
  jd.set_project_len(1);
  auto p1 = jd.add_project_list();
  p1->set_colname("b");
  p1->set_side(JoinColID_RelationSide_LEFT);
  START_TIMER(join_two_party_t);
  auto out1 = p->Join(to_join1, jd, sgx);
  END_AND_LOG_EXP5_STAT_TIMER(join_two_party_t, gen_level);
  LOG(EXEC) << "======END Join 1====";
}

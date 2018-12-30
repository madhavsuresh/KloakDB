//
// Created by madhav on 11/26/18.
//

#include "VaultDB.h"
#include <logger/LoggerDefs.h>
#include <rpc/HonestBrokerPrivate.h>

void exp5(HonestBrokerPrivate *p, int gen_level, bool sgx) {
  string lda = "left_deep_joins_512_a";
  string ldb = "left_deep_joins_512_b";
  string ldc = "left_deep_joins_512_c";
  string ldd = "left_deep_joins_512_d";
  string lde = "left_deep_joins_512_e";
  auto scan_a = p->ClusterDBMSQuery("dbname=vaultdb_", "SELECT * FROM " + lda);
  auto scan_b = p->ClusterDBMSQuery("dbname=vaultdb_", "SELECT * FROM " + ldb);
  auto scan_c = p->ClusterDBMSQuery("dbname=vaultdb_", "SELECT * FROM " + ldc);
  auto scan_d = p->ClusterDBMSQuery("dbname=vaultdb_", "SELECT * FROM " + ldd);
  auto scan_e = p->ClusterDBMSQuery("dbname=vaultdb_", "SELECT * FROM " + lde);


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

  to_gen_t to_gen_c;
  to_gen_c.column = "b";
  to_gen_c.dbname = "vaultdb_";
  to_gen_c.scan_tables.insert(to_gen_c.scan_tables.end(), scan_c.begin(),
                              scan_c.end());
  gen_in[ldc] = to_gen_c;

  to_gen_t to_gen_d;
  to_gen_d.column = "b";
  to_gen_d.dbname = "vaultdb_";
  to_gen_d.scan_tables.insert(to_gen_d.scan_tables.end(), scan_d.begin(),
                              scan_d.end());
  gen_in[ldd] = to_gen_d;

  to_gen_t to_gen_e;
  to_gen_e.column = "b";
  to_gen_e.dbname = "vaultdb_";
  to_gen_e.scan_tables.insert(to_gen_e.scan_tables.end(), scan_e.begin(),
                              scan_e.end());
  gen_in[lde] = to_gen_e;

  LOG(EXEC) << "======Start Generalize====";
  START_TIMER(generalize);
  auto gen_zipped_map = p->Generalize(gen_in, gen_level);
  END_AND_LOG_EXEC_TIMER(generalize);
  LOG(EXEC) << "======End Generalize====";

  p->SetControlFlowColName("b");
  LOG(EXEC) << "======Start Repartition==== GEN_LEVEL:[" << gen_level << "]";
  START_TIMER(repartition);
  auto repart_a = p->Repartition(gen_zipped_map[lda]);
  auto repart_b = p->Repartition(gen_zipped_map[ldb]);
  auto repart_c = p->Repartition(gen_zipped_map[ldc]);
  auto repart_d = p->Repartition(gen_zipped_map[ldd]);
  auto repart_e = p->Repartition(gen_zipped_map[lde]);
  END_AND_LOG_EXP5_STAT_TIMER(repartition, gen_level);
  LOG(EXEC) << "======End Repartition====";
  auto to_join1 = zip_join_tables(repart_a, repart_b);
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
  auto to_join2 = zip_join_tables(repart_c, out1);
  LOG(EXEC) << "======Start Join 2====";
  START_TIMER(join2);
  auto out2 = p->Join(to_join2, jd, sgx);
  END_AND_LOG_EXP5_STAT_TIMER(join2, gen_level);
  LOG(EXEC) << "======END Join 2====";
  auto to_join3 = zip_join_tables(repart_d, out2);
  LOG(EXEC) << "======Start Join 3====";
  START_TIMER(join3);
  auto out3 = p->Join(to_join3, jd, sgx);
  END_AND_LOG_EXP5_STAT_TIMER(join3, gen_level);
  LOG(EXEC) << "======END Join 3====";
  auto to_join4 = zip_join_tables(repart_e, out3);
  if (gen_level != 0) {
    LOG(EXEC) << "======Start Join 4====";
    START_TIMER(join4);
    auto out4 = p->Join(to_join4, jd, sgx);
    END_AND_LOG_EXP5_STAT_TIMER(join4, gen_level);
    LOG(EXEC) << "======END Join 4====";
  }
}

//
// Created by madhav on 11/26/18.
//

#include "VaultDB.h"
#include <logger/LoggerDefs.h>
#include <rpc/HonestBrokerPrivate.h>

void agg_two_party(HonestBrokerPrivate *p, int gen_level, bool sgx) {
  string lda = "left_deep_joins_4096";
  string ldb = "left_deep_joins_4096";
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
  LOG(EXEC) << "======End Repartition====";
  START_TIMER(agg_two_party_t);
  GroupByDef gbd;
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_col_name("a");
  gbd.set_secure(true);
  gbd.set_kanon_col_name("b");
  gbd.add_gb_col_names("a");
 auto final_avg = p->Aggregate(repart_a, gbd, sgx);
  END_AND_LOG_EXP5_STAT_TIMER(agg_two_party_t, gen_level);
  LOG(EXEC) << "======End Generalize====";
}

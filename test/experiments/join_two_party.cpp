//
// Created by madhav on 11/26/18.
//

#include "VaultDB.h"
#include <logger/LoggerDefs.h>
#include <rpc/HonestBrokerPrivate.h>

void join_two_party(HonestBrokerPrivate *p, int gen_level, bool sgx) {
  string dbname = "tpch_scale_001";
  auto scan_a = p->DBMSQuery(0,"dbname=" + dbname, "SELECT l_orderkey FROM lineitem ORDER BY l_orderkey LIMIT 6000");
  auto scan_b = p->DBMSQuery(0,"dbname=" + dbname, "SELECT o_orderkey FROM orders ORDER BY o_orderkey LIMIT 1480");

  unordered_map<table_name, to_gen_t> gen_in;
  to_gen_t to_gen_a;
  to_gen_a.column = "l_orderkey";
  to_gen_a.dbname = dbname;
  to_gen_a.scan_tables.push_back(scan_a);
  gen_in["lineitem"] = to_gen_a;

  to_gen_t to_gen_b;
  to_gen_b.column = "o_orderkey";
  to_gen_b.dbname = dbname;
  to_gen_b.scan_tables.push_back(scan_b);
  gen_in["orders"] = to_gen_b;

  LOG(EXEC) << "======Start Generalize====";
  START_TIMER(generalize);
  auto gen_zipped_map = p->Generalize(gen_in, gen_level);
  END_AND_LOG_EXEC_TIMER(generalize);
  LOG(EXEC) << "======End Generalize====";

  START_TIMER(repartition);
  auto repart_a = p->Repartition(gen_zipped_map["lineitem"]);
  auto repart_b = p->Repartition(gen_zipped_map["orders"]);
  LOG(EXEC) << "======End Repartition====";
  auto to_join1 = zip_join_tables(repart_a, repart_b);
  JoinDef jd;
  jd.set_l_col_name("l_orderkey");
  jd.set_r_col_name("o_orderkey");
  jd.set_project_len(1);
  auto p1 = jd.add_project_list();
  p1->set_colname("l_orderkey");
  p1->set_side(JoinColID_RelationSide_LEFT);
  START_TIMER(join_two_party_t);
  auto out1 = p->Join(to_join1, jd, sgx);
  END_AND_LOG_EXP5_STAT_TIMER(join_two_party_t, gen_level);
  LOG(EXEC) << "======END Join 1====";
}

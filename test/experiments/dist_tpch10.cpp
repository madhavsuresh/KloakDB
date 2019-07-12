//
// Created by madhav on 11/29/18.
//
#include "distributed_aspirin_profile.h"
#include "logger/LoggerDefs.h"
#include <chrono>
#include <gflags/gflags.h>
#include <thread>

void tpch_10_encrypted(HonestBrokerPrivate *p, std::string database, bool sgx) {

  LOG(EXEC) << "STARTING TPCH-10 ENCRYPTED DISTRIBUTED";
  START_TIMER(tpch_10_full);
  START_TIMER(postgres_read);
  auto lineitem = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT l_orderkey, l_extendedprice*(1-l_discount) "
                            "as revenue FROM lineitem");
  auto orders = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT o_custkey, o_orderkey from orders WHERE o_orderdate "
      "<'1995-10-01' AND o_orderdate >= '1993-07-01'");
  auto customer = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT c_custkey, c_name, c_acctbal, "
      "c_phone, c_address, c_comment, c_nationkey FROM customer");
  auto nation = p->ClusterDBMSQuery("dbname=" + database,
                                    "SELECT n_name, n_nationkey FROM nation");

  // JOIN1
  LOG(EXEC) << "JOIN 1 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_orderkey");
  auto lineitem_repart = p->RepartitionJustHash(lineitem);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_orderkey");
  auto orders_repart = p->RepartitionJustHash(orders);
  JoinDef jd1;
  jd1.set_l_col_name("l_orderkey");
  jd1.set_r_col_name("o_orderkey");
  jd1.set_project_len(2);
  auto j1p1 = jd1.add_project_list();
  j1p1->set_side(JoinColID_RelationSide_RIGHT);
  j1p1->set_colname("o_custkey");
  auto j1p2 = jd1.add_project_list();
  j1p2->set_side(JoinColID_RelationSide_LEFT);
  j1p2->set_colname("revenue");
  auto to_join1 = zip_join_tables(lineitem_repart, orders_repart);
  auto lo = p->Join(to_join1, jd1, sgx);
  LOG(EXEC) << "JOIN 1 END==";

  // JOIN2
  LOG(EXEC) << "JOIN 2 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_custkey");
  auto lo_repart = p->RepartitionJustHash(lo);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_custkey");
  auto cust_repart = p->RepartitionJustHash(customer);

  JoinDef jd2;
  jd2.set_l_col_name("o_custkey");
  jd2.set_r_col_name("c_custkey");
  jd2.set_project_len(8);
  auto j2p1 = jd2.add_project_list();
  j2p1->set_side(JoinColID_RelationSide_RIGHT);
  j2p1->set_colname("c_custkey");
  auto j2p2 = jd2.add_project_list();
  j2p2->set_side(JoinColID_RelationSide_RIGHT);
  j2p2->set_colname("c_name");
  auto j2p3 = jd2.add_project_list();
  j2p3->set_side(JoinColID_RelationSide_RIGHT);
  j2p3->set_colname("c_acctbal");
  auto j2p4 = jd2.add_project_list();
  j2p4->set_side(JoinColID_RelationSide_RIGHT);
  j2p4->set_colname("c_phone");
  auto j2p5 = jd2.add_project_list();
  j2p5->set_side(JoinColID_RelationSide_RIGHT);
  j2p5->set_colname("c_address");
  auto j2p6 = jd2.add_project_list();
  j2p6->set_side(JoinColID_RelationSide_RIGHT);
  j2p6->set_colname("c_comment");
  auto j2p7 = jd2.add_project_list();
  j2p7->set_side(JoinColID_RelationSide_RIGHT);
  j2p7->set_colname("c_nationkey");
  auto j2p8 = jd2.add_project_list();
  j2p8->set_side(JoinColID_RelationSide_LEFT);
  j2p8->set_colname("revenue");
  auto to_join2 = zip_join_tables(lo_repart, cust_repart);
  auto loc = p->Join(to_join2, jd2, sgx);
  LOG(EXEC) << "JOIN 2 END==";

  // JOIN3
  LOG(EXEC) << "JOIN 3 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_nationkey");
  auto loc_repart = p->RepartitionJustHash(loc);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("n_nationkey");
  //auto nation_repart = p->RepartitionJustHash(nation);

  JoinDef jd3;
  jd3.set_l_col_name("c_nationkey");
  jd3.set_r_col_name("n_nationkey");
  jd3.set_project_len(9);
  auto j3p1 = jd3.add_project_list();
  j3p1->set_side(JoinColID_RelationSide_LEFT);
  j3p1->set_colname("c_custkey");
  auto j3p2 = jd3.add_project_list();
  j3p2->set_side(JoinColID_RelationSide_LEFT);
  j3p2->set_colname("c_name");
  auto j3p3 = jd3.add_project_list();
  j3p3->set_side(JoinColID_RelationSide_LEFT);
  j3p3->set_colname("c_acctbal");
  auto j3p4 = jd3.add_project_list();
  j3p4->set_side(JoinColID_RelationSide_LEFT);
  j3p4->set_colname("c_phone");
  auto j3p5 = jd3.add_project_list();
  j3p5->set_side(JoinColID_RelationSide_LEFT);
  j3p5->set_colname("c_address");
  auto j3p6 = jd3.add_project_list();
  j3p6->set_side(JoinColID_RelationSide_LEFT);
  j3p6->set_colname("c_comment");
  auto j3p7 = jd3.add_project_list();
  j3p7->set_side(JoinColID_RelationSide_LEFT);
  j3p7->set_colname("revenue");
  auto j3p8 = jd3.add_project_list();
  j3p8->set_side(JoinColID_RelationSide_RIGHT);
  j3p8->set_colname("n_name");
  auto j3p9 = jd3.add_project_list();
  j3p9->set_side(JoinColID_RelationSide_LEFT);
  j3p9->set_colname("c_nationkey");
  auto to_join3 = zip_join_tables(loc_repart, nation);
  auto locn = p->Join(to_join3, jd3, sgx);

  GroupByDef gbd;
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_secure(true);
  gbd.set_col_name("revenue");
  gbd.set_kanon_col_name("c_nationkey");
  gbd.add_gb_col_names("c_custkey");
  gbd.add_gb_col_names("c_name");
  gbd.add_gb_col_names("c_acctbal");
  gbd.add_gb_col_names("c_phone");
  gbd.add_gb_col_names("n_name");
  gbd.add_gb_col_names("c_address");
  gbd.add_gb_col_names("c_comment");
  auto agg_out = p->Aggregate(locn, gbd, sgx);
  END_AND_LOG_EXP_TPCH_TIMER(tpch_10_full, 0);
}


void tpch_10_gen(HonestBrokerPrivate *p, std::string database, bool sgx, int gen_level, bool truncate) {

  SortDef sort;
  sort.set_sorting_dummies(true);
  sort.set_truncate(true);
  LOG(EXEC) << "STARTING TPCH-10 ENCRYPTED DISTRIBUTED";
  START_TIMER(tpch_10_full);
  START_TIMER(tpch_10_full_truncate);
  START_TIMER(tpch_10_full_no_truncate);
  START_TIMER(tpch_10_postgres_read);
  auto lineitem = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT l_orderkey, l_extendedprice*(1-l_discount) "
                            "as revenue FROM lineitem");
  auto orders = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT o_custkey, o_orderkey from orders WHERE o_orderdate "
      "<'1995-10-01' AND o_orderdate >= '1993-07-01'");
  auto customer = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT c_custkey, c_name, c_acctbal, "
      "c_phone, c_address, c_comment, c_nationkey FROM customer");
  auto nation = p->ClusterDBMSQuery("dbname=" + database,
                                    "SELECT n_name, n_nationkey FROM nation");

  END_AND_LOG_EXP_TPCH_TIMER(tpch_10_postgres_read, gen_level);
  START_TIMER(tpch_10_gen);
  /*JOIN 1 ANON*/
  unordered_map<table_name, to_gen_t> gen_in;
  to_gen_t lineitem_gen;
  lineitem_gen.column = "l_orderkey";
  lineitem_gen.dbname = database;
  lineitem_gen.scan_tables.insert(lineitem_gen.scan_tables.end(), lineitem.begin(), lineitem.end());
  gen_in["lineitem"] = lineitem_gen;

  to_gen_t orders_gen1;
  orders_gen1.column = "o_orderkey";
  orders_gen1.dbname = database;
  orders_gen1.scan_tables.insert(orders_gen1.scan_tables.end(), orders.begin(), orders.end());
  gen_in["orders"] = orders_gen1;
  auto gen_zipped_map1 = p->Generalize(gen_in, gen_level);
  /*END JOIN 1 ANON*/
  /*lineitem, orders*/

  /*JOIN 2 ANON*/
  unordered_map<table_name, to_gen_t> gen_in2;
  to_gen_t orders_gen_2;
  orders_gen_2.column = "o_custkey";
  orders_gen_2.dbname = database;
  orders_gen_2.scan_tables.insert(orders_gen_2.scan_tables.end(), gen_zipped_map1["orders"].begin(), 
	  gen_zipped_map1["orders"].end());
  gen_in2["orders"] = orders_gen_2;

  to_gen_t customer_gen1;
  customer_gen1.column = "c_custkey";
  customer_gen1.dbname = database;
  customer_gen1.scan_tables.insert(customer_gen1.scan_tables.end(), 
	  customer.begin(), customer.end());
  gen_in2["customer"] = customer_gen1;
  auto gen_zipped_map2 = p->Generalize(gen_in2, gen_level);
  /*END JOIN 2 ANON*/
  /*orders, customer*/

  /*JOIN 3 ANON*/
  /*
  unordered_map<table_name, to_gen_t> gen_in3;
  to_gen_t customer_gen_2;
  customer_gen_2.column = "c_nationkey";
  customer_gen_2.dbname = database;
  customer_gen_2.scan_tables.insert(customer_gen_2.scan_tables.end(), gen_zipped_map2["customer"].begin(), 
	  gen_zipped_map2["customer"].end());
  gen_in3["customer"] = customer_gen_2;

  to_gen_t nation_gen1;
  nation_gen1.column = "n_nationkey";
  nation_gen1.dbname = database;
  nation_gen1.scan_tables.insert(nation_gen1.scan_tables.end(), 
	  nation.begin(), nation.end());
  gen_in3["nation"] = nation_gen1;
  auto gen_zipped_map3 = p->Generalize(gen_in3, gen_level);
  /*END JOIN 3 ANON*/

  END_AND_LOG_EXP_TPCH_TIMER(tpch_10_gen, gen_level);
  /*customer, nation*/
  START_TIMER(tpch_10_no_gen_full);
  // JOIN1
  LOG(EXEC) << "JOIN 1 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_orderkey");
  auto lineitem_repart = p->RepartitionJustHash(gen_zipped_map1["lineitem"]);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_orderkey");
  auto orders_repart = p->RepartitionJustHash(gen_zipped_map2["orders"]);
  JoinDef jd1;
  jd1.set_l_col_name("l_orderkey");
  jd1.set_r_col_name("o_orderkey");
  jd1.set_project_len(2);
  auto j1p1 = jd1.add_project_list();
  j1p1->set_side(JoinColID_RelationSide_RIGHT);
  j1p1->set_colname("o_custkey");
  auto j1p2 = jd1.add_project_list();
  j1p2->set_side(JoinColID_RelationSide_LEFT);
  j1p2->set_colname("revenue");
  auto to_join1 = zip_join_tables(lineitem_repart, orders_repart);
  auto lo = p->Join(to_join1, jd1, sgx);
  if (truncate) {
      auto sorted_lo = p->Sort(lo, sort, sgx);
      lo = sorted_lo;
  }
  LOG(EXEC) << "JOIN 1 END==";

  // JOIN2
  LOG(EXEC) << "JOIN 2 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_custkey");
  auto lo_repart = p->RepartitionJustHash(lo);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_custkey");
  auto cust_repart = p->RepartitionJustHash(gen_zipped_map2["customer"]);

  JoinDef jd2;
  jd2.set_l_col_name("o_custkey");
  jd2.set_r_col_name("c_custkey");
  jd2.set_project_len(8);
  auto j2p1 = jd2.add_project_list();
  j2p1->set_side(JoinColID_RelationSide_RIGHT);
  j2p1->set_colname("c_custkey");
  auto j2p2 = jd2.add_project_list();
  j2p2->set_side(JoinColID_RelationSide_RIGHT);
  j2p2->set_colname("c_name");
  auto j2p3 = jd2.add_project_list();
  j2p3->set_side(JoinColID_RelationSide_RIGHT);
  j2p3->set_colname("c_acctbal");
  auto j2p4 = jd2.add_project_list();
  j2p4->set_side(JoinColID_RelationSide_RIGHT);
  j2p4->set_colname("c_phone");
  auto j2p5 = jd2.add_project_list();
  j2p5->set_side(JoinColID_RelationSide_RIGHT);
  j2p5->set_colname("c_address");
  auto j2p6 = jd2.add_project_list();
  j2p6->set_side(JoinColID_RelationSide_RIGHT);
  j2p6->set_colname("c_comment");
  auto j2p7 = jd2.add_project_list();
  j2p7->set_side(JoinColID_RelationSide_RIGHT);
  j2p7->set_colname("c_nationkey");
  auto j2p8 = jd2.add_project_list();
  j2p8->set_side(JoinColID_RelationSide_LEFT);
  j2p8->set_colname("revenue");
  auto to_join2 = zip_join_tables(lo_repart, cust_repart);
  auto loc = p->Join(to_join2, jd2, sgx);
  LOG(EXEC) << "JOIN 2 END==";
  if (truncate) {
      auto sorted_loc = p->Sort(loc, sort, sgx);
      loc = sorted_loc;
  }

  // JOIN3
  LOG(EXEC) << "JOIN 3 START==";
  //p->ResetControlFlowCols();
  //p->SetControlFlowColName("c_nationkey");
  //auto loc_repart = p->RepartitionJustHash(loc);
  //p->ResetControlFlowCols();
  //p->SetControlFlowColName("n_nationkey");
  //auto nation_repart = p->RepartitionJustHash(nation);

  JoinDef jd3;
  jd3.set_l_col_name("c_nationkey");
  jd3.set_r_col_name("n_nationkey");
  jd3.set_project_len(9);
  auto j3p1 = jd3.add_project_list();
  j3p1->set_side(JoinColID_RelationSide_LEFT);
  j3p1->set_colname("c_custkey");
  auto j3p2 = jd3.add_project_list();
  j3p2->set_side(JoinColID_RelationSide_LEFT);
  j3p2->set_colname("c_name");
  auto j3p3 = jd3.add_project_list();
  j3p3->set_side(JoinColID_RelationSide_LEFT);
  j3p3->set_colname("c_acctbal");
  auto j3p4 = jd3.add_project_list();
  j3p4->set_side(JoinColID_RelationSide_LEFT);
  j3p4->set_colname("c_phone");
  auto j3p5 = jd3.add_project_list();
  j3p5->set_side(JoinColID_RelationSide_LEFT);
  j3p5->set_colname("c_address");
  auto j3p6 = jd3.add_project_list();
  j3p6->set_side(JoinColID_RelationSide_LEFT);
  j3p6->set_colname("c_comment");
  auto j3p7 = jd3.add_project_list();
  j3p7->set_side(JoinColID_RelationSide_LEFT);
  j3p7->set_colname("revenue");
  auto j3p8 = jd3.add_project_list();
  j3p8->set_side(JoinColID_RelationSide_RIGHT);
  j3p8->set_colname("n_name");
  auto j3p9 = jd3.add_project_list();
  j3p9->set_side(JoinColID_RelationSide_LEFT);
  j3p9->set_colname("c_nationkey");
  auto to_join3 = zip_join_tables(loc, nation);
  auto locn = p->Join(to_join3, jd3, sgx);
  if (truncate) {
      auto sorted_locn = p->Sort(locn, sort, sgx);
      locn = sorted_locn;
  }

  GroupByDef gbd;
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_secure(true);
  gbd.set_col_name("revenue");
  gbd.set_kanon_col_name("c_nationkey");
  gbd.add_gb_col_names("c_custkey");
  gbd.add_gb_col_names("c_name");
  gbd.add_gb_col_names("c_acctbal");
  gbd.add_gb_col_names("c_phone");
  gbd.add_gb_col_names("n_name");
  gbd.add_gb_col_names("c_address");
  gbd.add_gb_col_names("c_comment");
  auto agg_out = p->Aggregate(locn, gbd, sgx);
  if(truncate) {
      END_AND_LOG_EXP_TPCH_TIMER(tpch_10_full_truncate, gen_level);
  } else {
      END_AND_LOG_EXP_TPCH_TIMER(tpch_10_full_no_truncate, gen_level);
  }
  END_AND_LOG_EXP_TPCH_TIMER(tpch_10_full, gen_level);
  END_AND_LOG_EXP_TPCH_TIMER(tpch_10_no_gen_full, gen_level);
}

void tpch_10_obli(HonestBrokerPrivate *p, std::string database, bool sgx) {

  SortDef sort;
  sort.set_sorting_dummies(true);
  sort.set_truncate(true);
  LOG(EXEC) << "STARTING TPCH-10 ENCRYPTED DISTRIBUTED";
  START_TIMER(tpch_10_full);
  START_TIMER(postgres_read);
  auto lineitem = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT l_orderkey, l_extendedprice*(1-l_discount) "
                            "as revenue FROM lineitem");
  auto orders = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT o_custkey, o_orderkey from orders WHERE o_orderdate "
      "<'1995-10-01' AND o_orderdate >= '1993-07-01'");
  auto customer = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT c_custkey, c_name, c_acctbal, "
      "c_phone, c_address, c_comment, c_nationkey FROM customer");
  auto nation = p->ClusterDBMSQuery("dbname=" + database,
                                    "SELECT n_name, n_nationkey FROM nation");

  p->MakeObli(lineitem, "l_orderkey");
  p->MakeObli(orders, "o_orderkey");
  p->MakeObli(orders, "o_custkey");
  p->MakeObli(customer, "c_custkey");
  p->MakeObli(customer, "c_nationkey");
  p->MakeObli(nation, "n_nationkey");

  // JOIN1
  LOG(EXEC) << "JOIN 1 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_orderkey");
  auto lineitem_repart = p->RepartitionJustHash(lineitem);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_orderkey");
  auto orders_repart = p->RepartitionJustHash(orders);
  JoinDef jd1;
  jd1.set_l_col_name("l_orderkey");
  jd1.set_r_col_name("o_orderkey");
  jd1.set_project_len(2);
  auto j1p1 = jd1.add_project_list();
  j1p1->set_side(JoinColID_RelationSide_RIGHT);
  j1p1->set_colname("o_custkey");
  auto j1p2 = jd1.add_project_list();
  j1p2->set_side(JoinColID_RelationSide_LEFT);
  j1p2->set_colname("revenue");
  auto to_join1 = zip_join_tables(lineitem_repart, orders_repart);
  auto lo = p->Join(to_join1, jd1, sgx);
  auto truncated_lo = p->Sort(lo, sort, sgx);
  lo = truncated_lo;
  LOG(EXEC) << "JOIN 1 END==";

  // JOIN2
  LOG(EXEC) << "JOIN 2 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_custkey");
  auto lo_repart = p->RepartitionJustHash(lo);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_custkey");
  auto cust_repart = p->RepartitionJustHash(customer);

  JoinDef jd2;
  jd2.set_l_col_name("o_custkey");
  jd2.set_r_col_name("c_custkey");
  jd2.set_project_len(8);
  auto j2p1 = jd2.add_project_list();
  j2p1->set_side(JoinColID_RelationSide_RIGHT);
  j2p1->set_colname("c_custkey");
  auto j2p2 = jd2.add_project_list();
  j2p2->set_side(JoinColID_RelationSide_RIGHT);
  j2p2->set_colname("c_name");
  auto j2p3 = jd2.add_project_list();
  j2p3->set_side(JoinColID_RelationSide_RIGHT);
  j2p3->set_colname("c_acctbal");
  auto j2p4 = jd2.add_project_list();
  j2p4->set_side(JoinColID_RelationSide_RIGHT);
  j2p4->set_colname("c_phone");
  auto j2p5 = jd2.add_project_list();
  j2p5->set_side(JoinColID_RelationSide_RIGHT);
  j2p5->set_colname("c_address");
  auto j2p6 = jd2.add_project_list();
  j2p6->set_side(JoinColID_RelationSide_RIGHT);
  j2p6->set_colname("c_comment");
  auto j2p7 = jd2.add_project_list();
  j2p7->set_side(JoinColID_RelationSide_RIGHT);
  j2p7->set_colname("c_nationkey");
  auto j2p8 = jd2.add_project_list();
  j2p8->set_side(JoinColID_RelationSide_LEFT);
  j2p8->set_colname("revenue");
  auto to_join2 = zip_join_tables(lo_repart, cust_repart);
  auto loc = p->Join(to_join2, jd2, sgx);
  auto truncated_loc = p->Sort(loc, sort, sgx);
  loc = truncated_loc;
  LOG(EXEC) << "JOIN 2 END==";

  // JOIN3
  LOG(EXEC) << "JOIN 3 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_nationkey");
  auto loc_repart = p->RepartitionJustHash(loc);
  //p->ResetControlFlowCols();
  //p->SetControlFlowColName("n_nationkey");
  //auto nation_repart = p->RepartitionJustHash(nation);

  JoinDef jd3;
  jd3.set_l_col_name("c_nationkey");
  jd3.set_r_col_name("n_nationkey");
  jd3.set_project_len(9);
  auto j3p1 = jd3.add_project_list();
  j3p1->set_side(JoinColID_RelationSide_LEFT);
  j3p1->set_colname("c_custkey");
  auto j3p2 = jd3.add_project_list();
  j3p2->set_side(JoinColID_RelationSide_LEFT);
  j3p2->set_colname("c_name");
  auto j3p3 = jd3.add_project_list();
  j3p3->set_side(JoinColID_RelationSide_LEFT);
  j3p3->set_colname("c_acctbal");
  auto j3p4 = jd3.add_project_list();
  j3p4->set_side(JoinColID_RelationSide_LEFT);
  j3p4->set_colname("c_phone");
  auto j3p5 = jd3.add_project_list();
  j3p5->set_side(JoinColID_RelationSide_LEFT);
  j3p5->set_colname("c_address");
  auto j3p6 = jd3.add_project_list();
  j3p6->set_side(JoinColID_RelationSide_LEFT);
  j3p6->set_colname("c_comment");
  auto j3p7 = jd3.add_project_list();
  j3p7->set_side(JoinColID_RelationSide_LEFT);
  j3p7->set_colname("revenue");
  auto j3p8 = jd3.add_project_list();
  j3p8->set_side(JoinColID_RelationSide_RIGHT);
  j3p8->set_colname("n_name");
  auto j3p9 = jd3.add_project_list();
  j3p9->set_side(JoinColID_RelationSide_LEFT);
  j3p9->set_colname("c_nationkey");
  auto to_join3 = zip_join_tables(loc_repart, nation);
  auto locn = p->Join(to_join3, jd3, sgx);
  auto truncated_locn = p->Sort(locn, sort, sgx);
  locn = truncated_locn;

  GroupByDef gbd;
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_secure(true);
  gbd.set_col_name("revenue");
  gbd.set_kanon_col_name("c_nationkey");
  gbd.add_gb_col_names("c_custkey");
  gbd.add_gb_col_names("c_name");
  gbd.add_gb_col_names("c_acctbal");
  gbd.add_gb_col_names("c_phone");
  gbd.add_gb_col_names("n_name");
  gbd.add_gb_col_names("c_address");
  gbd.add_gb_col_names("c_comment");
  auto agg_out = p->Aggregate(locn, gbd, sgx);
  END_AND_LOG_EXP_TPCH_TIMER(tpch_10_full, 0);
}

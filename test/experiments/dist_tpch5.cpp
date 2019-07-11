//
// Created by madhav on 11/29/18.
//
#include "distributed_aspirin_profile.h"
#include "logger/LoggerDefs.h"
#include <chrono>
#include <gflags/gflags.h>
#include <thread>

void tpch_5_encrypted(HonestBrokerPrivate *p, std::string database, bool sgx) {

  LOG(EXEC) << "STARTING TPCH-5 ENCRYPTED DISTRIBUTED";
  START_TIMER(tpch_5_full);
  START_TIMER(postgres_read);
  auto customer = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT c_custkey, c_nationkey FROM customer");
  auto orders = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT o_custkey, o_orderkey FROM orders WHERE o_orderdate "
      ">='1993-01-01' AND o_orderdate < '1994-01-01'");
  auto lineitem = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT l_orderkey, l_suppkey, l_extendedprice*(1-l_discount) "
      "as revenue FROM lineitem");

  auto supplier = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT s_suppkey, s_nationkey FROM supplier");
  auto nation = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT n_name, n_regionkey, n_nationkey FROM nation");
  auto region = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT r_regionkey FROM region WHERE r_name= 'AFRICA'");

  p->SetControlFlowColName("n_regionkey");
  auto nation_repart = p->RepartitionJustHash(nation);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("r_regionkey");
  auto region_repart = p->RepartitionJustHash(region);

  LOG(EXEC) << "JOIN 1 START==";
  // JOIN1
  // join def vitals-diagnoses
  START_TIMER(join_one);
  JoinDef jd_vd;
  jd_vd.set_l_col_name("n_regionkey");
  jd_vd.set_r_col_name("r_regionkey");
  jd_vd.set_project_len(2);
  // vitals-diagnoses-join project 1
  auto vdjp1 = jd_vd.add_project_list();
  vdjp1->set_side(JoinColID_RelationSide_LEFT);
  vdjp1->set_colname("n_nationkey");
  auto vdjp2 = jd_vd.add_project_list();
  vdjp2->set_side(JoinColID_RelationSide_LEFT);
  vdjp2->set_colname("n_name");
  auto to_join1 = zip_join_tables(nation_repart, region_repart);
  auto nr = p->Join(to_join1, jd_vd, sgx);
  LOG(EXEC) << "JOIN 1 END==";

  // JOIN2
  LOG(EXEC) << "JOIN 2 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("n_nationkey");
  auto nr_repart = p->RepartitionJustHash(nr);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_nationkey");
  auto customer_repart = p->RepartitionJustHash(customer);
  START_TIMER(join_two);
  JoinDef jd_vd2;
  jd_vd2.set_l_col_name("c_nationkey");
  jd_vd2.set_r_col_name("n_nationkey");
  // PROJECT
  jd_vd2.set_project_len(2);
  auto j2p1 = jd_vd2.add_project_list();
  j2p1->set_side(JoinColID_RelationSide_RIGHT);
  j2p1->set_colname("n_name");
  auto j2p2 = jd_vd2.add_project_list();
  j2p2->set_side(JoinColID_RelationSide_LEFT);
  j2p2->set_colname("c_custkey");
  auto to_join2 = zip_join_tables(customer_repart, nr_repart);
  auto cnr = p->Join(to_join2, jd_vd2, sgx);
  LOG(EXEC) << "JOIN 2 END==";
  LOG(EXEC) << "JOIN 3 START==";

  // JOIN 3
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_custkey");
  auto orders_repart = p->RepartitionJustHash(orders);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_custkey");
  auto cnr_repart = p->RepartitionJustHash(cnr);
  JoinDef jd3;
  jd3.set_l_col_name("o_custkey");
  jd3.set_r_col_name("c_custkey");
  jd3.set_project_len(2);
  auto j3p1 = jd3.add_project_list();
  j3p1->set_side(JoinColID_RelationSide_RIGHT);
  j3p1->set_colname("n_name");
  auto j3p2 = jd3.add_project_list();
  j3p2->set_side(JoinColID_RelationSide_LEFT);
  j3p2->set_colname("o_orderkey");
  auto to_join3 = zip_join_tables(orders_repart, cnr_repart);
  auto ocnr = p->Join(to_join3, jd3, sgx);
  LOG(EXEC) << "JOIN 3 END==";
  LOG(EXEC) << "JOIN 4 START==";
  // JOIN 4
  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_orderkey");
  auto lineitem_repart = p->RepartitionJustHash(lineitem);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_orderkey");
  auto ocnr_repart = p->RepartitionJustHash(ocnr);

  JoinDef jd4;
  jd4.set_l_col_name("l_orderkey");
  jd4.set_r_col_name("o_orderkey");
  jd4.set_project_len(3);
  auto j4p1 = jd4.add_project_list();
  j4p1->set_colname("n_name");
  j4p1->set_side(JoinColID_RelationSide_RIGHT);
  auto j4p2 = jd4.add_project_list();
  j4p2->set_colname("l_suppkey");
  j4p2->set_side(JoinColID_RelationSide_LEFT);
  auto j4p3 = jd4.add_project_list();
  j4p3->set_colname("revenue");
  j4p3->set_side(JoinColID_RelationSide_LEFT);
  auto to_join4 = zip_join_tables(lineitem_repart, ocnr_repart);
  auto locnr = p->Join(to_join4, jd4, sgx);

  LOG(EXEC) << "JOIN 4 END==";
  LOG(EXEC) << "JOIN 5 START==";

  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_suppkey");
  auto locnr_repart = p->RepartitionJustHash(locnr);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("s_suppkey");
  auto supp_repart = p->RepartitionJustHash(supplier);

  JoinDef jd5;
  jd5.set_l_col_name("s_suppkey");
  jd5.set_r_col_name("l_suppkey");
  jd5.set_project_len(3);
  auto j5p1 = jd5.add_project_list();
  j5p1->set_colname("n_name");
  j5p1->set_side(JoinColID_RelationSide_RIGHT);
  auto j5p2 = jd5.add_project_list();
  j5p2->set_colname("revenue");
  j5p2->set_side(JoinColID_RelationSide_RIGHT);
  auto j5p3 = jd5.add_project_list();
  j5p3->set_colname("l_suppkey");
  j5p3->set_side(JoinColID_RelationSide_RIGHT);
  // auto to_join5 = zip_join_tables(supplier, locnr);
  auto to_join5 = zip_join_tables(supp_repart, locnr_repart);
  auto slocnr = p->Join(to_join5, jd5, sgx);
  LOG(EXEC) << "JOIN 5 END==";

  LOG(EXEC) << "AGGREGATE START";
  GroupByDef gbd;
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_secure(true);
  gbd.set_col_name("revenue");
  gbd.add_gb_col_names("n_name");
  gbd.set_kanon_col_name("l_suppkey");
  auto agg_out = p->Aggregate(slocnr, gbd, sgx);
  // TODO(madhavsuresh): merge all of the aggregates together.
  END_AND_LOG_EXP_TPCH_TIMER(tpch_5_full, 0);
  // TODO(madhavsuresh): add sort
}

void tpch_5_gen(HonestBrokerPrivate *p, std::string database, bool sgx, int gen_level, bool truncate) {

  LOG(EXEC) << "STARTING TPCH-5 ENCRYPTED DISTRIBUTED";
  SortDef sort;
  sort.set_sorting_dummies(true);
  sort.set_truncate(true);
  START_TIMER(tpch_5_full);
  START_TIMER(tpch_5_full_truncate);
  START_TIMER(tpch_5_full_no_truncate);
  START_TIMER(postgres_read);
  auto customer = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT c_custkey, c_nationkey FROM customer");
  auto orders = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT o_custkey, o_orderkey FROM orders WHERE o_orderdate "
      ">='1993-01-01' AND o_orderdate < '1994-01-01'");
  auto lineitem = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT l_orderkey, l_suppkey, l_extendedprice*(1-l_discount) "
      "as revenue FROM lineitem");

  auto supplier = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT s_suppkey, s_nationkey FROM supplier");
  auto nation = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT n_name, n_regionkey, n_nationkey FROM nation");
  auto region = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT r_regionkey FROM region WHERE r_name= 'AFRICA'");


  /* ANON JOIN 2*/
  unordered_map<table_name, to_gen_t> gen_in2;
  /* END JOIN 2 ANON*/
  /*nation, customer*/
  /* ANON JOIN 3*/
  unordered_map<table_name, to_gen_t> gen_in3;
  to_gen_t customer_gen2;
  customer_gen2.column = "c_custkey";
  customer_gen2.dbname = database;
  customer_gen2.scan_tables.insert(customer_gen2.scan_tables.end(), customer.begin(), 
	  customer.end());
  gen_in3["customer"] = customer_gen2;

  to_gen_t orders_gen1;
  orders_gen1.column = "o_custkey";
  orders_gen1.dbname = database;
  orders_gen1.scan_tables.insert(orders_gen1.scan_tables.end(), 
	  orders.begin(), orders.end());
  gen_in3["orders"] = orders_gen1;
  auto gen_zipped_map3 = p->Generalize(gen_in3, gen_level);
  /* END JOIN 3 ANON*/
  /* customer, order*/

  /* START JOIN 4 ANON*/
  unordered_map<table_name, to_gen_t> gen_in4;
  to_gen_t orders_gen2;
  orders_gen2.column = "o_orderkey";
  orders_gen2.dbname = database;
  orders_gen2.scan_tables.insert(orders_gen2.scan_tables.end(), gen_zipped_map3["orders"].begin(), 
	  gen_zipped_map3["orders"].end());
  gen_in4["orders"] = orders_gen2;

  to_gen_t lineitem_gen1;
  lineitem_gen1.column = "l_orderkey";
  lineitem_gen1.dbname = database;
  lineitem_gen1.scan_tables.insert(lineitem_gen1.scan_tables.end(), 
	  lineitem.begin(), lineitem.end());
  gen_in4["lineitem"] = lineitem_gen1;
  auto gen_zipped_map4 = p->Generalize(gen_in4, gen_level);
  /* order, lineitem*/
  /* END JOIN 4 ANON*/

  /* START JOIN 5 ANON*/
  /*
  unordered_map<table_name, to_gen_t> gen_in5;
  to_gen_t lineitem_gen2;
  lineitem_gen2.column = "l_suppkey";
  lineitem_gen2.dbname = database;
  lineitem_gen2.scan_tables.insert(lineitem_gen2.scan_tables.end(), gen_zipped_map4["lineitem"].begin(), 
	  gen_zipped_map4["lineitem"].end());
  gen_in5["lineitem"] = lineitem_gen2;

  /*
  to_gen_t supp_gen1;
  supp_gen1.column = "s_suppkey";
  supp_gen1.dbname = database;
  supp_gen1.scan_tables.insert(supp_gen1.scan_tables.end(), 
	  supplier.begin(), supplier.end());
  gen_in5["supplier"] = supp_gen1;
  auto gen_zipped_map5 = p->Generalize(gen_in5, gen_level);
  */
  /* lineitem*/
  /* END JOIN 5 ANON*/

  //END_AND_LOG_EXP_TPCH_TIMER(tpch_5_gen, gen_level);
  /****END GEN****/



  p->SetControlFlowColName("n_regionkey");
  //auto nation_repart = p->RepartitionJustHash(nation);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("r_regionkey");
  //auto region_repart = p->RepartitionJustHash(region);
  LOG(EXEC) << "JOIN 1 START==";
  // JOIN1
  // join def vitals-diagnoses
  START_TIMER(join_one);
  JoinDef jd_vd;
  jd_vd.set_l_col_name("n_regionkey");
  jd_vd.set_r_col_name("r_regionkey");
  jd_vd.set_project_len(2);
  // vitals-diagnoses-join project 1
  auto vdjp1 = jd_vd.add_project_list();
  vdjp1->set_side(JoinColID_RelationSide_LEFT);
  vdjp1->set_colname("n_nationkey");
  auto vdjp2 = jd_vd.add_project_list();
  vdjp2->set_side(JoinColID_RelationSide_LEFT);
  vdjp2->set_colname("n_name");
  auto to_join1 = zip_join_tables(nation, region);
  auto nr = p->Join(to_join1, jd_vd, sgx);
  LOG(EXEC) << "JOIN 1 END==";

  // JOIN2
  LOG(EXEC) << "JOIN 2 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("n_nationkey");
  auto nr_repart = p->RepartitionJustHash(nr);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_nationkey");
  auto customer_repart = p->RepartitionJustHash(gen_zipped_map3["customer"]);
  START_TIMER(join_two);
  JoinDef jd_vd2;
  jd_vd2.set_l_col_name("c_nationkey");
  jd_vd2.set_r_col_name("n_nationkey");
  // PROJECT
  jd_vd2.set_project_len(2);
  auto j2p1 = jd_vd2.add_project_list();
  j2p1->set_side(JoinColID_RelationSide_RIGHT);
  j2p1->set_colname("n_name");
  auto j2p2 = jd_vd2.add_project_list();
  j2p2->set_side(JoinColID_RelationSide_LEFT);
  j2p2->set_colname("c_custkey");
  auto to_join2 = zip_join_tables(customer_repart, nr_repart);
  auto cnr = p->Join(to_join2, jd_vd2, sgx);
  if (truncate) {
      auto sorted_cnr = p->Sort(cnr, sort, sgx);
      cnr = sorted_cnr;
  }
  LOG(EXEC) << "JOIN 2 END==";
  LOG(EXEC) << "JOIN 3 START==";

  // JOIN 3
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_custkey");
  auto orders_repart = p->RepartitionJustHash(gen_zipped_map4["orders"]);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_custkey");
  auto cnr_repart = p->RepartitionJustHash(cnr);
  JoinDef jd3;
  jd3.set_l_col_name("o_custkey");
  jd3.set_r_col_name("c_custkey");
  jd3.set_project_len(2);
  auto j3p1 = jd3.add_project_list();
  j3p1->set_side(JoinColID_RelationSide_RIGHT);
  j3p1->set_colname("n_name");
  auto j3p2 = jd3.add_project_list();
  j3p2->set_side(JoinColID_RelationSide_LEFT);
  j3p2->set_colname("o_orderkey");
  auto to_join3 = zip_join_tables(orders_repart, cnr_repart);
  auto ocnr = p->Join(to_join3, jd3, sgx);
  if (truncate) {
      auto sorted_ocnr = p->Sort(ocnr, sort, sgx);
      ocnr = sorted_ocnr;
  }
  LOG(EXEC) << "JOIN 3 END==";
  LOG(EXEC) << "JOIN 4 START==";

  // JOIN 4
  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_orderkey");
  auto lineitem_repart = p->RepartitionJustHash(gen_zipped_map4["lineitem"]);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_orderkey");
  auto ocnr_repart = p->RepartitionJustHash(ocnr);

  JoinDef jd4;
  jd4.set_l_col_name("l_orderkey");
  jd4.set_r_col_name("o_orderkey");
  jd4.set_project_len(4);
  auto j4p1 = jd4.add_project_list();
  j4p1->set_colname("n_name");
  j4p1->set_side(JoinColID_RelationSide_RIGHT);
  auto j4p2 = jd4.add_project_list();
  j4p2->set_colname("l_suppkey");
  j4p2->set_side(JoinColID_RelationSide_LEFT);
  auto j4p3 = jd4.add_project_list();
  j4p3->set_colname("revenue");
  j4p3->set_side(JoinColID_RelationSide_LEFT);
  auto j4p4 = jd4.add_project_list();
  j4p4->set_colname("l_orderkey");
  j4p4->set_side(JoinColID_RelationSide_LEFT);
  auto to_join4 = zip_join_tables(lineitem_repart, ocnr_repart);
  auto locnr = p->Join(to_join4, jd4, sgx);
  if (truncate) {
      auto sorted_locnr = p->Sort(locnr, sort, sgx);
      locnr = sorted_locnr;
  }

  LOG(EXEC) << "JOIN 4 END==";
  LOG(EXEC) << "JOIN 5 START==";

  /*
  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_suppkey");
  auto locnr_repart = p->RepartitionJustHash(locnr);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("s_suppkey");
  auto supp_repart = p->RepartitionJustHash(gen_zipped_map5["supplier"]);
  */

  JoinDef jd5;
  jd5.set_l_col_name("s_suppkey");
  jd5.set_r_col_name("l_suppkey");
  jd5.set_project_len(3);
  auto j5p1 = jd5.add_project_list();
  j5p1->set_colname("n_name");
  j5p1->set_side(JoinColID_RelationSide_RIGHT);
  auto j5p2 = jd5.add_project_list();
  j5p2->set_colname("revenue");
  j5p2->set_side(JoinColID_RelationSide_RIGHT);
  auto j5p3 = jd5.add_project_list();
  j5p3->set_colname("l_orderkey");
  j5p3->set_side(JoinColID_RelationSide_RIGHT);
  // auto to_join5 = zip_join_tables(supplier, locnr);
  auto to_join5 = zip_join_tables(supplier, locnr);
  auto slocnr = p->Join(to_join5, jd5, sgx);
  if (truncate) {
      auto sorted_slocnr = p->Sort(slocnr, sort, sgx);
      slocnr = sorted_slocnr;
  }
  LOG(EXEC) << "JOIN 5 END==";

  LOG(EXEC) << "AGGREGATE START";
  GroupByDef gbd;
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_secure(true);
  gbd.set_col_name("revenue");
  gbd.add_gb_col_names("n_name");
  gbd.set_kanon_col_name("l_orderkey");
  auto agg_out = p->Aggregate(slocnr, gbd, sgx);
  if(truncate) {
      END_AND_LOG_EXP_TPCH_TIMER(tpch_5_full_truncate, gen_level);
  } else {
      END_AND_LOG_EXP_TPCH_TIMER(tpch_5_full_no_truncate, gen_level);
  }
  // TODO(madhavsuresh): merge all of the aggregates together.
  // TODO(madhavsuresh): add sort
}

void tpch_5_obli(HonestBrokerPrivate *p, std::string database, bool sgx, bool truncate) {

  LOG(EXEC) << "STARTING TPCH-5 ENCRYPTED DISTRIBUTED";
  SortDef sort;
  sort.set_sorting_dummies(true);
  sort.set_truncate(true);
  START_TIMER(tpch_5_full_truncate);
  START_TIMER(postgres_read);
  auto customer = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT c_custkey, c_nationkey FROM customer");
  auto orders = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT o_custkey, o_orderkey FROM orders WHERE o_orderdate "
      ">='1993-01-01' AND o_orderdate < '1994-01-01'");
  auto lineitem = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT l_orderkey, l_suppkey, l_extendedprice*(1-l_discount) "
      "as revenue FROM lineitem");

  auto supplier = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT s_suppkey, s_nationkey FROM supplier");
  auto nation = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT n_name, n_regionkey, n_nationkey FROM nation");
  auto region = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT r_regionkey FROM region WHERE r_name= 'AFRICA'");

  p->SetControlFlowColName("n_regionkey");
  //auto nation_repart = p->RepartitionJustHash(nation);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("r_regionkey");
  //auto region_repart = p->RepartitionJustHash(region);
  LOG(EXEC) << "JOIN 1 START==";
  // JOIN1
  // join def vitals-diagnoses
  START_TIMER(join_one);
  JoinDef jd_vd;
  jd_vd.set_l_col_name("n_regionkey");
  jd_vd.set_r_col_name("r_regionkey");
  jd_vd.set_project_len(2);
  // vitals-diagnoses-join project 1
  auto vdjp1 = jd_vd.add_project_list();
  vdjp1->set_side(JoinColID_RelationSide_LEFT);
  vdjp1->set_colname("n_nationkey");
  auto vdjp2 = jd_vd.add_project_list();
  vdjp2->set_side(JoinColID_RelationSide_LEFT);
  vdjp2->set_colname("n_name");
  auto to_join1 = zip_join_tables(nation, region);
  auto nr = p->Join(to_join1, jd_vd, sgx);
  LOG(EXEC) << "JOIN 1 END==";
  //auto sorted_nr = p->Sort(nr, sort, sgx);

  // JOIN2
  LOG(EXEC) << "JOIN 2 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("n_nationkey");
  p->MakeObli(nr, "n_nationkey");
  //auto nr_repart = p->RepartitionJustHash(sorted_nr);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_nationkey");
  p->MakeObli(customer, "c_nationkey");
  auto customer_repart = p->RepartitionJustHash(customer);
  START_TIMER(join_two);
  JoinDef jd_vd2;
  jd_vd2.set_l_col_name("c_nationkey");
  jd_vd2.set_r_col_name("n_nationkey");
  // PROJECT
  jd_vd2.set_project_len(2);
  auto j2p1 = jd_vd2.add_project_list();
  j2p1->set_side(JoinColID_RelationSide_RIGHT);
  j2p1->set_colname("n_name");
  auto j2p2 = jd_vd2.add_project_list();
  j2p2->set_side(JoinColID_RelationSide_LEFT);
  j2p2->set_colname("c_custkey");
  auto to_join2 = zip_join_tables(customer_repart, nr);
  auto cnr = p->Join(to_join2, jd_vd2, sgx);
  LOG(EXEC) << "JOIN 2 END==";
  LOG(EXEC) << "JOIN 3 START==";
  if (truncate) {
      auto sorted_cnr = p->Sort(cnr, sort, sgx);
      cnr = sorted_cnr;
  }

  // JOIN 3
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_custkey");
  p->MakeObli(orders, "o_custkey");
  auto orders_repart = p->RepartitionJustHash(orders);
  p->ResetControlFlowCols();
  p->MakeObli(cnr, "c_custkey");
  p->SetControlFlowColName("c_custkey");
  auto cnr_repart = p->RepartitionJustHash(cnr);
  JoinDef jd3;
  jd3.set_l_col_name("o_custkey");
  jd3.set_r_col_name("c_custkey");
  jd3.set_project_len(2);
  auto j3p1 = jd3.add_project_list();
  j3p1->set_side(JoinColID_RelationSide_RIGHT);
  j3p1->set_colname("n_name");
  auto j3p2 = jd3.add_project_list();
  j3p2->set_side(JoinColID_RelationSide_LEFT);
  j3p2->set_colname("o_orderkey");
  auto to_join3 = zip_join_tables(orders_repart, cnr_repart);
  auto ocnr = p->Join(to_join3, jd3, sgx);
  if (truncate) {
      auto sorted_ocnr = p->Sort(ocnr, sort, sgx);
      ocnr = sorted_ocnr;
  }

  LOG(EXEC) << "JOIN 3 END==";
  LOG(EXEC) << "JOIN 4 START==";
  // JOIN 4
  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_orderkey");
  p->MakeObli(lineitem, "l_orderkey");
  auto lineitem_repart = p->RepartitionJustHash(lineitem);
  p->ResetControlFlowCols();
  p->MakeObli(ocnr, "o_orderkey");
  p->SetControlFlowColName("o_orderkey");
  //auto ocnr_repart = p->RepartitionJustHash(ocnr);

  JoinDef jd4;
  jd4.set_l_col_name("l_orderkey");
  jd4.set_r_col_name("o_orderkey");
  jd4.set_project_len(3);
  auto j4p1 = jd4.add_project_list();
  j4p1->set_colname("n_name");
  j4p1->set_side(JoinColID_RelationSide_RIGHT);
  auto j4p2 = jd4.add_project_list();
  j4p2->set_colname("l_suppkey");
  j4p2->set_side(JoinColID_RelationSide_LEFT);
  auto j4p3 = jd4.add_project_list();
  j4p3->set_colname("revenue");
  j4p3->set_side(JoinColID_RelationSide_LEFT);
  auto to_join4 = zip_join_tables(lineitem_repart, ocnr);
  auto locnr = p->Join(to_join4, jd4, sgx);
  if (truncate) {
      auto sorted_locnr = p->Sort(locnr, sort, sgx);
      locnr = sorted_locnr;
  }

  LOG(EXEC) << "JOIN 4 END==";
  LOG(EXEC) << "JOIN 5 START==";

  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_suppkey");
  //p->MakeObli(locnr, "l_suppkey");
  /*
  auto locnr_repart = p->RepartitionJustHash(locnr);
  p->ResetControlFlowCols();
  p->MakeObli(supplier, "s_suppkey");
  p->SetControlFlowColName("s_suppkey");
  auto supp_repart = p->RepartitionJustHash(supplier);
  */

  JoinDef jd5;
  jd5.set_l_col_name("s_suppkey");
  jd5.set_r_col_name("l_suppkey");
  jd5.set_project_len(3);
  auto j5p1 = jd5.add_project_list();
  j5p1->set_colname("n_name");
  j5p1->set_side(JoinColID_RelationSide_RIGHT);
  auto j5p2 = jd5.add_project_list();
  j5p2->set_colname("revenue");
  j5p2->set_side(JoinColID_RelationSide_RIGHT);
  auto j5p3 = jd5.add_project_list();
  j5p3->set_colname("l_suppkey");
  j5p3->set_side(JoinColID_RelationSide_RIGHT);
  // auto to_join5 = zip_join_tables(supplier, locnr);
  auto to_join5 = zip_join_tables(supplier, locnr);
  auto slocnr = p->Join(to_join5, jd5, sgx);
  LOG(EXEC) << "JOIN 5 END==";
  if (truncate) {
      auto sorted_slocnr = p->Sort(slocnr, sort, sgx);
      slocnr = sorted_slocnr;
  }

  LOG(EXEC) << "AGGREGATE START";
  GroupByDef gbd;
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_secure(true);
  gbd.set_col_name("revenue");
  gbd.add_gb_col_names("n_name");
  gbd.set_kanon_col_name("l_suppkey");
  auto agg_out = p->Aggregate(slocnr, gbd, sgx);
  // TODO(madhavsuresh): merge all of the aggregates together.
  END_AND_LOG_EXP_TPCH_TIMER(tpch_5_full_truncate, -1);
  // TODO(madhavsuresh): add sort
  // 
}

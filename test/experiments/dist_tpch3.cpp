//
// Created by madhav on 11/29/18.
//
#include "distributed_aspirin_profile.h"
#include "logger/LoggerDefs.h"
#include <gflags/gflags.h>

void tpch_3_encrypted(HonestBrokerPrivate *p, std::string database, bool sgx) {

  LOG(EXEC) << "STARTING TPCH-3 ENCRYPTED DISTRIBUTED";
  START_TIMER(tpch_3_full);
  START_TIMER(postgres_read);
  unordered_map<table_name, to_gen_t> gen_in;
  auto lineitem = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT l_orderkey, l_extendedprice *(1 - l_discount) as revenue FROM "
      "lineitem where l_shipdate > '1995-03-22'");
  auto customer = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT c_custkey FROM customer WHERE c_mktsegment='BUILDING'");
  auto orders = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT o_orderdate, o_shippriority, o_custkey, o_orderkey from orders "
      "WHERE o_orderdate <'1995-03-22'");

  START_TIMER(repartition);
  p->SetControlFlowColName("o_custkey");
  auto orders_repart = p->RepartitionJustHash(orders);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_custkey");
  auto cust_repart = p->RepartitionJustHash(customer);

  // join def vitals-diagnoses
  START_TIMER(join_one);
  JoinDef jd_vd;
  jd_vd.set_l_col_name("o_custkey");
  jd_vd.set_r_col_name("c_custkey");
  jd_vd.set_project_len(3);
  // vitals-diagnoses-join project 1
  auto vdjp1 = jd_vd.add_project_list();
  vdjp1->set_side(JoinColID_RelationSide_LEFT);
  vdjp1->set_colname("o_orderkey");
  auto vdjp2 = jd_vd.add_project_list();
  vdjp2->set_side(JoinColID_RelationSide_LEFT);
  vdjp2->set_colname("o_orderdate");
  auto vdjp3 = jd_vd.add_project_list();
  vdjp3->set_side(JoinColID_RelationSide_LEFT);
  vdjp3->set_colname("o_shippriority");
  auto to_join1 = zip_join_tables(orders_repart, cust_repart);
  auto out_oc_join = p->Join(to_join1, jd_vd, sgx);

  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_orderkey");
  auto oc_join_repart = p->RepartitionJustHash(out_oc_join);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_orderkey");
  auto lineitem_repart = p->RepartitionJustHash(lineitem);
  START_TIMER(join_two);
  JoinDef jd_vd2;
  jd_vd2.set_l_col_name("o_orderkey");
  jd_vd2.set_r_col_name("l_orderkey");
  // PROJECT
  jd_vd2.set_project_len(4);
  auto j2p1 = jd_vd2.add_project_list();
  j2p1->set_side(JoinColID_RelationSide_RIGHT);
  j2p1->set_colname("l_orderkey");
  auto j2p2 = jd_vd2.add_project_list();
  j2p2->set_side(JoinColID_RelationSide_RIGHT);
  j2p2->set_colname("revenue");
  auto j2p3 = jd_vd2.add_project_list();
  j2p3->set_side(JoinColID_RelationSide_LEFT);
  j2p3->set_colname("o_orderdate");
  auto j2p4 = jd_vd2.add_project_list();
  j2p4->set_side(JoinColID_RelationSide_LEFT);
  j2p4->set_colname("o_shippriority");
  auto to_join2 = zip_join_tables(oc_join_repart, lineitem_repart);
  auto out_loc_join = p->Join(to_join2, jd_vd2, sgx);

  GroupByDef gbd;
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_secure(true);
  gbd.set_col_name("revenue");
  gbd.add_gb_col_names("l_orderkey");
  gbd.add_gb_col_names("o_orderdate");
  gbd.add_gb_col_names("o_shippriority");
  gbd.set_kanon_col_name("l_orderkey");
  auto agg_out = p->Aggregate(out_loc_join, gbd, sgx);
  // TODO(madhavsuresh): merge all of the aggregates together.
  // TODO(madhavsuresh): add sort
}

void tpch_3_gen(HonestBrokerPrivate *p, std::string database, bool sgx, int gen_level) {

  LOG(EXEC) << "STARTING TPCH-3 ENCRYPTED DISTRIBUTED";
  START_TIMER(tpch_3_full);
  START_TIMER(postgres_read);
  auto lineitem = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT l_orderkey, l_extendedprice *(1 - l_discount) as revenue FROM "
      "lineitem where l_shipdate > '1995-03-22'");
  auto customer = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT c_custkey FROM customer WHERE c_mktsegment='BUILDING'");
  auto orders = p->ClusterDBMSQuery(
      "dbname=" + database,
      "SELECT o_orderdate, o_shippriority, o_custkey, o_orderkey from orders "
      "WHERE o_orderdate <'1995-03-22'");


  /* ANON JOIN 1*/
  unordered_map<table_name, to_gen_t> gen_in;
  to_gen_t customer_gen;
  customer_gen.column = "c_custkey";
  customer_gen.dbname = "tpch";
  customer_gen.scan_tables.insert(customer_gen.scan_tables.end(), customer.begin(), customer.end());
  gen_in["customer"] = customer_gen;
  to_gen_t orders_gen;
  orders_gen.column = "o_custkey";
  orders_gen.dbname = "tpch";
  orders_gen.scan_tables.insert(orders_gen.scan_tables.end(), orders.begin(), orders.end());
  gen_in["orders"] = orders_gen;
  auto gen_zipped_map = p->Generalize(gen_in, 5);

  /* ANON JOIN 2*/
  unordered_map<table_name, to_gen_t> gen_in2;
  to_gen_t lineitem_gen;
  lineitem_gen.column = "l_orderkey";
  lineitem_gen.dbname = "tpch";
  lineitem_gen.scan_tables.insert(lineitem_gen.scan_tables.end(), lineitem.begin(), lineitem.end());
  gen_in2["lineitem"] = lineitem_gen;
  to_gen_t orders2_gen;
  orders2_gen.column = "o_orderkey";
  orders2_gen.dbname = "tpch";
  orders2_gen.scan_tables.insert(orders2_gen.scan_tables.end(), orders.begin(), orders.end());
  gen_in2["orders"] = orders2_gen;
  auto gen_zipped_mapJ2 = p->Generalize(gen_in2, 5);


  START_TIMER(repartition);
  p->SetControlFlowColName("o_custkey");
  auto orders_repart = p->RepartitionJustHash(gen_zipped_mapJ2["orders"]);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_custkey");
  auto cust_repart = p->RepartitionJustHash(gen_zipped_map["customer"]);

  /*
  // join def vitals-diagnoses
  START_TIMER(join_one);
  JoinDef jd_vd;
  jd_vd.set_l_col_name("o_custkey");
  jd_vd.set_r_col_name("c_custkey");
  jd_vd.set_project_len(3);
  // vitals-diagnoses-join project 1
  auto vdjp1 = jd_vd.add_project_list();
  vdjp1->set_side(JoinColID_RelationSide_LEFT);
  vdjp1->set_colname("o_orderkey");
  auto vdjp2 = jd_vd.add_project_list();
  vdjp2->set_side(JoinColID_RelationSide_LEFT);
  vdjp2->set_colname("o_orderdate");
  auto vdjp3 = jd_vd.add_project_list();
  vdjp3->set_side(JoinColID_RelationSide_LEFT);
  vdjp3->set_colname("o_shippriority");
  auto to_join1 = zip_join_tables(orders_repart, cust_repart);
  auto out_oc_join = p->Join(to_join1, jd_vd, sgx);

  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_orderkey");
  auto oc_join_repart = p->RepartitionJustHash(out_oc_join);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_orderkey");
  auto lineitem_repart = p->RepartitionJustHash(gen_zipped_mapJ2["lineitem"]);
  START_TIMER(join_two);
  JoinDef jd_vd2;
  jd_vd2.set_l_col_name("o_orderkey");
  jd_vd2.set_r_col_name("l_orderkey");
  // PROJECT
  jd_vd2.set_project_len(4);
  auto j2p1 = jd_vd2.add_project_list();
  j2p1->set_side(JoinColID_RelationSide_RIGHT);
  j2p1->set_colname("l_orderkey");
  auto j2p2 = jd_vd2.add_project_list();
  j2p2->set_side(JoinColID_RelationSide_RIGHT);
  j2p2->set_colname("revenue");
  auto j2p3 = jd_vd2.add_project_list();
  j2p3->set_side(JoinColID_RelationSide_LEFT);
  j2p3->set_colname("o_orderdate");
  auto j2p4 = jd_vd2.add_project_list();
  j2p4->set_side(JoinColID_RelationSide_LEFT);
  j2p4->set_colname("o_shippriority");
  auto to_join2 = zip_join_tables(oc_join_repart, lineitem_repart);
  auto out_loc_join = p->Join(to_join2, jd_vd2, sgx);

  GroupByDef gbd;
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_secure(true);
  gbd.set_col_name("revenue");
  gbd.add_gb_col_names("l_orderkey");
  gbd.add_gb_col_names("o_orderdate");
  gbd.add_gb_col_names("o_shippriority");
  gbd.set_kanon_col_name("l_orderkey");
  auto agg_out = p->Aggregate(out_loc_join, gbd, sgx);
  // TODO(madhavsuresh): merge all of the aggregates together.
  // TODO(madhavsuresh): add sort
  // */
}

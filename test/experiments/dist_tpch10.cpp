//
// Created by madhav on 11/29/18.
//
#include "distributed_aspirin_profile.h"
#include "logger/LoggerDefs.h"
#include <gflags/gflags.h>
#include <chrono>
#include <thread>


void tpch_10_encrypted(HonestBrokerPrivate *p, std::string database, bool sgx) {

  LOG(EXEC) << "STARTING TPCH-10 ENCRYPTED DISTRIBUTED";
  START_TIMER(tpch_10_full);
  START_TIMER(postgres_read);
  auto lineitem = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT l_orderkey, l_extendedprice*(1-l_discount) "
                "as revenue FROM lineitem");
  auto orders =
      p->ClusterDBMSQuery("dbname="+database, 
	      "SELECT o_custkey, o_orderkey from orders WHERE o_orderdate "
                "<'1995-10-01' AND o_orderdate >= '1993-07-01'");
  auto customer =
      p->ClusterDBMSQuery("dbname="+database, "SELECT c_custkey, c_name, c_acctbal, "
	  "c_phone, c_address, c_comment, c_nationkey FROM customer");
  auto nation =
      p->ClusterDBMSQuery("dbname="+database, "SELECT n_name, n_nationkey FROM nation");

  //JOIN1
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

  //JOIN2
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
  auto to_join2= zip_join_tables(lo_repart, cust_repart);
  auto loc = p->Join(to_join2, jd2, sgx);
  LOG(EXEC) << "JOIN 2 END==";


  //JOIN3
  LOG(EXEC) << "JOIN 3 START==";
  p->ResetControlFlowCols();
  p->SetControlFlowColName("c_nationkey");
  auto loc_repart = p->RepartitionJustHash(loc);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("n_nationkey");
  auto nation_repart = p->RepartitionJustHash(nation);

  JoinDef jd3;
  jd3.set_l_col_name("c_nationkey");
  jd3.set_r_col_name("n_nationkey");
  jd3.set_project_len(8);
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
  auto to_join3= zip_join_tables(loc_repart, nation_repart);
  auto locn = p->Join(to_join3, jd3, sgx);
}

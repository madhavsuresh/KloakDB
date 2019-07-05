//
// Created by madhav on 11/29/18.
//
#include "distributed_aspirin_profile.h"
#include "logger/LoggerDefs.h"
#include <gflags/gflags.h>
#include <chrono>
#include <thread>


void tpch_5_encrypted(HonestBrokerPrivate *p, std::string database, bool sgx) {

  LOG(EXEC) << "STARTING TPCH-5 ENCRYPTED DISTRIBUTED";
  START_TIMER(tpch_5_full);
  START_TIMER(postgres_read);
  auto customer =
      p->ClusterDBMSQuery("dbname="+database, "SELECT c_custkey, c_nationkey FROM customer");
  auto orders =
      p->ClusterDBMSQuery("dbname="+database, "SELECT o_custkey, o_orderkey FROM orders WHERE o_orderdate "
                ">='1993-01-01' AND o_orderdate < '1994-01-01'");
  auto lineitem = p->ClusterDBMSQuery(
      "dbname=" + database, "SELECT l_orderkey, l_suppkey, l_extendedprice*(1-l_discount) "
                "as revenue FROM lineitem");

  auto supplier =
      p->ClusterDBMSQuery("dbname="+database, "SELECT s_suppkey, s_nationkey FROM supplier");
  auto nation =
      p->ClusterDBMSQuery("dbname="+database, "SELECT n_name, n_regionkey, n_nationkey FROM nation");
  auto region = p->ClusterDBMSQuery("dbname="+database,
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
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  //JOIN2
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
  //PROJECT
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
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  LOG(EXEC) << "JOIN 3 START==";

  //JOIN 3
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
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  LOG(EXEC) << "JOIN 4 START==";
  //JOIN 4
  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_orderkey");
  auto lineitem_repart = p->RepartitionJustHash(lineitem);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("o_orderkey");
  auto ocnr_repart = p->RepartitionJustHash(ocnr);

  JoinDef jd4;
  jd4.set_l_col_name("l_orderkey");
  jd4.set_r_col_name("o_orderkey");
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
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  LOG(EXEC) << "JOIN 5 START==";

  p->ResetControlFlowCols();
  p->SetControlFlowColName("l_suppkey");
//  auto locnr_repart = p->RepartitionJustHash(locnr);
  p->ResetControlFlowCols();
  p->SetControlFlowColName("s_suppkey");
  auto supp_repart = p->RepartitionJustHash(supplier);
  
  JoinDef jd5;
  jd5.set_l_col_name("s_suppkey");
  jd5.set_r_col_name("l_suppkey");
  jd5.set_project_len(2);
  auto j5p1 = jd5.add_project_list();
  j5p1->set_colname("n_name");
  j5p1->set_side(JoinColID_RelationSide_RIGHT);
  auto j5p2 = jd5.add_project_list();
  j5p2->set_colname("revenue");
  j5p2->set_side(JoinColID_RelationSide_RIGHT);
  //auto to_join5 = zip_join_tables(supp_repart, locnr_repart);
  auto to_join5 = zip_join_tables(supplier, locnr);
  auto slocnr = p->Join(to_join5, jd5, sgx);
  LOG(EXEC) << "JOIN 5 END==";
  
}

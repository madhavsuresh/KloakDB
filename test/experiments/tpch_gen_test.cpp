//
// Created by madhav on 12/23/18.
//

#include "dist_gen_test.h"
#include <VaultDB.h>
#include <logger/LoggerDefs.h>


void tpch_gen_test(HonestBrokerPrivate *p, std::string dbname, int k) {

  START_TIMER(end_to_end);
  auto customer = p->ClusterDBMSQuery("dbname=" + dbname, "SELECT c_custkey FROM customer");
  auto orders = p->ClusterDBMSQuery("dbname=" + dbname, "SELECT o_custkey, o_orderkey FROM orders");
  auto lineitem = p->ClusterDBMSQuery("dbname=" + dbname, "SELECT l_suppkey, l_orderkey FROM lineitem");
  auto supplier = p->ClusterDBMSQuery("dbname=" + dbname, "SELECT s_suppkey FROM supplier");

  /* ANON 0*/
  unordered_map<table_name, to_gen_t> gen_in0;
  to_gen_t customer_gen0;
  customer_gen0.column = "c_custkey";
  customer_gen0.dbname = dbname;
  customer_gen0.scan_tables.insert(customer_gen0.scan_tables.end(), customer.begin(), customer.end());
  gen_in0["customer"] = customer_gen0;
  auto gen_zipped_map0 = p->Generalize(gen_in0, k);
  customer = gen_zipped_map0["customer"];


  /*ANON 1*/
  unordered_map<table_name, to_gen_t> gen_in1;
  to_gen_t customer_gen1;
  customer_gen1.column = "c_custkey";
  customer_gen1.dbname = dbname;
  customer_gen1.scan_tables.insert(customer_gen1.scan_tables.end(), customer.begin(), customer.end());
  gen_in1["customer"] = customer_gen1;
  to_gen_t orders_gen1;
  orders_gen1.column = "o_custkey";
  orders_gen1.dbname = dbname;
  orders_gen1.scan_tables.insert(orders_gen1.scan_tables.end(), 
	  orders.begin(), orders.end());
  gen_in1["orders"] = orders_gen1;
  auto gen_zipped_map = p->Generalize(gen_in1, k);

  /*ANON 2*/
  unordered_map<table_name, to_gen_t> gen_in2;
  to_gen_t orders_gen2;
  orders_gen2.column = "o_orderkey";
  orders_gen2.dbname = dbname;
  orders_gen2.scan_tables.insert(orders_gen2.scan_tables.end(), 
	  gen_zipped_map["orders"].begin(), gen_zipped_map["orders"].end());
  gen_in2["orders"] = orders_gen2;
  to_gen_t lineitem_gen;
  lineitem_gen.column = "l_orderkey";
  lineitem_gen.dbname = dbname;
  lineitem_gen.scan_tables.insert(lineitem_gen.scan_tables.end(), lineitem.begin(), lineitem.end());
  gen_in2["lineitem"] = lineitem_gen;
  auto gen_zipped_map2 = p->Generalize(gen_in2, k);

  /*ANON 3*/
  unordered_map<table_name, to_gen_t> gen_in3;
  to_gen_t lineitem_gen2;
  lineitem_gen2.column = "l_suppkey";
  lineitem_gen2.dbname = dbname;
  lineitem_gen2.scan_tables.insert(lineitem_gen2.scan_tables.end(), 
	  gen_zipped_map2["lineitem"].begin(), gen_zipped_map2["lineitem"].end());
  gen_in2["lineitem"] = lineitem_gen2;
  to_gen_t supplier_gen;
  supplier_gen.column = "s_suppkey";
  supplier_gen.dbname = dbname;
  supplier_gen.scan_tables.insert(supplier_gen.scan_tables.end(), supplier.begin(), supplier.end());
  gen_in3["supplier"] = supplier_gen;
  auto gen_zipped_map3 = p->Generalize(gen_in3, k);
}

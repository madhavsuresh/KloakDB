//
// Created by madhav on 11/8/18.
//

#include <chrono>
#include <operators/HashJoin.h>
#include <operators/Generalize.h>
#include <gflags/gflags.h>
#include <rpc/HonestBrokerPrivate.h>
#include "data/postgres_client.h"
#include "data/pqxx_compat.h"
#include "exp3_reparition_scaling.h"


DEFINE_int32(exp3_data_size, 1000, "exp3: reparition Data Size");
DEFINE_int32(exp3_data_range, 1000, "exp3: reparition Data range");

void exp3_hb() {
  std::string xdbname = "kloak_experiments_exp3";
  std::string command = "createdb " + xdbname;
  system(command.c_str());
  std::string dbname = "dbname=" + xdbname;


  command = "dropdb " + xdbname;
  system(command.c_str());

  auto finish = std::chrono::high_resolution_clock::now();

}
std::string xdbname = "kloak_experiments_exp3";

void exp3_script_HB(HonestBrokerPrivate *p) {
  p->NumHosts();
  std::vector<std::shared_ptr<::vaultdb::TableID>> tids;
  for (int i = 0; i < p->NumHosts(); i++) {
    vaultdb::TableID t1 =
            p->DBMSQuery(i, "dbname=" +xdbname, "SELECT * from exp3_random;");
    tids.emplace_back(std::make_shared<::vaultdb::TableID>(t1));
  }
  p->SetControlFlowColID(1);
  auto repartition_ids = p->Repartition(tids);

}

void exp3_setup_do() {
  std::string command = "createdb " + xdbname;
  system(command.c_str());
  std::string dbname = "dbname=" + xdbname;

    query("DROP TABLE IF EXISTS exp3_random", dbname);
    query("CREATE TABLE exp3_random (a INT, b INT)", dbname);
    char buf[128];
    sprintf(buf,
            "INSERT into exp3_random (a,b) select i, floor(random() * %d+1)::int from generate_series(1, %d) s(i);",
            FLAGS_exp3_data_range, FLAGS_exp3_data_size);
    std::string query_create(buf);
    query(query_create, dbname);



}
 void exp3_teardown_do() {
   std::string command = "dropdb " + xdbname;
   system(command.c_str());
}

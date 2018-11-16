//
// Created by madhav on 9/25/18.
//

#include "VaultDB.h"
#include "rpc/DataOwnerImpl.h"
#include "rpc/DataOwnerPrivate.h"
#include "rpc/HonestBrokerClient.h"
#include "rpc/HonestBrokerImpl.h"
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <gflags/gflags.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_builder.h>
#include <thread>
#include "test/experiments/exp3_reparition_scaling.h"

DEFINE_bool(honest_broker, false, "Setup as honest broker");
DEFINE_string(address, "", "IPV4 Address for current instance");
DEFINE_string(
    honest_broker_address, "",
    "IPV4 Address for honest broker instance (set by non-honest broker)");

void runHonestBrokerServer(HonestBrokerPrivate *p) {
  HonestBrokerImpl hb(p);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(p->HostName(), grpc::InsecureServerCredentials());
  builder.RegisterService(&hb);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server->Wait();
}

void runDataOwnerServer(DataOwnerPrivate *p) {
  DataOwnerImpl d(p);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(p->HostName(), grpc::InsecureServerCredentials());
  builder.RegisterService(&d);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server->Wait();
}

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::unique_ptr<g3::LogWorker> logworker{g3::LogWorker::createLogWorker()};

  if (FLAGS_honest_broker == true) {
    auto defaultHandler = logworker->addDefaultLogger("HB", "logs");
    g3::initializeLogging(logworker.get());
    HonestBrokerPrivate *p = new HonestBrokerPrivate(FLAGS_address);
    std::thread hb_thread(runHonestBrokerServer, p);
    // Wait til all hosts are ready
    p->WaitForAllHosts();
    p->RegisterPeerHosts();
    exp3_script_HB(p);
    /*
    vaultdb::TableID t1 =
        p->DBMSQuery(0, "dbname=test", "SELECT * from rpc_test_small_5;");
    vaultdb::TableID t2 =
        p->DBMSQuery(1, "dbname=test", "SELECT * from rpc_test_small_5;");
    std::vector<std::shared_ptr<::vaultdb::TableID>> tids;
    p->SetControlFlowColID(1);
    tids.emplace_back(std::make_shared<::vaultdb::TableID>(t1));
    tids.emplace_back(std::make_shared<::vaultdb::TableID>(t2));
    auto repartition_ids = p->Repartition(tids);

    std::vector<std::pair<std::shared_ptr<const ::vaultdb::TableID>,
                          std::shared_ptr<const ::vaultdb::TableID>>>
        joinTables;
    for (auto &i : repartition_ids) {
      joinTables.emplace_back(std::make_pair(i, i));
    }
    ::vaultdb::JoinDef join;
    join.set_l_col(1);
    join.set_r_col(1);
    join.set_project_len(2);
    auto pl1 = join.add_project_list();
    pl1->set_side(::vaultdb::JoinColID_RelationSide_RIGHT);
    pl1->set_col_no(1);
    auto pl2 = join.add_project_list();
    pl2->set_side(::vaultdb::JoinColID_RelationSide_LEFT);
    pl2->set_col_no(0);
    auto j_output = p->Join(joinTables, join);

    ::vaultdb::GroupByDef gbd;
    gbd.set_col_no(0);
    gbd.set_type(::vaultdb::GroupByDef_GroupByType_COUNT);
    auto agg_output = p->Aggregate(j_output, gbd);

    ::vaultdb::SortDef sort;
    sort.set_colno(0);
    sort.set_ascending(true);
    auto sorted_ids = p->Sort(agg_output, sort);
    ::vaultdb::Expr exp;
    exp.set_colno(1);
    exp.set_type(::vaultdb::Expr_ExprType_EQ_EXPR);
    exp.mutable_desc()->set_field_type(::vaultdb::FieldDesc_FieldType_INT);
    exp.set_intfield(5);
    p->Filter(sorted_ids, exp);
     */
    hb_thread.join();
    exit(0);
  } else {
    exp3_setup_do();

    DataOwnerPrivate *p =
        new DataOwnerPrivate(FLAGS_address, FLAGS_honest_broker_address);
    std::thread do_thread(runDataOwnerServer, p);
    p->Register();
    auto defaultHandler = logworker->addDefaultLogger(
        "DO" + std::to_string(p->HostNum()), "logs");
    g3::initializeLogging(logworker.get());
    do_thread.join();
  }
}
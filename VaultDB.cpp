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
    getchar();
    p->RegisterPeerHosts();
    vaultdb::TableID t1 =
        p->DBMSQuery(0, "dbname=test", "SELECT * from rpc_test_small_5;");
    vaultdb::TableID t2 =
        p->DBMSQuery(1, "dbname=test", "SELECT * from rpc_test_small_5;");
    std::vector<std::reference_wrapper<::vaultdb::TableID>> tids;
    p->SetControlFlowColID(1);
    tids.emplace_back(t1);
    tids.emplace_back(t2);
    p->Repartition(tids);
    hb_thread.join();
  } else {
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
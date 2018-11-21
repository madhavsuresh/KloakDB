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
DEFINE_int32(experiment, 1, "experiment number");

std::promise<void> exit_requested;



int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::unique_ptr<g3::LogWorker> logworker{g3::LogWorker::createLogWorker()};

  if (FLAGS_honest_broker == true) {
    auto defaultHandler = logworker->addDefaultLogger("HB", "logs");
    g3::initializeLogging(logworker.get());
    HonestBrokerPrivate *p = new HonestBrokerPrivate(FLAGS_address);
    HonestBrokerImpl hb(p);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(p->HostName(), grpc::InsecureServerCredentials());
    builder.RegisterService(&hb);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    auto serveFn = [&]() {
        server->Wait();
    };
    std::thread hb_thread(serveFn);

    p->WaitForAllHosts();
    p->RegisterPeerHosts();
    p->Shutdown();
    switch (FLAGS_experiment) {
      case 1 : {

        break;
      }
      case 2 : {

        break;
      }
      case 3 : {

        break;
      }
      case 4 : {

        break;
      }
      case 5 : {

        break;
      }
      case 6 : {

        break;
      }
      case 7 : {
        break;
      }
      default: {
        printf("NOTHING HAPPENS HERE\n");
      }
    }

    server->Shutdown();
    delete p;
    hb_thread.join();
  } else {
    DataOwnerPrivate *p =
        new DataOwnerPrivate(FLAGS_address, FLAGS_honest_broker_address);
    p->Register();
    auto defaultHandler = logworker->addDefaultLogger(
            "DO" + std::to_string(p->HostNum()), "logs");
    g3::initializeLogging(logworker.get());

    DataOwnerImpl d(p);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(p->HostName(), grpc::InsecureServerCredentials());
    builder.RegisterService(&d);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    auto serveFn = [&]() {
      server->Wait();
    };
    std::thread do_thread(serveFn);
    auto f = exit_requested.get_future();
    f.wait();
    server->Shutdown();
    delete p;
    do_thread.join();
  }
}
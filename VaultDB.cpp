//
// Created by madhav on 9/25/18.
//

#include "VaultDB.h"
#include "rpc/HonestBrokerImpl.h"
#include "rpc/HonestBrokerClient.h"
#include "rpc/DataOwnerImpl.h"
#include "rpc/DataOwnerPrivate.h"
#include <gflags/gflags.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/grpcpp.h>
#include <thread>


DEFINE_bool(honest_broker, false, "Setup as honest broker");
DEFINE_string(address, "", "IPV4 Address for current instance");
DEFINE_string(honest_broker_address, "", "IPV4 Address for honest broker instance (set by non-honest broker)");

void runHonestBrokerServer(HonestBrokerPrivate *p){
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

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (FLAGS_honest_broker == true) {
        HonestBrokerPrivate *p = new HonestBrokerPrivate(FLAGS_address);
        std::thread hb_thread(runHonestBrokerServer, p);
        getchar();
        p->DBMSQuery(1, "dbname=test",
                "SELECT floor, COUNT(*) from rpc_test group by floor;");
        hb_thread.join();
    } else {
        DataOwnerPrivate * p = new DataOwnerPrivate(FLAGS_address,
                                                    FLAGS_honest_broker_address);
        std::thread do_thread(runDataOwnerServer, p);
        p->Register();
        do_thread.join();
    }
}
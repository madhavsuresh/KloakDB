//
// Created by madhav on 10/1/18.
//

#ifndef PROJECT_DATAOWNERCLIENT_H
#define PROJECT_DATAOWNERCLIENT_H


#include <string>
#include "vaultdb.grpc.pb.h"
#include "../postgres_client.h"
#include <grpcpp/grpcpp.h>

class DataOwnerClient {

public:
    DataOwnerClient(std::shared_ptr<grpc::Channel> channel)
    :stub_(vaultdb::DataOwner::NewStub(channel)) {}

    int DBMSQuery(std::string dbname, std::string query);

    int SendTable(table_t * t);
private:
    std::unique_ptr<vaultdb::DataOwner::Stub> stub_;

};


#endif //PROJECT_DATAOWNERCLIENT_H

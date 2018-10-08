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

    ::vaultdb::TableID DBMSQuery(std::string dbname, std::string query);

    std::vector<::vaultdb::TableID> RepartitionStepOne(::vaultdb::TableID tid);
    std::vector<::vaultdb::TableID> RepartitionStepTwo(std::vector<::vaultdb::TableID> table_fragments);

    //TODO(madhavsuresh): this needs to be renamed to be consistent
    int SendTable(table_t * t);
private:
    int host_num;
    std::unique_ptr<vaultdb::DataOwner::Stub> stub_;

};


#endif //PROJECT_DATAOWNERCLIENT_H

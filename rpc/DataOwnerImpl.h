//
// Created by madhav on 10/1/18.
//

#ifndef PROJECT_DATAOWNERIMPL_H
#define PROJECT_DATAOWNERIMPL_H


#include "vaultdb.grpc.pb.h"
#include "DataOwnerPrivate.h"

class DataOwnerImpl final : public vaultdb::DataOwner::Service {

public:
    DataOwnerImpl(DataOwnerPrivate *p);

   ::grpc::Status
   GetPeerHosts(::grpc::ServerContext* context, const ::vaultdb::GetPeerHostsRequest* request,
            ::vaultdb::GetPeerHostsResponse* response);

    ::grpc::Status
    DBMSQuery(::grpc::ServerContext* context,
            const ::vaultdb::DBMSQueryRequest* request,
            ::vaultdb::DBMSQueryResponse* response);

    ::grpc::Status
    SendTable(::grpc::ServerContext* context, ::grpc::ServerReader< ::vaultdb::SendTableRequest>* reader,
            ::vaultdb::SendTableResponse* response);

    ::grpc::Status
    RepartitionStepOne(::grpc::ServerContext* context, const ::vaultdb::RepartitionStepOneRequest* request,
            ::vaultdb::RepartitionStepOneResponse* response);

    ::grpc::Status
    RepartitionStepTwo(::grpc::ServerContext* context, const ::vaultdb::RepartitionStepTwoRequest* request,
                       ::vaultdb::RepartitionStepTwoResponse* response);
private:
    DataOwnerPrivate *p;
};


#endif //PROJECT_DATAOWNERIMPL_H
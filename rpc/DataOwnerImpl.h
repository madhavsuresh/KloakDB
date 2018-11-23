//
// Created by madhav on 10/1/18.
//

#ifndef PROJECT_DATAOWNERIMPL_H
#define PROJECT_DATAOWNERIMPL_H

#include "DataOwnerPrivate.h"
#include "vaultdb.grpc.pb.h"

class DataOwnerImpl final : public vaultdb::DataOwner::Service {

public:
  DataOwnerImpl(DataOwnerPrivate *p);

  ::grpc::Status ShutDown(::grpc::ServerContext *context,
                          const ::vaultdb::ShutDownRequest *request,
                          ::vaultdb::ShutDownResponse *response);

  ::grpc::Status GetPeerHosts(::grpc::ServerContext *context,
                              const ::vaultdb::GetPeerHostsRequest *request,
                              ::vaultdb::GetPeerHostsResponse *response);

  ::grpc::Status DBMSQuery(::grpc::ServerContext *context,
                           const ::vaultdb::DBMSQueryRequest *request,
                           ::vaultdb::DBMSQueryResponse *response);

  ::grpc::Status
  GetTable(::grpc::ServerContext *context,
           const ::vaultdb::GetTableRequest *request,
           ::grpc::ServerWriter<::vaultdb::GetTableResponse> *writer);

  ::grpc::Status GeneralizeZip(::grpc::ServerContext* context, const ::vaultdb::GeneralizeZipRequest* request,
                                 ::vaultdb::GeneralizeZipResponse* response);
  ::grpc::Status
  SendTable(::grpc::ServerContext *context,
            ::grpc::ServerReader<::vaultdb::SendTableRequest> *reader,
            ::vaultdb::SendTableResponse *response);

  ::grpc::Status
  RepartitionStepOne(::grpc::ServerContext *context,
                     const ::vaultdb::RepartitionStepOneRequest *request,
                     ::vaultdb::RepartitionStepOneResponse *response);

  ::grpc::Status
  RepartitionStepTwo(::grpc::ServerContext *context,
                     const ::vaultdb::RepartitionStepTwoRequest *request,
                     ::vaultdb::RepartitionStepTwoResponse *response);

  ::grpc::Status CoalesceTables(::grpc::ServerContext *context,
                                const ::vaultdb::CoaleseTablesRequest *request,
                                ::vaultdb::CoaleseTablesResponse *response);

  ::grpc::Status KFilter(::grpc::ServerContext *context,
                         const ::vaultdb::KFilterRequest *request,
                         ::vaultdb::KFilterResponse *response);

  ::grpc::Status KSort(::grpc::ServerContext *context,
                       const ::vaultdb::KSortRequest *request,
                       ::vaultdb::KSortResponse *response);

  ::grpc::Status KJoin(::grpc::ServerContext *context,
                       const ::vaultdb::KJoinRequest *request,
                       ::vaultdb::KJoinResponse *response);

  ::grpc::Status KAggregate(::grpc::ServerContext *context,
                            const ::vaultdb::KAggregateRequest *request,
                            ::vaultdb::KAggregateResponse *response);

private:
  DataOwnerPrivate *p;
};

#endif // PROJECT_DATAOWNERIMPL_H

//
// Created by madhav on 9/28/18.
//

#ifndef PROJECT_HONESTBROKER_H
#define PROJECT_HONESTBROKER_H

#include "HonestBrokerPrivate.h"
#include "vaultdb.grpc.pb.h"

class HonestBrokerImpl final : public vaultdb::HonestBroker::Service {
public:
  HonestBrokerImpl(HonestBrokerPrivate *p);
  ::grpc::Status NumHosts(::grpc::ServerContext *context,
                          const ::vaultdb::NumHostsRequest *request,
                          ::vaultdb::NumHostResp *response);

  ::grpc::Status Register(::grpc::ServerContext *context,
                          const ::vaultdb::RegisterRequest *request,
                          ::vaultdb::RegisterResponse *response);

  ::grpc::Status
  GetControlFlowColumn(::grpc::ServerContext *context,
                       const ::vaultdb::GetControlFlowColumnRequest *request,
                       ::vaultdb::GetControlFlowColumnResponse *response);
  void Start(std::string);

private:
  HonestBrokerPrivate *p;
};

#endif // PROJECT_HONESTBROKER_H

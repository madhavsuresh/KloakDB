//
// Created by madhav on 10/1/18.
//

#ifndef PROJECT_HONESTBROKERCLIENT_H
#define PROJECT_HONESTBROKERCLIENT_H

//#include "vaultdb.grpc.pb.h"
#include <grpcpp/grpcpp.h>

class HonestBrokerClient {
public:
  HonestBrokerClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(vaultdb::HonestBroker::NewStub(channel)) {}

  int GetNumHosts();
  ::vaultdb::ControlFlowColumn GetControlFlowColID();
  int Register(std::string hostname);

private:
  std::unique_ptr<vaultdb::HonestBroker::Stub> stub_;
};

#endif // PROJECT_HONESTBROKERCLIENT_H

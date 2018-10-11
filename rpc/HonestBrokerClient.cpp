//
// Created by madhav on 10/1/18.
//

#include "HonestBrokerClient.h"
#include "vaultdb.grpc.pb.h"
#include "vaultdb.pb.h"

::vaultdb::ControlFlowColumn HonestBrokerClient::GetControlFlowColID() {
  vaultdb::GetControlFlowColumnRequest req;
  vaultdb::GetControlFlowColumnResponse resp;
  grpc::ClientContext context;

  auto status = stub_->GetControlFlowColumn(&context, req, &resp);
  if (status.ok()) {
    return resp.cf();
  } else {
    std::cerr << status.error_code() << ": " << status.error_message()
              << std::endl;
    throw;
  }
}

int HonestBrokerClient::GetNumHosts() {
  vaultdb::NumHostsRequest req;
  vaultdb::NumHostResp resp;
  grpc::ClientContext context;

  auto status = stub_->NumHosts(&context, req, &resp);

  if (status.ok()) {
    return resp.num_hosts();
  } else {
    std::cerr << status.error_code() << ": " << status.error_message()
              << std::endl;
    throw;
  }
}

int HonestBrokerClient::Register(std::string hostname) {
  vaultdb::RegisterRequest req;
  vaultdb::RegisterResponse resp;
  grpc::ClientContext context;

  req.set_hostname(hostname);

  auto status = stub_->Register(&context, req, &resp);
  if (status.ok()) {
    return resp.host_num();
  } else {
    std::cerr << status.error_code() << ": " << status.error_message()
              << std::endl;
    throw;
  }
}
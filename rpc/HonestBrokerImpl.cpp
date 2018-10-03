//
// Created by madhav on 9/28/18.
//

#include "HonestBrokerImpl.h"
#include "HonestBrokerPrivate.h"

HonestBrokerImpl::HonestBrokerImpl(HonestBrokerPrivate *p) {
    this->p = p;
}

::grpc::Status
HonestBrokerImpl::NumHosts(::grpc::ServerContext* context, const ::vaultdb::NumHostsRequest* request,
                           ::vaultdb::NumHostResp* response) {
    response->set_num_hosts(this->p->NumHosts());
    return ::grpc::Status::OK;
}

::grpc::Status
HonestBrokerImpl::Register(::grpc::ServerContext* context, const ::vaultdb::RegisterRequest* request,
                           ::vaultdb::RegisterResponse* response) {
    int num = this->p->RegisterHost(request->hostname());
    response->set_host_num(num);
    return ::grpc::Status::OK;
}



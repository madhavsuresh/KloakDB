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
    printf("Registering Host: %d, %s\n", num, request->hostname().c_str());
    response->set_host_num(num);
    return ::grpc::Status::OK;
}


::grpc::Status
HonestBrokerImpl::GetControlFlowColumn(::grpc::ServerContext* context,
        const ::vaultdb::GetControlFlowColumnRequest* request, ::vaultdb::GetControlFlowColumnResponse* response) {
    auto cf = response->mutable_cf();
    cf->set_cfid(p->GetControlFlowColID());
    return ::grpc::Status::OK;
}

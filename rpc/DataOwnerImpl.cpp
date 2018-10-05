//
// Created by madhav on 10/1/18.
//

#include "DataOwnerImpl.h"
#include "DataOwnerPrivate.h"


DataOwnerImpl::DataOwnerImpl(DataOwnerPrivate *p) {
    this->p = p;
}

::grpc::Status
GetPeerHosts(::grpc::ServerContext* context, const ::vaultdb::GetPeerHostsRequest* request, ::vaultdb::GetPeerHostsResponse* response) {
    for (int i = 0; i < request->hostnames_size(); i++) {
        auto host = request->hostnames(i);
    }

    return grpc::Status::OK;
}

::grpc::Status
DataOwnerImpl::DBMSQuery(::grpc::ServerContext* context,
          const ::vaultdb::DBMSQueryRequest* request,
          ::vaultdb::DBMSQueryResponse* response) {

    table *t = get_table(request->query(), request->dbname());
    int table_id = this->p->AddTable(t);
    vaultdb::TableID *tid = response->mutable_tableid();
    tid->set_tableid(table_id);
    tid->set_hostnum(this->p->HostNum());
    tid->set_query(request->query());
    tid->set_dbname(request->dbname());

    return grpc::Status::OK;
}

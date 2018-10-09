//
// Created by madhav on 10/1/18.
//

#include "DataOwnerImpl.h"
#include "DataOwnerPrivate.h"
#include "../Repartition.h"


DataOwnerImpl::DataOwnerImpl(DataOwnerPrivate *p) {
    this->p = p;
}

::grpc::Status
DataOwnerImpl::GetPeerHosts(::grpc::ServerContext* context, const ::vaultdb::GetPeerHostsRequest* request, ::vaultdb::GetPeerHostsResponse* response) {
    for (int i = 0; i < request->hostnames_size(); i++) {
        auto host = request->hostnames(i);
        p->SetDataOwnerClient(host.hostnum(), host.hostname());
    }
    return grpc::Status::OK;
}

::grpc::Status
DataOwnerImpl::RepartitionStepTwo(::grpc::ServerContext* context, const ::vaultdb::RepartitionStepTwoRequest* request,
                   ::vaultdb::RepartitionStepTwoResponse* response) {
    std::vector<table_t *> table_ptrs;
    for (int i = 0; i < request->tablefragments_size(); i++) {
        table_ptrs.push_back(p->GetTable(request->tablefragments(i).tableid()));
    }
    repartition_step_two(table_ptrs, p->NumHosts(), p);
    return grpc::Status::OK;
}

::grpc::Status
DataOwnerImpl::RepartitionStepOne(::grpc::ServerContext* context, const ::vaultdb::RepartitionStepOneRequest* request,
        ::vaultdb::RepartitionStepOneResponse* response) {
    table_t * t = p->GetTable(request->tableid().tableid());
    std::vector<std::pair<int32_t, int32_t>> info = repart_step_one(t, p->NumHosts(), p);
    for (auto i : info) {
        ::vaultdb::TableID * id = response->add_remoterepartitionids();
        id->set_hostnum(i.first);
        id->set_tableid(i.second);
    }
    return grpc::Status::OK;
}

::grpc::Status
DataOwnerImpl::SendTable(::grpc::ServerContext* context, ::grpc::ServerReader< ::vaultdb::SendTableRequest>* reader,
        ::vaultdb::SendTableResponse* response) {

    vaultdb::SendTableRequest req;

    table_t *t;

    while (reader->Read(&req)) {

        if (req.is_header()) {
            t = allocate_table(req.num_tuple_pages());

            t->num_tuples = req.num_tuples();
            t->size_of_tuple = req.size_of_tuple();
            t->num_tuple_pages = req.num_tuple_pages();

            // TODO(madhavsuresh): refactor this code block out
            t->schema.num_fields = req.schema().num_fields();
            for (int i = 0; i < t->schema.num_fields; i++) {
                t->schema.fields[i].col_no = req.schema().field(i).col_no();
                strncpy(t->schema.fields[i].field_name, req.schema().field(i).field_name().c_str(), FIELD_NAME_LEN);
                switch (req.schema().field(i).field_type()) {
                    case vaultdb::FieldDesc_FieldType_FIXEDCHAR: {
                        //TODO(madhavsuresh): this should all be one enum.
                        t->schema.fields[i].type = FIXEDCHAR;
                        break;
                    }
                    case vaultdb::FieldDesc_FieldType_INT: {
                        t->schema.fields[i].type = INT;
                        break;
                    }
                    default: {
                        throw;
                    }
                }
            }
        } else {
            t->tuple_pages[req.page_no()] = (tuple_page_t *) malloc(PAGE_SIZE);
            memcpy(t->tuple_pages[req.page_no()], req.page().c_str(), PAGE_SIZE);
        }
    }
    int table_id = this->p->AddTable(t);
    response->set_tableid(table_id);
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
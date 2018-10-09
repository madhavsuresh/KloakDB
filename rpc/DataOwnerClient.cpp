//
// Created by madhav on 10/1/18.
//

#include "DataOwnerClient.h"

void DataOwnerClient::GetPeerHosts(std::map<int, std::string> numToHostsMap) {
    vaultdb::GetPeerHostsRequest req;
    vaultdb::GetPeerHostsResponse resp;
    grpc::ClientContext context;

    std::map<int, std::string>::iterator it;
    for (it = numToHostsMap.begin(); it != numToHostsMap.end(); it++) {
        vaultdb::PeerHostInfo *p = req.add_hostnames();
        p->set_hostnum(it->first);
        p->set_hostname(it->second);
    }

    auto status = stub_->GetPeerHosts(&context, req, &resp);
    if(status.ok()) {
        logger_->info("success");
    }  else {
        logger_->warn("failure");
    }

}

std::vector<::vaultdb::TableID> DataOwnerClient::RepartitionStepTwo(std::vector<::vaultdb::TableID>* table_fragments) {

}


 std::vector<std::reference_wrapper<const ::vaultdb::TableID>>  DataOwnerClient::RepartitionStepOne(::vaultdb::TableID& tid) {
    vaultdb::RepartitionStepOneRequest req;
    vaultdb::RepartitionStepOneResponse resp;
    grpc::ClientContext context;

    auto t = req.mutable_tableid();
    t->set_hostnum(tid.hostnum());
    t->set_tableid(tid.tableid());

    stub_->RepartitionStepOne(&context, req, &resp);
    std::vector<std::reference_wrapper<const vaultdb::TableID>> vec;
    for (int i = 0; i < resp.remoterepartitionids_size();i++) {
        ::vaultdb::TableID id;
        id.CopyFrom(resp.remoterepartitionids(i));
        vec.emplace_back(id);
    }
    return vec;
}

::vaultdb::TableID DataOwnerClient::DBMSQuery(std::string dbname, std::string query) {
    vaultdb::DBMSQueryRequest req;
    vaultdb::DBMSQueryResponse resp;
    grpc::ClientContext context;

    req.set_dbname(dbname);
    req.set_query(query);

    auto status = stub_->DBMSQuery(&context, req, &resp);
    if (status.ok()) {
        return resp.tableid();
    } else {
        std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
        throw;
    }
}

void table_schema_to_proto_schema(table_t * t, vaultdb::Schema *s) {
    s->set_num_fields(t->schema.num_fields);
    for (int i = 0; i < t->schema.num_fields; i++) {
        vaultdb::FieldDesc *fd = s->add_field();
        fd->set_field_name(t->schema.fields[i].field_name, FIELD_LEN);
        fd->set_col_no(t->schema.fields[i].col_no);
        switch (t->schema.fields[i].type) {
            case FIXEDCHAR: {
                fd->set_field_type(vaultdb::FieldDesc_FieldType_FIXEDCHAR);
                break;
            }
            case INT: {
                fd->set_field_type(vaultdb::FieldDesc_FieldType_INT);
                break;
            }
            default: {
                throw;
            }
        }
    }
}

    //TODO(madhavsuresh): refactor this to return
int DataOwnerClient::SendTable(table_t * t) {
    ::vaultdb::SendTableResponse resp;
    ::grpc::ClientContext context;

    std::unique_ptr<::grpc::ClientWriter<::vaultdb::SendTableRequest>> writer(stub_->SendTable(&context, &resp));

    ::vaultdb::SendTableRequest header;
    header.set_is_header(true);
    header.set_num_tuples(t->num_tuples);
    header.set_num_tuple_pages(t->num_tuple_pages);
    header.set_size_of_tuple(t->size_of_tuple);

    ::vaultdb::Schema *s = header.mutable_schema();
    table_schema_to_proto_schema(t, s);
    writer->Write(header);

    ::vaultdb::SendTableRequest pages;
    for (int i = 0; i < t->num_tuple_pages; i++) {
        pages.set_is_header(false) ;
        pages.set_page_no(i);
        pages.set_page((char *) t->tuple_pages[i], PAGE_SIZE);
        writer->Write(pages);
    }
    writer->WritesDone();
    ::grpc::Status status = writer->Finish();

    if (status.ok()) {
        return resp.tableid();
    } else {
        std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
        throw;
    }
}

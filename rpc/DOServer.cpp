//
// Created by madhav on 9/17/18.
//
#include "vaultdb.grpc.pb.h"
#include "../postgres_client.h"
#include <grpcpp/grpcpp.h>

using grpc::Server;
using grpc::Status;
using grpc::ServerContext;
using grpc::ServerWriter;
using vaultdb::DBQueryRequest;
using vaultdb::DBQueryResponse;
using vaultdb::TableRequest;
using vaultdb::TableResponse;
using vaultdb::TableStream;
using grpc::ServerBuilder;
using vaultdb::TableQueryRequest;
using vaultdb::TableQueryResponse;
using vaultdb::Schema;

class DOServerImpl final : public vaultdb::VaultDBOperators::Service {
    virtual Status DBQuery(ServerContext * context, const DBQueryRequest * request, DBQueryResponse* response) {
        table_builder_t * tb = table_builder(request->query_string(), request->dbname());
        for (int i = 0; i < tb->table->num_tuples; i ++) {
            auto tup = get_tuple(i, tb->table);
            fprintf(stderr,"[%d, %d]", tup->field_list[0].f.int_field.val, tup->field_list[1].f.int_field.val);
        }
        return Status::OK;
    }

    virtual Status GetTableStream(ServerContext * context, const TableRequest * request, ServerWriter<TableStream> * writer) {
        table_builder_t * tb = table_builder("SELECT * FROM rpc_test;", "dbname=test");
        TableStream t;
        for (int i = 0; i < tb->table->num_tuple_pages; i++) {
            t.set_page(tb->table->tuple_pages[i],PAGE_SIZE);
            writer->Write(t);
        }
    }

    virtual Status NumHosts(::grpc::ServerContext* context, const ::vaultdb::NumHostsRequest* request,
            ::vaultdb::NumHostResp* response) {
        response->set_num_hosts(this->num_hosts);
        return Status::OK;
    }

    virtual Status GetTable(ServerContext * context, const TableQueryRequest* request,
            ServerWriter<TableQueryResponse> * writer) {
        //request->dbname();
        //TODO(madhavsuresh): this api needs to be changed.
        table_builder_t *tb = table_builder(request->query(), request->dbname());
        table * t = tb->table;
        //TODO(madhavsuresh): this is a poor abstraction.
        free(tb);
        TableQueryResponse header;
        header.set_is_header(true);
        header.set_num_tuples(t->num_tuples);
        header.set_num_tuple_pages(t->num_tuple_pages);
        header.set_size_of_tuple(t->size_of_tuple);

        Schema *s = header.mutable_schema();
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
        writer->Write(header);
        TableQueryResponse pages;
        for (int i = 0; i < t->num_tuple_pages; i++) {
            pages.set_is_header(false) ;
            pages.set_page_no(i);
            pages.set_page((char *) t->tuple_pages[i], PAGE_SIZE);
            writer->Write(pages);
            free(t->tuple_pages[i]);
        }
        free(t);
        return Status::OK;
    }

private:
    int num_hosts = 3;

};

void RunServer() {
    //TODO(madhavssuresh): set as flag
    std::string server_address("0.0.0.0:50051");
    DOServerImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    //std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
}

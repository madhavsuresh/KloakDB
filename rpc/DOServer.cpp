//
// Created by madhav on 9/17/18.
//
#include "vaultdb.grpc.pb.h"
#include "../postgres_client.h"
#include <grpcpp/grpcpp.h>

using grpc::Server;
using grpc::Status;
using grpc::ServerContext;
using vaultdb::DBQueryRequest;
using vaultdb::DBQueryResponse;
using grpc::ServerBuilder;

class DOServerImpl final : public vaultdb::VaultDBOperators::Service {
    virtual Status DBQuery(ServerContext * context, const DBQueryRequest * request, DBQueryResponse* response) {
        table_builder_t * tb = table_builder(request->query_string(), request->dbname());
        for (int i = 0; i < tb->table->num_tuples; i ++) {
            auto tup = get_tuple(i, tb->table);
            fprintf(stderr,"[%d, %d]", tup->field_list[0].f.int_field.val, tup->field_list[1].f.int_field.val);
        }
        return Status::OK;
    }
};

void RunServer() {
    //TODO(madhavssuresh): set as flag
    std::string server_address("0.0.0.0:50051");
    DOServerImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
}




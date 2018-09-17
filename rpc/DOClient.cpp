//
// Created by madhav on 9/17/18.
//
#include "vaultdb.grpc.pb.h"
#include <grpcpp/grpcpp.h>

class VaultDBClient {
public:
    VaultDBClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(vaultdb::VaultDBOperators::NewStub(channel)) {}

    int DBQuery() {
        vaultdb::DBQueryRequest req;
        vaultdb::DBQueryResponse resp;
        req.set_dbname("dbname=test");
        req.set_query_string("SELECT * FROM rpc_test;");

        grpc::ClientContext context;
        auto status = stub_->DBQuery(&context,req, &resp);
        if (status.ok()) {

        } else {
                std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
                return -1;
        }
    }
private:
    std::unique_ptr<vaultdb::VaultDBOperators::Stub> stub_;

};

int main() {
       VaultDBClient client( grpc::CreateChannel("0.0.0.0:50051",
                       grpc::InsecureChannelCredentials()));
       client.DBQuery();
}


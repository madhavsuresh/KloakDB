//
// Created by madhav on 10/1/18.
//

#include "DataOwnerClient.h"

int DataOwnerClient::DBMSQuery(std::string dbname, std::string query) {
    vaultdb::DBMSQueryRequest req;
    vaultdb::DBMSQueryResponse resp;
    grpc::ClientContext context;

    req.set_dbname(dbname);
    req.set_query(query);

    auto status = stub_->DBMSQuery(&context, req, &resp);
    if (status.ok()) {
        return resp.tableid().tableid();
    } else {
        std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
        throw;
    }
}

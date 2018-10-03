//
// Created by madhav on 9/25/18.
//

#ifndef PROJECT_DOCLIENT_H
#define PROJECT_DOCLIENT_H
#include "../postgres_client.h"


table_t * GetTable(std::string dbname, std::string query);
class VaultDBClient {
public:
    VaultDBClient(std::shared_ptr<grpc::Channel> channel);
    table_t * GetTable(std::string dbname, std::string query);
private:
};

#endif //PROJECT_DOCLIENT_H

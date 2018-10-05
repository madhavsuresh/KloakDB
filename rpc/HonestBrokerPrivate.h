//
// Created by madhav on 9/28/18.
//

#ifndef PROJECT_HONESTBROKERPRIVATE_H
#define PROJECT_HONESTBROKERPRIVATE_H


#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "InfoPrivate.h"
#include "DataOwnerClient.h"

class HonestBrokerPrivate : public InfoPrivate {

public:
    HonestBrokerPrivate(std::string address);
    int RegisterHost(std::string hostName);
    int NumHosts();
    int DBMSQuery(int host_num, std::string dbname, std::string query);
private:
    std::mutex registrationMutex;
    //Expected num hosts is used for the bootstrapping process
    int expected_num_hosts;
    int num_hosts;
    std::vector<std::string> remoteHostnames;
    std::map<int, std::string> numToHostMap;
    std::map<int, DataOwnerClient *> do_clients;
};

#endif //PROJECT_HONESTBROKERPRIVATE_H

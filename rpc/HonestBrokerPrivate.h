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
    void Repartition(std::vector<::vaultdb::TableID> tids);
    std::vector<::vaultdb::TableID> RepartitionStepOne(::vaultdb::TableID id);
    std::vector<::vaultdb::TableID> RepartitionStepTwo(int host_num, std::vector<::vaultdb::TableID> table_fragments);
    int GetControlFlowColID();
    void SetControlFlowColID(int col_ID);

    ::vaultdb::TableID DBMSQuery(int host_num, std::string dbname, std::string query);
private:
    std::mutex registrationMutex;
    //Expected num hosts is used for the bootstrapping process
    int expected_num_hosts;
    int num_hosts;
    ::vaultdb::ControlFlowColumn cf;
    std::vector<std::string> remoteHostnames;
    std::map<int, std::string> numToHostMap;
    std::map<int, DataOwnerClient *> do_clients;
};

#endif //PROJECT_HONESTBROKERPRIVATE_H

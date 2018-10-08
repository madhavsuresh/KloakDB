//
// Created by madhav on 9/28/18.
//

#include <gflags/gflags.h>
#include "HonestBrokerPrivate.h"


DEFINE_int32(expected_num_hosts, 1, "Expected number of hosts");

HonestBrokerPrivate::HonestBrokerPrivate(std::string honest_broker_address)
: InfoPrivate(honest_broker_address){
    this->num_hosts = 0;
    this->expected_num_hosts = FLAGS_expected_num_hosts;
}

int HonestBrokerPrivate::RegisterHost(std::string hostName) {
    this->registrationMutex.lock();
    this->num_hosts++;
    int host_num = this->num_hosts;
    remoteHostnames.push_back(hostName);
    numToHostMap[host_num] = hostName;
    do_clients[host_num] = new DataOwnerClient(grpc::CreateChannel(hostName,
            grpc::InsecureChannelCredentials()));
    this->registrationMutex.unlock();
    return host_num;
}

::vaultdb::TableID HonestBrokerPrivate::DBMSQuery(int host_num, std::string dbname,
        std::string query) {
    return this->do_clients[host_num]->DBMSQuery(dbname, query);
}


void HonestBrokerPrivate::Repartition(std::vector<::vaultdb::TableID> ids) {
    std::map<int, std::vector<::vaultdb::TableID>> table_fragments;
    for (const auto &i : ids) {
        auto k = RepartitionStepOne(i);
        for (const ::vaultdb::TableID &j : k) {
            table_fragments[j.hostnum()].push_back(j);
        }
    }

    std::map<int, std::vector<::vaultdb::TableID>>::iterator it;
    for (it = table_fragments.begin(); it != table_fragments.end(); it++) {
        RepartitionStepTwo(it->first, it->second);
    }
}

void HonestBrokerPrivate::SetControlFlowColID(int col_ID) {
    cf.set_cfid(col_ID);
}

int HonestBrokerPrivate::GetControlFlowColID() {
    return cf.cfid();
}


std::vector<::vaultdb::TableID> HonestBrokerPrivate::RepartitionStepOne(::vaultdb::TableID id) {
    return do_clients[id.hostnum()]->RepartitionStepOne(id);
}

std::vector<::vaultdb::TableID> HonestBrokerPrivate::RepartitionStepTwo(int host_num, std::vector<::vaultdb::TableID> table_fragments) {
    return do_clients[host_num]->RepartitionStepTwo(table_fragments);
}

int HonestBrokerPrivate::NumHosts() {
    return this->num_hosts;
}

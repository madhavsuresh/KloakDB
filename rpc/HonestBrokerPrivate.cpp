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

int HonestBrokerPrivate::RegisterPeerHosts() {
    for (int i = 0; i < num_hosts; i++) {
        do_clients[i]->GetPeerHosts(numToHostMap);
    }
}

int HonestBrokerPrivate::RegisterHost(std::string hostName) {
    this->registrationMutex.lock();
    int host_num = this->num_hosts;
    remoteHostnames.push_back(hostName);
    numToHostMap[host_num] = hostName;
    do_clients[host_num] = new DataOwnerClient(grpc::CreateChannel(hostName,
            grpc::InsecureChannelCredentials()));
    this->registrationMutex.unlock();
    this->num_hosts++;
    return host_num;
}

::vaultdb::TableID HonestBrokerPrivate::DBMSQuery(int host_num, std::string dbname,
        std::string query) {
    return this->do_clients[host_num]->DBMSQuery(dbname, query);
}


void HonestBrokerPrivate::Repartition(std::vector<std::reference_wrapper<::vaultdb::TableID>> &ids) {
  std::map<int, std::vector<std::reference_wrapper<const ::vaultdb::TableID>>> table_fragments;
    for (auto id : ids) {
        auto k = HonestBrokerPrivate::RepartitionStepOne(id);
        for (const ::vaultdb::TableID &j : k) {
            table_fragments[j.hostnum()].emplace_back(j);
        }
    }

    for (int i = 0; i < num_hosts; i++) {
        //auto fragment = table_fragments[i];
    }
}

void HonestBrokerPrivate::SetControlFlowColID(int col_ID) {
    cf.set_cfid(col_ID);
}

int HonestBrokerPrivate::GetControlFlowColID() {
    return cf.cfid();
}


std::vector<std::reference_wrapper<const ::vaultdb::TableID>> HonestBrokerPrivate::RepartitionStepOne(std::reference_wrapper<::vaultdb::TableID> id) {
    return do_clients[id.get().hostnum()]->RepartitionStepOne(id);
}

std::vector<::vaultdb::TableID> HonestBrokerPrivate::RepartitionStepTwo(int host_num, std::vector<int> &table_fragments) {
    //return do_clients[host_num]->RepartitionStepTwo(table_fragments);
}

int HonestBrokerPrivate::NumHosts() {
    return this->num_hosts;
}

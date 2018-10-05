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
    do_clients[host_num] = new DataOwnerClient( grpc::CreateChannel(hostName,
            grpc::InsecureChannelCredentials()));
    this->registrationMutex.unlock();
    return host_num;
}

int HonestBrokerPrivate::DBMSQuery(int host_num, std::string dbname,
        std::string query) {
    return this->do_clients[host_num]->DBMSQuery(dbname, query);
}

int HonestBrokerPrivate::Repartition(int host_num, int table_id) {

}

int HonestBrokerPrivate::NumHosts() {
    return this->num_hosts;
}

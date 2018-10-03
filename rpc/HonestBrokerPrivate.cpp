//
// Created by madhav on 9/28/18.
//

#include <gflags/gflags.h>
#include "HonestBrokerPrivate.h"


HonestBrokerPrivate::HonestBrokerPrivate(std::string honest_broker_address)
: InfoPrivate(honest_broker_address){
    this->num_hosts = 0;
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

int HonestBrokerPrivate::NumHosts() {
    return this->num_hosts;
}

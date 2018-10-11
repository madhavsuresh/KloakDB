//
// Created by madhav on 9/28/18.
//

#include "HonestBrokerPrivate.h"
#include <gflags/gflags.h>

DEFINE_int32(expected_num_hosts, 1, "Expected number of hosts");
using namespace std;

HonestBrokerPrivate::HonestBrokerPrivate(std::string honest_broker_address)
    : InfoPrivate(honest_broker_address) {
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
  do_clients[host_num] = new DataOwnerClient(
      host_num,
      grpc::CreateChannel(hostName, grpc::InsecureChannelCredentials()));
  this->registrationMutex.unlock();
  this->num_hosts++;
  LOG(INFO) << "registered host: [" << hostName << "] at hostnum : ["
            << host_num << "]";
  return host_num;
}

::vaultdb::TableID HonestBrokerPrivate::DBMSQuery(int host_num,
                                                  std::string dbname,
                                                  std::string query) {
  return this->do_clients[host_num]->DBMSQuery(dbname, query);
}

std::vector<std::shared_ptr<const ::vaultdb::TableID>>
HonestBrokerPrivate::Repartition(
    std::vector<std::shared_ptr<::vaultdb::TableID>> &ids) {
  std::map<int, std::vector<std::shared_ptr<const ::vaultdb::TableID>>>
      table_fragments;
  for (auto id : ids) {
    auto k = RepartitionStepOne(id);
    for (auto j : k) {
      table_fragments[j.get()->hostnum()].emplace_back(j);
    }
  }

  std::map<int, std::vector<std::shared_ptr<const ::vaultdb::TableID>>>
      hashed_table_fragments;
  for (int i = 0; i < num_hosts; i++) {
    auto out = RepartitionStepTwo(i, table_fragments[i]);
    for (auto j : out) {
      hashed_table_fragments[j.get()->hostnum()].emplace_back(j);
    }
  }

  std::vector<std::shared_ptr<const ::vaultdb::TableID>> coalesced_tables;
  for (int i = 0; i < num_hosts; i++) {
    std::shared_ptr<const ::vaultdb::TableID> tmp =
        Coalesce(i, hashed_table_fragments[i]);
    coalesced_tables.emplace_back(tmp);
  }
  return coalesced_tables;
}

void HonestBrokerPrivate::SetControlFlowColID(int col_ID) {
  cf.set_cfid(col_ID);
}

int HonestBrokerPrivate::GetControlFlowColID() { return cf.cfid(); }

std::shared_ptr<const ::vaultdb::TableID> HonestBrokerPrivate::Coalesce(
    int host_num,
    std::vector<std::shared_ptr<const ::vaultdb::TableID>> tables) {
  return do_clients[host_num]->CoalesceTables(tables);
}

std::vector<std::shared_ptr<const ::vaultdb::TableID>>
HonestBrokerPrivate::RepartitionStepOne(
    std::shared_ptr<::vaultdb::TableID> id) {
  return do_clients[id.get()->hostnum()]->RepartitionStepOne(id);
}

std::vector<std::shared_ptr<const ::vaultdb::TableID>>
HonestBrokerPrivate::RepartitionStepTwo(
    int host_num,
    std::vector<std::shared_ptr<const ::vaultdb::TableID>> table_fragments) {
  return do_clients[host_num]->RepartitionStepTwo(table_fragments);
}

int HonestBrokerPrivate::NumHosts() { return this->num_hosts; }

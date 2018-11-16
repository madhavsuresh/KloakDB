//
// Created by madhav on 9/28/18.
//

#include "HonestBrokerPrivate.h"
#include "operators/Generalize.h"
#include <gflags/gflags.h>

DEFINE_int32(expected_num_hosts, 1, "Expected number of hosts");
using namespace std;

HonestBrokerPrivate::HonestBrokerPrivate(std::string honest_broker_address)
    : InfoPrivate(honest_broker_address) {
  this->num_hosts = 0;
  this->expected_num_hosts = FLAGS_expected_num_hosts;
}
void HonestBrokerPrivate::WaitForAllHosts() {
  while (this->NumHosts() < this->expected_num_hosts) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  printf("All hosts registered");
}

int HonestBrokerPrivate::RegisterPeerHosts() {
  for (int i = 0; i < num_hosts; i++) {
    do_clients[i]->GetPeerHosts(numToHostMap);
  }
}

std::string count_star_query(std::string table_name, std::string column) {
  return "SELECT " + column + ", count(*) FROM " + table_name + " GROUP BY " + column;
}
// TODO(madhavsuresh): support multiple column generalization
// TODO(madhavsuresh): this is a work in progress. this needs to be filled in.
void HonestBrokerPrivate::Generalize(std::string table_name, std::string column, std::string dbname) {
  std::string query_string = count_star_query(table_name, column);

  std::vector<::vaultdb::TableID> tids;
  for (int i = 0; i < this->num_hosts; i++) {
    auto tid = this->DBMSQuery(i, "dbname=" + dbname, query_string);
    tids.push_back(tid);
  }
  std::vector<std::pair<hostnum, table_t*>> gen_tables;
  for (auto &t : tids ) {
    gen_tables.emplace_back(t.hostnum(), do_clients[t.hostnum()]->GetTable(t));
  }
  table_t * gen_map = generalize_table(gen_tables, this->NumHosts(), 5);
  std::vector<::vaultdb::TableID> gen_ids;
  for (int i = 0; i < this->num_hosts; i++) {
     auto resp = do_clients[i]->SendTable(gen_map);
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

std::vector<std::shared_ptr<const ::vaultdb::TableID>>
HonestBrokerPrivate::Filter(
    std::vector<std::shared_ptr<const ::vaultdb::TableID>> &ids,
    ::vaultdb::Expr &expr) {

  std::vector<std::shared_ptr<const ::vaultdb::TableID>> filtered_tables;
  for (auto &i : ids) {
    filtered_tables.emplace_back(
        do_clients[i.get()->hostnum()]->Filter(i, expr));
  }
  return filtered_tables;
}

std::vector<std::shared_ptr<const ::vaultdb::TableID>>
HonestBrokerPrivate::Join(
    std::vector<std::pair<std::shared_ptr<const ::vaultdb::TableID>,
                          std::shared_ptr<const ::vaultdb::TableID>>> &ids,
    ::vaultdb::JoinDef &join) {

  std::vector<std::shared_ptr<const ::vaultdb::TableID>> joined_tables;
  for (auto &i : ids) {
    joined_tables.emplace_back(
        do_clients[i.first.get()->hostnum()]->Join(i.first, i.second, join));
  }
  return joined_tables;
}

std::vector<std::shared_ptr<const ::vaultdb::TableID>>
HonestBrokerPrivate::Sort(
    std::vector<std::shared_ptr<const ::vaultdb::TableID>> &ids,
    ::vaultdb::SortDef &sort) {
  std::vector<std::shared_ptr<const ::vaultdb::TableID>> sorted_tables;
  for (auto &i : ids) {
    sorted_tables.emplace_back(do_clients[i.get()->hostnum()]->Sort(i, sort));
  }
  return sorted_tables;
}

std::vector<std::shared_ptr<const ::vaultdb::TableID>>
HonestBrokerPrivate::Aggregate(
    std::vector<std::shared_ptr<const ::vaultdb::TableID>> &ids,
    ::vaultdb::GroupByDef &groupby) {
  std::vector<std::shared_ptr<const ::vaultdb::TableID>> aggregate_tables;
  for (auto &i : ids) {
    aggregate_tables.emplace_back(
        do_clients[i.get()->hostnum()]->Aggregate(i, groupby));
  }
  return aggregate_tables;
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

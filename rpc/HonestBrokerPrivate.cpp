//
// Created by madhav on 9/28/18.
//

#include "HonestBrokerPrivate.h"
#include "operators/Generalize.h"
#include <gflags/gflags.h>

DEFINE_int32(expected_num_hosts, 2, "Expected number of hosts");
using namespace std;
using namespace vaultdb;


HonestBrokerPrivate::HonestBrokerPrivate(string honest_broker_address)
    : InfoPrivate(honest_broker_address) {
  this->num_hosts = 0;
  this->expected_num_hosts = FLAGS_expected_num_hosts;
}

HonestBrokerPrivate::~HonestBrokerPrivate() {
  for (int i = 0; i < this->num_hosts; i++) {
    delete do_clients[i];
  }
}

void HonestBrokerPrivate::Shutdown() {
  for (int i = 0; i < this->NumHosts(); i++) {
    do_clients[i]->Shutdown();
  }
}

void HonestBrokerPrivate::WaitForAllHosts() {
  while (this->NumHosts() < this->expected_num_hosts) {
    this_thread::sleep_for(chrono::milliseconds(1000));
  }
  printf("All hosts registered");
}

int HonestBrokerPrivate::RegisterPeerHosts() {
  for (int i = 0; i < num_hosts; i++) {
    do_clients[i]->GetPeerHosts(numToHostMap);
  }
}

string count_star_query(string table_name, string column) {
  return "SELECT " + column + ", count(*) FROM " + table_name + " GROUP BY " +
         column;
}

void log_gen_stats(table_t * gen_map, std::string column) {
  map<int, int> mapping_table;
  for (int i = 0; i < gen_map->num_tuples; i++) {
    mapping_table[get_tuple(i, gen_map)->field_list[0].f.int_field.genval]++;
  }
  for (auto t: mapping_table) {
    LOG(INFO) << "Gen value:" << t.first << ", COUNT:" << t.second;
  }
}

// TODO(madhavsuresh): support multiple column generalization
// TODO(madhavsuresh): this is a work in progress. this needs to be filled in.
vector<tableid_ptr>
HonestBrokerPrivate::Generalize(
    string table_name, string column, string dbname,
    vector<tableid_ptr> scanned_tables, int gen_level) {
  string query_string = count_star_query(table_name, column);

  vector<tableid_ptr> out_vec;
  vector<tableid_ptr> tids;
  for (int i = 0; i < this->num_hosts; i++) {
    auto tid = this->DBMSQuery(i, "dbname=" + dbname, query_string);
    tids.push_back(tid);
  }
  vector<pair<hostnum, table_t *>> gen_tables;
  for (auto &t : tids) {
    gen_tables.emplace_back(t.get()->hostnum(),
                            do_clients[t.get()->hostnum()]->GetTable(t));
  }
  table_t *gen_map = generalize_table(gen_tables, this->NumHosts(), gen_level);
  log_gen_stats(gen_map, column);
  for (int i = 0; i < this->num_hosts; i++) {
    auto resp = do_clients[i]->SendTable(gen_map);
    ::vaultdb::TableID out;
    out.set_hostnum(i);
    out.set_tableid(resp);
    auto outptr = make_shared<const ::vaultdb::TableID>(out);
    for (auto &st : scanned_tables) {
      if (st.get()->hostnum() == i) {
        out_vec.emplace_back(do_clients[i]->GenZip(outptr, st));
      }
    }
  }
  free_table(gen_map);
  for (auto &t : gen_tables) {
    free_table(t.second);
  }
  return out_vec;
}

int HonestBrokerPrivate::RegisterHost(string hostName) {
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

vector<tableid_ptr> HonestBrokerPrivate::ClusterDBMSQuery(string dbname,
                                                          string query) {
  vector<tableid_ptr> queried_tables;
  for (int i = 0; i < num_hosts; i++) {
    queried_tables.emplace_back(DBMSQuery(i, dbname, query));
  }
  return queried_tables;
}

tableid_ptr HonestBrokerPrivate::DBMSQuery(int host_num, string dbname,
                                           string query) {

  return this->do_clients[host_num]->DBMSQuery(dbname, query);
}

vector<tableid_ptr> HonestBrokerPrivate::Repartition(vector<tableid_ptr> &ids) {
  map<int, vector<tableid_ptr>> table_fragments;
  for (auto id : ids) {
    auto k = RepartitionStepOne(id);
    for (auto j : k) {
      table_fragments[j.get()->hostnum()].emplace_back(j);
    }
  }

  map<int, vector<tableid_ptr>> hashed_table_fragments;
  for (int i = 0; i < num_hosts; i++) {
    auto out = RepartitionStepTwo(i, table_fragments[i]);
    for (auto j : out) {
      hashed_table_fragments[j.get()->hostnum()].emplace_back(j);
    }
  }

  vector<tableid_ptr> coalesced_tables;
  for (int i = 0; i < num_hosts; i++) {
    tableid_ptr tmp = Coalesce(i, hashed_table_fragments[i]);
    coalesced_tables.emplace_back(tmp);
  }
  return coalesced_tables;
}

vector<tableid_ptr> HonestBrokerPrivate::Filter(vector<tableid_ptr> &ids,
                                                ::vaultdb::Expr &expr) {

  vector<tableid_ptr> filtered_tables;
  for (auto &i : ids) {
    filtered_tables.emplace_back(
        do_clients[i.get()->hostnum()]->Filter(i, expr));
  }
  return filtered_tables;
}

void HonestBrokerPrivate::FreeTables(vector<tableid_ptr> &ids) {
  for (auto &i : ids) {
            do_clients[i.get()->hostnum()]->FreeTable(i);
  }
}

vector<tableid_ptr>
HonestBrokerPrivate::Join(vector<pair<tableid_ptr, tableid_ptr>> &ids,
                          ::vaultdb::JoinDef &join, bool in_sgx) {

  vector<tableid_ptr> joined_tables;
  for (auto &i : ids) {
    joined_tables.emplace_back(
        do_clients[i.first.get()->hostnum()]->Join(i.first, i.second, join, in_sgx));
  }
  return joined_tables;
}

vector<tableid_ptr> HonestBrokerPrivate::Sort(vector<tableid_ptr> &ids,
                                              ::vaultdb::SortDef &sort) {
  vector<tableid_ptr> sorted_tables;
  for (auto &i : ids) {
    sorted_tables.emplace_back(do_clients[i.get()->hostnum()]->Sort(i, sort));
  }
  return sorted_tables;
}

vector<tableid_ptr>
HonestBrokerPrivate::Aggregate(vector<tableid_ptr> &ids,
                               ::vaultdb::GroupByDef &groupby) {
  vector<tableid_ptr> aggregate_tables;
  for (auto &i : ids) {
    aggregate_tables.emplace_back(
        do_clients[i.get()->hostnum()]->Aggregate(i, groupby));
  }
  return aggregate_tables;
}

void HonestBrokerPrivate::SetControlFlowColID(int col_ID) {
  cf.set_cfid(col_ID);
}

void HonestBrokerPrivate::SetControlFlowColName(string name) {
  cf.set_cf_name(name);
}

::vaultdb::ControlFlowColumn HonestBrokerPrivate::GetControlFlowColID() { return cf; }

tableid_ptr HonestBrokerPrivate::Coalesce(int host_num,
                                          vector<tableid_ptr> tables) {
  return do_clients[host_num]->CoalesceTables(tables);
}

vector<tableid_ptr> HonestBrokerPrivate::RepartitionStepOne(tableid_ptr id) {
  return do_clients[id.get()->hostnum()]->RepartitionStepOne(id);
}

vector<tableid_ptr>
HonestBrokerPrivate::RepartitionStepTwo(int host_num,
                                        vector<tableid_ptr> table_fragments) {
  return do_clients[host_num]->RepartitionStepTwo(table_fragments);
}

int HonestBrokerPrivate::NumHosts() { return this->num_hosts; }

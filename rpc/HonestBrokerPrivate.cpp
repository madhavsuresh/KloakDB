//
// Created by madhav on 9/28/18.
//

#include "HonestBrokerPrivate.h"
#include <future>
#include <gflags/gflags.h>
#include <logger/LoggerDefs.h>

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
    return "SELECT " + column + ", count(*) FROM " + table_name + " GROUP BY " + column + " ORDER BY " + column;
}

void log_gen_stats(table_t *gen_map) {
  map<int, int> counter;
  for (int i = 0; i < gen_map->num_tuples; i++) {
    counter[get_tuple(i, gen_map)->field_list[0].f.int_field.genval]++;
  }
  int max_val = 0;
  int min_val = 100000;
  int num_classes = 0;
  for (auto &i : counter) {
    num_classes++;
    if (max_val < i.second) {
      max_val = i.second;
    }
    if (min_val > i.second) {
      min_val = i.second;
    }
  }
  LOG(STATS) << "Gen Stats: MIN:[" << min_val << "], MAX:[" << max_val
             << "] AVG:[" << (double)gen_map->num_tuples / num_classes << "]";
}

unordered_map<table_name, vector<tableid_ptr>>
HonestBrokerPrivate::Generalize(unordered_map<table_name, to_gen_t> in,
                                int gen_level) {

  unordered_map<table_name, vector<tableid_ptr>> out_map;
  std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
      gen_input;
  std::vector<tableid_ptr> input_scans;
  for (auto &table : in) {
    auto to_gen = table.second;
    input_scans.insert(input_scans.end(), to_gen.scan_tables.begin(),
                       to_gen.scan_tables.end());
  }

  for (auto &table : in) {
    vector<tableid_ptr> tids;
    string column = table.second.column;
    auto query = count_star_query(table.first, column);
    auto dbname = table.second.dbname;
    for (int i = 0; i < this->num_hosts; i++) {
      auto tid = this->DBMSQuery(i, "dbname=" + dbname, query);
      tids.push_back(tid);
    }
    vector<pair<hostnum, table_t *>> count_tables;
    for (auto &t : tids) {
      count_tables.emplace_back(t.get()->hostnum(),
                                do_clients[t.get()->hostnum()]->GetTable(t));
    }
    gen_input[table.first] = count_tables;
  }
  START_TIMER(generalize_inner);
  table_t *gen_map = generalize_table_fast(gen_input, num_hosts, gen_level);//generalize_table(gen_input, num_hosts, gen_level);
  LOG(EXEC) << "Size of Generalize: " << gen_map->num_tuple_pages << " num tuples: " << gen_map->num_tuples << " size: " << gen_map->num_tuples*gen_map->size_of_tuple/(1000*1000);
  LOG(EXEC) << "END OF GENERALIZATION";
  END_AND_LOG_TIMER(generalize_inner);
  log_gen_stats(gen_map);

  std::unordered_map<cf_hash, cf_gen> gen_z;
  for (int i = 0; i < gen_map->num_tuples; i++) {
    tuple_t *tup = get_tuple(i, gen_map);
    gen_z[tup->field_list[0].f.int_field.val] = tup->field_list[0].f.int_field.genval;
  }
  for (auto &table : gen_input) {
    unordered_map<int, int> gen_val_to_count;
    for (auto &t : table.second) {
      table_t * z_table = t.second;
      for (int i = 0 ; i < z_table->num_tuples; i++) {
        tuple_t *tup = get_tuple(i, z_table);
        gen_val_to_count[gen_z[tup->field_list[0].f.int_field.val]] += tup->field_list[1].f.int_field.val;
      }
    }
    int max = 0;
    int total = 0;
    int num = 0;
    int moment = 15;
    int above_moment = 0;
    for (auto &cc : gen_val_to_count) {
      num++;
      total+= cc.second;
      if (max < cc.second) {
        max = cc.second;
      }
      if (cc.second > moment) {
        above_moment++;
      }
    }
    printf("Gen for %s: AVG: %f, TOTAL NUM: %d, ABOVE_MOMENT: %d, max: %d\n", table.first.c_str(), (double)total/num, num, above_moment, max);
  }

  for (int i = 0; i < num_hosts; i++) {
    auto resp = do_clients[i]->SendTable(gen_map, false);
    ::vaultdb::TableID out;
    out.set_hostnum(i);
    out.set_tableid(resp);
    auto outptr = make_shared<const ::vaultdb::TableID>(out);
    for (auto &table : in) {
      auto tup = table.second.scan_tables;
      for (auto &st : tup) {
        if (st.get()->hostnum() == i) {
          auto zipped = do_clients[i]->GenZip(outptr, st, table.second.column);
          out_map[table.first].emplace_back(zipped);
        }
      }
    }
  }
  return out_map;
}

// TODO(madhavsuresh): support multiple column generalization
// TODO(madhavsuresh): this is a work in progress. this needs to be filled in.
vector<tableid_ptr>
HonestBrokerPrivate::Generalize(string table_name, string column, string dbname,
                                vector<tableid_ptr> scanned_tables,
                                int gen_level) {
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
  for (int i = 0; i < this->num_hosts; i++) {
    auto resp = do_clients[i]->SendTable(gen_map, false);
    ::vaultdb::TableID out;
    out.set_hostnum(i);
    out.set_tableid(resp);
    auto outptr = make_shared<const ::vaultdb::TableID>(out);
    for (auto &st : scanned_tables) {
      if (st.get()->hostnum() == i) {
        out_vec.emplace_back(do_clients[i]->GenZip(outptr, st, column));
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
      hostName, host_num,
      grpc::CreateChannel(hostName, grpc::InsecureChannelCredentials()));
  this->registrationMutex.unlock();
  this->num_hosts++;
  LOG(INFO) << "registered host: [" << hostName << "] at hostnum : ["
            << host_num << "]";
  return host_num;
}

vector<tableid_ptr> HonestBrokerPrivate::ClusterDBMSQuery(string dbname,
                                                          string query) {
  LOG(HB_P) << "Cluster Querying: " + query;
  vector<tableid_ptr> queried_tables;
  vector<std::future<tableid_ptr>> threads;
  for (int i = 0; i < num_hosts; i++) {
    threads.push_back(std::async(std::launch::async,
                                 &HonestBrokerPrivate::DBMSQuery, this, i,
                                 dbname, query));
  }
  for (auto &f : threads) {
    auto tt = f.get();
    queried_tables.push_back(tt);
  }
  return queried_tables;
}

tableid_ptr HonestBrokerPrivate::DBMSQuery(int host_num, string dbname,
                                           string query) {
  LOG(HB_P) << "Point Querying: " + query;
  return this->do_clients[host_num]->DBMSQuery(dbname, query);
}

vector<tableid_ptr> HonestBrokerPrivate::RepartitionJustHash(vector<tableid_ptr> &ids) {
  vector<std::future<vector<tableid_ptr>>> threads_repart2;
  map<int, vector<tableid_ptr>> hashed_table_fragments;
  for (auto &i : ids) {
    vector<tableid_ptr> tmp;
    tmp.push_back(i);
    threads_repart2.push_back(
            std::async(std::launch::async, &HonestBrokerPrivate::RepartitionStepTwo,
                       this, i.get()->hostnum(), tmp));
  }

  for (auto &f : threads_repart2) {
    auto out = f.get();
    for (auto j : out) {
      hashed_table_fragments[j.get()->hostnum()].emplace_back(j);
    }
  }
  vector<tableid_ptr> coalesced_tables;
  vector<std::future<tableid_ptr>> threads_coalesced;
  START_TIMER(repartition_coalesce_outer_private);
  for (int i = 0; i < num_hosts; i++) {
    threads_coalesced.push_back(std::async(std::launch::async,
                                           &HonestBrokerPrivate::Coalesce, this,
                                           i, hashed_table_fragments[i]));
  }
  for (auto &f : threads_coalesced) {
    auto out = f.get();
    coalesced_tables.emplace_back(out);
  }
  return coalesced_tables;
}
vector<tableid_ptr> HonestBrokerPrivate::Repartition(vector<tableid_ptr> &ids) {
  map<int, vector<tableid_ptr>> table_fragments;
  START_TIMER(repartition_step_one_outer_private);
  vector<std::future<vector<tableid_ptr>>> threads_repart1;
  for (auto id : ids) {
    threads_repart1.push_back(
        std::async(std::launch::async, &HonestBrokerPrivate::RepartitionStepOne,
                   this, id));
  }
  for (auto &f : threads_repart1) {
    auto k = f.get();
    for (auto j : k) {
      table_fragments[j.get()->hostnum()].emplace_back(j);
    }
  }
  END_AND_LOG_EXP3_STAT_TIMER(repartition_step_one_outer_private);

  map<int, vector<tableid_ptr>> hashed_table_fragments;
  START_TIMER(repartition_step_two_outer_private);

  vector<std::future<vector<tableid_ptr>>> threads_repart2;
  for (int i = 0; i < num_hosts; i++) {
    threads_repart2.push_back(
        std::async(std::launch::async, &HonestBrokerPrivate::RepartitionStepTwo,
                   this, i, table_fragments[i]));
  }
  for (auto &f : threads_repart2) {
    auto out = f.get();
    for (auto j : out) {
      hashed_table_fragments[j.get()->hostnum()].emplace_back(j);
    }
  }

  END_AND_LOG_EXP3_STAT_TIMER(repartition_step_two_outer_private);

  vector<tableid_ptr> coalesced_tables;
  vector<std::future<tableid_ptr>> threads_coalesced;
  START_TIMER(repartition_coalesce_outer_private);
  for (int i = 0; i < num_hosts; i++) {
    threads_coalesced.push_back(std::async(std::launch::async,
                                           &HonestBrokerPrivate::Coalesce, this,
                                           i, hashed_table_fragments[i]));
  }
  for (auto &f : threads_coalesced) {
    auto out = f.get();
    coalesced_tables.emplace_back(out);
  }
  END_AND_LOG_EXP3_STAT_TIMER(repartition_coalesce_outer_private);
  return coalesced_tables;
}

vector<tableid_ptr> HonestBrokerPrivate::Filter(vector<tableid_ptr> &ids,
                                                ::vaultdb::Expr &expr,
                                                bool in_sgx) {

  vector<tableid_ptr> filtered_tables;
  for (auto &i : ids) {
    filtered_tables.emplace_back(
        do_clients[i.get()->hostnum()]->Filter(i, expr, in_sgx));
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
  vector<std::future<tableid_ptr>> threads;
  LOG(HB_P) << "Join STARTING";
  for (auto &i : ids) {
    auto client = do_clients[i.first.get()->hostnum()];
    threads.push_back(async(launch::async, &DataOwnerClient::Join, client,
                            i.first, i.second, join, in_sgx));
  }
  for (auto &j : threads) {
    joined_tables.emplace_back(j.get());
  }
  return joined_tables;
}

vector<tableid_ptr> HonestBrokerPrivate::Sort(vector<tableid_ptr> &ids,
                                              ::vaultdb::SortDef &sort,
                                              bool in_sgx) {
  vector<tableid_ptr> sorted_tables;
  for (auto &i : ids) {
    sorted_tables.emplace_back(
        do_clients[i.get()->hostnum()]->Sort(i, sort, in_sgx));
  }
  return sorted_tables;
}

vector<tableid_ptr>
HonestBrokerPrivate::Aggregate(vector<tableid_ptr> &ids,
                               ::vaultdb::GroupByDef &groupby, bool in_sgx) {
  vector<tableid_ptr> aggregate_tables;
  for (auto &i : ids) {
    aggregate_tables.emplace_back(
        do_clients[i.get()->hostnum()]->Aggregate(i, groupby, in_sgx));
  }
  return aggregate_tables;
}
vector<tableid_ptr> HonestBrokerPrivate::MakeObli(vector<tableid_ptr> &ids,
                                                  string col_name) {

  vector<tableid_ptr> obli_tables;
  vector<std::future<tableid_ptr>> threads;
  for (auto &i : ids) {
    auto client = do_clients[i.get()->hostnum()];
    threads.push_back(
        async(launch::async, &DataOwnerClient::MakeObli, client, i, col_name));
  }
  for (auto &j : threads) {
    obli_tables.emplace_back(j.get());
  }
  return obli_tables;
}
void  HonestBrokerPrivate::SetControlFlowNotAnon(bool not_anon) {
  cf.set_not_anon(not_anon);
}

void HonestBrokerPrivate::SetControlFlowColName(string name) {
  cf.add_cf_name_strings(name);
}

void HonestBrokerPrivate::ResetControlFlowCols() {
  cf.Clear();
}

void HonestBrokerPrivate::SetControlFlowColNames(vector<string> names) {
  for (auto &name : names) {
    cf.add_cf_name_strings(name);
  }
}

::vaultdb::ControlFlowColumn HonestBrokerPrivate::GetControlFlowColID() {
  return cf;
}

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

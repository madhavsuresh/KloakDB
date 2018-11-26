//
// Created by madhav on 9/28/18.
//

#ifndef PROJECT_HONESTBROKERPRIVATE_H
#define PROJECT_HONESTBROKERPRIVATE_H

#include "DataOwnerClient.h"
#include "InfoPrivate.h"
#include "operators/Generalize.h"
#include <map>
#include <mutex>
#include <string>
#include <vector>

using namespace std;
using namespace vaultdb;
typedef shared_ptr<const TableID> tableid_ptr;
typedef string column;
typedef string dbname;

typedef struct to_gen {
    string column;
    string dbname;
    vector<tableid_ptr> scan_tables;
} to_gen_t;

class HonestBrokerPrivate : public InfoPrivate {

public:
  HonestBrokerPrivate(string address);
  ~HonestBrokerPrivate();
  void Shutdown();
  int RegisterHost(string hostName);
  int NumHosts();
  vector<shared_ptr<const TableID>>
  Repartition(vector<std::shared_ptr<const TableID>> &ids);
  vector<shared_ptr<const TableID>>
  RepartitionStepOne(shared_ptr<const TableID> id);
  vector<shared_ptr<const TableID>>
  RepartitionStepTwo(int host_num,
                     vector<shared_ptr<const TableID>> table_fragments);
  shared_ptr<const TableID> Coalesce(int host_num,
                                     vector<shared_ptr<const TableID>> tables);
  vector<shared_ptr<const TableID>>
  Filter(vector<shared_ptr<const TableID>> &ids, Expr &expr);
  vector<shared_ptr<const TableID>> Sort(vector<shared_ptr<const TableID>> &ids,
                                         SortDef &sort);
  vector<shared_ptr<const TableID>>
  Join(vector<pair<shared_ptr<const TableID>, shared_ptr<const TableID>>> &ids,
       JoinDef &join, bool in_sgx);
  vector<shared_ptr<const TableID>>
  Aggregate(vector<shared_ptr<const TableID>> &ids, GroupByDef &groupby);
  ControlFlowColumn GetControlFlowColID();
  void SetControlFlowColID(int col_ID);
  void SetControlFlowColName(string name);
  int RegisterPeerHosts();
  vector<shared_ptr<const TableID>>
  Generalize(string table_name, string column, string dbname,
             vector<shared_ptr<const TableID>> scanned_tables, int gen_level);
    unordered_map<table_name,vector<tableid_ptr>>
    Generalize(unordered_map<table_name, to_gen_t> in, int gen_level);
  vector<shared_ptr<const TableID>> ClusterDBMSQuery(string dbname,
                                                     string query);
  shared_ptr<const TableID> DBMSQuery(int host_num, string dbname,
                                      string query);
  void FreeTables(vector<tableid_ptr> &ids);
  void WaitForAllHosts();

private:
  mutex registrationMutex;
  // Expected num hosts is used for the bootstrapping process
  int expected_num_hosts;
  int num_hosts;
  ControlFlowColumn cf;
  vector<string> remoteHostnames;
  map<int, string> numToHostMap;
  map<int, DataOwnerClient *> do_clients;
};

#endif // PROJECT_HONESTBROKERPRIVATE_H

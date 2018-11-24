//
// Created by madhav on 10/1/18.
//

#ifndef PROJECT_DATAOWNERCLIENT_H
#define PROJECT_DATAOWNERCLIENT_H

#include "../data/postgres_client.h"
#include "vaultdb.grpc.pb.h"
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <grpcpp/grpcpp.h>
#include <string>

void table_schema_to_proto_schema(table_t *t, vaultdb::Schema *s);

using namespace std;
using namespace vaultdb;
class DataOwnerClient {

public:
  DataOwnerClient(int host_num, shared_ptr<grpc::Channel> channel)
      : stub_(DataOwner::NewStub(channel)), host_num(host_num) {}

  void Shutdown();
  shared_ptr<const TableID> DBMSQuery(string dbname, string query);
  void GetPeerHosts(map<int, string> numToHostsMap);
  table_t *GetTable(shared_ptr<const TableID> id_ptr);
  void *FreeTable(std::shared_ptr<const ::vaultdb::TableID> id_ptr);

  shared_ptr<const TableID> Filter(shared_ptr<const TableID>, Expr);
  vector<shared_ptr<const TableID>>
  RepartitionStepOne(shared_ptr<const TableID> tid);
  vector<shared_ptr<const TableID>>
  RepartitionStepTwo(vector<shared_ptr<const TableID>> table_fragments);
  shared_ptr<const TableID>
  CoalesceTables(vector<shared_ptr<const TableID>> &tables);

  shared_ptr<const TableID> Sort(shared_ptr<const TableID> tid, SortDef sort);

  shared_ptr<const TableID> Aggregate(shared_ptr<const TableID> tid,
                                      GroupByDef groupby);
  shared_ptr<const TableID> Join(shared_ptr<const TableID> left_tid,
                                 shared_ptr<const TableID> right_tid,
                                 JoinDef join, bool in_sgx);

  shared_ptr<const TableID> GenZip(shared_ptr<const TableID> gen_map,
                                   shared_ptr<const TableID> scan_table);
  // TODO(madhavsuresh): this needs to be renamed to be consistent
  int SendTable(table_t *t);

private:
  int host_num;
  unique_ptr<DataOwner::Stub> stub_;
};

#endif // PROJECT_DATAOWNERCLIENT_H

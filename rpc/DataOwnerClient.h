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

class DataOwnerClient {

public:
  DataOwnerClient(int host_num, std::shared_ptr<grpc::Channel> channel)
      : stub_(vaultdb::DataOwner::NewStub(channel)), host_num(host_num) {}

  void Shutdown();
  std::shared_ptr<const ::vaultdb::TableID> DBMSQuery(std::string dbname,
                                                      std::string query);
  void GetPeerHosts(std::map<int, std::string> numToHostsMap);
  table_t * GetTable(std::shared_ptr<const ::vaultdb::TableID> id_ptr);

  std::shared_ptr<const ::vaultdb::TableID>
  Filter(std::shared_ptr<const ::vaultdb::TableID>, ::vaultdb::Expr);
  std::vector<std::shared_ptr<const ::vaultdb::TableID>>
  RepartitionStepOne(std::shared_ptr<const ::vaultdb::TableID> tid);
  std::vector<std::shared_ptr<const ::vaultdb::TableID>> RepartitionStepTwo(
      std::vector<std::shared_ptr<const ::vaultdb::TableID>> table_fragments);
  std::shared_ptr<const ::vaultdb::TableID> CoalesceTables(
      std::vector<std::shared_ptr<const ::vaultdb::TableID>> &tables);

  std::shared_ptr<const ::vaultdb::TableID>
  Sort(std::shared_ptr<const ::vaultdb::TableID> tid, ::vaultdb::SortDef sort);

  std::shared_ptr<const ::vaultdb::TableID>
  Aggregate(std::shared_ptr<const ::vaultdb::TableID> tid,
            ::vaultdb::GroupByDef groupby);
  std::shared_ptr<const ::vaultdb::TableID>
  Join(std::shared_ptr<const ::vaultdb::TableID> left_tid,
       std::shared_ptr<const ::vaultdb::TableID> right_tid,
       ::vaultdb::JoinDef join);

  std::shared_ptr<const ::vaultdb::TableID>
  GenZip(std::shared_ptr<const ::vaultdb::TableID> gen_map, std::shared_ptr<const ::vaultdb::TableID> scan_table);
  // TODO(madhavsuresh): this needs to be renamed to be consistent
  int SendTable(table_t *t);

private:
  int host_num;
  std::unique_ptr<vaultdb::DataOwner::Stub> stub_;
};

#endif // PROJECT_DATAOWNERCLIENT_H

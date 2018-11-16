//
// Created by madhav on 9/28/18.
//

#ifndef PROJECT_HONESTBROKERPRIVATE_H
#define PROJECT_HONESTBROKERPRIVATE_H

#include "DataOwnerClient.h"
#include "InfoPrivate.h"
#include <map>
#include <mutex>
#include <string>
#include <vector>

class HonestBrokerPrivate : public InfoPrivate {

public:
  HonestBrokerPrivate(std::string address);
  int RegisterHost(std::string hostName);
  int NumHosts();
  std::vector<std::shared_ptr<const ::vaultdb::TableID>>
  Repartition(std::vector<std::shared_ptr<::vaultdb::TableID>> &ids);
  std::vector<std::shared_ptr<const ::vaultdb::TableID>>
  RepartitionStepOne(std::shared_ptr<::vaultdb::TableID> id);
  std::vector<std::shared_ptr<const ::vaultdb::TableID>> RepartitionStepTwo(
      int host_num,
      std::vector<std::shared_ptr<const ::vaultdb::TableID>> table_fragments);
  std::shared_ptr<const ::vaultdb::TableID>
  Coalesce(int host_num,
           std::vector<std::shared_ptr<const ::vaultdb::TableID>> tables);
  std::vector<std::shared_ptr<const ::vaultdb::TableID>>
  Filter(std::vector<std::shared_ptr<const ::vaultdb::TableID>> &ids,
         ::vaultdb::Expr &expr);
  std::vector<std::shared_ptr<const ::vaultdb::TableID>>
  Sort(std::vector<std::shared_ptr<const ::vaultdb::TableID>> &ids,
       ::vaultdb::SortDef &sort);
  std::vector<std::shared_ptr<const ::vaultdb::TableID>>
  Join(std::vector<std::pair<std::shared_ptr<const ::vaultdb::TableID>,
                             std::shared_ptr<const ::vaultdb::TableID>>> &ids,
       ::vaultdb::JoinDef &join);
  std::vector<std::shared_ptr<const ::vaultdb::TableID>>
  Aggregate(std::vector<std::shared_ptr<const ::vaultdb::TableID>> &ids,
            ::vaultdb::GroupByDef &groupby);
  int GetControlFlowColID();
  void SetControlFlowColID(int col_ID);
  int RegisterPeerHosts();
  void Generalize(std::string table_name, std::string column, std::string dbname);

  ::vaultdb::TableID DBMSQuery(int host_num, std::string dbname,
                               std::string query);
   void WaitForAllHosts();

private:
  std::mutex registrationMutex;
  // Expected num hosts is used for the bootstrapping process
  int expected_num_hosts;
  int num_hosts;
  ::vaultdb::ControlFlowColumn cf;
  std::vector<std::string> remoteHostnames;
  std::map<int, std::string> numToHostMap;
  std::map<int, DataOwnerClient *> do_clients;
};

#endif // PROJECT_HONESTBROKERPRIVATE_H

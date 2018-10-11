//
// Created by madhav on 10/1/18.
//

#ifndef PROJECT_DATAOWNERPRIVATE_H
#define PROJECT_DATAOWNERPRIVATE_H

#include "../postgres_client.h"
#include "DataOwnerClient.h"
#include "HonestBrokerClient.h"
#include "InfoPrivate.h"
#include <string>

class DataOwnerPrivate : public InfoPrivate {

public:
  DataOwnerPrivate(std::string hostname, std::string hb_host_name);
  int HostNum();
  int NumHosts();
  void Register();
  int AddTable(table_t *t);
  int SendTable(int worker_host_num, table_t *t);
  void SetDataOwnerClient(int host_num, std::string host_name);
  ::vaultdb::ControlFlowColumn GetControlFlowColID();
  table_t *GetTable(int table_id);

private:
  // TODO(madhavsuresh): should this be a managed pointer
  HonestBrokerClient *client;
  std::map<int, DataOwnerClient *> data_owner_clients;
  int num_hosts;
  std::string hb_host_name;
  int host_num;
  // TODO(madhavsuresh): how many tables should a table have?
  // TODO(madhavsuresh): what does this mean?
  std::mutex table_catalog_mutex;
  int table_counter;
  std::map<int, table_t *> table_catalog;
};
#endif // PROJECT_DATAOWNERPRIVATE_H

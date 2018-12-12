//
// Created by madhav on 10/1/18.
//

#ifndef PROJECT_DATAOWNERPRIVATE_H
#define PROJECT_DATAOWNERPRIVATE_H

#include "../data/postgres_client.h"
#include "DataOwnerClient.h"
#include "HonestBrokerClient.h"
#include "InfoPrivate.h"
#include <string>

class DataOwnerPrivate : public InfoPrivate {

public:
  DataOwnerPrivate(std::string hostname, std::string hb_host_name,
				   const std::string& key,
				   const std::string& cert,
				   const std::string& root);
  ~DataOwnerPrivate();
  int HostNum();
  int NumHosts();
  void Register();
  int AddTable(table_t *t);
  int AddBatch(table_batch_t *batch);
  std::pair<int,int> SendTable(int worker_host_num, table_t *t);
  void SetDataOwnerClient(int host_num, std::string host_name, 
						   	 const std::string& key,
						   	 const std::string& cert,
							 const std::string& root);
  void DeleteDataOwnerClient(int host_num);
  void FreeTable(int i);
  void FreeAllTables();
  ::vaultdb::ControlFlowColumn GetControlFlowColID();
  table_t *GetTable(int table_id);
  table_batch_t *GetBatch(int batch_id);

  const std::string& key;
  const std::string& cert;
  const std::string& root;

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
    std::mutex table_catalog_batch_mutex;
    int table_batch_counter;
    std::map<int, table_batch_t *> table_batch_catalog;
};

void proto_schema_to_table_schema(table_t *t, const vaultdb::Schema &s);
#endif // PROJECT_DATAOWNERPRIVATE_H

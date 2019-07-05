//
// Created by madhav on 10/1/18.
//

#include "DataOwnerPrivate.h"

void proto_schema_to_table_schema(table_t *t, const vaultdb::Schema &s) {
  t->schema.num_fields = s.num_fields();
  for (int i = 0; i < t->schema.num_fields; i++) {
    t->schema.fields[i].col_no = s.field(i).col_no();
    strncpy(t->schema.fields[i].field_name, s.field(i).field_name().c_str(),
            FIELD_NAME_LEN);
    switch (s.field(i).field_type()) {
    case vaultdb::FieldDesc_FieldType_FIXEDCHAR: {
      t->schema.fields[i].type = FIXEDCHAR;
      break;
    }
    case vaultdb::FieldDesc_FieldType_INT: {
      t->schema.fields[i].type = INT;
      break;
    }
    case vaultdb::FieldDesc_FieldType_DOUBLE: {
      t->schema.fields[i].type = DOUBLE;
      break;
    }
    case vaultdb::FieldDesc_FieldType_TIMESTAMP: {
      t->schema.fields[i].type = TIMESTAMP;
      break;
    }
    case vaultdb::FieldDesc_FieldType_UNSUPPORTED: {
      throw;
    }
    default: { throw; }
    }
  }
}
DataOwnerPrivate::DataOwnerPrivate(std::string host_name,
                                   std::string hb_host_name,
				   const std::string& key,
				   const std::string& cert,
				   const std::string& root)
    : InfoPrivate(host_name), key(key), cert(cert), root(root) {

  grpc::SslCredentialsOptions opts = { root,key,cert};

  this->hb_host_name = hb_host_name;
  this->num_hosts = 0;
  client = new HonestBrokerClient(grpc::CreateChannel(
      this->hb_host_name, grpc::SslCredentials(opts)));
  this->table_counter = 0;

}
DataOwnerPrivate::~DataOwnerPrivate() { delete client; }
void DataOwnerPrivate::DeleteDataOwnerClient(int host_num) {
  // delete this->client;
  delete this->data_owner_clients[host_num];
}

void DataOwnerPrivate::SetDataOwnerClient(int host_num, std::string host_name,
						   	 const std::string& key,
						   	 const std::string& cert,
							 const std::string& root)
{
  grpc::SslCredentialsOptions opts = { root,key,cert};
  data_owner_clients[host_num] = new DataOwnerClient(
      host_name, host_num,
      grpc::CreateChannel(host_name, grpc::SslCredentials(opts)));
}

table_t *DataOwnerPrivate::GetTable(int table_id) {
  return table_catalog[table_id];
}
table_batch_t *DataOwnerPrivate::GetBatch(int table_batch_id) {
  return table_batch_catalog[table_batch_id];
}

void DataOwnerPrivate::Register() {
  this->host_num = client->Register(this->HostName());
  LOG(INFO) << "Registered, host num:" << this->host_num;
}

int DataOwnerPrivate::HostNum() { return this->host_num; }

int in_null_table(uint64_t i) {return i;} ;

int DataOwnerPrivate::AddTable(table_t *t) {
    if (t == NULL) {
	LOG(INFO) << "GETTING NULL TABLE";
	in_null_table((uint64_t) t);
    }
  int table_id;
  table_catalog_mutex.lock();
  table_id = table_counter;
  table_catalog[table_id] = t;
  table_counter++;
  table_catalog_mutex.unlock();
  LOG(INFO) << "Adding table at :[" << table_id << "]";
  return table_id;
}

int DataOwnerPrivate::AddBatch(table_batch_t *batch) {
  int table_batch_id;
  table_catalog_batch_mutex.lock();
  table_batch_id = table_batch_counter;
  table_batch_catalog[table_batch_id] = batch;
  table_counter++;
  table_catalog_batch_mutex.unlock();
  return table_batch_id;
}

std::pair<int,int> DataOwnerPrivate::SendTable(int worker_host_num, table_t *t) {
  auto worker_client = this->data_owner_clients[worker_host_num];
  int table_id = worker_client->SendTable(t, true);
  LOG(INFO) << "sent table to: [" << worker_host_num << "],received at:["
            << table_id << "]";
  return std::make_pair(worker_host_num, table_id);
}

int DataOwnerPrivate::NumHosts() {
  if (num_hosts == 0) {
    num_hosts = client->GetNumHosts();
  }
  return num_hosts;
}

void DataOwnerPrivate::FreeAllTables() {
  std::map<table_t *, bool> freed_tables;
  for (const auto &pair : table_catalog) {
    if (!freed_tables[pair.second]) {
      FreeTable(pair.first);
      freed_tables[pair.second] = true;
      table_catalog[pair.first] = nullptr;
    }
  }
}

// TODO(madhavsuresh): this can get super nasty because some tables
// modify the table in place, others return new tables
// (sort, filter - in place); (join, aggregate - new tables)
void DataOwnerPrivate::FreeTable(int i) {
  if (GetTable(i) != nullptr) {
    free_table(GetTable(i));
  }
}

::vaultdb::ControlFlowColumn DataOwnerPrivate::GetControlFlowColID() {
  return client->GetControlFlowColID();
}

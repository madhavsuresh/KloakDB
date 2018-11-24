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
                                   std::string hb_host_name)
    : InfoPrivate(host_name) {

  this->hb_host_name = hb_host_name;
  this->num_hosts = 0;
  client = new HonestBrokerClient(grpc::CreateChannel(
      this->hb_host_name, grpc::InsecureChannelCredentials()));
  this->table_counter = 0;
}
DataOwnerPrivate::~DataOwnerPrivate() {
  delete client;
}
void DataOwnerPrivate::DeleteDataOwnerClient(int host_num) {
  //delete this->client;
  delete this->data_owner_clients[host_num];
}

void DataOwnerPrivate::SetDataOwnerClient(int host_num, std::string host_name) {
  data_owner_clients[host_num] = new DataOwnerClient(
      host_num,
      grpc::CreateChannel(host_name, grpc::InsecureChannelCredentials()));
}

table_t *DataOwnerPrivate::GetTable(int table_id) {
  return table_catalog[table_id];
}

void DataOwnerPrivate::Register() {
  this->host_num = client->Register(this->HostName());
  LOG(INFO) << "Registered, host num:" << this->host_num;
}

int DataOwnerPrivate::HostNum() { return this->host_num; }

int DataOwnerPrivate::AddTable(table_t *t) {
  int table_id;
  table_catalog_mutex.lock();
  table_id = table_counter;
  table_catalog[table_id] = t;
  table_counter++;
  table_catalog_mutex.unlock();
  LOG(INFO) << "Added Table [" << table_id << "]";
  return table_id;
}

int DataOwnerPrivate::SendTable(int worker_host_num, table_t *t) {
  auto worker_client = this->data_owner_clients[worker_host_num];
  int table_id = worker_client->SendTable(t);
  LOG(INFO) << "sent table to: [" << worker_host_num << "],received at:["
            << table_id << "]";
  return table_id;
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
      printf("%p %d\n", pair.second, freed_tables[pair.second]);
      FreeTable(pair.first);
      freed_tables[pair.second] = true;
      table_catalog[pair.first] = nullptr;
      printf("%p %d\n", pair.second, freed_tables[pair.second]);
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

//
// Created by madhav on 10/1/18.
//

#include "DataOwnerPrivate.h"

DataOwnerPrivate::DataOwnerPrivate(std::string host_name, std::string hb_host_name)
:InfoPrivate(host_name)
{

    this->hb_host_name = hb_host_name;
    client = new HonestBrokerClient(grpc::CreateChannel(this->hb_host_name,
            grpc::InsecureChannelCredentials()));
    this->table_counter = 0;
}


table_t * DataOwnerPrivate::GetTable(int table_id) {
    return table_catalog[table_id];
}

void DataOwnerPrivate::Register() {
    this->host_num = client->Register(this->HostName());
}

int DataOwnerPrivate::HostNum() {
    return this->host_num;
}

int DataOwnerPrivate::AddTable(table_t *t) {
    int table_id;
    table_catalog_mutex.lock();
    table_id = table_counter;
    table_catalog[table_id] = t;
    table_counter++;
    table_catalog_mutex.unlock();
    return table_id;
}

int DataOwnerPrivate::SendTable(int worker_host_num, table_t * t) {
    auto worker_client = this->data_owner_clients[worker_host_num];
    int table_id = worker_client->SendTable(t);
    return table_id;
}

int DataOwnerPrivate::NumHosts() {
    return num_hosts;
}

::vaultdb::ControlFlowColumn DataOwnerPrivate::GetControlFlowColID() {
    return client->GetControlFlowColID();
}

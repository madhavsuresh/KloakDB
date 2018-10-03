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

void DataOwnerPrivate::Register() {
    this->host_num = client->Register(this->HostName());
}

int DataOwnerPrivate::HostNum() {
    return this->host_num;
}

int DataOwnerPrivate::AddTable(table_t *t) {
    int table_id;
    this->table_catalog_mutex.lock();
    table_id = this->table_counter;
    table_catalog[table_id] = t;
    this->table_counter++;
    this->table_catalog_mutex.unlock();
    return table_id;
}

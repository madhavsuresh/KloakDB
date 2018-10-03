//
// Created by madhav on 10/1/18.
//

#ifndef PROJECT_DATAOWNERPRIVATE_H
#define PROJECT_DATAOWNERPRIVATE_H


#include <string>
#include "HonestBrokerClient.h"
#include "../postgres_client.h"
#include "InfoPrivate.h"

class DataOwnerPrivate : public InfoPrivate {

public:
    DataOwnerPrivate(std::string hostname, std::string hb_host_name);
    int HostNum();
    void Register();
    int AddTable(table_t *t);

private:
    //TODO(madhavsuresh): should this be a managed pointer
    HonestBrokerClient *client;
    int num_hosts;
    std::string hb_host_name;
    int host_num;
    //TODO(madhavsuresh): how many tables should a table have?
    //TODO(madhavsuresh): what does this mean?
    std::mutex table_catalog_mutex;
    int table_counter;
    std::map <int, table_t *> table_catalog;
};
#endif //PROJECT_DATAOWNERPRIVATE_H

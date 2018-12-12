//
// Created by madhav on 9/25/18.
//

#ifndef PROJECT_VAULTDB_H
#define PROJECT_VAULTDB_H

#include <vaultdb.grpc.pb.h>
#include <sstream>
#include <iostream>
#include <fstream>

void read(const std::string& filename, std::string& data);
using namespace std;
using namespace vaultdb;

vector<pair<shared_ptr<const TableID>, shared_ptr<const TableID>>>
zip_join_tables(vector<shared_ptr<const TableID>> &left_tables,
                vector<shared_ptr<const TableID>> &right_tables);
#endif // PROJECT_VAULTDB_H

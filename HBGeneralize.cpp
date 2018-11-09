//
// Created by madhav on 9/24/18.
//

#include "HBGeneralize.h"
#include "../TableStatistics.h"
#include "DOClient.h"
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

void HBGeneralize::Generalize(std::vector<TableStatistics> allStats) {
  int num_hosts = allStats.size();
  for (int i = 0; i < num_hosts; i++) {
    for (int j = 0; j < num_hosts; j++) {
      std::map<int, int> newMap;
      if (j == i) {
        continue;
      }
      std::map<int, int> currMap = allStats.at(j).GetMap();
      for (std::map<int, int>::iterator it = currMap.begin();
           it != currMap.end(); ++it) {
        if (newMap.count(it->first) == 1) {
          newMap[it->first] += it->second;
        } else {
          newMap[it->first] = it->second;
        }
      }
    }
  }
}

void HBGeneralize::generalize() {
  std::vector<TableStatistics> allStats;
  VaultDBClient client(
      grpc::CreateChannel("0.0.0.0:50051", grpc::InsecureChannelCredentials()));
  std::string db1 = "dbname=test";
  std::string q1 = "SELECT floor, COUNT(*) from rpc_test group by floor;";
  std::string q2 = "SELECT floor, COUNT(*) from rpc_test1 group by floor;";
  table_t *t1 = client.GetTable(db1, q1);
  table_t *t2 = client.GetTable(db1, q2);
  TableStatistics ts;
  ts.IngestAllocatedTable(t1);
  TableStatistics ts2;
  ts2.IngestAllocatedTable(t2);
  allStats.push_back(ts);
  allStats.push_back(ts2);
  HBGeneralize::Generalize(allStats);
}

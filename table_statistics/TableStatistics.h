//
// Created by madhav on 9/27/18.
//

#ifndef PROJECT_TABLESTATISTICS_H
#define PROJECT_TABLESTATISTICS_H >
#include "../postgres_client/postgres_client.h"
#include <map>

class TableStatistics {
public:
  TableStatistics();

  void IngestAllocatedTable(table_t *t);
  int GetCount(int colVal);
  std::map<int, int> GetMap();

  void SetCount(int colVal, int count);

private:
  // TODO(madhavsuresh): for now this only supports int types
  std::map<int, int> histogram;
  std::string colName;
};

#endif // PROJECT_TABLESTATISTICS_H

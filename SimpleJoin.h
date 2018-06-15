#include <vector>
#ifndef SimpleJoin
#define SimpleJoin
using namespace std;
#include "Repartition.h"

vector<vector<int>> simplejoin(vector<vector<int>> table1,
                               vector<vector<int>> table2, int join_column1_id,
                               int join_column2_id);
uint8_t *hashjoin(uint8_t * left_table_buf, uint8_t * right_table_buf,
                  uint8_t * eq_left, uint8_t * eq_right);

#endif

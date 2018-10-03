//
// Created by madhav on 9/27/18.
//

#include "TableStatistics.h"




TableStatistics::TableStatistics() {}

//IngestAllocatedTable ingests a table and frees the associated table
void TableStatistics::IngestAllocatedTable(table_t *t) {
    //this->colName.reserve(FIELD_NAME_LEN);
    //this->colName.copy(t->schema.fields[0].field_name, 0, FIELD_NAME_LEN);
    if (t->schema.fields[0].type != INT) {
        throw;
    }
    //TODO(madhavsuresh): copy this in a way which reduces memory overhead.
    for (int i = 0; i < t->num_tuples; i++) {
        tuple_t * tup = get_tuple(i, t);
        this->SetCount(get_int_field(tup, 0), get_int_field(tup, 1));
    }
    free_table(t);
}


std::map<int,int> TableStatistics::GetMap() {
    return this->histogram;
}

void TableStatistics::SetCount(int colVal, int count) {
    this->histogram[colVal] = count;
}

int TableStatistics::GetCount(int colVal) {
    return this->histogram[colVal];
}

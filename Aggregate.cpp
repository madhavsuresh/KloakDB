//
// Created by madhav on 7/13/18.
//
#include "vaultdb_generated.h"
#include "Expressions.h"
#include <map>

void aggregate_min(flatbuffers::FlatBufferBuilder * b) {

}

template <class T>
void build_aggregate_from_map(flatbuffers::FlatBufferBuilder &b, std::map<T, int> agg) {
    //first create a two
    auto column_name = b.CreateString("count");
    auto field_desc = CreateFieldDesc(b, FieldType_INT, column_name,100);
    
}

template <class T>
void aggregate_count(flatbuffers::FlatBufferBuilder *b, const GroupByDef *gb_def, const Table *t) {
    auto tuples = t->tuples();
    auto type = t->schema()->fielddescs()->Get(gb_def->colno())->type();
    auto col = gb_def->colno();
    std::map<T, int> agg;
    for (int i = 0; i < (int) tuples->Length(); i++) {
        auto t = tuples->GetMutableObject(i);
        auto f = t->fields()->LookupByKey(col);
        agg.count(f->genval_type());
        if (agg.count(f->genval_as<T>()) == 0) {
            agg[f->genval_as<T>()] = 1;
        } else {
            agg[f->genval_as<T>()]++;
        }
    }
}

void aggregate(uint8_t *table_buf, uint8_t *groupby_defs_buf) {
    flatbuffers::FlatBufferBuilder builder(1024);
    Table * table;
    table = flatbuffers::GetMutableRoot<Table>(table_buf);
    GroupByDef * groupby_defs;
    groupby_defs = flatbuffers::GetMutableRoot<GroupByDef>(groupby_defs_buf);
    switch (groupby_defs->optype()) {
        case GroupByType_MIN:break;
        case GroupByType_COUNT: {
            auto type = table->schema()->fielddescs()->LookupByKey(groupby_defs->colno())->type();
            switch (type) {
                case FieldType_VARCHAR: {
                    aggregate_count<std::string>(&builder, groupby_defs, table);
                    break;
                }
                case FieldType_INT: {
                    aggregate_count<int>(&builder, groupby_defs, table);
                    break;
                }
                case FieldType_TIMESTAMP : {
                    aggregate_count<int>(&builder, groupby_defs, table);
                    break;
                }
                case FieldType_UNSUPPORTED:break;
            }
        }
        case GroupByType_UNSUPPPORTED:break;
    }
}
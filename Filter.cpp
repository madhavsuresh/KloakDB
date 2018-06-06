#include "Filter.h"
#include "Expressions.h"
#include "vaultdb_generated.h"
#include <iostream>

void filter(uint8_t * table_buf, uint8_t * expr_buf) {
    flatbuffers::FlatBufferBuilder builder(1024);
    auto table = flatbuffers::GetRoot<Table>(table_buf);
    auto expr = flatbuffers::GetRoot<Expr>(expr_buf);
    auto &tuples = *table->tuples();
    auto schema = table->schema();
    std::vector<const Tuple*> tuple_vector;
    for (auto tuple : tuples) {
	if(expression_eval(expr, tuple)) {
	    tuple_vector.push_back(tuple);
	}
    }
    auto output_tuples = builder.CreateVector(tuple_vector);
    auto output_schema = CreateSchema(builder,schema->fielddescs());
    auto output_table = CreateTable(builder, schema, output_tuples);
    builder.Finish(output_table);
    uint8_t *ret_buf = builder.GetBufferPointer();
}

#include "Filter.h"
#include "Expressions.h"
#include "vaultdb_generated.h"
#include "postgres_client.h"
#include <iostream>

// filter mutates the table in place.
table_t * filter(table_t * table, expr_t * expr) {
    for (int i = 0; i < table->num_tuples; i++) {
        tuple_t * t = get_tuple(i, table);
        if (expression_eval(t, expr)) {
            t->is_dummy = false;
        } else {
            t->is_dummy = true;
        }
    }
    return table;
}

table_t * filter(uint8_t *table_buf, uint8_t *expr_buf) {
  flatbuffers::FlatBufferBuilder builder(1024);
  auto table = flatbuffers::GetMutableRoot<Table>(table_buf);
  auto expr = flatbuffers::GetRoot<Expr>(expr_buf);
  auto tuples = table->mutable_tuples();
  for (int i = 0; i < (int)tuples->Length(); i++) {
    auto t = tuples->GetMutableObject(i);
    if (expression_eval(expr, t)) {
      t->mutate_isdummy(false);
    } else {
      t->mutate_isdummy(true);
    }
  }

}

#include "Filter.h"
#include "Expressions.h"
#include "vaultdb_generated.h"
#include <iostream>

void filter(uint8_t *table_buf, uint8_t *expr_buf) {
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

#include "Filter.h"
#include "Expressions.h"
#include "postgres_client.h"
#include "vaultdb_generated.h"
#include <iostream>

// filter mutates the table in place.
table_t *filter(table_t *table, expr_t *expr) {
  for (int i = 0; i < table->num_tuples; i++) {
    tuple_t *t = get_tuple(i, table);
    if (expression_eval(t, expr)) {
      t->is_dummy = false;
    } else {
      t->is_dummy = true;
    }
  }
  return table;
}

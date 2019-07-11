#include "Truncate.h"
#include <strings.h>

table_t *truncate_dummies(table_t *t) {
  int i = 0;
  for (; i < t->num_tuples; i++) {
    if (get_tuple(i, t)->is_dummy) {
      break;
    }
  }
  t->num_tuples = i;
  int page_to_truncate_after = get_page_num_for_tuple(i, t);
  for (int j = page_to_truncate_after+1; j < t->num_tuple_pages; j++) {
    free(t->tuple_pages[j]);
  }
  t->num_tuple_pages = page_to_truncate_after + 1;
  return t;
}

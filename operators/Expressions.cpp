#include "Expressions.h"
#include <iostream>

bool eq_expr(tuple *t, expr_t *ex) {
  switch (ex->field_val.type) {
  case FIXEDCHAR: {
    // TODO(madhavsuresh): turn this into a proper error
    throw;
  }
  case INT: {
    if (t->field_list[ex->colno].f.int_field.val ==
        ex->field_val.f.int_field.val) {
      return true;
    } else {
      return false;
    }
  }
  case UNSUPPORTED:
    throw;
  case TIMESTAMP:
    throw;
  case DOUBLE:
    throw;
  }
}

bool expression_eval(tuple_t *t, expr_t *ex) {
  switch (ex->expr_type) {
  case EQ_EXPR: {
    return eq_expr(t, ex);
  }
  case UNSUPPORTED_EXPR: {
    throw;
  }
  case LIKE_EXPR:
    throw;
  }
}

#include "Expressions.h"
#include <iostream>
#include <cstring>

bool eq_expr(tuple *t, expr_t *ex) {
  switch (ex->field_val.type) {
  case FIXEDCHAR: {
    return strncmp(t->field_list[ex->colno].f.fixed_char_field.val,
                   ex->field_val.f.fixed_char_field.val, FIXEDCHAR_LEN) == 0;
  }
  case INT: {
    return t->field_list[ex->colno].f.int_field.val ==
           ex->field_val.f.int_field.val;
  }
  case UNSUPPORTED:
    throw;
  case TIMESTAMP:
    throw;
  case DOUBLE_V:
    throw;
  }
}

bool like_expr(tuple *t, expr_t *ex) {
  if (ex->field_val.type != FIXEDCHAR) {
    throw;
  }
  if (strstr(t->field_list[ex->colno].f.fixed_char_field.val, ex->field_val.f.fixed_char_field.val) != NULL) {
    return true;
  }
  return false;
}

bool expression_eval(tuple_t *t, expr_t *ex) {
  switch (ex->expr_type) {
  case EQ_EXPR: {
    return eq_expr(t, ex);
  }
  case NEQ_EXPR: {
    return !eq_expr(t, ex);
  }
  case UNSUPPORTED_EXPR: {
    throw;
  }
  case LIKE_EXPR:
    return like_expr(t, ex);
  }
}

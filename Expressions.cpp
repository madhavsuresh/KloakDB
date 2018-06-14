#include "Expressions.h"
#include <iostream>

bool expression_eval(const Expr *expr, const Tuple *tuple) {
  auto union_type = expr->expr_type();

  switch (union_type) {
  case ExprUnion_EqExpr: {
    auto eqexpr = expr->expr_as_EqExpr();
    auto colno = eqexpr->colno();
    auto tup_field = tuple->fields()->LookupByKey(colno);
    if (tup_field->val_type() != eqexpr->val_type()) {
      return false;
      // TODO: add logging
    }
    switch (eqexpr->val_type()) {
    case FieldVal_VarCharField:
      break;
    case FieldVal_IntField:
      if (eqexpr->val_as_IntField()->val() ==
          tup_field->val_as_IntField()->val()) {
        return true;
      } else {
        return false;
      }
      break;
    case FieldVal_TimeStampField:
      break;
    };
    break;
  }
  };
}

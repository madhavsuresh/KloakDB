#include "Expressions.h"
#include "postgres_client.h"
#include <iostream>

bool eq_expr(tuple * t, expr_t * ex) {
    switch (ex->field_val.type) {
        case FIXEDCHAR: {
            // TODO(madhavsuresh): turn this into a proper error
            printf("ERROR UNSUPPORTED");
        }
        case INT: {
            if (t->field_list[ex->colno].f.int_field.val ==
                ex->field_val.f.int_field.val) {
                return true;
            } else {
                return false;
            }
        }
    }
}


bool expression_eval(tuple_t * t, expr_t * ex) {
    switch (ex->expr_type) {
        case EQ_EXPR : {
            return eq_expr(t, ex);
        }
        case UNSUPPORTED_EXPR :{
            printf("ERROR UNSUPPORTED");

        }
    }

}

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
          tup_field->genval_as_IntField()->val()) {
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

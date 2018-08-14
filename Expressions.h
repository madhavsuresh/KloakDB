#include "vaultdb_generated.h"
#include "postgres_client.h"

bool expression_eval(const Expr* expr, const Tuple* tuple);
bool expression_eval(tuple_t * t, expr_t * ex);

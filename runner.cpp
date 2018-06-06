#include "postgres_client.h"
#include "Filter.h"
#include "flatbuffers/minireflect.h"
#include <iostream>

int main() {
    std::string query_string("SELECT * FROM t_random_300");
    std::string dbname("dbname=test");
    auto table = postgres_query_writer(query_string, dbname);

    flatbuffers::FlatBufferBuilder b(1024);
    auto int_field = CreateIntField(b, 2);
    auto eqexpr = CreateEqExpr(b, 1, FieldVal_IntField, int_field.Union());
    auto expr = CreateExpr(b, ExprUnion_EqExpr, eqexpr.Union());
    b.Finish(expr);
    //b.Finish(int_field);
    uint8_t * buff = b.GetBufferPointer();
    int size = b.GetSize();
//auto expr2 = flatbuffers::GetRoot<Expr>(buff);
    //std::cout << ii->val();
    //uint8_t* eq_expr_buf  = b.GetBufferPointer();

    filter(table, buff);
}

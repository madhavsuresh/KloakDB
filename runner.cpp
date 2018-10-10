#include "Filter.h"
#include "Repartition.h"
#include "flatbuffers/minireflect.h"
#include "postgres_client.h"
#include <iostream>

int main() {
  std::string query_string("SELECT * FROM t_random_500");
  std::string dbname("dbname=test");
  auto table3 = postgres_query_writer(query_string, dbname);

  /*
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

  auto table = table3.data();
  repart_step_one(table);
  filter(table, buff);

  auto table2 = flatbuffers::GetRoot<Table>(table);
  auto &tuples = *table2->tuples();
  std::cout << std::endl;
  for (auto t : tuples) {
      if(!t->isdummy()) {
          for (auto f : *t->fields()) {
              std::cout << f->val_as_IntField()->val() << "| ";
          }
              std::cout << std::endl;
      }
//	std::cout << std::endl;
  }
   */
}

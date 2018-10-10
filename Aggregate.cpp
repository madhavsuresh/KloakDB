//
//
//
#include "Aggregate.h"
#include "Expressions.h"
#include "vaultdb_generated.h"
#include <map>

uint8_t *build_aggregate_from_map_str(flatbuffers::FlatBufferBuilder &b,
                                      std::map<std::string, int> agg,
                                      const FieldDesc *fd) {
  // first create a two
  std::vector<flatbuffers::Offset<FieldDesc>> field_vector_desc;
  auto col_name1 = b.CreateString("count");
  auto fd1 = CreateFieldDesc(b, FieldType_INT, col_name1, 100);
  field_vector_desc.push_back(fd1);
  auto col_name2 = b.CreateString(fd->name());
  auto fd2 = CreateFieldDesc(b, fd->type(), col_name2, fd->col());
  field_vector_desc.push_back(fd2);
  auto fielddescs = b.CreateVectorOfSortedTables(&field_vector_desc);
  auto schema_offset = CreateSchema(b, fielddescs);

  std::vector<flatbuffers::Offset<Tuple>> tuple_vector;
  for (const auto &agg_pair : agg) {
    std::vector<flatbuffers::Offset<Field>> field_vector;
    switch (fd->type()) {
    case FieldType_VARCHAR: {
      auto s = b.CreateString(agg_pair.first);
      FieldBuilder fb1(b);
      auto f = CreateVarCharField(b, s);
      fb1.add_fieldnum(0);
      fb1.add_val_type(FieldVal_VarCharField);
      fb1.add_val(f.Union());
      field_vector.push_back(fb1.Finish());
      break;
    }
    }
    auto f = CreateIntField(b, agg_pair.second);
    FieldBuilder fb2(b);
    fb2.add_fieldnum(1);
    fb2.add_val_type(FieldVal_IntField);
    fb2.add_val(f.Union());
    field_vector.push_back(fb2.Finish());
    auto row = b.CreateVector(field_vector);
    auto tuple = CreateTuple(b, schema_offset, row);
    tuple_vector.push_back(tuple);
  }
  auto tuples = b.CreateVector(tuple_vector);
  auto table = CreateTable(b, schema_offset, tuples);
  b.Finish(table);
  auto buf = b.GetBufferPointer();
  return buf;
}

uint8_t *build_aggregate_from_map_int(flatbuffers::FlatBufferBuilder &b,
                                      std::map<int, int> agg,
                                      const FieldDesc *fd) {
  // first create a two
  std::vector<flatbuffers::Offset<FieldDesc>> field_vector_desc;
  auto col_name1 = b.CreateString("count");
  auto fd1 = CreateFieldDesc(b, FieldType_INT, col_name1, 100);
  field_vector_desc.push_back(fd1);
  auto col_name2 = b.CreateString(fd->name());
  auto fd2 = CreateFieldDesc(b, fd->type(), col_name2, fd->col());
  field_vector_desc.push_back(fd2);
  auto fielddescs = b.CreateVectorOfSortedTables(&field_vector_desc);
  auto schema_offset = CreateSchema(b, fielddescs);

  std::vector<flatbuffers::Offset<Tuple>> tuple_vector;
  for (const auto &agg_pair : agg) {
    std::vector<flatbuffers::Offset<Field>> field_vector;
    switch (fd->type()) {
    case FieldType_INT: {
      auto f = CreateIntField(b, (int32_t)agg_pair.first);
      FieldBuilder fb1(b);
      fb1.add_fieldnum(0);
      fb1.add_val_type(FieldVal_IntField);
      fb1.add_val(f.Union());
      auto out = fb1.Finish();
      field_vector.push_back(out);
      break;
    }
    case FieldType_TIMESTAMP: {
      auto f = CreateIntField(b, (int32_t)agg_pair.first);
      FieldBuilder fb1(b);
      fb1.add_fieldnum(0);
      fb1.add_val_type(FieldVal_IntField);
      fb1.add_val(f.Union());
      field_vector.push_back(fb1.Finish());
      break;
    }
    case FieldType_UNSUPPORTED:
      break;
    }
    auto f = CreateIntField(b, agg_pair.second);
    FieldBuilder fb2(b);
    fb2.add_fieldnum(1);
    fb2.add_val_type(FieldVal_IntField);
    fb2.add_val(f.Union());
    field_vector.push_back(fb2.Finish());
    auto row = b.CreateVector(field_vector);
    auto tuple = CreateTuple(b, schema_offset, row);
    tuple_vector.push_back(tuple);
  }
  auto tuples = b.CreateVector(tuple_vector);
  auto table = CreateTable(b, schema_offset, tuples);
  b.Finish(table);
  auto buf = b.GetBufferPointer();
  return buf;
}

template <class T>
uint8_t *aggregate_count_varchar(flatbuffers::FlatBufferBuilder &b,
                                 const GroupByDef *gb_def, const Table *t) {
  auto tuples = t->tuples();
  auto col = gb_def->colno();
  std::map<T, int> agg;
  for (int i = 0; i < (int)tuples->Length(); i++) {
    auto tup = tuples->GetMutableObject(i);
    if (tup->isdummy()) {
      continue;
    }
    auto f = tup->fields()->LookupByKey(col);
    if (agg.count(f->genval_as_VarCharField()->val()->str()) == 0) {
      agg[f->genval_as_VarCharField()->val()->str()] = 1;
    } else {
      agg[f->genval_as_VarCharField()->val()->str()]++;
    }
  }
  auto ret = build_aggregate_from_map_str(
      b, agg, t->schema()->fielddescs()->LookupByKey(gb_def->colno()));
  return ret;
}

template <class T>
uint8_t *aggregate_count_int(flatbuffers::FlatBufferBuilder &b,
                             const GroupByDef *gb_def, const Table *t) {
  auto tuples = t->tuples();
  auto col = gb_def->colno();
  std::map<T, int> agg;
  for (int i = 0; i < (int)tuples->Length(); i++) {
    auto tup = tuples->GetMutableObject(i);
    if (tup->isdummy()) {
      continue;
    }
    auto f = tup->fields()->LookupByKey(col);
    if (agg.count(f->val_as_IntField()->val()) == 0) {
      agg[f->genval_as_IntField()->val()] = 1;
    } else {
      agg[f->genval_as_IntField()->val()]++;
    }
  }
  auto ret = build_aggregate_from_map_int(
      b, agg, t->schema()->fielddescs()->LookupByKey(gb_def->colno()));
  return ret;
}

template <class T>
uint8_t *aggregate_count_timestamp(flatbuffers::FlatBufferBuilder &b,
                                   const GroupByDef *gb_def, const Table *t) {
  auto tuples = t->tuples();
  auto col = gb_def->colno();
  std::map<T, int> agg;
  for (int i = 0; i < (int)tuples->Length(); i++) {
    auto tup = tuples->GetMutableObject(i);
    if (tup->isdummy()) {
      continue;
    }
    auto f = tup->fields()->LookupByKey(col);
    if (agg.count(f->genval_as_TimeStampField()->val()) == 0) {
      agg[f->genval_as_TimeStampField()->val()] = 1;
    } else {
      agg[f->genval_as_TimeStampField()->val()]++;
    }
  }
  auto ret = build_aggregate_from_map_int(
      b, agg, t->schema()->fielddescs()->LookupByKey(gb_def->colno()));
  return ret;
}

template <class T>
uint8_t *aggregate_count(flatbuffers::FlatBufferBuilder &b,
                         const GroupByDef *gb_def, const Table *t,
                         FieldType ft) {
  std::map<T, int> agg;
  switch (ft) {
  case FieldType_VARCHAR: {
    return aggregate_count_varchar<std::string>(b, gb_def, t);
  }
  case FieldType_INT: {
    return aggregate_count_int<int>(b, gb_def, t);
  }
  case FieldType_TIMESTAMP: {
    return aggregate_count_timestamp<int>(b, gb_def, t);
  }
  case FieldType_UNSUPPORTED:
    break;
  }
}

table_t *aggregate_count(table_t *t, uint32_t colno) {
  std::map<std::string, int> agg_map;
  // This will always be the same.
  FIELD_TYPE type = get_tuple(0, t)->field_list[colno].type;
  for (int i = 0; i < t->num_tuples; i++) {
    tuple_t *tup = get_tuple(i, t);
    std::string key;
    switch (type) {
    case INT: {
      key = std::to_string(tup->field_list[colno].f.int_field.val);
    }
    case FIXEDCHAR: {
      key = std::string(tup->field_list[colno].f.fixed_char_field.val);
    }
    case UNSUPPORTED:
      break;
    }
    if (agg_map[key] == 0) {
      agg_map[key] = 1;
    } else {
      agg_map[key]++;
    }
  }
  free_table(t);
  agg_map.size();
  for (const auto &agg_pair : agg) {
    // TODO(madhavsuresh): output representation of aggregate
    // merge aggregate outputs
    agg_pair.first
  }
  return nullptr;
}

table_t *create_aggregate_output_table(table_t *t, uint32_t colno){
    t->schema.fields[colno].field_name

}

table_t *aggregate(table_t *t, groupby_def_t *def) {
  switch (def->type) {
  case COUNT: {
    return aggregate_count(t, def->colno);
    break;
  }
  case MINX: {
    printf("UNIMPLEMENTED");
  }
  case GROUPBY_UNSUPPORTED: {
    printf("UNSUPPORTED");
  }
  }
  for (int i = 0; i < t->num_tuples; i++) {
  }
}

uint8_t *aggregate(uint8_t *table_buf, uint8_t *groupby_defs_buf) {
  flatbuffers::FlatBufferBuilder builder(1024);
  Table *table;
  table = flatbuffers::GetMutableRoot<Table>(table_buf);
  GroupByDef *groupby_defs;
  groupby_defs = flatbuffers::GetMutableRoot<GroupByDef>(groupby_defs_buf);
  switch (groupby_defs->optype()) {
  case GroupByType_MINX:
    break;
  case GroupByType_COUNT: {
    auto type = table->schema()
                    ->fielddescs()
                    ->LookupByKey(groupby_defs->colno())
                    ->type();
    switch (type) {
    case FieldType_VARCHAR: {
      return aggregate_count<std::string>(builder, groupby_defs, table, type);
    }
    case FieldType_INT: {
      return aggregate_count<int>(builder, groupby_defs, table, type);
    }
    case FieldType_TIMESTAMP: {
      return aggregate_count<int>(builder, groupby_defs, table, type);
    }
    case FieldType_UNSUPPORTED:
      break;
    }
  }
  case GroupByType_UNSUPPPORTED:
    break;
  }
}
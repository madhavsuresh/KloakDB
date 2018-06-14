#include "vaultdb_generated.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <vector>
#include "sgx_tcrypto.h"

flatbuffers::Offset<Schema> cp_schema(const Schema *schema_to_cp,
                                      flatbuffers::FlatBufferBuilder &builder) {

  std::vector<flatbuffers::Offset<FieldDesc>> field_vector_desc;
  auto &fd_to_cp = *schema_to_cp->fielddescs();
  for (auto fd : fd_to_cp) {
    auto col_name = builder.CreateString(fd->name()->c_str());
    auto new_fd = CreateFieldDesc(builder, fd->type(), col_name, fd->col());
    field_vector_desc.push_back(new_fd);
  }
  auto fielddescs = builder.CreateVectorOfSortedTables(&field_vector_desc);
  auto schema_offset = CreateSchema(builder, fielddescs);
  return schema_offset;
}

flatbuffers::Offset<Field> cp_field(const Field *f,
                                    flatbuffers::FlatBufferBuilder &builder) {

  switch (f->val_type()) {
  case FieldVal_VarCharField: {
    auto s = builder.CreateString(f->val_as_VarCharField()->val()->c_str());
    auto var_char = CreateVarCharField(builder, s);
    FieldBuilder field_builder(builder);
    field_builder.add_fieldnum(f->fieldnum());
    field_builder.add_val_type(FieldVal_VarCharField);
    field_builder.add_val(var_char.Union());
    auto field = field_builder.Finish();
    return field;
  }
  case FieldVal_IntField: {
    auto int_field = CreateIntField(builder, f->val_as_IntField()->val());
    FieldBuilder field_builder(builder);
    field_builder.add_fieldnum(f->fieldnum());
    field_builder.add_val_type(FieldVal_IntField);
    field_builder.add_val(int_field.Union());
    auto field = field_builder.Finish();
    return field;
  }
  case FieldVal_TimeStampField: {
    auto time_stamp_field =
        CreateTimeStampField(builder, f->val_as_TimeStampField()->val());
    FieldBuilder field_builder(builder);
    field_builder.add_fieldnum(f->fieldnum());
    field_builder.add_val_type(FieldVal_TimeStampField);
    field_builder.add_val(time_stamp_field.Union());
    auto field = field_builder.Finish();
    return field;
  }
  case FieldVal_NONE: {
    // TODO: Log ERROR:
  }
  }
  return NULL;
}

flatbuffers::Offset<Tuple> cp_tuple(const Tuple *tuple_to_cp,
                                    flatbuffers::FlatBufferBuilder &builder) {
  auto schema_offset = cp_schema(tuple_to_cp->schema(), builder);
  std::vector<flatbuffers::Offset<Field>> field_vector;
  for (auto field_to_cp : *tuple_to_cp->fields()) {
    flatbuffers::Offset<Field> field_offset = cp_field(field_to_cp, builder);
    field_vector.push_back(field_offset);
  }
  auto row = builder.CreateVector(field_vector);
  return CreateTuple(builder, schema_offset, row);
}

flatbuffers::Offset<Tuple> cp_tuple(const Tuple *tuple_to_cp,
                                    flatbuffers::Offset<Schema> schema_offset,
                                    flatbuffers::FlatBufferBuilder &builder) {
  std::vector<flatbuffers::Offset<Field>> field_vector;
  for (auto field_to_cp : *tuple_to_cp->fields()) {
    flatbuffers::Offset<Field> field_offset = cp_field(field_to_cp, builder);
    field_vector.push_back(field_offset);
  }
  auto row = builder.CreateVector(field_vector);
  return CreateTuple(builder, schema_offset, row);
}

uint8_t *cp_tuples_by_index_lst(
    const flatbuffers::Vector<flatbuffers::Offset<Tuple>> *tuples,
    std::vector<int> indices) {

  flatbuffers::FlatBufferBuilder builder(1024);
  std::vector<flatbuffers::Offset<Tuple>> tuple_vector;
  if (tuples->Length() == 0) {
    // TODO: THROW ERROR
    return 0;
  }
  auto schema_offset = cp_schema(tuples->Get(0)->schema(), builder);
  for (auto i : indices) {
    auto tuple = cp_tuple(tuples->Get(i), schema_offset, builder);
    tuple_vector.push_back(tuple);
  }
  auto new_tuples = builder.CreateVector(tuple_vector);
  auto table = CreateTable(builder, schema_offset, new_tuples);
  builder.Finish(table);
  uint8_t *buf = builder.GetBufferPointer();
  return buf;
}

int repart_step_one(uint8_t *table_buf) {
  // TODO: FILL IN NUMBER OF ACTUAL HOSTS
  uint64_t num_hosts = 10; // get_num_hosts_HB();
  std::srand(std::time(nullptr));
  auto table = flatbuffers::GetRoot<Table>(table_buf);
  std::vector<flatbuffers::FlatBufferBuilder> builders(num_hosts);
  auto tuples = table->tuples();
  std::map<int, std::vector<int>> rand_assignment;
  std::vector<std::pair<int, uint8_t *>> outbound_queue(num_hosts);
  // TODO: make this process better
  // The reason we first populate a random map is
  // so we can pack the Flatbuffers all at once.
  for (int i = 0; i < (int)tuples->Length(); i++) {
    rand_assignment[std::rand() % num_hosts].push_back(i);
  }
  for (std::map<int, std::vector<int>>::iterator it = rand_assignment.begin();
       it != rand_assignment.end(); it++) {
    int host = it->first;
    uint8_t *buf = cp_tuples_by_index_lst(tuples, it->second);
    outbound_queue.push_back(std::make_pair(host, buf));
  }

  // TODO: SEND OUT REPARITION
  return 0;
  // send_out_repartition_step_one()
}

auto hash_field(const Field * f) {
  sgx_sha256_hash_t hash_output;
  switch (f->val_type()) {
  case FieldVal_IntField: {
    auto val = f->val_as_IntField();
    return val;
  }
  case FieldVal_VarCharField: {
    auto val = f->val_as_VarCharField();
  }
  case FieldVal_TimeStampField: {
    auto val = f->val_as_TimeStampField();
  }
  case FieldVal_NONE: {
  }
  }
}

uint64_t hash_to_host(uint64_t control_flow_col, uint64_t num_hosts,
                      const Tuple *tuple) {
  auto field = tuple->fields()->Get(control_flow_col);
  uint64_t auto hash_val = hash_field(field);
}

int repart_step_two(std::vector<uint8_t *> tables) {

  // TODO: only support for one control flow attribute column
  uint64_t control_flow_column = 1; // TODO: get_cf_col_HB();
  uint64_t num_hosts = 10;          // TODO: get_num_hosts_HB();
  std::map<uint64_t, std::vector<flatbuffers::Offset<Tuple>>> host_tuple_vec;
  std::map<uint64_t, flatbuffers::FlatBufferBuilder> host_fbb;

  for (auto t : tables) {
    auto table = flatbuffers::GetRoot<Table>(t);
    for (auto tuple : *table->tuples()) {
      uint64_t dest_host = hash_to_host(control_flow_column, num_hosts, tuple);
      host_tuple_vec[dest_host].push_back(cp_tuple(tuple, host_fbb[dest_host]));
    }
    // iterate through table, add
  }
}

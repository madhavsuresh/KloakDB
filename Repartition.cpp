#include "sgx_tcrypto.h"
#include "vaultdb_generated.h"
#include "postgres_client.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <vector>


int repart_step_one(table_t * t, int num_hosts) {
    std::srand(std::time(nullptr));
    std::map<int, table_t *> partition_map;
    std::map<int, std::vector<int>> rand_assignment;
    for (int i = 0; i < t->num_tuples; i++) {
       rand_assignment[std::rand() % num_hosts].push_back(i);
    }

    for (auto it = rand_assignment.begin();
    it!= rand_assignment.end(); it++) {
        int host = it->first;
        std::vector<int> index_lst = it->second;
    }
    //TODO(madhavsuresh) need insert tuple method

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
  for (auto it = rand_assignment.begin();
       it != rand_assignment.end(); it++) {
    int host = it->first;
    uint8_t *buf = cp_tuples_by_index_lst(tuples, it->second);
    outbound_queue.emplace_back(std::make_pair(host, buf));
  }

  // TODO: SEND OUT REPARITION
  return 0;
  // send_out_repartition_step_one()
}

sgx_sha256_hash_t *hash_field(const Field *f) {
  auto *hash_output =
      reinterpret_cast<sgx_sha256_hash_t *>(malloc(sizeof(sgx_sha256_hash_t)));
  uint32_t size;
  const unsigned char * data_ptr;
  switch (f->val_type()) {
  case FieldVal_IntField: {
    int32_t val = f->val_as_IntField()->val();
    size = sizeof(int32_t);
    data_ptr = reinterpret_cast<const unsigned char *>(&val);
  }
  case FieldVal_VarCharField: {
    const flatbuffers::String *val = f->val_as_VarCharField()->val();
    data_ptr = reinterpret_cast<const unsigned char *>(val->c_str());
    size = val->size();
  }
  case FieldVal_TimeStampField: {
    float val = f->val_as_TimeStampField()->val();
    data_ptr = reinterpret_cast<const unsigned char *>(&val);
  }
  case FieldVal_NONE: {
  }
  }
  sgx_sha256_msg(data_ptr, size, hash_output);
  return hash_output;
}

uint32_t hash_to_host(uint64_t control_flow_col, uint64_t num_hosts,
                      const Tuple *tuple) {
  auto field = tuple->fields()->Get(control_flow_col);
  sgx_sha256_hash_t *hash_output = hash_field(field);

  //For now, we'll look at the first four words. 
  union {
      uint32_t u;
      unsigned char u8[sizeof(uint32_t)];
  } out;
  out.u8[0] = *hash_output[0];
  out.u8[1] = *hash_output[1];
  out.u8[2] = *hash_output[2];
  out.u8[3] = *hash_output[3];
  return out.u % num_hosts;
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

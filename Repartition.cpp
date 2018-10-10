//#include "sgx_tcrypto.h"
#include "vaultdb_generated.h"
#include "postgres_client.h"
#include "rpc/DataOwnerPrivate.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <vector>
#include "picosha2.h"


// this is here just to make the host table repartitioning a little bit easier.
#define MAX_NUM_HOSTS 32
typedef int32_t hostNum;
typedef int32_t tableID;
typedef std::pair<hostNum, tableID> HostIDPair;

//TODO(madhavsuresh): this is an example of how having typedefs can make
// the code more clear. This needs to be extended to the whole codebase.
std::vector<std::pair<int32_t, int32_t>> repart_step_one(table_t * t, int num_hosts, DataOwnerPrivate * p) {
    // If there is only one host, we do not want to reparition.
    if (num_hosts == 1) {
        //TODO(madhavsuresh): better error handling and logging
        throw;
        return {};
    }

    std::srand(time(nullptr));
    std::map<int, table_t *> partition_map;
    std::map<int, std::vector<int>> rand_assignment;
    std::vector<HostIDPair> host_and_ID;
    for (int i = 0; i < t->num_tuples; i++) {
        rand_assignment[std::rand() % num_hosts].push_back(i);
    }

    for (auto it = rand_assignment.begin();
         it != rand_assignment.end(); it++) {
        int host = it->first;
        std::vector<int> index_lst = it->second;
        table_t *output_table = copy_table_by_index(t, index_lst);
        //TODO(madhavsuresh): have the argument to this be a function pointer to this function.
        tableID id = 0;
        if (host == p->HostNum()) {
            LOG(INFO) << "Adding self partitioned table from repartition";
            id = p->AddTable(output_table);
        } else {
            id = p->SendTable(host, output_table);
        }
        host_and_ID.push_back(std::make_pair(host, id));
        free_table(output_table);
    }
    return host_and_ID;
}

uint32_t hash_field_to_int(field_t f) {
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    union {
        uint32_t u;
        unsigned char u8[sizeof(uint32_t)];
    } out;
    switch (f.type) {
        case FIXEDCHAR: {
            std::vector<unsigned char> input(f.f.fixed_char_field.val, f.f.fixed_char_field.val+16);
            picosha2::hash256(input.begin(), input.end(), hash.begin(), hash.end());
            break;
        }
        case INT : {
            std::vector<unsigned char> input(f.f.int_field.val, f.f.int_field.val+8);
            picosha2::hash256(input.begin(), input.end(), hash.begin(), hash.end());
            break;
        }
    }
    out.u8[0] = hash[0];
    out.u8[1] = hash[1];
    out.u8[2] = hash[2];
    out.u8[3] = hash[4];
    return out.u;
}

int hash_to_host(::vaultdb::ControlFlowColumn cf, int num_hosts, tuple_t * t) {
    uint32_t i = hash_field_to_int(t->field_list[cf.cfid()]);
    return i % num_hosts;
}

int repartition_step_two(std::vector<table_t *> tables, int num_hosts, DataOwnerPrivate *p) {
    int max_tuples = 0;
    for (auto t : tables) {
        max_tuples += t->num_tuples;
    }

    if (num_hosts > MAX_NUM_HOSTS) {
        throw;
    }
    table_builder_t host_tb[MAX_NUM_HOSTS];

    for (int i = 0; i < num_hosts; i++)  {
        init_table_builder(max_tuples, tables[0]->schema.num_fields, &tables[0]->schema, &host_tb[i]);
    }

    ::vaultdb::ControlFlowColumn control_flow_col = p->GetControlFlowColID();
    for (auto t: tables) {
        for (int i = 0; i < t->num_tuples; i++) {
            int host = hash_to_host(control_flow_col, num_hosts, get_tuple(i, t));
            append_tuple_table_builder(&host_tb[host], get_tuple(i, t));
        }
    }

    for (int i = 0; i < num_hosts; i ++) {
        p->SendTable(i, host_tb[i].table);
        free_table(host_tb[i].table);
    }
}




/*
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
 */

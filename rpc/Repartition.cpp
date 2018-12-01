#include "data/postgres_client.h"
#include "hash/picosha2.h"
#include "logger/LoggerDefs.h"
#include "rpc/DataOwnerPrivate.h"
#include "sgx_tcrypto.h"
#include <cstdlib>
#include <ctime>
#include <future>
#include <iostream>
#include <map>
#include <vector>

// this is here just to make the host table repartitioning a little bit easier.
#define MAX_NUM_HOSTS 32
typedef int32_t hostNum;
typedef int32_t tableID;
typedef std::pair<hostNum, tableID> HostIDPair;

HostIDPair ship_off_repart_one(DataOwnerPrivate *p, int host,
                               std::vector<int> index_lst, table_t *t) {

  table_t *output_table = copy_table_by_index(t, index_lst);
  // TODO(madhavsuresh): have the argument to this be a function pointer to
  // this function.
  tableID id = 0;
  if (host == p->HostNum()) {
    LOG(INFO) << "Adding self partitioned table from repartition";
    id = p->AddTable(output_table);
  } else {
    id = std::get<1>(p->SendTable(host, output_table));
  }
  return std::make_pair(host, id);
}

// TODO(madhavsuresh): this is an example of how having typedefs can make
// the code more clear. This needs to be extended to the whole codebase.
std::vector<std::pair<int32_t, int32_t>>
repart_step_one(table_t *t, int num_hosts, DataOwnerPrivate *p) {
  // If there is only one host, we do not want to reparition.
  if (num_hosts == 1) {
    // TODO(madhavsuresh): better error handling and logging
    LOG(FATAL) << "Cannot repartition on just one host";
    throw;
    return {};
  }

  std::srand(time(nullptr));
  std::map<int, table_t *> partition_map;
  std::map<int, std::vector<int>> rand_assignment;
  std::vector<HostIDPair> host_and_ID;
  START_TIMER(repart_one_shuffle_assign);
  for (int i = 0; i < t->num_tuples; i++) {
    rand_assignment[std::rand() % num_hosts].push_back(i);
  }
  END_AND_LOG_EXP3_STAT_TIMER(repart_one_shuffle_assign);

  START_TIMER(repart_one_data_movement);
  vector<std::future<HostIDPair>> threads_send;
  for (auto it = rand_assignment.begin(); it != rand_assignment.end(); it++) {
    HostIDPair hidp = ship_off_repart_one(p, it->first, it->second, t);
    threads_send.push_back(std::async(std::launch::async, ship_off_repart_one,
                                      p, it->first, it->second, t));
  }
  for (auto &h : threads_send) {
    host_and_ID.push_back(h.get());
  }
  END_AND_LOG_EXP3_STAT_TIMER(repart_one_data_movement);
  free_table(t);
  return host_and_ID;
}
uint32_t hash_fields_to_int_sgx(uint8_t f[], uint32_t len) {
  sgx_sha256_hash_t hash;
  union {
      uint32_t u;
      unsigned char u8[sizeof(uint32_t)];
  } out;
  sgx_sha256_msg(f, len, &hash);
  out.u8[0] = hash[0];
  out.u8[1] = hash[1];
  out.u8[2] = hash[2];
  out.u8[3] = hash[4];
  return out.u;
}


int hash_to_host(::vaultdb::ControlFlowColumn &cf, int num_hosts, tuple_t *t,
                 table_t *table) {
  if (cf.cf_name_strings_size() > 20) {
    LOG(FATAL) << "Too many control flow attributes";
    throw;
  }
  uint8_t f[MAX_FIELDS*FIXEDCHAR_LEN];
  uint32_t ptr = 0;
  for (int i = 0; i < cf.cf_name_strings_size(); i++) {
    int input_col = colno_from_name(table,cf.cf_name_strings(i));
    switch(t->field_list[input_col].type){
      case FIXEDCHAR : {
        memcpy(&f[ptr], t->field_list[input_col].f.fixed_char_field.val, FIXEDCHAR_LEN);
        ptr += FIXEDCHAR_LEN;
        break;
      }
      case INT: {
        memcpy(&f[ptr], &(t->field_list[input_col].f.int_field.genval), sizeof(uint64_t));
        ptr += sizeof(uint64_t);
        break;
      }
     default :{
       LOG(PQXX) << "Unsupported type :[" << t->field_list[input_col].type << "]";
       LOG(FATAL) << "Unsupported type for hash_to_host";
        throw;
      }
    }
  }
  uint32_t i = hash_fields_to_int_sgx(f, ptr);
  return i % num_hosts;
}

std::vector<std::pair<int32_t, int32_t>>
repartition_step_two(std::vector<table_t *> tables, int num_hosts,
                     DataOwnerPrivate *p) {
  int max_tuples = 0;
  for (auto t : tables) {
    max_tuples += t->num_tuples;
  }

  if (num_hosts > MAX_NUM_HOSTS) {
    LOG(FATAL) << "Repartition: num_hosts > MAX_NUM_HOSTS";
    throw;
  }
  table_builder_t host_tb[MAX_NUM_HOSTS];
  table_builder_t dummy_host_tb[MAX_NUM_HOSTS];

  for (int i = 0; i < num_hosts; i++) {
    init_table_builder(max_tuples, tables[0]->schema.num_fields,
                       &tables[0]->schema, &host_tb[i]);
  }

  for (int i = 0; i < num_hosts; i++) {
    init_table_builder(max_tuples, tables[0]->schema.num_fields,
                       &tables[0]->schema, &dummy_host_tb[i]);
    append_tuple(&dummy_host_tb[i], get_tuple(0, tables[0]));
  }


  START_TIMER(repart_two_hashing);
  ::vaultdb::ControlFlowColumn control_flow_col = p->GetControlFlowColID();
  for (auto t : tables) {
    for (int i = 0; i < t->num_tuples; i++) {
      int host = hash_to_host(control_flow_col, num_hosts, get_tuple(i, t), t);
      for (int j = 0; j < num_hosts; j++) {
        if (j == host) {
          append_tuple(&host_tb[host], get_tuple(i, t));
        } else {
          copy_tuple_to_position(dummy_host_tb[j].table, 0, get_tuple(i,t));
        }
      }
    }
  }
  END_AND_LOG_EXP3_STAT_TIMER(repart_two_hashing);
  for (auto t : tables) {
    free_table(t);
  }
  for (int i = 0; i < num_hosts; i++) {
    free_table(dummy_host_tb[i].table);
  }
  std::vector<HostIDPair> host_and_ID;
  int myid = p->AddTable(host_tb[p->HostNum()].table);
  host_and_ID.push_back(std::make_pair(p->HostNum(), myid));

  vector<std::future<HostIDPair>> threads_send;
  START_TIMER(repart_two_data_movement);
  for (int i = 0; i < num_hosts; i++) {
    // TODO(madhavsuresh): need to send all of these IDs over to HB after step
    // two so that way tables can be coalesced
    if (i == p->HostNum()) {
      continue;
    } else {
      threads_send.push_back(std::async(std::launch::async,
                                        &DataOwnerPrivate::SendTable, p, i,
                                        host_tb[i].table));
    }
  }

  for (auto &h : threads_send) {
    host_and_ID.push_back(h.get());
  }
  END_AND_LOG_EXP3_STAT_TIMER(repart_two_data_movement);
  return host_and_ID;
}

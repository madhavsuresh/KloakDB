//
// Created by madhav on 4/15/19.
//

#include "data/postgres_client.h"
#include <data/pqxx_compat.h>
#include <emp-sh2pc/emp-sh2pc.h>
#include <gflags/gflags.h>
#include <operators/Generalize.h>
#include <pqxx/pqxx>

#define INT8OID 20
#define INT4OID 23

using namespace emp;
using namespace std;
using namespace chrono;

DEFINE_int32(party, 1, "party for EMP execution");
DEFINE_int32(port, 43439, "port for EMP execution");
DEFINE_int32(gen, 5, "anonymization level");
DEFINE_string(dbname, "tpch_scale_001", "database to query from");
DEFINE_bool(obli, false, "run obliviously");
enum VAULT_FIELD_TYPE { UNSUPPORTED_EMP, INT64, FLOAT, BITSTRING };

NetIO *io;

// TODO(madhavsuresh): we only support Integer types right now. Not even floats
#define MAX_FIELD_NAME_LEN 32

typedef struct field_desc_emp {
  char field_name[MAX_FIELD_NAME_LEN];
  int field_ordinal;
  VAULT_FIELD_TYPE type;
} field_descE_t;
#define MAX_NUM_FIELDS 32
typedef struct schema_emp {
  int32_t num_fields;
  char field_names;
  field_descE_t fields[MAX_NUM_FIELDS];
} schemaE_t;

typedef struct join_def_emp {
  int l_col;
  int r_col;
} join_defE_t;

typedef struct truncate {
  int cardinality_bound;
} truncate_t;

typedef struct field_emp {
  VAULT_FIELD_TYPE type;
  int field_ordinal;
  int size;
  union {
    Integer *int_f;
    Float *float_f;
    Bit *bit_f;
  };
} fieldE_t;

Integer *field_as_int(fieldE_t f) { return f.int_f; }

Float *field_as_float(fieldE_t f) { return f.float_f; }

Bit *field_as_bit(fieldE_t f) { return f.bit_f; }

typedef struct tuple_emp {
  Bit is_dummy;
  bool ignore;
  int keyed_tupled_index;
  fieldE_t fields[MAX_NUM_FIELDS];
  int eq_class;
} tupleE_t;

typedef struct table_emp {
  schemaE_t table_schema;
  int num_tuples;
  int num_real_tuples;
  Integer *columns[MAX_NUM_FIELDS];
  tupleE_t tuples[];
} tableE_t;

typedef struct sort_def_emp {
  bool asc;
  int field_no;
} sort_defE_t;

inline int greatest_power_of_two_less_than(int n) {
  int k = 1;
  while (k < n)
    k = k << 1;
  return k >> 1;
}

tupleE_t *get_tuple(tableE_t *t, int i) { return &t->tuples[i]; }

void mem_swap_tuples(tupleE_t *t1, tupleE_t *t2) {
  // TODO(madhavsuresh): make sure that the dummy bit is set correctly.
  // TODO(madhavsuresh) is this non-oblivious?
  tupleE_t tmp;
  memcpy(&tmp, t1, sizeof(tupleE_t));
  memcpy(t1, t2, sizeof(tupleE_t));
  memcpy(t2, &tmp, sizeof(tupleE_t));
}

/*
void index_swap_tuples(tableE_t *t, int i, int j, Bit to_swap) {
  If(to_swap, t->tuples[i],t->tuples[j]);

}
*/

Bit compare_fields(fieldE_t f1, fieldE_t f2, Bit asc) {
  switch (f1.type) {
  case INT64: {
    return *f1.int_f > *f2.int_f == asc;
  }
  default: { throw; }
  }
}

table_t *get_gen_fields(std::string dbname, int gen) {
	// hack to get generalize to work on split tpch_scale_001 table
	// in particular, "double counting" to ensure single counting
	// since data originates from single split host
  std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
      tables;
  table_t *tpch_lineitem = get_table(
      "SELECT l_orderkey, count(l_orderkey) from lineitem group by l_orderkey",
      "dbname=" + dbname);
  table_t *tpch_lineitem2 = get_table(
      "SELECT l_orderkey, count(l_orderkey) from lineitem group by l_orderkey",
      "dbname=" + dbname);
  tables["lineitem"].emplace_back(0, tpch_lineitem);
  tables["lineitem"].emplace_back(1, tpch_lineitem2);

  table_t *tpch_orders = get_table(
      "SELECT o_orderkey, count(o_orderkey) from orders group by o_orderkey",
      "dbname=" + dbname);
  table_t *tpch_orders2 = get_table(
      "SELECT o_orderkey, count(o_orderkey) from orders group by o_orderkey",
      "dbname=" + dbname);
  tables["orders"].emplace_back(0, tpch_orders);
  tables["orders"].emplace_back(1, tpch_orders2);
  auto out = generalize_table_fast(tables, 2, gen);
  std::cout << "Size of gen table" << out->num_tuples << std::endl;
  return out;
}

void compare_and_swap(tableE_t *t, sort_defE_t s, int tuple_left,
                      int tuple_right, Bit asc) {
  tupleE_t *left = get_tuple(t, tuple_left);
  tupleE_t *right = get_tuple(t, tuple_right);
  Bit dummy_left = left->is_dummy;
  Bit dummy_right = right->is_dummy;
  Bit to_swap =
      compare_fields(left->fields[s.field_no], right->fields[s.field_no], asc);
}

/*
void *table_bitonic_merge(tableE_t *t, sort_defE_t s, int lo, int n, Bit asc) {
  if (n > 1) {
    int m = greatest_power_of_two_less_than(n);
    for (int i = lo; i < lo + n - m; i++) {
      compare_and_swap(t, )
    }
  }
}

void table_bitonic_sort(tableE_t *t, sort_defE_t s, int lo, int n, Bit asc) {
  if (n > 1) {
    int m = n / 2;
    table_bitonic_sort(t, s, lo, m, !asc);
    table_bitonic_sort(t, s, lo + m, n - m, asc);
    table_bitonic_merge(t, s, lo, n, asc);
  }
}

tableE_t *table_sort(tableE_t *t, sort_defE_t s) {
  Bit asc = s->asc;
  // table_bitonic_sort()
}
*/

tableE_t *sort_by_dummies(tableE_t *t) {}

// Truncate on sorted assumes that the tuples are ordered by dummy tuples
tableE_t *truncate_on_sorted(tableE_t *t, truncate_t *trunc) {
  if (trunc->cardinality_bound < t->num_tuples) {
    return t;
  }
  for (int i = trunc->cardinality_bound; i < t->num_tuples; i++) {
    t->tuples[i].ignore = true;
  }
}

tableE_t *truncate_on_unsorted(tableE_t *t, truncate_t *trunc) {
  int fields_to_ignore = t->num_tuples - trunc->cardinality_bound;
  Bit tru = true;
  Bit fal = false;
  for (int i = 0; i < t->num_tuples; i++) {
    If(t->tuples->is_dummy, tru, fal);
  }
}

bool same_eq(int gen_map, int left_index, int right_index) {}

tableE_t *emp_join_table(tableE_t *left, tableE_t *right, join_defE_t def) {
  // TODO(madhavsuresh): This will fail when there are too many tuples.
  tableE_t *t = (tableE_t *)malloc(sizeof(tableE_t) + right->num_tuples *
                                                          left->num_tuples *
                                                          sizeof(tupleE_t));
  t->table_schema.num_fields =
      left->table_schema.num_fields + right->table_schema.num_fields;

  int curr_tuple = 0;
  for (int i = 0; i < left->num_tuples; i++) {
    int curr_eq =  left->tuples[i].eq_class;
    for (int j = 0; j < right->num_tuples; j++) {
	if (!FLAGS_obli) {
	    if ( !(right->tuples[j].eq_class == curr_eq && curr_eq !=1)) {
		    continue;
	    }
	}
	    int curr_eq =  left->tuples[i].eq_class;
      // EMP Codepath
      Bit cmp = *left->tuples[i].fields[def.l_col].int_f ==
                *right->tuples[j].fields[def.r_col].int_f;
      // END EMP Codepath
      t->tuples[curr_tuple].is_dummy.bit = cmp.bit;
      int field_num = 0;
      // TODO(madhavsuresh): write this out in separate merge function
      for (int f = 0; f < left->table_schema.num_fields; f++) {
        t->tuples[curr_tuple].fields[field_num].int_f =
            left->tuples[i].fields[f].int_f;
        field_num++;
      }
      for (int f = 0; f < right->table_schema.num_fields; f++) {
        t->tuples[curr_tuple].fields[field_num].int_f =
            right->tuples[j].fields[f].int_f;
        field_num++;
      }
      t->tuples[curr_tuple].eq_class = left->tuples[i].eq_class;
      t->num_tuples++;
      curr_tuple++;
    }
  }
  return t;
}

Integer *emp_join(tableE_t *left, tableE_t *right, join_defE_t def) {

  Integer *output = new Integer[left->num_tuples * right->num_tuples];
  int output_counter = 0;
  Integer dst = Integer(64, 0, PUBLIC);
  for (int i = 0; i < left->num_tuples; i++) {
    for (int j = 0; j < right->num_tuples; j++) {
      Bit cmp = left->columns[def.l_col][i] == right->columns[def.r_col][j];
      memcpy(dst.bits, left->columns[def.l_col][i].bits, 64 * sizeof(Bit));
      dst = If(cmp, dst, Integer(64, 0, PUBLIC));
      output[output_counter] = dst;
      output_counter++;
    }
  }
  return output;
}

pqxx::result query_emp(std::string query_string, std::string dbname) {
  try {
    pqxx::connection c(dbname);
    pqxx::work txn(c);
    pqxx::result res;
    res = txn.exec(query_string);
    txn.commit();
    return res;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    throw e;
  }
}

schemaE_t build_schema(pqxx::result res) {
  schemaE_t s;
  s.num_fields = res.columns();
  for (int i = 0; i < s.num_fields; i++) {
    // TODO(madhavsuresh): assumes field length is less than 32 characters
    strncpy(s.fields[i].field_name, res.column_name(i), MAX_FIELD_NAME_LEN);
    s.fields[i].field_ordinal = i;
    if (res.column_type(i) != INT4OID && res.column_type(i) != INT8OID) {
      throw std::invalid_argument("Only INT types supported");
    }

    s.fields[i].type = INT64;
  }
  return s;
}

// benchmark without batcher.
tableE_t *ingest_and_share_as_table(std::string query_string,
                                    std::string dbname, int party,
                                    table_t *gen_map_table) {

  std::unordered_map<int, int> gen_map;
  for (int i = 0; i < gen_map_table->num_tuples; i++) {
    tuple_t *tup = get_tuple(i, gen_map_table);
    gen_map[tup->field_list[0].f.int_field.val] =
        tup->field_list[0].f.int_field.genval;
  }
  auto query_result = query_emp(query_string, dbname);
  int num_rows = query_result.size();
  cout << num_rows << "<< NUM ROWS" << endl;
  tableE_t *t =
      (tableE_t *)malloc(sizeof(tableE_t) + num_rows*2 * sizeof(tupleE_t));
  bzero(t,sizeof(tableE_t) + num_rows*2 * sizeof(tupleE_t));
  t->table_schema = build_schema(query_result);
  Batcher b1;
  Batcher b2;
  
  Batcher b1_gen;
  Batcher b2_gen;

  std::map<long, long> batched_to_gen;
  int i = 0;
  // ONLY FIRST FIELD CAN BE ANON'D FOR NOW
  for (auto row : query_result) {
    int j = 0;
    for (auto field : row) {
      if (j == 0) {
        batched_to_gen[i] = gen_map[field.as<long>()];
	b1_gen.add<Integer>(64, gen_map[field.as<long>()]);
	b2_gen.add<Integer>(64, gen_map[field.as<long>()]);
      }
      j++;
      b1.add<Integer>(64, field.as<long>());
      b2.add<Integer>(64, field.as<long>());
    }
    i++;
  }
  b1.make_semi_honest(BOB);
  b2.make_semi_honest(ALICE);
  b1_gen.make_semi_honest(BOB);
  b2_gen.make_semi_honest(ALICE);
  

  auto start_time = std::chrono::high_resolution_clock::now();
  int tuple_no = 0;
  for (int i = 0; i < num_rows; i++) {
    auto tup = &(t->tuples[tuple_no]);
    for (int j = 0; j < t->table_schema.num_fields; j++) {
      tup->fields[j].int_f = new Integer(b1.next<Integer>());
    }
    tup->eq_class = b1_gen.next<Integer>().reveal<uint64_t>();
    tuple_no++;
    t->num_tuples++;
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end_time - start_time;
  std::cout << "Abstracted: Table 1 Elapsed Time" << elapsed.count()
            << std::endl;

  start_time = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < num_rows; i++) {
    auto tup = &(t->tuples[tuple_no]);
    for (int j = 0; j < t->table_schema.num_fields; j++) {
      tup->fields[j].int_f = new Integer(b2.next<Integer>());
    }
    tup->eq_class = b2_gen.next<Integer>().reveal<uint64_t>();
    tuple_no++;
    t->num_tuples++;
  }
  end_time = std::chrono::high_resolution_clock::now();
  elapsed = end_time - start_time;
  std::cout << "Abstracted: Table 2 Elapsed Time" << elapsed.count()
            << std::endl;

  // sort(res, num_rows * 2);
  //t->num_tuples = num_rows * 2;
  return t;
}



int main(int argc, char **argv) {
  printf("sizeof: %d", sizeof(tupleE_t));
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  io = new NetIO(FLAGS_party == ALICE ? nullptr : "127.0.0.1", FLAGS_port);
  setup_semi_honest(io, FLAGS_party);
  auto gen_table = get_gen_fields(FLAGS_dbname, FLAGS_gen);

  HalfGateGen<NetIO> *gate_counter =
      (HalfGateGen<NetIO> *)CircuitExecution::circ_exec;
  int gates1 = gate_counter->gid;
  auto start_time = high_resolution_clock::now();
  tableE_t* t1, *t2;
  if (FLAGS_party == ALICE) {
  t1 = ingest_and_share_as_table("select l_orderkey from lineitem WHERE MOD(l_orderkey, 2)= 0 ORDER BY l_orderkey LIMIT 3000", "dbname=" + FLAGS_dbname, FLAGS_party, gen_table);
  t2 = ingest_and_share_as_table("select o_orderkey from orders WHERE MOD(o_orderkey,2) = 1 ORDER BY o_orderkey LIMIT 740", "dbname=" + FLAGS_dbname, FLAGS_party, gen_table);
  } else {
  t1 = ingest_and_share_as_table("select l_orderkey from lineitem WHERE MOD(l_orderkey, 2)= 1 ORDER BY l_orderkey LIMIT 3000", "dbname=" + FLAGS_dbname, FLAGS_party, gen_table);
  t2 = ingest_and_share_as_table("select o_orderkey from orders WHERE MOD(o_orderkey,2) = 0 ORDER BY o_orderkey LIMIT 740", "dbname=" + FLAGS_dbname, FLAGS_party, gen_table);
  }
  printf("T2: :%d", t2->num_tuples);
  int gates2 = gate_counter->gid;
  auto end_time = high_resolution_clock::now();
  duration<double> elapsed = end_time - start_time;
  cout << "Abstracted Merge Elapsed Time" << elapsed.count()
       << " Gates: " << gates2 - gates1 << endl;
  start_time = high_resolution_clock::now();
  // tableE_t *t2 = ingest_and_share("select a from left_deep_joins_512 ORDER BY
  // a",
  //                              "dbname=vaultdb_", FLAGS_party);
  int gates3 = gate_counter->gid;

  end_time = high_resolution_clock::now();
  elapsed = end_time - start_time;
  cout << "NonAbstracted Merge Elapsed Time" << elapsed.count()
       << " Gates: " << gates3 - gates2 << endl;
  join_defE_t jd = {0, 0};
  start_time = high_resolution_clock::now();
  // Integer *out = emp_join(t2, t2, jd);
  end_time = high_resolution_clock::now();
  elapsed = end_time - start_time;
  cout << "Join (non-table): " << elapsed.count() << endl;
  int gates4 = gate_counter->gid;
  cout << " Gates: " << gates4 - gates3 << endl;
  start_time = high_resolution_clock::now();

  fflush(0);
  auto joined_table = emp_join_table(t1, t2, jd);
  end_time = high_resolution_clock::now();
  elapsed = end_time - start_time;
  cout << "Join (table): " << elapsed.count() << endl;
  printf("O: :%d, T1%d, T2 %d", joined_table->num_tuples, t1->num_tuples,
         t2->num_tuples);
  int gates5 = gate_counter->gid;
  cout << " Gates: " << gates5 - gates4 << endl;
}

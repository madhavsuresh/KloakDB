//
// Created by madhav on 11/14/18.
//

// Typedefs to make code easier to read without using classes.
#include "Generalize.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
//#include <random>
#include <string>
#include <unordered_map>


bool is_kanon(std::vector<std::tuple<hostnum, tup_count, cf_hash>> equiv_class,
              int num_hosts, int k) {

  int curr_host = std::get<0>(equiv_class[0]);
  bool one_host = true;
  for (auto &i : equiv_class) {
    if (std::get<0>(i) != curr_host) {
      one_host = false;
      break;
    }
  }
  // If there is only one host in an equivalence class,
  // we only need it to be more than k.
  if (one_host) {
    int num_tuples = 0;
    for (auto &i : equiv_class) {
      num_tuples += std::get<1>(i);
    }
    if (num_tuples < k) {
      return false;
    }
    return true;
  }

  for (int i = 0; i < num_hosts; i++) {
    int accumulator = 0;
    for (auto &v : equiv_class) {
      if (i != std::get<0>(v)) {
        accumulator += std::get<1>(v);
      }
    }
    if (accumulator < k) {
      return false;
    }
  }
  return true;
}

void merge(
    std::unordered_map<
        cf_hash, std::vector<std::tuple<hostnum, tup_count, cf_hash>>> &gen_map,
    cf_hash h1, cf_hash h2) {

  if (gen_map.find(h1) == gen_map.end() || gen_map.find(h2) == gen_map.end()) {
    throw std::invalid_argument("Trying to merge to nonexistant class");
  }
  for (auto &i : gen_map[h2]) {
    gen_map[h1].push_back(i);
  }
  gen_map.erase(h2);
}

bool kscore(std::tuple<int, tup_count> p1, std::tuple<int, tup_count> p2) {
  return std::get<1>(p1) < std::get<1>(p2);
}

table_t *generate_genmap_table(
    std::unordered_map<cf_hash,
                       std::vector<std::tuple<hostnum, tup_count, cf_hash>>>
        gen_map) {
  std::unordered_map<cf_hash, cf_gen> output_map;
  for (auto &g : gen_map) {
    for (auto &v : g.second) {
      auto orig_cf = std::get<2>(v);
      output_map[orig_cf] = g.first;
    }
  }

  table_builder_t tb;
  schema_t schema;
  schema.num_fields = 1;
  schema.fields[0].col_no = 0;
  schema.fields[0].type = INT;
  strncpy(schema.fields[0].field_name, "cf_hash_orig\0", FIELD_NAME_LEN);
  init_table_builder(output_map.size(), 1 /* num_columns */, &schema, &tb);
  auto *tup = (tuple_t *)malloc(tb.size_of_tuple);
  tup->num_fields = 1;
  tup->field_list[0].type = INT;
  for (auto &g : output_map) {
    tup->field_list[0].f.int_field.val = g.first;
    tup->field_list[0].f.int_field.genval = g.second;
    append_tuple(&tb, tup);
  }
  free(tup);
  return tb.table;
}

template <typename TK, typename TV>
std::vector<TK> extract_keys(std::unordered_map<TK, TV> const &input_map) {
  std::vector<TK> retval;
  for (auto const &element : input_map) {
    retval.push_back(element.first);
  }
  return retval;
}

cf_hash find_smallest_equiv_not_eq(
    std::unordered_map<cf_hash,
                       std::vector<std::tuple<hostnum, tup_count, cf_hash>>>
        gen_map,
    cf_hash eq_class) {
  auto keys = extract_keys(gen_map);
  uint64_t min_val = INTMAX_MAX;
  cf_hash cf_hash_key = -1;
  for (auto k : keys) {
    if (k == eq_class) {
      continue;
    }
    int num_tuples = 0;
    for (auto &i : gen_map[k]) {
      num_tuples += std::get<1>(i);
    }
    if (num_tuples < min_val) {
      min_val = num_tuples;
      cf_hash_key = k;
    }
  }
  return cf_hash_key;
}

// TODO(madhavsuresh): there is a terrible bug here!
// If the top range between two tables does not overlap,
// there will be srs consequences
int get_min_range(gen_map_t gen_map, int min_value, int max_value,
                  int num_hosts, int k) {

  std::unordered_map<cf_hash, eq_class_t> val_map;
  eq_class_t eqv;
  for (int i = min_value; i <= max_value; i++) {
    // TODO(madhavsuresh) test out
    for (auto &a : gen_map[i]) {
      eqv.push_back(a);
    }
    if ((eqv.size() > 0) && is_kanon(eqv, num_hosts, k)) {
      return i;
    }
  }
  if (eqv.size() > 0) {
    return -1;
  }
  return max_value;
}
table_t *generalize_table_fast_string(
    std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
        table_map_host_table_pairs,
    int num_hosts, int k) {
  unordered_map<string, int> input_to_internal_gen;
  unordered_map<string, int> counter;
  for (auto &table_queries : table_map_host_table_pairs) {
    for (auto &ht : table_queries.second) {
      table_t *t = ht.second;
      for (int i = 0; i < t->num_tuples; i++) {
        tuple_t *tup = get_tuple(i, t);
        string field_label;
        switch (tup->field_list[0].type) {
        case FIXEDCHAR: {
          field_label = string(tup->field_list[0].f.fixed_char_field.val);
          break;
        }
        case INT: {
          field_label = to_string(tup->field_list[0].f.int_field.val);
          break;
        }
        }
        counter[field_label] += tup->field_list[1].f.int_field.val;
      }
    }
  }

  // Taken from
  // https://www.quora.com/How-can-one-sort-a-map-using-its-value-in-ascending-order
  vector<pair<string, int>> sorted_counter;

  for (auto &x : counter) {
    sorted_counter.emplace_back(x);
  }
  sort(sorted_counter.begin(), sorted_counter.end(),
       [](pair<string, int> elem1, pair<string, int> elem2) {
         return elem1.second > elem2.second;
       });

  for (int i = 0; i < sorted_counter.size(); i++) {
    input_to_internal_gen[sorted_counter[i].first] = i;
  }

}
typedef struct relation_count {
  int64_t *arr;
  int num_hosts;
} rc_t;

int64_t *rcget(rc_t rc, int64_t genval, int64_t host) {
  return (rc.arr + genval * rc.num_hosts + host);
}

int64_t rcval(rc_t rc, int64_t genval, int64_t host) {
  return *(rcget(rc, genval, host));
}

void rcset(rc_t rc, int64_t genval, int64_t host, int64_t val_to_set) {
  *(rcget(rc, genval, host)) = val_to_set;
}

void rcadd(rc_t rc, int64_t genval, int64_t host, int64_t val_to_add) {
  rcset(rc, genval, host, val_to_add + rcval(rc, genval, host));
}
#define MAX_RELATION 10
#define MAX_HOST_GEN 10
// Is k anon up to NOT INCLUDING max. So max is new bottom_pointer;
bool is_range_kanon(rc_t rc, int k, int64_t min, int64_t max) {
  int64_t minus_host[MAX_HOST_GEN];
  for (int64_t i = 0; i < rc.num_hosts; i++) {
    minus_host[i] = 0;
  }

  for (int64_t gen = min; gen < max; gen++) {
    int64_t total = 0;
    for (int host_num = 0; host_num < rc.num_hosts; host_num++) {
      total += rcval(rc, gen, host_num);
    }

    for (int host_num = 0; host_num < rc.num_hosts; host_num++) {
      minus_host[host_num] += total - rcval(rc, gen, host_num);
    }
  }
  bool is_kanon = true;
  for (int64_t i = 0; i < rc.num_hosts; i++) {
    if (minus_host[i] < k) {
      is_kanon = false;
    }
  }
  return is_kanon;
}

int64_t find_min_range(rc_t rc, int num_relations, int k, int64_t min,
                       int64_t final_range) {
  int64_t max = min;
  int64_t minus_host[MAX_HOST_GEN];
  for (int i = 0; i < rc.num_hosts; i++) {
    minus_host[i] = 0;
  }

  bool not_k_anon = true;
  while (not_k_anon) {
    for (int host_num = 0; host_num < rc.num_hosts; host_num++) {
      minus_host[host_num] += rcval(rc, max, host_num);

      if (rcval(rc, max, host_num) == 0) {
        bool single_host = true;
        int64_t val;
        if (host_num == 0) {
          val = rcval(rc, max, 1);
        } else {
          val = rcval(rc, max, 0);
        }
        for (int i = 0; i < rc.num_hosts; i++) {
          if (host_num != i) {
            if (val != rcval(rc, max, i)) {
              single_host = false;
            }
          }
        }
        if (single_host) {
          minus_host[host_num] += val;
        }
      }
    }

    not_k_anon = false;
    for (int host_num = 0; host_num < rc.num_hosts; host_num++) {
      if (minus_host[host_num] < k) {
        not_k_anon = true;
      }
    }
    if (max >= final_range) {
      return -1;
    }
    max++;
  }
  return max;
}


unordered_map<int64_t, pair<int, int>> tuple_val_to_occurences(
    std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
        table_map_host_table_pairs,
    unordered_map<table_name, int> relation_name_to_num, int num_hosts) {
  unordered_map<int64_t, pair<int, int>> counter;
  for (auto &table_queries : table_map_host_table_pairs) {
    int relation_num = relation_name_to_num[table_queries.first];
    for (auto &ht : table_queries.second) {
      table_t *t = ht.second;
      for (int i = 0; i < t->num_tuples; i++) {
        tuple_t *tup = get_tuple(i, t);
        int64_t field_label;
        switch (tup->field_list[0].type) {
        case INT: {
          field_label = tup->field_list[0].f.int_field.val;
          break;
        }
        default: { throw; }
        }
        counter[field_label].first |= 1 << (relation_num + 1);
        counter[field_label].second += tup->field_list[1].f.int_field.val;
      }
    }
  }
  int max_relations = 0;
  for (int i = 0; i < relation_name_to_num.size(); i++) {
    max_relations |= 1 << (i + 1);
  }
  int all_relations = 0;
  int total_count = 0;
  int all_tuples = 0;
  int all_tuple_count = 0;
  for (auto &k : counter) {
    if (k.second.first == max_relations) {
      all_relations++;
      total_count += k.second.second;
    }
    all_tuple_count += k.second.second;
    all_tuples++;
  }
  return counter;
}

unordered_map<int64_t, int> union_counts(
    std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
        table_map_host_table_pairs) {

  unordered_map<int64_t, int> counter;
  for (auto &table_queries : table_map_host_table_pairs) {
    for (auto &ht : table_queries.second) {
      table_t *t = ht.second;
      for (int i = 0; i < t->num_tuples; i++) {
        tuple_t *tup = get_tuple(i, t);
        int64_t field_label;
        switch (tup->field_list[0].type) {
        case INT: {
          field_label = tup->field_list[0].f.int_field.val;
          break;
        }
        default: { throw; }
        }
        counter[field_label] += tup->field_list[1].f.int_field.val;
      }
    }
  }
  return counter;
}

std::tuple<uint64_t, unordered_map<int64_t, int64_t>, vector<int64_t>>
get_range(unordered_map<int64_t, int> counter,
          unordered_map<int64_t, pair<int, int>> tuple_val, int num_relations) {
  // Taken from
  // https://www.quora.com/How-can-one-sort-a-map-using-its-value-in-ascending-order
  unordered_map<int64_t, int64_t>
      input_to_internal_gen; // original input -> internal gen
  vector<pair<int64_t, int>> sorted_counter;
  vector<int64_t> discard_pile;

  int max_relations = 0;
  for (int i = 0; i < num_relations; i++) {
    max_relations |= 1 << (i + 1);
  }
  // Taken from
  // https://www.quora.com/How-can-one-sort-a-map-using-its-value-in-ascending-order
  for (auto &x : counter) {
    sorted_counter.emplace_back(x);
  }
  /*
  auto rng = std::default_random_engine{};
  std::shuffle(std::begin(sorted_counter), end(sorted_counter), rng);
   */

  int all_counter = 0;
  for (int i = 0; i < sorted_counter.size(); i++) {
    if (tuple_val[sorted_counter[i].first].first == max_relations) {
      input_to_internal_gen[sorted_counter[i].first] = all_counter;
      all_counter++;
    } else {
      discard_pile.emplace_back(sorted_counter[i].first);
    }
  }
  const int final_range = all_counter;
  return make_tuple(final_range, input_to_internal_gen, discard_pile);
}

int populate_rc_map(
    rc_t *rc_map,
    std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
        table_map_host_table_pairs,
    unordered_map<int64_t, int64_t> input_to_internal_gen, uint64_t final_range,
    int num_hosts) {
  int relation_num = 0;
  for (auto &relation : table_map_host_table_pairs) {
    rc_map[relation_num].arr =
        (int64_t *)malloc(final_range * num_hosts * sizeof(int64_t));
    memset(rc_map[relation_num].arr, '\0',
           final_range * num_hosts * sizeof(int64_t));

    rc_map[relation_num].num_hosts = num_hosts;
    for (auto &table : relation.second) {
      int host_num = table.first;
      if (host_num >= num_hosts) {
        throw;
      }
      table_t *t = table.second;
      for (int i = 0; i < t->num_tuples; i++) {
        tuple_t *tup = get_tuple(i, t);
        int64_t count = tup->field_list[1].f.int_field.val;
        int64_t internal_gen =
            input_to_internal_gen[tup->field_list[0].f.int_field.val];
        for (int j = 0; j < num_hosts; j++) {
          if (j != host_num) {
            rcadd(rc_map[relation_num], internal_gen, j, count);
          }
        }
      }
    }
    relation_num++;
  }
  return relation_num;
}

table_builder_t get_gen_tb(uint64_t final_range) {
  table_builder_t tb;
  schema_t schema;
  schema.num_fields = 1;
  schema.fields[0].col_no = 0;
  schema.fields[0].type = INT;
  strncpy(schema.fields[0].field_name, "cf_hash_orig\0", FIELD_NAME_LEN);
  init_table_builder(final_range, 1 /* num_columns */, &schema, &tb);
  return tb;
}


table_t *generalize_table_fast(
    std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
        table_map_host_table_pairs,
    int num_hosts, int k) {
  unordered_map<int64_t, int64_t>
      input_to_internal_gen; // original input -> internal gen
  unordered_map<int64_t, int64_t>
      internal_gen_to_input; // internal gen-> original input
  unordered_map<table_name, int> relation_name_to_num;
  int num_relations = 0;
  for (auto &get_name : table_map_host_table_pairs) {
    relation_name_to_num[get_name.first] = num_relations;
    num_relations++;
  }
  auto tvo = tuple_val_to_occurences(table_map_host_table_pairs,
                                     relation_name_to_num, num_hosts);

  unordered_map<int64_t, int> counter =
      union_counts(table_map_host_table_pairs);
  int tup_append = 0;
  int main_tup_append = 0;

  uint64_t range_of_tuples = 0;
  auto range_info = get_range(counter, tvo, num_relations);
  range_of_tuples = std::get<0>(range_info);
  input_to_internal_gen = std::get<1>(range_info);
  auto discard_pile = std::get<2>(range_info);
  // Get Reverse Map
  for (auto &i : input_to_internal_gen) {
    internal_gen_to_input[i.second] = i.first;
  }
  const uint64_t final_range = range_of_tuples;
  rc_t rc_map[MAX_RELATION];
  populate_rc_map(rc_map, table_map_host_table_pairs, input_to_internal_gen,
                  final_range, num_hosts);
  table_builder_t tb = get_gen_tb(final_range);
  auto *tup = (tuple_t *)malloc(tb.size_of_tuple);
  tup->num_fields = 1;
  tup->field_list[0].type = INT;

  int min_val = 0;
  int prev_min_val = 0;
  bool range_exceeded[MAX_RELATION];
  for (int r = 0; r < num_relations; r++) {
    range_exceeded[r] = false;
  }
  while (min_val <= final_range) {
    vector<int> min_ks;
    for (int rel = 0; rel < num_relations; rel++) {
      if (!range_exceeded[rel]) {
        int mr =
            find_min_range(rc_map[rel], num_relations, k, min_val, final_range);
        if (mr == -1) {
          range_exceeded[rel] = true;
        }
        min_ks.emplace_back(mr);
      }
    }
    int max_top = *max_element(std::begin(min_ks), std::end(min_ks));
    int min_top = *min_element(std::begin(min_ks), std::end(min_ks));
    if (min_top == -1 && max_top == -1) {
      for (int scan = min_val; scan < final_range; scan++) {
        int64_t original_table_val = internal_gen_to_input[scan];
        tup->field_list[0].f.int_field.val = original_table_val;
        tup->field_list[0].f.int_field.genval = prev_min_val;
        append_tuple(&tb, tup);
        tup_append++;
      }
      max_top = final_range + 1;
    } else {
      for (int scan = min_val; scan < max_top; scan++) {
        int64_t original_table_val = internal_gen_to_input[scan];
        tup->field_list[0].f.int_field.val = original_table_val;
        tup->field_list[0].f.int_field.genval = min_val;
        append_tuple(&tb, tup);
        main_tup_append++;
      }
    }
    prev_min_val = min_val;
    min_val = max_top;
  }
  int zero_append = 0;
  for (auto &discard : discard_pile) {
    tup->field_list[0].f.int_field.val = discard;
    tup->field_list[0].f.int_field.genval = -1;
    append_tuple(&tb, tup);
    zero_append++;
  }
  free(tup);
  return tb.table;
}

/*
 * This assume generalized values are integers.
 * This algorithm accounts for generalization accross multiple tables.
 * We assume that the ranges given for the control flow attribute are
 * continuous integer ranges.
 */
table_t *generalize_table(
    std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
        table_map_host_table_pairs,
    int num_hosts, int k) {

  bool needs_merge = true;
  // Hard coded assumption that we are only dealing with natural numbers.
  std::unordered_map<table_name, gen_map_t> table_gen_map;
  uint64_t max_val = 0;
  for (auto &table_queries : table_map_host_table_pairs) {
    auto host_table_pairs = table_queries.second;
    gen_map_t gen_map;
    for (auto &ht : host_table_pairs) {
      table_t *t = ht.second;
      for (int i = 0; i < t->num_tuples; i++) {
        tuple_t *tup = get_tuple(i, t);
        if (max_val < tup->field_list[0].f.int_field.val) {
          max_val = tup->field_list[0].f.int_field.val;
        }
        gen_map[tup->field_list[0].f.int_field.val].emplace_back(
            (hostnum)ht.first, (tup_count)tup->field_list[1].f.int_field.val,
            tup->field_list[0].f.int_field.val);
      }
    }
    table_gen_map[table_queries.first] = gen_map;
  }

  table_builder_t tb;
  schema_t schema;
  schema.num_fields = 1;
  schema.fields[0].col_no = 0;
  schema.fields[0].type = INT;
  strncpy(schema.fields[0].field_name, "cf_hash_orig\0", FIELD_NAME_LEN);
  init_table_builder(max_val, 1 /* num_columns */, &schema, &tb);
  auto *tup = (tuple_t *)malloc(tb.size_of_tuple);
  tup->num_fields = 1;
  tup->field_list[0].type = INT;

  int min_val = 0;
  int prev_min = min_val;
  while (min_val <= max_val) {
    vector<int> top_values;
    for (auto &a : table_gen_map) {
      int top_end = get_min_range(a.second, min_val, max_val, num_hosts, k);
      top_values.push_back(top_end);
    }
    int max_top = *max_element(std::begin(top_values), std::end(top_values));
    int min_top = *min_element(std::begin(top_values), std::end(top_values));
    if (min_top == -1 && max_top != -1) {
      throw;
    }
    if (max_top == -1) {
      for (int i = min_val; i <= max_val; i++) {
        tup->field_list[0].f.int_field.val = i;
        tup->field_list[0].f.int_field.genval = prev_min;
        append_tuple(&tb, tup);
        max_top = max_val;
      }
    } else {
      for (int i = min_val; i <= max_top; i++) {
        tup->field_list[0].f.int_field.val = i;
        tup->field_list[0].f.int_field.genval = min_val;
        append_tuple(&tb, tup);
      }
    }
    prev_min = min_val;
    min_val = max_top + 1;
  }
  free(tup);
  return tb.table;
}

/*
 * This is a greedy generalization algorithm. As input, it takes a list of
 * tables indexed by host, number of hosts in the cluster, and the k anonymous
 * parameter 'k'. The tables are assumed to be generated from
 * SELECT col, count(*) FROM table GROUP BY col
 *
 * The algorithm works in two phases:
 * First it builds a map of control flow attributes to vectors containing the
 * hostname and counts per host name. It is crucial that we keep the
 * information about how many tuples come from each host.
 *
 * Second, it iterates through the equivalence classes and determines if they
 * are k-anonymous. If they are not k-anonymous the equivalence class along
 * with the total number of tuples is added to a "merge-list". The greedy
 * heurstic we use is that classes with the high tuple count should be merged
 * with classes of low tuple count.
 *
 *
 */
table_t *
generalize_table(std::vector<std::pair<hostnum, table_t *>> host_table_pairs,
                 int num_hosts, int k) {
  // std::pair<host_num, count>

  // Step 1: Figure `
  std::unordered_map<cf_hash,
                     std::vector<std::tuple<hostnum, tup_count, cf_hash>>>
      gen_map;
  for (auto &ht : host_table_pairs) {
    table_t *t = ht.second;
    for (int i = 0; i < t->num_tuples; i++) {
      tuple_t *tup = get_tuple(i, t);
      gen_map[tup->field_list[0].f.int_field.val].emplace_back(
          (hostnum)ht.first, (tup_count)tup->field_list[1].f.int_field.val,
          tup->field_list[0].f.int_field.val);
    }
  }

  bool needs_merging = true;
  std::vector<std::tuple<cf_hash, score>> merges;

  // TODO(madhavsuresh): there is a bug here
  while (needs_merging) {
    std::sort(merges.begin(), merges.end(), kscore);
    if (merges.size() >= 2) {
      int forward = 0;
      int backward = merges.size() - 1;
      while (forward < backward) {
        if (merges.size() <= forward) {
          throw std::invalid_argument("This is not a valid merge argument");
        }
        auto m1 = std::get<0>(merges[forward]);
        auto m2 = std::get<0>(merges[backward]);
        merge(gen_map, m1, m2);
        backward--;
        while (!is_kanon(gen_map[m1], num_hosts, k) && backward > forward) {
          auto m3 = std::get<0>(merges[backward]);
          merge(gen_map, m1, m3);
          backward--;
        }
        forward++;
      }
    }
    merges.clear();
    for (auto g : gen_map) {
      if (!is_kanon(g.second, num_hosts, k)) {
        int score = 0;
        for (auto i : g.second) {
          // Get the total number of tuples
          score += std::get<1>(i);
        }
        merges.emplace_back(g.first, score);
      }
    }
    if (merges.size() > 1) {
      needs_merging = true;
    } else if (merges.size() == 1) {
      cf_hash smallest =
          find_smallest_equiv_not_eq(gen_map, std::get<0>(merges[0]));
      // This will happen when k is too large for the input values.
      // In this case, we just return the table which is "oblivious"
      if (smallest == -1) {
        needs_merging = false;
      } else {
        merge(gen_map, smallest, std::get<0>(merges[0]));
      }
    } else {
      needs_merging = false;
    }
  }
  return generate_genmap_table(gen_map);
}

table_t *generalize_zip(table_t *t, table_t *gen_map_table, int col_no) {
  std::unordered_map<cf_hash, cf_gen> gen_map;
  for (int i = 0; i < gen_map_table->num_tuples; i++) {
    tuple_t *tup = get_tuple(i, gen_map_table);
    gen_map[tup->field_list[0].f.int_field.val] =
        tup->field_list[0].f.int_field.genval;
  }
  int expected_tuples = t->num_tuples;
  int num_columns = t->schema.num_fields;
  table_builder_t tb;
  init_table_builder(t->num_tuples, t->schema.num_fields, &t->schema, &tb);
  for (int i = 0; i < t->num_tuples; i++) {
    tuple_t *tup = get_tuple(i, t);
    auto gen_val = gen_map[tup->field_list[col_no].f.int_field.val];
    if (gen_val != -1) {
      tup->field_list[col_no].f.int_field.genval = gen_val;
      append_tuple(&tb, tup);
    }
  }
  free_table(t);
  return tb.table;
}

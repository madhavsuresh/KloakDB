//
// Created by madhav on 11/14/18.
//

// Typedefs to make code easier to read without using classes.
#include "Generalize.h"
#include "logger/Logger.h"
#include "logger/LoggerDefs.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

void log_stats(
    std::unordered_map<cf_hash,
                       std::vector<std::tuple<hostnum, tup_count, cf_hash>>>
        gen_map,
    int k) {
  int max_size = 0;
  cf_hash _max = 0;
  double total_size = 0;
  double avg_num = 0;
  for (auto &i : gen_map) {
    int curr_size = 0;
    avg_num += i.second.size();
    for (auto &j : i.second) {
      curr_size += std::get<1>(j);
    }
    if (max_size < curr_size) {
      max_size = curr_size;
      _max = i.first;
    }
    if (curr_size < k) {
      // std::cout << "THIS IS BAD!!" << std::endl;
    }
    total_size += curr_size;
  }
  LOG(OP) << "max key: [" << _max << "] : len:(" << gen_map[_max].size()
          << ") max size: " << max_size << "average size "
          << total_size / gen_map.size() << " num cf_per class"
          << avg_num / gen_map.size();
}

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


//TODO(madhavsuresh): there is a terrible bug here!
// If the top range between two tables does not overlap,
// there will be srs consequences
int get_min_range(gen_map_t gen_map, int min_value, int max_value,
                  int num_hosts, int k) {

  std::unordered_map<cf_hash, eq_class_t> val_map;
  eq_class_t eqv;
  for (int i = min_value;i <= max_value; i++) {
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
                (hostnum) ht.first, (tup_count) tup->field_list[1].f.int_field.val,
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
    auto *tup = (tuple_t *) malloc(tb.size_of_tuple);
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
          max_top  = max_val;
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
  table_t *generalize_table(
      std::vector<std::pair<hostnum, table_t *>> host_table_pairs,
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
    log_stats(gen_map, k);
    return generate_genmap_table(gen_map);
  }

  table_t *generalize_zip(table_t * t, table_t * gen_map_table, int col_no) {
    std::unordered_map<cf_hash, cf_gen> gen_map;
    for (int i = 0; i < gen_map_table->num_tuples; i++) {
      tuple_t *tup = get_tuple(i, gen_map_table);
      gen_map[tup->field_list[0].f.int_field.val] =
          tup->field_list[0].f.int_field.genval;
    }
    for (int i = 0; i < t->num_tuples; i++) {
      tuple_t *tup = get_tuple(i, t);
      auto gen_val = gen_map[tup->field_list[col_no].f.int_field.val];
      tup->field_list[col_no].f.int_field.genval = gen_val;
    }
    return t;
  }

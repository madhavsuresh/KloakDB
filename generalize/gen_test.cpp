//
// Created by madhav on 11/12/18.
//

#include <string>
#include <gtest/gtest.h>
#include <data/pqxx_compat.h>
#include <chrono>
#include <unordered_map>

class gen_test : public ::testing::Test {
public:
    std::string xdbname;
    std::string dbname;

protected:
    void SetUp() override {
      xdbname = "kloak_testing_gen";
      std::string command = "createdb " + xdbname;
      system(command.c_str());
      dbname = "dbname=" + xdbname;
    }

    void TearDown() override{
      std::string command = "dropdb " + xdbname;
      system(command.c_str());
    };

};

typedef int cf_hash;
typedef int hostnum;
typedef int count;
typedef int numhosts;
typedef double score;

bool is_kanon(std::vector<std::tuple<hostnum, count, cf_hash>> equiv_class, int num_hosts, int k) {

  // If there is only one host in an equivalence class,
  // we only need it to be more than k.
  if (equiv_class.size() == 1) {
    if (std::get<1>(equiv_class[0]) < k) {
      return false;
    }
    return true;
  }

  for (int i = 0; i < num_hosts; i++) {
    int accumulator = 0;
    for (auto &v : equiv_class) {
      if (i != std::get<0>(v)) {
        accumulator+= std::get<1>(v);
      }
    }
    if (accumulator < k) {
      return false;
    }
  }
  return true;
}

void merge(std::unordered_map<cf_hash, std::vector<std::tuple<hostnum, count, cf_hash>>> &gen_map, cf_hash h1, cf_hash h2) {
  for (auto &i : gen_map[h2]) {
    gen_map[h1].push_back(i);
  }
  gen_map.erase(h2);
}

bool kscore(std::tuple<int, count> p1, std::tuple<int, count> p2) {
  return std::get<1>(p1) < std::get<1>(p2);
}

/*
 * This is a greedy generalization algorithm. As input, it takes a list of tables indexed by host,
 * number of hosts in the cluster, and the k anonymous parameter 'k'
 *
 * The algorithm works in two phases:
 * First it builds a map of control flow attributes to vectors containing the hostname and counts per host name.
 * It is crucial that we keep the information about how many tuples come from each host.
 *
 * Second, it iterates through the equivalence classes and determines if they are k-anonymous. If they are not
 * k-anonymous the equivalence class along with the total number of tuples is added to a "merge-list".
 * The greedy heurstic we use is that classes with the high tuple count should be merged with classes of low tuple count.
 *
 *
 */
void generalize(std::vector<std::pair<hostnum, table_t *>> host_table_pairs,
        int num_hosts, int k) {
  // std::pair<host_num, count>

  // Step 1: Figure `
  std::unordered_map<cf_hash, std::vector<std::tuple<hostnum, count, cf_hash>>> gen_map;
  for (auto &ht: host_table_pairs) {
    table_t * t = ht.second;
    for (int i = 0; i < t->num_tuples; i++) {
      tuple_t *tup = get_tuple(i, t);
      gen_map[tup->field_list[0].f.int_field.val].emplace_back((hostnum) ht.first,
              (count) tup->field_list[1].f.int_field.val, tup->field_list[0].f.int_field.val);
    }
  }

  bool needs_merging = true;
  std::vector<std::tuple<cf_hash, score>> merges;

  while (needs_merging) {
    std::sort(merges.begin(), merges.end(), kscore);
    if (merges.size() >= 2) {
      int forward = 0;
      int backward = merges.size()-1;
      while (forward < backward) {

        auto m1 = std::get<0>(merges[forward]);
        auto m2 = std::get<0>(merges[backward]);
        merge(gen_map, m1, m2);
        while (!is_kanon(gen_map[m1], num_hosts, k) && backward > forward) {
          auto m3 = std::get<0>(merges[backward]);
          merge(gen_map, m1, m3);
          backward--;
        }
        forward++;
      }
    }
    merges.clear();
    //std::cout << gen_map.size() << std::endl;
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
    std::cout << "MERGES SIZE: " << merges.size();
    if (merges.size() > 1) {
      needs_merging = true;
    } else if (merges.size() == 1) {

    } else {
      needs_merging = false;
    }
  }
  int max_size = 0;
  double total_size = 0;
  double avg_num_blah = 0;
  for (auto &i : gen_map) {
    int curr_size = 0;
    avg_num_blah += i.second.size();
    for (auto &j : i.second) {
      curr_size += std::get<1>(j);
    }
    if (max_size < curr_size) {
      max_size = curr_size;
    }
    if (curr_size < k) {
      std::cout << "THIS IS BAD!!" << std::endl;
    }
    total_size += curr_size;
  }
  std::cout << std::endl << gen_map.size() <<  "max size: " << max_size <<  "average size " << total_size/ gen_map.size() << " num cf_per class" << avg_num_blah / gen_map.size() <<  std::endl;
}

TEST_F(gen_test, single_host) {
  query("CREATE TABLE gen_test (a INT, b INT)", dbname);
  query("INSERT into gen_test (a,b) select i, floor(random() * 1000+1)::int from generate_series(1,10000) s(i)", dbname);
  table_t * t = get_table("SELECT b, count(b) FROM gen_test group by b order by b", dbname);
  auto start = std::chrono::high_resolution_clock::now();

  std::vector<std::pair<hostnum , table_t*>> input;
  input.emplace_back(0, t);
  input.emplace_back(1, t);
  input.emplace_back(2, t);
  input.emplace_back(3, t);
  generalize(input, 4, 4000);
  auto end_first_join = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end_first_join - start;
  std::cout << "First Join Elapsed time: " << elapsed.count() << " s\n";
  free_table(t);

}


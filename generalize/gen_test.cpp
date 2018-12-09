//
// Created by madhav on 11/12/18.
//

#include <string>
#include <gtest/gtest.h>
#include <data/pqxx_compat.h>
#include <chrono>
#include <unordered_map>
#include <operators/Generalize.h>
#include <logger/Logger.h>

class gen_test : public ::testing::Test {
public:
    std::string xdbname;
    std::string dbname;

protected:
    void SetUp() override {
    }

    void TearDown() override{
    };

};



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
  auto i = generalize_table(input, 4, 4000);
  auto end_first_join = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end_first_join - start;
  std::cout << "First Join Elapsed time: " << elapsed.count() << " s\n";
  free_table(t);

}
void log_gen_stats(table_t *gen_map) {
  std::map<int, int> mapping_table;
  for (int i = 0; i < gen_map->num_tuples; i++) {
    mapping_table[get_tuple(i, gen_map)->field_list[0].f.int_field.genval]++;
  }
  std::string out;
  for (auto t : mapping_table) {
    out += "Gen value:" + std::to_string(t.first) +
           ", COUNT:" + std::to_string(t.second) + "\n";
  }
  printf("%s\n", out.c_str());
}
TEST_F(gen_test, left_deep) {
  table_t *t =  get_table("SELECT b, count(b) FROM left_deep_joins_1024 group by b", "dbname=vaultdb_");


  std::vector<std::pair<hostnum , table_t*>> input;
  input.emplace_back(0, t);
  input.emplace_back(1, t);
  input.emplace_back(2, t);
  input.emplace_back(3, t);
  auto i = generalize_table(input, 4, 10);
  auto i2 = generalize_zip(t, i, 1);
  for (int q = 0; q < i2->num_tuples; q++) {
    //tuple_t * z = get_tuple(q, i2);
    printf("%s\n", tuple_string(get_tuple(q, i2)).c_str());
  }
  log_gen_stats(i);
}

TEST_F(gen_test, new_gen) {
  table_t * t = get_table("SELECT b, count(b) FROM left_deep_joins_1024 group by b order by b asc", "dbname=vaultdb_");
  table_t * actual_table = get_table("SELECT * FROM left_deep_joins_1024;", "dbname=vaultdb_");
  std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
          tables;
  tables["one"].emplace_back(0,t);
  //tables["one"].emplace_back(1,t);
  //jtables["two"].emplace_back(0,t);
  //tables["two"].emplace_back(1,t);
  auto out = generalize_table(tables, 2, 6);
  printf("NUM TUPLES %d", out->num_tuples);
  for (int i = 0; i < out->num_tuples; i++) {
    if (get_tuple(i, out)->field_list[0].f.int_field.val != i) {
      printf("\nWHAT %d", i);
    }
    //std::cout << tuple_string(get_tuple(i, out)) << std::endl;
  }
  auto zipped = generalize_zip(actual_table, out, colno_from_name(actual_table, "b"));
  map<int, int> counter;
  for (int i = 0; i < zipped->num_tuples; i++) {
    counter[get_tuple(i, zipped)->field_list[1].f.int_field.genval]++;
    if (get_tuple(i, zipped)->field_list[1].f.int_field.genval == 9993) {
      std::cout << tuple_string(get_tuple(i, zipped)) << std::endl;
    }
  }
  int max_val = 0;
  int min_val = 100000;
  int num_classes = 0;
  for (auto &i : counter) {
    num_classes++;
    if (max_val < i.second) {
      max_val = i.second;
    }
    if (min_val > i.second) {
      min_val = i.second;
    }
    if (i.second == 891) {
      printf("\n891: %d", i.first);
    }
    if (i.second == 1) {

      printf("\n1: %d", i.first);
    }
  }
  printf("\nMIN: %d, MAX: %d, AVG: %f", min_val, max_val, (double)zipped->num_tuples/num_classes);
}

TEST_F(gen_test, gen_fast) {
  std::unordered_map<table_name, std::vector<std::pair<hostnum, table_t *>>>
          tables;
  table_t *r1h1 = get_table("SELECT b, count(b) FROM gen_test_a_h1 group by b", "dbname=vaultdb_");
  printf("Table 1\n");
  table_t *r1h2 = get_table("SELECT b, count(b) FROM gen_test_a_h2 group by b", "dbname=vaultdb_");
  printf("Table 2\n");
  table_t *r1h3 = get_table("SELECT b, count(b) FROM gen_test_a_h3 group by b", "dbname=vaultdb_");
  printf("Table 3\n");
  table_t *r1h4 = get_table("SELECT b, count(b) FROM gen_test_a_h4 group by b", "dbname=vaultdb_");
  printf("Table 4\n");
  tables["one"].emplace_back(0,r1h1);
  tables["one"].emplace_back(1,r1h2);
  tables["one"].emplace_back(2,r1h3);
  tables["one"].emplace_back(3,r1h4);
  table_t *r1[4];
  r1[0] = r1h1;
  r1[1] = r1h2;
  r1[2] = r1h3;
  r1[3] = r1h4;

  table_t *r2h1 = get_table("SELECT b, count(b) FROM gen_test_b_h1 group by b", "dbname=vaultdb_");
  printf("Table 5\n");
  table_t *r2h2 = get_table("SELECT b, count(b) FROM gen_test_b_h2 group by b", "dbname=vaultdb_");
  printf("Table 6\n");
  table_t *r2h3 = get_table("SELECT b, count(b) FROM gen_test_b_h3 group by b", "dbname=vaultdb_");
  printf("Table 7\n");
  table_t *r2h4 = get_table("SELECT b, count(b) FROM gen_test_b_h4 group by b", "dbname=vaultdb_");
  printf("Table 8\n");
  tables["two"].emplace_back(0,r2h1);
  tables["two"].emplace_back(1,r2h2);
  tables["two"].emplace_back(2,r2h3);
  tables["two"].emplace_back(3,r2h4);

  table_t *r3h1 = get_table("SELECT b, count(b) FROM gen_test_c_h1 group by b", "dbname=vaultdb_");
  printf("Table 9\n");
  table_t *r3h2 = get_table("SELECT b, count(b) FROM gen_test_c_h2 group by b", "dbname=vaultdb_");
  printf("Table 10\n");
  table_t *r3h3 = get_table("SELECT b, count(b) FROM gen_test_c_h3 group by b", "dbname=vaultdb_");
  printf("Table 11\n");
  table_t *r3h4 = get_table("SELECT b, count(b) FROM gen_test_c_h4 group by b", "dbname=vaultdb_");
  printf("Table 12\n");
  tables["three"].emplace_back(0,r3h1);
  tables["three"].emplace_back(1,r3h2);
  tables["three"].emplace_back(2,r3h3);
  tables["three"].emplace_back(3,r3h4);

  table_t *r4h1 = get_table("SELECT b, count(b) FROM gen_test_d_h1 group by b", "dbname=vaultdb_");
  printf("Table 13\n");
  table_t *r4h2 = get_table("SELECT b, count(b) FROM gen_test_d_h2 group by b", "dbname=vaultdb_");
  printf("Table 14\n");
  table_t *r4h3 = get_table("SELECT b, count(b) FROM gen_test_d_h3 group by b", "dbname=vaultdb_");
  printf("Table 15\n");
  table_t *r4h4 = get_table("SELECT b, count(b) FROM gen_test_d_h4 group by b", "dbname=vaultdb_");
  printf("Table 16\n");
  tables["four"].emplace_back(0,r4h1);
  tables["four"].emplace_back(1,r4h2);
  tables["four"].emplace_back(2,r4h3);
  tables["four"].emplace_back(3,r4h4);

  auto start = std::chrono::high_resolution_clock::now();
  auto out = generalize_table_fast(tables, 4, 5);
  auto end= std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout << "Gen Fast Time: " << elapsed.count() << " s\n";
  table_t * t1 = get_table("SELECT b FROM gen_test_a_h1", "dbname=vaultdb_");
  table_t * t2 = get_table("SELECT b FROM gen_test_a_h2", "dbname=vaultdb_");
  table_t * t3 = get_table("SELECT b FROM gen_test_a_h3", "dbname=vaultdb_");
  table_t * t4 = get_table("SELECT b FROM gen_test_a_h4", "dbname=vaultdb_");
  table_t *t[4];
  t[0] = t1;
  t[1] = t2;
  t[2] = t3;
  t[3] = t4;
  unordered_map<int, int> k_map;
  unordered_map<int, int> k_map1;
  unordered_map<int, int> k_map2;
  unordered_map<int, int> k_map3;
  unordered_map<int, int> k_map4;
  unordered_map<int, unordered_map<int,int>> k_maps;

  unordered_map<int, int> o_map;
  for (int i = 0; i < 4; i++) {
    generalize_zip(t[i], out, 0);
  }
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < t[i]->num_tuples; j++){
      auto field = get_tuple(j, t[i])->field_list[0].f.int_field;
      k_map[field.genval]++;
      o_map[field.val]++;
      for (int k = 0; k < 4; k++) {
        if (k != i) {
          k_maps[k][field.genval]++;
        }
      }
    }
  }

  int count = 0;
  double num = 0;
  for (auto &jj : k_map) {
    count += jj.second;
    num++;
  }
  std::cout << "Average Size Gen: " << count /num;
  count = 0;
  num = 0;
  for (auto &jj : o_map) {
    count += jj.second;
    num++;
  }
  std::cout << "Average Size Reg: " << count /num << std::endl;

  int moment = 25;
  for (int i = 0; i < 4; i++) {
    count = 0;
    num = 0;
    int max = 0;
    int bad_count = 0;
    int moment_counter = 0;
    int lower_moment_counter = 0;
    auto z = k_maps[i];
    for (auto &jj : z) {
      if (jj.second < 5) {
        bad_count++;
      }
      if (jj.second > max) {
        max = jj.second;
      }
      if (jj.second > moment) {
        moment_counter++;
      } else {
        lower_moment_counter++;
      }
      count += jj.second;
      num++;
    }
    ASSERT_EQ(bad_count, 0);
    std::cout << "Average Size Gen: " << i << "; "<<  count /num << " MAX:" << max << " Moment: " << moment_counter << " Lower moment: " << lower_moment_counter<< "Bad Count: " << bad_count << "\n";
  }
  moment = 20;
  for (int i = 0; i < 4; i++) {
    int max = 0;
    int bad_count = 0;
    int moment_counter = 0;
    int lower_moment_counter = 0;
    auto z = k_maps[i];
    for (auto &jj : z) {
      if (jj.second < 5) {
        bad_count++;
      }
      if (jj.second > max) {
        max = jj.second;
      }
      if (jj.second > moment) {
        moment_counter++;
      } else {
        lower_moment_counter++;
      }
      count += jj.second;
      num++;
    }
    ASSERT_EQ(bad_count, 0);
    std::cout << "Average Size Gen: " << i << "; "<<  count /num << " MAX:" << max << " Moment: " << moment_counter << " Lower moment: " << lower_moment_counter<< "Bad Count: " << bad_count << "\n";
  }
  moment = 15;
  for (int i = 0; i < 4; i++) {
    int max = 0;
    int bad_count = 0;
    int moment_counter = 0;
    int lower_moment_counter = 0;
    auto z = k_maps[i];
    for (auto &jj : z) {
      if (jj.second < 5) {
        bad_count++;
      }
      if (jj.second > max) {
        max = jj.second;
      }
      if (jj.second > moment) {
        moment_counter++;
      } else {
        lower_moment_counter++;
      }
      count += jj.second;
      num++;
    }
    ASSERT_EQ(bad_count, 0);
    std::cout << "Average Size Gen: " << i << "; "<<  count /num << " MAX:" << max << " Moment: " << moment_counter << " Lower moment: " << lower_moment_counter<< "Bad Count: " << bad_count << "\n";
  }
}

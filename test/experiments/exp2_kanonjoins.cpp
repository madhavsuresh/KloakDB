//
// Created by madhav on 11/8/18.
//

#include <gtest/gtest.h>
#include <chrono>
#include <operators/HashJoin.h>
#include <operators/Generalize.h>
#include "data/postgres_client.h"
#include "data/pqxx_compat.h"

// This test file tests "Experiment 2" for the KloakDB VLDB 2019 paper.
// Description:
//  - Run a single join on synthetic data, adjusting the k-anonymous parameter k
//​    The schema for the table is a single integer valued column.
//  - The synthetic data consists of packed ranges (?) or should it be random data (?) , run
//    both, see which is more fair. What leads to better introspection on our system? What
//    should the data look like?
// Questions:
//  - As k increases towards full oblivious, does the execution time for the join increase?
//  - How does the execution time grow as a function of k
//  - What is the shape of the execution time vs $k$ graph. Is it jumpy around areas where
//    generalization levels increase?
// Implementation:
//  - Generate a table of 10,000 tuples  (find LCM of 1*10*20*30*40*50*60*70*80*90*100)
//  - Manually set the dummy values for groups of 1,10,20,30,40,50,60,70,80,90,100
//  - Time Join for each value
//  - randomly generate two tables, or one table?
//  - Generalization will not be correlated to the input data.
//  - still need to scan over the output join table to get the correct join value.

class exp2_kanonjoins : public ::testing::Test {
public:
    std::string dbname;
    std::string xdbname;
    void SetUp() override {
      //TODO(madhavsuresh): append string
      xdbname = "kloak_experiments_exp2";
      std::string command = "createdb " + xdbname;
      system(command.c_str());
      dbname = "dbname=" + xdbname;
    }

    void TearDown() override{
      std::string command = "dropdb " + xdbname;
      system(command.c_str());
    };

};



TEST_F(exp2_kanonjoins, test1) {

  query("CREATE TABLE exp2_random (a INT, b INT)", dbname);
  query("INSERT into exp2_random (a,b) select i, floor(random() * 10000+1)::int from generate_series(1,25200) s(i)", dbname);
  table_t * t = get_table("SELECT * FROM exp2_random", dbname);
  table_t * cnt = get_table("SELECT b, count(*) from exp2_random group by b;", dbname);

  std::vector<std::pair<hostnum , table_t*>> input;
  input.emplace_back(0, cnt);
  join_def_t jd;
  jd.r_col = colno_from_name(t, "b");
  jd.l_col = colno_from_name(t, "b");
  jd.project_len = 1;
  jd.project_list[0].side = LEFT_RELATION;
  jd.project_list[0].col_no = colno_from_name(t, "b");

  for (int i = 1; i < 501; i+=10) {
    auto start = std::chrono::high_resolution_clock::now();
    table_t * z = generalize_table(input, 1, i);
    table_t * genned = generalize_zip(t, z, colno_from_name(t, "b"));
    auto join = std::chrono::high_resolution_clock::now();
    table_t * out =  hash_join(genned,genned, jd);
    free_table(out);
    free_table(z);
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
  }

  for (int i = 1; i < 501; i+=10) {
    auto start = std::chrono::high_resolution_clock::now();
    table_t * z = generalize_table(input, 1, i);
    table_t * genned = generalize_zip(t, z, colno_from_name(t, "b"));
    auto join = std::chrono::high_resolution_clock::now();
    table_t * out =  hash_join(genned,genned, jd);
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    printf("\n%d\n",out->num_tuples);
    int total_output = 0;
    for (int j = 0; j < out->num_tuples; j++) {
      if (!get_tuple(j, out)->is_dummy) {
        total_output++;
      }
    }
    free_table(out);
    free_table(z);
    std::cout << "Total Output:" << total_output;
    std::cout << i << "-anon Total Elapsed time: " << elapsed.count() << " s\n";
    elapsed = join - start;
    std::cout << i << "Gen Elapsed time: " << elapsed.count() << " s\n";
    elapsed = finish - join;
    std::cout << i << "Join Elapsed time: " << elapsed.count() << " s\n";
  }
}

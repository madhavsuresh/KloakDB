//
// Created by madhav on 11/12/18.
//

#include <string>
#include <gtest/gtest.h>
#include <data/pqxx_compat.h>
#include <chrono>
#include <unordered_map>
#include <operators/Generalize.h>

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


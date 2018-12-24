//
// Created by madhav on 12/23/18.
//

#ifndef PROJECT_DIST_GEN_TEST_H
#define PROJECT_DIST_GEN_TEST_H
#include <rpc/HonestBrokerPrivate.h>

void gen_test_rand_table(HonestBrokerPrivate *p, std::string dbname,
                         std::string table1, std::string table2, int k,
                         bool in_sgx, string tag);

#endif // PROJECT_DIST_GEN_TEST_H

//
// Created by madhav on 11/29/18.
//

#ifndef PROJECT_DIST_TPCH_10
#define PROJECT_DIST_TPCH_10

#include <VaultDB.h>
#include <rpc/HonestBrokerPrivate.h>
void tpch_10_encrypted(HonestBrokerPrivate *p, std::string database, bool sgx);
void tpch_10_gen(HonestBrokerPrivate *p, std::string database, bool sgx, int gen_level); 

#endif // PROJECT_DISTRIBUTED_ASPIRIN_PROFILE_H

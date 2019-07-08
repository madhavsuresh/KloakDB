//
// Created by madhav on 11/29/18.
//

#ifndef PROJECT_DIST_TPCH_3
#define PROJECT_DIST_TPCH_3

#include <VaultDB.h>
#include <rpc/HonestBrokerPrivate.h>
void tpch_3_encrypted(HonestBrokerPrivate *p, std::string database, bool sgx);
void tpch_3_gen(HonestBrokerPrivate *p, std::string database, bool sgx, int gen_level);
void tpch_3_obli(HonestBrokerPrivate *p, std::string database, bool sgx);

#endif // PROJECT_DISTRIBUTED_ASPIRIN_PROFILE_H

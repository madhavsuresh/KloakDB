//
// Created by madhav on 11/29/18.
//

#ifndef PROJECT_DISTRIBUTED_ASPIRIN_PROFILE_H
#define PROJECT_DISTRIBUTED_ASPIRIN_PROFILE_H

#include <VaultDB.h>
#include <rpc/HonestBrokerPrivate.h>

void aspirin_profile(HonestBrokerPrivate *p, std::string database,
                     std::string diagnoses_table, std::string vitals_table,
                     std::string medications_table,
                     std::string demographics_table, std::string year,
                     bool sgx);

#endif // PROJECT_DISTRIBUTED_ASPIRIN_PROFILE_H

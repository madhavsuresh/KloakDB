//
// Created by madhav on 12/1/18.
//

#ifndef PROJECT_DISTRIBUTED_DOSAGE_H
#define PROJECT_DISTRIBUTED_DOSAGE_H
#include "logger/LoggerDefs.h"
#include <rpc/HonestBrokerPrivate.h>
void dosage_encrypted(HonestBrokerPrivate *p, std::string dbname,
                      std::string diag, std::string meds, std::string year);

void dosage_obliv(HonestBrokerPrivate *p, std::string dbname,
                  std::string diag, std::string meds, std::string year);
void dosage_k(HonestBrokerPrivate *p, std::string dbname,
              std::string diag, std::string meds, std::string year, int gen_level);
#endif // PROJECT_DISTRIBUTED_DOSAGE_H

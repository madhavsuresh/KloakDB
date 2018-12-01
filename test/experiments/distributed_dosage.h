//
// Created by madhav on 12/1/18.
//

#ifndef PROJECT_DISTRIBUTED_DOSAGE_H
#define PROJECT_DISTRIBUTED_DOSAGE_H
#include "logger/LoggerDefs.h"
#include <rpc/HonestBrokerPrivate.h>
void dosage_encrypted(HonestBrokerPrivate *p, std::string dbname,
                      std::string diag, std::string meds, std::string year);

#endif // PROJECT_DISTRIBUTED_DOSAGE_H

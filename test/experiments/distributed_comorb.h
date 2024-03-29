//
// Created by madhav on 11/30/18.
//

#ifndef PROJECT_DISTRIBUTED_COMORB_H
#define PROJECT_DISTRIBUTED_COMORB_H

#include <rpc/HonestBrokerPrivate.h>
#include "logger/LoggerDefs.h"

void comorbidity_encrypted(HonestBrokerPrivate *p, std::string dbname, std::string year);
void comorbidity_oliv(HonestBrokerPrivate *p, std::string dbname, std::string year);
void comorbidity_keq5(HonestBrokerPrivate *p, std::string dbname, std::string year);
#endif //PROJECT_DISTRIBUTED_COMORB_H

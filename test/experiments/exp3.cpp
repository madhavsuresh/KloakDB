//
// Created by madhav on 11/26/18.
//

#include <gflags/gflags.h>
#include <rpc/HonestBrokerPrivate.h>
#include <logger/LoggerDefs.h>

DEFINE_int32(exp3_datasize, 100, "exp3 data size");
/*
 * CREATE statement  FOR oblivious_partitioning*
 * CREATE TABLE oblivious_partitioning_1000 (a INT, b INT, c INT, d INT);
 *
 */


void exp3(HonestBrokerPrivate *p) {
  auto scan = p->ClusterDBMSQuery("dbname=vaultdb_",
                                  "SELECT * from oblivious_partitioning" +
                                      std::to_string(FLAGS_exp3_datasize));
  p->SetControlFlowColName("b");
  START_TIMER(repartition_outer);
  p->Repartition(scan);
  END_AND_LOG_EXEC_TIMER(repartition_outer);
}

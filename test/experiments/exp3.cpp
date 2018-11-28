//
// Created by madhav on 11/26/18.
//

#include <gflags/gflags.h>
#include <logger/LoggerDefs.h>
#include <rpc/HonestBrokerPrivate.h>

DEFINE_int32(exp3_datasize, 100, "exp3 data size");
/*
 * CREATE statement  FOR oblivious_partitioning*
 * CREATE TABLE oblivious_partitioning_1000 (a INT, b INT, c INT, d INT);
 *
 */

void exp3(HonestBrokerPrivate *p) {
  LOG(EXP3_STAT) << "=============STARTING EXP3 WITH DATA SIZE:[" << FLAGS_exp3_datasize << "] ============";
  p->SetControlFlowColName("b");
  auto scan = p->ClusterDBMSQuery("dbname=vaultdb_",
                                  "SELECT * from oblivious_partitioning_" +
                                      std::to_string(FLAGS_exp3_datasize));
  START_TIMER(repartition_outer);
  p->Repartition(scan);
  END_AND_LOG_EXP3_STAT_TIMER(repartition_outer);
  LOG(EXP3_STAT) << "============ENDING EXP3 WITH DATA SIZE:[" << FLAGS_exp3_datasize << "]=========";
}

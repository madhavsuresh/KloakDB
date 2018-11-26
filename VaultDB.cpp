//
// Created by madhav on 9/25/18.
//

#include "VaultDB.h"
#include "logger/Logger.h"
#include "logger/LoggerDefs.h"
#include "rpc/DataOwnerImpl.h"
#include "rpc/DataOwnerPrivate.h"
#include "rpc/HonestBrokerClient.h"
#include "rpc/HonestBrokerImpl.h"
#include <future>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <gflags/gflags.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_builder.h>
#include <thread>

DEFINE_bool(honest_broker, false, "Setup as honest broker");
DEFINE_string(address, "", "IPV4 Address for current instance");
DEFINE_string(
    honest_broker_address, "",
    "IPV4 Address for honest broker instance (set by non-honest broker)");
DEFINE_int32(experiment, 1, "experiment number");

DEFINE_int32(gen_level, 5, "generalization level");

DEFINE_string(database, "smcql_testDB", "database name");
DEFINE_string(diagnoses_table, "diagnoses", "table name for diagnoses");
DEFINE_string(medications_table, "medications", "table name for medications");
DEFINE_string(demographics_table, "demographics",
              "table name for demographics");
DEFINE_string(vitals_table, "vitals", "table name for vitals");
DEFINE_string(cdiff_cohort_diag_table, "cdiff_cohort_diagnoses",
              "table name for cdiff cohort diagnoses");
DEFINE_string(logger_host_name, "guinea-pig.cs.northwestern.edu:60000",
              "port for logger");
DEFINE_string(host_short, "vaultdb", "short host name");

using namespace std;
using namespace vaultdb;

std::promise<void> exit_requested;

vector<pair<shared_ptr<const TableID>, shared_ptr<const TableID>>>
zip_join_tables(vector<shared_ptr<const TableID>> &left_tables,
                vector<shared_ptr<const TableID>> &right_tables) {
  vector<pair<shared_ptr<const TableID>, shared_ptr<const TableID>>> ret;

  for (auto &lt : left_tables) {
    bool set = false;
    google::int32 host_num;
    host_num = lt.get()->hostnum();
    for (auto &rt : right_tables) {
      if (host_num == rt.get()->hostnum()) {
        if (set) {
          throw;
        }
        ret.emplace_back(lt, rt);
        set = true;
        continue;
      }
    }
    if (!set) {
      throw;
    }
  }
  return ret;
}

void exp5(HonestBrokerPrivate *p) {
  auto scan = p->ClusterDBMSQuery("dbname=vaultdb_",
                                  "SELECT * FROM left_deep_joins_1024");
  unordered_map<table_name, to_gen_t> gen_in;
  to_gen_t to_gen_ld;
  to_gen_ld.column = "b";
  to_gen_ld.dbname = "vaultdb_";
  to_gen_ld.scan_tables.insert(to_gen_ld.scan_tables.end(), scan.begin(),
                               scan.end());
  gen_in["left_deep_joins_1024"] = to_gen_ld;
  auto gen_zipped_map = p->Generalize(gen_in, FLAGS_gen_level);
  auto gen_zipped = gen_zipped_map["left_deep_joins_1024"];

  p->SetControlFlowColName("b");
  LOG(EXEC) << "======Start Repartition====";
  START_TIMER(repartition_exec);
  auto repart = p->Repartition(gen_zipped);
  END_AND_LOG_EXEC_TIMER(repartition_exec);
  LOG(EXEC) << "======End Repartition====";
  auto to_join1 = zip_join_tables(repart, repart);
  JoinDef jd;
  jd.set_l_col_name("b");
  jd.set_r_col_name("b");
  jd.set_project_len(1);
  auto p1 = jd.add_project_list();
  p1->set_colname("b");
  p1->set_side(JoinColID_RelationSide_LEFT);
  LOG(EXEC) << "======Start Join 1====";
  START_TIMER(join1);
  auto out1 = p->Join(to_join1, jd, true);
  END_AND_LOG_EXEC_TIMER(join1);
  LOG(EXEC) << "======END Join 1====";
  auto to_join2 = zip_join_tables(repart, out1);
  LOG(EXEC) << "======Start Join 2====";
  START_TIMER(join2);
  auto out2 = p->Join(to_join2, jd, true);
  END_AND_LOG_EXEC_TIMER(join2);
  LOG(EXEC) << "======END Join 2====";
  auto to_join3 = zip_join_tables(repart, out2);
  /*
  LOG(EXEC) << "======Start Join 3====";
  START_TIMER(join3);
  */
  // auto out3 = p->Join(to_join3, jd, false /* in_sgx */);
  /*
  END_AND_LOG_EXEC_TIMER(join3);
  LOG(EXEC) << "======END Join 3====";
  auto to_join4 = zip_join_tables(repart, out3);
  LOG(EXEC) << "======Start Join 4====";
  START_TIMER(join4);
  */
  // auto out4 = p->Join(to_join4, jd, false /* in_sgx */);
  /*
  END_AND_LOG_EXEC_TIMER(join4);
  LOG(EXEC) << "======END Join 4====";
  */
}

void aspirin_profile(HonestBrokerPrivate *p) {
  auto meds_scan = p->ClusterDBMSQuery(
      "dbname=" + FLAGS_database, "SELECT * from " + FLAGS_medications_table);
  auto demographics_scan = p->ClusterDBMSQuery(
      "dbname=" + FLAGS_database, "SELECT * from " + FLAGS_demographics_table);
  auto diagnoses_scan = p->ClusterDBMSQuery(
      "dbname=" + FLAGS_database, "SELECT * from " + FLAGS_diagnoses_table);
  auto vitals_scan = p->ClusterDBMSQuery("dbname=" + FLAGS_database,
                                         "SELECT * from " + FLAGS_vitals_table);
  p->Generalize("vitals" /* table name */,
                "patient_id" /* generalization column */, FLAGS_database,
                vitals_scan, FLAGS_gen_level);

  // join def vitals-diagnoses
  JoinDef jd_vd;
  jd_vd.set_l_col_name("patient_id");
  jd_vd.set_r_col_name("patient_id");
  jd_vd.set_project_len(2);
  // vitals-diagnoses-join project 1
  auto vdjp1 = jd_vd.add_project_list();
  vdjp1->set_side(JoinColID_RelationSide_LEFT);
  vdjp1->set_colname("patient_id");
  auto vdjp2 = jd_vd.add_project_list();
  vdjp2->set_side(JoinColID_RelationSide_LEFT);
  vdjp2->set_colname("pulse");
  auto to_join1 = zip_join_tables(vitals_scan, diagnoses_scan);
  auto out_vd_join = p->Join(to_join1, jd_vd, false /* in_sgx */);

  // join def first join "plus medications"
  // join between output of vitals/diagnonses join and medications
  JoinDef jd_pm2;
  jd_pm2.set_l_col_name("patient_id");
  jd_pm2.set_r_col_name("patient_id");
  jd_pm2.set_project_len(2);
  // plus medications project 1
  auto pmp1 = jd_pm2.add_project_list();
  pmp1->set_side(JoinColID_RelationSide_LEFT);
  pmp1->set_colname("patient_id");
  auto pmp2 = jd_pm2.add_project_list();
  pmp2->set_side(JoinColID_RelationSide_LEFT);
  pmp2->set_colname("pulse");
  auto to_join2 = zip_join_tables(out_vd_join, meds_scan);
  auto out_pm_join = p->Join(to_join2, jd_pm2, false /* in_sgx */);

  // join def second join "plus demographics"
  JoinDef jd_pd3;
  jd_pd3.set_l_col_name("patient_id");
  jd_pd3.set_r_col_name("patient_id");
  jd_pd3.set_project_len(3);
  auto pdp1 = jd_pd3.add_project_list();
  pdp1->set_side(JoinColID_RelationSide_LEFT);
  pdp1->set_colname("pulse");
  auto pdp2 = jd_pd3.add_project_list();
  pdp2->set_side(JoinColID_RelationSide_RIGHT);
  pdp2->set_colname("gender");
  auto pdp3 = jd_pd3.add_project_list();
  pdp3->set_side(JoinColID_RelationSide_RIGHT);
  pdp3->set_colname("race");
  auto to_join3 = zip_join_tables(out_pm_join, demographics_scan);
  auto out_pd_join = p->Join(to_join3, jd_pd3, false /* in_sgx */);

  GroupByDef gbd;
  gbd.set_type(GroupByDef_GroupByType_AVG);
  gbd.set_col_name("pulse");
  gbd.add_gb_col_names("gender");
  gbd.add_gb_col_names("pulse");
  auto final_avg = p->Aggregate(out_pd_join, gbd);
}

void comorbidity(HonestBrokerPrivate *p) {
  auto cdiff_cohort_scan =
      p->ClusterDBMSQuery("dbname=" + FLAGS_database,
                          "SELECT * from " + FLAGS_cdiff_cohort_diag_table);
  p->SetControlFlowColName("major_icd9");
  // TODO(madhavsuresh): add generalization
  auto cdiff_cohort_repart = p->Repartition(cdiff_cohort_scan);

  GroupByDef gbd;
  gbd.set_col_name("major_icd9");
  gbd.set_type(GroupByDef_GroupByType_COUNT);
  auto agg_out = p->Aggregate(cdiff_cohort_repart, gbd);

  p->SetControlFlowColName("major_icd9");
  auto cnt_repartition = p->Repartition(agg_out);
  SortDef sort;
  sort.set_colname("count");
  sort.set_ascending(false);
  auto sorted = p->Sort(cnt_repartition, sort);
}

void dosage_study(HonestBrokerPrivate *p) {
  auto diag_scan = p->ClusterDBMSQuery(
      "dbname=" + FLAGS_database, "SELECT * from " + FLAGS_diagnoses_table);
  auto med_scan = p->ClusterDBMSQuery(
      "dbname=" + FLAGS_database, "SELECT * from " + FLAGS_medications_table);
  // auto to_join = zip_join_tables(diag_scan, med_scan);
  p->SetControlFlowColName("patient_id");
  auto diag_repart = p->Repartition(diag_scan);
  auto med_repart = p->Repartition(med_scan);
  auto to_join = zip_join_tables(diag_repart, med_repart);

  JoinDef jd;
  jd.set_l_col_name("patient_id");
  jd.set_r_col_name("patient_id");
  jd.set_project_len(1);
  auto join_project = jd.add_project_list();
  join_project->set_colname("patient_id");
  join_project->set_col_no(JoinColID_RelationSide_LEFT);

  auto output_join = p->Join(to_join, jd, true /* in_sgx */);
}

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::unique_ptr<g3::LogWorker> logworker{g3::LogWorker::createLogWorker()};
  logworker->addSink(
      std::make_unique<RemoteSink>(
          FLAGS_host_short,
          grpc::CreateChannel(FLAGS_logger_host_name,
                              grpc::InsecureChannelCredentials())),
      &RemoteSink::ReceiveLogMessage);
  g3::initializeLogging(logworker.get());

  if (FLAGS_honest_broker == true) {
    LOG(INFO) << "Starting Vaultdb Sesssion";
    // auto defaultHandler = logworker->addDefaultLogger("HB", "logs");
    HonestBrokerPrivate *p = new HonestBrokerPrivate(FLAGS_address);
    HonestBrokerImpl hb(p);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(p->HostName(), grpc::InsecureServerCredentials());
    builder.RegisterService(&hb);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    auto serveFn = [&]() { server->Wait(); };
    std::thread hb_thread(serveFn);

    p->WaitForAllHosts();
    p->RegisterPeerHosts();

    // dosage_study(p);
    // comorbidity(p);
    // aspirin_profile(p);
    exp5(p);
    p->Shutdown();
    switch (FLAGS_experiment) {
    case 1: {

      break;
    }
    case 2: {

      break;
    }
    case 3: {

      break;
    }
    case 4: {

      break;
    }
    case 5: {

      break;
    }
    case 6: {

      break;
    }
    case 7: {
      break;
    }
    default: { printf("NOTHING HAPPENS HERE\n"); }
    }

    server->Shutdown();
    delete p;
    hb_thread.join();
  } else {
    DataOwnerPrivate *p =
        new DataOwnerPrivate(FLAGS_address, FLAGS_honest_broker_address);
    p->Register();

    DataOwnerImpl d(p);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(p->HostName(), grpc::InsecureServerCredentials());
    builder.RegisterService(&d);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    auto serveFn = [&]() { server->Wait(); };
    std::thread do_thread(serveFn);
    auto f = exit_requested.get_future();
    f.wait();
    p->FreeAllTables();
    server->Shutdown();
    delete p;
    do_thread.join();
  }
  LOG(INFO) << "closing vaultdb session";
  g3::internal::shutDownLogging();
}

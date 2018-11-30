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
#include "test/experiments/exp3.h"
#include "test/experiments/exp4.h"
#include "test/experiments/exp5.h"
#include "test/experiments/distributed_aspirin_profile.h"
#include <future>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <gflags/gflags.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_builder.h>
#include <sgx/App/VaultDBSGXApp.h>
#include <thread>

DEFINE_bool(honest_broker, false, "Setup as honest broker");
DEFINE_string(address, "", "IPV4 Address for current instance");
DEFINE_string(
    honest_broker_address, "",
    "IPV4 Address for honest broker instance (set by non-honest broker)");
DEFINE_int32(experiment, 1, "experiment number");
DEFINE_string(hl_query, "aspirin", "healthlnk query name");

DEFINE_int32(gen_level, 5, "generalization level");

DEFINE_string(year, "", "year for healthlnk queries");
DEFINE_string(db, "smcql_testDB", "database name");
DEFINE_string(di_table, "diagnoses", "table name for diagnoses");
DEFINE_string(meds_table, "medications", "table name for medications");
DEFINE_string(dem_table, "demographics",
              "table name for demographics");
DEFINE_string(vit_table, "vitals", "table name for vitals");
DEFINE_string(cdiff_cohort_diag_table, "cdiff_cohort_diagnoses",
              "table name for cdiff cohort diagnoses");
DEFINE_string(logger_host_name, "guinea-pig.cs.northwestern.edu:60000",
              "port for logger");
DEFINE_string(host_short, "vaultdb", "short host name");
DEFINE_bool(sgx, false, "Use SGX for queries");

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


void dosage_study(HonestBrokerPrivate *p) {
  auto diag_scan = p->ClusterDBMSQuery(
      "dbname=" + FLAGS_db, "SELECT * from " + FLAGS_di_table);
  auto med_scan = p->ClusterDBMSQuery(
      "dbname=" + FLAGS_db, "SELECT * from " + FLAGS_meds_table);
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
    switch (FLAGS_experiment) {
    case 1: {

      break;
    }
    case 2: {

      break;
    }
    case 3: {
      exp3(p);
      break;
    }
    case 4: {
      exp4(p);

      break;
    }
    case 5: {
      exp5(p, FLAGS_gen_level, FLAGS_sgx);
      break;
    }
    case 6: {

      break;
    }
    case 7: {
      if (FLAGS_hl_query == "aspirin") {
        aspirin_profile(p,FLAGS_db, FLAGS_di_table, FLAGS_vit_table, FLAGS_meds_table, FLAGS_dem_table, FLAGS_year, FLAGS_sgx);
      }

      break;
    }
    default: { printf("NOTHING HAPPENS HERE\n"); }
    }
    p->Shutdown();

    server->Shutdown();
    delete p;
    hb_thread.join();
  } else {
    DataOwnerPrivate *p =
        new DataOwnerPrivate(FLAGS_address, FLAGS_honest_broker_address);
    auto enclave = get_enclave();
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
    //p->FreeAllTables();
    server->Shutdown();
    delete p;
    do_thread.join();
  }
  LOG(INFO) << "closing vaultdb session";
  g3::internal::shutDownLogging();
}

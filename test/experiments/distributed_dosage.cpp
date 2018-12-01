//
// Created by madhav on 12/1/18.
//
#include <VaultDB.h>
#include "distributed_dosage.h"

void dosage_encrypted(HonestBrokerPrivate *p, std::string dbname, std::string diag, std::string meds, std::string year) {
  std::string year_append = "";
  if (year != "") {
    year_append = " where year=" + year;
  }

  auto diag_scan = p->ClusterDBMSQuery("dbname=" + dbname,
                                       "SELECT * from " + diag + year_append);
  auto med_scan = p->ClusterDBMSQuery("dbname=" + dbname,
                                      "SELECT * from " + meds + year_append);
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


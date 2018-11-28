//
// Created by madhav on 11/27/18.
//



#include <rpc/HonestBrokerPrivate.h>

void comorbidity(HonestBrokerPrivate *p, std::string dbname,  int gen_level) {
    auto cdiff_cohort_scan =
            p->ClusterDBMSQuery("dbname=" + dbname ,
                                "SELECT * from cdiff_cohort_diagnoses");
    unordered_map<string, to_gen_t> gen_in;
    to_gen_t tg;
    tg.column = "major_icd9";
    tg.dbname = "vaultdb_";
   gen_in["cdiff_cohort"] = tg;
    tg.scan_tables.insert(tg.scan_tables.end(), cdiff_cohort_scan.begin(), cdiff_cohort_scan.end());
    p->SetControlFlowColName("major_icd9");
    auto gen_out = p->Generalize(gen_in, gen_level);
    auto cdiff_cohort_repart = p->Repartition(cdiff_cohort_scan);

    GroupByDef gbd;
    gbd.set_col_name("major_icd9");
    gbd.set_type(GroupByDef_GroupByType_COUNT);
    auto agg_out = p->Aggregate(cdiff_cohort_repart, gbd, true);

    p->SetControlFlowColName("count");
    auto cnt_repartition = p->Repartition(agg_out);
    SortDef sort;
    sort.set_colname("count");
    sort.set_ascending(false);
    auto sorted = p->Sort(cnt_repartition, sort, true);
}

void exp4(HonestBrokerPrivate *p) {
    comorbidity(p, "vaultdb_", 5);

}
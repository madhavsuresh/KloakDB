#for i in `seq 1 7`;
#do
#	./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true"
#done

#for i in `seq 1 2`;
#do
#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=-1 -di_table=hd_cohort_dist_25_1 -vit_table=vit_ex_asp_1 -meds_table=meds_ex_asp_1 -dem_table=dem_ex_asp_1 -random_run=true"
#1,3,4,7,9,10 work, 2,5,6 do not work

#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=0 -di_table=hd_cohort_dist_25_1 -vit_table=vit_ex_asp_1 -meds_table=meds_ex_asp_1 -dem_table=dem_ex_asp_1 -random_run=true"
#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=0 -di_table=hd_cohort_dist_25_4 -vit_table=vit_ex_asp_4 -meds_table=meds_ex_asp_4 -dem_table=dem_ex_asp_4 -random_run=true"
#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=0 -di_table=hd_cohort_dist_25_7 -vit_table=vit_ex_asp_7 -meds_table=meds_ex_asp_7 -dem_table=dem_ex_asp_7 -random_run=true"
#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=0 -di_table=hd_cohort_dist_25_9 -vit_table=vit_ex_asp_9 -meds_table=meds_ex_asp_9 -dem_table=dem_ex_asp_9 -random_run=true"
#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=0 -di_table=hd_cohort_dist_25_10 -vit_table=vit_ex_asp_10 -meds_table=meds_ex_asp_10 -dem_table=dem_ex_asp_10 -random_run=true"


#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=17 -di_table=hd_cohort_dist_25_1 -vit_table=vit_ex_asp_1 -meds_table=meds_ex_asp_1 -dem_table=dem_ex_asp_1 -random_run=true"
#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=17 -di_table=hd_cohort_dist_25_4 -vit_table=vit_ex_asp_4 -meds_table=meds_ex_asp_4 -dem_table=dem_ex_asp_4 -random_run=true"
#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=17 -di_table=hd_cohort_dist_25_7 -vit_table=vit_ex_asp_7 -meds_table=meds_ex_asp_7 -dem_table=dem_ex_asp_7 -random_run=true"
#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=17 -di_table=hd_cohort_dist_25_9 -vit_table=vit_ex_asp_9 -meds_table=meds_ex_asp_9 -dem_table=dem_ex_asp_9 -random_run=true"
#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=17 -di_table=hd_cohort_dist_25_10 -vit_table=vit_ex_asp_10 -meds_table=meds_ex_asp_10 -dem_table=dem_ex_asp_10 -random_run=true"

./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=-1 -di_table=hd_cohort_dist_25_1 -vit_table=vit_ex_asp_1 -meds_table=meds_ex_asp_1 -dem_table=dem_ex_asp_1 -random_run=true"
./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=-1 -di_table=hd_cohort_dist_25_4 -vit_table=vit_ex_asp_4 -meds_table=meds_ex_asp_4 -dem_table=dem_ex_asp_4 -random_run=true"
./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=-1 -di_table=hd_cohort_dist_25_7 -vit_table=vit_ex_asp_7 -meds_table=meds_ex_asp_7 -dem_table=dem_ex_asp_7 -random_run=true"
./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=-1 -di_table=hd_cohort_dist_25_9 -vit_table=vit_ex_asp_9 -meds_table=meds_ex_asp_9 -dem_table=dem_ex_asp_9 -random_run=true"
./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=-1 -di_table=hd_cohort_dist_25_10 -vit_table=vit_ex_asp_10 -meds_table=meds_ex_asp_10 -dem_table=dem_ex_asp_10 -random_run=true"

#./run_hb.sh "-experiment=7 -hl_query=aspirin -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=false -gen_level=17 -di_table=hd_cohort_dist_25_3 -vit_table=vit_ex_asp_3 -meds_table=meds_ex_asp_3 -dem_table=dem_ex_asp_3 -random_run=true"
#done

#for i in `seq 1 7`;
#do
#	./run_hb.sh "-experiment=7 -expected_num_hosts=4 -hl_query=aspirin -db=healthlnk -year=2008 -gen_level=17 -sgx=true"
#done

#100 PIDS
#for i in `seq 1 7`;
#do
	#./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=0 -di_table=hd_cohort_dist_all_100 -meds_table=meds_ex_dos_100 -num_pids=100"
#done

#for i in `seq 1 7`;
#do
	#./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=-1 -di_table=hd_cohort_dist_all_100 -meds_table=meds_ex_dos_100 -num_pids=100"
#done

#for i in `seq 1 7`;
#do
	#./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=5 -di_table=hd_cohort_dist_all_100 -meds_table=meds_ex_dos_100 -num_pids=100"
#done

#500 PIDS
#for i in `seq 1 7`;
#do
	#./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=0 -di_table=hd_cohort_dist_all_500 -meds_table=meds_ex_dos_500 -num_pids=500"
#done

#for i in `seq 1 7`;
#do
	#./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=-1 -di_table=hd_cohort_dist_all_500 -meds_table=meds_ex_dos_500 -num_pids=500"
#done

for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=5 -di_table=hd_cohort_dist_all_500 -meds_table=meds_ex_dos_filter_500 -num_pids=500"
done

#1000 PIDS
#for i in `seq 1 7`;
#do
	#./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=0 -di_table=hd_cohort_dist_all_1000 -meds_table=meds_ex_dos_1000 -num_pids=1000"
#done

#for i in `seq 1 7`;
#do
	#./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=-1 -di_table=hd_cohort_dist_all_1000 -meds_table=meds_ex_dos_1000 -num_pids=1000"
#done

for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=5 -di_table=hd_cohort_dist_all_1000 -meds_table=meds_ex_dos_filter_1000 -num_pids=1000"
done

#2000 PIDS
#for i in `seq 1 7`;
#do
	#./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=0 -di_table=hd_cohort_dist_all_2000 -meds_table=meds_ex_dos_2000 -num_pids=2000"
#done

#for i in `seq 1 7`;
#do
	#./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=-1 -di_table=hd_cohort_dist_all_2000 -meds_table=meds_ex_dos_2000 -num_pids=2000"
#done

for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=5 -di_table=hd_cohort_dist_all_2000 -meds_table=meds_ex_dos_filter_2000 -num_pids=2000"
done

#3000 PIDS
#for i in `seq 1 7`;
#do
	#./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=0 -di_table=hd_cohort_dist_all_3000 -meds_table=meds_ex_dos_3000 -num_pids=3000"
#done

#for i in `seq 1 7`;
#do
	#./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=-1 -di_table=hd_cohort_dist_all_3000 -meds_table=meds_ex_dos_3000 -num_pids=3000"
#done

for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=5 -di_table=hd_cohort_dist_all_3000 -meds_table=meds_ex_dos_filter_3000 -num_pids=3000"
done

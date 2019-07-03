for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=0"
done

for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=5 -meds_table=meds_ex_filter"
done

for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=7 -hl_query=dos -expected_num_hosts=4 -db=healthlnk -year=2008 -sgx=true -gen_level=-1"
done

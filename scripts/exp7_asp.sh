#for i in `seq 1 2`;
#do
	#./run_hb_no_compile.sh "-experiment=7 -hl_query=dos -expected_num_hosts=3 -db=healthlnk -year=2008 -sgx=true"
#done

#for i in `seq 1 2`;
#do
	#./run_hb_no_compile.sh "-experiment=7 -hl_query=dos -expected_num_hosts=3 -db=healthlnk -year=2008 -sgx=true -gen_level=-1"
#done
#for i in `seq 1 7`;
#do
./run_hb.sh "-experiment=7 -expected_num_hosts=3 -hl_query=aspirin -db=healthlnk -year=2008 -sgx=false -gen_level=0 "

#done

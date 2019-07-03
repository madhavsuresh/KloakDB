#
for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=10 -expected_num_hosts=2 -db=vaultdb_ -sgx=true -gen_level=5"
	./run_hb.sh "-experiment=10 -expected_num_hosts=2 -db=vaultdb_ -sgx=true -gen_level=10"
	./run_hb.sh "-experiment=10 -expected_num_hosts=2 -db=vaultdb_ -sgx=true -gen_level=15"
	./run_hb.sh "-experiment=10 -expected_num_hosts=2 -db=vaultdb_ -sgx=true -gen_level=20"
	./run_hb.sh "-experiment=10 -expected_num_hosts=2 -db=vaultdb_ -sgx=true -gen_level=25"
done


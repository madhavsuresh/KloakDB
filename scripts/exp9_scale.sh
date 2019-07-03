#
for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=8 -expected_num_hosts=4 -db=vaultdb_ -sgx=true -gen_level=5 -gen_test_size=1000 -gen_test_range=1000"
done

for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=8 -expected_num_hosts=4 -db=vaultdb_ -sgx=true -gen_level=5 -gen_test_size=10000 -gen_test_range=1000"
done
#
for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=8 -expected_num_hosts=4 -db=vaultdb_ -sgx=true -gen_level=5 -gen_test_size=100000 -gen_test_range=1000"
done
#
for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=8 -expected_num_hosts=4 -db=vaultdb_ -sgx=true -gen_level=5 -gen_test_size=1000 -gen_test_range=10000"
done
#
for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=8 -expected_num_hosts=4 -db=vaultdb_ -sgx=true -gen_level=5 -gen_test_size=10000 -gen_test_range=10000"
done
#
for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=8 -expected_num_hosts=4 -db=vaultdb_ -sgx=true -gen_level=5 -gen_test_size=100000 -gen_test_range=10000"
done
#
for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=8 -expected_num_hosts=4 -db=vaultdb_ -sgx=true -gen_level=5 -gen_test_size=1000 -gen_test_range=100000"
done
#
for i in `seq 1 7`;
do
	./run_hb.sh "-experiment=8 -expected_num_hosts=4 -db=vaultdb_ -sgx=true -gen_level=5 -gen_test_size=10000 -gen_test_range=100000"
done
#
for i in `seq 1 2`;
do
	./run_hb.sh "-experiment=8 -expected_num_hosts=4 -db=vaultdb_ -sgx=true -gen_level=5 -gen_test_size=100000 -gen_test_range=100000"
done

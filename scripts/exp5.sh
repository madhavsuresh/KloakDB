for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=5 -gen_level=0 -sgx=true -expected_num_hosts=4"
done
for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=5 -gen_level=1 -sgx=true -expected_num_hosts=4"
done
for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=5 -gen_level=5 -sgx=true -expected_num_hosts=4"
done
for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=5 -gen_level=10 -sgx=true -expected_num_hosts=4"
done
for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=5 -gen_level=15 -sgx=true -expected_num_hosts=4"
done
for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=5 -gen_level=20 -sgx=true -expected_num_hosts=4"
done

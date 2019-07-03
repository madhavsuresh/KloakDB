for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=3 -exp3_datasize=1000 -sgx=true -expected_num_hosts=4"
done
echo "Finished 1000, starting 10000"

read 
for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=3 -exp3_datasize=10000 -sgx=true -expected_num_hosts=4"
done
echo "Finished 10000, starting 100000"

read 

for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=3 -exp3_datasize=100000 -sgx=true -expected_num_hosts=4"
done

echo "Finished 100000, starting 1000000"
read 

for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=3 -exp3_datasize=1000000 -sgx=true -expected_num_hosts=4"
done

echo "Finished 1000000, starting 10000000"
read 
for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=3 -exp3_datasize=10000000 -sgx=true -expected_num_hosts=4"
done

read 

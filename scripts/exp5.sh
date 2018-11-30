#for i in `seq 1 5`;
#do
#	./run_hb.sh "-experiment=5 -gen_level=1 -sgx=true"
#done
#for i in `seq 1 5`;
#do
#	./run_hb.sh "-experiment=5 -gen_level=5 -sgx=true"
#done
#for i in `seq 1 5`;
#do
#	./run_hb.sh "-experiment=5 -gen_level=10 -sgx=true"
#done
#
#for i in `seq 1 5`;
#do
#	./run_hb.sh "-experiment=5 -gen_level=15 -sgx=true"
#done
#
#for i in `seq 1 5`;
#do
#	./run_hb.sh "-experiment=5 -gen_level=20 -sgx=true"
#done
#
#for i in `seq 1 5`;
#do
#	./run_hb.sh "-experiment=5 -gen_level=25 -sgx=true"
#done

#for i in `seq 1 2`;
#do
	#./run_hb.sh "-experiment=5 -gen_level=30 -sgx=true"
#done


for i in `seq 1 5`;
do
	./run_hb.sh "-experiment=5 -gen_level=-1 -sgx=true"
done

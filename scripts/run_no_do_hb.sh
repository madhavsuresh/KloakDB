#!/bin/bash
echo $1
killall -9 vaultdb
git pull 
cd build && make -j10 vaultdb
echo "Starting vaultdb"
 LD_LIBRARY_PATH=../lib/lib/ ./vaultdb --expected_num_hosts=4 -address="guinea-pig.cs.northwestern.edu:50000" -honest_broker=true  -host_short='pg' $1&
wait


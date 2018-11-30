#!/bin/bash
echo $1
killall -9 vaultdb
git pull 
cd build && make -j10 vaultdb
echo "Starting vaultdb"
 LD_LIBRARY_PATH=../lib/lib/ ./vaultdb -address="guinea-pig.cs.northwestern.edu:50000" -honest_broker=true  -host_short='pg' $1&
ssh mas384@vaultdb01.research.northwestern.edu 'cd vaultdb_operators && ./run_do.sh' &
ssh mas384@vaultdb02.research.northwestern.edu 'cd vaultdb_operators && ./run_do.sh' &
#ssh mas384@vaultdb03.research.northwestern.edu 'cd vaultdb_operators && ./run_do.sh' &
ssh mas384@vaultdb04.research.northwestern.edu 'cd vaultdb_operators && ./run_do.sh' &
wait


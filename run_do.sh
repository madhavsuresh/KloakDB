#!/bin/bash
killall -9 vaultdb
git pull 
cd build && make -j10 vaultdb
HOSTNAME=`hostname`
PREFIX='v'
H2="${HOSTNAME: -1}"
SHORT="$PREFIX$H2"
echo $SHORT
LD_LIBRARY_PATH=../lib/lib/ ./vaultdb -honest_broker_address="guinea-pig.cs.northwestern.edu:50000" -address="$HOSTNAME.research.northwestern.edu:50000" -host_short=$SHORT

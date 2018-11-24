#!/bin/bash
git pull 
cd build && make -j10
HOSTNAME=`hostname`
echo $HOSTNAME
LD_LIBRARY_PATH=../lib/lib/ ./vaultdb -honest_broker_address="guinea-pig.cs.northwestern.edu:50000" -address="$HOSTNAME.research.northwestern.edu:50000"

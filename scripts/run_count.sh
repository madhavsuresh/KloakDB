#!/bin/bash
echo "VAULT01"
ssh mas384@vaultdb01.research.northwestern.edu 'cd vaultdb_operators/sql && psql healthlnk -f get_count_star.sql' 
echo "VAULT02"
ssh mas384@vaultdb02.research.northwestern.edu 'cd vaultdb_operators/sql && psql healthlnk -f get_count_star.sql' 
echo "VAULT03"
ssh mas384@vaultdb03.research.northwestern.edu 'cd vaultdb_operators/sql && psql healthlnk -f get_count_star.sql' 
echo "VAULT04"
ssh mas384@vaultdb04.research.northwestern.edu 'cd vaultdb_operators/sql && psql healthlnk -f get_count_star.sql' 
wait


dbname='vaultdb_'
dropdb $dbname
createdb $dbname
psql $dbname -f table_defs.sql
psql $dbname  -1 -f create-random-tables.sql

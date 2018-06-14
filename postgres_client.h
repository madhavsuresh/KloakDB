#include "vaultdb_generated.h"
	//OID constants taken from postgres/catalog/pg_type.h, not included in Ubuntu 16.04 postgres package. These are global constants set in postgres
#define VARCHAROID	1043
#define INT4OID		23
#define TIMESTAMPOID	1114

flatbuffers::DetachedBuffer postgres_query_writer(std::string query_string, std::string dbname);

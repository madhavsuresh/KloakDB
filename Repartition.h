#include <cstdint>
#include <utility>
#include "postgres_client.h"
#include "rpc/DataOwnerPrivate.h"

//TODO(madhavsuresh): This can be a list of tableIDs
std::vector<std::pair<int32_t, int32_t>> repart_step_one(table_t * t, int num_hosts, DataOwnerPrivate * p);
int repartition_step_two(std::vector<table_t *> tables, int num_hosts, DataOwnerPrivate *p);

#include "postgres_client.h"
#include "rpc/DataOwnerPrivate.h"
#include <cstdint>
#include <utility>

// TODO(madhavsuresh): This can be a list of tableIDs
std::vector<std::pair<int32_t, int32_t>>
repart_step_one(table_t *t, int num_hosts, DataOwnerPrivate *p);
std::vector<std::pair<int32_t, int32_t>>
repartition_step_two(std::vector<table_t *> tables, int num_hosts,
                     DataOwnerPrivate *p);

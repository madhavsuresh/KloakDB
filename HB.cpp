#include "vaultdb_generated.h"
#include <cstdlib>

struct {
  uint64_t num_hosts;
  uint64_t cf_col;

} dist_query_info;

uint64_t HB_get_num_hosts() { return 0; }

uint64_t HB_get_cf_col() { return 0; }

void HB_increment_hosts() {}

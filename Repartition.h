#include "vaultdb_generated.h"
int repart_step_one(uint8_t * table_buf);
uint32_t hash_to_uint(const Field *f);
flatbuffers::Offset<Field> cp_field(const Field *f,
                                    flatbuffers::FlatbufferBuilder &builder);

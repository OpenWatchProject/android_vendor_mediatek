#include <tz_private/system.h>

void rewrite_param_mem_addr(uint32_t paramTypes, MTEEC_PARAM *params, uint32_t buffer);
void rewrite_inout_param_mem_addr(uint32_t paramTypes, MTEEC_PARAM *params, uint64_t in_buffer, uint64_t out_buffer);
int copy_mem_to_buffer(uint32_t paramTypes, MTEEC_PARAM *params, void *buffer);
int copy_inout_mem_to_buffer(uint32_t paramTypes, MTEEC_PARAM *params, void *buffer, int is_server);
int copy_out_mem_from_buffer(uint32_t paramTypes, MTEEC_PARAM *params, void *buffer);


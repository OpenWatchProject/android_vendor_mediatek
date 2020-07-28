#ifndef __GF_DR_H__
#define __GF_DR_H__

#include "gf_error.h"

extern gf_error_t gf_dr_get_timestamp(uint64_t *timestamp);
extern gf_error_t gf_dr_sleep(uint32_t sleep_miliseconds);

extern gf_error_t gf_dr_sync_auth_result(uint32_t finger_id, uint64_t time);

extern gf_error_t gf_dr_share_memory_performance(uint8_t *buf, uint32_t len, uint64_t *time);

#endif // __GF_DR_H__

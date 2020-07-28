/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_DR_H__
#define __GF_DR_H__

#include "gf_error.h"

gf_error_t gf_dr_get_timestamp(uint64_t *timestamp);
gf_error_t gf_dr_sleep(uint32_t sleep_miliseconds);

gf_error_t gf_dr_sync_auth_result(uint32_t finger_id, uint64_t time);

gf_error_t gf_dr_share_memory_performance(uint8_t *buf, uint32_t len, uint64_t *time);
gf_error_t gf_dr_sync_finger_list(uint32_t *fingerIDs, uint32_t count);
gf_error_t gf_dr_sync_authenticator_version(uint32_t version);

#endif  // __GF_DR_H__

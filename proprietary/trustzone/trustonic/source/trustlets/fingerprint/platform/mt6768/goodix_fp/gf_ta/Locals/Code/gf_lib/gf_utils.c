/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include "gf_tee_internal_api.h"
#include "gf_utils.h"

static uint64_t g_start_time = 0;

uint32_t gf_get_time_delta(void) {
    uint32_t delta = 0;

    uint64_t current_time = 0;
    gf_get_timestamp(&current_time);

    delta = (uint32_t) (current_time - g_start_time);
    g_start_time = current_time;

    return delta;
}

/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <taStd.h>
#include <tee_internal_api.h>
#include "gf_tee_internal_api.h"
#include "gf_dr.h"

#define LOG_TAG "[gf_tee_time]"

void gf_sleep(uint32_t milliseconds) {
    VOID_FUNC_ENTER();
    gf_dr_sleep(milliseconds);
    VOID_FUNC_EXIT();
}

gf_error_t gf_get_timestamp(uint64_t *timestamp) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_dr_get_timestamp(timestamp);
    FUNC_EXIT(err);
    return err;
}

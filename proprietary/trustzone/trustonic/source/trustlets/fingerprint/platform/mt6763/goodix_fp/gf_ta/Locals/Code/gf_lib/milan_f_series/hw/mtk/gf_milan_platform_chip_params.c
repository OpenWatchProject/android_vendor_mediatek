/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "gf_milan_platform_chip_params.h"
#include "gf_log.h"
#include "gf_error.h"
#include "milan_common.h"
#include "milan_regs.h"

#define LOG_TAG "[gf_milan_qualcomm_chip_params]"

gf_error_t gf_milan_platform_init_chip_params(milan_chip_handle_t* handle,
        uint32_t* read_frame_num_for_nav_in_continue) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    UNUSED_VAR(handle);
    UNUSED_VAR(read_frame_num_for_nav_in_continue);

    FUNC_EXIT(err);
    return err;
}


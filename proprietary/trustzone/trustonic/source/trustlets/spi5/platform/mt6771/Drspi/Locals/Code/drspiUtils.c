/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include "drStd.h"

#include "drUtils.h"
#include "drCommon.h"
#include "DrApi/DrApi.h"


/**
 * Exchange registers of current thread or another thread.
 *
 * @param  threadNo   Thread no
 * @param  ip         ip value to be set
 * @param  sp         sp value to be set
 *
 * @retval DRAPI_OK or relevant error code.
 */
drApiResult_t drUtilsRestartThread(
    threadno_t threadNo,
    addr_t ip,
    addr_t sp )
{
    drApiResult_t ret  = E_INVALID;
    uint32_t      ctrl = THREAD_EX_REGS_IP | THREAD_EX_REGS_SP;

    /* Set ip and sp registers */
    ret = drApiThreadExRegs(threadNo,
                            ctrl,
                            ip,
                            sp);
    if (ret != DRAPI_OK)
    {
        return ret;
    }

    /* Resume thread */
    ret = drApiResumeThread(threadNo);

    return ret;
}

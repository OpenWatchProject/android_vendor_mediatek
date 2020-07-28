/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


/*
 * @file   rpmb_ipch.c
 * @brief  Implements IPC handler of the driver.
 *
 */
#include <drv_fwk.h>
#include <drv_fwk_macro.h>
#include <drv_error.h>
#include <drv_defs.h>

#include "drrpmb_Api.h"
#include "drRpmbOps.h"

extern dciMessage_t *message;

int rpmb_open_session(unsigned int sid, unsigned long args)
{
    int ret = DRV_FWK_API_OK;
    tlApiRpmb_ptr    pRpmbData = NULL;

    msee_logd("rpmb_open_session: sid %d\n", sid);
    return DRV_FWK_API_OK;
}

int rpmb_close_session(unsigned int sid)
{
    int ret = DRV_FWK_API_OK;
    tlApiRpmb_ptr    pRpmbData = NULL;

    msee_logd("rpmb_close_session: sid %d\n", sid);
    return DRV_FWK_API_OK;
}

int rpmb_invoke_command(unsigned int sid, unsigned int cmd, unsigned long args)
{
    int ret = DRV_FWK_API_OK;
    tlApiRpmb_ptr    pRpmbData = NULL;

    msee_logd("rpmb_invoke_command: sid %d, cmd %d\n", sid, cmd);
    MSEE_PROFILE_START;

    message = (dciMessage_t *) msee_get_ree_shared_msg();
    if (message == NULL) {
        msee_logd("[rpmb t-driver] FID_DR_EXECUTE: get ree shared msg failed\n");
        return DRV_FWK_API_MAP_TASK_BUFFER_FAILED;
    }
    ret = msee_map_user(&pRpmbData, args, sizeof(tlApiRpmb_t), MSEE_MAP_USER_DEFAULT);

    if (ret != DRV_FWK_API_OK) {
        msee_logd("%s map task buffer failed", __func__);
        return DRV_FWK_API_MAP_TASK_BUFFER_FAILED;
    }

    switch (cmd) {
    case FID_DRV_OPEN_SESSION:
        drRpmbOpenSession(pRpmbData);
        break;
    case FID_DRV_CLOSE_SESSION:
        drRpmbCloseSession(pRpmbData);
        break;
    case FID_DRV_GET_KEY:
        drRpmbGetKey(pRpmbData);
        break;
    case FID_DRV_GET_REL_WR_SEC_C:
        drRpmbGetRelWrSecC(pRpmbData);
        break;
    case FID_DRV_GET_PART_SIZE:
        drRpmbGetPartSize(pRpmbData);
        break;
    case FID_DRV_GET_PART_ADDR:
        drRpmbGetPartAddr(pRpmbData);
        break;
    case FID_DRV_READ_DATA:
        drRpmbReadData(pRpmbData);
        break;
    case FID_DRV_GET_WCOUNTER:
        drRpmbGetWcounter(pRpmbData);
        break;
    case FID_DRV_WRITE_DATA:
        drRpmbWriteData(pRpmbData);
        break;
#if (CFG_RPMB_KEY_PROGRAMED_IN_KERNEL == 1)
    case FID_DRV_PROGRAM_KEY:
    	drRpmbProgramKey(pRpmbData);
        break;
    case FID_DRV_CHECK_FLAG:
    	drRpmbProgramKeyCheckFlag(pRpmbData);
        break;
    case FID_DRV_SET_KEY_FLAG:
    	drRpmbProgramKeySetFlag(pRpmbData);
        break;
#endif
    default:
        msee_logd("command is not supported %d", cmd);
        break;
    }
    msee_unmap_user(&pRpmbData);
    MSEE_PROFILE_END;

    return ret;
}

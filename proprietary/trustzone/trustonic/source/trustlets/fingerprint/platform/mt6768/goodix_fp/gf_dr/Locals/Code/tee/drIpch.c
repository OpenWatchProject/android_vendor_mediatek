/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include "drStd.h"
#include "DrApi/DrApi.h"

#include "drCommon.h"

#include "gf_dr_common.h"
#include "gf_dr_ipch.h"

DECLARE_STACK(drIpchStack, 2048);

#define LOG_TAG "[drIpch] "

#if TBASE_API_LEVEL >= 5
#define drUtilsUnmapTaskBuffers(clientTaskId)       drApiUnmapTaskBuffers((clientTaskId))
#else
#define drUtilsUnmapTaskBuffers(clientTaskId)
#endif

void *drUtilsMapTaskBuffer(const taskid_t taskId, const void *startVirtClient, const size_t len,
        const uint32_t attr) {

    void *pBuf = NULL;

#if TBASE_API_LEVEL >= 5
    {
        drApiResult_t drRet = DRAPI_OK;

        if (DRAPI_OK
                != (drRet = drApiMapTaskBuffer(taskId, startVirtClient, len, attr, (void **) &pBuf))) {
            GF_LOGE(LOG_TAG ":drUtilsMapTaskBuffer error:0x%X\n", drRet);
        }
    }
#elif TBASE_API_LEVEL >= 3
    pBuf = drApiAddrTranslateAndCheck(pMarshal->payload.pduBuf.pBuf);
#endif

    return pBuf;
}

/**
 * IPC handler loop. this is the function where IPC messages are handled
 */_NORETURN void drIpchLoop(void) {
    /* Set IPC parameters for initial MSG_RD to IPCH */
    threadid_t ipcClient = NILTHREAD;
    message_t ipcMsg = MSG_RD;
    uint32_t ipcData = 0;
    uint32_t ipcMsgId = 0;
    drApiResult_t drRet = TLAPI_OK;

    for (;;) {

        if (DRAPI_OK != drApiIpcCallToIPCH(&ipcClient, &ipcMsg, &ipcData)) {
            GF_LOGE("drIpchLoop(): drApiIpcCallToIPCH failed\n");
            continue;
        }

        /* Dispatch request */
        switch (ipcMsg) {
            case MSG_CLOSE_TRUSTLET: {
                GF_LOGD("drIpchLoop(): Acknowledging trustlet close\n");
                ipcMsg = MSG_CLOSE_TRUSTLET_ACK;
                ipcData = TLAPI_OK;
                break;
            }

            case MSG_CLOSE_DRIVER: {
                /* Acknowledge */
                GF_LOGD("[IPC] MSG_CLOSE_DRIVER\n");
                ipcMsg = MSG_CLOSE_DRIVER_ACK;
                ipcData = TLAPI_OK;
                break;
            }

            case MSG_GET_DRIVER_VERSION: {
                GF_LOGD("[IPC] MSG_GET_DRIVER_VERSION\n");
                ipcMsg = (message_t) TLAPI_OK;
                ipcData = DRIVER_VERSION;
                break;
            }

            case MSG_RQ: {
                /* init tlRet value */
                drRet = TLAPI_OK;
                /**
                 * Handle incoming IPC requests via TL API.
                 * Map the caller trustlet to access to the marshaling data
                 */
                gf_dr_cmd_t *cmd = (gf_dr_cmd_t *) drUtilsMapTaskBuffer(
                        THREADID_TO_TASKID(ipcClient), (addr_t) ipcData, sizeof(gf_dr_cmd_t),
                        MAP_READABLE | MAP_WRITABLE);

                //GF_LOGD("[%s] GF_DR version: GF_DR_v%d.%d\n",__func__, DRIVER_VERSION_MAJOR, DRIVER_VERSION_MINOR);
                drRet = gf_dr_invoke_cmd_entry_point(cmd, ipcClient);

                /* Update response data */
                ipcMsg = MSG_RS;
                ipcData = drRet;
                drUtilsUnmapTaskBuffers(THREADID_TO_TASKID(ipcClient));
                break;
            }

            default: {
                /* Unknown message has been received*/
                ipcMsg = MSG_RS;
                ipcData = E_TLAPI_DRV_UNKNOWN;
                break;
            }
        }
    }
}

/**
 * IPC handler thread entry point
 */_THREAD_ENTRY void drIpch(void) {
    drIpchLoop();
}

/**
 * IPC handler thread init
 */
void drIpchInit(void) {
    /* ensure thread stack is clean */clearStack(drIpchStack);

    /**
     * Start IPC handler thread. Exception handler thread becomes local
     * exception handler of IPC handler thread
     */
    if (DRAPI_OK
            != drApiStartThread(DRIVER_THREAD_NO_IPCH, FUNC_PTR(drIpch), getStackTop(drIpchStack),
                    IPCH_PRIORITY, DRIVER_THREAD_NO_EXCH)) {
        GF_LOGE("[Driver DrGoodixUtils] drIpchInit(): drApiStartThread failed");
    }
}

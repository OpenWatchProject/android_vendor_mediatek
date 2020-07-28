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

/* IPC handler stack */
EXTERNAL_STACK(drIpchStack)

/* External functions */
extern _NORETURN void drIpchLoop(void);

/**
 * Exchange registers of current thread or another thread.
 *
 * @param  threadNo   Thread no
 * @param  ip         ip value to be set
 * @param  sp         sp value to be set
 *
 * @retval DRAPI_OK or relevant error code.
 */
drApiResult_t drRestartThread(threadno_t threadNo, addr_t ip, addr_t sp) {
    drApiResult_t ret = E_INVALID;
    uint32_t ctrl = THREAD_EX_REGS_IP | THREAD_EX_REGS_SP;

    /* Set ip and sp registers */
    ret = drApiThreadExRegs(threadNo, ctrl, ip, sp);
    if (ret != DRAPI_OK) {
        return ret;
    }

    /* Resume thread */
    ret = drApiResumeThread(threadNo);

    return ret;
}

/**
 * Cleanup function
 */
static void doCleanup(void) {
}

/**
 * Exception handler loop.
 */
_NORETURN void drExchLoop(void) {
    threadno_t faultedThread;
    threadid_t ipcPartner;
    uint32_t mr0, mr1, mr2;
    addr_t ip;
    addr_t sp;

    drDbgPrintLnf("[Driver DrGoodixUtils] drExchLoop(): Exception handler thread is running");

    for (;;) {
        ipcPartner = 0;
        mr0 = 0;
        mr1 = 0;
        mr2 = 0;

        /* Wait for exception */
        if (DRAPI_OK != drApiIpcWaitForMessage(&ipcPartner, &mr0, &mr1, &mr2)) {
            /* Unable to receive IPC message */
            drDbgPrintLnf("[Driver DrGoodixUtils] drExchLoop(): drApiIpcWaitForMessage failed");
            continue;
        }

        /* mr0 holds threadid value of thread that caused the exception */
        faultedThread = GET_THREADNO(mr0);

        /* Process exception */
        switch (mr1) {
            //--------------------------------------
            case TRAP_SEGMENTATION:
                /* Check which thread caused exception */
                switch (faultedThread) {
                    //--------------------------------------
                    case DRIVER_THREAD_NO_IPCH:
                        /* Update sp and ip accordingly */
                        ip = FUNC_PTR(drIpchLoop);
                        sp = getStackTop(drIpchStack);

                        /* Resume thread execution */
                        if (DRAPI_OK != drRestartThread(faultedThread, ip, sp)) {
                            drDbgPrintLnf(
                                    "[Driver DrGoodixUtils] drExchLoop(): restarting thread failed");  // NOLINT
                        }

                        break;
                        //--------------------------------------
                    default:
                        drDbgPrintLnf(
                                "[Driver DrGoodixUtils] drExchLoop(): Unknown thread. This should never happen");  // NOLINT
                        break;
                }

                break;
                //--------------------------------------
            case TRAP_ALIGNMENT:
            case TRAP_UNDEF_INSTR:
                /**
                 * This should never happen. If it does, do the cleanup and exit gracefully
                 */
                doCleanup();

                /* Stop IPC handler thread */
                if (DRAPI_OK != drApiStopThread(DRIVER_THREAD_NO_IPCH)) {
                    drDbgPrintLnf(
                            "[Driver DrGoodixUtils] drExchLoop(): Unable to stop IPC handler thread");  // NOLINT
                }

                /* Stop main thread */
                if (DRAPI_OK != drApiStopThread(NILTHREAD)) {
                    drDbgPrintLnf(
                            "[Driver DrGoodixUtils] drExchLoop(): Unable to stop main thread");
                }

                /* Will not come to this point */
                break;
                //--------------------------------------
            default:
                /**
                 * Unknown exception occurred. Do cleanup in case.
                 */
                doCleanup();
                break;
        }
    }
}

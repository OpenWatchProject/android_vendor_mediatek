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

/*
 * @file   testMain.c
 * @brief  Implements the entry point of the driver.
 *
 */

#include "drStd.h"
#include "DrApi/DrApi.h"

#include "drTuiCommon.h"
#include "drTuiUtils.h"

#include "drTestMarshal.h"
#include "DrError.h"
#include "dciTest.h"
#include "testDrSmgmt.h"

#define TAG "[testMain]"
#include "log.h"

#define DRIVER_THREAD_NO_DCIH   4


/* Allocate driver stack */
DECLARE_DRIVER_MAIN_STACK(2048);
DECLARE_STACK(drDcihStack, 2048);
DECLARE_STACK(drIpchStack, 2048);

/* Driver version (used by the IPC handler) */
#define DRIVER_VERSION       1

/* Static variables */
static dciMessage_t *message;

/*
 * Initialization code
 */
static drApiResult_t InitDriver(void)
{
    // Nothing to do here now
    drApiResult_t ret = DRAPI_OK;

    /*
     * TODO: This is the function where certain initialization
     * can be done before proceeding further. Such as HW related
     * initialization. Update the return code accordingly
     */

    return ret;
}

/**
 * DCI handler loop. this is the function where notifications from Nwd are handled
 */
_NORETURN void drDcihLoop( void )
{
    dciCommandId_t  commandId;
    dciReturnCode_t ret;
    taskid_t        taskid = drApiGetTaskid();

    printf("drDcihLoop(): DCI handler thread is running");

    /* Initial notification */
    printf("Sending initial notification from driver");
    ret = drApiNotify();
    printf("Sent initial notification from driver ret=%x", ret);

    for(;;)
    {
        /* Initialize return value */
        ret = RET_OK;

        printf("drDcihLoop(): start to wait the signal from normal world");

        /* Wait for IPC signal */
        if (DRAPI_OK != drApiIpcSigWait())
        {
            printf("drDcihLoop(): drApiIpcSigWait failed");
            continue;
        }

        /* Get commandid */
        commandId = message->command.header.commandId;

        printf("drDcihLoop(): commandId(%d)", commandId);

        /* Get & process DCI command */
        switch( commandId )
        {
            case CMD_ID_01:
                // Test i2c
                test_i2c();
                break;
            case CMD_ID_02:
                // Test touch
                test_touch();
                break;
            case CMD_ID_03:
                // Test GPIO
                test_gpio();
                break;
            case CMD_ID_04:
                // Test Display
                test_display();
                break;
            case CMD_ID_05:
                // Test LED
                test_led();
                break;
            default:
                break;
        }

        message->response.header.responseId = RSP_ID(commandId);
        message->response.header.returnCode = ret;

        /* Notify Nwd */
        if (DRAPI_OK != drApiNotify())
        {
            printf("drDcihLoop(): drApiNotify failed");
        }
        printf("drDcihLoop(): drApiNotify success");
    }
}

/**
 * DCI handler thread entry point
 */
_THREAD_ENTRY void drDcih( void )
{
    drDcihLoop();
}

/**
 * DCI handler init
 */
void drDcihInit(
    const addr_t    dciBuffer,
    const uint32_t  dciBufferLen
){
    /* Ensure thread stack is clean */
    clearStack( drDcihStack );

    /* DCI message buffer */
    message = (dciMessage_t*) dciBuffer;

    /**
     * Update DCI handler thread so that notifications will be delivered
     * to DCI handler thread
     */
    if (DRAPI_OK != drUtilsUpdateNotificationThread( DRIVER_THREAD_NO_DCIH ))
    {
        printf("drDcihInit(): Updating notification thread failed");
    }

    /**
     * Start DCI handler thread. EXH thread becomes local
     * exception handler of DCIH thread
     */
    if (DRAPI_OK != drApiStartThread(
                    DRIVER_THREAD_NO_DCIH,
                    FUNC_PTR(drDcih),
                    getStackTop(drDcihStack),
                    DCIH_PRIORITY,
                    DRIVER_THREAD_NO_EXCH))

    {
            printf("drDcihInit(): drApiStartThread failed");
    }
}

/*
 * IPC handler loop. this is the function where IPC messages are handled
 */
_NORETURN void drIpchLoop( void )
{
    /* Set IPC parameters for initial MSG_RD to IPCH */
    threadid_t              ipcClient = NILTHREAD;
    message_t               ipcMsg    = MSG_RD;
    uint32_t                ipcData   = 0;
    tlApiResult_t           tlRet     = TLAPI_OK;
    drMarshalingParam_ptr   pMarshal;
    taskid_t                taskid = drApiGetTaskid();
    uint8_t                 *text = NULL;
    uint32_t                x, y, result;
    tlApiTest_ptr            TestData = NULL;

    for(;;)
    {
        /*
         * When called first time sends ready message to IPC server and
         * then waits for IPC requests
         */
        if (DRAPI_OK != drApiIpcCallToIPCH(&ipcClient, &ipcMsg, &ipcData))
        {
            printf("drIpchLoop(): drApiIpcCallToIPCH failed");
            continue;
        }

        /* Dispatch request */
        switch(ipcMsg)
        {
            case MSG_CLOSE_TRUSTLET:
                /*
                 * Trustlet close message
                 */
                printf("drIpchLoop(): Acknowledging trustlet close");
                /* Close active sessions owned by trustlet, which is being shutdown */
                //For MSG_CLOSE_TRUSTLET message threadId is in ipcData parameter
                drSmgmtCloseSessionForThread(ipcData);
                ipcMsg = MSG_CLOSE_TRUSTLET_ACK;
                ipcData = TLAPI_OK;
                break;
            case MSG_CLOSE_DRIVER:
                /*
                 * Driver close message
                 */
                ipcMsg = MSG_CLOSE_DRIVER_ACK;
                ipcData = TLAPI_OK;
                break;
            case MSG_RQ:
#ifdef DR_FEATURE_TL_API
                /*
                 * Handle incoming IPC requests via TL API.
                 * Map the caller trustlet to access to the marshalling data
                 */
                pMarshal = (drMarshalingParam_ptr)drApiMapClientAndParams(
                                            ipcClient,
                                            ipcData);

                if(pMarshal)
                {
                    /* Process the request */
                    switch (pMarshal->functionId)
                    {

                    case FID_DR_OPEN_SESSION:
                        /*
                         * Handle open session request
                         */

                        pMarshal->sid = drSmgmtOpenSession(ipcClient);
                        /*
                         * Update IPC status as success. If there is an error with opening session
                         * invalid sid will indicatethat opening session failed
                         */
                        tlRet = TLAPI_OK;
                        break;
                    case FID_DR_CLOSE_SESSION:
                        /*
                         * Handle close session request based on the sid provided by the client
                         */
                        drSmgmtCloseSession(pMarshal->sid);
                        tlRet = TLAPI_OK;
                        break;
                    case FID_DR_INIT_DATA:
                        /*
                         * Initialize sesion data
                         */
                        if (E_DR_SMGMT_OK == drSmgmtSetSessionData(
                                pMarshal->sid,
                                ipcClient,
                                &(pMarshal->payload.TestData)))
                        {
                            tlRet = TLAPI_OK;
                        }
                        break;
                    case FID_DR_EXECUTE:
                        /*
                         * TODO: Read registry data compare threadids to make sure that client is allowed to use
                         * registry data. Then execute the command and update tlRet accordingly
                         */

                        drApiNotify();

                        TestData = drApiAddrTranslateAndCheck(pMarshal->payload.TestData);

                        if(FID_DRV_ADD_FOO == TestData->commandId)
                        {
                            printf("FID_DRV_ADD_FOO: x(%d), y(%d)", TestData->x, TestData->y);
                            HAL_FooAdd(TestData);
                        }

                        if(FID_DRV_SUB_FOO == TestData->commandId)
                        {
                            printf("FID_DRV_SUB_FOO: x(%d), y(%d)", TestData->x, TestData->y);
                            HAL_FooSub(TestData);
                        }

                        break;
                    default:
                        /* Unknown message has been received*/
                        break;
                    }
                }

                /* Update response data */
                ipcMsg  = MSG_RS;
                ipcData = 0;
                pMarshal->payload.retVal = tlRet;
#endif
                break;
            default:
                drApiIpcUnknownMessage(&ipcClient, &ipcMsg, &ipcData);
                /* Unknown message has been received*/
                ipcMsg = (message_t) E_TLAPI_DRV_UNKNOWN;
                ipcData = 0;
                break;
        }
    }
}

/*
 * IPC handler thread entry point
 */
_THREAD_ENTRY void drIpch( void )
{
    drIpchLoop();
}

/**
 * IPC handler thread init
 */
void drIpchInit(
    const addr_t    dciBuffer,
    const uint32_t  dciBufferLen
){
    /* ensure thread stack is clean */
    clearStack(drIpchStack);

    /* DCI message buffer */
    message = (dciMessage_t*) dciBuffer;

    /*
     * DCI buffer data can be accessed via message
     * pointer.
     */

    /*
     * Start IPC handler thread. Exception handler thread becomes local
     * exception handler of IPC handler thread
     */
    if (DRAPI_OK != drApiStartThread(
                    DRIVER_THREAD_NO_IPCH,
                    FUNC_PTR(drIpch),
                    getStackTop(drIpchStack),
                    IPCH_PRIORITY,
                    DRIVER_THREAD_NO_EXCH))

    {
        printf("drIpchInit(): drApiStartThread failed");
    }
}

/*
 * Cleanup function
 */
static void doCleanup( void )
{
    /* Close all sessions */
    drSmgmtCloseAllSessions();

    /*
     * TODO: Add cleanup code here
     *
     */
}

/**
 * Exception handler loop.
 */
_NORETURN void drExchLoop(
    const addr_t    dciBuffer,
    const uint32_t  dciBufferLen
){
    threadno_t      faultedThread;
    threadid_t      ipcPartner;
    uint32_t        mr0, mr1, mr2;
    addr_t          ip = NULL;
    addr_t          sp = NULL;

    printf("drExchLoop(): Exception handler thread is running");

    for(;;)
    {
        /* Wait for exception */
        if ( DRAPI_OK != drApiIpcWaitForMessage(
                     &ipcPartner,
                     &mr0,
                     &mr1,
                     &mr2) )
        {
            /* Unable to receive IPC message */
            printf("drExchLoop(): drApiIpcWaitForMessage failed");
            continue;
        }

        /* mr0 holds threadid value of thread that caused the exception */
        faultedThread = GET_THREADNO(mr0);

        /* Process exception */
        switch(mr1)
        {
            //--------------------------------------
            case TRAP_SEGMENTATION:
                /* Check which thread caused exception */
                switch(faultedThread)
                {
                    //--------------------------------------
                    case DRIVER_THREAD_NO_IPCH:
                        /* Update sp and ip accordingly */
                        ip = FUNC_PTR(drIpchLoop);
                        sp = getStackTop(drIpchStack);

                        /* Resume thread execution */
                        if (DRAPI_OK != drUtilsRestartThread(
                                        faultedThread,
                                        ip,
                                        sp))
                        {
                            printf("drExchLoop(): restarting thread failed");
                        }

                        break;
                    //--------------------------------------
                    case DRIVER_THREAD_NO_DCIH:
                        ip = FUNC_PTR(drDcihLoop);
                        sp = getStackTop(drDcihStack);
                        break;
                    //--------------------------------------
                    default:
                        printf("drExchLoop(): Unknown thread. This should never happen");
                        break;
                }

                if ((ip != NULL) && (sp != NULL))
                {
                    /* Set sp and ip accordingly and resume execution if DCIH thread */
                    if (DRAPI_OK != drUtilsRestartThread(
                                    faultedThread,
                                    ip,
                                    sp))
                    {
                        printf("drExchLoop(): restarting thread failed");
                    }
                }

                break;
            //--------------------------------------
            case TRAP_ALIGNMENT:
            case TRAP_UNDEF_INSTR:
                /*
                 * This should never happen. If it does, do the cleanup and exit gracefully
                 */
                doCleanup();

                /* Stop DCI handler thread */
                if (DRAPI_OK != drApiStopThread(DRIVER_THREAD_NO_DCIH))
                {
                    printf("drExchLoop(): Unable to stop DCI handler thread");
                }

                /* Stop IPC handler thread */
                if (DRAPI_OK != drApiStopThread(DRIVER_THREAD_NO_IPCH))
                {
                    printf("drExchLoop(): Unable to stop IPC handler thread");
                }

                /* Stop main thread */
                if (DRAPI_OK != drApiStopThread(NILTHREAD))
                {
                    printf("drExchLoop(): Unable to stop main thread");
                }

                /* Will not come to this point */
                break;
            //--------------------------------------
            default:
                   /* TODO: Update this sestion accordingly
                    *
                    * Unknown exception occured.
                    *  Do cleanup in case
                 */
                doCleanup();
                break;
        }
    }
}


void __attribute((weak)) test_display(void)
{
	return 0;
}

/*
 * Main routine for the example driver.
 * Initializes the Api data structures and starts the required threads.
 */
_DRAPI_ENTRY void drMain(
    const addr_t    dciBuffer,
    const uint32_t  dciBufferLen)
{
    drApiLogPrintf("[<t Driver tuitest] %d.%d, Build " __DATE__ ", " __TIME__ EOL,
                        DRIVER_VERSION_MAJOR, DRIVER_VERSION_MINOR);

    /* Initialization */
    if (DRAPI_OK != InitDriver())
    {
        printf("drMain(): Initialization failed");

        /* No need to proceed further. Shutdown the main thread */
        if (DRAPI_OK != drApiStopThread(NILTHREAD))
        {
            printf("drMain(): Unable to stop main thread");
        }
    }

    printf("dciBuffer=%p dciBufferLen=%d", dciBuffer, dciBufferLen);

    /* Start IPC handler */
    drIpchInit(dciBuffer, dciBufferLen);

    /* Start DCI handler */
    drDcihInit(dciBuffer, dciBufferLen);

    /* Start exception handler */
    drExchLoop(dciBuffer, dciBufferLen);
}

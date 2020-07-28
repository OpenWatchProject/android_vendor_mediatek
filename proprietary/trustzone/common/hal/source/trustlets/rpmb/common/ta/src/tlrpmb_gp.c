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
 * @file   tlrpmb.c
 * @brief  Trustlet implementation
 *
 * TCI data is processed and handled according to the command id and data
 * given by the associated TLC. Normal world is notified accordingly with
 * relevant return code after processing TCI command data
 *
 */


#include "taStd.h"

#include "tlrpmb_Api.h"
#include "tlRpmbDriverApi.h"


// Reserve 2048 byte for stack.
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16*1024);

#define TATAG "TA RPMB "

struct session {
    uint32_t calls; 	//count number of calls in this session
    uint32_t nEncrypted; 	//count number of encrypted data in this session
};

void TA_EXPORT TA_CloseSessionEntryPoint(IN OUT void* pSessionContext)
{
    TEE_DbgPrintLnf(TATAG "TA_CloseSessionEntryPoint");

    struct session *clientSession = (struct session *)pSessionContext;

    TEE_Free(clientSession);
}

void TA_EXPORT TA_DestroyEntryPoint(void)
{
    TEE_DbgPrintLnf(TATAG "TA_DestroyEntryPoint");
}


TEE_Result TA_EXPORT TA_CreateEntryPoint(void)
{
    TEE_DbgPrintLnf(TATAG "TA_CreateEntry Point");


    return TEE_SUCCESS;
}

TEE_Result TA_EXPORT TA_InvokeCommandEntryPoint(
    void* pSessionContext,
    uint32_t nCommandID,
    uint32_t nParamTypes,
    TEE_Param pParams[4])
{
    TEE_Result ret;
    uint8_t* pInput;
    uint32_t nInputSize;
    uint8_t* pOutput;
    uint32_t nOutputSize;
    uint32_t *offset;
    uint32_t *userID;
    struct session *clientSession = (struct session *)pSessionContext;
    uint32_t crSession;
    int result;

    TEE_DbgPrintLnf(TATAG "TA_InvokeCommandEntryPoint");
    clientSession->calls++;

#if (CFG_RPMB_KEY_PROGRAMED_IN_KERNEL == 1)
    if (nCommandID == CMD_RPMB_PROGRAM_KEY) {
        userID = (uint32_t *)pParams[2].memref.buffer;
    } else {
#endif
        pInput = pParams[0].memref.buffer;
        nInputSize = pParams[0].memref.size;
        offset = (uint32_t *)pParams[1].memref.buffer;
        userID = (uint32_t *)pParams[2].memref.buffer;

        TEE_DbgPrintLnf(TATAG "offset=%d, bufSize=%d\n", *offset, pParams[0].memref.size);

        if ((pInput == NULL) || (nInputSize == 0))
            return TEE_ERROR_BAD_PARAMETERS;

        switch (nCommandID) {
        /* Needs to write buffer to user (OUTPUT type) */
        case CMD_RPMB_READ_DATA:
        case CMD_RPMB_READ_BY_OFFSET:
            ret = TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_WRITE |
                                              TEE_MEMORY_ACCESS_ANY_OWNER,
                                              pParams[0].memref.buffer,
                                              pParams[0].memref.size);
            break;

        /* Needs to read buffer from user (INPUT type) */
        case CMD_RPMB_WRITE_DATA:
        case CMD_RPMB_WRITE_BY_OFFSET:
        default:
            ret = TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_READ |
                                              TEE_MEMORY_ACCESS_ANY_OWNER,
                                              pParams[0].memref.buffer,
                                              pParams[0].memref.size);
            break;
        }

        if (ret != TEE_SUCCESS) {
            TEE_DbgPrintLnf(TATAG "wrong input access rights!");
            return ret;
        }

        TEE_DbgPrintLnf(TATAG "user id = %d\n", *userID);

#if (CFG_RPMB_KEY_PROGRAMED_IN_KERNEL == 1)
    }
#endif

    TEE_DbgPrintLnf(TATAG "user id === %d\n", *userID);
    crSession = TEE_RpmbOpenSession(*userID);
    if (crSession == 0xFFFFFFFF)
        return TEE_ERROR_GENERIC;

    switch (nCommandID) {
    case CMD_RPMB_READ_DATA:
        TEE_DbgPrintLnf(TATAG "CMD_RPMB_READ_DATA");
        ret = TEE_RpmbReadData(crSession, pParams[0].memref.buffer, pParams[0].memref.size, &result);
        break;
    case CMD_RPMB_WRITE_DATA:
        TEE_DbgPrintLnf(TATAG "CMD_RPMB_WRITE_DATA");
        ret = TEE_RpmbWriteData(crSession, pParams[0].memref.buffer, pParams[0].memref.size, &result);
        break;
    case CMD_RPMB_READ_BY_OFFSET:
        TEE_DbgPrintLnf(TATAG "CMD_RPMB_READ_BY_OFFSET");
        ret = TEE_RpmbReadDatabyOffset(crSession, *offset, pParams[0].memref.buffer, pParams[0].memref.size, &result);
        if (ret)
            TEE_DbgPrintLnf(TATAG "TEE_RpmbReadDatabyOffset error!!!\n");
        break;
    case CMD_RPMB_WRITE_BY_OFFSET:
        TEE_DbgPrintLnf(TATAG "CMD_RPMB_WRITE_BY_OFFSET");
        ret = TEE_RpmbWriteDatabyOffset(crSession, *offset, pParams[0].memref.buffer, pParams[0].memref.size, &result);
        if (ret)
            TEE_DbgPrintLnf(TATAG "TEE_RpmbWriteDatabyOffset error!!!\n");
        break;
#if (CFG_RPMB_KEY_PROGRAMED_IN_KERNEL == 1)
    case CMD_RPMB_PROGRAM_KEY:
        TEE_DbgPrintLnf(TATAG "CMD_RPMB_PROGRAM_KEY");
        ret = TEE_RpmbProgramKey(crSession, &result);
        if (ret)
            TEE_DbgPrintLnf(TATAG "TEE_RpmbProgramKey error!!!\n");
        pParams[3].value.a = result;

        break;
#endif

    default:
        TEE_DbgPrintLnf(TATAG "invalid command ID: 0x%X", nCommandID);
        return TEE_ERROR_BAD_PARAMETERS;
    }
    TEE_RpmbCloseSession(crSession);
    return ret;
}

TEE_Result TA_EXPORT TA_OpenSessionEntryPoint(
    uint32_t nParamTypes,
    TEE_Param pParams[4],
    void** ppSessionContext)
{
    S_VAR_NOT_USED(nParamTypes);
    S_VAR_NOT_USED(pParams);
    S_VAR_NOT_USED(ppSessionContext);


    TEE_DbgPrintLnf(TATAG "TA_OpenSessionEntryPoint");
    TEE_LogPrintf(TATAG "TA_OpenSessionEntryPoint\n");

    struct session *clientSession = (struct session *)TEE_Malloc(sizeof(*clientSession), 0);
    if (clientSession == NULL) {
        TEE_DbgPrintLnf(TATAG "out of memory!");
        return TEE_ERROR_OUT_OF_MEMORY;
    }

    TEE_DbgPrintLnf(TATAG "allocated session at %p", clientSession);

    if (TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_READ |
                                    TEE_MEMORY_ACCESS_WRITE,
                                    clientSession, sizeof(*clientSession)) != TEE_SUCCESS) {
        TEE_DbgPrintLnf(TATAG "wrong access rights!");
        TEE_Free(clientSession);
        return TEE_ERROR_ACCESS_CONFLICT;
    }

    clientSession->calls = 0;
    clientSession->nEncrypted = 0;

    *ppSessionContext = clientSession;

    return TEE_SUCCESS;
}




/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "mcVersionHelper.h"
#include "MobiCoreDriverApi.h"
#include "TlCm/3.0/tlCmApi.h"
#include "TlCm/tlCmUuid.h"

#include "tBaseInfo.h"

mcResult_t getTlCmInfo(mcSuid_t *suid, mcVersionInfo_t *versionInfo) {
    mcResult_t ret = MC_DRV_ERR_UNKNOWN;

    LOG_D("%s; %s", __FILE__, __func__);

    // Open device and session.
    const mcUuid_t UUID = TL_CM_UUID;
    cmp_t *tci;
    mcSessionHandle_t sessionHandle;

    ret = mcOpenDevice(MC_DEVICE_ID_DEFAULT);
    if(MC_DRV_OK != ret) {
        LOG_E("mcOpenDevice failed: %d", ret);
        return ret;
    }
    tci = (cmp_t*)malloc(sizeof(cmp_t));
    if(NULL == tci) {
        ret = MC_DRV_ERR_NO_FREE_MEMORY;
        LOG_E("TCI malloc failed: %d", ret);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }
    memset(&sessionHandle, 0, sizeof(sessionHandle));
    sessionHandle.deviceId = MC_DEVICE_ID_DEFAULT;
    ret = mcOpenSession(&sessionHandle, &UUID, (uint8_t*)tci, sizeof(cmp_t));
    if(MC_DRV_OK != ret) {
        LOG_E("mcOpenSession failed: %d", ret);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }

    mcBulkMap_t mapInfo;

    memset(&mapInfo, 0, sizeof(mapInfo));

    // GetSuid command.
    cmpMapGetSuid_t cmpMapGetSuid;
    cmpMapGetSuid.cmpCmdGetSuid.cmdHeader.commandId = MC_CMP_CMD_GET_SUID;
    ret = mcMap(&sessionHandle, &cmpMapGetSuid, sizeof(cmpMapGetSuid_t), &mapInfo);
    if(MC_DRV_OK != ret) {
        LOG_E("mcMap GetSuid failed: %d", ret);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }
    tci->msg.cmpCmdGetSuidTci.cmdHeader.version = MC_MAKE_VERSION(3, 0);
    tci->msg.cmpCmdGetSuidTci.cmdHeader.commandId = MC_CMP_CMD_GET_SUID;
#if ( __WORDSIZE == 64 )
    tci->msg.cmpCmdGetSuidTci.cmdHeader.mapInfo.addr =
        mapInfo.sVirtualAddr;
#else
    tci->msg.cmpCmdGetSuidTci.cmdHeader.mapInfo.addr =
        (uint8_t*)mapInfo.sVirtualAddr;
#endif
    tci->msg.cmpCmdGetSuidTci.cmdHeader.mapInfo.len =
        sizeof(cmpMapGetSuid_t);
    tci->msg.cmpCmdGetSuidTci.cmdHeader.cmpCmdMapOffsetInfo.offset = 0;
    tci->msg.cmpCmdGetSuidTci.cmdHeader.cmpCmdMapOffsetInfo.len =
        sizeof(cmpCmdGetSuid_t);

    // Send command and receive response.
    ret = mcNotify(&sessionHandle);
    if(MC_DRV_OK != ret) {
        LOG_E("mcNotify GetSuid failed: %d", ret);
        (void)mcUnmap(&sessionHandle, &cmpMapGetSuid, &mapInfo);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }
    ret = mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT);
    if(MC_DRV_OK != ret) {
        LOG_E("mcWaitNotification GetSuid failed: %d", ret);
        (void)mcUnmap(&sessionHandle, &cmpMapGetSuid, &mapInfo);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }

    // GetSuid response.
    if((RSP_ID(MC_CMP_CMD_GET_SUID) !=
                tci->msg.responseHeader.responseId)) {
        LOG_E("Not valid GetSuid responseId: %d",
                tci->msg.responseHeader.responseId);
        (void)mcUnmap(&sessionHandle, &cmpMapGetSuid, &mapInfo);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }
    if(0 != tci->msg.responseHeader.len) {
        LOG_E("Not enough GetSuid mapped length, required: %d",
                tci->msg.responseHeader.len);
        (void)mcUnmap(&sessionHandle, &cmpMapGetSuid, &mapInfo);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }
    if(MC_DRV_OK != cmpMapGetSuid.cmpRspGetSuid.rspHeader.returnCode) {
        LOG_E("GetSuid failed: %d",
                cmpMapGetSuid.cmpRspGetSuid.rspHeader.returnCode);
        (void)mcUnmap(&sessionHandle, &cmpMapGetSuid, &mapInfo);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }

    memcpy(suid, &cmpMapGetSuid.cmpRspGetSuid.suid, sizeof(mcSuid_t));

    ret = mcUnmap(&sessionHandle, &cmpMapGetSuid, &mapInfo);
    if(MC_DRV_OK != ret) {
        LOG_E("mcUnmap GetSuid failed: %d", ret);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }

    // GetVersion command.
    cmpMapGetVersion_t cmpMapGetVersion;
    cmpMapGetVersion.cmpCmdGetVersion.cmdHeader.commandId = MC_CMP_CMD_GET_VERSION;
    ret = mcMap(&sessionHandle, &cmpMapGetVersion, sizeof(cmpMapGetVersion_t), &mapInfo);
    if(MC_DRV_OK != ret) {
        LOG_E("mcMap GetVersion failed: %d", ret);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }

    tci->msg.cmpCmdGetVersionTci.cmdHeader.version = MC_MAKE_VERSION(3, 0);
    tci->msg.cmpCmdGetVersionTci.cmdHeader.commandId =
        MC_CMP_CMD_GET_VERSION;
#if ( __WORDSIZE == 64 )
    tci->msg.cmpCmdGetVersionTci.cmdHeader.mapInfo.addr =
        mapInfo.sVirtualAddr;
#else
    tci->msg.cmpCmdGetVersionTci.cmdHeader.mapInfo.addr =
        (uint8_t*)mapInfo.sVirtualAddr;
#endif
    tci->msg.cmpCmdGetVersionTci.cmdHeader.mapInfo.len =
        sizeof(cmpMapGetVersion_t);
    tci->msg.cmpCmdGetVersionTci.cmdHeader.cmpCmdMapOffsetInfo.offset = 0;
    tci->msg.cmpCmdGetVersionTci.cmdHeader.cmpCmdMapOffsetInfo.len =
        sizeof(cmpCmdGetVersion_t);

    // Send command and receive response.
    ret = mcNotify(&sessionHandle);
    if(MC_DRV_OK != ret) {
        LOG_E("mcNotify GetVersion failed: %d", ret);
        (void)mcUnmap(&sessionHandle, &cmpMapGetVersion, &mapInfo);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }
    ret = mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT);
    if(MC_DRV_OK != ret) {
        LOG_E("mcWaitNotification GetVersion failed: %d", ret);
        (void)mcUnmap(&sessionHandle, &cmpMapGetVersion, &mapInfo);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }

    // GetVersion response.
    if((RSP_ID(MC_CMP_CMD_GET_VERSION) !=
                tci->msg.responseHeader.responseId)) {
        LOG_E("Not valid GetVersion responseId: %d",
                tci->msg.responseHeader.responseId);
        (void)mcUnmap(&sessionHandle, &cmpMapGetVersion, &mapInfo);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }
    if(0 != tci->msg.responseHeader.len) {
        LOG_E("Not enough GetVersion mapped length, required: %d",
                tci->msg.responseHeader.len);
        (void)mcUnmap(&sessionHandle, &cmpMapGetVersion, &mapInfo);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }
    if(MC_DRV_OK != cmpMapGetVersion.cmpRspGetVersion.rspHeader.returnCode) {
        LOG_E("GetVersion failed: %d",
                cmpMapGetVersion.cmpRspGetVersion.rspHeader.returnCode);
        (void)mcUnmap(&sessionHandle, &cmpMapGetVersion, &mapInfo);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }

    memcpy(versionInfo,
            &cmpMapGetVersion.cmpRspGetVersion.data.versionData2.versionInfo,
            sizeof(mcVersionInfo_t));

    ret = mcUnmap(&sessionHandle, &cmpMapGetVersion, &mapInfo);
    if(MC_DRV_OK != ret) {
        LOG_E("mcUnmap GetVersion failed: %d", ret);
        mcCloseSession(&sessionHandle);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }

    // Close session and device.
    ret = mcCloseSession(&sessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("mcCloseSession failed: %d", ret);
        free(tci);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return ret;
    }
    free(tci);
    ret = mcCloseDevice(MC_DEVICE_ID_DEFAULT);
    if (MC_DRV_OK != ret) {
        LOG_E("mcCloseDevice failed: %d", ret);
    }

    return ret;
}

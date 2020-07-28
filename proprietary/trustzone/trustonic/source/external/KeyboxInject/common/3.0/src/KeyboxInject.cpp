/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
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
#include <stdio.h>
#include <string.h>

#define KEYMASTER_WANTED_VERSION 3

#include "MobiCoreDriverApi.h"
#include "tlTeeKeymaster_Api.h"
#include "KeyboxInject.h"
#include "log.h"

static const uint32_t DEVICE_ID = MC_DEVICE_ID_DEFAULT;
static const mcUuid_t gUuid = TEE_KEYMASTER_N_TA_UUID;

tciMessage_t *tci;
mcSessionHandle_t sessionHandle;

// -------------------------------------------------------------
// -------------------------------------------------------------
// -------------------------------------------------------------
mcResult_t tlcOpen(mcSpid_t spid)
{
    (void) spid;
    mcResult_t mcRet;
    mcVersionInfo_t versionInfo;

    memset(&versionInfo, 0, sizeof(versionInfo));

    LOG_D("Opening Trustonic TEE device.");
    mcRet = mcOpenDevice(DEVICE_ID);
    if(MC_DRV_OK != mcRet) {
        LOG_E("Error opening device: 0x%X.", mcRet);
        return mcRet;
    }

    tci = (tciMessage_t*)malloc(sizeof(tciMessage_t));
    if (tci == NULL) {
        LOG_E("Allocation of TCI failed");
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    memset(tci, 0, sizeof(tciMessage_t));

    LOG_D("Opening the session.");
    memset(&sessionHandle, 0, sizeof(mcSessionHandle_t));
    // The device ID (default device is used).
    sessionHandle.deviceId = DEVICE_ID;
    mcRet = mcOpenSession(&sessionHandle,
                          &gUuid,
                          (uint8_t *) tci,
                          (uint32_t) sizeof(tciMessage_t));

    if (MC_DRV_OK != mcRet) {
        LOG_E("Open session failed: 0x%X", mcRet);
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
    } else {
        LOG_D("open() succeeded.");
    }

    return mcRet;
}

// -------------------------------------------------------------
#if RPMB_SUPPORT
mcResult_t tlcSetAttestationData(uint8_t *data_p, uint32_t data_len)
#else
mcResult_t tlcSetAttestationData(uint8_t *data_p, uint32_t data_len,
                                 uint8_t *blob_p, uint32_t *blob_len)
#endif
{
    mcResult_t ret;
    mcBulkMap_t srcMapInfo;
#if !RPMB_SUPPORT
    mcBulkMap_t dstMapInfo;
#endif

    if (NULL == tci) {
        LOG_E("TCI has not been set up properly - exiting.");
        return MC_DRV_ERR_NOT_IMPLEMENTED;
    }

    ret = mcMap(&sessionHandle, data_p, data_len, &srcMapInfo);
    if (MC_DRV_OK != ret) {
        LOG_E("mcMap data_p failed: 0x%X", ret);
        return ret;
    }

#if !RPMB_SUPPORT
    ret = mcMap(&sessionHandle, blob_p, *blob_len, &dstMapInfo);
    if (MC_DRV_OK != ret) {
        LOG_E("mcMap blob_p failed: 0x%X", ret);
        mcUnmap(&sessionHandle, data_p, &srcMapInfo);
        return ret;
    }
#endif

    // Prepare command message in TCI.
    LOG_D("Preparing command message in TCI");
    tci->command.header.commandId = CMD_ID_TEE_SET_ATTESTATION_DATA;
    tci->set_attestation_data.data.data = (uint32_t)srcMapInfo.sVirtualAddr;
    tci->set_attestation_data.data.data_length = (uint32_t)srcMapInfo.sVirtualLen;
#if !RPMB_SUPPORT
    tci->set_attestation_data.blob.data = (uint32_t)dstMapInfo.sVirtualAddr;
    tci->set_attestation_data.blob.data_length = (uint32_t)dstMapInfo.sVirtualLen;
#endif

    LOG_D("Notifying the Trusted Application");
    ret = mcNotify(&sessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("Notify failed: 0x%X", ret);
        goto exit;
    }

    LOG_D("Waiting for the Trusted Application response");
    ret = mcWaitNotification(&sessionHandle, -1);
    if (MC_DRV_OK != ret) {
        LOG_E("Wait for response notification failed: 0x%X", ret);
        goto exit;
    }

    // Verify that the TA sent a response.
    if (RSP_ID(CMD_ID_TEE_SET_ATTESTATION_DATA) != tci->response.header.responseId) {
        LOG_E("TA did not send a response: 0x%X.",
            tci->response.header.responseId);
        ret = MC_DRV_ERR_INVALID_RESPONSE;
        goto exit;
    }

    if (KM_ERROR_OK != tci->response.header.returnCode) {
        LOG_E("Trusted Application did not send a valid return code: 0x%X (%d)",
            tci->response.header.returnCode, tci->response.header.returnCode);
        ret = tci->response.header.returnCode;
        goto exit;
    }

#if !RPMB_SUPPORT
    *blob_len = tci->set_attestation_data.blob.data_length;
#endif
exit:
    (void)mcUnmap(&sessionHandle, data_p, &srcMapInfo);
#if !RPMB_SUPPORT
    (void)mcUnmap(&sessionHandle, blob_p, &dstMapInfo);
#endif

    return ret;
}

// -------------------------------------------------------------
#if RPMB_SUPPORT
mcResult_t tlcCheckAttestationData(uint8_t *data_p, uint32_t data_len)
#else
mcResult_t tlcCheckAttestationData(uint8_t *data_p, uint32_t data_len,
                                 uint8_t *blob_p, uint32_t blob_len)
#endif
{
    mcResult_t ret;
    mcBulkMap_t dataMapInfo;
#if !RPMB_SUPPORT
    mcBulkMap_t blobMapInfo;
#endif

    if (NULL == tci) {
        LOG_E("TCI has not been set up properly - exiting.");
        return MC_DRV_ERR_NOT_IMPLEMENTED;
    }

    ret = mcMap(&sessionHandle, data_p, data_len, &dataMapInfo);
    if (MC_DRV_OK != ret) {
        LOG_E("mcMap data_p failed: 0x%X", ret);
        return ret;
    }

#if !RPMB_SUPPORT
    ret = mcMap(&sessionHandle, blob_p, blob_len, &blobMapInfo);
    if (MC_DRV_OK != ret) {
        LOG_E("mcMap blob_p failed: 0x%X", ret);
        mcUnmap(&sessionHandle, data_p, &dataMapInfo);
        return ret;
    }
#endif

    // Prepare command message in TCI.
    LOG_D("Preparing command message in TCI");
    tci->command.header.commandId = CMD_ID_TEE_CHECK_ATTESTATION_DATA;
    tci->check_attestation_data.data.data = (uint32_t)dataMapInfo.sVirtualAddr;
    tci->check_attestation_data.data.data_length = (uint32_t)dataMapInfo.sVirtualLen;
#if !RPMB_SUPPORT
    tci->check_attestation_data.blob.data = (uint32_t)blobMapInfo.sVirtualAddr;
    tci->check_attestation_data.blob.data_length = (uint32_t)blobMapInfo.sVirtualLen;
#endif

    LOG_D("Notifying the Trusted Application");
    ret = mcNotify(&sessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("Notify failed: 0x%X", ret);
        goto exit;
    }

    LOG_D("Waiting for the Trusted Application response");
    ret = mcWaitNotification(&sessionHandle, -1);
    if (MC_DRV_OK != ret) {
        LOG_E("Wait for response notification failed: 0x%X", ret);
        goto exit;
    }

    // Verify that the TA sent a response.
    if (RSP_ID(CMD_ID_TEE_CHECK_ATTESTATION_DATA) != tci->response.header.responseId) {
        LOG_E("TA did not send a response: 0x%X.",
            tci->response.header.responseId);
        ret = MC_DRV_ERR_INVALID_RESPONSE;
        goto exit;
    }

    if (KM_ERROR_OK != tci->response.header.returnCode) {
        LOG_E("Trusted Application did not send a valid return code: 0x%X (%d)",
            tci->response.header.returnCode, tci->response.header.returnCode);
        ret = tci->response.header.returnCode;
        goto exit;
    }

exit:
    (void)mcUnmap(&sessionHandle, data_p, &dataMapInfo);
#if !RPMB_SUPPORT
    (void)mcUnmap(&sessionHandle, blob_p, &blobMapInfo);
#endif

    return ret;
}

// -------------------------------------------------------------
void tlcClose(void)
{
    mcResult_t ret;

    LOG_D("Closing the session.");
    ret = mcCloseSession(&sessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("Closing session failed: 0x%X.", ret);
        // Continue even in case of error.
    }

    ret = mcCloseDevice(DEVICE_ID);
    if (MC_DRV_OK != ret) {
        LOG_E("Closing <t-base device failed: 0x%X.", ret);
        // Continue even in case of error.
    }

    free(tci);
    tci = NULL;
}

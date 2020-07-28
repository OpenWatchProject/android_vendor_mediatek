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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tlcfoo.h"
#include "MobiCoreDriverApi.h"
#include "dciTest.h"

#define LOG_TAG "tuitest"
#include <android/log.h>

static const uint32_t DEVICE_ID = MC_DEVICE_ID_DEFAULT;
dciMessage_t *dci;
mcSessionHandle_t sessionHandle;

// -------------------------------------------------------------
/*
static mcResult_t executeCmd(uint32_t *num1, uint32_t *num2, uint32_t *result, tciCommandId_t cmd)
{
	mcResult_t  ret;

	if(NULL == tci)
	{
	    LOG_E("TCI has not been set up properly - exiting");
	    return MC_DRV_ERR_NO_FREE_MEMORY;
	}

	tci->cmdfoo.header.commandId = cmd;
	tci->cmdfoo.len = 0;
	tci->cmdfoo.respLen = 0;
	tci->Num1 = *num1;
	tci->Num2 = *num2;

	LOG_I("Preparing command message in TCI");

	LOG_I("Notifying the trustlet");
	ret = mcNotify(&sessionHandle);

	if (MC_DRV_OK != ret)
	{
	    LOG_E("Notify failed: %d", ret);
	    goto exit;
	}

	LOG_I("Waiting for the Trustlet response");
	ret = mcWaitNotification(&sessionHandle, -1);

	if (MC_DRV_OK != ret)
	{
	    LOG_E("Wait for response notification failed: 0x%x", ret);
	    goto exit;
	}

	*result = tci->ResultData;

	LOG_I("Verifying that the Trustlet sent a response.");
	if (RSP_ID(cmd) != tci->rspfoo.header.responseId)
	{
	    LOG_E("Trustlet did not send a response: %d",
	        tci->rspfoo.header.responseId);
	    ret = MC_DRV_ERR_INVALID_RESPONSE;
	    goto exit;
	}

	if (RET_OK != tci->rspfoo.header.returnCode)
	{
	    LOG_E("Trustlet did not send a valid return code: %d",
	        tci->rspfoo.header.returnCode);
	    ret = tci->rspfoo.header.returnCode;
	}

	exit:
	return ret;
}
*/

// -------------------------------------------------------------
static size_t getFileContent(
    const char* pPath,
    uint8_t** ppContent)
{
	FILE*   pStream;
	long    filesize;
	uint8_t* content = NULL;

	/*
	 * The stat function is not used (not available in WinCE).
	 */

	/* Open the file */
	pStream = fopen(pPath, "rb");
	if (pStream == NULL)
	{
		fprintf(stderr, "Error: Cannot open file: %s.\n", pPath);
		return 0;
	}

	if (fseek(pStream, 0L, SEEK_END) != 0)
	{
		fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
		goto error;
	}

	filesize = ftell(pStream);
	if (filesize < 0)
	{
		fprintf(stderr, "Error: Cannot get the file size: %s.\n", pPath);
		goto error;
	}

	if (filesize == 0)
	{
		fprintf(stderr, "Error: Empty file: %s.\n", pPath);
		goto error;
	}

	/* Set the file pointer at the beginning of the file */
	if (fseek(pStream, 0L, SEEK_SET) != 0)
	{
		fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
		goto error;
	}

	/* Allocate a buffer for the content */
	content = (uint8_t*)malloc(filesize);
	if (content == NULL)
	{
		fprintf(stderr, "Error: Cannot read file: Out of memory.\n");
		goto error;
	}

	/* Read data from the file into the buffer */
	if (fread(content, (size_t)filesize, 1, pStream) != 1)
	{
		fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
		goto error;
	}

	/* Close the file */
	fclose(pStream);
	*ppContent = content;

	/* Return number of bytes read */
	return (size_t)filesize;

	error:
	if (content  != NULL)
	{
		free(content);
	}
	fclose(pStream);
	return 0;

}

// -------------------------------------------------------------
mcResult_t tlcOpen(void)
{
    mcResult_t mcRet;
    mcVersionInfo_t versionInfo;
    uint8_t* pTrustletData = NULL;
    uint32_t nTrustletSize;

    LOG_I("Opening <t-base device");
    mcRet = mcOpenDevice(DEVICE_ID);
    if (MC_DRV_OK != mcRet)
    {
	    LOG_E("Error opening device: %d", mcRet);
	    return mcRet;
    }

    mcRet = mcGetMobiCoreVersion(MC_DEVICE_ID_DEFAULT, &versionInfo);
    if (MC_DRV_OK != mcRet)
    {
        LOG_E("mcGetMobiCoreVersion failed %d", mcRet);
        mcCloseDevice(DEVICE_ID);
        return mcRet;
    }
    LOG_I("productId        = %s", versionInfo.productId);
    LOG_I("versionMci       = 0x%08X", versionInfo.versionMci);
    LOG_I("versionSo        = 0x%08X", versionInfo.versionSo);
    LOG_I("versionMclf      = 0x%08X", versionInfo.versionMclf);
    LOG_I("versionContainer = 0x%08X", versionInfo.versionContainer);
    LOG_I("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
    LOG_I("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
    LOG_I("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
    LOG_I("versionCmp       = 0x%08X", versionInfo.versionCmp);

    dci = (dciMessage_t*)malloc(sizeof(dciMessage_t));

    if (dci == NULL)
    {
        LOG_E("Allocation of DCI failed");
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    memset(dci, 0, sizeof(dciMessage_t));

    nTrustletSize = getFileContent(
                        "/vendor/app/mcRegistry/070c0000000000000000000000000001.tlbin",
                        &pTrustletData);

    if (nTrustletSize == 0)
    {
        LOG_E("Driver not found");
        free(dci);
        dci = NULL;
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    }

    LOG_I("Opening the session");
    memset(&sessionHandle, 0, sizeof(sessionHandle));
    sessionHandle.deviceId = DEVICE_ID; // The device ID (default device is used)
    mcRet = mcOpenTrustlet(
            &sessionHandle,
            //MC_SPID_RESERVED_TEST, /* mcSpid_t */
            MC_SPID_SYSTEM, /* mcSpid_t */
            pTrustletData,
            nTrustletSize,
            (uint8_t *) dci,
            sizeof(dciMessage_t));

    // Whatever the result is, free the buffer
    free(pTrustletData);

    if (MC_DRV_OK != mcRet)
    {
        LOG_E("Open session failed: %d", mcRet);
        free(dci);
        dci = NULL;
        mcCloseDevice(DEVICE_ID);
    }
    else
    {
        LOG_I("open() succeeded");
    }

	mcWaitNotification(&sessionHandle, -1);

    return mcRet;
}

// -------------------------------------------------------------
static mcResult_t sendCmd(int cmd)
{
	mcResult_t  ret;

	if(NULL == dci)
	{
	    LOG_E("TCI has not been set up properly - exiting");
	    return MC_DRV_ERR_NO_FREE_MEMORY;
	}

	LOG_I("Preparing command message in TCI");
	dci->command.header.commandId = cmd;

	LOG_I("Notifying the trustlet");
	ret = mcNotify(&sessionHandle);

	if (MC_DRV_OK != ret)
	{
	    LOG_E("Notify failed: %d", ret);
	    goto exit;
	}

	LOG_I("Waiting for the Trustlet response");
	ret = mcWaitNotification(&sessionHandle, -1);

	if (MC_DRV_OK != ret)
	{
	    LOG_E("Wait for response notification failed: 0x%x", ret);
	    goto exit;
	}

	LOG_I("Verifying that the Trustlet sent a response.");
	if (RSP_ID(cmd) != dci->response.header.responseId)
	{
	    LOG_E("Trustlet did not send a response: %d",
	        dci->response.header.responseId);
	    ret = MC_DRV_ERR_INVALID_RESPONSE;
	    goto exit;
	}

	if (RET_OK != dci->response.header.returnCode)
	{
	    LOG_E("Trustlet did not send a valid return code: %d",
	        dci->response.header.returnCode);
	    ret = dci->response.header.returnCode;
	}

	exit:
	return ret;
}

mcResult_t send_cmd(int cmd_id)
{
    mcResult_t ret=RET_OK;

    LOG_I("send_cmd(%d)", cmd_id);
    ret = sendCmd(cmd_id);
    return ret;
}

// -------------------------------------------------------------
void tlcClose(void)
{
    mcResult_t ret;

    LOG_I("Closing the session");
    ret = mcCloseSession(&sessionHandle);
    if (MC_DRV_OK != ret)
    {
        LOG_E("Closing session failed: %d", ret);
        /* continue even in case of error */
    }

    LOG_I("Closing <t-base device");
    ret = mcCloseDevice(DEVICE_ID);
    if (MC_DRV_OK != ret)
    {
        LOG_E("Closing <t-base device failed: %d", ret);
        /* continue even in case of error */;
    }
    free(dci);
    dci = NULL;
}

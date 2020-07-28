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
#include <string.h>

#include "MobiCoreDriverApi.h"
#include "tlrpmb_Api.h"

#define LOG_TAG "TLC RPMB"
#include "cutils/log.h"

#define LOG_I ALOGD
#define LOG_E ALOGE

static const uint32_t DEVICE_ID = MC_DEVICE_ID_DEFAULT;
tciMessage_t *tci;
tciMessage_t *dci;
mcSessionHandle_t sessionHandle;
mcSessionHandle_t sessionHandle1;

// -------------------------------------------------------------
static mcResult_t executeCmd(uint32_t *buf, uint32_t size, uint32_t *num1, uint32_t *num2, uint32_t *result, tciCommandId_t cmd)
{
	mcResult_t  ret;

	if(NULL == tci)
	{
	    LOG_E("TCI has not been set up properly - exiting");
	    return MC_DRV_ERR_NO_FREE_MEMORY;
	}

	tci->cmdrpmb.header.commandId = cmd;
	tci->cmdrpmb.len = 0;
	tci->cmdrpmb.respLen = 0;
    memcpy(tci->Buf, buf, size);
    tci->BufSize    = size;
	tci->Num1       = *num1;
	tci->Num2       = *num2;

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
	if (RSP_ID(cmd) != tci->rsprpmb.header.responseId)
	{
	    LOG_E("Trustlet did not send a response: %d",
	        tci->rsprpmb.header.responseId);
	    ret = MC_DRV_ERR_INVALID_RESPONSE;
	    goto exit;
	}

	if (RET_OK != tci->rsprpmb.header.returnCode)
	{
	    LOG_E("Trustlet did not send a valid return code: %d",
	        tci->rsprpmb.header.returnCode);
	    ret = tci->rsprpmb.header.returnCode;
	}

	exit:
	return ret;
}

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

    tci = (tciMessage_t*)malloc(sizeof(tciMessage_t));
//    dci = (tciMessage_t*)malloc(sizeof(tciMessage_t));

    if (tci == NULL)
    {
        LOG_E("Allocation of TCI failed");
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    memset(tci, 0, sizeof(tciMessage_t));
/*
    if (dci == NULL)
    {
        LOG_E("Allocation of DCI failed");
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    memset(dci, 0, sizeof(tciMessage_t));
*/

    nTrustletSize = getFileContent(
                        "/system/vendor/app/mcRegistry/06040000000000000000000000000000.tlbin",
                        &pTrustletData);
    if (nTrustletSize == 0)
    {
        LOG_E("Trustlet not found");
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    }

    LOG_I("Opening the session");
    memset(&sessionHandle, 0, sizeof(sessionHandle));
    sessionHandle.deviceId = DEVICE_ID; // The device ID (default device is used)
    mcRet = mcOpenTrustlet(
            &sessionHandle,
            MC_SPID_RESERVED_TEST, /* mcSpid_t */
            pTrustletData,
            nTrustletSize,
            (uint8_t *) tci,
            sizeof(tciMessage_t));

    if (MC_DRV_OK != mcRet)
    {
        LOG_E("Open session failed: %d", mcRet);
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
    }
    else
    {
        LOG_I("open() succeeded");
    }
#if 0
    nTrustletSize = getFileContent(
                        "/system/app/mcRegistry/07050000000000000000000000000000.drbin",
                        &pTrustletData);

    if (nTrustletSize == 0)
    {
        LOG_E("Driver not found");
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    }

    LOG_I("Opening the session");
    memset(&sessionHandle1, 0, sizeof(sessionHandle));
    sessionHandle.deviceId = DEVICE_ID; // The device ID (default device is used)
    mcRet = mcOpenTrustlet(
            &sessionHandle1,
            MC_SPID_RESERVED_TEST, /* mcSpid_t */
            pTrustletData,
            nTrustletSize,
            (uint8_t *) dci,
            sizeof(tciMessage_t));

    // Whatever the result is, free the buffer
    free(pTrustletData);

    if (MC_DRV_OK != mcRet)
    {
        LOG_E("Open session failed: %d", mcRet);
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
    }
    else
    {
        LOG_I("open() succeeded");
    }
#endif
    return mcRet;
}
/*
// -------------------------------------------------------------
mcResult_t add(uint32_t *num1, uint32_t *num2, uint32_t *result)
{
    mcResult_t  ret;

	LOG_I("Rpmb perform addition (%d) + (%d)", *num1, *num2);

    ret = executeCmd(num1, num2, result, CMD_RPMB_ADD);
    if (ret != RET_OK)
    {
        LOG_E("Unable to execute CMD_RPMB_ADD command: %d", ret);
        goto exit;
    }

    LOG_I("The result is %d", *result);

exit:
    return ret;
}

// -------------------------------------------------------------
mcResult_t minus(uint32_t *num1, uint32_t *num2, uint32_t *result)
{
    mcResult_t  ret;

	LOG_I("Rpmb perform minus (%d) - (%d)", *num2, *num1);

    ret = executeCmd(num1, num2, result, CMD_RPMB_SUB);
    if (ret != RET_OK)
    {
        LOG_E("Unable to execute CMD_RPMB_SUB command: %d", ret);
        goto exit;
    }

    LOG_I("The result is %d", *result);

exit:
    return ret;
}
*/

// -------------------------------------------------------------
mcResult_t read(uint32_t *buf, uint32_t size, uint32_t *result)
{
    mcResult_t ret;
    uint32_t num1 = 1;
    uint32_t num2 = 5;

	LOG_I("Rpmb perform read");

    ret = executeCmd(buf, size, &num1, &num2, result, CMD_RPMB_READ_DATA);
    //if (ret != RET_OK)
    //{
    //    LOG_E("Unable to execute CMD_RPMB_READ_DATA command: %d", ret);
    //    goto exit;
    //}

    memcpy(buf, tci->Buf, size);


    LOG_I("The result is %d", *result);

exit:
    return ret;
}

// -------------------------------------------------------------
mcResult_t write(uint32_t *buf, uint32_t size, uint32_t *result)
{
    mcResult_t ret;
    uint32_t num1 = 10;
    uint32_t num2 = 50;

    LOG_I("Rpmb perform write");

    ret = executeCmd(buf, size, &num1, &num2, result, CMD_RPMB_WRITE_DATA);
    if (ret != RET_OK)
    {
        LOG_E("Unable to execute CMD_RPMB_WRITE_DATA command: %d", ret);
        goto exit;
    }

    LOG_I("The result is %d", *result);

exit:
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
    free(tci);
    tci = NULL;
}

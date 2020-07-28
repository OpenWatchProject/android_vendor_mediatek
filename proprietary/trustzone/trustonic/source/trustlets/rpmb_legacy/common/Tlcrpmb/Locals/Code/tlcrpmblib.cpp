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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include "MobiCoreDriverApi.h"
#include "tlrpmb_Api.h"
#include "tlcrpmb.h"

#define LOG_TAG "TLC_RPMB_LIB"
#include "cutils/log.h"
#define LOG_I ALOGD
#define LOG_E ALOGE

#define DEVICE_ID  MC_DEVICE_ID_DEFAULT
static tciMessage_t *tci = NULL;
static mcSessionHandle_t sessionHandle;
static uint8_t* pTrustletData = NULL;
static uint16_t currUid = 0;
static uint32_t rpmbSize = 0;

#define TLC_RPMB_LOCK_INITED   0x21
#define TLC_RPMB_LOCK_UNINIT   0x31
static pthread_mutex_t tlcRpmbLock;
static int lockStat = TLC_RPMB_LOCK_UNINIT;

// -------------------------------------------------------------
static bool checkSessionOpenStat(rpmb_uid_t uid)
{

    if (uid < RPMB_USER_ID_BASE ||
        uid >= RPMB_USER_ID_END)
    {
        LOG_E("rpmb user id invalid!!");
        return false;
    }

    if (sessionHandle.sessionId == 0 ||
        currUid == 0 ||
        rpmbSize == 0)
    {
        return false;
    }

    if (currUid != 0 &&
        currUid != uid)
    {
        return false;
    }

    return true;
}

static bool isLockInitialized(void)
{
    if (lockStat == TLC_RPMB_LOCK_INITED)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static mcResult_t ExecuteCmd(rpmb_uid_t uid, uint32_t offset, uint8_t *buf, uint32_t size, tciCommandId_t cmd, uint32_t *result)
{
	mcResult_t  ret;

	if(NULL == tci)
	{
	    LOG_E("TCI has not been set up properly - exiting");
	    return MC_DRV_ERR_NO_FREE_MEMORY;
	}

    if (size > RPMB_TCI_BUF_SIZE)
    {
        size = RPMB_TCI_BUF_SIZE;
    }

	LOG_I("Preparing command message in TCI");
	tci->cmdrpmb.header.commandId = cmd;
	tci->cmdrpmb.len = 0;
	tci->cmdrpmb.respLen = 0;
	if (cmd == CMD_RPMB_WRITE_DATA)
	{
        memcpy(tci->buf, buf, size);
    }
    tci->size = size;
	tci->uid = uid;
	tci->rpmbOffset = offset;

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

	*result = tci->resultData;

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
	    goto exit;
	}

    if (cmd == CMD_RPMB_READ_DATA)
	{
        memcpy(buf, tci->buf, tci->size);
    }
exit:
	return ret;
}

// -------------------------------------------------------------
static size_t GetFileContent(
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
mcResult_t tlcRpmbOpen(rpmb_uid_t uid)
{
    mcResult_t mcRet;
    mcVersionInfo_t versionInfo;
    uint32_t nTrustletSize;
	int fd = 0;
	int ret = 0;
	struct rpmb_user user_stat;

    if (!isLockInitialized())
    {
        pthread_mutex_init(&tlcRpmbLock, NULL);
        lockStat = TLC_RPMB_LOCK_INITED;
        LOG_I("RPMB lock initialized");
    }

    pthread_mutex_lock(&tlcRpmbLock);
    //atexit(tlcRpmbClose);

    if (uid < RPMB_USER_ID_BASE ||
        uid >= RPMB_USER_ID_END)
    {
        LOG_E("rpmb user id invalid!!");
        pthread_mutex_unlock(&tlcRpmbLock);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    if (uid == currUid)
    {
        LOG_I("rpmb session for user:%d has been opened in the same process!!", convertUserId(uid));
        pthread_mutex_unlock(&tlcRpmbLock);
        return MC_DRV_OK;
    }

    if (currUid != 0 &&
        uid != currUid)
    {
        LOG_E("It can not be opened more than one rpmb session in the same process!!");
        pthread_mutex_unlock(&tlcRpmbLock);
        return MC_DRV_ERR_OUT_OF_RESOURCES;
    }


    //Check if the corresponding uid sessioin has been opened.
    fd = open("/dev/emmcrpmb0", O_RDONLY);
    if (fd < 0) {
    	LOG_E("emmcrpmb0 open failed, errno: %d!!", fd);
    	pthread_mutex_unlock(&tlcRpmbLock);
        return MC_DRV_ERR_UNKNOWN;
    }
	user_stat.uid = uid;
	ret = ioctl(fd, RPMB_IOCTL_GET_USER_STAT, &user_stat);
    if (ret < 0)
    {
        LOG_E("call ioctl failed, errno: %d!!", ret);
        close(fd);
        pthread_mutex_unlock(&tlcRpmbLock);
        return MC_DRV_ERR_UNKNOWN;
    }

    if (user_stat.sessionid != 0)
    {
        char tmp_buf[64];
        struct stat dir_stat;
        snprintf(tmp_buf, sizeof(tmp_buf), "/proc/%d", user_stat.pid);
        ret = stat(tmp_buf, &dir_stat);
        if (ret < 0)
        {
            //if process was killed by SIGKILL, there is no chance to call tlcRpmbClose(),
            //when reopen rpmb, we need to check if this case has been occured and clear user record first.
            LOG_I("One session for uid %d is not closed by process(pid,%d) which has been terminated!!",
                  convertUserId(uid), user_stat.pid);
            user_stat.uid = uid;
            user_stat.sessionid = 0;
            user_stat.pid = 0;
            ioctl(fd, RPMB_IOCTL_SET_USER_STAT, &user_stat);
        }
        else
        {
            LOG_E("One session for user:%d has been opened by other process(pid, %d)!!", convertUserId(uid), user_stat.pid);
            close(fd);
            pthread_mutex_unlock(&tlcRpmbLock);
            return MC_DRV_ERR_OUT_OF_RESOURCES;
        }
    }

    LOG_I("Opening <t-base device");
    mcRet = mcOpenDevice(DEVICE_ID);
    if (MC_DRV_OK != mcRet)
    {
	    LOG_E("Error opening device: %d", mcRet);
	    close(fd);
	    pthread_mutex_unlock(&tlcRpmbLock);
	    return mcRet;
    }

    mcRet = mcGetMobiCoreVersion(MC_DEVICE_ID_DEFAULT, &versionInfo);
    if (MC_DRV_OK != mcRet)
    {
        LOG_E("mcGetMobiCoreVersion failed %d", mcRet);
        close(fd);
        mcCloseDevice(DEVICE_ID);
        pthread_mutex_unlock(&tlcRpmbLock);
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
    if (tci == NULL)
    {
        LOG_E("Allocation of TCI failed");
        close(fd);
        mcCloseDevice(DEVICE_ID);
        pthread_mutex_unlock(&tlcRpmbLock);
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    memset(tci, 0, sizeof(tciMessage_t));

    nTrustletSize = GetFileContent(
                        "/system/app/mcRegistry/06040000000000000000000000000000.tlbin",
                        &pTrustletData);
    if (nTrustletSize == 0)
    {
        LOG_E("Trustlet not found");
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
        close(fd);
        pthread_mutex_unlock(&tlcRpmbLock);
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
        free(pTrustletData);
        pTrustletData = NULL;
        mcCloseDevice(DEVICE_ID);
        close(fd);
        pthread_mutex_unlock(&tlcRpmbLock);
        return mcRet;
    }

    user_stat.sessionid = sessionHandle.sessionId;
    user_stat.pid = getpid();
	if ((ret = ioctl(fd, RPMB_IOCTL_SET_USER_STAT, &user_stat)) < 0 ||
	    (ret = ioctl(fd, RPMB_IOCTL_GET_RPMB_SIZE, &rpmbSize)) < 0)
    {
        LOG_E("call ioctl failed, errno: %d!!", ret);
        mcCloseSession(&sessionHandle);
        sessionHandle.sessionId = 0;
        free(tci);
        tci = NULL;
        free(pTrustletData);
        pTrustletData = NULL;
        mcCloseDevice(DEVICE_ID);
        close(fd);
        pthread_mutex_unlock(&tlcRpmbLock);
        return MC_DRV_ERR_UNKNOWN;
    }
    close(fd);
    currUid = uid;

    //due to the last half-block of rpmb can not be written with its half-block address
    //we reserve RPMB_MAX_USER_NUM full-blocks(512*RPMB_MAX_USER_NUM) to prevent last half-block writing failure.
    rpmbSize -= MAX_RPMB_USER_NUMBER*512; 
    rpmbSize /= MAX_RPMB_USER_NUMBER;
    LOG_I("uid: %d, RpmbSize: %d", convertUserId(uid), rpmbSize);
    LOG_I("tlcRpmbOpen() succeeded, uid: %d", convertUserId(uid));
    pthread_mutex_unlock(&tlcRpmbLock);
    return MC_DRV_OK;
}

//--------------------------------------------------------------
mcResult_t tlcRpmbGetSize(rpmb_uid_t uid, uint32_t *size)
{
	LOG_I("Rpmb get size");

    if (NULL == size)
    {
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

	if (!isLockInitialized())
	{
        LOG_E("lock has not been initialized, maybe rpmb session is not opened.");
        return MC_DRV_ERR_INIT;
	}
	pthread_mutex_lock(&tlcRpmbLock);
    if (!checkSessionOpenStat(uid))
    {
        LOG_E("Corresponding session has not been opened, UID:%d!!", convertUserId(uid));
        return MC_DRV_ERR_INIT;
    }

    *size = rpmbSize;
    pthread_mutex_unlock(&tlcRpmbLock);
    return MC_DRV_OK;
}

// -------------------------------------------------------------
static mcResult_t tlcRpmbReadWrite(rpmb_uid_t uid, uint32_t offset, uint8_t *buf, uint32_t *size, tciCommandId_t cmd)
{
    mcResult_t ret;
    uint32_t result = 0;
    uint32_t leftSize;
    uint32_t tranSize;
    uint32_t tranOffset = offset;
    uint8_t  *bufPtr = buf;

	if (!isLockInitialized())
	{
        LOG_E("lock has not been initialized, maybe rpmb session is not opened.");
        return MC_DRV_ERR_INIT;
	}
	pthread_mutex_lock(&tlcRpmbLock);

    if (!checkSessionOpenStat(uid))
    {
        LOG_E("Corresponding session has not been open, UID:%d!!", convertUserId(uid));
        pthread_mutex_unlock(&tlcRpmbLock);
        return MC_DRV_ERR_INIT;
    }

    //check rpmb offset and buffer scope valid
    if (offset >= rpmbSize ||
        buf == NULL || size == NULL ||
        *size == 0 ||
        (cmd != CMD_RPMB_READ_DATA && cmd != CMD_RPMB_WRITE_DATA))
    {
        LOG_E("Invalid parameters");
        pthread_mutex_unlock(&tlcRpmbLock);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    if (offset + *size > rpmbSize)
    {
        *size = rpmbSize - offset;
    }

    leftSize = *size;


    while (leftSize > 0)
    {
        if (leftSize > RPMB_TCI_BUF_SIZE)
        {
            tranSize = RPMB_TCI_BUF_SIZE;
        }
        else
        {
            tranSize = leftSize;
        }

        ret = ExecuteCmd(uid, tranOffset, bufPtr, tranSize, cmd, &result);
        if (ret != MC_DRV_OK)
        {
            LOG_E("Some error occured during %s data", (cmd==CMD_RPMB_WRITE_DATA) ? "WRITE" : "READ");
            break;
        }
        bufPtr += tranSize;
        tranOffset += tranSize;
        leftSize -= tranSize;
    }

    pthread_mutex_unlock(&tlcRpmbLock);
    LOG_I("The result is %d, Transfer Data Len: %d", result, *size);
    return ret;
}

// -------------------------------------------------------------
mcResult_t tlcRpmbRead(rpmb_uid_t uid, uint32_t offset, uint8_t *buf, uint32_t *size)
{
	LOG_I("Rpmb perform READ, UID:%d", convertUserId(uid));
    memset(tci, 0, sizeof(tciMessage_t));	
    return tlcRpmbReadWrite(uid, offset, buf, size, CMD_RPMB_READ_DATA);
}

// -------------------------------------------------------------
mcResult_t tlcRpmbWrite(rpmb_uid_t uid, uint32_t offset, uint8_t *buf, uint32_t *size)
{
	LOG_I("Rpmb perform WRITE, UID:%d", convertUserId(uid));
	memset(tci, 0, sizeof(tciMessage_t));
    return tlcRpmbReadWrite(uid, offset, buf, size, CMD_RPMB_WRITE_DATA);
}

// -------------------------------------------------------------
void tlcRpmbClose(void)
{
    mcResult_t ret;
    int fd = 0;
    struct rpmb_user user_stat;

    LOG_I("Closing the session and <t-base device");
	if (!isLockInitialized())
	{
        LOG_E("lock has not been initialized, maybe rpmb session is not opened.");
        return;
	}
    pthread_mutex_lock(&tlcRpmbLock);

    if (sessionHandle.sessionId == 0)
    {
        LOG_I("RPMB session has been closed, no need do re-close.");
        pthread_mutex_unlock(&tlcRpmbLock);
        return;
    }

    fd = open("/dev/emmcrpmb0", O_RDONLY);
    if (fd < 0) {
    	LOG_E("emmcrpmb0 open failed, errno: %d!!", fd);
    }

    if (fd > 0)
    {
        user_stat.uid = currUid;
        user_stat.sessionid = 0;
        user_stat.pid = 0;
        ioctl(fd, RPMB_IOCTL_SET_USER_STAT, &user_stat);
    }

    ret = mcCloseSession(&sessionHandle);
    if (MC_DRV_OK != ret)
    {
        LOG_E("Closing session failed: %d", ret);
        /* continue even in case of error */
    }

    ret = mcCloseDevice(DEVICE_ID);
    if (MC_DRV_OK != ret)
    {
        LOG_E("Closing <t-base device failed: %d", ret);
        /* continue even in case of error */;
    }
    if (tci)
    {
        free(tci);
        tci = NULL;
    }
    if (pTrustletData)
    {
        free(pTrustletData);
        pTrustletData = NULL;
    }
    memset(&sessionHandle, 0, sizeof(sessionHandle));
    currUid = 0;
    rpmbSize = 0;
    pthread_mutex_unlock(&tlcRpmbLock);
    return;
}


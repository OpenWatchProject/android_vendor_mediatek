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
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

//#include "tlc_sec_mem.h"
/* From the TLC interface */
#include "MobiCoreDriverApi.h"

#define SKIP_DEBUG_ALIAS_MACRO
#include "tlc_sec_mem.h"
#include "secmem.h"

#define LOG_TAG "TLC_SECMEM"
#include "tlsecmem_log.h"

#define RET_OK	0

#if 0
static const uint32_t DEVICE_ID = MC_DEVICE_ID_DEFAULT;
static tciMessage_t *tci_tl = NULL;
static mcSessionHandle_t sessionHandle_tci_tl;
#endif

static unsigned int open_device_count = 0;
static int secmem_fd = -1;

/* Used for dci interface */
#ifdef NEVER
dciMessage_t *dci;
mcSessionHandle_t sessionHandle_dci;
#endif /* NEVER */

pthread_mutex_t mem_mutex = PTHREAD_MUTEX_INITIALIZER;

#if 0
// -------------------------------------------------------------
static mcResult_t executeCmd(UREE_SECMEM_PARAM *sec_mem_param, dciCommandId_t cmd)
{
	mcResult_t  ret;    

	if(NULL == tci_tl) 
	{
	    LOG_E("TCI has not been set up properly - exiting");
	    return MC_DRV_ERR_NO_FREE_MEMORY;
	}

	tci_tl->cmd_secmem.header.commandId = cmd;
	tci_tl->cmd_secmem.len = 0;
    tci_tl->sec_handle= sec_mem_param->sec_handle;
	tci_tl->alignment = sec_mem_param->alignment;
	tci_tl->size = sec_mem_param->size;
    tci_tl->refcount = sec_mem_param->refcount;
#ifdef SECMEM_TLC_DEBUG
    tci_tl->sender.id = sec_mem_param->id;
    if (sec_mem_param->owner) {
        char *owner = strrchr((char*)sec_mem_param->owner, '/') + 1;
        int len;
        owner = (owner == NULL)? (char*)sec_mem_param->owner : owner;
        len = strlen(owner) + 1;
        len = len > MAX_NAME_SZ ? MAX_NAME_SZ : len;
        memcpy(tci_tl->sender.name, owner, len);
        tci_tl->sender.name[MAX_NAME_SZ - 1] = '\0';
    } else {
        strcpy((char*)&tci_tl->sender.name[0], "Unknown");
    }
#endif

	LOG_I("Preparing command message in TCI : (%d)", cmd);
	LOG_I("sec_mem_param handle: (0x%x)", sec_mem_param->sec_handle);
    LOG_I("sec_mem_param tci_handle: (0x%x)", tci_tl->sec_handle);
    LOG_I("sec_mem_param alignment: (%d)", sec_mem_param->alignment);
    LOG_I("sec_mem_param refcount: (%d)", sec_mem_param->refcount);
	LOG_I("sec_mem_param size: (%d)", sec_mem_param->size);

	LOG_I("Notifying the trustlet"); //tci_tl is ready at this point
	ret = mcNotify(&sessionHandle_tci_tl);

	if (MC_DRV_OK != ret)
	{
	    LOG_E("Notify failed: %d", ret);
	    goto exit;
	}

	LOG_I("Waiting for the Trustlet response");
	ret = mcWaitNotification(&sessionHandle_tci_tl, -1);

	if (MC_DRV_OK != ret) 
	{
	    LOG_E("Wait for response notification failed: 0x%x", ret);
	    goto exit;
	}
    /* Correct handle should be get after return from secure world. */
    sec_mem_param->sec_handle = tci_tl->sec_handle;
    sec_mem_param->refcount = tci_tl->refcount;
    sec_mem_param->alignment = tci_tl->alignment;
    sec_mem_param->size = tci_tl->size;

	LOG_I("Verifying that the Trustlet sent a response.");
	if (RSP_ID(cmd) != tci_tl->rsp_secmem.header.responseId) 
	{
	    LOG_E("Trustlet did not send a response: %d",
	        tci_tl->rsp_secmem.header.responseId);
	    ret = MC_DRV_ERR_INVALID_RESPONSE;
	    goto exit;
	}

	if (RET_OK != tci_tl->rsp_secmem.header.returnCode) 
	{
	    LOG_E("Trustlet did not send a valid return code: %d",
	        tci_tl->rsp_secmem.header.returnCode);
	    ret = tci_tl->rsp_secmem.header.returnCode;
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
	 * The stat function is not used .
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
mcResult_t tlcMemOpenInternal(void)
{
    mcResult_t mcRet;
    mcVersionInfo_t versionInfo;
    uint8_t* pTrustletData = NULL;
    uint32_t nTrustletSize;

    LOG_I("Opening <t-base device");
    
    open_device_count++;
	
    if(tci_tl != NULL || open_device_count > 1){
        
        LOG_I("tlcMemOpen has been called!, ignore this time, open_device_count: %d", open_device_count);
        return MC_DRV_OK;
    }
    
    mcRet = mcOpenDevice(DEVICE_ID);
    
    if (MC_DRV_OK != mcRet) 
    {
        LOG_I("Error opening device: %d", mcRet);
        if (MC_DRV_ERR_DEVICE_ALREADY_OPEN == mcRet) {
            // leave it
            LOG_I("<t-base device already opened, leave it");
            //return MC_DRV_OK;
        }
        else {
            LOG_E("mcGetMobiCoreVersion failed %d", mcRet);
            mcCloseDevice(DEVICE_ID);
            open_device_count--;
            return mcRet;
        }
    }
    
    mcRet = mcGetMobiCoreVersion(MC_DEVICE_ID_DEFAULT, &versionInfo);
    LOG_I("productId        = %s", versionInfo.productId);
    LOG_I("versionMci       = 0x%08X", versionInfo.versionMci);
    LOG_I("versionSo        = 0x%08X", versionInfo.versionSo);
    LOG_I("versionMclf      = 0x%08X", versionInfo.versionMclf);
    LOG_I("versionContainer = 0x%08X", versionInfo.versionContainer);
    LOG_I("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
    LOG_I("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
    LOG_I("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
    LOG_I("versionCmp       = 0x%08X", versionInfo.versionCmp);

    tci_tl = (tciMessage_t*)malloc(sizeof(tciMessage_t));
    
    /* TCI interface, used to load temp TL for secmem test only */
    if (tci_tl == NULL) 
    {
        LOG_E("TCI failed");
        mcCloseDevice(DEVICE_ID);
        open_device_count--;
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    memset(tci_tl, 0, sizeof(tciMessage_t));

    /* Load the TL */
    nTrustletSize = getFileContent(
                        "/vendor/app/mcRegistry/08030000000000000000000000000000.tlbin",
                        &pTrustletData);                      
                        
    if(nTrustletSize == 0) 
    {
        LOG_E("Trustlet not found");
        free(tci_tl);
        tci_tl = NULL;
        mcCloseDevice(DEVICE_ID);
        open_device_count--;
        return MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    }

    LOG_I("Opening the session");
    memset(&sessionHandle_tci_tl, 0, sizeof(sessionHandle_tci_tl));
    sessionHandle_tci_tl.deviceId = DEVICE_ID; // The device ID (default device is used)
    mcRet = mcOpenTrustlet(
            &sessionHandle_tci_tl,
            MC_SPID_RESERVED_TEST, /* mcSpid_t */
            pTrustletData,
            nTrustletSize,
            (uint8_t *) tci_tl,
            sizeof(tciMessage_t));

    /* Whatever the result is, free the buffer */
    free(pTrustletData);

    if (MC_DRV_OK != mcRet) 
    {
        LOG_E("Open session failed: %d", mcRet);
        free(tci_tl);
        tci_tl = NULL;
        mcCloseDevice(DEVICE_ID);
        open_device_count--;
    }
    else 
    {
        LOG_I("open() succeeded");
    }

    return mcRet;
}

mcResult_t tlcMemOpen(void)
{
    mcResult_t ret = MC_DRV_OK;
    pthread_mutex_lock(&mem_mutex);
    ret = tlcMemOpenInternal();
    pthread_mutex_unlock(&mem_mutex);

    return ret;
}


// -------------------------------------------------------------
void tlcMemCloseInternal(void)
{
    mcResult_t ret = MC_DRV_OK;

    LOG_I("Closing the session");
    if(tci_tl == NULL || open_device_count == 0){
        LOG_I("Call tlcMemOpen first!  Ignore this time");
        LOG_I("open_device_count: %d", open_device_count);
    }
    else if(open_device_count > 1){
        open_device_count--;
        LOG_I("Can't really close trustlet, open_device_count: %d", open_device_count);

        return ;
    }
    else{
        open_device_count --;
        LOG_I("Close trustlet, open_device_count: %d", open_device_count);
        ret = mcCloseSession(&sessionHandle_tci_tl);
        if (MC_DRV_OK != ret) 
        {
            LOG_E("Closing session failed: %d", ret);
            /* continue even in case of error */
        }

        LOG_I("Closing <t-base device");
        ret = mcCloseDevice(DEVICE_ID);
        if (MC_DRV_OK != ret) 
        {
            LOG_E("Closing <t-base device failed: 0x%x", ret);
            /* continue even in case of error */;
        }
        free(tci_tl);
        tci_tl = NULL;
    }
}

void tlcMemClose(void)
{
    pthread_mutex_lock(&mem_mutex);
    tlcMemCloseInternal();
    pthread_mutex_unlock(&mem_mutex);
}
#endif

static mcResult_t tlcMemOpenInternal(void)
{
    mcResult_t ret = MC_DRV_OK;

	/* check if fd is already opened */
    open_device_count++;
    if(open_device_count > 1)
	{
        LOG_I("tlcMemOpenInternal: ignore this time, open_device_count: %d, fd: %d", open_device_count, secmem_fd);
        goto exit;
    }

	/* open the secure memory device */
	secmem_fd = open(SECMEM_DEV, O_RDWR, 0);
    if (secmem_fd < 0)
	{
		LOG_E("tlcMemOpenInternal: open device file failed for R/W: %d (%s)", errno, strerror(errno));

		secmem_fd = open(SECMEM_DEV, O_RDONLY, 0);
		if (secmem_fd < 0)
		{
			ret = MC_DRV_ERR_DEVICE_FILE_OPEN;
			open_device_count--;
			goto exit;
		}
		LOG_E("tlcMemOpenInternal: open device file for read only");
	}

	LOG_I("tlcMemOpenInternal: open secure memory driver success, open_device_count: %d, fd: %d", open_device_count, secmem_fd);

exit:

	return ret;
}

mcResult_t tlcMemOpen(void)
{
    mcResult_t ret = MC_DRV_OK;

    pthread_mutex_lock(&mem_mutex);

    LOG_I("tlcMemOpen: opening secure memory driver");

	ret = tlcMemOpenInternal();
	
exit:
    pthread_mutex_unlock(&mem_mutex);

    return ret;
}

void tlcMemClose(void)
{
    pthread_mutex_lock(&mem_mutex);

	/* close file description */
	LOG_I("tlcMemClose: closing secure memory driver");
	
    if(open_device_count == 0)
	{
        LOG_I("tlcMemClose: device is still not opened, open_device_count: %d", open_device_count);
    }
    else if(open_device_count > 1)
	{
        open_device_count--;
        LOG_I("tlcMemClose: can't really close the device, remained open_device_count: %d", open_device_count);
    }
    else
	{
        open_device_count--;
        LOG_I("tlcMemClose: close device, open_device_count: %d, fd: %d", open_device_count, secmem_fd);
        close(secmem_fd);
		secmem_fd = -1;
    }

    pthread_mutex_unlock(&mem_mutex);
}

static mcResult_t executeCmd(UREE_SECMEM_PARAM *sec_mem_param, unsigned int ioctl_cmd)
{
	mcResult_t ret = MC_DRV_OK;
	struct secmem_param_ioctl ioctl_param = {0};

	/* execute IOCTL commands */
	if(secmem_fd < 0)
	{
		LOG_E("executeCmd: device is still not opened: %d (try open)", secmem_fd);
		ret = MC_DRV_ERR_INVALID_DEVICE_FILE;
		if((ret = tlcMemOpenInternal()) != MC_DRV_OK)
		{
			LOG_E("executeCmd: try open failed, open_device_count: %d, fd: %d", open_device_count, secmem_fd);
			goto exit;
		}
	}

	ioctl_param.alignment = sec_mem_param->alignment;
	ioctl_param.refcount = sec_mem_param->refcount;
	ioctl_param.sec_handle = sec_mem_param->sec_handle;
	ioctl_param.size = sec_mem_param->size;
#ifdef SECMEM_TLC_DEBUG
    ioctl_param.id = sec_mem_param->id;
    if (sec_mem_param->owner) {
        char *owner = strrchr((char*)sec_mem_param->owner, '/');
        int len;
        owner = (owner == NULL)? (char*)sec_mem_param->owner : owner+1;
        len = strlen(owner) + 1;
        len = len > MAX_NAME_SZ ? MAX_NAME_SZ : len;
        ioctl_param.owner_len = len;
        memcpy(ioctl_param.owner, owner, len);
        ioctl_param.owner[MAX_NAME_SZ - 1] = '\0';
    } else {
        strcpy((char*)ioctl_param.owner,"Unknown");
        ioctl_param.owner_len = strlen((char*)ioctl_param.owner)+1;
    }
#else
    ioctl_param.id = 0;
    ioctl_param.owner_len = 0;
    ioctl_param.owner[0] = 0;
#endif

	if ((ret = ioctl(secmem_fd, ioctl_cmd, &ioctl_param)) != 0)
	{
		LOG_E("executeCmd: ioctl for command %d failed: %d (%s)", _IOC_NR(ioctl_cmd), ret, strerror(errno));
		ret = MC_DRV_ERR_INVALID_PARAMETER;
        goto exit;
	}

	/* get responses */
	sec_mem_param->alignment = ioctl_param.alignment;
	sec_mem_param->refcount = ioctl_param.refcount;
	sec_mem_param->sec_handle = ioctl_param.sec_handle;
	sec_mem_param->size = ioctl_param.size;

exit:

	return ret;
}

//------------------------------------
mcResult_t UREE_AllocSecuremem (UREE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id)
{
    mcResult_t ret = MC_DRV_OK;
    UREE_SECMEM_PARAM sec_mem_param;

    LOG_I("SEC mem perform allocation");
    pthread_mutex_lock(&mem_mutex);

    sec_mem_param.alignment = alignment;
    sec_mem_param.size = size;
    sec_mem_param.refcount = 0;
    sec_mem_param.sec_handle = 0;
#ifdef SECMEM_TLC_DEBUG
    sec_mem_param.id = id;
    sec_mem_param.owner = owner;
#else
    sec_mem_param.id = 0;
    sec_mem_param.owner = NULL;
#endif

    ret = executeCmd(&sec_mem_param, SECMEM_MEM_ALLOC);

    *mem_handle = sec_mem_param.sec_handle;

    if (ret != RET_OK) 
    {
        LOG_E("Unable to execute CMD_SEC_MEM_ALLOC command: %d", ret);
        goto exit;
    }

    LOG_I("The handle in secure world is (0x%x)", *mem_handle);

exit:
    pthread_mutex_unlock(&mem_mutex);
    return ret;
}

mcResult_t UREE_AllocSecurememZero (UREE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id)
{
    mcResult_t ret = MC_DRV_OK;
    UREE_SECMEM_PARAM sec_mem_param;

    LOG_I("SEC mem perform allocation (Zero)");
    pthread_mutex_lock(&mem_mutex);

    sec_mem_param.alignment = alignment | 0x1;	
    sec_mem_param.size = size;
    sec_mem_param.refcount = 0;
    sec_mem_param.sec_handle = 0;
#ifdef SECMEM_TLC_DEBUG
    sec_mem_param.id = id;
    sec_mem_param.owner = owner;
#else
    sec_mem_param.id = 0;
    sec_mem_param.owner = NULL;
#endif

    ret = executeCmd(&sec_mem_param, SECMEM_MEM_ALLOC);

    *mem_handle = sec_mem_param.sec_handle;

    if (ret != RET_OK) 
    {
        LOG_E("Unable to execute CMD_SEC_MEM_ALLOC command: %d", ret);
        goto exit;
    }

    LOG_I("The handle in secure world is (0x%x)", *mem_handle);

exit:
    pthread_mutex_unlock(&mem_mutex);
    return ret;
}

mcResult_t UREE_ReferenceSecuremem (UREE_SECUREMEM_HANDLE mem_handle, uint32_t *refcount, uint8_t *owner, uint32_t id)
{
    /* Need to implement whole Picture */
    mcResult_t ret = MC_DRV_OK;
    UREE_SECMEM_PARAM sec_mem_param;

    LOG_I("SEC mem perform Reference, handle(0x%x)", mem_handle);
    pthread_mutex_lock(&mem_mutex);

    sec_mem_param.alignment = 0;
    sec_mem_param.size = 0;
    sec_mem_param.refcount = 0;
    sec_mem_param.sec_handle = mem_handle;
#ifdef SECMEM_TLC_DEBUG
    sec_mem_param.id = id;
    sec_mem_param.owner = owner;
#else
    sec_mem_param.id = 0;
    sec_mem_param.owner = NULL;
#endif

    ret = executeCmd(&sec_mem_param, SECMEM_MEM_REF);

    *refcount = sec_mem_param.refcount;

    if (ret != RET_OK) 
    {
        LOG_E("Unable to execute CMD_SEC_MEM_REF command: %d", ret);
        goto exit;
    }

    LOG_I("The refcount is (0x%x)", *refcount);

exit:
    pthread_mutex_unlock(&mem_mutex);
    return ret;
}

mcResult_t UREE_UnreferenceSecuremem (UREE_SECUREMEM_HANDLE mem_handle, uint32_t *refcount, uint8_t *owner, uint32_t id)
{
    /* Need to implement whole Picture */
    mcResult_t ret = MC_DRV_OK;
    UREE_SECMEM_PARAM sec_mem_param;

    LOG_I("SEC mem perform Unreference, handle(0x%x)", mem_handle);
    pthread_mutex_lock(&mem_mutex);

    sec_mem_param.alignment = 0;
    sec_mem_param.size = 0;
    sec_mem_param.refcount = 0;
    sec_mem_param.sec_handle = mem_handle;
#ifdef SECMEM_TLC_DEBUG
    sec_mem_param.id = id;
    sec_mem_param.owner = owner;
#else
    sec_mem_param.id = 0;
    sec_mem_param.owner = NULL;
#endif

    ret = executeCmd(&sec_mem_param, SECMEM_MEM_UNREF);

    *refcount = sec_mem_param.refcount;

    if (ret != RET_OK)
    {
        LOG_E("Unable to execute CMD_SEC_MEM_UNREF command: %d", ret);
        goto exit;
    }

    LOG_I("The refcount is (0x%x)", *refcount);

exit:
    pthread_mutex_unlock(&mem_mutex);
    return ret;
}

mcResult_t UREE_AllocSecurememTBL (UREE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id)
{
    mcResult_t ret = MC_DRV_OK;
    UREE_SECMEM_PARAM sec_mem_param;

    LOG_I("SEC mem perform allocation with TBL");
    pthread_mutex_lock(&mem_mutex);

    sec_mem_param.alignment = alignment;
    sec_mem_param.size = size;
    sec_mem_param.refcount = 0;
    sec_mem_param.sec_handle = 0;
#ifdef SECMEM_TLC_DEBUG
    sec_mem_param.id = id;
    sec_mem_param.owner = owner;
#else
    sec_mem_param.id = 0;
    sec_mem_param.owner = NULL;
#endif

    ret = executeCmd(&sec_mem_param, SECMEM_MEM_ALLOC_TBL);

    *mem_handle = sec_mem_param.sec_handle;

    if (ret != RET_OK)
    {
        LOG_E("Unable to execute CMD_SEC_MEM_ALLOC_TBL command: %d", ret);
        goto exit;
    }

    LOG_I("The handle in secure world is (0x%x)", *mem_handle);

exit:
    pthread_mutex_unlock(&mem_mutex);
    return ret;
}

mcResult_t UREE_AllocSecurememTBLZero (UREE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, uint8_t *owner, uint32_t id)
{
    mcResult_t ret = MC_DRV_OK;
    UREE_SECMEM_PARAM sec_mem_param;

    LOG_I("SEC mem perform allocation with TBL (Zero)");
    pthread_mutex_lock(&mem_mutex);

    sec_mem_param.alignment = alignment | 0x1;
    sec_mem_param.size = size;
    sec_mem_param.refcount = 0;
    sec_mem_param.sec_handle = 0;
#ifdef SECMEM_TLC_DEBUG
    sec_mem_param.id = id;
    sec_mem_param.owner = owner;
#else
    sec_mem_param.id = 0;
    sec_mem_param.owner = NULL;
#endif

    ret = executeCmd(&sec_mem_param, SECMEM_MEM_ALLOC_TBL);

    *mem_handle = sec_mem_param.sec_handle;

    if (ret != RET_OK)
    {
        LOG_E("Unable to execute CMD_SEC_MEM_ALLOC_TBL command: %d", ret);
        goto exit;
    }

    LOG_I("The handle in secure world is (0x%x)", *mem_handle);

exit:
    pthread_mutex_unlock(&mem_mutex);
    return ret;
}

mcResult_t UREE_UnreferenceSecurememTBL (UREE_SECUREMEM_HANDLE mem_handle, uint32_t *refcount, uint8_t *owner, uint32_t id)
{
    /* Need to implement whole Picture */
    mcResult_t ret = MC_DRV_OK;
    UREE_SECMEM_PARAM sec_mem_param;

    LOG_I("SEC mem perform Unreference with TBL, handle(0x%x)", mem_handle);
    pthread_mutex_lock(&mem_mutex);

    sec_mem_param.alignment = 0;
    sec_mem_param.size = 0;
    sec_mem_param.refcount = 0;
    sec_mem_param.sec_handle = mem_handle;
#ifdef SECMEM_TLC_DEBUG
    sec_mem_param.id = id;
    sec_mem_param.owner = owner;
#else
    sec_mem_param.id = 0;
    sec_mem_param.owner = NULL;
#endif

    ret = executeCmd(&sec_mem_param, SECMEM_MEM_UNREF_TBL);

    *refcount = sec_mem_param.refcount;

    if (ret != RET_OK)
    {
        LOG_E("Unable to execute CMD_SEC_MEM_UNREF_TBL command: %d", ret);
        goto exit;
    }

    LOG_I("The refcount is (0x%x)", *refcount);

exit:
    pthread_mutex_unlock(&mem_mutex);
    return ret;
}


mcResult_t UREE_DumpSecurememInfo(void)
{
    mcResult_t ret = MC_DRV_OK;
    UREE_SECMEM_PARAM sec_mem_param;

    memset(&sec_mem_param, 0, sizeof(UREE_SECMEM_PARAM));

    pthread_mutex_lock(&mem_mutex);

    ret = executeCmd(&sec_mem_param, SECMEM_MEM_DUMP_INFO);

exit:
    pthread_mutex_unlock(&mem_mutex);
    return ret;
}



#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <tz_cross/trustzone.h>
#include <tz_cross/ta_mem.h>
#include <uree/system.h>
#include <uree/mem.h>
#include <kree/tz_mod.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <log/log.h>

static inline TZ_RESULT _allocFunc(uint32_t cmd,
                                   UREE_SESSION_HANDLE session,
                                   uint32_t *mem_handle,
                                   uint32_t alignment,
                                   uint32_t size,
                                   char *dbg,
                                   const char *tag) {
    MTEEC_PARAM p[4];
    TZ_RESULT ret;
    uint32_t mhndl = 0;

    if ((mem_handle == NULL) || (size == 0)) {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    p[0].value.a = alignment;
    p[1].value.a = size;
    switch(cmd) {
    case TZCMD_MEM_SECUREMEM_ALLOC:
        ret = UREE_TeeServiceCall(session, cmd,
                TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT), p);
        mhndl = p[2].value.a;
    break;
    case TZCMD_MEM_SECUREMEM_ALLOC_WITH_TAG:
    case TZCMD_MEM_SECUREMEM_ZALLOC_WITH_TAG:
        p[2].mem.buffer = (char *)tag;
        p[2].mem.size = strlen(tag)+1;
        ret = UREE_TeeServiceCall(session, cmd,
                TZ_ParamTypes4(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT, TZPT_MEM_INPUT, TZPT_VALUE_OUTPUT), p);
        mhndl = p[3].value.a;
    break;

    default:
        ret = TZ_RESULT_ERROR_BAD_PARAMETERS;
    }
    if (ret != TZ_RESULT_SUCCESS) {
#ifdef DBG_UREE_MEM
        printf("[uree] %s Error: %d\n", dbg, ret);
#endif
        return ret;
    }

    *mem_handle = (UREE_SECUREMEM_HANDLE)mhndl;

    return TZ_RESULT_SUCCESS;
}

static inline TZ_RESULT _handleOpFunc(uint32_t cmd, UREE_SESSION_HANDLE session, uint32_t mem_handle, char *dbg) {
    MTEEC_PARAM p[4];
    TZ_RESULT ret;

    if (mem_handle == 0) {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    p[0].value.a = (uint32_t) mem_handle;
    ret = UREE_TeeServiceCall(session, cmd,
            TZ_ParamTypes1(TZPT_VALUE_INPUT), p);
    if (ret < 0) {
#ifdef DBG_UREE_MEM
        printf("[uree] %s Error: %d\n", dbg, ret);
#endif
        return ret;
    }

    return TZ_RESULT_SUCCESS;
}

static inline TZ_RESULT _handleOpFunc_1(uint32_t cmd,
                    UREE_SESSION_HANDLE session,
                    uint32_t mem_handle,
                    uint32_t *count, char *dbg) {
    MTEEC_PARAM p[4];
    TZ_RESULT ret;

    if ((mem_handle == 0) || (count == NULL)) {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    p[0].value.a = (uint32_t) mem_handle;
    ret = UREE_TeeServiceCall(session, cmd,
            TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT), p);
    if (ret < 0) {
#ifdef DBG_UREE_MEM
        printf("[uree] %s Error: %d\n", dbg, ret);
#endif
        *count = 0;
        return ret;
    }

    *count = p[1].value.a;

    return TZ_RESULT_SUCCESS;
}


/* APIs
*/

TZ_RESULT UREE_AllocSecuremem(UREE_SESSION_HANDLE session,
    UREE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size) {
    TZ_RESULT ret;

    ret = _allocFunc(TZCMD_MEM_SECUREMEM_ALLOC, session, mem_handle, alignment, size, "UREE_AllocSecuremem", NULL);

    return ret;
}

TZ_RESULT UREE_AllocSecurememWithTag(UREE_SESSION_HANDLE session,
    UREE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, const char *tag) {
    TZ_RESULT ret;

    ret = _allocFunc(TZCMD_MEM_SECUREMEM_ALLOC_WITH_TAG, session, mem_handle, alignment, size, "UREE_AllocSecuremem", tag);

    return ret;
}

TZ_RESULT UREE_ZallocSecurememWithTag(UREE_SESSION_HANDLE session,
    UREE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, const char *tag) {
    TZ_RESULT ret;

	//fix me!!!
    ret = _allocFunc(TZCMD_MEM_SECUREMEM_ZALLOC_WITH_TAG, session, mem_handle, alignment, size, "UREE_ZallocSecuremem", tag);

    return ret;
}

TZ_RESULT UREE_ReferenceSecuremem(UREE_SESSION_HANDLE session, UREE_SECUREMEM_HANDLE mem_handle) {
    TZ_RESULT ret;

    ret = _handleOpFunc(TZCMD_MEM_SECUREMEM_REF, session, mem_handle, "UREE_ReferenceSecuremem");

    return ret;
}

TZ_RESULT UREE_UnreferenceSecuremem(UREE_SESSION_HANDLE session, UREE_SECUREMEM_HANDLE mem_handle) {
    TZ_RESULT ret;
    uint32_t count = 0;

    ret = _handleOpFunc_1(TZCMD_MEM_SECUREMEM_UNREF, session, mem_handle, &count, "UREE_UnreferenceSecuremem");
#ifdef DBG_UREE_MEM
    printf("UREE_UnreferenceSecuremem: count = 0x%x\n", count);
#endif

    return ret;
}


static inline TZ_RESULT _allocchmFunc(uint32_t cmd,
				UREE_SESSION_HANDLE session,
				UREE_SHAREDMEM_HANDLE chm_handle,
				UREE_SECUREMEM_HANDLE *mem_handle,
				uint32_t alignment, uint32_t size, char *dbg, const char *tag)
{
	MTEEC_PARAM p[4];
	TZ_RESULT ret;

	if ((chm_handle == 0) || (mem_handle == NULL) || (size == 0)) {
		ALOGE("_allocchmFunc: invalid parameters\n");
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}

	p[0].value.a = alignment;
	p[1].value.a = size;
	p[1].value.b = chm_handle;
	switch(cmd){
	case TZCMD_MEM_SECUREMULTICHUNKMEM_ALLOC:
	case TZCMD_MEM_SECUREMULTICHUNKMEM_ZALLOC:
		ret = UREE_TeeServiceCall(session, cmd,
					TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT), p);
		*mem_handle = (UREE_SECUREMEM_HANDLE) p[2].value.a;

		break;
	case TZCMD_MEM_SECUREMULTICHUNKMEM_ALLOC_WITH_TAG:
	case TZCMD_MEM_SECUREMULTICHUNKMEM_ZALLOC_WITH_TAG:
		p[2].mem.buffer = (char *)tag;
		p[2].mem.size = strlen(tag)+1;
		ret = UREE_TeeServiceCall(session, cmd,
					TZ_ParamTypes4(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT, TZPT_MEM_INPUT, TZPT_VALUE_OUTPUT), p);
		*mem_handle = (UREE_SECUREMEM_HANDLE) p[3].value.a;

		break;
		
	default:
        ret = TZ_RESULT_ERROR_BAD_PARAMETERS;
	}

	if (ret != TZ_RESULT_SUCCESS) {
		ALOGE("%s Error: 0x%x\n", dbg, ret);
		return ret;
	}
	
	ALOGI("Alloc ret mem_handle=0x%x\n", *mem_handle);

	if (*mem_handle == 0) {
		ALOGE("[%d]Alloc chmem=NULL: mem_handle=0x%x\n", __LINE__, *mem_handle);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}



TZ_RESULT UREE_AllocSecureMultichunkmem(UREE_SESSION_HANDLE session,
				UREE_SHAREDMEM_HANDLE chm_handle,
				UREE_SECUREMEM_HANDLE *mem_handle,
				uint32_t alignment, uint32_t size) 
{
	TZ_RESULT ret;

	ret = _allocchmFunc(TZCMD_MEM_SECUREMULTICHUNKMEM_ALLOC,
			session, chm_handle, mem_handle,
			alignment, size, "UREE_Mem_AllocSecureMultichunkmem", NULL);

	ALOGI ("[%d]after _allocchmFunc mem_handle=0x%x\n",
							__LINE__, *mem_handle);

	if (ret != TZ_RESULT_SUCCESS) {
		ALOGE("UREE_Mem_AllocSecureMultichunkmem Error: 0x%x\n", ret);
	}

	return ret;
}

TZ_RESULT UREE_ZallocSecureMultichunkmem(UREE_SESSION_HANDLE session,
				UREE_SHAREDMEM_HANDLE chm_handle,
				UREE_SECUREMEM_HANDLE *mem_handle,
				uint32_t alignment, uint32_t size)
{
	TZ_RESULT ret;

	ret = _allocchmFunc(TZCMD_MEM_SECUREMULTICHUNKMEM_ZALLOC,
			session, chm_handle, mem_handle,
			alignment, size, "UREE_Mem_ZallocSecureMultichunkmem", NULL);

	if (ret != TZ_RESULT_SUCCESS) {
		ALOGE("UREE_Mem_ZallocSecureMultichunkmem Error: 0x%x\n", ret);
	}

	return ret;
}

TZ_RESULT UREE_AllocSecureMultichunkmemWithTag(UREE_SESSION_HANDLE session,
				UREE_SHAREDMEM_HANDLE chm_handle,
				UREE_SECUREMEM_HANDLE *mem_handle,
				uint32_t alignment, uint32_t size, const char *tag) 
{
	TZ_RESULT ret;

	ret = _allocchmFunc(TZCMD_MEM_SECUREMULTICHUNKMEM_ALLOC_WITH_TAG,
			session, chm_handle, mem_handle,
			alignment, size, "UREE_Mem_AllocSecureMultichunkmem", tag);

	ALOGI ("[%d]after _allocchmFunc mem_handle=0x%x\n",
							__LINE__, *mem_handle);

	if (ret != TZ_RESULT_SUCCESS) {
		ALOGE("UREE_Mem_AllocSecureMultichunkmem Error: 0x%x\n", ret);
	}

	return ret;
}

TZ_RESULT UREE_ZallocSecureMultichunkmemWithTag(UREE_SESSION_HANDLE session,
				UREE_SHAREDMEM_HANDLE chm_handle,
				UREE_SECUREMEM_HANDLE *mem_handle,
				uint32_t alignment, uint32_t size, const char *tag)
{
	TZ_RESULT ret;

	ret = _allocchmFunc(TZCMD_MEM_SECUREMULTICHUNKMEM_ZALLOC_WITH_TAG,
			session, chm_handle, mem_handle,
			alignment, size, "UREE_Mem_ZallocSecureMultichunkmem", tag);

	if (ret != TZ_RESULT_SUCCESS) {
		ALOGE("UREE_Mem_ZallocSecureMultichunkmem Error: 0x%x\n", ret);
	}

	return ret;
}


TZ_RESULT UREE_ReferenceSecureMultichunkmem(UREE_SESSION_HANDLE session,
					UREE_SECUREMEM_HANDLE mem_handle)
{
	MTEEC_PARAM p[4];
	TZ_RESULT ret;

	if (mem_handle == 0) {
		ALOGE("%s: invalid parameters\n", __FUNCTION__);
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}

	p[0].value.a = mem_handle;
	ret = UREE_TeeServiceCall(session, TZCMD_MEM_SECUREMULTICHUNKMEM_REF,
					TZ_ParamTypes1(TZPT_VALUE_INPUT), p);
	if (ret != TZ_RESULT_SUCCESS)
		ALOGE("%s Error: 0x%x\n", __FUNCTION__, ret);

	return ret;
}

TZ_RESULT UREE_UnreferenceSecureMultichunkmem(UREE_SESSION_HANDLE session,
					UREE_SECUREMEM_HANDLE mem_handle)
{
	TZ_RESULT ret;
	uint32_t count = 0;
	MTEEC_PARAM p[4];

	if (mem_handle == 0) {
		ALOGE("%s: invalid parameters\n", __FUNCTION__);
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}

	p[0].value.a = mem_handle;

	ret = UREE_TeeServiceCall(session, TZCMD_MEM_SECUREMULTICHUNKMEM_UNREF,
					TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT), p);

	count = p[1].value.a;

#ifdef DBG_KREE_MEM
	ALOGI("UREE_Mem_UnreferenceSecureMultichunkmem: count = 0x%x\n", count);
#endif

	if (ret != TZ_RESULT_SUCCESS) {
		ALOGE("UREE_Mem_UnreferenceSecureMultichunkmem Error: 0x%x\n", ret);
	}

	return ret;
}

TZ_RESULT UREE_ReferenceSecurechunkmem(UREE_SESSION_HANDLE session, UREE_SECURECM_HANDLE cm_handle) {
    TZ_RESULT ret;


    ret = _handleOpFunc(TZCMD_MEM_SECURECM_REF, session, cm_handle, "UREE_ReferenceSecurechunkmem");

    return ret;
}

TZ_RESULT UREE_UnreferenceSecurechunkmem(UREE_SESSION_HANDLE session, UREE_SECURECM_HANDLE cm_handle) {
    TZ_RESULT ret;
    uint32_t count = 0;

    ret = _handleOpFunc_1(TZCMD_MEM_SECURECM_UNREF, session, cm_handle, &count, "UREE_UnreferenceSecurechunkmem");
#ifdef DBG_UREE_MEM
        printf("UREE_UnreferenceSecurechunkmem: count = 0x%x\n", count);
#endif

    return ret;
}

TZ_RESULT UREE_ReleaseSecurechunkmem(UREE_SESSION_HANDLE session, uint32_t *size) {
    MTEEC_PARAM p[4] = {{{0}}};
    TZ_RESULT ret;

    if (session == 0) {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    ret = UREE_TeeServiceCall(session, TZCMD_MEM_SECURECM_RELEASE, TZ_ParamTypes1(TZPT_VALUE_OUTPUT), p);
    if (ret != TZ_RESULT_SUCCESS) {
#ifdef DBG_UREE_MEM
        printf("[uree] UREE_ReleaseSecurechunkmem Error: %d\n", ret);
#endif
        return ret;
    }

    *size = p[0].value.a;

    return TZ_RESULT_SUCCESS;
}

TZ_RESULT UREE_AppendSecurechunkmem(UREE_SESSION_HANDLE session) {
    MTEEC_PARAM p[4];
    TZ_RESULT ret;

    if (session == 0) {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    ret = UREE_TeeServiceCall(session, TZCMD_MEM_SECURECM_APPEND, 0, p);
    if (ret != TZ_RESULT_SUCCESS) {
#ifdef DBG_UREE_MEM
        printf("[uree] UREE_ReleaseSecurechunkmem Error: %d\n", ret);
#endif
        return ret;
    }

    return TZ_RESULT_SUCCESS;
}

TZ_RESULT UREE_ReadSecurechunkmem(UREE_SESSION_HANDLE session, uint32_t offset, uint32_t size, void *buffer) {
    MTEEC_PARAM p[4];
    TZ_RESULT ret;

    if ((session == 0) || (size == 0)) {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    p[0].value.a = offset;
    p[1].value.a = size;
    p[2].mem.buffer = buffer;
    p[2].mem.size = size;  // fix me!!!!
    ret = UREE_TeeServiceCall(session, TZCMD_MEM_SECURECM_READ,
            TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT, TZPT_MEM_OUTPUT), p);
    if (ret != TZ_RESULT_SUCCESS) {
#ifdef DBG_UREE_MEM
        printf("[uree] UREE_ReadSecurechunkmem Error: %d\n", ret);
#endif
        return ret;
    }

    return TZ_RESULT_SUCCESS;
}

TZ_RESULT UREE_WriteSecurechunkmem(UREE_SESSION_HANDLE session, uint32_t offset, uint32_t size, void *buffer) {
    MTEEC_PARAM p[4];
    TZ_RESULT ret;

    if ((session == 0) || (size == 0)) {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    p[0].value.a = offset;
    p[1].value.a = size;
    p[2].mem.buffer = buffer;
    p[2].mem.size = size;  // fix me!!!!
    ret = UREE_TeeServiceCall(session, TZCMD_MEM_SECURECM_WRITE,
            TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT, TZPT_MEM_INPUT), p);
    if (ret != TZ_RESULT_SUCCESS) {
#ifdef DBG_UREE_MEM
        printf("[uree] UREE_WriteSecurechunkmem Error: %d\n", ret);
#endif
        return ret;
    }

    return TZ_RESULT_SUCCESS;
}


typedef struct {
    pthread_mutex_t mutex;
    UREE_SESSION_HANDLE session;
    uint32_t control;
} mtee_tzmem_t;

static mtee_tzmem_t mtee_tzmemInfo = {PTHREAD_MUTEX_INITIALIZER, 0, 0};

TZ_RESULT UREE_ReleaseTzmem(int *fd_p) {
    int fd;
    int ret = TZ_RESULT_SUCCESS;
    UREE_SESSION_HANDLE session;
    uint32_t size;

    if (fd_p == NULL) {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    pthread_mutex_lock(&mtee_tzmemInfo.mutex);

    if (mtee_tzmemInfo.control != 0) {
        ret = TZ_RESULT_ERROR_GENERIC;
        goto out;
    }

    ret = UREE_CreateSession(TZ_TA_MEM_UUID, &session);
    if (ret != TZ_RESULT_SUCCESS) {
        printf("UREE_ReleaseTzmem: CreateSession Error = %s\n", TZ_GetErrorString(ret));
        goto out;
    }

    ret = UREE_ReleaseSecurechunkmem(session, &size);
    if (ret != TZ_RESULT_SUCCESS) {
        ret = TZ_RESULT_ERROR_GENERIC;
        goto out_release_fail;
    }

    fd = open("/dev/tzmem", O_RDWR);
    if (fd < 0) {
        ret = TZ_RESULT_ERROR_GENERIC;
        goto out_release_fail;
    }
    else {
        *fd_p = fd;
    }

    mtee_tzmemInfo.session = session;
    mtee_tzmemInfo.control = 1;

    pthread_mutex_unlock(&mtee_tzmemInfo.mutex);
    return ret;

out_release_fail:
    UREE_CloseSession(session);
out:
    pthread_mutex_unlock(&mtee_tzmemInfo.mutex);

    return ret;
}

TZ_RESULT UREE_AppendTzmem(int fd) {
    int ret = TZ_RESULT_SUCCESS;
    UREE_SESSION_HANDLE session;

    if (fd == 0) {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    pthread_mutex_lock(&mtee_tzmemInfo.mutex);

    if (mtee_tzmemInfo.control == 0) {
        ret = TZ_RESULT_ERROR_GENERIC;
        goto out;
    }

    close(fd);

    session = mtee_tzmemInfo.session;
    ret = UREE_AppendSecurechunkmem(session);
    if (ret != TZ_RESULT_SUCCESS) {
        ret = TZ_RESULT_ERROR_GENERIC;
    }

    ret = UREE_CloseSession(session);
    if (ret != TZ_RESULT_SUCCESS) {
        printf("UREE_AppendTzmem: UREE_CloseSession Error = %s\n", TZ_GetErrorString(ret));
        goto out;
    }

    mtee_tzmemInfo.control = 0;

out:
    pthread_mutex_unlock(&mtee_tzmemInfo.mutex);
    return ret;
}



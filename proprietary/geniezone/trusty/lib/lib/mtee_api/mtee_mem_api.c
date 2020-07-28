/*
 * Copyright (C) 2012-2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <trusty_std.h>

#include <lib/mtee/mtee_mem_srv.h>
#include <tz_private/ta_sys_mem.h>
#include <tz_private/log.h>
#include <tz_cross/ta_test.h>

//#define MTEE_MEM_API_LOG(fmt...) printf("[MTEE MEM API] "fmt)
#define LOG_TAG "MTEE MEM API"

// fix me!!!, need init
static MTEE_HANDLE_TYPE_ID _TaShmHandleTypeId;


TZ_RESULT ioctl_mem(const char *tag, enum MTEE_MEM_USER_CMD cmd, void *ptr)
{
	int rc;

	rc = ioctl(SYS_MEM_SRV_FD, cmd, ptr);
	if (rc != NO_ERROR) {
		ERR_LOG("ioctl %s failed. rc = 0x%x\n", tag, rc);
		return TZ_RESULT_ERROR_GENERIC;
	}
	return TZ_RESULT_SUCCESS;
}


/**************** User Space Memory APIs *******************/
static void *TA_Mem_AllocMem_Body(MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, int zalloc, char *tag)
{
	secure_mem_t mem_param;
	TZ_RESULT tz_ret;

	mem_param.alignment = alignment;
	mem_param.size = size;

	mem_param.tag=  (uint64_t)tag;
	if (zalloc) {
		if(tag) tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMEM_ZALLOC_WITH_TAG, &mem_param);
		else	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMEM_ZALLOC, &mem_param);
	} else {
		if(tag) tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMEM_ALLOC_WITH_TAG, &mem_param);
		else	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMEM_ALLOC, &mem_param);
	}
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
		return tz_ret;
		//return NULL;
	}
	//DBG_LOG("ioctl return: handle=0x%x, size=%d, align=%d, addr=%p \n", mem_param.mem_handle, mem_param.size, mem_param.alignment, mem_param.addr);

	*mem_handle = mem_param.mem_handle;

	return mem_param.addr;
}

void *TA_Mem_AllocMem(MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size)
{
	if(alignment < 0) {
		ERR_LOG("Invalid alignment value: %d. \n", alignment);
		return NULL;
	}
	if(size <= 0) {
		ERR_LOG("Invalid size value: %d. \n", size);
		return NULL;
	}

	return TA_Mem_AllocMem_Body(mem_handle, alignment, size, 0, NULL);
}

void *TA_Mem_AllocMemWithTag(MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, char *tag)
{
    return TA_Mem_AllocMem_Body(mem_handle, alignment, size, 0, tag);
}
void *TA_Mem_ZallocMem(MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size)
{
	if(alignment < 0) {
		ERR_LOG("Invalid alignment value: %d. \n", alignment);
		return NULL;
	}
	if(size <= 0) {
		ERR_LOG("Invalid size value: %d. \n", size);
		return NULL;
	}

	return TA_Mem_AllocMem_Body(mem_handle, alignment, size, 1, NULL);
}
void *TA_Mem_ZallocMemWithTag(MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, char *tag)
{
    return TA_Mem_AllocMem_Body(mem_handle, alignment, size, 1, tag);
}

void *TA_Mem_ReferenceMem (MTEE_SECUREMEM_HANDLE handle)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	mem_param.alignment = 0;
	mem_param.size = 0;
	mem_param.mem_handle= handle;
	if (handle == 0)
    {
    	ERR_LOG("Invalid memory handle. \n");
        return NULL;
    }
	//DBG_LOG("ioctl: handle=0x%x \n", mem_param.mem_handle);
	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMEM_REF, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("returned by ioctl_mem: 0x%x. \n", tz_ret);
		return NULL;
	}
	//DBG_LOG("ioctl return: handle=0x%x, addr=%p \n", mem_param.mem_handle, mem_param.addr);

	return mem_param.addr;
}

int TA_Mem_UnreferenceMem (MTEE_SECUREMEM_HANDLE handle)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	if (handle == 0)
    {
    	ERR_LOG("ERROR: Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.alignment = 0;
	mem_param.size = 0;
	mem_param.mem_handle= handle;

	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMEM_UNREF, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("returned by ioctl_mem: 0x%x. \n", tz_ret);
		return -1; // return ref_count -1 to indicate error
	}
	//DBG_LOG("ioctl return: ref_count=%d \n", mem_param.ref_count);

	return mem_param.ref_count;
}

TZ_RESULT TA_Mem_QueryMem (uint32_t handle, MTEE_MEM_PARAM *param)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	if (handle == 0)
    {
    	ERR_LOG("Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.mem_handle= handle;

    //DBG_LOG("memory handle: %d \n", mem_param.mem_handle);
	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMEM_QUERY, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
		return tz_ret;
	}

	//DBG_LOG("Return param: addr=0x%x, size=0x%x \n", mem_param.addr, mem_param.size);

	param->buffer = (void *)mem_param.addr;
	param->size = mem_param.size;

    return TZ_RESULT_SUCCESS;
}

TZ_RESULT TA_Mem_QuerySharedmem_body(MTEE_SHAREDMEM_HANDLE handle, MTEE_SHAREDMEM_PARAM *param, uint32_t cmd)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	if (handle == 0)
    {
    	ERR_LOG("Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.mem_handle= handle;

    //DBG_LOG("memory handle: %d \n", mem_param.mem_handle);
    if (cmd == (uint32_t) TZCMD_SHARED_MEM_TEST)
		tz_ret = ioctl_mem("mem", MTEE_MEM_SHAREDMEM_QUERY, &mem_param);

	if(tz_ret != TZ_RESULT_SUCCESS) {
			ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
			return tz_ret;
	}

	//DBG_LOG("Return param: addr=0x%x, size=%x \n", mem_param.addr, mem_param.size);

	param->buffer = (void *)mem_param.addr;
	param->size = mem_param.size;
	param->pa = mem_param.paddr;


    return TZ_RESULT_SUCCESS;

}


#if 0
static void *TA_Mem_AllocChunkmem_Body(MTEE_SECURECM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, char *tag)
{
    void *buffer;
    MTEE_MEM_OBJ *obj;
    MTEE_SECURECM_HANDLE handle;
    MTEE_HANDLE_TYPE_ID htid = _TaMemHandleTypeId;
#ifndef USER_BUILD
    char str[STRLEN_OF_TAG];
#endif

    *mem_handle = 0; //reset handle

    // allocate
    buffer = MTEE_AllocChunkmemAlignWithTag(alignment, size, tag);
    if (buffer == NULL)
    {
        MTEE_LOG(MTEE_LOG_LVL_BUG, "AllocChunkMem Fail!!!, try VirtMapped one \n");
        buffer = TA_Mem_AllocVirtMappedmemAlign(alignment, size);

        if (buffer == NULL)
            return NULL;
    }

    // create object
#ifndef USER_BUILD
    snprintf(str, STRLEN_OF_TAG, "CM_OBJ(%s)", tag);
    obj = (MTEE_MEM_OBJ *) TA_MEM_AllocMemWithTag(sizeof (MTEE_MEM_OBJ), str);
#else
    obj = (MTEE_MEM_OBJ *) TA_MEM_AllocMemWithTag(sizeof (MTEE_MEM_OBJ), "CM_OBJ");
#endif
    if (obj == NULL)
    {
        MTEE_FreeChunkmem (buffer);
        return NULL;
    }

    obj->buffer = buffer;
    obj->size = size;
    obj->alignment = alignment;
    obj->reference = 1;
    obj->extra_info = NULL;

    // get handle
    if (MTEE_HandleAllocateWithTag(htid, obj, &handle, tag) != TZ_RESULT_SUCCESS)
    {
        TA_MEM_FreeMem (obj);
        MTEE_FreeChunkmem (buffer);
        return NULL;
    }

    *mem_handle = handle;

#ifdef DBG_TA_MEM
    ta_memCmCount ++;
    printf ("===> TA_Mem_AllocChunkmem: buffer = %p, count = %d\n", obj->buffer, ta_memCmCount);
#endif
    return buffer;
}

void *TA_Mem_AllocChunkmem(MTEE_SECURECM_HANDLE *mem_handle, uint32_t alignment, uint32_t size)
{
    return TA_Mem_AllocChunkmem_Body(mem_handle, alignment, size, NULL);
}

#endif


TZ_RESULT TA_Mem_QuerySharedmem (MTEE_SHAREDMEM_HANDLE handle, MTEE_SHAREDMEM_PARAM *param)
{
	return TA_Mem_QuerySharedmem_body(handle, param, TZCMD_SHARED_MEM_TEST);
}


static void *TA_Mem_RegisterSharedMem_Body (MTEE_SHAREDMEM_HANDLE *shm_handle, MTEE_SHAREDMEM *param, const char *tag)
{
	TZ_RESULT tz_ret;
	//shared_mem_t mem_param;
	secure_mem_t mem_param;

	mem_param.paddr = param->pa;
	mem_param.size = param->size;
	mem_param.region_id = param->region_id;

	mem_param.mmuTable = param->mmuTable;

	//DBG_LOG("[%s] Set ioctl param: PA=%d\n", __FUNCTION__, mem_param.paddr);

	tz_ret = ioctl_mem("mem", MTEE_MEM_SHAREDMEM_REG, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
			ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
			return tz_ret;
	}

	//DBG_LOG("Return ioctl param: size=%d, align=%d, handle=%d, addr=%p \n", mem_param.size, mem_param.alignment, mem_param.mem_handle, mem_param.addr);

	*shm_handle = mem_param.mem_handle;

	return mem_param.addr;

}

static void setshmem_param(shared_mem_t *mem_param, MTEE_SHAREDMEM *param)
{
	(*mem_param).paddr = param->pa;
	(*mem_param).size = param->size;
	(*mem_param).mmuTable = param->mmuTable;
}

static void *MTEE_AppendMultiChunkmem_Body(MTEE_SHAREDMEM_HANDLE *chm_handle, MTEE_SHAREDMEM *param, const char *tag, uint32_t cmd)
{
	TZ_RESULT tz_ret;


	//shared_mem_t mem_param;
	secure_mem_t mem_param;

	mem_param.paddr = param->pa;
	mem_param.size = param->size;
	mem_param.region_id = param->region_id;
	mem_param.mmuTable = param->mmuTable;

	//DBG_LOG("[%s] Set ioctl param: PA=%d\n", __FUNCTION__, mem_param.paddr);


	//for (int m = 0;m < (mem_param.size/PAGE_SIZE); m++) {
	//		printf("mem_param.mmuTable[%d]=0x%x\n", m, mem_param.mmuTable[m]);
	//}

	if (cmd == (uint32_t) TZCMD_MEM_APPEND_MULTI_CHUNKMEM_ION) /*proprietary for ION*/
		tz_ret = ioctl_mem("mem", MTEE_MEM_APPEND_MULTI_CHUNKMEM_ION, &mem_param);
	else
		tz_ret = ioctl_mem("mem", MTEE_MEM_APPEND_MULTI_CHUNKMEM, &mem_param);	/*default: general case*/

	if(tz_ret != TZ_RESULT_SUCCESS) {
			ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
			return tz_ret;
	}

	//DBG_LOG("Return ioctl param: size=%d, align=%d, handle=%d, addr=%p \n", mem_param.size, mem_param.alignment, mem_param.mem_handle, mem_param.addr);

	*chm_handle = mem_param.chm_mem_handle;

	return mem_param.addr;	/*NULL*/

}

void *MTEE_AppendMultiChunkmem(MTEE_SHAREDMEM_HANDLE *chm_handle, MTEE_SHAREDMEM *param, uint32_t cmd)
{
	if(param->size <= 0) {
		ERR_LOG("Invalid size value: %d. \n", param->size);
		return NULL;
	}

    return MTEE_AppendMultiChunkmem_Body(chm_handle, param, NULL, cmd);
}

/*chunk memory related APIs*/
void *TA_Mem_AllocMultiChunkmem_Body(MTEE_SHAREDMEM_HANDLE chm_handle, MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, int zalloc, char *tag, uint64_t in_key)
{
	secure_mem_t mem_param;
	TZ_RESULT tz_ret;

	mem_param.alignment = alignment;
	mem_param.size = size;
	mem_param.chm_mem_handle = chm_handle;
	mem_param.key = in_key;

	if (zalloc) {
		tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_ZALLOC, &mem_param);
	} else {
		tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_ALLOC, &mem_param);
	}
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("error returned by ioctl_mem: ret=0x%x. mem_handle=0x%x\n", tz_ret, mem_param.mem_handle);
		return tz_ret;
		//return NULL;
	}
	//DBG_LOG("ioctl return: handle=0x%x, size=%d, align=%d, addr=%p \n", mem_param.mem_handle, mem_param.size, mem_param.alignment, mem_param.addr);

	*mem_handle = mem_param.mem_handle;

	return mem_param.addr;	/*NULL*/
}

void *TA_Mem_AllocMultiChunkmem(MTEE_SHAREDMEM_HANDLE chm_handle, MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, uint64_t in_key)
{
	if(alignment < 0) {
		ERR_LOG("Invalid alignment value: %d. \n", alignment);
		return NULL;
	}
	if(size <= 0) {
		ERR_LOG("Invalid size value: %d. \n", size);
		return NULL;
	}

	return TA_Mem_AllocMultiChunkmem_Body(chm_handle, mem_handle, alignment, size, 0, NULL, in_key);
}

/*new MTEE+ API*/
void *TA_Mem_ZallocMultiChunkmem(MTEE_SHAREDMEM_HANDLE chm_handle, MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, uint64_t in_key)
{
	if(alignment < 0) {
		ERR_LOG("Invalid alignment value: %d. \n", alignment);
		return NULL;
	}
	if(size <= 0) {
		ERR_LOG("Invalid size value: %d. \n", size);
		return NULL;
	}

	return TA_Mem_AllocMultiChunkmem_Body(chm_handle, mem_handle, alignment, size, 1, NULL, in_key);
}

/*
* handle: memory handle (not chm_handle)
*/
void *TA_Mem_ReferenceChunkmem_Body (MTEE_SECUREMEM_HANDLE handle, uint64_t in_key)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	mem_param.alignment = 0;
	mem_param.size = 0;
	mem_param.mem_handle= handle;
	mem_param.key = in_key;

	if (handle == 0)
    {
    	ERR_LOG("Invalid memory handle. \n");
        return NULL;
    }
	//DBG_LOG("ioctl: handle=0x%x \n", mem_param.mem_handle);
	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_REF, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("returned by ioctl_mem: 0x%x. \n", tz_ret);
		return NULL;
	}
	//DBG_LOG("ioctl return: handle=0x%x, addr=%p \n", mem_param.mem_handle, mem_param.addr);

	return mem_param.addr;	/*addr=0, NULL*/
}


/*
* handle: memory handle (not chm_handle)
*/
void *TA_Mem_ReferenceChunkmem (MTEE_SECUREMEM_HANDLE handle, uint64_t in_key)
{
	return TA_Mem_ReferenceChunkmem_Body (handle, in_key);
}

/*new MTEE+ API*/
//int TA_Mem_UnreferenceMultiChunkmem (MTEE_SHAREDMEM_HANDLE chm_handle, MTEE_SECUREMEM_HANDLE mem_handle, uint64_t in_key)
int TA_Mem_UnreferenceMultiChunkmem (MTEE_SECUREMEM_HANDLE mem_handle, uint64_t in_key)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;
	int ret_val = -1;

	if (mem_handle == 0)
    {
    	ERR_LOG("ERROR: Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.alignment = 0;
	mem_param.size = 0;
	mem_param.mem_handle= mem_handle;
	mem_param.key = in_key;

	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_UNREF, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("returned by ioctl_mem: 0x%x. \n", tz_ret);
		return ret_val; // return ref_count (-1) to indicate error
	}
	//DBG_LOG("ioctl return: ref_count=%d \n", mem_param.ref_count);

	ret_val = mem_param.ref_count;
	return ret_val;
}

/*
* handle: memory handle (not chm_handle)
*/
#ifdef __NEBULA_HEE__
TZ_RESULT TA_Mem_QueryChunkmem_Body (MTEE_SECUREMEM_HANDLE handle, MTEE_MEM_PARAM *param, uint64_t in_key)
{
	//need use nbl api
	ERR_LOG("[%s] NOT SUPPORTED \n",__func__);
	return TZ_RESULT_ERROR_NOT_SUPPORTED;
}
#else
TZ_RESULT TA_Mem_QueryChunkmem_Body (MTEE_SECUREMEM_HANDLE handle, MTEE_MEM_PARAM *param, uint64_t in_key)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	if (handle == 0)
	{
		ERR_LOG("Invalid memory handle. \n");
		return TZ_RESULT_ERROR_GENERIC;
	}

	mem_param.mem_handle= handle;
	mem_param.key = in_key;

	//printf("[before] call ioctl MTEE_MEM_SECUREMULTICHUNKMEM_QUERY: mem_param.key = 0x%llx\n", mem_param.key);
	//DBG_LOG("memory handle: %d \n", mem_param.mem_handle);
	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_QUERY, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
		return tz_ret;
	}

	DBG_LOG("Return param: addr=0x%llx, size=0x%x \n", mem_param.addr, mem_param.size);

	param->buffer = (void *)mem_param.addr;	/*VA @ GZ user space*/
	param->size = mem_param.size;

	return TZ_RESULT_SUCCESS;
}
#endif

/*
* handle: memory handle (not chm_handle)
*/
TZ_RESULT TA_Mem_QueryChunkmem (MTEE_SECUREMEM_HANDLE mem_handle, MTEE_MEM_PARAM *param, uint64_t in_key)
{
	return TA_Mem_QueryChunkmem_Body (mem_handle, param, in_key);
}

TZ_RESULT TA_Mem_QueryChmDebugInfo_Body (MTEE_SHAREDMEM_HANDLE chm_handle, uint64_t in_key)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	if (chm_handle == 0)
    {
    	ERR_LOG("Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.chm_mem_handle= chm_handle;
	mem_param.key = in_key;

	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_QUERY_DEBUG, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
		return tz_ret;
	}

    //return TZ_RESULT_SUCCESS;
    return mem_param.size; /*total free space*/
}

TZ_RESULT TA_Mem_QueryChmDebugInfo (MTEE_SHAREDMEM_HANDLE chm_handle, uint64_t in_key)
{
	return TA_Mem_QueryChmDebugInfo_Body (chm_handle, in_key);
}

TZ_RESULT TA_Mem_QueryChmInfo_Body (MTEE_SHAREDMEM_HANDLE chm_handle, uint64_t in_key, uint64_t *pa, uint32_t *size)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	if ((chm_handle != NULL) && (chm_handle == 0))
    {
		ERR_LOG("Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.chm_mem_handle= chm_handle;
	mem_param.key = in_key;

	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_INFO_QUERY, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
		return tz_ret;
	}


	*pa = mem_param.paddr;
	*size = mem_param.size;

	DBG_LOG("[%s] ret pa = 0x%llx, size=0x%x\n", __func__, *pa, *size);

    return TZ_RESULT_SUCCESS;
}

/*get chunk memory PA & size*/
TZ_RESULT TA_Mem_QueryChmInfo (MTEE_SHAREDMEM_HANDLE chm_handle, uint64_t in_key, uint64_t *pa, uint32_t *size)
{
	return TA_Mem_QueryChmInfo_Body (chm_handle, in_key, pa, size);
}

/*
* a proprietary API for ION to get secure shared memory PA & size information
* input: no input for any chunk memory (use a default chunk memory)
* limitation: Only one chunk memory is as the 'default' chunk memory.
*             The first append chunk memory was set as the 'default' one.
*             Original multi-chunk memory function will be disabled.
* output: pa & size
*/
TZ_RESULT TA_Mem_Query_ION_ChmInfo (uint64_t *pa, uint32_t *size)
{
	return TA_Mem_QueryChmInfo_Body (NULL, 0, pa, size);
}



/*
* handle: memory handle (not chm_handle)
*/
uint64_t TA_Mem_QueryChmPA_Body (MTEE_SECUREMEM_HANDLE handle, uint64_t in_key)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;
	uint64_t ret_pa;

	if (handle == 0)
    {
    	ERR_LOG("Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.mem_handle= handle;
	mem_param.key = in_key;

	//printf("[before] call ioctl MTEE_MEM_SECUREMULTICHUNKMEM_QUERY: mem_param.key = 0x%llx\n", mem_param.key);
    //DBG_LOG("memory handle: %d \n", mem_param.mem_handle);
	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_QUERY_PA, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
		return tz_ret;
	}

	ret_pa = mem_param.paddr;
	//DBG_LOG("Return param: paddr=0x%llx \n", (uint64_t)ret_pa);

	return ret_pa;
}

/*
* handle: memory handle (not chm_handle)
*/
uint64_t TA_Mem_QueryChmPA (MTEE_SECUREMEM_HANDLE mem_handle, uint64_t in_key)
{
	return TA_Mem_QueryChmPA_Body (mem_handle, in_key);
}


/*
* handle: memory handle (not chm_handle)
*/
uint64_t TA_Mem_QueryChm_GZ_PA_Body (MTEE_SECUREMEM_HANDLE handle, uint64_t in_key)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;
	uint64_t ret_pa = 0;

	if (handle == 0)
    {
    	ERR_LOG("Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.mem_handle= handle;
	mem_param.key = in_key;

    //DBG_LOG("memory handle: %d \n", mem_param.mem_handle);
	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_QUERY_GZ_PA, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
		return tz_ret;
	}

	ret_pa = mem_param.paddr;
	//DBG_LOG("Return param: paddr=0x%llx \n", (uint64_t)ret_pa);

	return ret_pa;
}

/*
* handle: memory handle (not chm_handle)
*/
uint64_t TA_Mem_QueryChm_GZ_PA (MTEE_SECUREMEM_HANDLE mem_handle, uint64_t in_key)
{
	return TA_Mem_QueryChm_GZ_PA_Body (mem_handle, in_key);
}


/*
* handle: memory handle (not chm_handle)
*/
uint64_t TA_Mem_Query_IONHandle_Body (MTEE_SECUREMEM_HANDLE handle, uint64_t in_key)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;
	uint32_t IONHandle = -1;

	if (handle == 0)
    {
    	ERR_LOG("Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.mem_handle= handle;
	mem_param.key = in_key;

	//printf("[before] call ioctl MTEE_MEM_SECUREMULTICHUNKMEM_QUERY: mem_param.key = 0x%llx\n", mem_param.key);
    //DBG_LOG("memory handle: %d \n", mem_param.mem_handle);
	tz_ret = ioctl_mem("mem", MTEE_MEM_QUERY_IONHandle, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
		return tz_ret;
	}

	IONHandle = mem_param.chm_mem_handle;

	DBG_LOG("Return param: IONHandle=0x%x \n", (uint32_t) IONHandle);

	return IONHandle;
}

/*
* handle: memory handle (not chm_handle)
*/
uint64_t TA_Mem_Query_IONHandle (MTEE_SECUREMEM_HANDLE mem_handle, uint64_t in_key)
{
	return TA_Mem_Query_IONHandle_Body (mem_handle, in_key);
}

TZ_RESULT TA_Mem_CP_Chm2Shm_body (MTEE_SECUREMEM_HANDLE shm_handle, MTEE_SECUREMEM_HANDLE chm_mem_handle, uint32_t size, uint64_t in_key)
{
	int ret;
	MTEE_MEM_PARAM mem_param;		/*chm*/
    MTEE_SHAREDMEM_PARAM shm_param;	/*shm*/

	//char *shm_va, *chm_va;
	void *shm_va, *chm_va;

	//Query chm
	ret = TA_Mem_QueryChunkmem (chm_mem_handle, &mem_param, in_key);
	if (ret != TZ_RESULT_SUCCESS) {
		DBG_LOG("TA_Mem_QueryChunkmem Fail. ret=0x%x\n", ret);
		return TZ_RESULT_ERROR_GENERIC;
	}
	chm_va = (void *) mem_param.buffer;
	if (!chm_va)
		return TZ_RESULT_ERROR_GENERIC;

	// Query shm
    ret = TA_Mem_QuerySharedmem(shm_handle, &shm_param);
    if (ret) {
        ERR_LOG("TA_Mem_QuerySharedmem error.[ret=%d], return\n", ret);
		return TZ_RESULT_ERROR_GENERIC;
    }
	shm_va = (void *) shm_param.buffer;
	if (!shm_va)
		return TZ_RESULT_ERROR_GENERIC;

	//memset(shm_va, 0, size);
	memcpy(shm_va, chm_va, size);

	return TZ_RESULT_SUCCESS;
}

static inline bool is_copy_allow(void)
{
	secure_mem_t param;
	TZ_RESULT rc;

#if (SC_DEBUG == 1)
	/* force allow via compile option */
	return true;
#endif

	rc = ioctl_mem("platform", MTEE_MEM_QUERY_PROJECT_BUILD_TYPE, &param);
	if (rc != TZ_RESULT_SUCCESS)
		return false;

	/* build variant passed from pre-loader
	 * - 1: release build
	 * - 0: debug build
	 */
	if (((uint32_t)param.tag) == 1) {
		WARN_LOG("NOTICE: copy is forbid!\n");
		return false;
	}

	ERR_LOG("WARNING: copy is allowed!\n");
	return true;
}

TZ_RESULT TA_Mem_CP_Chm2Shm (MTEE_SECUREMEM_HANDLE shm_handle, MTEE_SECUREMEM_HANDLE chm_mem_handle, uint32_t size, uint64_t in_key)
{
	if (unlikely(is_copy_allow()))
		return TA_Mem_CP_Chm2Shm_body (shm_handle, chm_mem_handle, size, in_key);
	return TZ_RESULT_ERROR_NOT_SUPPORTED;
}

TZ_RESULT MTEE_ReleaseMultiChunkmem(MTEE_SHAREDMEM_HANDLE chm_handle, uint32_t cmd)
{
	TZ_RESULT tz_ret;
    secure_mem_t mem_param;

	if (chm_handle == 0)
    {
    	ERR_LOG("Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.chm_mem_handle= chm_handle;

    //DBG_LOG("memory handle: %d \n", mem_param.mem_handle);

	if (cmd == (uint32_t) TZCMD_MEM_RELEASE_CHUNKMEM_ION)
		tz_ret = ioctl_mem("mem", MTEE_MEM_RELEASE_MULTI_CHUNKMEM_ION, &mem_param);	/*fix: proprietary for ION*/
	else
		tz_ret = ioctl_mem("mem", MTEE_MEM_RELEASE_MULTI_CHUNKMEM, &mem_param);	/*default relase cmd*/


	if(tz_ret != TZ_RESULT_SUCCESS) {
			ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
			return tz_ret;
	}

    return TZ_RESULT_SUCCESS;
}


typedef uint32_t MTEE_RELEASECM_HANDLE;
TZ_RESULT MTEE_ReleaseChunkmem (MTEE_RELEASECM_HANDLE *handle, uint32_t alignment, uint32_t size)
{
#if 0
	void *shared_va;
    unsigned long cm_pa;
    MTEE_HANDLE_TYPE_ID htid = _TaMemHandleTypeId;
    MTEE_FREECM_INFO *obj;
    void *cm_va;
    MTEE_RELEASECM_HANDLE hdl;

    *handle = 0;

    // allocate for handle resource
    obj = MTEE_AllocMem (sizeof (MTEE_FREECM_INFO));
    if (obj == NULL)
    {
        return TZ_RESULT_ERROR_OUT_OF_MEMORY;
    }

    // allocate memory for releasing
    cm_va = MTEE_AllocChunkmemAlign (alignment, size);
    if (cm_va == NULL)
    {
        return TZ_RESULT_ERROR_OUT_OF_MEMORY;
    }
    /* clear buffer context */
    memset(cm_va, 0, size);

    cm_pa = MTEE_Physical (cm_va);
    obj->va = cm_va;
    obj->size = size;
    obj->pa = cm_pa;

    // get va for remap
    shared_va = MTEE_AllocSharedmem (size + SHM_GUARD_SPACE); // guard? fix me!!!!
    if (shared_va == NULL)
    {
        MTEE_FreeMem (obj);
        return TZ_RESULT_ERROR_OUT_OF_MEMORY;
    }

#if 1
    obj->shared_va = cm_va; ////
    obj->shared_pa = (unsigned long) shared_va;///(uint32_t) MTEE_Physical (shared_va);

    // get handle
    if (MTEE_HandleAllocate (htid, obj, (uint32_t *) &hdl) != TZ_RESULT_SUCCESS)
    {
        MTEE_FreeMem (obj);
        MTEE_FreeSharedmem (shared_va);
        return TZ_RESULT_ERROR_GENERIC;
    }
#else
    obj->shared_va = shared_va;
    obj->shared_pa = MTEE_Physical (shared_va);

    MTEE_ReleaseCmMmu (cm_va, shared_va, (void *) cm_pa, size);

    // get handle
    if (MTEE_HandleAllocate (htid, obj, (uint32_t *) &hdl) != TZ_RESULT_SUCCESS)
    {
        MTEE_FreeMem (obj);
        MTEE_FreeSharedmem (shared_va);
        //MTEE_AppendMmu (shared_va, buffer, (void *) pa, size); // fix me!!!!
        //MTEE_AppendMmu (obj->shared_va, obj->shared_va, (void *) obj->shared_pa, obj->size); // fix me!!!!
        return TZ_RESULT_ERROR_GENERIC;
    }
#endif

    *handle = hdl;

    return TZ_RESULT_SUCCESS;
#endif
	printf("%s not supported.\n",__func__);
	return -1;
}



TZ_RESULT MTEE_AppendChunkmem (MTEE_RELEASECM_HANDLE handle)
{
#if 0
    MTEE_HANDLE_TYPE_ID htid = _TaMemHandleTypeId;
    MTEE_FREECM_INFO *obj;
    uint32_t state;

    // query object
    obj = (MTEE_FREECM_INFO *) _mtee_HandleGetObject_maskIrq (htid, handle, &state);
    if (obj == NULL)
    {
        return TZ_RESULT_ERROR_GENERIC;
    }

#if 1
    MTEE_FreeSharedmem ((void *) obj->shared_pa);
#else
    MTEE_AppendMmu (obj->va, (void *) obj->pa, obj->shared_va, (void *) obj->shared_pa, obj->size);
    MTEE_FreeSharedmem (obj->shared_va);
#endif

    MTEE_FreeChunkmem (obj->va);

    MTEE_FreeMem (obj);

    _mtee_HandleFree_maskIrq (htid, handle, state);

    return TZ_RESULT_SUCCESS;
#endif
	printf("%s not supported.\n",__func__);
	return -1;

}




void *TA_Mem_RegisterSharedMem(MTEE_SHAREDMEM_HANDLE *shm_handle, MTEE_SHAREDMEM *param)
{
	if(param->size <= 0) {
		ERR_LOG("Invalid size value: %d. \n", param->size);
		return NULL;
	}

    return TA_Mem_RegisterSharedMem_Body(shm_handle, param, NULL);
}

/*fix mtee sync*/
void *TA_Mem_RegisterSharedMemWithTag(MTEE_SHAREDMEM_HANDLE *shm_handle, MTEE_SHAREDMEM *param, const char *tag)
{
#if 0
	return TA_Mem_RegisterSharedMem_Body(shm_handle, param, tag);
#endif
	print_mtee_orig_msg;
	return; //not support
}


TZ_RESULT TA_Mem_UnregisterSharedmem (MTEE_SHAREDMEM_HANDLE handle)
{
	TZ_RESULT tz_ret;
    secure_mem_t mem_param;

	if (handle == 0)
    {
    	ERR_LOG("Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.mem_handle= handle;

    //DBG_LOG("memory handle: %d \n", mem_param.mem_handle);
	tz_ret = ioctl_mem("mem", MTEE_MEM_SHAREDMEM_UNREG, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
			ERR_LOG("error returned by ioctl_mem: 0x%x. \n", tz_ret);
			return tz_ret;
	}

    return TZ_RESULT_SUCCESS;
}

int TA_Mem_SetExtraInfo(uint32_t handle, void *pex_info)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	mem_param.mem_handle = handle;
	mem_param.paddr = pex_info;

	tz_ret = ioctl_mem("mem", MTEE_MEM_SET_EXTRA, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
        ERR_LOG("set extra error returned by ioctl_mem: 0x%x. \n", tz_ret);
        return tz_ret;
	}

    return TZ_RESULT_SUCCESS;
}

int TA_Mem_GetExtraInfo(uint32_t handle, void **ppex_info)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	mem_param.mem_handle = handle;

	tz_ret = ioctl_mem("mem", MTEE_MEM_GET_EXTRA, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
        ERR_LOG("get extra error returned by ioctl_mem: 0x%x. \n", tz_ret);
        return tz_ret;
	}

    *ppex_info = (void*)mem_param.paddr;

    return TZ_RESULT_SUCCESS;
}

int TA_Mem_CopyExtraInfo(uint32_t src_handle, uint32_t dst_handle)
{
    void *extra_info;
    int ret;

    ret = TA_Mem_GetExtraInfo(src_handle, &extra_info);
    if (ret != 0)
        return ret;

    ret = TA_Mem_SetExtraInfo(dst_handle, extra_info);

    return ret;
}

/**************************************************************************/
#if 0
/*orig MTEE: later porting*/

/*chunk memory related APIs*/
//static void *TA_Mem_AllocChunkmem_Body(MTEE_SECURECM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, char *tag)	//orig MTEE: diff para
/*new MTEE+ API: diff para with orig MTEE*/
static void *TA_Mem_AllocChunkmem_Body(MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size, int zalloc, char *tag)
{
	secure_mem_t mem_param;
	TZ_RESULT tz_ret;

	mem_param.alignment = alignment;
	mem_param.size = size;

	if (zalloc) {
		tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_ZALLOC, &mem_param);
	} else {
		tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_ALLOC, &mem_param);
	}
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("error returned by ioctl_mem: %d. \n", tz_ret);
		return tz_ret;
		//return NULL;
	}
	//DBG_LOG("ioctl return: handle=0x%x, size=%d, align=%d, addr=%p \n", mem_param.mem_handle, mem_param.size, mem_param.alignment, mem_param.addr);

	*mem_handle = mem_param.mem_handle;

	return mem_param.addr;
}

/*Copy from MTEE*/
void *TA_Mem_AllocChunkmem(MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size)
{
	if(alignment < 0) {
		ERR_LOG("Invalid alignment value: %d. \n", alignment);
		return NULL;
	}
	if(size <= 0) {
		ERR_LOG("Invalid size value: %d. \n", size);
		return NULL;
	}

	return TA_Mem_AllocChunkmem_Body(mem_handle, alignment, size, 0, NULL);
}

/*new MTEE+ API*/
void *TA_Mem_ZallocChunkmem(MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size)
{
	if(alignment < 0) {
		ERR_LOG("Invalid alignment value: %d. \n", alignment);
		return NULL;
	}
	if(size <= 0) {
		ERR_LOG("Invalid size value: %d. \n", size);
		return NULL;
	}

	return TA_Mem_AllocChunkmem_Body(mem_handle, alignment, size, 1, NULL);
}

/*Copy from MTEE*/
void *TA_Mem_ReferenceChunkmem (MTEE_SECUREMEM_HANDLE handle)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	mem_param.alignment = 0;
	mem_param.size = 0;
	mem_param.mem_handle= handle;
	if (handle == 0)
    {
    	ERR_LOG("Invalid memory handle. \n");
        return NULL;
    }
	//DBG_LOG("ioctl: handle=0x%x \n", mem_param.mem_handle);
	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_REF, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("returned by ioctl_mem: %d. \n", tz_ret);
		return NULL;
	}
	//DBG_LOG("ioctl return: handle=0x%x, addr=%p \n", mem_param.mem_handle, mem_param.addr);

	return mem_param.addr;	/*addr=0*/
}

/*Copy from MTEE*/
int TA_Mem_UnreferenceChunkmem (MTEE_SECUREMEM_HANDLE handle)
{
	TZ_RESULT tz_ret;
	secure_mem_t mem_param;

	if (handle == 0)
    {
    	ERR_LOG("ERROR: Invalid memory handle. \n");
        return TZ_RESULT_ERROR_GENERIC;
    }

	mem_param.alignment = 0;
	mem_param.size = 0;
	mem_param.mem_handle= handle;

	tz_ret = ioctl_mem("mem", MTEE_MEM_SECUREMULTICHUNKMEM_UNREF, &mem_param);
	if(tz_ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("returned by ioctl_mem: %d. \n", tz_ret);
		return -1; // return ref_count -1 to indicate error
	}
	//DBG_LOG("ioctl return: ref_count=%d \n", mem_param.ref_count);

	return mem_param.ref_count;
}

#endif

/**************************************************************************/

/*fix mtee sync*/
int TA_Mem_GetVirtMappedmemRange (void **base, uint32_t *size)
{
#if 0
	if(base != NULL)
        *base = (void *)VIRTMAPPED_MEM_BASE;
    if(size != NULL)
        *size = VIRTMAPPED_MEM_SIZE;

    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int TA_Mem_HookVirtMappingFunc(iommu_func_t mapfunc, iommu_func_t unmapfunc)
{
#if 0
    hookfunc_map = mapfunc;
    hookfunc_unmap = unmapfunc;
    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

int TA_Mem_ConfigChunkMemInfo(uint64_t pa, uint32_t size, uint32_t region_id)
{
	TZ_RESULT ret;
	secure_mem_t mem_param;

	mem_param.paddr = pa;
	mem_param.size = size;
	mem_param.region_id = region_id;

	ret = ioctl_mem("mem", MTEE_MEM_CONFIG_CHUNKMEM_INFO_ION, &mem_param);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("%s: ioctl failed=0x%x\n", __func__, ret);
		return ret;
	}

	return TZ_RESULT_SUCCESS;
}

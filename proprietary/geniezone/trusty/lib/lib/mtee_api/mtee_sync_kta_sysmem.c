#include <stdio.h>
#include <stdlib.h>

#include <kta_sys_mem.h>
#include <tz_private/log.h>

/*fix mtee sync*/
TZ_RESULT KTA_Mem_QuerySharedmem_body(MTEE_SHAREDMEM_HANDLE handle, MTEE_SHAREDMEM_PARAM *param, uint32_t cmd)
{
#if 0
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
			ERR_LOG("error returned by ioctl_mem: %d. \n", tz_ret);
			return tz_ret;
	}

	//DBG_LOG("Return param: addr=0x%x, size=%x \n", mem_param.addr, mem_param.size);

	param->buffer = (void *)mem_param.addr;
	param->size = mem_param.size;
	param->pa = mem_param.paddr;


    return TZ_RESULT_SUCCESS;
#endif		
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void *KTA_Mem_RegisterSharedMem(KTEE_SHAREDMEM_HANDLE *shm_handle, KTEE_SHAREDMEM *param)
{
#if 0
	if(param->size <= 0) {
		ERR_LOG("Invalid size value: %d. \n", param->size);
		return NULL;
	}

    return TA_Mem_RegisterSharedMem_Body(shm_handle, param, NULL);
#endif		
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void *KTA_Mem_RegisterSharedMemWithTag(KTEE_SHAREDMEM_HANDLE *shm_handle, KTEE_SHAREDMEM *param, const char *tag)
{
#if 0
	return TA_Mem_RegisterSharedMem_Body(shm_handle, param, tag);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
TZ_RESULT KTA_Mem_UnregisterSharedmem (MTEE_SHAREDMEM_HANDLE handle)
{
#if 0
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
			ERR_LOG("error returned by ioctl_mem: %d. \n", tz_ret);
			return tz_ret;
	}

    return TZ_RESULT_SUCCESS;
#endif
	print_mtee_orig_msg;
	return; //not support
}

#if 0
/*fix mtee sync: mtee_mem_srv.c originally has this API*/
void *KTA_Mem_AllocMem(MTEE_SECUREMEM_HANDLE *mem_handle, uint32_t alignment, uint32_t size)
{
#if 0
	if(alignment < 0) {
		ERR_LOG("Invalid alignment value: %d. \n", alignment);
		return NULL;
	}
	if(size <= 0) {
		ERR_LOG("Invalid size value: %d. \n", size);
		return NULL;
	}

	return TA_Mem_AllocMem_Body(mem_handle, alignment, size, 0, NULL);
#endif
	print_mtee_orig_msg;
	return; //not support
}
#endif

#if 0
/*fix mtee sync: mtee_mem_srv.c originally has this API*/
void *KTA_Mem_ReferenceMem (MTEE_SECUREMEM_HANDLE handle)
{
#if 0
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
		ERR_LOG("returned by ioctl_mem: %d. \n", tz_ret);
		return NULL;
	}
	//DBG_LOG("ioctl return: handle=0x%x, addr=%p \n", mem_param.mem_handle, mem_param.addr);

	return mem_param.addr;
#endif
	print_mtee_orig_msg;
	return; //not support
}
#endif

#if 0
/*fix mtee sync: mtee_mem_srv.c originally has this API*/
int KTA_Mem_UnreferenceMem (MTEE_SECUREMEM_HANDLE handle)
{
#if 0
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
		ERR_LOG("returned by ioctl_mem: %d. \n", tz_ret);
		return -1; // return ref_count -1 to indicate error
	}
	//DBG_LOG("ioctl return: ref_count=%d \n", mem_param.ref_count);

	return mem_param.ref_count;
#endif
	print_mtee_orig_msg;
	return; //not support
}
#endif

#if 0
/*fix mtee sync: move to mtee_mem_srv.c*/
void *KTA_Mem_AllocChunkmem (KTEE_SECUREMEM_HANDLE *handle, uint32_t alignment, uint32_t size)
{
	print_mtee_orig_msg;
	return; //not support
}
#endif

#if 0
/*fix mtee sync: move to mtee_mem_srv.c*/
void *KTA_Mem_ReferenceChunkmem (KTEE_SECUREMEM_HANDLE handle)
{
	print_mtee_orig_msg;
	return; //not support
}
#endif

#if 0
/*fix mtee sync: move to mtee_mem_srv.c*/
int KTA_Mem_UnreferenceChunkmem (KTEE_SECUREMEM_HANDLE handle)
{
	print_mtee_orig_msg;
	return; //not support
}
#endif

/*fix mtee sync*/
int KTA_Mem_GetVirtMappedmemRange (void **base, uint32_t *size)
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
int KTA_Mem_HookVirtMappingFunc(iommu_func_t mapfunc, iommu_func_t unmapfunc)
{
#if 0
    hookfunc_map = mapfunc;
    hookfunc_unmap = unmapfunc;
    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}


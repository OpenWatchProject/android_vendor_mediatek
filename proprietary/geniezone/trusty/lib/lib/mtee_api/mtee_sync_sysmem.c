#include <stdio.h>
#include <stdlib.h>

#include <tz_private/system.h>
#include <tz_private/log.h>
#include <tz_private/sys_mem.h>

/*fix mtee sync*/
int MTEE_InitSharedmem(void *start, uint32_t size)
{
#if 0
    MemOne_init(&sharedmemInfo, start, size, SHM_PAGE_SIZE);
    MTEE_InitSpinlock((MTEE_SPINLOCK *) &shm_lock, 0);

    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void *MTEE_AllocSharedmem(uint32_t size)
{
#if 0
    void *buffer;
    uint32_t state;

    state = MTEE_SpinLockMaskIrq((MTEE_SPINLOCK *)&shm_lock);

    buffer = MemOne_alloc(&sharedmemInfo, SHM_PAGE_SIZE, size);

    MTEE_SpinUnlockRestoreIrq((MTEE_SPINLOCK *)&shm_lock, state);

    if (buffer == NULL)
    {
#ifdef DBG_MEM
        MTEE_LOG(MTEE_LOG_LVL_WARN, "MTEE_AllocSharedMem: error, NULL buffer!\n");
#endif
        return NULL;
    }

    return buffer;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int MTEE_FreeSharedmem (void *buffer)
{
#if 0
    int ret;
    uint32_t state;

    state = MTEE_SpinLockMaskIrq((MTEE_SPINLOCK *) &shm_lock);

    ret = MemOne_free(&sharedmemInfo, buffer);

    MTEE_SpinUnlockRestoreIrq((MTEE_SPINLOCK *) &shm_lock, state);

    if (ret < 0)
    {
#ifdef DBG_MEM
        MTEE_LOG(MTEE_LOG_LVL_WARN, "MTEE_FreeSharedMem: error = 0x%x!\n", ret);
#endif
        return -1;
    }

    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
uint32_t * MTEE_CreateMmutbl(MTEE_SHAREDMEM *param)
{
#if 0
    return MTEE_CreateMmutbl_Body(param, NULL);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
uint32_t * MTEE_CreateMmutblWithTag(MTEE_SHAREDMEM *param, const char *tag)
{
#if 0
    return MTEE_CreateMmutbl_Body(param, tag);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void MTEE_DestroyMmutbl(uint32_t *mmutbl)
{
#if 0
    free (mmutbl);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void MTEE_SetupMmutbl(uint32_t *mmutbl, void *va_base, uint32_t num)
{
#if 0
    struct MTEE_MMUMaps *mmu_p;
    uint32_t i;

    // avoid overflow
    if(num > (0xffffffffl / sizeof(struct MTEE_MMUMaps)))
        return;

    // prepare register table
    mmu_p = (struct MTEE_MMUMaps *) malloc (num * sizeof (struct MTEE_MMUMaps));

    if(mmu_p == NULL)
        return;

    for (i = 0; i < num; i ++)
    {
        mmu_p[i].phys_addr = *mmutbl;
        mmu_p[i].virt_addr = va_base;
        mmu_p[i].size = SZ_4K;
        mmu_p[i].prot = PROT_NORMAL_MEMORY;
        va_base = (char *)va_base + SZ_4K;
        mmutbl ++;
    }

    MTEE_SetMMUMaps (mmu_p, num);

    free (mmu_p);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void MTEE_ClearMmutbl (void *virt_addr, int size)
{
#if 0
    MTEE_SetMMUTable (virt_addr, 0, size, PROT_NO_ACCESS);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void MTEE_ReleaseCmMmu (void *cm_va, void *shared_va, unsigned long cm_pa, uint32_t size)
{
#if 0
    // clean memory for security issue
    // must make sure dram is clean, too
    memset (cm_va, 0, size);
    MTEE_DmaMapMemory (cm_va, size, MTEE_DMA_BIDIRECTION);

    // umap cm mmu
    MTEE_ClearMmutbl (cm_va, size);

    // remap cm to shm region for protection
    MTEE_SetMMUTable (shared_va, cm_pa, size, PROT_NORMAL_MEMORY);

    // iommu ... TBD
#endif
		print_mtee_orig_msg;
		return; //not support
}

/*fix mtee sync*/
void MTEE_AppendMmu (void *cm_va, unsigned long cm_pa, void *shared_va, unsigned long shared_pa, uint32_t size)
{
#if 0
    // remap cm back
    MTEE_SetMMUTable (cm_va, cm_pa, size, PROT_SECURE_MEMORY);

    MTEE_SetMMUTable (shared_va, shared_pa, size, PROT_NORMAL_MEMORY);

    // iommu ... TBD
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int MTEE_InitNonCachedmem (void)
{
#if 0
    MTEE_InitSpinlock((MTEE_SPINLOCK *) &ncm_lock, 0);
    noncachedmemInfo.prealbuf = NULL;
    noncachedmemInfo.pnoncachedbuf = NULL;
    list_initialize(&noncachedmemInfo.list);
    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void *MTEE_AllocNonCachedmem (uint32_t size)
{
#if 0
    uint32_t state;
    uint32_t map_size;
    NONCACHEDMEM_INFO *new_noncached_info;

    if(size <= 0) return NULL;

    new_noncached_info = MTEE_AllocMem(sizeof(NONCACHEDMEM_INFO));
    if(new_noncached_info == NULL)
    {
        return NULL;
    }

    // alloc the real memory buffer
    size = (size + (CACHE_LINE_SIZE-1)) & (~(CACHE_LINE_SIZE-1));
    new_noncached_info->prealbuf = MTEE_AllocMemAlign(SZ_4K, size);
    if(new_noncached_info->prealbuf == NULL)
    {
        MTEE_FreeMem(new_noncached_info);
        return NULL;
    }

    // alloc the noncached memory space
    new_noncached_info->pnoncachedbuf = MTEE_AllocSharedmem(size);
    if(new_noncached_info->pnoncachedbuf == NULL)
    {
        MTEE_FreeMem(new_noncached_info->prealbuf);
        MTEE_FreeMem(new_noncached_info);
        return NULL;
    }

    list_initialize (&new_noncached_info->list);
    new_noncached_info->size = size;
    map_size = (size + SZ_4K - 1) & (~(SZ_4K-1));

    // flush cache
    MTEE_DmaMapMemory (new_noncached_info->prealbuf, size, MTEE_DMA_BIDIRECTION);

    // map to the noncached memory space
    MTEE_SetMMUTable (new_noncached_info->pnoncachedbuf, MTEE_Physical(new_noncached_info->prealbuf), map_size, PROT_SECURE_DEVICE);

    state = MTEE_SpinLockMaskIrq ((MTEE_SPINLOCK *) &ncm_lock);
    list_add_tail (&noncachedmemInfo.list, &new_noncached_info->list);
    MTEE_SpinUnlockRestoreIrq ((MTEE_SPINLOCK *) &ncm_lock, state);

    return new_noncached_info->pnoncachedbuf;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int MTEE_FreeNonCachedmem (void *buffer)
{
#if 0
    struct list_node *node;
    NONCACHEDMEM_INFO *ncm_info;
    uint32_t state;
    uint32_t size, map_size;
    void *prealbuf = NULL, *pnoncachedbuf = NULL;

    if(buffer == NULL) return -1;

    state = MTEE_SpinLockMaskIrq ((MTEE_SPINLOCK *) &ncm_lock);
    list_for_every(&noncachedmemInfo.list, node)
    {
        ncm_info = containerof(node, NONCACHEDMEM_INFO, list);
        if(buffer == ncm_info->pnoncachedbuf)
        {
            prealbuf = ncm_info->prealbuf;
            pnoncachedbuf = ncm_info->pnoncachedbuf;
            size = ncm_info->size;
            list_delete (&ncm_info->list);
            break;
        }
    }
    MTEE_SpinUnlockRestoreIrq ((MTEE_SPINLOCK *) &ncm_lock, state);

    if(prealbuf!=NULL && pnoncachedbuf != NULL)
    {
        // unmap the noncached memory space
        map_size = (size + SZ_4K - 1) & (~(SZ_4K-1));
        MTEE_SetMMUTable (pnoncachedbuf, 0, map_size, PROT_NO_ACCESS);
        MTEE_FreeSharedmem(pnoncachedbuf);
        MTEE_FreeMem(prealbuf);
        MTEE_FreeMem(ncm_info);
        return 0;
    }

    return -1;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int MTEE_InitVirtMappedmem(void *start, uint32_t size)
{
#if 0
    MemOne_init(&virtmappedmemInfo, start, size, SHM_PAGE_SIZE);
    MTEE_InitSpinlock((MTEE_SPINLOCK *) &vmm_lock, 0);

    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void *MTEE_AllocVirtMappedmemAlign(uint32_t alignment, uint32_t size)
{
#if 0
    void *buffer;
    uint32_t state;

    if (alignment < SHM_PAGE_SIZE)
        alignment = SHM_PAGE_SIZE;

    state = MTEE_SpinLockMaskIrq((MTEE_SPINLOCK *) &vmm_lock);

    buffer = MemOne_alloc(&virtmappedmemInfo, alignment, size);

    MTEE_SpinUnlockRestoreIrq((MTEE_SPINLOCK *) &vmm_lock, state);

    if (buffer == NULL)
    {
#ifdef DBG_MEM
        MTEE_LOG(MTEE_LOG_LVL_WARN, "MTEE_AllocVirtMappedMem: error, NULL buffer!\n");
#endif
        return NULL;
    }

    return buffer;
#endif
		print_mtee_orig_msg;
		return; //not support
}

/*fix mtee sync*/
int MTEE_FreeVirtMappedmem (void *buffer)
{
#if 0
    int ret;
    uint32_t state;

    state = MTEE_SpinLockMaskIrq((MTEE_SPINLOCK *) &vmm_lock);

    ret = MemOne_free(&virtmappedmemInfo, buffer);

    MTEE_SpinUnlockRestoreIrq((MTEE_SPINLOCK *) &vmm_lock, state);

    if (ret < 0)
    {
#ifdef DBG_MEM
        MTEE_LOG(MTEE_LOG_LVL_WARN, "MTEE_FreeSharedMem: error = 0x%x!\n", ret);
#endif
        return -1;
    }

    return 0;
#endif
		print_mtee_orig_msg;
		return; //not support
}

/*fix mtee sync*/
int MTEE_GetChunkMemoryPoolFromREE(unsigned long *chunkmem_pa, size_t *sz)
{
#if 0
    struct ree_service_chunk_mem *ree_chunk_mem;

    if (chunkmem_pa == NULL || sz == NULL)
        return -1;

    if (chunkmem_from_ree == 0) {
        ree_chunk_mem = (struct ree_service_chunk_mem *)MTEE_GetReeParamAddress();

        if(TZ_RESULT_SUCCESS == MTEE_ReeServiceCall(REE_SERV_GET_CHUNK_MEMPOOL)) {
            cm_pa = ree_chunk_mem->chunkmem_pa;
            cm_sz = ree_chunk_mem->size;
            chunkmem_from_ree = 1;
        }
        else
            return -1;
    }

    *chunkmem_pa = cm_pa;
    *sz = cm_sz;
    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int MTEE_ReleaseChunkMemoryPoolToREE(void)
{
#if 0
    if (chunkmem_from_ree != 0) {
        if (TZ_RESULT_SUCCESS ==
                MTEE_ReeServiceCall(REE_SERV_REL_CHUNK_MEMPOOL)) {
            chunkmem_from_ree = 0;
            cm_pa = 0;
            cm_sz = 0;
            return 0;
		}
    }

    return -1;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int MTEE_GetChunkMemoryPoolInfo(unsigned long *chunkmem_pa, size_t *size)
{
#if 0
    if (chunkmem_pa == NULL || size == NULL)
        return -1;

    if (chunkmem_from_ree != 0) {
        *chunkmem_pa = cm_pa;
        *size = cm_sz;
    } else {
        *chunkmem_pa = 0;
        *size = 0;
    }

    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void tz_platform_set_chunk_protection(uint64_t phys_addr, size_t size)
{
#if 0
    int ret = 0;
    unsigned int sec_chunk_mpu_attr;

    if (phys_addr == 0 || size == 0) {
        sec_chunk_mpu_attr = SET_ACCESS_PERMISSON(NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION);
    } else {
        sec_chunk_mpu_attr = SET_ACCESS_PERMISSON(FORBIDDEN, FORBIDDEN, SEC_RW, SEC_RW);
    }
    if (size > 0)
        size -= 1;

    ret = emi_mpu_set_region_protection(phys_addr,           /*START_ADDR*/
                                        phys_addr+size,      /*END_ADDR*/
                                        sec_chunk_mpu_id,    /*region*/
                                        sec_chunk_mpu_attr);

    if (ret != 0) {
	MTEE_LOG(MTEE_LOG_LVL_BUG, "%s(%llx, %zx) failed!\n", __func__, phys_addr, size);
    }

    return;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
uint32_t KTEE_QueryMemoryrank (void *buffer)
{
#if 0
    uint32_t rank;

    rank = secure_memrank (buffer);

    return rank;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
unsigned long get_physicaladdr (void *buffer)
{
#if 0
    return MTEE_Physical (buffer);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int KTEE_InitSharedmem(void *start, uint32_t size)
{
#if 0
    MemOne_init(&sharedmemInfo, start, size, SHM_PAGE_SIZE);
    MTEE_InitSpinlock((MTEE_SPINLOCK *) &shm_lock, 0);

    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void *KTEE_AllocSharedmem(uint32_t size)
{
#if 0
    void *buffer;
    uint32_t state;

    state = MTEE_SpinLockMaskIrq((MTEE_SPINLOCK *)&shm_lock);

    buffer = MemOne_alloc(&sharedmemInfo, SHM_PAGE_SIZE, size);

    MTEE_SpinUnlockRestoreIrq((MTEE_SPINLOCK *)&shm_lock, state);

    if (buffer == NULL)
    {
#ifdef DBG_MEM
        MTEE_LOG(MTEE_LOG_LVL_WARN, "MTEE_AllocSharedMem: error, NULL buffer!\n");
#endif
        return NULL;
    }

    return buffer;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int KTEE_FreeSharedmem (void *buffer)
{
#if 0
    int ret;
    uint32_t state;

    state = MTEE_SpinLockMaskIrq((MTEE_SPINLOCK *) &shm_lock);

    ret = MemOne_free(&sharedmemInfo, buffer);

    MTEE_SpinUnlockRestoreIrq((MTEE_SPINLOCK *) &shm_lock, state);

    if (ret < 0)
    {
#ifdef DBG_MEM
        MTEE_LOG(MTEE_LOG_LVL_WARN, "MTEE_FreeSharedMem: error = 0x%x!\n", ret);
#endif
        return -1;
    }

    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
uint32_t * KTEE_CreateMmutbl(MTEE_SHAREDMEM *param)
{
#if 0
    return MTEE_CreateMmutbl_Body(param, NULL);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
uint32_t * KTEE_CreateMmutblWithTag(MTEE_SHAREDMEM *param, const char *tag)
{
#if 0
    return MTEE_CreateMmutbl_Body(param, tag);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_DestroyMmutbl(uint32_t *mmutbl)
{
#if 0
    free (mmutbl);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_SetupMmutbl(uint32_t *mmutbl, void *va_base, uint32_t num)
{
#if 0
    struct MTEE_MMUMaps *mmu_p;
    uint32_t i;

    // avoid overflow
    if(num > (0xffffffffl / sizeof(struct MTEE_MMUMaps)))
        return;

    // prepare register table
    mmu_p = (struct MTEE_MMUMaps *) malloc (num * sizeof (struct MTEE_MMUMaps));

    if(mmu_p == NULL)
        return;

    for (i = 0; i < num; i ++)
    {
        mmu_p[i].phys_addr = *mmutbl;
        mmu_p[i].virt_addr = va_base;
        mmu_p[i].size = SZ_4K;
        mmu_p[i].prot = PROT_NORMAL_MEMORY;
        va_base = (char *)va_base + SZ_4K;
        mmutbl ++;
    }

    MTEE_SetMMUMaps (mmu_p, num);

    free (mmu_p);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_ClearMmutbl (void *virt_addr, int size)
{
#if 0
    MTEE_SetMMUTable (virt_addr, 0, size, PROT_NO_ACCESS);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_ReleaseCmMmu (void *cm_va, void *shared_va, unsigned long cm_pa, uint32_t size)
{
#if 0
    // clean memory for security issue
    // must make sure dram is clean, too
    memset (cm_va, 0, size);
    MTEE_DmaMapMemory (cm_va, size, MTEE_DMA_BIDIRECTION);

    // umap cm mmu
    MTEE_ClearMmutbl (cm_va, size);

    // remap cm to shm region for protection
    MTEE_SetMMUTable (shared_va, cm_pa, size, PROT_NORMAL_MEMORY);

    // iommu ... TBD
#endif
		print_mtee_orig_msg;
		return; //not support
}

/*fix mtee sync*/
void KTEE_AppendMmu (void *cm_va, unsigned long cm_pa, void *shared_va, unsigned long shared_pa, uint32_t size)
{
#if 0
    // remap cm back
    MTEE_SetMMUTable (cm_va, cm_pa, size, PROT_SECURE_MEMORY);

    MTEE_SetMMUTable (shared_va, shared_pa, size, PROT_NORMAL_MEMORY);

    // iommu ... TBD
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int KTEE_InitNonCachedmem (void)
{
#if 0
    MTEE_InitSpinlock((MTEE_SPINLOCK *) &ncm_lock, 0);
    noncachedmemInfo.prealbuf = NULL;
    noncachedmemInfo.pnoncachedbuf = NULL;
    list_initialize(&noncachedmemInfo.list);
    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void *KTEE_AllocNonCachedmem (uint32_t size)
{
#if 0
    uint32_t state;
    uint32_t map_size;
    NONCACHEDMEM_INFO *new_noncached_info;

    if(size <= 0) return NULL;

    new_noncached_info = MTEE_AllocMem(sizeof(NONCACHEDMEM_INFO));
    if(new_noncached_info == NULL)
    {
        return NULL;
    }

    // alloc the real memory buffer
    size = (size + (CACHE_LINE_SIZE-1)) & (~(CACHE_LINE_SIZE-1));
    new_noncached_info->prealbuf = MTEE_AllocMemAlign(SZ_4K, size);
    if(new_noncached_info->prealbuf == NULL)
    {
        MTEE_FreeMem(new_noncached_info);
        return NULL;
    }

    // alloc the noncached memory space
    new_noncached_info->pnoncachedbuf = MTEE_AllocSharedmem(size);
    if(new_noncached_info->pnoncachedbuf == NULL)
    {
        MTEE_FreeMem(new_noncached_info->prealbuf);
        MTEE_FreeMem(new_noncached_info);
        return NULL;
    }

    list_initialize (&new_noncached_info->list);
    new_noncached_info->size = size;
    map_size = (size + SZ_4K - 1) & (~(SZ_4K-1));

    // flush cache
    MTEE_DmaMapMemory (new_noncached_info->prealbuf, size, MTEE_DMA_BIDIRECTION);

    // map to the noncached memory space
    MTEE_SetMMUTable (new_noncached_info->pnoncachedbuf, MTEE_Physical(new_noncached_info->prealbuf), map_size, PROT_SECURE_DEVICE);

    state = MTEE_SpinLockMaskIrq ((MTEE_SPINLOCK *) &ncm_lock);
    list_add_tail (&noncachedmemInfo.list, &new_noncached_info->list);
    MTEE_SpinUnlockRestoreIrq ((MTEE_SPINLOCK *) &ncm_lock, state);

    return new_noncached_info->pnoncachedbuf;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int KTEE_FreeNonCachedmem (void *buffer)
{
#if 0
    struct list_node *node;
    NONCACHEDMEM_INFO *ncm_info;
    uint32_t state;
    uint32_t size, map_size;
    void *prealbuf = NULL, *pnoncachedbuf = NULL;

    if(buffer == NULL) return -1;

    state = MTEE_SpinLockMaskIrq ((MTEE_SPINLOCK *) &ncm_lock);
    list_for_every(&noncachedmemInfo.list, node)
    {
        ncm_info = containerof(node, NONCACHEDMEM_INFO, list);
        if(buffer == ncm_info->pnoncachedbuf)
        {
            prealbuf = ncm_info->prealbuf;
            pnoncachedbuf = ncm_info->pnoncachedbuf;
            size = ncm_info->size;
            list_delete (&ncm_info->list);
            break;
        }
    }
    MTEE_SpinUnlockRestoreIrq ((MTEE_SPINLOCK *) &ncm_lock, state);

    if(prealbuf!=NULL && pnoncachedbuf != NULL)
    {
        // unmap the noncached memory space
        map_size = (size + SZ_4K - 1) & (~(SZ_4K-1));
        MTEE_SetMMUTable (pnoncachedbuf, 0, map_size, PROT_NO_ACCESS);
        MTEE_FreeSharedmem(pnoncachedbuf);
        MTEE_FreeMem(prealbuf);
        MTEE_FreeMem(ncm_info);
        return 0;
    }

    return -1;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int KTEE_InitVirtMappedmem(void *start, uint32_t size)
{
#if 0
    MemOne_init(&virtmappedmemInfo, start, size, SHM_PAGE_SIZE);
    MTEE_InitSpinlock((MTEE_SPINLOCK *) &vmm_lock, 0);

    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void *KTEE_AllocVirtMappedmemAlign(uint32_t alignment, uint32_t size)
{
#if 0
    void *buffer;
    uint32_t state;

    if (alignment < SHM_PAGE_SIZE)
        alignment = SHM_PAGE_SIZE;

    state = MTEE_SpinLockMaskIrq((MTEE_SPINLOCK *) &vmm_lock);

    buffer = MemOne_alloc(&virtmappedmemInfo, alignment, size);

    MTEE_SpinUnlockRestoreIrq((MTEE_SPINLOCK *) &vmm_lock, state);

    if (buffer == NULL)
    {
#ifdef DBG_MEM
        MTEE_LOG(MTEE_LOG_LVL_WARN, "MTEE_AllocVirtMappedMem: error, NULL buffer!\n");
#endif
        return NULL;
    }

    return buffer;
#endif
		print_mtee_orig_msg;
		return; //not support
}

/*fix mtee sync*/
int KTEE_FreeVirtMappedmem (void *buffer)
{
#if 0
    int ret;
    uint32_t state;

    state = MTEE_SpinLockMaskIrq((MTEE_SPINLOCK *) &vmm_lock);

    ret = MemOne_free(&virtmappedmemInfo, buffer);

    MTEE_SpinUnlockRestoreIrq((MTEE_SPINLOCK *) &vmm_lock, state);

    if (ret < 0)
    {
#ifdef DBG_MEM
        MTEE_LOG(MTEE_LOG_LVL_WARN, "MTEE_FreeSharedMem: error = 0x%x!\n", ret);
#endif
        return -1;
    }

    return 0;
#endif
		print_mtee_orig_msg;
		return; //not support
}

/*fix mtee sync*/
int KTEE_GetChunkMemoryPoolFromREE(unsigned long *chunkmem_pa, size_t *sz)
{
#if 0
    struct ree_service_chunk_mem *ree_chunk_mem;

    if (chunkmem_pa == NULL || sz == NULL)
        return -1;

    if (chunkmem_from_ree == 0) {
        ree_chunk_mem = (struct ree_service_chunk_mem *)MTEE_GetReeParamAddress();

        if(TZ_RESULT_SUCCESS == MTEE_ReeServiceCall(REE_SERV_GET_CHUNK_MEMPOOL)) {
            cm_pa = ree_chunk_mem->chunkmem_pa;
            cm_sz = ree_chunk_mem->size;
            chunkmem_from_ree = 1;
        }
        else
            return -1;
    }

    *chunkmem_pa = cm_pa;
    *sz = cm_sz;
    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int KTEE_ReleaseChunkMemoryPoolToREE(void)
{
#if 0
    if (chunkmem_from_ree != 0) {
        if (TZ_RESULT_SUCCESS ==
                MTEE_ReeServiceCall(REE_SERV_REL_CHUNK_MEMPOOL)) {
            chunkmem_from_ree = 0;
            cm_pa = 0;
            cm_sz = 0;
            return 0;
		}
    }

    return -1;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int KTEE_GetChunkMemoryPoolInfo(unsigned long *chunkmem_pa, size_t *size)
{
#if 0
    if (chunkmem_pa == NULL || size == NULL)
        return -1;

    if (chunkmem_from_ree != 0) {
        *chunkmem_pa = cm_pa;
        *size = cm_sz;
    } else {
        *chunkmem_pa = 0;
        *size = 0;
    }

    return 0;
#endif
	print_mtee_orig_msg;
	return; //not support
}


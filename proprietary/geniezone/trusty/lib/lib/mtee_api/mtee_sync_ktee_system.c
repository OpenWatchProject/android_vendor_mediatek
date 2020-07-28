#include <stdio.h>
#include <stdlib.h>

#include <tz_private/ktee_system.h>
#include <tz_private/log.h>

#if 0 
/*mapping to ktee_system.h*/
/*fix mtee sync*/
TZ_RESULT KTEE_RequestIrq(unsigned int irq_no, KTEE_IrqHandler handler,
                          unsigned int flags, void *user_data)
{
#if 0
	int rc;

	//check if irq num is valid
	if (irq >= MAX_INT)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	rc = request_irq(irq, flags);
	if (rc != NO_ERROR)
		return TZ_RESULT_ERROR_GENERIC;

	return TZ_RESULT_SUCCESS;
#endif
	
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_EnableIrq(unsigned int irq_no)
{
#if 0
    MTEE_DoEnableIrq(irq_no, 1);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_DisableIrq(unsigned int irq_no)
{
#if 0
		MTEE_DoEnableIrq(irq_no, 0);
#endif
	
	print_mtee_orig_msg;
	return; //not support
}
	
/*fix mtee sync*/
int KTEE_InIrqHanlder()
{
#if 0
		struct thread_stack *current_thread;
#ifndef __clang__
		register unsigned long tee_sp asm ("sp");
#else
		unsigned long tee_sp;
		asm volatile ("mov %0, sp"
				   :  "=r" (tee_sp));
#endif
		current_thread = (struct thread_stack*)(tee_sp&(~(THREAD_STACK_SIZE-1)));
		return current_thread->status_flag & THREAD_STATUS_IRQ;
#endif
	
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
TZ_RESULT KTEE_RegisterIrqCb(KTEE_IRQ_CB_FUNC_TYPE pf_trng_callback)
{
	/*no source code in original MTEE*/
	print_mtee_orig_msg;
	return; //not support	
}

#endif

#if 0
/*fix mtee sync*/
TZ_RESULT KTEE_EnableClock(unsigned int id, char *name)
{
#if 0
    return MTEE_ControlClock(id, name, REE_SERV_ENABLE_CLOCK);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
TZ_RESULT KTEE_DisableClock(unsigned int id, char *name)
{
#if 0
    return MTEE_ControlClock(id, name, REE_SERV_DISABLE_CLOCK);
#endif

	print_mtee_orig_msg;
	return; //not support
}
#endif

#if (GZ_API_MAIN_VERSION > 2)
/*fix mtee sync*/
TZ_RESULT KTEE_PMGet(const char *pmdev_name, int *res)
{
#if 0
    int ret;
    void *ptr = MTEE_GetReeParamAddress();

    if (pmdev_name == NULL)
        return TZ_RESULT_ERROR_BAD_PARAMETERS;

    strncpy(ptr, pmdev_name, REE_SERVICE_BUFFER_SIZE);
    ret = MTEE_ReeServiceCall(REE_SERV_PMGET);
    if (res != NULL)
        *res = *(int *)ptr;

    return ret;
#endif

	//MTEE_LOG(MTEE_LOG_LVL_WARN, "This method [%s] is not supported.\n", __func__);
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
TZ_RESULT KTEE_PMPut(const char *pmdev_name, int *res)
{
#if 0
    int ret;
    void *ptr = MTEE_GetReeParamAddress();

    if (pmdev_name == NULL) {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    strncpy(ptr, pmdev_name, REE_SERVICE_BUFFER_SIZE);
    ret = MTEE_ReeServiceCall(REE_SERV_PMPUT);
    if (res != NULL)
        *res = *(int *)ptr;

    return ret;
#endif

	//MTEE_LOG(MTEE_LOG_LVL_WARN, "This method [%s] is not supported.\n", __func__);
	print_mtee_orig_msg;
	return; //not support
}
#endif

/*fix mtee sync*/
uint32_t KTEE_SecureFunction (KTEE_SecureFunctionPrototype func, void *user_data)
{
#if 0
    uint32_t irq;
    uint32_t *addr = NULL, *end;
    uint32_t i;
    int valid;
    uint32_t ret;
    SECURE_STACK_LIST *info_p;
    uint32_t state;

    if (secure_stack_info.flag == 0)
    {
        return 0xFFFFFFFF;
    }

    valid = 0;
    info_p = NULL;
    state = MTEE_SpinLockMaskIrq(&secure_stack_info.lock);
    for (i = 0; i < SECURE_STACK_NUM; i ++)
    {
        info_p = (SECURE_STACK_LIST *) &secure_stack_info.stack[i];

        if (info_p->control == 0)
        {
            valid = 1;
            info_p = (SECURE_STACK_LIST *) &secure_stack_info.stack[i];
            info_p->control = 1;
            break;
        }
    }
    MTEE_SpinUnlockRestoreIrq(&secure_stack_info.lock, state);

    if (valid == 1)
    {
        addr = (uint32_t *) info_p->addr;
        end = (uint32_t *) info_p->end;

        irq = MTEE_SpinLockMaskIrq(&info_p->lock);

        addr[0] = addr[1] = SECURE_STACK_GUARD;
        end[0] = end[1] = SECURE_STACK_GUARD;

        _SecureFunction (func, user_data, addr);

        /* Check guard */
        if (addr[0] != SECURE_STACK_GUARD || addr[1] != SECURE_STACK_GUARD ||
            end[0] != SECURE_STACK_GUARD || end[1] != SECURE_STACK_GUARD)
        {
            printf("MTEE_SecureFunction: func %p data %p, guard overwritten\n",
                   func, user_data);
            assert(0);
        }

        MTEE_SpinUnlockRestoreIrq (&info_p->lock, irq);

        state = MTEE_SpinLockMaskIrq(&secure_stack_info.lock);
        info_p->control = 0;
        MTEE_SpinUnlockRestoreIrq(&secure_stack_info.lock, state);
        ret = 0;
    }
    else
    {
        ret = 0xFFFFFFFF;
    }

    return ret;
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void MTEE_DmaMapMemory(void *cpu_addr, unsigned int size,
                       KTEE_DMA_DIRECTION direction)
{
#if 0
    switch (direction)
    {
        case MTEE_DMA_TO_DEVICE:
            __ArmFlushDCache(cpu_addr, size);
            break;

        case MTEE_DMA_FROM_DEVICE:
            __ArmInvalidateDCache(cpu_addr, size);
            break;

        case MTEE_DMA_BIDIRECTION:
            __ArmFlushInvalidateDCache(cpu_addr, size);
            break;

        default:
            break;
    }
#endif
	
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void MTEE_DmaUnmapMemory(void *cpu_addr, unsigned int size,
                         KTEE_DMA_DIRECTION direction)
{
#if 0
    switch (direction)
    {
        case MTEE_DMA_FROM_DEVICE:
        case MTEE_DMA_BIDIRECTION:
            __ArmInvalidateDCache(cpu_addr, size);
            break;

        case MTEE_DMA_TO_DEVICE:
        default:
            break;
    }
#endif
	
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void *KTEE_Virtual(unsigned long phys_addr)
{
#if 0
    unsigned long phys_base;
    unsigned int phys_size;
    unsigned long chunk_phys_base;
    size_t chunk_phys_size;
// shift to avoid overflow
    phys_base = tee_memory_config.phys_memory_base>>1;
    phys_size = tee_memory_config.total_memory_size>>1;
    phys_addr >>= 1;

#ifdef DATA_ASLR_SUPPORT
    if (phys_addr >= _heap_phys_start/2 && phys_addr < phys_base + phys_size)
        return (void*)((((VIRT_OFFSET>>1) + phys_addr - phys_base)<<1) + _heap_data_offset);
#endif

    if (phys_addr >= phys_base && phys_addr < phys_base + phys_size)
        return (void*)(((VIRT_OFFSET>>1) + phys_addr - phys_base)<<1);

    MTEE_GetChunkMemoryPoolInfo(&chunk_phys_base, &chunk_phys_size);
    chunk_phys_base >>= 1;
    chunk_phys_size >>= 1;
    if (phys_size < chunk_phys_size &&
        phys_addr >= chunk_phys_base && phys_addr < chunk_phys_base + chunk_phys_size) {
#ifdef DATA_ASLR_SUPPORT
        return (void*)((unsigned long)(((VIRT_OFFSET>>1) + phys_size + phys_addr - chunk_phys_base)<<1) + _heap_data_offset);
#else
        return (void*)(((VIRT_OFFSET>>1) + phys_size + phys_addr - chunk_phys_base)<<1);
#endif
    }

    return 0;
#endif
	
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
unsigned long KTEE_Physical(void* virt_addr)
{
#if 0
	uint64_t pa;

	MTEE_VAToPA(virt_addr, &pa);
	return (unsigned long)pa;
#endif
		
	print_mtee_orig_msg;
	return; //not support
}

#if (GZ_API_MAIN_VERSION > 2)
/*fix mtee sync*/
uint32_t KTEE_IsKernelMemory(void *virt_addr, size_t size)
{
#if 0
    void *m_paddr, *m_vaddr;
    uint32_t m_size;
    m_vaddr = (void*)((unsigned long)virt_addr / 2);
    m_size = size / 2;

    if (MTEE_IsKernel32Memory(virt_addr, size))
        return true;

    /* outside the kerenel lowmem possible region  */
    if (m_vaddr < (void*)(0xffffffc000000000/2) ||
        (m_vaddr+m_size) >= (void*)(0xffffffc100000000/2)) /* limit as 4GB */
        return false;

    m_paddr = (void*)(MTEE_Physical(virt_addr) / 2);

    /* inside the available memory */
    if (m_paddr >= (void*)(DRAM_PHY_ADDR/2) &&
        (m_paddr+m_size) <= (void*)(tee_memory_config.phys_memory_base / 2))
        return true;

    /* in last one GB */
    if (m_paddr >= (void*)((DRAM_PHY_ADDR+0xc0000000)/2) &&
        (m_paddr+m_size) <= (void*)((DRAM_PHY_ADDR+0x100000000) / 2))
        return true;

    return false;
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int KTEE_IsNonSecureMemory(uint64_t phys_addr, size_t size)
{
#if 0
    uint32_t m_phys_addr = phys_addr/2;
    uint32_t m_size = size/2;

    if (m_phys_addr < DRAM_PHY_ADDR/2 ||
        m_phys_addr + m_size > tee_memory_config.phys_memory_base/2)
        return 0;

    return 1;
#endif
	
	print_mtee_orig_msg;
	return; //not support
}
#endif

#if 0
/*fix mtee sync*/
void KTEE_Puts(const char *buf)
{
#if 0
    char *ptr = MTEE_GetReeParamAddress();
    int num, len = strlen(buf);

    while (len)
    {
        num = (len < REE_SERVICE_BUFFER_SIZE) ? len : (REE_SERVICE_BUFFER_SIZE-1);
        strncpy(ptr, buf, num);
        len -= num;
        buf += num;
        ptr[num] = 0;
        MTEE_ReeServiceCall(REE_SERV_PUTS);
    }
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_USleep(unsigned long ustime)
{
#if 0
	nanosleep(0, 0, 1000 * ustime);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_UDelay(unsigned long ustime)
{
#if 0
	int64_t start, now, diff = 0;
	int64_t nstime = ustime * 1000;

	gettime(0, 0, &start);
	while(diff < nstime) {
        gettime(0, 0, &now);
		diff = now - start;
	}
#endif
	print_mtee_orig_msg;
	return; //not support
}
#endif

/*fix mtee sync*/
TZ_RESULT KTEE_CreateThread (KTEE_THREAD_HANDLE *handle, KTEE_ThreadFunc fuc, void *user_data, char *name)
{
#if 0
	if (!handle)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	ree_fork(MTEE_THREAD_REE_SESSION, 1);
	fork_threads(1, handle, func, args, arg_size);
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
TZ_RESULT KTEE_JoinThread (KTEE_THREAD_HANDLE handle, void **result)
{
#if 0
	long ret;
	if (!thread)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	ret = user_thread_join(&thread, INFINITE_TIME);
	if (ret < 0)
		return TZ_RESULT_ERROR_GENERIC;

	return TZ_RESULT_SUCCESS;
#endif
	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
TZ_RESULT KTEE_PM_RegisterDevOp(const struct KTEE_PM_DEV_OP *ppmop)
{
#if 0
    struct _MTEE_PM_OpList *ppmlst;
    uint32_t state;

    if(ppmops == NULL)
    {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    ppmlst = (struct _MTEE_PM_OpList*)malloc(sizeof(struct _MTEE_PM_OpList));
    if(ppmlst==NULL)
    {
        return TZ_RESULT_ERROR_OUT_OF_MEMORY;
    }

    list_initialize(&ppmlst->list);
    ppmlst->ppmdevops = ppmops;

    state = MTEE_SpinLockMaskIrq(&_gPMOpLock);
    list_add_tail(&_teepm_ops_list, &ppmlst->list);
    MTEE_SpinUnlockRestoreIrq(&_gPMOpLock, state);
    return TZ_RESULT_SUCCESS;
#endif
		
	print_mtee_orig_msg;
	return; //not support
}

#if 0
/*fix mtee sync*/
int KTEE_IsInEarlyStage()
{
#if 0
    return (int)(early_stage_leaved == 0);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
uint32_t KTEE_getReleaseCmSize (void)
{
#if 0
    return SECURE_TZMEM_RELEASECM_SIZE;
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_SetPlatformSecureWorldLeaveFunction(void (*func)(void))
{
#if 0
    platform_leaving_secure_world = func;
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int32_t KTEE_GetCurrentFreeStackSize(void)
{
#if 0
    unsigned long top_of_stack;
    unsigned long thrd_data = offsetof(struct thread_stack,stack);
#ifndef __clang__
    register unsigned long tee_sp asm ("sp");
#else
    unsigned long tee_sp;
    asm volatile ("mov %0, sp"
               :  "=r" (tee_sp));
#endif
    top_of_stack = (tee_sp&(~(THREAD_STACK_SIZE-1)));
    top_of_stack += thrd_data;

    return tee_sp - top_of_stack;
#endif
	
	print_mtee_orig_msg;
	return; //not support
}
#endif

/*fix mtee sync*/
void KTEE_EnterNeon(void)
{
#if 0
	ASSERT(0);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void KTEE_LeaveNeon(void)
{
#if 0
	ASSERT(0);
#endif

	print_mtee_orig_msg;
	return; //not support
}


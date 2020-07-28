#include <stdio.h>
#include <stdlib.h>

#include <tz_private/system.h>
#include <tz_private/log.h>

/*fix mtee sync*/
uint32_t MTEE_SecureFunction (MTEE_SecureFunctionPrototype func, void *user_data)
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
void MTEE_SetPlatformSecureWorldLeaveFunction(void (*func)(void))
{
#if 0
    platform_leaving_secure_world = func;
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void MTEE_EnterNeon(void)
{
#if 0
	ASSERT(0);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void MTEE_LeaveNeon(void)
{
#if 0
	ASSERT(0);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
TZ_RESULT MTEE_EnableClock(unsigned int id, char *name)
{
#if 0
    return MTEE_ControlClock(id, name, REE_SERV_ENABLE_CLOCK);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
TZ_RESULT MTEE_DisableClock(unsigned int id, char *name)
{
#if 0
    return MTEE_ControlClock(id, name, REE_SERV_DISABLE_CLOCK);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void *MTEE_Virtual(unsigned long phys_addr)
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
uint32_t MTEE_IsKernelMemory(void *virt_addr, size_t size)
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
int MTEE_IsNonSecureMemory(uint64_t phys_addr, size_t size)
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

/*fix mtee sync*/
TZ_RESULT MTEE_PM_RegisterDevOp(const struct MTEE_PM_DEV_OP *ppmops)
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

/*fix mtee sync*/
int MTEE_IsInEarlyStage()
{
#if 0
    return (int)(early_stage_leaved == 0);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
uint32_t MTEE_getReleaseCmSize (void)
{
#if 0
    return SECURE_TZMEM_RELEASECM_SIZE;
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
int32_t MTEE_GetCurrentFreeStackSize(void)
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


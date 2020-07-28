#include <trusty_std.h>
#include <tz_cross/trustzone.h>
#include <tz_private/system.h>
#include <tz_private/log.h>
#include <platform/gic.h>
#include <err.h>
#include <mm.h>
#include <tz_private/gz_swreg.h>

#define LOG_TAG "mtee_irq"

TZ_RESULT MTEE_RequestIrq(unsigned int irq, unsigned int flags)
{
	int rc;

	//check if irq num is valid
	if (irq >= MAX_INT)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	rc = request_irq(irq, flags);
	if (rc != NO_ERROR)
		return TZ_RESULT_ERROR_GENERIC;

	return TZ_RESULT_SUCCESS;
}

/*fix mtee sync*/
void MTEE_EnableIrq(unsigned int irq_no)
{
#if 0
    MTEE_DoEnableIrq(irq_no, 1);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
void MTEE_DisableIrq(unsigned int irq_no)
{
#if 0
		MTEE_DoEnableIrq(irq_no, 0);
#endif
	
	print_mtee_orig_msg;
	return; //not support
}
	
/*fix mtee sync*/
int MTEE_InIrqHanlder()
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

TZ_RESULT MTEE_RegisterIrqCb(MTEE_IRQ_CB_FUNC_TYPE pf_trng_callback)
{
	/*no source code in original MTEE*/
	print_mtee_orig_msg;
	return; //not support	
}

TZ_RESULT MTEE_ReleaseIrq(unsigned int irq)
{
	// check if irq num is valid
	if (irq >= MAX_INT)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	release_irq(irq);
	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_WaitIrq(unsigned int timeout_ms)
{
	int rc;
	rc = wait_irq(timeout_ms); // wait here

	if (rc == NO_ERROR) {
		return TZ_RESULT_SUCCESS;
	} else {
		return rc;
	}
}

TZ_RESULT MTEE_RequestGZRegIrq(unsigned int reg, unsigned int flags)
{
	int rc;
	unsigned int irq = reg + MAX_INT + 1;

	rc = request_irq(irq, flags);
	if (rc != NO_ERROR)
		return TZ_RESULT_ERROR_GENERIC;

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_ReleaseGZRegIrq(unsigned int reg)
{
	unsigned int irq = reg + MAX_INT + 1;
	release_irq(irq);
	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_MapGZReg(unsigned int reg, void** va_p)
{
	if (!va_p || reg >= GZREG_MAX_NUM)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	return MTEE_MmapIORegion(GZREG_BASE + (reg * GZREG_WIDTH), va_p, GZREG_WIDTH, MTEE_MAP_USER_DEFAULT);
}

TZ_RESULT MTEE_UnmapGZReg(void *va)
{
	if (!va)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	return MTEE_UnmmapIORegion(va, GZREG_WIDTH);
}

static struct GZ_data_reg_config dreg_list[GZ_DREG_NUM] = {
	{
		.id = GZ_DREG_FPSGO,
		.base = GZ_DREG_OFFSET_FPSGO,
		.size = GZ_DREG_SIZE_FPSGO,
		.va = NULL
	},
	{
		.id = GZ_DREG_CODEC,
		.base = GZ_DREG_OFFSET_CODEC,
		.size = GZ_DREG_SIZE_CODEC,
		.va = NULL
	}
};

static int is_overlapped(struct GZ_data_reg_config *old, struct GZ_data_reg_config *new)
{
	if (new->base < old->base) {
		if ((new->base + new->size) > old->base)
			return 1;
	} else { /* new.base > old.base*/
		if ((old->base + old->size) > new->base)
			return 1;
	}
	return 0;
}

TZ_RESULT MTEE_QueryDataReg(unsigned int _id, void** va_p)
{
	int i;
	struct GZ_data_reg_config *tmp;

	if (_id > GZ_DREG_NUM) {
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}

	tmp = &dreg_list[_id];

	DBG_LOG("dreg: map id %u,  pa 0x%llx, size %u\n",
		_id, GZ_DREG_PA(tmp->base), tmp->size);

	if (!tmp->va) {

		/* check if it's overlaped with others */
		for (i = 0; i < GZ_DREG_NUM; i++) {

			if (i == _id) continue;

			if (dreg_list[i].base && is_overlapped(&dreg_list[i], tmp)) {
				ERR_LOG("overlaped with dreg %d: base=0x%llx, size=%u\n",
						i, dreg_list[i].base, dreg_list[i].size);
				return TZ_RESULT_ERROR_GENERIC;
			}
		}

		tmp->va = mmap(NULL, tmp->size, MMAP_FLAG_IO_HANDLE, _id);
		DBG_LOG("dreg: map va = 0x%llx\n", tmp->va);
	}

	*va_p = tmp->va;

	if (*va_p) return TZ_RESULT_SUCCESS;
	else return TZ_RESULT_ERROR_GENERIC;
}

TZ_RESULT MTEE_UnmapDataReg(unsigned int _id)
{
	long rc;
	struct GZ_data_reg_config *tmp;
	TZ_RESULT ret = TZ_RESULT_SUCCESS;

	if (_id > GZ_DREG_NUM) {
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}

	tmp = &dreg_list[_id];

	DBG_LOG("dreg: unmap id %u\n", _id);
	if (tmp->va) {
		rc = munmap(tmp->va, tmp->size);
		tmp->va = NULL;

		if (rc) {
			ERR_LOG("dreg: munmap failed, rc = %ld\n", rc);
			ret = TZ_RESULT_ERROR_GENERIC;
		}
	}

	return ret;
}


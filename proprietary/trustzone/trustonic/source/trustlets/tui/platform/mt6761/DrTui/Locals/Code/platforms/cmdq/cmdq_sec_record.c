/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "drStd.h"
#include "DrApi/DrApiMm.h"
#include "DrApi/DrApi.h"

#include "cmdq_sec_record.h"
#include "cmdq_sec_core.h"
#include "cmdq_sec_reg.h"
#include "cmdq_sec_platform.h"
#include "tz_mem.h"

/* TUI only use one GCE thread, allocate handle array to max size 16 */
static cmdqRecStruct gRecordArray[CMD_MAX_FIXED_HANDLE];
static struct list_node gCmdqFreeRecord;

static uint32_t gCmdqTaskCookie = 1;
static uint8_t	*gCmdBufferBaseVA = 0;
static uint32_t gCmdBufferBasePA = 0;
static uint32_t gCmdqBufferShareSize = 0;

int cmdq_tui_set_base_pa(uint32_t physAddr, uint32_t size)
{
	(void)size;
	/*
	gCmdBufferBasePA = physAddr;
	gCmdqBufferShareSize = size;
	*/
	return 0;
}

int32_t cmdq_rec_initialize_cmd_buffer(void)
{
	int32_t status = 0;
	sec_mem_arg_t *sec_mem_arg = NULL;
	drApiResult_t result = DRAPI_OK;
	uint8_t *tee_parm_vaddr = 0;

	if((result = drApiMapPhysicalBuffer((uint64_t)TEE_PARAMETER_BASE,
		0x1000, MAP_READABLE | MAP_WRITABLE, (void **)&tee_parm_vaddr)) != DRAPI_OK) {
		CMDQ_ERR("map virtual address fail result:%d\n", result);
		return -CMDQ_ERR_FAULT;
	}

	sec_mem_arg = (sec_mem_arg_t*)(tee_parm_vaddr + (TEE_PARAMETER_ADDR - TEE_PARAMETER_BASE));
	if (!sec_mem_arg->shared_secmem) {
		CMDQ_LOG("Pre allocate CMDQ memory is not supported.\n");
		status = -CMDQ_ERR_FAULT;
	} else {
		gCmdBufferBasePA = sec_mem_arg->cmdq_mem_start;
		gCmdqBufferShareSize = sec_mem_arg->cmdq_mem_size;

		/* map this range as command buffer */
		status = cmdq_tz_map_physical_addr(&gCmdBufferBaseVA, (uint64_t)gCmdBufferBasePA, gCmdqBufferShareSize);
	}

	drApiUnmapBuffer(tee_parm_vaddr);

	return status;
}

int32_t cmdq_rec_release_cmd_buffer()
{
	return cmdq_tz_unmap_physical_addr(gCmdBufferBaseVA, gCmdqBufferShareSize);
}

void cmdqRecResetTaskCookie(void)
{
	gCmdqTaskCookie = 1;
}

int32_t cmdqRecInitialize(void)
{
	int32_t index;

	CMDQ_MSG("-->init: record\n");

	cmdq_rec_initialize_cmd_buffer();
	// Reset handle status
	// note t-base dose not support dynamic paging and limits secure drivers can not to talk with each other
	// we have to delay CMD buffers init when setup path resource,
	// and map/unmap CMD buffer to driver's virtual address space when receive a task submit from IPC each time
	list_initialize(&gCmdqFreeRecord);

	for (index = 0; index < CMD_MAX_FIXED_HANDLE; index++)
	{
		memset(&gRecordArray[index], 0, sizeof(cmdqRecStruct));
		gRecordArray[index].pBuffer = (uint32_t *)(gCmdBufferBaseVA + index*CMDQ_TZ_CMD_BLOCK_SIZE);
		gRecordArray[index].MVABase = gCmdBufferBasePA + index*CMDQ_TZ_CMD_BLOCK_SIZE;
		list_initialize(&(gRecordArray[index].listEntry));
		// be carefully that list parameter order is not same as linux version
		// secure world's list: list_add_tail(list, list_node)
		list_add_tail(&gCmdqFreeRecord, &(gRecordArray[index].listEntry));
	}

	gCmdqTaskCookie = 1;
	cmdqTzInitialize();
	cmdqTzSetupTaskResouce();

	CMDQ_MSG("-->init: record done\n");
	return 0;
}

int32_t cmdq_rec_realloc_cmd_buffer(cmdqRecHandle handle, uint32_t size)
{

	drApiResult_t status = DRAPI_OK;

	if (size <= handle->bufferSize) {
		return 0;
	}
	if (size > CMDQ_TZ_CMD_BLOCK_SIZE){
		CMDQ_ERR("The command buffer is over MAX size: %d !!\n", CMDQ_TZ_CMD_BLOCK_SIZE);
		return -1;
	}

	CMDQ_MSG("REC: realloc size from base:0x%p\n", handle->pBuffer);

	/* reset extended area */
	memset(handle->pBuffer, handle->bufferSize, size -handle->bufferSize);

	CMDQ_MSG("REC: realloc size from %d to %u bytes\n", handle->bufferSize, size);

	handle->bufferSize = size;

	return status;
}

int32_t cmdqRecCreate(CMDQ_SCENARIO_ENUM scenario, cmdqRecHandle *pHandle)
{
	cmdqRecHandle handle = NULL;
	int32_t status = 0;

	if (gCmdBufferBaseVA == NULL){
		CMDQ_ERR("CMDQ is not initialized !!\n");
		return -(CMDQ_REC_ERR_FAULT);
	}

	if (scenario < 0 || scenario >= CMDQ_MAX_SCENARIO_COUNT) {
		CMDQ_ERR("Unknown scenario type %d\n", scenario);
		return -(CMDQ_REC_ERR_FAULT);
	}

	handle = list_peek_head_type(&gCmdqFreeRecord, cmdqRecStruct, listEntry);
	do {
		if (!handle) {
			CMDQ_ERR("Handle: no free handle gCmdqFreeRecord:0x%p prev:0x%p next:0x%p\n",
				&gCmdqFreeRecord, gCmdqFreeRecord.prev, gCmdqFreeRecord.next);
			break;
		}

		/* init basic information */
		handle->scenario = scenario;
		/* No need to assign pBuffer and MVABase due to already set with fixed value in cmdqRecInitialize */
		handle->bufferSize = 0;
		handle->blockSize = 0;
		handle->engineFlag = cmdq_tz_rec_flag_from_scenario(scenario);
		handle->priority = CMDQ_THR_PRIO_NORMAL;
		handle->prefetchCount = 0;
		handle->finalized = false;

		/* command buffer related */
		/* CMD */
		if (0 != cmdq_rec_realloc_cmd_buffer(handle, CMDQ_TZ_CMD_BLOCK_SIZE)) {
			status = (CMDQ_REC_ERR_NOMEM);
			break;
		}

		/* 2. compose secure path command*/
		memset(handle->pBuffer, 0, CMDQ_TZ_CMD_BLOCK_SIZE);

		/* remove task form free list*/
		list_delete(&(handle->listEntry));
	} while(0);

	*pHandle = handle;

	return status;
}

/**
 * centralize the write/polling/read command for APB and GPR handle
 * this function must be called inside cmdq_append_command
 * because we ignore buffer and pre-fetch check here.
 * Parameter:
 *     same as cmdq_append_command
 * Return:
 *     same as cmdq_append_command
 */
static int32_t cmdq_append_wpr_command(cmdqRecHandle handle, CMDQ_CODE_ENUM code,
	uint32_t argA, uint32_t argB, uint32_t argAType, uint32_t argBType)
{
	int32_t subsys;
	uint32_t *pCommand;
	bool bUseGPR = false;
	/* use new argA to present final inserted argA*/
	uint32_t newArgA;
	uint32_t newArgAType = argAType;
	uint32_t argType = 0;

	/* be careful that subsys encoding position is different among platforms */
	const uint32_t subsysBit = cmdq_tz_get_subsys_LSB_in_argA();

	pCommand = (uint32_t *) ((uint8_t *) handle->pBuffer + handle->blockSize);

	if (CMDQ_CODE_READ != code && CMDQ_CODE_WRITE != code && CMDQ_CODE_POLL != code){
		CMDQ_ERR("Record 0x%p, flow error, should not append comment in wpr API", handle);
		return -(CMDQ_REC_ERR_FAULT);
	}

	/* we must re-calculate current PC at first. */
	pCommand = (uint32_t *) ((uint8_t *) handle->pBuffer + handle->blockSize);

	CMDQ_MSG("REC: 0x%p CMD: 0x%p, op: 0x%02x\n", handle,
		     pCommand, code);
	CMDQ_MSG("REC: 0x%p CMD: argA: 0x%08x, argB: 0x%08x, argAType: %d, argBType: %d\n",
		     handle, argA, argB, argAType, argBType);

	if (0 == argAType){
		/* argA is the HW register address to read from */
		subsys = cmdq_tz_subsys_from_phys_addr(argA);
		if (0 == argAType && 0 > subsys){
			CMDQ_ERR("REC: Unsupported memory base address 0x%08x\n", argA);
			return -(CMDQ_REC_ERR_FAULT);
		}else{
			/* compose final argA according to subsys table */
			newArgA = (argA & 0xffff) | ((subsys & 0x1f) << subsysBit);
		}
	}else{
		/* compose final argA according GPR value */
		newArgA = ((argA & 0x1f) << 16);
	}
	argType = (newArgAType << 2) | (argBType << 1);

	/* newArgA is the HW register address to access from or GPR value store the HW register address */
	/* argB is the value or register id  */
	/* bit 55: argA type, 1 for GPR */
	/* bit 54: argB type, 1 for GPR */
	/* argType: ('newArgAType', 'argBType', '0') */
	*pCommand++ = argB;
	*pCommand++ = (code << 24) | newArgA | (argType << 21);
	handle->blockSize += CMDQ_INST_SIZE;

	if (bUseGPR) {
		/* Set for GPR mutex token to leave mutex*/
		*pCommand++ = ((1 << 31) | (1 << 16));
		*pCommand++ = (CMDQ_CODE_WFE << 24) | CMDQ_SYNC_TOKEN_GPR_SET_4;
		handle->blockSize += CMDQ_INST_SIZE;
	}
	CMDQ_MSG("after wpr append, block size is %d\n", handle->blockSize);
	return 0;
}

int32_t cmdq_append_command(cmdqRecHandle handle, CMDQ_CODE_ENUM code,
	uint32_t argA, uint32_t argB, uint32_t argAType, uint32_t argBType)
{
	uint32_t *pCommand;

	pCommand = (uint32_t *)((uint8_t *) handle->pBuffer + handle->blockSize);

	if (handle->finalized) {
		CMDQ_ERR("Already finalized record 0x%p, cannot add more command", handle);
		return -(CMDQ_REC_ERR_BUSY);
	}

	/* check if we have sufficient buffer size */
	/* we leave a 4 instruction (4 bytes each) margin. */
	if ((handle->blockSize + 32) >= handle->bufferSize) {
		if (0 != cmdq_rec_realloc_cmd_buffer(handle, handle->bufferSize * 2)) {
			return -(CMDQ_REC_ERR_NOMEM);
		}
	}

	/* force insert MARKER if prefetch memory is full */
	/* GCE deadlocks if we don't do so */
	if (CMDQ_CODE_EOC != code && cmdq_tz_should_enable_prefetch(handle->scenario)) {
		if (handle->prefetchCount >= CMDQ_MAX_PREFETCH_INSTUCTION) {
			CMDQ_ERR("prefetchCount(%d) > MAX_PREFETCH_INSTUCTION, force insert disable prefetch marker\n",
						handle->prefetchCount);
			/* Mark END of prefetch section */
			cmdqRecDisablePrefetch(handle);
			/* BEGING of next prefetch section */
			cmdqRecMark(handle);
		} else {
			/* prefetch enabled marker exist */
			if (1 <= handle->prefetchCount) {
				++handle->prefetchCount;
				CMDQ_VERBOSE("handle->prefetchCount: %d, %s, %d\n", handle->prefetchCount, __func__, __LINE__);
			}
		}
	}

	/* we must re-calculate current PC because we may already insert MARKER inst. */
	pCommand = (uint32_t *) ((uint8_t *) handle->pBuffer + handle->blockSize);

	CMDQ_MSG("REC: 0x%p CMD: 0x%p, op: 0x%02x, argA: 0x%08x, argB: 0x%08x\n", handle,
		     pCommand, code, argA, argB);

	if (CMDQ_CODE_READ == code || CMDQ_CODE_WRITE == code || CMDQ_CODE_POLL == code){
		/* Because read/write/poll have similar format, handle them together*/
		return cmdq_append_wpr_command(handle, code, argA, argB, argAType, argBType);
	}

	switch (code) {
	case CMDQ_CODE_MOVE:
		*pCommand++ = argB;
		*pCommand++ = CMDQ_CODE_MOVE << 24 | (argA & 0xffffff);
		break;
	case CMDQ_CODE_JUMP:
		*pCommand++ = argB;
		*pCommand++ = (CMDQ_CODE_JUMP << 24) | (argA & 0x0FFFFFF);
		break;
	case CMDQ_CODE_WFE:
		/* bit 0-11: wait_value, 1 */
		/* bit 15: to_wait, true */
		/* bit 31: to_update, true */
		/* bit 16-27: update_value, 0 */
		*pCommand++ = ((1 << 31) | (1 << 15) | 1);
		*pCommand++ = (CMDQ_CODE_WFE << 24) | argA;
		break;

	case CMDQ_CODE_SET_TOKEN:
		/* this is actually WFE(SYNC) but with different parameter */
		/* interpretation */
		/* bit 15: to_wait, false */
		/* bit 31: to_update, true */
		/* bit 16-27: update_value, 1 */
		*pCommand++ = ((1 << 31) | (1 << 16));
		*pCommand++ = (CMDQ_CODE_WFE << 24) | argA;
		break;

	case CMDQ_CODE_WAIT_NO_CLEAR:
		/* bit 0-11: wait_value, 1 */
		/* bit 15: to_wait, true */
		/* bit 31: to_update, false */
		*pCommand++ = ((0 << 31) | (1 << 15) | 1);
		*pCommand++ = (CMDQ_CODE_WFE << 24) | argA;
		break;

	case CMDQ_CODE_CLEAR_TOKEN:
		/* this is actually WFE(SYNC) but with different parameter */
		/* interpretation */
		/* bit 15: to_wait, false */
		/* bit 31: to_update, true */
		/* bit 16-27: update_value, 0 */
		*pCommand++ = ((1 << 31) | (0 << 16));
		*pCommand++ = (CMDQ_CODE_WFE << 24) | argA;
		break;

	case CMDQ_CODE_EOC:
		*pCommand++ = argB;
		*pCommand++ = (CMDQ_CODE_EOC << 24) | (argA & 0x0FFFFFF);
		break;

	case CMDQ_CODE_RAW:
		*pCommand++ = argB;
		*pCommand++ = argA;
		break;

	default:
		return -(CMDQ_REC_ERR_FAULT);
	}

	handle->blockSize += CMDQ_INST_SIZE;
	CMDQ_MSG("after append, block size is %d\n", handle->blockSize);
	return 0;
}

int32_t cmdqRecReset(cmdqRecHandle handle)
{
	if (!handle) {
		return -(CMDQ_REC_ERR_FAULT);
	}

	handle->blockSize = 0;
	handle->prefetchCount = 0;
	handle->finalized = false;

	/* reset secure path data */
	handle->enginesNeedDAPC = 0LL;
	handle->enginesNeedPortSecurity = 0LL;

	return 0;
}

int32_t cmdqRecSecureEnableDAPC(cmdqRecHandle handle, const uint64_t engineFlag)
{
	handle->enginesNeedDAPC |= engineFlag;
	return 0;
}

int32_t cmdqRecSecureEnablePortSecurity(cmdqRecHandle handle, const uint64_t engineFlag)
{
	handle->enginesNeedPortSecurity |= engineFlag;
	return 0;
}

int32_t cmdqRecMark(cmdqRecHandle handle)
{
	int32_t status;

	/* Do not check prefetch-ability here. */
	/* because cmdqRecMark may have other purposes. */

	/* bit 53: non-suspendable. set to 1 because we don't want */
	/* CPU suspend this thread during pre-fetching. */
	/* If CPU change PC, then there will be a mess, */
	/* because prefetch buffer is not properly cleared. */
	/* bit 48: do not increase CMD_COUNTER (because this is not the end of the task) */
	/* bit 20: prefetch_marker */
	/* bit 17: prefetch_marker_en */
	/* bit 16: prefetch_en */
	/* bit 0:  irq_en (set to 0 since we don't want EOC interrupt) */
	status = cmdq_append_command(handle,
				     CMDQ_CODE_EOC,
				     (0x1 << (53 - 32)) | (0x1 << (48 - 32)), 0x00130000, 0, 0);

	/* if we're in a prefetch region, */
	/* this ends the region so set count to 0. */
	/* otherwise we start the region by setting count to 1. */
	handle->prefetchCount = 1;

	if (0 != status) {
		return -(CMDQ_REC_ERR_FAULT);
	}

	return 0;
}

int32_t cmdqRecWrite(cmdqRecHandle handle, uint32_t addr, uint32_t value, uint32_t mask)
{
	int32_t status;

	if (0xFFFFFFFF != mask) {
		status = cmdq_append_command(handle, CMDQ_CODE_MOVE, 0, ~mask, 0, 0);
		if (0 != status) {
			return status;
		}

		addr = addr | 0x1;
	}

	status = cmdq_append_command(handle, CMDQ_CODE_WRITE, addr, value, 0, 0);
	if (0 != status) {
		return status;
	}

	return 0;
}

int32_t cmdqRecPoll(cmdqRecHandle handle, uint32_t addr, uint32_t value, uint32_t mask)
{
	int32_t status;

	status = cmdq_append_command(handle, CMDQ_CODE_MOVE, 0, ~mask, 0, 0);
	if (0 != status) {
		return status;
	}

	status = cmdq_append_command(handle, CMDQ_CODE_POLL, (addr | 0x1), value, 0, 0);
	if (0 != status) {
		return status;
	}

	return 0;
}

int32_t cmdqRecWait(cmdqRecHandle handle, CMDQ_EVENT_ENUM event)
{
	if (CMDQ_SYNC_TOKEN_INVALID == event || CMDQ_SYNC_TOKEN_MAX <= event || 0 > event) {
		return -(CMDQ_REC_ERR_INVAL);
	}

	return cmdq_append_command(handle, CMDQ_CODE_WFE, event, 0, 0, 0);
}


int32_t cmdqRecWaitNoClear(cmdqRecHandle handle, CMDQ_EVENT_ENUM event)
{
	if (CMDQ_SYNC_TOKEN_INVALID == event || CMDQ_SYNC_TOKEN_MAX <= event || 0 > event) {
		return -(CMDQ_REC_ERR_INVAL);
	}

	return cmdq_append_command(handle, CMDQ_CODE_WAIT_NO_CLEAR, event, 0, 0, 0);
}

int32_t cmdqRecClearEventToken(cmdqRecHandle handle, CMDQ_EVENT_ENUM event)
{
	if (CMDQ_SYNC_TOKEN_INVALID == event || CMDQ_SYNC_TOKEN_MAX <= event || 0 > event) {
		return -(CMDQ_REC_ERR_INVAL);
	}

	return cmdq_append_command(handle, CMDQ_CODE_CLEAR_TOKEN, event, 1,	/* actually this param is ignored. */
		0, 0);
}

int32_t cmdqRecSetEventToken(cmdqRecHandle handle, CMDQ_EVENT_ENUM event)
{
	if (CMDQ_SYNC_TOKEN_INVALID == event || CMDQ_SYNC_TOKEN_MAX <= event || 0 > event) {
		return -(CMDQ_REC_ERR_INVAL);
	}

	return cmdq_append_command(handle, CMDQ_CODE_SET_TOKEN, event, 1,	/* actually this param is ignored. */
		0, 0);
}

int32_t cmdqRecReadToDataRegister(cmdqRecHandle handle, uint32_t hwRegAddr,
				  CMDQ_DATA_REGISTER_ENUM dstDataReg)
{
#ifdef CMDQ_GPR_SUPPORT
	/* read from hwRegAddr(argA) to dstDataReg(argB) */
	return cmdq_append_command(handle, CMDQ_CODE_READ, hwRegAddr, dstDataReg, 0, 1);

#else
	CMDQ_ERR("func:%s failed since CMDQ dosen't support GPR\n", __func__);
	(void)handle;
	(void)hwRegAddr;
	(void)dstDataReg;
	return -(CMDQ_REC_ERR_FAULT);
#endif
}

int32_t cmdqRecWriteFromDataRegister(cmdqRecHandle handle,
				  CMDQ_DATA_REGISTER_ENUM srcDataReg, uint32_t hwRegAddr)
{
#ifdef CMDQ_GPR_SUPPORT
	const uint32_t subsys = cmdq_tz_subsys_from_phys_addr(hwRegAddr);
	const uint32_t subsysBit = cmdq_tz_get_subsys_LSB_in_argA();

	/* write HW register(argA) with data of GPR data register(argB)*/
	return cmdq_append_command(handle, CMDQ_CODE_WRITE, hwRegAddr, srcDataReg, 0, 1);
#else
	CMDQ_ERR("func:%s failed since CMDQ dosen't support GPR\n", __func__);
	(void)handle;
	(void)srcDataReg;
	(void)hwRegAddr;
	return -(CMDQ_REC_ERR_FAULT);
#endif /* CMDQ_GPR_SUPPORT */
}

int32_t cmdqRecEnablePrefetchNoMarker(cmdqRecHandle handle)
{
	int32_t status = 0;

	if (!handle->finalized) {
		/* with prefetch threads without marker we should start with */
		/* bit 48: no_inc_exec_cmds_cnt = 1 */
		/* bit 20: prefetch_mark = 0 */
		/* bit 17: prefetch_mark_en = 0 */
		/* bit 16: prefetch_en = 1 */
		status = cmdq_append_command(handle,
			CMDQ_CODE_EOC,
			(0x1 << (53 - 32)) | (0x1 << (48 - 32)), 0x00010000, 0, 0);
	}

	CMDQ_MSG("cmdqRecEnablePrefetchNoMarker, status:%d\n", status);
	return status;
}

int32_t cmdqRecEnablePrefetch(cmdqRecHandle handle)
{
	if (cmdq_tz_should_enable_prefetch(handle->scenario)) {
		/* enable prefetch */
        	CMDQ_VERBOSE("REC: enable prefetch\n");
		cmdqRecMark(handle);
		return 0;
	}
	CMDQ_ERR("not allow enable prefetch, scenario: %d\n", handle->scenario);
	return -(CMDQ_REC_ERR_FAULT);
}

int32_t cmdqRecDisablePrefetch(cmdqRecHandle handle)
{
	uint32_t argB = 0;
	uint32_t argA = 0;
	int32_t status = 0;

	if (!handle->finalized) {

        if (handle->prefetchCount > 0) {
        /* with prefetch threads we should end with */
        /* bit 48: no_inc_exec_cmds_cnt = 1 */
        /* bit 20: prefetch_mark = 1 */
        /* bit 17: prefetch_mark_en = 0 */
        /* bit 16: prefetch_en = 0 */
            argB = 0x00100000;
            argA = (0x1 << 16); /* not increse execute counter */
            /* since we're finalized, no more prefetch */
            handle->prefetchCount = 0;
            status = cmdq_append_command(handle, CMDQ_CODE_EOC, argA, argB, 0, 0);
        }

        if (0 != status) {
            return status;
        }
    }

    CMDQ_MSG("cmdqRecDisablePrefetch, status:%d\n", status);
    return status;
}

int32_t cmdq_rec_finalize_command(cmdqRecHandle handle, bool loop)
{
	int32_t status = 0;
	uint32_t argB = 0;

	if (!handle->finalized) {
		if ((handle->prefetchCount > 0) && cmdq_tz_should_enable_prefetch(handle->scenario)) {
			CMDQ_ERR("not insert prefetch disble marker when prefetch enabled, prefetchCount:%d\n", handle->prefetchCount);
			cmdqRecDumpCommand(handle);

			status = -(CMDQ_REC_ERR_FAULT);
			return status;
		}

		/* insert EOF instruction */
		argB = 0x1;	/* generate IRQ for each command iteration */
		status = cmdq_append_command(handle, CMDQ_CODE_EOC, 0, argB, 0, 0);

		if (0 != status) {
			return status;
		}

		/* insert JUMP to loop to beginning or as a scheduling mark(8) */
		status = cmdq_append_command(handle, CMDQ_CODE_JUMP, 0,	/* not absolute */
					     loop ? -handle->blockSize : 8, 0, 0);
		if (0 != status) {
			return status;
		}

		handle->finalized = true;
	}

	return status;
}

int32_t cmdq_rec_make_message(cmdqRecHandle handle, struct iwcCmdqMessage_t *outMessage)
{
	int32_t status;

	status = cmdq_rec_finalize_command(handle, false);
	if (status < 0) {
		return status;
	}

	CMDQ_MSG("test: before dump command, command size:%d\n", handle->blockSize);
	/* cmdqRecDumpCommand(handle); */
	CMDQ_MSG("test: after dump command\n");

	CMDQ_MSG("Submit task scenario: %d, priority: %d, engine: 0x%x%08x, size: %d, buffer: 0x%08x\n",
		 handle->scenario, handle->priority, (uint32_t)((handle->engineFlag) >> 32), (uint32_t)(handle->engineFlag),
		 handle->blockSize, handle->pBuffer);

	outMessage->command.scenario = handle->scenario;
	outMessage->command.priority = handle->priority;
	outMessage->command.engineFlag = handle->engineFlag;
	outMessage->command.thread = cmdq_tz_thread_index_from_scenario(handle->scenario);
	outMessage->command.pVABase = handle->pBuffer;
	outMessage->command.MVABase = handle->MVABase;
	outMessage->command.commandSize = handle->blockSize;
	if (gCmdqTaskCookie == 1)
		outMessage->command.resetExecCnt = true;

	outMessage->command.waitCookie = gCmdqTaskCookie++;
	if (gCmdqTaskCookie > CMDQ_MAX_COOKIE_VALUE) {
		/* Reach the maximum cookie */
		gCmdqTaskCookie = 0;
	}
	return status;
}

int32_t cmdqRecFlush(cmdqRecHandle handle)
{
	int32_t status;
	struct iwcCmdqMessage_t message = {0};

	status = cmdq_rec_make_message(handle, &message);
	if (status < 0) {
		return status;
	}

	return cmdqTzComposeAndSubmitTask(&message, NULL);
}

int32_t cmdqRecFlushNoWait(cmdqRecHandle handle)
{
	int32_t status;
	struct iwcCmdqMessage_t message = {0};

	status = cmdq_rec_make_message(handle, &message);
	if (status < 0) {
		CMDQ_ERR("Make message failed\n");
		return status;
	}

	status = cmdqTzComposeAndSubmitTask(&message, NULL);
	if (status < 0) {
		CMDQ_ERR("Submit task failed failed\n");
	}
	return status;
}

int32_t cmdqRecGetInstructionCount(cmdqRecHandle handle)
{
	if (!handle)
		return 0;

	return (handle->blockSize / CMDQ_INST_SIZE);
}

void cmdq_rec_dump_buffer(void* buffer, uint32_t size, char* bufferName)
{
	uint32_t instrCount = (size / 8);
	uint32_t* pCMD = (uint32_t*) (buffer);
	uint32_t i = 0;

	// 64 bit per instruction
	for(i = 0;  i < instrCount; i = i + 1)
	{
		uint32_t indexA = 2 * i + 1;
		uint32_t indexB = 2 * i;
		CMDQ_MSG(" ...%s[%3d] = 0x%08x, %s[%3d] = 0x%08x\n",
			bufferName, (indexA), pCMD[indexA],
			bufferName, (indexB), pCMD[indexB]);
	}
}

int32_t cmdqRecDumpCommand(cmdqRecHandle handle)
{
	CMDQ_MSG("======REC 0x%p command buffer:\n", handle->pBuffer);
	cmdq_rec_dump_buffer(handle->pBuffer, handle->blockSize, "[RecDump]");
	CMDQ_MSG("======REC 0x%p command buffer END\n", handle->pBuffer);

	return 0;
}

void cmdqRecDestroy(cmdqRecHandle handle)
{
	if (!handle) {
		CMDQ_ERR("release NULL handle\n");
		return;
	}

	/* reset command buffer */
	memset(handle->pBuffer, 0, handle->bufferSize);
	/* Do not reset pBuffer since we need to keep it */
	/* handle->pBuffer = NULL; */
	handle->bufferSize = 0;

	// be carefully that list parameter order is not same as linux version
	// secure world's list: list_add_tail(list, list_node)
	list_add_tail(&gCmdqFreeRecord, &(handle->listEntry));
}

void test_cmdq(void)
{
#define CMDQ_TEST_MMSYS_DUMMY_OFFSET	(0xb0)	/* use GPR12 */
#define CMDQ_TEST_MMSYS_BASE_PA		GCE_BASE_PA
#define CMDQ_TEST_MMSYS_DUMMY_PA	(CMDQ_TEST_MMSYS_BASE_PA + CMDQ_TEST_MMSYS_DUMMY_OFFSET)

	uint8_t *va = 0;
	bool testMultiHandle = true;
	cmdqRecHandle handle;
	const uint32_t PATTERN = (1 << 0) | (1 << 2) | (1 << 16);
	uint32_t value = 0;
	const uint32_t MASK = (1 << 16);
	const uint32_t EXPECT_RESULT = PATTERN & MASK;

	if (!gCmdBufferBaseVA) {
		CMDQ_ERR("CMDQ is not initialized, Stop test!\n");
		return;
	} else {
		CMDQ_MSG("already initialized, skip in test\n");
	}

	CMDQ_MSG("start test cmdq\n");
	if (cmdq_tz_map_physical_addr(&va, CMDQ_TEST_MMSYS_BASE_PA, 0x1000) < 0)
		return;

	CMDQ_MSG("test: after map, va: 0x%p\n", va);
	CMDQ_MSG("test: after map, value: 0x%x\n", *(va+CMDQ_TEST_MMSYS_DUMMY_OFFSET));

	CMDQ_MSG("test: before set dummy\n");
	/* set to 0xFFFFFFFF */
	CMDQ_REG_SET32((va + CMDQ_TEST_MMSYS_DUMMY_OFFSET), ~0);
	CMDQ_MSG("test: after set dummy, value: 0x%x\n", *(va+CMDQ_TEST_MMSYS_DUMMY_OFFSET));

	/* Write test */
	CMDQ_MSG("test: before create\n");
	/* use CMDQ to set to PATTERN */
	cmdqRecCreate(CMDQ_SCENARIO_DEBUG, &handle);
	CMDQ_MSG("test: after create\n");
	cmdqRecReset(handle);
	cmdqRecWrite(handle, CMDQ_TEST_MMSYS_DUMMY_PA, PATTERN, ~0);
	CMDQ_MSG("test: after add write instr\n");
	cmdqRecFlush(handle);
	CMDQ_MSG("test: after flush call\n");

	/* value check */
	value = CMDQ_REG_GET32(va+CMDQ_TEST_MMSYS_DUMMY_OFFSET);
	if (value != PATTERN) {
		CMDQ_ERR("TEST FAIL: wrote value is 0x%08x, not 0x%08x\n", value, PATTERN);
	} else {
		CMDQ_MSG("TEST PASS: wrote value is 0x%08x\n", value);
	}

	if (testMultiHandle) {
		cmdqRecDestroy(handle);
		cmdqRecCreate(CMDQ_SCENARIO_DEBUG, &handle);
	}

	/* Write with mask test */
	/* set to 0x0 */
	CMDQ_REG_SET32(va+CMDQ_TEST_MMSYS_DUMMY_OFFSET, 0x0);

	/* use CMDQ to set to PATTERN */
	cmdqRecReset(handle);
	cmdqRecWrite(handle, CMDQ_TEST_MMSYS_DUMMY_PA, PATTERN, MASK);
	cmdqRecFlush(handle);

	/* value check */
	value = CMDQ_REG_GET32(va+CMDQ_TEST_MMSYS_DUMMY_OFFSET);
	if (EXPECT_RESULT != value ) {
		CMDQ_ERR("TEST FAIL: wrote value with mask is 0x%08x, not 0x%08x\n", value, EXPECT_RESULT);
	} else {
		CMDQ_MSG("TEST PASS: wrote value with mask is 0x%08x\n", value);
	}

	if (testMultiHandle) {
		cmdqRecDestroy(handle);
		cmdqRecCreate(CMDQ_SCENARIO_DEBUG, &handle);
	}

	/* polling with mask test */
	/* set to 0x0 */
	CMDQ_REG_SET32(va+CMDQ_TEST_MMSYS_DUMMY_OFFSET, 0x0);

	/* use CMDQ to set to PATTERN */
	cmdqRecReset(handle);
	cmdqRecWrite(handle, CMDQ_TEST_MMSYS_DUMMY_PA, PATTERN, MASK);
	cmdqRecPoll(handle, CMDQ_TEST_MMSYS_DUMMY_PA, PATTERN, MASK);
	cmdqRecFlush(handle);

	/* value check */
	value = CMDQ_REG_GET32(va+CMDQ_TEST_MMSYS_DUMMY_OFFSET);
	if (EXPECT_RESULT != value) {
		CMDQ_ERR("TEST FAIL: polling value with mask is 0x%08x, not 0x%08x\n", value, EXPECT_RESULT);
	} else {
		CMDQ_MSG("TEST PASS: polling value with mask is 0x%08x\n", value);
	}

	if (testMultiHandle) {
		cmdqRecDestroy(handle);
		cmdqRecCreate(CMDQ_SCENARIO_DEBUG, &handle);
	}

	/* Event handle test */
	/* set to 0x0 */
	CMDQ_REG_SET32(va+CMDQ_TEST_MMSYS_DUMMY_OFFSET, 0x0);

	/* use CMDQ to set to PATTERN */
	cmdqRecReset(handle);
	cmdqRecSetEventToken(handle, CMDQ_SYNC_TOKEN_USER_0);
	cmdqRecWaitNoClear(handle, CMDQ_SYNC_TOKEN_USER_0);
	cmdqRecWrite(handle, CMDQ_TEST_MMSYS_DUMMY_PA, PATTERN, MASK);
	cmdqRecClearEventToken(handle, CMDQ_SYNC_TOKEN_USER_0);
	cmdqRecFlush(handle);

	/* value check */
	value = cmdqTzCoreGetEvent(CMDQ_SYNC_TOKEN_USER_0);
	if (0 != value) {
		CMDQ_ERR("TEST FAIL: event value is not clear: %d\n", value);
	}else{
		CMDQ_MSG("TEST PASS: event value is clear\n");
	}
#ifdef CMDQ_POLL_TIMEOUT
	if (testMultiHandle) {
		cmdqRecDestroy(handle);
		cmdqRecCreate(CMDQ_SCENARIO_PRIMARY_DISP, &handle);
	}

	/* clear token */
	CMDQ_REG_SET32(CMDQ_SYNC_TOKEN_UPD, CMDQ_SYNC_TOKEN_USER_0);

	/* use CMDQ to set to PATTERN */
	cmdqRecReset(handle);
	cmdqRecClearEventToken(handle, CMDQ_SYNC_TOKEN_USER_0);
	cmdq_append_command(handle, CMDQ_CODE_RAW, 0xffff, 0x0, 0, 0);
	cmdqRecWait(handle, CMDQ_SYNC_TOKEN_USER_0);
	cmdqRecFlush(handle);
	CMDQ_ERR("TEST: error handling\n");
#endif
	cmdqRecDestroy(handle);
	cmdq_tz_unmap_physical_addr(va, 0x1000);
	CMDQ_MSG("end test cmdq\n");
}

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

#include "cmdq_sec_core.h"
#include "cmdq_sec_reg.h"
#include "cmdq_sec_platform.h" /* platform  impl*/

#include "cmdq_sec_record.h"

/***************************************************************************
SECURE CMDQ Flow

1. Need enable DAPC protect
	- goal: to prevent dump secure data via register access in NWd

2. trigger a secure thread
	- warm reset ( per thread 0x100)
	- config time out cycles (per thread 0x150)
	- enable IRQ
	- set secure thread bit (per thread 0x118)
	- config PC & command end add (per thread 0x120/0x124)
	- enable thread (per thread 0x104)

3. register secure_IRQ_FLAG (0x14) to handle secure task executed result


Note:
1. secure world dose not support following util:
	- spinlock / watiqueue / complete
	- GPT time query

****************************************************************************/
/* debug */
#define CMDQ_DEBUG (0)

/* global variable */
static ContextStruct		gCmdqContext;
static struct list_node		gCmdqFreeTask;
uint8_t		*gCmdqRegBaseVA = 0;
uint8_t		*gCmdqSharedCookieVA = 0;

#define CMDQ_GET_COOKIE_CNT(thread) (CMDQ_REG_GET32(CMDQ_THR_EXEC_CNT(thread)) & CMDQ_MAX_COOKIE_VALUE)

/* function declaration */
void cmdq_tz_remove_task_from_thread_array_by_cookie(
		ThreadStruct *pThread, int32_t index, TASK_STATE_ENUM targetTaskState);
void cmdq_tz_remove_task_from_thread_array_and_release_task_by_cookie(
		TaskStruct *pTask, int32_t thread, const uint32_t cookie, TASK_STATE_ENUM targetTaskState);

const static uint64_t gCmdqEngineGroupBits[CMDQ_MAX_GROUP_COUNT] = { CMDQ_ENG_ISP_GROUP_BITS,
	CMDQ_ENG_MDP_GROUP_BITS,
	CMDQ_ENG_DISP_GROUP_BITS,
	CMDQ_ENG_JPEG_GROUP_BITS,
	CMDQ_ENG_VENC_GROUP_BITS
};

bool cmdq_tz_is_valid_group(CMDQ_GROUP_ENUM engGroup)
{
	/* check range */
	if (engGroup < 0 || engGroup >= CMDQ_MAX_GROUP_COUNT) {
		return false;
	}
	return true;
}

int32_t cmdq_tz_is_group_flag(CMDQ_GROUP_ENUM engGroup, uint64_t engineFlag)
{
	if (!cmdq_tz_is_valid_group(engGroup)) {
		return false;
	}

	if (gCmdqEngineGroupBits[engGroup] & engineFlag) {
		return true;
	}
	return false;
}

void cmdq_tz_print_hex_dump(const char *prefix_str, const void *buf, size_t len)
{
    const uint32_t rowsize = 16;
    int32_t lineLen = 0;
    int32_t remaining = len;
    int32_t i;
    uint32_t *ptr;

	if(NULL == buf)
	{
		return;
	}

    drDbgPrintf("%s hex dump, buf: 0x%08x, len: %d\n", prefix_str, buf, len);
	for (i = 0; i < len; i += rowsize)
    {
		lineLen = (remaining < rowsize) ? (remaining) : (rowsize);
		remaining = remaining - rowsize;

        ptr = (uint32_t*)(buf + i);
        if(rowsize == lineLen)
        {
            drDbgPrintf("%s %08x: %08x %08x %08x %08x\n", prefix_str, ptr, ptr[0], ptr[1], ptr[2], ptr[3]);
            continue;
        }

        // calucate word count in lst line
        switch (lineLen / 4)
        {
        case 3:
            drDbgPrintf("%s %08x: %08x %08x %08x\n", prefix_str, ptr, ptr[0], ptr[1], ptr[2]);
            break;
        case 2:
            drDbgPrintf("%s %08x: %08x %08x\n", prefix_str, ptr, ptr[0], ptr[1]);
            break;
        case 1:
            drDbgPrintf("%s %08x: %08x\n", prefix_str, ptr, ptr[0]);
            break;
        case 0:
            drDbgPrintf("%s %08x:  %08x %08x %08x %08x\n", prefix_str, ptr, ptr[0], ptr[1], ptr[2], ptr[3]);
            break;
        default:
            break;
        }
    }
}
static void cmdq_tz_append_command_into_task(TaskStruct *pTask, uint32_t argA, uint32_t argB)
{
	pTask->pCMDEnd[1] = argB;
	pTask->pCMDEnd[2] = argA;
	pTask->commandSize += 1 * CMDQ_INST_SIZE;
	pTask->pCMDEnd += 2;
}

void cmdq_tz_reset_engine_struct(void)
{
	struct EngineStruct *pEngine;
	int index;

	/* Reset engine status */
	pEngine = gCmdqContext.engine;
	for (index = 0; index < CMDQ_MAX_ENGINE_COUNT; index++) {
		pEngine[index].currOwner = CMDQ_INVALID_THREAD;
	}
}

int32_t cmdq_tz_map_physical_addr(uint8_t **va, const uint64_t pa, const uint32_t length)
{
	const uint32_t flag = (MAP_READABLE | MAP_WRITABLE | MAP_UNCACHED | MAP_IO);
	const drApiResult_t drRes = drApiMapPhysicalBuffer(pa, length, flag, (void **)va);

	if (DRAPI_OK != drRes) {
		CMDQ_ERR("drApiMapPhysicalBuffer(0x%x, 0x%x, 0x%x) = ERR(%x, %x:%x)\n",
			*va, pa, length, drRes, DRAPI_ERROR_MAJOR(drRes), DRAPI_ERROR_DETAIL(drRes));
		return -(CMDQ_REC_ERR_NOMEM);
	}else{
		CMDQ_MSG("drApiMapPhysicalBuffer(0x%x, 0x%x, 0x%x) = STATUS(%x)\n",
			*va, pa, length, drRes);
	}

	return 0;
}

int32_t cmdq_tz_unmap_physical_addr(const uint8_t *va, const uint32_t length)
{
	const drApiResult_t status = drApiUnmapBuffer(va);

	if (DRAPI_OK != status) {
		CMDQ_ERR("unmap_physical_addr(0x%x, 0x%x) = ERR(%x, %x:%x)\n",
			va, length, status, DRAPI_ERROR_MAJOR(status), DRAPI_ERROR_DETAIL(status));
		return -(CMDQ_REC_ERR_NOMEM);
	}

	return 0;
}

int32_t cmdq_tz_init_module_base_VA(void)
{
	int32_t status = 0;
	cmdq_tz_map_physical_addr(&gCmdqRegBaseVA, GCE_BASE_PA, 0x1000);

	return status;
}

void cmdq_tz_deinit_module_base_VA(void)
{
	cmdq_tz_unmap_physical_addr(gCmdqRegBaseVA, 0x1000);
}

int32_t cmdqTzInitialize(void)
{
	TaskStruct			*pTask;
	int32_t			   index;

	CMDQ_MSG("-->init: core\n");

	// secure: register map
	if(0 > cmdq_tz_init_module_base_VA())
	{
		return -1;
	} else {
		CMDQ_LOG("INIT: map module to VA base 0x%08x\n", gCmdqRegBaseVA);
	}

	// Reset overall context
	memset(&gCmdqContext, 0x0, sizeof(ContextStruct));

	// Reset engine status
	cmdq_tz_reset_engine_struct();

	// Reset task status
	// note t-base dose not support dynamic paging and limits secure drivers can not to talk with each other
	// we have to delay CMD buffers init when setup path resource,
	// and map/unmap CMD buffer to driver's virtual address space when receive a task submit from IPC each time
	list_initialize(&gCmdqFreeTask);
	pTask = gCmdqContext.taskInfo;

	for (index = 0; index < CMDQ_MAX_FIXED_TASK; index++)
	{
		list_initialize(&(pTask[index].listEntry));
		memset(&pTask[index], 0, sizeof(TaskStruct));

		pTask[index].taskState = TASK_STATE_IDLE;
		/* Init CMD buffer when setup secure path resource*/
		pTask[index].pVABase = NULL;
		pTask[index].pCMDEnd = NULL;
		pTask[index].MVABase = 0; /* Delay CMD buffer init when setup path resource */

		// be carefully that list parameter order is not same as linux version
		// secure world's list: list_add_tail(list, list_node)
		list_add_tail(&gCmdqFreeTask, &(pTask[index].listEntry));
	}

	// skip: clear CMDQ event (processed in normal world)
	// skip: not support proc debug in secure world

	CMDQ_MSG("-->init: core done[%d]\n");
	return 0;
}


ThreadStruct* cmdq_tz_get_thread_struct_by_id(const int32_t thread, const int32_t line)
{
	if(0 > thread || CMDQ_MAX_THREAD_COUNT <= thread) {
		CMDQ_ERR("Thread: %d get failed\n", thread);
		return NULL;
	}

	// support one thread in secure world now
	// the thread id is dispatched from normal world, and secure context
	ThreadStruct* pThread = &(gCmdqContext.thread[thread]);
	return pThread;
}

void cmdq_tz_dump_array(int32_t *pList, int32_t length, const char* tag)
{
	int32_t i = 0;
	for(i = 0; i < length; i++)
	{
		CMDQ_MSG("...%s[%2d]: 0x%08x\n", tag, i, pList[i]);
	}
}

void cmdq_tz_dump_buffer(void* buffer, uint32_t size, char* bufferName)
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

void cmdq_tz_dump_free_task(bool dumpTask)
{
	TaskStruct *pTask = NULL;
	uint32_t index = 0;

	CMDQ_MSG("============================ cmdq_tz_dump_free_list BEGIN ============================\n");

	list_for_every_entry(&gCmdqFreeTask, pTask, TaskStruct, listEntry){

		if(dumpTask){
			CMDQ_ERR( "idx:%d, Task: 0x%08x, SCE: %d, State: %d, Priority: %d, ENG:0x%x%08x, VABase: 0x%08x, MVABase: 0x%08x\n",
				index, (uint32_t)pTask, pTask->scenario, pTask->taskState, pTask->priority,
				(uint32_t)((pTask->engineFlag) >> 32), (uint32_t)(pTask->engineFlag), (uint32_t)pTask->pVABase, pTask->MVABase);
		} else {
			CMDQ_MSG("idx:%d:%d, pTask:0x%08x\n", index, pTask);
		}
		index ++;
	}

	CMDQ_MSG("============================ cmdq_tz_dump_free_list END ============================\n");
}

void cmdq_tz_dump_thread(uint32_t thread, uint32_t line)
{
	ThreadStruct *pThread = NULL;
	uint32_t value[10] = { 0 };
	pThread = cmdq_tz_get_thread_struct_by_id(thread, __LINE__);

	value[0] = CMDQ_REG_GET32(CMDQ_THR_CURR_ADDR(thread));
	value[1] = CMDQ_REG_GET32(CMDQ_THR_END_ADDR(thread));
	value[2] = CMDQ_REG_GET32(CMDQ_THR_WAIT_TOKEN(thread));
	value[3] = CMDQ_GET_COOKIE_CNT(thread);
	value[4] = CMDQ_REG_GET32(CMDQ_THR_IRQ_STATUS(thread));
	value[5] = CMDQ_REG_GET32(CMDQ_THR_INST_CYCLES(thread));
	value[6] = CMDQ_REG_GET32(CMDQ_THR_CURR_STATUS(thread));
	value[7] = CMDQ_REG_GET32(CMDQ_THR_IRQ_ENABLE(thread));
	value[8] = CMDQ_REG_GET32(CMDQ_THR_ENABLE_TASK(thread));
	value[9] = CMDQ_REG_GET32(CMDQ_THR_SECURITY(thread));

	CMDQ_ERR("Index: %d, Enabled: %d, IRQ: 0x%08x, Thread PC: 0x%08x, End: 0x%08x, Wait Token: 0x%08x\n",
		 thread, value[8], value[4], value[0], value[1], value[2]);
	CMDQ_ERR("Curr Cookie: %d, Wait Cookie: %d, Next Cookie: %d, Task Count %d,\n",
		value[3], pThread->waitCookie, pThread->nextCookie, pThread->taskCount);
	CMDQ_ERR("Timeout Cycle:%d, Status:0x%08x, IRQ_EN: 0x%08x, Secure: %d\n",
		value[5], value[6], value[7], value[9]);
}

void cmdq_tz_dump_task(const TaskStruct *pTask)
{
	CMDQ_ERR("Task: 0x%p, Scenario: %d, State: %d, Priority: %d, Flag: 0x%x%08x, VABase: 0x%08x\n",
		 pTask, pTask->scenario, pTask->taskState, pTask->priority,
		 (uint32_t)((pTask->engineFlag) >> 32), (uint32_t)(pTask->engineFlag),
		 (uint32_t) pTask->pVABase);

	/* dump last Inst only when VALID command buffer */
	/* otherwise data abort is happened */
	if (pTask->pVABase) {
		CMDQ_ERR("CMDEnd: 0x%p, MVABase: %x, Size: %d, Last Inst: 0x%08x:0x%08x, 0x%08x:0x%08x\n",
			pTask->pCMDEnd, (pTask->MVABase), pTask->commandSize, pTask->pCMDEnd[-3],
			pTask->pCMDEnd[-2], pTask->pCMDEnd[-1], pTask->pCMDEnd[0]);
	} else {
		CMDQ_ERR("CMDEnd: 0x%p, MVABase: %pa, Size: %d\n",
			pTask->pCMDEnd, &pTask->MVABase, pTask->commandSize);
	}

	CMDQ_ERR("Caller pid:%d name:%s\n", pTask->callerPid, pTask->callerName);

	return;
}

void cmdq_tz_dump_task_usage_impl(TaskStruct *pTask, bool dumpCmd)
{
	if (NULL == pTask) {
		CMDQ_ERR("dump_task_usage_impl, task[null]\n");
		return;
	}

	cmdq_tz_dump_task(pTask);

	if (dumpCmd && NULL != pTask->pVABase) {
		cmdq_tz_print_hex_dump("[CMDQ][tz]", pTask->pVABase, pTask->commandSize);
	}
}

const char *cmdq_tz_get_event_name(CMDQ_EVENT_ENUM event)
{
#undef DECLARE_CMDQ_EVENT
#define DECLARE_CMDQ_EVENT(name, val) case val:return #name;

	switch (event) {
#include "cmdq_sec_event.h"
	}

	return "CMDQ_EVENT_UNKNOWN";
#undef DECLARE_CMDQ_EVENT
}

void cmdqTzCoreClearEvent(CMDQ_EVENT_ENUM event)
{
	CMDQ_MSG("clear event %d\n", event);
	CMDQ_REG_SET32(CMDQ_SYNC_TOKEN_UPD, event);
}

void cmdqTzCoreSetEvent(CMDQ_EVENT_ENUM event)
{
	CMDQ_REG_SET32(CMDQ_SYNC_TOKEN_UPD, (1L << 16) | event);
}

uint32_t cmdqTzCoreGetEvent(CMDQ_EVENT_ENUM event)
{
	uint32_t regValue = 0;
	CMDQ_REG_SET32(CMDQ_SYNC_TOKEN_ID, (0x3FF & event));
	regValue = CMDQ_REG_GET32(CMDQ_SYNC_TOKEN_VAL);
	return regValue;
}

uint32_t *cmdq_tz_get_pc(const TaskStruct *pTask, uint32_t thread,
					uint32_t insts[4])
{
	long currPC = 0L;
	uint8_t *pInst = NULL;

	insts[0] = 0;
	insts[1] = 0;
	insts[2] = 0;
	insts[3] = 0;

	currPC = CMDQ_AREG_TO_PHYS(CMDQ_REG_GET32(CMDQ_THR_CURR_ADDR(thread)));
	pInst = (uint8_t *) pTask->pVABase + (currPC - pTask->MVABase);

	/* CMDQ_ERR("[PC]thr:%d, paPC:0x%08x, pVABase:0x%08x, PABase:0x%08x\n", thread, currPC, pTask->pVABase, pTask->MVABase); */

	if (((uint8_t *) pTask->pVABase <= pInst) && (pInst <= (uint8_t *) pTask->pCMDEnd)) {
		if (pInst != (uint8_t *) pTask->pCMDEnd) {
			/* If PC points to start of pCMD, */
			/* - 8 causes access violation */
			/* insts[0] = CMDQ_REG_GET32(pInst - 8); */
			/* insts[1] = CMDQ_REG_GET32(pInst - 4); */
			insts[2] = CMDQ_REG_GET32(pInst + 0);
			insts[3] = CMDQ_REG_GET32(pInst + 4);
		} else {
			/* insts[0] = CMDQ_REG_GET32(pInst - 16); */
			/* insts[1] = CMDQ_REG_GET32(pInst - 12); */
			insts[2] = CMDQ_REG_GET32(pInst - 8);
			insts[3] = CMDQ_REG_GET32(pInst - 4);
		}
	} else {
		/* invalid PC address */
		return NULL;
	}

	return (uint32_t *) pInst;
}

uint32_t cmdq_tz_subsys_to_reg_addr(uint32_t argA)
{
	const uint32_t subsysBit = cmdq_tz_get_subsys_LSB_in_argA();
	const int32_t subsys_id = (argA & 0x001F0000) >> subsysBit;
	const uint32_t offset = (argA & 0xFFFF);
	uint32_t base_addr = 0;

#undef DECLARE_CMDQ_SUBSYS
#define DECLARE_CMDQ_SUBSYS(addr, id, grp, base) case id: base_addr = addr; break;
	switch (subsys_id) {
#include "cmdq_sec_subsys.h"
	}
#undef DECLARE_CMDQ_SUBSYS

	return (base_addr << subsysBit) | offset;
}

static const char *cmdq_tz_parse_op(uint32_t opCode)
{
	switch (opCode) {
	case CMDQ_CODE_POLL:
		return "POLL";
	case CMDQ_CODE_WRITE:
		return "WRIT";
	case CMDQ_CODE_WFE:
		return "SYNC";
	case CMDQ_CODE_READ:
		return "READ";
	case CMDQ_CODE_MOVE:
		return "MASK";
	case CMDQ_CODE_JUMP:
		return "JUMP";
	case CMDQ_CODE_EOC:
		return "MARK";
	}
	return NULL;
}

static uint32_t *cmdq_tz_dump_pc(const TaskStruct *pTask, int thread, const char *tag)
{
	uint32_t *pcVA = NULL;
	uint32_t insts[4] = { 0 };
	char parsedInstruction[128] = { 0 };

	pcVA = cmdq_tz_get_pc(pTask, thread, insts);
	if (pcVA) {
		const uint32_t op = (insts[3] & 0xFF000000) >> 24;

		cmdq_tz_parse_instruction(pcVA, parsedInstruction, sizeof(parsedInstruction));
		CMDQ_ERR("[%s]Thread %d PC(VA): 0x%p, 0x%08x:0x%08x => %s\n",
			 tag, thread, pcVA, insts[2], insts[3], parsedInstruction);

		/* for WFE, we specifically dump the event value */
		if (op == CMDQ_CODE_WFE) {
			uint32_t regValue = 0;
			const uint32_t eventID = 0x3FF & insts[3];

			CMDQ_REG_SET32(CMDQ_SYNC_TOKEN_ID, eventID);
			regValue = CMDQ_REG_GET32(CMDQ_SYNC_TOKEN_VAL);
			CMDQ_ERR("[%s]CMDQ_SYNC_TOKEN_VAL of %s is %d\n", tag,
				 cmdq_tz_get_event_name(eventID), regValue);
		}
	} else {
		CMDQ_ERR("[%s]Thread %d PC(VA): Not available\n", tag, thread);
	}

	return pcVA;
}

void cmdq_tz_dump_status(const char *tag)
{
	int32_t coreExecThread = CMDQ_INVALID_THREAD;
	uint32_t value[6] = { 0 };

	value[0] = CMDQ_REG_GET32(CMDQ_CURR_LOADED_THR);
	value[1] = CMDQ_REG_GET32(CMDQ_THR_EXEC_CYCLES);
	value[2] = CMDQ_REG_GET32(CMDQ_THR_TIMEOUT_TIMER);
	value[3] = CMDQ_REG_GET32(CMDQ_BUS_CONTROL_TYPE);

	coreExecThread = value[0]; //__builtin_ffs(value[0]) - 1;	/* this returns (1 + index of least bit set) or 0 if input is 0. */
	CMDQ_ERR("[%s]IRQ flag:0x%08x,0x%08x, Execing:%d, Exec Thread:0x%x, CMDQ_CURR_LOADED_THR: 0x%08x\n",
		 tag,
		 CMDQ_REG_GET32(CMDQ_CURR_IRQ_STATUS),
		 CMDQ_REG_GET32(CMDQ_SECURE_IRQ_STATUS),
		 (0x80000000 & value[0]) ? 1 : 0, coreExecThread, value[0]);
	CMDQ_ERR("[%s]CMDQ_THR_EXEC_CYCLES:0x%08x, CMDQ_THR_TIMER:0x%08x, CMDQ_BUS_CTRL:0x%08x\n",
		 tag, value[1], value[2], value[3]);
	CMDQ_ERR("[%s]CMDQ_DEBUG_1: 0x%08x\n", tag, CMDQ_REG_GET32((GCE_BASE_VA + 0xF0)));
	CMDQ_ERR("[%s]CMDQ_DEBUG_2: 0x%08x\n", tag, CMDQ_REG_GET32((GCE_BASE_VA + 0xF4)));
	CMDQ_ERR("[%s]CMDQ_DEBUG_3: 0x%08x\n", tag, CMDQ_REG_GET32((GCE_BASE_VA + 0xF8)));
	CMDQ_ERR("[%s]CMDQ_DEBUG_4: 0x%08x\n", tag, CMDQ_REG_GET32((GCE_BASE_VA + 0xFC)));
	return;
}

void cmdq_tz_dump_task_usage(void)
{
	int32_t index;
	TaskStruct *pTask;

	for (index = 0; index < CMDQ_MAX_FIXED_TASK; index++) {
		CMDQ_ERR("====== Task %d Usage =======\n", index);

		pTask = &(gCmdqContext.taskInfo[index]);
		cmdq_tz_dump_task_usage_impl(pTask, false);
	}
}

void cmdq_tz_dump_sec_metadata(iwcCmdqCommand_t *pCommandDec, tlApiCmdqExecMetadata_ptr pMetadata, TaskStruct *pTask)
{
	uint32_t i = 0;
	uint32_t indexArgB = 0;
	uint32_t indexIntrs = 0;

	CMDQ_ERR("=============== [CMDQ] Sec Metadata ===============\n");

	CMDQ_ERR("addrList[%d][0x%08x]\n",
		pCommandDec->metadata.addrListLength, pCommandDec->metadata.addrList);

	for(i = 0; i < (pCommandDec->metadata.addrListLength); i++) {
		indexIntrs = pCommandDec->metadata.addrList[i].instrIndex;
		indexArgB  = 2 * indexIntrs;

		pTask->pVABase[indexArgB] = pMetadata->pSecFdAddr[i];
		CMDQ_ERR("translate_table_%02d, indexIntrs[%03d], argA[0x%08x], indexArgB[%d], secPA[0x%08x]\n",
				i, indexIntrs, pTask->pVABase[indexArgB+1], indexArgB, (pMetadata->pSecFdAddr[i]));
	}
}

int32_t cmdq_tz_replace_secure_handle_in_task(TaskStruct *pTask,
			iwcCmdqCommand_t *pCommandDesc,
			tlApiCmdqExecMetadata_ptr pMetadata,
			const uint32_t prependInstr)
{
	int32_t i;
	iwcCmdqAddrMetadata_t *pAddr = NULL;
	const int32_t count = pCommandDesc->metadata.addrListLength;
	uint32_t *pCMD = pTask->pVABase;

	int32_t instrIdx = 0;
	int32_t indexB = 0;
	uint32_t oldValue =0;


	CMDQ_VERBOSE("TASK: replace sec handle, count(%d:%d)\n",
		pCommandDesc->metadata.addrListLength, pMetadata->pSecFdCount);
	if (pCommandDesc->metadata.addrListLength != pMetadata->pSecFdCount) {
		CMDQ_ERR("TASK: replace sec handle failed since invalid param, count(%d:%d)\n",
			pCommandDesc->metadata.addrListLength, pMetadata->pSecFdCount);
		return -(CMDQ_ERR_FAULT);
	}

	for (i = 0; i < count; i++) {
		instrIdx = prependInstr + (pCommandDesc->metadata.addrList[i].instrIndex);
		indexB = 2 * instrIdx;

		oldValue = pTask->pVABase[indexB];
		pCMD[indexB] = pMetadata->pSecFdAddr[i];

		CMDQ_MSG("TASK: translate_%2d, instrIdx:%03d(%d,%d), instr:(0x%08x,0x%08x), oldArgB: 0x%08x\n",
			i, instrIdx, prependInstr, (pCommandDesc->metadata.addrList[i].instrIndex),
			pCMD[indexB+1], pCMD[indexB], oldValue);
	}

	return 0;
}

#define CMDQ_DEBUG_COMPOSE_CMD (0)
int32_t cmdq_tz_insert_security_reg_command(TaskStruct *pTask, iwcCmdqCommand_t *pCommandDesc, tlApiCmdqExecMetadata_ptr pMetadata)
{
	const uint32_t copiedFromEOF = (2 * CMDQ_INST_SIZE);
	const uint32_t copiedBeforeEOF = pTask->commandSize - copiedFromEOF;

	uint32_t prependBufferSize = 0;
	int32_t offset = 0;
	int32_t status = 0;

#if CMDQ_DEBUG_COMPOSE_CMD
	CMDQ_VERBOSE("[1]pCommandDesc->pVABase:%p, size:%d, pCMDEnd:%p\n",
		pCommandDesc->pVABase, pTask->MVABase, pTask->commandSize);

	cmdq_tz_dump_buffer(pCommandDesc->pVABase, pCommandDesc->commandSize, "acquire_task_0");
#endif

	/* 0. init pCMDEnd */
	pTask->pCMDEnd = pTask->pVABase - 1;

	/* 1. copy original content except EOF+JUMP */
	memcpy(pTask->pCMDEnd + 1, pCommandDesc->pVABase, copiedBeforeEOF);
	pTask->pCMDEnd += (copiedBeforeEOF / sizeof(pTask->pCMDEnd[0]));

	/* 2. copy EOF+JUMP */
	memcpy(pTask->pCMDEnd + 1,
		(pCommandDesc->pVABase + (copiedBeforeEOF / sizeof(pTask->pCMDEnd[0]))), /*pVABase is not void*, ensure correct start size*/
		copiedFromEOF);
	pTask->pCMDEnd += (copiedFromEOF / sizeof(pTask->pCMDEnd[0]));

	CMDQ_VERBOSE("pCommandDesc->pVABase:%p, pTask->MVABase:0x%x, size:%d, pCMDEnd:%p\n",
		pCommandDesc->pVABase, pTask->MVABase, pTask->commandSize, pTask->pCMDEnd);

	/* 5. replace with memory */
	if(0 > cmdq_tz_replace_secure_handle_in_task(pTask, pCommandDesc, pMetadata, (prependBufferSize / CMDQ_INST_SIZE))) {
		return -(CMDQ_ERR_FAULT);
	}

#if CMDQ_DEBUG_COMPOSE_CMD
	CMDQ_VERBOSE("[5]pCommandDesc->pVABase:%p, pTask->MVABase:0x%x, size:%d, pCMDEnd:%p, commandSize:%d\n",
		pCommandDesc->pVABase, pTask->MVABase, pTask->commandSize, pTask->pCMDEnd);

	cmdq_tz_dump_buffer(pTask->pVABase, pTask->commandSize, "acquire_task_1");
#endif

	return status;
}

TaskStruct* cmdq_tz_acquire_task_with_metadata(iwcCmdqCommand_t *pCommandDesc, tlApiCmdqExecMetadata_ptr pMetadata)
{
	TaskStruct *pTask   = NULL;
	drApiResult_t drRes = DRAPI_OK;

	CMDQ_MSG("-->TASK: acquire, Flag:0x%x%08x, secVABase: 0x%p, commandSize: %d\n",
		(uint32_t)((pCommandDesc->engineFlag) >> 32), (uint32_t)(pCommandDesc->engineFlag),
		pCommandDesc->pVABase, pCommandDesc->commandSize);

	CMDQ_TZ_PROF_START("CMDQ_TZ_TASK_ACQ");

	pTask = list_peek_head_type(&gCmdqFreeTask, TaskStruct, listEntry);
	do {
		if (NULL == pTask) {
			CMDQ_ERR("TASK: no free task, gCmdqFreeTask[0x%08x], prev[0x%08x], next[0x%08x]\n",
				gCmdqFreeTask, gCmdqFreeTask.prev, gCmdqFreeTask.next);
			cmdq_tz_dump_task_usage();
			break;
		}

		if (0 >= pTask->bufferSize) {
			CMDQ_ERR("TASK: invalid task %p command buffer, size:%d\n", pTask, pTask->bufferSize);
			pTask = NULL;
			break;
		}

		/* init basic information */
		pTask->taskState  = TASK_STATE_WAITING;
		pTask->scenario   = pCommandDesc->scenario;
		pTask->priority   = pCommandDesc->priority;
		pTask->engineFlag = pCommandDesc->engineFlag;
		pTask->thread	 = pCommandDesc->thread; /* note we dispatch thread form normal path */
		pTask->irqFlag = 0;

		pTask->commandSize = pCommandDesc->commandSize; /* recorded command size */
		pTask->waitCookie = pCommandDesc->waitCookie;
		pTask->resetExecCnt = pCommandDesc->resetExecCnt;
		pTask->hNormalTask = pCommandDesc->hNormalTask;
		pTask->enginesNeedDAPC = pCommandDesc->metadata.enginesNeedDAPC;
		pTask->enginesNeedPortSecurity = pCommandDesc->metadata.enginesNeedPortSecurity;

		/* command buffer related */
		if (pCommandDesc->pVABase != NULL){
			/* Already map VA in outside */
			pTask->pVABase = pCommandDesc->pVABase;
			pTask->MVABase = pCommandDesc->MVABase;
			pTask->pCMDEnd = pTask->pVABase + (pTask->commandSize / sizeof(pTask->pCMDEnd[0])) - 1;
			/* Also no need to insert security register */
		} else {
			/* compose secure path command */
			memset(pTask->pVABase, 0, CMDQ_TZ_CMD_BLOCK_SIZE);
			if ( 0 > cmdq_tz_insert_security_reg_command(pTask, pCommandDesc, pMetadata)) {
				pTask = NULL;
				break;
			}
		}

		/* remove task form free list*/
		list_delete(&(pTask->listEntry));
	} while(0);

	CMDQ_TZ_PROF_END("CMDQ_TZ_TASK_ACQ");

	CMDQ_MSG("<--TASK: acquire, pTask[0x%p]\n", pTask);
	return pTask;

}

int32_t cmdq_tz_suspend_HW_thread(int32_t thread)
{
	int32_t loop = 0;

	CMDQ_MSG("EXEC: suspend HW thread[%d]\n", thread);

	CMDQ_REG_SET32(CMDQ_THR_SUSPEND_TASK(thread), 0x01);

	loop = 0;
	while(0x0 == (CMDQ_REG_GET32(CMDQ_THR_CURR_STATUS(thread)) & 0x2))
	{
		if(loop > CMDQ_MAX_LOOP_COUNT)
		{
			CMDQ_ERR("suspend HW thread %d failed\n", thread);
			return -CMDQ_ERR_FAULT;
		}
		loop++;
	}
	return 0;
}

void cmdq_tz_resume_HW_thread(int32_t thread)
{
	CMDQ_MSG("EXEC: resume HW thread[%d]\n", thread);
	CMDQ_REG_SET32(CMDQ_THR_SUSPEND_TASK(thread), 0x00);
}

int32_t cmdq_tz_reset_HW_thread(int32_t thread)
{
	int32_t loop = 0;

	CMDQ_MSG("EXEC: reset HW thread %d\n", thread);

	CMDQ_MSG("EXEC: test warm reset address 0x%08x\n", CMDQ_THR_WARM_RESET(thread));
	CMDQ_REG_SET32(CMDQ_THR_WARM_RESET(thread), 0x01);
	CMDQ_MSG("EXEC: test warm reset address value: 0x%x\n", *(CMDQ_THR_WARM_RESET(thread)));

	while(0x1 == (CMDQ_REG_GET32(CMDQ_THR_WARM_RESET(thread))))
	{
		CMDQ_MSG("EXEC: loop test warm reset address value: 0x%x\n", *(CMDQ_THR_WARM_RESET(thread)));
		if(loop > CMDQ_MAX_LOOP_COUNT)
		{
			CMDQ_ERR("reset HW thread %d failed\n", thread);
			return -CMDQ_ERR_FAULT;
		}
		loop++;
	}

	/* reset THR_EXEC_COUNT in shared DRAM */
	//CMDQ_VERBOSE("EXEC: reset HW thread %d+ , CNT:%d\n", thread, CMDQ_REG_GET32(CMDQ_THR_SHARED_EXEC_CNT(thread)));
	//CMDQ_REG_SET32(CMDQ_THR_SHARED_EXEC_CNT(thread), 0);
	//CMDQ_VERBOSE("EXEC: reset HW thread %d-, CNT:%d\n", thread, CMDQ_REG_GET32(CMDQ_THR_SHARED_EXEC_CNT(thread)));

	return 0;
}

int32_t cmdq_tz_parse_instruction(const uint32_t *pCmd, char *textBuf, int bufLen)
{
	int reqLen = 0;
	const uint32_t op = (pCmd[1] & 0xFF000000) >> 24;
	const uint32_t argA = pCmd[1] & (~0xFF000000);
	const uint32_t argB = pCmd[0];

	return reqLen;
}

int32_t cmdq_tz_disable_HW_thread(int32_t thread)
{
	cmdq_tz_reset_HW_thread(thread);

	CMDQ_MSG("EXEC: disable HW thread[%d]\n", thread);
	CMDQ_REG_SET32(CMDQ_THR_ENABLE_TASK(thread), 0x00);
	return 0;
}

int32_t cmdq_tz_enable_HW_thread(int32_t thread)
{
	CMDQ_MSG("EXEC: enable thread[%d]\n", thread);
	CMDQ_REG_SET32(CMDQ_THR_ENABLE_TASK(thread), 0x01);
	return 0;
}

void cmdq_tz_dump_disp_trigger_loop(const char *tag)
{
	uint32_t regValue = 0;
	uint32_t trigerCurr, trigerEnd;

	trigerCurr = CMDQ_REG_GET32(CMDQ_THR_CURR_ADDR(CMDQ_MAX_HIGH_PRIORITY_THREAD_COUNT));
	trigerEnd = CMDQ_REG_GET32(CMDQ_THR_END_ADDR(CMDQ_MAX_HIGH_PRIORITY_THREAD_COUNT));
	CMDQ_ERR("Trigger Thread PC: 0x%08x, End: 0x%08x\n", trigerCurr, trigerEnd);

	CMDQ_REG_SET32(CMDQ_SYNC_TOKEN_ID, CMDQ_EVENT_DISP_RDMA0_EOF);
	regValue = CMDQ_REG_GET32(CMDQ_SYNC_TOKEN_VAL);
	CMDQ_ERR("[%s]CMDQ_SYNC_TOKEN_VAL of CMDQ_EVENT_DISP_RDMA0_EOF is %d\n",
		 tag, regValue);
}

static void cmdq_tz_attach_error_task(const TaskStruct *pTask, int32_t thread)
{
	EngineStruct *pEngine = NULL;
	ThreadStruct *pThread = NULL;
	uint64_t engFlag = 0;
	int32_t index = 0;
	uint32_t *hwPC = NULL;
	uint32_t value[10] = { 0 };

	static const char *engineGroupName[] = {
		CMDQ_FOREACH_GROUP(GENERATE_STRING)
	};

	pThread = cmdq_tz_get_thread_struct_by_id(thread, __LINE__);
	pEngine = gCmdqContext.engine;

	/*  */
	/* Then we just print out info */
	/*  */
	CMDQ_ERR("================= [CMDQ] Begin of Error %d================\n",
		 gCmdqContext.errNum);

	CMDQ_ERR("=============== [CMDQ] Error Thread Status ===============\n");

	value[0] = CMDQ_REG_GET32(CMDQ_THR_CURR_ADDR(thread));
	value[1] = CMDQ_REG_GET32(CMDQ_THR_END_ADDR(thread));
	value[2] = CMDQ_REG_GET32(CMDQ_THR_WAIT_TOKEN(thread));
	value[3] = CMDQ_GET_COOKIE_CNT(thread);
	value[4] = CMDQ_REG_GET32(CMDQ_THR_IRQ_STATUS(thread));
	value[5] = CMDQ_REG_GET32(CMDQ_THR_INST_CYCLES(thread));
	value[6] = CMDQ_REG_GET32(CMDQ_THR_CURR_STATUS(thread));
	value[7] = CMDQ_REG_GET32(CMDQ_THR_IRQ_ENABLE(thread));
	value[8] = CMDQ_REG_GET32(CMDQ_THR_ENABLE_TASK(thread));
	value[9] = CMDQ_REG_GET32(CMDQ_THR_SECURITY(thread));

	CMDQ_ERR("Index: %d, Enabled: %d, IRQ: 0x%08x, Thread PC: 0x%08x, End: 0x%08x, Wait Token: 0x%08x\n",
		 thread, value[8], value[4], value[0], value[1], value[2]);
	CMDQ_ERR("Curr Cookie: %d, Wait Cookie: %d, Next Cookie: %d, Task Count %d,\n",
		value[3], pThread->waitCookie, pThread->nextCookie, pThread->taskCount);
	CMDQ_ERR("Timeout Cycle:%d, Status:0x%08x, IRQ_EN: 0x%08x, Secure: %d\n",
		value[5], value[6], value[7], value[9]);

	CMDQ_ERR("=============== [CMDQ] CMDQ Status ===============\n");
	cmdq_tz_dump_status("ERR");

	if (NULL != pTask) {
		CMDQ_ERR("=============== [CMDQ] Error Thread PC ===============\n");
		hwPC = cmdq_tz_dump_pc(pTask, thread, "ERR");

		CMDQ_ERR("=============== [CMDQ] Error Task Status ===============\n");
		cmdq_tz_dump_task(pTask);
	}

	CMDQ_ERR("=============== [CMDQ] All Task in Error Thread Info ===============\n");
	do {
		struct TaskStruct *pDumpTask = NULL;
		for (index = 0; index < CMDQ_MAX_TASK_IN_THREAD; index++) {
			pDumpTask = pThread->pCurTask[index];
			if (NULL != pDumpTask) {
				cmdq_tz_dump_task(pDumpTask);
			}
		}
	} while (0);

	CMDQ_ERR("=============== [CMDQ] DISP Trigger Loop Info ===============\n");
	cmdq_tz_dump_disp_trigger_loop("ERR");

	CMDQ_ERR("=============== [CMDQ] Error Command Buffer ===============\n");
	cmdq_tz_print_hex_dump("[CMDQ][tz]", pTask->pVABase, (pTask->commandSize));

	CMDQ_ERR("================= [CMDQ] End of Error %d ================\n",
		 gCmdqContext.errNum);

	gCmdqContext.errNum++;
}

int32_t cmdq_tz_insert_task_from_thread_array_by_cookie(
								TaskStruct *pTask,
								ThreadStruct *pThread,
								const int32_t cookie,
								const bool resetHWThread)
{

	if (NULL == pTask || NULL == pThread) {
		CMDQ_ERR("invalid param, pTask[0x%p], pThread[0x%p], cookie[%d], needReset[%d]\n",
			pTask, pThread, cookie, resetHWThread);
		return -(CMDQ_ERR_FAULT);
	}

	CMDQ_VERBOSE("EXEC: insert to thread\n");

	if (true == resetHWThread) {
		pThread->waitCookie = cookie;
		pThread->nextCookie = cookie + 1;
		if (pThread->nextCookie > CMDQ_MAX_COOKIE_VALUE) {
			/* Reach the maximum cookie */
			pThread->nextCookie = 0;
		}

		/* taskCount must start from 0. */
		/* and we are the first task, so set to 1. */
		pThread->taskCount = 1;

	} else {
		pThread->nextCookie += 1;
		if (pThread->nextCookie > CMDQ_MAX_COOKIE_VALUE) {
			/* Reach the maximum cookie */
			pThread->nextCookie = 0;
		}

		pThread->taskCount++;
	}

	pThread->pCurTask[cookie % CMDQ_MAX_TASK_IN_THREAD] = pTask;

	CMDQ_VERBOSE("EXEC: insert to thread done\n");
	return 0;
}


void cmdq_tz_remove_task_from_thread_array_by_cookie(
		ThreadStruct *pThread, int32_t index, TASK_STATE_ENUM targetTaskState)
{
	if((NULL == pThread) || (CMDQ_MAX_TASK_IN_THREAD <= index) || (0 > index))
	{
		CMDQ_ERR("remove task, invalid param. pThread[0x%p], task_slot[%d], targetTaskState[%d]\n",
			pThread, index, targetTaskState);
		return;
	}

	// note timing to switch a task to done_status(_ERROR, _KILLED, _DONE) is aligned with thread's taskcount change
	// check task status to prevent double clean-up thread's taskcount
	if(TASK_STATE_BUSY != pThread->pCurTask[index]->taskState)
	{
		CMDQ_ERR("remove task, taskStatus err[%d]. pThread[0x%p], task_slot[%d], targetTaskState[%d]\n",
			pThread->pCurTask[index]->taskState, pThread, index, targetTaskState);
		return;
	}

	CMDQ_MSG("remove task, slot[%d], targetTaskState: %d\n", index, targetTaskState);
	pThread->pCurTask[index]->taskState = targetTaskState;

	pThread->pCurTask[index] = NULL;
	pThread->taskCount--;
}

void cmdq_tz_remove_all_task_in_thread_unlocked(int32_t thread)
{
	ThreadStruct *pThread = NULL;
	TaskStruct   *pTask   = NULL;
	uint32_t index   = 0;

	do {
		pThread = cmdq_tz_get_thread_struct_by_id(thread, __LINE__);
		for (index = 0; index < CMDQ_MAX_TASK_IN_THREAD; ++index) {

			pTask = pThread->pCurTask[index];
			if (NULL == pTask){
				continue;
			}

			CMDQ_ERR("RELEASE_ALL_TASK: release task 0x%p\n", pTask);
			pTask->irqFlag = 0xDEADDEAD; // unknown state;

			#if 0
			cmdq_tz_remove_task_from_thread_array_by_cookie(pThread, index, TASK_STATE_ERROR);
			#else
			cmdq_tz_remove_task_from_thread_array_and_release_task_by_cookie(
				pTask, thread, index, TASK_STATE_ERROR);
			#endif
		}
		pThread->taskCount = 0;
		pThread->waitCookie = pThread->nextCookie;
	} while(0);
}

int32_t cmdq_tz_insert_task_to_thread_multiple_tasks_impl(TaskStruct *pTask, int32_t thread)
{
	int32_t status;
	int32_t threadPC;
	ThreadStruct *pThread;
	TaskStruct *pPrevTask;
	const int32_t cookie = pTask->waitCookie;
	int32_t prev;

	status = 0;
	pThread = cmdq_tz_get_thread_struct_by_id(thread, __LINE__);

	/* find previous task */
	prev = (cookie % CMDQ_MAX_TASK_IN_THREAD) -1;
	if (0 > prev) {
		prev = CMDQ_MAX_TASK_IN_THREAD - 1; /* counter wrapped */
	}
	pPrevTask = pThread->pCurTask[prev];

	CMDQ_MSG("EXEC: reuse HW thread(%d), cookie:%d\n", thread, cookie);

	/* suspend HW thread first, so that we work in a consistent state */
	status = cmdq_tz_suspend_HW_thread(thread);
	if (0 > status) {

		return status;
	}

	threadPC = CMDQ_REG_GET32(CMDQ_THR_CURR_ADDR(thread));

	/* append task to the end of task list. */
	/*
	 * there are 3 possible condition:
	 * 1. Not execute pPrevTask's JUMP instruction:
	 *		.modify pPrevTask's JUMP to pTask directly
	 * 2. thread PC locates pPrevTask's JUMP
	 * 3. pPrevTask execute done
	 *		.revise PC to begin fo pTask
	 *
	 * simplify above condiftion to check END_ADDR and PC
	 */

	/* Boundary case tested: EOC have been executed, but JUMP is not executed */
	/* Thread PC: 0x9edc0dd8, End: 0x9edc0de0, Curr Cookie: 1, Next Cookie: 2 */
	if ( (threadPC == (CMDQ_REG_GET32(CMDQ_THR_END_ADDR(thread)) - 8)) ||	/* PC = END - 8, EOC is executed, PC locates JUMP */
		 (threadPC == (CMDQ_REG_GET32(CMDQ_THR_END_ADDR(thread)) - 0))) {	/* PC = END - 0, All CMDs are executed */

		CMDQ_MSG("EXEC: Set HW thread(%d) pc:%pa\n", thread, &pTask->MVABase);
		CMDQ_REG_SET32(CMDQ_THR_CURR_ADDR(thread), (pTask->MVABase));
	}
	else
	{
		/* go here, there is a previous task */
		if (NULL == pPrevTask) {
			CMDQ_ERR("EXEC: noPrevTask, cookie:%d, thread:%d, prev:%d\n", cookie, thread, prev);
			return -CMDQ_ERR_FAULT;
		}

		/* not execute pPrevTask's JUMP yet, so revise pPrevTask's JUMP to concatenate pTask */
		pPrevTask->pCMDEnd[0] = 0x10000001; /* Jump: Absolute */
		pPrevTask->pCMDEnd[-1] = pTask->MVABase;	/* Jump to here */
	}

	/* extent thread's end addr */
	CMDQ_REG_SET32(CMDQ_THR_END_ADDR(thread), (pTask->MVABase + pTask->commandSize));

	/* motify pThread for add new task */
	cmdq_tz_insert_task_from_thread_array_by_cookie(pTask, pThread, cookie, false);

	/* enable thread again */
	CMDQ_TZ_PROF_ONESHOT("CMDQ_TZ_EN_GCE");

	cmdq_tz_resume_HW_thread(thread);

	return status;
}

int32_t cmdq_tz_exec_task_async(TaskStruct *pTask, int32_t thread)
{
	int32_t	  status;
	ThreadStruct *pThread;
	int32_t	  waitQ;
	const int32_t cookie = pTask->waitCookie;
	uint32_t threadPrio;

	CMDQ_LOG("-->EXEC: task 0x%p on thread %d begin, VABase: 0x%p, MVABase: %x, Size: %d, bufferSize: %d, scenario:%d, flag:0x%x%08x\n",
		pTask, thread, pTask->pVABase, (pTask->MVABase), pTask->commandSize, pTask->bufferSize, pTask->scenario,
		(uint32_t)((pTask->engineFlag) >> 32), (uint32_t)(pTask->engineFlag));
	CMDQ_LOG("cookie:%d, reset:%d, hNormalTask:0x%x%08x\n",
		cookie, pTask->resetExecCnt,
		(uint32_t)((pTask->hNormalTask) >> 32), (uint32_t)(pTask->hNormalTask));

	pThread = cmdq_tz_get_thread_struct_by_id(thread, __LINE__);
	status = 0;

	/* attach task to thread, so switch state to BUSY */
	pTask->taskState = TASK_STATE_BUSY;

	/* inseart task according to NWd's dispatch result */

	if (pTask->resetExecCnt) {

		if (1 != pTask->waitCookie) {
			CMDQ_ERR("try to reset THR %d exec CNT when %dth task\n", thread, pTask->waitCookie);
		}

		if (0 > cmdq_tz_reset_HW_thread(thread)) {
			return -CMDQ_ERR_FAULT;
		}

		CMDQ_MSG("EXEC: new HW thread(%d)\n", thread);

		/* NWd CMDQ will monitor task execute result, so we enable EXEC_CMD only */
#if (1 == CMDQ_DEBUG_POLL_TASK_DONE)
		CMDQ_MSG("use poll status, not register IRQ\n");
#else
		CMDQ_REG_SET32(CMDQ_THR_IRQ_ENABLE(thread), CMDQ_THR_IRQ_FALG_EXEC_CMD | CMDQ_THR_IRQ_FALG_INVALID_INSTN);
#endif
		/* HW thread config */
		CMDQ_REG_SET32(CMDQ_THR_INST_CYCLES(thread), CMDQ_MAX_INST_CYCLE);
		CMDQ_REG_SET32(CMDQ_THR_SECURITY(thread), 0x1 | (0x1 << 3)); /* set THRx_SECURITY=1, THRx_SECURE_IRQ_EN=1 */
		CMDQ_REG_SET32(CMDQ_THR_CURR_ADDR(thread), pTask->MVABase);
		CMDQ_REG_SET32(CMDQ_THR_END_ADDR(thread), pTask->MVABase + pTask->commandSize);

		/* thread priority */
		threadPrio = cmdq_tz_priority_from_scenario(pTask->scenario);
		CMDQ_REG_SET32(CMDQ_THR_CFG(thread), threadPrio & 0x7);	/* bit 0-2 for priority level; */
		CMDQ_MSG("EXEC: set HW thread(%d) pc:%pa, qos:%d\n",
				thread, &pTask->MVABase, threadPrio);

		/* attach pTask to the thread */
		cmdq_tz_insert_task_from_thread_array_by_cookie(pTask, pThread, cookie, true);

		/* enable HW */
		CMDQ_TZ_PROF_ONESHOT("CMDQ_TZ_EN_GCE");
		cmdq_tz_enable_HW_thread(thread);
	} else {

		status = cmdq_tz_insert_task_to_thread_multiple_tasks_impl(pTask, thread);
	}

	CMDQ_LOG("<--EXEC: pTask[0x%08x], thr[%d], status[%d]\n", pTask, thread, status);
	return status;
}

void cmdq_tz_release_task(TaskStruct *pTask)
{
	if(NULL == pTask)
	{
		CMDQ_ERR("release NULL task\n", pTask);
		return;
	}

	CMDQ_TZ_PROF_START("CMDQ_TZ_TASK_REL");

	do {
		pTask->taskState = TASK_STATE_IDLE;
		pTask->thread	 = CMDQ_INVALID_THREAD;

		// be carefully that list parameter order is not same as linux version
		// secure world's list: list_add_tail(list, list_node)
		list_add_tail(&gCmdqFreeTask, &(pTask->listEntry));
	} while(0);

	CMDQ_TZ_PROF_END("CMDQ_TZ_TASK_REL");
	CMDQ_MSG("TASK release done, pTask[0x%p], state[%d]\n", pTask, pTask->taskState);
}

static const char *gCmdqThreadLabel[CMDQ_MAX_THREAD_COUNT] = {
	"CMDQ_IRQ_THR_0",
	"CMDQ_IRQ_THR_1",
	"CMDQ_IRQ_THR_2",
	"CMDQ_IRQ_THR_3",
	"CMDQ_IRQ_THR_4",
	"CMDQ_IRQ_THR_5",
	"CMDQ_IRQ_THR_6",
	"CMDQ_IRQ_THR_7",
	"CMDQ_IRQ_THR_8",
	"CMDQ_IRQ_THR_9",
	"CMDQ_IRQ_THR_10",
	"CMDQ_IRQ_THR_11",
	"CMDQ_IRQ_THR_12",
	"CMDQ_IRQ_THR_13",
	"CMDQ_IRQ_THR_14",
	"CMDQ_IRQ_THR_15",
};

void cmdqTzHandleIRQ(int32_t thread)
{
	unsigned long flags = 0;
	bool	  isInvalidInstruction = false;
	bool	  isAlreadySuspended = false;
	const uint32_t value = CMDQ_REG_GET32(CMDQ_THR_IRQ_STATUS(thread));
	const uint32_t validIrqFlag = (CMDQ_THR_IRQ_FALG_INVALID_INSTN | CMDQ_THR_IRQ_FALG_INSTN_TIMEOUT | CMDQ_THR_IRQ_FALG_EXEC_CMD);

	CMDQ_VERBOSE("IRQ: thread %d got interrupt, flag:0x%08x\n", thread, value);

	CMDQ_TZ_PROF_ONESHOT(gCmdqThreadLabel[thread]);

	// we must suspend thread before query cookie
	isAlreadySuspended =
		((CMDQ_REG_GET32(CMDQ_THR_SUSPEND_TASK(thread)) & 0x1) > 0) ? true : false;
	isInvalidInstruction = (0 < (value & CMDQ_THR_IRQ_FALG_INVALID_INSTN)) ? true : false;
	if (false == isAlreadySuspended || isInvalidInstruction)
	{
		CMDQ_VERBOSE("IRQ: thread %d status: %x, start to suspend\n", thread, CMDQ_REG_GET32(CMDQ_THR_CURR_STATUS(thread)));
		cmdq_tz_suspend_HW_thread(thread);
	}

	do {
		if (value & CMDQ_THR_IRQ_FALG_EXEC_CMD) {
			cmdqTzHandleDone(thread, value);
		} else {
			cmdqTzHandleError(thread, value);
		}

		// IRQ value may mix success and error result, e.g. value = 0x3 = 1 means HW timeout + 1 success
		// so clear up IRQ flag with irq value, not 0x12, to prevent duplicate IRQ handle
		CMDQ_REG_SET32(CMDQ_THR_IRQ_STATUS(thread), ~value);
	}while(0);


	//
	// HW thread continues to execute from last instrction after driver resumes HW thread
	// in invalid instruciton case,
	// driver MUST correct PC from invalid instruciton and clean irq status before resume HW thread
	// otherwise, we will trap into inifinite invalid instruciton IRQ loop
	//
	// so do not resume if it is invalide instruction.
	// let user space handle this situation.
	//
	if (false == isAlreadySuspended && false == isInvalidInstruction){
		cmdq_tz_resume_HW_thread(thread);
	}
}

int32_t cmdq_tz_get_and_update_CNT_to_shared_CNT_region(int32_t thread)
{
	const int32_t cookie = CMDQ_GET_COOKIE_CNT(thread);
	//CMDQ_REG_SET32(CMDQ_THR_SHARED_EXEC_CNT(thread), cookie);

	CMDQ_LOG("SHARED_CNT: update thread %d shared_cookie %d\n, ", thread, cookie);

	return cookie;
}

void cmdq_tz_remove_task_from_thread_array_and_release_task_by_cookie(
		TaskStruct *pTask, int32_t thread, const uint32_t cookie, TASK_STATE_ENUM targetTaskState)
{
	ThreadStruct *pThread = cmdq_tz_get_thread_struct_by_id(thread, __LINE__);

	CMDQ_MSG("HACK: update task %p status %d and release it, thread: %d, cookie:%d\n",
		pTask, targetTaskState, thread, cookie);

	/* update to task done status, and remove it from thread array */
	cmdq_tz_remove_task_from_thread_array_by_cookie(pThread, cookie, targetTaskState);

	/* HACK: we wait task done in NWds, so pTask life cycle is ended after driver gets execution result in*/
	/* release pTask, and add to free list */
	cmdq_tz_release_task(pTask);
}

void cmdqTzHandleError(int32_t thread, int32_t value)
{
	ThreadStruct  *pThread;
	TaskStruct	*pTask;
	int32_t	   cookie;
	int32_t	   count;
	int32_t	   inner;
	uint32_t	  *hwPC;

	CMDQ_VERBOSE("cmdqTzHandleError, thr[%d], irqValue[%02x]\n", thread, value);

	// get thread
	pThread = cmdq_tz_get_thread_struct_by_id(thread, __LINE__);
	pTask = NULL;
	hwPC  = NULL;

	// we assume error happens BEFORE EOC
	// because it wouldn't be error if this interrupt is issue by EOC.
	// So we should inc by 1 to locate "current" task
	cookie = cmdq_tz_get_and_update_CNT_to_shared_CNT_region(thread);
	cookie = cookie + 1;
	cookie = (CMDQ_MAX_COOKIE_VALUE < cookie) ? (cookie - CMDQ_MAX_COOKIE_VALUE - 1) : (cookie);

	CMDQ_ERR("cmdqTzHandleError, thr[%d], irqValue[%02x], cookie[CNT+1=%d], waitCookie[%d]\n",
		thread, value, cookie, pThread->waitCookie);

	// Set the issued task to error state
	pTask = pThread->pCurTask[cookie % CMDQ_MAX_TASK_IN_THREAD];
	if (NULL != pTask)
	{
		CMDQ_VERBOSE("IRQ: change to ERROR_STATE, cookie[%d], index[%d]\n", cookie, (cookie % CMDQ_MAX_TASK_IN_THREAD));

		//hwPC = cmdq_tz_dump_pc(pTask, thread, "ERR");
		/* Can not dump error instr here */
		/* because we have to ensure atomic map/unmap command buffer by IPC thread. */
		CMDQ_ERR("thread %d pc:0x%08x\n", thread, CMDQ_REG_GET32(CMDQ_THR_CURR_ADDR(thread)));

		// error task process
		pTask->irqFlag = value;

		/* mark pTask as ERROR but not remove from thread */
		/* excpet remove it from thread's task array */
		/* when normal CMDQ detect SW timeout and entry SWd to cancle task*/
		#if 1
		pTask->taskState = TASK_STATE_ERROR;
		#else
		cmdq_tz_remove_task_from_thread_array_and_release_task_by_cookie(
			pTask,
			thread,
			(cookie % CMDQ_MAX_TASK_IN_THREAD),
			TASK_STATE_ERROR);
		#endif

		/*  Print error task information */
		cmdq_tz_attach_error_task(pTask, thread);
	}

	// Set the remain tasks to done state
	// note the current task ("error" one) has been processed, so +1
	if (pThread->waitCookie <= cookie) {
		count = cookie - pThread->waitCookie + 1;
	} else {
		// Counter wrapped
		count = (CMDQ_MAX_COOKIE_VALUE - pThread->waitCookie + 1) + (cookie + 1);
	}

	for (inner = (pThread->waitCookie % CMDQ_MAX_TASK_IN_THREAD); count > 0; count--, inner++) {
		if (inner >= CMDQ_MAX_TASK_IN_THREAD){
			inner = 0;
		}

		if (NULL != pThread->pCurTask[inner])
		{
			CMDQ_VERBOSE("IRQ: change to DONE_STATE, cookie[%d], index[%d]\n", cookie, inner);

			pTask = pThread->pCurTask[inner];
			pTask->irqFlag = value;
			cmdq_tz_remove_task_from_thread_array_and_release_task_by_cookie(
				pTask, thread, inner, TASK_STATE_DONE);
		}
	}

	pThread->waitCookie = cookie + 1;
	if (pThread->waitCookie > CMDQ_MAX_COOKIE_VALUE)
	{
		pThread->waitCookie -= (CMDQ_MAX_COOKIE_VALUE + 1); /* min cookie value is 0*/
	}

	if (pThread->taskCount <= 0) {
		cmdqRecResetTaskCookie();
	}
}

void cmdqTzHandleDone(int32_t thread, int32_t value)
{
	ThreadStruct *pThread;
	TaskStruct   *pTask;
	int32_t	   cookie;
	int32_t	   count;
	int32_t	   inner;

	// get thread
	pThread = cmdq_tz_get_thread_struct_by_id(thread, __LINE__);
	pTask   = NULL;

	cookie = cmdq_tz_get_and_update_CNT_to_shared_CNT_region(thread);

	CMDQ_MSG("cmdqTzHandleDone, thr[%d], irqValue[%02x], cookie[%d], waitCookie[%d]\n", thread, value, cookie, pThread->waitCookie);

	if (pThread->waitCookie <= cookie) {
		count = cookie - pThread->waitCookie + 1;
	}
	else{
		// Counter wrapped
		count = (CMDQ_MAX_COOKIE_VALUE - pThread->waitCookie + 1) + (cookie + 1);
	}


	for (inner = (pThread->waitCookie % CMDQ_MAX_TASK_IN_THREAD); count > 0; count--, inner++)
	{
		if (inner >= CMDQ_MAX_TASK_IN_THREAD)
		{
			inner = 0;
		}

		if (NULL != pThread->pCurTask[inner])
		{
			pTask = pThread->pCurTask[inner];
			pTask->irqFlag = value;
		#if 0
			cmdq_tz_remove_task_from_thread_array_by_cookie(pThread, inner, TASK_STATE_DONE);
		#else
			cmdq_tz_remove_task_from_thread_array_and_release_task_by_cookie(
				pTask, thread, inner, TASK_STATE_DONE);
		#endif
		}
	}
	pThread->waitCookie = cookie + 1;
	if (pThread->waitCookie > CMDQ_MAX_COOKIE_VALUE)
	{
		pThread->waitCookie -= (CMDQ_MAX_COOKIE_VALUE + 1); /* min cookie value is 0*/
	}

	if (pThread->taskCount <= 0) {
		cmdqRecResetTaskCookie();
	}
}

bool cmdq_tz_poll_timeout_impl(uint32_t *pollCount)
{
#ifdef CMDQ_POLL_TIMEOUT
	uint32_t poll_count = (*pollCount)++;

	if (poll_count >= CMDQ_POLL_COUNT_MAX)
		return true;
	else
		return false;
#else
	return false;
#endif
}

int32_t cmdq_tz_poll_task_done(TaskStruct *pTask, int32_t thread)
{
	const uint32_t cookie = pTask->waitCookie;
	uint32_t pollCount = 0;
	uint32_t value;

	do {
		value = CMDQ_REG_GET32(CMDQ_THR_EXEC_CNT(thread));
#ifdef CMDQ_POLL_TIMEOUT
		CMDQ_MSG("[HACK]POLL: Current Cookie:%d, pTask->waitCookie:%d\n", value, cookie);
#endif
		if ((value >= cookie) || cmdq_tz_poll_timeout_impl(&pollCount) ) {
			/* task execute done */
#ifdef CMDQ_POLL_TIMEOUT
			if (pollCount < CMDQ_POLL_COUNT_MAX) {
				CMDQ_LOG("[HACK]POLL: task %p exec done, thread %d, Cookie:%d, Curr Cookie:%d\n",
					pTask, thread, cookie, value);
			} else {
				CMDQ_ERR("[HACK]POLL: task %p timeout, thread %d, Cookie:%d, Curr Cookie:%d\n",
					pTask, thread, cookie, value);
			}
#else
			CMDQ_LOG("[HACK]POLL: task %p exec done, thread %d, Cookie:%d, Curr Cookie:%d\n",
					pTask, thread, cookie, value);
#endif
			cmdqTzHandleIRQ(thread);

			/* notify normal CMDQ driver */
			//cmdq_tz_poke_notify_loop();
			break;
		}
	}while(1);

	return 0;
}

int32_t cmdqTzComposeAndSubmitTaskAsync(iwcCmdqMessage_ptr pIwcMessage,
										tlApiCmdqExecMetadata_ptr pMetadata,
										TaskStruct **ppTaskOut)
{
	/* pIwcMessage is from normal world, */
	/* pMetadata describe secure releated metadata */
	TaskStruct *pTask = NULL;
	int32_t retry	= 0;
	int32_t status	= 0;
	int32_t thread;

	CMDQ_MSG("-->cmdqTzComposeAndSubmitTaskAsync\n");

	if (0 == cmdq_tz_is_path_resource_ready()) {
		CMDQ_ERR("path resource doesn't setup yet\n");
		status = -CMDQ_ERR_FAULT;
		return status;
	}

	/* acquire a trask, and compose data from normal world(pIwcMessage) and secure meta data */
	pTask = cmdq_tz_acquire_task_with_metadata(&(pIwcMessage->command), pMetadata);
	if (NULL == pTask) {
		status = -CMDQ_ERR_FAULT;
		return status;
	}

	do {
		/* bypass acquire thread */
		/* (note we have dispatched thread id and do clock control in normal world) */
		thread = pIwcMessage->command.thread;
		CMDQ_MSG("THREAD: test secure thread %d\n", thread);

		if(false == cmdq_tz_is_a_secure_thread(thread)) {
			CMDQ_ERR("THREAD: invalid secure thread %d\n", thread);
			status = -CMDQ_ERR_FAULT;
			break;
		}

		CMDQ_MSG("[HACK]: Current Cookie:%d\n", CMDQ_REG_GET32(CMDQ_THR_EXEC_CNT(thread)));
		/* submit to GCE */
		status = cmdq_tz_exec_task_async(pTask, thread);
		if(0 > status){
			cmdq_tz_dump_sec_metadata(pIwcMessage, pMetadata, pTask);
			break;
		}

	}while(0);

	if (NULL != ppTaskOut) {
		*ppTaskOut = pTask;
	}

	CMDQ_MSG("<--cmdqTzComposeAndSubmitTaskAsync, status[%d]\n", status);
	return status;
}

int32_t cmdqTzComposeAndSubmitTask(iwcCmdqMessage_ptr pIwcMessage, tlApiCmdqExecMetadata_ptr pMetadata)
{
	TaskStruct *pTask = NULL;
	int32_t status	= 0;
	CMDQ_MSG("-->cmdqTzComposeAndSubmitTask\n");

	status = cmdqTzComposeAndSubmitTaskAsync(pIwcMessage, pMetadata, &pTask);
	if(0 > status){
		CMDQ_ERR("cmdqTzComposeAndSubmitTaskAsync failed with thread\n");
		return status;
	}

	status = cmdqTzWaitTask(pTask, pIwcMessage->command.thread);
	CMDQ_MSG("<--cmdqTzComposeAndSubmitTask, status[%d]\n", status);
	return status;
}

int32_t cmdqTzSubmitTask(TaskStruct *pTask, int32_t thread)
{
	/* pIwcMessage is from normal world, */
	/* pMetadata describe secure releated metadata */
	int32_t retry	 = 0;
	int32_t status	= 0;

	CMDQ_MSG("-->SubmitTask\n");

	if (NULL == pTask) {
		status = -CMDQ_ERR_FAULT;
		return status;
	}

	do {
		/* bypass acquire thread */
		/* TODO: (note we have dispatched thread id and do clock control in normal world) */

		if(false == cmdq_tz_is_a_secure_thread(thread)) {
			CMDQ_ERR("THREAD: invalid secure thread %d\n", thread);
			status = -CMDQ_ERR_FAULT;
			break;
		}

		/* submit to GCE */
		status = cmdq_tz_exec_task_async(pTask, thread);
		if(0 > status){
			CMDQ_ERR("THREAD: execute task failed with thread: %d\n", thread);
			break;
		}

		status = cmdqTzWaitTask(pTask, thread);
	}while(0);

	CMDQ_MSG("<--SubmitTask, status[%d]\n", status);
	return status;
}

int32_t cmdqTzWaitTask(TaskStruct *pTask, int32_t thread)
{
	int32_t status	= 0;

	CMDQ_MSG("-->WaitTask\n");

	if (NULL == pTask) {
		status = -CMDQ_ERR_FAULT;
		return status;
	}

	do {
#if (1 == CMDQ_DEBUG_POLL_TASK_DONE)
		status = cmdq_tz_poll_task_done(pTask, thread);
#else
		/* wakeup ISR woker thread to prepare receive IRQ... */
		status = cmdq_tz_resume_worker_thread(DRIVER_THREAD_NO_IRQH, "ISR", "IPC");
		if (0 > status) {
			break;
		}
		status = cmdq_tz_suspend_worker_thread(DRIVER_THREAD_NO_DCIH, "CUR", "CUR");
#endif

		/* note noraml CMDQ will wait execution result */
		/* bypass release thread */
		/* it will be handled in NWd. */
	}while(0);

	CMDQ_MSG("<--WaitTask, status[%d]\n", status);
	return status;
}

void cmdqTzDeInitialize(void)
{
	cmdq_tz_deinit_module_base_VA();
}

int32_t cmdqTzSetupPathResouce(const uint32_t sharedCookiePA, const uint32_t sharedCookieSize, tlApiCmdqPathResMetadata_t *pResource)
{
	int32_t status;
	TaskStruct *pTask;
	uint32_t MVABase;
	int32_t i;

	do {
		status = 0;

		/* setup shared cookie CNT */
		if(0 == sharedCookiePA) {
			CMDQ_ERR("PATH_RES: setup with invalid param,sharedCookiePA:0x%08x\n", sharedCookiePA);
			status = -CMDQ_ERR_FAULT;
			break;
		}

		status = cmdq_tz_init_shared_exec_cnt_region(sharedCookiePA, sharedCookieSize);
		if(0 > status){
			CMDQ_ERR("PATH_RES: setup with shared_exec_cnt failed, status:%d\n", status);
			break;
		}

		/* setup task command buffer */
		if ((0 == pResource->secPA) ||
			(CMDQ_TZ_CMD_BLOCK_SIZE * CMDQ_MAX_FIXED_TASK != pResource->size)) {
			CMDQ_ERR("PATH_RES: setup with invalid param, command PA:0x%08x, size: %d\n", pResource->secPA, pResource->size);
			status = -CMDQ_ERR_FAULT;
			break;
		}

		MVABase = pResource->secPA;
		do {
			i = 0;
			list_for_every_entry(&gCmdqFreeTask, pTask, TaskStruct, listEntry){

				pTask->bufferSize = CMDQ_TZ_CMD_BLOCK_SIZE;
				pTask->pVABase = NULL; /* delay VA life cycle when map */
				pTask->MVABase = MVABase;
				//CMDQ_MSG("i: %d, pTask: %p, MVABase:0x%08x, VA:0x%08x, size:%d\n", i, pTask, pTask->MVABase, pTask->pVABase, pTask->bufferSize);
				MVABase += CMDQ_TZ_CMD_BLOCK_SIZE;
				i++;
			}
		} while(0);

		/* mark path resource init done */
		gCmdqContext.initPathResDone = true;
	} while(0);

	return 0;
};

int32_t cmdqTzSetupTaskResouce()
{
	TaskStruct *pTask;
	int32_t i;

	i = 0;
	list_for_every_entry(&gCmdqFreeTask, pTask, TaskStruct, listEntry){

		pTask->bufferSize = CMDQ_TZ_CMD_BLOCK_SIZE;
		pTask->pVABase = NULL; /* delay VA life cycle when map */
		i++;
	}
	/* mark path resource init done */
	gCmdqContext.initPathResDone = true;

	return 0;
};

int32_t cmdqTzTeardownResouce(void)
{
	return 0;
};

int32_t cmdqTzCancelTask(iwcCmdqCancelTask_t *pCancelTask)
{
	int32_t thread;
	uint32_t cookie;
	uint32_t currCookie;
	ThreadStruct *pThread = NULL;
	TaskStruct  *pTask = NULL;
	uint32_t pc;

	bool hasReset = false;
	bool throwAEE = false;
	uint32_t *pcVA = NULL;
	uint32_t insts[4] = { 0 };

	if (NULL == pCancelTask) {
		CMDQ_ERR("CANCEL_TASK: NULL param\n");
		return -(CMDQ_ERR_FAULT);
	}

	thread = pCancelTask->thread;
	if (false == cmdq_tz_is_a_secure_thread(thread)){
		/* only allow cancel task on a secure thread */
		CMDQ_ERR("CANCEL_TASK: invalid thread param (%d)\n", thread);
		return -(CMDQ_ERR_FAULT);
	}

	/* suspend HW therad */
	cmdq_tz_suspend_HW_thread(thread);
	const int32_t irqStatus = CMDQ_REG_GET32(CMDQ_SECURE_IRQ_STATUS);
	const int32_t irqFlag = CMDQ_REG_GET32(CMDQ_THR_IRQ_STATUS(thread));

	do {
		thread = pCancelTask->thread;
		pThread = cmdq_tz_get_thread_struct_by_id(thread, __LINE__);

		cookie = pCancelTask->waitCookie;
		pTask = pThread->pCurTask[cookie];
		currCookie = cmdq_tz_get_and_update_CNT_to_shared_CNT_region(thread);

		if (NULL == pTask) {
			CMDQ_ERR("CANCEL_TASK: thread %d pCurTask[%d] is NULL task\n", thread, cookie);
			break;
		}

		CMDQ_ERR("CANCEL_TASK: start to remove all tasks on thread %d since error task(cookie:%d), currCookie:%d\n",
			thread, cookie, currCookie);

		throwAEE = true;
		/* get PC and err intr */
		pc = CMDQ_REG_GET32(CMDQ_THR_CURR_ADDR(thread));
		pcVA = cmdq_tz_get_pc(pTask, thread, insts);
		/* dump error */
		cmdq_tz_attach_error_task(pTask, thread);

		/* remove all task in thread */
		cmdq_tz_remove_all_task_in_thread_unlocked(thread);

	}while(0);

	/* resume HW thread */
	if (0 >= pThread->taskCount) {
		/* clear thread's cookie value in shared CNT region */
		CMDQ_ERR("CANCEL_TASK: clear thread %d shared_cookie to %d\n, ", thread, 0);
		CMDQ_REG_SET32(CMDQ_THR_SHARED_EXEC_CNT(thread), 0);

		cmdq_tz_disable_HW_thread(thread);
		hasReset = true;
	} else {
		CMDQ_ERR("CANCEL_TASK: resume HW thread %d\n");
		cmdq_tz_resume_HW_thread(thread);
	}

	/* fill reply message */
	pCancelTask->hasReset = hasReset;
	pCancelTask->throwAEE = throwAEE;
	pCancelTask->errInstr[0] = (pcVA) ? (insts[2]) : (0); /* argA */
	pCancelTask->errInstr[1] = (pcVA) ? (insts[3]) : (0); /* argB */
	pCancelTask->irqFlag = irqFlag;
	pCancelTask->irqStatus = irqStatus;
	pCancelTask->pc = pc;

	return 0;
}

const bool cmdq_tz_is_path_resource_ready(void) {
	return (gCmdqContext.initPathResDone);
}

void cmdqTzErrorDump(void)
{
	bool hasDumped = false;
	uint32_t i = 0;
	uint32_t thread = 12;
	ThreadStruct *pThread = cmdq_tz_get_thread_struct_by_id(thread, __LINE__);
	TaskStruct  *pTask = NULL;

	CMDQ_ERR("================= cmdqTzErrorDump(err:%d) ================\n", gCmdqContext.errNum);
	for (i = 0; i < CMDQ_MAX_TASK_IN_THREAD ;i++) {
		if (pThread && pThread->pCurTask[i]) {
			pTask = pThread->pCurTask[i];
			cmdq_tz_attach_error_task(pTask, thread);
			hasDumped = true;
		}
	}

	if (!hasDumped) {
		cmdq_tz_attach_error_task(NULL, thread);
	}
}

int32_t cmdq_tz_init_shared_exec_cnt_region(const uint32_t startPA, const uint32_t size)
{
	int32_t status = 0;

	gCmdqContext.sharedThrExecCntPA = startPA;
	gCmdqContext.sharedThrExecCntSize = size;

	do {
		status = cmdq_tz_map_physical_addr(&gCmdqSharedCookieVA, startPA, size);
		if (status < 0) {
			break;
		}

		memset(gCmdqSharedCookieVA, 0, gCmdqContext.sharedThrExecCntSize);
	} while(0);

	return status;
}

int32_t cmdq_tz_suspend_worker_thread(
			const threadno_t threadNumber,
			const char* suspendThreadName,
			const char* callerThreadName)
{
	int32_t status = 0;
	drApiResult_t drRes = DRAPI_OK;

	CMDQ_LOG("[SYNC_SUBMIT]%s: suspend thread %s\n",
		callerThreadName, suspendThreadName);

	drRes = drApiIpcSigWait();

	if (DRAPI_OK != drRes) {
		CMDQ_ERR("[SYNC_SUBMIT]%s: drApiIpcSigWait failed, ret(%x, %x:%x)\n",
			callerThreadName,
			drRes, DRAPI_ERROR_MAJOR(drRes), DRAPI_ERROR_DETAIL(drRes));
		status = -CMDQ_ERR_FAULT;
	}
	return status;
}

int32_t cmdq_tz_resume_worker_thread(
			const threadno_t threadNumber,
			const char* resumeThreadName,
			const char* callerThreadName)
{
	int32_t status = 0;
	const threadid_t threadId = drApiGetLocalThreadid(threadNumber);
	drApiResult_t drRes = drApiIpcSignal(threadId);

	if (DRAPI_OK == drRes) {
		CMDQ_LOG("[SYNC_SUBMIT]%s: signal to wake up %s\n",
			callerThreadName, resumeThreadName);
	} else {
		CMDQ_ERR("[SYNC_SUBMIT]%s: failed to wake up %s, ret(%x, %x:%x)\n",
			callerThreadName, resumeThreadName,
			drRes, DRAPI_ERROR_MAJOR(drRes), DRAPI_ERROR_DETAIL(drRes));
		status = -CMDQ_ERR_FAULT;
	}
	return status;
}

void cmdq_tz_set_log_level(int32_t level)
{
	gCmdqContext.logLevel = level;
}

const bool cmdq_tz_should_print_msg(void)
{
	return (gCmdqContext.logLevel > 0);
}

void cmdq_tz_set_profile_enabled(const int32_t enable)
{
	gCmdqContext.enableProfile = enable;
}

const int32_t cmdq_tz_profile_enabled()
{
	return (gCmdqContext.enableProfile);
}

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
#ifndef __CMDQ_SEC_RECORD_H__
#define __CMDQ_SEC_RECORD_H__

#include "cmdq_sec_def.h"
#include "cmdq_sec_core.h"

#define CMD_BUFFER_SIZE			(0x20000)	/* 128KB := 4 CMD SIZE (32KB) */
#define CMD_MAX_FIXED_HANDLE		(0x4)

#define CMDQ_MAX_PREFETCH_INSTUCTION    (240)	/* Maximum prefetch buffer size, in instructions. */

typedef unsigned long long cmdqU32Ptr_t;      // Custom "wide" pointer type for 64-bit compatibility. Always cast from uint32_t*.

struct TaskStruct;

typedef struct cmdqRecStruct {
	struct list_node	listEntry;
	uint64_t engineFlag;
	int32_t scenario;
	uint32_t blockSize;	/* command size */
	uint32_t *pBuffer;	/* VA base */
	uint32_t MVABase;	/* PA base */
	uint32_t bufferSize;	/* allocated buffer size */
	CMDQ_HW_THREAD_PRIORITY_ENUM priority;	/* setting high priority. This implies Prefetch ENABLE. */
	bool finalized;		/* set to true after flush() or startLoop() */
	uint32_t prefetchCount;	/* maintainence prefetch instruction */
	uint64_t enginesNeedDAPC;
	uint64_t enginesNeedPortSecurity;
} cmdqRecStruct, *cmdqRecHandle;

 typedef void *CmdqRecLoopHandle;

#ifdef __cplusplus
extern "C" {
#endif

void cmdqRecResetTaskCookie(void);

/**
 * Create command queue recorder handle
 * Parameter:
 *     pHandle: pointer to retrieve the handle
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecCreate(CMDQ_SCENARIO_ENUM scenario, cmdqRecHandle *pHandle);

/**
 * Reset command queue recorder commands
 * Parameter:
 *    handle: the command queue recorder handle
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecReset(cmdqRecHandle handle);

/**
 * Configure as secure task
 * Parameter:
 *     handle: the command queue recorder handle
 *     isSecure: true, execute the command in secure world
 * Return:
 *     0 for success; else the error code is returned
 *
 * Note:
 *     a. Secure CMDQ support when t-base enabled only
 *     b. By default cmdqRecHandle records a normal command,
 *		  please call cmdqRecSetSecure to set command as SECURE after cmdqRecReset
 */
int32_t cmdqRecSetSecure(cmdqRecHandle handle, const bool isSecure);

/**
 * Add DPAC protection flag
 * Parameter:
 * Note:
 *     a. Secure CMDQ support when t-base enabled only
 *     b. after reset handle, user have to specify protection flag again
 */
int32_t cmdqRecSecureEnableDAPC(cmdqRecHandle handle, const uint64_t engineFlag);

/**
 * Add flag for M4U security ports
 * Parameter:
 * Note:
 *	   a. Secure CMDQ support when t-base enabled only
 *	   b. after reset handle, user have to specify protection flag again
 */
int32_t cmdqRecSecureEnablePortSecurity(cmdqRecHandle handle, const uint64_t engineFlag);

/**
 * Append mark command to the recorder
 * Parameter:
 *     handle: the command queue recorder handle
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecMark(cmdqRecHandle handle);

/**
 * Append mark command to enable prefetch
 * Parameter:
 *     handle: the command queue recorder handle
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecEnablePrefetch(cmdqRecHandle handle);

/**
 * Append mark command to disable prefetch
 * Parameter:
 *     handle: the command queue recorder handle
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecDisablePrefetch(cmdqRecHandle handle);

/**
 * Append write command to the recorder
 * Parameter:
 *     handle: the command queue recorder handle
 *     addr: the specified target register physical address
 *     value: the specified target register value
 *     mask: the specified target register mask
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecWrite(cmdqRecHandle handle, uint32_t addr, uint32_t value, uint32_t mask);

 /**
 * Append poll command to the recorder
 * Parameter:
 *     handle: the command queue recorder handle
 *     addr: the specified register physical address
 *     value: the required register value
 *     mask: the required register mask
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecPoll(cmdqRecHandle handle, uint32_t addr, uint32_t value, uint32_t mask);

/**
 * Append wait command to the recorder
 * Parameter:
 *     handle: the command queue recorder handle
 *     event: the desired event type to "wait and CLEAR"
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecWait(cmdqRecHandle handle, CMDQ_EVENT_ENUM event);

/**
 * like cmdqRecWait, but won't clear the event after
 * leaving wait state.
 *
 * Parameter:
 *     handle: the command queue recorder handle
 *     event: the desired event type wait for
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecWaitNoClear(cmdqRecHandle handle, CMDQ_EVENT_ENUM event);

/**
 * Unconditionally set to given event to 0.
 * Parameter:
 *     handle: the command queue recorder handle
 *     event: the desired event type to set
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecClearEventToken(cmdqRecHandle handle, CMDQ_EVENT_ENUM event);

/**
 * Unconditionally set to given event to 1.
 * Parameter:
 *     handle: the command queue recorder handle
 *     event: the desired event type to set
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecSetEventToken(cmdqRecHandle handle, CMDQ_EVENT_ENUM event);

/**
 * Read a register value to a CMDQ general purpose register(GPR)
 * Parameter:
 *     handle: the command queue recorder handle
 *     hwRegAddr: register address to read from
 *     dstDataReg: CMDQ GPR to write to
 * Return:
 *     0 for success; else the error code is returned
 */
int32_t cmdqRecReadToDataRegister(cmdqRecHandle handle, uint32_t hwRegAddr,
					  CMDQ_DATA_REGISTER_ENUM dstDataReg);

/**
 * Write a register value from a CMDQ general purpose register(GPR)
 * Parameter:
 *     handle: the command queue recorder handle
 *     srcDataReg: CMDQ GPR to read from
 *     hwRegAddr: register address to write to
 * Return:
 *     0 for success; else the error code is returned
 */
 int32_t cmdqRecWriteFromDataRegister(cmdqRecHandle handle,
				  CMDQ_DATA_REGISTER_ENUM srcDataReg, uint32_t hwRegAddr);

/**
 * Trigger CMDQ to execute the recorded commands
 * Parameter:
 *     handle: the command queue recorder handle
 * Return:
 *     0 for success; else the error code is returned
 * Note:
 *     This is a synchronous function. When the function
 *     returned, the recorded commands have been done.
 */
int32_t cmdqRecFlush(cmdqRecHandle handle);


/**
 *  Flush the command; Also at the end of the command, backup registers
 *  appointed by addrArray.
 *
 */
int32_t cmdqRecFlushAndReadRegister(cmdqRecHandle handle, uint32_t regCount,
					    uint32_t *addrArray, uint32_t *valueArray);

/**
 * Trigger CMDQ to asynchronously execute the recorded commands
 * Parameter:
 *     handle: the command queue recorder handle
 * Return:
 *     0 for successfully start execution; else the error code is returned
 * Note:
 *     This is an ASYNC function. When the function
 *     returned, it may or may not be finished. There is no way to retrieve the result.
 */
int32_t cmdqRecFlushAsync(cmdqRecHandle handle);

int32_t cmdqRecFlushAsyncCallback(cmdqRecHandle handle, CmdqAsyncFlushCB callback,
					  uint32_t userData);

/**
 * returns current count of instructions in given handle
 */
int32_t cmdqRecGetInstructionCount(cmdqRecHandle handle);

/**
 * Dump command buffer to kernel log
 * This is for debugging purpose.
 */
int32_t cmdqRecDumpCommand(cmdqRecHandle handle);

/**
 * Destroy command queue recorder handle
 * Parameter:
 *     handle: the command queue recorder handle
 */
void cmdqRecDestroy(cmdqRecHandle handle);

int32_t cmdqRecInitialize(void);
#ifdef __cplusplus
}
#endif
#endif				/* __CMDQ_SEC_RECORD_H__ */

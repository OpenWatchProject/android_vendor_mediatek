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
#ifndef __CMDQ_SEC_CORE_H__
#define __CMDQ_SEC_CORE_H__

#include "cmdq_sec_util.h"
#include "cmdq_sec_iwc_common.h"
#include "cmdq_sec_reg.h"
#include "cmdq_sec_def.h"
#include "DrApi/DrApiThread.h"

/* #define CMDQ_POLL_TIMEOUT (1) */

#ifdef CMDQ_POLL_TIMEOUT
#define CMDQ_POLL_COUNT_MAX (300)
#define CMDQ_MSG_LEVEL (1)
#else
#define CMDQ_MSG_LEVEL (0)
#endif

//-------------------------------------------------------------------------------

#include "DrApi/DrApiLogging.h"

#define CMDQ_LOG(string, args...) \
	if (1) \
	{ \
		drDbgPrintf("[CMDQ][tz]"string, ##args); \
	}

#define CMDQ_VERBOSE(string, args...) \
	if (CMDQ_MSG_LEVEL) \
	{ \
		drDbgPrintf("[CMDQ][tz]"string, ##args); \
	}

#define CMDQ_MSG(string, args...) \
	if (CMDQ_MSG_LEVEL) \
	{ \
		drDbgPrintf("[CMDQ][tz]"string, ##args); \
	}

#define CMDQ_ERR(string, args...) \
	if (1) \
	{ \
		drDbgPrintf("[CMDQ][tz][ERR]"string, ##args); \
	}
#define CMDQ_AEE(tag, string, args...) \
	if (1) \
	{ \
		drDbgPrintf("[CMDQ][tz][AEE]"string, ##args); \
	}

#define CMDQ_PHYS_TO_AREG(addr) (addr)	/* truncate directly */
#define CMDQ_AREG_TO_PHYS(addr) (addr)

#define CMDQ_ENG_ISP_GROUP_BITS                 ((1LL << CMDQ_ENG_ISP_IMGI) |       \
						 (1LL << CMDQ_ENG_ISP_IMGO) |       \
						 (1LL << CMDQ_ENG_ISP_IMG2O))

#define CMDQ_ENG_MDP_GROUP_BITS                 ((1LL << CMDQ_ENG_MDP_CAMIN) |      \
						 (1LL << CMDQ_ENG_MDP_RDMA0) |      \
						 (1LL << CMDQ_ENG_MDP_RSZ0) |       \
						 (1LL << CMDQ_ENG_MDP_RSZ1) |       \
						 (1LL << CMDQ_ENG_MDP_TDSHP0) |     \
						 (1LL << CMDQ_ENG_MDP_WROT0) |      \
						 (1LL << CMDQ_ENG_MDP_WDMA))

/* DISP path doesn't care engineFlag, */
/* Instead of CMDQ_ENG_DISP_GROUP_BITS,
 * use cmdq_tz_is_disp_scenario to ientify DISP path */
#define CMDQ_ENG_DISP_GROUP_BITS                ((1LL << CMDQ_ENG_DISP_UFOE) |      \
						 (1LL << CMDQ_ENG_DISP_AAL) |       \
						 (1LL << CMDQ_ENG_DISP_COLOR0) |    \
						 (1LL << CMDQ_ENG_DISP_RDMA0) |     \
						 (1LL << CMDQ_ENG_DISP_RDMA1) |     \
						 (1LL << CMDQ_ENG_DISP_WDMA0) |     \
						 (1LL << CMDQ_ENG_DISP_WDMA1) |     \
						 (1LL << CMDQ_ENG_DISP_OVL0) |      \
						 (1LL << CMDQ_ENG_DISP_OVL1) |      \
						 (1LL << CMDQ_ENG_DISP_GAMMA) |     \
						 (1LL << CMDQ_ENG_DISP_DSI0_VDO) |  \
						 (1LL << CMDQ_ENG_DISP_DSI0_CMD) |  \
						 (1LL << CMDQ_ENG_DISP_DSI0) |      \
						 (1LL << CMDQ_ENG_DISP_DPI))

#define CMDQ_ENG_VENC_GROUP_BITS                ((1LL << CMDQ_ENG_VIDEO_ENC))

#define CMDQ_ENG_JPEG_GROUP_BITS                ((1LL << CMDQ_ENG_JPEG_ENC) | \
						 (1LL << CMDQ_ENG_JPEG_REMDC) | \
						 (1LL << CMDQ_ENG_JPEG_DEC))

#define CMDQ_ENG_ISP_GROUP_FLAG(flag)   ((flag) & (CMDQ_ENG_ISP_GROUP_BITS))

#define CMDQ_ENG_MDP_GROUP_FLAG(flag)   ((flag) & (CMDQ_ENG_MDP_GROUP_BITS))

#define CMDQ_ENG_DISP_GROUP_FLAG(flag)  ((flag) & (CMDQ_ENG_DISP_GROUP_BITS))

#define CMDQ_ENG_JPEG_GROUP_FLAG(flag)  ((flag) & (CMDQ_ENG_JPEG_GROUP_BITS))

#define CMDQ_ENG_VENC_GROUP_FLAG(flag)  ((flag) & (CMDQ_ENG_VENC_GROUP_BITS))

#define GENERATE_ENUM(_enum, _string) _enum,
#define GENERATE_STRING(_enum, _string) #_string,

#define CMDQ_FOREACH_GROUP(ACTION)\
    ACTION(CMDQ_GROUP_ISP, ISP) \
    ACTION(CMDQ_GROUP_MDP, MDP) \
    ACTION(CMDQ_GROUP_DISP, DISP) \
    ACTION(CMDQ_GROUP_JPEG, JPEG) \
    ACTION(CMDQ_GROUP_VENC, VENC)

typedef enum CMDQ_GROUP_ENUM {
	CMDQ_FOREACH_GROUP(GENERATE_ENUM)
	    CMDQ_MAX_GROUP_COUNT,	/* ALWAYS keep at the end */
} CMDQ_GROUP_ENUM;

//--------------------------------------------------------------------------

/* IRQ trigger condition */
typedef enum CMDQ_THR_IRQ_FLAG_ENUM
{
	CMDQ_THR_IRQ_FALG_EXEC_CMD = 0x01, /* trigger IRQ if CMD executed done */
	CMDQ_THR_IRQ_FALG_INSTN_TIMEOUT  = 0x02, /* trigger IRQ if instuction timeout */
	CMDQ_THR_IRQ_FALG_INVALID_INSTN  = 0x10
} CMDQ_THR_IRQ_FLAG_ENUM;

/* data are user data passed to APIs */
typedef int32_t(*CmdqAsyncFlushCB) (unsigned long data);

typedef enum CMDQ_CODE_ENUM {
	/* these are actual HW op code */
	CMDQ_CODE_READ = 0x01,
	CMDQ_CODE_MOVE = 0x02,
	CMDQ_CODE_WRITE = 0x04,
	CMDQ_CODE_POLL = 0x08,
	CMDQ_CODE_JUMP = 0x10,
	CMDQ_CODE_WFE = 0x20,	/* wait for event and clear */
	CMDQ_CODE_EOC = 0x40,	/* end of command */

	/* these are pseudo op code defined by SW */
	/* for instruction generation */
	CMDQ_CODE_SET_TOKEN = 0x21,	/* set event */
	CMDQ_CODE_WAIT_NO_CLEAR = 0x22,	/* wait event, but don't clear it */
	CMDQ_CODE_CLEAR_TOKEN = 0x23,	/* clear event */
	CMDQ_CODE_RAW = 0x24,	/* allow entirely custom argA/argB */
} CMDQ_CODE_ENUM;


typedef enum TASK_STATE_ENUM {
	TASK_STATE_IDLE	   = 0,	/* free task */
	TASK_STATE_BUSY	   = 1,	/* task running on a thread */
	TASK_STATE_KILLED  = 2,	/* task process being killed */
	TASK_STATE_ERROR   = 3,	/* task execution error */
	TASK_STATE_DONE	   = 4,	/* task finished */
	TASK_STATE_WAITING = 5,	/* allocated but waiting for available thread */
} TASK_STATE_ENUM;

typedef struct TaskStruct {

	struct list_node	listEntry;

	/* For buffer state */
	/* note secure driver has to map secure PA to its virtual address before access secure DRAM */
	/* so life cycle of pVABase and MVABase are same */
	TASK_STATE_ENUM	 taskState;
	uint32_t *pVABase;		/* VA: denote CMD addr in cmdqSecDr's virtual address space */
	uint32_t MVABase;		/* PA: denote the PA for secure CMD */
	uint32_t secVABase; 	/* planned CMD start address in cmdqSecDrv's virtual address space */
	uint32_t bufferSize;	/* size of allocated command buffer */

	/* For execution */
	int32_t scenario;
	int32_t priority;
	uint64_t engineFlag;
	int32_t commandSize;
	uint32_t *pCMDEnd;
	int32_t thread;		/* pre-dispatch in NWd */
	int32_t irqFlag;	/* flag of IRQ received */

	int32_t waitCookie; /* task index in thread's tasklist. dispatched by NWd*/
	bool resetExecCnt;
	uint64_t enginesNeedDAPC;
	uint64_t enginesNeedPortSecurity;
	threadno_t waitThreadNumber;	/* Use to wake up suspend secure thread */

	CmdqAsyncFlushCB flushCallback;	/* Callback on AsyncFlush (fire-and-forget) tasks */
	unsigned long flushData;	/* for callbacks & error handling */

	/* Debug */
	uint64_t hNormalTask;

	/* For statistics & debug */
	int32_t callerPid;
	char callerName[CMDQ_IWC_CLIENT_NAME];
} TaskStruct;


typedef struct EngineStruct {
	int32_t userCount;
	int32_t currOwner;
	int32_t resetCount;
	int32_t failCount;
} EngineStruct;

typedef struct ThreadStruct {
	uint32_t taskCount;
	uint32_t waitCookie;
	uint32_t nextCookie;
	TaskStruct *pCurTask[CMDQ_MAX_TASK_IN_THREAD];
} ThreadStruct;

typedef struct ContextStruct
{
	/* Basic information */
	TaskStruct taskInfo[CMDQ_MAX_FIXED_TASK];
	EngineStruct engine[CMDQ_MAX_ENGINE_COUNT];
	ThreadStruct thread[CMDQ_MAX_THREAD_COUNT]; // note we only handle secure thread

	/* Share region with NWd */
	uint32_t sharedThrExecCntPA;	/* PA start address of THR cookie */
	uint32_t sharedThrExecCntSize;
	bool initPathResDone;

	/* Error information */
	int32_t	errNum;
	int32_t	logLevel;
	bool bypassIrqNotify; //default false. true for disable that IRQ handler thread notify (testcase only)
} ContextStruct;


int32_t cmdqTzInitialize(void);

int32_t cmdqSuspendTask(void);

int32_t cmdqResumeTask(void);

void cmdqTzHandleIRQ(int32_t thread);

void cmdqTzHandleError(int32_t index, int32_t value);

void cmdqTzHandleDone(int32_t index, int32_t value);

int32_t cmdqTzComposeAndSubmitTaskAsync(struct iwcCmdqMessage_t *pIwcMessage,
	struct tlApiCmdqExecMetadata_t *pMetadata, TaskStruct **ppTaskOut);
int32_t cmdqTzComposeAndSubmitTask(struct iwcCmdqMessage_t *pIwcMessage,
	struct tlApiCmdqExecMetadata_t *pMetadata);
int32_t cmdqTzSubmitTask(TaskStruct *pTask, int32_t thread);
int32_t cmdqTzWaitTask(TaskStruct *pTask, int32_t thread);

void cmdqTzDeInitialize(void);

int32_t cmdqTzSetupPathResouce(const uint32_t sharedCookiePA, const uint32_t sharedCookieSize,
	struct tlApiCmdqPathResMetadata_t *pResource);
int32_t cmdqTzTeardownResouce(void);
int32_t cmdqTzCancelTask(iwcCmdqCancelTask_t *pCancelTask);
int32_t cmdqTzSetupTaskResouce();
void cmdqTzCoreClearEvent(CMDQ_EVENT_ENUM event);
void cmdqTzCoreSetEvent(CMDQ_EVENT_ENUM event);
uint32_t cmdqTzCoreGetEvent(CMDQ_EVENT_ENUM event);

bool cmdq_tz_is_path_resource_ready(void);

void cmdqTzErrorDump(void);

int32_t cmdq_tz_parse_instruction(const uint32_t *pCmd, char *textBuf, int bufLen);

int32_t cmdq_tz_init_shared_exec_cnt_region(const uint32_t startPA, const uint32_t size);

void cmdq_tz_dump_thread(uint32_t thread, uint32_t line);
void cmdq_tz_dump_status(const char *tag);

int32_t cmdq_tz_map_physical_addr(uint8_t **va, const uint64_t pa, const uint32_t length);
int32_t cmdq_tz_unmap_physical_addr(const uint8_t *va, const uint32_t length);

void cmdq_tz_set_log_level(int32_t level);
bool cmdq_tz_should_print_msg(void);
int32_t cmdq_tz_map_physical_addr_by_flag(uint8_t **va, const uint64_t pa, const uint32_t length, uint32_t flag);


#endif  // __CMDQ_SEC_CORE_H__


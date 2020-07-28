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

#ifndef __CMDQ_SEC_DEF_H__
#define __CMDQ_SEC_DEF_H__

#include "cmdq_sec_common.h"
#include "cmdq_sec_engine.h"


/**
 * Option for wait implementation
 * .Target is async submit. However t-base doesn't support sync methods between NWd and SWd,
 * .Config as CMDQ_SYNC_SUBMIT_SUPPORT.
 *
 * Possible options:
 * 1. polling: set CMDQ_DEBUG_POLL_TASK_DONE = 1, and don't care CMDQ_SYNC_SUBMIT_IMPL
 * 2. sync submit: set CMDQ_DEBUG_POLL_TASK_DONE = 0 and CMDQ_SYNC_SUBMIT_IMPL = 1
 * 3. async submit: set CMDQ_DEBUG_POLL_TASK_DONE = 0 and CMDQ_SYNC_SUBMIT_IMPL = 0
 *
 * 2014.09. tbase secure IRQ has HW reboot issue, use polling method temporary
 */

#define CMDQ_DEBUG_POLL_TASK_DONE (1) /* 1: polling CMDQ HW status to know exec done */

/*  */
/* ERROR code number (ERRNO) */
/* note the error result returns negative value, i.e, -(ERRNO) */
/*  */
#define	CMDQ_REC_ERR_NOMEM			(12)	/* out of memory */
#define	CMDQ_REC_ERR_FAULT			(14)	/* bad address */
#define	CMDQ_REC_ERR_INVAL			(16)	/* bad address */
#define	CMDQ_REC_ERR_BUSY			(18)	/* bad address */

/**
 * CMDQ HW define
 */

/* secure path new added data */
#define CMDQ_MIN_SECURE_THREAD_ID	(12)
#define CMDQ_MAX_SECURE_THREAD_COUNT (3) // support executing secure task
#define CMDQ_MAX_FIXED_TASK	 (3 * 16)	 // static allocation task
#define CMDQ_IRQ_STATUS_BIT_FIELD (0xFFFF)

/* same as NWd*/
#define CMDQ_INIT_FREE_TASK_COUNT       (8)
#define CMDQ_MAX_THREAD_COUNT           (16)
#define CMDQ_MAX_HIGH_PRIORITY_THREAD_COUNT (6)	/* Thread that are high-priority (display threads) */
#define CMDQ_MAX_ERROR_COUNT            (2)
#define CMDQ_MAX_RETRY_COUNT            (1)
#define CMDQ_MAX_TASK_IN_THREAD         (16)
#define CMDQ_MAX_READ_SLOT_COUNT        (4)

#define CMDQ_INST_SIZE                  (2 * sizeof(uint32_t))	/* instruction is 64-bit */

#define CMDQ_MAX_LOOP_COUNT             (1000000)
#define CMDQ_MAX_INST_CYCLE             (27)
#define CMDQ_MIN_AGE_VALUE              (5)

#define CMDQ_MAX_COOKIE_VALUE           (0xFFFFFFFF)	/* max value of CMDQ_THR_EXEC_CMD_CNT (value starts from 0) */

#define CMDQ_INVALID_THREAD             (-1)
#define CMDQ_ARGA_SUBSYS_LSB            (16)


typedef enum CMDQ_SCENARIO_ENUM {
	CMDQ_SCENARIO_JPEG_DEC = 0,
	CMDQ_SCENARIO_PRIMARY_DISP = 1,
	CMDQ_SCENARIO_PRIMARY_MEMOUT = 2,
	CMDQ_SCENARIO_PRIMARY_ALL = 3,
	CMDQ_SCENARIO_SUB_DISP = 4,
	CMDQ_SCENARIO_SUB_MEMOUT = 5,
	CMDQ_SCENARIO_SUB_ALL = 6,
	CMDQ_SCENARIO_MHL_DISP = 7,
	CMDQ_SCENARIO_RDMA0_DISP = 8,
	CMDQ_SCENARIO_RDMA0_COLOR0_DISP = 9,
	CMDQ_SCENARIO_RDMA1_DISP = 10,

	/* Trigger loop scenario does not enable HWs */
	CMDQ_SCENARIO_TRIGGER_LOOP = 11,

	/* client from user space, so the cmd buffer is in user space. */
	CMDQ_SCENARIO_USER_MDP = 12,

	CMDQ_SCENARIO_DEBUG = 13,
	CMDQ_SCENARIO_DEBUG_PREFETCH = 14,

	/* ESD check */
	CMDQ_SCENARIO_DISP_ESD_CHECK = 15,
	/* for screen capture to wait for RDMA-done without blocking config thread */
	CMDQ_SCENARIO_DISP_SCREEN_CAPTURE = 16,

	/* notifiy there are some tasks exec done in secure path */
	CMDQ_SCENARIO_SECURE_NOTIFY_LOOP = 17,

	CMDQ_SCENARIO_DISP_PRIMARY_DISABLE_SECURE_PATH = 18,
	CMDQ_SCENARIO_DISP_SUB_DISABLE_SECURE_PATH = 19,

	/* color path request from kernel */
	CMDQ_SCENARIO_DISP_COLOR = 20,
	/* color path request from user sapce */
	CMDQ_SCENARIO_USER_DISP_COLOR = 21,

	/* [phased out]client from user space, so the cmd buffer is in user space. */
	CMDQ_SCENARIO_USER_SPACE = 22,

	CMDQ_SCENARIO_DISP_MIRROR_MODE = 23,

	CMDQ_SCENARIO_DISP_CONFIG_AAL = 24,
	CMDQ_SCENARIO_DISP_CONFIG_PRIMARY_GAMMA = 25,
	CMDQ_SCENARIO_DISP_CONFIG_SUB_GAMMA = 26,
	CMDQ_SCENARIO_DISP_CONFIG_PRIMARY_DITHER = 27,
	CMDQ_SCENARIO_DISP_CONFIG_SUB_DITHER = 28,
	CMDQ_SCENARIO_DISP_CONFIG_PRIMARY_PWM = 29,
	CMDQ_SCENARIO_DISP_CONFIG_SUB_PWM = 30,
	CMDQ_SCENARIO_DISP_CONFIG_PRIMARY_PQ = 31,
	CMDQ_SCENARIO_DISP_CONFIG_SUB_PQ = 32,
	CMDQ_SCENARIO_DISP_CONFIG_OD = 33,

	CMDQ_SCENARIO_RDMA2_DISP = 34,

	CMDQ_MAX_SCENARIO_COUNT	/* ALWAYS keep at the end */
} CMDQ_SCENARIO_ENUM;

typedef enum CMDQ_HW_THREAD_PRIORITY_ENUM {
	CMDQ_THR_PRIO_NORMAL = 0,	/* nomral (lowest) priority */
	CMDQ_THR_PRIO_DISPLAY_TRIGGER = 1,	/* trigger loop (enables display mutex) */

	CMDQ_THR_PRIO_DISPLAY_ESD = 3,	/* display ESD check (every 2 secs) */
	CMDQ_THR_PRIO_DISPLAY_CONFIG = 3,	/* display config (every frame) */

	CMDQ_THR_PRIO_MAX = 7,	/* maximum possible priority */
} CMDQ_HW_THREAD_PRIORITY_ENUM;

typedef enum CMDQ_DATA_REGISTER_ENUM {
	/* Value Reg, we use 32-bit */
	/* Address Reg, we use 64-bit */
	/* Note that R0-R15 and P0-P7 actullay share same memory */
	/* and R1 cannot be used. */

	CMDQ_DATA_REG_JPEG = 0x00,	/* R0 */
	CMDQ_DATA_REG_JPEG_DST = 0x11,	/* P1 */

	CMDQ_DATA_REG_PQ_COLOR = 0x04,	/* R4 */
	CMDQ_DATA_REG_PQ_COLOR_DST = 0x13,	/* P3 */

	CMDQ_DATA_REG_2D_SHARPNESS_0 = 0x05,	/* R5 */
	CMDQ_DATA_REG_2D_SHARPNESS_0_DST = 0x14,	/* P4 */

	CMDQ_DATA_REG_2D_SHARPNESS_1 = 0x0a,	/* R10 */
	CMDQ_DATA_REG_2D_SHARPNESS_1_DST = 0x16,	/* P6 */

	CMDQ_DATA_REG_DEBUG = 0x0b,	/* R11 */
	CMDQ_DATA_REG_DEBUG_DST = 0x17,	/* P7 */

	/* sentinel value for invalid register ID */
	CMDQ_DATA_REG_INVALID = -1,
} CMDQ_DATA_REGISTER_ENUM;

/* CMDQ Events */
#undef DECLARE_CMDQ_EVENT
#define DECLARE_CMDQ_EVENT(name, val) name = val,
typedef enum CMDQ_EVENT_ENUM {
#include "cmdq_sec_event.h"
} CMDQ_EVENT_ENUM;
#undef DECLARE_CMDQ_EVENT

typedef struct cmdqReadRegStruct {
	uint32_t count;		/* number of entries in regAddresses */
	uint32_t *regAddresses;	/* an array of register addresses */
} cmdqReadRegStruct;

typedef struct cmdqRegValueStruct {
	/* number of entries in result */
	uint32_t count;

	/* array of register values. */
	/* in the same order as cmdqReadRegStruct */
	uint32_t *regValues;
} cmdqRegValueStruct;

typedef struct cmdqReadAddressStruct {
	uint32_t count;		/* [IN] number of entries in result. */

	/* [IN] array of physical addresses to read. */
	/* these value must allocated by CMDQ_IOCTL_ALLOC_WRITE_ADDRESS ioctl */
	/*  */
	/* indeed param dmaAddresses should be UNSIGNED LONG type for 64 bit kernel.*/
	/* Considering our plartform supports max 4GB RAM(upper-32bit don't care for SW)*/
	/* and consistent common code interface, remain uint32_t type.*/
	uint32_t *dmaAddresses;

	uint32_t *values;	/* [OUT] values that dmaAddresses point into */
} cmdqReadAddressStruct;

typedef struct cmdqCommandStruct {
	uint32_t scenario;	/* [IN] deprecated. will remove in the future. */
	uint32_t priority;	/* [IN] task schedule priorty. this is NOT HW thread priority. */
	uint64_t engineFlag;	/* [IN] bit flag of engines used. */
	uint32_t *pVABase;	/* [IN] pointer to instruction buffer */
	uint32_t blockSize;	/* [IN] size of instruction buffer, in bytes. */
	cmdqReadRegStruct regRequest;	/* [IN] request to read register values at the end of command */
	cmdqRegValueStruct regValue;	/* [OUT] register values of regRequest */
	cmdqReadAddressStruct readAddress;	/* [IN/OUT] physical addresses to read value */
	uint32_t debugRegDump;	/* [IN] set to non-zero to enable register debug dump. */
	void *privateData;	/* [Reserved] This is for CMDQ driver usage itself. Not for client. */
} cmdqCommandStruct;

#endif				/* __CMDQ_SEC_DEF_H__ */

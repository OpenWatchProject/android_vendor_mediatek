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

#include "cmdq_sec_platform.h"
#include "cmdq_sec_core.h"
#include "cmdq_sec_reg.h"

#include "drStd.h"
#include "DrApi/DrApiMm.h"

typedef struct RegDef {
	int offset;
	const char *name;
} RegDef;

void cmdq_tz_poke_notify_loop(void)
{
	const uint32_t notifyEvent = CMDQ_SYNC_SECURE_THR_EOF;
	int32_t value = 0;

	/* raise event to notify normal CMDQ driver that secure path exec done */
	CMDQ_REG_SET32(CMDQ_SYNC_TOKEN_UPD, (1L << 16) | notifyEvent);
	CMDQ_REG_SET32(CMDQ_SYNC_TOKEN_ID, notifyEvent);

	CMDQ_VERBOSE("value of CMDQ_SYNC_SECURE_THR_EOF is %d\n", CMDQ_REG_GET32(CMDQ_SYNC_TOKEN_VAL));
}

static uint32_t gIsqCount = 0;
void cmdq_tz_irq_handler(void)
{
    const uint32_t startIndex = CMDQ_MIN_SECURE_THREAD_ID;
    const uint32_t endIndex   = CMDQ_MIN_SECURE_THREAD_ID + CMDQ_MAX_SECURE_THREAD_COUNT;
    uint32_t i;
    uint32_t irqStatus;
	uint32_t raisedIRQ = 0;
	uint32_t oldRaisedIRQ = 0;
    drApiResult_t ret = DRAPI_OK;
	bool notifyToNWd = false;

	gIsqCount += 1;

	/* get IRQ status */
    irqStatus = CMDQ_REG_GET32(CMDQ_SECURE_IRQ_STATUS) & CMDQ_IRQ_STATUS_BIT_FIELD;

	CMDQ_MSG("-->IRQ: irqStatus:0x%08x, gIsqCount:%d\n", irqStatus, gIsqCount);

	/* handle IRQ */
    for (i = startIndex; ((CMDQ_IRQ_STATUS_BIT_FIELD != irqStatus) && (i < endIndex)); i++) {
        /* STATUS bit 0 means IRQ asserted */
        if (0x0 == (irqStatus & (1 << i))) {
            cmdqTzHandleIRQ(i);
            /* mark irqStatus to 1 to denote finished processing */
            /* and we can early-exit if no more threads being asserted */
            irqStatus |= (1 << i);

			/* should notify normal world */
			raisedIRQ |= (1 << i); /* mark 1 to normal world which IRQ is asserted*/
			//notifyToNWd = true;
        }
    }

	CMDQ_MSG("IRQ: [1]gIsqCount:%d\n", gIsqCount);

	/* notify to NWD that secure path execute done */
	if (notifyToNWd) {
		oldRaisedIRQ = CMDQ_REG_GET32(CMDQ_SHARED_IRQ_RAISED_FLAG);
		CMDQ_REG_SET32(CMDQ_SHARED_IRQ_RAISED_FLAG, (raisedIRQ | oldRaisedIRQ));
		cmdq_tz_poke_notify_loop();
	}

#if (0 == CMDQ_DEBUG_POLL_TASK_DONE)
	cmdq_tz_resume_worker_thread(DRIVER_THREAD_NO_DCIH, "IPC", "ISR");
#endif

	CMDQ_MSG("<--IRQ: notifyToNWd:%d, raisedIRQ:0x%08x, oldRaisedIRQ:0x%08x, setRaisedIRQ:0x%08x, gIsqCount:%d\n",
		notifyToNWd, raisedIRQ, oldRaisedIRQ, (raisedIRQ | oldRaisedIRQ), gIsqCount);
}

const bool cmdq_tz_is_a_secure_thread(const int32_t thread)
{
	if ((CMDQ_MIN_SECURE_THREAD_ID <= thread) &&
		(CMDQ_MIN_SECURE_THREAD_ID + CMDQ_MAX_SECURE_THREAD_COUNT> thread)) {
		return true;
	}

	return false;
}

/*
#define MAP_PHYS_AND_CHECK(va, pa, length, flag, ret) \
{ \
    ret = drApiMapPhys((addr_t)va, length, (addr_t)pa, flag); \
    CMDQ_MSG("map phys, result[0x%x, 0x%x, %d][%x]\n", va, pa, length, ret); \
    if (DRAPI_OK != ret) { \
        CMDQ_ERR("map phys failed: VA[0x%x], PA[0x%x], lendth[%d], ret=[%x]=[%x]\n", va, pa, length, ret, DRAPI_ERROR_MAJOR(ret)); \
        break; \
    } \
} \
*/

const uint32_t cmdq_tz_get_subsys_LSB_in_argA(void)
{
	return 16;
}

int32_t cmdq_tz_subsys_from_phys_addr(uint32_t physAddr)
{
#define DISP_PWM_BASE 0x1100E000

	const int32_t msb = (physAddr & 0x0FFFF0000) >> 16;

#undef DECLARE_CMDQ_SUBSYS
#define DECLARE_CMDQ_SUBSYS(addr, id, grp, base) case addr: return id;
	switch (msb) {
#include "cmdq_sec_subsys.h"
	}

	/* Extra handle for HW registers which not in GCE subsys table, should check and add by case */
	if (DISP_PWM_BASE == (physAddr & 0xFFFFF000)) {
		CMDQ_MSG("Special handle subsys (PWM), physAddr:0x%08x\n", physAddr);
		return CMDQ_SPECIAL_SUBSYS_ADDR;
	}
	CMDQ_ERR("unrecognized subsys, msb=0x%04x, physAddr:0x%08x\n", msb, physAddr);
	return -1;
#undef DECLARE_CMDQ_SUBSYS
}

bool cmdq_tz_should_enable_prefetch(CMDQ_SCENARIO_ENUM scenario)
{
	switch (scenario) {
	case CMDQ_SCENARIO_PRIMARY_DISP:
	case CMDQ_SCENARIO_PRIMARY_ALL:
	case CMDQ_SCENARIO_DEBUG_PREFETCH:	/* HACK: force debug into 0/1 thread */
		/* any path that connects to Primary DISP HW */
		/* should enable prefetch. */
		/* MEMOUT scenarios does not. */
		/* Also, since thread 0/1 shares one prefetch buffer, */
		/* we allow only PRIMARY path to use prefetch. */
		return true;

	default:
		return false;
	}

	return false;
}

int cmdq_tz_thread_index_from_scenario(CMDQ_SCENARIO_ENUM scenario)
{
	if (cmdq_tz_is_disp_scenario(scenario)){
		/* only support display scenario now */
		return 12;
	}else if (CMDQ_SCENARIO_DEBUG == scenario){
		return 12;
	}
	return CMDQ_INVALID_THREAD;
}

bool cmdq_tz_is_disp_scenario(const CMDQ_SCENARIO_ENUM scenario)
{
	switch (scenario) {
	case CMDQ_SCENARIO_PRIMARY_DISP:
	case CMDQ_SCENARIO_PRIMARY_MEMOUT:
	case CMDQ_SCENARIO_PRIMARY_ALL:
	case CMDQ_SCENARIO_SUB_DISP:
	case CMDQ_SCENARIO_SUB_MEMOUT:
	case CMDQ_SCENARIO_SUB_ALL:
	case CMDQ_SCENARIO_MHL_DISP:
	case CMDQ_SCENARIO_RDMA0_DISP:
	case CMDQ_SCENARIO_RDMA0_COLOR0_DISP:
	case CMDQ_SCENARIO_RDMA1_DISP:
	case CMDQ_SCENARIO_TRIGGER_LOOP:
	case CMDQ_SCENARIO_DISP_ESD_CHECK:
	case CMDQ_SCENARIO_DISP_SCREEN_CAPTURE:
	case CMDQ_SCENARIO_DISP_MIRROR_MODE:
	case CMDQ_SCENARIO_DISP_CONFIG_OD:
		/* color path */
	case CMDQ_SCENARIO_DISP_COLOR:
	case CMDQ_SCENARIO_USER_DISP_COLOR:
		/* secure path */
	case CMDQ_SCENARIO_DISP_PRIMARY_DISABLE_SECURE_PATH:
	case CMDQ_SCENARIO_DISP_SUB_DISABLE_SECURE_PATH:
		return true;
	default:
		return false;
	}
	/* freely dispatch */
	return false;
}

CMDQ_HW_THREAD_PRIORITY_ENUM cmdq_tz_priority_from_scenario(CMDQ_SCENARIO_ENUM scenario)
{
	switch (scenario) {
	case CMDQ_SCENARIO_PRIMARY_DISP:
	case CMDQ_SCENARIO_PRIMARY_ALL:
	case CMDQ_SCENARIO_SUB_DISP:
	case CMDQ_SCENARIO_SUB_ALL:
	case CMDQ_SCENARIO_MHL_DISP:
	case CMDQ_SCENARIO_RDMA0_DISP:
	case CMDQ_SCENARIO_DISP_MIRROR_MODE:
	case CMDQ_SCENARIO_DISP_CONFIG_OD:
	case CMDQ_SCENARIO_SUB_MEMOUT:
	case CMDQ_SCENARIO_PRIMARY_MEMOUT:
		/* color path */
	case CMDQ_SCENARIO_DISP_COLOR:
	case CMDQ_SCENARIO_USER_DISP_COLOR:
		/* secure path */
	case CMDQ_SCENARIO_DISP_PRIMARY_DISABLE_SECURE_PATH:
	case CMDQ_SCENARIO_DISP_SUB_DISABLE_SECURE_PATH:
		/* currently, a prefetch thread is always in high priority. */
		return CMDQ_THR_PRIO_DISPLAY_CONFIG;

		/* HACK: force debug into 0/1 thread */
	case CMDQ_SCENARIO_DEBUG_PREFETCH:
		return CMDQ_THR_PRIO_DISPLAY_CONFIG;

	case CMDQ_SCENARIO_DISP_ESD_CHECK:
	case CMDQ_SCENARIO_DISP_SCREEN_CAPTURE:
		return CMDQ_THR_PRIO_DISPLAY_ESD;

	default:
		/* other cases need exta logic, see below. */
		break;
	}

	if (CMDQ_SCENARIO_TRIGGER_LOOP == scenario)
		return CMDQ_THR_PRIO_DISPLAY_TRIGGER;
	else
		return CMDQ_THR_PRIO_NORMAL;
}

uint64_t cmdq_tz_rec_flag_from_scenario(CMDQ_SCENARIO_ENUM scn)
{
	uint64_t flag = 0;

	switch (scn) {
	case CMDQ_SCENARIO_JPEG_DEC:
		flag = (1LL << CMDQ_ENG_JPEG_DEC);
		break;
	case CMDQ_SCENARIO_PRIMARY_DISP:
		flag = (1LL << CMDQ_ENG_DISP_OVL0) |
		    (1LL << CMDQ_ENG_DISP_COLOR0) |
		    (1LL << CMDQ_ENG_DISP_AAL) |
		    (1LL << CMDQ_ENG_DISP_GAMMA) |
		    (1LL << CMDQ_ENG_DISP_RDMA0) |
		    (1LL << CMDQ_ENG_DISP_UFOE) | (1LL << CMDQ_ENG_DISP_DSI0_CMD);
		break;
	case CMDQ_SCENARIO_PRIMARY_MEMOUT:
		flag = 0LL;
		break;
	case CMDQ_SCENARIO_PRIMARY_ALL:
		flag = ((1LL << CMDQ_ENG_DISP_OVL0) |
			(1LL << CMDQ_ENG_DISP_WDMA0) |
			(1LL << CMDQ_ENG_DISP_COLOR0) |
			(1LL << CMDQ_ENG_DISP_AAL) |
			(1LL << CMDQ_ENG_DISP_GAMMA) |
			(1LL << CMDQ_ENG_DISP_RDMA0) |
			(1LL << CMDQ_ENG_DISP_UFOE) | (1LL << CMDQ_ENG_DISP_DSI0_CMD));
		break;
	case CMDQ_SCENARIO_SUB_DISP:
		flag = ((1LL << CMDQ_ENG_DISP_OVL1) |
			(1LL << CMDQ_ENG_DISP_RDMA1) | (1LL << CMDQ_ENG_DISP_DPI));
		break;
	case CMDQ_SCENARIO_SUB_MEMOUT:
		flag = ((1LL << CMDQ_ENG_DISP_OVL1) | (1LL << CMDQ_ENG_DISP_WDMA1));
		break;
	case CMDQ_SCENARIO_SUB_ALL:
		flag = ((1LL << CMDQ_ENG_DISP_OVL1) |
			(1LL << CMDQ_ENG_DISP_WDMA1) |
			(1LL << CMDQ_ENG_DISP_RDMA1) | (1LL << CMDQ_ENG_DISP_DPI));
		break;
	case CMDQ_SCENARIO_RDMA0_DISP:
		flag = ((1LL << CMDQ_ENG_DISP_RDMA0) | (1LL << CMDQ_ENG_DISP_DSI0_CMD));
		break;
	case CMDQ_SCENARIO_RDMA0_COLOR0_DISP:
		flag = ((1LL << CMDQ_ENG_DISP_RDMA0) |
			(1LL << CMDQ_ENG_DISP_COLOR0) |
			(1LL << CMDQ_ENG_DISP_AAL) |
			(1LL << CMDQ_ENG_DISP_GAMMA) |
			(1LL << CMDQ_ENG_DISP_UFOE) | (1LL << CMDQ_ENG_DISP_DSI0_CMD));
		break;
	case CMDQ_SCENARIO_MHL_DISP:
	case CMDQ_SCENARIO_RDMA1_DISP:
		flag = ((1LL << CMDQ_ENG_DISP_RDMA1) | (1LL << CMDQ_ENG_DISP_DPI));
		break;
	case CMDQ_SCENARIO_DISP_CONFIG_OD:
		flag = 0LL;
		break;
	case CMDQ_SCENARIO_TRIGGER_LOOP:
		/* Trigger loop does not related to any HW by itself. */
		flag = 0LL;
		break;

	case CMDQ_SCENARIO_USER_SPACE:
		/* user space case, engine flag is passed seprately */
		flag = 0LL;
		break;

	case CMDQ_SCENARIO_DEBUG:
	case CMDQ_SCENARIO_DEBUG_PREFETCH:
		flag = 0LL;
		break;

	case CMDQ_SCENARIO_DISP_ESD_CHECK:
	case CMDQ_SCENARIO_DISP_SCREEN_CAPTURE:
		/* ESD check uses separate thread (not config, not trigger) */
		flag = 0LL;
		break;

	case CMDQ_SCENARIO_DISP_COLOR:
	case CMDQ_SCENARIO_USER_DISP_COLOR:
		/* color path */
		flag = 0LL;
		break;

	case CMDQ_SCENARIO_DISP_MIRROR_MODE:
		flag = 0LL;
		break;

	case CMDQ_SCENARIO_DISP_PRIMARY_DISABLE_SECURE_PATH:
	case CMDQ_SCENARIO_DISP_SUB_DISABLE_SECURE_PATH:
		/* secure path */
		flag = 0LL;
		break;

	case CMDQ_SCENARIO_SECURE_NOTIFY_LOOP:
		flag = 0LL;
		break;

	default:
		if (scn < 0 || scn >= CMDQ_MAX_SCENARIO_COUNT) {
			/* Error status print */
			CMDQ_ERR("Unknown scenario type %d\n", scn);
		}
		flag = 0LL;
		break;
	}

	return flag;
}

void cmdq_tz_dump_mmsys_config(void)
{
#define MMSYS_CONFIG_BASE 0x14000000
	int i = 0;
	uint32_t value = 0;

	static const struct RegDef configRegisters[] = {
		{0x01c, "ISP_MOUT_EN"},
		{0x020, "MDP_RDMA_MOUT_EN"},
		{0x024, "MDP_PRZ0_MOUT_EN"},
		{0x028, "MDP_PRZ1_MOUT_EN"},
		{0x02C, "MDP_TDSHP_MOUT_EN"},
		{0x030, "DISP_OVL0_MOUT_EN"},
		{0x034, "DISP_OVL1_MOUT_EN"},
		{0x038, "DISP_DITHER_MOUT_EN"},
		{0x03C, "DISP_UFOE_MOUT_EN"},
		/* {0x040, "MMSYS_MOUT_RST"}, */
		{0x044, "MDP_PRZ0_SEL_IN"},
		{0x048, "MDP_PRZ1_SEL_IN"},
		{0x04C, "MDP_TDSHP_SEL_IN"},
		{0x050, "MDP_WDMA_SEL_IN"},
		{0x054, "MDP_WROT_SEL_IN"},
		{0x058, "DISP_COLOR_SEL_IN"},
		{0x05C, "DISP_WDMA_SEL_IN"},
		{0x060, "DISP_UFOE_SEL_IN"},
		{0x064, "DSI0_SEL_IN"},
		{0x068, "DPI0_SEL_IN"},
		{0x06C, "DISP_RDMA0_SOUT_SEL_IN"},
		{0x070, "DISP_RDMA1_SOUT_SEL_IN"},
		{0x0F0, "MMSYS_MISC"},
		/* ACK and REQ related */
		{0x8a0, "DISP_DL_VALID_0"},
		{0x8a4, "DISP_DL_VALID_1"},
		{0x8a8, "DISP_DL_READY_0"},
		{0x8ac, "DISP_DL_READY_1"},
		{0x8b0, "MDP_DL_VALID_0"},
		{0x8b4, "MDP_DL_READY_0"}
	};

	for (i = 0; i < sizeof(configRegisters) / sizeof(configRegisters[0]); ++i) {
		value = CMDQ_REG_GET16(MMSYS_CONFIG_BASE + configRegisters[i].offset);
		CMDQ_ERR("%s: 0x%08x\n", configRegisters[i].name, value);
	}

	return;
}


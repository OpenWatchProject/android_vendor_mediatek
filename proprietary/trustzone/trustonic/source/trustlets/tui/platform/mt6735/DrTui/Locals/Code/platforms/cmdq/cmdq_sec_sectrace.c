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
#ifdef CMDQ_SECTRACE_SUPPORT
#include "drsectrace.h"

#include "cmdq_sec_sectrace.h"
#include "cmdq_sec_core.h"
#include "cmdq_sec_reg.h"

#define SECTRACE_MAP_START  (CMDQ_SECTRACE_VA)
#define SECTRACE_MAP_SZ		(2*1024*1024)
#define SECTRACE_MAP_END	(SECTRACE_MAP_START + SECTRACE_MAP_SZ)

#define SZ_2M				(0x00200000)

#define ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))
#define ALIGN(x,a)			ALIGN_MASK(x,(a)-1)


static unsigned long map_size = 0;

int32_t cmdq_tz_sectrace_map(unsigned long pa, unsigned long size)
{
	drApiResult_t ret;
	unsigned long pa_align;

	pa_align = pa & ~(SZ_2M - 1);
	map_size = pa + size - pa_align;
	map_size = ALIGN(map_size, SZ_2M);

	CMDQ_LOG("[sectrace]map: pa:0x%x, size:%d\n", pa, size);

	ret = drApiMapPhys((addr_t)SECTRACE_MAP_START, map_size, (addr_t)pa_align, MAP_READABLE|MAP_WRITABLE|MAP_NOT_SECURE);
	if (E_OK != ret) {
		CMDQ_ERR("[sectrace]map: drApiMapPhys failed, ret:0x%x\n",ret);
		map_size = 0;
		return -1;
	}

	sectrace_enable((addr_t)(SECTRACE_MAP_START + (pa & (SZ_2M-1))), size);

	return 0;
}

int32_t cmdq_tz_sectrace_unmap(void)
{
	drApiResult_t ret;

	CMDQ_LOG("[sectrace]unmap\n");

	sectrace_disable();

	if (0 != map_size) {
		ret = drApiUnmap((addr_t)SECTRACE_MAP_START, map_size);
		if (E_OK != ret) {
			CMDQ_ERR("[sectrace]unmap: drApiUnmap failed, ret:0x%x\n", ret);
			return -1;
		}
		map_size = 0;
	}

	return 0;
}

void cmdq_tz_sectrace_transact(void)
{
	CMDQ_LOG("[sectrace]transact\n");

	sectrace_transact();
}

/**
 * global variables for timestamp log
 * tlApiGetSecureTimestamp returns microseconds(us)
 * note timestamp log dose not support nest profile */
#include "DrApi/DrApiTime.h"
timestamp_t gCmdqProfT1;
timestamp_t gCmdqProfT2;

/* sectrace record APIs */
void cmdq_tz_sectrace_profile_start(const char *tag)
{
	const int32_t profile = cmdq_tz_profile_enabled();

	CMDQ_VERBOSE("SECTRACE_START:%s\n", tag);

	if (0 >=profile) {
		/* profile disable */
	} else if (1 == profile) {
		/* sectrace */
		SECTRACE_BEGIN(tag);
	} else if (2 == profile) {
		/* timestamp log */
		drApiGetSecureTimestamp(&gCmdqProfT1);
	}
}

void cmdq_tz_sectrace_profile_end(const char *tag)
{
	const int32_t profile = cmdq_tz_profile_enabled();

	CMDQ_VERBOSE("SECTRACE_END:%s\n", tag);

	if (0 >=profile) {
		/* profile disable */
	} else if (1 == profile) {
		/* sectrace */
		SECTRACE_END();
	} else if (2 == profile) {
		/* timestamp log */
		drApiGetSecureTimestamp(&gCmdqProfT2);
		CMDQ_LOG("[PROF]%s :%d us, T1: 0x%x%x, T2: 0x%x%x\n",
			tag,
			(uint32_t)(gCmdqProfT2 - gCmdqProfT1),
			(uint32_t)(gCmdqProfT1 >> 32), (uint32_t)gCmdqProfT1,
			(uint32_t)(gCmdqProfT2 >> 32), (uint32_t)gCmdqProfT2);
	}
}

void cmdq_tz_sectrace_profile_oneshot(const char *tag)
{
	timestamp_t t1;
	timestamp_t t2;
	const int32_t profile = cmdq_tz_profile_enabled();

	CMDQ_VERBOSE("SECTRACE_ONCSHOT:%s\n", tag);

	if (0 >=profile) {
		/* profile disable */
	} else if (1 == profile) {
		/* sectrace */
		SECTRACE_BEGIN(tag);
		SECTRACE_END();
	} else if (2 == profile) {
		/* timestamp log */
		drApiGetSecureTimestamp(&t1);
		drApiGetSecureTimestamp(&t2);

		CMDQ_LOG("[PROF]%s: %d us, T1: 0x%x%x, T2: 0x%x%x\n",
			tag,
			(uint32_t)(t2 - t1),
			(uint32_t)(t1 >> 32), (uint32_t)t1,
			(uint32_t)(t2 >> 32), (uint32_t)t2);
	}
}

#else
#include "cmdq_sec_common.h"
#include "cmdq_sec_sectrace.h"

int32_t cmdq_tz_sectrace_map(unsigned long pa, unsigned long size){ return 0; }
int32_t cmdq_tz_sectrace_unmap(void){ return 0; }
void cmdq_tz_sectrace_transact(void){};

void cmdq_tz_sectrace_profile_start(const char *tag){};
void cmdq_tz_sectrace_profile_end(const char *tag){};
void cmdq_tz_sectrace_profile_oneshot(const char *tag){};
#endif

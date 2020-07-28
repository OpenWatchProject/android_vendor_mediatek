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

#ifndef _H_DDP_LOG_
#define _H_DDP_LOG_

#ifndef LOG_TAG
#define LOG_TAG "DDP"
#endif

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "ddp_debug.h"
#include "log.h"

#define DISP_LOG_RAW(fmt, args...)	TUI_LOGV(fmt, ##args)

#define DISP_LOG_V(fmt, args...)	TUI_LOGV("["LOG_TAG"]"fmt, ##args)
#define DISP_LOG_D(fmt, args...)	TUI_LOGD("["LOG_TAG"]"fmt, ##args)
#define DISP_LOG_I(fmt, args...)	TUI_LOGI("["LOG_TAG"]"fmt, ##args)
#define DISP_LOG_W(fmt, args...)	TUI_LOGW("["LOG_TAG"]"fmt, ##args)
#define DISP_LOG_E(fmt, args...)	TUI_LOGE("["LOG_TAG"]error:"fmt, ##args);

#define DDPIRQ(fmt, args...)		DISP_LOG_V(fmt, ##args)
#define DDPDUMP(fmt, args...)		DISP_LOG_V(fmt, ##args)
#define DDPDBG(fmt, args...)		DISP_LOG_V(fmt, ##args)
#define DDPDEBUG_D(fmt, args...)	DISP_LOG_V(fmt, ##args)
#define DDPMSG(fmt, args...)		DISP_LOG_V(fmt, ##args)
#define DDPERR(fmt, args...)		DISP_LOG_E(fmt, ##args)

#ifndef ASSERT
#define ASSERT(expr)                             \
    do {                                         \
        if(expr) break;                          \
		while(1) {								\
			DISP_LOG_E("ASSERT FAILED %s, %d\n", __FILE__, __LINE__);\
		}\
    }while(0)
#endif

#define DDPAEE(fmt, args...)                        \
    do {\
		DISP_LOG_E(fmt, ##args);\
		DISP_LOG_E("AEE %s, %d\n", __FILE__, __LINE__);\
    }while(1)

#define DISPFUNC() DISP_LOG_I("[DISP]func|%s\n", __func__)


#endif

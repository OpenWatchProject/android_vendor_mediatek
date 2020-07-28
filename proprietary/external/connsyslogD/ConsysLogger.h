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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*******************************************************************************
 *
 * Filename:
 * ---------
 *   consyslogger.h
 *
 * Description:
 * ------------
 *   Header file of logger
 *
 *******************************************************************************/

#ifndef __CONSYS_MAIN_H__
#define __CONSYS_MAIN_H__

#include <semaphore.h>
#include <pthread.h>
#include "GlbDefine.h"
#include "Engine.h"
#include "LogWriter.h"
#include "ConsysLog.h"

using namespace consyslogger;


typedef enum {
    MODE_UNKNOWN = -1,// -1
    MODE_IDLE,
    MODE_START,
    MODE_END
} LOGGING_MODE;

typedef enum {
	NORMAL_BOOT_MODE = 0,	
	META_BOOT_MODE = 1,
	FACTORY_BOOT_MODE = 4,
	ATE_FACTORY_BOOT_MODE = 6,
	END_BOOT_MODE
}BOOT_MODE;

// this is for factory mode feature.LENOVO request

typedef enum {
    OP_RESERVED = 0,                // 0
//    OP_GET_AUTOSTART_LOGGING_MODE,  // 2
    OP_SET_AUTOSTART_LOGGING_MODE,  // 3
    OP_START_LOGGING,               // 4
    OP_RESUME_LOGGING,              // 6
    OP_PAUSE_LOGGING,               // 7
    OP_DEEP_START_LOGGING,
    OP_DEEP_STOP_LOGGING,
    OP_GET_RECYCLE_SIZE,
    OP_SET_RECYCLE_SIZE,
    OP_GET_LOGFILE_CAPACITY,
    OP_SET_LOGFILE_CAPACITY,
    OP_SET_DEBUG_LEVEL,
    OP_META_START_LOGGING,
    OP_META_STOP_LOGGING,
    OP_PULL_LOG_START,
    OP_PULL_LOG_STOP,
    OP_END
} OP;

bool sendCommandToAPK(const int msgid, const char *msgdata=NULL);

bool getReceiveStartCommand();
bool isEngineerBuild();
int getDebugLevel();
int executeCommand(int cmd, int parameter = 0);

#endif

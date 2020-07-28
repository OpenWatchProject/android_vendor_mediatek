/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "EMMODULES"
#define MTK_LOG_ENABLE 1
#include "Modules.h"
#include "RPCClient.h"
#include <cutils/log.h>
#include <sys/wait.h>

#include "ModuleMsdc.h"
#include "ModuleMdLogFilters.h"
#include "RscAssist.h"
static void exec_cmd(RPCClient* msgSender) {
#define ERRORINFO(msg) "#$ERROR^&_" #msg
#define RESULT_BUFFER_SIZE 1024
#define CMD_BUFFER_SIZE 1024
    static pthread_mutex_t exec_lock = PTHREAD_MUTEX_INITIALIZER;

    int paramCount = msgSender->ReadInt();
    ALOGD("paramCount:%d", paramCount);
    if (paramCount != 1) {
        msgSender->PostMsg(ERRORINFO(INVALID_PARAM_COUNT));
        return;
    }
    int type = msgSender->ReadInt();
    ALOGD("type:%d", type);
    if (type != PARAM_TYPE_STRING) {
        msgSender->PostMsg(ERRORINFO(INVALID_PARAM_TYPE));
        return;
    }
    int len = msgSender->ReadInt();
    if (len > CMD_BUFFER_SIZE - 1) {
        msgSender->PostMsg(ERRORINFO(INVALID_CMD_LEN_TOO_LARGE));
        return;
    }
    char cmd_buff[CMD_BUFFER_SIZE] = { 0 };
    int recv_count = 0;
    int ret = 0;
    do {
        ret = msgSender->Receive(cmd_buff + recv_count, len + 1 - recv_count);
        if (ret < 0) {
            ALOGD("Receive fail; error:%s", strerror(errno));
            msgSender->PostMsg(ERRORINFO(RECIEVE_FAIL));
            return;
        }
        recv_count += ret;
    } while (recv_count < len);
    ALOGD("exec_cmd:%s", cmd_buff);
    char result[RESULT_BUFFER_SIZE] = { 0 };
    FILE * fpipe = NULL;
    pthread_mutex_lock(&exec_lock);
    fpipe = popen(cmd_buff, "r");
    pthread_mutex_unlock(&exec_lock);
    if (fpipe == NULL) {
        ALOGE("exec_cmd popen fail, error:%s", strerror(errno));
        msgSender->PostMsg(ERRORINFO(FAIL_CALL_POPEN));
        return;
    }

    while (!feof(fpipe)) {
        ret = fread(result, 1, sizeof(result) - 1, fpipe);
        if (ret < 0) {
            ALOGD("fread error:%s", strerror(errno));
            msgSender->PostMsg(ERRORINFO(READ_CMD_FEEDBACK_FAIL));
            goto out;
        }
        result[ret] = 0;
        if (ret > 0) {
            msgSender->PostMsg(result);
        }
    }

    out: pthread_mutex_lock(&exec_lock);
    ret = pclose(fpipe);
    pthread_mutex_unlock(&exec_lock);
    if (ret != 0) {
        ALOGD("abnormal status code:%d, cmd exit code:%d, errno:%d, %s", ret, WEXITSTATUS(ret),
                errno, strerror(errno));
        msgSender->PostMsg(ERRORINFO(ABNORMAL_STATUS_CODE));
    }
}

AFMModules::AFMModules() {
}
AFMModules::~AFMModules() {
}

void AFMModules::Execute(int feature_id, RPCClient* msgSender) {
    switch (feature_id) {
    case FUNCTION_EM_MD_LOG_FILTER_GEN:
        ModuleMdLogFilters::getFilterPath(msgSender);
        break;
    case FUNCTION_EM_RSC_WRITE:
        RscAssist::switchProject(msgSender);
        break;
    case FUNCTION_EM_MSDC_SET_CURRENT:
        ModuleMsdc::setCurrent(msgSender);
        break;
    case FUNCTION_EM_MSDC_GET_CURRENT:
        ModuleMsdc::getCurrent(msgSender);
        break;
    case FUNCTION_EM_SHELL_CMD_EXECUTION:
        exec_cmd(msgSender);
        break;
    default:
        msgSender->PostMsg((char*) "This Feature Is Not Supported.");
        ALOGE("unsupported feature id = %d", feature_id);
        break;
    }

    return;
}

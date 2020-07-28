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

#define LOG_TAG "EMMDLOGFILTER"
#define MTK_LOG_ENABLE 1

#include <cutils/log.h>
#include "RPCClient.h"
#include "ModuleMdLogFilters.h"
#include "hardware/ccci_intf.h"

void ModuleMdLogFilters::getFilterPath(RPCClient* msgSender) {
    int paramCount = msgSender->ReadInt();
    ALOGD("paramCount:%d", paramCount);
    if (paramCount != 2) {
        msgSender->PostMsg(ERRORINFO(INVALID_PARAM_COUNT));
        return;
    }

    char keyword_buff[BUFFER_SIZE] = { 0 };
    extractString(msgSender, keyword_buff);
    ALOGD("keyword: %s", keyword_buff);
    char filterpath_buff[BUFFER_SIZE] = { 0 };
    extractString(msgSender, filterpath_buff);
    ALOGD("filterpath_buff: %s", filterpath_buff);
    int success = restore_image_from_pt(keyword_buff, filterpath_buff);
    char result[2] = { 0 };
    result[0] = (success >= 0) ? '1' : '0';
    msgSender->PostMsg(result);
    ALOGD("success is %d\n", success);
}

void ModuleMdLogFilters::extractString(RPCClient* msgSender, char* outstr) {
    int type = msgSender->ReadInt();
    ALOGD("type:%d", type);
    if (type != PARAM_TYPE_STRING) {
        return;
    }
    int len = msgSender->ReadInt();
    ALOGD("len:%d", len);
    if (len > BUFFER_SIZE - 1) {
        return;
    }
    int recv_count = 0;
    int ret = 0;
    do {
        ret = msgSender->Receive(outstr + recv_count, len - recv_count);
        if (ret < 0) {
            ALOGD("Receive fail; error:%s", strerror(errno));
            return;
        }
        recv_count += ret;
    } while (recv_count < len);
    ALOGD("recv_count %d", recv_count);
}


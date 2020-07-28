/*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
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
#include <hardware_legacy/power.h>
#include <telephony/ril_cdma_sms.h>
#include <telephony/mtk_ril.h>
#include <cutils/sockets.h>
#include <cutils/jstring.h>
#include <telephony/record_stream.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>
#include <pthread.h>
#include <cutils/jstring.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <sys/system_properties.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <sys/un.h>
#include <assert.h>
#include <netinet/in.h>
#include <cutils/properties.h>
#include <ril_service.h>
#include "libmtkrilutils.h"
#include "ril_internal.h"
#include <telephony/mtk_rilop.h>
#include <rilop_service.h>

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif

#undef LOG_TAG
#define LOG_TAG "RILC-OP"

namespace android {
#define NUM_ELEMS(a)     (sizeof (a) / sizeof (a)[0])

static CommandInfo s_mtk_op_commands[] = {
#include <telephony/mtk_ril_op_commands.h>
};

static UnsolResponseInfo s_mtk_op_unsolResponses[] = {
#include <telephony/mtk_ril_op_unsol_commands.h>
};

extern "C" android::CommandInfo *getOpCommandInfo(int request) {
    RLOGI("getOpCommandInfo: request %d", request);
    android::CommandInfo *pCI = NULL;

   for (int i = 0; i < (int)NUM_ELEMS(s_mtk_op_commands); i++) {
        if (request == s_mtk_op_commands[i].requestNumber) {
            pCI = &(s_mtk_op_commands[i]);
            break;
        }
    }
    if (pCI == NULL) {
        RLOGI("getOpCommandInfo: unsupported request %d", request);
    }
    return pCI;
}

extern "C" UnsolResponseInfo *getOpUnsolResponseInfo(int unsolResponse) {
    RLOGI("getOpUnsolResponseInfo: unsolResponse %d", unsolResponse);
    android::UnsolResponseInfo *pUnsolResponseInfo = NULL;

    for (int i = 0; i < (int)NUM_ELEMS(s_mtk_op_unsolResponses); i++) {
        if (unsolResponse == s_mtk_op_unsolResponses[i].requestNumber) {
            pUnsolResponseInfo = &(s_mtk_op_unsolResponses[i]);
            RLOGD("find mtk op unsol index %d for %d, waketype: %d", i, unsolResponse, pUnsolResponseInfo->wakeType);
            break;
        }
    }

    if (pUnsolResponseInfo == NULL) {
        RLOGD("Can not find mtk op unsol index %d", unsolResponse);
    }
    return pUnsolResponseInfo;
}

}

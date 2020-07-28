/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2018
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

/****************************************************************************/
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cutils/properties.h>
#include <cutils/log.h>
#include <strings.h>
#include <semaphore.h>


#define C_INVALID_PID  (-1)   /*invalid process id*/
#define C_INVALID_TID  (-1)   /*invalid thread id*/
#define C_INVALID_FD   (-1)   /*invalid file handle*/
#define C_INVALID_SOCKET (-1)  /*invalid socket id*/

//// HIDL implement
#define LOG_HIDL_INTERFACE        "mtk_lbs_log_v2s"
#include <hidl/LegacySupport.h>
#include <vendor/mediatek/hardware/lbs/1.0/ILbs.h>
#include <vendor/mediatek/hardware/lbs/1.0/ILbsCallback.h>
#include "lbs_dbg.h"
#include "LbsLogInterface.h"


using ::android::sp;
using ::android::wp;
using ::android::status_t;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::Return;
using ::android::hidl::base::V1_0::IBase;
using vendor::mediatek::hardware::lbs::V1_0::ILbs;
using vendor::mediatek::hardware::lbs::V1_0::ILbsCallback;
struct LbsDeathRecipient : virtual public hidl_death_recipient
{
    // hidl_death_recipient interface
    virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override {
      // TODO(gomo): implement a better death recovery mechanism without
      // crashing system server process as described in go//treble-gnss-death
      LOGE("Abort due to ILBS hidl service failure,%lu",sizeof(who)); //sizeof(who) used to solve build warning
      UNUSED(cookie);
    }
};
struct LbslogCallback : public ILbsCallback {
    Return<bool> callbackToClient(const hidl_vec<uint8_t>& data) override;
};

sp<ILbs> lbslogInterface = nullptr;
sp<LbsDeathRecipient> lbsHalDeathRecipient = nullptr;
sp<ILbsCallback> lbslogCbIface = new LbslogCallback();

#ifdef  __cplusplus
extern "C" {
#endif
extern int gps_debug_logd_init();
extern const char* LbsLogInterface_LogCategory_to_string(LbsLogInterface_LogCategory data);
extern bool Lbs_openLog(const char* dist, char* filepath);
extern bool Lbs_writeLog(const char* dist, char* log, int log_size);
extern bool Lbs_closeLog(const char* dist);
#ifdef __cplusplus
} /* end of extern "C" */
#endif


sem_t g_mnld_exit_sem;
const char *__version__ = "0.01";

/// Mnld2Gpslog inerface callback handler
void LbsLogInterface_openLog_hdlr(LbsLogInterface_LogCategory type, char* filePath) {
    if (!Lbs_openLog(LbsLogInterface_LogCategory_to_string(type), filePath)) {
       LOGE("LbsLogInterface_openLog_hdlr error dist = %s, filePath= %s, error:%s", LbsLogInterface_LogCategory_to_string(type), filePath, strerror(errno));
    }
}

void LbsLogInterface_closeLog_hdlr(LbsLogInterface_LogCategory type) {
    if (!Lbs_closeLog(LbsLogInterface_LogCategory_to_string(type))) {
       LOGE("LbsLogInterface_closeLog_hdlr error dist = %s, error:%s", LbsLogInterface_LogCategory_to_string(type), strerror(errno));
    }
}

void LbsLogInterface_writeLog_hdlr(LbsLogInterface_LogCategory type, char log[], int log_size) {
    if (!Lbs_writeLog(LbsLogInterface_LogCategory_to_string(type), log, log_size)) {
       LOGE("LbsLogInterface_writeLog_hdlr error dist = %s, log_size= %d, error:%s", LbsLogInterface_LogCategory_to_string(type), log_size, strerror(errno));
    }
}

LbsLogInterface_callbacks g_LbsLogInterface_callbacks = {
    LbsLogInterface_openLog_hdlr,
    LbsLogInterface_closeLog_hdlr,
    LbsLogInterface_writeLog_hdlr
};

Return<bool> LbslogCallback::callbackToClient(const hidl_vec<uint8_t>& data) {
   char* cdata = (char*) &(data[0]);
   return LbsLogInterface_receiver_decode(cdata, &g_LbsLogInterface_callbacks);
}

int block_here() {
    if (sem_init(&g_mnld_exit_sem, 0, 0) == -1) {
        LOGE("block_here() sem_init failure reason=%s\n", strerror(errno));
        return -1;
    }
    sem_wait(&g_mnld_exit_sem);
    if (sem_destroy(&g_mnld_exit_sem) == -1) {
        LOGE("block_here() sem_destroy reason=%s\n", strerror(errno));
    }
    return 0;
}

void hidl_set_lbs_log_callback(void) {
    if (lbslogInterface == nullptr) {
        lbslogInterface = ILbs::getService(LOG_HIDL_INTERFACE);

        if (lbslogInterface != nullptr) {
          lbsHalDeathRecipient = new LbsDeathRecipient();
          android::hardware::Return<bool> linked = (android::hardware::Return<bool>)lbslogInterface->linkToDeath(
              lbsHalDeathRecipient, /*cookie*/ 0);
            if (!linked.isOk()) {
                LOGD("Transaction error in linking to LbsHAL death: %s",
                        linked.description().c_str());
            } else if (!linked) {
                LOGD("Unable to link to LbsHal death notifications");
            } else {
                LOGD("Link to death notification successful");
            }
        }
    }
    if (lbslogInterface != nullptr) {
        lbslogInterface->setCallback(lbslogCbIface);
    }

}

int main(void) {
    LOGD("LBS debug daemon begin running, Version=%s", __version__);
    if (gps_debug_logd_init() != 0) {
        LOGE("gps_debug_logd_init error, exit");
        return -1;
    }
    hidl_set_lbs_log_callback();

    block_here();
    return 0;
}



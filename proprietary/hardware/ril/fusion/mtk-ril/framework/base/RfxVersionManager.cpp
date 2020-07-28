/* Copyright Statement:
 *
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

#include "RfxVersionManager.h"
#include <semaphore.h>
#include "utils/Mutex.h"
#include "RfxLog.h"
#include "RfxRilUtils.h"

using ::android::Mutex;

#define RFX_LOG_TAG "RfxVersionMgr"

static sem_t sWaitLooperSem;
static bool sNeedWaitLooper = true;
static Mutex sWaitLooperMutex;

RfxVersionManager* RfxVersionManager::sSelf = NULL;

void RfxVersionManager::waitVersion() {
    sWaitLooperMutex.lock();
    if (sNeedWaitLooper) {
        RFX_LOG_D(RFX_LOG_TAG, "waitLooper() begin");
        sem_wait(&sWaitLooperSem);
        sNeedWaitLooper = false;
        sem_destroy(&sWaitLooperSem);
        RFX_LOG_D(RFX_LOG_TAG, "waitLooper() end");
    }
    sWaitLooperMutex.unlock();
}

RfxVersionManager* RfxVersionManager::init() {
    if (sSelf == NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "init");
        sSelf = new RfxVersionManager();
        sem_init(&sWaitLooperSem, 0, 0);
    }
    return sSelf;
}

RfxVersionManager* RfxVersionManager::getInstance() {
    return sSelf;
}

void RfxVersionManager::initVersion(RfxAtLine *line) {
    int err = 0;
    char *feature = NULL;
    int version = 0;

    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "initVersion error: %d!", err);
        return;
    }


    if (RfxRilUtils::getRilRunMode() == RIL_RUN_MODE_MOCK) {
        RFX_LOG_E(RFX_LOG_TAG, "initVersion ignored in UT test");
        return;
    }

    while(1) {
        line->atTokStart(&err);
        feature = line->atTokNextstr(&err);
        version = line->atTokNextint(&err);
        FeatureVersion fv (String8(feature), version);
        mVersionList.add(fv);
        if (NULL == line->getNext()) {
            break;
        }
        line = line->getNext();
    }

    // debug
    for (size_t i = 0; i < mVersionList.size(); i++) {
        FeatureVersion fv = mVersionList.itemAt(i);
        RFX_LOG_D(RFX_LOG_TAG, "[%zu] feature: [%s], version: [%d]",
                i, fv.getFeature().string(), fv.getVersion());
    }

    // finish to update all feature version
    sem_post(&sWaitLooperSem);
}

int RfxVersionManager::getFeatureVersion(char *feature) {
    waitVersion();
    return getFeatureVersion(feature, 0);
}

int RfxVersionManager::getFeatureVersion(char *feature, int defaultVaule) {
    if (RfxRilUtils::getRilRunMode() == RIL_RUN_MODE_MOCK) {
        RFX_LOG_E(RFX_LOG_TAG, "return default version on UT");
        return defaultVaule;
    }
    waitVersion();
    for (size_t i = 0; i < mVersionList.size(); i++) {
        FeatureVersion fv = mVersionList.itemAt(i);
        if (fv.getFeature() == String8(feature)) {
            RFX_LOG_D(RFX_LOG_TAG, "find version: %d", fv.getVersion());
            return fv.getVersion();
        }
    }
    return defaultVaule;
}

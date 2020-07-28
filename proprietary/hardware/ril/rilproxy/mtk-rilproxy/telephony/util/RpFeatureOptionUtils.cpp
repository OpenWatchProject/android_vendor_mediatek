 /*
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RpFeatureOptionUtils.h"

#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>
#include <log/log.h>
#include "ratconfig.h"

#define RFX_LOG_TAG "RpFOUtils"

int RpFeatureOptionUtils::mMultiSIMConfig = -1;

int RpFeatureOptionUtils::mIsC2kSupport = -1;
int RpFeatureOptionUtils::mC2kLteMode = -1;

int RpFeatureOptionUtils::mIsMultiIms = -1;

/// M: add for op09 volte setting @{
int RpFeatureOptionUtils::mIsOp09 = -1;
int RpFeatureOptionUtils::mIsCtVolteSupport = -1;
/// @}
int RpFeatureOptionUtils::mMdVersion = -1;

enum MultiSIMConfig {
    MSIM_MODE_SS,
    MSIM_MODE_DSDS,
    MSIM_MODE_DSDA,
    MSIM_MODE_TSTS,
    MSIM_MODE_QSQS,
    MSIM_MODE_NUM
};
void RpFeatureOptionUtils::readMultiSIMConfig(void) {
    if (RpFeatureOptionUtils::mMultiSIMConfig == -1) {
        char property_value[PROPERTY_VALUE_MAX];
        memset(property_value, 0, sizeof(property_value));
        property_get("persist.radio.multisim.config", property_value, NULL);
        if (strcmp(property_value, "ss") == 0) {
            RpFeatureOptionUtils::mMultiSIMConfig = MSIM_MODE_SS;
        } else if (strcmp(property_value, "dsds") == 0) {
            RpFeatureOptionUtils::mMultiSIMConfig = MSIM_MODE_DSDS;
        } else if (strcmp(property_value, "dsda") == 0) {
            RpFeatureOptionUtils::mMultiSIMConfig = MSIM_MODE_DSDA;
        } else if (strcmp(property_value, "tsts") == 0) {
            RpFeatureOptionUtils::mMultiSIMConfig = MSIM_MODE_TSTS;
        } else if (strcmp(property_value, "qsqs") == 0) {
            RpFeatureOptionUtils::mMultiSIMConfig = MSIM_MODE_QSQS;
        } else {
            RLOGE("unknown msim mode property (%s)", property_value);
        }
        RLOGD("msim config: %d", RpFeatureOptionUtils::mMultiSIMConfig);
    }
}

int RpFeatureOptionUtils::getSimCount() {
    int count = 2;
    RpFeatureOptionUtils::readMultiSIMConfig();
    if (RpFeatureOptionUtils::mMultiSIMConfig == -1) {
        RLOGE("getSimCount fail. default return 2. (unknown msim config: %d)", RpFeatureOptionUtils::mMultiSIMConfig);
        count = 2;
    } else if (RpFeatureOptionUtils::mMultiSIMConfig == MSIM_MODE_SS) {
        count = 1;
    } else if (RpFeatureOptionUtils::mMultiSIMConfig == MSIM_MODE_DSDS
            || RpFeatureOptionUtils::mMultiSIMConfig == MSIM_MODE_DSDA) {
        count = 2;
    } else if (RpFeatureOptionUtils::mMultiSIMConfig == MSIM_MODE_TSTS) {
        count = 3;
    } else if (RpFeatureOptionUtils::mMultiSIMConfig == MSIM_MODE_QSQS) {
        count = 4;
    }
    return count;
}

int RpFeatureOptionUtils::isC2kSupport(void) {
    if (RpFeatureOptionUtils::mIsC2kSupport == -1) {
        RpFeatureOptionUtils::mIsC2kSupport = RatConfig_isC2kSupported();
    }
    return RpFeatureOptionUtils::mIsC2kSupport;
}

bool RpFeatureOptionUtils::isSvlteSupport() {
    if (RpFeatureOptionUtils::mC2kLteMode == -1) {
        char property_value[PROPERTY_VALUE_MAX] = { 0 };
        property_get("ro.vendor.mtk_c2k_lte_mode", property_value, "0");
        RpFeatureOptionUtils::mC2kLteMode = atoi(property_value);
    }
    return (RpFeatureOptionUtils::mC2kLteMode == 1);
}

int RpFeatureOptionUtils::isMultipleImsSupport() {
    if (mIsMultiIms == -1) {
        char property_value[PROPERTY_VALUE_MAX] = { 0 };
        property_get("persist.vendor.mims_support", property_value, "0");
        mIsMultiIms = atoi(property_value);
    }
    return (mIsMultiIms > 1) ? 1 : 0;
}

/// M: add for op09 volte setting @{
bool RpFeatureOptionUtils::isOp09() {
    if (mIsOp09 == -1) {
        char optrstr[PROPERTY_VALUE_MAX] = { 0 };
        property_get("persist.vendor.operator.optr", optrstr, "");
        if (strncmp(optrstr, "OP09", 4) == 0) {
            mIsOp09 = 1;
        } else {
            mIsOp09 = 0;
        }
    }
    return (mIsOp09 == 1);
}

bool RpFeatureOptionUtils::isCtVolteSupport() {
    if (mIsCtVolteSupport == -1) {
        char ctstr[PROPERTY_VALUE_MAX] = { 0 };
        property_get("persist.vendor.mtk_ct_volte_support", ctstr, "");
        if (strcmp(ctstr, "1") == 0) {
            mIsCtVolteSupport = 1;
        } else {
            mIsCtVolteSupport = 0;
        }
    }
    return (mIsCtVolteSupport == 1);
}
/// @}

int RpFeatureOptionUtils::getMdVersion() {
    if (mMdVersion == -1) {
        char property_value[PROPERTY_VALUE_MAX] = {0};
        property_get("vendor.ril.md.version", property_value, "0");
        mMdVersion = atoi(property_value);
    }
    return mMdVersion;
}

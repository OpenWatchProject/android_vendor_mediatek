/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2019. All rights reserved.
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
* have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
* applicable license agreements with MediaTek Inc.
*/

package com.mediatek.internal.telephony;

import android.os.SystemProperties;
import android.telephony.Rlog;

import com.android.internal.telephony.DeviceStateMonitor;
import com.android.internal.telephony.Phone;

/**
 * MtkDeviceStateMonitor handle LOW_DATA_EXPECTED event based on some system properties.
 */

public class MtkDeviceStateMonitor extends DeviceStateMonitor {
    private static final boolean DBG = true;
    private static final String LOG_TAG = "MtkDeviceStateMonitor";

    private static final String PROPERTY_FD_ON_CHARGE = "persist.vendor.fd.on.charge";
    private static final String PROPERTY_FD_SCREEN_OFF_ONLY = "persist.vendor.fd.screen.off.only";
    private static final String PROPERTY_RIL_FD_MODE = "vendor.ril.fd.mode";

    private static final boolean MTK_FD_SUPPORT = (Integer.parseInt(SystemProperties
            .get("ro.vendor.mtk_fd_support", "0")) == 1 ? true : false);

    public MtkDeviceStateMonitor(Phone phone) {
        super(phone);
        logd("Initialize MtkDeviceStateMonitor");
        mIsLowDataExpected = isLowDataExpected();
    }

    @Override
    protected boolean isLowDataExpected() {
        if (isFdAllowed()) {
            logd("isLowDataExpected mIsScreenOn = " + mIsScreenOn
                    + " mIsCharging = " + mIsCharging
                    + " mIsTetheringOn = " + mIsTetheringOn);
            return (!(isFdEnabledOnlyWhenScreenOff() && mIsScreenOn)
                    && !(mIsCharging && !isFdEnabledWhenCharging())
                    && !mIsTetheringOn);
        }

        return false;
    }

    private boolean isFdAllowed() {
        if (MTK_FD_SUPPORT) {
            int fdMode = Integer.parseInt(SystemProperties.get(PROPERTY_RIL_FD_MODE, "0"));
            return (fdMode == 1);
        }

        return false;
    }

    private static boolean isFdEnabledWhenCharging() {
        return (SystemProperties.getInt(PROPERTY_FD_ON_CHARGE, 0) == 1);
    }

    private static boolean isFdEnabledOnlyWhenScreenOff() {
        return (SystemProperties.getInt(PROPERTY_FD_SCREEN_OFF_ONLY, 0) == 1);
    }

    private void logd(String s) {
        Rlog.d(LOG_TAG, "[phoneId" + mPhone.getPhoneId() + "]" + s);
    }
}

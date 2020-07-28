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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony.datasub;

import android.content.Context;
import android.content.Intent;

import android.telephony.PreciseCallState;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import android.text.TextUtils;

import com.mediatek.internal.telephony.dataconnection.MtkDcHelper;
import com.mediatek.internal.telephony.datasub.SmartDataSwitchAssistant;
import com.mediatek.internal.telephony.MtkSubscriptionManager;

public class SmartDataSwitchAssistantOpExt implements ISmartDataSwitchAssistantOpExt {
    private static boolean DBG = true;
    private static String LOG_TAG = "SmartDataSwitchOpExt";
    private static Context mContext = null;

    private static SmartDataSwitchAssistant mSmartData = null;

    protected int mVoiceNetworkType = ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;

    public SmartDataSwitchAssistantOpExt(Context context) {
        mContext = context;
    }

    @Override
    public void init(SmartDataSwitchAssistant smartDataSwitchAssistant) {
        logd("init()");
        mSmartData = smartDataSwitchAssistant;
    }

    @Override
    public void onCallStarted() {
        // register event
        mSmartData.regServiceStateChangedEvent();
    }

    @Override
    public void onCallEnded() {
        // de-register event
        mSmartData.unregServiceStateChangedEvent();
    }

    @Override
    public void onSubChanged() {}

    @Override
    public void onTemporaryDataSettingsChanged() {}

    @Override
    public void onSrvccStateChanged() {}

    @Override
    public boolean onServiceStateChanged(int phoneId) {
        if (phoneId != mSmartData.getInCallPhoneId()) {
            return false;
        }
        int voiceNwType = mSmartData.getVoiceNetworkType(phoneId);
        if (isNetworkTypeChanged(voiceNwType)) {
            return true;
        }
        return false;
    }

    @Override
    public void onHandoverToWifi() {}

    @Override
    public void onHandoverToCellular() {}

    @Override
    public boolean preCheckByCallStateExt(Intent intent, boolean result) {
        return result;
    }

    @Override
    public boolean isNeedSwitchCallType(int callType) {
        return false;
    }

    @Override
    public boolean isSmartDataSwtichAllowed() {
        return true;
    }

    @Override
    public boolean checkIsSwitchAvailable(int phoneId) {
        // M: check UI settings first
        if (!mSmartData.getTemporaryDataSettings()) {
            logd("checkIsSwitchAvailable() settings is off, not passed");
            return false;
        }

        // M: check if Dsda mode. Disable Temp data switch in Dsda mode.
        MtkDcHelper dcHelper = MtkDcHelper.getInstance();
        if (dcHelper != null) {
            if(dcHelper.getDsdaMode() == 1) {
                logd("checkIsSwitchAvailable(): Dsda mode, not passed");
                return false;
            }
        }

        int nwType = mSmartData.getVoiceNetworkType(phoneId);
        boolean isWifiCalling = mSmartData.isWifcCalling(phoneId);
        logd("checkIsSwitchAvailable() nwType=" + nwType
                + ", isCdma=" + ServiceState.isCdma(nwType)
                + ", isWifcCalling=" + isWifiCalling);
        if (ServiceState.isCdma(nwType) ||
                nwType == ServiceState.RIL_RADIO_TECHNOLOGY_GSM ||
                isWifiCalling){
            logd("checkIsSwitchAvailable(): not passed");
            return false;
        }
        logd("checkIsSwitchAvailable(): passed");
        return true;
    }

    private boolean isNetworkTypeChanged(int newVoiceNwType) {
        boolean result = false;

        logd("isNetworkTypeChanged: mVoiceNetworkType=" + mVoiceNetworkType +
                " newVoiceNwType=" + newVoiceNwType);

        if (mVoiceNetworkType != newVoiceNwType) {
            mVoiceNetworkType = newVoiceNwType;
            result = true;
        }
        return result;
    }

    protected static void logv(String s) {
        if (DBG) {
            Rlog.v(LOG_TAG, s);
        }
    }

    protected static void logd(String s) {
        if (DBG) {
            Rlog.d(LOG_TAG, s);
        }
    }

    protected static void loge(String s) {
        if (DBG) {
            Rlog.e(LOG_TAG, s);
        }
    }

    protected static void logi(String s) {
        if (DBG) {
            Rlog.i(LOG_TAG, s);
        }
    }
}

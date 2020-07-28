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
package com.mediatek.op08.settings;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;

import com.mediatek.settings.ext.DefaultSettingsMiscExt;


/**
 * For settings small feature misc.
 */
public class Op08SettingsMiscExt extends DefaultSettingsMiscExt {

    private static final String TAG = "Op08SettingsMiscExt";


    private Context mContext;

    /**
     * Constructor method.
     * @param context Settings's context
     */
    public Op08SettingsMiscExt(Context context) {
        super(context);
        mContext = context;
        Log.d(TAG, "SettingsMiscExt this=" + this);
    }

    /**
     * Returns if wifi only mode is set.
     * @return boolean
     */
    @Override
    public boolean isWifiOnlyModeSet() {
        Log.i(TAG, "isWifiOnlyModeSet");
        int wfcMode = ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED;
        try {
            int subId = SubscriptionManager.getDefaultDataSubscriptionId();
            ImsManager imsManager = ImsManager.getInstance(
                        mContext, SubscriptionManager.getPhoneId(subId));
            wfcMode = imsManager.getWfcMode(mContext);
            boolean wfcEnabled = imsManager.isWfcEnabledByUser(mContext);
            Log.d(TAG, "wfcMode = " + wfcMode + "wfcEnabled: " + wfcEnabled);
            if (wfcMode == ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY && wfcEnabled) {
                Log.d(TAG, "Disable Cellular Settings");
                return true;
            }
        } catch (Exception ex) {
            Log.d(TAG, "Fail to get wfcMode");
        }
        return false;
    }
}

/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.settings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.phone.ext.ExtensionManager;

/**
 * Plugin implementation for OP08 Roaming condition.
 */

public class Op08ServiceStateReceiver extends BroadcastReceiver {

    private Context mContext;
    private static final String TAG = "Op08ServiceStateReceiver";
    private boolean mDisplayed = false;
    public static final String DIALOG_DISPLAYED = "dialogueDisplayed";
    public static String mMCC = "";
    private TelephonyManager mTelephonyManager;
    @Override
    public void onReceive(final Context context, final Intent intent) {
        mContext = context;
        final String action = intent.getAction();
        if (TelephonyIntents.ACTION_SERVICE_STATE_CHANGED.equals(action)) {
            handleServiceStateChanged(intent);
        } else if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            Log.i(TAG, "ACTION_BOOT_COMPLETED");
            Settings.Global.putInt(mContext.getContentResolver(), DIALOG_DISPLAYED, 0);
        }
    }

    private void handleServiceStateChanged(Intent intent) {
        boolean isTMOLoad = ExtensionManager.getSimDialogExt().isNeedShowRoamingDialog();
        Log.v(TAG, "handleServiceStateChange. isTMOLoad = " + isTMOLoad);
        if (!isTMOLoad) {
            return;
        }
        Bundle extras = intent.getExtras();
        mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        if (extras != null) {
            ServiceState serviceState = ServiceState.newFromBundle(extras);
            Log.v(TAG, "ServiceState: " + serviceState.getState()
                    + " serviceState.getVoiceRoamingType() = "
                    + serviceState.getVoiceRoamingType()
                    + " serviceState.getDataRoamingType() = "
                    + serviceState.getDataRoamingType());
            /*
             * for TMO requirement
             * 1. INTERNATIONAL roaming show
             * 2. INTERNATIONAL country --- > lost signal --- > same INTERNATIONAL country, do not show
             * 3. INTERNATIONAL country --- > lost signal --- > other INTERNATIONAL country, show
             */
            if (serviceState.getState() == ServiceState.STATE_IN_SERVICE  &&
                    (serviceState.getVoiceRoamingType() == ServiceState.ROAMING_TYPE_INTERNATIONAL ||
                    serviceState.getDataRoamingType() == ServiceState.ROAMING_TYPE_INTERNATIONAL)) {
                String mccmnc = serviceState.getOperatorNumeric();
                String mcc = mccmnc.substring(0, 3);
                Log.i(TAG, "handleServiceStateChange mccmnc = " + mccmnc
                        + " mcc = " + mcc
                        + " mMCC = " + mMCC);
                if (null != mcc && mcc.equals(mMCC)) {
                    return;
                }

                try {
                    int roamingOn = Settings.Global.getInt(mContext.getContentResolver(),
                            Settings.Global.DATA_ROAMING);
                    Log.v(TAG, "roamingOn: " + roamingOn);
                    int displayed = Settings.Global.getInt(mContext.getContentResolver(),
                            DIALOG_DISPLAYED, 0);
                    mDisplayed = (displayed == 1) ;
                    if (mTelephonyManager.isDataRoamingEnabled() && !mDisplayed) {
                        Settings.Global.putInt(mContext.getContentResolver(), DIALOG_DISPLAYED, 1);
                        Log.i(TAG, "show out of home dialog and set mMCC = mcc");
                        mMCC = mcc;
                        showOutOfHomeDialog();
                    }
                } catch (Exception e) {
                    Log.e(TAG, "No phone object: ");
                }
            } else if (serviceState.getState() == ServiceState.STATE_IN_SERVICE
                && (serviceState.getVoiceRoamingType() == ServiceState.ROAMING_TYPE_NOT_ROAMING ||
                serviceState.getDataRoamingType() == ServiceState.ROAMING_TYPE_NOT_ROAMING)) {
                Log.i(TAG, "handleServiceStateChange in home, reset mMCC");
                mMCC = "";
            }
        }
    }

    private void showOutOfHomeDialog() {
        Intent newIntent = new Intent(mContext, Op08OutOfHomeDialog.class);
        newIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(newIntent);
    }
}

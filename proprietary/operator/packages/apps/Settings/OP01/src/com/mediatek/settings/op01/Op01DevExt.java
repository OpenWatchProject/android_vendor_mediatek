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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.settings.op01;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemProperties;
import android.util.Log;
import androidx.preference.Preference;

import com.mediatek.settings.ext.DefaultDevExt;

public class Op01DevExt extends DefaultDevExt {

    private static final String INTENT_USB_ACTIVATION = "com.mediatek.cba.USB_ACTIVATION";
    private static final String PROPERTY_FEATURE_SUPPORT = "ro.vendor.mtk_usb_cba_support";
    private static final String PROPERTY_USB_ACTIVATION = "persist.vendor.sys.usb.activation";

    private static final String TAG = "UsbChecker.UsbCheckerExt";

    private Preference mPreference;
    private EventReceiver mReceiver;
    private Context mContext;

    private class EventReceiver extends BroadcastReceiver {

        public void initialize() {
            Log.d(TAG, "EventReceiver initialize()");
            IntentFilter filter = new IntentFilter();
            filter.addAction(INTENT_USB_ACTIVATION);
            mContext.registerReceiver(this, filter);
        }

        public void destroy() {
            Log.d(TAG, "EventReceiver destroy()");
            mContext.unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "onReceive():" + intent.getAction());

            String action = intent.getAction();
            if (action == null) {
                return;
            }

            if (action.equals(INTENT_USB_ACTIVATION)) {
                if (mPreference != null) {
                    updateUI();
                }
            }
        }
    }

    public Op01DevExt(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public void customUSBPreference(Preference pref) {
        if (!isFeatureSupported()) {
            return;
        }

        Log.d(TAG, "customUSBPreference()");
        if (null != pref) {
            if (pref instanceof Preference) {
                mPreference = pref;
                updateUI();
            }
        }
    }

    private void updateUI() {
        boolean enable = true;
        if(!getActivateState()) {
            enable = false;
            if (mReceiver == null) {
                mReceiver = new EventReceiver();
                mReceiver.initialize();
            }
        } else {
            if (mReceiver != null) {
                mReceiver.destroy();
                mReceiver = null;
            }
        }
        Log.d(TAG, "updateUI() enable state = " + enable);
        mPreference.setEnabled(enable);
    }

    /**
     * PROPERTY_USB_ACTIVATION: whether has sim when plugged in with USB cable before.
     * Property will be reset if factory reset or reinstall system.
     * @return yes - has sim once; no - never has
     */
    private boolean getActivateState() {
        String activate = SystemProperties.get(PROPERTY_USB_ACTIVATION, "no");
        Log.d(TAG, "getActivateState=" + activate);
        if (activate.equals("yes")) {
            return true;
        } else {
            return false;
        }
    }

    private boolean isFeatureSupported() {
        int property = SystemProperties.getInt(PROPERTY_FEATURE_SUPPORT, -1);
        if (property != 1) {
            Log.i(TAG, "[isFeatureSupported] not support");
            return false;
        }
        return true;
    }
}

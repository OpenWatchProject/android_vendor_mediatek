/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.op18.systemui;

import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;

import com.android.internal.telephony.RILConstants;

import com.mediatek.internal.telephony.MtkPhoneConstants;



/** Platform implementation of the WFC controller. **/
public class WifiCallingController {
    private static final String TAG = "OP18WifiCallingController";
    private final Context mContext;
    private static WifiCallingController sWifiCallingController = null;
    private int mCallType = RILConstants.NO_PHONE;

    /**
     * Constructor.
     * @param context A Context object
     */
    private WifiCallingController(Context context) {
        mContext = context;
        Log.d(TAG, "new WifiCallingController()");
    }

    /**
         * Provides instance of WifiCallingController.
         * @param context A Context object
         * @return WifiCallingController
         */
    public static WifiCallingController getInstance(Context context) {
        if (sWifiCallingController == null) {
            sWifiCallingController = new WifiCallingController(context);
        }
        return sWifiCallingController;
    }

    /**
         * Tells whether wifi calling is enabled by user or not.
         * @return WifiCallingController
         */
    public boolean isWifiCallingOn() {
        return ImsManager.isWfcEnabledByUser(mContext);
    }

    /**
         * Sets WifiCalling new status via ImsManager.
         * @param enabled user's choice
         * @return
         */
    public void setWifiCallingSetting(boolean enabled) {
        ImsManager.setWfcSetting(mContext, enabled);
    }

    /**
         * Whether WifiCalling Tile is clickable or not.
         * @param context context
         * @return boolean
         */
    public boolean isClickable(Context context) {
        Log.d(TAG, "call_state: " + mCallType);
        return mCallType == RILConstants.IMS_PHONE ? false : true;
    }

    /**
         * Sets ongoing call type.
         * @param intent call intent
         * @return
         */
    public void setCallType(Intent intent) {
        String state = intent.getStringExtra(TelephonyManager.EXTRA_STATE);
        int phoneType = intent.getIntExtra(MtkPhoneConstants.PHONE_TYPE_KEY, RILConstants.NO_PHONE);
        Log.d(TAG, "phone state:" + state);
        Log.d(TAG, "phone type:" + phoneType);
        // TODO: check if checking is needed or we can directly store the phonetype received
        if (TelephonyManager.EXTRA_STATE_OFFHOOK.equals(state)
                || TelephonyManager.EXTRA_STATE_RINGING.equals(state)) {
            if (phoneType == RILConstants.IMS_PHONE) {
                mCallType = RILConstants.IMS_PHONE;
            } else {
                mCallType = RILConstants.GSM_PHONE;
            }
        } else {
            mCallType = RILConstants.NO_PHONE;
        }
    }
}


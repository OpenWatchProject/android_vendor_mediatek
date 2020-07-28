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

package com.mediatek.lbs;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.settings.FeatureOption;
import com.mediatek.lbs.em2.utils.AgpsInterface;
import com.mediatek.lbs.em2.utils.SuplProfile;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

public class LbsReceiver extends BroadcastReceiver {

    private static final String TAG = "LbsReceiver";
    private static final String PREFERENCE_FILE = "com.android.settings_preferences";

    private static final String ACTION_OMA_CP = "com.mediatek.omacp.settings";
    private static final String ACTION_OMA_CP_FEEDBACK = "com.mediatek.omacp.settings.result";
    private static final String ACTION_OMA_CP_CAPABILITY = "com.mediatek.omacp.capability";
    private static final String ACTION_OMA_CP_CAPABILITY_FEEDBACK
                                   = "com.mediatek.omacp.capability.result";
    private static final String APP_ID = "ap0004";

    private static final String EXTRA_APP_ID = "appId";
    private static final String EXTRA_SUPL = "supl";
    private static final String EXTRA_RESULT = "result";
    private static final String EXTRA_SUPL_PROVIDER_ID = "supl_provider_id";
    private static final String EXTRA_SUPL_SEVER_NAME = "supl_server_name";
    private static final String EXTRA_SUPL_SEVER_ADDRESS = "supl_server_addr";
    private static final String EXTRA_SUPL_SEVER_ADDRESS_TYPE = "supl_addr_type";
    private static final String EXTRA_SUPL_TO_NAPID = "supl_to_napid";

    private static final String EM_ENABLE_KEY = "EM_Indication";
    private static final String UNKNOWN_VALUE = "UNKNOWN_VALUE";
    private static final String KEY_SUB_ID = "subId";

    // Add for OMACP
    private static final String ACTION_OMA_UP_FEEDBACK = "com.mediatek.omacp.settings.result";
    private static final String AGPS_OMACP_PROFILE_UPDATE = "com.mediatek.agps.OMACP_UPDATED";
    private static final int SLP_PORT = 7275;
    private static final int SLP_TTL = 1;
    private static final int SLP_SHOW_TYPE = 2;
    private static final int Phone_DUAL_SIM = 2;

    // Add for OMA Profile
    private static final String OMA_PROFILE_TAG = "omacp_profile";
    private static final String EXTRA_OMA_PROFILE_NAME = "name";
    private static final String EXTRA_OMA_PROFILE_ADDR = "addr";
    private static final String EXTRA_OMA_PROFILE_PORT = "port";
    private static final String EXTRA_OMA_PROFILE_TLS = "tls";
    private static final String EXTRA_OMA_PROFILE_CODE = "code";
    private static final String EXTRA_OMA_PROFILE_ADDR_TYPE = "addrType";
    private static final String EXTRA_OMA_PROFILE_ID = "providerId";
    private static final String EXTRA_OMA_PROFILE_APN = "defaultApn";
    private static final String EXTRA_OMA_PROFILE_CHANGED = "changed";

    private static final String EMPTY_CONTENT = "";

    private String mCurOperatorCode;
    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        String action = intent.getAction();
        Log.d(TAG, "Receive : " + action);
        if (FeatureOption.MTK_AGPS_APP && FeatureOption.MTK_GPS_SUPPORT) {
            // BroadcastReceiver will reset all of member after onReceive
            if (action.equals(AGPS_OMACP_PROFILE_UPDATE)) {
                handleAgpsOmaProfileUpdate(context, intent);
            } else if (action.equals(ACTION_OMA_CP)) {
                handleOmaCpSetting(context, intent);
            } else if (action.equals(ACTION_OMA_CP_CAPABILITY)) {
                handleOmaCpCapability(context, intent);
            }
        }
    }

    private void handleAgpsOmaProfileUpdate(Context context, Intent intent) {
        Bundle bundle = intent.getExtras();

        SharedPreferences prefs = context.getSharedPreferences(OMA_PROFILE_TAG,
                Context.MODE_PRIVATE);
        prefs.edit()
                .putString(EXTRA_OMA_PROFILE_NAME,
                        bundle.getString(EXTRA_OMA_PROFILE_NAME))
                .putString(EXTRA_OMA_PROFILE_ADDR,
                        bundle.getString(EXTRA_OMA_PROFILE_ADDR))
                .putInt(EXTRA_OMA_PROFILE_PORT, bundle.getInt(EXTRA_OMA_PROFILE_PORT))
                .putInt(EXTRA_OMA_PROFILE_TLS, bundle.getInt(EXTRA_OMA_PROFILE_TLS))
                .putString(EXTRA_OMA_PROFILE_CODE,
                        bundle.getString(EXTRA_OMA_PROFILE_CODE))
                .putString(EXTRA_OMA_PROFILE_ADDR_TYPE,
                        bundle.getString(EXTRA_OMA_PROFILE_ADDR_TYPE))
                .putString(EXTRA_OMA_PROFILE_ID, bundle.getString(EXTRA_OMA_PROFILE_ID))
                .putString(EXTRA_OMA_PROFILE_APN, bundle.getString(EXTRA_OMA_PROFILE_APN))
                .putBoolean(EXTRA_OMA_PROFILE_CHANGED, true).commit();
    }

    private void handleOmaCpSetting(Context context, Intent intent) {
        if (!FeatureOption.MTK_OMACP_SUPPORT) {
            Log.d(TAG, "handleOmaCpSetting, MTK OMACP NOT SUPPOR ");
            return;
        }
        String appId = intent.getStringExtra(EXTRA_APP_ID);
        if (appId == null || !appId.equals(APP_ID)) {
            Log.d(TAG, "get the OMA CP broadcast, but it's not for AGPS");
            return;
        }

        int subId = intent.getIntExtra(KEY_SUB_ID, PhoneConstants.SIM_ID_1);
        String providerId = intent.getStringExtra("PROVIDER-ID");
        String slpName = intent.getStringExtra("NAME");
        String defaultApn = EMPTY_CONTENT;
        String address = EMPTY_CONTENT;
        String addressType = EMPTY_CONTENT;
        String port = EMPTY_CONTENT;

        Bundle bundle = intent.getExtras();
        ArrayList<HashMap<String, String>> appAddrMapList =
              (ArrayList<HashMap<String, String>>) bundle.get("APPADDR");
        if (appAddrMapList != null && !appAddrMapList.isEmpty()) {
            HashMap<String, String> addrMap = appAddrMapList.get(0);
            if (addrMap != null) {
                address = addrMap.get("ADDR");
                addressType = addrMap.get("ADDRTYPE");
            }
        }
        if (address == null || address.equals(EMPTY_CONTENT)) {
            Log.d(TAG, "Invalid oma cp pushed supl address");
            dealWithOmaUpdataResult(false, "Invalide oma cp pushed supl address");
            return;
        }
        // provider ID
        @SuppressWarnings("unchecked")
        ArrayList<String> defaultApnList = (ArrayList<String>) bundle.get("TO-NAPID");
        if (defaultApnList != null && !defaultApnList.isEmpty()) {
            defaultApn = defaultApnList.get(0);
        }

        // initialize sim status.
        initSIMStatus(subId);

        // update value if exist.
        String profileCode = EMPTY_CONTENT;
        profileCode = mCurOperatorCode;
        if (profileCode == null || EMPTY_CONTENT.equals(profileCode)) {
            dealWithOmaUpdataResult(false, "invalid profile code.");
            return;
        }
        Intent mIntent = new Intent(AGPS_OMACP_PROFILE_UPDATE);
        mIntent.putExtra(EXTRA_OMA_PROFILE_CODE, profileCode);
        mIntent.putExtra(EXTRA_OMA_PROFILE_ADDR, address);
        try {
            AgpsInterface agpsInterface = new AgpsInterface();
            SuplProfile profile = agpsInterface.getAgpsConfig().curSuplProfile;
            profile.addr = address;

            if (providerId != null && !EMPTY_CONTENT.equals(providerId)) {
                mIntent.putExtra(EXTRA_OMA_PROFILE_ID, providerId);
                profile.providerId = providerId;
            }
            if (slpName != null && !EMPTY_CONTENT.equals(slpName)) {
                mIntent.putExtra(EXTRA_OMA_PROFILE_NAME, slpName);
                profile.name = slpName;
            }
            if (defaultApn != null && !EMPTY_CONTENT.equals(defaultApn)) {
                mIntent.putExtra(EXTRA_OMA_PROFILE_APN, defaultApn);
                profile.defaultApn = defaultApn;
            }
            if (addressType != null && !EMPTY_CONTENT.equals(addressType)) {
                mIntent.putExtra(EXTRA_OMA_PROFILE_ADDR_TYPE, addressType);
                profile.addressType = addressType;
            }

            // M: because the TTL port is Fixed and the message doesn't include
            // the information about port number, we fix it.
            mIntent.putExtra(EXTRA_OMA_PROFILE_PORT, SLP_PORT);
            profile.port = SLP_PORT;

            mIntent.putExtra(EXTRA_OMA_PROFILE_TLS, SLP_TTL);
            profile.tls = true;

            mContext.sendBroadcast(mIntent,"com.mediatek.permission.omacp.install");
            agpsInterface.setSuplProfile(profile);
        } catch (IOException e) {
            Log.d(TAG, "IOException happened when new AgpsInterface object");
        }
        dealWithOmaUpdataResult(true, "OMA CP update successfully finished");

    }

    private void handleOmaCpCapability(Context context, Intent intent) {
        if (!FeatureOption.MTK_OMACP_SUPPORT) {
            Log.d(TAG, "handleOmaCpCapability, MTK OMACP NOT SUPPOR ");
            return;
        }
        Intent it = new Intent();
        it.setAction(ACTION_OMA_CP_CAPABILITY_FEEDBACK);
        it.putExtra(EXTRA_APP_ID, APP_ID);
        it.putExtra(EXTRA_SUPL, true);
        it.putExtra(EXTRA_SUPL_PROVIDER_ID, false);
        it.putExtra(EXTRA_SUPL_SEVER_NAME, true);
        it.putExtra(EXTRA_SUPL_TO_NAPID, false);
        it.putExtra(EXTRA_SUPL_SEVER_ADDRESS, true);
        it.putExtra(EXTRA_SUPL_SEVER_ADDRESS_TYPE, false);

        Log.d(TAG, "Feedback OMA CP capability information");
        context.sendBroadcast(it,"com.mediatek.permission.omacp.install");
    }

    /* Get current mobile network status */
    private void initSIMStatus(int subId) {
        int simStatus = -1;
        mCurOperatorCode = EMPTY_CONTENT;
        TelephonyManager telMgr = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);
        if (TelephonyManager.getDefault().getPhoneCount()
            >= Phone_DUAL_SIM) {
            int slotId = SubscriptionManager.getSlotIndex(subId);
            simStatus = telMgr.getSimState(slotId);
            Log.d(TAG, "SubId : " + subId + " SlotId : " + slotId + " simStatus: " + simStatus);
            if (TelephonyManager.SIM_STATE_READY == simStatus) {
                mCurOperatorCode = telMgr.getSimOperator(subId);
            }
        } else {
            simStatus = telMgr.getSimState();
            if (TelephonyManager.SIM_STATE_READY == simStatus) {
                mCurOperatorCode = telMgr.getSimOperator();
            }
        }
        Log.d(TAG, "SubId : " + subId + " Status : " + simStatus
                + " OperatorCode : " + mCurOperatorCode);
    }

    /**
     * Notify the result of dealing with OMA CP broadcast
     * @param success
     * @param message
     */
    private void dealWithOmaUpdataResult(boolean success, String message) {
        Toast.makeText(mContext, "Deal with OMA CP operation : " + message,
                Toast.LENGTH_LONG).show();
        Log.d(TAG, "Deal with OMA UP operation : " + message);
        Intent it = new Intent();
        it.setAction(ACTION_OMA_UP_FEEDBACK);
        it.putExtra(EXTRA_APP_ID, APP_ID);
        it.putExtra(EXTRA_RESULT, success);

        mContext.sendBroadcast(it,"com.mediatek.permission.omacp.install");
    }
}

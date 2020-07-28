/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.settings.op01;

import android.content.Context;
import android.content.IntentFilter;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.Switch;
import android.widget.TextView;

import com.android.internal.telephony.TelephonyIntents;
import com.android.settingslib.CustomDialogPreferenceCompat;

import com.mediatek.ims.internal.MtkImsManagerEx;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.settings.ext.DefaultDataUsageSummaryExt;

import java.util.List;

/**
 * Data Usage summary info plugin.
 */
public class Op01DataUsageSummaryExt extends DefaultDataUsageSummaryExt {
    private static final String TAG = "Op01DataUsageSummaryExt";
    private static final String[] MCCMNC_TABLE_TYPE_CMCC = {
        "46000", "46002", "46004", "46007", "46008"};
    private static final int TYPE_TEMP_DATA_SERVICE_SUMMARY = 0;
    private static final int TYPE_TEMP_DATA_SERVICE_MESSAGE = 1;

    private Context mContext;

    /**
     * Init context.
     * @param context The Context
     */
    public Op01DataUsageSummaryExt(Context context) {
        super(context);
        mContext = context;
    }

    /**
     * app use to judge the Card is CMCC
     * @param slotId
     * @return true is CMCC
     */
    private boolean isCMCCCard(int subId) {
        Log.d("@M_" + TAG, "isCMCCCard, subId = " + subId);
        String simOperator = null;
        simOperator = getSimOperator(subId);
        if (simOperator != null) {
            Log.d("@M_" + TAG, "isCMCCCard, simOperator =" + simOperator);
            for (String mccmnc : MCCMNC_TABLE_TYPE_CMCC) {
                if (simOperator.equals(mccmnc)) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Gets the MCC+MNC (mobile country code + mobile network code)
     * of the provider of the SIM. 5 or 6 decimal digits.
     * Availability: The result of calling getSimState()
     * must be android.telephony.TelephonyManager.SIM_STATE_READY.
     * @param slotId  Indicates which SIM to query.
     * @return MCC+MNC (mobile country code + mobile network code)
     * of the provider of the SIM. 5 or 6 decimal digits.
     */
    private String getSimOperator(int subId) {
        if (subId < 0) {
            return null;
        }
        String simOperator = null;
        int status = TelephonyManager.SIM_STATE_UNKNOWN;
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
             status = TelephonyManager.getDefault().getSimState(slotId);
        }
        if (status == TelephonyManager.SIM_STATE_READY) {
            simOperator = TelephonyManager.getDefault().getSimOperator(subId);
        }
        Log.d("@M_" + TAG, "getSimOperator, simOperator = " + simOperator + " subId = " + subId);
        return simOperator;
    }

    @Override
    public boolean customDualReceiver(String action) {
        Log.d("@M_" + TAG, "action = "+  action);

        if (null != action
                && (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action))) {
            return true;
        }
        return false;
    }

    @Override
    public void customReceiver(IntentFilter intentFilter) {
        Log.d("@M_" + TAG, "intentFilter add sim_state_changed ");
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
    }

    public boolean customTempdata(int phoneId) {
        int imsCallCount = 0;
        try {
           imsCallCount = MtkImsManagerEx.getInstance().getCurrentCallCount(phoneId);
        } catch(Exception e) {
            Log.e(TAG, "getImsCallCount: " + e);
        }
        Log.i(TAG, "customTempdata phoneid = " + phoneId
                + ", imsCallCount = " + imsCallCount);
        if (imsCallCount > 0){
            return true;
        }
        return false;
    }

    public int getMainCapabilityPhoneId() {
        int phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx"));
        if (iTelEx != null) {
            try {
                phoneId = iTelEx.getMainCapabilityPhoneId();
            } catch (RemoteException e) {
                Log.i(TAG, "getMainCapabilityPhoneId, RemoteException=" + e);
            }
        } else {
            Log.i(TAG, "getMainCapabilityPhoneId, IMtkTelephonyEx service not ready.");
            phoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        }
        return phoneId;
    }
   @Override
    public boolean customTempdata() {
       int tempPhoneid = 0;
       int mainPhoneid = getMainCapabilityPhoneId();
       if (mainPhoneid == 0) {
           tempPhoneid = 1;
       } else {
           tempPhoneid = 0;
       }
       Log.i(TAG, "customTempdata mainPhoneid = " + mainPhoneid
               + " tempPhoneid = " + tempPhoneid);
       return customTempdata(tempPhoneid);
    }

    @Override
    public void customTempdataHide(Object object) {
        CustomDialogPreferenceCompat cdpc = (CustomDialogPreferenceCompat)object;
        List<SubscriptionInfo> si = SubscriptionManager.from(mContext)
                .getActiveSubscriptionInfoList();

        if (si == null) {
            Log.d(TAG, "si == null");
            return;
        }
        boolean allCmccCard[] = new boolean[si.size()];
        for (int i = 0; i < si.size(); i++) {
            allCmccCard[i] = false;
        }
        for (int i = 0; i < si.size(); i++) {
            SubscriptionInfo subscriptionInfo = si.get(i);
            allCmccCard[i] = isCMCCCard(subscriptionInfo.getSubscriptionId());
        }
        boolean hideTempDataOrNot = true;
        for (int i = 0; i < si.size(); i++) {
            hideTempDataOrNot = hideTempDataOrNot && allCmccCard[i];
        }
        Log.i(TAG, "customTempdataHide hideTempDataOrNot = " + hideTempDataOrNot);
        if (null != cdpc) {
            cdpc.setVisible(hideTempDataOrNot);
            /// set summary for opeartor.
            String customerSummaryString = customTempDataSummary(
                    "", TYPE_TEMP_DATA_SERVICE_SUMMARY);
            Log.i(TAG, "customTempdataHide hideTempDataOrNots = " + hideTempDataOrNot
                    + " customerSummaryString = " + customerSummaryString);
            cdpc.setSummary(customerSummaryString);
        }
    }

    @Override
    public String customTempDataSummary(String summary, int type) {
        Log.i(TAG, "customTempdataSummary summary");
        if (TYPE_TEMP_DATA_SERVICE_MESSAGE == type) {
            Log.i(TAG, "customTempdataSummary alert message = "
                    + mContext.getString(R.string.data_service_prompt));
            return mContext.getString(R.string.data_service_prompt);
        } else if (TYPE_TEMP_DATA_SERVICE_SUMMARY == type){
            Log.i(TAG, "customTempdataSummary summary = "
                    + mContext.getString(R.string.data_service_summary));
            return mContext.getString(R.string.data_service_summary);
        }
        return super.customTempDataSummary(summary, type);
    }

    @Override
    public boolean customizeBroadcastReceiveIntent() {
        Log.i(TAG, "customizeBroadcastReceiveIntent return true");
        return true;
    }
}

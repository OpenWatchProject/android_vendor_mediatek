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
package com.mediatek.op18.phone;


import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.telephony.RadioAccessFamily;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.ProxyController;

import com.mediatek.phone.ext.DefaultPhoneGlobalsExt;

import java.util.List;

/**
 * Operator requirements add interface for Phone Settings UI.
 * opeator like China Mobile,China Telecom, China Unicom, etc.
 */
public class OP18PhoneGlobalsExt extends DefaultPhoneGlobalsExt {


    //private UpdateNWTypeHandler mUpdateNWTypeHandler = new UpdateNWTypeHandler();

    private Handler mProtocolHandler;
    Context mContext;
    private static final String TAG = "OP18PhoneGlobalsExt";
    /** Constructor.
     * @param context context
     */
    public OP18PhoneGlobalsExt(Context context) {
        super();
        mContext = context;
    }

    public void handlePrimarySimUpdate(Context context, int subId) {
/*        log("handlePrimarySimUpdate for subId " + subId);
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        final List<SubscriptionInfo> subInfoList =
                subscriptionManager.getActiveSubscriptionInfoList();
        if (subInfoList == null) {
            log("subInfoList null");
            return;
        }

        mProtocolHandler = new Handler();
        final Runnable r = new Runnable() {
            public void run() {
                if (subInfoList.size() == 1) {
                    updateNetworkModeUtil(subId, Phone.NT_MODE_LTE_GSM_WCDMA);
                } else if (subInfoList.size() > 1) {
                    for (int index = 0; index < subInfoList.size(); index ++) {
                        int tempSubId = subInfoList.get(index).getSubscriptionId();
                        if (tempSubId == subId) {
                            updateNetworkModeUtil(tempSubId, Phone.NT_MODE_LTE_GSM_WCDMA);
                            setSimIdCapabilityforSubId(tempSubId, Phone.NT_MODE_LTE_GSM_WCDMA);
                        } else {
                            updateNetworkModeUtil(tempSubId, Phone.NT_MODE_GSM_ONLY);
                        }
                    }
                }
            }
        };

        mProtocolHandler.postDelayed(r, 5000);*/
    }
/*
    private void updateNetworkModeUtil(int subId, int mode) {
        log("Updating network mode for subId " + subId + "mode " + mode);
        Phone phone = PhoneFactory.getPhone(SubscriptionManager.getPhoneId(subId));

        phone.setPreferredNetworkType(mode,
                mUpdateNWTypeHandler.obtainMessage(
                        UpdateNWTypeHandler.MESSAGE_SET_PREFERRED_NETWORK_TYPE, subId, mode));
    }
*/
    /**
     * UpdateNWTypeHandler class to update network type.
     */
/*    private class UpdateNWTypeHandler extends Handler {

        static final int MESSAGE_SET_PREFERRED_NETWORK_TYPE = 0;

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_SET_PREFERRED_NETWORK_TYPE:
                    handleSetPreferredNetworkTypeResponse(msg, msg.arg1, msg.arg2);
                    break;
                default:
                    break;
            }
        }

        private void handleSetPreferredNetworkTypeResponse(Message msg, int subId, int mode) {
            AsyncResult ar = (AsyncResult) msg.obj;
            Phone phone = PhoneFactory.getPhone(SubscriptionManager.getPhoneId(subId));
            if (ar.exception == null) {
                log("handleSetPreferredNetwrokTypeResponse2: networkMode:" + mode);
                android.provider.Settings.Global.putInt(phone.getContext().getContentResolver(),
                        android.provider.Settings.Global.PREFERRED_NETWORK_MODE + subId,
                        mode);
            } else {
                log("handleSetPreferredNetworkTypeResponse:exception in setting network.");
            }
        }
    }
    /////////////////

    private boolean setSimIdCapabilityforSubId(int phoneSubId, int mode) {
        ProxyController proxyController = ProxyController.getInstance();
        List<SubscriptionInfo> simList = SubscriptionManager.from(mContext).
                getActiveSubscriptionInfoList();
        int simCount = simList.size();
        RadioAccessFamily[] rafs = new RadioAccessFamily[simCount];
        int raf;
        int count = SubscriptionManager.from(mContext).getActiveSubscriptionInfoCount();
        Log.d(TAG, "Active Sub Info Count " + count);
        if (count == 1) {
            return true;
        }
        if (mode == Phone.NT_MODE_GSM_ONLY) {
            for (int index = 0; index < simCount; index ++) {
                int subId = simList.get(index).getSubscriptionId();
                if (subId == phoneSubId) {
                    raf = proxyController.getMinRafSupported();
                    Log.d(TAG, "Set proxyController for GSM Only to GSM");
                } else {
                    raf = proxyController.getMaxRafSupported();
                    Log.d(TAG, "Set proxyController for GSM Only to LTE");
                }
                rafs[index] = new RadioAccessFamily(index, raf);
            }
        } else if (mode == Phone.NT_MODE_WCDMA_ONLY ||
                        mode == Phone.NT_MODE_WCDMA_PREF) {
            for (int index = 0; index < simCount; index ++) {
                int subId = simList.get(index).getSubscriptionId();
                if (subId == phoneSubId) {
                    raf = RadioAccessFamily.RAF_UMTS;
                    Log.d(TAG, "Set proxyController to WCDMA");
                } else {
                    raf = proxyController.getMaxRafSupported();
                    Log.d(TAG, "Set proxyController to LTE");
                }
                rafs[index] = new RadioAccessFamily(index, raf);
            }
        } else {
            for (int index = 0; index < simCount; index ++) {
                int subId = simList.get(index).getSubscriptionId();
                if (subId == phoneSubId) {
                    raf = proxyController.getMaxRafSupported();
                    Log.d(TAG, "Set proxyController for LTE to LTE");
                } else {
                    raf = proxyController.getMinRafSupported();
                    Log.d(TAG, "Set proxyController for LTE to GSM");
                }
                rafs[index] = new RadioAccessFamily(index, raf);
            }
        }
        boolean result = proxyController.setRadioCapability(rafs);
        Log.d(TAG, "Result after setting ProxyController: " + result);
        return result;
    }
*/
    private void log(String s) {
        Log.d("OP18PhoneGlobalsExt", s);
    }
}

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

package com.mediatek.settings.sim;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.telephony.TelephonyIntents;

/// M: Add for operator config changed.
import com.mediatek.common.carrierexpress.CarrierExpressManager;

import java.util.Arrays;
import java.util.List;

public class SimHotSwapHandler {

    private static final String TAG = "SimHotSwapHandler";
    private SubscriptionManager mSubscriptionManager;
    private Context mContext;
    private List<SubscriptionInfo> mSubscriptionInfoList;
    private OnSimHotSwapListener mListener;
    private BroadcastReceiver mSubReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive, action=" + action);
            if (null != action
                    && action.equals(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED)) {
                if (mListener != null) {
                    mListener.onSimHotSwap();
                }
            } else if (null != action
                    && action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                handleHotSwap();
            }
        }
    };

    public SimHotSwapHandler(Context context) {
        mContext = context;
        mSubscriptionManager = SubscriptionManager.from(context);
        mSubscriptionInfoList = mSubscriptionManager.getActiveSubscriptionInfoList(true);
        Log.d(TAG, "handler=" + this + ", cacheList=" + mSubscriptionInfoList);
    }

    public void registerOnSimHotSwap(OnSimHotSwapListener listener) {
        if (mContext != null) {
            IntentFilter intentFilter =  new IntentFilter();
            intentFilter.addAction(
                     TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
            intentFilter.addAction(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED);
            mContext.registerReceiver(mSubReceiver, intentFilter);
            mListener = listener;
            Log.d(TAG, "registerOnSimHotSwap, handler=" + this
                    + ", listener=" + listener);
        }
    }

    public void unregisterOnSimHotSwap() {
        if (mContext != null) {
            mContext.unregisterReceiver(mSubReceiver);
            Log.d(TAG, "unregisterOnSimHotSwap, handler=" + this);
        }
        mListener = null;
    }

    private void handleHotSwap() {
        List<SubscriptionInfo> subscriptionInfoListCurrent =
                mSubscriptionManager.getActiveSubscriptionInfoList(true);
        Log.d(TAG, "handleHotSwap, handler=" + this
                + ", currentSubIdList=" + subscriptionInfoListCurrent);
        if (hasHotSwapHappened(mSubscriptionInfoList, subscriptionInfoListCurrent)
                && mListener != null) {
            mListener.onSimHotSwap();
        }
    }

    public interface OnSimHotSwapListener {
        void onSimHotSwap();
    }

    /**
     * Return whether the phone is hot swap or not.
     * @return If hot swap, return true, else return false
     */
    public static boolean hasHotSwapHappened(List<SubscriptionInfo> originalList,
            List<SubscriptionInfo> currentList) {
        boolean result = false;
        int oriCount = (originalList == null ? 0 : originalList.size());
        int curCount = (currentList == null ? 0 : currentList.size());

        if (oriCount == 0 && curCount == 0) {
            return false;
        }
        if (oriCount == 0 || curCount == 0 ||
                originalList.size() != currentList.size()) {
            Log.d(TAG, "hasHotSwapHappened, SIM count is different"
                    + ", oriCount=" + oriCount + ", curCount=" + curCount);
            return true;
        }
        for (int i = 0; i < currentList.size(); i++) {
            SubscriptionInfo currentSubInfo = currentList.get(i);
            SubscriptionInfo originalSubInfo = originalList.get(i);
            if (!(currentSubInfo.getIccId()).equals(originalSubInfo.getIccId())) {
                result = true;
                break;
            }
        }

        Log.d(TAG, "hasHotSwapHappened, result=" + result);
        return result;
    }
}

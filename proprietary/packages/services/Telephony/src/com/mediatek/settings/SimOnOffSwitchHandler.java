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

package com.mediatek.settings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.telephony.MtkTelephonyManagerEx;

public class SimOnOffSwitchHandler {

    private static final String TAG = "SimOnOffSwitchHandler";
    private Context mContext;
    private boolean mSimOnOffEnabled;
    private int mSlotId;
    private int mSimState;
    private OnSimOnOffSwitchListener mListener;
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            handleSimApplicationStateChanged(intent);
        }
    };

    public SimOnOffSwitchHandler(Context context, int slotId) {
        mContext = context;
        mSimOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        mSlotId = slotId;
        mSimState = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
        Log.d(TAG, "handler=" + this + ", simOnOffEnabled=" + mSimOnOffEnabled
                + ", slotId=" + mSlotId + ", state=" + mSimState);
    }

    public void registerOnSimOnOffSwitch(OnSimOnOffSwitchListener listener) {
        if (mContext != null && mSimOnOffEnabled) {
            mContext.registerReceiver(mReceiver, new IntentFilter(
                    TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED));
            mListener = listener;
            Log.d(TAG, "registerOnSimOnOffSwitch, handler=" + this
                    + ", listener=" + listener);
        }
    }

    public void unregisterOnSimOnOffSwitch() {
        if (mContext != null && mSimOnOffEnabled) {
            mContext.unregisterReceiver(mReceiver);
            Log.d(TAG, "unregisterOnSimOnOffSwitch, handler=" + this);
        }
        mListener = null;
    }

    private void handleSimApplicationStateChanged(Intent intent) {
        Bundle extra = intent.getExtras();
        if (extra == null) {
            Log.d(TAG, "handleSimApplicationStateChange, extra=null");
            return;
        }

        int slotId = extra.getInt(PhoneConstants.SLOT_KEY,
                SubscriptionManager.INVALID_SIM_SLOT_INDEX);
        if (SubscriptionManager.isValidSlotIndex(slotId)) {
            int state = MtkTelephonyManagerEx.getDefault().getSimOnOffState(mSlotId);
            Log.d(TAG, "handleSimApplicationStateChange, slotId=" + slotId
                    + ", prevState=" + mSimState + ", currState=" + state);
            if (mSimState != state && mListener != null) {
                mListener.onSimOnOffStateChanged();
            }
            mSimState = state;
        }
    }

    public interface OnSimOnOffSwitchListener {
        void onSimOnOffStateChanged();
    }
}

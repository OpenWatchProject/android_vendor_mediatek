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
package com.mediatek.contacts.util;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.contacts.ContactsApplicationEx;

import java.util.ArrayList;
import java.util.List;

/**
 * this class used to handle the state change as phb, hotswap.
 */
public class PhbStateHandler {
    private static final String TAG = "PhbStateHandler";

    public interface Listener {

        /**
         * the callback to handle the phb change.
         *
         * @param subId
         *            The current sim contact refer to this sub id.
         */
        public void onPhbStateChange(int subId);
    }

    private List<Listener> mListeners = new ArrayList<Listener>();
    private Context mContext = (Context) ContactsApplicationEx.getContactsApplication();
    private boolean mRegistered;
    private static PhbStateHandler sInstance;

    private BroadcastReceiver mPhbStateListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                    ContactsConstants.ERROR_SUB_ID);
            Log.i(TAG, "[onReceive]action: " + action + ",subId:" + subId);
            // for phb state change
            if (TelephonyIntents.ACTION_PHB_STATE_CHANGED.equals(action)) {
                for (Listener listener : mListeners) {
                    if (listener != null) {
                        listener.onPhbStateChange(subId);
                    }
                }
            }
        }
    };

    private PhbStateHandler() {
    }

    /**
     * get the instance of the PhbStateHandler.
     */
    public static synchronized PhbStateHandler getInstance() {
        if (sInstance == null) {
            sInstance = new PhbStateHandler();
        }
        return sInstance;
    }

    /**
     * register the listener.
     *
     * @param target
     *            the target register the listener.
     */
    public synchronized void register(Listener target) {
        Log.i(TAG, "[register]target: " + target + ",mRegistered = " + mRegistered);
        // one target only register once
        if (target != null && !mListeners.contains(target)) {
            mListeners.add(target);
        }
        if (!mRegistered) {
            mContext.registerReceiver(mPhbStateListener, new IntentFilter(
                    TelephonyIntents.ACTION_PHB_STATE_CHANGED));
            Log.d(TAG, "register phb");
            mRegistered = true;
        }
    }

    /**
     * unRegister the listener.
     *
     * @param target
     *            the target unRegister the listener.
     */
    public synchronized void unRegister(Listener target) {
        Log.i(TAG, "[unRegister]target: " + target + ",mRegistered = " + mRegistered);
        if (target != null && mListeners.contains(target)) {
            mListeners.remove(target);
        }
        if (mListeners.isEmpty() && mRegistered) {
            mContext.unregisterReceiver(mPhbStateListener);
            Log.d(TAG, "unRegister phb");
            mRegistered = false;
        }
    }
}

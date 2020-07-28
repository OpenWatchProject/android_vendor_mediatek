/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.dialer.calldetails.presence;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import com.mediatek.dialer.ext.DefaultCallDetailsActivityExt;
import com.mediatek.dialer.presence.ContactNumberUtils;
import com.mediatek.dialer.presence.PresenceApiManager;
import com.mediatek.dialer.presence.PresenceApiManager.CapabilitiesChangeListener;
import com.mediatek.dialer.presence.PresenceApiManager.ContactInformation;

import com.mediatek.dialer.common.LogUtils;

public class CallDetailsActivityExt extends DefaultCallDetailsActivityExt
       implements CapabilitiesChangeListener {
    private static final String TAG = "CallDetailsActivityExt ";
    private Context mHostContext;
    private RecyclerView.Adapter mAdapter;
    private String mNumber;
    private Handler mHandler = null;
    private PresenceApiManager mTapi = null;
    private String mFormatNumber;

    CallDetailsActivityExt(Context context, RecyclerView.Adapter adapter, String number) {
        Log.i(TAG, "CallDetailsActivityExt");
        mHostContext = context;
        mAdapter = adapter;
        mNumber = number;
        mHandler = new Handler(Looper.getMainLooper());
        if (PresenceApiManager.initialize(mHostContext)) {
            mTapi = PresenceApiManager.getInstance();
        }

        mFormatNumber = ContactNumberUtils.getDefault().getFormatNumber(number);
        LogUtils.printSensitiveInfo(TAG,
                "CallDetailsActivityExt: mFormatNumber = " + mFormatNumber);
        if (mTapi != null) {
            //for subscribe
            mTapi.requestContactPresence(mFormatNumber, false);
        }
    }

    @Override
    public void onResume() {
        Log.i(TAG, "onResume");
        if (mTapi != null) {
            mTapi.addCapabilitiesChangeListener(this);
        }
    }

    @Override
    public void onPause() {
        Log.i(TAG, "onPause");
        if (mTapi != null) {
            mTapi.removeCapabilitiesChangeListener(this);
        }
    }

    @Override
    public void onCapabilitiesChanged(String contact, ContactInformation info) {
        if (contact.equals(mFormatNumber) && info != null) {
            LogUtils.printSensitiveInfo(TAG,
                    "onCapabilitiesChanged update ui. mFormatNumber:" + mFormatNumber);
            mHandler.post(() -> mAdapter.notifyItemChanged(0));
        }
    }

    public void onErrorReceived(String contact, int type, int status, String reason) {
    }
}

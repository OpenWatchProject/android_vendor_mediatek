/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.server.ppl;

import android.app.StatusBarManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

public class MtkPplManagerImpl extends MtkPplManager{
    private static final String TAG = "MtkPplManager";
    private boolean mPplStatus = false;
    private StatusBarManager mStatusBarManager;

    @Override
    public IntentFilter registerPplIntents() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(PPL_LOCK);
        filter.addAction(PPL_UNLOCK);
        return filter;
    }

    @Override
    public int calculateStatusBarStatus(boolean pplStatus) {
        int status = 0;
        if (pplStatus) {
            status = StatusBarManager.DISABLE_EXPAND | StatusBarManager.DISABLE_NOTIFICATION_ALERTS
                    | StatusBarManager.DISABLE_NOTIFICATION_ICONS
                    | StatusBarManager.DISABLE_NOTIFICATION_TICKER;
        }
        return status;
    }

    @Override
    public boolean getPplLockStatus() {
        return mPplStatus;
    }

    @Override
    public boolean filterPplAction(String action) {
        if (action.equals(PPL_LOCK)) {
            Log.d(TAG, "filterPplAction, recevier action = " + action);
            mPplStatus = true;
        } else if (action.equals(PPL_UNLOCK)) {
            Log.d(TAG, "filterPplAction, recevier action = " + action);
            mPplStatus = false;
        }
        return mPplStatus;
    }

    @Override
    public void registerPplReceiver(Context context) {
        context.registerReceiver(mPPLReceiver, registerPplIntents());
    }

    private final BroadcastReceiver mPPLReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            pplEnable(context, filterPplAction(intent.getAction()));
        }
    };

    private void pplEnable(Context context, boolean enable) {
        int what = calculateStatusBarStatus(enable);
        if (mStatusBarManager == null) {
            mStatusBarManager = (StatusBarManager) context.getSystemService(
                    Context.STATUS_BAR_SERVICE);
        }
        mStatusBarManager.disable(what);
    }
}

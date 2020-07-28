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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.android.camera.externaldevice;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.display.DisplayManager;
import android.hardware.display.WifiDisplayStatus;

import com.android.camera.CameraActivity;

import com.mediatek.camera.util.Log;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

public class WfdDeviceCtrl implements IExternalDeviceCtrl {

    private static final String TAG = "WfdDeviceCtrl";

    private CameraActivity mContext;
    private DisplayManager mDisplayManager;
    private WifiDisplayStatus mWifiDisplayStatus;
    private List<Listener> mListeners = new CopyOnWriteArrayList<Listener>();

    public WfdDeviceCtrl(CameraActivity context) {
        mContext = context;
        mDisplayManager = (DisplayManager) mContext.getSystemService(Context.DISPLAY_SERVICE);
    }

    @Override
    public boolean onCreate() {
        return false;
    }

    @Override
    public boolean onResume() {
        IntentFilter filter = new IntentFilter(
                DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED);
        mContext.registerReceiver(mReceiver, filter);
        mWifiDisplayStatus = null;
        notifyStateChanged(isWfdEnabled());

        return false;
    }

    @Override
    public boolean onPause() {
        mContext.unregisterReceiver(mReceiver);
        mWifiDisplayStatus = null;

        return false;
    }

    @Override
    public boolean onDestory() {
        return false;
    }

    @Override
    public boolean onOrientationChanged(int orientation) {
        // do-noting
        return false;
    }


    @Override
    public void addListener(Object listenr) {
        Listener l = (Listener) listenr;
        if (!mListeners.contains(l)) {
            mListeners.add(l);
        }
    }

    @Override
    public void removeListener(Object listenr) {
        mListeners.remove((Listener) listenr);
    }


    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED)) {
                WifiDisplayStatus status = (WifiDisplayStatus) intent
                        .getParcelableExtra(DisplayManager.EXTRA_WIFI_DISPLAY_STATUS);
                mWifiDisplayStatus = status;
                notifyStateChanged(isWfdEnabled());
            }
        };
    };

    private boolean isWfdEnabled() {
        boolean enabled = false;
        int activeDisplayState = -1;
        if (mWifiDisplayStatus == null) {
            mWifiDisplayStatus = mDisplayManager.getWifiDisplayStatus();
        }
        activeDisplayState = mWifiDisplayStatus.getActiveDisplayState();
        enabled = activeDisplayState == WifiDisplayStatus.DISPLAY_STATE_CONNECTED;
        Log.d(TAG, "[isWfdEnabled()] mWifiDisplayStatus=" + mWifiDisplayStatus + ", return "
                + enabled);

        return enabled;
    }

    private void notifyStateChanged(boolean enabled) {
        Log.d(TAG, "[notifyStateChanged](" + enabled + ")");
        for (Listener listener : mListeners) {
            if (listener != null) {
                listener.onStateChanged(enabled);
            }
        }
    }
}

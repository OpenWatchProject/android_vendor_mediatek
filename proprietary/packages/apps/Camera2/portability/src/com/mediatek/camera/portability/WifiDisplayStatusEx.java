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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.camera.portability;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.display.DisplayManager;
import android.hardware.display.WifiDisplayStatus;
import android.util.Log;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Used to listener wifi display status change.
 */
public class WifiDisplayStatusEx {

    /**
     * Wifi display state change listener.
     */
    public interface OnStateChangeListener {
        /**
         * Notify state change.
         * @param isConnected true: the wifi display is connected.
         *                false: the wifi display is not connected.
         */
        void onStateChanged(boolean isConnected);
    }

    private static final String TAG = "WifiDisplayStatusEx";
    private Context mContext;
    private DisplayManager mDisplayManager;
    private List<OnStateChangeListener> mListeners = new CopyOnWriteArrayList<>();

    /**
     * Contructor.
     * @param context Application context.
     */
    public WifiDisplayStatusEx(Context context) {
        mContext = context;
        mDisplayManager = (DisplayManager) mContext.getSystemService(Context.DISPLAY_SERVICE);
    }

    /**
     * Init and register broadcast receiver.
     */
    public void init() {
        IntentFilter filter = new IntentFilter(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED);
        mContext.registerReceiver(mReceiver, filter);
        notifyStateChanged(isWfdEnabled());
    }

    /**
     * Uninit and unregister broadcast receiver.
     */
    public void unInit() {
        mContext.unregisterReceiver(mReceiver);
    }

    /**
     * Register state change listener.
     * @param listener The listener.
     */
    public void setStateChangeListener(OnStateChangeListener listener) {
        if (!mListeners.contains(listener)) {
            mListeners.add(listener);
        }
    }

    /**
     * Judge if the wifi display is connected or not.
     * @param context Application context.
     * @return true: the wifi display is connected.
     *         false: the wifi display is not connected.
     */
    public static boolean isWfdEnabled(Context context) {
        boolean enabled = false;
        int activeDisplayState = -1;
        DisplayManager displayManager =
                (DisplayManager) context.getSystemService(Context.DISPLAY_SERVICE);
        WifiDisplayStatus status = displayManager.getWifiDisplayStatus();

        activeDisplayState = status.getActiveDisplayState();
        enabled = activeDisplayState == WifiDisplayStatus.DISPLAY_STATE_CONNECTED;
        return enabled;
    }

    private boolean isWfdEnabled() {
        boolean enabled = false;
        int activeDisplayState = -1;

        WifiDisplayStatus status = mDisplayManager.getWifiDisplayStatus();

        activeDisplayState = status.getActiveDisplayState();
        enabled = activeDisplayState == WifiDisplayStatus.DISPLAY_STATE_CONNECTED;
        return enabled;
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED)) {
                notifyStateChanged(isWfdEnabled());
            }
        };
    };

    private void notifyStateChanged(boolean enabled) {
        for (OnStateChangeListener listener : mListeners) {
            if (listener != null) {
                listener.onStateChanged(enabled);
            }
        }
    }
}
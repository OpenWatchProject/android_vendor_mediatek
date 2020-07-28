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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

package com.mediatek.camera.v2.services.storage;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

import com.mediatek.camera.v2.services.storage.IStorageService.IStorageStateListener;

import java.util.concurrent.CopyOnWriteArrayList;

/**
 * The class use to monitor storage state change. and notify listener
 */
public class StorageMonitor {

    private static final String TAG = "StorageMonitor";
    private Context mContext;
    private CopyOnWriteArrayList<IStorageStateListener> mIStorageStateListener
    = new CopyOnWriteArrayList<IStorageStateListener>();

    /**
     * the constructor.
     * @param context
     *            use for register listener
     */
    public StorageMonitor(Context context) {
        mContext = context;
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "mReceiver.onReceive(" + intent + ")");
            String action = intent.getAction();
            if (action == null) {
                Log.d(TAG, "[mReceiver.onReceive] action is null");
                return;
            }
            switch (intent.getAction()) {

            case Intent.ACTION_MEDIA_EJECT:

                if (Storage.isSameStorage(intent)) {
                    for (IStorageStateListener listener : mIStorageStateListener) {
                        listener.onStorageStateChanged(IStorageService.STORAGE_IS_OUT_OF_SERVICES);
                    }
                }
                break;

            case Intent.ACTION_MEDIA_UNMOUNTED:
            case Intent.ACTION_MEDIA_MOUNTED:
            case Intent.ACTION_MEDIA_SCANNER_FINISHED:

                Storage.updateDefaultDirectory();
                for (IStorageStateListener listener : mIStorageStateListener) {
                    listener.onStorageStateChanged(
                            Storage.isStorageReady() ? IStorageService.STORAGE_IS_IN_SERVICES
                            : IStorageService.STORAGE_IS_OUT_OF_SERVICES);
                }
                break;

            case Intent.ACTION_MEDIA_CHECKING:
            case Intent.ACTION_MEDIA_SCANNER_STARTED:

                if (Storage.isSameStorage(intent)) {
                    for (IStorageStateListener listener : mIStorageStateListener) {
                        listener.onStorageStateChanged(IStorageService.STORAGE_IS_PREPARING);
                    }
                }
                break;

            default:
                break;
            }
        }
    };

    /**
     * Register listener to monitor the storage state change.
     * @param listener
     *            who want to know storage state change.
     */
    public void registerStorageStateListener(IStorageStateListener listener) {
        Log.i(TAG, "[registerStorageStateListener], listener:" + listener);
        Storage.updateDefaultDirectory();
        if (listener != null && !mIStorageStateListener.contains(listener)) {
            mIStorageStateListener.add(listener);
        }
    }

    /**
     * Remove the listener to monitor the storage state change.
     * @param listener
     *            who want to know storage state change.
     */
    public void unRegisterStorageStateListener(IStorageStateListener listener) {
        Log.i(TAG, "[unRegisterStorageStateListener], listener:" + listener);
        mIStorageStateListener.remove(listener);
    }

    /**
     * Register an intent filter to receive SD card related events.
     * storage monitor will receive these action when storage change.
     */
    public void registerIntentFilter() {
        IntentFilter intentFilter = new IntentFilter(
                Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        intentFilter.addAction(Intent.ACTION_MEDIA_CHECKING);
        intentFilter.addDataScheme("file");
        mContext.registerReceiver(mReceiver, intentFilter);
    }

    /**
     * Unregister intent for filter.
     */
    public void unregisterIntentFilter() {
        mContext.unregisterReceiver(mReceiver);
    }

}
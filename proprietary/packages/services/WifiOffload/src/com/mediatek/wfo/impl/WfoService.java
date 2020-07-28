/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.wfo.impl;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.SystemProperties;
import android.os.Message;
import android.os.ServiceManager;
import android.os.SystemProperties;

import android.telephony.Rlog;
import android.telephony.TelephonyManager;

import com.mediatek.wfo.IMwiService;
import com.mediatek.wfo.IWifiOffloadService;
import com.mediatek.wfo.MwisConstants;
import com.mediatek.wfo.WifiOffloadManager;
import com.mediatek.wfo.impl.MwiService;

import com.mediatek.wfo.plugin.ExtensionFactory;
import com.mediatek.wfo.plugin.LegacyComponentFactory;

/**
 * Mobile Wi-Fi Interaction Service.
 */
public class WfoService {
    public static WfoService mInstance = null;

    static final String TAG = "WfoService";

    private IWifiOffloadService.Stub mService;
    private MwiService mMwiService;
    private Context mContext;


    public static WfoService getInstance(Context context) {
        if (mInstance == null) {
            mInstance = new WfoService(context);
        }
        return mInstance;
    }

    private WfoService(Context context) {
        mContext = context;
    }

    public void makeWfoService() {
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            Rlog.d(TAG, "WfoService new MWIService");
            // mMwiService = new MwiService(mContext);
            mMwiService = MwiService.getInstance(mContext);
            ServiceManager.addService(MwisConstants.MWI_SERVICE, mMwiService.asBinder(), true);
        } else {
            Rlog.d(TAG, "WfoService new WifiOffloadService");
            LegacyComponentFactory factory = ExtensionFactory.makeLegacyComponentFactory(mContext);
            mService = factory.makeWifiOffloadService(mContext);
            if(mService == null) {
                Rlog.e(TAG, "WfoService cannot be found");
            }
            else {
                ServiceManager.addService(WifiOffloadManager.WFO_SERVICE, mService.asBinder(), true);
            }
        }
    }
}

// public class WfoService extends Service {
//     static final String TAG = "WfoService";

//     private Context mContext;
//     private IWifiOffloadService.Stub mService;
//     private MwiService mMwiService;

//     public MyBinder myBinder = new MyBinder();

//     public class MyBinder extends Binder {
//         public WfoService getService() {
//             return WfoService.this;
//         }
//     }

//     @Override
//     public boolean onUnbind(Intent intent) {
//         Rlog.d(TAG, "MainService onUnbind");
//         return super.onUnbind(intent);
//     }

//     @Override
//     public void onCreate() {
//         super.onCreate();
//         Rlog.d(TAG, "onCreate()");

//         mContext = this.getBaseContext();


//         if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
//             Rlog.d(TAG, "WfoService new MWIService");
//             mMwiService = new MwiService(this);
//             ServiceManager.addService(MwisConstants.MWI_SERVICE, mMwiService.asBinder(), true);
//         } else {
//             Rlog.d(TAG, "WfoService new WifiOffloadService");
//             LegacyComponentFactory factory = ExtensionFactory.makeLegacyComponentFactory();
//             mService = factory.makeWifiOffloadService(this);
//             if(mService == null) {
//                 Rlog.e(TAG, "WfoService cannot be found");
//             }
//             else {
//                 ServiceManager.addService(WifiOffloadManager.WFO_SERVICE, mService.asBinder(), true);
//             }
//         }
//     }

//     @Override
//     public int onStartCommand(Intent intent, int flags, int startId) {
//         Rlog.d(TAG, "onStartCommand()");
//         if (intent == null) {
//             return Service.START_STICKY;
//         }

//         String action = intent.getAction();
//         Rlog.d(TAG, "Service starting for intent " + action);

//         return Service.START_STICKY;
//     }

//     @Override
//     public IBinder onBind(Intent intent) {
//         Rlog.d(TAG, "onBind()");
//         return myBinder;
//     }

//     @Override
//     public void onDestroy() {
//         Rlog.d(TAG, "Service is onDestroy");
//         if (mMwiService != null) {
//             mMwiService.dispose();
//         }
//     }

// }


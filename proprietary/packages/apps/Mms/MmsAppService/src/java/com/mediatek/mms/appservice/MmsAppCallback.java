/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.mms.appservice;

import com.mediatek.mms.appcallback.IMmsCallbackService;
import com.mediatek.mms.appcallback.SmsEntry;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.UserHandle;
import android.os.RemoteException;
import java.util.ArrayList;
import android.util.Log;

public class MmsAppCallback {
    public static final String TAG = "MmsAppService/MmsConfig";
    public static final String TAG_TXN = "MmsAppCallback/Txn";
    private Context mContext;

    private static MmsAppCallback sInstance = null;
    private static Object syncObj = new Object();
    private Object bindWaitObj = new Object();
    private final int bindWaitTimer = 3000;

    private MmsAppCallback(Context context) {
        mContext = context;
    }

    public static MmsAppCallback getInstance(Context context) {
        Log.d(TAG_TXN, "MmsAppCallback.getInstance");
        synchronized (syncObj) {
            Log.d(TAG_TXN, "MmsAppCallback.getInstance1");
            if (sInstance == null) {
                Log.d(TAG_TXN, "new MmsAppCallback instance");
                sInstance = new MmsAppCallback(context);
            }
        }
        Log.d(TAG_TXN, "return instance");
        return sInstance;
    }

    //private AppCallbackServiceConnection conn = new AppCallbackServiceConnection();
    private IMmsCallbackService appCallbackService = null;

    private ServiceConnection conn = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG_TXN, "onServiceConnected");
            synchronized(bindWaitObj) {
                Log.d(TAG_TXN, "enter bindWaitObj");
                appCallbackService = IMmsCallbackService.Stub.asInterface(service);
                Log.d(TAG_TXN, "bindWaitObj.notify, appCallbackService = " + appCallbackService);
                bindWaitObj.notify();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG_TXN, "onServiceDisconnected, appCallbackService = " + appCallbackService);
            synchronized(bindWaitObj) {
                appCallbackService = null;
            }
        }
    };

    public synchronized void bindAppCallbackService(Context context) {
        Log.d(TAG_TXN, "bindAppCallbackService");
        Intent intent = new Intent();
        intent.setClassName("com.android.mms", "com.android.mms.transaction.MmsCallbackService");
        context.bindServiceAsUser(intent, conn, Service.BIND_AUTO_CREATE, UserHandle.CURRENT);
        Log.d(TAG_TXN, "after bindAppCallbackService");
    }

    public synchronized void unbindAppCallbackService(Context context) {
        Log.d(TAG_TXN, "unbindAppCallbackService, appCallbackService = " + appCallbackService);
        synchronized(bindWaitObj) {
            Log.d(TAG_TXN, "check appCallbackService");
            if (appCallbackService != null)
            {
                Log.d(TAG_TXN, "will do unbindService, conn = " + conn);
                try {
                    context.unbindService(conn);
                } catch (IllegalArgumentException e) {
                    Log.e(TAG_TXN, "unbindService excpetion! " + e);
                } finally {
                    appCallbackService = null;
                }
            }
        }
    }

    public synchronized boolean appCallbackSendSms(Context context, int subid, String destAddr,
            String scAddr, ArrayList<String> parts, int encodingType,
            ArrayList<PendingIntent> sentIntents,ArrayList<PendingIntent> deliveryIntents) {
        Log.d(TAG_TXN, "appCallbackSendSms");
        synchronized(bindWaitObj) {
            Log.d(TAG_TXN, "appCallbackSendSms, appCallbackService = " + appCallbackService);
            if (appCallbackService == null) {
                Log.d(TAG_TXN, "appCallbackService null, wait");
                try {
                    bindWaitObj.wait(bindWaitTimer);
                } catch (InterruptedException e) {
                    Log.d(TAG_TXN, "appCallbackService null, wait exception");
                    return false;
                }
                if (appCallbackService == null) {
                    Log.d(TAG_TXN, "appCallbackService null, wait 500ms but not done");
                    return false;
                }
                Log.d(TAG_TXN, "appCallbackService null, bind done, go on");
            }
        }
        try {
            SmsEntry entry = new SmsEntry(subid, destAddr, scAddr, parts,
                encodingType, sentIntents, deliveryIntents);
            Log.d(TAG_TXN, "to call back Send Sms");
            appCallbackService.sendMultipartTextMessageWithEncodingType(entry);
            Log.d(TAG_TXN, "after call back Send Sms");
            return true;
        } catch (RemoteException e) {
            Log.d(TAG_TXN, "RemoteException");
            return false;
        }
    }
}

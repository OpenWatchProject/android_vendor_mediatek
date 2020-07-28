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

package com.mediatek.mms.appserviceproxy;

import com.mediatek.mms.appserviceproxy.IAppServiceProxy;
import com.mediatek.mms.appserviceproxy.AppServiceProxyParam;
import android.app.Service;
import android.content.Context;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import java.util.ArrayList;
import android.util.Log;

public class AppServiceProxy {
    public static final String TAG = "Mms/AppServiceProxy";
    public static final String TAG_TXN = "Mms/Txn";
    private Context mContext;

    private static AppServiceProxy sInstance = null;
    private static Object syncObj = new Object();
    private Object bindWaitObj = new Object();
    private final int bindWaitTimer = 10000;

    private AppServiceProxy(Context context) {
        mContext = context;
    }

    public static AppServiceProxy getInstance(Context context) {
        Log.d(TAG_TXN, "AppServiceProxy.getInstance");
        synchronized (syncObj) {
            Log.d(TAG_TXN, "AppServiceProxy.getInstance1");
            if (sInstance == null) {
                Log.d(TAG_TXN, "new AppServiceProxy instance");
                sInstance = new AppServiceProxy(context);
            }
        }
        Log.d(TAG_TXN, "return instance");
        return sInstance;
    }

    //private appServiceProxyConnection conn = new appServiceProxyConnection();
    private IAppServiceProxy appServiceProxy = null;

    private ServiceConnection conn = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG_TXN, "onServiceConnected");
            synchronized(bindWaitObj) {
                Log.d(TAG_TXN, "enter bindWaitObj");
                appServiceProxy = IAppServiceProxy.Stub.asInterface(service);
                Log.d(TAG_TXN, "bindWaitObj.notify");
                bindWaitObj.notify();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG_TXN, "onServiceDisconnected");
            appServiceProxy = null;
        }
    };

    public synchronized void bindAppServiceProxy(Context context) {
        Log.d(TAG_TXN, "bindAppServiceProxy");
        Intent intent = new Intent();
        intent.setClassName("com.mediatek.mms.appservice", "com.mediatek.mms.appserviceproxy.MmsAppServiceProxy");
        context.bindService(intent, conn, Service.BIND_AUTO_CREATE);
        Log.d(TAG_TXN, "after bindAppServiceProxy");
    }

    public synchronized void unbindAppServiceProxy(Context context) {
        Log.d(TAG_TXN, "unbindAppServiceProxy");
        context.unbindService(conn);
    }

    public synchronized void startServiceByParam(Context context, Intent intent) {
        Log.d(TAG_TXN, "startServiceByParam");
        bindAppServiceProxy(context);

        synchronized(bindWaitObj) {
            Log.d(TAG_TXN, "startServiceByParam, appServiceProxy = " + appServiceProxy);
            if (appServiceProxy == null) {
                Log.d(TAG_TXN, "appServiceProxy null, wait");
                try {
                    bindWaitObj.wait(bindWaitTimer);
                } catch (InterruptedException e) {
                    Log.d(TAG_TXN, "appServiceProxy null, wait exception");
                    unbindAppServiceProxy(context);
                    return;
                }
                if (appServiceProxy == null) {
                    Log.d(TAG_TXN, "appServiceProxy null, wait 10s but not done");
                    unbindAppServiceProxy(context);
                    return;
                }
                Log.d(TAG_TXN, "appServiceProxy null, bind done, go on");
            }
        }
        try {
            AppServiceProxyParam param = new AppServiceProxyParam(intent);
            Log.d(TAG_TXN, "to call startServiceByParam");
            appServiceProxy.startServiceByParam(param);
            Log.d(TAG_TXN, "after call startServiceByParam");
        } catch (RemoteException e) {
            Log.d(TAG_TXN, "RemoteException");
        }
        unbindAppServiceProxy(context);
    }
}

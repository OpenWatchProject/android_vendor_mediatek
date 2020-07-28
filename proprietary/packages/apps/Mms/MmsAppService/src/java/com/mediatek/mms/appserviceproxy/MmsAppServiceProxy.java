/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

/*
 * Copyright (C) 2007 The Android Open Source Project
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
import android.content.Intent;
import android.os.IBinder;
import android.os.Binder;
import android.util.Log;

import java.lang.ref.SoftReference;

public class MmsAppServiceProxy extends Service {
    private final IBinder mBinder = new ServiceStub(this);
    private static final String TAG = "Mms/Txn";
    private static final String MTK_MMS_PACKAGE = "com.android.mms";

    public MmsAppServiceProxy() {
        Log.d(TAG, "MmsAppServiceProxy.MmsAppServiceProxy");
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "MmsAppServiceProxy.onCreate");
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "MmsAppServiceProxy.onDestroy");
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "MmsAppServiceProxy.onBind");
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "MmsAppServiceProxy.onUnbind");
        //stopSelf(mServiceStartId);
        return true;
    }

    public class ServiceStub extends IAppServiceProxy.Stub {
        SoftReference<MmsAppServiceProxy> mService;

        ServiceStub(MmsAppServiceProxy service) {
            Log.d(TAG, "MmsAppServiceProxy.ServiceStub.ServiceStub");
            mService = new SoftReference<MmsAppServiceProxy>(service);
        }

        public void startServiceByParam(AppServiceProxyParam param){
            Log.d(TAG, "MmsAppServiceProxy.startServiceByParam");
            Intent intent = param.getIntent();
            Log.d(TAG, "intent = " + intent);
            Context context = getApplicationContext();
            context.startService(intent);
        }

        @Override
        public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException {
            Log.d(TAG, "MmsAppServiceProxy.onTransact");
            int uid = Binder.getCallingUid();
            String[] pkgs = getPackageManager().getPackagesForUid(uid);
            Log.d(TAG, "MmsAppServiceProxy.onTransact, uid = " + uid + ", pkgs = " + pkgs);
            if (isMtkMessagePackage(pkgs, uid)) {
                Log.d(TAG, "MmsAppServiceProxy.onTransact call super");
                return super.onTransact(code, data, reply, flags);
            }
            Log.d(TAG, "MmsAppServiceProxy.onTransact return false");
            return false;
        }

        private boolean isMtkMessagePackage(String[] pkgs, int uid) {
            if (pkgs == null) {
                return false;
            }
            for (int i = 0; i < pkgs.length; i++) {
                Log.d(TAG, "package = " + pkgs[i]);
                if (MTK_MMS_PACKAGE.equals(pkgs[i])) {
                    return true;
                }
            }
            return false;
        }

    }
}

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

package com.android.mms.transaction;


import com.mediatek.mms.appcallback.IMmsCallbackService;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import java.lang.ref.SoftReference;
import com.mediatek.mms.appcallback.SmsEntry;
import mediatek.telephony.MtkSmsManager;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.mms.ext.IOpSmsSingleRecipientSenderExt;

public class MmsCallbackService extends Service {
    private final IBinder mBinder = new ServiceStub(this);
    private static final String TAG = "MmsCallbackService/Txn";

    public MmsCallbackService() {
        Log.d(TAG, "MmsCallbackService.MmsCallbackService");
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "MmsCallbackService.onCreate");
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "MmsCallbackService.onDestroy");
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "MmsCallbackService.onBind");
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "MmsCallbackService.onUnbind");
        //stopSelf(mServiceStartId);
        return true;
    }

    public class ServiceStub extends IMmsCallbackService.Stub {
        SoftReference<MmsCallbackService> mService;

        ServiceStub(MmsCallbackService service) {
            Log.d(TAG, "MmsCallbackService.ServiceStub.ServiceStub");
            mService = new SoftReference<MmsCallbackService>(service);
        }

        public void sendMultipartTextMessageWithEncodingType(SmsEntry param){
            Log.d(TAG, "MmsCallbackService sendMultipartTextMessageWithEncodingType");
            IOpSmsSingleRecipientSenderExt mOpSmsSenderExt = OpMessageUtils.getOpMessagePlugin().
                              getOpSmsSingleRecipientSenderExt();
            if (!mOpSmsSenderExt.sendMessage(getApplicationContext(),param.subId, param.destAddr, param.scAddr,
                param.parts, param.sentIntents, param.deliveryIntents)){
                Log.d(TAG, "send common");
                MtkSmsManager smsManager = MtkSmsManager.getSmsManagerForSubscriptionId(param.subId);
                smsManager.sendMultipartTextMessageWithEncodingType(param.destAddr, param.scAddr,
                    param.parts, param.encodingType, param.sentIntents, param.deliveryIntents);
            }
            Log.d(TAG, "MmsCallbackService sendMultipartTextMessageWithEncodingType end");
        }
    }
}

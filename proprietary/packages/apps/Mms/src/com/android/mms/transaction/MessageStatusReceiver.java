/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2007 Esmertec AG.
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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.util.MmsLog;

import com.mediatek.mms.appserviceproxy.AppServiceProxy;
import com.mediatek.mms.util.PermissionCheckUtil;

public class MessageStatusReceiver extends BroadcastReceiver {
    public static final String MESSAGE_STATUS_RECEIVED_ACTION =
            "com.android.mms.transaction.MessageStatusReceiver.MESSAGE_STATUS_RECEIVED";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        MmsLog.i("MessageStatusReceiver", "onReceive action: " + action);

        if (MESSAGE_STATUS_RECEIVED_ACTION.equals(intent.getAction())) {
            if (!PermissionCheckUtil.checkRequiredPermissions(context)) {
                MmsLog.d(MmsApp.TXN_TAG,
                        "MessageStatusReceiver: onReceive() no permissions return !!");
                return;
            }
            intent.setClassName(MmsConfig.MMS_APP_SERVICE_PACKAGE,
                    MmsConfig.MESSAGE_STATUS_SERVICE);
            /* start service*/
            context.startService(intent);
            //Context ct = context.getApplicationContext();
            //AppServiceProxy.getInstance(ct).startServiceByParam(ct, intent);
            /*
            new Thread(new Runnable() {
                public void run() {
                    MmsLog.d(MmsApp.TXN_TAG,
                        "MessageStatusReceiver: call startServiceByParam");
                    Context ct = context.getApplicationContext();
                    AppServiceProxy.getInstance(ct).startServiceByParam(ct, intent);
                }
            }).start();*/
       }
    }
}

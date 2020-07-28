/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
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
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Outbox;
import android.telephony.SmsManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.util.MmsLog;

import com.mediatek.mms.util.PermissionCheckUtil;
/**
 * SmsSystemEventReceiver receives the
 * {@link android.content.intent.ACTION_BOOT_COMPLETED},
 */
public class SmsSystemEventReceiver extends BroadcastReceiver {
    private static final String TAG = "SmsSystemEventReceiver";
    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        MmsLog.i(TAG, "SmsSystemEventReceiver, onReceive action: " + action);

        /// Add for runtime permission @{
        if (!PermissionCheckUtil.checkRequiredPermissions(context)) {
            MmsLog.d(MmsApp.LOG_TAG, "SmsSystemEventReceiver no permissions return !!");
            return;
        }
        /// @}

        if  (action != null
                && action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
            String newState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
            // if sim is plug out, then set outbox and queued sms to failed.
            if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(newState)) {
                // get the absent sub id
                final Context simContxt = context;
                final int subId = intent.getIntExtra(
                        PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);

                if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID
                        || subId < 0) {
                    return;
                }
                new Thread(new Runnable() {
                    public void run() {
                        String selection = " and sub_id=" + subId;
                        MmsLog.i(MmsApp.TXN_TAG, "SmsSystemEventReceiver" +
                                " onReceive() SIM absent, selection: " + selection);
                        int numMoved = moveOutboxAndQueuedMessagesToFailedBox(context, selection);
                        if (numMoved > 0) {
                            MessagingNotification.notifySendFailed(simContxt.getApplicationContext(), true);
                            // Called off of the UI thread so ok to block.
                            MessagingNotification.blockingUpdateNewMessageIndicator(
                                simContxt, MessagingNotification.THREAD_ALL, false, null);
                        }
                    }
                }).start();
            }
        }
    }

    private int moveOutboxAndQueuedMessagesToFailedBox(Context context,
                String selection) {
        int messageCount = 0;
        StringBuilder mStringBuilder = new StringBuilder();
        String where = "(" + Sms.TYPE + " = " + Sms.MESSAGE_TYPE_QUEUED +
                        " OR " + Sms.TYPE + " = " + Sms.MESSAGE_TYPE_OUTBOX + ")";
        if (selection != null) {
            where = where + selection;
        }
        Cursor cursor = SqliteWrapper.query(
                context,
                context.getContentResolver(),
                Sms.CONTENT_URI,
                new String[] { Sms._ID },
                where,
                null,
                null);
        if (cursor != null) {
            try {
                messageCount = cursor.getCount();
                mStringBuilder.append(Sms._ID + " IN(");
                while (cursor.moveToNext()) {
                    long id = cursor.getLong(0);
                    mStringBuilder.append(" " + id);
                    if (!cursor.isLast()) {
                        mStringBuilder.append(',');
                    }
                }
                mStringBuilder.append(')');
            } finally {
                cursor.close();
            }
        }

        if (messageCount > 0) {
            ContentValues values = new ContentValues(3);
            values.put(Sms.TYPE, Sms.MESSAGE_TYPE_FAILED);
            values.put(Sms.ERROR_CODE, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
            values.put(Sms.READ, Integer.valueOf(0));

            int numMoved = SqliteWrapper.update(
                    context, context.getContentResolver(),
                    Sms.CONTENT_URI, values, mStringBuilder.toString(), null);
            MmsLog.i(TAG, "moveOutboxAndQueuedMessagesToFailedBox, numMoved: " + numMoved);
            return numMoved;
        }
        return 0;
    }
}

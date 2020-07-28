/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
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

import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Handler;
import android.provider.Telephony.Mms;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.NotificationInd;
import com.mediatek.android.mms.pdu.MtkPduPersister;


public class DownloadManager {
    private static final String TAG = "DownloadManager";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = false;

    public static final int DEFERRED_MASK           = 0x04;

    public static final int STATE_UNKNOWN           = 0x00;
    public static final int STATE_UNSTARTED         = 0x80;
    public static final int STATE_DOWNLOADING       = 0x81;
    public static final int STATE_TRANSIENT_FAILURE = 0x82;
    public static final int STATE_PERMANENT_FAILURE = 0x87;
    public static final int STATE_PRE_DOWNLOADING   = 0x88;
    // TransactionService will skip downloading Mms if auto-download is off
    public static final int STATE_SKIP_RETRYING = 0x82; // M:[ALPS01772380] 0x89;

    private final Context mContext;
    private boolean mAutoDownload;
    private static DownloadManager sInstance;

    private DownloadManager(Context context) {
        mContext = context;
    }

    public static void init(Context context) {
        if (sInstance != null) {
            Log.w(TAG, "Already initialized.");
        }
        sInstance = new DownloadManager(context);
    }

    public static DownloadManager getInstance() {
        if (sInstance == null) {
            throw new IllegalStateException("Uninitialized.");
        }
        return sInstance;
    }

    private boolean isNotificationExprired(final Uri uri, int state) {
        // Notify user if the message has expired.
        try {
            ///M: modify for ALPS00437262 &{
            GenericPdu pdu = MtkPduPersister.getPduPersister(mContext.getApplicationContext()).load(uri);
            if (!(pdu instanceof NotificationInd)) {
                Log.e(TAG, "markState: pdu type is not NotificationInd.");
                return true;
            }
            NotificationInd nInd = (NotificationInd) pdu;
            /// @}
            if ((nInd.getExpiry() < System.currentTimeMillis() / 1000L)
                    && (state == STATE_DOWNLOADING || state == STATE_PRE_DOWNLOADING)) {
                MmsConfig.showToast(mContext, MmsConfig.SERVICE_MESSAGE_NOT_FOUND);
                SqliteWrapper.delete(mContext, mContext.getContentResolver(), uri, null, null);
                return true;
            }
        } catch (Exception e) {
            Log.e(TAG, e.getMessage(), e);
            return true;
        }
        return false;
    }

    public void markState(final Uri uri, int state) {
        // check if notification has exprited
        if(isNotificationExprired(uri, state)) {
            return;
        }

        // Notify user if downloading permanently failed.
        // change for op
        if (!mAutoDownload) {
            state |= DEFERRED_MASK;
        }
        // Use the STATUS field to store the state of downloading process
        // because it's useless for M-Notification.ind.
        ContentValues values = new ContentValues(1);
        values.put(Mms.STATUS, state);
        SqliteWrapper.update(mContext, mContext.getContentResolver(),
                    uri, values, null, null);
    }

    /// M:Code analyze 004,add for msim,mark download state,and update it into database @{
    public void markState(final Uri uri, int state, int subId) {
        // check if notification has exprited
        if(isNotificationExprired(uri, state)) {
            return;
        }

        // Notify user if downloading permanently failed.
        if (state == STATE_PERMANENT_FAILURE) {
            MmsConfig.showToast(mContext, MmsConfig.DOWNLOAD_LATER);
        } else if (!MmsConfig.isAutoDownload(subId)) {
            state |= DEFERRED_MASK;
        }

        // Use the STATUS field to store the state of downloading process
        // because it's useless for M-Notification.ind.
        ContentValues values = new ContentValues(1);
        values.put(Mms.STATUS, state);
        SqliteWrapper.update(mContext, mContext.getContentResolver(),
                    uri, values, null, null);
    }
    /// @}

    public int getState(Uri uri) {
        Cursor cursor = SqliteWrapper.query(mContext, mContext.getContentResolver(),
                            uri, new String[] {Mms.STATUS}, null, null, null);

        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    return cursor.getInt(0) & ~DEFERRED_MASK;
                }
            } finally {
                cursor.close();
            }
        }
        return STATE_UNSTARTED;
    }
}

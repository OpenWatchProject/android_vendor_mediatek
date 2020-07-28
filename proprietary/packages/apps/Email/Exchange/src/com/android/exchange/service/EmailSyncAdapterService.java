/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.exchange.service;

import android.content.AbstractThreadedSyncAdapter;
import android.content.ContentProviderClient;
import android.content.Context;
import android.content.Intent;
import android.content.SyncResult;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.text.format.DateUtils;
import android.util.Log;

import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.EmailContent.HostAuthColumns;
import com.android.emailcommon.provider.HostAuth;
import com.android.emailcommon.provider.Mailbox;
import com.android.emailcommon.service.EmailServiceStatus;
import com.android.emailcommon.service.IEmailService;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.emailcommon.utility.Utility;
import com.android.exchange.Eas;
import com.android.mail.utils.LogUtils;
import com.mediatek.exchange.smartpush.SmartPushService;

public class EmailSyncAdapterService extends AbstractSyncAdapterService {

    private static final String TAG = Eas.LOG_TAG;

    /// M: Eas account selection
    private static final String WHERE_PROTOCOL_EAS = HostAuthColumns.PROTOCOL + "=\""
            + HostAuth.LEGACY_SCHEME_EAS + "\"";

    private static final Object sSyncAdapterLock = new Object();
    private static AbstractThreadedSyncAdapter sSyncAdapter = null;

    public EmailSyncAdapterService() {
        super();
    }

    @Override
    public void onCreate() {
        LogUtils.v(TAG, "EmailSyncAdapterService.onCreate()");
        super.onCreate();
        //startService(new Intent(this, EmailSyncAdapterService.class));
        /// M: Start smart push service if any eas account existed. @{
        EmailAsyncTask.runAsyncParallel(new Runnable() {
            @Override
            public void run() {
                if (EmailContent.count(EmailSyncAdapterService.this,
                        HostAuth.CONTENT_URI, WHERE_PROTOCOL_EAS, null) > 0) {
            try {
                    startService(new Intent(EmailSyncAdapterService.this, SmartPushService.class));
            } catch (IllegalStateException e) {
                LogUtils.d(LogUtils.TAG, "killService, unable to start service due to illegal state");
                return;
            }
                    //startService(new Intent(EmailSyncAdapterService.this, SmartPushService.class));

                }
            }
        });
        /// @}
    }

    @Override
    public void onDestroy() {
        LogUtils.v(TAG, "EmailSyncAdapterService.onDestroy()");
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return super.onBind(intent);
    }

    @Override
    protected AbstractThreadedSyncAdapter getSyncAdapter() {
        synchronized (sSyncAdapterLock) {
            if (sSyncAdapter == null) {
                sSyncAdapter = new SyncAdapterImpl(this);
            }
            return sSyncAdapter;
        }
    }

    // TODO: Handle cancelSync() appropriately.
    private class SyncAdapterImpl extends AbstractThreadedSyncAdapter {
        public SyncAdapterImpl(Context context) {
            super(context, true /* autoInitialize */);
        }

        @Override
        public void onPerformSync(final android.accounts.Account acct, final Bundle extras,
                final String authority, final ContentProviderClient provider,
                final SyncResult syncResult) {
            if (LogUtils.isLoggable(TAG, Log.DEBUG)) {
                LogUtils.d(TAG, "onPerformSync email: %s, %s", acct.toString(), extras.toString());
            } else {
                LogUtils.i(TAG, "onPerformSync email: %s", extras.toString());
            }
            /** M: Don't try to perform any sync operation in the low storage mode @{ */
            if (Utility.mayLowStorage() ||
                    Utility.isStorageLowExt(getApplicationContext())) {
                LogUtils.d(TAG, "no need to perform email sync in the low storage mode");
                return;
            }
            /** M: @} */
            if (!waitForService()) {
                // The service didn't connect, nothing we can do.
                return;
            }

            // TODO: Perform any connectivity checks, bail early if we don't have proper network
            // for this sync operation.
            // FLAG: Do we actually need to do this? I don't think the sync manager will invoke
            // a sync if we don't have good network.

            final Account emailAccount = Account.restoreAccountWithAddress(
                    EmailSyncAdapterService.this, acct.name);
            if (emailAccount == null) {
                // There could be a timing issue with onPerformSync() being called and
                // the account being removed from our database.
                LogUtils.w(TAG,
                        "onPerformSync() - Could not find an Account, skipping email sync.");
                return;
            }

            // Push only means this sync request should only refresh the ping (either because
            // settings changed, or we need to restart it for some reason).
            final boolean pushOnly = Mailbox.isPushOnlyExtras(extras);

            if (pushOnly) {
                LogUtils.d(TAG, "onPerformSync: mailbox push only");
                try {
                    mEasService.pushModify(emailAccount.mId);
                    return;
                } catch (final RemoteException re) {
                    LogUtils.e(TAG, re, "While trying to pushModify within onPerformSync");
                    // TODO: how to handle this?
                }
                return;
            } else {
                try {
                    final int result = mEasService.sync(emailAccount.mId, extras);
                    writeResultToSyncResult(result, syncResult);
                    if (syncResult.stats.numAuthExceptions > 0 &&
                            result != EmailServiceStatus.PROVISIONING_ERROR) {
                        showAuthNotification(emailAccount.mId, emailAccount.mEmailAddress);
                    }
                } catch (RemoteException e) {
                     LogUtils.e(TAG, e, "While trying to pushModify within onPerformSync");
                }
            }

            LogUtils.d(TAG, "onPerformSync email: finished");
        }
    }

    /**
     * M: Get the syncAdapter for test
     * @return the syncAdapter
     */
    AbstractThreadedSyncAdapter getTestSyncAdapter(Context context) {
        return new SyncAdapterImpl(context);
    }
}

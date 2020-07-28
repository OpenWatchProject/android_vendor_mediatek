/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.exchange;

import java.lang.Thread.UncaughtExceptionHandler;

import android.app.Application;
import android.content.Context;

import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.MailboxUtilities;
import com.android.emailcommon.provider.ProviderUnavailableException;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.emailcommon.utility.Utility;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;

public class Exchange extends Application {
    public static final int NO_BSK_MAILBOX = -1;
    /// M: The bad sync key mailbox id. At present just suppose at
     // most only 1 mailbox may occurs bad sync key at the same time
    public static long sBadSyncKeyMailboxId = NO_BSK_MAILBOX;
    private UncaughtHandler mExceptionHandler;
    private UncaughtExceptionHandler mDefaultExceptionHandler;

    static {
        LogTag.setLogTag(Eas.LOG_TAG);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        /** M: Stop the exchange service in the low storage mode @{ */
        if (Utility.mayLowStorage() ||
                   Utility.isStorageLowExt(getApplicationContext())) {
            LogUtils.d(LogTag.getLogTag(), "Stops service in the low storage mode");
            //android.os.Process.killProcess(android.os.Process.myPid());
            //System.exit(1);
            return;
        }
        /** M: @} */
        EmailContent.init(this);
        /** M: Use for catch UncaughtException in thread @{ */
        mExceptionHandler = new UncaughtHandler();
        mDefaultExceptionHandler = Thread.getDefaultUncaughtExceptionHandler();
        Thread.setDefaultUncaughtExceptionHandler(mExceptionHandler);
        /** @} */

        /// M: Update mailbox if parentKeys not match with parentServerId.
        /// Better move to backgound, it may block UIThread when EmailProvider is busy. @{
        EmailAsyncTask.runAsyncParallel(new Runnable() {
            @Override
            public void run() {
                try {
                    getContentResolver().call(EmailContent.CONTENT_URI,
                            MailboxUtilities.FIX_PARENT_KEYS_METHOD, "", null);
                } catch (IllegalArgumentException e) {
                    // If there is no Email provider (which happens if eg the
                    // Email app is disabled), ignore.
                }
            }
        });
        /// @}

        /** M: This is to check if the bad sync key had ever happened and its recovery process was
            halted by Exchange process crash or device rebooting etc. @{ */
        ExchangePreferences pref = ExchangePreferences.getPreferences(this);
        sBadSyncKeyMailboxId = pref.getBadSyncKeyMailboxId();
        if (sBadSyncKeyMailboxId != NO_BSK_MAILBOX) {
            LogUtils.i(Eas.BSK_TAG, "Unfinished Bad sync key recovery detected," +
                    " mailbox id: " + sBadSyncKeyMailboxId);
        }
        /** @} */
    }

    /**
     * M:The class is used to catch UncaughtException in thread,mainly
     * ProviderUnavailableException. The exception don't need to be visible for
     * user,so the operation is only mainly to kill the thread which being
     * terminated by providerUnavailableException and the exception dialog will
     * not be displayed.
     */
    private class UncaughtHandler implements Thread.UncaughtExceptionHandler {

        @Override
        public void uncaughtException(Thread thread, Throwable ex) {
            LogUtils.e(LogTag.getLogTag(), "uncaughtException :", ex);
            if (ex instanceof ProviderUnavailableException) {
                android.os.Process.killProcess(android.os.Process.myPid());
                System.exit(1);
            } else {
                mDefaultExceptionHandler.uncaughtException(thread, ex);
            }
        }
    }
}

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

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Inbox;
import android.util.Log;

public class SmsMessageSender {
    protected final Context mContext;
    protected final int mNumberOfDests;
    private final String[] mDests;
    protected final String mMessageText;
    protected final String mServiceCenter;
    protected final long mThreadId;
    protected int mSubId;
    protected long mTimestamp;
    private static final String TAG = "SmsMessageSender";

    private static final String[] SERVICE_CENTER_PROJECTION = new String[] {
        Sms.Conversations.REPLY_PATH_PRESENT,
        Sms.Conversations.SERVICE_CENTER,
    };

    private static final int COLUMN_REPLY_PATH_PRESENT = 0;
    private static final int COLUMN_SERVICE_CENTER     = 1;

    public SmsMessageSender(Context context, String[] dests, String msgText, long threadId,
            int subId) {
        mContext = context;
        mMessageText = msgText;
        if (dests != null) {
            mNumberOfDests = dests.length;
            mDests = new String[mNumberOfDests];
            System.arraycopy(dests, 0, mDests, 0, mNumberOfDests);
        } else {
            mNumberOfDests = 0;
            mDests = null;
        }
        mTimestamp = System.currentTimeMillis();
        mThreadId = threadId;
        mSubId = subId;
        mServiceCenter = getOutgoingServiceCenter(mThreadId);
    }

    /**
     * Get the service center to use for a reply.
     *
     * The rule from TS 23.040 D.6 is that we send reply messages to
     * the service center of the message to which we're replying, but
     * only if we haven't already replied to that message and only if
     * <code>TP-Reply-Path</code> was set in that message.
     *
     * Therefore, return the service center from the most recent
     * message in the conversation, but only if it is a message from
     * the other party, and only if <code>TP-Reply-Path</code> is set.
     * Otherwise, return null.
     */
    private String getOutgoingServiceCenter(long threadId) {
        Cursor cursor = null;

        try {
            cursor = SqliteWrapper.query(mContext, mContext.getContentResolver(),
                            Inbox.CONTENT_URI, SERVICE_CENTER_PROJECTION,
                            "thread_id = " + threadId, null, "date DESC");

            if ((cursor == null) || !cursor.moveToFirst()) {
                return null;
            }

            boolean replyPathPresent = (1 == cursor.getInt(COLUMN_REPLY_PATH_PRESENT));
            return replyPathPresent ? cursor.getString(COLUMN_SERVICE_CENTER) : null;
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }
}

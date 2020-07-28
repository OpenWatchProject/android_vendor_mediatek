/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.cb.cbmsg;

import android.content.AsyncQueryHandler;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.R;
import com.android.mms.util.MmsLog;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import mediatek.telephony.MtkTelephony;

/**
 * M:
 * An interface for finding information about conversations and/or creating new
 * ones.
 */
public class CbMessage {
    private static final String TAG = "MmsCB/message";
    private static final boolean DEBUG = false;

    // Uri.parse("content://cb/messages");
    private static final Uri MESSAGE_URI = MtkTelephony.SmsCb.CONTENT_URI;
    // Query all URI
    private static final Uri ALL_MESSAGE_URI = MtkTelephony.SmsCb.CONTENT_URI;

    /**
     * Uri.parse("content://cb/messages");
     * Query messge URI by thread_id
     */
    public static final Uri THREAD_MESSAGE_URI = Uri
            .parse("content://cb/messages/#");

    private static final String[] ALL_MESSAGE_PROJECTION = { "_id",
            MtkTelephony.SmsCb.SUBSCRIPTION_ID, MtkTelephony.SmsCb.CHANNEL_ID, //fixme
            MtkTelephony.SmsCb.BODY, MtkTelephony.SmsCb.READ,
            MtkTelephony.SmsCb.DATE, MtkTelephony.SmsCb.THREAD_ID };
    private static final String[] ALL_CHANNEL_PROJECTION = { "_id",
            MtkTelephony.SmsCb.CbChannel.NAME,
            MtkTelephony.SmsCb.CbChannel.NUMBER };

    private static final int ID = 0;
    private static final int SUB_ID = 1;
    private static final int CHANNEL_ID = 2;
    private static final int BODY = 3;
    private static final int READ = 4;
    private static final int DATE = 5;
    private static final int THREAD_ID = 6;

    private final Context mContext;

    /**
     * The thread ID of this CBMessages. Can be zero in the case of a new
     * CBMessages where the recipient set is changing as the user // types and
     * we have not hit the database yet to create a thread.
     */
    private long mId;
    private int mSubId;
    private int mChannelId;
    private String mChannelName;
    private int mRead;
    // The last update time.
    private long mDate;
    // Text of the most recent message.
    private String mBody;
    // Number of messages.
    private int mAddressId;
    private int mThreadId;

    private static ContentValues sReadContentValues;
    private static boolean sLoadingMessages;

    private CbMessage(Context context) {
        mContext = context;
        mId = 0;
    }

    private CbMessage(Context context, long id, boolean allowQuery) {
        mContext = context;
        if (!loadFromId(id, allowQuery)) {
            mId = 0;
        }
    }

    private CbMessage(Context context, Cursor cursor, boolean allowQuery) {
        mContext = context;
        fillFromCursor(context, this, cursor, allowQuery);
    }

    /**
     * Create a new CBMessages with no recipients. {@link setRecipients} can be
     * called as many times as you like; the CBMessages will not be created in
     * the database until {@link ensureThreadId} is called.
     */
    public static CbMessage createNew(Context context) {
        return new CbMessage(context);
    }

    /**
     * Returns a temporary CBMessages (not representing one on disk) wrapping
     * the contents of the provided cursor. The cursor should be the one
     * returned to your AsyncQueryHandler passed in to {@link startQueryForAll}.
     * The recipient list of this CBMessages can be empty if the results were
     * not in cache.
     */
    // TODO: check why can't load a cached CBMessages object here.
    public static CbMessage from(Context context, Cursor cursor) {
        return new CbMessage(context, cursor, false);
    }

    private void buildReadContentValues() {
        if (sReadContentValues == null) {
            sReadContentValues = new ContentValues(1);
            sReadContentValues.put("read", 1);
        }
    }

    /**
     * Returns a content:// URI referring to this CBMessages, or null if it does
     * not exist on disk yet.
     */
    public synchronized Uri getUri() {
        if (mThreadId <= 0) {
            return null;
        }

        return ContentUris.withAppendedId(MESSAGE_URI, mThreadId);
    }

    /**
     * Return the Uri for all messages in the given thread ID.
     *
     * @deprecated
     */
    public static Uri getUri(long threadId) {
        /**
         * TODO: Callers using this should really just have a CBMessages
         * and call getUri() on it, but this guarantees no blocking.
         */
        return ContentUris.withAppendedId(MESSAGE_URI, threadId);
    }

    /**
     * Returns the thread ID of this CBMessages. Can be zero if
     * {@link ensureThreadId} has not been called yet.
     */
    public synchronized long getMessageId() {
        return mId;
    }

    public synchronized long getThreadId() {
        return mThreadId;
    }

    public synchronized int getChannelId() {
        return mChannelId;
    }

    public synchronized void setChannelName(String name) {
        mChannelName = name;
    }

    public synchronized String getChannelName() {
        /// TODO  if No channel Name query! Maybe need remove
        if (mChannelName == null) {
            mChannelName = CbMessage.getCBChannelName(mSubId, mChannelId);
        }
        return mChannelName;
    }

    public synchronized String getDisplayName() {
        String displayName = getChannelName();
        if (displayName.equals(mContext.getString(R.string.cb_default_channel_name))) {
            displayName = displayName + " " + getChannelId();
        }
        return displayName;
    }

    public static synchronized String getCBChannelName(int subId, int channelId) {
        Uri uri = null;
        if (subId > 0) {
            uri = Uri.parse("content://cb/channel").buildUpon()
                    .appendQueryParameter(PhoneConstants.SUBSCRIPTION_KEY, String.valueOf(subId))
                    .build();
            MmsLog.dpi(TAG, "uri = " + uri);
        } else {
            uri = Uri.parse("content://cb/channel");
        }

        Cursor c = MmsApp.getApplication().getApplicationContext().getContentResolver().query(uri,
                ALL_CHANNEL_PROJECTION,
                MtkTelephony.SmsCb.CbChannel.NUMBER + " = ?",
                new String[]{Integer.toString(channelId)},
                null);
        try {
            if (c == null || c.getCount() == 0) {
                return MmsApp.getApplication()
                        .getApplicationContext().getString(R.string.cb_default_channel_name);
            } else {
                c.moveToFirst();
                return c.getString(1);
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }
    }

    public synchronized void clearThreadId() {
        // remove ourself from the cache
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            LogTag.debug("clearThreadId old threadId was: " + mId
                    + " now zero");
        }
        Cache.remove(mId);

        mId = 0;
    }

    /**
     * Returns the time of the last update to this CBMessages in milliseconds,
     * on the {@link System.currentTimeMillis} timebase.
     */
    public synchronized long getDate() {
        return mDate;
    }

    /**
     * Returns a snippet of text from the most recent message in the CBMessages.
     */
    public synchronized String getBody() {
        return mBody;
    }

    /**
     * The primary key of a CBMessages is its recipient set; override equals()
     * and hashCode() to just pass through to the internal recipient sets.
     */
    @Override
    public synchronized boolean equals(Object obj) {
        try {
            CbMessage other = (CbMessage) obj;
            return (this.mChannelId == other.mChannelId
                    && this.mBody.equals(other.mBody) && this.mDate == other.mDate);
        } catch (ClassCastException e) {
            return false;
        }
    }

    @Override
    public synchronized int hashCode() {
        return (mBody + String.valueOf(mDate) + String.valueOf(mChannelId)).hashCode();
    }

    /**
     * Remove any obsolete CBMessagess sitting around on disk.
     *
     * @deprecated
     */
    public static void cleanup(Context context) {
        /// M: TODO DO nothing here.
    }

    /**
     * Start a query for all CBMessagess in the database on the specified
     * AsyncQueryHandler.
     *
     * @param handler
     *            An AsyncQueryHandler that will receive onQueryComplete upon
     *            completion of the query
     * @param token
     *            The token that will be passed to onQueryComplete
     */
    public static void startQueryForAll(AsyncQueryHandler handler, int token) {
        handler.cancelOperation(token);
        handler.startQuery(token, null, ALL_MESSAGE_URI, ALL_MESSAGE_PROJECTION,
                null, null, MtkTelephony.SmsCb.DEFAULT_SORT_ORDER);
    }

    public static void startQueryForThreadId(AsyncQueryHandler handler,
            long threadId, int token) {
        handler.cancelOperation(token);
        handler.startQuery(token, null, ContentUris
                .withAppendedId(THREAD_MESSAGE_URI, threadId),
                ALL_MESSAGE_PROJECTION, null, null, null);
    }

    /**
     * Start a delete of the CBMessages with the specified thread ID.
     *
     * @param handler
     *            An AsyncQueryHandler that will receive onDeleteComplete upon
     *            completion of the CBMessages being deleted
     * @param token
     *            The token that will be passed to onDeleteComplete
     * @param deleteAll
     *            Delete the whole thread including locked messages
     * @param threadId
     *            Thread ID of the CBMessages to be deleted
     */
    public static void startDelete(AsyncQueryHandler handler, int token,
            boolean deleteAll, long threadId) {
        Uri uri = ContentUris.withAppendedId(MESSAGE_URI, threadId);
        handler.startDelete(token, null, uri, null, null);
    }

    /**
     * Start deleting all CBMessagess in the database.
     *
     * @param handler
     *            An AsyncQueryHandler that will receive onDeleteComplete upon
     *            completion of all CBMessagess being deleted
     * @param token
     *            The token that will be passed to onDeleteComplete
     * @param deleteAll
     *            Delete the whole thread including locked messages
     */
    public static void startDeleteAll(AsyncQueryHandler handler, int token,
            boolean deleteAll) {
        handler.startDelete(token, null, MESSAGE_URI, null, null);
    }

    /**
     * Fill the specified CBMessages with the values from the specified cursor,
     * possibly setting recipients to empty if {@value allowQuery} is false and
     * the recipient IDs are not in cache. The cursor should be one made via
     * {@link startQueryForAll}.
     */
    private static void fillFromCursor(Context context, CbMessage message,
            Cursor c, boolean allowQuery) {
        synchronized (message) {
            message.mId = c.getLong(ID);
            message.mSubId = c.getInt(SUB_ID);
            message.mChannelId = c.getInt(CHANNEL_ID);
            message.mBody = c.getString(BODY);
            message.mRead = c.getInt(READ);
            message.mDate = c.getLong(DATE);
            message.mThreadId = c.getInt(THREAD_ID);
            message.mChannelName = message.getChannelName();
        }
    }

    /**
     * Private cache for the use of the various forms of CBMessages.get.
     */
    private static class Cache {
        private static Cache sInstance = new Cache();

        static Cache getInstance() {
            return sInstance;
        }

        private final HashSet<CbMessage> mCache;

        private Cache() {
            mCache = new HashSet<CbMessage>(10);
        }

        /**
         * Return the CBMessages with the specified thread ID, or null if it's
         * not in cache.
         */
        static CbMessage get(long messageId) {
            synchronized (sInstance) {
                if (DEBUG) {
                    LogTag.debug("CBMessages get with threadId: " + messageId);
                }
                dumpCache();
                for (CbMessage c : sInstance.mCache) {
                    if (DEBUG) {
                        LogTag.debug("CBMessages get() threadId: " + messageId
                                + " c.getThreadId(): " + c.getMessageId());
                    }
                    if (c.getMessageId() == messageId) {
                        return c;
                    }
                }
            }
            return null;
        }

        /**
         * Put the specified CBMessages in the cache. The caller should not
         * place an already-existing CBMessages in the cache, but rather update
         * it in place.
         */
        static void put(CbMessage c) {
            synchronized (sInstance) {
                /**
                 * We update cache entries in place so people with long-
                 * held references get updated.
                 */
                if (DEBUG) {
                    LogTag.debug("CBMessages c: " + c + " put with threadid: "
                            + c.getMessageId() + " c.hash: " + c.hashCode());
                    dumpCache();
                }

                if (sInstance.mCache.contains(c)) {
                    throw new IllegalStateException("cache already contains "
                            + c + " threadId: " + c.mThreadId);
                }
                sInstance.mCache.add(c);
            }
        }

        static void remove(long messageId) {
            if (DEBUG) {
                LogTag.debug("remove threadid: " + messageId);
                dumpCache();
            }
            for (CbMessage c : sInstance.mCache) {
                if (c.getMessageId() == messageId) {
                    sInstance.mCache.remove(c);
                    return;
                }
            }
        }

        static void dumpCache() {
            if (DEBUG) {
                synchronized (sInstance) {
                    LogTag.debug("CBMessages dumpCache: ");
                    for (CbMessage c : sInstance.mCache) {
                        LogTag.debug("   c: " + c + " c.getThreadId(): "
                                + c.getMessageId() + " hash: " + c.hashCode());
                    }
                }
            }
        }

        /**
         * Remove all CBMessagess from the cache that are not in the provided
         * set of thread IDs.
         */
        static void keepOnly(Set<Long> messageIds) {
            synchronized (sInstance) {
                Iterator<CbMessage> iter = sInstance.mCache.iterator();
                CbMessage c = null;
                while (iter.hasNext()) {
                    c = iter.next();
                    if (!messageIds.contains(c.getMessageId())) {
                        iter.remove();
                    }
                }
            }
        }
    }

    /**
     * Set up the CBMessages cache. To be called once at application startup
     * time.
     */
    public static void init(final Context context) {
        new Thread(new Runnable() {
            public void run() {
                cacheAllMessages(context);
            }
        }).start();
    }

    /**
     * Are we in the process of loading and caching all the threads?.
     */
    public static boolean loadingMessages() {
        synchronized (Cache.getInstance()) {
            return sLoadingMessages;
        }
    }

    private static void cacheAllMessages(Context context) {
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            LogTag.debug("[CBMessages] cacheAllThreads");
        }
        synchronized (Cache.getInstance()) {
            if (sLoadingMessages) {
                return;
            }
            sLoadingMessages = true;
        }

        /**
         * Keep track of what threads are now on disk so we
         * can discard anything removed from the cache.
         */
        HashSet<Long> messagesOnDisk = new HashSet<Long>();

        /// Query for all CBMessagess.
        Cursor c = context.getContentResolver().query(ALL_MESSAGE_URI,
                ALL_MESSAGE_PROJECTION, null, null, null);
        try {
            if (c != null) {
                long messageId = 0L;
                while (c.moveToNext()) {
                    messageId = c.getLong(ID);
                    messagesOnDisk.add(messageId);

                    /// Try to find this thread ID in the cache.
                    CbMessage message;
                    synchronized (Cache.getInstance()) {
                        message = Cache.get(messageId);
                    }

                    if (message == null) {
                        /**
                         * Make a new CBMessages and put it in
                         * the cache if necessary.
                         */
                        message = new CbMessage(context, c, true);
                        try {
                            synchronized (Cache.getInstance()) {
                                Cache.put(message);
                            }
                        } catch (IllegalStateException e) {
                            LogTag
                                    .error("Tried to add duplicate CBMessages to Cache");
                        }
                    } else {
                        /**
                         * Or update in place so people with references
                         * to CBMessagess get updated too.
                         */
                        fillFromCursor(context, message, c, true);
                    }
                }
            }
        } finally {
            if (c != null) {
                c.close();
            }
            synchronized (Cache.getInstance()) {
                sLoadingMessages = false;
            }
        }

        // Purge the cache of threads that no longer exist on disk.
        Cache.keepOnly(messagesOnDisk);
    }

    private boolean loadFromId(long messageId, boolean allowQuery) {
        Cursor c = mContext.getContentResolver().query(ALL_MESSAGE_URI,
                ALL_MESSAGE_PROJECTION, "_id=" + Long.toString(messageId), null,
                null);
        try {
            if (c != null) {
                if (c.moveToFirst()) {
                    fillFromCursor(mContext, this, c, allowQuery);
                } else {
                    LogTag.error("loadFromThreadId: Can't find thread ID "
                            + messageId);
                    return false;
                }
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }
        return true;
    }

    public int getSubId() {
        return mSubId;
    }
}

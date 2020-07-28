package com.android.mms.data;

import android.app.Activity;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.Manifest;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.BaseColumns;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Conversations;
import android.provider.Telephony.Threads;
import android.provider.Telephony.ThreadsColumns;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.util.Log;

import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.R;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.MmsLog;
import com.google.android.mms.util.PduCache;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.callback.IConversationCallback;
import com.mediatek.mms.ipmessage.IIpContactExt;
import com.mediatek.mms.ipmessage.IIpConversationExt;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.wappush.WapPushMessagingNotification;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import mediatek.telephony.MtkTelephony;
import mediatek.telephony.MtkTelephony.WapPush;

//import android.provider.Telephony.WapPush;

/**
 * An interface for finding information about conversations and/or creating new ones.
 */
public class Conversation implements IConversationCallback {
    private static final String TAG = "Mms/conv";
    private static final boolean DEBUG = false;

    public static final Uri sAllThreadsUri =
        Threads.CONTENT_URI.buildUpon().appendQueryParameter("simple", "true").build();

    public static final Uri sAllUnreadMessagesUri = Uri.parse("content://mms-sms/unread_count");

    public static final String[] ALL_THREADS_PROJECTION = {
        Threads._ID, Threads.DATE, Threads.MESSAGE_COUNT, Threads.RECIPIENT_IDS,
        Threads.SNIPPET, Threads.SNIPPET_CHARSET, Threads.READ, Threads.ERROR,
        Threads.HAS_ATTACHMENT
        /// M:
        , Threads.TYPE , MtkTelephony.MtkThreads.READ_COUNT , MtkTelephony.MtkThreads.STATUS
    };

    public static final String[] UNREAD_PROJECTION = {
        Threads._ID,
        Threads.READ
    };

    private static final String UNREAD_SELECTION = "(read=0 OR seen=0)";

    private static final String[] SEEN_PROJECTION = new String[] {
        "seen"
    };

    private static final int ID             = 0;
    private static final int DATE           = 1;
    private static final int MESSAGE_COUNT  = 2;
    private static final int RECIPIENT_IDS  = 3;
    private static final int SNIPPET        = 4;
    private static final int SNIPPET_CS     = 5;
    private static final int READ           = 6;
    private static final int ERROR          = 7;
    private static final int HAS_ATTACHMENT = 8;

    /// M: Code analyze 001, For new feature ALPS00131956, wappush: add new params . @{
    private static final int TYPE           = 9;
    /// @}
    private static final int READCOUNT      = 10;
    /// M: Code analyze 058, For bug ALPS00091288, to solve Sent MMS
    /// toself and receive the MMS phone exception happened . @}
    private static final int STATUS         = 11;
    /// @}

    private final Context mContext;

    // The thread ID of this conversation.  Can be zero in the case of a
    // new conversation where the recipient set is changing as the user
    // types and we have not hit the database yet to create a thread.
    private long mThreadId;

    private ContactList mRecipients;    // The current set of recipients.
    private long mDate;                 // The last update time.
    private int mMessageCount;          // Number of messages.
    private String mSnippet;            // Text of the most recent message.
    private boolean mHasUnreadMessages; // True if there are unread messages.
    private boolean mHasAttachment;     // True if any message has an attachment.
    private boolean mHasError;          // True if any message is in an error state.
    private boolean mIsChecked;         // True if user has selected the conversation for a
                                        // multi-operation such as delete.

    private static ContentValues sReadContentValues;
    private static boolean sLoadingThreads;
    private static boolean sDeletingThreads;
    private static Object sDeletingThreadsLock = new Object();
    private boolean mMarkAsReadBlocked;
    private boolean mMarkAsReadWaiting;

    private Object mMarkAsBlockedSyncer = new Object();

    /// M:
    private static final String UNSEEN_SELECTION = "seen=0";

    private static boolean sIsInitialized = false;
    /// M: Code analyze 061, For bug ALPS00257986, click the import information failure . @{
    private static final int UPDATE_LIMIT = 50;
    private static final String UNREAD_SELECTION_WITH_LIMIT = "(read=0 OR seen=0) "
            + "and _id in (select _id from "
            + "( select _id from sms where (read=0 OR seen=0) and thread_id=? union "
            + " select _id from pdu where (read=0 OR seen=0) and thread_id=? union "
            + " select _id from cellbroadcast where (read=0 OR seen=0) and thread_id=?"
            + ") order by _id limit 0," + UPDATE_LIMIT + " )";
    /// @}

    /// M: Code analyze 060, For bug ALPS00272115, add for WappushNotification
    /// canceled with other message . @{
    public static final int MARK_ALL_MESSAGE_AS_SEEN                   = 0;
    public static final int MARK_ALL_MESSAGE_AS_SEEN_WITHOUT_WAPPUSH   = 1;

    private boolean mIsDoingMarkAsRead = false;
    /// @
    /// M: Code analyze 062, For bug ALPS00041233, Gemini enhancment . @{
    private static final String[] ALL_ADDRESS_PROJECTION = { "_id",
        MtkTelephony.SmsCb.CanonicalAddressesColumns.ADDRESS };
    private static final Uri ADDRESS_ID_URI = Uri.parse("content://cb/addresses/#");
    // add for cb,Number of messages.
    private int mAddressId;
    // Text of the most recent message.
    private int mChannelId = -1;
    /// @}

    public static final int TYPE_MMSSMS   = 0;

    private static final String READ_SELECTION = "(read=1)";

    // Number of unread message.
    private int mUnreadMessageCount;
    /// M: Code analyze 058, For bug ALPS00091288, to solve Sent MMS
    /// toself and receive the MMS phone exception happened . @}
    private int mMessageStatus;
    /// @}

    /// M: Code analyze 001, For new feature ALPS00131956, wappush: add new params . @{
    private int mType;
    /// @}

    private boolean mComposeIsPause = false;

    /// M: Code analyze 063, For bug ALPS00331731, to resolveReject incoming call
    /// with a SMS]There is no recipient after we tap "Custom message" . @{
    private static boolean sNeedCacheConv = true;
    /// @}


    /// M: add for ipmessage
    private boolean mNotificationEnable;
    private long mThreadSettingsId;

    private static HashMap<Integer, Runnable> sQueryExtendMap = new HashMap<Integer, Runnable>();
    private static HashMap<Integer, Runnable> sQueryMap = new HashMap<Integer, Runnable>();

    //add for ipmessage
    public IIpConversationExt mIpConv;

    //remove the DraftCache, save the draft state here
    private boolean mHasDraft = false;

    private Conversation(Context context) {
        mContext = context;
        mIpConv = getIpConv(mContext);
        mRecipients = new ContactList();
        mThreadId = 0;
    }

    private Conversation(Context context, long threadId, boolean allowQuery) {
        if (DEBUG) {
            Log.v(TAG, "Conversation constructor threadId: " + threadId);
        }
        mContext = context;
        mIpConv = getIpConv(mContext);
        /// M: @{
        MmsLog.d(TAG, "new Conversation.loadFromThreadId(threadId, allowQuery): threadId = "
                + threadId + "allowQuery = " + allowQuery);
        /// @}
        if (!loadFromThreadId(threadId, allowQuery)) {
            mRecipients = new ContactList();
            mThreadId = 0;
        }
    }

    private Conversation(Context context, Cursor cursor, boolean allowQuery) {
        if (DEBUG) {
            Log.v(TAG, "Conversation constructor cursor, allowQuery: " + allowQuery);
        }
        mContext = context;
        mIpConv = getIpConv(mContext);
        fillFromCursor(context, this, cursor, allowQuery);
    }

    /**
     * Create a new conversation with no recipients.  {@link #setRecipients} can
     * be called as many times as you like; the conversation will not be
     * created in the database until {@link #ensureThreadId} is called.
     */
    public static Conversation createNew(Context context) {
        return new Conversation(context);
    }

    /**
     * Find the conversation matching the provided thread ID.
     */
    public static Conversation get(Context context, long threadId, boolean allowQuery) {
        /*if (DEBUG) {
            Log.v(TAG, "Conversation get by threadId: " + threadId);
        }*/
        Conversation conv = Cache.get(threadId);
        if (conv != null) {
            /// M: Fix CR : ALPS00998611 & ALPS00948057
            /// if the conversation has draft, when call composer from notification(coming a sms),
            /// the message count is 0, show recipientseditor,so will not query from composer.
            /// then the received sms will not show in the composer
            /// The change may be cause a ANR, because query in the main thread @{
            if (conv.getMessageCount() == 0) {
                if (!conv.loadFromThreadId(threadId, allowQuery)) {
                    conv.mMessageCount = 0;
                }
            }
            /// @}
            return conv;
        }

        conv = new Conversation(context, threadId, allowQuery);
        try {
            Cache.put(conv);
        } catch (IllegalStateException e) {
            MmsLog.dpi(TAG, "Tried to add duplicate Conversation to Cache (from threadId): "
                                         + conv);
            if (!Cache.replace(conv)) {
                MmsLog.dpi(TAG, "get by threadId cache.replace failed on " + conv);
            }
        }
        return conv;
    }

    /**
     * Find the conversation matching the provided recipient set.
     * When called with an empty recipient list, equivalent to {@link #createNew}.
     */
    public static Conversation get(Context context, ContactList recipients,
            boolean allowQuery, int type) {
        if (DEBUG) {
            MmsLog.vpi(TAG, "Conversation get by recipients: " + recipients.serialize());
        }
        // If there are no recipients in the list, make a new conversation.
        if (recipients.size() < 1) {
            return createNew(context);
        }

        Conversation conv = Cache.get(recipients, type);
        if (conv != null) {
            return conv;
        }

        long threadId = getOrCreateThreadId(context, recipients);
        conv = new Conversation(context, threadId, allowQuery);
        Log.d(TAG, "Conversation.get: created new conversation " + /*conv.toString()*/ "xxxxxxx");

        if (!conv.getRecipients().equals(recipients)) {
            LogTag.debug(TAG, "Conversation.get:new conv's recipient don't match input recpients "
                    + /*recipients*/ "xxxxxxx");
        }

        try {
            Cache.put(conv);
        } catch (IllegalStateException e) {
            MmsLog.dpi(TAG, "Tried to add dup Conversation to Cache (from recipients): " + conv);
            if (!Cache.replace(conv)) {
                MmsLog.dpi(TAG, "get by recipients cache.replace failed on " + conv);
            }
        }

        return conv;
    }

    /**
     * Find the conversation matching in the specified Uri.  Example
     * forms: {@value content://mms-sms/conversations/3} or
     * {@value sms:+12124797990}.
     * When called with a null Uri, equivalent to {@link #createNew}.
     */
    public static Conversation get(Context context, Uri uri, boolean allowQuery, int type) {
        if (DEBUG) {
            MmsLog.vpi(TAG, "Conversation get by uri: " + uri);
        }
        if (uri == null) {
            return createNew(context);
        }

        if (DEBUG) {
            MmsLog.vpi(TAG, "Conversation get URI: " + uri);
        }

        // Handle a conversation URI
        if (uri.getPathSegments().size() >= 2) {
            try {
                /// M: Google default.
                //long threadId = Long.parseLong(uri.getPathSegments().get(1));
                /// M: @{
                String threadIdStr = uri.getPathSegments().get(1);
                threadIdStr = threadIdStr.replaceAll("-", "");
                long threadId = Long.parseLong(threadIdStr);
                /// @}
                if (DEBUG) {
                    Log.v(TAG, "Conversation get threadId: " + threadId);
                }
                return get(context, threadId, allowQuery);
            } catch (NumberFormatException exception) {
                MmsLog.epi(TAG, "Invalid URI: " + uri);
            }
        }

        String recipients = PhoneNumberUtils.replaceUnicodeDigits(getRecipients(uri))
                .replace(',', ';');
        /// M: change google default.
        return get(context,
            ContactList.getByNumbers(
                context,
                recipients,
                false /* don't block */,
                true /* replace number */
                ),
            allowQuery, type);
    }

    /**
     * Returns true if the recipient in the uri matches the recipient list in this
     * conversation.
     */
    public boolean sameRecipient(Uri uri, Context context) {
        int size = mRecipients.size();
        if (size > 1) {
            return false;
        }
        if (uri == null) {
            return size == 0;
        }
        ContactList incomingRecipient = null;
        if (uri.getPathSegments().size() >= 2) {
            // it's a thread id for a conversation
            Conversation otherConv = get(context, uri, false, 0);
            if (otherConv == null) {
                return false;
            }
            incomingRecipient = otherConv.mRecipients;
        } else {
            String recipient = getRecipients(uri);
            incomingRecipient = ContactList.getByNumbers(recipient,
                    false /* don't block */, false /* don't replace number */);
        }
        if (DEBUG) {
            MmsLog.vpi(TAG, "sameRecipient incomingRecipient: " + incomingRecipient
                    + " mRecipients: " + mRecipients);
        }
        return mRecipients.equals(incomingRecipient);
    }

    /**
     * Returns a temporary Conversation (not representing one on disk) wrapping
     * the contents of the provided cursor.  The cursor should be the one
     * returned to your AsyncQueryHandler passed in to {@link #startQueryForAll}.
     * The recipient list of this conversation can be empty if the results
     * were not in cache.
     */
    public static Conversation from(Context context, Cursor cursor) {
        // First look in the cache for the Conversation and return that one. That way, all the
        // people that are looking at the cached copy will get updated when fillFromCursor() is
        // called with this cursor.
        long threadId = cursor.getLong(ID);
        if (threadId > 0) {
            Conversation conv = Cache.get(threadId);
            if (conv != null) {
                fillFromCursor(context, conv, cursor, false);   // update the existing conv in-place
                return conv;
            }
        }
        Conversation conv = new Conversation(context, cursor, false);
        try {
            /// M: Code analyze 063, For bug ALPS00331731, to resolveReject incoming call
            /// with a SMS]There is no recipient after we tap "Custom message" . @{
            if (sNeedCacheConv) {
                Cache.put(conv);
            } else if (conv.hasDraft() || conv.getMessageCount() > 0 || conv.hasAttachment()
                || conv.hasUnreadMessages()) {
                Cache.put(conv);
            }
            /// @}
        } catch (IllegalStateException e) {
            MmsLog.dpi(TAG,  "Tried to add duplicate Conversation to Cache (from cursor): " +
                    conv);
            if (!Cache.replace(conv)) {
                MmsLog.dpi(TAG, "Converations.from cache.replace failed on " + conv);
            }
        }
        return conv;
    }

    private synchronized void buildReadContentValues() {
        if (sReadContentValues == null) {
            sReadContentValues = new ContentValues(2);
            sReadContentValues.put("read", 1);
            sReadContentValues.put("seen", 1);
        }
    }

    /**
     * Marks all messages in this conversation as read and updates
     * relevant notifications.  This method returns immediately;
     * work is dispatched to a background thread. This function should
     * always be called from the UI thread.
     */
    public void markAsRead() {
        /// M: Code analyze 060, For bug ALPS00272115, add for WappushNotification
        /// canceled with other message . @{
        if (mIsDoingMarkAsRead) {
            MmsLog.d(TAG, "markAsRead(): mIsDoingMarkAsRead is true");
            return;
        }
        /// @}
        if (mMarkAsReadWaiting) {
            // We've already been asked to mark everything as read, but we're blocked.
            return;
        }
        if (mMarkAsReadBlocked) {
            // We're blocked so record the fact that we want to mark the messages as read
            // when we get unblocked.
            mMarkAsReadWaiting = true;
            return;
        }
        final Uri threadUri = getUri();

        new Thread(new Runnable() {
            public void run() {
               if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                   LogTag.debug("markAsRead");
               }
               // If we have no Uri to mark (as in the case of a conversation that
               // has not yet made its way to disk), there's nothing to do.
               if (!mIsDoingMarkAsRead && threadUri != null) {
                   setIsDoingMarkAsRead(true);
                   buildReadContentValues();

                   // Check the read flag first. It's much faster to do a query than
                   // to do an update. Timing this function show it's about 10x faster to
                   // do the query compared to the update, even when there's nothing to
                   // update.
                   /// M: Code analyze 060, For bug ALPS00272115, add for WappushNotification
                   /// canceled with other message . @{
                   int needUpdateCount = 0;
                   /// @}
                   Cursor c = mContext.getContentResolver().query(threadUri,
                           UNREAD_PROJECTION, UNREAD_SELECTION, null, null);
                   if (c != null) {
                       try {
                           /// M: Code analyze 060, For bug ALPS00272115,
                           /// add for WappushNotification
                           /// canceled with other message . @{
                           needUpdateCount = c.getCount();
                           /// @}
                       } finally {
                           c.close();
                       }
                   }
                   if (mIpConv.markAsRead(mContext, needUpdateCount)) {
                       return;
                   }
                   /// M: Code analyze 060, For bug ALPS00272115,
                   /// add for WappushNotification canceled
                   /// with other message . @{
                   if (needUpdateCount > 0) {
                       MmsLog.dpi(TAG, "markAsRead: update read/seen for thread uri: " +
                               threadUri);
                       final int allNeedUpdateCount = needUpdateCount;
                       final Conversation conv = Conversation.this;
                       new Thread(new Runnable() {
                           @Override
                           public void run() {
                               int updateCount = UPDATE_LIMIT;
                               if (allNeedUpdateCount >= UPDATE_LIMIT) {
                                   while (updateCount > 0) {
                                       updateCount = mContext.getContentResolver().update(
                                               threadUri, sReadContentValues,
                                               UNREAD_SELECTION_WITH_LIMIT,
                                               new String[] {mThreadId + "",
                                                       mThreadId + "", mThreadId + ""});
                                       MmsLog.d(TAG, "markAsRead-updateThread: updateCount="
                                               + updateCount);
                                       synchronized (this) {
                                           try {
                                               this.wait(UPDATE_LIMIT * 4);
                                           } catch (InterruptedException ex) {
                                               MmsLog.e(TAG, "InterruptedException: " + ex);
                                           }
                                       }
                                   }
                               } else {

                                   updateCount = mContext.getContentResolver().update(threadUri,
                                           sReadContentValues,
                                           UNREAD_SELECTION, null);
                                   MmsLog.d(TAG, "markAsRead-updateThread: updateCount="
                                           + updateCount);
                               }
                               conv.setIsDoingMarkAsRead(false);
                               // Always update notifications regardless of the read state.
                               MessagingNotification.blockingUpdateAllNotifications(mContext);
                           }
                       }, "markAsRead-updateThread").start();
                   } else {
                       setIsDoingMarkAsRead(false);
                   }
                   setHasUnreadMessages(false);
                }
                return;
            }
        }).start();
        /// @}
    }

    /**
     * Call this with false to prevent marking messages as read. The code calls this so
     * the DB queries in markAsRead don't slow down the main query for messages. Once we've
     * queried for all the messages (see ComposeMessageActivity.onQueryComplete), then we
     * can mark messages as read. Only call this function on the UI thread.
     */
    public void blockMarkAsRead(boolean block) {
        /// M: Code analyze 060, For bug ALPS00272115, add for WappushNotification canceled
        /// with other message . @{
        if (mIsDoingMarkAsRead) {
            return;
        }
        /// @}
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            LogTag.debug("blockMarkAsRead: " + block);
        }

        if (block != mMarkAsReadBlocked) {
            mMarkAsReadBlocked = block;
            if (!mMarkAsReadBlocked) {
                if (mMarkAsReadWaiting) {
                    mMarkAsReadWaiting = false;
                    markAsRead();
                }
            }
        }
    }

    /**
     * Returns a content:// URI referring to this conversation,
     * or null if it does not exist on disk yet.
     */
    public synchronized Uri getUri() {
        if (mThreadId <= 0)
            return null;

        return ContentUris.withAppendedId(Threads.CONTENT_URI, mThreadId);
    }

    /**
     * Return the Uri for all messages in the given thread ID.
     * @deprecated
     */
    public static Uri getUri(long threadId) {
        // TODO: Callers using this should really just have a Conversation
        // and call getUri() on it, but this guarantees no blocking.
        return ContentUris.withAppendedId(Threads.CONTENT_URI, threadId);
    }

    /**
     * Returns the thread ID of this conversation.  Can be zero if
     * {@link #ensureThreadId} has not been called yet.
     */
    public synchronized long getThreadId() {
        return mThreadId;
    }

    /**
     * Guarantees that the conversation has been created in the database.
     * This will make a blocking database call if it hasn't.
     *
     * @return The thread ID of this conversation in the database
     */
    public synchronized long ensureThreadId() {
        /// M: change google default.
        return ensureThreadId(false);
    }

    public synchronized void clearThreadId() {
        // remove ourself from the cache
        LogTag.debug("clearThreadId old threadId was: " + mThreadId + " now zero");
        Cache.remove(mThreadId);
        /// M: fix bug ALPS00473488, delete ObsoleteThread through threadID when discard()
        mThreadDiscardId = mThreadId;
        /// @}
        mThreadId = 0;
    }

    /**
     * Sets the list of recipients associated with this conversation.
     * If called, {@link #ensureThreadId} must be called before the next
     * operation that depends on this conversation existing in the
     * database (e.g. storing a draft message to it).
     */
    public synchronized void setRecipients(ContactList list) {
        /// M: remove the same contacts @{
        if (list == null) {
            return;
        }
        Set<Contact> contactSet = new HashSet<Contact>();
        contactSet.addAll(list);
        list.clear();
        list.addAll(contactSet);
        /// @}

        mRecipients = list;

        // Invalidate thread ID because the recipient set has changed.
        mThreadId = 0;

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "setRecipients after: " + this.toString());
        }
    }

    /**
     * Returns the recipient set of this conversation.
     */
    public synchronized ContactList getRecipients() {
        return mRecipients;
    }

    /**
     * Returns true if a draft message exists in this conversation.
     */
    public synchronized boolean hasDraft() {
        if (mThreadId <= 0)
            return false;
        return mHasDraft;
    }

    /**
     * Sets whether or not this conversation has a draft message.
     */
    public synchronized void setDraftState(boolean hasDraft) {
        if (mThreadId <= 0)
            return;
        mHasDraft = hasDraft;
    }

    /**
     * Returns the time of the last update to this conversation in milliseconds,
     * on the {@link System#currentTimeMillis} timebase.
     */
    public synchronized long getDate() {
        return mDate;
    }

    /**
     * Returns the number of messages in this conversation, excluding the draft
     * (if it exists).
     */
    public int getMessageCount() {
        return mMessageCount;
    }
    /**
     * Set the number of messages in this conversation, excluding the draft
     * (if it exists).
     */
    public synchronized void setMessageCount(int cnt) {
        mMessageCount = cnt;
    }

    /**
     * Returns a snippet of text from the most recent message in the conversation.
     */
    public synchronized String getSnippet() {
        return mSnippet;
    }

    /**
     * Returns true if there are any unread messages in the conversation.
     */
    public boolean hasUnreadMessages() {
        synchronized (this) {
            return mHasUnreadMessages;
        }
    }

    /// M:
    public void setHasUnreadMessages(boolean flag) {
        synchronized (this) {
            mHasUnreadMessages = flag;
        }
    }

    /**
     * Returns true if any messages in the conversation have attachments.
     */
    public synchronized boolean hasAttachment() {
        return mHasAttachment;
    }

    /**
     * Returns true if any messages in the conversation are in an error state.
     */
    public synchronized boolean hasError() {
        return mHasError;
    }

    /**
     * Returns true if this conversation is selected for a multi-operation.
     */
    public synchronized boolean isChecked() {
        return mIsChecked;
    }

    public synchronized void setIsChecked(boolean isChecked) {
        mIsChecked = isChecked;
    }

    public static long getOrCreateThreadId(Context context, ContactList list) {
        /** M: Google default
         * HashSet<String> recipients = new HashSet<String>();
         * Contact cacheContact = null;
         * for (Contact c : list) {
         * cacheContact = Contact.get(c.getNumber(), false);
         * if (cacheContact != null) {
         * recipients.add(cacheContact.getNumber());
         * } else {
         * recipients.add(c.getNumber());
         * }
         * }
         * long retVal = Threads.getOrCreateThreadId(context, recipients);
         * if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
         * LogTag.debug("[Conversation] getOrCreateThreadId for (%s) returned %d",
         * recipients, retVal);
         * }
         * return retVal;
         */
        /// M: change google default.
        return getOrCreateThreadId(context, list, false);
    }

    public static long getOrCreateThreadId(Context context, String address) {
        synchronized (sDeletingThreadsLock) {
            long now = System.currentTimeMillis();
            while (sDeletingThreads) {
                try {
                    sDeletingThreadsLock.wait(30000);
                } catch (InterruptedException e) {
                }
                if (System.currentTimeMillis() - now > 29000) {
                    // The deleting thread task is stuck or onDeleteComplete wasn't called.
                    // Unjam ourselves.
                    Log.e(TAG, "getOrCreateThreadId timed out waiting for delete to complete",
                            new Exception());
                    sDeletingThreads = false;
                    break;
                }
            }
            long retVal = Threads.getOrCreateThreadId(context, address);
            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                MmsLog.dpi(TAG, "[Conversation] getOrCreateThreadId for" + address + "returned " +
                                retVal);
            }
            return retVal;
        }
    }

    /*
     * The primary key of a conversation is its recipient set; override
     * equals() and hashCode() to just pass through to the internal
     * recipient sets.
     */
    @Override
    public synchronized boolean equals(Object obj) {
        try {
            Conversation other = (Conversation) obj;
            /** M: contains method of ConcurrentHashMap use equals to judge element existence.
             *  we take wap push message and cb message into seperate thread.
             *  they may be has the same recipients as the common thread, but different.
             */
            return (mRecipients.equals(other.mRecipients)) && (mType == other.mType);
        } catch (ClassCastException e) {
            return false;
        }
    }

    @Override
    public synchronized int hashCode() {
        /// M: Code analyze 067, For bug ALPS00273297, to resolve the message background
        /// color display abnormally . @{
        if (mType == MtkTelephony.MtkThreads.WAPPUSH_THREAD) {
            Log.d(TAG, "mRecipients.hashCode()*100 =" + mRecipients.hashCode() * 100);
            return mRecipients.hashCode() * 100;
        }
        /// @}
        else {
            return mRecipients.hashCode();
        }
    }

    public String toString() {
        return String.format("[%s] (tid %d)", mRecipients.serialize(), mThreadId);
    }

    /**
     * Remove any obsolete conversations sitting around on disk. Obsolete threads are threads
     * that aren't referenced by any message in the pdu or sms tables.
     */
    public static void asyncDeleteObsoleteThreads(AsyncQueryHandler handler, int token) {
        handler.startDelete(token, null, Threads.OBSOLETE_THREADS_URI, null, null);
    }

    /// M: fix bug ALPS00473488, delete ObsoleteThread through threadID when discard()
    public static void asyncDeleteObsoleteThreadID(AsyncQueryHandler handler, long threadID) {
        handler.startDelete(0, null,
                Uri.withAppendedPath(Threads.OBSOLETE_THREADS_URI,
                        String.valueOf(threadID)), null, null);
    }
    /// @}

    /**
     * Start a query for all conversations in the database on the specified
     * AsyncQueryHandler.
     *
     * @param handler An AsyncQueryHandler that will receive onQueryComplete
     *                upon completion of the query
     * @param token   The token that will be passed to onQueryComplete
     */
    public static void startQueryForAll(AsyncQueryHandler handler, int token) {
        /// M: Google default
        // handler.cancelOperation(token);

        // This query looks like this in the log:
        // I/Database(  147): elapsedTime4Sql|/data/data/com.android.providers.telephony/databases/
        // mmssms.db|2.253 ms|SELECT _id, date, message_count, recipient_ids, snippet, snippet_cs,
        // read, error, has_attachment FROM threads ORDER BY  date DESC

        startQuery(handler, token, null);
    }

    /**
     * Start a query for in the database on the specified AsyncQueryHandler with the specified
     * "where" clause.
     *
     * @param handler An AsyncQueryHandler that will receive onQueryComplete
     *                upon completion of the query
     * @param token   The token that will be passed to onQueryComplete
     * @param selection   A where clause (can be null) to select particular conv items.
     */
    public static void startQuery(AsyncQueryHandler handler, int token, String selection) {
        handler.cancelOperation(token);
        if (IpMessageUtils.startQueryForConversation(handler, ALL_THREADS_PROJECTION,
                token, selection)) {
            //let ipmessage process this query.
            return;
        }

        if (selection != null && selection.equals("allunread")) {
            handler.startQuery(token, null, sAllUnreadMessagesUri, null, selection, null,
                    Conversations.DEFAULT_SORT_ORDER);
        } else {
            handler.startQuery(token, null, sAllThreadsUri,
                    ALL_THREADS_PROJECTION, selection, null,
                    Conversations.DEFAULT_SORT_ORDER);
        }
    }

    /**
     * M: Code analyze 023, For bug ALPS00268161, when delete on MMS,
     * one sms will not be deleted . @{
     * Start a delete of the conversation with the specified thread ID.
     *
     * @param handler An AsyncQueryHandler that will receive onDeleteComplete
     *                upon completion of the conversation being deleted
     * @param token   The token that will be passed to onDeleteComplete
     * @param deleteAll Delete the whole thread including locked messages
     * @param threadId Thread ID of the conversation to be deleted
     */
    public static void startDelete(AsyncQueryHandler handler, int token, boolean deleteAll,
            long threadId, int maxMmsId, int maxSmsId) {
        /// M: wappush: do not need modify the code here,
        /// but delete function in provider has been modified.
        Uri uri = ContentUris.withAppendedId(Threads.CONTENT_URI, threadId);
        String selection = deleteAll ? null : "locked=0";
        PduCache.getInstance().purge(uri);

        /// M: @{
        if (maxMmsId != 0 || maxSmsId != 0) {
            uri = uri.buildUpon().appendQueryParameter("mmsId", String.valueOf(maxMmsId)).build();
            uri = uri.buildUpon().appendQueryParameter("smsId", String.valueOf(maxSmsId)).build();
        }
        /// @}
        /// M: ConversationDelete cookie value: multidelete: -2;
        /// delete all: -1; single delete: thread_id;
        uri = IpMessageUtils.startDeleteForConversation(uri);
        handler.startDelete(token, new Long(threadId), uri, selection, null);
    }

    /**
     * Start a delete of the conversation with the specified thread ID.
     *
     * @param handler An AsyncQueryHandler that will receive onDeleteComplete
     *                upon completion of the conversation being deleted
     * @param token   The token that will be passed to onDeleteComplete
     * @param deleteAll Delete the whole thread including locked messages
     * @param threadId Thread ID of the conversation to be deleted
     */
    public static void startDelete(ConversationQueryHandler handler, int token, boolean deleteAll,
            long threadId) {
        synchronized (sDeletingThreadsLock) {
            if (sDeletingThreads) {
                Log.e(TAG, "startDeleteAll already in the middle of a delete", new Exception());
            }
            sDeletingThreads = true;
            Uri uri = ContentUris.withAppendedId(Threads.CONTENT_URI, threadId);
            String selection = deleteAll ? null : "locked=0";

            MmsApp.getApplication().getPduLoaderManager().clear();

            // HACK: the keys to the thumbnail cache are the part uris, such as mms/part/3
            // Because the part table doesn't have auto-increment ids, the part ids are reused
            // when a message or thread is deleted. For now, we're clearing the whole thumbnail
            // cache so we don't retrieve stale images when part ids are reused. This will be
            // fixed in the next release in the mms provider.
            MmsApp.getApplication().getThumbnailManager().clear();

            handler.setDeleteToken(token);
            handler.startDelete(token, new Long(threadId), uri, selection, null);
        }
    }

    /**
     * Start deleting all conversations in the database.
     * @param handler An AsyncQueryHandler that will receive onDeleteComplete
     *                upon completion of all conversations being deleted
     * @param token   The token that will be passed to onDeleteComplete
     * @param deleteAll Delete the whole thread including locked messages
     */
    public static void startDeleteAll(ConversationQueryHandler handler, int token,
            boolean deleteAll) {
        synchronized (sDeletingThreadsLock) {
            if (sDeletingThreads) {
                Log.e(TAG, "startDeleteAll already in the middle of a delete", new Exception());
            }
            sDeletingThreads = true;
            String selection = deleteAll ? null : "locked=0";

            MmsApp.getApplication().getPduLoaderManager().clear();

            // HACK: the keys to the thumbnail cache are the part uris, such as mms/part/3
            // Because the part table doesn't have auto-increment ids, the part ids are reused
            // when a message or thread is deleted. For now, we're clearing the whole thumbnail
            // cache so we don't retrieve stale images when part ids are reused. This will be
            // fixed in the next release in the mms provider.
            MmsApp.getApplication().getThumbnailManager().clear();

            handler.setDeleteToken(token);
            handler.startDelete(token, new Long(-1), Threads.CONTENT_URI, selection, null);
        }
    }

    /**
     * M: Code analyze 023, For bug ALPS00268161,
     * when delete on MMS, one sms will not be deleted . @{
     * Start deleting all conversations in the database.
     * @param handler An AsyncQueryHandler that will receive onDeleteComplete
     *                upon completion of all conversations being deleted
     * @param token   The token that will be passed to onDeleteComplete
     * @param deleteAll Delete the whole thread including locked messages
     */
    public static void startDeleteAll(AsyncQueryHandler handler,
            int token, boolean deleteAll, int maxMmsId, int maxSmsId) {
        /// M: wappush: do not need modify the code here,
        /// but delete function in provider has been modified.
        String selection = deleteAll ? null : "locked=0";
        PduCache.getInstance().purge(Threads.CONTENT_URI);

        /// M: @{
        Uri uri = Threads.CONTENT_URI;
        Log.d(TAG, "startDeleteAll maxMmsId=" + maxMmsId + " maxSmsId=" + maxSmsId);
        if (maxMmsId != 0 || maxSmsId != 0) {
            uri = uri.buildUpon().appendQueryParameter("mmsId", String.valueOf(maxMmsId)).build();
            uri = uri.buildUpon().appendQueryParameter("smsId", String.valueOf(maxSmsId)).build();
        }
        /// @}
        ///M: add for cmcc, delete 1500 mms must bolow 200s, so for cmcc to change some rule @{
        uri = MessageUtils.sOpMessageUtilsExt.startDeleteAll(uri);
        /// @}
        MmsLog.dpi(TAG, "uri = " + uri);
        MmsApp.getApplication().getThumbnailManager().clear();
        /// M: change google default.
        /// M: ConversationDelete cookie value: multidelete: -2;
        /// delete all: -1; single delete: thread_id;
        uri = IpMessageUtils.startDeleteForConversation(uri);
        handler.startDelete(token, new Long(-1), uri, selection, null);
    }

    public static class ConversationQueryHandler extends AsyncQueryHandler {
        private int mDeleteToken;

        public ConversationQueryHandler(ContentResolver cr) {
            super(cr);
        }

        public void setDeleteToken(int token) {
            mDeleteToken = token;
        }

        /**
         * Always call this super method from your overridden onDeleteComplete function.
         */
        @Override
        protected void onDeleteComplete(int token, Object cookie, int result) {
            if (token == mDeleteToken) {
                // Test code
//                try {
//                    Thread.sleep(10000);
//                } catch (InterruptedException e) {
//                }

                // release lock
                synchronized (sDeletingThreadsLock) {
                    sDeletingThreads = false;
                    sDeletingThreadsLock.notifyAll();
                }
            }
        }
    }

    /**
     * Check for locked messages in all threads or a specified thread.
     * @param handler An AsyncQueryHandler that will receive onQueryComplete
     *                upon completion of looking for locked messages
     * @param threadIds   A list of threads to search. null means all threads
     * @param token   The token that will be passed to onQueryComplete
     */
    public static void startQueryHaveLockedMessages(AsyncQueryHandler handler,
            Collection<Long> threadIds,
            int token) {
        handler.cancelOperation(token);
        Uri uri = MmsSms.CONTENT_LOCKED_URI;

        String selection = null;
        if (threadIds != null) {
            StringBuilder buf = new StringBuilder();
            int i = 0;
            /// M: change google default.
            buf.append("thread_id in ( ");
            for (long threadId : threadIds) {
                if (i++ > 0) {
                    buf.append(",");
                }
                // We have to build the selection arg into the selection because deep down in
                // provider, the function buildUnionSubQuery takes selectionArgs, but ignores it.
                buf.append(Long.toString(threadId));
            }
            /// M:change google default.
            buf.append(" )");
            selection = buf.toString();
        }
        /// M: change google default.
        boolean handled = IpMessageUtils.startQueryHaveLockedMessages(handler, token, threadIds,
                uri, UNREAD_PROJECTION, selection, null);
        if (!handled) {
            handler.startQuery(token, threadIds, uri, UNREAD_PROJECTION, selection, null, null);
        }
    }

    /**
     * Check for locked messages in all threads or a specified thread.
     * @param handler An AsyncQueryHandler that will receive onQueryComplete
     *                upon completion of looking for locked messages
     * @param threadId   The threadId of the thread to search. -1 means all threads
     * @param token   The token that will be passed to onQueryComplete
     */
    public static void startQueryHaveLockedMessages(AsyncQueryHandler handler,
            long threadId,
            int token) {
        ArrayList<Long> threadIds = null;
        if (threadId != -1) {
            threadIds = new ArrayList<Long>();
            threadIds.add(threadId);
        }
        startQueryHaveLockedMessages(handler, threadIds, token);
    }

    /**
     * Fill the specified conversation with the values from the specified
     * cursor, possibly setting recipients to empty if {@value allowQuery}
     * is false and the recipient IDs are not in cache.  The cursor should
     * be one made via {@link #startQueryForAll}.
     */
    private static void fillFromCursor(Context context, Conversation conv,
                                       Cursor c, boolean allowQuery) {
        synchronized (conv) {
            conv.mThreadId = c.getLong(ID);
            conv.mDate = c.getLong(DATE);
            conv.mMessageCount = c.getInt(MESSAGE_COUNT);

            // Replace the snippet with a default value if it's empty.
            String snippet = MessageUtils.cleanseMmsSubject(context,
                    MessageUtils.extractEncStrFromCursor(c, SNIPPET, SNIPPET_CS));
            if (TextUtils.isEmpty(snippet)) {
                snippet = context.getString(R.string.no_subject_view);
            }
            conv.mSnippet = snippet;

            conv.setHasUnreadMessages(c.getInt(READ) == 0);
            conv.mHasError = (c.getInt(ERROR) != 0);
            conv.mHasAttachment = (c.getInt(HAS_ATTACHMENT) != 0);

            /// M: Code analyze 001, For new feature ALPS00131956,
            /// wappush: get the value for mType.
            conv.mType = c.getInt(TYPE);
            /// M: Code analyze 058, For bug ALPS00091288, to solve Sent MMS
            /// toself and receive the MMS phone exception happened . @}
            conv.mMessageStatus = c.getInt(STATUS);
            conv.mUnreadMessageCount = conv.mMessageCount - c.getInt(READCOUNT);
        }
        // Fill in as much of the conversation as we can before doing the slow stuff of looking
        // up the contacts associated with this conversation.
        String recipientIds = c.getString(RECIPIENT_IDS);
        ContactList recipients = ContactList.getByIds(recipientIds, allowQuery);
        synchronized (conv) {
            conv.mRecipients = recipients;
            String number = recipients.size() != 0 ? recipients.get(0).getNumber() : "";
            conv.mType = conv.mIpConv.onIpFillFromCursor(context,
                    c, recipients.size(), number, conv.mType, conv.mDate);
        }

        if (Log.isLoggable(LogTag.THREAD_CACHE, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "fillFromCursor: conv=" + conv + ", recipientIds=" + recipientIds);
        }
    }

    /**
     * Private cache for the use of the various forms of Conversation.get.
     */
    private static class Cache {
        private static Cache sInstance = new Cache();
        static Cache getInstance() { return sInstance; }
        /// M: use ConcurrentHashMap is better
        private final ConcurrentHashMap<Long, Conversation> mCache;
        private Cache() {
            /// M: use ConcurrentHashMap is better
            mCache = new ConcurrentHashMap<Long, Conversation>();
        }

        /**
         * Return the conversation with the specified thread ID, or
         * null if it's not in cache.
         */
        static Conversation get(long threadId) {
            /** M: use ConcurrentHashMap is better
            synchronized (sInstance) {
                if (Log.isLoggable(LogTag.THREAD_CACHE, Log.VERBOSE)) {
                    LogTag.debug("Conversation get with threadId: " + threadId);
                }
                for (Conversation c : sInstance.mCache) {
                    if (DEBUG) {
                        LogTag.debug("Conversation get() threadId: " + threadId +
                                " c.getThreadId(): " + c.getThreadId());
                    }
                    if (c.getThreadId() == threadId) {
                        return c;
                    }
                }
            }
            return null;
            */
            /// M: Code analyze 066, For bug ALPS00309629, Mms performance enhancement . @{
            Conversation c = sInstance.mCache.get(threadId);
            if (c != null && c.getThreadId() == threadId) {
                return c;
            }
            /// @}
            return null;
        }

        /**
         * Return the conversation with the specified recipient
         * list, or null if it's not in cache.
         */
        static Conversation get(ContactList list, int messagetype) {
            /** M: use ConcurrentHashMap is better
            synchronized (sInstance) {
                if (Log.isLoggable(LogTag.THREAD_CACHE, Log.VERBOSE)) {
                    LogTag.debug("Conversation get with ContactList: " + list);
                }
                for (Conversation c : sInstance.mCache) {
                    if (c.getRecipients().equals(list)) {
                        return c;
                    }
                }
            }
            */
            /// M: Code analyze 066, For bug ALPS00309629, Mms performance enhancement . @{
            Collection<Conversation> conv = sInstance.mCache.values();
            for (Conversation c : conv) {
            /// @}
                if (c.getRecipients().equals(list)) {
                    int convType = c.getType();
                    if (convType != MtkTelephony.MtkThreads.CELL_BROADCAST_THREAD
                            && convType != MtkTelephony.MtkThreads.WAPPUSH_THREAD){//mms or sms
                        convType = TYPE_MMSSMS;
                    }
                    if (convType != messagetype) {
                        continue;
                    }
                    return c;
                }
            }
            return null;
        }

        /**
         * Put the specified conversation in the cache.  The caller
         * should not place an already-existing conversation in the
         * cache, but rather update it in place.
         */
        static void put(Conversation c) {
            /** M: use ConcurrentHashMap is better
            synchronized (sInstance) {
                // We update cache entries in place so people with long-
                // held references get updated.
                if (Log.isLoggable(LogTag.THREAD_CACHE, Log.VERBOSE)) {
                    Log.d(TAG, "Conversation.Cache.put: conv= " + c + ", hash: " + c.hashCode());
                }

                if (sInstance.mCache.contains(c)) {
                    if (DEBUG) {
                        dumpCache();
                    }
                    throw new IllegalStateException("cache already contains " + c +
                            " threadId: " + c.mThreadId);
                }
                sInstance.mCache.add(c);
            }
            */
            /// M: Code analyze 066, For bug ALPS00309629, Mms performance enhancement . @{
            if (sInstance.mCache.contains(c)) {
                if (DEBUG) {
                    dumpCache();
                }
                throw new IllegalStateException("cache already contains " + c +
                        " threadId: " + c.mThreadId);
            }
            sInstance.mCache.put(c.getThreadId(), c);
            /// @}
        }

        /**
         * Replace the specified conversation in the cache. This is used in cases where we
         * lookup a conversation in the cache by threadId, but don't find it. The caller
         * then builds a new conversation (from the cursor) and tries to add it, but gets
         * an exception that the conversation is already in the cache, because the hash
         * is based on the recipients and it's there under a stale threadId. In this function
         * we remove the stale entry and add the new one. Returns true if the operation is
         * successful
         */
        static boolean replace(Conversation c) {
            /** M: use ConcurrentHashMap is better
            synchronized (sInstance) {
                if (Log.isLoggable(LogTag.THREAD_CACHE, Log.VERBOSE)) {
                    LogTag.debug("Conversation.Cache.put: conv= " + c + ", hash: " + c.hashCode());
                }

                if (!sInstance.mCache.contains(c)) {
                    if (DEBUG) {
                        dumpCache();
                    }
                    return false;
                }
                // Here it looks like we're simply removing and then re-adding the same object
                // to the hashset. Because the hashkey is the conversation's recipients, and not
                // the thread id, we'll actually remove the object with the stale threadId and
                // then add the the conversation with updated threadId, both having the same
                // recipients.
                sInstance.mCache.remove(c);
                sInstance.mCache.add(c);
                return true;
            }
            */
            /// M: Code analyze 066, For bug ALPS00309629, Mms performance enhancement . @{
            if (!sInstance.mCache.contains(c)) {
                if (DEBUG) {
                    dumpCache();
                }
                return false;
            }
            sInstance.mCache.replace(c.getThreadId(), c);
            return true;
            /// @
        }

        static void remove(long threadId) {
            /** M: use ConcurrentHashMap is better
            synchronized (sInstance) {
                if (DEBUG) {
                    LogTag.debug("remove threadid: " + threadId);
                    dumpCache();
                }
                for (Conversation c : sInstance.mCache) {
                    if (c.getThreadId() == threadId) {
                        sInstance.mCache.remove(c);
                        return;
                    }
                }
            }
            */
            /// M: Code analyze 066, For bug ALPS00309629, Mms performance enhancement . @{
            if (DEBUG) {
                LogTag.debug("remove threadid: " + threadId);
                dumpCache();
            }
            sInstance.mCache.remove(threadId);
            /// @}
        }

        static void dumpCache() {
            /** M: use ConcurrentHashMap is better
            synchronized (sInstance) {
                LogTag.debug("Conversation dumpCache: ");
                for (Conversation c : sInstance.mCache) {
                    LogTag.debug("   conv: " + c.toString() + " hash: " + c.hashCode());
                }
            }
            */
            /// M: Code analyze 066, For bug ALPS00309629, Mms performance enhancement . @{
            LogTag.debug("Conversation dumpCache: ");
            Collection<Conversation> conv = sInstance.mCache.values();
            for (Conversation c : conv) {
                MmsLog.dpi(TAG, "   conv: " + c.toString() + " hash: " + c.hashCode());
            }
            /// @}
        }

        /**
         * Remove all conversations from the cache that are not in
         * the provided set of thread IDs.
         */
        static void keepOnly(Set<Long> threads) {
            synchronized (sInstance) {
                Iterator<Entry<Long, Conversation>> iter = sInstance.mCache.entrySet().iterator();
                while (iter.hasNext()) {
                    Map.Entry<Long, Conversation> entry =
                        (Map.Entry<Long, Conversation>) iter.next();
                    Long key = entry.getKey();
                    Conversation c = entry.getValue();
                    if ((!threads.contains(c.getThreadId())) || (!threads.contains(key))) {
                        iter.remove();
                    }
                }
                MmsLog.d(TAG, "after keepOnly() mCache size is " + sInstance.mCache.size());
            }
            if (DEBUG) {
                LogTag.debug("after keepOnly");
            }
        }

        static void clear() {
            synchronized (sInstance) {
                sInstance.mCache.clear();
            }
        }
    }

    /**
     * Set up the conversation cache.  To be called once at application
     * startup time.
     */
    public static void init(final Context context) {
        /// M:
        sIsInitialized = true;
        /** M: Code analyze 066, For bug ALPS00309629, Mms performance enhancement . @{
        Thread thread = new Thread(new Runnable() {
                @Override
            public void run() {
                cacheAllThreads(context);
            }
        }, "Conversation.init");
        thread.setPriority(Thread.MIN_PRIORITY);
        thread.start();
        */
    }


    public static boolean isInitialized() {
        return sIsInitialized;
    }

    /// M: Code analyze 060, For bug ALPS00272115, add for WappushNotification
    /// canceled with other message . @{
    public static void markAllConversationsAsSeen(final Context context, final int wappushFlag) {
    /// @}
        if (DEBUG) {
            LogTag.debug("Conversation.markAllConversationsAsSeen");
        }
        String[] permissions = new String[] {
                Manifest.permission.READ_SMS,
                Manifest.permission.WRITE_SMS};
        if (!PermissionCheckUtil.checkPermissions(context, permissions)) {
            Log.w(TAG, "markAllConversationsAsSeen has no permissions!");
            return;
        }
        Thread thread = new Thread(new Runnable() {
            public void run() {
                blockingMarkAllSmsMessagesAsSeen(context);
                blockingMarkAllMmsMessagesAsSeen(context);

                /// M: Code analyze 059, For bug ALPS00087254, to resolve CB notification
                /// won't disappear when entry MMS app from status bar in case of receiving
                /// more than one msgs . @{
                blockingMarkAllCellBroadcastMessagesAsSeen(context);
                /// @}

                // Always update notifications regardless of the read state.
                MessagingNotification.blockingUpdateAllNotificationsExceptFailed(context);
            }
        }, "Conversation.markAllConversationsAsSeen");
        thread.setPriority(Thread.MIN_PRIORITY);
        thread.start();
    }

    private static void blockingMarkAllSmsMessagesAsSeen(final Context context) {
        ContentResolver resolver = context.getContentResolver();
        Cursor cursor = resolver.query(Sms.Inbox.CONTENT_URI,
                SEEN_PROJECTION,
                "seen=0",
                null,
                null);

        int count = 0;

        if (cursor != null) {
            try {
                count = cursor.getCount();
            } finally {
                cursor.close();
            }
        }

        if (count == 0) {
            return;
        }

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            Log.d(TAG, "mark " + count + " SMS msgs as seen");
        }

        ContentValues values = new ContentValues(1);
        values.put("seen", 1);

        resolver.update(Sms.Inbox.CONTENT_URI,
                values,
                "seen=0",
                null);
    }

    private static void blockingMarkAllMmsMessagesAsSeen(final Context context) {
        ContentResolver resolver = context.getContentResolver();
        Cursor cursor = resolver.query(Mms.Inbox.CONTENT_URI,
                SEEN_PROJECTION,
                "seen=0",
                null,
                null);

        int count = 0;

        if (cursor != null) {
            try {
                count = cursor.getCount();
            } finally {
                cursor.close();
            }
        }

        if (count == 0) {
            return;
        }

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            Log.d(TAG, "mark " + count + " MMS msgs as seen");
        }

        ContentValues values = new ContentValues(1);
        values.put("seen", 1);

        resolver.update(Mms.Inbox.CONTENT_URI,
                values,
                "seen=0",
                null);

    }

    /**
     * Are we in the process of loading and caching all the threads?.
     */
    public static boolean loadingThreads() {
        synchronized (Cache.getInstance()) {
            return sLoadingThreads;
        }
    }

    /** M: Code analyze 066, For bug ALPS00309629, Mms performance enhancement . @{
    private static void cacheAllThreads(Context context) {
        if (Log.isLoggable(LogTag.THREAD_CACHE, Log.VERBOSE)) {
            LogTag.debug("[Conversation] cacheAllThreads: begin");
        }
        synchronized (Cache.getInstance()) {
            if (sLoadingThreads) {
                return;
                }
            sLoadingThreads = true;
        }

        // Keep track of what threads are now on disk so we
        // can discard anything removed from the cache.
        HashSet<Long> threadsOnDisk = new HashSet<Long>();

        // Query for all conversations.
        /// M: modify
        Cursor c = null;
        c = context.getContentResolver().query(sAllThreadsUri,
                ALL_THREADS_PROJECTION, null, null, null);
        try {
            if (c != null) {
                /// M:
                long threadId = 0L;
                while (c.moveToNext()) {
                    threadId = c.getLong(ID);
                    threadsOnDisk.add(threadId);

                    // Try to find this thread ID in the cache.
                    Conversation conv;
                    synchronized (Cache.getInstance()) {
                        conv = Cache.get(threadId);
                    }

                    if (conv == null) {
                        // Make a new Conversation and put it in
                        // the cache if necessary.
                        conv = new Conversation(context, c, true);

                        /// M: @{
                        try {
                            Thread.sleep(10);
                        } catch (InterruptedException e) {
                            MmsLog.w(TAG,e.toString());
                        }
                        try {
                            synchronized (Cache.getInstance()) {
                                Cache.put(conv);
                            }
                        } catch (IllegalStateException e) {
                            MmsLog.e(TAG, "Tried to add duplicate Conversation to Cache" +
                                    " for threadId: " + threadId + " new conv: " + conv);
                            if (!Cache.replace(conv)) {
                                MmsLog.e(TAG, "cacheAllThreads cache.replace failed on " + conv);
                            }
                        }
                        /// @}
                    } else {
                        // Or update in place so people with references
                        // to conversations get updated too.
                        fillFromCursor(context, conv, c, true);
                    }
                }
            }
        } finally {
            if (c != null) {
                c.close();
            }
            synchronized (Cache.getInstance()) {
                sLoadingThreads = false;
            }
        }

        // Purge the cache of threads that no longer exist on disk.
        Cache.keepOnly(threadsOnDisk);

        if (Log.isLoggable(LogTag.THREAD_CACHE, Log.VERBOSE)) {
            LogTag.debug("[Conversation] cacheAllThreads: finished");
            Cache.dumpCache();
        }
    }
    */

    private boolean loadFromThreadId(long threadId, boolean allowQuery) {
        /// M: modify for ipmessage
        Cursor c = null;
        c = mIpConv.loadFromThreadId(mContext, sAllThreadsUri, ALL_THREADS_PROJECTION,
                                     threadId);
        if (c == null) {
            c = mContext.getContentResolver().query(sAllThreadsUri, ALL_THREADS_PROJECTION,
                    "threads._id=" + Long.toString(threadId), null, null);
            if (c == null) {
                LogTag.error("loadFromThreadId: query thread ID failed " + threadId);
                return false;
            }
        }
        try {
            if (c.moveToFirst()) {
                fillFromCursor(mContext, this, c, allowQuery);

                if (threadId != mThreadId) {
                    LogTag.error("loadFromThreadId: fillFromCursor returned differnt thread_id!" +
                            " threadId=" + threadId + ", mThreadId=" + mThreadId);
                }
            } else {
                LogTag.error("loadFromThreadId: Can't find thread ID " + threadId);
                return false;
            }
        } finally {
            c.close();
        }
        return true;
    }

    public static String getRecipients(Uri uri) {
        String base = uri.getSchemeSpecificPart();
        int pos = base.indexOf('?');
        return (pos == -1) ? base : base.substring(0, pos);
    }

    public static void dump() {
        Cache.dumpCache();
    }

    public static void dumpThreadsTable(Context context) {
        LogTag.debug("**** Dump of threads table ****");
        Cursor c = context.getContentResolver().query(sAllThreadsUri,
                ALL_THREADS_PROJECTION, null, null, "date ASC");
        try {
            c.moveToPosition(-1);
            while (c.moveToNext()) {
                String snippet = MessageUtils.extractEncStrFromCursor(c, SNIPPET, SNIPPET_CS);
                MmsLog.dpi(TAG, "dumpThreadsTable threadId: " + c.getLong(ID) +
                        " " + ThreadsColumns.DATE + " : " + c.getLong(DATE) +
                        " " + ThreadsColumns.MESSAGE_COUNT + " : " + c.getInt(MESSAGE_COUNT) +
                        " " + ThreadsColumns.SNIPPET + " : " + snippet +
                        " " + ThreadsColumns.READ + " : " + c.getInt(READ) +
                        " " + ThreadsColumns.ERROR + " : " + c.getInt(ERROR) +
                        " " + ThreadsColumns.HAS_ATTACHMENT + " : " + c.getInt(HAS_ATTACHMENT) +
                        " " + ThreadsColumns.RECIPIENT_IDS + " : " + c.getString(RECIPIENT_IDS));

                ContactList recipients = ContactList.getByIds(c.getString(RECIPIENT_IDS), false);
                MmsLog.dpi(TAG, "----recipients: " + recipients.serialize());
            }
        } finally {
            c.close();
        }
    }

    static final String[] SMS_PROJECTION = new String[] {
        BaseColumns._ID,
        // For SMS
        Sms.THREAD_ID,
        Sms.ADDRESS,
        Sms.BODY,
        Sms.DATE,
        Sms.READ,
        Sms.TYPE,
        Sms.STATUS,
        Sms.LOCKED,
        Sms.ERROR_CODE,
    };

    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_ID                  = 0;
    static final int COLUMN_THREAD_ID           = 1;
    static final int COLUMN_SMS_ADDRESS         = 2;
    static final int COLUMN_SMS_BODY            = 3;
    static final int COLUMN_SMS_DATE            = 4;
    static final int COLUMN_SMS_READ            = 5;
    static final int COLUMN_SMS_TYPE            = 6;
    static final int COLUMN_SMS_STATUS          = 7;
    static final int COLUMN_SMS_LOCKED          = 8;
    static final int COLUMN_SMS_ERROR_CODE      = 9;

    public static void dumpSmsTable(Context context) {
        LogTag.debug("**** Dump of sms table ****");
        Cursor c = context.getContentResolver().query(Sms.CONTENT_URI,
                SMS_PROJECTION, null, null, "_id DESC");
        try {
            // Only dump the latest 20 messages
            c.moveToPosition(-1);
            while (c.moveToNext() && c.getPosition() < 20) {
                String body = c.getString(COLUMN_SMS_BODY);
                MmsLog.dpi(TAG, "dumpSmsTable " + BaseColumns._ID + ": " + c.getLong(COLUMN_ID) +
                        " " + Sms.THREAD_ID + " : " + c.getLong(DATE) +
                        " " + Sms.ADDRESS + " : " + c.getString(COLUMN_SMS_ADDRESS) +
                        " " + Sms.BODY + " : " + body.substring(0, Math.min(body.length(), 8)) +
                        " " + Sms.DATE + " : " + c.getLong(COLUMN_SMS_DATE) +
                        " " + Sms.TYPE + " : " + c.getInt(COLUMN_SMS_TYPE));
            }
        } finally {
            c.close();
        }
    }

    /**
     * verifySingleRecipient takes a threadId and a string recipient [phone number or email
     * address]. It uses that threadId to lookup the row in the threads table and grab the
     * recipient ids column. The recipient ids column contains a space-separated list of
     * recipient ids. These ids are keys in the canonical_addresses table. The recipient is
     * compared against what's stored in the mmssms.db, but only if the recipient id list has
     * a single address.
     * @param context is used for getting a ContentResolver
     * @param threadId of the thread we're sending to
     * @param recipientStr is a phone number or email address
     * @return the verified number or email of the recipient
     */
    public static String verifySingleRecipient(final Context context,
            final long threadId, final String recipientStr) {
        if (threadId <= 0) {
            MmsLog.dpi(TAG, "verifySingleRecipient threadId is ZERO, recipient: " + recipientStr);
            LogTag.dumpInternalTables(context);
            return recipientStr;
        }
        Cursor c = context.getContentResolver().query(sAllThreadsUri, ALL_THREADS_PROJECTION,
                "_id=" + Long.toString(threadId), null, null);
        if (c == null) {
            MmsLog.dpi(TAG, "verifySingleRecipient threadId: " + threadId +
                    " resulted in NULL cursor , recipient: " + recipientStr);
            LogTag.dumpInternalTables(context);
            return recipientStr;
        }
        String address = recipientStr;
        String recipientIds;
        try {
            if (!c.moveToFirst()) {
                MmsLog.dpi(TAG, "verifySingleRecipient threadId: " + threadId +
                        " can't moveToFirst , recipient: " + recipientStr);
                LogTag.dumpInternalTables(context);
                return recipientStr;
            }
            recipientIds = c.getString(RECIPIENT_IDS);
        } finally {
            c.close();
        }
        String[] ids = recipientIds.split(" ");

        if (ids.length != 1) {
            // We're only verifying the situation where we have a single recipient input against
            // a thread with a single recipient. If the thread has multiple recipients, just
            // assume the input number is correct and return it.
            return recipientStr;
        }

        // Get the actual number from the canonical_addresses table for this recipientId
        address = RecipientIdCache.getSingleAddressFromCanonicalAddressInDb(context, ids[0]);

        if (TextUtils.isEmpty(address)) {
            MmsLog.epi(TAG, "verifySingleRecipient threadId: " + threadId +
                    " getSingleNumberFromCanonicalAddresses returned empty number for: " +
                    ids[0] + " recipientIds: " + recipientIds);
            LogTag.dumpInternalTables(context);
            return recipientStr;
        }
        if (PhoneNumberUtils.compareLoosely(recipientStr, address)) {
            // Bingo, we've got a match. We're returning the input number because of area
            // codes. We could have a number in the canonical_address name of "232-1012" and
            // assume the user's phone's area code is 650. If the user sends a message to
            // "(415) 232-1012", it will loosely match "232-1202". If we returned the value
            // from the table (232-1012), the message would go to the wrong person (to the
            // person in the 650 area code rather than in the 415 area code).
            return recipientStr;
        }

        if (context instanceof Activity) {
            LogTag.warnPossibleRecipientMismatch("verifySingleRecipient for threadId: " +
                    threadId + " original recipient: " + recipientStr +
                    " recipient from DB: " + address, (Activity) context);
        }
        LogTag.dumpInternalTables(context);
        if (Log.isLoggable(LogTag.THREAD_CACHE, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "verifySingleRecipient for threadId: " +
                    threadId + " original recipient: " + recipientStr +
                    " recipient from DB: " + address);
        }
        return address;
    }

    /// M: Code analyze 069, For bug ALPS00126550, to resolve can not save draft
    /// when have context and number . @{
    public static Conversation upDateThread(Context context, long threadId, boolean allowQuery) {
        Conversation cacheConv = Cache.get(threadId);
        Cache.remove(threadId);
        Conversation conv =  get(context, threadId, allowQuery);
        if (cacheConv != null) {
            conv.setIsChecked(cacheConv.isChecked());
        }
        return conv;
    }
    /// @}

    /// M: Code analyze 070, For bug ALPS00035289, Contact group . @{
    public static List<String> getNumbers(String recipient) {
        int len = recipient.length();
        List<String> list = new ArrayList<String>();

        int start = 0;
        int i = 0;
        char c;
        while (i < len + 1) {
            if ((i == len) || ((c = recipient.charAt(i)) == ',') || (c == ';')) {
                if (i > start) {
                    list.add(recipient.substring(start, i));

                    /**
                     * calculate the recipients total length. This is so if the name contains
                     * commas or semis, we'll skip over the whole name to the next
                     * recipient, rather than parsing this single name into multiple
                     * recipients.
                     */
                    int spanLen = recipient.substring(start, i).length();
                    if (spanLen > i) {
                        i = spanLen;
                    }
                }

                i++;

                while ((i < len) && (recipient.charAt(i) == ' ')) {
                    i++;
                }

                start = i;
            } else {
                i++;
            }
        }

        return list;
    }
    /// @}

    /// M: Code analyze 062, For bug ALPS00041233, Gemini enhancment . @{
    public synchronized int getChannelId() {
        if (mChannelId == -1) {
            setChannelIdFromDatabase();
        }
        return mChannelId;
    }

    private void setChannelIdFromDatabase() {
        Uri uri = ContentUris.withAppendedId(ADDRESS_ID_URI, mAddressId);
        Cursor c = mContext.getContentResolver().query(uri,
                ALL_ADDRESS_PROJECTION, null, null, null);
        try {
            if (c == null || c.getCount() == 0) {
                mChannelId = -1;
            } else {
                c.moveToFirst();
                // Name
                mChannelId = c.getInt(1);
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }
    }
    /// @}

    /// M: Code analyze 001, For new feature ALPS00131956, wappush: because of different uri . @{
    public void wpMarkAsRead() {
        final Uri threadUri = ContentUris.withAppendedId(
                WapPush.CONTENT_URI_THREAD, mThreadId);
        new Thread(new Runnable() {
            public void run() {
                synchronized (mMarkAsBlockedSyncer) {
                    if (mMarkAsReadBlocked) {
                        try {
                            mMarkAsBlockedSyncer.wait();
                        } catch (InterruptedException e) {
                        }
                    }

                    if (threadUri != null) {
                        buildReadContentValues();

                        /** M: Check the read flag first. It's much faster to do a query than
                         * to do an update. Timing this function show it's about 10x faster to
                         * do the query compared to the update, even when there's nothing to
                         * update.
                         */
                        boolean needUpdate = true;

                        Cursor c = mContext.getContentResolver().query(threadUri,
                                UNREAD_PROJECTION, UNREAD_SELECTION, null, null);
                        if (c != null) {
                            try {
                                needUpdate = c.getCount() > 0;
                            } finally {
                                c.close();
                            }
                        }

                        if (needUpdate) {
                            mContext.getContentResolver().update(threadUri,
                                    sReadContentValues, UNREAD_SELECTION, null);
                            mHasUnreadMessages = false;
                        }
                    }
                }
                WapPushMessagingNotification.nonBlockingUpdateNewMessageIndicator(mContext,
                    WapPushMessagingNotification.THREAD_ALL);
            }
        }).start();
    }
    /// @}

    /**
     * M: Code analyze 070, for bug ALPS00046775, to resolve have
     * no message in thread but have a delete message option . @}
     * Returns a content:// URI referring to this conversation,
     * or null if it does not exist on disk yet.
     */
    public synchronized Uri getQueryMsgUri() {
        if (mThreadId < 0) {
            return null;
        }

        return ContentUris.withAppendedId(Threads.CONTENT_URI, mThreadId);
    }

    /// M: Code analyze 054, For bug ALPS00120202, Message]can't save draft
    /// if enter message from messageDirect widget Sometimes thread id is deleted
    /// as obsolete thread, so need to guarantee it exists . @{
    public synchronized void guaranteeThreadId() {
        mThreadId = mIpConv.guaranteeIpThreadId(mThreadId);
        if (mThreadId != 0) {
            return;
        }
        mThreadId = getOrCreateThreadId(mContext, mRecipients, false);
    }
    /// @}

    /// M: Code analyze 065, For bug ALPS00066836 . @{
    public synchronized long ensureThreadId(boolean scrubForMmsAddress) {
        if (DEBUG) {
            LogTag.debug("ensureThreadId before: " + mThreadId);
        }
        if (mNeedForceUpdateThreadId) {
            MmsLog.d(TAG, "ensureThreadId(): Need force update thread id.");
        }

        if (mThreadId <= 0 || (mThreadId > 0 && mNeedForceUpdateThreadId)) {
            mThreadId = getOrCreateThreadId(mContext, mRecipients, scrubForMmsAddress);
            mNeedForceUpdateThreadId = false;
            MmsLog.d(TAG, "ensureThreadId(): after: ThreadId=" + mThreadId);
        }

        if (DEBUG) {
            LogTag.debug("ensureThreadId after: " + mThreadId);
        }

        return mThreadId;
    }

    public synchronized int getUnreadMessageCount() {
        return mUnreadMessageCount;
    }

    /// M: Code analyze 058, For bug ALPS00091288, to solve Sent MMS
    /// toself and receive the MMS phone exception happened . @}
    public synchronized int getMessageStatus() {
        return mMessageStatus;
    }
    /// @}

    /// M: Code analyze 001, For new feature ALPS00131956, wappush: add getType function . @{
    public synchronized int getType() {
        return mType;
    }
    /// @}

    private static long getOrCreateThreadId(Context context,
            ContactList list, final boolean scrubForMmsAddress) {
        HashSet<String> recipients = new HashSet<String>();
        Contact cacheContact = null;
        for (Contact c : list) {
            cacheContact = Contact.get(c.getNumber(), false);
            String number;
            if (cacheContact != null) {
                number = cacheContact.getNumber();
            } else {
                number = c.getNumber();
            }
            if (scrubForMmsAddress) {
                number = MessageUtils.parseMmsAddress(number);
            }

            if (!TextUtils.isEmpty(number) && !recipients.contains(number)) {
                recipients.add(number);
            }
        }
        long retVal = 0;
        try {
            retVal = Threads.getOrCreateThreadId(context, recipients);
        } catch (IllegalArgumentException e) {
            LogTag.error("Can't get or create the thread id");
            return 0;
        }
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "[Conversation] getOrCreateThreadId for " + recipients + " returned" +
                            retVal);
        }

        return retVal;
    }

    /// M: Code analyze 001, For new feature ALPS00131956, wappush: mark all wappush
    /// message as seen . @{
    private static void blockingMarkAllWapPushMessagesAsSeen(final Context context) {
        ContentResolver resolver = context.getContentResolver();
        Cursor cursor = resolver.query(WapPush.CONTENT_URI,
                SEEN_PROJECTION,
                "seen=0",
                null,
                null);

        int count = 0;

        if (cursor != null) {
            try {
                count = cursor.getCount();
            } finally {
                cursor.close();
            }
        }

        if (count == 0) {
            return;
        }

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.d(TAG, "mark " + count + " MMS msgs as seen");
        }

        ContentValues values = new ContentValues(1);
        values.put("seen", 1);

        resolver.update(WapPush.CONTENT_URI,
                values,
                "seen=0",
                null);

    }
    /// @}

    /// M: Code analyze 059, For bug ALPS00087254, to resolve CB notification
    /// won't disappear when entry MMS app from status bar in case of receiving
    /// more than one msgs . @{
    private static void blockingMarkAllCellBroadcastMessagesAsSeen(final Context context) {
        ContentResolver resolver = context.getContentResolver();
        Cursor cursor = resolver.query(MtkTelephony.SmsCb.CONTENT_URI,
                SEEN_PROJECTION,
                "seen=0",
                null,
                null);

        int count = 0;

        if (cursor != null) {
            try {
                count = cursor.getCount();
            } finally {
                cursor.close();
            }
        }

        if (count == 0) {
            return;
        }

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.d(TAG, "mark " + count + " CB msgs as seen");
        }

        ContentValues values = new ContentValues(1);
        values.put("seen", 1);

        resolver.update(MtkTelephony.SmsCb.CONTENT_URI,
                values,
                "seen=0",
                null);
    }
    /// @}

    public static void removeInvalidCache(Cursor cursor) {
        if (cursor != null) {
            synchronized (Cache.getInstance()) {
                HashSet<Long> threadsOnDisk = new HashSet<Long>();
                if (cursor.moveToFirst()) {
                    do {
                        long threadId = cursor.getLong(ID);
                        threadsOnDisk.add(threadId);
                    } while (cursor.moveToNext());
                }
                MmsLog.d(TAG, "keepOnly() threads size is "
                        + threadsOnDisk.size() + " mCache size is "
                        + Cache.sInstance.mCache.size());
                Cache.keepOnly(threadsOnDisk);
            }
        }
    }

    /// M: Code analyze 060, For bug ALPS00272115, add for WappushNotification
    /// canceled with other message . @{
    public void setIsDoingMarkAsRead(boolean isDoingMarkAsRead) {
        this.mIsDoingMarkAsRead = isDoingMarkAsRead;
    }
    /// @}

    private boolean mNeedForceUpdateThreadId = false;
    public void setNeedForceUpdateThreadId(boolean isNeeded) {
        mNeedForceUpdateThreadId = isNeeded;
    }

    /// M: Code analyze 064, For bug ALPS00303011, to resolve it can't pop up
    /// dialog when delete 1500 messages . @{
    public static Conversation getFromCursor(Context context, Cursor cursor) {
        /**
         * First look in the cache for the Conversation and return that one. That way, all the
         * people that are looking at the cached copy will get updated when fillFromCursor() is
         * called with this cursor.
         */
        long threadId = cursor.getLong(ID);
        if (threadId > 0) {
            Conversation conv = Cache.get(threadId);
            if (conv != null) {
                return conv;
            }
        }
        Conversation conv = new Conversation(context, cursor, false);
        try {
            Cache.put(conv);
        } catch (IllegalStateException e) {
            MmsLog.dpi(TAG, "Tried to add duplicate Conversation to Cache (from cursor): " +
                    conv);
            if (!Cache.replace(conv)) {
                MmsLog.dpi(TAG, "Converations.from cache.replace failed on " + conv);
            }
        }
        return conv;
    }
    /// @}

    /// M: Code analyze 063, For bug ALPS00331731, to resolveReject incoming call
    /// with a SMS]There is no recipient after we tap "Custom message" . @{
    public static void setNeedCacheConv(boolean need) {
        sNeedCacheConv = need;
    }
    /// @}

//    /**
//     * Returns true if this conversation is a spam thread.
//     */
//    public synchronized boolean isSpam() {
//        return mSpamStatus;
//    }
//
//    public synchronized void setSpam(boolean isSpam) {
//        this.mSpamStatus = isSpam;
//    }

    public synchronized Uri getThreadSettingsUri() {
        if (mThreadId <= 0 || mThreadSettingsId <= 0) {
            return null;
        }

        return ContentUris.withAppendedId(
                Uri.withAppendedPath(Telephony.MmsSms.CONTENT_URI,
                        "thread_settings"), mThreadSettingsId);
    }

    /**
     * IMPORTANT!!!!!!!!   this is a copy of startQuery with the same parameters
     * the differece is that this query can query thread_settings table columns.
     * the DEFECT is you should always use threads._id
     * in the where clause and whereargs.[it is false now]
     * see more info about this interface in MmssmsProvider.java's getSimpleConversationsExtend
     */
    public static void startQueryExtend(AsyncQueryHandler handler,
            int token, final String selection) {

        final int queryToken = token;
        final AsyncQueryHandler queryHandler = handler;

        // This query looks like this in the log:
        // I/Database(147): elapsedTime4Sql|/data/data/com.android.providers.telephony/databases/
        // mmssms.db|2.253 ms|SELECT _id, date, message_count, recipient_ids, snippet, snippet_cs,
        // read, error, has_attachment FROM threads ORDER BY  date DESC

        //handler.startQuery(token, null, sAllThreadsUri,
        //        ALL_THREADS_PROJECTION, selection, null, Conversations.DEFAULT_SORT_ORDER);

        Runnable r = sQueryExtendMap.get(token);
        if (r != null) {
            queryHandler.removeCallbacks(r);
            sQueryExtendMap.remove(r);
        }

        Runnable queryExtendRunnable = new Runnable() {
            public void run() {
                queryHandler.startQuery(queryToken, null, sAllThreadsUri,
                    ALL_THREADS_PROJECTION, selection,
                    null, Conversations.DEFAULT_SORT_ORDER);
            }
        };

        queryHandler.postDelayed(queryExtendRunnable, 200);

        sQueryExtendMap.put(token, queryExtendRunnable);
    }

    /**
     * Find the conversation matching the provided recipient set.
     * return null if no match found.
     */
    public static Conversation getCached(Context context, ContactList recipients) {
        MmsLog.dpi(TAG, "Conversation getCached by recipients: " + recipients.serialize());
        // If there are no recipients in the list, make a new conversation.
        if (recipients.size() < 1) {
            return null;
        }

        Conversation conv = Cache.get(recipients, 0);
        return conv;
    }

    /** M: for new design, when into Compose we don't mark as read, but we need mark as seen
     * and update notifications, so the new messages will disappear after into Compose.
     * our common design is to show unread messages in the bottom, so we can't mark as read.
     *
     * Marks all messages in this conversation as seen and updates
     * relevant notifications.  This method returns immediately;
     * work is dispatched to a background thread. This function should
     * always be called from the UI thread.
     */
    public void markAsSeen() {
        final Uri threadUri = getUri();

        new AsyncTask<Void, Void, Void>() {
            protected Void doInBackground(Void... none) {
                // If we have no Uri to mark (as in the case of a conversation that
                // has not yet made its way to disk), there's nothing to do.
                if(mIpConv.markAsSeen(mContext)) {
                    return null;
                }
                if (threadUri != null) {
                    ContentValues ct = new ContentValues(1);
                    ct.put("seen", 1);
                    // Check the seen flag first. It's much faster to do a query than
                    // to do an update. Timing this function show it's about 10x faster to
                    // do the query compared to the update, even when there's nothing to
                    // update.
                    boolean needUpdate = true;

                    Cursor c = mContext.getContentResolver().query(threadUri,
                            SEEN_PROJECTION, UNSEEN_SELECTION, null, null);
                    if (c != null) {
                        try {
                            needUpdate = c.getCount() > 0;
                        } finally {
                            c.close();
                        }
                    }

                    if (needUpdate && !mComposeIsPause) {
                        MmsLog.dpi(TAG, "markAsRead:update read/seen for thread uri:" + threadUri);
                        mContext.getContentResolver()
                            .update(threadUri, ct, UNSEEN_SELECTION, null);
                    }
                }
                // Always update notifications regardless of the seen state.
                MessagingNotification.blockingUpdateAllNotificationsExceptFailed(mContext);

                return null;
            }
        } .execute();
    }

    public void setComposeIsPaused(boolean paused) {
        mComposeIsPause = paused;
    }

    public static void clearCache() {
        Cache.getInstance().clear();
    }

    /// M: fix bug ALPS00439894, MTK MR1 new feature: Group Mms
    private boolean mIsGroupMms;

    public boolean getIsGroupMms() {
        return mIsGroupMms;
    }

    public void setIsGroupMms(boolean isGroup) {
        mIsGroupMms = isGroup;
    }

    /// M: fix bug ALPS00473488, delete ObsoleteThread through threadID when discard()
    private long mThreadDiscardId;

    public long getDiscardThreadId() {
        return mThreadDiscardId;
    }

    public void setDiscardThreadId(long id) {
        mThreadDiscardId = id;
    }
    /// @}

    public static Conversation getConvFromCache(Context context, Cursor cursor) {
        if (cursor == null) {
            return null;
        }
        Conversation conv = null;
        long threadId = cursor.getLong(ID);
        if (threadId > 0) {
             conv = Cache.get(threadId);
             if (conv == null) {
                 conv = new Conversation(context);
                 conv.mThreadId = cursor.getLong(ID);
                 conv.setHasUnreadMessages(cursor.getInt(READ) == 0);
             }
        }
        return conv;
    }

    /**
     * M: Fix bug ALPS00780175, The 1300 threads deleting will cost more than 10 minutes.
     * Avoid to delete multi threads one by one, let MmsSmsProvider handle this action.
     *
     * Start a delete of the conversations with the specified thread ID array.
     *
     * @param handler An AsyncQueryHandler that will receive onDeleteComplete
     *                upon completion of the conversation being deleted
     * @param token   The token that will be passed to onDeleteComplete
     * @param deleteAll Delete the whole thread including locked messages
     * @param threadIds Thread ID array of the conversations to be deleted
     */
    public static void startMultiDelete(AsyncQueryHandler handler, int token, boolean deleteAll,
            String[] threadIds, int maxMmsId, int maxSmsId) {
        MmsLog.d(TAG, "startMultiDelete() threadIds length is " + threadIds.length);
        Uri uri = Threads.CONTENT_URI;
        String selection = deleteAll ? null : "locked=0";
        PduCache.getInstance().purge(uri);
        uri = uri.buildUpon().appendQueryParameter("multidelete", "true").build();
        if (maxMmsId != 0 || maxSmsId != 0) {
            uri = uri.buildUpon().appendQueryParameter("mmsId", String.valueOf(maxMmsId)).build();
            uri = uri.buildUpon().appendQueryParameter("smsId", String.valueOf(maxSmsId)).build();
        }
        /// M: ConversationDelete cookie value: multidelete: -2;
        /// delete all: -1; single delete: thread_id;
        uri = IpMessageUtils.startDeleteForConversation(uri);
        handler.startDelete(token, new Long(-2), uri, selection, threadIds);
    }

    public IIpConversationExt getIpConv(Context context) {
        if (mIpConv == null) {
            mIpConv = IpMessageUtils.getIpMessagePlugin(context).getIpConversation();
            mIpConv.onIpInit(this);
        }
        return mIpConv;
    }

    /**
     * Get IpContactList.
     * Notice: This function is also called by operator plugin(OP01). When modify this function,
     * please notify operator team.
     *
     * @return the List of IIpContactExt of this conversation.
     */
    public List<IIpContactExt> getIpContactList() {
        List<IIpContactExt> list = new ArrayList<IIpContactExt>();
        for (Contact c : mRecipients) {
            list.add(c.getIpContact(mContext));
        }
        return list;
    }

    /// M: IpConversationCallback @{
    public boolean hasUnreadMessagesCallback() {
        return hasUnreadMessages();
    }

    public boolean hasDraftCallback() {
        return hasDraft();
    }

    public boolean hasErrorCallback() {
        return hasError();
    }

    public int getTypeCallback() {
        return getType();
    }

    public long getThreadIdCallback() {
        return getThreadId();
    }
    /// @}
}

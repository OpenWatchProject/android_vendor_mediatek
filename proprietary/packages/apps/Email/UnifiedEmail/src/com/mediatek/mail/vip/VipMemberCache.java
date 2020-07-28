package com.mediatek.mail.vip;

import java.util.HashSet;

import com.android.emailcommon.Logging;
import com.android.emailcommon.mail.Address;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.EmailContent.Message;
import com.android.emailcommon.provider.EmailContent.MessageColumns;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.mail.utils.Throttle;
import com.mediatek.mail.vip.utils.ClosingMatrixCursor;

import android.content.Context;
import android.database.ContentObserver;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.database.MatrixCursor.RowBuilder;
import android.net.Uri;
import android.os.Handler;
import android.text.TextUtils;

/**
 * Cache the vip members in memory to improve the performance
 *
 */
public class VipMemberCache {
    public static final String TAG = "VIP_Settings";
    public static final long COMBINED_ACCOUNT_ID = 0x10000000;

    // This static hash set stores all the vip addresses.
    private static HashSet<String> sVipAddresses = new HashSet<String>();
    private static VipMemberCache sInstance = null;

    private Context mContext;
    private VipContentObserver mContentObserver;
    private UpdateRunnable mUpdateRunnable;

    private VipMemberCache(Context context) {
        Logging.d(TAG, "VipMemberCache init...");
        mContext = context;
        mUpdateRunnable = new UpdateRunnable();
        mContentObserver = new VipContentObserver(new Handler(), mContext,
                mUpdateRunnable);
        // only VipMember's change should callback from this
        // contentObserver.Performance optimize
        mContentObserver.register(VipMember.NOTIFIER_URI);
        EmailAsyncTask.runAsyncParallel(mUpdateRunnable);
    }

    /**
     * In order to improve the performance, we keep a vip member cache in
     * memory. The cache must be initialized after Email running
     */
    public static void init(Context context) {
        if (sInstance == null) {
            sInstance = new VipMemberCache(context);
        }
    }

    /**
     * Check is there VIP members existed
     *
     * @return true if has some VIP members
     */
    public static boolean hasVipMembers() {
        synchronized (sVipAddresses) {
            return sVipAddresses.size() > 0;
        }
    }

    public static int getVipMembersCount() {
        synchronized (sVipAddresses) {
            return sVipAddresses.size();
        }
    }

    /**
     * Check is the email address belong to a Vip Member
     *
     * @param fromList
     *            the email address or from list to be checked
     * @return true if the email address belong to a Vip Member
     */
    public static boolean isVIP(String fromList) {
        String emailAddress = Address.getFirstMailAddress(fromList);
        if (TextUtils.isEmpty(emailAddress)) {
            return false;
        }
        /// keep sync between multi-thread access
        synchronized (sVipAddresses) {
            boolean vip = sVipAddresses.contains(emailAddress.toLowerCase());
            return vip;
        }
    }

    /**
     * Get the message count of this account sent from the VIP
     *
     * @param context
     *            the context
     * @param accountId
     *            the id of the account
     * @return the count of VIP messages of the account
     */
    public static int getVipMessagesCount(Context context, long accountId) {
        return getVipMessagesCount(context, accountId, false);
    }

    /**
     * Get the message count of this account sent from the VIP
     *
     * @param context
     *            the context
     * @param accountId
     *            the id of the account
     * @param onlyUnread
     *            is only find the unread messages
     * @return the count of VIP messages of the account
     */
    public static int getVipMessagesCount(Context context, long accountId,
            boolean onlyUnread) {
        Cursor cursor = getVipMessagesIds(context, accountId, onlyUnread);
        if (cursor == null) {
            return 0;
        }
        int count = cursor.getCount();
        cursor.close();
        return count;
    }

    /**
     * Get the messages of this account sent from the VIP
     *
     * @param context
     *            the context
     * @param accountId
     *            the id of the account
     * @return the VIP messages of the account
     */
    public static Cursor getVipMessagesIds(Context context, long accountId) {
        return getVipMessagesIds(context, accountId, false);
    }

    /**
     * Get the messages of this account sent from the VIP
     *
     * @param context
     *            the context
     * @param accountId
     *            the id of the account
     * @param onlyUnread
     *            is only find the unread messages
     * @return the VIP messages of the account
     */
    public static Cursor getVipMessagesIds(Context context, long accountId,
            boolean onlyUnread) {
        if (!VipMemberCache.hasVipMembers()) {
            return new MatrixCursor(EmailContent.ID_PROJECTION);
        }
        String vipSelection = VipMember.ALL_VIP_SELECTION;
        if (onlyUnread) {
            vipSelection = MessageColumns.FLAG_READ + "=0 AND " + vipSelection;
        }
        if (accountId > 0 && accountId != Account.ACCOUNT_ID_COMBINED_VIEW
                && accountId != COMBINED_ACCOUNT_ID) {
            vipSelection = MessageColumns.ACCOUNT_KEY + " = " + accountId
                    + " AND " + vipSelection;
        }
        Cursor c = context.getContentResolver().query(Message.CONTENT_URI,
                new String[] { MessageColumns._ID, MessageColumns.FROM_LIST },
                vipSelection, null, MessageColumns.TIMESTAMP + " DESC");
        if (c == null) {
            Logging.e(TAG,
                    "getVipMessagesIds return empty cursor because cursor is null");
            return new MatrixCursor(EmailContent.ID_PROJECTION);
        }
        ClosingMatrixCursor matrixCursor = new ClosingMatrixCursor(
                EmailContent.ID_PROJECTION, c);
        while (c.moveToNext()) {
            String fromList = c.getString(1);
            if (VipMemberCache.isVIP(fromList)) {
                RowBuilder row = matrixCursor.newRow();
                row.add(c.getLong(0));
            }
        }
        return matrixCursor;
    }

    /** get the unread Vip msg count for update */
    public static int getFolderVipMsgCount(Context context, long mailboxId,
            boolean onlyUnread) {
        if (!VipMemberCache.hasVipMembers()) {
            return 0;
        }
        Cursor c = null;
        String vipSelection = Message.FLAG_LOADED_SELECTION;
        if (mailboxId != -1L) {
            vipSelection = MessageColumns.MAILBOX_KEY + " = " + mailboxId
                    + " AND " + vipSelection;
        }
        if (onlyUnread) {
            vipSelection = MessageColumns.FLAG_READ + "=0 AND " + vipSelection;
        }
        int count = 0;
        try {
            c = context.getContentResolver()
                    .query(Message.CONTENT_URI,
                            new String[] { MessageColumns._ID,
                                    MessageColumns.FROM_LIST }, vipSelection,
                            null, MessageColumns.TIMESTAMP + " DESC");
            if (c == null) {
                Logging.e(TAG,
                        "getVipMessagesIds return empty cursor because cursor is null");
                return 0;
            }
            while (c.moveToNext()) {
                String fromList = c.getString(1);
                if (VipMemberCache.isVIP(fromList)) {
                    count++;
                }
            }
            return count;
        } finally {
            if (c != null) {
                c.close();
            }
        }
    }

    /**
     * Update the Vip members cache. Do not call it in UI thread.
     */
    public static void updateVipMemberCache() {
        if (sInstance != null) {
            sInstance.mUpdateRunnable.run();
        }
    }

    private class UpdateRunnable implements Runnable {

        @Override
        public void run() {
            if (mContext == null) {
                return;
            }
            Cursor c = null;
            try {
                c = mContext.getContentResolver().query(VipMember.CONTENT_URI,
                        VipMember.CONTENT_PROJECTION, null, null, null);
                if (c == null) {
                    Logging.d(TAG, "get a null cursor, just return.");
                    return;
                }
                synchronized (sVipAddresses) {
                    sVipAddresses.clear();
                    while (c.moveToNext()) {
                        String emailAddress = c
                                .getString(VipMember.EMAIL_ADDRESS_COLUMN);
                        if (TextUtils.isEmpty(emailAddress)) {
                            continue;
                        }
                        sVipAddresses.add(emailAddress.toLowerCase());
                    }
                }
                // Notify the VIP members changed
                // Turn "sync to network" parameter to "false" as this
                // operation is unnecessary to upsync
                mContext.getContentResolver().notifyChange(
                        VipMember.UIPROVIDER_VIPMEMBER_NOTIFIER, null, false);
                mContext.getContentResolver().notifyChange(
                        VipMember.UIPROVIDER_VIPMESSAGES_NOTIFIER, null, false);
            } finally {
                if (c != null) {
                    c.close();
                }
            }
        }
    }

    private class VipContentObserver extends ContentObserver implements
            Runnable {
        private final Throttle mThrottle;
        private Context mInnerContext;
        private boolean mRegistered;
        private Runnable mInnerRunnable;

        public VipContentObserver(Handler handler, Context context,
                Runnable runnable) {
            super(handler);
            mInnerContext = context;
            mThrottle = new Throttle("VipContentObserver", this, handler);
            mInnerRunnable = runnable;
        }

        @Override
        public void onChange(boolean selfChange) {
            if (mRegistered) {
                mThrottle.onEvent();
            }
        }

        public void unregister() {
            if (!mRegistered) {
                return;
            }
            mThrottle.cancelScheduledCallback();
            mInnerContext.getContentResolver().unregisterContentObserver(this);
            mRegistered = false;
        }

        public void register(Uri notifyUri) {
            unregister();
            mInnerContext.getContentResolver().registerContentObserver(
                    notifyUri, true, this);
            mRegistered = true;
            Logging.d(TAG, "VipContentObserver register");
        }

        @Override
        public void run() {
            EmailAsyncTask.runAsyncParallel(mInnerRunnable);
        }
    }

    /**
     * Support for VIP folder, filter messages of VIP folder
     *
     * @param messages
     *            the message cursor should contain MessageColumns.FROM_LIST
     *            Column.
     * @return the cursor contains mails of VIP folder
     */
    public static Cursor filterVipMessages(Cursor messages) {
        int fromListIndex = messages.getColumnIndex(MessageColumns.FROM_LIST);
        MatrixCursor matrixCursor = new ClosingMatrixCursor(
                messages.getColumnNames(), messages);
        if (!VipMemberCache.hasVipMembers() || fromListIndex == -1) {
            return matrixCursor;
        }
        while (messages.moveToNext()) {
            String fromList = messages.getString(fromListIndex);
            if (isVIP(fromList)) {
                addMessageRow(matrixCursor, messages);
            }
        }
        return matrixCursor;
    }

    private static void addMessageRow(MatrixCursor targetCursor,
            Cursor sourceCursor) {
        String[] projection = sourceCursor.getColumnNames();
        RowBuilder row = targetCursor.newRow();
        int type = Cursor.FIELD_TYPE_NULL;
        for (int i = 0; i < projection.length; i++) {
            type = sourceCursor.getType(i);
            switch (type) {
            case Cursor.FIELD_TYPE_INTEGER:
                row.add(sourceCursor.getLong(i));
                break;
            case Cursor.FIELD_TYPE_FLOAT:
                row.add(sourceCursor.getFloat(i));
                break;
            case Cursor.FIELD_TYPE_BLOB:
                row.add(sourceCursor.getBlob(i));
                break;
            case Cursor.FIELD_TYPE_STRING:
            default:
                row.add(sourceCursor.getString(i));
                break;
            }
        }
    }
}

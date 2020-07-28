package com.android.exchange.eas;

import android.content.Context;
import android.database.Cursor;

import com.android.emailcommon.TrafficFlags;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent.Message;
import com.android.emailcommon.provider.EmailContent.MessageColumns;
import com.android.emailcommon.provider.EmailContent.SyncColumns;
import com.android.emailcommon.provider.Mailbox;
import com.android.emailcommon.service.SyncWindow;
import com.android.exchange.Eas;
import com.android.exchange.adapter.AbstractSyncParser;
import com.android.exchange.adapter.EmailSyncParser;
import com.android.exchange.adapter.Serializer;
import com.android.exchange.adapter.Tags;

import com.mediatek.exchange.MTKEas;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

/**
 * Subclass to handle sync details for mail collections.
 */
public class EasSyncMail extends EasSyncCollectionTypeBase {

    /**
     * The projection used for building the fetch request list.
     */
    private static final String[] FETCH_REQUEST_PROJECTION = { SyncColumns.SERVER_ID };
    private static final int FETCH_REQUEST_SERVER_ID = 0;

    private static final int EMAIL_WINDOW_SIZE = 10;

    /// M: Add for Bad Sync Key recovery @{
    // Stand for the window size of syncing as "RECOVER"
    private int mWindowSizeRecover = 0;
    // We should ensure that the last window contains about 50 mails,
    // for avoiding too many empty mails in the front of the message list
    private static final int LASTWINDOW_MAILCOUNT = 50;
    // The window size of syncing as "RECOVER". Set it as big as possible for
    // reducing the communication times between server and client in the recovery.
    // Some exchange server only support window size 100
    private static final int BSK_WINDOW_SIZE_LIMIT = 100;
    public boolean mBskSyncAsNormal = true;
    /// @}

    /// M: Determine whether it's the first window of sync mails.
    private boolean mIsFirstWindow;

    @Override
    public int getTrafficFlag() {
        return TrafficFlags.DATA_EMAIL;
    }

    @Override
    public void setSyncOptions(final Context context, final Serializer s,
            final double protocolVersion, final Account account, final Mailbox mailbox,
            final boolean isInitialSync, final int numWindows) throws IOException {
        if (isInitialSync) {
            /// M: If initial sync, set this value to true;
            mIsFirstWindow = true;
            // No special options to set for initial mailbox sync.
            return;
        }

        /** M: Remove original partial download for EAS 2.5
        // Check for messages that aren't fully loaded.
        final ArrayList<String> messagesToFetch = addToFetchRequestList(context, mailbox);
        // The "empty" case is typical; we send a request for changes, and also specify a sync
        // window, body preference type (HTML for EAS 12.0 and later; MIME for EAS 2.5), and
        // truncation
        // If there are fetch requests, we only want the fetches (i.e. no changes from the server)
        // so we turn MIME support off.  Note that we are always using EAS 2.5 if there are fetch
        // requests
         *   *
         */

        // Permanently delete if in trash mailbox
        // In Exchange 2003, deletes-as-moves tag = true; no tag = false
        // In Exchange 2007 and up, deletes-as-moves tag is "0" (false) or "1" (true)
        final boolean isTrashMailbox = mailbox.mType == Mailbox.TYPE_TRASH;
        if (protocolVersion < Eas.SUPPORTED_PROTOCOL_EX2007_DOUBLE) {
            if (!isTrashMailbox) {
                s.tag(Tags.SYNC_DELETES_AS_MOVES);
            }
        } else {
            s.data(Tags.SYNC_DELETES_AS_MOVES, isTrashMailbox ? "0" : "1");
        }
        s.tag(Tags.SYNC_GET_CHANGES);

        /// M: Set the first window size to 10 and to 50 for others
        //  For EAS 2.5 we use default
        if (protocolVersion >= Eas.SUPPORTED_PROTOCOL_EX2007_DOUBLE) {
            s.data(Tags.SYNC_WINDOW_SIZE,
                    mIsFirstWindow ? MTKEas.EMAIL_FIRST_WINDOW_SIZE : MTKEas.EMAIL_WINDOW_SIZE_EX);
        } else {
            final int windowSize = numWindows * EMAIL_WINDOW_SIZE;
            if (windowSize > MAX_WINDOW_SIZE  + EMAIL_WINDOW_SIZE) {
                throw new IOException("Max window size reached and still no data");
            }
            s.data(Tags.SYNC_WINDOW_SIZE,
                    String.valueOf(windowSize < MAX_WINDOW_SIZE ? windowSize : MAX_WINDOW_SIZE));
        }

        /// M: Set mIsFirstWindow to false.
        if (mIsFirstWindow) {
            mIsFirstWindow = false;
        }

        s.start(Tags.SYNC_OPTIONS);
        // Set the lookback appropriately (EAS calls this a "filter")
        s.data(Tags.SYNC_FILTER_TYPE, getEmailFilter(account, mailbox));
        // Set the truncation amount for all classes
        if (protocolVersion >= Eas.SUPPORTED_PROTOCOL_EX2007_DOUBLE) {
            s.start(Tags.BASE_BODY_PREFERENCE);
            // HTML for email
            s.data(Tags.BASE_TYPE, Eas.BODY_PREFERENCE_HTML);
            // Partial download about 5KB for EAS 12 or above
            s.data(Tags.BASE_TRUNCATION_SIZE, MTKEas.EAS12_TRUNCATION_SIZE_PARTIAL);
            s.end();
        } else {
            /** M: For EAS 2.5 we used to fetch FULL of MIME data which lead high
             * data usage issue
             * Use FULL TEXT data for EAS 2.5 first
             * TODO: Support Partial download for EAS 2.5
             */
            s.data(Tags.SYNC_MIME_SUPPORT, Eas.MIME_BODY_PREFERENCE_TEXT);
            s.data(Tags.SYNC_MIME_TRUNCATION, MTKEas.EAS2_5_FULL_SIZE);
        }
        s.end();
    }

    @Override
    public AbstractSyncParser getParser(final Context context, final Account account,
            final Mailbox mailbox, final InputStream is) throws IOException {
        return new EmailSyncParser(context, is, mailbox, account);
    }

    /**
     * Query the provider for partially loaded messages.
     * @return Server ids for partially loaded messages.
     */
    private ArrayList<String> addToFetchRequestList(final Context context, final Mailbox mailbox) {
        final ArrayList<String> messagesToFetch = new ArrayList<String>();
        final Cursor c = context.getContentResolver().query(Message.CONTENT_URI,
                FETCH_REQUEST_PROJECTION,  MessageColumns.FLAG_LOADED + "=" +
                Message.FLAG_LOADED_PARTIAL + " AND " +  MessageColumns.MAILBOX_KEY + "=?",
                new String[] {Long.toString(mailbox.mId)}, null);
        if (c != null) {
            try {
                while (c.moveToNext()) {
                    messagesToFetch.add(c.getString(FETCH_REQUEST_SERVER_ID));
                }
            } finally {
                c.close();
            }
        }
        return messagesToFetch;
    }

    /**
     * Get the sync window for this collection and translate it to EAS's value for that (EAS refers
     * to this as the "filter").
     * @param account The {@link Account} for this sync; its sync window is used if the mailbox
     *                doesn't specify an override.
     * @param mailbox The {@link Mailbox} for this sync.
     * @return The EAS string value for the sync window specified for this mailbox.
     */
    private String getEmailFilter(final Account account, final Mailbox mailbox) {
        final int syncLookback = mailbox.mSyncLookback == SyncWindow.SYNC_WINDOW_ACCOUNT
                ? account.mSyncLookback : mailbox.mSyncLookback;
        switch (syncLookback) {
            case SyncWindow.SYNC_WINDOW_1_DAY:
                return Eas.FILTER_1_DAY;
            case SyncWindow.SYNC_WINDOW_3_DAYS:
                return Eas.FILTER_3_DAYS;
            case SyncWindow.SYNC_WINDOW_1_WEEK:
                return Eas.FILTER_1_WEEK;
            case SyncWindow.SYNC_WINDOW_2_WEEKS:
                return Eas.FILTER_2_WEEKS;
            case SyncWindow.SYNC_WINDOW_1_MONTH:
                return Eas.FILTER_1_MONTH;
            case SyncWindow.SYNC_WINDOW_ALL:
                return Eas.FILTER_ALL;
            default:
                // Auto window is deprecated and will also use the default.
                /// M: Changed from Eas.FILTER_1_WEEK to Eas.FILTER_3_DAYS
                return Eas.FILTER_3_DAYS;
        }
    }
}

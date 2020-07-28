/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.android.email.service;

import android.app.Service;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.TrafficStats;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.text.TextUtils;

import com.android.email.DebugUtils;
import com.android.email.EmailApplication;
import com.android.email.NotificationController;
import com.android.email.NotificationControllerCreatorHolder;
import com.android.email.mail.Store;
//import com.android.email.mail.store.Pop3Store;
//import com.android.email.mail.store.Pop3Store.Pop3Folder;
//import com.android.email.mail.store.Pop3Store.Pop3Message;
import com.android.email.provider.Utilities;
import com.android.emailcommon.Logging;
import com.android.emailcommon.TrafficFlags;
import com.android.emailcommon.internet.BinaryTempFileBody.BinaryTempFileBodyInputStream;
import com.android.emailcommon.internet.MimeUtility;
import com.android.emailcommon.mail.AuthenticationFailedException;
import com.android.emailcommon.mail.Folder.OpenMode;
import com.android.emailcommon.mail.MessagingException;
import com.android.emailcommon.mail.Part;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.EmailContent.Attachment;
import com.android.emailcommon.provider.EmailContent.AttachmentColumns;
import com.android.emailcommon.provider.EmailContent.Message;
import com.android.emailcommon.provider.EmailContent.MessageColumns;
import com.android.emailcommon.provider.EmailContent.SyncColumns;
import com.android.emailcommon.provider.Mailbox;
import com.android.emailcommon.service.EmailServiceStatus;
import com.android.emailcommon.service.IEmailServiceCallback;
import com.android.emailcommon.utility.AttachmentUtilities;
import com.android.emailcommon.utility.Utility;
import com.android.mail.providers.UIProvider;
import com.android.mail.providers.UIProvider.AttachmentState;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.StorageLowState;
import com.mediatek.email.mail.store.Pop3Folder;
import com.mediatek.email.mail.store.Pop3Folder.Pop3Message;
import com.mediatek.email.mail.store.Pop3Store;

import org.apache.james.mime4j.EOLConvertingInputStream;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;

public class Pop3Service extends Service {
    private static final String TAG = Logging.LOG_TAG + "/Pop3Service";
    /// M: change default sync mail count from 100 to 25.
    private static final int DEFAULT_SYNC_COUNT = 25;

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        EmailApplication.checkAndStartForeground(this, 6, "Pop3Service, onStartCommand");
        return Service.START_STICKY;
    }

    /**
     * Create our EmailService implementation here.
     */
    private final EmailServiceStub mBinder = new EmailServiceStub() {
        @Override
        public void loadAttachment(final IEmailServiceCallback callback, final long accountId,
                final long attachmentId, final boolean background) throws RemoteException {
            Attachment att = Attachment.restoreAttachmentWithId(mContext, attachmentId);
            if (att == null) {
                callback.loadAttachmentStatus(0, attachmentId,
                        EmailServiceStatus.ATTACHMENT_NOT_FOUND, 0);
                return;
            }
            long inboxId = Mailbox.findMailboxOfType(mContext, att.mAccountKey, Mailbox.TYPE_INBOX);
            if (inboxId == Mailbox.NO_MAILBOX) {
                return;
            }

            final long messageId = att.mMessageKey;
            final EmailContent.Message message =
                    EmailContent.Message.restoreMessageWithId(mContext, att.mMessageKey);
            if (message == null) {
                callback.loadAttachmentStatus(messageId, attachmentId,
                        EmailServiceStatus.MESSAGE_NOT_FOUND, 0);
                return;
            }

            /// M: start a network attachment downloading instead of startSync. @{
            Account account = Account.restoreAccountWithId(mContext, att.mAccountKey);
            Mailbox mailbox = Mailbox.restoreMailboxWithId(mContext, inboxId);
            if (account == null || mailbox == null) {
                // If the account/mailbox are gone, just report success; the UI handles this
                callback.loadAttachmentStatus(messageId, attachmentId,
                        EmailServiceStatus.SUCCESS, 0);
                return;
            }

            // If the message is loaded, just report that we're finished
            if (Utility.attachmentExists(mContext, att)
                    && att.mUiState == UIProvider.AttachmentState.SAVED) {
                callback.loadAttachmentStatus(messageId, attachmentId, EmailServiceStatus.SUCCESS,
                        0);
                return;
            }

            /// M: Say we're starting... @{
            callback.loadAttachmentStatus(messageId, attachmentId, EmailServiceStatus.IN_PROGRESS, 0);
            /// @}

            Pop3Folder remoteFolder = null;
            try {
                Pop3Store remoteStore = (Pop3Store) Store.getInstance(account, mContext);
                // The account might have been deleted
                if (remoteStore == null) {
                    return;
                }
                // Open the remote folder and create the remote folder if necessary
                remoteFolder = (Pop3Folder) remoteStore.getFolder(mailbox.mServerId);
                // Open the remote folder. This pre-loads certain metadata like message
                // count.
                remoteFolder.open(OpenMode.READ_WRITE);
                // Get the remote message count.
                final int remoteMessageCount = remoteFolder.getMessageCount();
                /*
                 * Get all messageIds in the mailbox.
                 * We don't necessarily need to sync all of them.
                 */
                Pop3Message[] remoteMessages = remoteFolder.getMessages(remoteMessageCount, remoteMessageCount);
                LogUtils.d(TAG, "remoteMessageCount " + remoteMessageCount);

                HashMap<String, Pop3Message> remoteUidMap = new HashMap<String, Pop3Message>();
                for (final Pop3Message remoteMessage : remoteMessages) {
                    final String uid = remoteMessage.getUid();
                    remoteUidMap.put(uid, remoteMessage);
                }

                /// M: pass in callback to report progress
                fetchAttachment(mContext, att, remoteFolder, remoteUidMap, callback);
                callback.loadAttachmentStatus(att.mMessageKey, attachmentId, EmailServiceStatus.SUCCESS, 0);
            } catch (MessagingException me) {
                LogUtils.i(TAG, me, "Error loading attachment");

                final ContentValues cv = new ContentValues(1);
                cv.put(AttachmentColumns.UI_STATE, UIProvider.AttachmentState.FAILED);
                final Uri uri = ContentUris.withAppendedId(Attachment.CONTENT_URI, attachmentId);
                mContext.getContentResolver().update(uri, cv, null, null);
                callback.loadAttachmentStatus(0, attachmentId, EmailServiceStatus.CONNECTION_ERROR, 0);
            } finally {
                if (remoteFolder != null) {
                    remoteFolder.close(false);
                }
            }
            /// @}
        }

        /**
         * M: POP partial download fetch an entire message.
         */
        @Override
        public void fetchMessage(long messageId) throws RemoteException {
            if (StorageLowState.checkIfStorageLow(mContext)) {
                LogUtils.e(Logging.LOG_TAG, "Can't create account due to low storage");
                return;
            }

            boolean success = false;
            /// M: Indicated message body content was truncated or not.
            boolean bodyTruncated = false;

            // 1. Resample the message, in case it disappeared or synced while
            // this command was in queue
            EmailContent.Message message =
                    EmailContent.Message.restoreMessageWithId(mContext, messageId);
            Pop3Folder remoteFolder = null;

            try {
                if (message == null) {
                    LogUtils.d(Logging.LOG_TAG, "Message is null!");
                    return;
                }
                if (message.mFlagLoaded == EmailContent.Message.FLAG_LOADED_COMPLETE) {
                    throw new MessagingException("Message's flagLoaded is FLAG_LOADED_COMPLETE!");
                }
                // 2. Open the remote folder.
                Account account = Account.restoreAccountWithId(mContext, message.mAccountKey);
                Mailbox mailbox = Mailbox.restoreMailboxWithId(mContext, message.mMailboxKey);
                if (account == null || mailbox == null) {
                    LogUtils.d(Logging.LOG_TAG, "Account or Mailbox is null!");
                    return;
                }

                TrafficStats.setThreadStatsTag(TrafficFlags.getSyncFlags(mContext, account));

                Store remoteStore = Store.getInstance(account, mContext);
                String remoteServerId = mailbox.mServerId;
                // If this is a search result, use the protocolSearchInfo field to get the
                // correct remote location
                if (!TextUtils.isEmpty(message.mProtocolSearchInfo)) {
                    remoteServerId = message.mProtocolSearchInfo;
                }
                remoteFolder = (Pop3Folder) remoteStore.getFolder(remoteServerId);
                remoteFolder.open(OpenMode.READ_WRITE);

                // 3. Set up to download the entire message
                Pop3Message remoteMessage = (Pop3Message) remoteFolder.getMessage(message.mServerId);

                /// M: message on server may deleted, if that just stop UI process and return. @{
                if (remoteMessage == null) {
                    ContentValues cv = new ContentValues(1);
                    cv.put(EmailContent.MessageColumns.FLAGS,
                            message.changeMessageStateFlags(EmailContent.Message.FLAG_LOAD_STATUS_FAILED));
                    message.update(mContext, cv);
                    LogUtils.d(Logging.LOG_TAG, "Message may has deleted on server!");
                    return;
                }
                /// @}
                remoteFolder.fetchBody(remoteMessage, -1, null);
                /** M: Check is the body size too large, and should be truncated. @{ */
                ArrayList<Part> viewables = new ArrayList<Part>();
                ArrayList<Part> attachments = new ArrayList<Part>();
                MimeUtility.collectParts(remoteMessage, viewables, attachments);
                for (Part part : viewables) {
                    BinaryTempFileBodyInputStream in = (BinaryTempFileBodyInputStream) part.getBody()
                            .getInputStream();
                    LogUtils.d(Logging.LOG_TAG,
                            " fetchMessage: POP3 fetch body size = " + in.getLength());
                    if (in.getLength() > MimeUtility.FETCH_BODY_SIZE_LIMIT) {
                        bodyTruncated = true;
                        break;
                    }
                }
                /** @} */

                // 4. Write to provider
                Utilities.copyOneMessageToProvider(mContext, remoteMessage, account, mailbox,
                        EmailContent.Message.FLAG_LOADED_COMPLETE);
                success = true;
                LogUtils.d(Logging.LOG_TAG, "POP3 fetch message success");
            } catch (MessagingException me) {
                LogUtils.d(Logging.LOG_TAG, "POP3 fetch message failed, " + me.getMessage());
            } catch (IOException ioe) {
                LogUtils.d(Logging.LOG_TAG, "POP3 fetch message failed, " + ioe.getMessage());
            } finally {
                if (remoteFolder != null) {
                    remoteFolder.close(false);
                }
            }

            /** M: Message may be updated. restore it again @{ */
            message = EmailContent.Message
                    .restoreMessageWithId(mContext, messageId);
            if (message == null) {
                LogUtils.d(Logging.LOG_TAG, "Message is null!!");
                return;
            }
            /** @} */
            int resultStatus = success ? message
                    .changeMessageStateFlags(EmailContent.Message.FLAG_LOAD_STATUS_SUCCESS)
                    : message.changeMessageStateFlags(EmailContent.Message.FLAG_LOAD_STATUS_FAILED);
            ///M: Update FLAG_BODY_TOO_LARGE into this message.
            if (bodyTruncated) {
                LogUtils.d(Logging.LOG_TAG, "POP3 messageId: %d set too large flag", message.mId);
                resultStatus |= EmailContent.Message.FLAG_BODY_TOO_LARGE;
            }
            ContentValues cv = new ContentValues();
            cv.put(EmailContent.MessageColumns.FLAGS, resultStatus);
            message.update(mContext, cv);
        }

    };

    @Override
    public IBinder onBind(Intent intent) {
        mBinder.init(this);
        return mBinder;
    }

    /**
     * Start foreground synchronization of the specified folder. This is called
     * by synchronizeMailbox or checkMail. TODO this should use ID's instead of
     * fully-restored objects
     *
     * @param account
     * @param folder
     * @param deltaMessageCount the requested change in number of messages to sync.
     * @return The status code for whether this operation succeeded.
     * @throws MessagingException
     */
    public static int synchronizeMailboxSynchronous(Context context, final Account account,
            final Mailbox folder, final int deltaMessageCount) throws MessagingException {
        TrafficStats.setThreadStatsTag(TrafficFlags.getSyncFlags(context, account));
        final NotificationController nc =
                NotificationControllerCreatorHolder.getInstance(context);
        try {
            synchronizePop3Mailbox(context, account, folder, deltaMessageCount);
            // Clear authentication notification for this account
            if (nc != null) {
                nc.cancelLoginFailedNotification(account.mId);
            }
        } catch (MessagingException e) {
            if (Logging.LOGD) {
                LogUtils.v(Logging.LOG_TAG, "synchronizeMailbox", e);
            }
            if (e instanceof AuthenticationFailedException && nc != null) {
                // Generate authentication notification
                nc.showLoginFailedNotificationSynchronous(account.mId, true /* incoming */);
            }
            throw e;
        }
        // TODO: Rather than use exceptions as logic aobve, return the status and handle it
        // correctly in caller.
        return EmailServiceStatus.SUCCESS;
    }

    /**
     * Lightweight record for the first pass of message sync, where I'm just
     * seeing if the local message requires sync. Later (for messages that need
     * syncing) we'll do a full readout from the DB.
     */
    private static class LocalMessageInfo {
        private static final int COLUMN_ID = 0;
        private static final int COLUMN_FLAG_LOADED = 1;
        private static final int COLUMN_SERVER_ID = 2;
        private static final String[] PROJECTION = new String[] {
                EmailContent.RECORD_ID, MessageColumns.FLAG_LOADED, SyncColumns.SERVER_ID
        };

        final long mId;
        final int mFlagLoaded;
        final String mServerId;

        public LocalMessageInfo(Cursor c) {
            mId = c.getLong(COLUMN_ID);
            mFlagLoaded = c.getInt(COLUMN_FLAG_LOADED);
            mServerId = c.getString(COLUMN_SERVER_ID);
            // Note: mailbox key and account key not needed - they are projected
            // for the SELECT
        }
    }

    /**
     * Load the structure and body of messages not yet synced
     *
     * @param account the account we're syncing
     * @param remoteFolder the (open) Folder we're working on
     * @param unsyncedMessages an array of Message's we've got headers for
     * @param toMailbox the destination mailbox we're syncing
     * @throws MessagingException
     */
    static void loadUnsyncedMessages(final Context context, final Account account,
            Pop3Folder remoteFolder, ArrayList<Pop3Message> unsyncedMessages,
            final Mailbox toMailbox) throws MessagingException {

        if (DebugUtils.DEBUG) {
            LogUtils.d(TAG, "Loading " + unsyncedMessages.size() + " unsynced messages");
        }

        try {
            int cnt = unsyncedMessages.size();
            // They are in most recent to least recent order, process them that way.
            for (int i = 0; i < cnt; i++) {
                final Pop3Message message = unsyncedMessages.get(i);
                remoteFolder.fetchBody(message, Pop3Store.FETCH_BODY_SANE_SUGGESTED_SIZE / 76,
                        null);
                int flag = EmailContent.Message.FLAG_LOADED_COMPLETE;
                /**
                 * M: Modified for getting pop message down load state
                 * correctly.Specialy for some messages which have not boundary. @{
                 */
                // For the message size is 5k, get 5k size from server, original
                // we only get 67 line'data, so use the accurate size as
                // boundary value
                long boundarySize = (Pop3Store.FETCH_BODY_SANE_SUGGESTED_SIZE / 76) * 76;
                if (message.getSize() > boundarySize || message.getSize() == 0
                        || !message.isComplete()) {
                    // TODO: when the message is not complete, this should mark the message as
                    // partial.  When that change is made, we need to make sure that:
                    // 1) Partial messages are shown in the conversation list
                    // 2) We are able to download the rest of the message/attachment when the
                    //    user requests it.
                    flag = EmailContent.Message.FLAG_LOADED_PARTIAL;
                }
                if (DebugUtils.DEBUG) {
                    LogUtils.d(TAG, "Message's download state is "
                            + (flag == EmailContent.Message.FLAG_LOADED_PARTIAL ? "Not Competed."
                                    : "Completed.") + " message subject : %s size : %d",
                                    message.getSubject(),  message.getSize());
                }
                /** @} */
                // If message is incomplete, create a "fake" attachment
                Utilities.copyOneMessageToProvider(context, message, account, toMailbox, flag);
            }
        } catch (IOException e) {
            throw new MessagingException(MessagingException.IOERROR);
        }
    }

    private static class FetchCallback implements EOLConvertingInputStream.Callback {
        private final ContentResolver mResolver;
        private final Uri mAttachmentUri;
        private final ContentValues mContentValues = new ContentValues();

        FetchCallback(ContentResolver resolver, Uri attachmentUri) {
            mResolver = resolver;
            mAttachmentUri = attachmentUri;
        }

        @Override
        public void report(int bytesRead) {
            mContentValues.put(AttachmentColumns.UI_DOWNLOADED_SIZE, bytesRead);
            mResolver.update(mAttachmentUri, mContentValues, null, null);
        }
    }

    /**
     * M: This method is the enhancement of Google default delete remote message flow.
     * Default:
     * Only check the update_message table and mailbox key is trash and delete the server message.
     *
     * Shortages of default design:
     * a. If user delete the trash message and the update_message will be removed too,
     *    in this case those messages will never be deleted from server.
     * c. The delete_message will never be removed from local db in this case.
     *
     * This method will handle user delete trash folder case, it will check the delete_message table
     * and remove remote messages which deleted from trash mailbox.
     *
     */
    private static void processDeleteTrashMessages(Context context,
            String[] accountIdArgs, long trashMailboxId, Pop3Folder remoteFolder)
                    throws MessagingException {
        Cursor deletes = context.getContentResolver().query(
                EmailContent.Message.DELETED_CONTENT_URI,
                EmailContent.Message.CONTENT_PROJECTION,
                EmailContent.MessageColumns.ACCOUNT_KEY + "=?", accountIdArgs,
                null);
        try {
            if (deletes == null || deletes.getCount() == 0) {
                return;
            }
            // loop through messages in messge delete table.
            while (deletes.moveToNext()) {
                EmailContent.Message localDeleteMsg = EmailContent.getContent(context,
                        deletes, EmailContent.Message.class);
                /**
                 * M: some deleted message not sync with server, use delete flag. @{
                 */
                if (localDeleteMsg != null
                        && (localDeleteMsg.mMailboxKey == trashMailboxId || localDeleteMsg
                                .isDeleteFromServer())) {
                    /** @} */
                    // Delete this on the server
                    Pop3Message popMessage = (Pop3Message) remoteFolder
                            .getMessage(localDeleteMsg.mServerId);
                    if (popMessage != null) {
                        LogUtils.d(TAG,
                                "processDeleteTrashMessages delete remote message [%s], id [%d]",
                                localDeleteMsg.mServerId, localDeleteMsg.mId);
                        remoteFolder.deleteMessage(popMessage);
                    }

                    // Finally, delete the delete
                    Uri uri = ContentUris.withAppendedId(
                            EmailContent.Message.DELETED_CONTENT_URI, localDeleteMsg.mId);
                    context.getContentResolver().delete(uri, null, null);
                }

            }
        } finally {
            if (deletes != null) {
                deletes.close();
            }
        }
    }

    /**
     * Synchronizer
     *
     * @param account the account to sync
     * @param mailbox the mailbox to sync
     * @param deltaMessageCount the requested change to number of messages to sync
     * @throws MessagingException
     */
    private static synchronized void synchronizePop3Mailbox(final Context context, final Account account,
            final Mailbox mailbox, final int deltaMessageCount) throws MessagingException {
        // TODO Break this into smaller pieces
        ContentResolver resolver = context.getContentResolver();

        // We only sync Inbox
        if (mailbox.mType != Mailbox.TYPE_INBOX) {
            return;
        }

        // Get the message list from EmailProvider and create an index of the uids

        Cursor localUidCursor = null;
        HashMap<String, LocalMessageInfo> localMessageMap = new HashMap<String, LocalMessageInfo>();

        try {
            localUidCursor = resolver.query(
                    EmailContent.Message.CONTENT_URI,
                    LocalMessageInfo.PROJECTION,
                    MessageColumns.MAILBOX_KEY + "=?",
                    new String[] {
                            String.valueOf(mailbox.mId)
                    },
                    null);
            while (localUidCursor.moveToNext()) {
                LocalMessageInfo info = new LocalMessageInfo(localUidCursor);
                localMessageMap.put(info.mServerId, info);
            }
        } finally {
            if (localUidCursor != null) {
                localUidCursor.close();
            }
        }

        // Open the remote folder and create the remote folder if necessary

        Pop3Store remoteStore = (Pop3Store)Store.getInstance(account, context);
        // The account might have been deleted
        if (remoteStore == null)
            return;
        Pop3Folder remoteFolder = (Pop3Folder)remoteStore.getFolder(mailbox.mServerId);

        // Open the remote folder. This pre-loads certain metadata like message
        // count.
        remoteFolder.open(OpenMode.READ_WRITE);

        String[] accountIdArgs = new String[] { Long.toString(account.mId) };
        long trashMailboxId = Mailbox.findMailboxOfType(context, account.mId, Mailbox.TYPE_TRASH);
        Cursor updates = resolver.query(
                EmailContent.Message.UPDATED_CONTENT_URI,
                EmailContent.Message.ID_COLUMN_PROJECTION,
                EmailContent.MessageColumns.ACCOUNT_KEY + "=?", accountIdArgs,
                null);
        try {
            // loop through messages marked as deleted
            while (updates.moveToNext()) {
                long id = updates.getLong(Message.ID_COLUMNS_ID_COLUMN);
                EmailContent.Message currentMsg =
                        EmailContent.Message.restoreMessageWithId(context, id);
                if (currentMsg.mMailboxKey == trashMailboxId) {
                    // Delete this on the server
                    Pop3Message popMessage =
                            (Pop3Message)remoteFolder.getMessage(currentMsg.mServerId);
                    if (popMessage != null) {
                        LogUtils.d(TAG,
                                "synchronizePop3Mailbox delete remote message [%s], id [%d]",
                                currentMsg.mServerId, id);
                        remoteFolder.deleteMessage(popMessage);
                    }
                }
                // Finally, delete the update
                Uri uri = ContentUris.withAppendedId(EmailContent.Message.UPDATED_CONTENT_URI, id);
                context.getContentResolver().delete(uri, null, null);
            }
        } finally {
            updates.close();
        }

        /// M: handle case of remove remote messages failed when delete trash messages.
        processDeleteTrashMessages(context, accountIdArgs, trashMailboxId, remoteFolder);

        /// M: Should expunge the remote folder to make download-able messages' right count here,
        // cause we may just setFlags to DELETE for some messages above @{
        remoteFolder.expunge();
        /// @}

        // Get the remote message count.
        final int remoteMessageCount = remoteFolder.getMessageCount();

        // Save the folder message count.
        mailbox.updateMessageCount(context, remoteMessageCount);

        // Create a list of messages to download
        Pop3Message[] remoteMessages = new Pop3Message[0];
        final ArrayList<Pop3Message> unsyncedMessages = new ArrayList<Pop3Message>();
        HashMap<String, Pop3Message> remoteUidMap = new HashMap<String, Pop3Message>();

        if (remoteMessageCount > 0) {
            /*
             * Get all messageIds in the mailbox.
             * We don't necessarily need to sync all of them.
             */
            remoteMessages = remoteFolder.getMessages(remoteMessageCount, remoteMessageCount);
            LogUtils.d(Logging.LOG_TAG, "remoteMessageCount " + remoteMessageCount);

            /*
             * TODO: It would be nicer if the default sync window were time based rather than
             * count based, but POP3 does not support time based queries, and the UIDL command
             * does not report timestamps. To handle this, we would need to load a block of
             * Ids, sync those messages to get the timestamps, and then load more Ids until we
             * have filled out our window.
             */
            int count = 0;
            int countNeeded = DEFAULT_SYNC_COUNT;
            for (final Pop3Message message : remoteMessages) {
                final String uid = message.getUid();
                remoteUidMap.put(uid, message);
            }

            /*
             * Figure out which messages we need to sync. Start at the most recent ones, and keep
             * going until we hit one of four end conditions:
             * 1. We currently have zero local messages. In this case, we will sync the most recent
             * DEFAULT_SYNC_COUNT, then stop.
             * 2. We have some local messages, and after encountering them, we find some older
             * messages that do not yet exist locally. In this case, we will load whichever came
             * before the ones we already had locally, and also deltaMessageCount additional
             * older messages.
             * 3. We have some local messages, but after examining the most recent
             * DEFAULT_SYNC_COUNT remote messages, we still have not encountered any that exist
             * locally. In this case, we'll stop adding new messages to sync, leaving a gap between
             * the ones we've just loaded and the ones we already had.
             * 4. We examine all of the remote messages before running into any of our count
             * limitations.
             */
            for (final Pop3Message message : remoteMessages) {
                final String uid = message.getUid();
                final LocalMessageInfo localMessage = localMessageMap.get(uid);
                if (localMessage == null) {
                    count++;
                } else {
                    // We have found a message that already exists locally. We may or may not
                    // need to keep looking, depending on what deltaMessageCount is.
                    LogUtils.d(Logging.LOG_TAG, "found a local message, need " +
                            deltaMessageCount + " more remote messages");
                    countNeeded = deltaMessageCount;
                    count = 0;
                }

                // localMessage == null -> message has never been created (not even headers)
                // mFlagLoaded != FLAG_LOADED_COMPLETE -> message failed to sync completely
                if (localMessage == null ||
                        (localMessage.mFlagLoaded != EmailContent.Message.FLAG_LOADED_COMPLETE &&
                                localMessage.mFlagLoaded != Message.FLAG_LOADED_PARTIAL)) {
                    LogUtils.d(Logging.LOG_TAG, "need to sync " + uid);
                    unsyncedMessages.add(message);
                } else {
                    LogUtils.d(Logging.LOG_TAG, "don't need to sync " + uid);
                }

                if (count >= countNeeded) {
                    LogUtils.d(Logging.LOG_TAG, "loaded " + count + " messages, stopping");
                    break;
                }
            }
        } else {
            if (DebugUtils.DEBUG) {
                LogUtils.d(TAG, "*** Message count is zero??");
            }
            remoteFolder.close(false);
            return;
        }

        // Remove any messages that are in the local store but no longer on the remote store.
        HashSet<String> localUidsToDelete = new HashSet<String>(localMessageMap.keySet());
        localUidsToDelete.removeAll(remoteUidMap.keySet());
        /// M: Use applyBatch for the mass deletions
        ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
        for (String uidToDelete : localUidsToDelete) {
            LogUtils.d(Logging.LOG_TAG, "need to delete " + uidToDelete);
            LocalMessageInfo infoToDelete = localMessageMap.get(uidToDelete);

            // Delete associated data (attachment files)
            // Attachment & Body records are auto-deleted when we delete the
            // Message record
            AttachmentUtilities.deleteAllAttachmentFiles(context, account.mId,
                    infoToDelete.mId);

            // Delete the message itself
            Uri uriToDelete = ContentUris.withAppendedId(
                    EmailContent.Message.CONTENT_URI, infoToDelete.mId);
            ops.add(ContentProviderOperation.newDelete(uriToDelete).build());

            // Delete extra rows (e.g. synced or deleted)
            Uri updateRowToDelete = ContentUris.withAppendedId(
                    EmailContent.Message.UPDATED_CONTENT_URI, infoToDelete.mId);
            ops.add(ContentProviderOperation.newDelete(updateRowToDelete).build());
            Uri deleteRowToDelete = ContentUris.withAppendedId(
                    EmailContent.Message.DELETED_CONTENT_URI, infoToDelete.mId);
            ops.add(ContentProviderOperation.newDelete(deleteRowToDelete).build());
        }
        try {
            resolver.applyBatch(EmailContent.AUTHORITY, ops);
        } catch (RemoteException e) {
            LogUtils.w(TAG, "RemoteException when removing local messages");
        } catch (OperationApplicationException e) {
            LogUtils.w(TAG, "OperationApplicationException when removing local messages");
        }

        LogUtils.d(TAG, "loadUnsynchedMessages " + unsyncedMessages.size());
        // Load messages we need to sync
        loadUnsyncedMessages(context, account, remoteFolder, unsyncedMessages, mailbox);

        // Clean up and report results
        remoteFolder.close(false);
    }

    /**
     * M: This is a code refactor of attachment fetching, we move these code from synchronizePop3Mailbox
     * to a stand alone function. Now, we do this in loadAttachment called by attachment download service.
     *
     * @param context
     * @param att
     * @param account
     * @param mailbox
     * @param remoteFolder
     * @param remoteUidMap
     * @param callback
     * @throws MessagingException
     */
    private static void fetchAttachment(Context context, Attachment att, Pop3Folder remoteFolder,
            HashMap<String, Pop3Message> remoteUidMap, final IEmailServiceCallback callback)
                    throws MessagingException, RemoteException {
        Message msg = Message.restoreMessageWithId(context, att.mMessageKey);

        if (msg == null) {
            LogUtils.d(Logging.LOG_TAG, "Message doesn't exist!");
            return;
        }
        String uid = msg.mServerId;
        Pop3Message popMessage = remoteUidMap.get(uid);
        if (popMessage != null) {
            LogUtils.d(TAG, " Pop3Service : synchronizePop3Mailbox : fetchAttachment : %d : popMessage : %d",
                    att.mId, att.mMessageKey);
            Uri attUri = ContentUris.withAppendedId(Attachment.CONTENT_URI, att.mId);
            try {
                remoteFolder.fetchBody(popMessage, -1, new FetchCallback(context.getContentResolver(), attUri));
            } catch (IOException e) {
                throw new MessagingException(MessagingException.IOERROR);
            }

            /// M: Until now, we have already fetch attachment from server, then we need update db @{
            callback.loadAttachmentStatus(msg.mId, att.mId, EmailServiceStatus.IN_PROGRESS, 100);
            /// @}

            if (!popMessage.isComplete()) {
                LogUtils.e(TAG, "How is this possible?");
            }

            // For pop message the location was the attachment's index in all attachments,
            // so get current attachment's location.
            int location = Integer.valueOf(att.mLocation);
            // Cause we just create it by partId in LegacyConversions.updateAttachments.
            int partIndex = location;

            // Now process attachments
            ArrayList<Part> viewables = new ArrayList<Part>();
            ArrayList<Part> attachments = new ArrayList<Part>();
            MimeUtility.collectParts(popMessage, viewables, attachments);
            // Now, only save the user specified attachment.
            // TODO : need update all attachments to save data downloading resource...?
            // LegacyConversions.updateAttachments(context, msg, attachments);
            /// M: Make sure partIndex is valid @{
            if (attachments.size() > partIndex && partIndex >= 0) {
                // Save the attachment to wherever it's going
                AttachmentUtilities.saveAttachment(context, attachments.get(partIndex).getBody().getInputStream(),
                        att);

                // Say we've downloaded the attachment
                final ContentValues values = new ContentValues(1);
                values.put(AttachmentColumns.UI_STATE, AttachmentState.SAVED);
                context.getContentResolver().update(attUri, values, null, null);
            } else {
                LogUtils.e(TAG, "fetchAttachment : could not save attachment[id=%s],  due to invalid location"
                        + "(partId %d with attachment size %d)", att.mId, partIndex, attachments.size());
            }
            /// @}
        } else {
            // TODO: Should we mark this attachment as failed so we don't
            // keep trying to download?
            LogUtils.e(TAG, "Could not find message for attachment " + uid);
        }
    }
    @Override
    public void onDestroy() {
        LogUtils.e(Logging.LOG_TAG, "onDestroy");
        EmailApplication.checkAndStopForeground(this, "Pop3Service, onDestroy");
    }
}

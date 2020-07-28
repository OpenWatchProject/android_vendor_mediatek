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

package com.android.email.service;

import android.app.Service;
import android.content.AbstractThreadedSyncAdapter;
import android.content.ContentProviderClient;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SyncResult;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.text.TextUtils;

import com.android.email.R;
import com.android.emailcommon.TempDirectory;
import com.android.emailcommon.mail.MessagingException;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.EmailContent.AccountColumns;
import com.android.emailcommon.provider.EmailContent.Message;
import com.android.emailcommon.provider.EmailContent.MessageColumns;
import com.android.emailcommon.provider.Mailbox;
import com.android.emailcommon.service.EmailServiceProxy;
import com.android.emailcommon.service.EmailServiceStatus;
import com.android.mail.providers.UIProvider;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.StorageLowState;

import java.util.ArrayList;
import java.util.HashMap;

public class PopImapSyncAdapterService extends Service {
    private static final String TAG = "PopImapSyncService";
    private SyncAdapterImpl mSyncAdapter = null;

    /// M: Cache the protocol of each account
    private static HashMap<String, String> sProtocolCache = new HashMap<String, String>();

    public PopImapSyncAdapterService() {
        super();
    }

    private static class SyncAdapterImpl extends AbstractThreadedSyncAdapter {
        public SyncAdapterImpl(Context context) {
            super(context, true /* autoInitialize */);
        }

        @Override
        public void onPerformSync(android.accounts.Account account, Bundle extras,
                String authority, ContentProviderClient provider, SyncResult syncResult) {
            PopImapSyncAdapterService.performSync(getContext(), account, extras, provider,
                    syncResult);
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mSyncAdapter = new SyncAdapterImpl(getApplicationContext());
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mSyncAdapter.getSyncAdapterBinder();
    }

    /**
     * @return whether or not this mailbox retrieves its data from the server (as opposed to just
     *     a local mailbox that is never synced).
     */
    private static boolean loadsFromServer(Context context, Mailbox m, String protocol) {
        String legacyImapProtocol = context.getString(R.string.protocol_legacy_imap);
        String pop3Protocol = context.getString(R.string.protocol_pop3);
        if (legacyImapProtocol.equals(protocol)) {
            // TODO: actually use a sync flag when creating the mailboxes. Right now we use an
            // approximation for IMAP.
            return m.mType != Mailbox.TYPE_DRAFTS
                    && m.mType != Mailbox.TYPE_OUTBOX
                    && m.mType != Mailbox.TYPE_SEARCH;
        } else if (pop3Protocol.equals(protocol)) {
            return Mailbox.TYPE_INBOX == m.mType;
        }

        return false;
    }

    private static void sync(final Context context, final long mailboxId,
            final Bundle extras, final SyncResult syncResult, final boolean uiRefresh,
            final int deltaMessageCount) {
        /// M: We Can't sync in low storage state
        // @NOTE: sync here is triggered by system sync manager, and sync manager won't do this
        // while in low storage state, so these section is not necessary @{
        if (StorageLowState.checkIfStorageLow(context)) {
            LogUtils.e(TAG, "Can't sync due to low storage");
            return;
        }
        /// @}
        TempDirectory.setTempDirectory(context);
        Mailbox mailbox = Mailbox.restoreMailboxWithId(context, mailboxId);
        if (mailbox == null) {
            return;
        }
        Account account = Account.restoreAccountWithId(context, mailbox.mAccountKey);
        if (account == null) {
            return;
        }
        ContentResolver resolver = context.getContentResolver();
        String protocol = account.getProtocol(context);
        if ((mailbox.mType != Mailbox.TYPE_OUTBOX) &&
                !loadsFromServer(context, mailbox, protocol)) {
            // This is an update to a message in a non-syncing mailbox; delete this from the
            // updates table and return
            resolver.delete(Message.UPDATED_CONTENT_URI, MessageColumns.MAILBOX_KEY + "=?",
                    new String[] {Long.toString(mailbox.mId)});
            return;
        }
        LogUtils.d(TAG, "About to sync mailbox: " + mailbox.mDisplayName);

        Uri mailboxUri = ContentUris.withAppendedId(Mailbox.CONTENT_URI, mailboxId);
        ContentValues values = new ContentValues();
        // Set mailbox sync state
        final int syncStatus = uiRefresh ? EmailContent.SYNC_STATUS_USER :
                EmailContent.SYNC_STATUS_BACKGROUND;
        values.put(Mailbox.UI_SYNC_STATUS, syncStatus);
        resolver.update(mailboxUri, values, null, null);
        try {
            int lastSyncResult;
            try {
                String legacyImapProtocol = context.getString(R.string.protocol_legacy_imap);
                lastSyncResult = UIProvider.createSyncValue(syncStatus,
                        EmailContent.LAST_SYNC_RESULT_SUCCESS);
                /* M: updates the status code "IN_PROGRESS" not only for normal mailbox, but also for outbox.
                 * Otherwise, the RefreshStatusMonitor will be triggered, that should not be happen.
                 * Before this, it only update status code for non-outbox type. @{*/
                EmailServiceStatus.syncMailboxStatus(resolver, extras, mailboxId,
                        EmailServiceStatus.IN_PROGRESS, 0, lastSyncResult);
                int status = UIProvider.LastSyncResult.SUCCESS;
                if (mailbox.mType == Mailbox.TYPE_OUTBOX) {
                    EmailServiceStub.sendMailImpl(context, account.mId);
                } else {
                    /** M: not suitable for MTK solution. @{ */
                    /*lastSyncResult = UIProvider.createSyncValue(syncStatus,
                            EmailContent.LAST_SYNC_RESULT_SUCCESS);
                    EmailServiceStatus.syncMailboxStatus(resolver, extras, mailboxId,
                            EmailServiceStatus.IN_PROGRESS, 0, lastSyncResult);
                    final int status;*/
                    /** @{ */
                    if (protocol.equals(legacyImapProtocol)) {
                        status = ImapService.synchronizeMailboxSynchronous(context, account,
                                mailbox, deltaMessageCount != 0, uiRefresh);
                    } else {
                        status = Pop3Service.synchronizeMailboxSynchronous(context, account,
                                mailbox, deltaMessageCount);
                    }
                    /// M: not suitable for MTK solution.
                    /*EmailServiceStatus.syncMailboxStatus(resolver, extras, mailboxId, status, 0,
                            lastSyncResult);*/
                }
                EmailServiceStatus.syncMailboxStatus(resolver, extras, mailboxId, status, 0,
                        lastSyncResult);
                /* @}*/
            } catch (MessagingException e) {
                final int type = e.getExceptionType();
                // type must be translated into the domain of values used by EmailServiceStatus
                switch (type) {
                    case MessagingException.IOERROR:
                        lastSyncResult = UIProvider.createSyncValue(syncStatus,
                                EmailContent.LAST_SYNC_RESULT_CONNECTION_ERROR);
                        EmailServiceStatus.syncMailboxStatus(resolver, extras, mailboxId,
                                EmailServiceStatus.FAILURE, 0, lastSyncResult);
                        syncResult.stats.numIoExceptions++;
                        break;
                    case MessagingException.AUTHENTICATION_FAILED:
                        lastSyncResult = UIProvider.createSyncValue(syncStatus,
                                EmailContent.LAST_SYNC_RESULT_AUTH_ERROR);
                        EmailServiceStatus.syncMailboxStatus(resolver, extras, mailboxId,
                                EmailServiceStatus.FAILURE, 0, lastSyncResult);
                        syncResult.stats.numAuthExceptions++;
                        break;
                    case MessagingException.SERVER_ERROR:
                        lastSyncResult = UIProvider.createSyncValue(syncStatus,
                                EmailContent.LAST_SYNC_RESULT_SERVER_ERROR);
                        EmailServiceStatus.syncMailboxStatus(resolver, extras, mailboxId,
                                EmailServiceStatus.FAILURE, 0, lastSyncResult);
                        break;

                    default:
                        lastSyncResult = UIProvider.createSyncValue(syncStatus,
                                EmailContent.LAST_SYNC_RESULT_INTERNAL_ERROR);
                        EmailServiceStatus.syncMailboxStatus(resolver, extras, mailboxId,
                                EmailServiceStatus.FAILURE, 0, lastSyncResult);
                }
            }
        } finally {
            // Always clear our sync state and update sync time.
            values.put(Mailbox.UI_SYNC_STATUS, EmailContent.SYNC_STATUS_NONE);
            values.put(Mailbox.SYNC_TIME, System.currentTimeMillis());
            resolver.update(mailboxUri, values, null, null);
        }
    }

    /**
     * Partial integration with system SyncManager; we initiate manual syncs upon request
     */
    private static void performSync(Context context, android.accounts.Account account,
            Bundle extras, ContentProviderClient provider, SyncResult syncResult) {
        /// M: We Can't sync in low storage state @{
        if (StorageLowState.checkIfStorageLow(context)) {
            LogUtils.e(TAG, "Can't performSync due to low storage");
            return;
        }
        /// @}
        // Find an EmailProvider account with the Account's email address
        Cursor c = null;
        try {
            c = provider.query(com.android.emailcommon.provider.Account.CONTENT_URI,
                    Account.CONTENT_PROJECTION, AccountColumns.EMAIL_ADDRESS + "=?",
                    new String[] {account.name}, null);
            if (c != null && c.moveToNext()) {
                Account acct = new Account();
                acct.restore(c);
                if (extras.getBoolean(ContentResolver.SYNC_EXTRAS_UPLOAD)) {
                    LogUtils.d(TAG, "Upload sync request for " + acct.mDisplayName);
                    // See if any boxes have mail...
                    ArrayList<Long> mailboxesToUpload = new ArrayList<Long>();
                    Cursor updatesCursor = provider.query(Message.UPDATED_CONTENT_URI,
                            new String[] {MessageColumns.MAILBOX_KEY},
                            MessageColumns.ACCOUNT_KEY + "=?",
                            new String[] {Long.toString(acct.mId)},
                            null);
                    try {
                        if (updatesCursor != null) {
                            while (updatesCursor.moveToNext()) {
                                Long mailboxId = updatesCursor.getLong(0);
                                if (!mailboxesToUpload.contains(mailboxId)) {
                                    mailboxesToUpload.add(mailboxId);
                                }
                            }
                        }
                    } finally {
                        if (updatesCursor != null) {
                            updatesCursor.close();
                        }
                    }

                    /** M: Also query if any local deletion need to be upsynced @{ */
                    Cursor deletesCursor = provider.query(Message.DELETED_CONTENT_URI,
                            new String[] {MessageColumns.MAILBOX_KEY},
                            MessageColumns.ACCOUNT_KEY + "=?",
                            new String[] {Long.toString(acct.mId)},
                            null);
                    try {
                        if (deletesCursor != null) {
                            while (deletesCursor.moveToNext()) {
                                Long mailboxId = deletesCursor.getLong(0);
                                if (!mailboxesToUpload.contains(mailboxId)) {
                                    mailboxesToUpload.add(mailboxId);
                                }
                            }
                        }
                    } finally {
                        if (deletesCursor != null) {
                            deletesCursor.close();
                        }
                    }
                    /** @} */

                    for (long mailboxId : mailboxesToUpload) {
                        LogUtils.d(TAG, "Upsyncing for mailbox:" + mailboxId);
                        sync(context, mailboxId, extras, syncResult, false, 0);
                    }
                } else {
                    LogUtils.d(TAG, "Sync request for " + acct.mDisplayName);
                    LogUtils.d(TAG, extras.toString());

                    /// M: Get the protocol of each account and save in cache
                    String protocol = sProtocolCache.get(acct.mEmailAddress);
                    if (TextUtils.isEmpty(protocol)) {
                        protocol = acct.getProtocol(context);
                        sProtocolCache.put(acct.mEmailAddress, protocol);
                    }
                    /// M: Update folder list only for IMAP at manually refresh occasion because POP does
                     // not have genuine folder hierarchy, update folder list in every sync for IMAP is
                     // excess as well.
                    boolean uiRefresh = extras.getBoolean(ContentResolver.SYNC_EXTRAS_MANUAL, false);
                    if (protocol.equalsIgnoreCase("IMAP") && uiRefresh) {
                        LogUtils.d(TAG, "Run updateFolderList for IMAP manually refresh");
                        final EmailServiceProxy service =
                                EmailServiceUtils.getServiceForAccount(context, acct.mId);
                        service.updateFolderList(acct.mId);
                    }

                    // Get the id for the mailbox we want to sync.
                    long [] mailboxIds = Mailbox.getMailboxIdsFromBundle(extras);
                    if (mailboxIds == null || mailboxIds.length == 0) {
                        // No mailbox specified, just sync the inbox.
                        // TODO: IMAP may eventually want to allow multiple auto-sync mailboxes.
                        final long inboxId = Mailbox.findMailboxOfType(context, acct.mId,
                                Mailbox.TYPE_INBOX);
                        if (inboxId != Mailbox.NO_MAILBOX) {
                            mailboxIds = new long[1];
                            mailboxIds[0] = inboxId;
                        }
                    }

                    if (mailboxIds != null) {
                        /// M: not suitable for MTK solution
                        /* boolean uiRefresh =
                            extras.getBoolean(ContentResolver.SYNC_EXTRAS_EXPEDITED, false);*/
                        int deltaMessageCount =
                                extras.getInt(Mailbox.SYNC_EXTRA_DELTA_MESSAGE_COUNT, 0);
                        for (long mailboxId : mailboxIds) {
                            sync(context, mailboxId, extras, syncResult, uiRefresh,
                                    deltaMessageCount);
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (c != null) {
                c.close();
            }
        }
    }

    /**
     * M: Remove account from the protocol cache
     */
    public static void removeProtocolCache(String emailAddress) {
        sProtocolCache.remove(emailAddress);
    }
}

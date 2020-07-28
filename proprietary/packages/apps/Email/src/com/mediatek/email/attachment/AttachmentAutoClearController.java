package com.mediatek.email.attachment;

import java.util.ArrayList;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;

import com.android.email.EmailApplication;
import com.android.email.Preferences;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.EmailContent.AccountColumns;
import com.android.emailcommon.provider.EmailContent.Message;
import com.android.emailcommon.provider.EmailContent.MessageColumns;
import com.android.emailcommon.utility.AttachmentUtilities;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.mail.providers.Conversation;
import com.android.mail.utils.LogUtils;
import com.mediatek.email.service.AttachmentAutoClearService;

public final class AttachmentAutoClearController {
    public static final String TAG = "AttachmentAutoClearController";

    private static final String[] MESSAGEID_TO_ACCOUNTID_PROJECTION = new String[] {
        EmailContent.RECORD_ID, EmailContent.MessageColumns.ACCOUNT_KEY };
    private static final int MESSAGEID_TO_ACCOUNTID_COLUMN_ACCOUNTID = 1;

    private static final Object SYNCHRONIZE_LOCK_FOR_RECENT_IDS = new Object();
    private static final int MAX_RECORD_COUNT = 10;
    private static ArrayList<Long> sRecentOpenedMsgIds = new ArrayList<Long>(MAX_RECORD_COUNT);

    /**
     * Clear the recently opened messages' id
     */
    public static void clearMessageIdsSync() {
        synchronized (SYNCHRONIZE_LOCK_FOR_RECENT_IDS) {
            sRecentOpenedMsgIds.clear();
        }
    }

    /**
     * Record recently opened messages' ids synchronously
     * @param messageId
     */
    public static void recordMessageIdSync(long messageId) {
        synchronized (SYNCHRONIZE_LOCK_FOR_RECENT_IDS) {
            if (!sRecentOpenedMsgIds.contains(messageId)) {
                int size = sRecentOpenedMsgIds.size();
                if (size >= MAX_RECORD_COUNT) {
                    sRecentOpenedMsgIds.remove(0);
                    sRecentOpenedMsgIds.add(size - 1, messageId);
                } else {
                    sRecentOpenedMsgIds.add(size, messageId);
                }
            }
            LogUtils.d(TAG, "recent Ids: " + sRecentOpenedMsgIds.toString());
        }
    }

    /**
     * Record converation's message id
     * @param c
     */
    public static void recordConversationMsgIdAsync(Conversation c) {
        if (c != null) {
            Long msgId = Long.parseLong(c.messageListUri.getLastPathSegment());
            LogUtils.d(TAG, "Record message id: " + msgId);
            if (msgId != null) {
                new EmailAsyncTask<Long, Void, Void>(null) {

                    @Override
                    protected Void doInBackground(Long... params) {
                        AttachmentAutoClearController.recordMessageIdSync(params[0]);
                        return null;
                    }
                } .executeExecutor(EmailAsyncTask.SERIAL_EXECUTOR_FOR_AUTO_CLEAR_ATTACH, msgId);
            }
        }
    }

    /**
     * action: clear old attachment files(internal storage) once, this is only called when we enter
     * low storage state, so we should try to do what we can to release as more space as possible
     * @param context
     */
    public static void actionClearOnce(Context context) {
        LogUtils.d(TAG, "actionClearOnce");
        Intent i = new Intent();
        i.setClass(context, AttachmentAutoClearService.class);
        i.setAction(AttachmentAutoClearService.ACTION_CLEAR_OLD_ATTACHMENT_ONCE);
        //context.startService(i);
        EmailApplication.checkAndStartForegroundService(context, i, "actionClearOnce");
    }

    /**
     * action: clear old attachment files(internal storage)
     * @param context
     */
    public static void actionAutoClear(Context context) {
        LogUtils.d(TAG, "actionAutoClear");
        // If auto clear is enabled
        if (Preferences.getPreferences(context).getAutoClearAtt()) {
            Intent i = new Intent();
            i.setClass(context, AttachmentAutoClearService.class);
            i.setAction(AttachmentAutoClearService.ACTION_CLEAR_OLD_ATTACHMENT);
            //context.startService(i);
            EmailApplication.checkAndStartForegroundService(context, i, "actionAutoClear");
        } else {
            actionCancelAutoClear(context);
        }
    }

    /**
     * Reschedule the action: clear old attachment files(internal storage)
     * @param context
     */
    public static void actionRescheduleAutoClear(Context context) {
        LogUtils.d(TAG, "actionRescheduleAutoClear");
        // If auto clear is enabled
        if (Preferences.getPreferences(context).getAutoClearAtt()) {
            Intent i = new Intent();
            i.setClass(context, AttachmentAutoClearService.class);
            i.setAction(AttachmentAutoClearService.ACTION_RESCHEDULE_CLEAR_OLD_ATTACHMENT);
            //context.startService(i);
            EmailApplication.checkAndStartForegroundService(context, i, "actionRescheduleAutoClear");
        } else {
            actionCancelAutoClear(context);
        }
    }

    /**
     * cancel clear old attachment files(internal storage)
     * @param context
     */
    public static void actionCancelAutoClear(Context context) {
        LogUtils.d(TAG, "actionCancelAutoClear");
        Intent i = new Intent();
        i.setClass(context, AttachmentAutoClearService.class);
        i.setAction(AttachmentAutoClearService.ACTION_CANCEL_CLEAR_OLD_ATTACHMENT);
        //context.startService(i);
        EmailApplication.checkAndStartForegroundService(context, i, "actionCancelAutoClear");
    }

    /**
     * This function delete the attachment files in internal storage which belongs to message
     * in non-(draft/sent/out)box some days before, however, in some special case, they won't be
     * deleted
     * @NOTE: in the following cases, attachment files won't be deleted:
     * 1, if the attachment belongs to an account enable wifi auto-download
     * 2, if the attachment belongs to a recently opened message
     * 3, the attachment files has just been download in the past day
     * @param days
     */
    public static void deleteInternalAttachmentsDaysBefore(Context context, int days) {
        deleteInternalAttachmentsTimeBefore(context, System.currentTimeMillis()
                - days * AttachmentAutoClearService.ONE_DAY_TIME);
    }

    /**
     * This function delete the attachment files in internal storage which belongs to message
     * in non-(draft/sent/out)box some time before, however, in some special case, they won't be
     * deleted
     * @NOTE: in the following cases, attachment files won't be deleted:
     * 1, if the attachment belongs to an account enable wifi auto-download
     * 2, if the attachment belongs to a recently opened message
     * 3, the attachment files has just been download in the past day
     * @param time
     */
    public static void deleteInternalAttachmentsTimeBefore(Context context, long time) {
        ContentResolver resolver = context.getContentResolver();
        // firstly, get all messages older than the specific time
        Cursor msgCursor = resolver.query(EmailContent.Message.CONTENT_URI,
                MESSAGEID_TO_ACCOUNTID_PROJECTION,
                "(" + MessageColumns.TIMESTAMP + " < ?) AND ("
                        + MessageColumns.FLAG_ATTACHMENT + " = 1" + ") AND ("
                        + Message.ALL_NON_OUTBOX_DRAFT_SENT_SELECTION + ")",
                new String[] { Long.toString(time) },
                null);
        // just do nothing if no messages found
        if (msgCursor == null) {
            return;
        }
        ArrayList<Long> wifiAccounts = new ArrayList<Long>();
        // secondly, get all accounts enable wifi auto downloading
        Cursor accountCursor = resolver.query(Account.CONTENT_URI, Account.ID_PROJECTION,
                "(" + AccountColumns.FLAGS + " & ?) != 0",
                new String[] {Integer.toString(Account.FLAGS_BACKGROUND_ATTACHMENTS)},
                null);
        if (accountCursor != null) {
            try {
                while (accountCursor.moveToNext()) {
                    wifiAccounts.add(accountCursor.getLong(Account.ID_PROJECTION_COLUMN));
                }
                LogUtils.d(TAG, "wifiAccounts: " + wifiAccounts.toString());
            } finally {
                accountCursor.close();
            }
        }
        time = System.currentTimeMillis() - AttachmentAutoClearService.ONE_DAY_TIME;
        try {
            while (msgCursor.moveToNext()) {
                long msgId = msgCursor.getLong(Message.ID_PROJECTION_COLUMN);
                long accountId = msgCursor.getLong(MESSAGEID_TO_ACCOUNTID_COLUMN_ACCOUNTID);
                // if the account dosen't enable wifi auto downloading, do the clear action
                if (!wifiAccounts.contains(accountId)) {
                    synchronized (SYNCHRONIZE_LOCK_FOR_RECENT_IDS) {
                        if (sRecentOpenedMsgIds == null || !sRecentOpenedMsgIds.contains(msgId)) {
                            AttachmentUtilities.deleteMsgAttachmentFiles(context, msgId,
                                    time);
                        }
                    }
                }
            }
        } finally {
            msgCursor.close();
        }
    }
}

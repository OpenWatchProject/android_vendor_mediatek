package com.android.emailcommon.utility;

import android.content.ContentProviderOperation;
import android.content.Context;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.os.RemoteException;
import android.util.Log;

import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.SmartPush;
import com.android.emailcommon.provider.EmailContent.Message;
import com.android.mail.utils.LogUtils;

import java.util.ArrayList;

/**
 * M: An untility class for collecting the user's habit of using Email
 */
public class DataCollectUtils {
    // The account list being recorded
    private static final ArrayList<Long> sAccountIds = new ArrayList<Long>();
    // The start time of current recording
    private static long sStartTime;
    // We will take down any EAS account viewed by the user during a Email using session.
    // Use this variable to store all the recorded account for avoiding record opening duplicated.
    private static final ArrayList<Long> sRecordedAccountIds = new ArrayList<Long>();

    /**
     * Start record an account using duration
     * @param context
     * @param accountId the account's id
     * @param recordOpening if true, record that the user used this account once.
     */
    public static void startRecord(final Context context, final long accountId, final boolean recordOpening) {
        EmailAsyncTask.runAsyncSerial(new Runnable() {
            @Override
            public void run() {
                sAccountIds.clear();
                // Add the account(s) to the recording account
                // list if it were an EAS account
                if (accountId == Account.ACCOUNT_ID_COMBINED_VIEW) {
                    getAllEasAccounts(context);
                } else if (accountId != Account.NO_ACCOUNT) {
                    addIfEasAccount(context, accountId);
                }

                sStartTime = System.currentTimeMillis();

                for (Long acctId : sAccountIds) {
                    Log.i("DataCollect", "record start, acctId:" + acctId);
                    // Just record the opening event for the account which had not
                    // been recorded during this session
                    if (recordOpening && !sRecordedAccountIds.contains(acctId)) {
                        Log.i("DataCollect", "record an open");
                        SmartPush sp = SmartPush.addEvent(context, sStartTime, acctId, SmartPush.TYPE_OPEN, 1);
                        sp.save(context);
                        sRecordedAccountIds.add(acctId);
                    }
                }
            }
        });
    }

    /**
     * Clear the recording account list,
     */
    public static void clearRecordedList() {
        sRecordedAccountIds.clear();
    }

    /**
     * Stop the recording of the duration for current account,
     * add the duration event to the database
     * @param context
     */
    public static void stopRecord(final Context context) {
        EmailAsyncTask.runAsyncSerial(new Runnable() {
            @Override
            public void run() {
                long duration = System.currentTimeMillis() - sStartTime;

                // Suppose a case like: stopRecord async task was executed before the startRecord due to
                // the "Parallel" executor. Just ignore this duration record since it is tiny.
                if (duration > 0) {
                    for (Long acctId : sAccountIds) {
                        Log.i("DataCollect", "record stop, acctId:" + acctId);
                        SmartPush sp = SmartPush.addEvent(context, sStartTime, acctId, SmartPush.TYPE_DURATION,
                                duration);
                        sp.save(context);
                    }
                    sAccountIds.clear();
                }
            }
        });
    }

    /**
     * Record the new-coming email one by one.
     * @param context
     * @param msgs the new-coming emails
     */
    public static void recordNewMails(final Context context, final ArrayList<Message> msgs) {
        EmailAsyncTask.runAsyncParallel(new Runnable() {
            @Override
            public void run() {
                /// M: Use applyBatch instead of massive single inserts @{
                ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
                for (Message msg : msgs) {
                    SmartPush sp = SmartPush.addEvent(context, msg.mTimeStamp, msg.mAccountKey, SmartPush.TYPE_MAIL, 1);
                    ops.add(ContentProviderOperation.newInsert(sp.mBaseUri).withValues(sp.toContentValues()).build());
                }
                try {
                    context.getContentResolver().applyBatch(SmartPush.AUTHORITY, ops);
                } catch (RemoteException re) {
                    LogUtils.e(LogUtils.TAG, re, "SmartPush recordNewMails fail!");
                } catch (OperationApplicationException oae) {
                    LogUtils.e(LogUtils.TAG, oae, "SmartPush recordNewMails fail!");
                }
                /// @}
            }
        });
    }

    /**
     * Add the account to the recording list if it were an EAS account
     * @param context
     * @param accountId
     */
    private static void addIfEasAccount(Context context, long accountId) {
        Account acct = Account.restoreAccountWithId(context, accountId);
        if (acct != null && "eas".equals(acct.getProtocol(context))) {
            sAccountIds.add(accountId);
        }
    }

    /**
     * Just for combined account, add its sub EAS account to the recording list
     * @param context
     */
    private static void getAllEasAccounts(Context context) {
        Cursor c = context.getContentResolver().query(Account.CONTENT_URI,
                new String[] {Account.RECORD_ID}, null, null, null);
        if (c != null) {
            try {
                while (c.moveToNext()) {
                    addIfEasAccount(context, c.getLong(0));
                }
            } finally {
                c.close();
            }
        }
    }
}

package com.mediatek.rcse.plugin.message;


import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.Telephony.Threads;

import com.mediatek.rcse.api.Logger;

import java.util.ArrayList;
import java.util.List;

/**
 * Class to combine/separate threads when integrated mode is changed.
 * @author MTK33296
 */
public class CombineAndSeperateUtil {
    private static String sTAG = "CombineAndSeperateUtil";

    /**
     * Constructor.
     * @author MTK33296
     */
    private class IpThreadIdAndThreadEntry {
        Long mIpThreadId;
        Long mMmsThreadId;
        String mAddress;
    }

    /**
     * Combine threads.
     * @param context context
     */
    public void combine(final Context context) {
        Logger.d(sTAG, "combine() entry");
        new Thread(new Runnable() {
            public void run() {
                List<IpThreadIdAndThreadEntry> ipThreadToThreadList;
                ipThreadToThreadList = getIpThreadIdAndThreadId(context);
                Logger.d(sTAG, "combine() ipThreadToThreadList size = "
                        + ipThreadToThreadList.size());
                for (IpThreadIdAndThreadEntry entry : ipThreadToThreadList) {
                    Long ipMessageCount = getMessageCount(context,
                            entry.mIpThreadId);
                    Long mmsMessageCount = getMessageCount(context,
                            entry.mMmsThreadId);
                    Logger.d(sTAG, "combine() ipMessageCount = "
                            + ipMessageCount + ", mmsMessageCount = "
                            + mmsMessageCount);
                    ContentValues updateMessageCountValues = new ContentValues(
                            1);
                    updateMessageCountValues.put("message_count",
                            ipMessageCount + mmsMessageCount);
                    ContentValues updateSmsValues = new ContentValues(2);
                    updateSmsValues.put("thread_id", entry.mMmsThreadId);
                    updateSmsValues.put("address", entry.mAddress);
                    context.getContentResolver().update(
                            Uri.parse("content://sms"),
                            updateSmsValues,
                            "thread_id" + "=" + entry.mIpThreadId + " and "
                                    + "ipmsg_id > 0", null);
                    Uri updateThreadUri = ContentUris
                            .withAppendedId(
                                    Uri.parse("content://mms-sms/conversations"),
                                    entry.mMmsThreadId).buildUpon()
                            .appendQueryParameter("isRcse", "true").build();
                    context.getContentResolver().update(updateThreadUri,
                            updateMessageCountValues, null, null);
                    context.getContentResolver().delete(
                            ContentUris.withAppendedId(Uri
                                    .parse("content://mms-sms/conversations"),
                                    entry.mIpThreadId), null, null);
                }
            }
        }).start();
    }
    /**
     * Separate threads.
     * @param context context
     */
    public void separate(final Context context) {
        Logger.d(sTAG, "separate() entry");
        new Thread(new Runnable() {
            public void run() {
                List<IpThreadIdAndThreadEntry> ipThreadToThreadList;
                ipThreadToThreadList = getThreadIdAndIpThreadId(context);
                Logger.d(sTAG, "separate() ipThreadToThreadList size = "
                        + ipThreadToThreadList.size());
                for (IpThreadIdAndThreadEntry entry : ipThreadToThreadList) {
                    int ipMessageCount = getIpMessageCount(context,
                            entry.mMmsThreadId);
                    Long allMessageCount = getMessageCount(context,
                            entry.mMmsThreadId);
                    ContentValues updateIpMessageCountValues = new ContentValues(
                            1);
                    ContentValues updateMmsMessageCountValues = new ContentValues(
                            1);
                    Logger.d(sTAG, "separate() ipMessageCount = "
                            + ipMessageCount + ", allMessageCount = "
                            + allMessageCount);
                    updateIpMessageCountValues.put("message_count",
                            ipMessageCount);
                    updateMmsMessageCountValues.put("message_count",
                            allMessageCount - ipMessageCount);
                    ContentValues updateSmsValues = new ContentValues(2);
                    updateSmsValues.put("thread_id", entry.mIpThreadId);
                    updateSmsValues.put("address", "9+++" + entry.mAddress);
                    context.getContentResolver().update(
                            Uri.parse("content://sms"),
                            updateSmsValues,
                            "thread_id" + "=" + entry.mMmsThreadId + " and "
                                    + "ipmsg_id > 0", null);
                    Uri updateIpThreadUri = ContentUris
                            .withAppendedId(
                                    Uri.parse("content://mms-sms/conversations"),
                                    entry.mIpThreadId).buildUpon()
                            .appendQueryParameter("isRcse", "true").build();
                    Uri updateMmsThreadUri = ContentUris
                            .withAppendedId(
                                    Uri.parse("content://mms-sms/conversations"),
                                    entry.mMmsThreadId).buildUpon()
                            .appendQueryParameter("isRcse", "true").build();
                    context.getContentResolver().update(updateIpThreadUri,
                            updateIpMessageCountValues, null, null);
                    context.getContentResolver().update(updateMmsThreadUri,
                            updateMmsMessageCountValues, null, null);
                }
            }
        }).start();
    }
    /**
     * get Ip thread Id.
     * @param context context
     * @return
     */
    private List<IpThreadIdAndThreadEntry> getIpThreadIdAndThreadId(
            Context context) {
        Cursor cursor = context.getContentResolver().query(
                Uri.parse("content://sms"),
                new String[] { "distinct thread_id", "address" },
                " ipmsg_id > 0 ", null, null);
        List<IpThreadIdAndThreadEntry> ipThreadToThreadList =
                new ArrayList<IpThreadIdAndThreadEntry>();
        try {
            if (cursor != null && cursor.moveToFirst()) {
                do {
                    String normalAddress = cursor.getString(1);
                    if (normalAddress.startsWith("9+++")) {
                        normalAddress = normalAddress.substring(4);
                        IpThreadIdAndThreadEntry ipThreadIdAndThreadEntry =
                                new IpThreadIdAndThreadEntry();
                        ipThreadIdAndThreadEntry.mIpThreadId = cursor
                                .getLong(0);
                        ipThreadIdAndThreadEntry.mAddress = normalAddress;
                        ipThreadIdAndThreadEntry.mMmsThreadId = Threads
                                .getOrCreateThreadId(context, normalAddress);
                        ipThreadToThreadList.add(ipThreadIdAndThreadEntry);
                    }
                } while (cursor.moveToNext());
            }
        } finally {
            cursor.close();
        }
        return ipThreadToThreadList;
    }
    /**
     * getThreadIdAndIpThreadId.
     * @param context context
     * @return
     */
    private List<IpThreadIdAndThreadEntry> getThreadIdAndIpThreadId(
            Context context) {
        Cursor cursor = context.getContentResolver().query(
                Uri.parse("content://sms"),
                new String[] { "distinct thread_id", "address" },
                " ipmsg_id > 0 ", null, null);
        List<IpThreadIdAndThreadEntry> ipThreadToThreadList =
                new ArrayList<IpThreadIdAndThreadEntry>();
        try {
            if (cursor != null && cursor.moveToFirst()) {
                do {
                    String normalAddress = cursor.getString(1);
                    if (normalAddress.startsWith("9+++")) {
                        continue;
                    }
                    IpThreadIdAndThreadEntry ipThreadIdAndThreadEntry =
                            new IpThreadIdAndThreadEntry();
                    ipThreadIdAndThreadEntry.mMmsThreadId = cursor.getLong(0);
                    ipThreadIdAndThreadEntry.mAddress = normalAddress;
                    ipThreadIdAndThreadEntry.mIpThreadId = Threads
                            .getOrCreateThreadId(context, "9+++"
                                    + normalAddress);
                    ipThreadToThreadList.add(ipThreadIdAndThreadEntry);
                } while (cursor.moveToNext());
            }
        } finally {
            cursor.close();
        }
        return ipThreadToThreadList;
    }
    /**
     * getMessageCount.
     * @param context context of app
     * @param threadId thread
     * @return
     */
    private Long getMessageCount(Context context, Long threadId) {
        Uri mmsThreadUri = ContentUris
                .withAppendedId(Uri.parse("content://mms-sms/conversations"),
                        threadId).buildUpon()
                .appendQueryParameter("isRcse", "true").build();
        Cursor cursor = context.getContentResolver().query(mmsThreadUri,
                new String[] { Threads.MESSAGE_COUNT }, null, null, null);
        Long messageCount = null;
        try {
            if (cursor != null && cursor.moveToFirst()) {
                messageCount = cursor.getLong(0);
            }
        } finally {
            cursor.close();
        }
        return messageCount;
    }
    /**
     * getIpMessageCount.
     * @param context context
     * @param threadId threadid
     * @return
     */
    private int getIpMessageCount(Context context, Long threadId) {
        Cursor cursor = context.getContentResolver().query(
                Uri.parse("content://sms"), null,
                "thread_id" + "=" + threadId + " and " + "ipmsg_id > 0", null,
                null);
        int messageCount = 0;
        try {
            if (cursor != null && cursor.moveToFirst()) {
                messageCount = cursor.getCount();
            }
        } finally {
            cursor.close();
        }
        return messageCount;
    }
}

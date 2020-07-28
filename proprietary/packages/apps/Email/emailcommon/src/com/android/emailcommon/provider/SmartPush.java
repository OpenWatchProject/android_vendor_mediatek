package com.android.emailcommon.provider;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;

import com.android.emailcommon.Logging;
import com.google.common.annotations.VisibleForTesting;

import com.mediatek.protect.emailcommon.provider.SmartPushContent;

/**
 * M: The smart push content
 */
public class SmartPush extends EmailContent implements Parcelable, SmartPushContent {
    public static final String TABLE_NAME = "SmartPush";

    public long mAccountKey;
    public int mEventType;
    public long mTimestamp;
    public long mValue;

    private static final int CONTENT_ID_COLUMN = 0;
    private static final int ACCOUNT_KEY_COLUMN = 1;
    private static final int EVENT_TYPE_COLUMN = 2;
    private static final int TIMESTAMP_COLUMN = 3;
    private static final int VALUE_COLUMN = 4;

    public static final Uri CONTENT_URI = Uri.parse(EmailContent.CONTENT_URI
            + "/smartpush");

    private static final String[] ACCOUNT_FLAGS_PROJECTION = new String[] {
        AccountColumns.FLAGS
        };
    private static final String ACCOUNT_ID_SELECTION = AccountColumns._ID + " =?";

    public SmartPush() {
        mBaseUri = CONTENT_URI;
    }

    @Override
    public void restore(Cursor cursor) {
        mBaseUri = CONTENT_URI;
        mId = cursor.getLong(CONTENT_ID_COLUMN);
        mAccountKey = cursor.getLong(ACCOUNT_KEY_COLUMN);
        mEventType = cursor.getInt(EVENT_TYPE_COLUMN);
        mTimestamp = cursor.getLong(TIMESTAMP_COLUMN);
        mValue = cursor.getLong(VALUE_COLUMN);
    }

    @Override
    public ContentValues toContentValues() {
        ContentValues values = new ContentValues();
        values.put(ACCOUNT_KEY, mAccountKey);
        values.put(EVENT_TYPE, mEventType);
        values.put(TIMESTAMP, mTimestamp);
        values.put(VALUE, mValue);
        return values;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeLong(mId);
        dest.writeLong(mAccountKey);
        dest.writeInt(mEventType);
        dest.writeLong(mTimestamp);
        dest.writeLong(mValue);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append('[');
        sb.append(mId);
        sb.append(", ");
        sb.append(mAccountKey);
        sb.append(", ");
        sb.append(mEventType);
        sb.append(", ");
        sb.append(mTimestamp);
        sb.append(", ");
        sb.append(mValue);
        sb.append("]");
        return sb.toString();
    }

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof SmartPush)) {
            return false;
        }
        SmartPush thatObject = (SmartPush) o;
        return mId == thatObject.mId && mAccountKey == thatObject.mAccountKey
                && mTimestamp == thatObject.mTimestamp
                && mValue == thatObject.mValue
                && mEventType == thatObject.mEventType;
    }

    @Override
    public int hashCode() {
        return (int) mTimestamp;
    }

    /**
     * Add a smart push event record to the database, the event can be "a new email comes",
     * "an using duration record" or "open Email app at a time"
     * @param context
     * @param timeStamp the time stamp that the event happens
     * @param accountId the ID of the account which the event belong to
     * @param eventType the type of the event (e.g. TYPE_MAIL, TYPE_OPEN, TYPE_DURATION)
     * @param value     the value of the event
     * @return a SmartPush object stands for a habit data record
     */
    public static SmartPush addEvent(Context context, long timeStamp, long accountId, int eventType, long value) {
        if (accountId == Account.NO_ACCOUNT) {
            return null;
        }

        SmartPush sp = new SmartPush();
        sp.mAccountKey = accountId;
        sp.mEventType = eventType;
        sp.mTimestamp = timeStamp;
        sp.mValue = value;
        return sp;
    }

    /**
     * Check if the account was a smart push EAS account
     * @param context
     * @param accountId the account's id
     * @return true if the account was a smart push EAS account
     */
    @VisibleForTesting
    public static boolean isSmartPushAccount(Context context, long accountId) {
      Cursor c = context.getContentResolver().query(Account.CONTENT_URI, ACCOUNT_FLAGS_PROJECTION, ACCOUNT_ID_SELECTION,
              new String[]{String.valueOf(accountId)}, null);
      if (c != null) {
          try {
              if (c.moveToFirst() && (c.getInt(0) & Account.FLAGS_SMART_PUSH) != 0) {
                  return true;
              }
          } finally {
              c.close();
         }
      }
      return false;
    }

    /**
     * Delete all the habit data record of the specified account
     * @param context
     * @param accountId which habit data will be deleted
     */
    @VisibleForTesting
    public static void deleteHabitData(Context context, long accountId) {
        int deleted =  context.getContentResolver().delete(CONTENT_URI,
                MailboxColumns.ACCOUNT_KEY + "=?", new String[]{String.valueOf(accountId)});
        Logging.v(deleted + " rows hait data deleted");
    }
}

package com.mediatek.email.provider.history;

import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import com.android.emailcommon.Logging;

/**
 * M: Add EmailAddressContent for email address history
 */
public abstract class EmailAddressContent {

    public static final String AUTHORITY = "com.mediatek.email.provider.history";
    public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY);

    public static final String PROVIDER_PERMISSION = "com.mediatek.email.provider.history.permission.ACCESS_PROVIDE";

    public static final String TAG = "EmailAddrerssContent";

    // define the interface for using the name of the items
    public interface AddressColumns {
        String ID = "_id";
        String EMAIL_ADDRESS = "emailAddress";
    }

    // Newly created objects get this id
    public static final int NOT_SAVED = -1;
    // The base Uri that this piece of content came from
    public Uri mBaseUri;
    // The id of the Content
    public long mId = NOT_SAVED;

    // Write the Content into a ContentValues container
    public abstract ContentValues toContentValues();

    // Read the Content from a ContentCursor
    public abstract void restore(Cursor cursor);

    // The Content sub class must have a no-arg constructor
    static public <T extends EmailAddressContent> T getContent(Cursor cursor, Class<T> klass) {
        try {
            T content = klass.newInstance();
            content.mId = cursor.getLong(0);
            content.restore(cursor);
            return content;
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * Restore a subclass of EmailAddressContent from the database
     *
     * @param context
     *            the caller's context
     * @param klass
     *            the class to restore
     * @param contentUri
     *            the content uri of the SNSContent subclass
     * @param contentProjection
     *            the content projection for the SNSContent subclass
     * @param id
     *            the unique id of the object
     * @return the instantiated object
     */
    public static <T extends EmailAddressContent> T restoreAddresstWithId(Context context,
            Class<T> klass, Uri contentUri, String[] contentProjection, long id) {
        Uri u = ContentUris.withAppendedId(contentUri, id);
        Cursor c = null;
        try {
            c = context.getContentResolver().query(u, contentProjection, null, null, null);
            if (c == null) {
                throw new IllegalStateException();
            }
            try {
                if (c.moveToFirst()) {
                    return getContent(c, klass);
                } else {
                    return null;
                }
            } finally {
                c.close();
            }
        } catch (IllegalStateException e) {
            Logging.w(TAG, "EmailAddrerssContent#restoreContentWithId throw out IllegalStateException",
                    e);
        } catch (SQLiteException e) {
            Logging.w(TAG, "EmailAddrerssContent#restoreContentWithId throw out SQLiteException", e);
        }
        return null;
    }
}

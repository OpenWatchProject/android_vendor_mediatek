package com.mediatek.email.provider.history;

import com.android.emailcommon.Logging;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

/**
 * M: Add EmailAddress for address table
 */
public final class EmailAddress extends EmailAddressContent implements
        EmailAddressContent.AddressColumns {
    private static final String TAG = "EmailAddress";
    public static final String TABLE_NAME = "Address";
    public static final Uri CONTENT_URI = Uri.parse(EmailAddressContent.CONTENT_URI + "/address");
    public static final Uri CONTENT_QUERY_URI = Uri.parse(CONTENT_URI + "/filter");

    // define the items of the address table
    public String mAddress;

    private static final int CONTENT_ID_COLUMN = 0;
    private static final int CONTENT_ADDRESS_COLUMN = 1;

    static String[] CONTENT_PROJECTION = new String[] { AddressColumns.ID,
            AddressColumns.EMAIL_ADDRESS };

    public EmailAddress() {
        // Add default values if need
        mBaseUri = CONTENT_URI;
    }

    public static EmailAddress restoreAddressWithId(Context context, long id) {
        return EmailAddressContent.restoreAddresstWithId(context, EmailAddress.class,
                EmailAddress.CONTENT_URI, EmailAddress.CONTENT_PROJECTION, id);
    }

    public String getEmailAddress() {
        return mAddress;
    }

    public void setEmailAddress(String emailAddress) {
        mAddress = emailAddress;
    }

    @Override
    public ContentValues toContentValues() {
        ContentValues values = new ContentValues();
        values.put(AddressColumns.EMAIL_ADDRESS, mAddress);
        return values;
    }

    @Override
    public void restore(Cursor cursor) {
        mId = cursor.getLong(CONTENT_ID_COLUMN);
        mBaseUri = CONTENT_URI;
        mAddress = cursor.getString(CONTENT_ADDRESS_COLUMN);
    }

    public static void saveAddress(Context context, String text) {
        ContentValues cv = new ContentValues();
        cv.put(AddressColumns.EMAIL_ADDRESS, text);
        Uri res = context.getContentResolver().insert(CONTENT_URI, cv);
        if (res == null) {
            Logging.v(TAG, "address is invalid");
        }
    }

    public static Cursor queryAddress(Context context, Uri uri) {
        return context.getContentResolver().query(uri, EmailAddress.CONTENT_PROJECTION, null, null,
                null);
    }
}

package com.mediatek.contacts.plugin;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.DisplayNameSources;
import android.util.Log;

import com.mediatek.contacts.ext.DefaultContactsCommonPresenceExtension;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;

/**
 * Plugin implementation for Contacts Common Presence.
 */
public class Op08ContactsCommonPresenceExt extends
        DefaultContactsCommonPresenceExtension {
    private static final String TAG = "Op08ContactsCommonPresenceExt";

    private static final int ALPHA_TRANSPARENT_VALUE = 100;
    private static final int UNKNOWN = 0;
    private static final int EDITOR = 6;
    private static final String EXTRA_PREVIOUS_SCREEN_TYPE = "previous_screen_type";
    private static final String LOOKUP_URI_ENCODED = "encoded";

    private Context mContext;
    private PresenceApiManager mInstance = null;

    /**
     * Constructor.
     * @param context context
     */
    public Op08ContactsCommonPresenceExt(Context context) {
        Log.d(TAG, "[Op08ContactsCommonPresenceExt] constuctor entry "
                + context);
        mContext = context;
        if (PresenceApiManager.initialize(context)) {
            mInstance = PresenceApiManager.getInstance();
        }
    }

    /**
     * Set Video Icon alpha value.
     * @param number contact number.
     * @param thirdIcon video icon.
     * @param isVideoEnabled device is video enabled or not.
     */
    @Override
    public void setVideoIconAlpha(String number, Drawable thirdIcon,
            boolean isVideoEnabled) {
        LogUtils.d(TAG, "setVideoIconAlpha number " + LogUtils.anonymize(number));
        if (mInstance != null && isVideoEnabled) {
            boolean isCapable = mInstance.isVideoCallCapable(number);
            Log.d(TAG, "setVideoIconAlpha isVideoEnabled " + isVideoEnabled +
                    " isCapable " + isCapable);
            if (!isCapable && thirdIcon != null) {
                thirdIcon.mutate();
                thirdIcon.setAlpha(ALPHA_TRANSPARENT_VALUE);
            }
        }
    }

    private void requestFromDispalyNumber(Uri uri) throws JSONException {
        if (uri.getLastPathSegment().equals(LOOKUP_URI_ENCODED)) {
            final String jsonString = uri.getEncodedFragment();
            final JSONObject json = new JSONObject(jsonString);
            final String displayName = json.optString(Contacts.DISPLAY_NAME);
            final int displayNameSource = json.getInt(Contacts.DISPLAY_NAME_SOURCE);
            Log.d(TAG, "requestFromDispalyNumber displayName: "
                    + LogUtils.anonymize(displayName) +
                    " displayNameSource: " + LogUtils.anonymize(displayNameSource));
            if (mInstance != null && displayNameSource == DisplayNameSources.PHONE) {
                mInstance.requestContactPresence(displayName, true);
            }
        }
    }

    /**
     * Send subscibe when contact edit or view.
     *
     * @param intent Intent.
     */
    @Override
    public void processIntent(Intent intent) {
        Log.d(TAG, "[processIntent] " + intent);
        if (intent == null) {
            return;
        }
        if (mInstance == null) {
            Log.d(TAG, "mTapi is null");
            return;
        }
        final Uri lookupUri = intent.getData();
        if (null != lookupUri) {
            int previousType = intent.getIntExtra(EXTRA_PREVIOUS_SCREEN_TYPE, UNKNOWN);
            Log.d(TAG, "[processIntent] type: " + previousType);
            if (previousType == EDITOR) {
                List<String> numberList = mInstance.getNumbersByContactId(Long.decode(lookupUri
                        .getLastPathSegment()));
                for (String number : numberList) {
                    mInstance.requestContactPresence(number, true);
                }
            } else {
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        Cursor cursor = null;
                        long contactId = -1;
                        try {
                            cursor = mContext.getContentResolver().query(lookupUri,
                                    new String[] { Contacts._ID }, null, null, null);
                            if (null == cursor || !cursor.moveToFirst()) {
                                Log.e(TAG, "processIntent error when loading cursor");
                                //Get number if no save in contacts
                                try {
                                    requestFromDispalyNumber(lookupUri);
                                } catch (JSONException je) {
                                    Log.e(TAG, "processIntent json exception");
                                }
                                return;
                            }
                            int indexContactId = cursor.getColumnIndex(Contacts._ID);
                            do {
                                contactId = cursor.getLong(indexContactId);
                            } while (cursor.moveToNext());
                        } finally {
                            if (null != cursor) {
                                cursor.close();
                            }
                        }
                        if (contactId > 0) {
                            Log.d(TAG, "[processIntent] request Capabilities By Id");
                            List<String> numberList = mInstance.getNumbersByContactId(contactId);
                            for (String number : numberList) {
                                mInstance.requestContactPresence(number, true);
                            }
                        }
                    }
                });
            }
        }
    }

}

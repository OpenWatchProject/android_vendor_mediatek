package com.mediatek.contacts.plugin;

import android.app.Activity;
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
import com.mediatek.contacts.plugin.PresenceApiManager.CapabilitiesChangeListener;
import com.mediatek.contacts.plugin.PresenceApiManager.ContactInformation;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Plugin implementation for Contacts Common Presence.
 */
public class Op07ContactsCommonPresenceExt extends
        DefaultContactsCommonPresenceExtension implements CapabilitiesChangeListener {
    private static final String TAG = "Op07ContactsCommonPresenceExt";

    private static final int ALPHA_TRANSPARENT_VALUE = 100;
    private static final int ALPHA_NO_TRANSPARENT_VALUE = 255;
    private static final int UNKNOWN = 0;
    private static final int EDITOR = 6;
    private static final String EXTRA_PREVIOUS_SCREEN_TYPE = "previous_screen_type";
    private static final String LOOKUP_URI_ENCODED = "encoded";

    private Context mContext;
    private PresenceApiManager mTapi = null;
    private Activity mHostActivity;
    private boolean mHostActivityResumed;
    private Map<String, Drawable> mIconMaps = new ConcurrentHashMap<>();

    /**
     * Constructor.
     * @param context context
     */
    public Op07ContactsCommonPresenceExt(Context context) {
        Log.d(TAG, "[OP07ContactsCommonPresenceExt] constuctor entry " + context);
        mContext = context;
        if (PresenceApiManager.initialize(context)) {
            mTapi = PresenceApiManager.getInstance();
            mTapi.addCapabilitiesChangeListener(this);
        }
    }

    /**
     * Checks if plugin is active for test.
     * @return true if op07 plugin active.
     */
    //@Override
    //public boolean isShowVideoIcon() {
    //    Log.d(TAG, "isShowVideoIcon plugin implementation");
    //    return true;
    //}

    /**
     * Set Video Icon alpha value.
     * @param number contact number.
     * @param thirdIcon video icon.
     */
    @Override
    public void setVideoIconAlpha(String number, Drawable thirdIcon,
            boolean isVideoEnabled) {
        LogUtils.d(TAG, "setVideoIconAlpha number " + LogUtils.anonymize(number));
        String formatNumber = ContactNumberUtils.getDefault().getFormatNumber(number.trim());
        LogUtils.d(TAG, "[setVideoIconAlpha] number before format:"
                + LogUtils.anonymize(number)
                + ", after format:" + LogUtils.anonymize(formatNumber)
                + ", thirdIcon:" + thirdIcon
                + ", isVideoEnabled:" + isVideoEnabled);
        boolean isIconDisabled = true;
        if (formatNumber != null && thirdIcon != null) {
            mIconMaps.put(formatNumber, thirdIcon);
            if (mTapi != null && isVideoEnabled) {
                boolean isCapable = mTapi.isVideoCallCapable(number);
                Log.d(TAG, "setVideoIconAlpha isCapable: " + isCapable);
                if (isCapable) {
                    isIconDisabled = false;
                }
            }
        }
        if (thirdIcon != null  && isIconDisabled) {
            thirdIcon.mutate();
            thirdIcon.setAlpha(ALPHA_TRANSPARENT_VALUE);
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
            if (mTapi != null && displayNameSource == DisplayNameSources.PHONE) {
                mTapi.requestContactPresence(displayName, false);
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
        mIconMaps.clear();
        if (mTapi == null) {
            Log.d(TAG, "mTapi is null");
            return;
        }
        final Uri lookupUri = intent.getData();
        if (null != lookupUri) {
            int previousType = intent.getIntExtra(EXTRA_PREVIOUS_SCREEN_TYPE, UNKNOWN);
            Log.d(TAG, "[processIntent] type: " + previousType);
            if (previousType == EDITOR) {
                List<String> numberList = mTapi.getNumbersByContactId(Long.decode(lookupUri
                        .getLastPathSegment()));
                for (String number : numberList) {
                    mTapi.requestContactPresenceDelay(number, false);
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
                            mTapi.requestCapabilitiesById(contactId);
                        }
                    }
                });
            }
        }
    }

    @Override
    public void onHostActivityResumed(Activity activity) {
        Log.d(TAG, "[onHostActivityResumed]");
        mHostActivity = activity;
        mHostActivityResumed = true;
    }

    @Override
    public void onHostActivityPaused() {
        Log.d(TAG, "[onHostActivityPaused]");
        mHostActivityResumed = false;
    }

    @Override
    public void onHostActivityStopped() {
        Log.d(TAG, "[onHostActivityStopped]");
        mHostActivity = null;
        mHostActivityResumed = false;
    }

    @Override
    public void onCapabilitiesChanged(String contact, ContactInformation info) {
        Log.d(TAG, "[onCapabilitiesChanged]contact=" + LogUtils.anonymize(contact)
                + ", isVideoCapable=" + info.isVideoCapable);
        if (info.isVideoCapable) {
            switchVideoIconState(contact, true);
        }
    }

    private void switchVideoIconState(String contact, boolean isEnable) {
        if (mHostActivity == null || mHostActivityResumed == false) {
            Log.w(TAG, "[switchVideoIconState] ignore. mHostActivity=" + mHostActivity +
                    ", mHostActivityResumed=" + mHostActivityResumed);
            return;
        }
        mHostActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Drawable icon = mIconMaps.get(contact);
                Log.d(TAG, "[switchVideoIconState]UiThread. contact="
                        + LogUtils.anonymize(contact) + ", icon=" + icon);
                if (icon != null && mHostActivity != null) {
                    //disalbe icon for avaiable query fail.
                    icon.mutate();
                    if (isEnable) {
                        icon.setAlpha(ALPHA_NO_TRANSPARENT_VALUE);
                    } else {
                        icon.setAlpha(ALPHA_TRANSPARENT_VALUE);
                    }
                }
            }
        });
    }
}
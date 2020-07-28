package com.mediatek.contacts.plugin;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.DisplayNameSources;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

import com.mediatek.contacts.plugin.R;
import com.mediatek.contacts.ext.DefaultContactsCommonPresenceExtension;
import com.mediatek.contacts.plugin.PresenceApiManager.CapabilitiesChangeListener;
import com.mediatek.contacts.plugin.PresenceApiManager.ContactInformation;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * Plugin implementation for Contacts Common Presence.
 */
public class Op12ContactsCommonPresenceExt extends
        DefaultContactsCommonPresenceExtension implements CapabilitiesChangeListener {
    private static final String TAG = "Op12ContactsCommonPresenceExt";

    private static final int REFRESH_MENU_ID = 101;
    private static final int ALPHA_TRANSPARENT_VALUE = 100;
    private static final int ALPHA_NO_TRANSPARENT_VALUE = 255;
    private static final int UNKNOWN = 0;
    private static final int EDITOR = 6;
    private static final String EXTRA_PREVIOUS_SCREEN_TYPE = "previous_screen_type";
    private static final String LOOKUP_URI_ENCODED = "encoded";

    private Context mContext;
    private Activity mHostActivity;
    private boolean mHostActivityResumed;
    private PresenceApiManager mPresenceManager = null;

    private Map<String, Drawable> mIconMaps = new ConcurrentHashMap<>();
    private Map<String, QueryState> mCurrQueryStateCache = new ConcurrentHashMap<>();

    public enum QueryState {
        NONE,
        CAPABILITY_DONE,  //capablitiliy querie before, but maybe not support vilte.
        AVAILABLILITY_START,
        AVAILABLILITY_DONE,
        PRESENCE_ERROR,
    }

    @Override
    public void onCapabilitiesChanged(String contact, ContactInformation info) {
        QueryState state = mCurrQueryStateCache.get(contact);
        Log.d(TAG, "[onCapabilitiesChanged]contact=" + LogUtils.anonymize(contact) +
                ", state=" + state);
        if (state == null) {
            return;
        }
        if (state == QueryState.NONE) {
            // Have vilt capability, query availability
            if (mPresenceManager.requestContactsAvailability(contact)) {
                mCurrQueryStateCache.put(contact, QueryState.AVAILABLILITY_START);
            } else {
                mCurrQueryStateCache.put(contact, QueryState.CAPABILITY_DONE);
            }
        } else if (state == QueryState.AVAILABLILITY_START) {
            // query availability done and update icon status if needed.
            mCurrQueryStateCache.put(contact, QueryState.AVAILABLILITY_DONE);
            if (info.isVideoCapable && info.isVoiceCapable) {
                switchVideoIconState(contact, true);
            }
        } else {
            Log.e(TAG, "[onCapabilitiesChanged]unexpected state:" + state.ordinal());
        }
        dumpCurrentQueryStateCache();
    }

    @Override
    public void onErrorReceived(String contact, int type, int status, String reason) {
        if (type == PresenceApiManager.QUERY_TYPE_AVAILABILITY) {
            mCurrQueryStateCache.put(contact, QueryState.PRESENCE_ERROR);
            switchVideoIconState(contact, false);
        }
        showDialog(reason);
    }

    //Pure UI requirement for 500ms available timeout, waive it.
    @Override
    public void onQueryAvailabilityTimeout() {
    }

    private void switchVideoIconState(String contact, boolean isEnable) {
        Log.d(TAG, "[switchVideoIconState] contact=" + LogUtils.anonymize(contact)
                + ", isEnable=" + isEnable);
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

    /**
     * Constructor.
     * @param context context
     */
    public Op12ContactsCommonPresenceExt(Context context) {
        Log.d(TAG, "[OP12ContactsCommonPresenceExt] constuctor entry " + context);
        mContext = context;
        if (PresenceApiManager.initialize(context)) {
            mPresenceManager = PresenceApiManager.getInstance();
            mPresenceManager.addCapabilitiesChangeListener(this);
        }
    }

    @Override
    public boolean isShowVideoIcon() {
        if (mPresenceManager != null) {
            boolean isVtEnabled = mPresenceManager.isVtEnabled();
            boolean isAirPlaneMode = mPresenceManager.isAirPlaneMode();
            if (!isVtEnabled || isAirPlaneMode) {
                return false;
            }
        //    boolean isVoiceCapable = mPresenceManager.isVoiceCapable(number);
        //    Log.d(TAG, "[isShowVideoIcon] isVoiceCapable=" + isVoiceCapable );
        }
        return true;
    }

    /**
     * Set Video Icon alpha value.
     * @param number contact number.
     * @param thirdIcon video icon.
     */
    @Override
    public void setVideoIconAlpha(String number, Drawable thirdIcon,
            boolean isVideoEnabled) {
        String formatNumber = ContactNumberUtils.getDefault().getFormatNumber(number.trim());
        LogUtils.d(TAG, "[setVideoIconAlpha] number before format:"
                + LogUtils.anonymize(number) +
                ", after format:" + LogUtils.anonymize(formatNumber)
                + ", thirdIcon:" + thirdIcon);
        boolean isIconDisabled = true;
        if (formatNumber != null && thirdIcon != null) {
            mIconMaps.put(formatNumber, thirdIcon);
            if (mPresenceManager != null && isVideoEnabled) {
                boolean isCapable = mPresenceManager.isVideoCallCapable(number);
                Log.d(TAG, "setVideoIconAlpha isVideoEnabled: " + isVideoEnabled +
                        " isCapable: " + isCapable);
                if (isCapable) {
                    isIconDisabled = false;
                }
            }
        }
        if (thirdIcon != null  && isIconDisabled) {
            thirdIcon.mutate();
            thirdIcon.setAlpha(ALPHA_TRANSPARENT_VALUE);
        }
        return;
    }

    @Override
    public boolean isVideoIconClickable(Uri uri) {
        Log.d(TAG, "[isVideoIconClickable] uri=" + uri);
        boolean ret = true;
        String number = null;
        String formatNumber = null;

        if (uri == null) {
            return ret;
        }
        String scheme = uri.getScheme();

        if (("tel").equals(scheme) || ("sip").equals(scheme)) {
            number = uri.getSchemeSpecificPart();
            if (number != null) {
                formatNumber = ContactNumberUtils.getDefault().getFormatNumber(number.trim());
            }
        }
        Log.d(TAG, "[isVideoIconClickable] number=" + LogUtils.anonymize(number)
                + ", formatNumber=" + LogUtils.anonymize(formatNumber));

        Drawable icon = mIconMaps.get(formatNumber);
        if (icon != null && ALPHA_TRANSPARENT_VALUE == icon.getAlpha()) {
            ret = false;
        }

        Log.d(TAG, "[isVideoIconClickable] ret=" + ret + "icon=" + icon);
        return ret;
    }

    private List<String> getValidNumbersFromDispalyNumber(Uri uri) throws JSONException {
        List<String> list = new ArrayList<String>();
        if (uri.getLastPathSegment().equals(LOOKUP_URI_ENCODED)) {
            final String jsonString = uri.getEncodedFragment();
            final JSONObject json = new JSONObject(jsonString);
            final String displayName = json.optString(Contacts.DISPLAY_NAME);
            final int displayNameSource = json.getInt(Contacts.DISPLAY_NAME_SOURCE);
            Log.d(TAG, "[getValidNumbersFromDispalyNumber] displayName: "
                    + LogUtils.anonymize(displayName) +
                    ", displayNameSource: " + LogUtils.anonymize(displayNameSource));
            if (mPresenceManager != null && displayNameSource == DisplayNameSources.PHONE) {
                list.add(displayName);
            }
        }
        return list;
    }

     private List<String> getValidNumbersByContactId(long contactId) {
        Log.d(TAG, "[getValidNumbersByContactId] contactId=" + contactId);
        List<String> list = new ArrayList<String>();
        String[] projection = { Phone.NUMBER };
        String selection = Phone.CONTACT_ID + "=? ";
        String[] selectionArgs = { Long.toString(contactId) };
        Cursor cursor = mContext.getContentResolver().query(Phone.CONTENT_URI,
            projection, selection, selectionArgs, null);
        try {
            if (cursor == null) {
                Log.d(TAG, "[getValidNumbersByContactId] cursor is null");
                return list;
            }
            while (cursor.moveToNext()) {
                String number = cursor.getString(0);
                String formatNumber = null;
                if (!TextUtils.isEmpty(number)) {
                    formatNumber = ContactNumberUtils.getDefault().
                            getFormatNumber(number.trim().replace(" ", ""));
                    if (formatNumber != null && !TextUtils.isEmpty(formatNumber)) {
                        list.add(formatNumber);
                    }
                }
                Log.w(TAG, "[getValidNumbersByContactId]number=" + LogUtils.anonymize(number)
                         + ", formatNumber=" + LogUtils.anonymize(formatNumber));
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        // remove duplicate number
        if (!list.isEmpty()) {
            HashSet<String> set = new HashSet<String>(list);
            list.clear();
            list.addAll(set);
        }
        return list;
    }

    private void requestContactsCapabilityOrAvailability(List<String> list) {
        if (list == null || list.isEmpty()) {
            return;
        }

        List<String> capQueryList = new ArrayList<String>();
        for (String number : list) {
            Log.w(TAG, "[requestContactsCapabilityOrAvailability] number: "
                    + LogUtils.anonymize(number));
            boolean isVideoCapable = mPresenceManager.isVideoCallCapable(number);
            if (isVideoCapable) {
                // query available if capability support.
                if (mPresenceManager.requestContactsAvailability(number)) {
                    mCurrQueryStateCache.put(number, QueryState.AVAILABLILITY_START);
                } else {
                    mCurrQueryStateCache.put(number, QueryState.CAPABILITY_DONE);
                }
            } else {
                // query capablility if no capability.
                mCurrQueryStateCache.put(number, QueryState.NONE);
                capQueryList.add(number);
            }
        }
        dumpCurrentQueryStateCache();
        if (!capQueryList.isEmpty()) {
            mPresenceManager.requestContactsCapability(capQueryList);
        }
    }

    private void requestContactsCapability(List<String> list) {
        if (list == null || list.isEmpty()) {
            return;
        }
        mPresenceManager.requestContactsCapability(list);
        dumpCurrentQueryStateCache();
    }

    private void requestContactsAvailability(List<String> list) {
        if (list == null || list.isEmpty()) {
            return;
        }
        List<String> queryList = new ArrayList<String>();
        for (String number : list) {
            Log.w(TAG, "[requestContactsAvailability]list: " + LogUtils.anonymize(number));
            boolean isVideoCapable = mPresenceManager.isVideoCallCapable(number);
            //if (isVideoCapable) {
                if (mPresenceManager.requestContactsAvailability(number)) {
                    mCurrQueryStateCache.put(number, QueryState.AVAILABLILITY_START);
                } else {
                    mCurrQueryStateCache.put(number, QueryState.CAPABILITY_DONE);
                }
            //} else {
            //    mCurrQueryStateCache.put(number, QueryState.CAPABILITY_DONE);
            //}
        }
        dumpCurrentQueryStateCache();
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
        mCurrQueryStateCache.clear();
        if (mPresenceManager == null) {
            Log.d(TAG, "[processIntent] mPresenceManager is null");
            return;
        }
        final Uri lookupUri = intent.getData();
        int previousType = intent.getIntExtra(EXTRA_PREVIOUS_SCREEN_TYPE, UNKNOWN);
        Log.d(TAG, "[processIntent] type= " + previousType + ", uri=" + lookupUri);
        if (null == lookupUri) {
            return;
        }

        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                List<String> list = new ArrayList<String>();
                long contactId = -1;
                if (previousType == EDITOR) {
                    // For edit action, query Capability firstly.
                    contactId = Long.decode(lookupUri.getLastPathSegment());
                    list = getValidNumbersByContactId(contactId);
                    requestContactsCapabilityOrAvailability(list);
                } else {
                    // For view action, query Availability directly.
                    boolean isQueryByUri = false;
                    Cursor cursor = null;
                    try {
                        cursor = mContext.getContentResolver().query(lookupUri,
                                new String[] { Contacts._ID }, null, null, null);
                        if (null == cursor || !cursor.moveToFirst()) {
                            Log.e(TAG, "[processIntent] error when loading cursor");
                            //Get number if no save in contacts
                            isQueryByUri = true;
                        } else {
                            int indexContactId = cursor.getColumnIndex(Contacts._ID);
                            do {
                                contactId = cursor.getLong(indexContactId);
                                Log.e(TAG, "[processIntent]loop contactId=" + contactId);
                            } while (cursor.moveToNext());
                        }
                    } finally {
                        if (null != cursor) {
                            cursor.close();
                        }
                    }
                    //get numbers list and query availability
                    if (isQueryByUri) {
                        try {
                            list = getValidNumbersFromDispalyNumber(lookupUri);
                        } catch (JSONException je) {
                            Log.e(TAG, "[processIntent] json exception");
                        }
                    } else if (contactId >= 0) {
                        list = getValidNumbersByContactId(contactId);
                    }
                    requestContactsAvailability(list);
                }
            }
        });
    }

    /**
     * Add a Refresh menu item.
     * @param menu item in which we need to add
     */
     @Override
     public void addRefreshMenu(Menu menu) {
        Log.d(TAG, "addRefreshMenu Entry");
        MenuItem refreshMenu = menu.findItem(REFRESH_MENU_ID);
        if (refreshMenu == null) {
            Log.d(TAG, "[addRefreshMenu] add menu id:" + REFRESH_MENU_ID
                    + ", resID=" + R.string.refresh_menu);
            menu.add(Menu.NONE, REFRESH_MENU_ID, Menu.NONE, "Refresh");
                    //mContext.getString(R.string.refresh_menu));
        }
    }

    /**
     * handling of refresh menu item.
     * @param itemId menuItem id
     * @param contactId Contact id
     * @return true if plugin handles this else false.
     */
     @Override
     public boolean onOptionsItemSelected(int itemId, long contactId) {
        Log.d(TAG, "[onOptionsItemSelected] itemId:" + itemId + ", contactId=" + contactId);
         //@TODO: skip for test
        //mPresenceManager.sendMessage(mPresenceManager.MESSAGE_TEST_AVAIBLE_ERROR, 5000);
        if (mPresenceManager != null && itemId == REFRESH_MENU_ID) {
            // refetch the latest capability of contactid
            mCurrQueryStateCache.clear();
            List<String> numbers = getValidNumbersByContactId(contactId);
            for (String number : numbers) {
                mCurrQueryStateCache.put(number, QueryState.NONE);
            }
            requestContactsCapability(numbers);
            return true;
        }
        return false;
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

    private void showDialog(String error) {
        if (mHostActivity != null && mHostActivityResumed) {
            Log.d(TAG, "[showDialog]" + error);
            AlertDialog dialog = new AlertDialog.Builder(mHostActivity)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setMessage(error)
                    .setPositiveButton(android.R.string.ok, null)
                    .create();
            dialog.show();
        } else {
            Log.d(TAG, "[showDialog] ignore. mHostActivity=" + mHostActivity +
                    ", mHostActivityResumed=" + mHostActivityResumed);
        }
    }

    private void dumpCurrentQueryStateCache() {
        final StringBuilder sb = new StringBuilder();
        for (String number : mCurrQueryStateCache.keySet()) {
            int state = mCurrQueryStateCache.get(number).ordinal();
            sb.append("[").append(number).append(",").append(state).append("]");
        }
        Log.d(TAG, "[dumpCurrentQueryStateCache]" + sb.toString());
    }
}

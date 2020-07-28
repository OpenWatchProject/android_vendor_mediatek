/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.settings.network;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.settings.SettingsEnums;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
/// M: Disable screen when MMS is in transaction. @{
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
/// @}
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.UserHandle;
import android.os.UserManager;
import android.provider.Telephony;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.widget.Toast;

import androidx.preference.Preference;
import androidx.preference.PreferenceGroup;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.UiccController;
import com.android.settings.R;
import com.android.settings.RestrictedSettingsFragment;
import com.android.settingslib.RestrictedLockUtils.EnforcedAdmin;

/// M: Add for checking MTK VoLTE feature.
import com.mediatek.settings.FeatureOption;
/// M: Add for APN setting plugin.
import com.mediatek.settings.UtilsExt;
/// M: Add for supporting CDMA APN.
import com.mediatek.settings.cdma.CdmaApnSetting;
/// M: Add for APN setting plugin.
import com.mediatek.settings.ext.IApnSettingsExt;
/// M: Add for supporting SIM hot swap. @{
import com.mediatek.settings.sim.SimHotSwapHandler;
import com.mediatek.settings.sim.SimHotSwapHandler.OnSimHotSwapListener;
/// @}
/// M: Add for SIM On/Off feature. @{
import com.mediatek.settings.sim.SimOnOffSwitchHandler;
import com.mediatek.settings.sim.SimOnOffSwitchHandler.OnSimOnOffSwitchListener;
/// @}
/// M: Add SIM utility.
import com.mediatek.settings.sim.TelephonyUtils;

/// M: Add for the new APN fields.
import mediatek.telephony.MtkTelephony;

import java.util.ArrayList;

public class ApnSettings extends RestrictedSettingsFragment
        implements Preference.OnPreferenceChangeListener {
    static final String TAG = "ApnSettings";

    public static final String EXTRA_POSITION = "position";
    public static final String RESTORE_CARRIERS_URI =
        "content://telephony/carriers/restore";
    public static final String PREFERRED_APN_URI =
        "content://telephony/carriers/preferapn";

    public static final String APN_ID = "apn_id";
    public static final String SUB_ID = "sub_id";
    public static final String MVNO_TYPE = "mvno_type";
    public static final String MVNO_MATCH_DATA = "mvno_match_data";

    private static final String[] CARRIERS_PROJECTION = new String[] {
            Telephony.Carriers._ID,
            Telephony.Carriers.NAME,
            Telephony.Carriers.APN,
            Telephony.Carriers.TYPE,
            Telephony.Carriers.MVNO_TYPE,
            Telephony.Carriers.MVNO_MATCH_DATA,
            Telephony.Carriers.EDITED_STATUS,
            /// M: Add for the new APN types.
            MtkTelephony.Carriers.SOURCE_TYPE
    };

    private static final int ID_INDEX = 0;
    private static final int NAME_INDEX = 1;
    private static final int APN_INDEX = 2;
    private static final int TYPES_INDEX = 3;
    private static final int MVNO_TYPE_INDEX = 4;
    private static final int MVNO_MATCH_DATA_INDEX = 5;
    private static final int EDITED_INDEX = 6;
    /// M: Add for the new APN types.
    private static final int SOURCE_TYPE_INDEX = 7;

    private static final int MENU_NEW = Menu.FIRST;
    private static final int MENU_RESTORE = Menu.FIRST + 1;

    private static final int EVENT_RESTORE_DEFAULTAPN_START = 1;
    private static final int EVENT_RESTORE_DEFAULTAPN_COMPLETE = 2;

    private static final int DIALOG_RESTORE_DEFAULTAPN = 1001;

    private static final Uri DEFAULTAPN_URI = Uri.parse(RESTORE_CARRIERS_URI);
    private static final Uri PREFERAPN_URI = Uri.parse(PREFERRED_APN_URI);

    private static boolean mRestoreDefaultApnMode;

    private UserManager mUserManager;
    private RestoreApnUiHandler mRestoreApnUiHandler;
    private RestoreApnProcessHandler mRestoreApnProcessHandler;
    private HandlerThread mRestoreDefaultApnThread;
    private SubscriptionInfo mSubscriptionInfo;
    private int mSubId;
    private int mSlotId;
    private UiccController mUiccController;
    private String mMvnoType;
    private String mMvnoMatchData;

    private String mSelectedKey;

    private IntentFilter mIntentFilter;

    private boolean mUnavailable;

    private boolean mHideImsApn;
    private boolean mAllowAddingApns;
    private boolean mHidePresetApnDetails;

    /// M: Add for supporting SIM hot swap.
    private SimHotSwapHandler mSimHotSwapHandler = null;
    /// M: Add for APN setting plugin.
    private IApnSettingsExt mApnExt;
    /// M: Add for SIM On/Off feature. @{
    private boolean mSimOnOffEnabled;
    private SimOnOffSwitchHandler mSimOnOffSwitchHandler = null;
    /// @}
    /// M: Add restore APN flag.
    private boolean mRestoreOngoing = false;

    public ApnSettings() {
        super(UserManager.DISALLOW_CONFIG_MOBILE_NETWORKS);
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(
                    TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED)) {
                PhoneConstants.DataState state = getMobileDataState(intent);
                switch (state) {
                case CONNECTED:
                    if (!mRestoreDefaultApnMode) {
                        fillList();
                    } else {
                        /// M: Add for ALPS02326359, don't show dialog here.
                        // showDialog(DIALOG_RESTORE_DEFAULTAPN);
                    }
                    break;
                }
                /// M: Disable screen when MMS is in transaction.
                updateScreenForDataStateChange(context, intent);
            } else if (intent.getAction().equals(
                    TelephonyManager.ACTION_SUBSCRIPTION_CARRIER_IDENTITY_CHANGED)) {
                if (!mRestoreDefaultApnMode) {
                    int extraSubId = intent.getIntExtra(TelephonyManager.EXTRA_SUBSCRIPTION_ID,
                            SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                    ///M: Ensure same soltid sub id changed @{
                    int extraSlotId = SubscriptionManager.getPhoneId(extraSubId);
                    if (mSlotId == extraSlotId && extraSubId != mSubId) {
                        // subscription has changed
                        mSubId = extraSubId;
                        mSubscriptionInfo = getSubscriptionInfo(mSubId);
                    }
                    fillList();
                }
            /// M: Update screen enabled status when airplane mode is changed. @{
            } else if (intent.getAction().equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                updateScreenEnableState(context);
            /// @}
            /// M: Update screen enabled status when call state is changed. @{
            } else if (intent.getAction().equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                updateScreenEnableState(context);
            /// @}
            }
        }
    };

    private static PhoneConstants.DataState getMobileDataState(Intent intent) {
        String str = intent.getStringExtra(PhoneConstants.STATE_KEY);
        if (str != null) {
            return Enum.valueOf(PhoneConstants.DataState.class, str);
        } else {
            return PhoneConstants.DataState.DISCONNECTED;
        }
    }

    @Override
    public int getMetricsCategory() {
        return SettingsEnums.APN;
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        final Activity activity = getActivity();
        mSubId = activity.getIntent().getIntExtra(SUB_ID,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);

        mIntentFilter = new IntentFilter(
                TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED);
        mIntentFilter.addAction(TelephonyManager.ACTION_SUBSCRIPTION_CARRIER_IDENTITY_CHANGED);
        /// M: Update screen enabled status when airplane mode is changed.
        mIntentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        /// M: Update screen enabled status when call state is changed.
        mIntentFilter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);

        setIfOnlyAvailableForAdmins(true);

        mSubscriptionInfo = getSubscriptionInfo(mSubId);
        mUiccController = UiccController.getInstance();

        CarrierConfigManager configManager = (CarrierConfigManager)
                getSystemService(Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle b = configManager.getConfigForSubId(mSubId);
        mHideImsApn = b.getBoolean(CarrierConfigManager.KEY_HIDE_IMS_APN_BOOL);
        mAllowAddingApns = b.getBoolean(CarrierConfigManager.KEY_ALLOW_ADDING_APNS_BOOL);
        if (mAllowAddingApns) {
            String[] readOnlyApnTypes = b.getStringArray(
                    CarrierConfigManager.KEY_READ_ONLY_APN_TYPES_STRING_ARRAY);
            // if no apn type can be edited, do not allow adding APNs
            if (ApnEditor.hasAllApns(readOnlyApnTypes)) {
                Log.d(TAG, "not allowing adding APN because all APN types are read only");
                mAllowAddingApns = false;
            }
        }
        mHidePresetApnDetails = b.getBoolean(CarrierConfigManager.KEY_HIDE_PRESET_APN_DETAILS_BOOL);
        mUserManager = UserManager.get(activity);

        /// M: Finish activity when no active subscription is found. @{
        if (mSubscriptionInfo == null) {
            Log.d(TAG, "onCreate, no active sub for subId=" + mSubId);
            getActivity().finish();
            return;
        }
        /// @}

        /// M: Add for APN setting plugin. @{
        mApnExt = UtilsExt.getApnSettingsExt(activity);
        mApnExt.initTetherField(this);
        /// @ }

        /// M: Add for supporting SIM hot swap. @{
        mSimHotSwapHandler = new SimHotSwapHandler(getActivity().getApplicationContext());
        mSimHotSwapHandler.registerOnSimHotSwap(new OnSimHotSwapListener() {
            @Override
            public void onSimHotSwap() {
                if (getActivity() != null) {
                    Log.d(TAG, "onSimHotSwap, finish activity.");
                    getActivity().finish();
                }
            }
        });
        /// @}

        mSlotId = SubscriptionManager.getPhoneId(mSubId);
        Log.d(TAG, "onCreate, subId=" + mSubId + ", mSlotId=" + mSlotId);
        /// M: Add for SIM On/Off feature. @{
        mSimOnOffEnabled = TelephonyUtils.isSimOnOffEnabled();
        if (mSimOnOffEnabled && SubscriptionManager.isValidSlotIndex(mSlotId)) {
            mSimOnOffSwitchHandler = new SimOnOffSwitchHandler(getActivity(), mSlotId);
            mSimOnOffSwitchHandler.registerOnSimOnOffSwitch(new OnSimOnOffSwitchListener() {
                @Override
                public void onSimOnOffStateChanged() {
                    if (getActivity() != null) {
                        Log.d(TAG, "onSimOnOffStateChanged, finish activity.");
                        getActivity().finish();
                    }
                }
            });
        }
        /// @}
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        getEmptyTextView().setText(R.string.apn_settings_not_available);
        mUnavailable = isUiRestricted();
        setHasOptionsMenu(!mUnavailable);
        if (mUnavailable) {
            addPreferencesFromResource(R.xml.placeholder_prefs);
            return;
        }

        addPreferencesFromResource(R.xml.apn_settings);
    }

    @Override
    public void onResume() {
        super.onResume();

        if (mUnavailable) {
            return;
        }

        getActivity().registerReceiver(mReceiver, mIntentFilter);

        if (!mRestoreDefaultApnMode) {
            fillList();
            /// M: In case dialog not dismiss as activity is in background, so when resume back,
            // need to remove the dialog @{
            removeDialog(DIALOG_RESTORE_DEFAULTAPN);
            /// @}
        }

        /// M: Add for APN setting plugin. @{
        mApnExt.updateTetherState();
        mApnExt.onApnSettingsEvent(IApnSettingsExt.RESUME);
        /// @}
    }

    @Override
    public void onPause() {
        super.onPause();

        if (mUnavailable) {
            return;
        }

        getActivity().unregisterReceiver(mReceiver);

        /// M: Add for APN setting plugin.
        mApnExt.onApnSettingsEvent(IApnSettingsExt.PAUSE);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if (mRestoreDefaultApnThread != null) {
            mRestoreDefaultApnThread.quit();
        }

        /// M: Add for supporting SIM hot swap. @{
        if (mSimHotSwapHandler != null) {
            mSimHotSwapHandler.unregisterOnSimHotSwap();
            mSimHotSwapHandler = null;
        }
        /// @}

        /// M: Add for SIM On/Off feature. @{
        if (mSimOnOffSwitchHandler != null) {
            mSimOnOffSwitchHandler.unregisterOnSimOnOffSwitch();
            mSimOnOffSwitchHandler = null;
        }
        /// @}
    }

    @Override
    public EnforcedAdmin getRestrictionEnforcedAdmin() {
        final UserHandle user = UserHandle.of(mUserManager.getUserHandle());
        if (mUserManager.hasUserRestriction(UserManager.DISALLOW_CONFIG_MOBILE_NETWORKS, user)
                && !mUserManager.hasBaseUserRestriction(UserManager.DISALLOW_CONFIG_MOBILE_NETWORKS,
                        user)) {
            return EnforcedAdmin.MULTIPLE_ENFORCED_ADMIN;
        }
        return null;
    }

    private SubscriptionInfo getSubscriptionInfo(int subId) {
        return SubscriptionManager.from(getActivity()).getActiveSubscriptionInfo(subId);
    }

    public void fillList() {
        final int subId = mSubscriptionInfo != null ? mSubscriptionInfo.getSubscriptionId()
                : SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        final Uri simApnUri = Uri.withAppendedPath(Telephony.Carriers.SIM_APN_URI,
                String.valueOf(subId));
        StringBuilder where = new StringBuilder("NOT (type='ia' AND (apn=\"\" OR apn IS NULL)) AND "
                + "user_visible!=0");

        /// M: Add for supporting MTK VoLTE.
        if (mHideImsApn || !FeatureOption.MTK_VOLTE_SUPPORT) {
            where.append(" AND NOT (type='ims')");
        }

        /// M: Add for APN Setting plugin.
        String order = mApnExt.getApnSortOrder(Telephony.Carriers.DEFAULT_SORT_ORDER);
        Log.d(TAG, "fillList, where=" + where + ", order=" + order);

        Cursor cursor = getContentResolver().query(simApnUri,
                CARRIERS_PROJECTION, where.toString(), null,
                order);

        if (cursor != null) {
            PreferenceGroup apnPrefList = (PreferenceGroup) findPreference("apn_list");
            apnPrefList.removeAll();

            ArrayList<ApnPreference> apnList = new ArrayList<ApnPreference>();
            ArrayList<ApnPreference> mmsApnList = new ArrayList<ApnPreference>();

            mSelectedKey = getSelectedApnKey();
            cursor.moveToFirst();
            while (!cursor.isAfterLast()) {
                String name = cursor.getString(NAME_INDEX);
                String apn = cursor.getString(APN_INDEX);
                String key = cursor.getString(ID_INDEX);
                String type = cursor.getString(TYPES_INDEX);
                int edited = cursor.getInt(EDITED_INDEX);
                mMvnoType = cursor.getString(MVNO_TYPE_INDEX);
                mMvnoMatchData = cursor.getString(MVNO_MATCH_DATA_INDEX);
                /// M: check source type, some types are not editable
                int sourcetype = cursor.getInt(SOURCE_TYPE_INDEX);

                /// M: Add for skipping specific APN type. @{
                if (shouldSkipApn(type)) {
                    cursor.moveToNext();
                    continue;
                }

                /// M: Add for APN Setting plugin.
                name = mApnExt.updateApnName(name, sourcetype);

                ApnPreference pref = new ApnPreference(getPrefContext());

                pref.setKey(key);
                pref.setTitle(name);
                pref.setPersistent(false);
                pref.setOnPreferenceChangeListener(this);
                pref.setSubId(subId);
                /// M: for [Read Only APN]
                pref.setApnEditable(mApnExt.isAllowEditPresetApn(subId, type, apn, sourcetype));

                if (mHidePresetApnDetails && edited == Telephony.Carriers.UNEDITED) {
                    pref.setHideDetails();
                } else {
                    pref.setSummary(apn);
                }

                boolean selectable = ((type == null) || (!type.equals("mms")
                        /// M: Add for ALPS02500557, do not select emergency APN.
                        && !type.equals("ia") && !type.equals("ims") && !type.equals("emergency")))
                        /// M: Add for APN Setting plugin.
                        && mApnExt.isSelectable(type);
                pref.setSelectable(selectable);
                Log.d(TAG, "fillList, selectedKey=" + mSelectedKey + ", key=" + key
                        + ", name=" + name + ", selectable=" + selectable);
                if (selectable) {
                    /// M: select prefer APN later, as the apn list are not solid now @{
                    //if ((mSelectedKey != null) && mSelectedKey.equals(key)) {
                    //    pref.setChecked();
                    //}
                    /// @}
                    apnList.add(pref);
                    /// for china telecom apn feature
                    mApnExt.customizeApnState(subId, apn, apnList);
                } else {
                    mmsApnList.add(pref);
                    /// for china telecom apn feature
                    mApnExt.customizeApnState(subId, apn, mmsApnList);
                }
                cursor.moveToNext();
            }
            cursor.close();

            for (Preference preference : apnList) {
                apnPrefList.addPreference(preference);
            }
            for (Preference preference : mmsApnList) {
                apnPrefList.addPreference(preference);
            }

            /// M: Add for setting set prefer APN.
            setPreferApnChecked(apnList);

            /// M: Add for updating screen enable state.
            updateScreenEnableState(getActivity());
        }
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        if (!mUnavailable) {
            /// M: Don't allow to add new APN when restoring APN.
            if (mAllowAddingApns && !mRestoreOngoing) {
                menu.add(0, MENU_NEW, 0,
                        getResources().getString(R.string.menu_new))
                        .setIcon(R.drawable.ic_add_24dp)
                        .setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
            }
            menu.add(0, MENU_RESTORE, 0,
                    getResources().getString(R.string.menu_restore))
                    .setIcon(android.R.drawable.ic_menu_upload);
        }

        super.onCreateOptionsMenu(menu, inflater);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case MENU_NEW:
            addNewApn();
            return true;

        case MENU_RESTORE:
            restoreDefaultApn();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void addNewApn() {
        /// M: Don't allow to add new APN when restoring APN. @{
        if (mRestoreOngoing) {
            return;
        }
        /// @}

        Log.d(TAG, "addNewApn");

        Intent intent = new Intent(Intent.ACTION_INSERT, Telephony.Carriers.CONTENT_URI);
        int subId = mSubscriptionInfo != null ? mSubscriptionInfo.getSubscriptionId()
                : SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        intent.putExtra(SUB_ID, subId);
        if (!TextUtils.isEmpty(mMvnoType) && !TextUtils.isEmpty(mMvnoMatchData)) {
            intent.putExtra(MVNO_TYPE, mMvnoType);
            intent.putExtra(MVNO_MATCH_DATA, mMvnoMatchData);
        }
        /// M: Add for APN Setting plugin.
        mApnExt.addApnTypeExtra(intent);
        startActivity(intent);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "onPreferenceChange(): Preference - " + preference
                + ", newValue - " + newValue + ", newValue type - "
                + newValue.getClass());
        if (newValue instanceof String) {
            setSelectedApnKey((String) newValue);
        }

        return true;
    }

    private void setSelectedApnKey(String key) {
        mSelectedKey = key;
        ContentResolver resolver = getContentResolver();

        ContentValues values = new ContentValues();
        values.put(APN_ID, mSelectedKey);
        resolver.update(getUriForCurrSubId(PREFERAPN_URI), values, null, null);
    }

    private String getSelectedApnKey() {
        String key = null;

        Cursor cursor = getContentResolver().query(getUriForCurrSubId(PREFERAPN_URI),
                new String[] {"_id"}, null, null, Telephony.Carriers.DEFAULT_SORT_ORDER);
        if (cursor.getCount() > 0) {
            cursor.moveToFirst();
            key = cursor.getString(ID_INDEX);
        }
        cursor.close();
        Log.d(TAG,"getSelectedApnKey, key=" + key);
        return key;
    }

    private boolean restoreDefaultApn() {
        Log.d(TAG, "restoreDefaultApn, restoreApn=" + mRestoreDefaultApnMode);

        /// M: Remove dialog first before restoring default APN again. @{
        if (mRestoreDefaultApnMode) {
            removeDialog(DIALOG_RESTORE_DEFAULTAPN);
        }
        /// @}

        showDialog(DIALOG_RESTORE_DEFAULTAPN);
        mRestoreDefaultApnMode = true;

        if (mRestoreApnUiHandler == null) {
            mRestoreApnUiHandler = new RestoreApnUiHandler();
        }

        if (mRestoreApnProcessHandler == null ||
            mRestoreDefaultApnThread == null) {
            mRestoreDefaultApnThread = new HandlerThread(
                    "Restore default APN Handler: Process Thread");
            mRestoreDefaultApnThread.start();
            mRestoreApnProcessHandler = new RestoreApnProcessHandler(
                    mRestoreDefaultApnThread.getLooper(), mRestoreApnUiHandler);
        }

        mRestoreApnProcessHandler
                .sendEmptyMessage(EVENT_RESTORE_DEFAULTAPN_START);
        return true;
    }

    // Append subId to the Uri
    private Uri getUriForCurrSubId(Uri uri) {
        int subId = mSubscriptionInfo != null ? mSubscriptionInfo.getSubscriptionId()
                : SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        if (SubscriptionManager.isValidSubscriptionId(subId)) {
            return Uri.withAppendedPath(uri, "subId/" + String.valueOf(subId));
        } else {
            return uri;
        }
    }

    private class RestoreApnUiHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_RESTORE_DEFAULTAPN_COMPLETE:
                    Log.d(TAG, "handleMessage, RESTORE_DEFAULTAPN_COMPLETE.");
                    Activity activity = getActivity();
                    if (activity == null) {
                        mRestoreDefaultApnMode = false;
                        return;
                    }
                    fillList();

                    /// M: Disable screen in airplane mode.
                    boolean airplaneModeOn = TelephonyUtils.isAirplaneModeOn(activity);
                    getPreferenceScreen().setEnabled(!airplaneModeOn);
                    mRestoreDefaultApnMode = false;
                    removeDialog(DIALOG_RESTORE_DEFAULTAPN);
                    Toast.makeText(
                        activity,
                        getResources().getString(
                                R.string.restore_default_apn_completed),
                        Toast.LENGTH_LONG).show();

                    /// M: Add restore APN flag.
                    mRestoreOngoing = false;
                    break;
            }
        }
    }

    private class RestoreApnProcessHandler extends Handler {
        private Handler mRestoreApnUiHandler;

        public RestoreApnProcessHandler(Looper looper, Handler restoreApnUiHandler) {
            super(looper);
            this.mRestoreApnUiHandler = restoreApnUiHandler;
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_RESTORE_DEFAULTAPN_START:
                    Log.d(TAG, "handleMessage, RESTORE_DEFAULTAPN_START");
                    ContentResolver resolver = getContentResolver();
                    resolver.delete(getUriForCurrSubId(DEFAULTAPN_URI), null, null);
                    mRestoreApnUiHandler
                        .sendEmptyMessage(EVENT_RESTORE_DEFAULTAPN_COMPLETE);

                    /// M: Add restore APN flag.
                    mRestoreOngoing = true;
                    break;
            }
        }
    }

    @Override
    public Dialog onCreateDialog(int id) {
        if (id == DIALOG_RESTORE_DEFAULTAPN) {
            ProgressDialog dialog = new ProgressDialog(getActivity()) {
                public boolean onTouchEvent(MotionEvent event) {
                    return true;
                }
            };
            dialog.setMessage(getResources().getString(R.string.restore_default_apn));
            dialog.setCancelable(false);
            return dialog;
        }
        return null;
    }

    @Override
    public int getDialogMetricsCategory(int dialogId) {
        if (dialogId == DIALOG_RESTORE_DEFAULTAPN) {
            return SettingsEnums.DIALOG_APN_RESTORE_DEFAULT;
        }
        return 0;
    }

    private void updateScreenForDataStateChange(Context context, Intent intent) {
        String apnType  = intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY);
        Log.d(TAG, "updateScreenForDataStateChange, apnType=" + apnType);
        if (PhoneConstants.APN_TYPE_MMS.equals(apnType)) {
            boolean airplaneModeOn = TelephonyUtils.isAirplaneModeOn(context);
            getPreferenceScreen().setEnabled(!airplaneModeOn
                    && !isMmsInTransaction(context)
                    /// M: Add for APN Setting plugin. @{
                    && mApnExt.getScreenEnableState(
                            (mSubscriptionInfo != null
                                    ? mSubscriptionInfo.getSubscriptionId()
                                    : SubscriptionManager.INVALID_SUBSCRIPTION_ID),
                            getActivity()));
                    /// @}
        }
    }

    private void updateScreenEnableState(Context context) {
        int subId = (mSubscriptionInfo != null ? mSubscriptionInfo.getSubscriptionId()
                : SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        boolean simReady = false;
        if (SubscriptionManager.isValidSubscriptionId(subId)) {
            simReady = (TelephonyManager.SIM_STATE_READY == TelephonyManager.getDefault()
                    .getSimState(SubscriptionManager.getSlotIndex(subId)));
        }
        boolean airplaneModeOn = TelephonyUtils.isAirplaneModeOn(context);
        /// M: Add for checking call state.
        boolean inCall = TelephonyUtils.isInCall();

        boolean enable = !airplaneModeOn && simReady && !inCall;
        Log.d(TAG, "updateScreenEnableState, subId=" + subId + ", enable=" + enable
                + ", airplaneModeOn=" + airplaneModeOn
                + ", simReady=" + simReady + ", inCall=" + inCall);
        getPreferenceScreen().setEnabled(enable
                /// M: Add for APN Setting plugin.
                && mApnExt.getScreenEnableState(subId, getActivity()));
        if (getActivity() != null) {
            getActivity().invalidateOptionsMenu();
        }
    }

    private boolean isMmsInTransaction(Context context) {
        boolean isMmsInTransaction = false;
        ConnectivityManager cm = (ConnectivityManager) context.getSystemService(
                Context.CONNECTIVITY_SERVICE);
        if (cm != null) {
            NetworkInfo networkInfo = cm.getNetworkInfo(ConnectivityManager.TYPE_MOBILE_MMS);
            if (networkInfo != null) {
                NetworkInfo.State state = networkInfo.getState();
                Log.d(TAG, "isMmsInTransaction, mmsState=" + state);
                isMmsInTransaction = (state == NetworkInfo.State.CONNECTING
                    || state == NetworkInfo.State.CONNECTED);
            }
        }
        return isMmsInTransaction;
    }

    public boolean shouldSkipApn(String type) {
        /// M: Add for APN Setting plugin.
        return "cmmail".equals(type);
    }

    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        int size = menu.size();
        boolean isAirplaneModeOn = TelephonyUtils.isAirplaneModeOn(getActivity());
        boolean inCall = TelephonyUtils.isInCall();
        Log.d(TAG,"onPrepareOptionsMenu, isAirplaneModeOn=" + isAirplaneModeOn);
        // When airplane mode on need to disable options menu
        for (int i = 0; i< size; i++) {
            menu.getItem(i).setEnabled(!isAirplaneModeOn && !inCall);
        }
        super.onPrepareOptionsMenu(menu);
    }

    // compare prefer apn and set preference checked state
    private void setPreferApnChecked(ArrayList<ApnPreference> apnList) {
        if (apnList == null || apnList.isEmpty()) {
            return;
        }

        String selectedKey = null;
        if (mSelectedKey != null) {
            for (Preference pref : apnList) {
                if (mSelectedKey.equals(pref.getKey())) {
                    ((ApnPreference) pref).setChecked();
                    selectedKey = mSelectedKey;
                }
            }
        }

        // can't find prefer APN in the list, reset to the first one
        ///M: Plug-in call to check whether reset to first one or not.
        if (mApnExt.shouldSelectFirstApn()) {
            if (selectedKey == null && apnList.get(0) != null) {
               ((ApnPreference) apnList.get(0)).setChecked();
               selectedKey = apnList.get(0).getKey();
            }
        }
        // save the new APN
        if (selectedKey != null && selectedKey != mSelectedKey) {
            setSelectedApnKey(selectedKey);
            mSelectedKey = selectedKey;
        }

        Log.d(TAG, "setPreferApnChecked, selectedKey=" + mSelectedKey);
    }

    public void onIntentUpdate(Intent newIntent) {
        final Activity activity = getActivity();

        if (activity == null) {
            return;
        }

        final int subId = newIntent.getIntExtra(SUB_ID,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        mSubscriptionInfo = SubscriptionManager.from(activity).getActiveSubscriptionInfo(subId);
        Log.d(TAG, "onIntentUpdate, subId=" + subId);
    }
}

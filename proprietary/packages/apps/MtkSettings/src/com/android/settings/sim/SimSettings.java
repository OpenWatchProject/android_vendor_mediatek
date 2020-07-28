/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.android.settings.sim;

import android.app.settings.SettingsEnums;
/// M: Receive broadcast to update SIM status.
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
/// M: Receive broadcast to update SIM status.
import android.content.IntentFilter;
import android.content.res.Resources;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.SystemProperties;
/// M: Add for changing the restriction for SIM setting.
import android.os.UserManager;
import android.provider.SearchIndexableResource;
/// M: Add for PhoneAccount checking.
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
/// M: Remove preference for tablet.
import androidx.preference.PreferenceCategory;

/// M: Add for SIM Lock feature.
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;
import com.android.settings.R;
/// M: Add for SIM On/Off feature.
import com.android.internal.telephony.PhoneConstants;
/// M: Receive broadcast to update SIM status.
import com.android.internal.telephony.TelephonyIntents;
import com.android.settings.RestrictedSettingsFragment;
import com.android.settings.Utils;
import com.android.settings.search.BaseSearchIndexProvider;
import com.android.settings.search.Indexable;
import com.android.settingslib.search.SearchIndexable;

/// M: Add for SIM Lock feature.
import com.mediatek.internal.telephony.MtkIccCardConstants;
/// M: Add for supporting RSIM.
import com.mediatek.internal.telephony.MtkTelephonyProperties;
/// M: Remove preference for tablet.
import com.mediatek.settings.FeatureOption;
/// M: Add for SIM settings plugin. @{
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.ISimManagementExt;
/// @}
/// M: Add for supporting radio power switching. @{
import com.mediatek.settings.sim.RadioPowerController;
import com.mediatek.settings.sim.RadioPowerPreference;
/// @}
/// M: Add for supporting SIM hot swap. @{
import com.mediatek.settings.sim.SimHotSwapHandler;
import com.mediatek.settings.sim.SimHotSwapHandler.OnSimHotSwapListener;
/// @}
/// M: Add for SIM On/Off feature.
import com.mediatek.settings.sim.SimPowerPreference;
/// M: Add for supporting radio power switching.
import com.mediatek.settings.sim.TelephonyUtils;

import java.util.ArrayList;
import java.util.List;

@SearchIndexable
public class SimSettings extends RestrictedSettingsFragment implements Indexable {
    private static final String TAG = "SimSettings";
    private static final boolean DBG = true;

    private static final boolean ENG_LOAD =
            SystemProperties.get("ro.build.type").equals("eng")
            || Log.isLoggable(TAG, Log.DEBUG);

    /// M: Add for removing preference for tablet.
    private static final String KEY_SIM_ACTIVITIES = "sim_activities";

    private static final String DISALLOW_CONFIG_SIM = "no_config_sim";
    private static final String SIM_CARD_CATEGORY = "sim_cards";
    private static final String KEY_CELLULAR_DATA = "sim_cellular_data";
    private static final String KEY_CALLS = "sim_calls";
    private static final String KEY_SMS = "sim_sms";
    public static final String EXTRA_SLOT_ID = "slot_id";

    /**
     * By UX design we use only one Subscription Information(SubInfo) record per SIM slot.
     * mAvalableSubInfos is the list of SubInfos we present to the user.
     * mSubInfoList is the list of all SubInfos.
     * mSelectableSubInfos is the list of SubInfos that a user can select for data, calls, and SMS.
     */
    private List<SubscriptionInfo> mAvailableSubInfos = null;
    private List<SubscriptionInfo> mSubInfoList = null;
    private List<SubscriptionInfo> mSelectableSubInfos = null;
    private PreferenceScreen mSimCards = null;
    private SubscriptionManager mSubscriptionManager;
    private int mNumSlots;
    private Context mContext;

    private int mPhoneCount = TelephonyManager.getDefault().getPhoneCount();
    private PhoneStateListener[] mPhoneStateListener = new PhoneStateListener[mPhoneCount];

    /// M: Add for SIM settings plugin.
    private ISimManagementExt mSimManagementExt;
    /// M: Add for supporting SIM hot swap.
    private SimHotSwapHandler mSimHotSwapHandler;
    /// M: Disable the radio switch button in airplane mode.
    private boolean mIsAirplaneModeOn = false;
    /// M: Add for supporting radio power switching.
    private RadioPowerController mRadioController;

    /// M: Record the SIM Lock information. @{
    private int mSimLockPolicy = TelephonyUtils.SIM_LOCK_POLICY_NONE;
    private int[] mSimLockSimValid = null;
    private boolean[] mSimLockSimInserted = null;
    /// @}

    /// M: Add for SIM On/Off feature.
    private boolean mSimOnOffEnabled = false;

    /// M: Receive broadcast to update SIM status. @{
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive, action=" + action);
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                handleAirplaneModeChange(intent);
            } else if (action.equals(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED)) {
                updateCellularDataValues();
            } else if (action.equals(TelephonyIntents.ACTION_DEFAULT_VOICE_SUBSCRIPTION_CHANGED)) {
                updateCallValues();
            } else if (action.equals(TelephonyIntents.ACTION_DEFAULT_SMS_SUBSCRIPTION_CHANGED)) {
                updateSmsValues();
            } else if (action.equals(TelecomManager.ACTION_PHONE_ACCOUNT_REGISTERED)
                    || action.equals(TelecomManager.ACTION_PHONE_ACCOUNT_UNREGISTERED)) {
                updateCallValues();
            } else if (action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE)
                    || action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED)) {
                updateActivitesCategory();
            } else if (action.equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                if (null != mSimManagementExt
                        && mSimManagementExt.customizeSimCardForPhoneState()) {
                    Log.i(TAG, "customizeSimCardForPhoneState update updateSimSlotValues");
                    updateSimSlotValues();
                }
                updateActivitesCategory();
            /// M: Handle the SIM lock state change event. @{
            } else if (action.equals(TelephonyIntents.ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION)) {
                handleSimLockStateChange(intent);
            /// @}
            /// M: Add for SIM On/Off feature. @{
            } else if (action.equals(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)) {
                handleSimApplicationStateChange(intent);
            /// @}
            }
        }
    };
    /// @}

    public SimSettings() {
        /// M: Add for changing the restriction for SIM setting.
        super(UserManager.DISALLOW_CONFIG_MOBILE_NETWORKS);
    }

    @Override
    public int getMetricsCategory() {
        return SettingsEnums.SIM;
    }

    @Override
    public void onCreate(final Bundle bundle) {
        super.onCreate(bundle);
        mContext = getActivity();

        mSubscriptionManager = SubscriptionManager.from(getActivity());
        final TelephonyManager tm =
                (TelephonyManager) getActivity().getSystemService(Context.TELEPHONY_SERVICE);
        addPreferencesFromResource(R.xml.sim_settings);

        mNumSlots = tm.getSimCount();
        mSimCards = (PreferenceScreen)findPreference(SIM_CARD_CATEGORY);
        mAvailableSubInfos = new ArrayList<SubscriptionInfo>(mNumSlots);
        mSelectableSubInfos = new ArrayList<SubscriptionInfo>();
        SimSelectNotification.cancelNotification(getActivity());

        /// M: Add for SIM On/Off feature. @{
        mSimOnOffEnabled = TelephonyUtils.isSimOnOffEnabled();
        log("onCreate, simOnOffEnabled=" + mSimOnOffEnabled);

        if (!mSimOnOffEnabled) {
            /// M: Add for supporting radio power switching.
            mRadioController = RadioPowerController.getInstance(getContext());
        }
        /// @}

        /// M: Add for SIM state changed checking.
        initForSimStateChange();

        /// M: Add for SIM settings plugin. @{
        mSimManagementExt = UtilsExt.getSimManagementExt(getActivity());
        mSimManagementExt.onCreate();
        mSimManagementExt.initPlugin(this);

        /// M: Primary SIM
        logInEng("PrimarySim add option");
        mSimManagementExt.initPrimarySim(this);
        /// @}
    }

    private final SubscriptionManager.OnSubscriptionsChangedListener mOnSubscriptionsChangeListener
            = new SubscriptionManager.OnSubscriptionsChangedListener() {
        @Override
        public void onSubscriptionsChanged() {
            if (DBG) log("onSubscriptionsChanged:");
            updateSubscriptions();
        }
    };

    private void updateSubscriptions() {
        mSubInfoList = mSubscriptionManager.getActiveSubscriptionInfoList(true);
        for (int i = 0; i < mNumSlots; ++i) {
            Preference pref = mSimCards.findPreference("sim" + i);
            if (pref instanceof SimPreference) {
                mSimCards.removePreference(pref);
            }
            /// M: Add for SIM On/Off feature. @{
            if (pref instanceof SimPowerPreference) {
                mSimCards.removePreference(pref);
            }
            /// @}
        }
        mAvailableSubInfos.clear();
        mSelectableSubInfos.clear();

        for (int i = 0; i < mNumSlots; ++i) {
            final SubscriptionInfo sir = mSubscriptionManager
                    .getActiveSubscriptionInfoForSimSlotIndex(i);
            /// M: Add for SIM On/Off feature. @{
            SimPreference simPreference = null;
            SimPowerPreference simOnOffPreference = null;
            if (mSimOnOffEnabled) {
                simOnOffPreference = new SimPowerPreference(
                        getPrefContext(), sir, i, mIsAirplaneModeOn);
                simOnOffPreference.setOrder(i - mNumSlots);
            } else {
                simPreference = new SimPreference(getPrefContext(), sir, i);
                simPreference.setOrder(i - mNumSlots);
            }
            /// @}

            /// M: Add for supporting radio power switching. @{
            if (sir != null) {
                /// M: Record the SIM Lock information. @{
                mSimLockSimInserted[i] = true;
                /// @}
                /// M: Add for SIM On/Off feature. @{
                if (mSimOnOffEnabled) {
                    simOnOffPreference.bindSimOnOffState(mIsAirplaneModeOn);
                } else {
                    int subId = sir.getSubscriptionId();
                    boolean radioOn = TelephonyUtils.isRadioOn(subId, mContext);
                    boolean normal = !mIsAirplaneModeOn
                            && mRadioController.isRadioSwitchComplete(subId, radioOn);
                    simPreference.bindRadioPowerState(
                            subId, normal, radioOn, mIsAirplaneModeOn);
                }
                /// @}
            } else {
                /// M: Record the SIM Lock information. @{
                mSimLockSimInserted[i] = false;
                /// @}
                /// M: Add for SIM On/Off feature. @{
                if (mSimOnOffEnabled) {
                    simOnOffPreference.bindSimOnOffState(mIsAirplaneModeOn);
                } else {
                    simPreference.bindRadioPowerState(
                            SubscriptionManager.INVALID_SUBSCRIPTION_ID,
                            false, false, mIsAirplaneModeOn);
                }
                /// @}
            }
            logInEng("addPreference slot=" + i
                    + ", subInfo=" + (sir == null ? "null" : sir));
            /// @}

            /// M: Add for SIM On/Off feature. @{
            if (mSimOnOffEnabled) {
                mSimCards.addPreference(simOnOffPreference);
            } else {
                mSimCards.addPreference(simPreference);
            }
            /// @}
            mAvailableSubInfos.add(sir);
            if (sir != null) {
                mSelectableSubInfos.add(sir);
            }
        }
        updateActivitesCategory();
    }

    private void updateAllOptions() {
        updateSimSlotValues();
        updateActivitesCategory();
    }

    private void updateSimSlotValues() {
        final int prefSize = mSimCards.getPreferenceCount();
        for (int i = 0; i < prefSize; ++i) {
            Preference pref = mSimCards.getPreference(i);
            if (pref instanceof SimPreference) {
                ((SimPreference) pref).update();
            }
            /// M: Add for SIM On/Off feature. @{
            if (pref instanceof SimPowerPreference) {
                ((SimPowerPreference) pref).update(mIsAirplaneModeOn);
            }
            /// @}
        }
    }

    private void updateActivitesCategory() {
        updateCellularDataValues();
        updateCallValues();
        updateSmsValues();

        /// M: For primary SIM
        mSimManagementExt.subChangeUpdatePrimarySIM();
    }

    private void updateSmsValues() {
        final Preference simPref = findPreference(KEY_SMS);

        /// M: Add checking for tablet. @{
        if (simPref == null) {
            return;
        }
        /// @}

        final SubscriptionInfo sir = mSubscriptionManager.getDefaultSmsSubscriptionInfo();
        simPref.setTitle(R.string.sms_messages_title);
        if (DBG) log("[updateSmsValues] mSubInfoList=" + mSubInfoList);

        if (sir != null) {
            simPref.setSummary(sir.getDisplayName());
        } else if (sir == null) {
            /// M: Add for supporting SMS always ask.
            simPref.setSummary(R.string.sim_calls_ask_first_prefs_title);

            /// M: Add for SIM settings plugin.
            mSimManagementExt.updateDefaultSmsSummary(simPref);
        }

        /// M: Add more conditions to set enabled state. @{
        boolean enabled = sir == null ? mSelectableSubInfos.size() >= 1
                : mSelectableSubInfos.size() > 1;
        // Check whether the SMS SIM can be enabled for SIM Lock.
        boolean enabledForSimLock = shouldEnableSmsPrefForSimLock();
        simPref.setEnabled(enabled && enabledForSimLock);
        /// @}

        /// M: Add for SIM settings plugin. @{
        mSimManagementExt.configSimPreferenceScreen(simPref, KEY_SMS,
                mSelectableSubInfos.size());
        mSimManagementExt.setPrefSummary(simPref, KEY_SMS);
        /// @}
    }

    private void updateCellularDataValues() {
        final Preference simPref = findPreference(KEY_CELLULAR_DATA);

        /// M: Add checking for tablet. @{
        if (simPref == null) {
            return;
        }
        /// @}

        SubscriptionInfo sir = mSubscriptionManager.getDefaultDataSubscriptionInfo();
        simPref.setTitle(R.string.cellular_data_title);
        if (DBG) log("[updateCellularDataValues] mSubInfoList=" + mSubInfoList);

        /// M: Add for SIM settings plugin. @{
        log("default subInfo=" + sir);
        sir = mSimManagementExt.setDefaultSubId(getActivity(), sir, KEY_CELLULAR_DATA);
        log("updated subInfo=" + sir);
        /// @}

        /// M: Add more conditions to set enabled state. @{
        boolean defaultState = (mSelectableSubInfos.size() > 1);
        if (sir != null) {
            simPref.setSummary(sir.getDisplayName());
        } else if (sir == null) {
            simPref.setSummary(R.string.sim_selection_required_pref);
            defaultState = (mSelectableSubInfos.size() >= 1);
        }
        simPref.setEnabled(shouldEnableSimPref(defaultState));
        /// @}

        /// M: Add for SIM settings plugin.
        mSimManagementExt.configSimPreferenceScreen(simPref, KEY_CELLULAR_DATA, -1);
    }

    private void updateCallValues() {
        final Preference simPref = findPreference(KEY_CALLS);

        /// M: Add checking for tablet. @{
        if (simPref == null) {
            return;
        }
        /// @}

        final TelecomManager telecomManager = TelecomManager.from(mContext);
        PhoneAccountHandle phoneAccount =
            telecomManager.getUserSelectedOutgoingPhoneAccount();
        final List<PhoneAccountHandle> allPhoneAccounts =
            telecomManager.getCallCapablePhoneAccounts();

        simPref.setTitle(R.string.calls_title);

        /// M: Add for SIM settings plugin.
        phoneAccount = mSimManagementExt.setDefaultCallValue(phoneAccount);

        log("updateCallValues, PhoneAccountSize=" + allPhoneAccounts.size()
                + ", phoneAccount=" + phoneAccount);

        /// M: For ALPS02320747, PhoneAccount may be unregistered. @{
        PhoneAccount defaultAccount = phoneAccount == null ? null
                : telecomManager.getPhoneAccount(phoneAccount);

        simPref.setSummary(defaultAccount == null
                ? mContext.getResources().getString(R.string.sim_calls_ask_first_prefs_title)
                : (String) defaultAccount.getLabel());
        /// @}

        /// M: Check whether the call SIM can be enabled for SIM Lock. @{
        boolean enabledForSimLock = shouldEnableCallPrefForSimLock();
        simPref.setEnabled(allPhoneAccounts.size() > 1 && enabledForSimLock);
        /// @}

        /// M: Add for SIM settings plugin.
        mSimManagementExt.configSimPreferenceScreen(simPref, KEY_CALLS,
                allPhoneAccounts.size());
        mSimManagementExt.setPrefSummary(simPref, KEY_CALLS);
    }

    @Override
    public void onResume() {
        super.onResume();
        mSubscriptionManager.addOnSubscriptionsChangedListener(mOnSubscriptionsChangeListener);
        updateSubscriptions();
        final TelephonyManager tm =
                (TelephonyManager) getActivity().getSystemService(Context.TELEPHONY_SERVICE);
        /// M: Listen the phone state change for selectable sub only. @{
        Log.d(TAG, "Register for phone state change");
        for (int i = 0; i < mSelectableSubInfos.size(); i++) {
            SubscriptionInfo subInfo = mSelectableSubInfos.get(i);
            int subId = subInfo.getSubscriptionId();
            int slotId = subInfo.getSimSlotIndex();
            tm.createForSubscriptionId(subId).listen(getPhoneStateListener(slotId, subId),
                    /// M: Add for handling radio state change.
                    PhoneStateListener.LISTEN_RADIO_POWER_STATE_CHANGED);
        }
        /// @}

        /// M: Remove preference for tablet.
        removeItemsForTablet();

        /// M: Add for SIM settings plugin.
        mSimManagementExt.onResume(getActivity());
    }

    @Override
    public void onPause() {
        super.onPause();
        mSubscriptionManager.removeOnSubscriptionsChangedListener(mOnSubscriptionsChangeListener);
        final TelephonyManager tm = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        for (int i = 0; i < mPhoneCount; i++) {
            if (mPhoneStateListener[i] != null) {
                tm.listen(mPhoneStateListener[i], PhoneStateListener.LISTEN_NONE);
                mPhoneStateListener[i] = null;
            }
        }

        /// M: Add for SIM settings plugin.
        mSimManagementExt.onPause();
    }

    private PhoneStateListener getPhoneStateListener(int phoneId, int subId) {
        // Disable Sim selection for Data when voice call is going on as changing the default data
        // sim causes a modem reset currently and call gets disconnected
        // ToDo : Add subtext on disabled preference to let user know that default data sim cannot
        // be changed while call is going on
        final int i = phoneId;
        final int subIdListened = subId;
        mPhoneStateListener[phoneId]  = new PhoneStateListener() {
            /// M: Add for handling radio state change. @{
            @Override
            public void onRadioPowerStateChanged(int state) {
                log("PhoneStateListener.onRadioPowerStateChanged, state=" + state
                        + ", phoneId=" + i);
                /// M: Add for SIM On/Off feature. @{
                if (mSimOnOffEnabled) {
                    handleRadioPowerSwitchComplete();
                } else {
                    if (mRadioController.isRadioSwitchComplete(subIdListened)) {
                        handleRadioPowerSwitchComplete();
                    }
                }
                /// @}
            }
            /// @}
        };
        return mPhoneStateListener[phoneId];
    }

    @Override
    public boolean onPreferenceTreeClick(final Preference preference) {
        final Context context = mContext;
        Intent intent = new Intent(context, SimDialogActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        if (preference instanceof SimPreference) {
            Intent newIntent = new Intent(context, SimPreferenceDialog.class);
            newIntent.putExtra(EXTRA_SLOT_ID, ((SimPreference)preference).getSlotId());
            startActivity(newIntent);
        /// M: Add for SIM On/Off feature. @{
        } else if (preference instanceof SimPowerPreference) {
            Intent newIntent = new Intent(context, SimPreferenceDialog.class);
            newIntent.putExtra(EXTRA_SLOT_ID, ((SimPowerPreference) preference).getSlotId());
            startActivity(newIntent);
        /// @}
        } else if (findPreference(KEY_CELLULAR_DATA) == preference) {
            intent.putExtra(SimDialogActivity.DIALOG_TYPE_KEY, SimDialogActivity.DATA_PICK);
            context.startActivity(intent);
        } else if (findPreference(KEY_CALLS) == preference) {
            intent.putExtra(SimDialogActivity.DIALOG_TYPE_KEY, SimDialogActivity.CALLS_PICK);
            context.startActivity(intent);
        } else if (findPreference(KEY_SMS) == preference) {
            intent.putExtra(SimDialogActivity.DIALOG_TYPE_KEY, SimDialogActivity.SMS_PICK);
            context.startActivity(intent);
        /// M: Preferred SIM
        } else if (findPreference("primary_SIM_key") == preference) {
            log("host onPreferenceTreeClick 1");
            mSimManagementExt.onPreferenceClick(context);
            return true;
        /// M: for plugin like Smart Call Forwarding] @{
        } else {
            mSimManagementExt.handleEvent(this, context, preference);
        /// @}
        }

        return true;
    }

    /// M: Revise for supporting radio power switching.
    private class SimPreference extends RadioPowerPreference {
        private SubscriptionInfo mSubInfoRecord;
        private int mSlotId;
        Context mContext;

        public SimPreference(Context context, SubscriptionInfo subInfoRecord, int slotId) {
            super(context);

            mContext = context;
            mSubInfoRecord = subInfoRecord;
            mSlotId = slotId;
            setKey("sim" + mSlotId);
            update();
        }

        public void update() {
            final Resources res = mContext.getResources();

            setTitle(String.format(mContext.getResources()
                    .getString(R.string.sim_editor_title), (mSlotId + 1)));
            if (mSubInfoRecord != null) {
                /// M: For ALPS02871084, get phone number once @{
                String phoneNum = getPhoneNumber(mSubInfoRecord);
                logInEng("slot=" + mSlotId + ", phoneNum=" + phoneNum);

                if (TextUtils.isEmpty(phoneNum)) {
                    setSummary(mSubInfoRecord.getDisplayName());
                } else {
                    setSummary(mSubInfoRecord.getDisplayName() + " - " +
                            PhoneNumberUtils.createTtsSpannable(phoneNum));
                    setEnabled(true);
                }
                /// @}

                setIcon(new BitmapDrawable(res, (mSubInfoRecord.createIconBitmap(mContext))));

                /// M: Add for supporting radio power switching. @{
                int subId = mSubInfoRecord.getSubscriptionId();
                boolean radioOn = TelephonyUtils.isRadioOn(subId, getContext());
                boolean radioSwitchDone = mRadioController.isRadioSwitchComplete(subId, radioOn);
                setRadioEnabled(!mIsAirplaneModeOn && radioSwitchDone
                        && mSimManagementExt.customizeCallStateNotInCall());
                if (radioSwitchDone) {
                    setRadioOn(!mIsAirplaneModeOn && radioOn);
                }
                /// @}
            } else {
                setSummary(R.string.sim_slot_empty);
                setFragment(null);
                setEnabled(false);
            }
        }

        private int getSlotId() {
            return mSlotId;
        }
    }

    // Returns the line1Number. Line1number should always be read from TelephonyManager since it can
    // be overridden for display purposes.
    private String getPhoneNumber(SubscriptionInfo info) {
        final TelephonyManager tm =
            (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        return tm.getLine1Number(info.getSubscriptionId());
    }

    private void log(String s) {
        Log.d(TAG, s);
    }

    /**
     * For search
     */
    public static final SearchIndexProvider SEARCH_INDEX_DATA_PROVIDER =
            new BaseSearchIndexProvider() {
                @Override
                public List<SearchIndexableResource> getXmlResourcesToIndex(Context context,
                        boolean enabled) {
                    ArrayList<SearchIndexableResource> result =
                            new ArrayList<SearchIndexableResource>();

                    if (Utils.showSimCardTile(context)) {
                        SearchIndexableResource sir = new SearchIndexableResource(context);
                        sir.xmlResId = R.xml.sim_settings;
                        result.add(sir);
                    }

                    return result;
                }
            };

    /// M: Add for SIM state changed checking. @{
    private void initForSimStateChange() {
        /// M: Add for supporting SIM hot swap. @{
        mSimHotSwapHandler = new SimHotSwapHandler(getActivity().getApplicationContext());
        mSimHotSwapHandler.registerOnSimHotSwap(new OnSimHotSwapListener() {
            @Override
            public void onSimHotSwap() {
                if (getActivity() != null) {
                    log("onSimHotSwap, finish Activity.");
                    getActivity().finish();
                }
            }
        });
        /// @}

        mIsAirplaneModeOn = TelephonyUtils.isAirplaneModeOn(getActivity().getApplicationContext());
        logInEng("initForSimStateChange, airplaneMode=" + mIsAirplaneModeOn);

        /// M: Get the SIM lock policy/valid/capability. @{
        mSimLockPolicy = TelephonyUtils.getSimLockPolicy();
        log("initForSimStateChange, policy="
                + TelephonyUtils.getSimLockPolicyString(mSimLockPolicy));

        mSimLockSimValid = new int[mNumSlots];
        mSimLockSimInserted = new boolean[mNumSlots];

        // Set the SIM validity values here.
        // SIM inserted values will be set in updateSubscriptions().
        for (int i = 0; i < mNumSlots; i++) {
            mSimLockSimValid[i] = TelephonyUtils.getSimLockSimValid(i);
        }
        /// @}

        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_DEFAULT_VOICE_SUBSCRIPTION_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_DEFAULT_SMS_SUBSCRIPTION_CHANGED);

        // For radio on/off
        intentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        intentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED);
        // listen to PHONE_STATE_CHANGE
        intentFilter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        // listen to Telecom Manager event
        intentFilter.addAction(TelecomManager.ACTION_PHONE_ACCOUNT_REGISTERED);
        intentFilter.addAction(TelecomManager.ACTION_PHONE_ACCOUNT_UNREGISTERED);
        /// M: Listen the SIM lock state change event. @{
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION);
        /// @}
        /// M: Add for SIM On/Off feature. @{
        if (mSimOnOffEnabled) {
            intentFilter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
        }
        /// @}

        getActivity().registerReceiver(mReceiver, intentFilter);
    }
    /// @}

    /// M: Update SIM values after radio switch. @{
    private void handleRadioPowerSwitchComplete() {
        logInEng("handleRadioPowerSwitchComplete");
        /// M: Add for SIM On/Off feature. @{
        // Update the SIM slot values when the SIM On/Off feature is not enabled only.
        if (!mSimOnOffEnabled) {
            updateSimSlotValues();
        }
        /// @}
        updateActivitesCategory();
    }
    /// @}

    /// M: Update SIM values after airplane mode changed. @{
    private void handleAirplaneModeChange(Intent intent) {
        mIsAirplaneModeOn = intent.getBooleanExtra("state", false);
        Log.d(TAG, "airplaneMode=" + mIsAirplaneModeOn);
        updateSimSlotValues();
        updateActivitesCategory();
        removeItemsForTablet();
        /// M: Add for SIM settings plugin.
        mSimManagementExt.updatePrefState();
    }
    /// @}

    /// M: Add for SIM Lock feature. @{
    /**
     * When SIM lock state changed, some parts need to be enabled or disabled.
     */
    private void handleSimLockStateChange(Intent intent) {
        Bundle extra = intent.getExtras();
        if (extra == null) {
            Log.d(TAG, "handleSimLockStateChange, extra=null");
            return;
        }

        mSimLockPolicy = extra.getInt(
                MtkIccCardConstants.INTENT_KEY_SML_SLOT_DEVICE_LOCK_POLICY,
                mSimLockPolicy);

        int slotId = extra.getInt(PhoneConstants.SLOT_KEY,
                SubscriptionManager.INVALID_SIM_SLOT_INDEX);
        if (SubscriptionManager.isValidSlotIndex(slotId)) {
            mSimLockSimValid[slotId] = extra.getInt(
                    MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM_VALID,
                    mSimLockSimValid[slotId]);
            Log.d(TAG, "handleSimLockStateChange, policy="
                    + TelephonyUtils.getSimLockPolicyString(mSimLockPolicy)
                    + ", slotId=" + slotId
                    + ", simValid=" + TelephonyUtils.getSimLockSimValidString(
                            mSimLockSimValid[slotId]));
        }

        updateActivitesCategory();
    }

    private boolean shouldEnableCellularDataPrefForSimLock() {
        boolean enable = true;
        int caseId = TelephonyUtils.SIM_LOCK_CASE_NONE;

        switch (mSimLockPolicy) {
            case TelephonyUtils.SIM_LOCK_POLICY_UNKNOWN:
                enable = false;
                break;

            case TelephonyUtils.SIM_LOCK_POLICY_SLOT1_ONLY:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT2_ONLY:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_CS:
                caseId = TelephonyUtils.getSimLockCase(
                        mNumSlots, mSimLockSimInserted, mSimLockSimValid);
                if (caseId != TelephonyUtils.SIM_LOCK_CASE_ALL_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_INVALID_N_VALID) {
                    enable = false;
                }
                break;

            case TelephonyUtils.SIM_LOCK_POLICY_SLOT1_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT2_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_ECC:
                caseId = TelephonyUtils.getSimLockCase(
                        mNumSlots, mSimLockSimInserted, mSimLockSimValid);
                if (caseId != TelephonyUtils.SIM_LOCK_CASE_ALL_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_INVALID_1_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_INVALID_N_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_UNKNOWN_1_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_UNKNOWN_N_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_UNKNOWN_INVALID_1_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_UNKNOWN_INVALID_N_VALID) {
                    enable = false;
                }
                break;

            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_REVERSE:
                caseId = TelephonyUtils.getSimLockCase(
                        mNumSlots, mSimLockSimInserted, mSimLockSimValid);
                if (caseId != TelephonyUtils.SIM_LOCK_CASE_ALL_VALID) {
                    enable = false;
                }
                break;

            default:
                break;
        }

        if (!enable) {
            Log.d(TAG, "Disable data SIM for policy="
                    + TelephonyUtils.getSimLockPolicyString(mSimLockPolicy)
                    + ", case=" + TelephonyUtils.getSimLockCaseString(caseId));
        }

        return enable;
    }

    private boolean shouldEnableCallPrefForSimLock() {
        boolean enable = true;
        // There is no call SIM requirement for SIM Lock feature now.
        return enable;
    }

    private boolean shouldEnableSmsPrefForSimLock() {
        boolean enable = true;
        // There is no SMS SIM requirement for SIM Lock feature now.
        return enable;
    }
    /// @}

    /// M: Add for SIM On/Off feature. @{
    /**
     * When SIM on/off state changed, some parts need to be enabled or disabled.
     */
    private void handleSimApplicationStateChange(Intent intent) {
        if (!mSimOnOffEnabled) {
            Log.d(TAG, "handleSimApplicationStateChange, SIM On/Off is not enabled.");
            return;
        }

        Bundle extra = intent.getExtras();
        if (extra == null) {
            Log.d(TAG, "handleSimApplicationStateChange, extra=null");
            return;
        }

        int slotId = extra.getInt(PhoneConstants.SLOT_KEY,
                SubscriptionManager.INVALID_SIM_SLOT_INDEX);
        if (SubscriptionManager.isValidSlotIndex(slotId)) {
            Preference pref = mSimCards.findPreference("sim" + slotId);
            if (pref != null && (pref instanceof SimPowerPreference)) {
                ((SimPowerPreference) pref).update(mIsAirplaneModeOn);
            }
            Log.d(TAG, "handleSimApplicationStateChange, slotId=" + slotId);
        }
    }
    /// @}

    /// M: Remove preference for tablet. @{
    private void removeItemsForTablet() {
        // remove some item when in 4gds wifi-only
        if (FeatureOption.MTK_PRODUCT_IS_TABLET) {
            Preference sim_call_Pref = findPreference(KEY_CALLS);
            Preference sim_sms_Pref = findPreference(KEY_SMS);
            Preference sim_data_Pref = findPreference(KEY_CELLULAR_DATA);
            PreferenceCategory mPreferenceCategoryActivities =
                (PreferenceCategory) findPreference(KEY_SIM_ACTIVITIES);
            TelephonyManager tm = TelephonyManager.from(getActivity());
            if (!tm.isSmsCapable() && sim_sms_Pref != null) {
                mPreferenceCategoryActivities.removePreference(sim_sms_Pref);
            }
            if (!tm.isMultiSimEnabled() && sim_data_Pref != null && sim_sms_Pref != null) {
                mPreferenceCategoryActivities.removePreference(sim_data_Pref);
                mPreferenceCategoryActivities.removePreference(sim_sms_Pref);
            }
            if (!tm.isVoiceCapable() && sim_call_Pref != null) {
                mPreferenceCategoryActivities.removePreference(sim_call_Pref);
            }
        }
    }
    /// @}

    @Override
    public void onDestroy() {
        logInEng("onDestroy()");
        /// M: Add for SIM state changed checking.
        getActivity().unregisterReceiver(mReceiver);
        /// M: Add for supporting SIM hot swap.
        mSimHotSwapHandler.unregisterOnSimHotSwap();
        /// M: Add for SIM settings plugin.
        mSimManagementExt.onDestroy();
        super.onDestroy();
    }

    /// M: Add more conditions to set enabled state. @{
    private boolean shouldEnableSimPref(boolean defaultState) {
        /// M: Check whether the data SIM can be enabled for SIM Lock. @{
        boolean enabledForSimLock = shouldEnableCellularDataPrefForSimLock();
        if (!enabledForSimLock) {
            return false;
        }
        /// @}

        String ecbMode = SystemProperties.get(TelephonyProperties.PROPERTY_INECM_MODE, "false");
        boolean isInEcbMode = false;
        if (ecbMode != null && ecbMode.contains("true")) {
            isInEcbMode = true;
        }
        boolean capSwitching = TelephonyUtils.isCapabilitySwitching();
        boolean inCall = TelephonyUtils.isInCall();
        /// M: Add for supporting RSIM. @{
        int rsimPhoneId = -1;
        if (SystemProperties.getInt("ro.vendor.mtk_non_dsda_rsim_support", 0) == 1) {
            rsimPhoneId = SystemProperties.getInt(
                    MtkTelephonyProperties.PROPERTY_PREFERED_REMOTE_SIM, -1);
        }
        /// @}

        log("defaultState=" + defaultState + ", capSwitching=" + capSwitching
                + ", airplaneModeOn=" + mIsAirplaneModeOn + ", inCall=" + inCall
                + ", ecbMode=" + ecbMode + ", rsimPhoneId=" + rsimPhoneId);
        return defaultState && !capSwitching && !mIsAirplaneModeOn && !inCall
                && !isInEcbMode && (rsimPhoneId == -1);
    }
    /// @}

    private void logInEng(String s) {
        if (ENG_LOAD) {
            Log.d(TAG, s);
        }
    }
}

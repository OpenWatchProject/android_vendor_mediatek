/*
 * Copyright (C) 2018 The Android Open Source Project
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
package com.android.settings.network.telephony;

import android.app.Activity;
import android.app.settings.SettingsEnums;
/// M: Add for handling broadcast and event.
import android.content.BroadcastReceiver;
import android.content.Context;
/// M: Add for handling broadcast and event. @{
import android.content.Intent;
import android.content.IntentFilter;
/// @}
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.provider.Settings;
import android.telephony.AccessNetworkConstants;
import android.telephony.CarrierConfigManager;
import android.telephony.CellIdentity;
import android.telephony.CellInfo;
import android.telephony.NetworkRegistrationInfo;
/// M: Add for handling broadcast and event.
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;

import androidx.annotation.VisibleForTesting;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;

import com.android.internal.telephony.OperatorInfo;
/// M: Add for SIM On/Off feature.
import com.android.internal.telephony.PhoneConstants;
import com.android.settings.R;
import com.android.settings.dashboard.DashboardFragment;
import com.android.settings.overlay.FeatureFactory;
import com.android.settingslib.core.instrumentation.MetricsFeatureProvider;
import com.android.settingslib.utils.ThreadUtils;

import com.mediatek.settings.UtilsExt;
/// M: Add for supporting SIM hot swap. @{
import com.mediatek.settings.sim.SimHotSwapHandler;
import com.mediatek.settings.sim.SimHotSwapHandler.OnSimHotSwapListener;
/// @}
/// M: Add for checking SIM state.
import com.mediatek.settings.sim.TelephonyUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * "Choose network" settings UI for the Phone app.
 */
public class NetworkSelectSettings extends DashboardFragment {

    private static final String TAG = "NetworkSelectSettings";

    private static final int EVENT_SET_NETWORK_SELECTION_MANUALLY_DONE = 1;
    private static final int EVENT_NETWORK_SCAN_RESULTS = 2;
    private static final int EVENT_NETWORK_SCAN_ERROR = 3;
    private static final int EVENT_NETWORK_SCAN_COMPLETED = 4;

    private static final String PREF_KEY_CONNECTED_NETWORK_OPERATOR =
            "connected_network_operator_preference";
    private static final String PREF_KEY_NETWORK_OPERATORS = "network_operators_preference";

    @VisibleForTesting
    PreferenceCategory mPreferenceCategory;
    @VisibleForTesting
    PreferenceCategory mConnectedPreferenceCategory;
    @VisibleForTesting
    NetworkOperatorPreference mSelectedPreference;
    private View mProgressHeader;
    private Preference mStatusMessagePreference;
    @VisibleForTesting
    List<CellInfo> mCellInfoList;
    private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    @VisibleForTesting
    TelephonyManager mTelephonyManager;
    private List<String> mForbiddenPlmns;
    private boolean mShow4GForLTE = false;
    private NetworkScanHelper mNetworkScanHelper;
    private final ExecutorService mNetworkScanExecutor = Executors.newFixedThreadPool(1);
    private MetricsFeatureProvider mMetricsFeatureProvider;
    private boolean mUseNewApi;

    /// M: Add for handling phone change. @{
    private PhoneChangeReceiver mPhoneChangeReceiver = new PhoneChangeReceiver();
    private PhoneStateListener mPhoneStateListener = new PhoneStateListener();
    private boolean mRegistered = false;
    /// @}

    /// M: Add for supporting SIM hot swap.
    private SimHotSwapHandler mSimHotSwapHandler;

    /// M: Add for SIM On/Off feature. @{
    private boolean mSimOnOffEnabled = false;
    private int mCurSimState = TelephonyUtils.SIM_ONOFF_STATE_ON;
    /// @}

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        mUseNewApi = getContext().getResources().getBoolean(
                com.android.internal.R.bool.config_enableNewAutoSelectNetworkUI);
        mSubId = getArguments().getInt(Settings.EXTRA_SUB_ID);

        mConnectedPreferenceCategory = findPreference(PREF_KEY_CONNECTED_NETWORK_OPERATOR);
        mPreferenceCategory = findPreference(PREF_KEY_NETWORK_OPERATORS);
        mStatusMessagePreference = new Preference(getContext());
        mStatusMessagePreference.setSelectable(false);
        mSelectedPreference = null;
        mTelephonyManager = TelephonyManager.from(getContext()).createForSubscriptionId(mSubId);
        mNetworkScanHelper = new NetworkScanHelper(
                mTelephonyManager, mCallback, mNetworkScanExecutor);
        PersistableBundle bundle = ((CarrierConfigManager) getContext().getSystemService(
                Context.CARRIER_CONFIG_SERVICE)).getConfigForSubId(mSubId);
        if (bundle != null) {
            mShow4GForLTE = bundle.getBoolean(
                    CarrierConfigManager.KEY_SHOW_4G_FOR_LTE_DATA_ICON_BOOL);
        }

        mMetricsFeatureProvider = FeatureFactory
                .getFactory(getContext()).getMetricsFeatureProvider();

        /// M: Add for supporting SIM hot swap. @{
        mSimHotSwapHandler = new SimHotSwapHandler(getContext());
        mSimHotSwapHandler.registerOnSimHotSwap(new OnSimHotSwapListener() {
            @Override
            public void onSimHotSwap() {
                if (getActivity() != null) {
                    Log.d(TAG, "onSimHotSwap, finish Activity.");
                    getActivity().finish();
                }
            }
        });
        /// @}

        /// M: Add for SIM On/Off feature. @{
        mSimOnOffEnabled = TelephonyUtils.isSimOnOffEnabled();
        int slotId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
        if (SubscriptionManager.isValidSubscriptionId(mSubId)) {
            slotId = SubscriptionManager.getSlotIndex(mSubId);
            mCurSimState = TelephonyUtils.getSimOnOffState(slotId);
        }
        Log.d(TAG, "onCreate, simOnOffEnabled=" + mSimOnOffEnabled
                + ", slotId=" + slotId + ", subId=" + mSubId
                + ", simState=" + mCurSimState);
        /// @}
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        final Activity activity = getActivity();
        if (activity != null) {
            mProgressHeader = setPinnedHeaderView(R.layout.progress_header)
                    .findViewById(R.id.progress_bar_animation);
            setProgressBarVisible(false);
        }
        /// M: Move updating connected network into onStart().
        //forceUpdateConnectedPreferenceCategory();
    }

    @Override
    public void onStart() {
        super.onStart();

        /// M: No need to start network scan when setting is disabled. @{
        boolean enabled = TelephonyUtils.shouldEnableMobileNetworkOption(
                getActivity(), mSubId);
        Log.d(TAG, "onStart, enabled=" + enabled);

        mConnectedPreferenceCategory.removeAll();
        mConnectedPreferenceCategory.setEnabled(enabled);
        mConnectedPreferenceCategory.setVisible(false);
        mPreferenceCategory.removeAll();
        mPreferenceCategory.setEnabled(enabled);

        if (enabled) {
            forceUpdateConnectedPreferenceCategory();

            updateForbiddenPlmns();
            setProgressBarVisible(true);

            mNetworkScanHelper.startNetworkScan(
                    mUseNewApi
                            ? NetworkScanHelper.NETWORK_SCAN_TYPE_INCREMENTAL_RESULTS
                            : NetworkScanHelper.NETWORK_SCAN_TYPE_WAIT_FOR_ALL_RESULTS);
        } else {
            addMessagePreference(R.string.network_query_error);
        }
        /// @}

        /// M: Add for handling broadcast and event.
        registerPhoneChangeReceiver();
    }

    /**
     * Update forbidden PLMNs from the USIM App
     */
    @VisibleForTesting
    void updateForbiddenPlmns() {
        final String[] forbiddenPlmns = mTelephonyManager.getForbiddenPlmns();
        mForbiddenPlmns = forbiddenPlmns != null
                ? Arrays.asList(forbiddenPlmns)
                : new ArrayList<>();
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop");
        super.onStop();
        stopNetworkQuery();

        /// M: Add for skipping network selection result. @{
        mSelectedPreference = null;
        setProgressBarVisible(false);
        getPreferenceScreen().setEnabled(true);
        /// @}

        /// M: Add for handling broadcast and event.
        unregisterPhoneChangeReceiver();
    }

    @Override
    public boolean onPreferenceTreeClick(Preference preference) {
        if (preference != mSelectedPreference) {
            stopNetworkQuery();
            ///M: for opearotr feature start @{
            NetworkOperatorPreference customizationSelectedPreference
                = (NetworkOperatorPreference) preference;
            CellInfo customizationCellInfo = customizationSelectedPreference.getCellInfo();
            OperatorInfo customizationOperatorInfo
                = CellInfoUtil.getOperatorInfoFromCellInfo(customizationCellInfo);
            Context customizationContext = getActivity();
            if (UtilsExt.getNetworkSettingExt(customizationContext)
                    .onPreferenceTreeClick(customizationOperatorInfo, mSubId)) {
                Log.i(TAG, "for opeartor feature return true");
                return true;
            }
            ///M: for opearotr feature end @ }
            // Refresh the last selected item in case users reselect network.
            if (mSelectedPreference != null) {
                mSelectedPreference.setSummary(null);
            }

            mSelectedPreference = (NetworkOperatorPreference) preference;
            CellInfo cellInfo = mSelectedPreference.getCellInfo();
            mSelectedPreference.setSummary(R.string.network_connecting);

            mMetricsFeatureProvider.action(getContext(),
                    SettingsEnums.ACTION_MOBILE_NETWORK_MANUAL_SELECT_NETWORK);

            // Set summary as "Disconnected" to the previously connected network
            if (mConnectedPreferenceCategory.getPreferenceCount() > 0) {
                NetworkOperatorPreference connectedNetworkOperator = (NetworkOperatorPreference)
                        (mConnectedPreferenceCategory.getPreference(0));
                /// M: Add for updating connected preference category. @{
                String connectedNumeric = CellInfoUtil.getOperatorInfoFromCellInfo(
                        connectedNetworkOperator.getCellInfo()).getOperatorNumeric();
                String selectedNumeric = CellInfoUtil.getOperatorInfoFromCellInfo(
                        cellInfo).getOperatorNumeric();
                CharSequence connectedSummary = connectedNetworkOperator.getSummary();
                Log.d(TAG, "onPreferenceTreeClick, connectedNumeric=" + connectedNumeric
                        + ", selectedNumeric=" + selectedNumeric
                        + ", connectedSummary="
                        + (connectedSummary == null ? "null" : connectedSummary));
                if (!selectedNumeric.equals(connectedNumeric)
                        && connectedSummary != null && connectedSummary.length() > 0) {
                    connectedNetworkOperator.setSummary(R.string.network_disconnected);
                }
                /// @}
            }

            setProgressBarVisible(true);
            // Disable the screen until network is manually set
            getPreferenceScreen().setEnabled(false);

            final OperatorInfo operatorInfo = CellInfoUtil.getOperatorInfoFromCellInfo(cellInfo);
            ThreadUtils.postOnBackgroundThread(() -> {
                Message msg = mHandler.obtainMessage(EVENT_SET_NETWORK_SELECTION_MANUALLY_DONE);
                msg.obj = mTelephonyManager.setNetworkSelectionModeManual(
                        operatorInfo, true /* persistSelection */);
                msg.sendToTarget();
            });
        }

        return true;
    }

    @Override
    protected int getPreferenceScreenResId() {
        return R.xml.choose_network;
    }

    @Override
    protected String getLogTag() {
        return TAG;
    }

    @Override
    public int getMetricsCategory() {
        return SettingsEnums.MOBILE_NETWORK_SELECT;
    }

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_SET_NETWORK_SELECTION_MANUALLY_DONE:
                    /// M: Add for skipping network selection result. @{
                    if (mSelectedPreference == null) {
                        Log.d(TAG, "handleMessage, selectedPreference doesn't exist.");
                        return;
                    }
                    /// @}

                    setProgressBarVisible(false);
                    getPreferenceScreen().setEnabled(true);

                    boolean isSucceed = (boolean) msg.obj;
                    mSelectedPreference.setSummary(isSucceed
                            ? R.string.network_connected
                            : R.string.network_could_not_connect);

                    /// M: Add for updating connected preference category. @{
                    Log.d(TAG, "handleMessage, select network manually done, result="
                            + isSucceed);
                    if (mConnectedPreferenceCategory.getPreferenceCount() > 0) {
                        NetworkOperatorPreference connectedNetworkOperator =
                                (NetworkOperatorPreference)
                                        (mConnectedPreferenceCategory.getPreference(0));
                        CellInfo cellInfo = mSelectedPreference.getCellInfo();
                        String connectedNumeric = CellInfoUtil.getOperatorInfoFromCellInfo(
                                connectedNetworkOperator.getCellInfo()).getOperatorNumeric();
                        String selectedNumeric = CellInfoUtil.getOperatorInfoFromCellInfo(
                                cellInfo).getOperatorNumeric();
                        Log.d(TAG, "handleMessage, connectedNumeric=" + connectedNumeric
                                + ", selectedNumeric=" + selectedNumeric);
                        if (selectedNumeric.equals(connectedNumeric)) {
                            if (isSucceed) {
                                connectedNetworkOperator.setSummary(R.string.network_connected);
                            } else {
                                connectedNetworkOperator.setSummary("");
                            }
                        } else {
                            connectedNetworkOperator.setSummary("");
                        }
                    }
                    /// @}
                    break;
                case EVENT_NETWORK_SCAN_RESULTS:
                    List<CellInfo> results = aggregateCellInfoList((List<CellInfo>) msg.obj);
                    Log.d(TAG, "CellInfoList after aggregation: "
                            + CellInfoUtil.cellInfoListToString(results));
                    mCellInfoList = new ArrayList<>(results);
                    if (mCellInfoList != null && mCellInfoList.size() != 0) {
                        updateAllPreferenceCategory();
                    } else {
                        addMessagePreference(R.string.empty_networks_list);
                    }

                    break;

                case EVENT_NETWORK_SCAN_ERROR:
                    stopNetworkQuery();
                    addMessagePreference(R.string.network_query_error);
                    break;

                case EVENT_NETWORK_SCAN_COMPLETED:
                    stopNetworkQuery();
                    if (mCellInfoList == null) {
                        // In case the scan timeout before getting any results
                        addMessagePreference(R.string.empty_networks_list);
                    }
                    break;
            }
            return;
        }
    };

    private final NetworkScanHelper.NetworkScanCallback mCallback =
            new NetworkScanHelper.NetworkScanCallback() {
                public void onResults(List<CellInfo> results) {
                    Message msg = mHandler.obtainMessage(EVENT_NETWORK_SCAN_RESULTS, results);
                    msg.sendToTarget();
                }

                public void onComplete() {
                    Message msg = mHandler.obtainMessage(EVENT_NETWORK_SCAN_COMPLETED);
                    msg.sendToTarget();
                }

                public void onError(int error) {
                    Message msg = mHandler.obtainMessage(EVENT_NETWORK_SCAN_ERROR, error,
                            0 /* arg2 */);
                    msg.sendToTarget();
                }
            };

    /**
     * Update the currently available network operators list, which only contains the unregistered
     * network operators. So if the device has no data and the network operator in the connected
     * network operator category shows "Disconnected", it will also exist in the available network
     * operator category for user to select. On the other hand, if the device has data and the
     * network operator in the connected network operator category shows "Connected", it will not
     * exist in the available network category.
     */
    @VisibleForTesting
    void updateAllPreferenceCategory() {
        updateConnectedPreferenceCategory();

        mPreferenceCategory.removeAll();
        for (int index = 0; index < mCellInfoList.size(); index++) {
            if (!mCellInfoList.get(index).isRegistered()) {
                NetworkOperatorPreference pref = new NetworkOperatorPreference(
                        mCellInfoList.get(index), getPrefContext(), mForbiddenPlmns, mShow4GForLTE);
                pref.setKey(CellInfoUtil.getNetworkTitle(mCellInfoList.get(index)));
                pref.setOrder(index);
                mPreferenceCategory.addPreference(pref);
            }
        }
    }

    /**
     * Config the connected network operator preference when the page was created. When user get
     * into this page, the device might or might not have data connection.
     * - If the device has data:
     * 1. use {@code ServiceState#getNetworkRegistrationInfoList()} to get the currently
     * registered cellIdentity, wrap it into a CellInfo;
     * 2. set the signal strength level as strong;
     * 3. use {@link TelephonyManager#getNetworkOperatorName()} to get the title of the
     * previously connected network operator, since the CellIdentity got from step 1 only has
     * PLMN.
     * - If the device has no data, we will remove the connected network operators list from the
     * screen.
     */
    private void forceUpdateConnectedPreferenceCategory() {
        /// M: Add for checking the data state for data SIM only.
        if (mSubId == SubscriptionManager.getDefaultDataSubscriptionId()
                && mTelephonyManager.getDataState() == mTelephonyManager.DATA_CONNECTED) {
            Log.d(TAG, "forceUpdateConnectedPreferenceCategory, data is connected.");
            // Try to get the network registration states
            ServiceState ss = mTelephonyManager.getServiceState();
            List<NetworkRegistrationInfo> networkList =
                    ss.getNetworkRegistrationInfoListForTransportType(
                            AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
            if (networkList == null || networkList.size() == 0) {
                Log.d(TAG, "forceUpdateConnectedPreferenceCategory, network list is empty.");
                // Remove the connected network operators category
                mConnectedPreferenceCategory.setVisible(false);
                return;
            }
            CellIdentity cellIdentity = networkList.get(0).getCellIdentity();
            CellInfo cellInfo = CellInfoUtil.wrapCellInfoWithCellIdentity(cellIdentity);
            if (cellInfo != null) {
                NetworkOperatorPreference pref = new NetworkOperatorPreference(
                        cellInfo, getPrefContext(), mForbiddenPlmns, mShow4GForLTE);
                pref.setTitle(mTelephonyManager.getNetworkOperatorName());
                pref.setSummary(R.string.network_connected);
                // Update the signal strength icon, since the default signalStrength value would be
                // zero (it would be quite confusing why the connected network has no signal)
                pref.setIcon(SignalStrength.NUM_SIGNAL_STRENGTH_BINS - 1);
                mConnectedPreferenceCategory.addPreference(pref);
                /// M: Add for showing the connected network.
                mConnectedPreferenceCategory.setVisible(true);
            } else {
                Log.d(TAG, "forceUpdateConnectedPreferenceCategory, no cellInfo is found.");
                // Remove the connected network operators category
                mConnectedPreferenceCategory.setVisible(false);
            }
        } else {
            Log.d(TAG, "forceUpdateConnectedPreferenceCategory, data is not connected.");
            // Remove the connected network operators category
            mConnectedPreferenceCategory.setVisible(false);
        }
    }

    /**
     * Configure the ConnectedNetworkOperatorsPreferenceCategory. The category only need to be
     * configured if the category is currently empty or the operator network title of the previous
     * connected network is different from the new one.
     */
    private void updateConnectedPreferenceCategory() {
        CellInfo connectedNetworkOperator = null;
        for (CellInfo cellInfo : mCellInfoList) {
            if (cellInfo.isRegistered()) {
                connectedNetworkOperator = cellInfo;
                break;
            }
        }

        Log.d(TAG, "updateConnectedPreferenceCategory, connectedNetworkOperator="
                + (connectedNetworkOperator == null ? "null" : connectedNetworkOperator));

        if (connectedNetworkOperator != null) {
            addConnectedNetworkOperatorPreference(connectedNetworkOperator);
        }
    }

    private void addConnectedNetworkOperatorPreference(CellInfo cellInfo) {
        mConnectedPreferenceCategory.removeAll();
        final NetworkOperatorPreference pref = new NetworkOperatorPreference(
                cellInfo, getPrefContext(), mForbiddenPlmns, mShow4GForLTE);
        pref.setSummary(R.string.network_connected);
        mConnectedPreferenceCategory.addPreference(pref);
        mConnectedPreferenceCategory.setVisible(true);
    }

    protected void setProgressBarVisible(boolean visible) {
        if (mProgressHeader != null) {
            mProgressHeader.setVisibility(visible ? View.VISIBLE : View.GONE);
        }
    }

    private void addMessagePreference(int messageId) {
        setProgressBarVisible(false);
        mStatusMessagePreference.setTitle(messageId);
        mConnectedPreferenceCategory.setVisible(false);
        mPreferenceCategory.removeAll();
        mPreferenceCategory.addPreference(mStatusMessagePreference);
    }

    /**
     * The Scan results may contains several cell infos with different radio technologies and signal
     * strength for one network operator. Aggregate the CellInfoList by retaining only the cell info
     * with the strongest signal strength.
     */
    private List<CellInfo> aggregateCellInfoList(List<CellInfo> cellInfoList) {
        Map<String, CellInfo> map = new HashMap<>();
        for (CellInfo cellInfo : cellInfoList) {
            String plmn = CellInfoUtil.getOperatorInfoFromCellInfo(cellInfo).getOperatorNumeric();

            /// M: Add for skipping the 46020 plmn. @{
            if (plmn != null && plmn.equals("46020")) {
                Log.d(TAG, "aggregateCellInfoList, skip the 46020 PLMN.");
                continue;
            }
            /// @}

            if (cellInfo.isRegistered() || !map.containsKey(plmn)) {
                map.put(plmn, cellInfo);
            } else {
                if (map.get(plmn).isRegistered()
                        || map.get(plmn).getCellSignalStrength().getLevel()
                        > cellInfo.getCellSignalStrength().getLevel()) {
                    // Skip if the stored cellInfo is registered or has higher signal strength level
                    continue;
                }
                // Otherwise replace it with the new CellInfo
                map.put(plmn, cellInfo);
            }
        }
        return new ArrayList<>(map.values());
    }

    private void stopNetworkQuery() {
        setProgressBarVisible(false);
        if (mNetworkScanHelper != null) {
            mNetworkScanHelper.stopNetworkQuery();
        }
    }

    @Override
    public void onDestroy() {
        /// M: Add for supporting SIM hot swap.
        mSimHotSwapHandler.unregisterOnSimHotSwap();
        mNetworkScanExecutor.shutdown();
        super.onDestroy();
    }

    /// M: Add for handling broadcast and event. @{
    private void registerPhoneChangeReceiver() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        intentFilter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);

        getActivity().registerReceiver(mPhoneChangeReceiver, intentFilter);

        mTelephonyManager.listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_RADIO_POWER_STATE_CHANGED);

        mRegistered = true;
    }

    private void unregisterPhoneChangeReceiver() {
        if (mRegistered) {
            getActivity().unregisterReceiver(mPhoneChangeReceiver);
            mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
            mRegistered = false;
        }
    }

    private void updateScreenStatus() {
        final Activity activity = getActivity();
        boolean inCall = TelephonyUtils.isInCall();
        boolean radioOn = TelephonyUtils.isRadioOn(mSubId, activity);
        boolean enabled = !inCall && radioOn
                && (mCurSimState == TelephonyUtils.SIM_ONOFF_STATE_ON);
        Log.d(TAG, "updateScreenStatus, subId=" + mSubId
                + ", enabled=" + enabled + ", inCall=" + inCall
                + ", radioOn=" + radioOn + ", simState=" + mCurSimState);

        mPreferenceCategory.setEnabled(enabled);
        mConnectedPreferenceCategory.setEnabled(enabled);
    }

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
        int subId = extra.getInt(PhoneConstants.SUBSCRIPTION_KEY,
                SubscriptionManager.INVALID_SIM_SLOT_INDEX);
        int state = TelephonyUtils.getSimOnOffState(slotId);
        if (SubscriptionManager.isValidSubscriptionId(subId)
                && (mSubId == subId) && (mCurSimState != state)) {
            Log.d(TAG, "handleSimApplicationStateChange, subId=" + subId
                    + ", preState=" + mCurSimState + ", curState=" + state);
            mCurSimState = state;
            updateScreenStatus();
        }
    }

    private class PhoneChangeReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive, action=" + action);

            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                finish();
            } else if (action.equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                updateScreenStatus();
            } else if (action.equals(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)) {
                handleSimApplicationStateChange(intent);
            }
        }
    }
    /// @}
}

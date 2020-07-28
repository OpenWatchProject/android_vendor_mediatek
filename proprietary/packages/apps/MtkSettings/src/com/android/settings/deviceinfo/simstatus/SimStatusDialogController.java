/*
 * Copyright (C) 2017 The Android Open Source Project
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

package com.android.settings.deviceinfo.simstatus;

import static android.content.Context.CARRIER_CONFIG_SERVICE;
import static android.content.Context.EUICC_SERVICE;
import static android.content.Context.TELEPHONY_SERVICE;
import static android.content.Context.TELEPHONY_SUBSCRIPTION_SERVICE;

import android.Manifest;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.os.UserHandle;
import android.telephony.CarrierConfigManager;
import android.telephony.CellBroadcastMessage;
/// M: Add for updating CDMA SIM status.
import android.telephony.CellLocation;
import android.telephony.PhoneStateListener;
/// M: Listen precise data connection state for temp data service.
import android.telephony.PreciseDataConnectionState;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.OnSubscriptionsChangedListener;
import android.telephony.TelephonyManager;
/// M: Add for updating CDMA SIM status. @{
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.data.ApnSetting;
/// @}
import android.telephony.euicc.EuiccManager;
import android.text.BidiFormatter;
import android.text.TextDirectionHeuristics;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.android.settings.R;
import com.android.settingslib.DeviceInfoUtils;
import com.android.settingslib.Utils;
import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnPause;
import com.android.settingslib.core.lifecycle.events.OnResume;

/// M: Listen precise data connection state for temp data service.
import com.mediatek.internal.telephony.MtkPhoneConstants;
/// M: Add for SIM settings plugin.
import com.mediatek.settings.UtilsExt;
/// M: Add for updating CDMA SIM status.
import com.mediatek.settings.cdma.CdmaSimStatus;
/// M: Add for SIM settings plugin.
import com.mediatek.settings.ext.ISettingsMiscExt;
import com.mediatek.settings.ext.IStatusExt;
/// M: Add for revising the SIM operator numeric.
import com.mediatek.settings.sim.TelephonyUtils;

public class SimStatusDialogController implements LifecycleObserver, OnResume, OnPause {

    private final static String TAG = "SimStatusDialogCtrl";

    @VisibleForTesting
    final static int NETWORK_PROVIDER_VALUE_ID = R.id.operator_name_value;
    @VisibleForTesting
    final static int PHONE_NUMBER_VALUE_ID = R.id.number_value;
    @VisibleForTesting
    final static int CELLULAR_NETWORK_STATE = R.id.data_state_value;
    @VisibleForTesting
    final static int OPERATOR_INFO_LABEL_ID = R.id.latest_area_info_label;
    @VisibleForTesting
    final static int OPERATOR_INFO_VALUE_ID = R.id.latest_area_info_value;
    @VisibleForTesting
    final static int SERVICE_STATE_VALUE_ID = R.id.service_state_value;
    @VisibleForTesting
    final static int SIGNAL_STRENGTH_LABEL_ID = R.id.signal_strength_label;
    @VisibleForTesting
    final static int SIGNAL_STRENGTH_VALUE_ID = R.id.signal_strength_value;
    @VisibleForTesting
    final static int CELL_VOICE_NETWORK_TYPE_VALUE_ID = R.id.voice_network_type_value;
    @VisibleForTesting
    final static int CELL_DATA_NETWORK_TYPE_VALUE_ID = R.id.data_network_type_value;
    @VisibleForTesting
    final static int ROAMING_INFO_VALUE_ID = R.id.roaming_state_value;
    @VisibleForTesting
    final static int ICCID_INFO_LABEL_ID = R.id.icc_id_label;
    @VisibleForTesting
    final static int ICCID_INFO_VALUE_ID = R.id.icc_id_value;
    /// M: Add for updating EID label.
    final static int EID_INFO_LABEL_ID = R.id.esim_id_label;
    @VisibleForTesting
    final static int EID_INFO_VALUE_ID = R.id.esim_id_value;
    @VisibleForTesting
    final static int IMS_REGISTRATION_STATE_LABEL_ID = R.id.ims_reg_state_label;
    @VisibleForTesting
    final static int IMS_REGISTRATION_STATE_VALUE_ID = R.id.ims_reg_state_value;

    /// M: Add for updating CDMA SIM status. @{
    @VisibleForTesting
    final static int MCC_MNC_INFO_LABEL_ID = R.id.mcc_mnc_id_label;
    @VisibleForTesting
    final static int MCC_MNC_INFO_VALUE_ID = R.id.mcc_mnc_id_value;
    @VisibleForTesting
    final static int SID_NID_INFO_LABEL_ID = R.id.sid_nid_id_label;
    @VisibleForTesting
    final static int SID_NID_INFO_VALUE_ID = R.id.sid_nid_id_value;
    @VisibleForTesting
    final static int BASE_ID_INFO_LABEL_ID = R.id.base_id_label;
    @VisibleForTesting
    final static int BASE_ID_INFO_VALUE_ID = R.id.base_id_value;

    private static final int MCC_LENGTH = 3;
    /// @}

    private final static String CB_AREA_INFO_RECEIVED_ACTION =
            "com.android.cellbroadcastreceiver.CB_AREA_INFO_RECEIVED";
    private final static String GET_LATEST_CB_AREA_INFO_ACTION =
            "com.android.cellbroadcastreceiver.GET_LATEST_CB_AREA_INFO";
    private final static String CELL_BROADCAST_RECEIVER_APP = "com.android.cellbroadcastreceiver";

    private final OnSubscriptionsChangedListener mOnSubscriptionsChangedListener =
            new OnSubscriptionsChangedListener() {
                @Override
                public void onSubscriptionsChanged() {
                    /// M: The SubscriptionInfo may be null. @{
                    if (mSubscriptionInfo != null) {
                        mSubscriptionInfo = mSubscriptionManager.getActiveSubscriptionInfo(
                                mSubscriptionInfo.getSubscriptionId());
                        updateNetworkProvider();
                    }
                    /// @}
                }
            };

    private SubscriptionInfo mSubscriptionInfo;

    private final SimStatusDialogFragment mDialog;
    private final TelephonyManager mTelephonyManager;
    private final SubscriptionManager mSubscriptionManager;
    private final CarrierConfigManager mCarrierConfigManager;
    private final EuiccManager mEuiccManager;
    private final Resources mRes;
    private final Context mContext;

    private boolean mShowLatestAreaInfo;
    private IStatusExt mStatusExt;

    private final BroadcastReceiver mAreaInfoReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (TextUtils.equals(action, CB_AREA_INFO_RECEIVED_ACTION)) {
                final Bundle extras = intent.getExtras();
                if (extras == null) {
                    return;
                }
                final CellBroadcastMessage cbMessage = (CellBroadcastMessage) extras.get("message");
                if (cbMessage != null
                        && mSubscriptionInfo.getSubscriptionId() == cbMessage.getSubId()) {
                    final String latestAreaInfo = cbMessage.getMessageBody();
                    mDialog.setText(OPERATOR_INFO_VALUE_ID, latestAreaInfo);
                }
            }
        }
    };

    private PhoneStateListener mPhoneStateListener;

    /// M: Add for updating CDMA SIM status.
    private CdmaSimStatus mCdmaSimStatus;
    /// M: Add for SIM settings plugin.
    private ISettingsMiscExt mMiscExt;

    public SimStatusDialogController(@NonNull SimStatusDialogFragment dialog, Lifecycle lifecycle,
            int slotId) {
        mDialog = dialog;
        mContext = dialog.getContext();
        mSubscriptionInfo = getPhoneSubscriptionInfo(slotId);

        /// M: Create TelephonyManager for subscription. @{
        if (mSubscriptionInfo != null) {
            mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(
                    mSubscriptionInfo.getSubscriptionId());
        } else {
            mTelephonyManager = mContext.getSystemService(TelephonyManager.class);
        }
        /// @}

        mCarrierConfigManager = mContext.getSystemService(CarrierConfigManager.class);
        mEuiccManager = mContext.getSystemService(EuiccManager.class);
        mSubscriptionManager = mContext.getSystemService(SubscriptionManager.class);

        mRes = mContext.getResources();

        if (lifecycle != null) {
            lifecycle.addObserver(this);
        }

        /// M: Add for updating CDMA SIM status.
        mCdmaSimStatus = new CdmaSimStatus(mDialog, mSubscriptionInfo);

        /// M: Add for SIM settings plugin.
        mMiscExt = UtilsExt.getMiscPlugin(mContext);
        /// M: Add for SIM status plugin.
        mStatusExt = UtilsExt.getStatusExt(mContext);
    }

    public void initialize() {
        updateEid();

        if (mSubscriptionInfo == null) {
            /// M: Remove CDMA status.
            removeCdmaStatus();
            return;
        }

        mPhoneStateListener = getPhoneStateListener();
        updateNetworkProvider();

        final ServiceState serviceState = getCurrentServiceState();
        updatePhoneNumber();
        updateLatestAreaInfo();
        updateServiceState(serviceState);
        updateSignalStrength(getSignalStrength());
        updateNetworkType();
        updateRoamingStatus(serviceState);
        updateIccidNumber();
        updateImsRegistrationState();

        /// M: Update CDMA status.
        updateCdmaStatus(serviceState);
    }

    @Override
    public void onResume() {
        if (mSubscriptionInfo == null) {
            return;
        }

        /// M: Update related status. @{
        updateDataState(mTelephonyManager.getDataState());
        final ServiceState serviceState = getCurrentServiceState();
        updateNetworkProvider();
        updateServiceState(serviceState);
        updateSignalStrength(getSignalStrength());
        updateNetworkType();
        updateRoamingStatus(serviceState);
        updateCdmaStatus(serviceState);
        /// @}

        mTelephonyManager.listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_DATA_CONNECTION_STATE
                        | PhoneStateListener.LISTEN_SIGNAL_STRENGTHS
                        | PhoneStateListener.LISTEN_SERVICE_STATE);
        mSubscriptionManager.addOnSubscriptionsChangedListener(mOnSubscriptionsChangedListener);

        if (mShowLatestAreaInfo) {
            mContext.registerReceiver(mAreaInfoReceiver,
                    new IntentFilter(CB_AREA_INFO_RECEIVED_ACTION),
                    Manifest.permission.RECEIVE_EMERGENCY_BROADCAST, null /* scheduler */);
            // Ask CellBroadcastReceiver to broadcast the latest area info received
            final Intent getLatestIntent = new Intent(GET_LATEST_CB_AREA_INFO_ACTION);
            getLatestIntent.setPackage(CELL_BROADCAST_RECEIVER_APP);
            mContext.sendBroadcastAsUser(getLatestIntent, UserHandle.ALL,
                    Manifest.permission.RECEIVE_EMERGENCY_BROADCAST);
        }
    }

    @Override
    public void onPause() {
        if (mSubscriptionInfo == null) {
            return;
        }

        mSubscriptionManager.removeOnSubscriptionsChangedListener(mOnSubscriptionsChangedListener);
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);

        if (mShowLatestAreaInfo) {
            mContext.unregisterReceiver(mAreaInfoReceiver);
        }
    }

    private void updateNetworkProvider() {
        final CharSequence carrierName =
                mSubscriptionInfo != null ? mSubscriptionInfo.getCarrierName() : null;
        mDialog.setText(NETWORK_PROVIDER_VALUE_ID, carrierName);
    }

    private void updatePhoneNumber() {
        // If formattedNumber is null or empty, it'll display as "Unknown".
        String number = mStatusExt.updatePhoneNumber(BidiFormatter.getInstance().unicodeWrap(
                getPhoneNumber(), TextDirectionHeuristics.LTR),
                mSubscriptionInfo.getSimSlotIndex(), mContext);
        mDialog.setText(PHONE_NUMBER_VALUE_ID, number);
    }

    private void updateDataState(int state) {
        String networkStateValue;

        switch (state) {
            case TelephonyManager.DATA_CONNECTED:
                networkStateValue = mRes.getString(R.string.radioInfo_data_connected);
                break;
            case TelephonyManager.DATA_SUSPENDED:
                networkStateValue = mRes.getString(R.string.radioInfo_data_suspended);
                break;
            case TelephonyManager.DATA_CONNECTING:
                networkStateValue = mRes.getString(R.string.radioInfo_data_connecting);
                break;
            case TelephonyManager.DATA_DISCONNECTED:
                networkStateValue = mRes.getString(R.string.radioInfo_data_disconnected);
                break;
            default:
                networkStateValue = mRes.getString(R.string.radioInfo_unknown);
                break;
        }

        mDialog.setText(CELLULAR_NETWORK_STATE, networkStateValue);
    }


    private void updateLatestAreaInfo() {
        mShowLatestAreaInfo = Resources.getSystem().getBoolean(
                com.android.internal.R.bool.config_showAreaUpdateInfoSettings)
                && mTelephonyManager.getPhoneType() != TelephonyManager.PHONE_TYPE_CDMA;

        if (!mShowLatestAreaInfo) {
            mDialog.removeSettingFromScreen(OPERATOR_INFO_LABEL_ID);
            mDialog.removeSettingFromScreen(OPERATOR_INFO_VALUE_ID);
        }
    }

    private void updateServiceState(ServiceState serviceState) {
        Log.d(TAG, "updateServiceState, serviceState=" + serviceState);
        final int state = Utils.getCombinedServiceState(serviceState);
        if (!Utils.isInService(serviceState)) {
            resetSignalStrength();
        }

        String serviceStateValue;

        switch (state) {
            case ServiceState.STATE_IN_SERVICE:
                serviceStateValue = mRes.getString(R.string.radioInfo_service_in);
                /// M: Add for ALPS03018051, update signal strength when entering IN_SERVICE state.
                updateSignalStrength(getSignalStrength());
                break;
            case ServiceState.STATE_OUT_OF_SERVICE:
            case ServiceState.STATE_EMERGENCY_ONLY:
                // Set summary string of service state to radioInfo_service_out when
                // service state is both STATE_OUT_OF_SERVICE & STATE_EMERGENCY_ONLY
                serviceStateValue = mRes.getString(R.string.radioInfo_service_out);
                break;
            case ServiceState.STATE_POWER_OFF:
                serviceStateValue = mRes.getString(R.string.radioInfo_service_off);
                break;
            default:
                serviceStateValue = mRes.getString(R.string.radioInfo_unknown);
                break;
        }

        mDialog.setText(SERVICE_STATE_VALUE_ID, serviceStateValue);

        /// M: Add for updating CDMA SIM status.
        mCdmaSimStatus.setServiceState(serviceState);
    }

    private void updateSignalStrength(SignalStrength signalStrength) {
        if (signalStrength == null) {
            return;
        }
        final int subscriptionId = mSubscriptionInfo.getSubscriptionId();
        final PersistableBundle carrierConfig =
                mCarrierConfigManager.getConfigForSubId(subscriptionId);
        // by default we show the signal strength
        boolean showSignalStrength = true;
        if (carrierConfig != null) {
            showSignalStrength = carrierConfig.getBoolean(
                    CarrierConfigManager.KEY_SHOW_SIGNAL_STRENGTH_IN_SIM_STATUS_BOOL);
        }
        if (!showSignalStrength) {
            mDialog.removeSettingFromScreen(SIGNAL_STRENGTH_LABEL_ID);
            mDialog.removeSettingFromScreen(SIGNAL_STRENGTH_VALUE_ID);
            return;
        }

        ServiceState serviceState = getCurrentServiceState();
        if (serviceState == null || !Utils.isInService(serviceState)) {
            return;
        }

        int signalDbm = getDbm(signalStrength);
        int signalAsu = getAsuLevel(signalStrength);

        if (signalDbm == -1) {
            signalDbm = 0;
        }

        if (signalAsu == -1) {
            signalAsu = 0;
        }

        mDialog.setText(SIGNAL_STRENGTH_VALUE_ID, mRes.getString(R.string.sim_signal_strength,
                signalDbm, signalAsu));

        /// M: Add for updating CDMA SIM status. @{
        mCdmaSimStatus.updateSignalStrength(signalStrength, SIGNAL_STRENGTH_VALUE_ID,
                mRes.getString(R.string.sim_signal_strength, signalDbm, signalAsu));
        /// @}
    }

    private void resetSignalStrength() {
        mDialog.setText(SIGNAL_STRENGTH_VALUE_ID, "0");
    }

    private void updateNetworkType() {
        // Whether EDGE, UMTS, etc...
        String dataNetworkTypeName = null;
        String voiceNetworkTypeName = null;
        final int subId = mSubscriptionInfo.getSubscriptionId();
        final int actualDataNetworkType = mTelephonyManager.getDataNetworkType(subId);
        final int actualVoiceNetworkType = mTelephonyManager.getVoiceNetworkType(subId);
        Log.d(TAG, "updateNetworkType, dataType=" + actualDataNetworkType
                + ", voiceType=" + actualVoiceNetworkType);

        if (TelephonyManager.NETWORK_TYPE_UNKNOWN != actualDataNetworkType) {
            dataNetworkTypeName = mTelephonyManager.getNetworkTypeName(actualDataNetworkType);
        }
        if (TelephonyManager.NETWORK_TYPE_UNKNOWN != actualVoiceNetworkType) {
            voiceNetworkTypeName = mTelephonyManager.getNetworkTypeName(actualVoiceNetworkType);
        }

        boolean show4GForLTE = false;
        final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(subId);
        if (carrierConfig != null) {
            show4GForLTE = carrierConfig.getBoolean(
                    CarrierConfigManager.KEY_SHOW_4G_FOR_LTE_DATA_ICON_BOOL);
        }

        if (show4GForLTE) {
            if ("LTE".equals(dataNetworkTypeName)) {
                dataNetworkTypeName = "4G";
            }
            if ("LTE".equals(voiceNetworkTypeName)) {
                voiceNetworkTypeName = "4G";
            }
        }

        /// M: Add for SIM settings plugin. @{
        dataNetworkTypeName = mMiscExt.getNetworktypeString(dataNetworkTypeName, subId);
        voiceNetworkTypeName = mMiscExt.getNetworktypeString(voiceNetworkTypeName, subId);
        /// @}

        mDialog.setText(CELL_VOICE_NETWORK_TYPE_VALUE_ID, voiceNetworkTypeName);
        mDialog.setText(CELL_DATA_NETWORK_TYPE_VALUE_ID, dataNetworkTypeName);
    }

    private void updateRoamingStatus(ServiceState serviceState) {
        if (serviceState.getRoaming()) {
            mDialog.setText(ROAMING_INFO_VALUE_ID, mRes.getString(R.string.radioInfo_roaming_in));
        } else {
            mDialog.setText(ROAMING_INFO_VALUE_ID, mRes.getString(R.string.radioInfo_roaming_not));
        }
    }

    private void updateIccidNumber() {
        final int subscriptionId = mSubscriptionInfo.getSubscriptionId();
        final PersistableBundle carrierConfig =
                mCarrierConfigManager.getConfigForSubId(subscriptionId);
        // do not show iccid by default
        boolean showIccId = false;
        if (carrierConfig != null) {
            showIccId = carrierConfig.getBoolean(
                    CarrierConfigManager.KEY_SHOW_ICCID_IN_SIM_STATUS_BOOL);
        }
        Log.i(TAG, "updateIccidNumber, showIccId=" + showIccId);
        if (!showIccId) {
            mDialog.removeSettingFromScreen(ICCID_INFO_LABEL_ID);
            mDialog.removeSettingFromScreen(ICCID_INFO_VALUE_ID);
        } else {
            /// M: Add ICCID setting on screen when it doesn't exist. @{
            if (!mDialog.isSettingOnScreen(ICCID_INFO_LABEL_ID)) {
                Log.i(TAG, "updateIccidNumber, add settings to screen.");
                mDialog.addSettingToScreen(ICCID_INFO_LABEL_ID);
                mDialog.addSettingToScreen(ICCID_INFO_VALUE_ID);
            }
            /// @}
            mDialog.setText(ICCID_INFO_VALUE_ID, getSimSerialNumber(subscriptionId));
        }
    }

    private void updateEid() {
        if (mEuiccManager.isEnabled()) {
            mDialog.setText(EID_INFO_VALUE_ID, mEuiccManager.getEid());
        } else {
            /// M: Add for updating EID label.
            mDialog.removeSettingFromScreen(EID_INFO_LABEL_ID);
            mDialog.removeSettingFromScreen(EID_INFO_VALUE_ID);
        }
    }

    private void updateImsRegistrationState() {
        final int subscriptionId = mSubscriptionInfo.getSubscriptionId();
        final PersistableBundle carrierConfig =
            mCarrierConfigManager.getConfigForSubId(subscriptionId);
        final boolean showImsRegState = carrierConfig == null ? false :
            carrierConfig.getBoolean(CarrierConfigManager.KEY_SHOW_IMS_REGISTRATION_STATUS_BOOL);
        if (showImsRegState) {
            final boolean isImsRegistered = mTelephonyManager.isImsRegistered(subscriptionId);
            mDialog.setText(IMS_REGISTRATION_STATE_VALUE_ID, mRes.getString(isImsRegistered ?
                R.string.ims_reg_status_registered : R.string.ims_reg_status_not_registered));
        } else {
            mDialog.removeSettingFromScreen(IMS_REGISTRATION_STATE_LABEL_ID);
            mDialog.removeSettingFromScreen(IMS_REGISTRATION_STATE_VALUE_ID);
        }
    }

    private SubscriptionInfo getPhoneSubscriptionInfo(int slotId) {
        return SubscriptionManager.from(mContext).getActiveSubscriptionInfoForSimSlotIndex(slotId);
    }

    @VisibleForTesting
    ServiceState getCurrentServiceState() {
        return mTelephonyManager.getServiceStateForSubscriber(
                mSubscriptionInfo.getSubscriptionId());
    }

    private int getDbm(SignalStrength signalStrength) {
        return signalStrength.getDbm();
    }

    private int getAsuLevel(SignalStrength signalStrength) {
        return signalStrength.getAsuLevel();
    }

    @VisibleForTesting
    PhoneStateListener getPhoneStateListener() {
        return new PhoneStateListener() {
            @Override
            public void onDataConnectionStateChanged(int state) {
                /// M: Add null check for getDialog. @{
                if (mDialog.getDialog() == null) {
                    Log.w(TAG, "DataConnectionStateChanged, dialog is null.");
                    return;
                }
                /// @}
                Log.d(TAG, "onDataConnectionStateChanged, state=" + state
                        + ", subInfo=" + mSubscriptionInfo);
                updateDataState(state);
                updateNetworkType();
            }

            @Override
            public void onSignalStrengthsChanged(SignalStrength signalStrength) {
                /// M: Add null check for getDialog. @{
                if (mDialog.getDialog() == null) {
                    Log.w(TAG, "onSignalStrengthsChanged, dialog is null.");
                    return;
                }
                /// @}
                Log.d(TAG, "onSignalStrengthsChanged, signalStrength=" + signalStrength);
                updateSignalStrength(signalStrength);
            }

            @Override
            public void onServiceStateChanged(ServiceState serviceState) {
                /// M: Add null check for getDialog. @{
                if (mDialog.getDialog() == null) {
                    Log.w(TAG, "onServiceStateChanged, dialog is null.");
                    return;
                }
                /// @}
                Log.d(TAG, "onServiceStateChanged, serviceState=" + serviceState);
                updateNetworkProvider();
                updateServiceState(serviceState);
                updateRoamingStatus(serviceState);
                /// M: Update network type.
                updateNetworkType();
                /// M: Update CDMA status.
                updateCdmaStatus(serviceState);
                /// M: Update ICCID number.
                updateIccidNumber();
            }
        };
    }

    @VisibleForTesting
    String getPhoneNumber() {
        return DeviceInfoUtils.getFormattedPhoneNumber(mContext, mSubscriptionInfo);
    }

    @VisibleForTesting
    SignalStrength getSignalStrength() {
        return mTelephonyManager.getSignalStrength();
    }

    @VisibleForTesting
    String getSimSerialNumber(int subscriptionId) {
        return mTelephonyManager.getSimSerialNumber(subscriptionId);
    }

    /// M: Add for updating CDMA SIM status. @{
    private void updateCdmaStatus(ServiceState serviceState) {
        int subId = mSubscriptionInfo.getSubscriptionId();
        int phoneType = mTelephonyManager.getCurrentPhoneType(subId);
        Log.d(TAG, "updateCdmaStatus, subId=" + subId + ", phoneType=" + phoneType);

        if (phoneType == TelephonyManager.PHONE_TYPE_CDMA) {
            // Update MCC and MNC
            updateMccMnc();
            // Update SID and NID
            updateSidNid(serviceState);
            // Update base ID
            updateBaseId();
        } else {
            removeCdmaStatus();
        }
    }

    private void removeCdmaStatus() {
        // Remove MCC/MNC, SID/NID and base ID
        mDialog.removeSettingFromScreen(MCC_MNC_INFO_LABEL_ID);
        mDialog.removeSettingFromScreen(MCC_MNC_INFO_VALUE_ID);
        mDialog.removeSettingFromScreen(SID_NID_INFO_LABEL_ID);
        mDialog.removeSettingFromScreen(SID_NID_INFO_VALUE_ID);
        mDialog.removeSettingFromScreen(BASE_ID_INFO_LABEL_ID);
        mDialog.removeSettingFromScreen(BASE_ID_INFO_VALUE_ID);
    }

    private void updateMccMnc() {
        int subId = mSubscriptionInfo.getSubscriptionId();
        /// M: Add for revising the SIM operator numeric.
        String numeric = TelephonyUtils.getSimOperatorNumeric(subId, mTelephonyManager);
        Log.d(TAG, "updateMccMnc, subId=" + subId + ", numeric=" + numeric);

        if (numeric.length() > MCC_LENGTH) {
            String mcc = numeric.substring(0, MCC_LENGTH);
            String mnc = numeric.substring(MCC_LENGTH);
            String mccmnc = mcc + "," + mnc;
            Log.d(TAG, "updateMccMnc, mccmnc=" + mccmnc);
            mDialog.setText(MCC_MNC_INFO_VALUE_ID, mccmnc);
        } else {
            Log.d(TAG, "updateMccMnc, numeric is too short.");
            mDialog.setText(MCC_MNC_INFO_VALUE_ID, null);
        }

        if (!mDialog.isSettingOnScreen(MCC_MNC_INFO_LABEL_ID)) {
            mDialog.addSettingToScreen(MCC_MNC_INFO_LABEL_ID);
            mDialog.addSettingToScreen(MCC_MNC_INFO_VALUE_ID);
        }
    }

    private void updateSidNid(ServiceState serviceState) {
        Log.d(TAG, "updateSidNid, serviceState=" + serviceState);
        int sid = serviceState.getCdmaSystemId();
        int nid = serviceState.getCdmaNetworkId();
        String sidnid = sid + "," + nid;
        Log.d(TAG, "updateSidNid, sidnid=" + sidnid);
        mDialog.setText(SID_NID_INFO_VALUE_ID, sidnid);

        if (!mDialog.isSettingOnScreen(SID_NID_INFO_LABEL_ID)) {
            mDialog.addSettingToScreen(SID_NID_INFO_LABEL_ID);
            mDialog.addSettingToScreen(SID_NID_INFO_VALUE_ID);
        }
    }

    private void updateBaseId() {
        CellLocation cellLocation = mTelephonyManager.getCellLocation();
        if (cellLocation instanceof CdmaCellLocation) {
            String baseId = Integer.toString(
                    ((CdmaCellLocation) cellLocation).getBaseStationId());
            Log.d(TAG, "updateBaseId, baseId=" + baseId);
            mDialog.setText(BASE_ID_INFO_VALUE_ID, baseId);
        } else {
            Log.d(TAG, "updateBaseId, not CDMA cell location.");
            mDialog.setText(BASE_ID_INFO_VALUE_ID, null);
        }

        if (!mDialog.isSettingOnScreen(BASE_ID_INFO_LABEL_ID)) {
            mDialog.addSettingToScreen(BASE_ID_INFO_LABEL_ID);
            mDialog.addSettingToScreen(BASE_ID_INFO_VALUE_ID);
        }
    }
    /// @}
}

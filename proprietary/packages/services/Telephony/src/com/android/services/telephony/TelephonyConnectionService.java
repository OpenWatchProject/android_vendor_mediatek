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

package com.android.services.telephony;

import android.annotation.NonNull;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.Looper;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telecom.Conference;
/// M: ALPS04811723. Don't call sort() for conferenceable list. @{
import android.telecom.Conferenceable;
/// @}
import android.telecom.Connection;
import android.telecom.ConnectionRequest;
import android.telecom.ConnectionService;
import android.telecom.DisconnectCause;
import android.telecom.ParcelableConnection;
import android.telecom.ParcelableConference;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.Logging.Session;
import android.telecom.TelecomManager;
import android.telecom.VideoProfile;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.RadioAccessFamily;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.emergency.EmergencyNumber;
import android.text.TextUtils;
import android.util.Pair;


import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.os.SomeArgs;
import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneSwitcher;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.imsphone.ImsExternalCallTracker;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.imsphone.ImsPhoneConnection;
import com.android.phone.MMIDialogActivity;
import com.android.phone.PhoneUtils;
import com.android.phone.R;

/// M: Self activation. @{
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.selfactivation.ISelfActivation;
/// @}

import com.mediatek.services.telephony.MtkGsmCdmaConnection;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.selfactivation.ISelfActivation;
import com.mediatek.services.telephony.MtkTelephonyConnectionServiceUtil;

import com.mediatek.telephony.MtkTelephonyManagerEx;

/// M: Mediatek import.
import com.mediatek.internal.telecom.IMtkConnectionService;
import com.mediatek.internal.telecom.IMtkConnectionServiceAdapter;

/// M: For ECC change feature @{
import com.mediatek.services.telephony.SwitchPhoneHelper;
/// @}

/// M: CC: [ALPS04214959] Smart Lock: not allow empty slot for ECC @{
import com.mediatek.internal.telephony.MtkIccCardConstants;
/// @}

/// M: CC: ECC for Fusion RIL @{
import com.android.internal.telephony.TelephonyDevController;
import com.mediatek.internal.telephony.MtkHardwareConfig;
/// @}
/// M: CC: Emergency mode for Fusion RIL @{
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
/// @}

import java.lang.ref.WeakReference;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

import javax.annotation.Nullable;

import mediatek.telecom.MtkConnection;
import mediatek.telecom.MtkTelecomManager;


// PhoneAccountHandle() print mId with Log.pii - VERBOSE
// Conference() has no sensitive log
// Connection() has no sensitive log
// TelephonyConnection() print address with Log.pii - VERBOSE
// ConnectionRequest print address with Connection.toLogSafePhoneNumber - DEBUG

import com.mediatek.services.telephony.EcbmCallHelper;
import com.mediatek.services.telephony.EcbmCallStateListener;


/*
 * Service for making GSM and CDMA connections.
 */
public class TelephonyConnectionService extends ConnectionService {

    private static final int MTK_MSG_BASE = 1000;
    /// M: CC: Interface for ECT
    private static final int MSG_ECT = MTK_MSG_BASE + 0;
    /// M: CC: HangupAll for FTA 31.4.4.2
    private static final int MSG_HANGUP_ALL = MTK_MSG_BASE + 1;
    /// M: CC: For MSMS/MSMA ordered user operations.
    private static final int MSG_HANDLE_ORDERED_USER_OPERATION = MTK_MSG_BASE + 2;

    /// M: VoLte
    private static final int MSG_INVITE_CONFERENCE_PARTICIPANTS = MTK_MSG_BASE + 3;
    private static final int MSG_CREATE_CONFERENCE = MTK_MSG_BASE + 4;
    private static final int MSG_BLIND_ASSURED_ECT = MTK_MSG_BASE + 5;
    private static final int MSG_DEVICE_SWITCH = MTK_MSG_BASE + 6;
    private static final int MSG_CANCEL_DEVICE_SWITCH = MTK_MSG_BASE + 7;

    /// M: CC: Softbank blacklist requirement
    private static final int MSG_REJECT_WITH_CAUSE = MTK_MSG_BASE + 8;

    // Timeout before we continue with the emergency call without waiting for DDS switch response
    // from the modem.
    private static final int DEFAULT_DATA_SWITCH_TIMEOUT_MS = 1000;

    // If configured, reject attempts to dial numbers matching this pattern.
    private static final Pattern CDMA_ACTIVATION_CODE_REGEX_PATTERN =
            Pattern.compile("\\*228[0-9]{0,2}");

    /// M: CC: ECC retry @{
    private SwitchPhoneHelper mSwitchPhoneHelper;
    /// @}

    private final TelephonyConnectionServiceProxy mTelephonyConnectionServiceProxy =
            new TelephonyConnectionServiceProxy() {
        @Override
        public Collection<Connection> getAllConnections() {
            return TelephonyConnectionService.this.getAllConnections();
        }
        @Override
        public void addConference(TelephonyConference mTelephonyConference) {
            TelephonyConnectionService.this.addConference(mTelephonyConference);
        }
        @Override
        public void addConference(ImsConference mImsConference) {
            TelephonyConnectionService.this.addConference(mImsConference);
        }
        @Override
        public void removeConnection(Connection connection) {
            TelephonyConnectionService.this.removeConnection(connection);
        }
        @Override
        public void addExistingConnection(PhoneAccountHandle phoneAccountHandle,
                                          Connection connection) {
            TelephonyConnectionService.this
                    .addExistingConnection(phoneAccountHandle, connection);
        }
        @Override
        public void addExistingConnection(PhoneAccountHandle phoneAccountHandle,
                Connection connection, Conference conference) {
            TelephonyConnectionService.this
                    .addExistingConnection(phoneAccountHandle, connection, conference);
        }
        @Override
        public void addConnectionToConferenceController(TelephonyConnection connection) {
            TelephonyConnectionService.this.addConnectionToConferenceController(connection);
        }

        @Override
        public void performImsConferenceSRVCC(Conference imsConf,
                ArrayList<com.android.internal.telephony.Connection> radioConnections,
                        String telecomCallId) {
            TelephonyConnectionService.this.performImsConferenceSRVCC(
                    imsConf, radioConnections, telecomCallId);
        }
    };

    private final Connection.Listener mConnectionListener = new Connection.Listener() {
        @Override
        public void onConferenceChanged(Connection connection, Conference conference) {
            mHoldTracker.updateHoldCapability(connection.getPhoneAccountHandle());
        }
    };

    private final BroadcastReceiver mTtyBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.v(this, "onReceive, action: %s", action);
            if (action.equals(TelecomManager.ACTION_TTY_PREFERRED_MODE_CHANGED)) {
                int newPreferredTtyMode = intent.getIntExtra(
                        TelecomManager.EXTRA_TTY_PREFERRED_MODE, TelecomManager.TTY_MODE_OFF);

                boolean isTtyNowEnabled = newPreferredTtyMode != TelecomManager.TTY_MODE_OFF;
                if (isTtyNowEnabled != mIsTtyEnabled) {
                    handleTtyModeChange(isTtyNowEnabled);
                }
            }
        }
    };

    private final TelephonyConferenceController mTelephonyConferenceController =
            new TelephonyConferenceController(mTelephonyConnectionServiceProxy);
    private final CdmaConferenceController mCdmaConferenceController =
            new CdmaConferenceController(this);
    private final ImsConferenceController mImsConferenceController =
            new ImsConferenceController(TelecomAccountRegistry.getInstance(this),
                    mTelephonyConnectionServiceProxy,
                    // FeatureFlagProxy; used to determine if standalone call emulation is enabled.
                    // TODO: Move to carrier config
                    () -> true);

    private ComponentName mExpectedComponentName = null;
    private RadioOnHelper mRadioOnHelper;
    private EmergencyTonePlayer mEmergencyTonePlayer;
    private HoldTracker mHoldTracker;
    private boolean mIsTtyEnabled;

    private EcbmCallHelper mEcbmCallHelper;

    // Contains one TelephonyConnection that has placed a call and a memory of which Phones it has
    // already tried to connect with. There should be only one TelephonyConnection trying to place a
    // call at one time. We also only access this cache from a TelephonyConnection that wishes to
    // redial, so we use a WeakReference that will become stale once the TelephonyConnection is
    // destroyed.
    @VisibleForTesting
    public Pair<WeakReference<TelephonyConnection>, Queue<Phone>> mEmergencyRetryCache;

    // M: CC: For MTK APK override
    private IMtkConnectionServiceAdapter mMtkAdapter = null;

    /**
     * Keeps track of the status of a SIM slot.
     */
    private static class SlotStatus {
        public int slotId;
        // RAT capabilities
        public int capabilities;
        // By default, we will assume that the slots are not locked.
        public boolean isLocked = false;
        // Is the emergency number associated with the slot
        public boolean hasDialedEmergencyNumber = false;
        //SimState
        public int simState;

        public SlotStatus(int slotId, int capabilities) {
            this.slotId = slotId;
            this.capabilities = capabilities;
        }
    }

    // SubscriptionManager Proxy interface for testing
    public interface SubscriptionManagerProxy {
        int getDefaultVoicePhoneId();
        int getSimStateForSlotIdx(int slotId);
        int getPhoneId(int subId);
    }

    private SubscriptionManagerProxy mSubscriptionManagerProxy = new SubscriptionManagerProxy() {
        @Override
        public int getDefaultVoicePhoneId() {
            return SubscriptionManager.getDefaultVoicePhoneId();
        }

        @Override
        public int getSimStateForSlotIdx(int slotId) {
            return SubscriptionManager.getSimStateForSlotIndex(slotId);
        }

        @Override
        public int getPhoneId(int subId) {
            return SubscriptionManager.getPhoneId(subId);
        }
    };

    // TelephonyManager Proxy interface for testing
    @VisibleForTesting
    public interface TelephonyManagerProxy {
        int getPhoneCount();
        boolean hasIccCard(int slotId);
        boolean isCurrentEmergencyNumber(String number);
        Map<Integer, List<EmergencyNumber>> getCurrentEmergencyNumberList();
    }

    private TelephonyManagerProxy mTelephonyManagerProxy;

    private class TelephonyManagerProxyImpl implements TelephonyManagerProxy {
        private final TelephonyManager mTelephonyManager;


        TelephonyManagerProxyImpl(Context context) {
            mTelephonyManager = new TelephonyManager(context);
        }

        @Override
        public int getPhoneCount() {
            return mTelephonyManager.getPhoneCount();
        }

        @Override
        public boolean hasIccCard(int slotId) {
            return mTelephonyManager.hasIccCard(slotId);
        }

        @Override
        public boolean isCurrentEmergencyNumber(String number) {
            return mTelephonyManager.isEmergencyNumber(number);
        }

        @Override
        public Map<Integer, List<EmergencyNumber>> getCurrentEmergencyNumberList() {
            return mTelephonyManager.getEmergencyNumberList();
        }
    }

    //PhoneFactory proxy interface for testing
    @VisibleForTesting
    public interface PhoneFactoryProxy {
        Phone getPhone(int index);
        Phone getDefaultPhone();
        Phone[] getPhones();
    }

    private PhoneFactoryProxy mPhoneFactoryProxy = new PhoneFactoryProxy() {
        @Override
        public Phone getPhone(int index) {
            return PhoneFactory.getPhone(index);
        }

        @Override
        public Phone getDefaultPhone() {
            return PhoneFactory.getDefaultPhone();
        }

        @Override
        public Phone[] getPhones() {
            return PhoneFactory.getPhones();
        }
    };

    @VisibleForTesting
    public void setSubscriptionManagerProxy(SubscriptionManagerProxy proxy) {
        mSubscriptionManagerProxy = proxy;
    }

    @VisibleForTesting
    public void setTelephonyManagerProxy(TelephonyManagerProxy proxy) {
        mTelephonyManagerProxy = proxy;
    }

    @VisibleForTesting
    public void setPhoneFactoryProxy(PhoneFactoryProxy proxy) {
        mPhoneFactoryProxy = proxy;
    }

    /// M: CC: ECC for Fusion RIL @{
    TelephonyDevController mTelDevController = TelephonyDevController.getInstance();
    private boolean hasC2kOverImsModem() {
        if (mTelDevController != null &&
                mTelDevController.getModem(0) != null &&
                ((MtkHardwareConfig) mTelDevController.getModem(0)).hasC2kOverImsModem() == true) {
                    return true;
        }
        return false;
    }
    /// @}

    /**
     * A listener to actionable events specific to the TelephonyConnection.
     */
    private final TelephonyConnection.TelephonyConnectionListener mTelephonyConnectionListener =
            new TelephonyConnection.TelephonyConnectionListener() {
        @Override
        public void onOriginalConnectionConfigured(TelephonyConnection c) {
            addConnectionToConferenceController(c);
        }

        @Override
        public void onOriginalConnectionRetry(TelephonyConnection c, boolean isPermanentFailure) {
            retryOutgoingOriginalConnection(c, isPermanentFailure);
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        Log.initLogging(this);
        setTelephonyManagerProxy(new TelephonyManagerProxyImpl(getApplicationContext()));
        mExpectedComponentName = new ComponentName(this, this.getClass());
        mEmergencyTonePlayer = new EmergencyTonePlayer(this);
        TelecomAccountRegistry.getInstance(this).setTelephonyConnectionService(this);
        mHoldTracker = new HoldTracker();
        mIsTtyEnabled = isTtyModeEnabled(getApplicationContext());

        IntentFilter intentFilter = new IntentFilter(
                TelecomManager.ACTION_TTY_PREFERRED_MODE_CHANGED);
        registerReceiver(mTtyBroadcastReceiver, intentFilter);

        /// M: CC: Use MtkTelephonyConnectionServiceUtil
        MtkTelephonyConnectionServiceUtil.getInstance().setService(this);
    }

    @Override
    public void onDestroy() {
        /// M: CC: Use MtkTelephonyConnectionServiceUtil
        MtkTelephonyConnectionServiceUtil.getInstance().unsetService();
        /// M: CC: Destroy CDMA conference controller.
        mCdmaConferenceController.onDestroy();
        /// M: CC: ECC switch phone for SVLTE @{
        if (mSwitchPhoneHelper != null) {
            mSwitchPhoneHelper.onDestroy();
        }
        /// @}
        /// M: CC: Cleanup all listeners to avoid callbacks after service destroyed. @{
        //[ALPS03629489]
        if (mRadioOnHelper != null) {
            mRadioOnHelper.cleanup();
        }
        /// @}
        super.onDestroy();
    }

    @Override
    public boolean onUnbind(Intent intent) {
        unregisterReceiver(mTtyBroadcastReceiver);
        return super.onUnbind(intent);
    }

    @Override
    public Connection onCreateOutgoingConnection(
            PhoneAccountHandle connectionManagerPhoneAccount,
            final ConnectionRequest request) {
        Log.i(this, "onCreateOutgoingConnection, request: " + request);

        Uri handle = request.getAddress();
        if (handle == null) {
            Log.d(this, "onCreateOutgoingConnection, handle is null");
            return Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.NO_PHONE_NUMBER_SUPPLIED,
                            "No phone number supplied"));
        }

        /// M: CC: [ALPS02340908] To avoid JE @{
        if (request.getAccountHandle() == null) {
            Log.d(this, "onCreateOutgoingConnection, PhoneAccountHandle is null");
            return Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.NO_PHONE_NUMBER_SUPPLIED,
                            "No phone number supplied"));
        }
        /// @}

        /// M: CC: ECC retry @{
        if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
            int phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
            try {
                phoneId = Integer.parseInt(request.getAccountHandle().getId());
            } catch (NumberFormatException e) {
                phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
            } finally {
                if (PhoneFactory.getPhone(phoneId) == null) {
                    // We don't stop ECC retry, because it's for ignoring normal call during ECC,
                    // the emergency call is still on going.
                    Log.i(this, "onCreateOutgoingConnection, phone is null, id=%d", phoneId);
                    return Connection.createFailedConnection(
                            DisconnectCauseUtil.toTelecomDisconnectCause(
                                    android.telephony.DisconnectCause.OUTGOING_FAILURE,
                                    "Phone is null"));
                }
            }
        }
        /// @}

        String scheme = handle.getScheme();
        String number;
        if (PhoneAccount.SCHEME_VOICEMAIL.equals(scheme)) {
            // TODO: We don't check for SecurityException here (requires
            // CALL_PRIVILEGED permission).
            final Phone phone = getPhoneForAccount(request.getAccountHandle(),
                    false /* isEmergencyCall */, null /* not an emergency call */);
            if (phone == null) {
                Log.d(this, "onCreateOutgoingConnection, phone is null");
                /// M: CC: ECC retry @{
                // [ALPS04034285] It may happen if user set an ECC number as voice mail number
                // [ALPS04723802] Set DisconnectCause to OUTGOING_FAILURE instead of OUT_OF_SERVICE
                if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                    Log.i(this, "ECC retry: clear ECC param");
                    MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
                    return Connection.createFailedConnection(
                            DisconnectCauseUtil.toTelecomDisconnectCause(
                                    android.telephony.DisconnectCause.OUTGOING_FAILURE,
                                    "Phone is null"));
                }
                /// @}
                return Connection.createFailedConnection(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                android.telephony.DisconnectCause.OUT_OF_SERVICE,
                                "Phone is null"));
            }
            number = phone.getVoiceMailNumber();
            if (TextUtils.isEmpty(number)) {
                Log.d(this, "onCreateOutgoingConnection, no voicemail number set.");
                return Connection.createFailedConnection(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                android.telephony.DisconnectCause.VOICEMAIL_NUMBER_MISSING,
                                "Voicemail scheme provided but no voicemail number set.",
                                phone.getPhoneId()));
            }

            // Convert voicemail: to tel:
            handle = Uri.fromParts(PhoneAccount.SCHEME_TEL, number, null);
        } else {
            /// M: [ALPS01906649] [ALPS03581193] For VoLTE, Allow SIP URI to be dialed out @{
            //if (!PhoneAccount.SCHEME_TEL.equals(scheme)){
            if (!PhoneAccount.SCHEME_TEL.equals(scheme) && !PhoneAccount.SCHEME_SIP.equals(scheme)){
            /// @}
                Log.d(this, "onCreateOutgoingConnection, Handle %s is not type tel", scheme);
                return Connection.createFailedConnection(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                android.telephony.DisconnectCause.INVALID_NUMBER,
                                "Handle scheme is not type tel"));
            }

            number = handle.getSchemeSpecificPart();
            if (TextUtils.isEmpty(number)) {
                Log.d(this, "onCreateOutgoingConnection, unable to parse number");
                return Connection.createFailedConnection(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                android.telephony.DisconnectCause.INVALID_NUMBER,
                                "Unable to parse number"));
            }

            /// M: CC: ECC retry @{
            //final Phone phone = getPhoneForAccount(request.getAccountHandle(),
            //        false /* isEmergencyCall*/, null /* not an emergency call */);
            Phone phone = null;
            if (!MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                phone = getPhoneForAccount(request.getAccountHandle(), false, null);
            }
            /// @}

            if (phone != null && CDMA_ACTIVATION_CODE_REGEX_PATTERN.matcher(number).matches()) {
                // Obtain the configuration for the outgoing phone's SIM. If the outgoing number
                // matches the *228 regex pattern, fail the call. This number is used for OTASP, and
                // when dialed could lock LTE SIMs to 3G if not prohibited..
                boolean disableActivation = false;
                CarrierConfigManager cfgManager = (CarrierConfigManager)
                        phone.getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
                if (cfgManager != null) {
                    disableActivation = cfgManager.getConfigForSubId(phone.getSubId())
                            .getBoolean(CarrierConfigManager.KEY_DISABLE_CDMA_ACTIVATION_CODE_BOOL);
                }

                if (disableActivation) {
                    return Connection.createFailedConnection(
                            DisconnectCauseUtil.toTelecomDisconnectCause(
                                    android.telephony.DisconnectCause
                                            .CDMA_ALREADY_ACTIVATED,
                                    "Tried to dial *228",
                                    phone.getPhoneId()));
                }
            }
        }

        final boolean isEmergencyNumber = mTelephonyManagerProxy.isCurrentEmergencyNumber(number);
        // Find out if this is a test emergency number
        final boolean isTestEmergencyNumber = isEmergencyNumberTestNumber(number);

        // Convert into emergency number if necessary
        // This is required in some regions (e.g. Taiwan).
        if (isEmergencyNumber) {
            final Phone phone = getPhoneForAccount(request.getAccountHandle(), false,
                    handle.getSchemeSpecificPart());
            // We only do the conversion if the phone is not in service. The un-converted
            // emergency numbers will go to the correct destination when the phone is in-service,
            // so they will only need the special emergency call setup when the phone is out of
            // service.
            if (phone == null || phone.getServiceState().getState()
                    != ServiceState.STATE_IN_SERVICE) {
                String convertedNumber = PhoneNumberUtils.convertToEmergencyNumber(this, number);
                if (!TextUtils.equals(convertedNumber, number)) {
                    Log.i(this, "onCreateOutgoingConnection, converted to emergency number");
                    number = convertedNumber;
                    handle = Uri.fromParts(PhoneAccount.SCHEME_TEL, number, null);
                }
            }
        }
        final String numberToDial = number;

        /// M: CC: ECC retry @{
        if (!isEmergencyNumber && MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
            Log.d(this, "ECC retry: clear ECC param due to SIM state/phone type change, not ECC");
            MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
            Log.d(this, "onCreateOutgoingConnection, phone is null");
            return Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.OUT_OF_SERVICE,
                            "Phone is null"));
        }
        /// @}

        /// M: CC: Emergency mode for Fusion RIL
        MtkTelephonyConnectionServiceUtil.getInstance().setEmergencyNumber(numberToDial);

        /// M: CC: ECC switch phone for SVLTE @{
        if (isEmergencyNumber) {
            Connection connection = switchPhoneIfNeeded(request, handle, numberToDial);
            if (connection != null) {
                return connection;
            }
        }
        /// @}

        final boolean isAirplaneModeOn = Settings.Global.getInt(getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) > 0;

        boolean needToTurnOnRadio = (isEmergencyNumber && (!isRadioOn() || isAirplaneModeOn))
                || isRadioPowerDownOnBluetooth();

        // M: CC: @{
        // AOSP trigger turn on radio for ECC when
        // 1. in airplane mode.
        // 2. any one of the phones is radio off (map to RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED)
        // Avoid timing issue between serviceState and radioState, check serviceState here
        if (isEmergencyNumber) {
            Phone phone = getPhoneForAccount(request.getAccountHandle(), true, numberToDial);
            needToTurnOnRadio |= !phone.isRadioOn()
                    || phone.getServiceState().getState() == ServiceState.STATE_POWER_OFF;
        }
        /// @}

        if (needToTurnOnRadio) {
            final Uri resultHandle = handle;
            // By default, Connection based on the default Phone, since we need to return to Telecom
            // now.
            final int originalPhoneType = PhoneFactory.getDefaultPhone().getPhoneType();
            final Connection resultConnection = getTelephonyConnection(request, numberToDial,
                    isEmergencyNumber, resultHandle, PhoneFactory.getDefaultPhone());

            /// M: CC: [ALPS03416213] Fix JE: Return the failed connection directly @{
            if (!(resultConnection instanceof TelephonyConnection)) {
                Log.i(this, "onCreateOutgoingConnection, create emergency connection failed");
                return resultConnection;
            }
            /// @}

            /// M: Self activation. @{
            Phone phone = getPhoneForAccount(request.getAccountHandle(),
                     isEmergencyNumber, numberToDial);
            if (phone instanceof MtkGsmCdmaPhone) {
                if (((MtkGsmCdmaPhone)phone).shouldProcessSelfActivation()) {
                    notifyEccToSelfActivationSM((MtkGsmCdmaPhone)phone);
                }
            }
            /// @}

            if (mRadioOnHelper == null) {
                mRadioOnHelper = new RadioOnHelper(this);
            }
            mRadioOnHelper.triggerRadioOnAndListen(new RadioOnStateListener.Callback() {
                @Override
                public void onComplete(RadioOnStateListener listener, boolean isRadioReady) {
                    handleOnComplete(isRadioReady, isEmergencyNumber, resultConnection, request,
                            numberToDial, resultHandle, originalPhoneType);
                }

                @Override
                public boolean isOkToCall(Phone phone, int serviceState) {
                    // HAL 1.4 introduced a new variant of dial for emergency calls, which includes
                    // an isTesting parameter. For HAL 1.4+, do not wait for IN_SERVICE, this will
                    // be handled at the RIL/vendor level by emergencyDial(...).
                    boolean waitForInServiceToDialEmergency = isTestEmergencyNumber
                            && phone.getHalVersion().less(RIL.RADIO_HAL_VERSION_1_4);
                    if (isEmergencyNumber && !waitForInServiceToDialEmergency) {
                        // We currently only look to make sure that the radio is on before dialing.
                        // We should be able to make emergency calls at any time after the radio has
                        // been powered on and isn't in the UNAVAILABLE state, even if it is
                        // reporting the OUT_OF_SERVICE state.
                        return (phone.getState() == PhoneConstants.State.OFFHOOK)
                            /// M: CC: [ALPS04527771] Check service state and radio state together{
                            // When radio state is unavailable during RILD resetting, service state
                            // is mapped to OUT_OF_SERVICE, not POWER_OFF.
                            // Happen with flight mode ECC if mtk_flight_mode_power_off_md =1.
                            //|| phone.getServiceState().getState() != ServiceState.STATE_POWER_OFF;
                            || (phone.getServiceState().getState() != ServiceState.STATE_POWER_OFF
                                    && phone.isRadioOn());
                            /// @}
                    } else {
                        // Wait until we are in service and ready to make calls. This can happen
                        // when we power down the radio on bluetooth to save power on watches or if
                        // it is a test emergency number and we have to wait for the device to move
                        // IN_SERVICE before the call can take place over normal routing.
                        return (phone.getState() == PhoneConstants.State.OFFHOOK)
                            || serviceState == ServiceState.STATE_IN_SERVICE;
                    }
                }
            });
            // Return the still unconnected GsmConnection and wait for the Radios to boot before
            // connecting it to the underlying Phone.
            return resultConnection;
        } else {
            if (!canAddCall() && !isEmergencyNumber) {
                Log.d(this, "onCreateOutgoingConnection, cannot add call .");
                return Connection.createFailedConnection(
                        new DisconnectCause(DisconnectCause.ERROR,
                                getApplicationContext().getText(
                                        R.string.incall_error_cannot_add_call),
                                getApplicationContext().getText(
                                        R.string.incall_error_cannot_add_call),
                                "Add call restricted due to ongoing video call"));
            }

            // Get the right phone object from the account data passed in.
            final Phone phone = getPhoneForAccount(request.getAccountHandle(), isEmergencyNumber,
                    /* Note: when not an emergency, handle can be null for unknown callers */
                    handle == null ? null : handle.getSchemeSpecificPart());
            if (!isEmergencyNumber) {
                final Connection resultConnection = getTelephonyConnection(request, numberToDial,
                        false, handle, phone);

                boolean bIsCdmaLess = (phone instanceof MtkGsmCdmaPhone &&
                                       ((MtkGsmCdmaPhone)phone).isCdmaLessDevice());
                boolean bInEcm = (!isEmergencyNumber && phone != null && phone.isInEcm());
                if (bIsCdmaLess && bInEcm) {
                    Log.d(this, "enableEcmbCalling");
                    if (mEcbmCallHelper == null) {
                        mEcbmCallHelper = new EcbmCallHelper(this);
                    }
                    mEcbmCallHelper.enableEcmbCalling(phone, new EcbmCallStateListener.Callback() {
                        @Override
                        public void onComplete(EcbmCallStateListener listener) {
                            if (resultConnection.getState() == Connection.STATE_DISCONNECTED) {
                                return;
                            }
                            placeOutgoingConnection(request, resultConnection, phone);
                        }
                    });
                    return resultConnection;
                } else {
                    return placeOutgoingConnection(request, resultConnection, phone);
                }
            } else {
                final Connection resultConnection = getTelephonyConnection(request, numberToDial,
                        true, handle, phone);
                /// M: CC: ECC retry @{
                if (!MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                    Log.d(this, "ECC retry: set param with Intial ECC.");
                    MtkTelephonyConnectionServiceUtil.getInstance().setEccRetryParams(
                            request,
                            phone.getPhoneId());
                }
                /// @}

                CompletableFuture<Boolean> phoneFuture = delayDialForDdsSwitch(phone);
                phoneFuture.whenComplete((result, error) -> {
                    if (error != null) {
                        Log.w(this, "onCreateOutgoingConn - delayDialForDdsSwitch exception= "
                                + error.getMessage());
                    }
                    Log.i(this, "onCreateOutgoingConn - delayDialForDdsSwitch result = " + result);
                    placeOutgoingConnection(request, resultConnection, phone);
                });
                return resultConnection;
            }
        }
    }

    private Connection placeOutgoingConnection(ConnectionRequest request,
            Connection resultConnection, Phone phone) {
        // If there was a failure, the resulting connection will not be a TelephonyConnection,
        // so don't place the call!
        if (resultConnection instanceof TelephonyConnection) {
            if (request.getExtras() != null && request.getExtras().getBoolean(
                    TelecomManager.EXTRA_USE_ASSISTED_DIALING, false)) {
                ((TelephonyConnection) resultConnection).setIsUsingAssistedDialing(true);
            }
            placeOutgoingConnection((TelephonyConnection) resultConnection, phone, request);
        }
        return resultConnection;
    }

    private boolean isEmergencyNumberTestNumber(String number) {
        number = PhoneNumberUtils.stripSeparators(number);
        Map<Integer, List<EmergencyNumber>> list =
                mTelephonyManagerProxy.getCurrentEmergencyNumberList();
        // Do not worry about which subscription the test emergency call is on yet, only detect that
        // it is an emergency.
        for (Integer sub : list.keySet()) {
            for (EmergencyNumber eNumber : list.get(sub)) {
                if (number.equals(eNumber.getNumber())
                        && eNumber.isFromSources(EmergencyNumber.EMERGENCY_NUMBER_SOURCE_TEST)) {
                    Log.i(this, "isEmergencyNumberTestNumber: " + number + " has been detected as "
                            + "a test emergency number.,");
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Whether the cellular radio is power off because the device is on Bluetooth.
     */
    private boolean isRadioPowerDownOnBluetooth() {
        final Context context = getApplicationContext();
        final boolean allowed = context.getResources().getBoolean(
                R.bool.config_allowRadioPowerDownOnBluetooth);
        final int cellOn = Settings.Global.getInt(context.getContentResolver(),
                Settings.Global.CELL_ON,
                PhoneConstants.CELL_ON_FLAG);
        return (allowed && cellOn == PhoneConstants.CELL_ON_FLAG && !isRadioOn());
    }

    /**
     * Handle the onComplete callback of RadioOnStateListener.
     */
    private void handleOnComplete(boolean isRadioReady, boolean isEmergencyNumber,
            Connection originalConnection, ConnectionRequest request, String numberToDial,
            Uri handle, int originalPhoneType) {

        /// M: CC: unset emergency mode @{
        int mainPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        Phone mainPhone = PhoneFactory.getPhone(mainPhoneId);
        /// @}

        // Make sure the Call has not already been canceled by the user.
        if (originalConnection.getState() == Connection.STATE_DISCONNECTED) {
            Log.i(this, "Call disconnected before the outgoing call was placed. Skipping call "
                    + "placement.");

            /// M: CC: unset emergency mode @{
            MtkTelephonyConnectionServiceUtil.getInstance()
                    .exitEmergencyMode(mainPhone, 1/*airplane*/);
            /// @}
            return;
        }
        // Get the right phone object since the radio has been turned on successfully.
        if (isRadioReady) {
            final Phone phone = getPhoneForAccount(request.getAccountHandle(), isEmergencyNumber,
                    /* Note: when not an emergency, handle can be null for unknown callers */
                    handle == null ? null : handle.getSchemeSpecificPart());

            /// M: CC: TDD data only @{
            if (MtkTelephonyConnectionServiceUtil.getInstance().
                    isDataOnlyMode(phone)) {
                Log.d(this, "enableEmergencyCalling, phoneId=" + phone.getPhoneId()
                        + " is in TDD data only mode.");

                /// M: CC: unset emergency mode @{
                MtkTelephonyConnectionServiceUtil.getInstance()
                        .exitEmergencyMode(mainPhone, 1/*airplane*/);
                /// @}

                // Assume only one ECC exists
                if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                    Log.d(this, "ECC retry: clear ECC param");
                    MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
                }
                originalConnection.setDisconnected(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                        android.telephony.DisconnectCause.OUTGOING_CANCELED, null));
                originalConnection.destroy();
                return;
            }
            /// @}

            /// M: CC: ECC retry @{
            if (!MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                Log.d(this, "ECC retry: set param with Intial ECC.");
                MtkTelephonyConnectionServiceUtil.getInstance().setEccRetryParams(
                        request, phone.getPhoneId());
            }
            /// @}

            if (!isEmergencyNumber) {
                adjustAndPlaceOutgoingConnection(phone, originalConnection, request, numberToDial,
                        handle, originalPhoneType, false);
            } else {
                delayDialForDdsSwitch(phone).whenComplete((result, error) -> {
                    if (error != null) {
                        Log.w(this, "handleOnComplete - delayDialForDdsSwitch exception= "
                                + error.getMessage());
                    }
                    Log.i(this, "handleOnComplete - delayDialForDdsSwitch result = " + result);
                    adjustAndPlaceOutgoingConnection(phone, originalConnection, request,
                            numberToDial, handle, originalPhoneType, true);
                });

            }
        } else {
            /// M: CC: unset emergency mode @{
            MtkTelephonyConnectionServiceUtil.getInstance()
                    .exitEmergencyMode(mainPhone, 1/*airplane*/);
            /// @}

            /// M: CC: ECC retry @{
            // Assume only one ECC exists. Don't trigger retry
            // since Modem fails to power on should be a bug
            if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                Log.d(this, "ECC retry: clear ECC param");
                MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
            }
            /// @}

            Log.w(this, "onCreateOutgoingConnection, failed to turn on radio");
            originalConnection.setDisconnected(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.POWER_OFF,
                            "Failed to turn on radio."));
            originalConnection.destroy();
        }
    }

    private void adjustAndPlaceOutgoingConnection(Phone phone, Connection connectionToEvaluate,
            ConnectionRequest request, String numberToDial, Uri handle, int originalPhoneType,
            boolean isEmergencyNumber) {
        // If the PhoneType of the Phone being used is different than the Default Phone, then we
        // need to create a new Connection using that PhoneType and replace it in Telecom.
        if (phone.getPhoneType() != originalPhoneType) {
            Connection repConnection = getTelephonyConnection(request, numberToDial,
                    isEmergencyNumber, handle, phone);

                /// M: CC: Modify the follow to handle the no sound issue. @{
                // 1. Add the new connection into Telecom;
                // 2. Disconnect the old connection;
                // 3. Place the new connection.
                if (repConnection instanceof TelephonyConnection) {
                    addExistingConnection(PhoneUtils.makePstnPhoneAccountHandle(phone),
                            repConnection);
                    // Reset the emergency call flag for destroying old connection.
                    resetTreatAsEmergencyCall(connectionToEvaluate);
                    connectionToEvaluate.setDisconnected(
                            DisconnectCauseUtil.toTelecomDisconnectCause(
                                    android.telephony.DisconnectCause.OUTGOING_CANCELED,
                                    "Reconnecting outgoing Emergency Call."));
                } else {
                    // Assume only one ECC exists
                    if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                        Log.d(this, "ECC retry: clear ECC param");
                        MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
                    }
                    connectionToEvaluate.setDisconnected(repConnection.getDisconnectCause());
                }
                connectionToEvaluate.destroy();
                /// @}
            // If there was a failure, the resulting connection will not be a TelephonyConnection,
            // so don't place the call, just return!
            if (repConnection instanceof TelephonyConnection) {
                placeOutgoingConnection((TelephonyConnection) repConnection, phone, request);
            }

                /// M: CC: Modify the follow to handle the no sound issue. @{
/*
            // Notify Telecom of the new Connection type.
            // TODO: Switch out the underlying connection instead of creating a new
            // one and causing UI Jank.
            boolean noActiveSimCard = SubscriptionController.getInstance()
                    .getActiveSubInfoCount(phone.getContext().getOpPackageName()) == 0;
            // If there's no active sim card and the device is in emergency mode, use E account.
            addExistingConnection(PhoneUtils.makePstnPhoneAccountHandleWithPrefix(
                    phone, "", isEmergencyNumber && noActiveSimCard), repConnection);
            // Remove the old connection from Telecom after.
            connectionToEvaluate.setDisconnected(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.OUTGOING_CANCELED,
                            "Reconnecting outgoing Emergency Call.",
                            phone.getPhoneId()));
            connectionToEvaluate.destroy();
*/
                /// @}
        } else {
            placeOutgoingConnection((TelephonyConnection) connectionToEvaluate, phone, request);
        }
    }


    /**
     * @return {@code true} if any other call is disabling the ability to add calls, {@code false}
     *      otherwise.
     */
    private boolean canAddCall() {
        Collection<Connection> connections = getAllConnections();
        for (Connection connection : connections) {
            if (connection.getExtras() != null &&
                    connection.getExtras().getBoolean(Connection.EXTRA_DISABLE_ADD_CALL, false)) {
                return false;
            }
        }
        return true;
    }

    /// M: CC: Set PhoneAccountHandle for ECC @{
    //[ALPS01794357]
    private PhoneAccountHandle makePstnPhoneAccountHandleForEcc(Phone phone) {
        PhoneAccountHandle phoneAccountHandle;
        String phoneIccId = phone.getFullIccSerialNumber();
        if (TextUtils.isEmpty(phoneIccId)) {
            // If No SIM is inserted, the corresponding IccId will be null,
            // take phoneId as PhoneAccountHandle::mId which is IccId originally
            phoneAccountHandle = PhoneUtils.makePstnPhoneAccountHandle(
                    Integer.toString(phone.getPhoneId()));
        } else {
            phoneAccountHandle = PhoneUtils.makePstnPhoneAccountHandle(phoneIccId);
        }
        Log.d(this, "Ecc PhoneAccountHandle mId: " + Log.pii(phoneAccountHandle.getId())
                + ", iccId: " + Log.pii(phoneIccId));
        return phoneAccountHandle;
    }
    /// @}

    private Connection getTelephonyConnection(final ConnectionRequest request, final String number,
            boolean isEmergencyNumber, final Uri handle, Phone phone) {

        if (phone == null) {
            final Context context = getApplicationContext();
            if (context.getResources().getBoolean(R.bool.config_checkSimStateBeforeOutgoingCall)) {
                // Check SIM card state before the outgoing call.
                // Start the SIM unlock activity if PIN_REQUIRED.
                final Phone defaultPhone = mPhoneFactoryProxy.getDefaultPhone();
                final IccCard icc = defaultPhone.getIccCard();
                IccCardConstants.State simState = IccCardConstants.State.UNKNOWN;
                if (icc != null) {
                    simState = icc.getState();
                }
                if (simState == IccCardConstants.State.PIN_REQUIRED) {
                    final String simUnlockUiPackage = context.getResources().getString(
                            R.string.config_simUnlockUiPackage);
                    final String simUnlockUiClass = context.getResources().getString(
                            R.string.config_simUnlockUiClass);
                    if (simUnlockUiPackage != null && simUnlockUiClass != null) {
                        Intent simUnlockIntent = new Intent().setComponent(new ComponentName(
                                simUnlockUiPackage, simUnlockUiClass));
                        simUnlockIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        try {
                            context.startActivity(simUnlockIntent);
                        } catch (ActivityNotFoundException exception) {
                            Log.e(this, exception, "Unable to find SIM unlock UI activity.");
                        }
                    }
                    return Connection.createFailedConnection(
                            DisconnectCauseUtil.toTelecomDisconnectCause(
                                    android.telephony.DisconnectCause.OUT_OF_SERVICE,
                                    "SIM_STATE_PIN_REQUIRED"));
                }
            }

            Log.d(this, "onCreateOutgoingConnection, phone is null");

            /// M: CC: Error message due to CellConnMgr checking @{
            Log.d(this, "onCreateOutgoingConnection, use default phone for cellConnMgr");
            if (MtkTelephonyConnectionServiceUtil.getInstance().
                    cellConnMgrShowAlerting(PhoneFactory.getDefaultPhone().getSubId())) {
                Log.d(this, "onCreateOutgoingConnection, cellConnMgrShowAlerting() check fail");
                return Connection.createFailedConnection(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                mediatek.telephony.MtkDisconnectCause.OUTGOING_CANCELED_BY_SERVICE,
                                "cellConnMgrShowAlerting() check fail"));
            }
            /// @}

            return Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.OUT_OF_SERVICE, "Phone is null"));
        }

        /// M: CC: Timing issue, radio maybe on even airplane mode on @{
        boolean isAirplaneModeOn = false;
        if (Settings.Global.getInt(phone.getContext().getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) > 0) {
            isAirplaneModeOn = true;
        }
        /// @}

        /// M: CC: TDD data only @{
        if (!isAirplaneModeOn
                && MtkTelephonyConnectionServiceUtil.getInstance().isDataOnlyMode(phone)) {
            /// M: CC: ECC retry @{
            // Assume only one ECC exists. Don't trigger retry
            // since Modem fails to power on should be a bug
            if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                Log.d(this, "4G data only, ECC retry: clear ECC param");
                MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
            }
            /// @}

            Log.d(this, "getTelephonyConnection, phoneId=" + phone.getPhoneId()
                    + " is in TDD data only mode.");
            return Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                    android.telephony.DisconnectCause.OUTGOING_CANCELED, null));
        }
        /// @}

        // Check both voice & data RAT to enable normal CS call,
        // when voice RAT is OOS but Data RAT is present.
        int state = phone.getServiceState().getState();
        if (state == ServiceState.STATE_OUT_OF_SERVICE) {
            int dataNetType = phone.getServiceState().getDataNetworkType();
            if (dataNetType == TelephonyManager.NETWORK_TYPE_LTE ||
                    dataNetType == TelephonyManager.NETWORK_TYPE_LTE_CA) {
                state = phone.getServiceState().getDataRegState();
            }
        }

        /// M : WFC <TO make MO call when WFC registered even radio off or out of service.> @{
        boolean isWfcEnabled = phone.isWifiCallingEnabled();
        Log.d(this, "WFC: phoneId: " + phone.getPhoneId() + " isWfcEnabled: " + isWfcEnabled
                + " isRadioOn: " + phone.isRadioOn());
        if (isWfcEnabled) {
            state = ServiceState.STATE_IN_SERVICE;
        }
        Log.d(this, "Service state:" + state + ", isAirplaneModeOn:" + isAirplaneModeOn);
        /// @}

        // If we're dialing a non-emergency number and the phone is in ECM mode, reject the call if
        // carrier configuration specifies that we cannot make non-emergency calls in ECM mode.
        Log.i(this, "isEmergencyNumber:" + isEmergencyNumber + ", phone.isInEcm():"
                + phone.isInEcm());
        if (!isEmergencyNumber && phone.isInEcm()) {
            boolean allowNonEmergencyCalls = true;
            CarrierConfigManager cfgManager = (CarrierConfigManager)
                    phone.getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
            if (cfgManager != null) {
                allowNonEmergencyCalls = cfgManager.getConfigForSubId(phone.getSubId())
                        .getBoolean(CarrierConfigManager.KEY_ALLOW_NON_EMERGENCY_CALLS_IN_ECM_BOOL);
                Log.i(this, "cfgManager != null");
            }

            Log.i(this, "allowNonEmergencyCalls:" + allowNonEmergencyCalls);
            // Because Sprint's C2K mccmnc config not set KEY_ALLOW_NON_EMERGENCY_CALLS_IN_ECM_BOOL
            // to false, We use op id to check and only work in OP package.
            if (allowNonEmergencyCalls &&
                    "OP20".equals(SystemProperties.get("persist.vendor.operator.optr", "OM"))) {
                allowNonEmergencyCalls = false;
            }
            if (!allowNonEmergencyCalls) {
                return Connection.createFailedConnection(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                android.telephony.DisconnectCause.CDMA_NOT_EMERGENCY,
                                "Cannot make non-emergency call in ECM mode.",
                                phone.getPhoneId()));
            }
        }

        if (!isEmergencyNumber) {
            /// M: SS: Error message due to VoLTE SS checking @{
            if (MtkTelephonyConnectionServiceUtil.getInstance().
                    shouldOpenDataConnection(number, phone)) {
                Log.d(this, "onCreateOutgoingConnection, shouldOpenDataConnection() check fail");
                return Connection.createFailedConnection(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                mediatek.telephony.MtkDisconnectCause.VOLTE_SS_DATA_OFF,
                                MtkTelecomManager.DISCONNECT_REASON_VOLTE_SS_DATA_OFF));
            }
            /// @}

            /// M: CC: Error message due to CellConnMgr checking @{
            if (isAirplaneModeOn && phone instanceof MtkGsmCdmaPhone &&
                    ((MtkGsmCdmaPhone)phone).shouldProcessSelfActivation()) {
                Log.d(this, "[Self-activation] Bypass Dial in flightmode.");
            /// M: CC: Error message due to CellConnMgr checking @{
            } else if (MtkTelephonyConnectionServiceUtil.getInstance().
                    cellConnMgrShowAlerting(phone.getSubId())) {
                Log.d(this, "onCreateOutgoingConnection, cellConnMgrShowAlerting() check fail");
                return Connection.createFailedConnection(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                mediatek.telephony.MtkDisconnectCause.OUTGOING_CANCELED_BY_SERVICE,
                                "cellConnMgrShowAlerting() check fail"));
            }
            /// @}

            switch (state) {
                case ServiceState.STATE_IN_SERVICE:
                case ServiceState.STATE_EMERGENCY_ONLY:
                    break;
                case ServiceState.STATE_OUT_OF_SERVICE:
                    if (phone.isUtEnabled() && number.endsWith("#")) {
                        Log.d(this, "onCreateOutgoingConnection dial for UT");
                        break;
                    } else {
                        /// M: CC: FTA requires call should be dialed out even out of service @{
                        if (SystemProperties.getInt("vendor.gsm.gcf.testmode", 0) == 2) {
                            break;
                        }
                        /// @}

                        /// M: CC: [ALPS04344405] MO even OOS for VzW CDMA-Less @{
                        if (phone instanceof MtkGsmCdmaPhone &&
                                ((MtkGsmCdmaPhone)phone).isCdmaLessDevice()) {
                            Log.d(this, "onCreateOutgoingConnection dial even OOS");
                            break;
                        }
                        /// @}

                        return Connection.createFailedConnection(
                                DisconnectCauseUtil.toTelecomDisconnectCause(
                                        android.telephony.DisconnectCause.OUT_OF_SERVICE,
                                        "ServiceState.STATE_OUT_OF_SERVICE",
                                        phone.getPhoneId()));
                    }
                case ServiceState.STATE_POWER_OFF:
                    // Don't disconnect if radio is power off because the device is on Bluetooth.
                    if (isRadioPowerDownOnBluetooth()) {
                        break;
                    }

                    /// M: self activation @{
                    if (phone instanceof MtkGsmCdmaPhone) {
                        if (((MtkGsmCdmaPhone)phone).shouldProcessSelfActivation()) {
                            Log.d(this, "POWER_OF and need to do self activation");
                            break;
                        }
                    }
                    /// @}

                    return Connection.createFailedConnection(
                            DisconnectCauseUtil.toTelecomDisconnectCause(
                                    android.telephony.DisconnectCause.POWER_OFF,
                                    "ServiceState.STATE_POWER_OFF",
                                    phone.getPhoneId()));
                default:
                    Log.d(this, "onCreateOutgoingConnection, unknown service state: %d", state);
                    return Connection.createFailedConnection(
                            DisconnectCauseUtil.toTelecomDisconnectCause(
                                    android.telephony.DisconnectCause.OUTGOING_FAILURE,
                                    "Unknown service state " + state,
                                    phone.getPhoneId()));
            }
        }

        final Context context = getApplicationContext();
        final boolean isTtyModeEnabled = isTtyModeEnabled(context);
        if (VideoProfile.isVideo(request.getVideoState()) && isTtyModeEnabled
                && !isEmergencyNumber) {
            return Connection.createFailedConnection(DisconnectCauseUtil.toTelecomDisconnectCause(
                    android.telephony.DisconnectCause.VIDEO_CALL_NOT_ALLOWED_WHILE_TTY_ENABLED,
                    null, phone.getPhoneId()));
        }

        // Check for additional limits on CDMA phones.
        final Connection failedConnection = checkAdditionalOutgoingCallLimits(phone);
        if (failedConnection != null) {
            return failedConnection;
        }

        // Check roaming status to see if we should block custom call forwarding codes
        if (blockCallForwardingNumberWhileRoaming(phone, number)) {
            return Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.DIALED_CALL_FORWARDING_WHILE_ROAMING,
                            "Call forwarding while roaming",
                            phone.getPhoneId()));
        }


        final TelephonyConnection connection =
                createConnectionFor(phone, null, true /* isOutgoing */, request.getAccountHandle(),
                        request.getTelecomCallId(), request.getAddress(), request.getVideoState());
        if (connection == null) {
            /// M: CC: ECC retry @{
            // Not trigger retry since connection is null should be a bug
            // Assume only one ECC exists
            if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                Log.d(this, "Fail to create connection, ECC retry: clear ECC param");
                MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
            }
            /// @}
            return Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.OUTGOING_FAILURE,
                            "Invalid phone type",
                            phone.getPhoneId()));
        }

        /// M: CC: ECC retry @{
        if (connection instanceof TelephonyConnection) {
            ((TelephonyConnection) connection).setEmergencyCall(isEmergencyNumber);
        }
        /// @}

        /// M: CC: Set PhoneAccountHandle for ECC @{
        //[ALPS01794357]
        if (isEmergencyNumber) {
            final PhoneAccountHandle phoneAccountHandle = makePstnPhoneAccountHandleForEcc(phone);
            connection.setAccountHandle(phoneAccountHandle);
        }
        /// @}

        connection.setAddress(handle, PhoneConstants.PRESENTATION_ALLOWED);
        connection.setInitializing();
        connection.setVideoState(request.getVideoState());
        connection.setRttTextStream(request.getRttTextStream());
        connection.setTtyEnabled(isTtyModeEnabled);
        return connection;
    }

    @Override
    public Connection onCreateIncomingConnection(
            PhoneAccountHandle connectionManagerPhoneAccount,
            ConnectionRequest request) {
        Log.i(this, "onCreateIncomingConnection, request: " + request);
        // If there is an incoming emergency CDMA Call (while the phone is in ECBM w/ No SIM),
        // make sure the PhoneAccount lookup retrieves the default Emergency Phone.
        PhoneAccountHandle accountHandle = request.getAccountHandle();
        boolean isEmergency = false;
        if (accountHandle != null && PhoneUtils.EMERGENCY_ACCOUNT_HANDLE_ID.equals(
                accountHandle.getId())) {
            Log.i(this, "Emergency PhoneAccountHandle is being used for incoming call... " +
                    "Treat as an Emergency Call.");
            isEmergency = true;
        }
        Phone phone = getPhoneForAccount(accountHandle, isEmergency,
                /* Note: when not an emergency, handle can be null for unknown callers */
                request.getAddress() == null ? null : request.getAddress().getSchemeSpecificPart());
        if (phone == null) {
            return Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.ERROR_UNSPECIFIED,
                            "Phone is null"));
        }

        Call call = phone.getRingingCall();
        if (!call.getState().isRinging()) {
            Log.i(this, "onCreateIncomingConnection, no ringing call");
            return Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.INCOMING_MISSED,
                            "Found no ringing call",
                            phone.getPhoneId()));
        }

        com.android.internal.telephony.Connection originalConnection =
                call.getState() == Call.State.WAITING ?
                    call.getLatestConnection() : call.getEarliestConnection();
        if (isOriginalConnectionKnown(originalConnection)) {
            Log.i(this, "onCreateIncomingConnection, original connection already registered");
            return Connection.createCanceledConnection();
        }

        // We should rely on the originalConnection to get the video state.  The request coming
        // from Telecom does not know the video state of the incoming call.
        int videoState = originalConnection != null ? originalConnection.getVideoState() :
                VideoProfile.STATE_AUDIO_ONLY;

        TelephonyConnection connection =
                createConnectionFor(phone, originalConnection, false /* isOutgoing */,
                        request.getAccountHandle(), request.getTelecomCallId(),
                        request.getAddress(), videoState);
        handleIncomingRtt(request, originalConnection);
        if (connection == null) {
            return Connection.createCanceledConnection();
        } else {
            return connection;
        }
    }

    private void handleIncomingRtt(ConnectionRequest request,
            com.android.internal.telephony.Connection originalConnection) {
        if (originalConnection == null
                || originalConnection.getPhoneType() != PhoneConstants.PHONE_TYPE_IMS) {
            if (request.isRequestingRtt()) {
                Log.w(this, "Requesting RTT on non-IMS call, ignoring");
            }
            return;
        }

        ImsPhoneConnection imsOriginalConnection = (ImsPhoneConnection) originalConnection;
        if (!request.isRequestingRtt()) {
            if (imsOriginalConnection.isRttEnabledForCall()) {
                Log.w(this, "Incoming call requested RTT but we did not get a RttTextStream");
            }
            return;
        }

        Log.i(this, "Setting RTT stream on ImsPhoneConnection in case we need it later");
        imsOriginalConnection.setCurrentRttTextStream(request.getRttTextStream());

        if (!imsOriginalConnection.isRttEnabledForCall()) {
            if (request.isRequestingRtt()) {
                Log.w(this, "Incoming call processed as RTT but did not come in as one. Ignoring");
            }
            return;
        }

        Log.i(this, "Setting the call to be answered with RTT on.");
        imsOriginalConnection.getImsCall().setAnswerWithRtt();
    }

    /**
     * Called by the {@link ConnectionService} when a newly created {@link Connection} has been
     * added to the {@link ConnectionService} and sent to Telecom.  Here it is safe to send
     * connection events.
     *
     * @param connection the {@link Connection}.
     */
    @Override
    public void onCreateConnectionComplete(Connection connection) {
        if (connection instanceof TelephonyConnection) {
            TelephonyConnection telephonyConnection = (TelephonyConnection) connection;
            maybeSendInternationalCallEvent(telephonyConnection);
        }
    }

    @Override
    public void triggerConferenceRecalculate() {
        if (mTelephonyConferenceController.shouldRecalculate()) {
            mTelephonyConferenceController.recalculate();
        }
    }

    @Override
    public Connection onCreateUnknownConnection(PhoneAccountHandle connectionManagerPhoneAccount,
            ConnectionRequest request) {
        Log.i(this, "onCreateUnknownConnection, request: " + request);
        // Use the registered emergency Phone if the PhoneAccountHandle is set to Telephony's
        // Emergency PhoneAccount
        PhoneAccountHandle accountHandle = request.getAccountHandle();
        boolean isEmergency = false;
        if (accountHandle != null && PhoneUtils.EMERGENCY_ACCOUNT_HANDLE_ID.equals(
                accountHandle.getId())) {
            Log.i(this, "Emergency PhoneAccountHandle is being used for unknown call... " +
                    "Treat as an Emergency Call.");
            isEmergency = true;
        }
        Phone phone = getPhoneForAccount(accountHandle, isEmergency,
                /* Note: when not an emergency, handle can be null for unknown callers */
                request.getAddress() == null ? null : request.getAddress().getSchemeSpecificPart());
        if (phone == null) {
            return Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.ERROR_UNSPECIFIED,
                            "Phone is null"));
        }
        Bundle extras = request.getExtras();

        final List<com.android.internal.telephony.Connection> allConnections = new ArrayList<>();

        // Handle the case where an unknown connection has an IMS external call ID specified; we can
        // skip the rest of the guesswork and just grad that unknown call now.
        if (phone.getImsPhone() != null && extras != null &&
                extras.containsKey(ImsExternalCallTracker.EXTRA_IMS_EXTERNAL_CALL_ID)) {

            ImsPhone imsPhone = (ImsPhone) phone.getImsPhone();
            ImsExternalCallTracker externalCallTracker = imsPhone.getExternalCallTracker();
            int externalCallId = extras.getInt(ImsExternalCallTracker.EXTRA_IMS_EXTERNAL_CALL_ID,
                    -1);

            if (externalCallTracker != null) {
                com.android.internal.telephony.Connection connection =
                        externalCallTracker.getConnectionById(externalCallId);

                if (connection != null) {
                    allConnections.add(connection);
                }
            }
        }

        if (allConnections.isEmpty()) {
            final Call ringingCall = phone.getRingingCall();
            if (ringingCall.hasConnections()) {
                allConnections.addAll(ringingCall.getConnections());
            }
            final Call foregroundCall = phone.getForegroundCall();
            if ((foregroundCall.getState() != Call.State.DISCONNECTED)
                    && (foregroundCall.hasConnections())) {
                allConnections.addAll(foregroundCall.getConnections());
            }
            if (phone.getImsPhone() != null) {
                final Call imsFgCall = phone.getImsPhone().getForegroundCall();
                if ((imsFgCall.getState() != Call.State.DISCONNECTED) && imsFgCall
                        .hasConnections()) {
                    allConnections.addAll(imsFgCall.getConnections());
                }
            }
            final Call backgroundCall = phone.getBackgroundCall();
            if (backgroundCall.hasConnections()) {
                allConnections.addAll(phone.getBackgroundCall().getConnections());
            }
        }

        com.android.internal.telephony.Connection unknownConnection = null;
        for (com.android.internal.telephony.Connection telephonyConnection : allConnections) {
            if (!isOriginalConnectionKnown(telephonyConnection)) {
                unknownConnection = telephonyConnection;
                Log.d(this, "onCreateUnknownConnection: conn = " + unknownConnection);
                break;
            }
        }

        if (unknownConnection == null) {
            Log.i(this, "onCreateUnknownConnection, did not find previously unknown connection.");
            return Connection.createCanceledConnection();
        }

        // We should rely on the originalConnection to get the video state.  The request coming
        // from Telecom does not know the video state of the unknown call.
        int videoState = unknownConnection != null ? unknownConnection.getVideoState() :
                VideoProfile.STATE_AUDIO_ONLY;

        TelephonyConnection connection =
                createConnectionFor(phone, unknownConnection,
                        !unknownConnection.isIncoming() /* isOutgoing */,
                        request.getAccountHandle(), request.getTelecomCallId(),
                        request.getAddress(), videoState);

        if (connection == null) {
            return Connection.createCanceledConnection();
        } else {
            connection.updateState();
            return connection;
        }
    }

    /**
     * Conferences two connections.
     *
     * Note: The {@link android.telecom.RemoteConnection#setConferenceableConnections(List)} API has
     * a limitation in that it can only specify conferenceables which are instances of
     * {@link android.telecom.RemoteConnection}.  In the case of an {@link ImsConference}, the
     * regular {@link Connection#setConferenceables(List)} API properly handles being able to merge
     * a {@link Conference} and a {@link Connection}.  As a result when, merging a
     * {@link android.telecom.RemoteConnection} into a {@link android.telecom.RemoteConference}
     * require merging a {@link ConferenceParticipantConnection} which is a child of the
     * {@link Conference} with a {@link TelephonyConnection}.  The
     * {@link ConferenceParticipantConnection} class does not have the capability to initiate a
     * conference merge, so we need to call
     * {@link TelephonyConnection#performConference(Connection)} on either {@code connection1} or
     * {@code connection2}, one of which is an instance of {@link TelephonyConnection}.
     *
     * @param connection1 A connection to merge into a conference call.
     * @param connection2 A connection to merge into a conference call.
     */
    @Override
    public void onConference(Connection connection1, Connection connection2) {
        if (connection1 instanceof TelephonyConnection) {
            ((TelephonyConnection) connection1).performConference(connection2);
        } else if (connection2 instanceof TelephonyConnection) {
            ((TelephonyConnection) connection2).performConference(connection1);
        } else {
            Log.w(this, "onConference - cannot merge connections " +
                    "Connection1: %s, Connection2: %2", connection1, connection2);
        }
    }

    @Override
    public void onConnectionAdded(Connection connection) {
        if (connection instanceof Holdable && !isExternalConnection(connection)) {
            connection.addConnectionListener(mConnectionListener);
            mHoldTracker.addHoldable(
                    connection.getPhoneAccountHandle(), (Holdable) connection);
        }
    }

    @Override
    public void onConnectionRemoved(Connection connection) {
        if (connection instanceof Holdable && !isExternalConnection(connection)) {
            mHoldTracker.removeHoldable(connection.getPhoneAccountHandle(), (Holdable) connection);
        }
    }

    @Override
    public void onConferenceAdded(Conference conference) {
        if (conference instanceof Holdable) {
            mHoldTracker.addHoldable(conference.getPhoneAccountHandle(), (Holdable) conference);
        }
    }

    @Override
    public void onConferenceRemoved(Conference conference) {
        if (conference instanceof Holdable) {
            mHoldTracker.removeHoldable(conference.getPhoneAccountHandle(), (Holdable) conference);
        }
    }

    private boolean isExternalConnection(Connection connection) {
        return (connection.getConnectionProperties() & Connection.PROPERTY_IS_EXTERNAL_CALL)
                == Connection.PROPERTY_IS_EXTERNAL_CALL;
    }

    private boolean blockCallForwardingNumberWhileRoaming(Phone phone, String number) {
        if (phone == null || TextUtils.isEmpty(number) || !phone.getServiceState().getRoaming()) {
            return false;
        }
        String[] blockPrefixes = null;
        CarrierConfigManager cfgManager = (CarrierConfigManager)
                phone.getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
        if (cfgManager != null) {
            blockPrefixes = cfgManager.getConfigForSubId(phone.getSubId()).getStringArray(
                    CarrierConfigManager.KEY_CALL_FORWARDING_BLOCKS_WHILE_ROAMING_STRING_ARRAY);
        }

        if (blockPrefixes != null) {
            for (String prefix : blockPrefixes) {
                if (number.startsWith(prefix)) {
                    return true;
                }
            }
        }
        return false;
    }

    private boolean isRadioOn() {
        boolean result = false;
        for (Phone phone : mPhoneFactoryProxy.getPhones()) {
            result |= phone.isRadioOn();
        }
        return result;
    }

    private Pair<WeakReference<TelephonyConnection>, Queue<Phone>> makeCachedConnectionPhonePair(
            TelephonyConnection c) {
        Queue<Phone> phones = new LinkedList<>(Arrays.asList(mPhoneFactoryProxy.getPhones()));
        return new Pair<>(new WeakReference<>(c), phones);
    }

    // Update the mEmergencyRetryCache by removing the Phone used to call the last failed emergency
    // number and then moving it to the back of the queue if it is not a permanent failure cause
    // from the modem.
    private void updateCachedConnectionPhonePair(TelephonyConnection c,
            boolean isPermanentFailure) {
        // No cache exists, create a new one.
        if (mEmergencyRetryCache == null) {
            Log.i(this, "updateCachedConnectionPhonePair, cache is null. Generating new cache");
            mEmergencyRetryCache = makeCachedConnectionPhonePair(c);
        // Cache is stale, create a new one with the new TelephonyConnection.
        } else if (mEmergencyRetryCache.first.get() != c) {
            Log.i(this, "updateCachedConnectionPhonePair, cache is stale. Regenerating.");
            mEmergencyRetryCache = makeCachedConnectionPhonePair(c);
        }

        Queue<Phone> cachedPhones = mEmergencyRetryCache.second;
        // Need to refer default phone considering ImsPhone because
        // cachedPhones is a list that contains default phones.
        Phone phoneUsed = c.getPhone().getDefaultPhone();
        if (phoneUsed == null) {
            return;
        }
        // Remove phone used from the list, but for temporary fail cause, it will be added
        // back to list further in this method. However in case of permanent failure, the
        // phone shouldn't be reused, hence it will not be added back again.
        cachedPhones.remove(phoneUsed);
        Log.i(this, "updateCachedConnectionPhonePair, isPermanentFailure:" + isPermanentFailure);
        if (!isPermanentFailure) {
            // In case of temporary failure, add the phone back, this will result adding it
            // to tail of list mEmergencyRetryCache.second, giving other phone more
            // priority and that is what we want.
            cachedPhones.offer(phoneUsed);
        }
    }

    /**
     * Updates a cache containing all of the slots that are available for redial at any point.
     *
     * - If a Connection returns with the disconnect cause EMERGENCY_TEMP_FAILURE, keep that phone
     * in the cache, but move it to the lowest priority in the list. Then, place the emergency call
     * on the next phone in the list.
     * - If a Connection returns with the disconnect cause EMERGENCY_PERM_FAILURE, remove that phone
     * from the cache and pull another phone from the cache to place the emergency call.
     *
     * This will continue until there are no more slots to dial on.
     */
    @VisibleForTesting
    public void retryOutgoingOriginalConnection(TelephonyConnection c, boolean isPermanentFailure) {
        int phoneId = (c.getPhone() == null) ? -1 : c.getPhone().getPhoneId();
        updateCachedConnectionPhonePair(c, isPermanentFailure);
        // Pull next phone to use from the cache or null if it is empty
        Phone newPhoneToUse = (mEmergencyRetryCache.second != null)
                ? mEmergencyRetryCache.second.peek() : null;
        if (newPhoneToUse != null) {
            int videoState = c.getVideoState();
            Bundle connExtras = c.getExtras();
            Log.i(this, "retryOutgoingOriginalConnection, redialing on Phone Id: " + newPhoneToUse);
            c.clearOriginalConnection();
            if (phoneId != newPhoneToUse.getPhoneId()) updatePhoneAccount(c, newPhoneToUse);
            placeOutgoingConnection(c, newPhoneToUse, videoState, connExtras);
        } else {
            // We have run out of Phones to use. Disconnect the call and destroy the connection.
            Log.i(this, "retryOutgoingOriginalConnection, no more Phones to use. Disconnecting.");
            c.setDisconnected(new DisconnectCause(DisconnectCause.ERROR));
            c.clearOriginalConnection();
            c.destroy();
        }
    }

    private void updatePhoneAccount(TelephonyConnection connection, Phone phone) {
        PhoneAccountHandle pHandle = PhoneUtils.makePstnPhoneAccountHandle(phone);
        // For ECall handling on MSIM, until the request reaches here (i.e PhoneApp), we don't know
        // on which phone account ECall can be placed. After deciding, we should notify Telecom of
        // the change so that the proper PhoneAccount can be displayed.
        Log.i(this, "updatePhoneAccount setPhoneAccountHandle, account = " + pHandle);
        connection.setPhoneAccountHandle(pHandle);
    }

    private void placeOutgoingConnection(
            TelephonyConnection connection, Phone phone, ConnectionRequest request) {
        placeOutgoingConnection(connection, phone, request.getVideoState(), request.getExtras());
    }

    private void placeOutgoingConnection(
            TelephonyConnection connection, Phone phone, int videoState, Bundle extras) {
        String number = connection.getAddress().getSchemeSpecificPart();

        /// M: CC: Set PhoneAccountHandle for ECC @{
        //[ALPS01794357]
        final boolean isEmergencyNumber = mTelephonyManagerProxy.isCurrentEmergencyNumber(number);
        if (isEmergencyNumber) {
            final PhoneAccountHandle phoneAccountHandle = makePstnPhoneAccountHandleForEcc(phone);
            Log.d(this, "placeOutgoingConnection, set back account mId: "
                    + Log.pii(phoneAccountHandle.getId()));
            connection.setPhoneAccountHandle(phoneAccountHandle);
            // Need to set current ECC phone type which will be used when retry to check if
            // need to switch phone or not.
            MtkTelephonyConnectionServiceUtil.getInstance().setEccPhoneType(phone.getPhoneType());
        }
        /// @}

        /// M: CC: unset emergency mode @{
        int mainPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        Phone mainPhone = PhoneFactory.getPhone(mainPhoneId);
        /// @}

        com.android.internal.telephony.Connection originalConnection = null;
        try {
            if (phone != null) {
                /// M: CC: Vzw requirement to turn off Wifi hotspot before ECC dial. @{
                if (isEmergencyNumber && SystemProperties.get("persist.vendor.operator.optr", "OM")
                        .equals("OP12")) {
                    Context context = getApplicationContext();
                    if (context != null) {
                        ConnectivityManager cm = (ConnectivityManager) context
                                .getSystemService(Context.CONNECTIVITY_SERVICE);
                        if (cm != null) {
                            Log.i(this, "placeOutgoingConnection, stop hotspot before ECC dial");
                            cm.stopTethering(ConnectivityManager.TETHERING_WIFI);
                        } else {
                            Log.i(this,
                                    "placeOutgoingConnection, cannot stop wifi-hotspot (CM service null)");
                        }
                    } else {
                        Log.i(this,
                                "placeOutgoingConnection, cannot stop wifi-hotspot (context null)");
                    }
                }
                ///@}

                originalConnection = phone.dial(number, new ImsPhone.ImsDialArgs.Builder()
                        .setVideoState(videoState)
                        .setIntentExtras(extras)
                        .setRttTextStream(connection.getRttTextStream())
                        .build());
            }
        } catch (CallStateException e) {
            Log.e(this, e, "placeOutgoingConnection, phone.dial exception: " + e);
            int cause = android.telephony.DisconnectCause.OUTGOING_FAILURE;
            switch (e.getError()) {
                case CallStateException.ERROR_OUT_OF_SERVICE:
                    cause = android.telephony.DisconnectCause.OUT_OF_SERVICE;
                    break;
                case CallStateException.ERROR_POWER_OFF:
                    cause = android.telephony.DisconnectCause.POWER_OFF;
                    break;
                case CallStateException.ERROR_ALREADY_DIALING:
                    cause = android.telephony.DisconnectCause.ALREADY_DIALING;
                    break;
                case CallStateException.ERROR_CALL_RINGING:
                    cause = android.telephony.DisconnectCause.CANT_CALL_WHILE_RINGING;
                    break;
                case CallStateException.ERROR_CALLING_DISABLED:
                    cause = android.telephony.DisconnectCause.CALLING_DISABLED;
                    break;
                case CallStateException.ERROR_TOO_MANY_CALLS:
                    cause = android.telephony.DisconnectCause.TOO_MANY_ONGOING_CALLS;
                    break;
                case CallStateException.ERROR_OTASP_PROVISIONING_IN_PROCESS:
                    cause = android.telephony.DisconnectCause.OTASP_PROVISIONING_IN_PROCESS;
                    break;
            }
            /// M: CC: unset emergency mode @{
            if (MtkTelephonyConnectionServiceUtil.getInstance().isInEccMode()) {
                MtkTelephonyConnectionServiceUtil.getInstance()
                    .exitEmergencyMode(mainPhone, 1/*airplane*/);
            }
            /// @}

            /// M: CC: ECC retry @{
            // Assume only one ECC exists
            if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                Log.d(this, "ECC retry: clear ECC param");
                MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
            }
            /// @}
            connection.setDisconnected(DisconnectCauseUtil.toTelecomDisconnectCause(
                    cause, e.getMessage(), phone.getPhoneId()));
            connection.clearOriginalConnection();
            connection.destroy();
            return;
        }

        if (originalConnection == null) {
            int telephonyDisconnectCause = android.telephony.DisconnectCause.OUTGOING_FAILURE;
            // On GSM phones, null connection means that we dialed an MMI code
            /// M: ALPS04848544, hVoLTE phone type will be CDMA.
            // The IMS call state may be DISCONNECTING when MMI code is 0 or 1.
            // if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
            if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM ||
                    isDialedMmiCode(phone)) {
                Log.d(this, "dialed MMI code");
                int subId = phone.getSubId();
                Log.d(this, "subId: "+subId);
                telephonyDisconnectCause = android.telephony.DisconnectCause.DIALED_MMI;
                final Intent intent = new Intent(this, MMIDialogActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
                        Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                if (SubscriptionManager.isValidSubscriptionId(subId)) {
                    intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, subId);
                }
                startActivity(intent);
            }
            Log.d(this, "placeOutgoingConnection, phone.dial returned null");

            /// M: CC: unset emergency mode @{
            if (MtkTelephonyConnectionServiceUtil.getInstance().isInEccMode()) {
                MtkTelephonyConnectionServiceUtil.getInstance()
                    .exitEmergencyMode(mainPhone, 1/*airplane*/);
            }
            /// @}

            /// M: CC: ECC retry @{
            // Assume only one ECC exists
            if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                Log.d(this, "ECC retry: clear ECC param");
                MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
            }
            /// @}
            connection.setDisconnected(DisconnectCauseUtil.toTelecomDisconnectCause(
                    telephonyDisconnectCause, "Connection is null", phone.getPhoneId()));
            connection.clearOriginalConnection();
            connection.destroy();
        } else {
            connection.setOriginalConnection(originalConnection);
        }
    }

    /// M: IMS CC @{
    private boolean isDialedMmiCode(Phone phone) {
        ImsPhone imsPhone = (ImsPhone) phone.getImsPhone();

        if (imsPhone == null) {
            return false;
        }

        if (imsPhone.isInCall()) {
            return true;
        }

        if (imsPhone.getForegroundCall().getState() == Call.State.DISCONNECTING ||
                imsPhone.getBackgroundCall().getState() == Call.State.DISCONNECTING ||
                imsPhone.getRingingCall().getState() == Call.State.DISCONNECTING) {
            return true;
        }

        return false;
    }
    /// @}

    /// M: CC: ECC retry @{
    /**
     * createConnection for ECC retry
     *
     * @param callId The call Id.
     * @param request The connection request.
     */
    public void createConnectionInternal(
            final String callId,
            final ConnectionRequest request) {
        Log.v(this, "createConnectionInternal, callId=" + callId + ", request=" + request);

        Connection connection = onCreateOutgoingConnection(null, request);
        Log.v(this, "createConnectionInternal, connection=", connection);
        if (connection == null) {
            connection = Connection.createFailedConnection(
                    new DisconnectCause(DisconnectCause.ERROR));
        }

        /// M: CC: Set PhoneAccountHandle for ECC @{
        //[ALPS01794357]
        PhoneAccountHandle handle = null;
        if (connection instanceof TelephonyConnection) {
            handle = ((TelephonyConnection) connection).getAccountHandle();
        }
        if (handle == null) {
            handle = request.getAccountHandle();
        } else {
            Log.i(this, "createConnectionInternal, set back phone account=" + handle);
        }
        //// @}

        connection.setTelecomCallId(callId);
        if (connection.getState() != Connection.STATE_DISCONNECTED) {
            addConnection(handle, callId, connection);
        }

        Uri address = connection.getAddress();
        String number = address == null ? "null" : address.getSchemeSpecificPart();
        Log.v(this, "createConnectionInternal"
                + ", number=" + Connection.toLogSafePhoneNumber(number)
                + ", state=" + Connection.stateToString(connection.getState())
                + ", capabilities="
                + MtkConnection.capabilitiesToString(connection.getConnectionCapabilities())
                + ", properties="
                + MtkConnection.propertiesToString(connection.getConnectionProperties()));

        Log.d(this, "createConnectionInternal, calling handleCreateConnectionComplete"
                + " for callId=" + callId);
        mAdapter.handleCreateConnectionComplete(
                callId,
                request,
                new ParcelableConnection(
                        handle,  /* M: CC: Set PhoneAccountHandle for ECC [ALPS01794357] */
                        connection.getState(),
                        connection.getConnectionCapabilities(),
                        connection.getConnectionProperties(),
                        connection.getSupportedAudioRoutes(),
                        connection.getAddress(),
                        connection.getAddressPresentation(),
                        connection.getCallerDisplayName(),
                        connection.getCallerDisplayNamePresentation(),
                        connection.getVideoProvider() == null ?
                                null : connection.getVideoProvider().getInterface(),
                        connection.getVideoState(),
                        connection.isRingbackRequested(),
                        connection.getAudioModeIsVoip(),
                        connection.getConnectTimeMillis(),
                        connection.getConnectElapsedTimeMillis(),
                        connection.getStatusHints(),
                        connection.getDisconnectCause(),
                        createIdList(connection.getConferenceables()),
                        connection.getExtras()));
    }

    /**
     * Remove Connection without removing callId from Telecom
     *
     * @param connection The connection.
     * @return String The callId mapped to the removed connection.
     * @hide
     */
    protected String removeConnectionInternal(Connection connection) {
        String id = mIdByConnection.get(connection);
        connection.unsetConnectionService(this);
        // M: Since TelephonyConnectionService has declared as self private connection listener
        // So here is to remove parent's listener
        connection.removeConnectionListener(super.mConnectionListener);
        mConnectionById.remove(mIdByConnection.get(connection));
        mIdByConnection.remove(connection);
        onConnectionRemoved(connection);  // [ALPS04034285]
        Log.d(this, "removeConnectionInternal, callId=" + id + ", connection=" + connection);
        return id;
    }
    /// @}

    private TelephonyConnection createConnectionFor(
            Phone phone,
            com.android.internal.telephony.Connection originalConnection,
            boolean isOutgoing,
            PhoneAccountHandle phoneAccountHandle,
            String telecomCallId,
            Uri address,
            int videoState) {
        TelephonyConnection returnConnection = null;
        int phoneType = phone.getPhoneType();
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        /*
        if (phoneType == TelephonyManager.PHONE_TYPE_GSM) {
            returnConnection = new GsmConnection(originalConnection, telecomCallId, isOutgoing);
        } else if (phoneType == TelephonyManager.PHONE_TYPE_CDMA) {
            boolean allowsMute = allowsMute(phone);
            returnConnection = new CdmaConnection(originalConnection, mEmergencyTonePlayer,
                    allowsMute, isOutgoing, telecomCallId);
        }
        */
        boolean allowsMute = allowsMute(phone);
        returnConnection = new MtkGsmCdmaConnection(phoneType, originalConnection, telecomCallId,
                mEmergencyTonePlayer, allowsMute, isOutgoing);
        /// @}
        if (returnConnection != null) {
            // Listen to Telephony specific callbacks from the connection
            returnConnection.addTelephonyConnectionListener(mTelephonyConnectionListener);
            returnConnection.setVideoPauseSupported(
                    TelecomAccountRegistry.getInstance(this).isVideoPauseSupported(
                            phoneAccountHandle));
            returnConnection.setManageImsConferenceCallSupported(
                    TelecomAccountRegistry.getInstance(this).isManageImsConferenceCallSupported(
                            phoneAccountHandle));
            returnConnection.setShowPreciseFailedCause(
                    TelecomAccountRegistry.getInstance(this).isShowPreciseFailedCause(
                            phoneAccountHandle));
        }
        return returnConnection;
    }

    private boolean isOriginalConnectionKnown(
            com.android.internal.telephony.Connection originalConnection) {
        for (Connection connection : getAllConnections()) {
            if (connection instanceof TelephonyConnection) {
                TelephonyConnection telephonyConnection = (TelephonyConnection) connection;
                if (telephonyConnection.getOriginalConnection() == originalConnection) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Determines which {@link Phone} will be used to place the call.
     * @param accountHandle The {@link PhoneAccountHandle} which was sent from Telecom to place the
     *      call on.
     * @param isEmergency {@code true} if this is an emergency call, {@code false} otherwise.
     * @param emergencyNumberAddress When {@code isEmergency} is {@code true}, will be the phone
     *      of the emergency call.  Otherwise, this can be {@code null}  .
     * @return
     */
    private Phone getPhoneForAccount(PhoneAccountHandle accountHandle, boolean isEmergency,
                                     @Nullable String emergencyNumberAddress) {
        Phone chosenPhone = null;
        int subId = PhoneUtils.getSubIdForPhoneAccountHandle(accountHandle);
        if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            int phoneId = mSubscriptionManagerProxy.getPhoneId(subId);
            chosenPhone = mPhoneFactoryProxy.getPhone(phoneId);
        }
        // If this is an emergency call and the phone we originally planned to make this call
        // with is not in service or was invalid, try to find one that is in service, using the
        // default as a last chance backup.
        if (isEmergency && (chosenPhone == null || !isAvailableForEmergencyCalls(chosenPhone))) {
            Log.d(this, "getPhoneForAccount: phone for phone acct handle %s is out of service "
                    + "or invalid for emergency call.", accountHandle);
            chosenPhone = getPhoneForEmergencyCall(emergencyNumberAddress);
            Log.d(this, "getPhoneForAccount: using subId: " +
                    (chosenPhone == null ? "null" : chosenPhone.getSubId()));
        }
        /// M: CC: ECC phone selection rule @{
        if (isEmergency) {
            chosenPhone = MtkTelephonyConnectionServiceUtil.getInstance()
                    .selectPhoneBySpecialEccRule(accountHandle, emergencyNumberAddress,
                            chosenPhone);
        }
        /// @}
        return chosenPhone;
    }

    private CompletableFuture<Boolean> delayDialForDdsSwitch(Phone phone) {
        if (phone == null) {
            return CompletableFuture.completedFuture(Boolean.TRUE);
        }
        return possiblyOverrideDefaultDataForEmergencyCall(phone)
                .completeOnTimeout(false, DEFAULT_DATA_SWITCH_TIMEOUT_MS,
                        TimeUnit.MILLISECONDS);
    }

    /**
     * If needed, block until Default Data subscription is switched for outgoing emergency call.
     *
     * In some cases, we need to try to switch the Default Data subscription before placing the
     * emergency call on DSDS devices. This includes the following situation:
     * - The modem does not support processing GNSS SUPL requests on the non-default data
     * subscription. For some carriers that do not provide a control plane fallback mechanism, the
     * SUPL request will be dropped and we will not be able to get the user's location for the
     * emergency call. In this case, we need to swap default data temporarily.
     * @param phone Evaluates whether or not the default data should be moved to the phone
     *              specified. Should not be null.
     */
    private CompletableFuture<Boolean> possiblyOverrideDefaultDataForEmergencyCall(
            @NonNull Phone phone) {
        TelephonyManager telephony = TelephonyManager.from(phone.getContext());
        int phoneCount = telephony.getPhoneCount();
        // Do not override DDS if this is a single SIM device.
        if (phoneCount <= PhoneConstants.MAX_PHONE_COUNT_SINGLE_SIM) {
            return CompletableFuture.completedFuture(Boolean.TRUE);
        }

        CarrierConfigManager cfgManager = (CarrierConfigManager)
                phone.getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
        if (cfgManager == null) {
            // For some reason CarrierConfigManager is unavailable. Do not block emergency call.
            Log.w(this, "possiblyOverrideDefaultDataForEmergencyCall: couldn't get"
                    + "CarrierConfigManager");
            return CompletableFuture.completedFuture(Boolean.TRUE);
        }
        // Only override default data if we are IN_SERVICE and on a home network. We don't want to
        // perform a DDS switch of we are on a roaming network, where SUPL may not be available.
        boolean isPhoneAvailableForEmergency = isAvailableForEmergencyCalls(phone);
        boolean isRoaming = phone.getServiceState().getVoiceRoaming();
        if (!isPhoneAvailableForEmergency || isRoaming) {
            Log.d(this, "possiblyOverrideDefaultDataForEmergencyCall: not switching DDS, avail = "
                    + isPhoneAvailableForEmergency + ", roaming = " + isRoaming);
            return CompletableFuture.completedFuture(Boolean.TRUE);
        }

        // Do not switch Default data if this device supports emergency SUPL on non-DDS.
        final boolean gnssSuplRequiresDefaultData = phone.getContext().getResources().getBoolean(
                R.bool.config_gnss_supl_requires_default_data_for_emergency);
        if (!gnssSuplRequiresDefaultData) {
            Log.d(this, "possiblyOverrideDefaultDataForEmergencyCall: not switching DDS, does not "
                    + "require DDS switch.");
            return CompletableFuture.completedFuture(Boolean.TRUE);
        }

        final boolean supportsCpFallback = cfgManager.getConfigForSubId(phone.getSubId())
                .getInt(CarrierConfigManager.Gps.KEY_ES_SUPL_CONTROL_PLANE_SUPPORT_INT,
                        CarrierConfigManager.Gps.SUPL_EMERGENCY_MODE_TYPE_CP_ONLY)
                != CarrierConfigManager.Gps.SUPL_EMERGENCY_MODE_TYPE_DP_ONLY;
        if (supportsCpFallback) {
            Log.d(this, "possiblyOverrideDefaultDataForEmergencyCall: not switching DDS, carrier "
                    + "supports CP fallback.");
            // Do not try to swap default data if we support CS fallback, do not want to introduce
            // a lag in emergency call setup time if possible.
            return CompletableFuture.completedFuture(Boolean.TRUE);
        }

        // Get extension time, may be 0 for some carriers that support ECBM as well. Use
        // CarrierConfig default if format fails.
        int extensionTime = 0;
        try {
            extensionTime = Integer.valueOf(cfgManager.getConfigForSubId(phone.getSubId())
                    .getString(CarrierConfigManager.Gps.KEY_ES_EXTENSION_SEC_STRING, "0"));
        } catch (NumberFormatException e) {
            // Just use default.
        }
        CompletableFuture<Boolean> modemResultFuture = new CompletableFuture<>();
        try {
            Log.d(this, "possiblyOverrideDefaultDataForEmergencyCall: overriding DDS for "
                    + extensionTime + "seconds");
            PhoneSwitcher.getInstance().overrideDefaultDataForEmergency(phone.getPhoneId(),
                    extensionTime, modemResultFuture);
            // Catch all exceptions, we want to continue with emergency call if possible.
        } catch (Exception e) {
            Log.w(this, "possiblyOverrideDefaultDataForEmergencyCall: exception = "
                    + e.getMessage());
        }
        return modemResultFuture;
    }

    /**
     * Get the Phone to use for an emergency call of the given emergency number address:
     *  a) If there are multiple Phones with the Subscriptions that support the emergency number
     *     address, and one of them is the default voice Phone, consider the default voice phone
     *     if 1.4 HAL is supported, or if it is available for emergency call.
     *  b) If there are multiple Phones with the Subscriptions that support the emergency number
     *     address, and none of them is the default voice Phone, use one of these Phones if 1.4 HAL
     *     is supported, or if it is available for emergency call.
     *  c) If there is no Phone that supports the emergency call for the address, use the defined
     *     Priority list to select the Phone via {@link #getFirstPhoneForEmergencyCall}.
     */
    public Phone getPhoneForEmergencyCall(String emergencyNumberAddress) {
        // Find the list of available Phones for the given emergency number address
        List<Phone> potentialEmergencyPhones = new ArrayList<>();
        int defaultVoicePhoneId = mSubscriptionManagerProxy.getDefaultVoicePhoneId();
        for (Phone phone : mPhoneFactoryProxy.getPhones()) {
            if (phone.getEmergencyNumberTracker() != null) {
                if (phone.getEmergencyNumberTracker().isEmergencyNumber(
                        emergencyNumberAddress, true)) {
                    if (phone.getHalVersion().greaterOrEqual(RIL.RADIO_HAL_VERSION_1_4)
                            || isAvailableForEmergencyCalls(phone)) {
                        // a)
                        if (phone.getPhoneId() == defaultVoicePhoneId) {
                            Log.i(this, "getPhoneForEmergencyCall, Phone Id that supports"
                                    + " emergency number: " + phone.getPhoneId());
                            return phone;
                        }
                        potentialEmergencyPhones.add(phone);
                    }
                }
            }
        }
        // b)
        if (potentialEmergencyPhones.size() > 0) {
            Log.i(this, "getPhoneForEmergencyCall, Phone Id that supports emergency number:"
                    + potentialEmergencyPhones.get(0).getPhoneId());
            return getFirstPhoneForEmergencyCall(potentialEmergencyPhones);
        }
        // c)
        return getFirstPhoneForEmergencyCall();
    }

    @VisibleForTesting
    public Phone getFirstPhoneForEmergencyCall() {
        return getFirstPhoneForEmergencyCall(null);
    }

    /**
     * Retrieves the most sensible Phone to use for an emergency call using the following Priority
     *  list (for multi-SIM devices):
     *  1) The User's SIM preference for Voice calling
     *  2) The First Phone that is currently IN_SERVICE or is available for emergency calling
     *  3) Prioritize phones that have the dialed emergency number as part of their emergency
     *     number list
     *  4) If there is a PUK locked SIM, compare the SIMs that are not PUK locked. If all the SIMs
     *     are locked, skip to condition 5).
     *  5) The Phone with more Capabilities.
     *  6) The First Phone that has a SIM card in it (Starting from Slot 0...N)
     *  7) The Default Phone (Currently set as Slot 0)
     */
    @VisibleForTesting
    public Phone getFirstPhoneForEmergencyCall(List<Phone> phonesWithEmergencyNumber) {
        // 1)
        int phoneId = mSubscriptionManagerProxy.getDefaultVoicePhoneId();
        if (phoneId != SubscriptionManager.INVALID_PHONE_INDEX) {
            Phone defaultPhone = mPhoneFactoryProxy.getPhone(phoneId);
            if (defaultPhone != null && isAvailableForEmergencyCalls(defaultPhone)) {
                if (phonesWithEmergencyNumber == null
                        || phonesWithEmergencyNumber.contains(defaultPhone)) {
                    return defaultPhone;
                }
            }
        }

        Phone firstPhoneWithSim = null;
        int phoneCount = mTelephonyManagerProxy.getPhoneCount();
        List<SlotStatus> phoneSlotStatus = new ArrayList<>(phoneCount);
        for (int i = 0; i < phoneCount; i++) {
            Phone phone = mPhoneFactoryProxy.getPhone(i);
            if (phone == null) {
                continue;
            }
            // 2)
            if (isAvailableForEmergencyCalls(phone)) {
                if (phonesWithEmergencyNumber == null
                        || phonesWithEmergencyNumber.contains(phone)) {
                    // the slot has the radio on & state is in service.
                    Log.i(this,
                            "getFirstPhoneForEmergencyCall, radio on & in service, Phone Id:" + i);
                    return phone;
                }
            }
            // 5)
            // Store the RAF Capabilities for sorting later.
            int radioAccessFamily = phone.getRadioAccessFamily();
            SlotStatus status = new SlotStatus(i, radioAccessFamily);
            phoneSlotStatus.add(status);
            Log.i(this, "getFirstPhoneForEmergencyCall, RAF:" +
                    Integer.toHexString(radioAccessFamily) + " saved for Phone Id:" + i);
            // 4)
            // Report Slot's PIN/PUK lock status for sorting later.
            int simState = mSubscriptionManagerProxy.getSimStateForSlotIdx(i);
            // Record SimState.
            status.simState = simState;
            if (simState == TelephonyManager.SIM_STATE_PIN_REQUIRED ||
                    simState == TelephonyManager.SIM_STATE_PUK_REQUIRED) {
                status.isLocked = true;
            }
            // 3) Store if the Phone has the corresponding emergency number
            if (phonesWithEmergencyNumber != null) {
                for (Phone phoneWithEmergencyNumber : phonesWithEmergencyNumber) {
                    if (phoneWithEmergencyNumber != null
                            && phoneWithEmergencyNumber.getPhoneId() == i) {
                        status.hasDialedEmergencyNumber = true;
                    }
                }
            }
            // 6)
            if (firstPhoneWithSim == null && mTelephonyManagerProxy.hasIccCard(i)) {
                // The slot has a SIM card inserted, but is not in service, so keep track of this
                // Phone. Do not return because we want to make sure that none of the other Phones
                // are in service (because that is always faster).
                firstPhoneWithSim = phone;
                Log.i(this, "getFirstPhoneForEmergencyCall, SIM card inserted, Phone Id:" +
                        firstPhoneWithSim.getPhoneId());
            }
        }
        // 7)
        if (firstPhoneWithSim == null && phoneSlotStatus.isEmpty()) {
            if (phonesWithEmergencyNumber == null || phonesWithEmergencyNumber.isEmpty()) {
                // No Phones available, get the default
                Log.i(this, "getFirstPhoneForEmergencyCall, return default phone");
                return  mPhoneFactoryProxy.getDefaultPhone();
            }
            return phonesWithEmergencyNumber.get(0);
        } else {
            // 5)
            final int defaultPhoneId = mPhoneFactoryProxy.getDefaultPhone().getPhoneId();
            final Phone firstOccupiedSlot = firstPhoneWithSim;
            if (!phoneSlotStatus.isEmpty()) {
                // Only sort if there are enough elements to do so.
                if (phoneSlotStatus.size() > 1) {
                    Collections.sort(phoneSlotStatus, (o1, o2) -> {
                        if (!o1.hasDialedEmergencyNumber && o2.hasDialedEmergencyNumber) {
                            return -1;
                        }
                        if (o1.hasDialedEmergencyNumber && !o2.hasDialedEmergencyNumber) {
                            return 1;
                        }
                        // Sort by non-absent SIM.
                        if (o1.simState == TelephonyManager.SIM_STATE_ABSENT
                                 && o2.simState != TelephonyManager.SIM_STATE_ABSENT) {
                             return -1;
                        }
                        if (o2.simState == TelephonyManager.SIM_STATE_ABSENT
                                 && o1.simState != TelephonyManager.SIM_STATE_ABSENT) {
                             return 1;
                        }
                        // First start by seeing if either of the phone slots are locked. If they
                        // are, then sort by non-locked SIM first. If they are both locked, sort
                        // by capability instead.
                        if (o1.isLocked && !o2.isLocked) {
                            return -1;
                        }
                        if (o2.isLocked && !o1.isLocked) {
                            return 1;
                        }
                        // sort by number of RadioAccessFamily Capabilities.
                        int compare = Integer.bitCount(o1.capabilities) -
                                Integer.bitCount(o2.capabilities);
                        if (compare == 0) {
                            // Sort by highest RAF Capability if the number is the same.
                            compare = RadioAccessFamily.getHighestRafCapability(o1.capabilities) -
                                    RadioAccessFamily.getHighestRafCapability(o2.capabilities);
                            if (compare == 0) {
                                if (firstOccupiedSlot != null) {
                                    // If the RAF capability is the same, choose based on whether or
                                    // not any of the slots are occupied with a SIM card (if both
                                    // are, always choose the first).
                                    if (o1.slotId == firstOccupiedSlot.getPhoneId()) {
                                        return 1;
                                    } else if (o2.slotId == firstOccupiedSlot.getPhoneId()) {
                                        return -1;
                                    }
                                } else {
                                    // No slots have SIMs detected in them, so weight the default
                                    // Phone Id greater than the others.
                                    if (o1.slotId == defaultPhoneId) {
                                        return 1;
                                    } else if (o2.slotId == defaultPhoneId) {
                                        return -1;
                                    }
                                }
                            }
                        }
                        return compare;
                    });
                }
                int mostCapablePhoneId = phoneSlotStatus.get(phoneSlotStatus.size() - 1).slotId;
                Log.i(this, "getFirstPhoneForEmergencyCall, Using Phone Id: " + mostCapablePhoneId +
                        "with highest capability");
                return mPhoneFactoryProxy.getPhone(mostCapablePhoneId);
            } else {
                // 6)
                return firstPhoneWithSim;
            }
        }
    }

    /**
     * Returns true if the state of the Phone is IN_SERVICE or available for emergency calling only.
     */
    private boolean isAvailableForEmergencyCalls(Phone phone) {
        /// M: CC: [ALPS04214959] Smart Lock: not allow empty slot for ECC @{
        MtkTelephonyManagerEx telEx = MtkTelephonyManagerEx.getDefault();
        if (telEx != null
                && telEx.getSimLockPolicy() ==
                        MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_CS
                && telEx.getShouldServiceCapability(phone.getPhoneId()) ==
                        MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_NO_SERVICE) {
            Log.i(this, "isAvailableForEmergencyCalls found NO_SERVICE constraints, return false."
                    + "(ServiceState=" + phone.getServiceState().getState()
                    + ", isEmergencyOnly=" + phone.getServiceState().isEmergencyOnly() + ")");
            return false;
        }
        /// @}
        return ServiceState.STATE_IN_SERVICE == phone.getServiceState().getState() ||
                phone.getServiceState().isEmergencyOnly();
    }

    /**
     * Determines if the connection should allow mute.
     *
     * @param phone The current phone.
     * @return {@code True} if the connection should allow mute.
     */
    private boolean allowsMute(Phone phone) {
        // For CDMA phones, check if we are in Emergency Callback Mode (ECM).  Mute is disallowed
        // in ECM mode.
        if (phone.getPhoneType() == TelephonyManager.PHONE_TYPE_CDMA) {
            if (phone.isInEcm()) {
                return false;
            }
        }

        return true;
    }

    @Override
    public void removeConnection(Connection connection) {
        /// M: CC: ECC retry @{
        //super.removeConnection(connection);
        boolean handleEcc = false;
        if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
            if (connection instanceof TelephonyConnection) {
                if (((TelephonyConnection) connection).shouldTreatAsEmergencyCall()) {
                    handleEcc = true;
                }
            }
        }

        if (handleEcc) {
            Log.d(this, "ECC retry: remove connection.");
            MtkTelephonyConnectionServiceUtil.getInstance().setEccRetryCallId(
                    removeConnectionInternal(connection));
        } else { //Original flow
            super.removeConnection(connection);
        }
        /// @}

        if (connection instanceof TelephonyConnection) {
            TelephonyConnection telephonyConnection = (TelephonyConnection) connection;
            telephonyConnection.removeTelephonyConnectionListener(mTelephonyConnectionListener);
        }
    }

    /**
     * When a {@link TelephonyConnection} has its underlying original connection configured,
     * we need to add it to the correct conference controller.
     *
     * @param connection The connection to be added to the controller
     */
    public void addConnectionToConferenceController(TelephonyConnection connection) {
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        int connPhoneType = PhoneConstants.PHONE_TYPE_NONE;
        if (connection instanceof MtkGsmCdmaConnection) {
            connPhoneType = ((MtkGsmCdmaConnection) connection)
                    .getPhoneType();
        }
        /// @}
        // TODO: Need to revisit what happens when the original connection for the
        // TelephonyConnection changes.  If going from CDMA --> GSM (for example), the
        // instance of TelephonyConnection will still be a CdmaConnection, not a GsmConnection.
        // The CDMA conference controller makes the assumption that it will only have CDMA
        // connections in it, while the other conference controllers aren't as restrictive.  Really,
        // when we go between CDMA and GSM we should replace the TelephonyConnection.
        if (connection.isImsConnection()) {
            Log.d(this, "Adding IMS connection to conference controller: " + connection);
            mImsConferenceController.add(connection);
            mTelephonyConferenceController.remove(connection);

            /// M: CC: Merge to MtkGsmCdmaConnection @{
/*
            if (connection instanceof CdmaConnection) {
                mCdmaConferenceController.remove((CdmaConnection) connection);
*/
            if (connPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                mCdmaConferenceController.remove((MtkGsmCdmaConnection) connection);
            }
            /// @}
        } else {
            /// M: CC: [ALPS03565497] To avoid JE @{
            if (connection.getCall() == null || connection.getCall().getPhone() == null) {
                Log.d(this, "Connection died, no need to add to conference controller");
                return;
            }
            /// @}
            int phoneType = connection.getCall().getPhone().getPhoneType();
            if (phoneType == TelephonyManager.PHONE_TYPE_GSM) {
                Log.d(this, "Adding GSM connection to conference controller: " + connection);
                mTelephonyConferenceController.add(connection);

                /// M: CC: Merge to MtkGsmCdmaConnection @{
/*
                if (connection instanceof CdmaConnection) {
                    mCdmaConferenceController.remove((CdmaConnection) connection);
*/
                if (connPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                    mCdmaConferenceController.remove((MtkGsmCdmaConnection) connection);
                }
                /// @}
            } else if (phoneType == TelephonyManager.PHONE_TYPE_CDMA &&
                    connPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                Log.d(this, "Adding CDMA connection to conference controller: " + connection);
                /// M: CC: Merge to MtkGsmCdmaConnection @{
                //mCdmaConferenceController.add((CdmaConnection) connection);
                mCdmaConferenceController.add((MtkGsmCdmaConnection) connection);
                /// @}
                mTelephonyConferenceController.remove(connection);
            }
            Log.d(this, "Removing connection from IMS conference controller: " + connection);
            mImsConferenceController.remove(connection);
        }
    }

    /**
     * Create a new CDMA connection. CDMA connections have additional limitations when creating
     * additional calls which are handled in this method.  Specifically, CDMA has a "FLASH" command
     * that can be used for three purposes: merging a call, swapping unmerged calls, and adding
     * a new outgoing call. The function of the flash command depends on the context of the current
     * set of calls. This method will prevent an outgoing call from being made if it is not within
     * the right circumstances to support adding a call.
     */
    private Connection checkAdditionalOutgoingCallLimits(Phone phone) {
        if (phone.getPhoneType() == TelephonyManager.PHONE_TYPE_CDMA) {
            // Check to see if any CDMA conference calls exist, and if they do, check them for
            // limitations.
            for (Conference conference : getAllConferences()) {
                if (conference instanceof CdmaConference) {
                    CdmaConference cdmaConf = (CdmaConference) conference;

                    // If the CDMA conference has not been merged, add-call will not work, so fail
                    // this request to add a call.
                    if (cdmaConf.can(Connection.CAPABILITY_MERGE_CONFERENCE)) {
                        return Connection.createFailedConnection(new DisconnectCause(
                                    DisconnectCause.RESTRICTED,
                                    null,
                                    getResources().getString(R.string.callFailed_cdma_call_limit),
                                    "merge-capable call exists, prevent flash command."));
                    }
                }
            }

            /// M: CC: there's no CDMA conference that has not been merged, but we are creating one.
            // Also don't add call. @{
            if (mCdmaConferenceController.hasDelayedConferenceCreation()) {
                return Connection.createFailedConnection(new DisconnectCause(
                            DisconnectCause.RESTRICTED,
                            null,
                            getResources().getString(R.string.callFailed_cdma_call_limit),
                            "merge-capable call exists, prevent flash command."));
            }
            /// @}
        }

        return null; // null means nothing went wrong, and call should continue.
    }

    private boolean isTtyModeEnabled(Context context) {
        return (android.provider.Settings.Secure.getInt(
                context.getContentResolver(),
                android.provider.Settings.Secure.PREFERRED_TTY_MODE,
                TelecomManager.TTY_MODE_OFF) != TelecomManager.TTY_MODE_OFF);
    }

    /**
     * For outgoing dialed calls, potentially send a ConnectionEvent if the user is on WFC and is
     * dialing an international number.
     * @param telephonyConnection The connection.
     */
    private void maybeSendInternationalCallEvent(TelephonyConnection telephonyConnection) {
        if (telephonyConnection == null || telephonyConnection.getPhone() == null ||
                telephonyConnection.getPhone().getDefaultPhone() == null) {
            return;
        }
        Phone phone = telephonyConnection.getPhone().getDefaultPhone();
        if (phone instanceof GsmCdmaPhone) {
            GsmCdmaPhone gsmCdmaPhone = (GsmCdmaPhone) phone;
            if (telephonyConnection.isOutgoingCall() &&
                    gsmCdmaPhone.isNotificationOfWfcCallRequired(
                            telephonyConnection.getOriginalConnection().getOrigDialString())) {
                // Send connection event to InCall UI to inform the user of the fact they
                // are potentially placing an international call on WFC.
                Log.i(this, "placeOutgoingConnection - sending international call on WFC " +
                        "confirmation event");
                telephonyConnection.sendConnectionEvent(
                        TelephonyManager.EVENT_NOTIFY_INTERNATIONAL_CALL_ON_WFC, null);
            }
        }
    }

    private void handleTtyModeChange(boolean isTtyEnabled) {
        Log.i(this, "handleTtyModeChange; isTtyEnabled=%b", isTtyEnabled);
        mIsTtyEnabled = isTtyEnabled;
        for (Connection connection : getAllConnections()) {
            if (connection instanceof TelephonyConnection) {
                TelephonyConnection telephonyConnection = (TelephonyConnection) connection;
                telephonyConnection.setTtyEnabled(isTtyEnabled);
            }
        }
    }

    /// M: CC: Set PhoneAccountHandle for ECC
    //[ALPS01794357]
    /**
     * This can be used by telecom to either create a new outgoing call or attach to an existing
     * incoming call. In either case, telecom will cycle through a set of services and call
     * createConnection util a connection service cancels the process or completes it successfully.
     */
    /** {@hide} */
    protected void createConnection(
            final PhoneAccountHandle callManagerAccount,
            final String callId,
            final ConnectionRequest request,
            boolean isIncoming,
            boolean isUnknown) {
        boolean isLegacyHandover = request.getExtras() != null &&
                request.getExtras().getBoolean(TelecomManager.EXTRA_IS_HANDOVER, false);
        boolean isHandover = request.getExtras() != null && request.getExtras().getBoolean(
                TelecomManager.EXTRA_IS_HANDOVER_CONNECTION, false);
        // ConnetionRequest print number with Connection.toLogSafePhoneNumber(), need to use Log.v
        Log.v(this, "createConnection, callManagerAccount: %s, callId: %s, request: %s, " +
                        "isIncoming: %b, isUnknown: %b, isLegacyHandover: %b, isHandover: %b",
                callManagerAccount, callId, request, isIncoming, isUnknown, isLegacyHandover,
                isHandover);

        /// M: CC: createConnection() may be called in post runnable, so there's timing issue that
        // it's invoked after call aborted. @{
        if (!isAdaptersAvailable()) {
            Log.i(this, "createConnection, adapter not available, call should have been aborted");
            return;
        }
        /// @}

        Connection connection = null;
        if (isHandover) {
            PhoneAccountHandle fromPhoneAccountHandle = request.getExtras() != null
                    ? (PhoneAccountHandle) request.getExtras().getParcelable(
                    TelecomManager.EXTRA_HANDOVER_FROM_PHONE_ACCOUNT) : null;
            if (!isIncoming) {
                connection = onCreateOutgoingHandoverConnection(fromPhoneAccountHandle, request);
            } else {
                connection = onCreateIncomingHandoverConnection(fromPhoneAccountHandle, request);
            }
        } else {
            connection = isUnknown ? onCreateUnknownConnection(callManagerAccount, request)
                    : isIncoming ? onCreateIncomingConnection(callManagerAccount, request)
                    : onCreateOutgoingConnection(callManagerAccount, request);
        }
        Log.d(this, "createConnection, connection: %s", connection);
        if (connection == null) {
            Log.i(this, "createConnection, implementation returned null connection.");
            connection = Connection.createFailedConnection(
                    new DisconnectCause(DisconnectCause.ERROR, "IMPL_RETURNED_NULL_CONNECTION"));
        }

        /// M: CC: Set PhoneAccountHandle for ECC @{
        //[ALPS01794357]
        PhoneAccountHandle handle = null;
        if (connection instanceof TelephonyConnection) {
            handle = ((TelephonyConnection) connection).getAccountHandle();
        }
        if (handle == null) {
            handle = request.getAccountHandle();
        } else {
            Log.d(this, "createConnection, set back phone account:%s", handle);
        }
        //// @}

        boolean isSelfManaged =
                (connection.getConnectionProperties() & Connection.PROPERTY_SELF_MANAGED)
                        == Connection.PROPERTY_SELF_MANAGED;
        // Self-managed Connections should always use voip audio mode; we default here so that the
        // local state within the ConnectionService matches the default we assume in Telecom.
        if (isSelfManaged) {
            connection.setAudioModeIsVoip(true);
        }
        connection.setTelecomCallId(callId);
        if (connection.getState() != Connection.STATE_DISCONNECTED) {
            addConnection(handle, callId, connection); /// M: CC: PhoneAccountHandle [ALPS01794357]
        }
        // Connection.toLogSafePhoneNumber() will print log if DEBUG=true, so need to use Log.v()
        Uri address = connection.getAddress();
        String number = address == null ? "null" : address.getSchemeSpecificPart();
        Log.v(this, "createConnection, number: %s, state: %s, capabilities: %s, properties: %s",
                Connection.toLogSafePhoneNumber(number),
                Connection.stateToString(connection.getState()),
                MtkConnection.capabilitiesToString(connection.getConnectionCapabilities()),
                MtkConnection.propertiesToString(connection.getConnectionProperties()));

        Log.d(this, "createConnection, calling handleCreateConnectionSuccessful %s", callId);
        mAdapter.handleCreateConnectionComplete(
                callId,
                request,
                new ParcelableConnection(
                        handle,  /* M: CC: Set PhoneAccountHandle for ECC [ALPS01794357] */
                        connection.getState(),
                        connection.getConnectionCapabilities(),
                        connection.getConnectionProperties(),
                        connection.getSupportedAudioRoutes(),
                        connection.getAddress(),
                        connection.getAddressPresentation(),
                        connection.getCallerDisplayName(),
                        connection.getCallerDisplayNamePresentation(),
                        connection.getVideoProvider() == null ?
                                null : connection.getVideoProvider().getInterface(),
                        connection.getVideoState(),
                        connection.isRingbackRequested(),
                        connection.getAudioModeIsVoip(),
                        connection.getConnectTimeMillis(),
                        connection.getConnectElapsedTimeMillis(),
                        connection.getStatusHints(),
                        connection.getDisconnectCause(),
                        createIdList(connection.getConferenceables()),
                        connection.getExtras()));

        if (isIncoming && request.shouldShowIncomingCallUi() && isSelfManaged) {
            // Tell ConnectionService to show its incoming call UX.
            connection.onShowIncomingCallUi();
        }
        if (isUnknown) {
            triggerConferenceRecalculate();
        }

        /// M: CC: Proprietary CRSS handling @{
        // [ALPS01956888] For FailureSignalingConnection, CastException JE will happen.
        if (connection.getState() != Connection.STATE_DISCONNECTED) {
            forceSuppMessageUpdate(connection);
        }
        /// @}
    }

    /// M: Telecom related. @{
    private final Handler mMtkHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            /// M: CC: HangupAll for FTA 31.4.4.2 @{
            case MSG_HANGUP_ALL:
                hangupAll((String) msg.obj);
                break;
            /// @}
            /// M: CC: For MSMS/MSMA ordered user operations. @{
            case MSG_HANDLE_ORDERED_USER_OPERATION: {
                SomeArgs args = (SomeArgs) msg.obj;
                try {
                    String callId = (String) args.arg1;
                    String currentOperation = (String) args.arg2;
                    String pendingOperation = (String) args.arg3;
                    if (MtkConnection.OPERATION_DISCONNECT_CALL.equals(currentOperation)) {
                        disconnect(callId, pendingOperation);
                    }
                } finally {
                    args.recycle();
                }
                break;
            }
            /// @}

            /// M: CC: Interface for ECT @{
            case MSG_ECT:
                explicitCallTransfer((String) msg.obj);
                break;
            /// @}

            /// M: For VoLTE @{
            case MSG_INVITE_CONFERENCE_PARTICIPANTS: {
                SomeArgs args = (SomeArgs) msg.obj;
                try {
                    String conferenceCallId = (String) args.arg1;
                    List<String> numbers = (List<String>) args.arg2;
                    inviteConferenceParticipants(conferenceCallId, numbers);
                } finally {
                    args.recycle();
                }
                break;
            }
            case MSG_CREATE_CONFERENCE: {
                SomeArgs args = (SomeArgs) msg.obj;
                try {
                    final PhoneAccountHandle connectionManagerPhoneAccount =
                            (PhoneAccountHandle) args.arg1;
                    final String conferenceCallId = (String) args.arg2;
                    final ConnectionRequest request = (ConnectionRequest) args.arg3;
                    final List<String> numbers = (List<String>) args.arg4;
                    final boolean isIncoming = args.argi1 == 1;
                    final Session.Info info = (Session.Info) args.arg5;
                    if (!mAreAccountsInitialized) {
                        Log.d(this, "Enqueueing pre-init request %s", conferenceCallId);
                        mPreInitializationConnectionRequests.add(new Runnable() {
                            @Override
                            public void run() {
                                createConference(
                                        connectionManagerPhoneAccount,
                                        conferenceCallId,
                                        request,
                                        numbers,
                                        isIncoming,
                                        info);
                            }
                        });
                    } else {
                        createConference(
                                connectionManagerPhoneAccount,
                                conferenceCallId,
                                request,
                                numbers,
                                isIncoming,
                                info);
                    }
                } finally {
                    args.recycle();
                }
                break;
            }
            /// M: CC: Interface for blind/assured ECT @{
            case MSG_BLIND_ASSURED_ECT: {
                SomeArgs args = (SomeArgs) msg.obj;
                try {
                    String callId = (String) args.arg1;
                    String number = (String) args.arg2;
                    int type = args.argi1;
                    explicitCallTransfer(callId, number, type);
                } finally {
                    args.recycle();
                }
                break;
            }
            /// @}
            /// M: CC: Softbank blacklist requirement @{
            case MSG_REJECT_WITH_CAUSE: {
                SomeArgs args = (SomeArgs) msg.obj;
                try {
                    String callId = (String) args.arg1;
                    int cause = (int) args.arg2;
                    rejectWithCause(callId, cause);
                } finally {
                    args.recycle();
                }
                break;
            }
            /// @}

            default:
                Log.d(this, "mMtkHandler default return (msg.what=%d)", msg.what);
                break;
            }
        }
    };

    private MtkConnectionServiceBinder mMtkBinder = null;

    @Override
    protected IBinder getConnectionServiceBinder() {
        if (mMtkBinder == null) {
            Log.d(this, "init MtkConnectionServiceBinder");
            mMtkBinder = new MtkConnectionServiceBinder();
        }
        return (IBinder) mMtkBinder;
    }

    /**
     * M: Mediatek APIs for call related operations.
     */
    private class MtkConnectionServiceBinder extends IMtkConnectionService.Stub {

        /// M: return AOSP binder synchronously
        public IBinder getBinder() {
            return mBinder;
        }

        public void addMtkConnectionServiceAdapter(IMtkConnectionServiceAdapter adapter) {
            Log.d(this, "MtkConnectionServiceBinder add IMtkConnectionServiceAdapter");
            mMtkAdapter = adapter;
            return;
        }

        public void clearMtkConnectionServiceAdapter() {
            Log.d(this, "MtkConnectionServiceBinder clear IMtkConnectionServiceAdapter");
            mMtkAdapter = null;
        }

        /// M: CC: HangupAll for FTA 31.4.4.2.
        @Override
        public void hangupAll(String callId) {
            mMtkHandler.obtainMessage(MSG_HANGUP_ALL, callId).sendToTarget();
        }

        /// M: CC: For MSMS/MSMA ordered user operations. @{
        @Override
        public void handleOrderedOperation(
                String callId, String currentOperation, String pendingOperation) {
            //mHandler.obtainMessage(MSG_DISCONNECT, callId).sendToTarget();
            SomeArgs args = SomeArgs.obtain();
            args.arg1 = callId;
            args.arg2 = currentOperation;
            args.arg3 = pendingOperation;
            mMtkHandler.obtainMessage(MSG_HANDLE_ORDERED_USER_OPERATION, args).sendToTarget();
        }
        /// @}

        /// M: CC: Interface for ECT @{
        @Override
        public void explicitCallTransfer(String callId) {
            mMtkHandler.obtainMessage(MSG_ECT, callId).sendToTarget();
        }
        /// @}

        // Interface for blind/assured ECT.
        @Override
        public void blindAssuredEct(String callId, String number, int type) {
            SomeArgs args = SomeArgs.obtain();
            args.arg1 = callId;
            args.arg2 = number;
            args.argi1 = type;
            mMtkHandler.obtainMessage(MSG_BLIND_ASSURED_ECT, args).sendToTarget();
        }

        @Override
        public void inviteConferenceParticipants(String conferenceCallId, List<String> numbers) {
            SomeArgs args = SomeArgs.obtain();
            args.arg1 = conferenceCallId;
            args.arg2 = numbers;
            mMtkHandler.obtainMessage(MSG_INVITE_CONFERENCE_PARTICIPANTS, args).sendToTarget();
        }

        @Override
        public void createConference(
                final PhoneAccountHandle connectionManagerPhoneAccount,
                final String conferenceCallId,
                final ConnectionRequest request,
                final List<String> numbers,
                boolean isIncoming,
                Session.Info sessionInfo) {
            SomeArgs args = SomeArgs.obtain();
            args.arg1 = connectionManagerPhoneAccount;
            args.arg2 = conferenceCallId;
            args.arg3 = request;
            args.arg4 = numbers;
            args.argi1 = isIncoming ? 1 : 0;
            args.arg5 = sessionInfo;
            mMtkHandler.obtainMessage(MSG_CREATE_CONFERENCE, args).sendToTarget();
        }

        /// M: CC: Softbank blacklist requirement @{
        public void rejectWithCause(String callId, int cause) {
            SomeArgs args = SomeArgs.obtain();
            args.arg1 = callId;
            args.arg2 = cause;
            mMtkHandler.obtainMessage(MSG_REJECT_WITH_CAUSE, args).sendToTarget();
        }
        /// @}
    }
    /// M: @}

    @Override
    protected void addConnection(PhoneAccountHandle handle, String callId, Connection connection) {
        connection.setTelecomCallId(callId);
        mConnectionById.put(callId, connection);
        mIdByConnection.put(connection, callId);
        // M: Since TelephonyConnectionService has declared as self private connection listener
        // So here is to remove parent's listener
        connection.addConnectionListener(super.mConnectionListener);
        connection.setConnectionService(this);
        connection.setPhoneAccountHandle(handle);
        onConnectionAdded(connection);
        /// M: CC: Force updateState for Connection once its ConnectionService is set @{
        // Forcing call state update after ConnectionService is set
        // to keep capabilities up-to-date.
        if (connection instanceof TelephonyConnection) {
            ((TelephonyConnection) connection).fireOnCallState();
        }
        /// @}
    }

    /// M: CC: Proprietary CRSS handling @{
    /**
     * Base class for forcing SuppMessage update after ConnectionService is set,
     * see {@link ConnectionService#addConnection}
     * To be overrided by children classes.
     * @hide
     */
    protected void forceSuppMessageUpdate(Connection conn) {
        MtkTelephonyConnectionServiceUtil.getInstance().forceSuppMessageUpdate(
                (TelephonyConnection) conn);
    }
    /// @}

    /// M: CC: HangupAll for FTA 31.4.4.2 @{
    private void hangupAll(String callId) {
        Log.d(this, "hangupAll %s", callId);
        if (mConnectionById.containsKey(callId)) {
            ((TelephonyConnection)findConnectionForAction(callId, "hangupAll")).onHangupAll();
        } else {
            Conference conf = findConferenceForAction(callId, "hangupAll");
            if (conf instanceof TelephonyConference) {
                ((TelephonyConference)conf).onHangupAll();
            } else if (conf instanceof CdmaConference) {
                ((CdmaConference)conf).onHangupAll();
            } else if (conf instanceof ImsConference) {
                ((ImsConference) conf).onHangupAll();
            }
        }
    }
    /// @}

    /// M: CC: For MSMS/MSMA ordered user operations. @{
    private void disconnect(String callId, String pendingOperation) {
        Log.d(this, "disconnect %s, pending call action %s", callId, pendingOperation);
        if (mConnectionById.containsKey(callId)) {
            ((TelephonyConnection) findConnectionForAction(callId,
                    MtkConnection.OPERATION_DISCONNECT_CALL)).onDisconnect();
        } else {
            Conference conf = findConferenceForAction(callId,
                    MtkConnection.OPERATION_DISCONNECT_CALL);
            if (conf instanceof TelephonyConference) {
                ((TelephonyConference) conf).onDisconnect(pendingOperation);
            } else if (conf instanceof CdmaConference) {
                ((CdmaConference) conf).onDisconnect();
            } else if (conf instanceof ImsConference) {
                ((ImsConference) conf).onDisconnect();
            }
        }
    }
    /// @}

    /// M: CC: Interface for ECT @{
    private void explicitCallTransfer(String callId) {
        if (!canTransfer(mConnectionById.get(callId))) {
            Log.d(this, "explicitCallTransfer %s fail", callId);
            return;
        }
        Log.d(this, "explicitCallTransfer %s", callId);
        ((TelephonyConnection) findConnectionForAction(callId, "explicitCallTransfer"))
                .onExplicitCallTransfer();
    }
    /// @}

    /// M: CC: Interface for blind/assured ECT @{
    private void explicitCallTransfer(String callId, String number, int type) {
        if (!canBlindAssuredTransfer(mConnectionById.get(callId))) {
            Log.d(this, "explicitCallTransfer %s fail", callId);
            return;
        }
        Log.d(this, "explicitCallTransfer %s %s %d", callId, Log.pii(number), type);
        ((TelephonyConnection) findConnectionForAction(callId, "explicitCallTransfer")).
                onExplicitCallTransfer(number, type);
    }
    /// @}

    /**
      * Check whether onExplicitCallTransfer() can be performed on a certain connection.
      * Default implementation, need to be overrided.
      * @param bgConnection
      * @return true allowed false disallowed
      * @hide
      */
    public boolean canTransfer(Connection bgConnection) {

        if (bgConnection == null) {
            Log.d(this, "canTransfer: connection is null");
            return false;
        }

        if (!(bgConnection instanceof TelephonyConnection)) {
            // the connection may be ConferenceParticipantConnection.
            Log.d(this, "canTransfer: the connection isn't telephonyConnection");
            return false;
        }

        TelephonyConnection bConnection = (TelephonyConnection) bgConnection;

        Phone activePhone = null;
        Phone heldPhone = null;

        TelephonyConnection fConnection = getFgConnection();
        if (fConnection != null) {
            activePhone = fConnection.getPhone();
        }

        if (bgConnection != null) {
            heldPhone = bConnection.getPhone();
        }

        return (heldPhone == activePhone && activePhone != null && activePhone.canTransfer());
    }

    public TelephonyConnection getFgConnection() {

        for (Connection c : getAllConnections()) {

            if (!(c instanceof TelephonyConnection)) {
                // the connection may be ConferenceParticipantConnection.
                continue;
            }

            TelephonyConnection tc = (TelephonyConnection) c;

            if (tc.getCall() == null) {
                continue;
            }

            Call.State s = tc.getCall().getState();

            // it assume that only one Fg call at the same time
            if (s == Call.State.ACTIVE || s == Call.State.DIALING || s == Call.State.ALERTING) {
                return tc;
            }
        }
        return null;
    }
    /// @}

    /// M: CC: Softbank blacklist requirement @{
    private void rejectWithCause(String callId, int cause) {
        Log.d(this, "reject %s with cause %d", callId, cause);
        ((TelephonyConnection) findConnectionForAction(callId, "reject")).onReject(cause);
    }
    /// @}

    /// M: CC: ECC retry. @{
    // Used for destroy the old connection when ECC phone type is not default phone type.
    private void resetTreatAsEmergencyCall(Connection connection) {
        if (connection instanceof TelephonyConnection) {
            ((TelephonyConnection) connection).resetTreatAsEmergencyCall();
        }
    }
    /// @}

    /// M: CC: ECC switch phone for SVLTE. @{
    private Connection switchPhoneIfNeeded(final ConnectionRequest request,
            final Uri emergencyHandle, final String numberToDial) {
        /// M: CC: ECC for Fusion RIL @{
        if (hasC2kOverImsModem()) {
            return null;
        } else if (mSwitchPhoneHelper == null) {
            mSwitchPhoneHelper = new SwitchPhoneHelper(this, numberToDial);
        }
        if (mSwitchPhoneHelper.needToPrepareForDial()) {
            final Connection emergencyConnection = getTelephonyConnection(request, numberToDial,
                    true, emergencyHandle, mPhoneFactoryProxy.getDefaultPhone());

            /// M: CC: Return the failed connection directly @{
            if (!(emergencyConnection instanceof TelephonyConnection)) {
                Log.i(this, "onCreateOutgoingConnection, create emergency connection failed");
                return emergencyConnection;
            }
            /// @}

            mSwitchPhoneHelper.prepareForDial(
                    new SwitchPhoneHelper.Callback() {
                        @Override
                        public void onComplete(boolean success) {
                            if (emergencyConnection.getState()
                                    == Connection.STATE_DISCONNECTED) {
                                Log.d(this, "prepareForDial, connection disconnect");
                                return;
                            } else if (success) {
                                Log.d(this, "startTurnOnRadio");
                                startTurnOnRadio(emergencyConnection, request,
                                        emergencyHandle, numberToDial);
                            } else {
                                /// M: CC: ECC retry @{
                                // Assume only one ECC exists. Don't trigger retry
                                // since MD fails to power on should be a bug.
                                if (MtkTelephonyConnectionServiceUtil.getInstance()
                                        .isEccRetryOn()) {
                                    Log.d(this, "ECC retry: clear ECC param");
                                    MtkTelephonyConnectionServiceUtil.getInstance()
                                            .clearEccRetryParams();
                                }
                                /// @}
                                Log.d(this, "prepareForDial, failed to turn on radio");
                                emergencyConnection.setDisconnected(
                                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                        android.telephony.DisconnectCause.POWER_OFF,
                                        "Failed to turn on radio."));
                                emergencyConnection.destroy();
                            }
                        }
                    });
            // Return the still unconnected GsmConnection and wait for the Radios to boot before
            // connecting it to the underlying Phone.
            return emergencyConnection;
        }
        return null;
    }

    private void startTurnOnRadio(final Connection connection,
            final ConnectionRequest request, final Uri emergencyHandle, String number) {
        final int defaultPhoneType = PhoneFactory.getDefaultPhone().getPhoneType();
        final String numberToDial = number;
        if (mRadioOnHelper == null) {
            mRadioOnHelper = new RadioOnHelper(this);
        }

        mRadioOnHelper.triggerRadioOnAndListen(new RadioOnStateListener.Callback() {
            @Override
            public void onComplete(RadioOnStateListener listener, boolean isRadioReady) {
                if (connection.getState() == Connection.STATE_DISCONNECTED) {
                    Log.d(this, "startTurnOnRadio, connection disconnect");
                    return;
                }
                if (isRadioReady) {
                    // Get the right phone object from the account data passed in.
                    Phone phone = getPhoneForAccount(request.getAccountHandle(), true,
                            numberToDial);

                    /// M: TDD data only @{
                    if (MtkTelephonyConnectionServiceUtil.getInstance().isDataOnlyMode(phone)) {
                        Log.i(this, "startTurnOnRadio, 4G data only");
                        // Assume only one ECC exists. Don't trigger retry
                        // since Modem fails to power on should be a bug
                        if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                            Log.i(this, "ECC retry: clear ECC param");
                            MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
                        }
                        connection.setDisconnected(
                                DisconnectCauseUtil.toTelecomDisconnectCause(
                                android.telephony.DisconnectCause.OUTGOING_CANCELED,
                                null));
                        connection.destroy();
                        return;
                    }
                    /// @}

                    /// M: CC: ECC retry @{
                    /* If current phone number will be treated as normal call in
                       Telephony Framework, do not need to enable ECC retry mechanism */
                    boolean isDialedByEmergencyCommand = false;
                    if (phone.getEmergencyNumberTracker() != null) {
                        if (phone.getEmergencyNumberTracker()
                                .getEmergencyCallRouting(number)
                                != EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL) {
                            isDialedByEmergencyCommand = true;
                        }
                    }

                    if (!MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn() &&
                        isDialedByEmergencyCommand) {
                        Log.i(this, "ECC retry : set param with Intial ECC.");
                        MtkTelephonyConnectionServiceUtil.getInstance().setEccRetryParams(
                                request,
                                phone.getPhoneId());
                    }
                    /// @}


                    delayDialForDdsSwitch(phone).whenComplete((result, error) -> {
                        if (error != null) {
                            Log.w(this, "handleOnComplete - delayDialForDdsSwitch exception= "
                                    + error.getMessage());
                        }
                        Log.i(this, "handleOnComplete - delayDialForDdsSwitch result = " + result);
                        adjustAndPlaceOutgoingConnection(phone, connection, request,
                                numberToDial, emergencyHandle, defaultPhoneType, true);
                    });

                } else {
                    /// M: CC: ECC retry @{
                    // Assume only one ECC exists. Don't trigger retry
                    // since Modem fails to power on should be a bug
                    if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                        Log.d(this, "ECC retry: clear ECC param");
                        MtkTelephonyConnectionServiceUtil.getInstance()
                                .clearEccRetryParams();
                    }
                    /// @}
                    Log.d(this, "startTurnOnRadio, failed to turn on radio");
                    connection.setDisconnected(
                            DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.POWER_OFF,
                            "Failed to turn on radio."));
                    connection.destroy();
                }
            }

            @Override
            public boolean isOkToCall(Phone phone, int serviceState) {
                // We currently only look to make sure that the radio is on before dialing.
                // We should be able to make emergency calls at any time after the radio has
                // been powered on and isn't in the UNAVAILABLE state, even if it is
                // reporting the OUT_OF_SERVICE state.
                return (phone.getState() == PhoneConstants.State.OFFHOOK)
                    /// M: CC: [ALPS04527771] Check service state and radio state together{
                    //|| phone.getServiceState().getState() != ServiceState.STATE_POWER_OFF;
                    || (phone.getServiceState().getState() != ServiceState.STATE_POWER_OFF
                            && phone.isRadioOn());
                    /// @}
            }
        });
    }
    /// @}

    /// M: CC: Set PhoneAccountHandle for ECC
    //[ALPS01794357]
    /// M: Check if adapters are still alive. Use reflection to access private member in AOSP. @{
    private boolean isAdaptersAvailable() {
        try {
            Field fieldAdapters = mAdapter.getClass().getDeclaredField("mAdapters");
            fieldAdapters.setAccessible(true);
            Object adapters = fieldAdapters.get(mAdapter);
            if (adapters != null) {
                Method method = adapters.getClass().getMethod("size");
                Object size = method.invoke(adapters);
                if (size != null && size instanceof Integer) {
                    if ((Integer) size == 0) {
                        Log.w(this, "isAdaptersAvailable, " + adapters + ", " + size);
                        return false;
                    }
                }
            }
        } catch (Exception e) {
            // It should never happen, just assume it's ok to call and return true.
            e.printStackTrace();
        }
        return true;
    }

    /// MTK Enhance IMS Conference @{
    /**
     * This can be used by telecom to either create a new outgoing conference call or
     * attach to an existing incoming conference call.
     */
    public void inviteConferenceParticipants(String conferenceCallId, List<String> numbers) {
        Log.d(this, "inviteConferenceParticipants %s", conferenceCallId);
        if (mConferenceById.containsKey(conferenceCallId)) {
            Conference conf =
                    findConferenceForAction(conferenceCallId, "inviteConferenceParticipants");
            if (conf instanceof ImsConference) {
                ((ImsConference)conf).onInviteConferenceParticipants(numbers);
            }
        }
    }
    private void createConference(
            final PhoneAccountHandle callManagerAccount,
            final String conferenceCallId,
            final ConnectionRequest request,
            final List<String> numbers,
            boolean isIncoming,
            Session.Info sessionInfo) {
        // Log.pii print log if VERBOSE=true, ConnectionRequest print address if DEBUG=true
        Log.v(this,
            "createConference, callManagerAccount: %s, conferenceCallId: %s, request: %s, " +
            "numbers: %s, isIncoming: %b", callManagerAccount, conferenceCallId, request,
            Log.pii(numbers), isIncoming);
        // Because the ConferenceController will be used when create Conference
        Conference conference = onCreateConference(
            callManagerAccount,
            conferenceCallId,
            request,
            numbers,
            isIncoming,
            sessionInfo);
        if (conference == null) {
            Log.d(this, "Fail to create conference!");
            conference = getNullConference();
        } else if (conference.getState() != Connection.STATE_DISCONNECTED) {
            if (mIdByConference.containsKey(conference)) {
                Log.d(this, "Re-adding an existing conference: %s.", conference);
            } else {
                mConferenceById.put(conferenceCallId, conference);
                mIdByConference.put(conference, conferenceCallId);
                conference.addListener(mConferenceListener);
            }
        }
        ParcelableConference parcelableConference = new ParcelableConference(
                conference.getPhoneAccountHandle(),
                conference.getState(),
                conference.getConnectionCapabilities(),
                conference.getConnectionProperties(),
                null,
                conference.getVideoProvider() == null ?
                        null : conference.getVideoProvider().getInterface(),
                conference.getVideoState(),
                conference.getConnectTimeMillis(),
                conference.getConnectionStartElapsedRealTime(),
                conference.getStatusHints(),
                conference.getExtras(),
                conference.getAddress(),
                conference.getAddressPresentation(),
                conference.getCallerDisplayName(),
                conference.getCallerDisplayNamePresentation());
        if (mMtkAdapter != null) {
            try {
                mMtkAdapter.handleCreateConferenceComplete(
                    conferenceCallId,
                    request,
                    parcelableConference,
                    conference.getDisconnectCause());
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }
    protected Conference onCreateConference(
            final PhoneAccountHandle connectionManagerPhoneAccount,
            final String conferenceCallId,
            final ConnectionRequest request,
            final List<String> numbers,
            boolean isIncoming,
            Session.Info sessionInfo) {
        if (conferenceCallId == null || mImsConferenceController.isConferenceExist()) {
            Log.d(this, "onCreateConference(), canDial check fail");
            /// M: ALPS02331568.  Should reture the failed conference. @{
            return MtkTelephonyConnectionServiceUtil.getInstance().createFailedConference(
                android.telephony.DisconnectCause.OUTGOING_FAILURE,
                "canDial() check fail");
            /// @}
        }
        Phone phone = getPhoneForAccount(request.getAccountHandle(), false, null);
        /// M: ALPS02209724. Toast if there are more than 5 numbers.
        /// M: ALPS02331568. Take away null-check for numbers. @{
        if (!isIncoming
                && numbers.size() > ImsConference.IMS_CONFERENCE_MAX_SIZE) {
            Log.d(this, "onCreateConference(), more than 5 numbers");
            if (phone != null) {
                ImsConference.toastWhenConferenceIsFull(phone.getContext());
            }
            return MtkTelephonyConnectionServiceUtil.getInstance().createFailedConference(
                    android.telephony.DisconnectCause.OUTGOING_FAILURE,
                    "more than 5 numbers");
        }
        /// @}

        ///M: add conference to HoldTracker
        Conference conference = MtkTelephonyConnectionServiceUtil.getInstance().createConference(
                mImsConferenceController,
                phone,
                request,
                numbers,
                isIncoming);
        Log.d(this, "onCreateConference(), add conference to HoldTracker, conference: " + conference);
        onConferenceAdded(conference);
        ///@}
        return conference;
    }
    /// @}

    /// M: For VoLTE conference SRVCC. (Legacy MD only)
    /// We use some mechanism to let ImsConference handover to TelephonyConference,
    /// so Telecomm see the conference id is the same. @{
    /**
     * perform Ims Conference SRVCC.
     * @param imsConf the ims conference.
     * @param radioConnections the new created radioConnection
     * @hide
     */
    void performImsConferenceSRVCC(
            Conference imsConf,
            ArrayList<com.android.internal.telephony.Connection> radioConnections,
            String telecomCallId) {
        if (imsConf == null) {
            Log.e(this, new CallStateException(),
                "performImsConferenceSRVCC(): abnormal case, imsConf is null");
            return;
        }
        if (radioConnections == null || radioConnections.size() < 2) {
            Log.e(this, new CallStateException(),
                "performImsConferenceSRVCC(): abnormal case, newConnections is null");
            return;
        }
        if (radioConnections.get(0) == null || radioConnections.get(0).getCall() == null ||
                radioConnections.get(0).getCall().getPhone() == null) {
            Log.e(this, new CallStateException(),
                "performImsConferenceSRVCC(): abnormal case, can't get phone instance");
            return;
        }
        /// M: CC: new TelephonyConference with phoneAccountHandle @{
        Phone phone = radioConnections.get(0).getCall().getPhone();
        PhoneAccountHandle handle = PhoneUtils.makePstnPhoneAccountHandle(phone);
        TelephonyConference newConf = new TelephonyConference(handle);
        /// @}
        replaceConference(imsConf, (Conference) newConf);
        if (mTelephonyConferenceController instanceof TelephonyConferenceController) {
            ((TelephonyConferenceController)
                    mTelephonyConferenceController).setHandoveredConference(newConf);
        }
        // we need to follow the order below:
        // 1. new empty GsmConnection
        // 2. addExistingConnection (and it will be added to TelephonyConferenceController)
        // 3. config originalConnection.
        // Then UI will not flash the participant calls during SRVCC.
        /// M: CC: Merge to MtkGsmCdmaConnection
        //ArrayList<TelephonyConnection> newGsmConnections = new ArrayList<TelephonyConnection>();
        ArrayList<TelephonyConnection> newGsmCdmaConnections = new ArrayList<TelephonyConnection>();
        for (com.android.internal.telephony.Connection radioConn : radioConnections) {
            /// M: CC: Merge to MtkGsmCdmaConnection
            MtkGsmCdmaConnection connection = new MtkGsmCdmaConnection(PhoneConstants.PHONE_TYPE_GSM,
                    null, telecomCallId, null, false, false);
            /// M: ALPS02136977. Sets address first for formatted dump log.
            connection.setAddress(
                    Uri.fromParts(PhoneAccount.SCHEME_TEL, radioConn.getAddress(), null),
                    PhoneConstants.PRESENTATION_ALLOWED);
            /// M: CC: Merge to MtkGsmCdmaConnection
            //newGsmConnections.add(connection);
            newGsmCdmaConnections.add(connection);
            addExistingConnection(handle, connection);
            connection.addTelephonyConnectionListener(mTelephonyConnectionListener);
        }
        for (int i = 0; i < newGsmCdmaConnections.size(); i++) {
            /// M: CC: Merge to MtkGsmCdmaConnection
            //newGsmConnections.get(i).setOriginalConnection(radioConnections.get(i));
            newGsmCdmaConnections.get(i).setOriginalConnection(radioConnections.get(i));
        }
    }

    protected void replaceConference(Conference oldConf, Conference newConf) {
        Log.d(this, "SRVCC: oldConf= %s , newConf= %s", oldConf, newConf);
        if (oldConf == newConf) {
            return;
        }
        if (mIdByConference.containsKey(oldConf)) {
            Log.d(this, "SRVCC: start to do replacement");
            oldConf.removeListener(mConferenceListener);
            String id = mIdByConference.get(oldConf);
            mConferenceById.remove(id);
            mIdByConference.remove(oldConf);
            mConferenceById.put(id, newConf);
            mIdByConference.put(newConf, id);
            newConf.addListener(mConferenceListener);
        }
    }
    /// @}
    /**
     * Check whether IMS ECT can be performed on a certain connection.
     *
     * @param connection The connection to be transferred
     * @return true allowed false disallowed
     * @hide
     */
    public boolean canBlindAssuredTransfer(Connection connection) {
        if (connection == null) {
            Log.d(this, "canBlindAssuredTransfer: connection is null");
            return false;
        }
        if (!(connection instanceof TelephonyConnection)) {
            // the connection may be ConferenceParticipantConnection.
            Log.d(this, "canBlindAssuredTransfer: the connection isn't telephonyConnection");
            return false;
        } else if (((TelephonyConnection) connection).isImsConnection() == false) {
            Log.d(this, "canBlindAssuredTransfer: the connection is not an IMS connection");
            return false;
        } else if (canTransfer(connection)) {
            // We only allow one kind of transfer at same time. If it can execute consultative
            // transfer, then we disable blind/assured transfer capability.
            Log.d(this, "canBlindAssuredTransfer: the connection has consultative ECT capability");
            return false;
        }
        return true;
    }
    /// @}

    /// M: Self activation. @{
    void notifyEccToSelfActivationSM(MtkGsmCdmaPhone phone) {
        Log.d(this, "notifyEccToSelfActivationSM()");
        Bundle extra = new Bundle();
        extra.putInt(ISelfActivation.EXTRA_KEY_MO_CALL_TYPE, ISelfActivation.CALL_TYPE_EMERGENCY);
        phone.getSelfActivationInstance().selfActivationAction(
                ISelfActivation.ACTION_MO_CALL, extra);
    }
    /// @}

    /// M: For multi anchor feature @{
    protected void conference(String callId1, String callId2) {
        Log.d(this, "conference %s, %s", callId1, callId2);

        // Attempt to get second connection or conference.
        Connection connection2 = findConnectionForAction(callId2, "conference");
        Conference conference2 = getNullConference();
        if (connection2 == getNullConnection()) {
            conference2 = findConferenceForAction(callId2, "conference");
            if (conference2 == getNullConference()) {
                Log.w(this, "Connection2 or Conference2 missing in conference request %s.",
                        callId2);
                return;
            }
        }

        // Attempt to get first connection or conference and perform merge.
        Connection connection1 = findConnectionForAction(callId1, "conference");
        if (connection1 == getNullConnection()) {
            Conference conference1 = findConferenceForAction(callId1, "addConnection");
            if (conference1 == getNullConference()) {
                Log.w(this,
                        "Connection1 or Conference1 missing in conference request %s.",
                        callId1);
            } else {
                // Call 1 is a conference.
                if (connection2 != getNullConnection()) {
                    // Call 2 is a connection so merge via call 1 (conference).
                    conference1.onMerge(connection2);
                } else {
                    // Should allow conference merge conference for multi anchor feature.
                    conference1.onMerge();
                    /* Log.wtf(this, "There can only be one conference and an attempt was made to " +
                            "merge two conferences.");
                    return; */
                }
            }
        } else {
            // Call 1 is a connection.
            if (conference2 != getNullConference()) {
                // Call 2 is a conference, so merge via call 2.
                conference2.onMerge(connection1);
            } else {
                // Call 2 is a connection, so merge together.
                onConference(connection1, connection2);
            }
        }
    }

    protected void setConferenceableConnections(String callId, List<String> conferenceableCallIds) {
        mAdapter.setConferenceableConnections(
                callId, conferenceableCallIds);
    }
    /// @}

    @Override
    protected List<String> createIdList(List<Conferenceable> conferenceables) {
        List<String> ids = new ArrayList<>();
        for (Conferenceable c : conferenceables) {
            // Only allow Connection and Conference conferenceables.
            if (c instanceof Connection) {
                Connection connection = (Connection) c;
                if (mIdByConnection.containsKey(connection)) {
                    ids.add(mIdByConnection.get(connection));
                }
            } else if (c instanceof Conference) {
                Conference conference = (Conference) c;
                if (mIdByConference.containsKey(conference)) {
                    ids.add(mIdByConference.get(conference));
                }
            }
        }
        /// M: ALPS04811723. Don't call sort() for conferenceable list. @{
        // When one foreground call merges with one background conference,
        // dialer will find the first call ID to merge in the conferenceable
        // list, but the conference ID may be placed to the end of list
        // after excuting sort(). It will cause onConference() is called
        // instead of onMerge() in ConnectionService.java.
        // Collections.sort(ids);
        /// @}
        return ids;
    }
}

/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.services.telephony;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.telephony.ServiceState;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SubscriptionController;
import com.android.phone.PhoneUtils;
import com.android.services.telephony.DisconnectCauseUtil;
import com.android.services.telephony.ImsConferenceController;
import com.android.services.telephony.Log;
import com.android.services.telephony.TelecomAccountRegistry;
import com.android.services.telephony.TelephonyConnection;
import com.android.services.telephony.TelephonyConnectionService;

import com.android.internal.telephony.gsm.GsmMmiCode;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccController;

import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.List;

/// M: CC: Error message due to CellConnMgr checking @{
import android.content.res.Resources;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.provider.Settings;
import android.telecom.Conference;
import android.telecom.ConnectionRequest;
import android.telecom.PhoneAccountHandle;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;

import com.android.ims.ImsManager;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.TelephonyIntents;
import com.android.phone.R;

import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.imsphone.MtkImsPhone;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkLteDataOnlyController;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkTelephonyProperties;
import com.mediatek.internal.telephony.gsm.MtkGsmMmiCode;
import com.mediatek.internal.telephony.imsphone.MtkImsPhone;
import com.mediatek.internal.telephony.imsphone.MtkImsPhoneMmiCode;
import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;


import com.mediatek.phone.MtkSimErrorDialog;
/// @}

/// M: CC: Emergency mode for Fusion RIL @{
import com.android.internal.telephony.TelephonyDevController;
import com.mediatek.internal.telephony.MtkHardwareConfig;
// Do not enter Emergency Mode for UI ECC (ATD)
import android.telephony.emergency.EmergencyNumber;

/// @}


/// M: CC: Proprietary CRSS handling @{
import com.android.internal.telephony.Connection;

import java.util.concurrent.ConcurrentHashMap;

///@}
import com.mediatek.settings.TelephonyUtils;

/**
 * Service for making GSM and CDMA connections.
 */
public class MtkTelephonyConnectionServiceUtil {

    private static final MtkTelephonyConnectionServiceUtil INSTANCE =
            new MtkTelephonyConnectionServiceUtil();
    private TelephonyConnectionService mService;
    private Context mContext;

    /// M: CC: Proprietary CRSS handling @{
    private MtkSuppMessageManager mSuppMessageManager;
    /// M: ALPS03909160, a HashMap for Phone and it's connections.
    private ConcurrentHashMap<Phone, ArrayList<Connection>> mSuppMsgPhoneConnections;
    /// @}

    /// M: CC: PPL (Phone Privacy Lock Service)
    private final BroadcastReceiver mPplReceiver = new TcsBroadcastReceiver();

    /// M: CC: Error message due to CellConnMgr checking @{
    private int mCurrentDialSubId;
    private int mCurrentDialSlotId;
    private CellConnMgr mCellConnMgr;
    private int mCellConnMgrCurrentRun;
    private int mCellConnMgrTargetRun;
    private int mCellConnMgrState;
    private ArrayList<String> mCellConnMgrStringArray;
    private MtkSimErrorDialog mSimErrorDialog;
    private final BroadcastReceiver mCellConnMgrReceiver = new TcsBroadcastReceiver();
    /// @}

    private static final boolean MTK_CT_VOLTE_SUPPORT
            = "1".equals(SystemProperties.get("persist.vendor.mtk_ct_volte_support", "0"));

    /// M: CC: TDD data only
    private MtkLteDataOnlyController mMtkLteDataOnlyController;

    /// M: CC: ECC retry @{
    private EmergencyRetryHandler mEccRetryHandler;
    private int mEccPhoneType = PhoneConstants.PHONE_TYPE_NONE;
    private int mEccRetryPhoneId = -1;
    private boolean mHasPerformEccRetry = false;
    /// @}

    /// M: CC: Emergency mode for Fusion RIL @{
    private String mEccNumber;
    private boolean mIsInEccMode = false;

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

    MtkTelephonyConnectionServiceUtil() {
        mService = null;
        mContext = null;
        /// M: CC: Proprietary CRSS handling
        mSuppMessageManager = null;

        /// M: CC: Error message due to CellConnMgr checking
        mSimErrorDialog = null;
        /// M: CC: TDD data only
        mMtkLteDataOnlyController = null;
        /// M: CC: ECC retry
        mEccRetryHandler = null;

        /// M: ALPS03909160, init HashMap
        mSuppMsgPhoneConnections = new ConcurrentHashMap<Phone, ArrayList<Connection>>();
    }

    public static MtkTelephonyConnectionServiceUtil getInstance() {
        return INSTANCE;
    }

    public void setService(TelephonyConnectionService s) {
        //Log.d(this, "setService: " + s);
        mService = s;
        mContext = mService.getApplicationContext();

        /// M: CC: CRSS notification
        enableSuppMessage(s);

        /// M: CC: PPL (Phone Privacy Lock Service) @{
        IntentFilter intentFilter = new IntentFilter("com.mediatek.ppl.NOTIFY_LOCK");
        mContext.registerReceiver(mPplReceiver, intentFilter);
        /// @}

        /// M: CC: TDD data only
        mMtkLteDataOnlyController = new MtkLteDataOnlyController(mContext);

        /// M: CC: ECC retry @{
        mEccRetryHandler = null;
        mEccPhoneType = PhoneConstants.PHONE_TYPE_NONE;
        mEccRetryPhoneId = -1;
        mHasPerformEccRetry = false;
        /// @}
    }

    /**
     * unset TelephonyConnectionService to be bind.
     */
    public void unsetService() {
        //Log.d(this, "unSetService: " + mService);
        mService = null;

        /// M: CC: CRSS notification
        disableSuppMessage();

        /// M: CC: PPL (Phone Privacy Lock Service)
        mContext.unregisterReceiver(mPplReceiver);

        /// M: CC: TDD data only
        mMtkLteDataOnlyController = null;

        /// M: CC: ECC retry @{
        mEccRetryHandler = null;
        mEccPhoneType = PhoneConstants.PHONE_TYPE_NONE;
        mEccRetryPhoneId = -1;
        mHasPerformEccRetry = false;
        /// @}
    }

    /// M: CC: Proprietary CRSS handling @{
    /**
     * Register for Supplementary Messages once TelephonyConnection is created.
     * @param cs TelephonyConnectionService
     * @param conn TelephonyConnection
     */
    private void enableSuppMessage(TelephonyConnectionService cs) {
        Log.d(this, "enableSuppMessage for " + cs);
        if (mSuppMessageManager == null) {
            mSuppMessageManager = new MtkSuppMessageManager(cs);
            mSuppMessageManager.registerSuppMessageForPhones();
        }
    }

    /**
     * Unregister for Supplementary Messages  once TelephonyConnectionService is destroyed.
     */
    private void disableSuppMessage() {
        Log.d(this, "disableSuppMessage");
        if (mSuppMessageManager != null) {
            mSuppMessageManager.unregisterSuppMessageForPhones();
            mSuppMessageManager = null;
        }
    }

    /**
     * Force Supplementary Message update once TelephonyConnection is created.
     * @param conn The connection to update supplementary messages.
     */
    public void forceSuppMessageUpdate(TelephonyConnection conn) {
        if (mSuppMessageManager != null) {
            Phone p = conn.getPhone();
            if (p != null) {
                Log.d(this, "forceSuppMessageUpdate for " + conn + ", " + p
                        + " phone " + p.getPhoneId());
                mSuppMessageManager.forceSuppMessageUpdate(conn, p);
            }
        }
    }

    /// M: For VoLTE enhanced conference call. @{
    /**
     * Create a conference connection given an incoming request. This is used to attach to existing
     * incoming calls.
     *
     * @param request Details about the incoming call.
     * @return The {@code GsmConnection} object to satisfy this call, or {@code null} to
     *         not handle the call.
     */
    private android.telecom.Connection createIncomingConferenceHostConnection(
            Phone phone, ConnectionRequest request) {
        Log.v(this, "createIncomingConferenceHostConnection, request: " + request);
        if (mService == null || phone == null) {
            return android.telecom.Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.ERROR_UNSPECIFIED));
        }
        Call call = phone.getRingingCall();
        if (!call.getState().isRinging()) {
            Log.i(this, "onCreateIncomingConferenceHostConnection, no ringing call");
            return android.telecom.Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.INCOMING_MISSED,
                            "Found no ringing call"));
        }
        com.android.internal.telephony.Connection originalConnection =
                call.getState() == Call.State.WAITING ?
                    call.getLatestConnection() : call.getEarliestConnection();
        for (android.telecom.Connection connection : mService.getAllConnections()) {
            if (connection instanceof TelephonyConnection) {
                TelephonyConnection telephonyConnection = (TelephonyConnection) connection;
                if (telephonyConnection.getOriginalConnection() == originalConnection) {
                    Log.i(this, "original connection already registered");
                    return android.telecom.Connection.createCanceledConnection();
                }
            }
        }
        /// M: CC: Merge to MtkGsmCdmaConnection
        //GsmConnection connection = new GsmConnection(originalConnection, null);
        MtkGsmCdmaConnection connection = new MtkGsmCdmaConnection(PhoneConstants.PHONE_TYPE_GSM,
            originalConnection, null, null, false, false);
        return connection;
    }
    /**
     * Create a conference connection given an outgoing request. This is used to initiate new
     * outgoing calls.
     *
     * @param request Details about the outgoing call.
     * @return The {@code GsmConnection} object to satisfy this call, or the result of an invocation
     *         of {@link Connection#createFailedConnection(DisconnectCause)} to not handle the call.
     */
    private android.telecom.Connection createOutgoingConferenceHostConnection(
            Phone phone, final ConnectionRequest request, List<String> numbers) {
        Log.v(this, "createOutgoingConferenceHostConnection, request: " + request);
        if (phone == null) {
            Log.d(this, "createOutgoingConferenceHostConnection, phone is null");
            return android.telecom.Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.OUTGOING_FAILURE, "Phone is null"));
        }
        if (MtkTelephonyConnectionServiceUtil.getInstance().
                cellConnMgrShowAlerting(phone.getSubId())) {
            Log.d(this,
                "createOutgoingConferenceHostConnection, cellConnMgrShowAlerting() check fail");
            return android.telecom.Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            mediatek.telephony.MtkDisconnectCause.OUTGOING_CANCELED_BY_SERVICE,
                                    "cellConnMgrShowAlerting() check fail"));
        }

        if (phone.getPhoneType() != PhoneConstants.PHONE_TYPE_GSM) {
            Log.d(this, "createOutgoingConferenceHostConnection, phone is not GSM Phone");
            return android.telecom.Connection.createFailedConnection(
                    DisconnectCauseUtil.toTelecomDisconnectCause(
                            android.telephony.DisconnectCause.OUTGOING_FAILURE, "Phone not GSM"));
        }

        int state = phone.getServiceState().getState();
        switch (state) {
            case ServiceState.STATE_IN_SERVICE:
            case ServiceState.STATE_EMERGENCY_ONLY:
                break;
            case ServiceState.STATE_OUT_OF_SERVICE:
                return android.telecom.Connection.createFailedConnection(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                android.telephony.DisconnectCause.OUT_OF_SERVICE,
                                "ServiceState.STATE_OUT_OF_SERVICE"));
            case ServiceState.STATE_POWER_OFF:
                return android.telecom.Connection.createFailedConnection(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                android.telephony.DisconnectCause.POWER_OFF,
                                "ServiceState.STATE_POWER_OFF"));
            default:
                Log.d(this, "onCreateOutgoingConnection, unknown service state: %d", state);
                return android.telecom.Connection.createFailedConnection(
                        DisconnectCauseUtil.toTelecomDisconnectCause(
                                android.telephony.DisconnectCause.OUTGOING_FAILURE,
                                "Unknown service state " + state));
        }
        // Don't call createConnectionFor() because we can't add this connection to
        // GsmConferenceController
        /// M: CC: Merge to MtkGsmCdmaConnection
        //GsmConnection connection = new GsmConnection(null, null);
        MtkGsmCdmaConnection connection = new MtkGsmCdmaConnection(PhoneConstants.PHONE_TYPE_GSM,
            null, null, null, false, true);
        connection.setInitializing();
        connection.setVideoState(request.getVideoState());
        PhoneAccountHandle handle = PhoneUtils.makePstnPhoneAccountHandle(phone);
        connection.setManageImsConferenceCallSupported(
                TelecomAccountRegistry.getInstance(mContext).isManageImsConferenceCallSupported(
                        handle));
        placeOutgoingConferenceHostConnection(connection, phone, request, numbers);
        return connection;
    }

    private void placeOutgoingConferenceHostConnection(
            TelephonyConnection connection, Phone phone, ConnectionRequest request,
            List<String> numbers) {
        com.android.internal.telephony.Connection originalConnection = null;
        try {
            if (phone instanceof MtkGsmCdmaPhone) {
                originalConnection =
                        ((MtkGsmCdmaPhone)phone).dial(numbers, request.getVideoState());
            } else {
                Log.d(this, "Phone is not MtkImsPhone");
            }
        } catch (CallStateException e) {
            Log.e(this, e, "placeOutgoingConfHostConnection, phone.dial exception: " + e);
            connection.setDisconnected(DisconnectCauseUtil.toTelecomDisconnectCause(
                    android.telephony.DisconnectCause.OUTGOING_FAILURE,
                    e.getMessage()));
            return;
        }
        if (originalConnection == null) {
            int telephonyDisconnectCause = android.telephony.DisconnectCause.OUTGOING_FAILURE;
            Log.d(this, "placeOutgoingConnection, phone.dial returned null");
            connection.setDisconnected(DisconnectCauseUtil.toTelecomDisconnectCause(
                    telephonyDisconnectCause, "Connection is null"));
        } else {
            connection.setOriginalConnection(originalConnection);
        }
    }

    /**
     * This can be used by telecom to either create a new outgoing conference call or attach
     * to an existing incoming conference call.
     */
    public Conference createConference(
            ImsConferenceController imsConfController,
            Phone phone,
            final ConnectionRequest request,
            final List<String> numbers,
            boolean isIncoming) {
        if (imsConfController == null) {
            return null;
        }
        android.telecom.Connection connection = isIncoming ?
            createIncomingConferenceHostConnection(phone, request)
                : createOutgoingConferenceHostConnection(phone, request, numbers);
        Log.d(this, "onCreateConference, connection: %s", connection);
        if (connection == null) {
            Log.d(this, "onCreateConference, connection: %s");
            return null;
        } else if (connection.getState() ==
                android.telecom.Connection.STATE_DISCONNECTED) {
            Log.d(this, "the host connection is dicsonnected");
            return createFailedConference(connection.getDisconnectCause());
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        //} else if (!(connection instanceof GsmConnection)) {
        } else if (!(connection instanceof MtkGsmCdmaConnection) ||
                ((MtkGsmCdmaConnection) connection).getPhoneType() != PhoneConstants.PHONE_TYPE_GSM) {
        /// @}
            Log.d(this, "abnormal case, the host connection isn't GsmConnection");
            int telephonyDisconnectCause = android.telephony.DisconnectCause.ERROR_UNSPECIFIED;
            connection.setDisconnected(DisconnectCauseUtil.toTelecomDisconnectCause(
                    telephonyDisconnectCause));
            return createFailedConference(telephonyDisconnectCause, "unexpected error");
        } else if (!(imsConfController instanceof ImsConferenceController)) {
            Log.d(this, "abnormal case, not ImsConferenceController");
            int telephonyDisconnectCause = android.telephony.DisconnectCause.ERROR_UNSPECIFIED;
            connection.setDisconnected(DisconnectCauseUtil.toTelecomDisconnectCause(
                    telephonyDisconnectCause));
            return createFailedConference(telephonyDisconnectCause,
                    "Not ImsConferenceController");
        }
        return ((ImsConferenceController)imsConfController).createConference(
                    (TelephonyConnection) connection);
    }

    public Conference createFailedConference(int disconnectCause, String reason) {
        return createFailedConference(
            DisconnectCauseUtil.toTelecomDisconnectCause(disconnectCause, reason));
    }

    public Conference createFailedConference(android.telecom.DisconnectCause disconnectCause) {
        Conference failedConference = new Conference(null) { };
        failedConference.setDisconnected(disconnectCause);
        return failedConference;
    }
    /// @}

    /// IMS SS
    /**
     * Register Supplementary Messages for ImsPhone.
     * @param phone ImsPhone
     */
    public void registerSuppMessageForImsPhone(Phone phone,
                                               com.android.internal.telephony.Connection conn) {
        if (mSuppMessageManager == null) {
            return;
        }

        /// M: ALPS03909160 Phone is registered SuppMessage if and only if it haven't registered.
        /// Also records the connections that is asking to register SuppMessage. @{
        ArrayList<Connection> conns;

        if (!mSuppMsgPhoneConnections.containsKey(phone)) {
            conns = new ArrayList<Connection>();
            conns.add(conn);
            mSuppMsgPhoneConnections.put(phone, conns);
        } else {
            conns = mSuppMsgPhoneConnections.get(phone);

            if (conns.size() == 0) {
                Log.d(this, "registerSuppMessageForImsPhone: error, empty connection list");
                return;
            }

            /// M: ALPS04063916 When IMS Video call merge to conference as participant,
            /// it will add a new TelephonyConnection which have the same original connection,
            /// and old TelephonyConnection will be closed.
            /// So we need to always add the original connection. @{
            conns.add(conn);
            Log.d(this,
                "registerSuppMessageForImsPhone: phone registered, add connection to list");
            return;
            /// @}
        }

        mSuppMessageManager.registerSuppMessageForPhone(phone);
        /// @}
    }
    /**
     * Unregister Supplementary Messages for ImsPhone.
     * @param phone ImsPhone
     */
    public void unregisterSuppMessageForImsPhone(Phone phone,
                                                 com.android.internal.telephony.Connection conn) {
        if (mSuppMessageManager == null) {
            return;
        }

        /// M: ALPS03909160 remove the Phone and connections from hashmap and list and @{
        ArrayList<Connection> conns;

        if (!mSuppMsgPhoneConnections.containsKey(phone)) {
            Log.d(this, "unregisterSuppMessageForImsPhone: error, phone not registered yet");
            return;
        }

        conns = mSuppMsgPhoneConnections.get(phone);

        if (conns.isEmpty()) {
            Log.d(this, "unregisterSuppMessageForImsPhone: error, empty list");
            return;
        }

        if (!conns.contains(conn)) {
            Log.d(this, "unregisterSuppMessageForImsPhone: error, Connection not in list");
            return;
        }

        conns.remove(conn);

        if (conns.isEmpty()) {
            mSuppMsgPhoneConnections.remove(phone);
            mSuppMessageManager.unregisterSuppMessageForPhone(phone);
        }
        /// @}
    }
    /// @}

    /// M: CC: TDD data only @{
    /**
     * check if the phone is in TDD data only mode.
     */
    public boolean isDataOnlyMode(Phone phone) {
        if (mMtkLteDataOnlyController != null && phone != null) {
            if (!mMtkLteDataOnlyController.checkPermission(phone.getSubId())) {
                Log.d(this, "isDataOnlyMode, phoneId=" + phone.getPhoneId()
                        + ", phoneType=" + phone.getPhoneType()
                        + ", dataOnly=true");
                return true;
            }
        }
        return false;
    }
    /// @}

    /// M: CC: ECC retry @{
    public void setEccPhoneType(int phoneType) {
        mEccPhoneType = phoneType;
        Log.d(this, "ECC retry: setEccPhoneType, phoneType=" + phoneType);
    }

    public int getEccPhoneType() {
        return mEccPhoneType;
    }

    public void setEccRetryPhoneId(int phoneId) {
        mEccRetryPhoneId = phoneId;
        Log.d(this, "ECC retry: setEccRetryPhoneId, phoneId=" + phoneId);
    }

    public int getEccRetryPhoneId() {
        return mEccRetryPhoneId;
    }

    public boolean hasPerformEccRetry() {
        return mHasPerformEccRetry;
    }

    public boolean isEccRetryOn() {
        boolean bIsOn = (mEccRetryHandler != null);
        Log.d(this, "ECC retry: isEccRetryOn, retryOn=" + bIsOn);
        return bIsOn;
    }

    /**
     * Save ECC retry requested parameters. Register once ECC is created.
     * @param request connection request
     * @param initPhoneId phone id of the initial ECC
     */
    public void setEccRetryParams(ConnectionRequest request, int initPhoneId) {
        // Check if UE is set to test mode or not (CTA=1, FTA=2, IOT=3, ...)
        // Skip ECC retry for TC26.9.6.2.2
        if (SystemProperties.getInt("vendor.gsm.gcf.testmode", 0) == 2) {
            Log.d(this, "ECC retry: setEccRetryParams, skip for FTA mode");
            return;
        }

        if (TelephonyManager.getDefault().getPhoneCount() <= 1) {
            if (!MTK_CT_VOLTE_SUPPORT) {
                Log.i(this, "ECC retry: setEccRetryParams, skip for SS project");
                return;
            }
        }

        Log.v(this, "ECC retry: setEccRetryParams, request=" + request
                + ", initPhoneId=" + initPhoneId);
        if (mEccRetryHandler == null) {
            mEccRetryHandler = new EmergencyRetryHandler(request, initPhoneId);
        }
    }

    public void clearEccRetryParams() {
        Log.d(this, "ECC retry: clearEccRetryParams");
        mEccRetryHandler = null;
    }

    public void setEccRetryCallId(String id) {
        Log.d(this, "ECC retry: setEccRetryCallId, id=" + id);
        if (mEccRetryHandler != null) {
            mEccRetryHandler.setCallId(id);
        }
    }

    public boolean eccRetryTimeout() {
        boolean bIsTimeout = false;
        if (mEccRetryHandler != null) {
            if (mEccRetryHandler.isTimeout()) {
                mEccRetryHandler = null;
                bIsTimeout = true;
            }
        }
        Log.d(this, "ECC retry: eccRetryTimeout, timeout=" + bIsTimeout);
        return bIsTimeout;
    }

    public void performEccRetry() {
        Log.d(this, "ECC retry: performEccRetry");
        if (mEccRetryHandler != null) {
            mHasPerformEccRetry = true;
            ConnectionRequest retryRequest = new ConnectionRequest(
                    mEccRetryHandler.getNextAccountHandle(),
                    mEccRetryHandler.getRequest().getAddress(),
                    mEccRetryHandler.getRequest().getExtras(),
                    mEccRetryHandler.getRequest().getVideoState());
            mService.createConnectionInternal(mEccRetryHandler.getCallId(), retryRequest);
        }
    }
    /// @}

    /// M: CC: ECC phone selection rule @{
    /**
     * Select the phone by special ECC rule.
     *
     * @param accountHandle The target PhoneAccountHandle.
     * @param number The ecc number.
     */
    public Phone selectPhoneBySpecialEccRule(
            PhoneAccountHandle accountHandle,
            String number, Phone defaultEccPhone) {
        EmergencyRuleHandler eccRuleHandler = null;
        if (getEccRetryPhoneId() != -1) {
            eccRuleHandler = new EmergencyRuleHandler(
                    PhoneUtils.makePstnPhoneAccountHandle(
                            Integer.toString(getEccRetryPhoneId())),
                    number, true, defaultEccPhone);
        } else {
            eccRuleHandler = new EmergencyRuleHandler(
                    accountHandle,
                    number, isEccRetryOn(), defaultEccPhone);
        }
        return eccRuleHandler.getPreferredPhone();
    }
    /// @}

    /// M: CC: Emergency mode for Fusion RIL @{
    public void setEmergencyNumber(String numberToDial) {
        mEccNumber = numberToDial;
    }

    public void enterEmergencyMode(Phone phone, int isAirplane) {
        if (!hasC2kOverImsModem()) {
            return;
        }

        // Do not enter Emergency Mode for UI ECC (ATD)
        // with SIM: 93(true) -> ATD,   91-legacy(true) -> ATD
        // w/o SIM: 93(false) -> ATDE,   91-legacy(true) -> ATD -> MD(ATDE)
        if (mEccNumber == null) {
            return;
        }
        if (phone != null && phone.getEmergencyNumberTracker() != null) {
            if (phone.getEmergencyNumberTracker()
                    .getEmergencyCallRouting(mEccNumber)
                    == EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL) {
                return;
            }
        }

        // TODO: Need to design for DSDS under airplane mode
        // ECM indicates different logic in Modem, set ECM per Modem request as below:
        // Condition to set ECM: let RILD to check
        // - 6M(with C2K) project: only to C2K-enabled phone
        // - 5M(without C2K) project: specific OP such as Vzw
        // Timing to set ECM:
        // - Under flight mode (set in TeleService, use EFUN channel):
        //    - before Radio on and to main capability slot only
        //       - covers SS, DSDS
        // - Not Under flight mode:
        //    - 91-legacy: set in TeleService, use EFUN channel
        //    - 93: set in RILD , use ATD channel
        Log.d(this, "Enter Emergency Mode, airplane mode:" + isAirplane);
        ((MtkGsmCdmaPhone) phone).mMtkCi.setEccMode(mEccNumber, 1, isAirplane,
                phone.isImsRegistered() ? 1 : 0,
                null);
        mIsInEccMode = true;
        mEccNumber = null;
    }

    public void exitEmergencyMode(Phone phone, int isAirplane) {
        if (!hasC2kOverImsModem() || !mIsInEccMode) {
            return;
        }

        Log.d(this, "Exit Emergency Mode, airplane mode:" + isAirplane);
        ((MtkGsmCdmaPhone) phone).mMtkCi.setEccMode("", 0, isAirplane,
                phone.isImsRegistered() ? 1 : 0,
                null);
        mIsInEccMode = false;
    }

    public boolean isInEccMode() {
        return mIsInEccMode;
    }
    /// @}

    /**
      For SIM unplugged, PhoneAccountHandle is null,
      hence TelephonyConnectionService returns OUTGOING_FAILURE,
      without CellConnMgr checking, UI will show "Call not Sent" Google default dialog.
      For SIM plugged, under
      (1) Flight mode on, MTK SimErrorDialog will show FLIGHT MODE string returned by CellConnMgr.
           Only turning off flight mode via notification bar can dismiss the dialog.
      (2) SIM off, MTK SimErrorDialog will show SIM OFF string returned by CellConnMgr.
           Turning on flight mode, or unplugging SIM can dismiss the dialog.
      (3) SIM locked, MTK SimErrorDialog will show SIM LOCKED string returned by CellConnMgr.
           Turning on flight mode, or unplugging SIM can dismiss the dialog.
      */

     /**
      * Listen to intent of Airplane mode and Sim mode.
      * In case of Airplane mode off or Sim Hot Swap, dismiss SimErrorDialog
      */

    private class TcsBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (isInitialStickyBroadcast()) {
                Log.d(this, "Skip initial sticky broadcast");
                return;
            }
            String action = intent.getAction();
            switch (action) {
                /// M: CC: PPL (Phone Privacy Lock Service) @{
                case "com.mediatek.ppl.NOTIFY_LOCK":
                    Log.d(this, "Receives com.mediatek.ppl.NOTIFY_LOCK");
                    for (android.telecom.Connection conn : mService.getAllConnections()) {
                        if (conn instanceof TelephonyConnection) {
                            ((TelephonyConnection)conn).onHangupAll();
                            break;
                        }
                    }
                    break;
                /// @}

                /// M: CC: Error message due to CellConnMgr checking @{
                case Intent.ACTION_AIRPLANE_MODE_CHANGED:
                    Log.d(this, "MtkSimErrorDialog finish due to ACTION_AIRPLANE_MODE_CHANGED");
                    mSimErrorDialog.dismiss();
                    break;
                case TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED:
                    int simState = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                            TelephonyManager.SIM_STATE_UNKNOWN);
                    int slotId = intent.getIntExtra(PhoneConstants.SLOT_KEY,
                            SubscriptionManager.INVALID_SIM_SLOT_INDEX);
                    Log.d(this, "slotId: " + slotId + " simState: " + simState);
                    if ((slotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX) &&
                            (slotId == mCurrentDialSlotId) &&
                            (simState == TelephonyManager.SIM_STATE_ABSENT)) {
                        Log.d(this, "MtkSimErrorDialog finish due hot plug out of SIM " +
                                (slotId + 1));
                        mSimErrorDialog.dismiss();
                    }
                    break;
                /// @}

                default:
                    break;
            }
        }
    }


    /// M: CC: Error message due to CellConnMgr checking @{
    /**
     * register broadcast Receiver.
     */
    private void cellConnMgrRegisterForSubEvent() {
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED);
        mContext.registerReceiver(mCellConnMgrReceiver, intentFilter);

        if (mCurrentDialSlotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            int slotId = mCurrentDialSlotId;
            int simState = MtkTelephonyManagerEx.getDefault().getSimCardState(slotId);
            Log.d(this, "slotId: " + slotId + " simState: " + simState);
            if (simState == TelephonyManager.SIM_STATE_ABSENT) {
                Log.d(this, "MtkSimErrorDialog finish due hot plug out of SIM " + (slotId + 1));
                mSimErrorDialog.dismiss();
            }
        }
    }

    /**
     * unregister broadcast Receiver.
     */
    private void cellConnMgrUnregisterForSubEvent() {
        mContext.unregisterReceiver(mCellConnMgrReceiver);
    }

    public void cellConnMgrSetSimErrorDialogActivity(MtkSimErrorDialog dialog) {
        if (mContext == null) {
            Log.d(this, "cellConnMgrSetSimErrorDialogActivity, mContext is null");
            return;
        }

        if (mSimErrorDialog == dialog) {
            Log.d(this, "cellConnMgrSetSimErrorDialogActivity, skip duplicate");
            return;
        }

        mSimErrorDialog = dialog;
        if (mSimErrorDialog != null) {
            cellConnMgrRegisterForSubEvent();
            Log.d(this, "cellConnMgrRegisterForSubEvent for setSimErrorDialogActivity");
        } else {
            cellConnMgrUnregisterForSubEvent();
            Log.d(this, "cellConnMgrUnregisterForSubEvent for setSimErrorDialogActivity");
        }
    }

    public boolean cellConnMgrShowAlerting(int subId) {
        if (mContext == null) {
            Log.d(this, "cellConnMgrShowAlerting, mContext is null");
            return false;
        }

        if (MtkTelephonyManagerEx.getDefault().isWifiCallingEnabled(subId)) {
            Log.d(this, "cellConnMgrShowAlerting: WiFi calling is enabled, return directly.");
            return false;
        }

        mCellConnMgr = new CellConnMgr(mContext);
        mCurrentDialSubId = subId;
        mCurrentDialSlotId = SubscriptionController.getInstance().getSlotIndex(subId);

        //Step1. Query state by indicated request type,
        //the return value are the combination of current states
        mCellConnMgrState = mCellConnMgr.getCurrentState(mCurrentDialSubId,
                CellConnMgr.STATE_FLIGHT_MODE |
                CellConnMgr.STATE_RADIO_OFF |
                CellConnMgr.STATE_NOIMSREG_FOR_CTVOLTE);

        // check if need to notify user to do something
        // Since UX might change, check the size of mCellConnMgrStringArray to show dialog.
        if (mCellConnMgrState != CellConnMgr.STATE_READY) {

            //Step2. Query string used to show dialog
            mCellConnMgrStringArray = mCellConnMgr.getStringUsingState(
                    mCurrentDialSubId, mCellConnMgrState);
            mCellConnMgrCurrentRun = 0;
            mCellConnMgrTargetRun = mCellConnMgrStringArray.size() / 4;

            Log.d(this, "cellConnMgrShowAlerting, slotId: " + mCurrentDialSlotId +
                " state: " + mCellConnMgrState + " size: " + mCellConnMgrStringArray.size());

            if (mCellConnMgrTargetRun > 0) {
                cellConnMgrShowAlertingInternal();
                return true;
            }
        }
        return false;
    }

    public void cellConnMgrHandleEvent() {

        //Handle the request if user click on positive button
        mCellConnMgr.handleRequest(mCurrentDialSubId, mCellConnMgrState);

        mCellConnMgrCurrentRun++;

        if (mCellConnMgrCurrentRun != mCellConnMgrTargetRun) {
            cellConnMgrShowAlertingInternal();
        } else {
            cellConnMgrShowAlertingFinalize();
        }
    }

    private void cellConnMgrShowAlertingInternal() {

        //Show confirm dialog with returned dialog title,
        //description, negative button and positive button

        ArrayList<String> stringArray = new ArrayList<String>();
        stringArray.add(mCellConnMgrStringArray.get(mCellConnMgrCurrentRun * 4));
        stringArray.add(mCellConnMgrStringArray.get(mCellConnMgrCurrentRun * 4 + 1));
        stringArray.add(mCellConnMgrStringArray.get(mCellConnMgrCurrentRun * 4 + 2));
        stringArray.add(mCellConnMgrStringArray.get(mCellConnMgrCurrentRun * 4 + 3));

        for (int i = 0; i < stringArray.size(); i++) {
            Log.d(this, "cellConnMgrShowAlertingInternal, string(" + i + ")=" +
                    stringArray.get(i));
        }

        // call dialog ...
        Log.d(this, "cellConnMgrShowAlertingInternal");
        /// M: CC: to enable this part when SimErrorDiaglogActivity class migration done @{
//        final Intent intent = new Intent(mContext, SimErrorDialogActivity.class);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
//            | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
//        intent.putStringArrayListExtra(SimErrorDialogActivity.DIALOG_INFORMATION, stringArray);
//        mContext.startActivity(intent);
        /// @}
        if (stringArray.size() < 4) {
            Log.d(this, "cellConnMgrShowAlertingInternal, stringArray is illegle, do nothing.");
            return;
        }
        if (mSimErrorDialog != null) {
            Log.w(this, "cellConnMgrShowAlertingInternal, There's an existing error dialog: "
                    + mSimErrorDialog + ", ignore displaying the new error.");
            return;
        }
        mSimErrorDialog = new MtkSimErrorDialog(mContext, stringArray);
        Log.d(this, "cellConnMgrShowAlertingInternal, show SimErrorDialog: " + mSimErrorDialog);
        mSimErrorDialog.show();
    }

    public void cellConnMgrShowAlertingFinalize() {
        Log.d(this, "cellConnMgrShowAlertingFinalize");
        mCellConnMgrCurrentRun = -1;
        mCellConnMgrTargetRun = 0;
        mCurrentDialSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        mCurrentDialSlotId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
        mCellConnMgrState = -1;
        mCellConnMgr = null;
    }

    public boolean isCellConnMgrAlive() {
        return (mCellConnMgr != null);
    }
    /// @}

    private static class CellConnMgr {
        /**
      * Bit mask: STATE_READY means the card is under ready state.
      *
      * @internal
      */
        public static final int STATE_READY = 0x00;

        /**
      * Bit mask: STATE_FLIGHT_MODE means under flight mode on.
      *
      * @internal
      */
        public static final int STATE_FLIGHT_MODE = 0x01;

        /**
      * Bit mask: STATE_RADIO_OFF means the card is under radio off state.
      *
      * @internal
      */
        public static final int STATE_RADIO_OFF = 0x02;

        /**
     * Bit mask: STATE_NOIMSREG_FOR_CTVOLTE means the CT SIM card is under ims unavailable state
     * when ENHANCED_4G_MODE_ENABLED is enabled.
     */
        public static final int STATE_NOIMSREG_FOR_CTVOLTE = 0x04;


        private Context mContext;
        private static final String INTENT_SET_RADIO_POWER =
                "com.mediatek.internal.telephony.RadioManager.intent.action.FORCE_SET_RADIO_POWER";

        /**
      * To use the utility function, please create the object on your local side.
      *
      * @param context the indicated context
      *
      * @internal
      */
        public CellConnMgr(Context context) {
            mContext = context;

            if (mContext == null) {
                throw new RuntimeException(
                    "CellConnMgr must be created by indicated context");
            }
        }

        /**
      * Query current state by indicated subscription and request type.
      *
      * @param subId indicated subscription
      * @param requestType the request type you cared
      *              STATE_FLIGHT_MODE means that you would like to query if under flight mode.
      *              STATE_RADIO_OFF means that you would like to query if this SIM radio off.
      *              STATE_SIM_LOCKED will check flight mode and radio state first, and then
      *                                             check if under SIM locked state.
      *              STATE_ROAMING will check flight mode and radio state first, and then
      *                                        check if under roaming.
      * @return a bit mask value composed by STATE_FLIGHT_MODE,
      *         STATE_RADIO_OFF, STATE_SIM_LOCKED and  STATE_ROAMING.
      *
      * @internal
      */
        public int getCurrentState(int subId, int requestType) {
            int state = STATE_READY;

            // Query flight mode settings
            int flightMode = Settings.Global.getInt(
                    mContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, -1);

            // Query radio state (need to check if the radio off is set by users)
            boolean isRadioOff = !isRadioOn(subId) && isRadioOffBySimManagement(subId);

            // Query SIM state
            int slotId = SubscriptionManager.getSlotIndex(subId);
            TelephonyManager telephonyMgr = TelephonyManager.getDefault();
            boolean isLocked =
                (TelephonyManager.SIM_STATE_PIN_REQUIRED == telephonyMgr.getSimState(slotId)
                || TelephonyManager.SIM_STATE_PUK_REQUIRED == telephonyMgr.getSimState(slotId)
                || TelephonyManager.SIM_STATE_NETWORK_LOCKED == telephonyMgr.getSimState(slotId));

            // Query roaming state
            boolean isRoaming = false;

            Log.d(this, "[getCurrentState]subId: " + subId + ", requestType:" + requestType +
                    "; (flight mode, radio off, locked, roaming) = ("
                    + flightMode + "," + isRadioOff + "," + isLocked + "," + isRoaming + ")");

            switch (requestType) {
                case STATE_FLIGHT_MODE:
                    state = ((flightMode == 1) ? STATE_FLIGHT_MODE : STATE_READY);
                    break;

                case STATE_RADIO_OFF:
                    state = ((isRadioOff) ? STATE_RADIO_OFF : STATE_READY);
                    break;

                default:
                    state = ((flightMode == 1) ? STATE_FLIGHT_MODE : STATE_READY) |
                            ((isRadioOff) ? STATE_RADIO_OFF : STATE_READY);
            }

            if (state == STATE_READY
                && (requestType & STATE_NOIMSREG_FOR_CTVOLTE) == STATE_NOIMSREG_FOR_CTVOLTE) {
                state =
                    isImsUnavailableForCTVolte(subId) ? STATE_NOIMSREG_FOR_CTVOLTE : STATE_READY;
            }
            Log.d(this, "[getCurrentState] state:" + state);

            return state;
        }

        /**
      * Get dialog showing description, positive button and negative button string by state.
      *
      * @param subId indicated subscription
      * @param state current state query by getCurrentState(int subId, int requestType).
      * @return title, description, positive button and negative strings with following format.
      *         stringList.get(0) = "state1's title"
      *         stringList.get(1) = "state1's description",
      *         stringList.get(2) = "state1's positive buttion"
      *         stringList.get(3) = "state1's negative button"
      *         stringList.get(4) = "state2's title"
      *         stringList.get(5) = "state1's description",
      *         stringList.get(6) = "state1's positive buttion"
      *         stringList.get(7) = "state1's negative button"
      *         A set is composited of four strings.
      *
      * @internal
      */
        public ArrayList<String> getStringUsingState(int subId, int state) {
            ArrayList<String> stringList = new ArrayList<String>();

            Log.d(this, "[getStringUsingState] subId: " + subId + ", state:" + state);

            if ((state & (STATE_FLIGHT_MODE | STATE_RADIO_OFF))
                    == (STATE_FLIGHT_MODE | STATE_RADIO_OFF)) {
                // 0. Turn off flight mode + turn radio on
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_flight_mode_radio_title));
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_flight_mode_radio_msg));
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_button_ok));
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_button_cancel));
                Log.d(this, "[getStringUsingState] STATE_FLIGHT_MODE + STATE_RADIO_OFF");
            } else if ((state & STATE_FLIGHT_MODE) == STATE_FLIGHT_MODE) {
                // 1. Turn off flight mode
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_flight_mode_title));
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_flight_mode_msg));
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_button_turn_off));
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_button_cancel));
                Log.d(this, "[getStringUsingState] STATE_FLIGHT_MODE");
            } else if ((state & STATE_RADIO_OFF) == STATE_RADIO_OFF) {
                // 2. Turn radio on
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_radio_title));
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_radio_msg));
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_button_turn_on));
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_button_cancel));
                Log.d(this, "[getStringUsingState] STATE_RADIO_OFF");
            } else if ((state & STATE_NOIMSREG_FOR_CTVOLTE) == STATE_NOIMSREG_FOR_CTVOLTE) {
                // 4. no imsreg for ct volte
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_noimsreg_for_ctvolte_title));
                stringList.add(mContext.getApplicationContext().getString(
                        R.string.alert_volte_no_service, PhoneUtils.getSubDisplayName(subId)));
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_button_ok));
                stringList.add(Resources.getSystem().getString(
                        com.mediatek.internal.R.string.confirm_button_cancel));
                Log.d(this, "[getStringUsingState] STATE_NOIMSREG_FOR_CTVOLTE");
            }

            Log.d(this, "[getStringUsingState]stringList size: " + stringList.size());

            return ((ArrayList<String>) stringList.clone());
        }

        /**
      * Handle positive button operation by indicated state.
      *
      * @param subId indicated subscription
      * @param state current state query by getCurrentState(int subId, int requestType).
      *
      * @internal
      */
        public void handleRequest(int subId, int state) {

            Log.d(this, "[handleRequest] subId: " + subId + ", state:" + state);

            // 1.Turn off flight mode
            if ((state & STATE_FLIGHT_MODE) == STATE_FLIGHT_MODE) {
                Settings.Global.putInt(
                        mContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, 0);
                mContext.sendBroadcastAsUser(
                        new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED).putExtra("state", false),
                        UserHandle.ALL);

                Log.d(this, "[handleRequest] Turn off flight mode.");
            }

            // 2.Turn radio on
            if ((state & STATE_RADIO_OFF) == STATE_RADIO_OFF) {
                int mSimMode = 0;
                for (int i = 0 ; i < TelephonyManager.getDefault().getSimCount() ; i++) {
                    // TODO: need to revise in case of sub-based modem support
                    int[] targetSubId = SubscriptionManager.getSubId(i);

                    if (((targetSubId != null && isRadioOn(targetSubId[0]))
                            || (i == SubscriptionManager.getSlotIndex(subId)))) {
                        mSimMode = mSimMode | (1 << i);
                    }
                }

                Intent intent = new Intent(INTENT_SET_RADIO_POWER);
                intent.putExtra(TelephonyIntents.EXTRA_MSIM_MODE, mSimMode);
                mContext.sendBroadcastAsUser(intent, UserHandle.ALL);

                Log.d(this, "[handleRequest] Turn radio on, MSIM mode:" + mSimMode);
            }

            // 3. no imsreg for ct volte,disable enhanced 4g mode
            if ((state & STATE_NOIMSREG_FOR_CTVOLTE) == STATE_NOIMSREG_FOR_CTVOLTE) {
                int phoneId = SubscriptionManager.getPhoneId(subId);
                ImsManager.getInstance(mContext, phoneId).setEnhanced4gLteModeSetting(false);
                Log.d(this, "[handleRequest] Turn off ct volte");
            }

        }


        private boolean isRadioOffBySimManagement(int subId) {
            boolean result = true;
            try {
                IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub
                        .asInterface(ServiceManager.getService("phoneEx"));

                if (null == iTelEx) {
                    Log.d(this, "[isRadioOffBySimManagement] iTelEx is null");
                    return false;
                }
                result = iTelEx.isRadioOffBySimManagement(subId);
            } catch (RemoteException ex) {
                ex.printStackTrace();
            }

            Log.d(this, "[isRadioOffBySimManagement]  subId " + subId + ", result = " + result);
            return result;
        }


        private boolean isRadioOn(int subId) {
            Log.d(this, "isRadioOff verify subId " + subId);
            boolean radioOn = true;
            try {
                ITelephony iTel = ITelephony.Stub.asInterface(
                        ServiceManager.getService(Context.TELEPHONY_SERVICE));

                if (null == iTel) {
                    Log.d(this, "isRadioOff iTel is null");
                    return false;
                }

                radioOn = iTel.isRadioOnForSubscriber(subId, mContext.getOpPackageName());
            } catch (RemoteException ex) {
                ex.printStackTrace();
            }

            Log.d(this, "isRadioOff subId " + subId + " radio on? " + radioOn);
            return radioOn;
        }

        private int getNetworkType(int subId) {
            int networkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
            final int dataNetworkType = TelephonyManager.getDefault().getDataNetworkType(subId);
            final int voiceNetworkType = TelephonyManager.getDefault().getVoiceNetworkType(subId);
            Log.d(this, "updateNetworkType(), dataNetworkType = " + dataNetworkType
                    + ", voiceNetworkType = " + voiceNetworkType);
            if (TelephonyManager.NETWORK_TYPE_UNKNOWN != dataNetworkType) {
                networkType = dataNetworkType;
            } else if (TelephonyManager.NETWORK_TYPE_UNKNOWN != voiceNetworkType) {
                networkType = voiceNetworkType;
            }
            return networkType;
        }

        private boolean isInEcbmMode(int phoneId) {
            String ecbmString = TelephonyManager.getTelephonyProperty(phoneId,
                    MtkTelephonyProperties.PROPERTY_INECM_MODE_BY_SLOT, "");
            Log.d(this, "[isInEcbmMode] phoneId = " + phoneId + ", ecbmString = " + ecbmString);
            return "true".equals(ecbmString);
        }

        private boolean isMainPhoneId(int index) {
            int phoneId =
                    SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
            return phoneId == index;
        }

        private boolean isDualVoLTESupport() {
            if (SystemProperties.getInt("persist.vendor.mims_support", 1) != 1) {
                return true;
            }
            return false;
        }

        private boolean isDualCTCard() {
            for (int i = 0; i < TelephonyManager.getDefault().getSimCount(); i++) {
                int[] targetSubId = SubscriptionManager.getSubId(i);
                if (targetSubId == null || !isCTCard(targetSubId[0])) {
                    return false;
                }
            }
            return true;
        }

        private boolean isDataOn(int subId) {
            int defaultDataSubId = SubscriptionManager.getDefaultDataSubscriptionId();
            if (SubscriptionManager.isValidSubscriptionId(defaultDataSubId)
                    && defaultDataSubId == subId ) {
                Log.d(this, "[isDataOn] default data is on it");
                return true;
            }
            return false;
        }

        private boolean isImsUnavailableForCTVolte(int subId) {
            if (MTK_CT_VOLTE_SUPPORT == true  && RatConfiguration.isC2kSupported()) {
                int phoneId = SubscriptionManager.getPhoneId(subId);
                boolean enable4G = true;
                boolean isCTClib = "OP09".equals(
                        SystemProperties.get("persist.vendor.operator.optr"));

                if (isCTClib == false) {
                    int settingNetworkMode = Settings.Global.getInt(
                        mContext.getContentResolver(),
                        Settings.Global.PREFERRED_NETWORK_MODE + subId,
                        Phone.PREFERRED_NT_MODE);
                    enable4G = (
                        settingNetworkMode == TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA
                        || settingNetworkMode == MtkGsmCdmaPhone.NT_MODE_LTE_TDD_ONLY
                        /// M: ALPS04664777 Add LTE network mode @{
                        || settingNetworkMode == TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO);
                        /// @}
                    Log.d(this, "[isImsUnavailableForCTVolte] enable 4g = " + enable4G);
                }

                /// ALPS04302084 Switch data don't trigger main protocol change when VoLTE switch is
                /// on with Dual CT cards. But when secondary sim turn off VoLTE switch, the
                /// main protocol will be trigger.
                if (!isMainPhoneId(phoneId) && isDualCTCard() && !isDataOn(subId)) {
                    Log.d(this, "isImsUnavailableForCTVolte, dual ct case.");
                    return false;
                }

                if (enable4G == true
                        && (isMainPhoneId(phoneId) || isDualVoLTESupport())
                        && isCTCard(subId)
                        && !TelephonyManager.getDefault().isNetworkRoaming(subId)
                        && ImsManager.getInstance(mContext, phoneId)
                                .isEnhanced4gLteModeSettingEnabledByUser()
                        && (isInEcbmMode(phoneId)
                            || getNetworkType(subId) == TelephonyManager.NETWORK_TYPE_LTE
                            || getNetworkType(subId) == TelephonyManager.NETWORK_TYPE_LTE_CA
                            || getNetworkType(subId) == TelephonyManager.NETWORK_TYPE_UNKNOWN)
                        && isImsReg(subId) == false) {
                    Log.d(this, "isImsUnavailableForCTVolte ture");
                    return true;
               }
            }
            return false;
        }

        private boolean isImsReg(int subId) {
            boolean isImsReg = MtkTelephonyManagerEx.getDefault().isImsRegistered(subId);
            Log.d(this, "[isImsReg] isImsReg = " + isImsReg);
            return isImsReg;
        }

        private boolean isCTCard(int subId) {
            TelephonyManager telephonyManager =
                    (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
            String iccid = telephonyManager.getSimSerialNumber(subId);
            if (TextUtils.isEmpty(iccid)) {
                Log.d(this, "[isCTCard] iccid is empty, " + ", subId = " + subId);
                return false;
            }

            if (iccid.startsWith("898603")
                    || iccid.startsWith("898611")
                    || iccid.startsWith("8985302")
                    || iccid.startsWith("8985307")) {
                Log.d(this, "[isCTCard] iccid matches, subId = " + subId);
                return true;
            }
            return false;
        }
    }

    /// M: SS: Error message due to VoLTE SS checking @{
    //--------------[VoLTE_SS] notify user when volte mmi request while data off-------------
    /**
     * This function used to judge whether the dialed mmi needs to be blocked (which needs XCAP)
     * Disallow SS setting/query.
     * @param phone The phone to dial
     * @param number The number to dial
     * @return {@code true} if the number has MMI format to be blocked and {@code false} otherwise.
     */
    private boolean isBlockedMmi(Phone phone, String dialString) {
        boolean isBlockedMmi = false;

        if (PhoneNumberUtils.isUriNumber(dialString)) {
            return false;
        }
        String dialPart = PhoneNumberUtils.extractNetworkPortionAlt(PhoneNumberUtils.
                stripSeparators(dialString));

        if (!((dialPart.startsWith("*") || dialPart.startsWith("#"))
                && dialPart.endsWith("#"))) {
            return false;
        }

        ImsPhone imsPhone = (ImsPhone)phone.getImsPhone();
        boolean imsUseEnabled = phone.isImsUseEnabled()
                 && imsPhone != null
                 && imsPhone.isVolteEnabled()
                 && imsPhone.isUtEnabled()
                 && (imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE);

        if (imsUseEnabled == true) {
            isBlockedMmi = MtkImsPhoneMmiCode.isUtMmiCode(
                    dialPart, imsPhone);
        } else if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
            int slot = SubscriptionController.getInstance().getSlotIndex(phone.getSubId());
            UiccCardApplication cardApp = UiccController.getInstance().
                    getUiccCardApplication(slot, UiccController.APP_FAM_3GPP);
            isBlockedMmi = MtkGsmMmiCode.isUtMmiCode(
                    dialPart, (MtkGsmCdmaPhone) phone, cardApp);
        }
        Log.d(this, "isBlockedMmi = " + isBlockedMmi + ", imsUseEnabled = " + imsUseEnabled);
        return isBlockedMmi;
    }

    /**
     * This function used to check whether we should notify user to open data connection.
     * For now, we judge certain mmi code + "IMS-phoneAccount" + data connection is off.
     * @param number The number to dial
     * @param phone The target phone
     * @return {@code true} if the notification should pop up and {@code false} otherwise.
     */
    public boolean shouldOpenDataConnection(String number,  Phone phone) {

        return (isBlockedMmi(phone, number) &&
                TelephonyUtils.shouldShowOpenMobileDataDialog(mContext, phone.getSubId()));
    }
    /// @}
}

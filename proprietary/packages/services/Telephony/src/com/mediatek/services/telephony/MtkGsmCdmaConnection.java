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

import java.util.LinkedList;
import java.util.Queue;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.imsphone.ImsPhoneConnection;
import com.android.phone.settings.SettingsConstants;
import com.android.services.telephony.DisconnectCauseUtil;
import com.android.services.telephony.EmergencyTonePlayer;
import com.android.services.telephony.Log;
import com.android.services.telephony.TelephonyConnection;
import com.android.services.telephony.TelephonyConnectionService;

import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.telephony.DisconnectCause;
import android.telephony.PhoneNumberUtils;

import mediatek.telecom.MtkConnection;
import com.mediatek.internal.telephony.imsphone.MtkImsPhone;

public class MtkGsmCdmaConnection extends TelephonyConnection {

    // GSM+CDMA
    private int mPhoneType;


    //GSM


    //CDMA
    private static final int MSG_CALL_WAITING_MISSED = 1;
    private static final int MSG_DTMF_SEND_CONFIRMATION = 2;
    private static final int MSG_CDMA_LINE_CONTROL_INFO_REC = 3;
    private static final int TIMEOUT_CALL_WAITING_MILLIS = 20 * 1000;
    /// M: CC: CDMA call fake hold handling. @{
    private static final int MSG_CDMA_CALL_SWITCH = 4;
    private static final int MSG_CDMA_CALL_SWITCH_DELAY = 200;
    private static final int FAKE_HOLD = 1;
    private static final int FAKE_UNHOLD = 0;
    /// @}

    //CDMA
    private final Handler mHandler = new Handler() {

        /** ${inheritDoc} */
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_CALL_WAITING_MISSED:
                    hangupCallWaiting(DisconnectCause.INCOMING_MISSED);
                    break;
                case MSG_DTMF_SEND_CONFIRMATION:
                    handleBurstDtmfConfirmation();
                    break;
                case MSG_CDMA_LINE_CONTROL_INFO_REC:
                    handleCdmaConnectionTimeReset();
                    break;
                /// M: CC: CDMA call fake hold handling. @{
                case MSG_CDMA_CALL_SWITCH:
                    handleFakeHold(msg.arg1);
                    break;
                /// @}
                default:
                    break;
            }
        }

    };

    /**
     * {@code True} if the CDMA connection should allow mute.
     */
    private boolean mAllowMute;
    private final boolean mIsOutgoing;
    // Queue of pending short-DTMF characters.
    private final Queue<Character> mDtmfQueue = new LinkedList<>();
    private final EmergencyTonePlayer mEmergencyTonePlayer;

    // Indicates that the DTMF confirmation from telephony is pending.
    private boolean mDtmfBurstConfirmationPending = false;
    private boolean mIsCallWaiting;
    private boolean mIsConnectionTimeReset = false;

    /// M: CC: CDMA second call @{
    // For CDMA third part call, if the second call is MO call,
    // the state will changed to ACTIVE during force dialing,
    // so need to check if need to update the ACTIVE to telecom.
    private boolean mIsForceDialing = false;
    /// @}

    /// M: CC: CDMA call fake hold handling. @{
    private static final boolean MTK_SVLTE_SUPPORT =
            "1".equals(android.os.SystemProperties.get("ro.vendor.mtk_c2k_lte_mode"));
    /// @}

    public MtkGsmCdmaConnection(
            int phoneType,
            Connection connection,
            String telecomCallId,
            EmergencyTonePlayer emergencyTonePlayer,
            boolean allowMute,
            boolean isOutgoing) {

        // GSM+CDMA
        super(connection, telecomCallId, isOutgoing);
        mPhoneType = phoneType;
        Log.d(this, "MtkGsmCdmaConnection constructor mPhoneType = " + mPhoneType);

        // CDMA
        mEmergencyTonePlayer = emergencyTonePlayer;
        mAllowMute = allowMute;
        mIsOutgoing = isOutgoing;
        mIsCallWaiting = connection != null && connection.getState() == Call.State.WAITING;
        boolean isImsCall = getOriginalConnection() instanceof ImsPhoneConnection;
        // Start call waiting timer for CDMA waiting call.
        if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            if (mIsCallWaiting && !isImsCall) {
                startCallWaitingTimer();
            }
        }
    }

    // GSM+CDMA
    public int getPhoneType() {
        return mPhoneType;
    }

    // GSM+CDMA
    @Override
    public void setOriginalConnection(Connection originalConnection) {
        int oldPhoneType = mPhoneType;
        int origPhoneType = originalConnection.getPhoneType();
        if (origPhoneType == PhoneConstants.PHONE_TYPE_IMS) {
            Phone origPhone = originalConnection.getCall().getPhone();
            mPhoneType = (((ImsPhone) origPhone).getDefaultPhone()).getPhoneType();
            mAllowMute = true;
        } else {
            mPhoneType = origPhoneType;
        }

        Log.d(this, "setOriginalConnection origPhoneType: " + origPhoneType
                + "mPhoneType: " + oldPhoneType + " -> " + mPhoneType);

        super.setOriginalConnection(originalConnection);
        // CDMA
        mIsCallWaiting = originalConnection != null &&
                originalConnection.getState() == Call.State.WAITING;
        boolean isImsCall = getOriginalConnection() instanceof ImsPhoneConnection;
        // Start call waiting timer for CDMA waiting call.
        if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            if (mIsCallWaiting && mHandler != null && !isImsCall) {
                startCallWaitingTimer();
            }
        }

        if (getPhone() != null) {
            getPhone().registerForLineControlInfo(mHandler, MSG_CDMA_LINE_CONTROL_INFO_REC, null);
        }

        // Emergency signal tone is only for CDMA, stop if phone type changed
        if (mEmergencyTonePlayer != null && mPhoneType != PhoneConstants.PHONE_TYPE_CDMA) {
            mEmergencyTonePlayer.stop();
        }
    }

    // GSM+CDMA
    /**
     * Clones the current {@link MtkGsmCdmaConnection}.
     * <p>
     * Listeners are not copied to the new instance.
     *
     * @return The cloned connection.
     */
    @Override
    public TelephonyConnection cloneConnection() {
        MtkGsmCdmaConnection gsmCdmaConnection = new MtkGsmCdmaConnection(
                mPhoneType,
                getOriginalConnection(),
                getTelecomCallId(),
                mEmergencyTonePlayer, mAllowMute, mIsOutgoing);
        return gsmCdmaConnection;
    }

    // GSM+CDMA
    /** {@inheritDoc} */
    @Override
    public void onPlayDtmfTone(char digit) {
        if (mPhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            if (getPhone() != null) {
                getPhone().startDtmf(digit);
                /// M: CC: DTMF request special handling @{
                // Start DTMF
                mDtmfRequestIsStarted = true;
                /// @}
            }
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            /// M: CC: error handling @{
            if (getPhone() == null) {
                return;
            }
            /// @}
            if (useBurstDtmf()) {
                Log.i(this, "sending dtmf digit as burst");
                sendShortDtmfToNetwork(digit);
            } else {
                Log.i(this, "sending dtmf digit directly");
                getPhone().startDtmf(digit);
            }
        }
    }

    // GSM+CDMA
    /** {@inheritDoc} */
    @Override
    public void onStopDtmfTone() {
        if (mPhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            if (getPhone() != null) {
                getPhone().stopDtmf();
                /// M: CC: DTMF request special handling @{
                // Stop DTMF when TelephonyConnection is disconnected
                mDtmfRequestIsStarted = false;
                /// @}
            }
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            /// M: CC: error handling @{
            if (getPhone() == null) {
                return;
            }
            /// @}
            if (!useBurstDtmf()) {
                getPhone().stopDtmf();
            }
        }
    }

    // GSM+CDMA
    @Override
    protected int buildConnectionProperties() {
        int properties = super.buildConnectionProperties();
        // PROPERTY_IS_DOWNGRADED_CONFERENCE is permanent on GSM connections -- once it is set, it
        // should be retained.
        if (mPhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            if ((getConnectionProperties() & PROPERTY_IS_DOWNGRADED_CONFERENCE) != 0) {
                properties |= PROPERTY_IS_DOWNGRADED_CONFERENCE;
            }
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA && !isImsConnection()) {
            properties |= MtkConnection.PROPERTY_CDMA;
        }
        return properties;
    }

    // GSM+CDMA
    @Override
    protected int buildConnectionCapabilities() {
        int capabilities = super.buildConnectionCapabilities();
        if (mPhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            capabilities |= CAPABILITY_MUTE;
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            if (mAllowMute) {
                capabilities |= CAPABILITY_MUTE;
            }
        }

        if (mPhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            // Overwrites TelephonyConnection.buildConnectionCapabilities() and resets the hold options
            // because all GSM calls should hold, even if the carrier config option is set to not show
            // hold for IMS calls.
            if (!shouldTreatAsEmergencyCall()) {
                capabilities |= CAPABILITY_SUPPORT_HOLD;
                if (isHoldable() && (getState() == STATE_ACTIVE || getState() == STATE_HOLDING)) {
                    capabilities |= CAPABILITY_HOLD;
                }
            }

            /// M: CC: Interface for ECT @{
            if (getConnectionService() != null) {
                TelephonyConnectionService cnService =
                        (TelephonyConnectionService) getConnectionService();

                Phone phone = null;
                if (mOriginalConnection != null && mOriginalConnection.getCall() != null) {
                    phone = mOriginalConnection.getCall().getPhone();
                }
                boolean isBlindAssuredEctSupported = false;
                boolean isConsultativeEctSupported = true;

                if (phone != null && phone instanceof MtkImsPhone) {
                    MtkImsPhone imsPhone = (MtkImsPhone)phone;
                    isBlindAssuredEctSupported =
                            imsPhone.isFeatureSupported(MtkImsPhone.FeatureType.BLINDASSURED_ECT);
                    isConsultativeEctSupported =
                            imsPhone.isFeatureSupported(MtkImsPhone.FeatureType.CONSULTATIVE_ECT);
                }

                if (cnService.canTransfer(this) && isConsultativeEctSupported) {
                    capabilities |= MtkConnection.CAPABILITY_CONSULTATIVE_ECT;
                }

                if (cnService.canBlindAssuredTransfer(this) && isBlindAssuredEctSupported) {
                    if (getState() == STATE_ACTIVE && !shouldTreatAsEmergencyCall()) {
                        capabilities |= MtkConnection.CAPABILITY_BLIND_OR_ASSURED_ECT;
                    }
                }
            }
            /// @}
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            /// M: CC: CDMA call fake hold handling. @{
            if (MTK_SVLTE_SUPPORT && !isImsConnection()) {
                boolean isRealConnected = false;
                Connection origConn = getOriginalConnection();
                if ((origConn instanceof com.mediatek.internal.telephony.MtkGsmCdmaConnection)
                        && (com.android.internal.telephony.PhoneConstants.PHONE_TYPE_CDMA
                        == origConn.getPhoneType())) {
                    com.mediatek.internal.telephony.MtkGsmCdmaConnection conn =
                            (com.mediatek.internal.telephony.MtkGsmCdmaConnection) origConn;
                    isRealConnected = conn.isRealConnected();
                } else {
                    isRealConnected = (getState() == STATE_ACTIVE);
                }
                Log.d(this, "buildConnectionCapabilities, origConn=" + origConn
                        + ", isRealConnected=" + isRealConnected);
                if (!shouldTreatAsEmergencyCall()) {
                    capabilities |= CAPABILITY_SUPPORT_HOLD;
                    if ((getState() == STATE_ACTIVE
                            && ((mIsOutgoing && isRealConnected) || !mIsOutgoing))
                            || getState() == STATE_HOLDING) {
                        capabilities |= CAPABILITY_HOLD;
                    }
                }
            }
            /// @}
        }
        Log.d(this, "buildConnectionCapabilities: "
                + MtkConnection.capabilitiesToString(capabilities));

        return capabilities;
    }

    // GSM+CDMA
    @Override
    protected void onRemovedFromCallService() {
        super.onRemovedFromCallService();
    }

    // CDMA
    @Override
    public void onReject() {
        if (mPhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            super.onReject();
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            Connection connection = getOriginalConnection();
            if (connection != null) {
                switch (connection.getState()) {
                    case INCOMING:
                        // Normal ringing calls are handled the generic way.
                        super.onReject();
                        break;
                    case WAITING:
                        hangupCallWaiting(DisconnectCause.INCOMING_REJECTED);
                        break;
                    default:
                        Log.e(this, new Exception(), "Rejecting a non-ringing call");
                        // might as well hang this up, too.
                        super.onReject();
                        break;
                }
            }
        }
    }

    // CDMA
    @Override
    public void onAnswer() {
        if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            mHandler.removeMessages(MSG_CALL_WAITING_MISSED);
        }
        super.onAnswer();
    }

    // CDMA
    @Override
    public void onStateChanged(int state) {
        if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            Connection originalConnection = getOriginalConnection();
            mIsCallWaiting = originalConnection != null &&
                    originalConnection.getState() == Call.State.WAITING;

            if (mEmergencyTonePlayer != null) {
                if (state == android.telecom.Connection.STATE_DIALING) {
                    if (isEmergency()) {
                        mEmergencyTonePlayer.start();
                    }
                } else {
                    // No need to check if it is an emergency call, since it is a no-op if it
                    // isn't started.
                    mEmergencyTonePlayer.stop();
                }
            }
        }
        super.onStateChanged(state);
    }

    // CDMA
    @Override
    public void performConference(android.telecom.Connection otherConnection) {
        if (mPhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            super.performConference(otherConnection);
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            if (isImsConnection()) {
                super.performConference(otherConnection);
            } else {
                Log.w(this, "Non-IMS CDMA Connection attempted to call performConference.");
            }
        }
    }

    // CDMA
    public void forceAsDialing(boolean isDialing) {
        if (isDialing) {
            setStateOverride(Call.State.DIALING);
            /// M: CC: CDMA second call. @{
            mIsForceDialing = true;
            /// @}
        } else {
            /// M: CC: CDMA second call. @{
            mIsForceDialing = false;
            /// @}
            resetStateOverride();
        }
    }

    // CDMA
    public boolean isOutgoing() {
        return mIsOutgoing;
    }

    // CDMA
    public boolean isCallWaiting() {
        return mIsCallWaiting;
    }

    // CDMA
    /**
     * We do not get much in the way of confirmation for Cdma call waiting calls. There is no
     * indication that a rejected call succeeded, a call waiting call has stopped. Instead we
     * simulate this for the user. We allow TIMEOUT_CALL_WAITING_MILLIS milliseconds before we
     * assume that the call was missed and reject it ourselves. reject the call automatically.
     */
    private void startCallWaitingTimer() {
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        mHandler.removeMessages(MSG_CALL_WAITING_MISSED);
        //// @}
        mHandler.sendEmptyMessageDelayed(MSG_CALL_WAITING_MISSED, TIMEOUT_CALL_WAITING_MILLIS);
    }

    // CDMA
    private void hangupCallWaiting(int telephonyDisconnectCause) {
        Connection originalConnection = getOriginalConnection();
        if (originalConnection != null) {
            try {
                originalConnection.hangup();
            } catch (CallStateException e) {
                Log.e(this, e, "Failed to hangup call waiting call");
            }
            setDisconnected(DisconnectCauseUtil.toTelecomDisconnectCause(telephonyDisconnectCause,
                    null, getPhone().getPhoneId()));

        }
    }

    // CDMA
    /**
     * Read the settings to determine which type of DTMF method this CDMA phone calls.
     */
    private boolean useBurstDtmf() {
        if (isImsConnection()) {
            Log.d(this, "in ims call, return false");
            return false;
        }
        int dtmfTypeSetting = Settings.System.getInt(
                getPhone().getContext().getContentResolver(),
                Settings.System.DTMF_TONE_TYPE_WHEN_DIALING,
                SettingsConstants.DTMF_TONE_TYPE_NORMAL);
        return dtmfTypeSetting == SettingsConstants.DTMF_TONE_TYPE_NORMAL;
    }

    // CDMA
    private void sendShortDtmfToNetwork(char digit) {
        synchronized (mDtmfQueue) {
            if (mDtmfBurstConfirmationPending) {
                mDtmfQueue.add(new Character(digit));
            } else {
                sendBurstDtmfStringLocked(Character.toString(digit));
            }
        }
    }

    // CDMA
    private void sendBurstDtmfStringLocked(String dtmfString) {
        /// M: Add null check to avoid timing issue. @{
        Phone phone = getPhone();
        if (phone != null) {
            phone.sendBurstDtmf(
                    dtmfString, 0, 0, mHandler.obtainMessage(MSG_DTMF_SEND_CONFIRMATION));
            mDtmfBurstConfirmationPending = true;
        }
        /// @}
    }

    // CDMA
    private void handleBurstDtmfConfirmation() {
        String dtmfDigits = null;
        synchronized (mDtmfQueue) {
            mDtmfBurstConfirmationPending = false;
            if (!mDtmfQueue.isEmpty()) {
                StringBuilder builder = new StringBuilder(mDtmfQueue.size());
                while (!mDtmfQueue.isEmpty()) {
                    builder.append(mDtmfQueue.poll());
                }
                dtmfDigits = builder.toString();

                // It would be nice to log the digit, but since DTMF digits can be passwords
                // to things, or other secure account numbers, we want to keep it away from
                // the logs.
                Log.i(this, "%d dtmf character[s] removed from the queue", dtmfDigits.length());
            }
            if (dtmfDigits != null) {
                sendBurstDtmfStringLocked(dtmfDigits);
            }
        }
    }

    // CDMA
    private boolean isEmergency() {
        Phone phone = getPhone();
        /// M: CC: Add null check. @{
        // Address could be null because when creating unknown connection onStateChanged could be
        // invoked earlier than setAddress.
        return phone != null && getAddress() != null &&
                PhoneNumberUtils.isLocalEmergencyNumber(
                    phone.getContext(), getAddress().getSchemeSpecificPart());
        /// @}
    }

    // CDMA
    /**
     * Called when ECM mode is exited; set the connection to allow mute and update the connection
     * capabilities.
     */
    @Override
    protected void handleExitedEcmMode() {
        // We allow mute upon existing ECM mode and rebuild the capabilities.
        mAllowMute = true;
        super.handleExitedEcmMode();
    }

    private void handleCdmaConnectionTimeReset() {
        boolean isImsCall = getOriginalConnection() instanceof ImsPhoneConnection;
        if (!isImsCall && !mIsConnectionTimeReset && mIsOutgoing
                && getOriginalConnection() != null
                && getOriginalConnection().getState() == Call.State.ACTIVE
                && getOriginalConnection().getDurationMillis() > 0) {
            mIsConnectionTimeReset = true;
            getOriginalConnection().resetConnectionTime();
            resetConnectionTime();
        }
    }

    @Override
    protected void close() {
        mIsConnectionTimeReset = false;
        if (getPhone() != null) {
            getPhone().unregisterForLineControlInfo(mHandler);
        }
        super.close();
    }

    @Override
    protected void setActiveInternal() {
        if (getState() == STATE_ACTIVE) {
            Log.w(this, "Should not be called if this is already ACTIVE");
            return;
        }

        // When we set a call to active, we need to make sure that there are no other active
        // calls. However, the ordering of state updates to connections can be non-deterministic
        // since all connections register for state changes on the phone independently.
        // To "optimize", we check here to see if there already exists any active calls.  If so,
        // we issue an update for those calls first to make sure we only have one top-level
        // active call.
        if (getConnectionService() != null) {
            for (android.telecom.Connection current : getConnectionService().getAllConnections()) {
                if (current != this && current instanceof MtkGsmCdmaConnection) {
                    MtkGsmCdmaConnection other = (MtkGsmCdmaConnection) current;
                    if (other.getState() == STATE_ACTIVE) {
                        other.updateState();
                    }
                }
            }
        }

        /// M: CC: CDMA second call. @{
        // If the CDMA connection is in FORCE DIALING status,
        // not update the state to ACTIVE, this will be done
        // after stop the FORCE DIALING
        if (mIsForceDialing) {
            return;
        }
        /// @}

        setActive();

        /// M: CC: CDMA call accepted
        // [ALPS03609946][ALPS03800348] Ensure there's CDMA call accept notification after active @{
        if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA && !isImsConnection()
                && getOriginalConnection() != null
                && getOriginalConnection()
                        instanceof com.mediatek.internal.telephony.MtkGsmCdmaConnection) {
            com.mediatek.internal.telephony.MtkGsmCdmaConnection c =
                    (com.mediatek.internal.telephony.MtkGsmCdmaConnection) getOriginalConnection();
            if (!mIsConnectionTimeReset && mIsOutgoing && c.isRealConnected()) {
                mIsConnectionTimeReset = true;
                resetConnectionTime();
            }
        }
        /// @}
    }

    // CDMA
    @Override
    public void performHold() {
        if (mPhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            super.performHold();
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            /// M: CDMA call fake hold handling. @{
            // AOSP doesn't support hold (no HOLD capability & UI) for pure CDMA call,
            // but for IMS CDMA call, HOLD capability & UI is supported
            if (MTK_SVLTE_SUPPORT && !isImsConnection()) {
                Log.d(this, "performHold, just set the hold status.");
                mHandler.sendMessageDelayed(
                        Message.obtain(mHandler, MSG_CDMA_CALL_SWITCH, FAKE_HOLD, 0),
                        MSG_CDMA_CALL_SWITCH_DELAY);
            } else {
                super.performHold();
            }
            /// @}
        }
    }

    // CDMA
    @Override
    public void performUnhold() {
        if (mPhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            super.performUnhold();
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            /// M: CDMA call fake hold handling. @{
            // AOSP doesn't support hold (no HOLD capability & UI) for pure CDMA call,
            // but for IMS CDMA call, HOLD capability & UI is supported
            if (MTK_SVLTE_SUPPORT && !isImsConnection()) {
                Log.d(this, "performUnhold, just set the active status.");
                mHandler.sendMessageDelayed(
                        Message.obtain(mHandler, MSG_CDMA_CALL_SWITCH, FAKE_UNHOLD, 0),
                        MSG_CDMA_CALL_SWITCH_DELAY);
            } else {
                super.performUnhold();
            }
            /// @}
        }
    }

    // CDMA
    /// M: CC: CDMA call fake hold handling. @{
    private void handleFakeHold(int fakeOp) {
        Log.d(this, "handleFakeHold, operation=", fakeOp);
        if (FAKE_HOLD == fakeOp) {
            setOnHold();
        } else if (FAKE_UNHOLD == fakeOp) {
            setActive();
        }
        updateState();
    }
    /// @}
}

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

package com.mediatek.internal.telephony;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.SystemProperties;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
import android.telephony.DisconnectCause;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.emergency.EmergencyNumber;
import android.text.TextUtils;
import mediatek.telephony.MtkCarrierConfigManager;
import android.telecom.VideoProfile;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.DriverCall;
import com.android.internal.telephony.GsmCdmaCall;
import com.android.internal.telephony.GsmCdmaCallTracker;
import com.android.internal.telephony.GsmCdmaConnection;
import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyDevController;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.UUSInfo;
import com.android.internal.telephony.cdma.CdmaCallWaitingNotification;

import com.mediatek.internal.telephony.cdma.pluscode.PlusCodeProcessor;
import com.mediatek.internal.telephony.imsphone.MtkImsPhoneConnection;

import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class MtkGsmCdmaCallTracker extends GsmCdmaCallTracker {

    private static final String PROP_LOG_TAG = "GsmCdmaCallTkr";
    public MtkRIL mMtkCi = null;

    protected static final int EVENT_MTK_BASE                      = 1000;
    /// M: CC: Proprietary incoming call handling
    protected static final int EVENT_INCOMING_CALL_INDICATION      = EVENT_MTK_BASE + 0;

    /// M: CC: Modem reset related handling
    protected static final int EVENT_RADIO_OFF_OR_NOT_AVAILABLE    = EVENT_MTK_BASE + 1;

    protected static final int EVENT_DIAL_CALL_RESULT              = EVENT_MTK_BASE + 2;

    /// M: CC: Hangup special handling @{
    protected static final int EVENT_HANG_UP_RESULT                = EVENT_MTK_BASE + 3;
    /// @}
    /// M: CC: CDMA call accepted @{
    protected static final int EVENT_CDMA_CALL_ACCEPTED            = EVENT_MTK_BASE + 4;
    /// @}

    // IMS conference call feature
    protected static final int EVENT_ECONF_SRVCC_INDICATION        = EVENT_MTK_BASE + 5;

    /// M: CC: Proprietary incoming call handling
    protected static final int EVENT_CALL_ADDITIONAL_INFO          = EVENT_MTK_BASE + 6;

    // M: CC: CDMA call waiting
    private static final int MIN_CONNECTIONS_IN_CDMA_CONFERENCE = 2;

    private OpTelephonyCustomizationFactoryBase mTelephonyCustomizationFactory = null;
    protected IMtkGsmCdmaCallTrackerExt mMtkGsmCdmaCallTrackerExt = null;

    /// M: CC: Proprietary CRSS handling @{
    boolean mHasPendingSwapRequest = false;
    WaitForHoldToRedial mWaitForHoldToRedialRequest = new WaitForHoldToRedial();
    WaitForHoldToHangup mWaitForHoldToHangupRequest = new WaitForHoldToHangup();
    /// @}

    /// M: CC: Use MtkGsmCdmaCallTrackerHelper @{
    public MtkGsmCdmaCallTrackerHelper mHelper;

    /// M: CC: Record current phone type
    private int mPhoneType = PhoneConstants.PHONE_TYPE_NONE;

    /// M: ALPS03487233 @{
    /* This flag is used to indicate that all calls disconnected but not handled for
       some corner cases */
    private boolean mHasPendingUpdatePhoneType = false;
    /// @}

    /// M: Indicate that phone type has changed from CDMA to GSM, but data is not checked yet
    private boolean mHasPendingCheckAndEnableData = false;

    /// M: CC: Vzw/CTVolte ECC for Fusion RIL @{
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

    // Declare as public for MtkGsmCdmaCallTrackerHelper to use
    public int getMaxConnections() {
        return mPhone.isPhoneTypeGsm() ?
                MAX_CONNECTIONS_GSM :
                MAX_CONNECTIONS_CDMA;
    }
    /// @}

    ///M: For IMS conference SRVCC @{
    // ALPS02019630
    /**
     * For conference host side, need to wait until ECONFSRVCC URC recieved then all participant's
     * address could be known by this URC and the conference XML file.
     */
    protected boolean mNeedWaitImsEConfSrvcc = false;

    /**
     * Identify the host connection for conference host side SRVCC.
     */
    protected Connection mImsConfHostConnection = null;
    private ArrayList<Connection> mImsConfParticipants = new ArrayList<Connection>();

    // for SRVCC purpose, put conference connection Ids temporarily
    private int[] mEconfSrvccConnectionIds = null;
    /// @}

    /// M: CC: Proprietary CRSS handling @{
    class WaitForHoldToRedial {

        private boolean mWaitToRedial = false;
        private String mDialString = null;
        private int mClirMode = 0;
        private UUSInfo mUUSInfo = null;
        private boolean mIsEmergencyCall = false;
        private EmergencyNumber mEmergencyNumberInfo;
        private boolean mHasKnownUserIntentEmergency = false;

        WaitForHoldToRedial() {
            resetToRedial();
        }

        boolean isWaitToRedial() {
            return mWaitToRedial;
        }

        void setToRedial() {
            mWaitToRedial = true;
        }

        public void setToRedial(String dialSting, boolean isEmergencyCall,
                EmergencyNumber emergencyNumberInfo, boolean hasKnownUserIntentEmergency,
                int clir, UUSInfo uusinfo) {
            mWaitToRedial = true;
            mDialString = dialSting;
            mIsEmergencyCall = isEmergencyCall;
            mEmergencyNumberInfo = emergencyNumberInfo;
            mHasKnownUserIntentEmergency = hasKnownUserIntentEmergency;
            mClirMode = clir;
            mUUSInfo = uusinfo;
        }

        public void resetToRedial() {

            Rlog.d(PROP_LOG_TAG, "Reset mWaitForHoldToRedialRequest variables");

            mWaitToRedial = false;
            mDialString = null;
            mClirMode = 0;
            mUUSInfo = null;
            mIsEmergencyCall = false;
            mHasKnownUserIntentEmergency = false;
        }

        /**
         * Check if there is another action need to be performed after holding request is done.
         *
         * @return Return true if there exists action need to be perform, else return false.
         */
        // Dial -> Switch since ACTIVE call exists -> queue Dial -> Switch done -> resume Dial
        private boolean resumeDialAfterHold() {
            Rlog.d(PROP_LOG_TAG, "resumeDialAfterHold begin");

            if (mWaitToRedial) {
                mCi.dial(mDialString, mIsEmergencyCall, mEmergencyNumberInfo,
                        mHasKnownUserIntentEmergency,
                        mClirMode, mUUSInfo, obtainCompleteMessage(EVENT_DIAL_CALL_RESULT));
                resetToRedial();
                Rlog.d(PROP_LOG_TAG, "resumeDialAfterHold end");
                return true;
            }
            return false;
        }
    }

    // Switch -> queue Hangup -> Switch done -> poll call -> poll call done -> resume Hangup
    // Switch -> Switch done -> poll call -> queue Hangup -> poll call done -> resume Hangup
    // Switch -> Switch done -> poll call -> poll call done -> resume Hangup (X)
    class WaitForHoldToHangup {

        private boolean mWaitToHangup = false;
        private boolean mHoldDone = false;
        private GsmCdmaCall mCall = null;

        WaitForHoldToHangup() {
            resetToHangup();
        }

        boolean isWaitToHangup() {
            return mWaitToHangup;
        }

        boolean isHoldDone() {
            return mHoldDone;
        }

        void setHoldDone() {
            mHoldDone = true;
        }

        void setToHangup() {
            mWaitToHangup = true;
        }

        public void setToHangup(GsmCdmaCall call) {
            mWaitToHangup = true;
            mCall = call;
        }

        public void resetToHangup() {

            Rlog.d(PROP_LOG_TAG, "Reset mWaitForHoldToHangupRequest variables");

            mWaitToHangup = false;
            mHoldDone = false;
            mCall = null;
        }

        /**
         * Check if there is another action need to be performed after holding request is done.
         *
         * @return Return true if there exists action need to be perform, else return false.
         */
        private boolean resumeHangupAfterHold() {
            Rlog.d(PROP_LOG_TAG, "resumeHangupAfterHold begin");

            if (mWaitToHangup) {
                if (mCall != null) {
                    Rlog.d(PROP_LOG_TAG, "resumeHangupAfterHold to hangup call");
                    mWaitToHangup = false;
                    mHoldDone = false;
                    try {
                        hangup(mCall);
                    } catch (CallStateException ex) {
                        ex.printStackTrace();
                        Rlog.e(PROP_LOG_TAG,
                                "unexpected error on hangup (" + ex.getMessage() + ")");
                    }
                    Rlog.d(PROP_LOG_TAG, "resumeHangupAfterHold end");
                    mCall = null;
                    return true;
                }
            }
            resetToHangup();
            return false;
        }

    }
    /// @}

    public MtkGsmCdmaCallTracker(GsmCdmaPhone phone) {
        super(phone);

        mRingingCall = new MtkGsmCdmaCall(this);
        // A call that is ringing or (call) waiting
        mForegroundCall = new MtkGsmCdmaCall(this);
        mBackgroundCall = new MtkGsmCdmaCall(this);

        mMtkCi = (MtkRIL)mCi;
        /// M: CC: Proprietary incoming call handling
        mMtkCi.setOnIncomingCallIndication(this, EVENT_INCOMING_CALL_INDICATION, null);
        mMtkCi.registerForCallAdditionalInfo(this, EVENT_CALL_ADDITIONAL_INFO, null);
        /// M: CC: Modem reset related handling
        mMtkCi.registerForOffOrNotAvailable(this, EVENT_RADIO_OFF_OR_NOT_AVAILABLE, null);

        /// M: CC: Use MtkGsmCdmaCallTrackerHelper @{
        mHelper = new MtkGsmCdmaCallTrackerHelper(phone.getContext(), this);
        /// @}

        // For IMS conference SRVCC
        mMtkCi.registerForEconfSrvcc(this, EVENT_ECONF_SRVCC_INDICATION, null);

        try {
            mTelephonyCustomizationFactory =
                    OpTelephonyCustomizationUtils.getOpFactory(mPhone.getContext());
            mMtkGsmCdmaCallTrackerExt =
                    mTelephonyCustomizationFactory.makeMtkGsmCdmaCallTrackerExt(phone.getContext());
        } catch (Exception e) {
            Rlog.d(LOG_TAG, "mMtkGsmCdmaCallTrackerExt init fail");
            e.printStackTrace();
        }
    }

    @Override
    protected void updatePhoneType(boolean duringInit) {
        updatePhoneType(duringInit, false);
    }

    private void updatePhoneType(boolean duringInit, boolean duringPollCallsResult) {
        /// M: CC: Don't update phone type when phone type is still CDMA @{
        if ((mPhoneType == PhoneConstants.PHONE_TYPE_CDMA)
                && !mPhone.isPhoneTypeGsm()) {
            return;
        }
        /// @}

        if (!duringInit) {
            /// M: ALPS03487233 @{
            /* UNSOL_VOICE_RADIO_TECH_CHANGED received after call disconnected, and handled
               before poll call result returned, so the phone state is changed to idle.
               We need to update phone state to upper layer before update phone type. */
            if (!duringPollCallsResult && mState != PhoneConstants.State.IDLE) {
                mHasPendingUpdatePhoneType = true;
                Rlog.d(LOG_TAG, "[updatePhoneType]mHasPendingUpdatePhoneType = true");
                /// M: When phone type changing from CDMA to GSM, need to check and enable data. @{
                if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA && mPhone.isPhoneTypeGsm()) {
                    mHasPendingCheckAndEnableData = true;
                }
                if (mLastRelevantPoll == null) {
                    pollCallsWhenSafe();
                }
                return;
            }
            /// @}

            reset();

            /// M: CC: Vzw/CTVolte ECC for Fusion RIL @{
            // If there is call exist, it may cause error
            // because mConnections is re-created in following code
            //pollCallsWhenSafe();
            if (hasC2kOverImsModem()) {
                if (VDBG) Rlog.d(LOG_TAG, "keep AOSP");
                Phone imsPhone = mPhone.getImsPhone();
                if (imsPhone == null ||
                        (imsPhone != null && imsPhone.getHandoverConnection() == null)) {
                    pollCallsWhenSafe();
                } else {
                    Rlog.d(LOG_TAG, "not trigger pollCall since imsCall exists");
                }
            }
            /// @}
        }

        super.updatePhoneType(true);

        /// M: CC: CDMA process call accepted @{
        if (mPhone.isPhoneTypeGsm()) {
            if (mMtkCi == null) {
                mMtkCi = (MtkRIL) mCi;
            }
            mMtkCi.unregisterForLineControlInfo(this);
            /// M: CC: Update current phone type
            mPhoneType = PhoneConstants.PHONE_TYPE_GSM;
        } else {
            if (mMtkCi == null) {
                mMtkCi = (MtkRIL) mCi;
            }
            mMtkCi.unregisterForLineControlInfo(this);
            mMtkCi.registerForLineControlInfo(this, EVENT_CDMA_CALL_ACCEPTED, null);
            /// M: CC: Update current phone type
            mPhoneType = PhoneConstants.PHONE_TYPE_CDMA;
        }
        /// @}
    }

    @Override
    protected void reset() {
        mHelper.setGwsdCall(false);

        /// M: CC: Vzw/CTVolte ECC for Fusion RIL @{
        if (!hasC2kOverImsModem()) {
            if (VDBG) Rlog.d(LOG_TAG, "keep AOSP");
            super.reset();
            return;
        }

        if (VDBG) Rlog.d(LOG_TAG, "For 93");

        Rlog.d(LOG_TAG, "reset");

        for (GsmCdmaConnection gsmCdmaConnection : mConnections) {
            if (gsmCdmaConnection != null) {
                /// M: CC: Vzw/CTVolte ECC for Fusion RIL @{
                // [ALPS03578681]
                // Not update disconnect to TeleService, since ECC still exists for phone switch
                //gsmCdmaConnection.onDisconnect(DisconnectCause.ERROR_UNSPECIFIED);
                /// @}
                gsmCdmaConnection.dispose();
            }
        }

        if (mPendingMO != null) {
            /// M: CC: Vzw/CTVolte ECC for Fusion RIL @{
            // [ALPS03578681]
            // Not update disconnect to TeleService, since ECC still exists for phone switch
            // Send the notification that the pending call was disconnected to the higher layers.
            //mPendingMO.onDisconnect(DisconnectCause.ERROR_UNSPECIFIED);
            // @}
            mPendingMO.dispose();
        }

        mConnections = null;
        mPendingMO = null;
        clearDisconnected();
    }

    @Override
    protected synchronized void handlePollCalls(AsyncResult ar) {
        List polledCalls;

        if (VDBG) log("handlePollCalls");
        if (ar.exception == null) {
            polledCalls = (List)ar.result;
        } else if (isCommandExceptionRadioNotAvailable(ar.exception)) {
            // just a dummy empty ArrayList to cause the loop
            // to hang up all the calls
            polledCalls = new ArrayList();
        } else if (mNeedWaitImsEConfSrvcc && !hasParsingCEPCapability()) {
            /// M: For IMS conference SRVCC @{
            // ALPS02019630. Needs to wait +ECONFSRVCC then the call number could be known.
            Rlog.d(PROP_LOG_TAG, "SRVCC: +ECONFSRVCC is still not arrival, skip this poll call.");
            return;
            /// @}
        } else {
            // Radio probably wasn't ready--try again in a bit
            // But don't keep polling if the channel is closed
            pollCallsAfterDelay();
            return;
        }

        Connection newRinging = null; //or waiting
        ArrayList<Connection> newUnknownConnectionsGsm = new ArrayList<Connection>();
        Connection newUnknownConnectionCdma = null;
        boolean hasNonHangupStateChanged = false;   // Any change besides
                                                    // a dropped connection
        boolean hasAnyCallDisconnected = false;
        boolean needsPollDelay = false;
        boolean unknownConnectionAppeared = false;
        int handoverConnectionsSize = mHandoverConnections.size();

        //CDMA
        boolean noConnectionExists = true;

        if (polledCalls.size() == 0) {
            mHelper.setGwsdCall(false);
        }

        for (int i = 0, curDC = 0, dcSize = polledCalls.size()
                ; i < mConnections.length; i++) {
            GsmCdmaConnection conn = mConnections[i];
            DriverCall dc = null;

            // polledCall list is sparse
            if (curDC < dcSize) {
                dc = (DriverCall) polledCalls.get(curDC);

                /// M: CC: CDMA plus code @{
                if (!isPhoneTypeGsm()) {
                    dc.number = processPlusCodeForDriverCall(
                            dc.number, dc.isMT, dc.TOA);
                }
                /// @}

                if (dc.index == i+1) {
                    curDC++;
                } else {
                    dc = null;
                }
            }

            //CDMA
            if (conn != null || dc != null) {
                noConnectionExists = false;
            }

            if (DBG_POLL) log("poll: conn[i=" + i + "]=" +
                    conn+", dc=" + dc);

            if (conn == null && dc != null) {

                /// M: CC
                if (DBG_POLL) log("case 1 : new Call appear");

                // Connection appeared in CLCC response that we don't know about
                if (mPendingMO != null && mPendingMO.compareTo(dc)) {

                    if (DBG_POLL) log("poll: pendingMO=" + mPendingMO);

                    // It's our pending mobile originating call
                    mConnections[i] = mPendingMO;
                    mPendingMO.mIndex = i;
                    mPendingMO.update(dc);
                    mPendingMO = null;

                    // Someone has already asked to hangup this call
                    if (mHangupPendingMO) {
                        mHangupPendingMO = false;

                        // M: CC: Allow ECM under GSM
                        // Re-start Ecm timer when an uncompleted emergency call ends
                        if (/*!isPhoneTypeGsm() && */mIsEcmTimerCanceled) {
                            handleEcmTimer(GsmCdmaPhone.RESTART_ECM_TIMER);
                        }

                        try {
                            if (Phone.DEBUG_PHONE) log(
                                    "poll: hangupPendingMO, hangup conn " + i);
                            hangup(mConnections[i]);
                        } catch (CallStateException ex) {
                            Rlog.e(LOG_TAG, "unexpected error on hangup");
                        }

                        // Do not continue processing this poll
                        // Wait for hangup and repoll
                        return;
                    }
                } else {
                    if (Phone.DEBUG_PHONE) {
                        log("pendingMo=" + mPendingMO + ", dc=" + dc);
                    }

                    /// M: CC: Remove handling for MO/MT conflict, not hangup MT @{
                    if (mPendingMO != null && !mPendingMO.compareTo(dc)) {
                        Rlog.d(PROP_LOG_TAG, "MO/MT conflict! MO should be hangup by MD");
                    }
                    /// @}

                    mConnections[i] = new MtkGsmCdmaConnection(mPhone, dc, this, i);

                    /// M: CC: Forwarding number via EAIC @{
                    if (isPhoneTypeGsm()) {
                        //To store forwarding address to connection object.
                        mHelper.setForwardingAddressToConnection(i, mConnections[i]);
                    }
                    /// @}

                    Connection hoConnection = getHoConnection(dc);
                    if (hoConnection != null) {
                        // Single Radio Voice Call Continuity (SRVCC) completed
                        /// M: modified to fulfill IMS conference SRVCC. @{
                        if (hoConnection instanceof MtkImsPhoneConnection &&
                                ((MtkImsPhoneConnection) hoConnection).isMultipartyBeforeHandover()
                                && ((MtkImsPhoneConnection) hoConnection).isConfHostBeforeHandover()
                                && !hasParsingCEPCapability()) {
                            Rlog.i(LOG_TAG, "SRVCC: goes to conference case.");
                            mConnections[i].mOrigConnection = hoConnection;
                            mImsConfParticipants.add(mConnections[i]);
                        } else {
                            Rlog.i(LOG_TAG, "SRVCC: goes to normal call case.");
                            /// @}
                            mConnections[i].migrateFrom(hoConnection);
                            // Updating connect time for silent redial cases (ex: Calls are
                            // transferred from DIALING/ALERTING/INCOMING/WAITING to ACTIVE)
                            if (hoConnection.mPreHandoverState != GsmCdmaCall.State.ACTIVE &&
                                    hoConnection.mPreHandoverState != GsmCdmaCall.State.HOLDING &&
                                    dc.state == DriverCall.State.ACTIVE) {
                                mConnections[i].onConnectedInOrOut();
                            } else {
                                mConnections[i].onConnectedConnectionMigrated();
                            }
                            mHandoverConnections.remove(hoConnection);

                            if (isPhoneTypeGsm()) {
                                for (Iterator<Connection> it = mHandoverConnections.iterator();
                                     it.hasNext(); ) {
                                    Connection c = it.next();
                                    Rlog.i(LOG_TAG, "HO Conn state is " + c.mPreHandoverState);
                                    if (c.mPreHandoverState == mConnections[i].getState()) {
                                        Rlog.i(LOG_TAG, "Removing HO conn "
                                                + hoConnection + c.mPreHandoverState);
                                        it.remove();
                                    }
                                }
                            }

                            /// M: CC: Set ECM timer canceled for SRVCC @{
                            if (mIsInEmergencyCall && !mIsEcmTimerCanceled
                                    && mPhone.isInEcm()) {
                                Rlog.i(LOG_TAG, "Ecm timer has been canceled in IMS, "
                                        + "so set mIsEcmTimerCanceled=true directly");
                                mIsEcmTimerCanceled = true;
                            }
                            /// @}

                            mPhone.notifyHandoverStateChanged(mConnections[i]);

                            ///	M: ALPS03621814 @{
                           /* Google AOSP issue, only IMS call will notify remotely held/unheld message.
                                Solution: Update remotely_unheld message to TeleService layer after SRVCC. */
                            mConnections[i].onConnectionEvent(android.telecom.Connection.EVENT_CALL_REMOTELY_UNHELD, null);
                           /// @}

                        }
                    } else {
                        // find if the MT call is a new ring or unknown connection
                        newRinging = checkMtFindNewRinging(dc,i);
                        if (newRinging == null) {
                            unknownConnectionAppeared = true;
                            if (isPhoneTypeGsm()) {
                                newUnknownConnectionsGsm.add(mConnections[i]);
                            } else {
                                newUnknownConnectionCdma = mConnections[i];
                            }
                        }
                    }
                }
                hasNonHangupStateChanged = true;
            } else if (conn != null && dc == null) {

                /// M: CC
                if (DBG_POLL) Rlog.d(PROP_LOG_TAG, "case 2 : old Call disappear");

                if (isPhoneTypeGsm() && mPhoneType != PhoneConstants.PHONE_TYPE_CDMA) {

                    /// M: CC: Convert state from WAITING to INCOMING @{
                    //[ALPS00401290]
                    if (((conn.getCall() == mForegroundCall &&
                            mForegroundCall.mConnections.size() == 1 &&
                            mBackgroundCall.isIdle()) ||
                            (conn.getCall() == mBackgroundCall &&
                            mBackgroundCall.mConnections.size() == 1 &&
                            mForegroundCall.isIdle())) &&
                            mRingingCall.getState() == GsmCdmaCall.State.WAITING) {
                        mRingingCall.mState = GsmCdmaCall.State.INCOMING;
                    }
                    /// @}

                    // Connection missing in CLCC response that we were
                    // tracking.
                    mDroppedDuringPoll.add(conn);

                    // M: CC: Allow ECM under GSM
                    // Re-start Ecm timer when the connected emergency call ends
                    if (mIsEcmTimerCanceled) {
                        handleEcmTimer(GsmCdmaPhone.RESTART_ECM_TIMER);
                    }

                    // Dropped connections are removed from the CallTracker
                    // list but kept in the GsmCdmaCall list
                    mConnections[i] = null;

                    /// M: CC: Proprietary incoming call handling @{
                    mHelper.CallIndicationEnd();
                    /// @}

                    /// M: CC: Forwarding number via EAIC @{
                    //To clear forwarding address if needed
                    mHelper.clearForwardingAddressVariables(i);
                    /// @}
                } else {
                    // This case means the RIL has no more active call anymore and
                    // we need to clean up the foregroundCall and ringingCall.
                    // Loop through foreground call connections as
                    // it contains the known logical connections.
                    int count = mForegroundCall.mConnections.size();
                    for (int n = 0; n < count; n++) {
                        if (Phone.DEBUG_PHONE)
                            log("adding fgCall cn " + n + " to droppedDuringPoll");
                        GsmCdmaConnection cn = (GsmCdmaConnection) mForegroundCall.mConnections
                                .get(n);
                        mDroppedDuringPoll.add(cn);
                    }
                    count = mRingingCall.mConnections.size();
                    // Loop through ringing call connections as
                    // it may contain the known logical connections.
                    for (int n = 0; n < count; n++) {
                        if (Phone.DEBUG_PHONE)
                            log("adding rgCall cn " + n + " to droppedDuringPoll");
                        GsmCdmaConnection cn = (GsmCdmaConnection) mRingingCall.mConnections
                                .get(n);
                        mDroppedDuringPoll.add(cn);
                    }

                    // Re-start Ecm timer when the connected emergency call ends
                    if (mIsEcmTimerCanceled) {
                        handleEcmTimer(GsmCdmaPhone.RESTART_ECM_TIMER);
                    }
                    // If emergency call is not going through while dialing
                    checkAndEnableDataCallAfterEmergencyCallDropped();
                }
                // Dropped connections are removed from the CallTracker
                // list but kept in the Call list
                mConnections[i] = null;
            } else if (conn != null && dc != null && !conn.compareTo(dc) && isPhoneTypeGsm()) {

                /// M: CC
                if (DBG_POLL) Rlog.d(PROP_LOG_TAG, "case 3 : old Call replaced");

                // Connection in CLCC response does not match what
                // we were tracking. Assume dropped call and new call

                mDroppedDuringPoll.add(conn);

                /// M: CC: Fix AOSP bug - to clear mPendingMO @{
                if (mPendingMO != null && mPendingMO.compareTo(dc)) {
                    // Use proprietary tag "GsmCdmaCallTkr" for extra log
                    Rlog.d("GsmCdmaCallTkr",
                            "ringing disc not updated yet & replaced by pendingMo");
                    mConnections[i] = mPendingMO;
                    mPendingMO.mIndex = i;
                    mPendingMO.update(dc);
                    mPendingMO = null;
                } else {
                    mConnections[i] = new MtkGsmCdmaConnection (mPhone, dc, this, i);
                }
                /// @}

                if (mConnections[i].getCall() == mRingingCall) {
                    newRinging = mConnections[i];
                } // else something strange happened
                hasNonHangupStateChanged = true;
            } else if (conn != null && dc != null) { /* implicit conn.compareTo(dc) */
                // Call collision case
                if (!isPhoneTypeGsm() && conn.isIncoming() != dc.isMT) {
                    if (dc.isMT == true) {
                        /// M: CC: Replace MO with MT when they are conflict. @{
                        mConnections[i] = new MtkGsmCdmaConnection(mPhone, dc, this, i);
                        /// @}
                        // Mt call takes precedence than Mo,drops Mo
                        mDroppedDuringPoll.add(conn);
                        // find if the MT call is a new ring or unknown connection
                        newRinging = checkMtFindNewRinging(dc,i);
                        if (newRinging == null) {
                            unknownConnectionAppeared = true;
                            newUnknownConnectionCdma = conn;
                        }
                        checkAndEnableDataCallAfterEmergencyCallDropped();
                    } else {
                        // Call info stored in conn is not consistent with the call info from dc.
                        // We should follow the rule of MT calls taking precedence over MO calls
                        // when there is conflict, so here we drop the call info from dc and
                        // continue to use the call info from conn, and only take a log.
                        Rlog.e(LOG_TAG,"Error in RIL, Phantom call appeared " + dc);
                    }
                } else {

                    /// M: CC
                    if (DBG_POLL) Rlog.d(PROP_LOG_TAG, "case 4 : old Call update");

                    boolean changed;
                    changed = conn.update(dc);
                    hasNonHangupStateChanged = hasNonHangupStateChanged || changed;
                }
            }

            if (REPEAT_POLLING) {
                if (dc != null) {
                    // FIXME with RIL, we should not need this anymore
                    if ((dc.state == DriverCall.State.DIALING
                            /*&& cm.getOption(cm.OPTION_POLL_DIALING)*/)
                        || (dc.state == DriverCall.State.ALERTING
                            /*&& cm.getOption(cm.OPTION_POLL_ALERTING)*/)
                        || (dc.state == DriverCall.State.INCOMING
                            /*&& cm.getOption(cm.OPTION_POLL_INCOMING)*/)
                        || (dc.state == DriverCall.State.WAITING
                            /*&& cm.getOption(cm.OPTION_POLL_WAITING)*/)) {
                        // Sometimes there's no unsolicited notification
                        // for state transitions
                        needsPollDelay = true;
                    }
                }
            }
        }

        // Safety check so that obj is not stuck with mIsInEmergencyCall set to true (and data
        // disabled). This should never happen though.
        if (!isPhoneTypeGsm() && noConnectionExists) {
            checkAndEnableDataCallAfterEmergencyCallDropped();
        }

        // This is the first poll after an ATD.
        // We expect the pending call to appear in the list
        // If it does not, we land here
        if (mPendingMO != null) {
            Rlog.d(LOG_TAG, "Pending MO dropped before poll fg state:"
                    + mForegroundCall.getState());

            mDroppedDuringPoll.add(mPendingMO);
            mPendingMO = null;
            mHangupPendingMO = false;

            // M: CC: Allow ECM under GSM
            if (mPendingCallInEcm) {
                mPendingCallInEcm = false;
            }
            /// M: CC: Re-start Ecm timer when the connected emergency call ends @{
            if (mIsEcmTimerCanceled) {
                handleEcmTimer(GsmCdmaPhone.RESTART_ECM_TIMER);
            }
            /// @}

            if (!isPhoneTypeGsm()) {
                checkAndEnableDataCallAfterEmergencyCallDropped();
            }
        }

        /// M: CC: handle CDMA flash pendingMO not disconnect during radio_off issue @{
        if (polledCalls.size() == 0 && mConnections.length == 0) {
            if (Phone.DEBUG_PHONE) log("check whether fgCall or ringCall have mConnections");
            if (!isPhoneTypeGsm()) {
                // This case means the RIL has no more active call anymore and
                // we need to clean up the foregroundCall and ringingCall.
                // Loop through foreground call connections as
                // it contains the known logical connections.
                int count = mForegroundCall.mConnections.size();
                for (int n = 0; n < count; n++) {
                    if (Phone.DEBUG_PHONE) log("adding fgCall cn " + n + " to droppedDuringPoll");
                    GsmCdmaConnection cn = (GsmCdmaConnection) mForegroundCall.mConnections.get(n);
                    mDroppedDuringPoll.add(cn);
                }

                count = mRingingCall.mConnections.size();
                // Loop through ringing call connections as
                // it may contain the known logical connections.
                for (int n = 0; n < count; n++) {
                    if (Phone.DEBUG_PHONE) log("adding rgCall cn " + n + " to droppedDuringPoll");
                    GsmCdmaConnection cn = (GsmCdmaConnection) mRingingCall.mConnections.get(n);
                    mDroppedDuringPoll.add(cn);
                }
            }
        }
        /// @}

        if (newRinging != null) {
            mPhone.notifyNewRingingConnection(newRinging);
        }

        // clear the "local hangup" and "missed/rejected call"
        // cases from the "dropped during poll" list
        // These cases need no "last call fail" reason
        ArrayList<GsmCdmaConnection> locallyDisconnectedConnections = new ArrayList<>();
        for (int i = mDroppedDuringPoll.size() - 1; i >= 0 ; i--) {
            GsmCdmaConnection conn = mDroppedDuringPoll.get(i);
            //CDMA
            boolean wasDisconnected = false;

            /// M: CC: Modem reset related handling @{
            if (isCommandExceptionRadioNotAvailable(ar.exception)) {
                mDroppedDuringPoll.remove(i);
                hasAnyCallDisconnected |= conn.onDisconnect(DisconnectCause.LOST_SIGNAL);
                wasDisconnected = true;
            /// @}
            } else if (conn.isIncoming() && conn.getConnectTime() == 0) {
                // Missed or rejected call
                int cause;
                if (conn.mCause == DisconnectCause.LOCAL) {
                    cause = DisconnectCause.INCOMING_REJECTED;
                } else {
                    cause = DisconnectCause.INCOMING_MISSED;
                }

                if (Phone.DEBUG_PHONE) {
                    log("missed/rejected call, conn.cause=" + conn.mCause);
                    log("setting cause to " + cause);
                }
                mDroppedDuringPoll.remove(i);
                hasAnyCallDisconnected |= conn.onDisconnect(cause);
                wasDisconnected = true;
                locallyDisconnectedConnections.add(conn);
            } else if (conn.mCause == DisconnectCause.LOCAL
                    || conn.mCause == DisconnectCause.INVALID_NUMBER) {
                mDroppedDuringPoll.remove(i);
                hasAnyCallDisconnected |= conn.onDisconnect(conn.mCause);
                wasDisconnected = true;
                locallyDisconnectedConnections.add(conn);
            }

            if (!isPhoneTypeGsm() && wasDisconnected && unknownConnectionAppeared
                    && conn == newUnknownConnectionCdma) {
                unknownConnectionAppeared = false;
                newUnknownConnectionCdma = null;
            }
        }
        if (locallyDisconnectedConnections.size() > 0) {
            mMetrics.writeRilCallList(mPhone.getPhoneId(), locallyDisconnectedConnections,
                                      TelephonyManager.getDefault().getNetworkCountryIso());
        }

        // IMS conference SRVCC
        // Added method to fulfill conference SRVCC for the host side.
        // The conference participant side is handled as normal call SRVCC.
        if (mImsConfHostConnection != null) {
            MtkImsPhoneConnection hostConn = (MtkImsPhoneConnection) mImsConfHostConnection;
            if (mImsConfParticipants.size() >= 2) {
                // Participants >= 2, apply MTK SRVCC solution.

                // Try to restore participants' address, we don't sure if +ECONFSRVCC is arrival.
                restoreConferenceParticipantAddress();

                Rlog.d(PROP_LOG_TAG, "SRVCC: notify new participant connections");
                hostConn.notifyConferenceConnectionsConfigured(mImsConfParticipants);
            } else if (mImsConfParticipants.size() == 1) {
                // Participants = 1, can't be a conference, so apply Google SRVCC solution.
                GsmCdmaConnection participant = (GsmCdmaConnection) mImsConfParticipants.get(0);

                // Conference host side with only one participant case.
                // Due to modem's limitation, we still need to restore the address since modem
                // doesn't notify address information.
                String address = hostConn.getConferenceParticipantAddress(0);
                Rlog.d(PROP_LOG_TAG,
                        "SRVCC: restore participant connection with address: "
                        + Rlog.pii(PROP_LOG_TAG, address));
                if (participant instanceof MtkGsmCdmaConnection) {
                    ((MtkGsmCdmaConnection) participant).
                            updateConferenceParticipantAddress(address);
                }

                Rlog.d(PROP_LOG_TAG,
                        "SRVCC: only one connection, consider it as a normal call SRVCC");
                mPhone.notifyHandoverStateChanged(participant);
            } else {
                Rlog.e(PROP_LOG_TAG, "SRVCC: abnormal case, no participant connections.");
            }
            mImsConfParticipants.clear();
            mImsConfHostConnection = null;
            mEconfSrvccConnectionIds = null;
        }

        /* Disconnect any pending Handover connections */
        for (Iterator<Connection> it = mHandoverConnections.iterator();
                it.hasNext();) {
            Connection hoConnection = it.next();
            log("handlePollCalls - disconnect hoConn= " + hoConnection +
                    " hoConn.State= " + hoConnection.getState());
            if (hoConnection.getState().isRinging()) {
                hoConnection.onDisconnect(DisconnectCause.INCOMING_MISSED);
            } else {
                hoConnection.onDisconnect(DisconnectCause.NOT_VALID);
            }
            // TODO: Do we need to update these hoConnections in Metrics ?
            it.remove();
        }

        // Any non-local disconnects: determine cause
        if (mDroppedDuringPoll.size() > 0) {
            mMtkCi.getLastCallFailCause(
                obtainNoPollCompleteMessage(EVENT_GET_LAST_CALL_FAIL_CAUSE));
        }

        if (needsPollDelay) {
            pollCallsAfterDelay();
        }

        // Cases when we can no longer keep disconnected Connection's
        // with their previous calls
        // 1) the phone has started to ring
        // 2) A Call/Connection object has changed state...
        //    we may have switched or held or answered (but not hung up)
        if ((newRinging != null || hasNonHangupStateChanged || hasAnyCallDisconnected)
            /// M: CC: Proprietary CRSS handling @{
            && !mHasPendingSwapRequest) {
            /// @}

            internalClearDisconnected();
        }

        if (VDBG) log("handlePollCalls calling updatePhoneState()");
        updatePhoneState();

        if (unknownConnectionAppeared) {
            if (isPhoneTypeGsm()) {
                for (Connection c : newUnknownConnectionsGsm) {
                    log("Notify unknown for " + c);
                    mPhone.notifyUnknownConnection(c);
                }
            } else {
                mPhone.notifyUnknownConnection(newUnknownConnectionCdma);
            }
        }

        if (hasNonHangupStateChanged || newRinging != null || hasAnyCallDisconnected) {
            mPhone.notifyPreciseCallStateChanged();
            updateMetrics(mConnections);
        }

        // If all handover connections are mapped during this poll process clean it up
        if (handoverConnectionsSize > 0 && mHandoverConnections.size() == 0) {
            Phone imsPhone = mPhone.getImsPhone();
            if (imsPhone != null) {
                imsPhone.callEndCleanupHandOverCallIfAny();
            }
        }

        /// M: CC: Convert state from WAITING to INCOMING @{
        //[ALPS00401290]
        if (isPhoneTypeGsm() && mConnections != null
                && mConnections.length == MAX_CONNECTIONS_GSM) {
            if ((mHelper.getCurrentTotalConnections() == 1) &&
                    (mRingingCall.getState() == GsmCdmaCall.State.WAITING)) {
                mRingingCall.mState = GsmCdmaCall.State.INCOMING;
            }
        }
        /// @}

        //dumpState();
    }

    @Override
    protected void dumpState() {
        List l;

        Rlog.i(LOG_TAG,"Phone State:" + mState);

        Rlog.i(LOG_TAG,"Ringing call: " + mRingingCall.toString());

        l = mRingingCall.getConnections();
        for (int i = 0, s = l.size(); i < s; i++) {
            Rlog.i(LOG_TAG,l.get(i).toString());
        }

        Rlog.i(LOG_TAG,"Foreground call: " + mForegroundCall.toString());

        l = mForegroundCall.getConnections();
        for (int i = 0, s = l.size(); i < s; i++) {
            Rlog.i(LOG_TAG,l.get(i).toString());
        }

        Rlog.i(LOG_TAG,"Background call: " + mBackgroundCall.toString());

        l = mBackgroundCall.getConnections();
        for (int i = 0, s = l.size(); i < s; i++) {
            Rlog.i(LOG_TAG,l.get(i).toString());
        }

        /// M: CC: Use GsmCallTrackerHelper @{
        if (isPhoneTypeGsm()) {
            mHelper.LogState();
        }
        // @}
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;

        /// M: CC: Use GsmCallTrackerHelper @{
        mHelper.LogerMessage(msg.what);
        /// @}

        switch (msg.what) {
        /// M: CC: Proprietary incoming call handling @{
        case EVENT_INCOMING_CALL_INDICATION:
            mHelper.CallIndicationProcess((AsyncResult) msg.obj);
            break;
        case EVENT_CALL_ADDITIONAL_INFO:
            mHelper.handleCallAdditionalInfo((AsyncResult) msg.obj);
            break;
        /// @}
        /// M: CC: Modem reset related handling @{
        case EVENT_RADIO_OFF_OR_NOT_AVAILABLE:
            Rlog.d(PROP_LOG_TAG, "Receives EVENT_RADIO_OFF_OR_NOT_AVAILABLE");
            // Exiting ECBM may trigger pending duplex mode switching during ECC
            // and trigger radio off event. We should ignore it or call will be
            // cancelled directly.
            if (!mPendingCallInEcm) {
                handlePollCalls(new AsyncResult(null, null,
                        new CommandException(CommandException.Error.RADIO_NOT_AVAILABLE)));
                mLastRelevantPoll = null;
            }
        break;
        /// @}
        case EVENT_DIAL_CALL_RESULT:
            ar = (AsyncResult) msg.obj;
            if (ar.exception != null) {
                Rlog.d(PROP_LOG_TAG, "dial call failed!!");
            }
            operationComplete();
        break;
        /// M: CC: Hangup special handling @{
        case EVENT_SWITCH_RESULT:
            if (isPhoneTypeGsm()) {
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    /// M: CC: Proprietary CRSS handling @{
                    if (mWaitForHoldToRedialRequest.isWaitToRedial()) {
                        /* mPendingMO may be reset to null, ex: receive
                             EVENT_RADIO_OFF_OR_NOT_AVAILABLE */
                        if (mPendingMO != null) {
                            mPendingMO.mCause = DisconnectCause.LOCAL;
                            mPendingMO.onDisconnect(DisconnectCause.LOCAL);
                            mPendingMO = null;
                            mHangupPendingMO = false;
                            updatePhoneState();
                        }

                        resumeBackgroundAfterDialFailed();
                        mPhone.notifyPreciseCallStateChanged();
                        mWaitForHoldToRedialRequest.resetToRedial();
                    }
                    /// @}
                    mPhone.notifySuppServiceFailed(getFailedService(msg.what));
                /// M: CC: Proprietary CRSS handling @{
                } else {
                    if (mWaitForHoldToRedialRequest.isWaitToRedial()) {
                        Rlog.d(PROP_LOG_TAG, "Switch success, then resume dial");
                        mWaitForHoldToRedialRequest.resumeDialAfterHold();
                    }
                }

                if (mWaitForHoldToHangupRequest.isWaitToHangup()) {
                    if (ar.exception == null) {
                        if (mWaitForHoldToHangupRequest.mCall != null) {
                            Rlog.d(PROP_LOG_TAG,
                                    "Switch ends, found waiting hangup. switch fg/bg call.");
                            if (mWaitForHoldToHangupRequest.mCall == mForegroundCall) {
                                mWaitForHoldToHangupRequest.setToHangup(mBackgroundCall);
                            } else if (mWaitForHoldToHangupRequest.mCall == mBackgroundCall) {
                                mWaitForHoldToHangupRequest.setToHangup(mForegroundCall);
                            }
                        }
                    }
                    Rlog.d(PROP_LOG_TAG, "Switch ends, wait for poll call done to hangup");
                    mWaitForHoldToHangupRequest.setHoldDone();
                }

                mHasPendingSwapRequest = false;
                /// @}
                operationComplete();
            }
        break;
        /// M: CC: Hangup special handling @{
        case EVENT_HANG_UP_RESULT:
            operationComplete();
        break;

        case EVENT_POLL_CALLS_RESULT:
            Rlog.d(LOG_TAG, "Event EVENT_POLL_CALLS_RESULT Received");

            if (msg == mLastRelevantPoll) {
                if (DBG_POLL) log(
                        "handle EVENT_POLL_CALL_RESULT: set needsPoll=F");
                mNeedsPoll = false;
                mLastRelevantPoll = null;

                /// M: ALPS03487233 @{
                /* UNSOL_VOICE_RADIO_TECH_CHANGED received after call disconnected, and handled
                   before poll call result returned, so the phone state is changed to idle.
                   We need to update phone state to upper layer before update phone type. */
                ar = (AsyncResult) msg.obj;
                boolean bNoCallExists = noAnyCallFromModemExist(ar);
                if (!bNoCallExists && mHasPendingUpdatePhoneType) {
                    /* For handling unknown connection(redial case), updatePhoneType() should
                       be called before handlePollCalls */
                    mHasPendingUpdatePhoneType = false;
                    updatePhoneType(false, true);
                    Rlog.d(LOG_TAG, "[EVENT_POLL_CALLS_RESULT]!bNoCallExists");
                }
                /// @}

                handlePollCalls((AsyncResult) msg.obj);

                /// M: ALPS03487233 @{
                /* UNSOL_VOICE_RADIO_TECH_CHANGED received after call disconnected, and handled
                   before poll call result returned, so the phone state is changed to idle.
                   We need to update phone state to upper layer before update phone type. */
                if (bNoCallExists && mHasPendingUpdatePhoneType) {
                    /* When no any call exists, reset() should be called after handlePollCalls */
                    mHasPendingUpdatePhoneType = false;
                    updatePhoneType(false, true);
                    Rlog.d(LOG_TAG, "[EVENT_POLL_CALLS_RESULT]bNoCallExists");
                }
                /// @}

                /// M: When phone type changing from CDMA to GSM, need to check and
                /// enable data. @{
                if (mHasPendingCheckAndEnableData) {
                    if (bNoCallExists) {
                        checkAndEnableDataCallAfterEmergencyCallDropped();
                    }
                    mHasPendingCheckAndEnableData = false;
                }
                /// @}

                /// M: CC: Proprietary CRSS handling @{
                if (mWaitForHoldToHangupRequest.isHoldDone()) {
                    Rlog.d(PROP_LOG_TAG, "Switch ends, and poll call done, then resume hangup");
                    mWaitForHoldToHangupRequest.resumeHangupAfterHold();
                }
                /// @}
            }
            break;

        /// M: CC: CDMA plus code & CDMA FDN @{
        case EVENT_EXIT_ECM_RESPONSE_CDMA:
            Rlog.d(PROP_LOG_TAG, "Receives EVENT_EXIT_ECM_RESPONSE_CDMA");
            /// M: CC: Resume dial no matter phone type is changed from C2K to GSM
            if (mPendingCallInEcm) {
                final String dialString = (String) ((AsyncResult) msg.obj).userObj;

                if (mPendingMO == null) {
                    mPendingMO = new GsmCdmaConnection(mPhone,
                            checkForTestEmergencyNumber(dialString),
                            this, mForegroundCall, false/*not ECC*/);
                }

                if (!isPhoneTypeGsm()) {
                    /// M: CDMA FDN @{
                    String tmpStr = mPendingMO.getAddress();
                    tmpStr += "," + PhoneNumberUtils.extractNetworkPortionAlt(dialString);
                    /// @}
                    mCi.dial(tmpStr, mPendingMO.isEmergencyCall(),
                                mPendingMO.getEmergencyNumberInfo(),
                                mPendingMO.hasKnownUserIntentEmergency(),
                                mPendingCallClirMode, obtainCompleteMessage());
                    /// M: CC: CDMA process plus code @{
                    if (needToConvert(dialString)) {
                        mPendingMO.setConverted(
                                PhoneNumberUtils.extractNetworkPortionAlt(dialString));
                    }
                    /// @}
                } else {
                    Rlog.e(LOG_TAG, "originally unexpected event " + msg.what +
                            " not handled by phone type " + mPhone.getPhoneType());
                    mCi.dial(mPendingMO.getAddress(), mPendingMO.isEmergencyCall(),
                                mPendingMO.getEmergencyNumberInfo(),
                                mPendingMO.hasKnownUserIntentEmergency(),
                                mPendingCallClirMode, null/*uusInfo*/,
                                obtainCompleteMessage());
                }
                mPendingCallInEcm = false;
            }
            mPhone.unsetOnEcbModeExitResponse(this);
            break;

        case EVENT_THREE_WAY_DIAL_BLANK_FLASH:
            Rlog.d(PROP_LOG_TAG, "Receives EVENT_THREE_WAY_DIAL_BLANK_FLASH");
            if (!isPhoneTypeGsm()) {
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    /// M: CC: CDMA plus code @{
                    final String dialString = (String) ((AsyncResult) msg.obj).userObj;
                    /// @}
                    postDelayed(
                            new Runnable() {
                                public void run() {
                                    if (mPendingMO != null) {
                                        /// M: CDMA FDN @{
                                        String tmpStr = mPendingMO.getAddress();
                                        tmpStr += "," +
                                                PhoneNumberUtils.extractNetworkPortionAlt(
                                                dialString);
                                        /// @}
                                        mCi.sendCDMAFeatureCode(tmpStr,
                                                obtainMessage(EVENT_THREE_WAY_DIAL_L2_RESULT_CDMA));
                                        /// M: CC: CDMA plus code @{
                                        if (needToConvert(dialString)) {
                                            mPendingMO.setConverted(
                                                    PhoneNumberUtils.extractNetworkPortionAlt(
                                                            dialString));
                                        }
                                        /// @}
                                    }
                                }
                            }, m3WayCallFlashDelay);
                } else {
                    mPendingMO = null;
                    Rlog.w(LOG_TAG, "exception happened on Blank Flash for 3-way call");
                }
            } else {
                Rlog.e(LOG_TAG, "unexpected event " + msg.what + " not handled by " +
                        "phone type " + mPhone.getPhoneType());
            }
            break;
        /// @}

        /// M: CC: CDMA call accepted @{
        case EVENT_CDMA_CALL_ACCEPTED:
            Rlog.d(PROP_LOG_TAG, "Receives EVENT_CDMA_CALL_ACCEPTED");
            ar = (AsyncResult) msg.obj;
            if (ar.exception == null) {
                handleCallAccepted();
            }
            break;
        /// @}
        // For IMS conference SRVCC
        case EVENT_ECONF_SRVCC_INDICATION:
            log("Receives EVENT_ECONF_SRVCC_INDICATION");
            if (!hasParsingCEPCapability()) {
                ar = (AsyncResult) msg.obj;
                mEconfSrvccConnectionIds = (int[]) ar.result;

                // Mark ECONFSRVCC is received, then do poll calls now.
                mNeedWaitImsEConfSrvcc = false;
                pollCallsWhenSafe();
            }
            break;

        /// M: Timing issue, mPendingMO could be reset to null in handlePollCalls @{
        case EVENT_THREE_WAY_DIAL_L2_RESULT_CDMA:
            if (!isPhoneTypeGsm()) {
                ar = (AsyncResult)msg.obj;
                if (ar.exception == null && mPendingMO != null) {
                    // Assume 3 way call is connected
                    mPendingMO.onConnectedInOrOut();
                    mPendingMO = null;
                }
            } else {
                Rlog.e(LOG_TAG, "unexpected event " + msg.what + " not handled by " +
                        "phone type " + mPhone.getPhoneType());
            }
            break;
        // @}

        default:
            super.handleMessage(msg);
        }
    }

    /// M: CC: HangupAll for FTA 31.4.4.2 @{
    /**
     * used to release all connections in the MS,
     * release all connections with one reqeust together, not seperated.
     * @throws CallStateException if the callState is unexpected.
     */
    public void hangupAll() throws CallStateException {
        if (Phone.DEBUG_PHONE) Rlog.d(PROP_LOG_TAG, "hangupAll");
        mMtkCi.hangupAll(obtainCompleteMessage());

        if (!mRingingCall.isIdle()) {
            mRingingCall.onHangupLocal();
        }
        if (!mForegroundCall.isIdle()) {
            mForegroundCall.onHangupLocal();
        }
        if (!mBackgroundCall.isIdle()) {
            mBackgroundCall.onHangupLocal();
        }
    }
    /// @}

    public void hangup(GsmCdmaConnection conn) throws CallStateException {
        if (conn.mOwner != this) {
            throw new CallStateException ("GsmCdmaConnection " + conn
                                    + "does not belong to GsmCdmaCallTracker " + this);
        }

        if (conn == mPendingMO) {
            // We're hanging up an outgoing call that doesn't have it's
            // GsmCdma index assigned yet

            if (Phone.DEBUG_PHONE) log("hangup: set hangupPendingMO to true");
            mHangupPendingMO = true;
        } else if (!isPhoneTypeGsm()
                && conn.getCall() == mRingingCall
                && mRingingCall.getState() == GsmCdmaCall.State.WAITING) {
            // Handle call waiting hang up case.
            //
            // The ringingCall state will change to IDLE in GsmCdmaCall.detach
            // if the ringing call connection size is 0. We don't specifically
            // set the ringing call state to IDLE here to avoid a race condition
            // where a new call waiting could get a hang up from an old call
            // waiting ringingCall.
            //
            // PhoneApp does the call log itself since only PhoneApp knows
            // the hangup reason is user ignoring or timing out. So conn.onDisconnect()
            // is not called here. Instead, conn.onLocalDisconnect() is called.
            conn.onLocalDisconnect();

            updatePhoneState();
            mPhone.notifyPreciseCallStateChanged();
            return;
        } else {
            try {
                mMetrics.writeRilHangup(mPhone.getPhoneId(), conn, conn.getGsmCdmaIndex(),
                                        TelephonyManager.getDefault().getNetworkCountryIso());
                /// M: CC: Hangup special handling @{
                //mCi.hangupConnection (conn.getGsmCdmaIndex(), obtainCompleteMessage());
                mCi.hangupConnection(conn.getGsmCdmaIndex(),
                        obtainCompleteMessage(EVENT_HANG_UP_RESULT));
            } catch (CallStateException ex) {
                // Ignore "connection not found"
                // Call may have hung up already
                Rlog.w(LOG_TAG,"GsmCdmaCallTracker WARN: hangup() on absent connection "
                                + conn);
            }
        }

        conn.onHangupLocal();
    }

    //***** Called from GsmCdmaCall

    public void hangup(GsmCdmaCall call) throws CallStateException {
        if (call.getConnections().size() == 0) {
            throw new CallStateException("no connections in call");
        }

        if (call == mRingingCall) {
            if (Phone.DEBUG_PHONE) log("(ringing) hangup waiting or background");
            logHangupEvent(call);
            /// M: CC: Softbank blacklist requirement @{
            /// M: CC: Hangup special handling
            //mCi.hangupWaitingOrBackground(obtainCompleteMessage());
            GsmCdmaConnection conn = (GsmCdmaConnection) (call.getConnections().get(0));
            if (conn instanceof com.mediatek.internal.telephony.MtkGsmCdmaConnection
                    && (((MtkGsmCdmaConnection) conn).getRejectWithCause() != -1)) {
                mMtkCi.hangupConnectionWithCause(conn.getGsmCdmaIndex(),
                        ((MtkGsmCdmaConnection) conn).getRejectWithCause(),
                        obtainCompleteMessage(EVENT_HANG_UP_RESULT));
            } else {
                mCi.hangupWaitingOrBackground(obtainCompleteMessage(EVENT_HANG_UP_RESULT));
            }
            /// @}
        } else if (call == mForegroundCall) {
            if (call.isDialingOrAlerting()) {
                if (Phone.DEBUG_PHONE) {
                    log("(foregnd) hangup dialing or alerting...");
                }
                hangup((GsmCdmaConnection)(call.getConnections().get(0)));
            /*
            /// M: CC: Use 1+SEND MMI to release active calls & accept held or waiting call @{
            // [ALPS02087255] [Call]MMI 1 cannot accept the waiting call.
            // 3GPP 22.030 6.5.5
            // "Releases all active calls (if any exist) and accepts
            //  the other (held or waiting) call."
            } else if (isPhoneTypeGsm()
                    && mRingingCall.isRinging()) {
                // Do not auto-answer ringing on CHUP, instead just end active calls
                log("hangup all conns in active/background call, without affecting ringing call");
                hangupAllConnections(call);
            */
            } else {
                logHangupEvent(call);
                /// M: CC: Can not end the ECC call when enable SIM PIN lock
                //[ALPS01431282][ALPS.KK1.MP2.V2.4 Regression Test]
                //hangupForegroundResumeBackground();
                if (Phone.DEBUG_PHONE) log("(foregnd) hangup active");
                if (isPhoneTypeGsm()) {
                    GsmCdmaConnection cn = (GsmCdmaConnection) call.getConnections().get(0);
                    String address = cn.getAddress();

                    boolean isEmergencyCall = TelephonyManager.getDefault()
                            .isEmergencyNumber(address);
                    if (isEmergencyCall) {
                        Rlog.d(PROP_LOG_TAG,
                                "(foregnd) hangup active ECC call by connection index");
                        hangup((GsmCdmaConnection) (call.getConnections().get(0)));
                    } else {
                        /// M: CC: Proprietary CRSS handling @{
                        //hangupForegroundResumeBackground();
                        if (!mWaitForHoldToHangupRequest.isWaitToHangup()) {
                            hangupForegroundResumeBackground();
                        } else {
                            mWaitForHoldToHangupRequest.setToHangup(call);
                        }
                        /// @}
                    }
                } else {
                    hangupForegroundResumeBackground();
                }
            }
        } else if (call == mBackgroundCall) {
            if (mRingingCall.isRinging()) {
                if (Phone.DEBUG_PHONE) {
                    log("hangup all conns in background call");
                }
                hangupAllConnections(call);
            } else {
                if (Phone.DEBUG_PHONE) log("(backgnd) hangup waiting/background");
                /// M: CC: Proprietary CRSS handling @{
                //hangupWaitingOrBackground();
                if (!mWaitForHoldToHangupRequest.isWaitToHangup()) {
                    hangupWaitingOrBackground();
                } else {
                    mWaitForHoldToHangupRequest.setToHangup(call);
                }
                /// @}
            }
        } else {
            throw new RuntimeException ("GsmCdmaCall " + call +
                    "does not belong to GsmCdmaCallTracker " + this);
        }

        call.onHangupLocal();
        mPhone.notifyPreciseCallStateChanged();
    }

    public void hangupWaitingOrBackground() {
        if (Phone.DEBUG_PHONE) log("hangupWaitingOrBackground");
        logHangupEvent(mBackgroundCall);
        /// M: CC: Hangup special handling @{
        //mCi.hangupWaitingOrBackground(obtainCompleteMessage());
        mCi.hangupWaitingOrBackground(obtainCompleteMessage(EVENT_HANG_UP_RESULT));
        /// @}
    }

    public void hangupForegroundResumeBackground() {
        if (Phone.DEBUG_PHONE) log("hangupForegroundResumeBackground");
        /// M: CC: Hangup special handling @{
        //mCi.hangupForegroundResumeBackground(obtainCompleteMessage());
        mCi.hangupForegroundResumeBackground(obtainCompleteMessage(EVENT_HANG_UP_RESULT));
        /// @}
    }

    /// M: ALPS03487233   @{
    /* Return if no call exists */
    private boolean noAnyCallFromModemExist(AsyncResult ar) {
        List polledCalls;

        if (ar.exception == null) {
            polledCalls = (List) ar.result;
        } else {
            // just a dummy empty ArrayList to cause the loop
            // to hang up all the calls
            polledCalls = new ArrayList();
        }

        return (polledCalls.size() == 0);
    }
    /// @}

    private PersistableBundle getCarrierConfig() {
        CarrierConfigManager configManager = (CarrierConfigManager)
                mPhone.getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
        return configManager.getConfigForSubId(mPhone.getSubId());
    }

    @Override
    public boolean canConference() {
        /// M: CC: Multiline cross line conference support by CarrierConfigManager. @{
        boolean bCrossLineConfSupport = false;
        PersistableBundle config = getCarrierConfig();
        if (config != null) {
            bCrossLineConfSupport = config.getBoolean(
                    MtkCarrierConfigManager.MTK_KEY_MULTILINE_ALLOW_CROSS_LINE_CONFERENCE_BOOL);
        }
        /// @}

        if (!bCrossLineConfSupport) {
            /// M: CC: Multiline No cross line conference {@
            boolean isSameLine = mMtkGsmCdmaCallTrackerExt.areConnectionsInSameLine(mConnections);
            return isSameLine && super.canConference();
            /// @}
        } else {
            return super.canConference();
        }
    }

    @Override
    public void conference() {
        /// M: CC: Multiline cross line conference support by CarrierConfigManager. @{
        boolean bCrossLineConfSupport = false;
        PersistableBundle config = getCarrierConfig();
        if (config != null) {
            bCrossLineConfSupport = config.getBoolean(
                    MtkCarrierConfigManager.MTK_KEY_MULTILINE_ALLOW_CROSS_LINE_CONFERENCE_BOOL);
        }
        /// @}

        if (!bCrossLineConfSupport) {
            /// M: CC: Multiline No cross line conference {@
            boolean isSameLine = mMtkGsmCdmaCallTrackerExt.areConnectionsInSameLine(mConnections);
            if (!isSameLine) {
                Rlog.e(PROP_LOG_TAG, "conference fail. (not same line)");
                return;
            }
            /// @}
        }

        super.conference();
    }

    //GSM
    /**
     * clirMode is one of the CLIR_ constants
     */
    public synchronized Connection dialGsm(String dialString, int clirMode, UUSInfo uusInfo,
                                        Bundle intentExtras)
            throws CallStateException {
        // note that this triggers call state changed notif
        clearDisconnected();

        // Check for issues which would preclude dialing and throw a CallStateException.
        boolean isEmergencyCall = PhoneNumberUtils.isLocalEmergencyNumber(mPhone.getContext(),
                dialString);
        checkForDialIssues(isEmergencyCall);

        String origNumber = dialString;
        dialString = convertNumberIfNecessary(mPhone, dialString);

        // The new call must be assigned to the foreground call.
        // That call must be idle, so place anything that's
        // there on hold
        if (mForegroundCall.getState() == GsmCdmaCall.State.ACTIVE) {
            // this will probably be done by the radio anyway
            // but the dial might fail before this happens
            // and we need to make sure the foreground call is clear
            // for the newly dialed connection

            /// M: CC: Proprietary CRSS handling @{
            mWaitForHoldToRedialRequest.setToRedial();
            /// @}

            switchWaitingOrHoldingAndActive();
            // This is a hack to delay DIAL so that it is sent out to RIL only after
            // EVENT_SWITCH_RESULT is received. We've seen failures when adding a new call to
            // multi-way conference calls due to DIAL being sent out before SWITCH is processed
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                // do nothing
            }

            // Fake local state so that
            // a) foregroundCall is empty for the newly dialed connection
            // b) hasNonHangupStateChanged remains false in the
            // next poll, so that we don't clear a failed dialing call
            fakeHoldForegroundBeforeDial();
        }

        if (mForegroundCall.getState() != GsmCdmaCall.State.IDLE) {
            //we should have failed in !canDial() above before we get here
            throw new CallStateException("cannot dial in current state");
        }

        /// M: CC: Replace as Mtk class
        mPendingMO = new MtkGsmCdmaConnection(mPhone, checkForTestEmergencyNumber(dialString),
                this, mForegroundCall, isEmergencyCall);
        if (intentExtras != null) {
            Rlog.d(LOG_TAG, "dialGsm - emergency dialer: " + intentExtras.getBoolean(
                    TelecomManager.EXTRA_IS_USER_INTENT_EMERGENCY_CALL));
            mPendingMO.setHasKnownUserIntentEmergency(intentExtras.getBoolean(
                    TelecomManager.EXTRA_IS_USER_INTENT_EMERGENCY_CALL));
        }
        mHangupPendingMO = false;
        mMetrics.writeRilDial(mPhone.getPhoneId(), mPendingMO, clirMode, uusInfo);

        /// M: CC: Reconstruct dialString based on extras @{
        String newDialString = mMtkGsmCdmaCallTrackerExt.convertDialString(
            intentExtras, mPendingMO.getAddress());
        if (newDialString != null) {
            mPendingMO.setConnectionExtras(intentExtras);
        }
        /// @}

        if ( mPendingMO.getAddress() == null || mPendingMO.getAddress().length() == 0
                || mPendingMO.getAddress().indexOf(PhoneNumberUtils.WILD) >= 0) {
            // Phone number is invalid
            mPendingMO.mCause = DisconnectCause.INVALID_NUMBER;

            /// M: CC: Proprietary CRSS handling @{
            mWaitForHoldToRedialRequest.resetToRedial();
            /// @}

            // handlePollCalls() will notice this call not present
            // and will mark it as dropped.
            pollCallsWhenSafe();
        } else {
            // Always unmute when initiating a new call
            setMute(false);

            /// M: CC: Proprietary CRSS handling @{
            if (!mWaitForHoldToRedialRequest.isWaitToRedial()) {
                /// M: CC: If dialString is reconstructed, need to mark it as converted
                if (newDialString != null) {
                    mNumberConverted = true;
                } else {
                    newDialString = mPendingMO.getAddress();
                }
                mCi.dial(newDialString, mPendingMO.isEmergencyCall(),
                        mPendingMO.getEmergencyNumberInfo(),
                        mPendingMO.hasKnownUserIntentEmergency(),
                        clirMode, uusInfo,
                        obtainCompleteMessage());
            } else {
                /// M: CC: If dialString is reconstructed, need to mark it as converted
                if (newDialString != null) {
                    mNumberConverted = true;
                } else {
                    newDialString = mPendingMO.getAddress();
                }
                mWaitForHoldToRedialRequest.setToRedial(newDialString, mPendingMO.isEmergencyCall(),
                            mPendingMO.getEmergencyNumberInfo(),
                            mPendingMO.hasKnownUserIntentEmergency(),
                            clirMode, uusInfo);
            }
            /// @}
        }

        if (mNumberConverted) {
            mPendingMO.setConverted(origNumber);
            mNumberConverted = false;
        }

        updatePhoneState();
        mPhone.notifyPreciseCallStateChanged();

        return mPendingMO;
    }

    public void switchWaitingOrHoldingAndActive() throws CallStateException {
        // Should we bother with this check?
        if (mRingingCall.getState() == GsmCdmaCall.State.INCOMING) {
            throw new CallStateException("cannot be in the incoming state");
        } else {
            if (isPhoneTypeGsm()) {
                /// M: CC: Proprietary CRSS handling @{
                //mCi.switchWaitingOrHoldingAndActive(
                //        obtainCompleteMessage(EVENT_SWITCH_RESULT));
                if (!mHasPendingSwapRequest) {
                    mWaitForHoldToHangupRequest.setToHangup();
                    mCi.switchWaitingOrHoldingAndActive(
                            obtainCompleteMessage(EVENT_SWITCH_RESULT));
                    mHasPendingSwapRequest = true;
                }
                /// @}
            } else {
                if (mForegroundCall.getConnections().size() > 1) {
                    flashAndSetGenericTrue();
                } else {
                    // Send a flash command to CDMA network for putting the other party on hold.
                    // For CDMA networks which do not support this the user would just hear a beep
                    // from the network. For CDMA networks which do support it will put the other
                    // party on hold.
                    mCi.sendCDMAFeatureCode("", obtainMessage(EVENT_SWITCH_RESULT));
                }
            }
        }
    }

    /// M: CC: Proprietary CRSS handling @{
    private void resumeBackgroundAfterDialFailed() {
        // We need to make a copy here, since fakeHoldBeforeDial()
        // modifies the lists, and we don't want to reverse the order
        List<Connection> connCopy = (List<Connection>) mBackgroundCall.mConnections.clone();

        for (int i = 0, s = connCopy.size() ; i < s ; i++) {
            MtkGsmCdmaConnection conn = (MtkGsmCdmaConnection) connCopy.get(i);

            conn.resumeHoldAfterDialFailed();
        }
    }
    /// @}

    private void disableDataCallInEmergencyCall(boolean isEmergencyCall) {
        if (isEmergencyCall) {
            if (Phone.DEBUG_PHONE) log("disableDataCallInEmergencyCall");
            setIsInEmergencyCall();
        }
    }

    // CDMA
    @Override
    protected Connection dialCdma(String dialString, int clirMode, Bundle intentExtras)
            throws CallStateException {
        // note that this triggers call state changed notif
        clearDisconnected();

        boolean isEmergencyCall =
                PhoneNumberUtils.isLocalEmergencyNumber(mPhone.getContext(), dialString);

        // Check for issues which would preclude dialing and throw a CallStateException.
        checkForDialIssues(isEmergencyCall);

        TelephonyManager tm =
                (TelephonyManager) mPhone.getContext().getSystemService(Context.TELEPHONY_SERVICE);
        String origNumber = dialString;
        String operatorIsoContry = tm.getNetworkCountryIsoForPhone(mPhone.getPhoneId());
        String simIsoContry = tm.getSimCountryIsoForPhone(mPhone.getPhoneId());
        boolean internationalRoaming = !TextUtils.isEmpty(operatorIsoContry)
                && !TextUtils.isEmpty(simIsoContry)
                && !simIsoContry.equals(operatorIsoContry);
        if (internationalRoaming) {
            if ("us".equals(simIsoContry)) {
                internationalRoaming = internationalRoaming && !"vi".equals(operatorIsoContry);
            } else if ("vi".equals(simIsoContry)) {
                internationalRoaming = internationalRoaming && !"us".equals(operatorIsoContry);
            }
        }
        if (internationalRoaming) {
            dialString = convertNumberIfNecessary(mPhone, dialString);
        }

        boolean isPhoneInEcmMode = mPhone.isInEcm();

        /// M: CC: Not allow normal call during ECBM. @{
        if ("OP20".equals(SystemProperties.get("persist.vendor.operator.optr", ""))
                && isPhoneInEcmMode && !isEmergencyCall) {
            throw new CallStateException("cannot dial in ECBM");
        }
        /// @}

        // Cancel Ecm timer if a second emergency call is originating in Ecm mode
        if (isPhoneInEcmMode && isEmergencyCall) {
            handleEcmTimer(GsmCdmaPhone.CANCEL_ECM_TIMER);
        }

        // The new call must be assigned to the foreground call.
        // That call must be idle, so place anything that's
        // there on hold
        if (mForegroundCall.getState() == GsmCdmaCall.State.ACTIVE) {
            return dialThreeWay(dialString, intentExtras);
        }

        mPendingMO = new MtkGsmCdmaConnection(mPhone, checkForTestEmergencyNumber(dialString),
                this, mForegroundCall, isEmergencyCall);
        if (intentExtras != null) {
            Rlog.d(LOG_TAG, "dialGsm - emergency dialer: " + intentExtras.getBoolean(
                    TelecomManager.EXTRA_IS_USER_INTENT_EMERGENCY_CALL));
            mPendingMO.setHasKnownUserIntentEmergency(intentExtras.getBoolean(
                    TelecomManager.EXTRA_IS_USER_INTENT_EMERGENCY_CALL));
        }
         mHangupPendingMO = false;

        if (mPendingMO.getAddress() == null || mPendingMO.getAddress().length() == 0
                || mPendingMO.getAddress().indexOf(PhoneNumberUtils.WILD) >= 0 ) {
            // Phone number is invalid
            mPendingMO.mCause = DisconnectCause.INVALID_NUMBER;

            // handlePollCalls() will notice this call not present
            // and will mark it as dropped.
            pollCallsWhenSafe();
        } else {
            // Always unmute when initiating a new call
            setMute(false);

            // Check data call
            disableDataCallInEmergencyCall(isEmergencyCall);

            // In Ecm mode, if another emergency call is dialed, Ecm mode will not exit.
            if(!isPhoneInEcmMode || (isPhoneInEcmMode && isEmergencyCall)) {
                mCi.dial(mPendingMO.getAddress(), mPendingMO.isEmergencyCall(),
                        mPendingMO.getEmergencyNumberInfo(),
                        mPendingMO.hasKnownUserIntentEmergency(),
                        clirMode,
                        obtainCompleteMessage());
                /// M: CC: CDMA plus code @{
                if (needToConvert(dialString)) {
                    mPendingMO.setConverted(PhoneNumberUtils.extractNetworkPortionAlt(dialString));
                }
                /// @}
            } else {
                mPhone.exitEmergencyCallbackMode();
                /// M: CC: CDMA plus code @{
                mPhone.setOnEcbModeExitResponse(this, EVENT_EXIT_ECM_RESPONSE_CDMA, dialString);
                /// @}
                mPendingCallClirMode = clirMode;
                mPendingCallInEcm = true;
            }
        }

        if (mNumberConverted) {
            mPendingMO.setConverted(origNumber);
            mNumberConverted = false;
        }

        updatePhoneState();
        mPhone.notifyPreciseCallStateChanged();

        return mPendingMO;
    }

    @Override
    protected Connection dialThreeWay(String dialString, Bundle intentExtras) {
        if (!mForegroundCall.isIdle()) {
            // Check data call and possibly set mIsInEmergencyCall
            disableDataCallInEmergencyCall(dialString);

            // Attach the new connection to foregroundCall
            mPendingMO = new MtkGsmCdmaConnection(mPhone,
                    checkForTestEmergencyNumber(dialString), this, mForegroundCall,
                    mIsInEmergencyCall);
            if (intentExtras != null) {
                Rlog.d(LOG_TAG, "dialThreeWay - emergency dialer " + intentExtras.getBoolean(
                        TelecomManager.EXTRA_IS_USER_INTENT_EMERGENCY_CALL));
                mPendingMO.setHasKnownUserIntentEmergency(intentExtras.getBoolean(
                        TelecomManager.EXTRA_IS_USER_INTENT_EMERGENCY_CALL));
            }
            // Some networks need an empty flash before sending the normal one
            CarrierConfigManager configManager = (CarrierConfigManager)
                    mPhone.getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
            PersistableBundle bundle = configManager.getConfigForSubId(mPhone.getSubId());
            if (bundle != null) {
                m3WayCallFlashDelay =
                        bundle.getInt(CarrierConfigManager.KEY_CDMA_3WAYCALL_FLASH_DELAY_INT);
            } else {
                // The default 3-way call flash delay is 0s
                m3WayCallFlashDelay = 0;
            }
            if (m3WayCallFlashDelay > 0) {
                /// M: CC: CDMA plus code @{
                mCi.sendCDMAFeatureCode("", obtainMessage(EVENT_THREE_WAY_DIAL_BLANK_FLASH,
                        dialString));
                /// @}
            } else {
                /// M: CDMA FDN @{
                String tmpStr = mPendingMO.getAddress();
                tmpStr += "," + PhoneNumberUtils.extractNetworkPortionAlt(dialString);
                /// @}
                mCi.sendCDMAFeatureCode(tmpStr,
                        obtainMessage(EVENT_THREE_WAY_DIAL_L2_RESULT_CDMA));
                /// M: CC: CDMA plus code @{
                if (needToConvert(dialString)) {
                    mPendingMO.setConverted(PhoneNumberUtils.extractNetworkPortionAlt(dialString));
                }
                /// @}
            }
            return mPendingMO;
        }
        return null;
    }

    @Override
    protected void handleCallWaitingInfo(CdmaCallWaitingNotification cw) {
        /// M: CC: CDMA plus code @{
        processPlusCodeForWaitingCall(cw);
        /// @}

        /// M: CC: CDMA waiting call @{
        if (!shouldNotifyWaitingCall(cw)) {
            return;
        }
        /// @}

        /// M: CC: Performance enhancement. If a CDMA waiting call is the same with a
        // connection before, remove the connection before to avoid too many connections.
        // Keep at least 2 connections because AOSP doesn't expect leaving only
        // one connection for a CDMA conference. @{
        if (mForegroundCall.mConnections.size() > MIN_CONNECTIONS_IN_CDMA_CONFERENCE) {
            for (Connection c : mForegroundCall.mConnections) {
                if (cw.number != null && cw.number.equals(c.getAddress())) {
                    c.onDisconnect(DisconnectCause.NORMAL);
                    break;
                }
            }
        }
        /// @}

        // Create a new GsmCdmaConnection which attaches itself to ringingCall.
        new MtkGsmCdmaConnection(mPhone.getContext(), cw, this, mRingingCall);
        updatePhoneState();

        // Finally notify application
        notifyCallWaitingInfo(cw);
    }

    /// M: CC: CDMA call accepted @{
    private void handleCallAccepted() {
        List connections = mForegroundCall.getConnections();
        int count = connections.size();
        Rlog.d(PROP_LOG_TAG, "handleCallAccepted, fgcall count=" + count);
        if (count == 1) {
            GsmCdmaConnection c = (GsmCdmaConnection) connections.get(0);
            if ((c instanceof MtkGsmCdmaConnection)
                    && (mPhone instanceof MtkGsmCdmaPhone)) {
                ((MtkGsmCdmaConnection) c).onCdmaCallAccepted();
            }
        }
    }
    /// @}

    /// M: CC: CDMA plus code @{
    private String processPlusCodeForDriverCall(String number, boolean isMt, int typeOfAddress) {
        if (isMt && typeOfAddress == PhoneNumberUtils.TOA_International) {
            if (number != null && number.length() > 0 && number.charAt(0) == '+') {
                number = number.substring(1, number.length());
            }
            number = PlusCodeProcessor.getPlusCodeUtils().removeIddNddAddPlusCode(number);
        }
        number = PhoneNumberUtils.stringFromStringAndTOA(number, typeOfAddress);
        return number;
    }

    private void processPlusCodeForWaitingCall(CdmaCallWaitingNotification cw) {
        String address = cw.number;
        // Make sure there's a leading + on addresses with a TOA of 145
        if (address != null && address.length() > 0) {
            cw.number = processPlusCodeForWaitingCall(address, cw.numberType);
        }
    }

    private String processPlusCodeForWaitingCall(String number, int numberType) {
        String format = PlusCodeProcessor.getPlusCodeUtils().removeIddNddAddPlusCode(number);
        if (format != null) {
            number = format;
            if (numberType == 1 && format.length() > 0 && format.charAt(0) != '+') {
                number = "+" + format;
            }
        }
        return number;
    }

    private boolean needToConvert(String source) {
        String target = GsmCdmaConnection.formatDialString(source);
        return source != null && target != null && !source.equals(target);
    }
    /// @}

    /// M: CC: CDMA waiting call @{
    private boolean shouldNotifyWaitingCall(CdmaCallWaitingNotification cw) {
        String address = cw.number;
        Rlog.d(PROP_LOG_TAG, "shouldNotifyWaitingCall");
        if (address != null && address.length() > 0) {
            GsmCdmaConnection lastRingConn
                    = (GsmCdmaConnection) (mRingingCall.getLatestConnection());
            if (lastRingConn != null) {
                if (address.equals(lastRingConn.getAddress())) {
                    Rlog.d(PROP_LOG_TAG, "handleCallWaitingInfo, skip duplicate waiting call!");
                    return false;
                }
            }
        }
        return true;
    }
    /// @}

    // IMS conference SRVCC
    @Override
    protected void updatePhoneState() {
        PhoneConstants.State oldState = mState;
        if (mRingingCall.isRinging()) {
            mState = PhoneConstants.State.RINGING;
        } else if (mPendingMO != null ||
                !(mForegroundCall.isIdle() && mBackgroundCall.isIdle())) {
            mState = PhoneConstants.State.OFFHOOK;
        } else {
            Phone imsPhone = mPhone.getImsPhone();
            // ALPS02192901
            // If the call is disconnected after CIREPH=1, before +CLCC, the original state is
            // idle and new state is still idle, so callEndCleanupHandOverCallIfAny isn't called.
            // Related CR: ALPS02015368, ALPS02161020, ALPS02192901.
            // if ( mState == PhoneConstants.State.OFFHOOK && (imsPhone != null)){
            if (imsPhone != null) {
                imsPhone.callEndCleanupHandOverCallIfAny();
            }
            mState = PhoneConstants.State.IDLE;
        }

        if (mState == PhoneConstants.State.IDLE && oldState != mState) {
            mVoiceCallEndedRegistrants.notifyRegistrants(
                    new AsyncResult(null, null, null));
        } else if (oldState == PhoneConstants.State.IDLE && oldState != mState) {
            mVoiceCallStartedRegistrants.notifyRegistrants (
                    new AsyncResult(null, null, null));
        }
        if (Phone.DEBUG_PHONE) {
            log("update phone state, old=" + oldState + " new="+ mState);
        }
        if (mState != oldState) {
            mPhone.notifyPhoneStateChanged();
            mMetrics.writePhoneState(mPhone.getPhoneId(), mState);
        }
    }
    @Override
    protected void notifySrvccState(Call.SrvccState state, ArrayList<Connection> c) {
        if (state == Call.SrvccState.STARTED && c != null) {
            // SRVCC started. Prepare handover connections list
            mHandoverConnections.addAll(c);
            // ALPS02019630. For IMS conference SRVCC. ECONFSRVCC is only for conference
            // host side.
            if (!hasParsingCEPCapability()) {
                for (Connection conn : mHandoverConnections) {
                    if (conn.isMultiparty() && conn instanceof MtkImsPhoneConnection
                            && conn.isConferenceHost()) {
                        log("srvcc: mNeedWaitImsEConfSrvcc set True");
                        mNeedWaitImsEConfSrvcc = true;
                        mImsConfHostConnection = conn;
                    }
                }
            }
        } else if (state != Call.SrvccState.COMPLETED) {
            // SRVCC FAILED/CANCELED. Clear the handover connections list
            // Individual connections will be removed from the list in handlePollCalls()
            mHandoverConnections.clear();
        }
        log("notifySrvccState: mHandoverConnections= " + mHandoverConnections.toString());
    }

    @Override
    protected Connection getHoConnection(DriverCall dc) {
        if (dc == null) {
            return null;
        }
        /*
         CallTracker.getHoConnection is used to find the original connection before SRVCC. It finds
         by call number and call state. But in conference SRVCC case, the call number is null and
         the call state might be different.
        */
        if (mEconfSrvccConnectionIds != null && dc != null) {
            int numOfParticipants = mEconfSrvccConnectionIds[0];
            for (int index = 1; index <= numOfParticipants; index++) {
                if (dc.index == mEconfSrvccConnectionIds[index]) {
                    Rlog.d(PROP_LOG_TAG, "SRVCC: getHoConnection for call-id:"
                            + dc.index + " in a conference is found!");
                    if (mImsConfHostConnection == null) {
                        Rlog.d(PROP_LOG_TAG, "SRVCC: but mImsConfHostConnection is null, " +
                                "try to find by callState");
                        break;
                    } else {
                        // address and cnap in Connection will be printed if log level = D
                        Rlog.v(PROP_LOG_TAG, "SRVCC: ret= " + mImsConfHostConnection);
                        return mImsConfHostConnection;
                    }
                }
            }
        }

        // ALPS01995466. JE because dc.number is null.
        //log("SRVCC: getHoConnection() with dc, number = " + dc.number + " state = " + dc.state);
        if (dc.number != null && !dc.number.isEmpty()) {
            for (Connection hoConn : mHandoverConnections) {
                log("getHoConnection - compare number: hoConn= " + hoConn.toString());
                if (hoConn.getAddress() != null && hoConn.getAddress().contains(dc.number)) {
                    log("getHoConnection: Handover connection match found = " + hoConn.toString());
                    return hoConn;
                }
            }
        }
        for (Connection hoConn : mHandoverConnections) {
            log("getHoConnection: compare state hoConn= " + hoConn.toString());
            if (hoConn.getStateBeforeHandover() == Call.stateFromDCState(dc.state)) {
                log("getHoConnection: Handover connection match found = " + hoConn.toString());
                return hoConn;
            }
        }
        return null;
    }

    /**
     * For conference participants, the call number will be empty after SRVCC.
     * So at handlePollCalls(), it will get new connections without address. We use +ECONFSRVCC
     * and conference XML to restore all addresses.
     *
     * @return true if connections are restored.
     */
    private synchronized boolean restoreConferenceParticipantAddress() {
        if (mEconfSrvccConnectionIds == null) {
            Rlog.d(PROP_LOG_TAG, "SRVCC: restoreConferenceParticipantAddress():" +
                    "ignore because mEconfSrvccConnectionIds is empty");
            return false;
        }

        boolean finishRestore = false;

        // int[] mEconfSrvccConnectionIds = { size, call-ID-1, call-ID-2, call-ID-3, ...}
        int numOfParticipants = mEconfSrvccConnectionIds[0];
        for (int index = 1; index <= numOfParticipants; index++) {

            int participantCallId = mEconfSrvccConnectionIds[index];
            GsmCdmaConnection participantConnection = mConnections[participantCallId - 1];

            if (participantConnection != null) {
                Rlog.d(PROP_LOG_TAG, "SRVCC: found conference connections!");

                MtkImsPhoneConnection hostConnection = null;
                if (participantConnection.mOrigConnection instanceof MtkImsPhoneConnection) {
                    hostConnection = (MtkImsPhoneConnection) participantConnection.mOrigConnection;
                } else {
                    Rlog.v(PROP_LOG_TAG, "SRVCC: host is abnormal, ignore connection: " +
                            participantConnection);
                    continue;
                }

                if (hostConnection == null) {
                    Rlog.v(PROP_LOG_TAG,
                            "SRVCC: no host, ignore connection: " + participantConnection);
                    continue;
                }

                String address = hostConnection.getConferenceParticipantAddress(index - 1);
                if (participantConnection instanceof MtkGsmCdmaConnection) {
                    ((MtkGsmCdmaConnection) participantConnection).
                            updateConferenceParticipantAddress(address);
                }
                finishRestore = true;

                Rlog.v(PROP_LOG_TAG, "SRVCC: restore Connection=" + participantConnection +
                        " with address:" + address);
            }
        }

        return finishRestore;
    }

    boolean hasParsingCEPCapability() {
        MtkHardwareConfig modem =
                ((MtkHardwareConfig) mTelDevController.getModem(0));
        if (modem == null) {
            return false;
        }
        return modem.hasParsingCEPCapability();
    }
    // end of IMS conference SRVCC

    public int getHandoverConnectionSize() {
        return mHandoverConnections.size();
    }
}

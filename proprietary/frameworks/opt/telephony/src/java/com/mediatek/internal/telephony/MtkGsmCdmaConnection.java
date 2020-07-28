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

import android.content.Context;
import android.os.AsyncResult;
import android.os.Message;
import android.os.Registrant;
import android.os.SystemClock;
import android.telephony.DisconnectCause;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.CallFailCause;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.DriverCall;
import com.android.internal.telephony.GsmCdmaCall;
import com.android.internal.telephony.GsmCdmaCallTracker;
import com.android.internal.telephony.GsmCdmaConnection;
import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.Connection.PostDialState;
import com.android.internal.telephony.cdma.CdmaCallWaitingNotification;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppState;
import com.android.internal.telephony.uicc.UiccCardApplication;

import com.mediatek.telephony.MtkTelephonyManagerEx;

public class MtkGsmCdmaConnection extends GsmCdmaConnection {

    private static final String PROP_LOG_TAG = "GsmCdmaConn";

    /// M: CC: Forwarding number via EAIC
    String mForwardingAddress;
    /// @}

    /// M: CC: Proprietary CRSS handling @{
    /// M: CC: Redirecting number via COLP
    String mRedirectingAddress;
    /// @}

    /// M: CC: Softbank blacklist requirement
    int mRejectCauseToRIL = -1;

    /// M: CC: CDMA call accepted @{
    private static final int MO_CALL_VIBRATE_TIME = 200;  // msec
    private boolean mIsRealConnected; // Indicate if the MO call has been accepted by remote side
    private boolean mReceivedAccepted; // Indicate if we receive call accepted event
    /// @}

    private boolean mIsIncomingCallGwsd = false;

    public synchronized boolean isIncomingCallGwsd() {
        Rlog.d(LOG_TAG, "isIncomingCallGwsd: " + mIsIncomingCallGwsd);
        return mIsIncomingCallGwsd;
    }

    /** This is an MO call, created when dialing */
    public MtkGsmCdmaConnection(GsmCdmaPhone phone, String dialString, GsmCdmaCallTracker ct,
            GsmCdmaCall parent, boolean isEmergencyCall) {
        super(phone, dialString, ct, parent, isEmergencyCall);

        /// M: CC: CDMA call accepted @{
        mIsRealConnected = false;
        mReceivedAccepted = false;
        /// @}
    }

    /** This is probably an MT call that we first saw in a CLCC response or a hand over. */
    public MtkGsmCdmaConnection(GsmCdmaPhone phone, DriverCall dc, GsmCdmaCallTracker ct,
            int index) {
        super(phone, dc, ct, index);

        if (((MtkGsmCdmaCallTracker) ct).mHelper.isGwsdCall()) {
            mIsIncomingCallGwsd = true;
            ((MtkGsmCdmaCallTracker) ct).mHelper.setGwsdCall(false);
        }
        /// M: CC: Reconstruct MT address by certain format @{
        String origAddress = ((MtkGsmCdmaCallTracker) ct).mMtkGsmCdmaCallTrackerExt
                .convertAddress(mAddress);
        if (origAddress != null) {
            setConnectionExtras(((MtkGsmCdmaCallTracker) ct).mMtkGsmCdmaCallTrackerExt
                    .getAddressExtras(mAddress));
            // same as setConverted(origAddress), except not to set mDialString
            mNumberConverted = true;
            mConvertedNumber = mAddress;
            mAddress = origAddress;
        }
        /// @}
    }

    //CDMA
    /** This is a Call waiting call*/
    public MtkGsmCdmaConnection(Context context, CdmaCallWaitingNotification cw,
            GsmCdmaCallTracker ct, GsmCdmaCall parent) {
        super(context, cw, ct, parent);
    }

    /*package*/ public boolean
    compareTo(DriverCall c) {
        // On mobile originated (MO) calls, the phone number may have changed
        // due to a SIM Toolkit call control modification.
        //
        // We assume we know when MO calls are created (since we created them)
        // and therefore don't need to compare the phone number anyway.
        if (! (mIsIncoming || c.isMT)) return true;

        // A new call appearing by SRVCC may have invalid number
        //  if IMS service is not tightly coupled with cellular modem stack.
        // Thus we prefer the preexisting handover connection instance.
        if (isPhoneTypeGsm() && mOrigConnection != null) return true;

        // ... but we can compare phone numbers on MT calls, and we have
        // no control over when they begin, so we might as well

        String cAddress = PhoneNumberUtils.stringFromStringAndTOA(c.number, c.TOA);

        /// M: CC: Digits checks mNumberConverted and ignore mAddress comparision @{
        //return mIsIncoming == c.isMT && equalsHandlesNulls(mAddress, cAddress);

        boolean addrChanged2 = ((MtkGsmCdmaCallTracker) mOwner).mMtkGsmCdmaCallTrackerExt
                .isAddressChanged(mNumberConverted, mAddress, cAddress);

        return mIsIncoming == c.isMT && !addrChanged2;
        /// @}
    }

    /// M: CC: Forwarding number via EAIC @{
    /**
     * Gets forwarding address (e.g. phone number) associated with connection.
     * A makes call to B and B redirects(Forwards) this call to C, the forwarding address is B.
     * @return address or null if unavailable
    */
    public String getForwardingAddress() {
       return mForwardingAddress;
    }

    /**
     * Sets forwarding address (e.g. phone number) associated with connection.
     * A makes call to B and B redirects(Forwards) this call to C, the forwarding address is B.
    */
    public void setForwardingAddress(String address) {
       mForwardingAddress = address;
    }
    /// @}

    /// M: CC: Proprietary CRSS handling @{
    /// M: CC: Redirecting number via COLP
    /**
     * Gets redirecting address (e.g. phone number) associated with connection.
     *
     * @return address or null if unavailable
    */
    public String getRedirectingAddress() {
       return mRedirectingAddress;
    }

    /**
     * Sets redirecting address (e.g. phone number) associated with connection.
     *
    */
    public void setRedirectingAddress(String address) {
        mRedirectingAddress = address;
    }

    // Returns true if state has changed, false if nothing changed
    public boolean
    update (DriverCall dc) {
        GsmCdmaCall newParent;
        boolean changed = false;
        boolean wasConnectingInOrOut = isConnectingInOrOut();
        boolean wasHolding = (getState() == GsmCdmaCall.State.HOLDING);

        newParent = parentFromDCState(dc.state);

        if (Phone.DEBUG_PHONE) log("parent= " +mParent +", newParent= " + newParent);

        //Ignore dc.number and dc.name in case of a handover connection
        if (isPhoneTypeGsm() && mOrigConnection != null) {
            if (Phone.DEBUG_PHONE) log("update: mOrigConnection is not null");
        } else if (isIncoming()) {
            /// M: CC: Digits checks mAddress & mNumberConverted only, ignore mConvertedNumber @{
            //if (!equalsBaseDialString(mAddress, dc.number) && (!mNumberConverted
            //        || !equalsBaseDialString(mConvertedNumber, dc.number))) {
            log(" mNumberConverted " + mNumberConverted);
            boolean addrChanged = ((MtkGsmCdmaCallTracker) mOwner).mMtkGsmCdmaCallTrackerExt
                    .isAddressChanged(mNumberConverted, dc.number, mAddress, mConvertedNumber);

            if (addrChanged) {
         /// @}
                if (Phone.DEBUG_PHONE) log("update: phone # changed!");
                mAddress = dc.number;
                changed = true;
            }
        }

        int newAudioQuality = getAudioQualityFromDC(dc.audioQuality);
        if (getAudioQuality() != newAudioQuality) {
            if (Phone.DEBUG_PHONE) {
                log("update: audioQuality # changed!:  "
                        + (newAudioQuality == Connection.AUDIO_QUALITY_HIGH_DEFINITION
                        ? "high" : "standard"));
            }
            setAudioQuality(newAudioQuality);
            changed = true;
        }

        // A null cnapName should be the same as ""
        if (TextUtils.isEmpty(dc.name)) {
            /// M: CC: CLCC without name information handling @{
            /* Name information is not updated by +CLCC, dc.name will be empty always,
               so ignore the following statements */
            //if (!TextUtils.isEmpty(mCnapName)) {
            //    changed = true;
            //    mCnapName = "";
            //}
            /// @}
        } else if (!dc.name.equals(mCnapName)) {
            changed = true;
            mCnapName = dc.name;
        }

        if (Phone.DEBUG_PHONE) log("--dssds----"+mCnapName);
        mCnapNamePresentation = dc.namePresentation;
        mNumberPresentation = dc.numberPresentation;

        if (newParent != mParent) {
            if (mParent != null) {
                mParent.detach(this);
            }
            newParent.attach(this, dc);
            mParent = newParent;
            changed = true;
        } else {
            boolean parentStateChange;
            parentStateChange = mParent.update (this, dc);
            changed = changed || parentStateChange;
        }

        /** Some state-transition events */

        if (Phone.DEBUG_PHONE) log(
                "update: parent=" + mParent +
                ", hasNewParent=" + (newParent != mParent) +
                ", wasConnectingInOrOut=" + wasConnectingInOrOut +
                ", wasHolding=" + wasHolding +
                ", isConnectingInOrOut=" + isConnectingInOrOut() +
                ", changed=" + changed);


        if (wasConnectingInOrOut && !isConnectingInOrOut()) {
            onConnectedInOrOut();
        }

        if (changed && !wasHolding && (getState() == GsmCdmaCall.State.HOLDING)) {
            // We've transitioned into HOLDING
            onStartedHolding();
        }

        /// M: CC: CDMA call accepted @{
        if (!isPhoneTypeGsm()) {
            log("state=" + getState() + ", mReceivedAccepted=" + mReceivedAccepted);
            if (getState() == GsmCdmaCall.State.ACTIVE && mReceivedAccepted) {
                onCdmaCallAccepted();
                mReceivedAccepted = false;
            }
        }
        /// @}

        return changed;
    }

    @Override
    protected void
    processNextPostDialChar() {
        char c = 0;
        Registrant postDialHandler;

        if (mPostDialState == PostDialState.CANCELLED) {
            releaseWakeLock();
            return;
        }

        if (mPostDialString == null ||
                mPostDialString.length() <= mNextPostDialChar ||
                /// M: CC: DTMF request special handling @{
                // [ALPS00093395] Stop processNextPostDialChar when conn is disconnected
                mDisconnected == true) {
                /// @}

            setPostDialState(PostDialState.COMPLETE);

            // We were holding a wake lock until pause-dial was complete, so give it up now
            releaseWakeLock();

            // notifyMessage.arg1 is 0 on complete
            c = 0;
        } else {
            boolean isValid;

            setPostDialState(PostDialState.STARTED);

            c = mPostDialString.charAt(mNextPostDialChar++);

            isValid = processPostDialChar(c);

            if (!isValid) {
                // Will call processNextPostDialChar
                mHandler.obtainMessage(EVENT_NEXT_POST_DIAL).sendToTarget();
                // Don't notify application
                Rlog.e(LOG_TAG, "processNextPostDialChar: c=" + c + " isn't valid!");
                return;
            }
        }

        notifyPostDialListenersNextChar(c);

        // TODO: remove the following code since the handler no longer executes anything.
        postDialHandler = mOwner.getPhone().getPostDialHandler();

        Message notifyMessage;

        if (postDialHandler != null
                && (notifyMessage = postDialHandler.messageForRegistrant()) != null) {
            // The AsyncResult.result is the Connection object
            PostDialState state = mPostDialState;
            AsyncResult ar = AsyncResult.forMessage(notifyMessage);
            ar.result = this;
            ar.userObj = state;

            // arg1 is the character that was/is being processed
            notifyMessage.arg1 = c;

            // Rlog.v("GsmCdma", "##### processNextPostDialChar: send msg to
            // postDialHandler, arg1=" + c);
            notifyMessage.sendToTarget();
        }
    }

    /// M: CC: CDMA call accepted @{
    /**
     * Check if this connection is really connected.
     * @return true if this connection is really connected, or return false.
     * @hide
     */
    public boolean isRealConnected() {
        return mIsRealConnected;
    }

    boolean onCdmaCallAccepted() {
        log("onCdmaCallAccepted, mIsRealConnected=" + mIsRealConnected
                + ", state=" + getState());
        if (getState() != GsmCdmaCall.State.ACTIVE) {
            mReceivedAccepted = true;
            return false;
        }
        if (!mIsRealConnected) {
            mIsRealConnected = true;
            // send DTMF when the CDMA call is really accepted.
            processNextPostDialChar();
            vibrateForAccepted();
        }
        return true;
    }

    private boolean isInChina() {
        int phoneId = mParent.getPhone().getPhoneId();
        String numeric = TelephonyManager.getDefault().getNetworkOperatorForPhone(phoneId);
        String countryIso = "";
        if (TextUtils.isEmpty(numeric)) {
            numeric = MtkTelephonyManagerEx.getDefault().getLocatedPlmn(phoneId);
        }
        log("isInChina, numeric=" + Rlog.pii(PROP_LOG_TAG, numeric));
        if (TextUtils.isEmpty(numeric)) {
            countryIso = TelephonyManager.getDefault().getNetworkCountryIsoForPhone(phoneId);
            log("isInChina, countryIso=" + Rlog.pii(PROP_LOG_TAG, countryIso));
        }
        return (numeric != null && numeric.indexOf("460") == 0) || "cn".equals(countryIso);
    }

    private void vibrateForAccepted() {
        String prop = android.os.SystemProperties.get("persist.vendor.radio.telecom.vibrate", "1");
        if ("0".equals(prop)) {
            log("vibrateForAccepted, disabled by Engineer Mode");
            return;
        }

        //if CDMA phone accepted, start a Vibrator
        android.os.Vibrator vibrator
                = (android.os.Vibrator) mParent.getPhone().getContext().getSystemService(
                        Context.VIBRATOR_SERVICE);
        vibrator.vibrate(MO_CALL_VIBRATE_TIME);
    }
    /// @}

    @Override
    public void onConnectedInOrOut() {
        mConnectTime = System.currentTimeMillis();
        mConnectTimeReal = SystemClock.elapsedRealtime();
        mDuration = 0;

        // bug #678474: incoming call interpreted as missed call, even though
        // it sounds like the user has picked up the call.
        if (Phone.DEBUG_PHONE) {
            log("onConnectedInOrOut: connectTime=" + mConnectTime);
        }

        if (!mIsIncoming) {
            // outgoing calls only
            /// M: CC: CDMA call accepted @{
            if (isPhoneTypeGsm()) {
                processNextPostDialChar();
            } else {
                // send DTMF when the CDMA call is really accepted.
                int count = mParent.mConnections.size();
                log("mParent.mConnections.size()=" + count);
                if (!isInChina() && !mIsRealConnected && count == 1) {
                    mIsRealConnected = true;
                    processNextPostDialChar();
                    vibrateForAccepted();
                }
                if (count > 1) {
                    mIsRealConnected = true;
                    processNextPostDialChar();
                }
            }
            /// @}
        } else {
            // Only release wake lock for incoming calls, for outgoing calls the wake lock
            // will be released after any pause-dial is completed
            releaseWakeLock();
        }
    }

    /// M: CC: Proprietary CRSS handling @{
    /**
     * Called when this Connection is fail to enter backgroundCall,
     * because we switch fail.
     * (We think we're going to end up HOLDING in the backgroundCall when dial is initiated)
     */
    void
    resumeHoldAfterDialFailed() {
        if (mParent != null) {
            mParent.detach(this);
        }

        mParent = mOwner.mForegroundCall;
        mParent.attachFake(this, GsmCdmaCall.State.ACTIVE);
    }
    /// @}

    // IMS Conference SRVCC
    void updateConferenceParticipantAddress(String address) {
        mAddress = address;
    }

    @Override
    public boolean isMultiparty() {
        // For IMS SRVCC. mOrigConnection is used when SRVCC, but its isMultiparty() should not be
        // believed
        // if (mOrigConnection != null) {
        //    return mOrigConnection.isMultiparty();
        // }
        if (mParent != null) {
            return mParent.isMultiparty();
        }
        /// @}

        return false;
    }

    /// M: CC: Softbank blacklist requirement @{
    public void setRejectWithCause(int telephonyDisconnectCode) {
        if (mParent != null && mOwner != null) {
            GsmCdmaPhone phone = mOwner.getPhone();
            if (MtkIncomingCallChecker.isMtkEnhancedCallBlockingEnabled(phone.getContext(),
                    phone.getSubId())) {
                Rlog.d(PROP_LOG_TAG, "setRejectWithCause set (" + mRejectCauseToRIL + " to "
                        + telephonyDisconnectCode + ")");
                mRejectCauseToRIL = telephonyDisconnectCode;
            }
        } else {
            Rlog.d(PROP_LOG_TAG, "setRejectWithCause fail. mParent(" + mParent
                    + "), mOwner(" + mOwner + ")");
        }
    }

    public int getRejectWithCause() {
        return mRejectCauseToRIL;
    }

    public void clearRejectWithCause() {
        if (mRejectCauseToRIL != -1) {
            Rlog.d(PROP_LOG_TAG, "clearRejectWithCause (" + mRejectCauseToRIL + " to -1)");
            mRejectCauseToRIL = -1;
        }
    }

    public void onHangupLocal() {
        clearRejectWithCause();
        super.onHangupLocal();
    }
    /// @}
}

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

package com.mediatek.internal.telephony.imsphone;

import android.net.Uri;
import android.os.Looper;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.PersistableBundle;
import android.telecom.ConferenceParticipant;
import android.telephony.DisconnectCause;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.CarrierConfigManager;
import android.text.TextUtils;
import android.content.Context;

import com.android.ims.ImsException;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.imsphone.ImsPhoneCall;
import com.android.internal.telephony.imsphone.ImsPhoneCallTracker;
import com.android.internal.telephony.imsphone.ImsPhoneConnection;
import com.android.internal.telephony.imsphone.ImsRttTextHandler;

import com.android.ims.ImsCall;
import com.mediatek.ims.MtkImsCall;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import mediatek.telephony.MtkCarrierConfigManager;

/**
 * {@hide}
 */
public class MtkImsPhoneConnection extends ImsPhoneConnection {
    private static final String LOG_TAG = "MtkImsPhoneConnection";
    /// M: @{
    // For VoLTE enhanced conference call.
    private ArrayList<String> mConfDialStrings = null;

    // For conference SRVCC.
    private List<ConferenceParticipant> mConferenceParticipants = null;

    // ALPS02136981. Prints debug logs for ImsPhone.
    private int mCallIdBeforeDisconnected = -1;

    // ALPS02501206. For OP07 requirement.
    private String mVendorCause = null;

    // for Ims Conference SRVCC.
    public boolean mWasMultiparty = false;
    public boolean mWasPreMultipartyHost = false;
    /// @}

    /// M: video ringtone @{
    public static final int SUPPORTS_VT_RINGTONE = 0x00000040;
    /// @}

    // Carrier config for switch between RTT and video call
    private boolean mIsRttVideoSwitchSupported = true;

    private boolean mIsIncomingCallGwsd = false;
    private static final String EXTRA_IMS_GWSD = "ims_gwsd";

    //***** Constructors

    /** This is probably an MT call */
    public MtkImsPhoneConnection(Phone phone, ImsCall imsCall, ImsPhoneCallTracker ct,
                                 ImsPhoneCall parent, boolean isUnknown) {
        super(phone, imsCall, ct, parent, isUnknown);

        if ((imsCall != null) && (imsCall.getCallProfile() != null)) {
            mIsIncomingCallGwsd =
                    imsCall.getCallProfile().getCallExtraInt(EXTRA_IMS_GWSD) == 1;
        }

        fetchIsRttVideoSwitchSupported(phone);
    }

    /** This is an MO call, created when dialing */
    public MtkImsPhoneConnection(Phone phone, String dialString, ImsPhoneCallTracker ct,
                              ImsPhoneCall parent, boolean isEmergency) {
        super(phone, dialString, ct, parent, isEmergency);

        // Ignore extraction for VoLTE SIP address which is NOT a PSTN phone number.
        if (PhoneNumberUtils.isUriNumber(dialString)) {
            mAddress = dialString;
            mPostDialString = "";
        }

        fetchIsRttVideoSwitchSupported(phone);
    }

    public synchronized boolean isIncomingCallGwsd() {
        Rlog.d(LOG_TAG, "isIncomingCallGwsd: " + mIsIncomingCallGwsd);
        return mIsIncomingCallGwsd;
    }

    @Override
    public String getVendorDisconnectCause() {
        /// M: ALPS02501206. For OP07 requirement. @{
        // return null;
        return mVendorCause;
        /// @}
    }

    @Override
    public void hangup() throws CallStateException {
        /// M: ALPS02136981. Prints debug logs for ImsPhone. @{
        if (mOwner != null && mOwner instanceof MtkImsPhoneCallTracker) {
            ((MtkImsPhoneCallTracker) mOwner).logDebugMessagesWithOpFormat("CC",
                    "Hangup", this, "MtkImsphoneConnection.hangup");
        }
        /// @}

        super.hangup();
    }

    @Override
    public boolean onDisconnect() {
        if (!mDisconnected) {
            /// M: ALPS02136981. Prints debug logs for ImsPhone. @{
            // Cache call ID before close mImsCall.
            mCallIdBeforeDisconnected = getCallId();
            /// @}
        }

        return super.onDisconnect();
    }

    /**
     * Notifies this Connection of a request to disconnect a participant of the conference managed
     * by the connection.
     *
     * @param endpoint the {@link android.net.Uri} of the participant to disconnect.
     */
    @Override
    public void onDisconnectConferenceParticipant(Uri endpoint) {
        /// M: ALPS02136981. Prints debug logs for ImsPhone. @{
        if (mOwner != null && mOwner instanceof MtkImsPhoneCallTracker) {
            ((MtkImsPhoneCallTracker) mOwner).logDebugMessagesWithOpFormat("CC", "RemoveMember",
                    this, " remove: " + MtkImsPhoneCallTracker.sensitiveEncode("" + endpoint));
        }
        /// @}
        super.onDisconnectConferenceParticipant(endpoint);
    }

    /**
     * Check for a change in the address display related fields for the {@link ImsCall}, and
     * update the {@link ImsPhoneConnection} with this information.
     *
     * @param imsCall The call to check for changes in address display fields.
     * @return Whether the address display fields have been changed.
     */
    @Override
    public boolean updateAddressDisplay(ImsCall imsCall) {
        boolean changed = super.updateAddressDisplay(imsCall);
        if (changed) {
            setConnectionAddressDisplay();
        }
        return changed;
    }

    /**
     * Provides a string representation of the {@link ImsPhoneConnection}.  Primarily intended for
     * use in log statements.
     *
     * @return String representation of call.
     */
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder(super.toString());
        /// M: @{
        sb.append(" state:" + getState());
        sb.append(" mParent:");
        sb.append(getParentCallName());
        /// @}
        return sb.toString();
    }

    /// M: MTK added functions @{
    /**
     * get Call Id of this connection. Used when SRVCC, GSMCallTracker needs to know the mapping
     * between old Ims connection and new Gsm Connection, according to Call-ID.
     *
     * @return Call Id
     * @hide
     */
    /* package */
    int getCallId() {
        ImsCall call = getImsCall();
        if (call == null || call.getCallSession() == null) {
            return -1;
        }

        String callId = call.getCallSession().getCallId();
        if (callId == null) {
            Rlog.d(LOG_TAG, "Abnormal! Call Id = null");
            return -1;
        }

        return Integer.parseInt(callId);
    }

    /**
     * Now the connection is disconnected, get the Call Id before disconnected.
     *
     * @return call ID
     * @hide
     */
    /* package */
    int getCallIdBeforeDisconnected() {
        return mCallIdBeforeDisconnected;
    }

    /**
     * For VoLTE enhanced conference call.
     *
     * @return the array of the conference dial strings.
     */
    /* package */
    ArrayList<String> getConfDialStrings() {
        return mConfDialStrings;
    }

    /**
     * After conference SRVCC, we need to restore the participants' address from XML.
     *
     * @param index the index(order) in the XML, start from 0.
     * @return String the conference participant's address.
     * @hide
     */
    public String getConferenceParticipantAddress(int index) {
        String emptyAddress = "";

        if (mConferenceParticipants == null) {
            Rlog.d(LOG_TAG, "getConferenceParticipantAddress(): no XML information");
            return emptyAddress;
        }

        if (index < 0 || index + 1 >= mConferenceParticipants.size()) {
            Rlog.d(LOG_TAG, "getConferenceParticipantAddress(): invalid index");
            return emptyAddress;
        }

        // ToDo: how to know which one is the host? we assume the first one is always host.
        ConferenceParticipant participant = mConferenceParticipants.get(index + 1);
        if (participant == null) {
            Rlog.d(LOG_TAG, "getConferenceParticipantAddress(): empty participant info");
            return emptyAddress;
        }

        Uri userEntity = participant.getHandle();
        Rlog.d(LOG_TAG, "getConferenceParticipantAddress(): ret=" +
                MtkImsPhoneCallTracker.sensitiveEncode("" + userEntity));
        return userEntity.toString();
    }

    /* package */
    String getParentCallName() {
        if (mOwner == null) {
            return "Unknown";
        }

        if (mParent == mOwner.mForegroundCall) {
            return "Foreground Call";
        } else if (mParent == mOwner.mBackgroundCall) {
            return "Background Call";
        } else if (mParent == mOwner.mRingingCall) {
            return "Ringing Call";
        } else if (mParent == mOwner.mHandoverCall) {
            return "Handover Call";
        } else {
            return "Abnormal";
        }
    }

    // For Ims Conference SRVCC
    /**
     * If this connection went through handover return the conference host state
     *  of the call that contained this connection before handover.
     * @return boolean is conference host or not.
     * @hide
     */
    public boolean isConfHostBeforeHandover() {
        return mWasPreMultipartyHost;
    }

    /**
     * If this connection went through handover return the isMultiparty state
     *  of the call that contained this connection before handover.
     * @return boolean is multiparty or not.
     * @hide
     */
    public boolean isMultipartyBeforeHandover() {
        return mWasMultiparty;
    }

    /**
     * Returns whether the ImsPhoneConnection was a conference incoming call.
     *
     * @return true if ImsPhoneConnection is a conference incoming call.
     * @hide
     */
    public synchronized boolean isIncomingCallMultiparty() {
        return mImsCall != null && mImsCall instanceof MtkImsCall
                && ((MtkImsCall)mImsCall).isIncomingCallMultiparty();
    }

    // For VoLTE enhanced conference call. @{
    /**
     * Invite one or more participants to the conference managed by this connection.
     *
     * @param numbers the numbers to invite into this conference.
     * @hide
     */
    public void inviteConferenceParticipants(List<String> numbers) {
        // ALPS02136981. Prints debug logs for ImsPhone. @{
        StringBuilder sb = new StringBuilder();
        for (String number : numbers) {
            sb.append(number);
            sb.append(", ");
        }
        if (mOwner != null && mOwner instanceof MtkImsPhoneCallTracker) {
            ((MtkImsPhoneCallTracker)mOwner).logDebugMessagesWithOpFormat("CC", "AddMember", this,
                    " invite with " + MtkImsPhoneCallTracker.sensitiveEncode(sb.toString()));
        }

        ImsCall imsCall = getImsCall();
        if (imsCall == null) {
            return;
        }

        ArrayList<String> list = new ArrayList<String>();
        for (String str : numbers) {
            if (PhoneNumberUtils.isUriNumber(str) || isTestSim()) {
                list.add(str);
            } else {
                list.add(PhoneNumberUtils.extractNetworkPortionAlt(str));
            }
        }

        String[] participants = list.toArray(new String[list.size()]);
        try {
            imsCall.inviteParticipants(participants);
        } catch (ImsException e) {
            Rlog.e(LOG_TAG,
                    "inviteConferenceParticipants: no call session and fail to invite participants "
                    + MtkImsPhoneCallTracker.sensitiveEncode(Arrays.toString(participants)));
        }
    }

    /**
     * For VoLTE enhanced conference call.
     *
     * @param dialStrings the dial strings of multiple MO.
     */
    /* package */
    void setConfDialStrings(ArrayList<String> dialStrings) {
        mConfDialStrings = dialStrings;
    }

    // ALPS02501206. For OP07 requirement. @{
    /* package */
    void setVendorDisconnectCause(String cause) {
        mVendorCause = cause;
    }

    @Override
    public void updateConferenceParticipants(List<ConferenceParticipant> conferenceParticipants) {
        mConferenceParticipants = conferenceParticipants;
        super.updateConferenceParticipants(conferenceParticipants);
    }

    /**
     * Mtk extends base listener implementation.
     */
    public abstract static class MtkListenerBase extends ListenerBase {
        // For VoLTE conference
        /**
         * For VoLTE enhanced conference call, notify invite conf. participants completed.
         * @param isSuccess is success or not.
         * @hide
         */
        public void onConferenceParticipantsInvited(boolean isSuccess) {}

        /**
         * For VoLTE conference SRVCC, notify the new participant connections in GsmPhone.
         * @param radioConnections the participant connections in GsmPhone
         * @hide
         */
        public void onConferenceConnectionsConfigured(ArrayList<Connection> radioConnections) {}

        /**
         * For device switch, notify the device switch success or not
         * @param isSuccess Is device switch success
         * @hide
         */
        public void onDeviceSwitched(boolean isSuccess) {}

        public void onAddressDisplayChanged() {}

        /**
         * For updating RTT text capability.
         */
        public void onTextCapabilityChanged(int localCapability, int remoteCapability,
                int localTextStatus, int realRemoteTextCapability) {}

        /**
         * For notify dial as emergency call.
         */
        public void onRedialEcc(boolean isNeedUserConfirm) {}
    }

    /**
     * Notify when the task of onInviteConferenceParticipants() is completed.
     * @param isSuccess is success or not.
     * @hide
     */
    /* package */
    void notifyConferenceParticipantsInvited(boolean isSuccess) {
        for (Listener l : mListeners) {
            if (l instanceof MtkListenerBase) {
                ((MtkListenerBase) l).onConferenceParticipantsInvited(isSuccess);
            }
        }
    }

    /**
     * Notify when the new participant connections in GsmPhone are maded.
     * @param radioConnections new participant connections in GsmPhone
     * @hide
     */
    public void notifyConferenceConnectionsConfigured(ArrayList<Connection> radioConnections) {
        for (Listener l : mListeners) {
            if (l instanceof MtkListenerBase) {
                ((MtkListenerBase) l).onConferenceConnectionsConfigured(radioConnections);
            }
        }
    }

    public void notifyDeviceSwitched(boolean isSuccess) {
        for (Listener l : mListeners) {
            if (l instanceof MtkListenerBase) {
                ((MtkListenerBase) l).onDeviceSwitched(isSuccess);
            }
        }
    }

    public void notifyRedialEcc(boolean isNeedUserConfirm) {
        for (Listener l : mListeners) {
            if (l instanceof MtkListenerBase) {
                ((MtkListenerBase) l).onRedialEcc(isNeedUserConfirm);
            }
        }
    }

    /**
     * Notifies listeners of a change to the address or presentation mode of the connection.
     * @hide
     */
    private void setConnectionAddressDisplay() {
        for (Listener l : mListeners) {
            if (l instanceof MtkListenerBase) {
                ((MtkListenerBase) l).onAddressDisplayChanged();
            }
        }
    }

    @Override
    protected int applyVideoRingtoneCapabilities(ImsCallProfile remoteProfile,
            int capabilities) {
        int remoteCapabilities = capabilities;
        if (remoteProfile.mMediaProfile.mVideoDirection ==
                ImsStreamMediaProfile.DIRECTION_RECEIVE) {
            Rlog.d(LOG_TAG, "Set video ringtone capability");
            remoteCapabilities = addCapability(capabilities, SUPPORTS_VT_RINGTONE);
        } else {
            remoteCapabilities = removeCapability(capabilities, SUPPORTS_VT_RINGTONE);
        }
        return remoteCapabilities;
    }

    @Override
    protected boolean skipSwitchingCallToForeground() {
        if (mParent != mOwner.mHandoverCall) {
            Rlog.d(LOG_TAG, "update() - Switch Connection to foreground call:" + this);
            return false;
        }
        return true;
    }

    @Override
    protected void switchCallToBackgroundIfNecessary() {
        // ALPS02617050. Sometimes swap is triggered from lower layer. e.q. merge conference.
        if (mParent == mOwner.mForegroundCall) {
            Rlog.d(LOG_TAG, "update() - Switch Connection to background call:" + this);
            mParent.detach(this);
            mParent = mOwner.mBackgroundCall;
            mParent.attach(this);
        }
    }

    @Override
    protected int calNumberPresentation(ImsCallProfile callProfile) {
        int nump = ImsCallProfile.OIRToPresentation(
                callProfile.getCallExtraInt(ImsCallProfile.EXTRA_OIR));
        /// M: ALPS02583234, always show the callee's number. @{
        if (!mIsIncoming) {
            nump = PhoneConstants.PRESENTATION_ALLOWED;
        }
        /// @}
        return nump;
    }

    @Override
    protected boolean needUpdateAddress(String address) {
        if(!equalsBaseDialString(mAddress, address)) {
            Rlog.d(LOG_TAG, "update address = " + MtkImsPhoneCallTracker.sensitiveEncode(address)
                    + " isMpty = " + isMultiparty());
            // Do not update the address if the modified address is empty.
            // The address in callProfile will be empty if:
            // 1. Number and PAU from ECPI are both empty. In this case, the OIR will be
            //    set as PRESENTATION_RESTRICTED, and UI will indicate "Private number".
            // 2. Dial failed, the callProfile did not update by ImsCallSessionProxy.
            if (!TextUtils.isEmpty(address)) {
                return true;
            }
        }
        return false;
    }

    @Override
    protected boolean allowedUpdateMOAddress() {
        return true;
    }
    /// @}

    private boolean mIsIncomingCallDuringRttEmcGuard = false;

    /**
     * Set incoming call is in RTT EMC guard timer
     * @hide
     */
    public void setIncomingCallDuringRttEmcGuard(boolean isDuringRttGuard) {
        mIsIncomingCallDuringRttEmcGuard = isDuringRttGuard;
        Rlog.d(LOG_TAG, "setIncomingCallDuringRttEmcGuard: "
                + mIsIncomingCallDuringRttEmcGuard);
    }

    /**
     * Check the incoming call is in RTT EMC guard timer
     * @hide
     */
    public boolean isIncomingCallDuringRttEmcGuard() {
        Rlog.d(LOG_TAG, "isIncomingCallDuringRttEmcGuard: "
                + mIsIncomingCallDuringRttEmcGuard);
        return mIsIncomingCallDuringRttEmcGuard;
    }

    @Override
    public boolean updateMediaCapabilities(ImsCall imsCall) {
        boolean changed = super.updateMediaCapabilities(imsCall);

        if (changed) {
            Rlog.d(LOG_TAG, "updateMediaCapabilities capabilities = " + getConnectionCapabilities());
        }

        return changed;
    }

    @Override
    protected int applyLocalCallCapabilities(ImsCallProfile localProfile, int capabilities) {
        capabilities = super.applyLocalCallCapabilities(localProfile, capabilities);
        boolean isRttActive = isRttEnabledForCall();
        Rlog.d(LOG_TAG, "applyLocalCallCapabilities: isRttEnabledForCall=" + isRttActive
                + " mIsRttVideoSwitchSupported=" + mIsRttVideoSwitchSupported);
        if (isRttActive && !mIsRttVideoSwitchSupported) {
            capabilities = removeCapability(capabilities,
                Connection.Capability.SUPPORTS_VT_LOCAL_BIDIRECTIONAL);
        }
        return capabilities;
    }

    private void fetchIsRttVideoSwitchSupported(Phone phone) {
        CarrierConfigManager configMgr = (CarrierConfigManager)
            phone.getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle b = configMgr.getConfigForSubId(phone.getSubId());
        if (b != null) {
            mIsRttVideoSwitchSupported =
                b.getBoolean(CarrierConfigManager.KEY_RTT_SUPPORTED_FOR_VT_BOOL);
        }
    }

    /// M: ALPS04234226. For GCF case 15.19. @{
    /// TC_15.17 (Creating and leaving a conference)
    /// TC_15.19 (Inviting user to conference by sending a REFER request to the conference focus)
    /// Based on TC_15.17 that will create a empty conference by one-key launched, TC_15.19
    /// needs to invite a member to the conference in conference management screen. The invited
    /// number is not URI number, such as tel:12345;phone-context=test.3gpp.com, and these invited
    /// characters can NOT be changed.
    private boolean isTestSim() {
      boolean isTestSim = false;
      isTestSim = SystemProperties.get("vendor.gsm.sim.ril.testsim").equals("1") ||
                  SystemProperties.get("vendor.gsm.sim.ril.testsim.2").equals("1") ||
                  SystemProperties.get("vendor.gsm.sim.ril.testsim.3").equals("1") ||
                  SystemProperties.get("vendor.gsm.sim.ril.testsim.4").equals("1");
      Rlog.d(LOG_TAG, "isTestSim: " + isTestSim);
      return isTestSim;
    }
    /// @}
}

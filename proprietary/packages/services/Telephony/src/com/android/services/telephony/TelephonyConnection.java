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

import android.content.Context;
import android.graphics.drawable.Icon;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.telecom.CallAudioState;
import android.telecom.ConferenceParticipant;
import android.telecom.Connection;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.StatusHints;
import android.telecom.TelecomManager;
import android.telecom.VideoProfile;
import android.telephony.CarrierConfigManager;
import android.telephony.DisconnectCause;
import android.telephony.PhoneNumberUtils;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsCallProfile;
import android.text.TextUtils;
import android.util.Pair;
/// M: IMS feature. @{
import android.widget.Toast;
/// @}

import com.android.ims.ImsCall;
import com.mediatek.ims.MtkImsCall;
import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallFailCause;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.Connection.Capability;
import com.android.internal.telephony.Connection.PostDialListener;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.gsm.SuppServiceNotification;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.imsphone.ImsPhoneCallTracker;
import com.android.internal.telephony.imsphone.ImsPhoneConnection;
import com.android.internal.telephony.PhoneInternalInterface.DialArgs;
import com.android.phone.ImsUtil;
import com.android.phone.PhoneGlobals;
import com.android.phone.PhoneUtils;
import com.android.phone.R;

import com.mediatek.internal.telephony.MtkCallStateException;
import com.mediatek.internal.telephony.imsphone.MtkImsPhone;
import com.mediatek.internal.telephony.imsphone.MtkImsPhoneConnection;
import com.mediatek.internal.telephony.imsphone.MtkImsPhoneCall;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.imsphone.MtkImsPhone;

import com.mediatek.services.telephony.MtkGsmCdmaConnection;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import mediatek.telecom.MtkConnection;
import mediatek.telecom.MtkTelecomManager;
import mediatek.telephony.MtkCarrierConfigManager;
import mediatek.telecom.MtkTelecomManager;
import mediatek.telephony.MtkDisconnectCause;

import com.mediatek.services.telephony.MtkTelephonyConnectionServiceUtil;

/// M: CC: ECC retry @{
// Don't retry if treated as normal call in Telephony Framework
import android.telephony.emergency.EmergencyNumber;
/// @}

/**
 * Base class for CDMA and GSM connections.
 */
 // M: CC: Declare TelephonyConnection as public for MtkTelephonyConnectionServiceUtil to access
public abstract class TelephonyConnection extends Connection implements Holdable {

    private static final int MSG_PRECISE_CALL_STATE_CHANGED = 1;
    private static final int MSG_RINGBACK_TONE = 2;
    private static final int MSG_HANDOVER_STATE_CHANGED = 3;
    private static final int MSG_DISCONNECT = 4;
    private static final int MSG_MULTIPARTY_STATE_CHANGED = 5;
    private static final int MSG_CONFERENCE_MERGE_FAILED = 6;
    private static final int MSG_SUPP_SERVICE_NOTIFY = 7;

    /**
     * Mappings from {@link com.android.internal.telephony.Connection} extras keys to their
     * equivalents defined in {@link android.telecom.Connection}.
     */
    private static final Map<String, String> sExtrasMap = createExtrasMap();

    private static final int MSG_SET_VIDEO_STATE = 8;
    private static final int MSG_SET_VIDEO_PROVIDER = 9;
    private static final int MSG_SET_AUDIO_QUALITY = 10;
    private static final int MSG_SET_CONFERENCE_PARTICIPANTS = 11;
    private static final int MSG_CONNECTION_EXTRAS_CHANGED = 12;
    private static final int MSG_SET_ORIGNAL_CONNECTION_CAPABILITIES = 13;
    private static final int MSG_ON_HOLD_TONE = 14;
    private static final int MSG_CDMA_VOICE_PRIVACY_ON = 15;
    private static final int MSG_CDMA_VOICE_PRIVACY_OFF = 16;
    private static final int MSG_HANGUP = 17;
    private static final int MSG_SET_CALL_RADIO_TECH = 18;
    private static final int MTK_EVENT_BASE = 1000;
    /// M: CC: Modem reset related handling
    private static final int EVENT_RADIO_OFF_OR_NOT_AVAILABLE       = MTK_EVENT_BASE;

    /// M: CC: DTMF request special handling @{
    // Stop DTMF when TelephonyConnection is disconnected
    protected boolean mDtmfRequestIsStarted = false;

    /// M: VoLte SRVCC @{
    private enum SrvccPendingAction {
        SRVCC_PENDING_NONE,
        SRVCC_PENDING_ANSWER_CALL,
        SRVCC_PENDING_HOLD_CALL,
        SRVCC_PENDING_UNHOLD_CALL,
        SRVCC_PENDING_HANGUP_CALL
    }
    private SrvccPendingAction mPendingAction = SrvccPendingAction.SRVCC_PENDING_NONE;
    /// @}

    /// M: CC: ECC retry
    private boolean mIsLocallyDisconnecting = false;

    // RTT for VzW: was video call cannot upgrade to RTT
    private static final String EXTRA_WAS_VIDEO_CALL = "mediatek:wasVideoCall";

    protected final Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_PRECISE_CALL_STATE_CHANGED:
                    Log.v(TelephonyConnection.this, "MSG_PRECISE_CALL_STATE_CHANGED");
                    updateState();
                    break;
                case MSG_HANDOVER_STATE_CHANGED:
                    Log.v(TelephonyConnection.this, "MSG_HANDOVER_STATE_CHANGED");
                    AsyncResult ar = (AsyncResult) msg.obj;
                    com.android.internal.telephony.Connection connection =
                         (com.android.internal.telephony.Connection) ar.result;
                    if (mOriginalConnection != null) {
                        if (connection != null &&
                            ((connection.getAddress() != null &&
                            mOriginalConnection.getAddress() != null &&
                            mOriginalConnection.getAddress().contains(connection.getAddress())) ||
                            connection.getState() == mOriginalConnection.getStateBeforeHandover())) {
                            Log.d(TelephonyConnection.this,
                                    "SettingOriginalConnection " + mOriginalConnection.toString()
                                            + " with " + connection.toString());
                            /// M: ALPS03245602, remove the property volte after SRVCC. @{
                            removePropertyVoLte();
                            /// @}
                            /// M: Fix hangup issue. In setOriginalConnection() it will clear all
                            // messages, check whether there's hangup message not handled before.
                            if (mHandler.hasMessages(MSG_HANGUP)) {
                                Log.i(TelephonyConnection.this, "MSG_HANGUP not handled in SRVCC");
                                mPendingAction = SrvccPendingAction.SRVCC_PENDING_HANGUP_CALL;
                            }
                            /// @}
                            /// M: ALPS02528198 Show toast when ViLTE SRVCC @{
                            if (mOriginalConnection.getVideoState() !=
                                    VideoProfile.STATE_AUDIO_ONLY &&
                                    connection.getVideoState() == VideoProfile.STATE_AUDIO_ONLY) {
                                Context context = getPhone().getContext();
                                Toast.makeText(context,
                                        context.getString(R.string.vilte_srvcc_tip),
                                        Toast.LENGTH_LONG).show();
                                Log.d(this, "Video call change to vocie call during SRVCC");
                            }
                            /// @}
                            setOriginalConnection(connection);
                            mWasImsConnection = false;
                            /// M: Try SRVCC pending action. @{
                            trySrvccPendingAction();
                            /// @}
                        }
                    } else {
                        Log.w(TelephonyConnection.this,
                                "MSG_HANDOVER_STATE_CHANGED: mOriginalConnection==null - invalid state (not cleaned up)");
                    }
                    break;
                case MSG_RINGBACK_TONE:
                    Log.v(TelephonyConnection.this, "MSG_RINGBACK_TONE");
                    // TODO: This code assumes that there is only one connection in the foreground
                    // call, in other words, it punts on network-mediated conference calling.
                    if (getOriginalConnection() != getForegroundConnection()) {
                        Log.v(TelephonyConnection.this, "handleMessage, original connection is " +
                                "not foreground connection, skipping");
                        return;
                    }
                    setRingbackRequested((Boolean) ((AsyncResult) msg.obj).result);
                    break;
                case MSG_DISCONNECT: {
                    /// M: CC: Disconnect cause special handling [ALPS03251817] @{
                    int cause = mOriginalConnection == null
                            ? android.telephony.DisconnectCause.NOT_DISCONNECTED
                            : mOriginalConnection.getDisconnectCause();
                    Log.i(this, "Receives MSG_DISCONNECT, cause=" + cause);
                    if (mOriginalConnection != null && cause ==
                            MtkDisconnectCause.IMS_EMERGENCY_REREG) {
                        try {
                            com.android.internal.telephony.Connection conn =
                                    getPhone().dial(mOriginalConnection.getOrigDialString(),
                                            new DialArgs.Builder().build());
                            setOriginalConnection(conn);
                            notifyEcc();
                        } catch (CallStateException e) {
                            Log.e(this, e, "Fail to redial as ECC");
                        }
                    /// M: CC: ECC retry @{
                    } else if (!mIsLocallyDisconnecting
                            && isCauseMatchEccRetry(cause)) {
                        Log.d(this, "ECC retry: check whether need to retry, "
                                + "connectionState=" + mConnectionState);
                        // Don't retry if treated as normal call in Telephony Framework
                        boolean isDialedByEmergencyCommand = false;
                        Phone phone = getPhone();
                        if (phone != null) {
                            boolean isIms = (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS)
                                    && (phone.getDefaultPhone() != null);
                            if (isIms) {
                                // default phone is GSMPhone or CDMAPhone, which owns the ImsPhone.
                                phone = phone.getDefaultPhone();
                            }
                            if (phone.getEmergencyNumberTracker() != null) {
                                if (phone.getEmergencyNumberTracker()
                                        .getEmergencyCallRouting(mOriginalConnection.getAddress())
                                        != EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL) {
                                    isDialedByEmergencyCommand = true;
                                }
                            }
                        }

                        // Assume only one ECC exists
                        if (mTreatAsEmergencyCall
                                && MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()
                                && (mConnectionState.isDialing()
                                        || mConnectionState == Call.State.IDLE) /*[ALPS03161496]*/
                                && !MtkTelephonyConnectionServiceUtil.getInstance()
                                        .eccRetryTimeout()
                                && isDialedByEmergencyCommand) {
                            Log.i(this, "ECC retry: meet retry condition");
                            close(); // To removeConnection
                            MtkTelephonyConnectionServiceUtil.getInstance().performEccRetry();
                            break;
                        }
                    }
                    /// @}
                    updateState();
                    break;
                }
                case MSG_MULTIPARTY_STATE_CHANGED:
                    boolean isMultiParty = (Boolean) msg.obj;
                    Log.i(this, "Update multiparty state to %s", isMultiParty ? "Y" : "N");
                    mIsMultiParty = isMultiParty;
                    if (isMultiParty) {
                        notifyConferenceStarted();
                    }
                    break;
                case MSG_CONFERENCE_MERGE_FAILED:
                    notifyConferenceMergeFailed();
                    break;
                case MSG_SUPP_SERVICE_NOTIFY:
                    Phone phone = getPhone();
                    Log.v(TelephonyConnection.this, "MSG_SUPP_SERVICE_NOTIFY on phoneId : "
                            + (phone != null ? Integer.toString(phone.getPhoneId())
                            : "null"));
                    SuppServiceNotification mSsNotification = null;
                    if (msg.obj != null && ((AsyncResult) msg.obj).result != null) {
                        mSsNotification =
                                (SuppServiceNotification)((AsyncResult) msg.obj).result;
                        if (mOriginalConnection != null) {
                            handleSuppServiceNotification(mSsNotification);
                        }
                    }
                    break;

                case MSG_SET_VIDEO_STATE:
                    int videoState = (int) msg.obj;
                    /// M: show toast when video change to voice for VILTE  @{
                    String optr = SystemProperties.get("persist.vendor.operator.optr");
                    Log.i(this, "operator: " + optr
                            + " mWasImsConnection: " + mWasImsConnection);
                    if (mWasImsConnection && optr != null && optr.equals("OP01")) {
                        if (videoState == VideoProfile.STATE_AUDIO_ONLY &&
                                getVideoState() != VideoProfile.STATE_AUDIO_ONLY) {
                            Context context = getPhone().getContext();
                            // Turn on while res_ext ready
                            //Toast.makeText(context,
                            //        context.getString(R.string.vilte_to_voice_call),
                            //        Toast.LENGTH_LONG).show();
                            Log.d(this,
                                    "Video call change to vocie call");
                        }
                    }
                    /// @}
                    setVideoState(videoState);

                    // A change to the video state of the call can influence whether or not it
                    // can be part of a conference, whether another call can be added, and
                    // whether the call should have the HD audio property set.
                    refreshConferenceSupported();
                    refreshDisableAddCall();
                    updateConnectionProperties();
                    break;

                case MSG_SET_VIDEO_PROVIDER:
                    VideoProvider videoProvider = (VideoProvider) msg.obj;
                    setVideoProvider(videoProvider);
                    break;

                case MSG_SET_AUDIO_QUALITY:
                    int audioQuality = (int) msg.obj;
                    setAudioQuality(audioQuality);
                    break;

                case MSG_SET_CONFERENCE_PARTICIPANTS:
                    List<ConferenceParticipant> participants = (List<ConferenceParticipant>) msg.obj;
                    updateConferenceParticipants(participants);
                    break;

                case MSG_CONNECTION_EXTRAS_CHANGED:
                    final Bundle extras = (Bundle) msg.obj;
                    updateExtras(extras);
                    break;

                case MSG_SET_ORIGNAL_CONNECTION_CAPABILITIES:
                    setOriginalConnectionCapabilities(msg.arg1);
                    break;

                case MSG_ON_HOLD_TONE:
                    AsyncResult asyncResult = (AsyncResult) msg.obj;
                    Pair<com.android.internal.telephony.Connection, Boolean> heldInfo =
                            (Pair<com.android.internal.telephony.Connection, Boolean>)
                                    asyncResult.result;

                    // Determines if the hold tone is starting or stopping.
                    boolean playTone = ((Boolean) (heldInfo.second)).booleanValue();

                    // Determine which connection the hold tone is stopping or starting for
                    com.android.internal.telephony.Connection heldConnection = heldInfo.first;

                    // Only start or stop the hold tone if this is the connection which is starting
                    // or stopping the hold tone.
                    if (heldConnection == mOriginalConnection) {
                        // If starting the hold tone, send a connection event to Telecom which will
                        // cause it to play the on hold tone.
                        if (playTone) {
                            sendConnectionEvent(EVENT_ON_HOLD_TONE_START, null);
                        } else {
                            sendConnectionEvent(EVENT_ON_HOLD_TONE_END, null);
                        }
                    }
                    break;

                case MSG_CDMA_VOICE_PRIVACY_ON:
                    Log.d(this, "MSG_CDMA_VOICE_PRIVACY_ON received");
                    setCdmaVoicePrivacy(true);
                    break;
                case MSG_CDMA_VOICE_PRIVACY_OFF:
                    Log.d(this, "MSG_CDMA_VOICE_PRIVACY_OFF received");
                    setCdmaVoicePrivacy(false);
                    break;
                case MSG_HANGUP:
                    int cause = (int) msg.obj;
                    hangup(cause);
                    break;

                case MSG_SET_CALL_RADIO_TECH:
                    int vrat = (int) msg.obj;
                    // Check whether Wi-Fi call tech is changed, it means call radio tech is:
                    //  a) changed from IWLAN to other value, or
                    //  b) changed from other value to IWLAN.
                    //
                    // In other word, below conditions are all met:
                    // 1) {@link #getCallRadioTech} is different from new vrat
                    // 2) Current call radio technology indicates Wi-Fi call, i.e. {@link #isWifi}
                    //    is true, or new vrat indicates Wi-Fi call.
                    boolean isWifiTechChange = getCallRadioTech() != vrat
                            && (isWifi() || vrat == ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN);

                    // Step 1) Updates call radio tech firstly, so that afterwards Wi-Fi related
                    // update actions are taken correctly.
                    setCallRadioTech(vrat);

                    // Step 2) Handles Wi-Fi call tech change.
                    if (isWifiTechChange) {
                        updateConnectionProperties();
                        updateStatusHints();
                        refreshDisableAddCall();
                    }
                    break;
                /// M: CC: Modem reset related handling @{
                case EVENT_RADIO_OFF_OR_NOT_AVAILABLE:
                    notifyConnectionLost();
                    break;
                /// @}
            }
        }
    };

    /**
     * Handles {@link SuppServiceNotification}s pertinent to Telephony.
     * @param ssn the notification.
     */
    private void handleSuppServiceNotification(SuppServiceNotification ssn) {
        Log.i(this, "handleSuppServiceNotification: type=%d, code=%d", ssn.notificationType,
                ssn.code);
        if (ssn.notificationType == SuppServiceNotification.NOTIFICATION_TYPE_CODE_1
                && ssn.code == SuppServiceNotification.CODE_1_CALL_FORWARDED) {
            sendConnectionEvent(TelephonyManager.EVENT_CALL_FORWARDED, null);
        }
        sendSuppServiceNotificationEvent(ssn.notificationType, ssn.code);
    }

    /**
     * Sends a supplementary service notification connection event.
     * This connection event includes the type and code, as well as a human readable message which
     * is suitable for display to the user if the UI chooses to do so.
     * @param type the {@link SuppServiceNotification#type}.
     * @param code the {@link SuppServiceNotification#code}.
     */
    private void sendSuppServiceNotificationEvent(int type, int code) {
        Bundle extras = new Bundle();
        extras.putInt(TelephonyManager.EXTRA_NOTIFICATION_TYPE, type);
        extras.putInt(TelephonyManager.EXTRA_NOTIFICATION_CODE, code);
        extras.putCharSequence(TelephonyManager.EXTRA_NOTIFICATION_MESSAGE,
                getSuppServiceMessage(type, code));
        sendConnectionEvent(TelephonyManager.EVENT_SUPPLEMENTARY_SERVICE_NOTIFICATION, extras);
    }

    /**
     * Retrieves a human-readable message for a supplementary service notification.
     * This message is suitable for display to the user.
     * @param type the code group.
     * @param code the code.
     * @return A {@link CharSequence} containing the message, or {@code null} if none defined.
     */
    private CharSequence getSuppServiceMessage(int type, int code) {
        int messageId = -1;
        if (type == SuppServiceNotification.NOTIFICATION_TYPE_CODE_1) {
            switch (code) {
                case SuppServiceNotification.CODE_1_CALL_DEFLECTED:
                    messageId = R.string.supp_service_notification_call_deflected;
                    break;
                case SuppServiceNotification.CODE_1_CALL_FORWARDED:
                    messageId = R.string.supp_service_notification_call_forwarded;
                    break;
                case SuppServiceNotification.CODE_1_CALL_IS_WAITING:
                    messageId = R.string.supp_service_notification_call_waiting;
                    break;
                case SuppServiceNotification.CODE_1_CLIR_SUPPRESSION_REJECTED:
                    messageId = R.string.supp_service_clir_suppression_rejected;
                    break;
                case SuppServiceNotification.CODE_1_CUG_CALL:
                    messageId = R.string.supp_service_closed_user_group_call;
                    break;
                case SuppServiceNotification.CODE_1_INCOMING_CALLS_BARRED:
                    messageId = R.string.supp_service_incoming_calls_barred;
                    break;
                case SuppServiceNotification.CODE_1_OUTGOING_CALLS_BARRED:
                    messageId = R.string.supp_service_outgoing_calls_barred;
                    break;
                case SuppServiceNotification.CODE_1_SOME_CF_ACTIVE:
                    // Intentional fall through.
                case SuppServiceNotification.CODE_1_UNCONDITIONAL_CF_ACTIVE:
                    messageId = R.string.supp_service_call_forwarding_active;
                    break;
            }
        } else if (type == SuppServiceNotification.NOTIFICATION_TYPE_CODE_2) {
            switch (code) {
                case SuppServiceNotification.CODE_2_ADDITIONAL_CALL_FORWARDED:
                    messageId = R.string.supp_service_additional_call_forwarded;
                    break;
                case SuppServiceNotification.CODE_2_CALL_CONNECTED_ECT:
                    messageId = R.string.supp_service_additional_ect_connected;
                    break;
                case SuppServiceNotification.CODE_2_CALL_CONNECTING_ECT:
                    messageId = R.string.supp_service_additional_ect_connecting;
                    break;
                case SuppServiceNotification.CODE_2_CALL_ON_HOLD:
                    messageId = R.string.supp_service_call_on_hold;
                    break;
                case SuppServiceNotification.CODE_2_CALL_RETRIEVED:
                    messageId = R.string.supp_service_call_resumed;
                    break;
                case SuppServiceNotification.CODE_2_CUG_CALL:
                    messageId = R.string.supp_service_closed_user_group_call;
                    break;
                case SuppServiceNotification.CODE_2_DEFLECTED_CALL:
                    messageId = R.string.supp_service_deflected_call;
                    break;
                case SuppServiceNotification.CODE_2_FORWARDED_CALL:
                    messageId = R.string.supp_service_forwarded_call;
                    break;
                case SuppServiceNotification.CODE_2_MULTI_PARTY_CALL:
                    messageId = R.string.supp_service_conference_call;
                    break;
                case SuppServiceNotification.CODE_2_ON_HOLD_CALL_RELEASED:
                    messageId = R.string.supp_service_held_call_released;
                    break;
            }
        }
        if (messageId != -1 && getPhone() != null && getPhone().getContext() != null) {
            return getPhone().getContext().getText(messageId);
        } else {
            return null;
        }
    }

    /**
     * @return {@code true} if carrier video conferencing is supported, {@code false} otherwise.
     */
    public boolean isCarrierVideoConferencingSupported() {
        return mIsCarrierVideoConferencingSupported;
    }

    /**
     * A listener/callback mechanism that is specific communication from TelephonyConnections
     * to TelephonyConnectionService (for now). It is more specific that Connection.Listener
     * because it is only exposed in Telephony.
     */
    public abstract static class TelephonyConnectionListener {
        public void onOriginalConnectionConfigured(TelephonyConnection c) {}
        public void onOriginalConnectionRetry(TelephonyConnection c, boolean isPermanentFailure) {}
        /// M: VoLTE. @{
        /**
         * For VoLTE enhanced conference call, notify invite conf. participants completed.
         * @param isSuccess is success or not.
         */
        public void onConferenceParticipantsInvited(boolean isSuccess) {}
        /**
         * For VoLTE conference SRVCC, notify when new participant connections maded.
         * @param radioConnections new participant connections.
         */
        public void onConferenceConnectionsConfigured(
            ArrayList<com.android.internal.telephony.Connection> radioConnections) {}
        /// @}
    }

    private final PostDialListener mPostDialListener = new PostDialListener() {
        @Override
        public void onPostDialWait() {
            Log.v(TelephonyConnection.this, "onPostDialWait");
            if (mOriginalConnection != null) {
                setPostDialWait(mOriginalConnection.getRemainingPostDialString());
            }
        }

        @Override
        public void onPostDialChar(char c) {
            Log.v(TelephonyConnection.this, "onPostDialChar: %s", c);
            if (mOriginalConnection != null) {
                setNextPostDialChar(c);
            }
        }
    };

    /**
     * Listener for listening to events in the {@link com.android.internal.telephony.Connection}.
     */
    // private final com.android.internal.telephony.Connection.Listener mOriginalConnectionListener =
    //         new com.android.internal.telephony.Connection.ListenerBase() {
    /// M: new MtkListenerBase
    protected com.android.internal.telephony.Connection.Listener mOriginalConnectionListener =
            new MtkImsPhoneConnection.MtkListenerBase() {
        @Override
        public void onVideoStateChanged(int videoState) {
            mHandler.obtainMessage(MSG_SET_VIDEO_STATE, videoState).sendToTarget();
        }

        /*
         * The {@link com.android.internal.telephony.Connection} has reported a change in
         * connection capability.
         * @param capabilities bit mask containing voice or video or both capabilities.
         */
        @Override
        public void onConnectionCapabilitiesChanged(int capabilities) {
            mHandler.obtainMessage(MSG_SET_ORIGNAL_CONNECTION_CAPABILITIES,
                    capabilities, 0).sendToTarget();
        }

        /**
         * The {@link com.android.internal.telephony.Connection} has reported a change in the
         * video call provider.
         *
         * @param videoProvider The video call provider.
         */
        @Override
        public void onVideoProviderChanged(VideoProvider videoProvider) {
            mHandler.obtainMessage(MSG_SET_VIDEO_PROVIDER, videoProvider).sendToTarget();
        }

        /**
         * Used by {@link com.android.internal.telephony.Connection} to report a change for
         * the call radio technology.
         *
         * @param vrat the RIL Voice Radio Technology used for current connection.
         */
        @Override
        public void onCallRadioTechChanged(@ServiceState.RilRadioTechnology int vrat) {
            mHandler.obtainMessage(MSG_SET_CALL_RADIO_TECH, vrat).sendToTarget();
        }

        /**
         * Used by the {@link com.android.internal.telephony.Connection} to report a change in the
         * audio quality for the current call.
         *
         * @param audioQuality The audio quality.
         */
        @Override
        public void onAudioQualityChanged(int audioQuality) {
            mHandler.obtainMessage(MSG_SET_AUDIO_QUALITY, audioQuality).sendToTarget();
        }
        /**
         * Handles a change in the state of conference participant(s), as reported by the
         * {@link com.android.internal.telephony.Connection}.
         *
         * @param participants The participant(s) which changed.
         */
        @Override
        public void onConferenceParticipantsChanged(List<ConferenceParticipant> participants) {
            mHandler.obtainMessage(MSG_SET_CONFERENCE_PARTICIPANTS, participants).sendToTarget();
        }

        /*
         * Handles a change to the multiparty state for this connection.
         *
         * @param isMultiParty {@code true} if the call became multiparty, {@code false}
         *      otherwise.
         */
        @Override
        public void onMultipartyStateChanged(boolean isMultiParty) {
            handleMultipartyStateChange(isMultiParty);
        }

        /**
         * Handles the event that the request to merge calls failed.
         */
        @Override
        public void onConferenceMergedFailed() {
            handleConferenceMergeFailed();
        }

        @Override
        public void onExtrasChanged(Bundle extras) {
            mHandler.obtainMessage(MSG_CONNECTION_EXTRAS_CHANGED, extras).sendToTarget();
        }

        /**
         * Handles the phone exiting ECM mode by updating the connection capabilities.  During an
         * ongoing call, if ECM mode is exited, we will re-enable mute for CDMA calls.
         */
        @Override
        public void onExitedEcmMode() {
            handleExitedEcmMode();
        }

        /**
         * Called from {@link ImsPhoneCallTracker} when a request to pull an external call has
         * failed.
         * @param externalConnection
         */
        @Override
        public void onCallPullFailed(com.android.internal.telephony.Connection externalConnection) {
            if (externalConnection == null) {
                return;
            }

            Log.i(this, "onCallPullFailed - pull failed; swapping back to call: %s",
                    externalConnection);

            // Inform the InCallService of the fact that the call pull failed (it may choose to
            // display a message informing the user of the pull failure).
            sendConnectionEvent(Connection.EVENT_CALL_PULL_FAILED, null);

            // Swap the ImsPhoneConnection we used to do the pull for the ImsExternalConnection
            // which originally represented the call.
            setOriginalConnection(externalConnection);

            // Set our state to active again since we're no longer pulling.
            setActiveInternal();
        }

        /**
         * Called from {@link ImsPhoneCallTracker} when a handover to WIFI has failed.
         */
        @Override
        public void onHandoverToWifiFailed() {
            sendConnectionEvent(TelephonyManager.EVENT_HANDOVER_TO_WIFI_FAILED, null);
        }

        /**
         * Informs the {@link android.telecom.ConnectionService} of a connection event raised by the
         * original connection.
         * @param event The connection event.
         * @param extras The extras.
         */
        @Override
        public void onConnectionEvent(String event, Bundle extras) {
            sendConnectionEvent(event, extras);
        }

        @Override
        public void onRttModifyRequestReceived() {
            sendRemoteRttRequest();
        }

        @Override
        public void onRttModifyResponseReceived(int status) {
            updateConnectionProperties();
            refreshConferenceSupported();
            if (status == RttModifyStatus.SESSION_MODIFY_REQUEST_SUCCESS) {
                sendRttInitiationSuccess();
            } else {
                sendRttInitiationFailure(status);
            }
        }

        @Override
        public void onDisconnect(int cause) {
            Log.i(this, "onDisconnect: callId=%s, cause=%s", getTelecomCallId(),
                    DisconnectCause.toString(cause));
            mHandler.obtainMessage(MSG_DISCONNECT).sendToTarget();
        }

        @Override
        public void onRttInitiated() {
            if (mOriginalConnection != null) {
                // if mOriginalConnection is null, the properties will get set when
                // mOriginalConnection gets set.
                updateConnectionProperties();
                refreshConferenceSupported();
            }
            sendRttInitiationSuccess();
        }

        @Override
        public void onRttTerminated() {
            updateConnectionProperties();
            sendRttSessionRemotelyTerminated();
        }

        @Override
        public void onOriginalConnectionReplaced(
                com.android.internal.telephony.Connection newConnection) {
            setOriginalConnection(newConnection);
        }

        @Override
        public void onIsNetworkEmergencyCallChanged(boolean isEmergencyCall) {
            setIsNetworkIdentifiedEmergencyCall(isEmergencyCall);
        }

        public void onRedialEcc(boolean isNeedUserConfirm) {
            Log.i(this, "onRedialEcc: callId=" + getTelecomCallId()
                    + " isNeedUserConfirm=" + isNeedUserConfirm);
            if (isNeedUserConfirm) {
            // Show a dialog to let user confirm
            } else {
                notifyEcc();
            }
        }

        /// M: For VoLTE conference call. @{
        /**
         * For VoLTE enhanced conference call, notify invite conf. participants completed.
         * @param isSuccess is success or not.
         */
        @Override
        public void onConferenceParticipantsInvited(boolean isSuccess) {
            notifyConferenceParticipantsInvited(isSuccess);
        }
        /**
         * For VoLTE conference SRVCC, notify when new participant connections maded.
         * @param radioConnections new participant connections.
        */
        @Override
        public void onConferenceConnectionsConfigured(
                ArrayList<com.android.internal.telephony.Connection> radioConnections) {
            notifyConferenceConnectionsConfigured(radioConnections);
        }
        /// @}

        /// M: ALPS04343550: AOSP update displayed address only when precise state change,
        /// but we need to update it once the value changed.
        @Override
        public void onAddressDisplayChanged() {
            updateAddress();
        }
    };

    protected com.android.internal.telephony.Connection mOriginalConnection;
    private Call.State mConnectionState = Call.State.IDLE;
    private Bundle mOriginalConnectionExtras = new Bundle();
    private boolean mIsStateOverridden = false;
    private Call.State mOriginalConnectionState = Call.State.IDLE;
    private Call.State mConnectionOverriddenState = Call.State.IDLE;
    private RttTextStream mRttTextStream = null;

    private boolean mWasImsConnection;

    /**
     * Tracks the multiparty state of the ImsCall so that changes in the bit state can be detected.
     */
    private boolean mIsMultiParty = false;

    /**
     * The {@link com.android.internal.telephony.Connection} capabilities associated with the
     * current {@link #mOriginalConnection}.
     */
    private int mOriginalConnectionCapabilities;

    /**
     * Determines the audio quality is high for the {@link TelephonyConnection}.
     * This is used when {@link TelephonyConnection#updateConnectionProperties}} is called to
     * indicate whether a call has the {@link Connection#PROPERTY_HIGH_DEF_AUDIO} property.
     */
    private boolean mHasHighDefAudio;

    /**
     * Indicates that the connection should be treated as an emergency call because the
     * number dialed matches an internal list of emergency numbers. Does not guarantee whether
     * the network will treat the call as an emergency call.
     */
    private boolean mTreatAsEmergencyCall;

    /**
     * Indicates whether the network has identified this call as an emergency call.  Where
     * {@link #mTreatAsEmergencyCall} is based on comparing dialed numbers to a list of known
     * emergency numbers, this property is based on whether the network itself has identified the
     * call as an emergency call (which can be the case for an incoming call from emergency
     * services).
     */
    private boolean mIsNetworkIdentifiedEmergencyCall;

    /**
     * For video calls, indicates whether the outgoing video for the call can be paused using
     * the {@link android.telecom.VideoProfile#STATE_PAUSED} VideoState.
     */
    private boolean mIsVideoPauseSupported;

    /**
     * Indicates whether this connection supports being a part of a conference..
     */
    private boolean mIsConferenceSupported;

    /**
     * Indicates whether managing conference call is supported after this connection being
     * a part of a IMS conference.
     */
    private boolean mIsManageImsConferenceCallSupported;

    /**
     * Indicates whether the carrier supports video conferencing; captures the current state of the
     * carrier config
     * {@link android.telephony.CarrierConfigManager#KEY_SUPPORT_VIDEO_CONFERENCE_CALL_BOOL}.
     */
    private boolean mIsCarrierVideoConferencingSupported;

    /**
     * Indicates whether or not this connection has CDMA Enhanced Voice Privacy enabled.
     */
    private boolean mIsCdmaVoicePrivacyEnabled;

    /**
     * Indicates whether this call is an outgoing call.
     */
    protected final boolean mIsOutgoing;

    /**
     * Indicates whether the connection can be held. This filed combined with the state of the
     * connection can determine whether {@link Connection#CAPABILITY_HOLD} should be added to the
     * connection.
     */
    private boolean mIsHoldable;

    /**
     * Indicates whether TTY is enabled; used to determine whether a call is VT capable.
     */
    private boolean mIsTtyEnabled;

    /**
     * Indicates whether this call is using assisted dialing.
     */
    private boolean mIsUsingAssistedDialing;

    /**
     * Indicates whether this connection supports showing preciese call failed cause.
     */
    private boolean mShowPreciseFailedCause;

    /**
     * Listeners to our TelephonyConnection specific callbacks
     */
    private final Set<TelephonyConnectionListener> mTelephonyListeners = Collections.newSetFromMap(
            new ConcurrentHashMap<TelephonyConnectionListener, Boolean>(8, 0.9f, 1));

    protected TelephonyConnection(com.android.internal.telephony.Connection originalConnection,
            String callId, boolean isOutgoingCall) {
        mIsOutgoing = isOutgoingCall;
        setTelecomCallId(callId);
        if (originalConnection != null) {
            setOriginalConnection(originalConnection);
        }
    }

    /**
     * Creates a clone of the current {@link TelephonyConnection}.
     *
     * @return The clone.
     */
    public abstract TelephonyConnection cloneConnection();

    @Override
    public void onCallAudioStateChanged(CallAudioState audioState) {
        // TODO: update TTY mode.
        if (getPhone() != null) {
            getPhone().setEchoSuppressionEnabled();
        }
    }

    @Override
    public void onStateChanged(int state) {
        Log.v(this, "onStateChanged, state: " + Connection.stateToString(state));
        updateStatusHints();
    }

    @Override
    public void onDisconnect() {
        Log.v(this, "onDisconnect");
        mHandler.obtainMessage(MSG_HANGUP, android.telephony.DisconnectCause.LOCAL).sendToTarget();
    }

    /**
     * Notifies this Connection of a request to disconnect a participant of the conference managed
     * by the connection.
     *
     * @param endpoint the {@link Uri} of the participant to disconnect.
     */
    @Override
    public void onDisconnectConferenceParticipant(Uri endpoint) {
        Log.v(this, "onDisconnectConferenceParticipant %s", endpoint);

        if (mOriginalConnection == null) {
            return;
        }

        mOriginalConnection.onDisconnectConferenceParticipant(endpoint);
    }

    @Override
    public void onSeparate() {
        Log.v(this, "onSeparate");
        if (mOriginalConnection != null) {
            try {
                mOriginalConnection.separate();
            } catch (CallStateException e) {
                Log.e(this, e, "Call to Connection.separate failed with exception");
            }
        }
    }

    @Override
    public void onAbort() {
        Log.v(this, "onAbort");
        mHandler.obtainMessage(MSG_HANGUP, android.telephony.DisconnectCause.LOCAL).sendToTarget();
    }

    @Override
    public void onHold() {
        performHold();
    }

    @Override
    public void onUnhold() {
        performUnhold();
    }

    @Override
    public void onAnswer(int videoState) {
        Log.v(this, "onAnswer");
        if (isValidRingingCall() && getPhone() != null) {
            try {
                getPhone().acceptCall(videoState);
            } catch (CallStateException e) {
                Log.e(this, e, "Failed to accept call.");
                /// M: Keep the pending action for SRVCC. @{
                if (e.getError() == MtkCallStateException.ERROR_INVALID_DURING_SRVCC) {
                    mPendingAction = SrvccPendingAction.SRVCC_PENDING_ANSWER_CALL;
                }
                /// @}
            }
        }
    }

    @Override
    public void onDeflect(Uri address) {
        Log.v(this, "onDeflect");
        if (mOriginalConnection != null && isValidRingingCall()) {
            if (address == null) {
                Log.w(this, "call deflect address uri is null");
                return;
            }
            String scheme = address.getScheme();
            String deflectNumber = "";
            String uriString = address.getSchemeSpecificPart();
            if (!PhoneAccount.SCHEME_VOICEMAIL.equals(scheme)) {
                if (!PhoneAccount.SCHEME_TEL.equals(scheme)) {
                    Log.w(this, "onDeflect, address scheme is not of type tel instead: " +
                            scheme);
                    return;
                }
                if (PhoneNumberUtils.isUriNumber(uriString)) {
                    Log.w(this, "Invalid deflect address. Not a legal PSTN number.");
                    return;
                }
                deflectNumber = PhoneNumberUtils.convertAndStrip(uriString);
                if (TextUtils.isEmpty(deflectNumber)) {
                    Log.w(this, "Empty deflect number obtained from address uri");
                    return;
                }
            } else {
                Log.w(this, "Cannot deflect to voicemail uri");
                return;
            }

            try {
                mOriginalConnection.deflect(deflectNumber);
            } catch (CallStateException e) {
                Log.e(this, e, "Failed to deflect call.");
            }
        }
    }

    @Override
    public void onReject() {
        Log.v(this, "onReject");
        if (isValidRingingCall()) {
            mHandler.obtainMessage(MSG_HANGUP, android.telephony.DisconnectCause.INCOMING_REJECTED)
                    .sendToTarget();
        }
        super.onReject();
    }

    /// M: CC: Softbank blacklist requirement @{
    /**
     * Notifies this Connection, which is in {@link #STATE_RINGING}, of
     * a request to reject with specified cause.
     *
     * @param cause the DisconnectCause specified for this connection rejection.
     *              If no special handle for the specified cause, it will follow
     *              default onReject behavior.
     */
    public void onReject(int cause) {
        Log.v(this, "onReject with cause %d", cause);
        if (isValidRingingCall()) {
            if (cause > MtkDisconnectCause.MTK_DISCONNECTED_CAUSE_BASE) {
                mHandler.obtainMessage(MSG_HANGUP, cause)
                .sendToTarget();
            } else {
                mHandler.obtainMessage(MSG_HANGUP,
                        android.telephony.DisconnectCause.INCOMING_REJECTED).sendToTarget();
            }
        }
        super.onReject();
    }
    /// @}

    @Override
    public void onPostDialContinue(boolean proceed) {
        Log.v(this, "onPostDialContinue, proceed: " + proceed);
        if (mOriginalConnection != null) {
            if (proceed) {
                mOriginalConnection.proceedAfterWaitChar();
            } else {
                mOriginalConnection.cancelPostDial();
            }
        }
    }

    /**
     * Handles requests to pull an external call.
     */
    @Override
    public void onPullExternalCall() {
        if ((getConnectionProperties() & Connection.PROPERTY_IS_EXTERNAL_CALL) !=
                Connection.PROPERTY_IS_EXTERNAL_CALL) {
            Log.w(this, "onPullExternalCall - cannot pull non-external call");
            return;
        }

        if (mOriginalConnection != null) {
            mOriginalConnection.pullExternalCall();
        }
    }

    @Override
    public void onStartRtt(RttTextStream textStream) {
        if (isImsConnection()) {
            ImsPhoneConnection originalConnection = (ImsPhoneConnection) mOriginalConnection;
            if (originalConnection.isRttEnabledForCall()) {
                originalConnection.setCurrentRttTextStream(textStream);
            } else {
                originalConnection.startRtt(textStream);
            }
        } else {
            Log.w(this, "onStartRtt - not in IMS, so RTT cannot be enabled.");
        }
    }

    @Override
    public void onStopRtt() {
        if (isImsConnection()) {
            ImsPhoneConnection originalConnection = (ImsPhoneConnection) mOriginalConnection;
            if (originalConnection.isRttEnabledForCall()) {
                originalConnection.stopRtt();
            } else {
                Log.w(this, "onStopRtt - not in RTT call, ignoring");
            }
        } else {
            Log.w(this, "onStopRtt - not in IMS, ignoring");
        }
    }

    @Override
    public void handleRttUpgradeResponse(RttTextStream textStream) {
        if (!isImsConnection()) {
            Log.w(this, "handleRttUpgradeResponse - not in IMS, so RTT cannot be enabled.");
            return;
        }
        ImsPhoneConnection originalConnection = (ImsPhoneConnection) mOriginalConnection;
        originalConnection.sendRttModifyResponse(textStream);
    }

    public void performHold() {
        Log.v(this, "performHold");
        // TODO: Can dialing calls be put on hold as well since they take up the
        // foreground call slot?
        if (Call.State.ACTIVE == mConnectionState) {
            Log.v(this, "Holding active call");
            try {
                Phone phone = mOriginalConnection.getCall().getPhone();

                Call ringingCall = phone.getRingingCall();

                // Although the method says switchHoldingAndActive, it eventually calls a RIL method
                // called switchWaitingOrHoldingAndActive. What this means is that if we try to put
                // a call on hold while a call-waiting call exists, it'll end up accepting the
                // call-waiting call, which is bad if that was not the user's intention. We are
                // cheating here and simply skipping it because we know any attempt to hold a call
                // while a call-waiting call is happening is likely a request from Telecom prior to
                // accepting the call-waiting call.
                // TODO: Investigate a better solution. It would be great here if we
                // could "fake" hold by silencing the audio and microphone streams for this call
                // instead of actually putting it on hold.
                if (ringingCall.getState() != Call.State.WAITING) {
                    // New behavior for IMS -- don't use the clunky switchHoldingAndActive logic.
                    if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS) {
                        ImsPhone imsPhone = (ImsPhone) phone;
                        imsPhone.holdActiveCall();
                        return;
                    }
                    phone.switchHoldingAndActive();
                }

                // TODO: Cdma calls are slightly different.
            } catch (CallStateException e) {
                Log.e(this, e, "Exception occurred while trying to put call on hold.");
                /// M: Keep the pending action for SRVCC. @{
                if (e.getError() == MtkCallStateException.ERROR_INVALID_DURING_SRVCC) {
                    mPendingAction = SrvccPendingAction.SRVCC_PENDING_HOLD_CALL;
                }
                /// @}
            }
        } else {
            Log.w(this, "Cannot put a call that is not currently active on hold.");
        }
    }

    public void performUnhold() {
        Log.v(this, "performUnhold");
        if (Call.State.HOLDING == mConnectionState) {
            try {
                // Here's the deal--Telephony hold/unhold is weird because whenever there exists
                // more than one call, one of them must always be active. In other words, if you
                // have an active call and holding call, and you put the active call on hold, it
                // will automatically activate the holding call. This is weird with how Telecom
                // sends its commands. When a user opts to "unhold" a background call, telecom
                // issues hold commands to all active calls, and then the unhold command to the
                // background call. This means that we get two commands...each of which reduces to
                // switchHoldingAndActive(). The result is that they simply cancel each other out.
                // To fix this so that it works well with telecom we add a minor hack. If we
                // have one telephony call, everything works as normally expected. But if we have
                // two or more calls, we will ignore all requests to "unhold" knowing that the hold
                // requests already do what we want. If you've read up to this point, I'm very sorry
                // that we are doing this. I didn't think of a better solution that wouldn't also
                // make the Telecom APIs very ugly.

                if (!hasMultipleTopLevelCalls()) {
                    /// M: ALPS04753500 ImsPhoneCallTracker will auto resume call when swap call @{
                    Phone phone = mOriginalConnection.getCall().getPhone();
                    // New behavior for IMS -- don't use the clunky switchHoldingAndActive logic.
                    if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS) {
                        ImsPhone imsPhone = (ImsPhone) phone;
                        imsPhone.unholdHeldCall();
                        return;
                    }
                    /// @}
                    mOriginalConnection.getCall().getPhone().switchHoldingAndActive();
                } else {
                    Log.i(this, "Skipping unhold command for %s", this);
                }
            } catch (CallStateException e) {
                Log.e(this, e, "Exception occurred while trying to release call from hold.");
                /// M: Keep the pending action for SRVCC. @{
                if (e.getError() == MtkCallStateException.ERROR_INVALID_DURING_SRVCC) {
                    mPendingAction = SrvccPendingAction.SRVCC_PENDING_UNHOLD_CALL;
                }
                /// @}
            }
        } else {
            Log.w(this, "Cannot release a call that is not already on hold from hold.");
        }
    }

    public void performConference(Connection otherConnection) {
        Log.d(this, "performConference - %s", this);
        if (getPhone() != null) {
            try {
                // We dont use the "other" connection because there is no concept of that in the
                // implementation of calls inside telephony. Basically, you can "conference" and it
                // will conference with the background call.  We know that otherConnection is the
                // background call because it would never have called setConferenceableConnections()
                // otherwise.
                getPhone().conference();
            } catch (CallStateException e) {
                Log.e(this, e, "Failed to conference call.");
            }
        }
    }

    /**
     * Builds connection capabilities common to all TelephonyConnections. Namely, apply IMS-based
     * capabilities.
     */
    protected int buildConnectionCapabilities() {
        int callCapabilities = 0;
        if (mOriginalConnection != null && mOriginalConnection.isIncoming()) {
            callCapabilities |= CAPABILITY_SPEED_UP_MT_AUDIO;
        }
        if (!shouldTreatAsEmergencyCall() && isImsConnection() && canHoldImsCalls()) {
            callCapabilities |= CAPABILITY_SUPPORT_HOLD;
            if (mIsHoldable && (getState() == STATE_ACTIVE || getState() == STATE_HOLDING)) {
                callCapabilities |= CAPABILITY_HOLD;
            }
        }

        Log.d(this, "buildConnectionCapabilities: isHoldable = "
                + mIsHoldable + " State = " + getState() + " capabilities = " + callCapabilities);

        return callCapabilities;
    }

    protected final void updateConnectionCapabilities() {
        int newCapabilities = buildConnectionCapabilities();

        newCapabilities = applyOriginalConnectionCapabilities(newCapabilities);
        newCapabilities = changeBitmask(newCapabilities, CAPABILITY_CAN_PAUSE_VIDEO,
                mIsVideoPauseSupported && isVideoCapable());
        newCapabilities = changeBitmask(newCapabilities, CAPABILITY_CAN_PULL_CALL,
                isExternalConnection() && isPullable());
        newCapabilities = applyConferenceTerminationCapabilities(newCapabilities);
        newCapabilities = changeBitmask(newCapabilities, CAPABILITY_SUPPORT_DEFLECT,
                isImsConnection() && canDeflectImsCalls());
        /// M: Video ringtone @{
        newCapabilities = applyVideoRingtoneCapabilities(
                mOriginalConnectionCapabilities, newCapabilities);
        /// @}
        if (getConnectionCapabilities() != newCapabilities) {
            setConnectionCapabilities(newCapabilities);
        }
    }

    protected int buildConnectionProperties() {
        int connectionProperties = 0;

        // If the phone is in ECM mode, mark the call to indicate that the callback number should be
        // shown.
        Phone phone = getPhone();
        if (phone != null && phone.isInEcm()) {
            connectionProperties |= PROPERTY_EMERGENCY_CALLBACK_MODE;
        }

        return connectionProperties;
    }

    /**
     * Updates the properties of the connection.
     */
    protected final void updateConnectionProperties() {
        int newProperties = buildConnectionProperties();

        newProperties = changeBitmask(newProperties, PROPERTY_HIGH_DEF_AUDIO,
                hasHighDefAudioProperty());
        newProperties = changeBitmask(newProperties, PROPERTY_WIFI, isWifi());
        newProperties = changeBitmask(newProperties, PROPERTY_IS_EXTERNAL_CALL,
                isExternalConnection());
        newProperties = changeBitmask(newProperties, PROPERTY_HAS_CDMA_VOICE_PRIVACY,
                mIsCdmaVoicePrivacyEnabled);
        newProperties = changeBitmask(newProperties, PROPERTY_ASSISTED_DIALING_USED,
                mIsUsingAssistedDialing);
        newProperties = changeBitmask(newProperties, PROPERTY_IS_RTT, isRtt());
        newProperties = changeBitmask(newProperties, PROPERTY_NETWORK_IDENTIFIED_EMERGENCY_CALL,
                isNetworkIdentifiedEmergencyCall());
        /// M: update property volte. @{
        newProperties = updatePropertyVoLte(newProperties);
        /// @}

        if (getConnectionProperties() != newProperties) {
            setConnectionProperties(newProperties);
        }
    }

    protected final void updateAddress() {
        updateConnectionCapabilities();
        updateConnectionProperties();
        if (mOriginalConnection != null) {
            Uri address;
            if (isShowingOriginalDialString()
                    && mOriginalConnection.getOrigDialString() != null) {
                address = getAddressFromNumber(mOriginalConnection.getOrigDialString());
            } else {
                address = getAddressFromNumber(mOriginalConnection.getAddress());
            }
            int presentation = mOriginalConnection.getNumberPresentation();
            if (!Objects.equals(address, getAddress()) ||
                    presentation != getAddressPresentation()) {
                Log.v(this, "updateAddress, address changed");
                if ((getConnectionProperties() & PROPERTY_IS_DOWNGRADED_CONFERENCE) != 0) {
                    address = null;
                }
                setAddress(address, presentation);
            }

            String name = filterCnapName(mOriginalConnection.getCnapName());
            int namePresentation = mOriginalConnection.getCnapNamePresentation();
            if (!Objects.equals(name, getCallerDisplayName()) ||
                    namePresentation != getCallerDisplayNamePresentation()) {
                Log.v(this, "updateAddress, caller display name changed");
                setCallerDisplayName(name, namePresentation);
            }

            if (PhoneNumberUtils.isEmergencyNumber(mOriginalConnection.getAddress())) {
                mTreatAsEmergencyCall = true;
            }

            // Changing the address of the connection can change whether it is an emergency call or
            // not, which can impact whether it can be part of a conference.
            refreshConferenceSupported();
        }
    }

    // M: CC: Declare as protected for MtkGsmCdmaConnection to access
    protected void onRemovedFromCallService() {
        // Subclass can override this to do cleanup.
    }

    // M: IMS: Declare as public for MtkTelephonyConnectionServiceUtil to access
    public
    void setOriginalConnection(com.android.internal.telephony.Connection originalConnection) {
        Log.v(this, "new TelephonyConnection, originalConnection: " + originalConnection);
        if (mOriginalConnection != null && originalConnection != null
               && !originalConnection.isIncoming()
               && originalConnection.getOrigDialString() == null
               && isShowingOriginalDialString()) {
            Log.i(this, "new original dial string is null, convert to: "
                   +  mOriginalConnection.getOrigDialString());
            originalConnection.setConverted(mOriginalConnection.getOrigDialString());
        }

        clearOriginalConnection();
        mOriginalConnectionExtras.clear();
        mOriginalConnection = originalConnection;
        mOriginalConnection.setTelecomCallId(getTelecomCallId());
        getPhone().registerForPreciseCallStateChanged(
                mHandler, MSG_PRECISE_CALL_STATE_CHANGED, null);
        getPhone().registerForHandoverStateChanged(
                mHandler, MSG_HANDOVER_STATE_CHANGED, null);
        getPhone().registerForRingbackTone(mHandler, MSG_RINGBACK_TONE, null);
        getPhone().registerForSuppServiceNotification(mHandler, MSG_SUPP_SERVICE_NOTIFY, null);
        getPhone().registerForOnHoldTone(mHandler, MSG_ON_HOLD_TONE, null);
        getPhone().registerForInCallVoicePrivacyOn(mHandler, MSG_CDMA_VOICE_PRIVACY_ON, null);
        getPhone().registerForInCallVoicePrivacyOff(mHandler, MSG_CDMA_VOICE_PRIVACY_OFF, null);
        /// M: CC: Modem reset related handling
        getPhone().registerForRadioOffOrNotAvailable(
                mHandler, EVENT_RADIO_OFF_OR_NOT_AVAILABLE, null);
        mOriginalConnection.addPostDialListener(mPostDialListener);
        mOriginalConnection.addListener(mOriginalConnectionListener);

        // M: [ALPS04103403] Avoid IMS ECC redial fail: addListener before register for notification
        /// M: CC: Proprietary CRSS handling @{
        /// M: Makes SuppMessageManager register for ImsPhone.
        registerSuppMessageManager(getPhone(), mOriginalConnection);
        /// @}

        // Set video state and capabilities
        setVideoState(mOriginalConnection.getVideoState());
        setOriginalConnectionCapabilities(mOriginalConnection.getConnectionCapabilities());
        setIsNetworkIdentifiedEmergencyCall(mOriginalConnection.isNetworkIdentifiedEmergencyCall());
        setAudioModeIsVoip(mOriginalConnection.getAudioModeIsVoip());
        setVideoProvider(mOriginalConnection.getVideoProvider());
        setAudioQuality(mOriginalConnection.getAudioQuality());
        setTechnologyTypeExtra();

        setCallRadioTech(mOriginalConnection.getCallRadioTech());

        // Post update of extras to the handler; extras are updated via the handler to ensure thread
        // safety. The Extras Bundle is cloned in case the original extras are modified while they
        // are being added to mOriginalConnectionExtras in updateExtras.
        Bundle connExtras = mOriginalConnection.getConnectionExtras();
            mHandler.obtainMessage(MSG_CONNECTION_EXTRAS_CHANGED, connExtras == null ? null :
                    new Bundle(connExtras)).sendToTarget();

        if (PhoneNumberUtils.isEmergencyNumber(mOriginalConnection.getAddress())) {
            mTreatAsEmergencyCall = true;
        }

        if (isImsConnection()) {
            mWasImsConnection = true;
        }
        mIsMultiParty = mOriginalConnection.isMultiparty();

        Bundle extrasToPut = new Bundle();
        List<String> extrasToRemove = new ArrayList<>();
        if (mOriginalConnection.isActiveCallDisconnectedOnAnswer()) {
            extrasToPut.putBoolean(Connection.EXTRA_ANSWERING_DROPS_FG_CALL, true);
        } else {
            extrasToRemove.add(Connection.EXTRA_ANSWERING_DROPS_FG_CALL);
        }

        if (shouldSetDisableAddCallExtra()) {
            extrasToPut.putBoolean(Connection.EXTRA_DISABLE_ADD_CALL, true);
        } else {
            extrasToRemove.add(Connection.EXTRA_DISABLE_ADD_CALL);
        }
        putExtras(extrasToPut);
        removeExtras(extrasToRemove);

        // updateState can set mOriginalConnection to null if its state is DISCONNECTED, so this
        // should be executed *after* the above setters have run.
        updateState();
        if (mOriginalConnection == null) {
            Log.w(this, "original Connection was nulled out as part of setOriginalConnection. " +
                    originalConnection);
        }

        fireOnOriginalConnectionConfigured();
    }

    /**
     * Filters the CNAP name to not include a list of names that are unhelpful to the user for
     * Caller ID purposes.
     */
    private String filterCnapName(final String cnapName) {
        if (cnapName == null) {
            return null;
        }
        PersistableBundle carrierConfig = getCarrierConfig();
        String[] filteredCnapNames = null;
        if (carrierConfig != null) {
            filteredCnapNames = carrierConfig.getStringArray(
                    CarrierConfigManager.KEY_FILTERED_CNAP_NAMES_STRING_ARRAY);
        }
        if (filteredCnapNames != null) {
            long cnapNameMatches = Arrays.asList(filteredCnapNames)
                    .stream()
                    .filter(filteredCnapName -> filteredCnapName.equals(cnapName.toUpperCase()))
                    .count();
            if (cnapNameMatches > 0) {
                Log.i(this, "filterCnapName: Filtered CNAP Name: " + cnapName);
                return "";
            }
        }
        return cnapName;
    }

    /**
     * Sets the EXTRA_CALL_TECHNOLOGY_TYPE extra on the connection to report back to Telecom.
     */
    private void setTechnologyTypeExtra() {
        if (getPhone() != null) {
            putExtra(TelecomManager.EXTRA_CALL_TECHNOLOGY_TYPE, getPhone().getPhoneType());
        }
    }

    private void refreshDisableAddCall() {
        if (shouldSetDisableAddCallExtra()) {
            putExtra(Connection.EXTRA_DISABLE_ADD_CALL, true);
        } else {
            removeExtras(Connection.EXTRA_DISABLE_ADD_CALL);
        }
    }

    private boolean shouldSetDisableAddCallExtra() {
        if (mOriginalConnection == null) {
            return false;
        }
        boolean carrierShouldAllowAddCall = mOriginalConnection.shouldAllowAddCallDuringVideoCall();
        if (carrierShouldAllowAddCall) {
            return false;
        }
        Phone phone = getPhone();
        if (phone == null) {
            return false;
        }
        boolean isCurrentVideoCall = false;
        boolean wasVideoCall = false;
        boolean isVowifiEnabled = false;
        if (phone instanceof ImsPhone) {
            ImsPhone imsPhone = (ImsPhone) phone;
            if (imsPhone.getForegroundCall() != null
                    && imsPhone.getForegroundCall().getImsCall() != null) {
                ImsCall call = imsPhone.getForegroundCall().getImsCall();
                isCurrentVideoCall = call.isVideoCall();
                wasVideoCall = call.wasVideoCall();
            }

            isVowifiEnabled = ImsUtil.isWfcEnabled(phone.getContext(), phone.getPhoneId());
        }

        if (isCurrentVideoCall) {
            return true;
        } else if (wasVideoCall && isWifi() && !isVowifiEnabled) {
            return true;
        }
        return false;
    }

    private boolean hasHighDefAudioProperty() {
        if (!mHasHighDefAudio) {
            return false;
        }

        boolean isVideoCall = VideoProfile.isVideo(getVideoState());

        PersistableBundle b = getCarrierConfig();
        boolean canWifiCallsBeHdAudio =
                b != null && b.getBoolean(CarrierConfigManager.KEY_WIFI_CALLS_CAN_BE_HD_AUDIO);
        boolean canVideoCallsBeHdAudio =
                b != null && b.getBoolean(CarrierConfigManager.KEY_VIDEO_CALLS_CAN_BE_HD_AUDIO);
        boolean canGsmCdmaCallsBeHdAudio =
                b != null && b.getBoolean(CarrierConfigManager.KEY_GSM_CDMA_CALLS_CAN_BE_HD_AUDIO);
        boolean shouldDisplayHdAudio =
                b != null && b.getBoolean(CarrierConfigManager.KEY_DISPLAY_HD_AUDIO_PROPERTY_BOOL);

        if (!shouldDisplayHdAudio) {
            return false;
        }

        if (isGsmCdmaConnection() && !canGsmCdmaCallsBeHdAudio) {
            return false;
        }

        if (isVideoCall && !canVideoCallsBeHdAudio) {
            return false;
        }

        if (isWifi() && !canWifiCallsBeHdAudio) {
            return false;
        }

        return true;
    }

    private boolean canHoldImsCalls() {
        PersistableBundle b = getCarrierConfig();
        // Return true if the CarrierConfig is unavailable
        return !doesDeviceRespectHoldCarrierConfig() || b == null ||
                b.getBoolean(CarrierConfigManager.KEY_ALLOW_HOLD_IN_IMS_CALL_BOOL);
    }

    private PersistableBundle getCarrierConfig() {
        Phone phone = getPhone();
        if (phone == null) {
            return null;
        }
        return PhoneGlobals.getInstance().getCarrierConfigForSubId(phone.getSubId());
    }

    private boolean canDeflectImsCalls() {
        PersistableBundle b = getCarrierConfig();
        // Return false if the CarrierConfig is unavailable
        if (b != null) {
            return b.getBoolean(
                    CarrierConfigManager.KEY_CARRIER_ALLOW_DEFLECT_IMS_CALL_BOOL) &&
                    isValidRingingCall();
        }
        return false;
    }

    /**
     * Determines if the device will respect the value of the
     * {@link CarrierConfigManager#KEY_ALLOW_HOLD_IN_IMS_CALL_BOOL} configuration option.
     *
     * @return {@code false} if the device always supports holding IMS calls, {@code true} if it
     *      will use {@link CarrierConfigManager#KEY_ALLOW_HOLD_IN_IMS_CALL_BOOL} to determine if
     *      hold is supported.
     */
    private boolean doesDeviceRespectHoldCarrierConfig() {
        Phone phone = getPhone();
        if (phone == null) {
            return true;
        }
        return phone.getContext().getResources().getBoolean(
                com.android.internal.R.bool.config_device_respects_hold_carrier_config);
    }

    /**
     * Whether the connection should be treated as an emergency.
     * @return {@code true} if the connection should be treated as an emergency call based
     * on the number dialed, {@code false} otherwise.
     */
    protected boolean shouldTreatAsEmergencyCall() {
        return mTreatAsEmergencyCall;
    }

    /**
     * Un-sets the underlying radio connection.
     */
    void clearOriginalConnection() {
        if (mOriginalConnection != null) {
            if (getPhone() != null) {
                /// M: CC: DTMF request special handling @{
                // Stop DTMF when TelephonyConnection is disconnected
                if (mDtmfRequestIsStarted) {
                    onStopDtmfTone();
                    mDtmfRequestIsStarted = false;
                }
                /// @}
                getPhone().unregisterForPreciseCallStateChanged(mHandler);
                getPhone().unregisterForRingbackTone(mHandler);
                getPhone().unregisterForHandoverStateChanged(mHandler);
                getPhone().unregisterForDisconnect(mHandler);
                getPhone().unregisterForSuppServiceNotification(mHandler);
                getPhone().unregisterForOnHoldTone(mHandler);
                getPhone().unregisterForInCallVoicePrivacyOn(mHandler);
                getPhone().unregisterForInCallVoicePrivacyOff(mHandler);
                /// M: CC: Modem reset related handling
                getPhone().unregisterForRadioOffOrNotAvailable(mHandler);
                /// M: CC: Proprietary CRSS handling @{
                /// M: Makes SuppMessageManager unregister for ImsPhone.
                unregisterSuppMessageManager(getPhone(), mOriginalConnection);
                /// @}
            }
            mOriginalConnection.removePostDialListener(mPostDialListener);
            mOriginalConnection.removeListener(mOriginalConnectionListener);
            // M: [ALPS03494743] Fix JE: Clear all message when connection is cleared
            mHandler.removeCallbacksAndMessages(null); // clear all messages
            mOriginalConnection = null;
        }
    }

    protected void hangup(int telephonyDisconnectCode) {
        /// M: CC: ECC retry @{
        mIsLocallyDisconnecting = true;
        /// @}
        if (mOriginalConnection != null) {
            try {
                // Hanging up a ringing call requires that we invoke call.hangup() as opposed to
                // connection.hangup(). Without this change, the party originating the call
                // will not get sent to voicemail if the user opts to reject the call.
                if (isValidRingingCall()) {
                    Call call = getCall();
                    if (call != null) {
                        /// M: CC: Softbank blacklist requirement @{
                        // this should be a valid ringing call
                        if (mOriginalConnection != null) {
                            if (mOriginalConnection
                                instanceof com.mediatek.internal.telephony.MtkGsmCdmaConnection) {

                                ((com.mediatek.internal.telephony.MtkGsmCdmaConnection) mOriginalConnection)
                                    .setRejectWithCause(telephonyDisconnectCode);
                                call.hangup();

                            } else if (isImsConnection()) {
                                ((MtkImsPhoneCall)call).hangupWithCause(telephonyDisconnectCode);
                            }
                        }
                        /// @}
                    } else {
                        Log.w(this, "Attempting to hangup a connection without backing call.");
                    }
                } else {
                    // We still prefer to call connection.hangup() for non-ringing calls
                    // in order to support hanging-up specific calls within a conference call.
                    // If we invoked call.hangup() while in a conference, we would end up
                    // hanging up the entire conference call instead of the specific connection.
                    mOriginalConnection.hangup();
                }
            } catch (CallStateException e) {
                Log.e(this, e, "Call to Connection.hangup failed with exception");
                /// M: Keep the pending action for SRVCC. @{
                mPendingAction = SrvccPendingAction.SRVCC_PENDING_HANGUP_CALL;
                /// M: CC: ECC retry @{
                mIsLocallyDisconnecting = false;
                /// @}
            }
        } else {
            /// M: CC: ECC retry @{
            if (mTreatAsEmergencyCall) {
                if (MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                    Log.d(this, "ECC Retry : clear ECC param");
                    MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
                }
            }
            /// @}
            if (getState() == STATE_DISCONNECTED) {
                Log.i(this, "hangup called on an already disconnected call!");
                close();
            } else {
                // There are a few cases where mOriginalConnection has not been set yet. For
                // example, when the radio has to be turned on to make an emergency call,
                // mOriginalConnection could not be set for many seconds.
                setDisconnected(DisconnectCauseUtil.toTelecomDisconnectCause(
                        android.telephony.DisconnectCause.LOCAL,
                        "Local Disconnect before connection established."));
                close();
            }
        }
    }

    // M: CC: Declare as public for MTK classes to access
    public com.android.internal.telephony.Connection getOriginalConnection() {
        return mOriginalConnection;
    }

    protected Call getCall() {
        if (mOriginalConnection != null) {
            return mOriginalConnection.getCall();
        }
        return null;
    }

    // M: CC: Declare as public for MTK classes to access
    public Phone getPhone() {
        Call call = getCall();
        if (call != null) {
            return call.getPhone();
        }
        return null;
    }

    private boolean hasMultipleTopLevelCalls() {
        int numCalls = 0;
        Phone phone = getPhone();
        if (phone != null) {
            if (!phone.getRingingCall().isIdle()) {
                numCalls++;
            }
            if (!phone.getForegroundCall().isIdle()) {
                numCalls++;
            }
            if (!phone.getBackgroundCall().isIdle()) {
                numCalls++;
            }
        }
        return numCalls > 1;
    }

    private com.android.internal.telephony.Connection getForegroundConnection() {
        // M: ALPS04302298: There is a race condition between Call.getEarliestConnection() and
        // ImsPhoneCall.detach(). After get the size of mConnections which was
        // 1, the connection just removed from mConnections. So we get an
        // IndexOutOfBoundsException.
        if (getPhone() != null) {
            try {
                return getPhone().getForegroundCall().getEarliestConnection();
            } catch (IndexOutOfBoundsException e) {
                Log.e(this, e, "IndexOutOfBoundsException in getEarliestConnection()");
                return null;
            }
        }
        return null;
    }

     /**
     * Checks for and returns the list of conference participants
     * associated with this connection.
     */
    public List<ConferenceParticipant> getConferenceParticipants() {
        if (mOriginalConnection == null) {
            Log.v(this, "Null mOriginalConnection, cannot get conf participants.");
            return null;
        }
        return mOriginalConnection.getConferenceParticipants();
    }

    /**
     * Checks to see the original connection corresponds to an active incoming call. Returns false
     * if there is no such actual call, or if the associated call is not incoming (See
     * {@link Call.State#isRinging}).
     */
    private boolean isValidRingingCall() {
        if (getPhone() == null) {
            Log.v(this, "isValidRingingCall, phone is null");
            return false;
        }

        Call ringingCall = getPhone().getRingingCall();
        if (!ringingCall.getState().isRinging()) {
            Log.v(this, "isValidRingingCall, ringing call is not in ringing state");
            return false;
        }

        if (ringingCall.getEarliestConnection() != mOriginalConnection) {
            Log.v(this, "isValidRingingCall, ringing call connection does not match");
            return false;
        }

        Log.v(this, "isValidRingingCall, returning true");
        return true;
    }

    // Make sure the extras being passed into this method is a COPY of the original extras Bundle.
    // We do not want the extras to be cleared or modified during mOriginalConnectionExtras.putAll
    // below.
    protected void updateExtras(Bundle extras) {
        if (mOriginalConnection != null) {
            if (extras != null) {
                // Check if extras have changed and need updating.
                if (!areBundlesEqual(mOriginalConnectionExtras, extras)) {
                    if (Log.DEBUG) {
                        Log.d(TelephonyConnection.this, "Updating extras:");
                        for (String key : extras.keySet()) {
                            Object value = extras.get(key);
                            if (value instanceof String) {
                                Log.d(this, "updateExtras Key=" + Log.pii(key) +
                                             " value=" + Log.pii((String)value));
                            }
                        }
                    }
                    mOriginalConnectionExtras.clear();

                    mOriginalConnectionExtras.putAll(extras);

                    // Remap any string extras that have a remapping defined.
                    for (String key : mOriginalConnectionExtras.keySet()) {
                        if (sExtrasMap.containsKey(key)) {
                            String newKey = sExtrasMap.get(key);
                            mOriginalConnectionExtras.putString(newKey, extras.getString(key));
                            mOriginalConnectionExtras.remove(key);
                        }
                    }

                    // Ensure extras are propagated to Telecom.
                    putExtras(mOriginalConnectionExtras);
                } else {
                    Log.d(this, "Extras update not required");
                }
            } else {
                Log.d(this, "updateExtras extras: " + Log.pii(extras));
            }
        }
    }

    private static boolean areBundlesEqual(Bundle extras, Bundle newExtras) {
        if (extras == null || newExtras == null) {
            return extras == newExtras;
        }

        if (extras.size() != newExtras.size()) {
            return false;
        }

        for(String key : extras.keySet()) {
            if (key != null) {
                final Object value = extras.get(key);
                final Object newValue = newExtras.get(key);
                if (!Objects.equals(value, newValue)) {
                    return false;
                }
            }
        }
        return true;
    }

    // M: CC: Declare as public for MtkGsmCdmaConnection to access
    public void setStateOverride(Call.State state) {
        mIsStateOverridden = true;
        mConnectionOverriddenState = state;
        // Need to keep track of the original connection's state before override.
        mOriginalConnectionState = mOriginalConnection.getState();
        updateStateInternal();
    }

    // M: CC: Declare as public for MtkGsmCdmaConnection to access
    public void resetStateOverride() {
        mIsStateOverridden = false;
        updateStateInternal();
    }

    void updateStateInternal() {
        if (mOriginalConnection == null) {
            return;
        }
        Call.State newState;
        // If the state is overridden and the state of the original connection hasn't changed since,
        // then we continue in the overridden state, else we go to the original connection's state.
        if (mIsStateOverridden && mOriginalConnectionState == mOriginalConnection.getState()) {
            newState = mConnectionOverriddenState;
        } else {
            newState = mOriginalConnection.getState();
        }
        int cause = mOriginalConnection.getDisconnectCause();
        Log.v(this, "Update state from %s to %s for %s", mConnectionState, newState,
                getTelecomCallId());

        /// M: CC: ECC retry @{
        // Don't retry if treated as normal call in Telephony Framework
        boolean isDialedByEmergencyCommand = false;
        Phone phone = getPhone();
        if (phone != null) {
            boolean isIms = (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS)
                    && (phone.getDefaultPhone() != null);
            if (isIms) {
                // default phone is GSMPhone or CDMAPhone, which owns the ImsPhone.
                phone = phone.getDefaultPhone();
            }
            if (phone.getEmergencyNumberTracker() != null) {
                if (phone.getEmergencyNumberTracker()
                        .getEmergencyCallRouting(mOriginalConnection.getAddress())
                        != EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL) {
                    isDialedByEmergencyCommand = true;
                }
            }
        }

        if (mTreatAsEmergencyCall
                && !mIsLocallyDisconnecting
                && mOriginalConnection.getState() == Call.State.DISCONNECTED
                && (mConnectionState.isDialing()
                    || mConnectionState == Call.State.IDLE)
                && isDialedByEmergencyCommand) {
            // Assume only one ECC exists
            if (isCauseMatchEccRetry(cause)
                    && MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()
                    && !MtkTelephonyConnectionServiceUtil.getInstance().eccRetryTimeout()) {
                newState = mConnectionState;
                Log.d(this, "ECC retry: meet retry condition, keep state=" + newState);
            }
        }
        /// @}

        if (mConnectionState != newState) {
            mConnectionState = newState;
            switch (newState) {
                case IDLE:
                    break;
                case ACTIVE:
                    /// M: CC: ECC retry @{
                    // Assume only one ECC exists
                    if (mTreatAsEmergencyCall
                            && MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                        Log.d(this, "ECC retry: clear ECC param");
                        MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
                    }
                    /// @}
                    setActiveInternal();
                    break;
                case HOLDING:
                    setOnHold();
                    break;
                case DIALING:
                case ALERTING:
                    if (mOriginalConnection != null && mOriginalConnection.isPulledCall()) {
                        setPulling();
                    } else {
                        setDialing();
                        /// M: CC: [ALPS03749766] notify phone state "alerting" to Telecom @{
                        if (newState == Call.State.ALERTING) {
                            notifyPhoneAlertingState();
                        }
                        /// @}
                    }
                    break;
                case INCOMING:
                case WAITING:
                    setRinging();
                    break;
                case DISCONNECTED:
                    /// M: CC: ECC retry @{
                    // Assume only one ECC exists
                    if (mTreatAsEmergencyCall
                            && MtkTelephonyConnectionServiceUtil.getInstance().isEccRetryOn()) {
                        Log.d(this, "ECC retry: clear ECC param");
                        MtkTelephonyConnectionServiceUtil.getInstance().clearEccRetryParams();
                    }
                    /// @}

                    if (shouldTreatAsEmergencyCall()
                            && (cause
                            == android.telephony.DisconnectCause.EMERGENCY_TEMP_FAILURE
                            || cause
                            == android.telephony.DisconnectCause.EMERGENCY_PERM_FAILURE)) {
                        // We can get into a situation where the radio wants us to redial the
                        // same emergency call on the other available slot. This will not set
                        // the state to disconnected and will instead tell the
                        // TelephonyConnectionService to
                        // create a new originalConnection using the new Slot.
                        fireOnOriginalConnectionRetryDial(cause
                                == android.telephony.DisconnectCause.EMERGENCY_PERM_FAILURE);
                    } else {
                        int preciseDisconnectCause = CallFailCause.NOT_VALID;
                        if (mShowPreciseFailedCause) {
                            preciseDisconnectCause =
                                    mOriginalConnection.getPreciseDisconnectCause();
                        }
                        setDisconnected(DisconnectCauseUtil.toTelecomDisconnectCause(
                                mOriginalConnection.getDisconnectCause(),
                                preciseDisconnectCause,
                                mOriginalConnection.getVendorDisconnectCause(),
                                getPhone().getPhoneId()));
                        close();
                    }
                    break;
                case DISCONNECTING:
                    /// M: CC: ECC retry @{
                    mIsLocallyDisconnecting = true;
                    /// @}
                    break;
            }
        }
    }

    // M: CC: Declare as public for MtkGsmCdmaConnection to access
    public void updateState() {
        if (mOriginalConnection == null) {
            return;
        }

        updateStateInternal();
        updateStatusHints();
        /// M: CC: @{
        // Remove duplicate updateConnectionCapabilities(), for it's called in updateAddress()
        //updateConnectionCapabilities();
        //updateConnectionProperties();
        /// @}
        updateAddress();
        updateMultiparty();
        refreshDisableAddCall();
    }

    /**
     * Checks for changes to the multiparty bit.  If a conference has started, informs listeners.
     */
    private void updateMultiparty() {
        if (mOriginalConnection == null) {
            return;
        }

        if (mIsMultiParty != mOriginalConnection.isMultiparty()) {
            mIsMultiParty = mOriginalConnection.isMultiparty();

            if (mIsMultiParty) {
                notifyConferenceStarted();
            }
        }
    }

    /**
     * Handles a failure when merging calls into a conference.
     * {@link com.android.internal.telephony.Connection.Listener#onConferenceMergedFailed()}
     * listener.
     */
    private void handleConferenceMergeFailed(){
        mHandler.obtainMessage(MSG_CONFERENCE_MERGE_FAILED).sendToTarget();
    }

    /**
     * Handles requests to update the multiparty state received via the
     * {@link com.android.internal.telephony.Connection.Listener#onMultipartyStateChanged(boolean)}
     * listener.
     * <p>
     * Note: We post this to the mHandler to ensure that if a conference must be created as a
     * result of the multiparty state change, the conference creation happens on the correct
     * thread.  This ensures that the thread check in
     * {@link com.android.internal.telephony.Phone#checkCorrectThread(android.os.Handler)}
     * does not fire.
     *
     * @param isMultiParty {@code true} if this connection is multiparty, {@code false} otherwise.
     */
    private void handleMultipartyStateChange(boolean isMultiParty) {
        Log.i(this, "Update multiparty state to %s", isMultiParty ? "Y" : "N");
        mHandler.obtainMessage(MSG_MULTIPARTY_STATE_CHANGED, isMultiParty).sendToTarget();
    }

    // M: CC: Declare as protected for MtkGsmCdmaConnection to access
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
            for (Connection current : getConnectionService().getAllConnections()) {
                if (current != this && current instanceof TelephonyConnection) {
                    TelephonyConnection other = (TelephonyConnection) current;
                    if (other.getState() == STATE_ACTIVE) {
                        other.updateState();
                    }
                }
            }
        }
        setActive();
    }

    protected void close() {
        Log.v(this, "close");
        clearOriginalConnection();
        destroy();
    }

    /**
     * Determines if the current connection is video capable.
     *
     * A connection is deemed to be video capable if the original connection capabilities state that
     * both local and remote video is supported.
     *
     * @return {@code true} if the connection is video capable, {@code false} otherwise.
     */
    private boolean isVideoCapable() {
        return can(mOriginalConnectionCapabilities, Capability.SUPPORTS_VT_LOCAL_BIDIRECTIONAL)
                && can(mOriginalConnectionCapabilities,
                Capability.SUPPORTS_VT_REMOTE_BIDIRECTIONAL);
    }

    /**
     * Determines if the current connection is an external connection.
     *
     * A connection is deemed to be external if the original connection capabilities state that it
     * is.
     *
     * @return {@code true} if the connection is external, {@code false} otherwise.
     */
    private boolean isExternalConnection() {
        return can(mOriginalConnectionCapabilities, Capability.IS_EXTERNAL_CONNECTION);
    }

    /**
     * Determines if the current connection has RTT enabled.
     */
    private boolean isRtt() {
        return mOriginalConnection != null
                && mOriginalConnection.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS
                && mOriginalConnection instanceof ImsPhoneConnection
                && ((ImsPhoneConnection) mOriginalConnection).isRttEnabledForCall();
    }

    /**
     * Determines if the current connection is pullable.
     *
     * A connection is deemed to be pullable if the original connection capabilities state that it
     * is.
     *
     * @return {@code true} if the connection is pullable, {@code false} otherwise.
     */
    private boolean isPullable() {
        return can(mOriginalConnectionCapabilities, Capability.IS_EXTERNAL_CONNECTION)
                && can(mOriginalConnectionCapabilities, Capability.IS_PULLABLE);
    }

    /**
     * Sets whether or not CDMA enhanced call privacy is enabled for this connection.
     */
    private void setCdmaVoicePrivacy(boolean isEnabled) {
        if(mIsCdmaVoicePrivacyEnabled != isEnabled) {
            mIsCdmaVoicePrivacyEnabled = isEnabled;
            updateConnectionProperties();
        }
    }

    /**
     * Applies capabilities specific to conferences termination to the
     * {@code ConnectionCapabilities} bit-mask.
     *
     * @param capabilities The {@code ConnectionCapabilities} bit-mask.
     * @return The capabilities with the IMS conference capabilities applied.
     */
    private int applyConferenceTerminationCapabilities(int capabilities) {
        int currentCapabilities = capabilities;

        // An IMS call cannot be individually disconnected or separated from its parent conference.
        // If the call was IMS, even if it hands over to GMS, these capabilities are not supported.
        /// M: CC: Fix Google bug: No disconnect icon in Conf. Management screen after Conf SRVCC@{
        //[ALPS02026496]
        //if (!mWasImsConnection) {
        if (!isImsConnection()) {
        /// @}
            currentCapabilities |= CAPABILITY_DISCONNECT_FROM_CONFERENCE;
            currentCapabilities |= CAPABILITY_SEPARATE_FROM_CONFERENCE;
        }

        return currentCapabilities;
    }

    /**
     * Stores the new original connection capabilities, and applies them to the current connection,
     * notifying any listeners as necessary.
     *
     * @param connectionCapabilities The original connection capabilties.
     */
    public void setOriginalConnectionCapabilities(int connectionCapabilities) {
        mOriginalConnectionCapabilities = connectionCapabilities;
        updateConnectionCapabilities();
        updateConnectionProperties();
    }

    /**
     * Called to apply the capabilities present in the {@link #mOriginalConnection} to this
     * {@link Connection}.  Provides a mapping between the capabilities present in the original
     * connection (see {@link com.android.internal.telephony.Connection.Capability}) and those in
     * this {@link Connection}.
     *
     * @param capabilities The capabilities bitmask from the {@link Connection}.
     * @return the capabilities bitmask with the original connection capabilities remapped and
     *      applied.
     */
    public int applyOriginalConnectionCapabilities(int capabilities) {
        // We only support downgrading to audio if both the remote and local side support
        // downgrading to audio.
        boolean supportsDowngradeToAudio = can(mOriginalConnectionCapabilities,
                Capability.SUPPORTS_DOWNGRADE_TO_VOICE_LOCAL |
                        Capability.SUPPORTS_DOWNGRADE_TO_VOICE_REMOTE);
        capabilities = changeBitmask(capabilities,
                CAPABILITY_CANNOT_DOWNGRADE_VIDEO_TO_AUDIO, !supportsDowngradeToAudio);

        capabilities = changeBitmask(capabilities, CAPABILITY_SUPPORTS_VT_REMOTE_BIDIRECTIONAL,
                can(mOriginalConnectionCapabilities, Capability.SUPPORTS_VT_REMOTE_BIDIRECTIONAL));

        boolean isLocalVideoSupported = can(mOriginalConnectionCapabilities,
                Capability.SUPPORTS_VT_LOCAL_BIDIRECTIONAL) && !mIsTtyEnabled;
        capabilities = changeBitmask(capabilities, CAPABILITY_SUPPORTS_VT_LOCAL_BIDIRECTIONAL,
                isLocalVideoSupported);

        return capabilities;
    }

    /**
     * Whether the call is using wifi.
     */
    boolean isWifi() {
        return getCallRadioTech() == ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN;
    }

    /**
     * Sets whether this call has been identified by the network as an emergency call.
     * @param isNetworkIdentifiedEmergencyCall {@code true} if the network has identified this call
     * as an emergency call, {@code false} otherwise.
     */
    public void setIsNetworkIdentifiedEmergencyCall(boolean isNetworkIdentifiedEmergencyCall) {
        Log.d(this, "setIsNetworkIdentifiedEmergencyCall; callId=%s, "
                + "isNetworkIdentifiedEmergencyCall=%b", getTelecomCallId(),
                isNetworkIdentifiedEmergencyCall);
        mIsNetworkIdentifiedEmergencyCall = isNetworkIdentifiedEmergencyCall;
        updateConnectionProperties();
    }

    /**
     * @return {@code true} if the network has identified this call as an emergency call,
     * {@code false} otherwise.
     */
    public boolean isNetworkIdentifiedEmergencyCall() {
        return mIsNetworkIdentifiedEmergencyCall;
    }

    /**
     * @return {@code true} if this is an outgoing call, {@code false} otherwise.
     */
    boolean isOutgoingCall() {
        return mIsOutgoing;
    }

    /**
     * Sets the current call audio quality. Used during rebuild of the properties
     * to set or unset the {@link Connection#PROPERTY_HIGH_DEF_AUDIO} property.
     *
     * @param audioQuality The audio quality.
     */
    public void setAudioQuality(int audioQuality) {
        mHasHighDefAudio = audioQuality ==
                com.android.internal.telephony.Connection.AUDIO_QUALITY_HIGH_DEFINITION;
        updateConnectionProperties();
    }

    // M: CC: Declare as public for MtkGsmCdmaConnection to access
    public void resetStateForConference() {
        if (getState() == Connection.STATE_HOLDING
                // M: CC: In CdmaConferenceController.mDelayRunnable we set all connections to
                // active before calling resetStateForConference(). So need to check STATE_ACTIVE
                // here to ensure state updated invoked correctly.
                || getState() == Connection.STATE_ACTIVE) {
            resetStateOverride();
        }
    }

    // M: CC: Declare as public for MtkGsmCdmaConnection to access
    public boolean setHoldingForConference() {
        if (getState() == Connection.STATE_ACTIVE) {
            setStateOverride(Call.State.HOLDING);
            return true;
        }
        return false;
    }

    public void setRttTextStream(RttTextStream s) {
        mRttTextStream = s;
    }

    public RttTextStream getRttTextStream() {
        return mRttTextStream;
    }

    /**
     * For video calls, sets whether this connection supports pausing the outgoing video for the
     * call using the {@link android.telecom.VideoProfile#STATE_PAUSED} VideoState.
     *
     * @param isVideoPauseSupported {@code true} if pause state supported, {@code false} otherwise.
     */
    public void setVideoPauseSupported(boolean isVideoPauseSupported) {
        mIsVideoPauseSupported = isVideoPauseSupported;
    }

    /**
     * @return {@code true} if this connection supports pausing the outgoing video using the
     * {@link android.telecom.VideoProfile#STATE_PAUSED} VideoState.
     */
    public boolean getVideoPauseSupported() {
        return mIsVideoPauseSupported;
    }

    /**
     * Sets whether this connection supports conference calling.
     * @param isConferenceSupported {@code true} if conference calling is supported by this
     *                                         connection, {@code false} otherwise.
     */
    public void setConferenceSupported(boolean isConferenceSupported) {
        mIsConferenceSupported = isConferenceSupported;
    }

    /**
     * @return {@code true} if this connection supports merging calls into a conference.
     */
    public boolean isConferenceSupported() {
        return mIsConferenceSupported;
    }

    /**
     * Sets whether managing conference call is supported after this connection being a part of a
     * Ims conference.
     *
     * @param isManageImsConferenceCallSupported {@code true} if manage conference calling is
     *        supported after this connection being a part of a IMS conference,
     *        {@code false} otherwise.
     */
    public void setManageImsConferenceCallSupported(boolean isManageImsConferenceCallSupported) {
        mIsManageImsConferenceCallSupported = isManageImsConferenceCallSupported;
    }

    /**
     * @return {@code true} if manage conference calling is supported after this connection being a
     * part of a IMS conference.
     */
    public boolean isManageImsConferenceCallSupported() {
        return mIsManageImsConferenceCallSupported;
    }

    /**
     * Sets whether this connection supports showing precise call disconnect cause.
     * @param showPreciseFailedCause  {@code true} if showing precise call
     * disconnect cause is supported by this connection, {@code false} otherwise.
     */
    public void setShowPreciseFailedCause(boolean showPreciseFailedCause) {
        mShowPreciseFailedCause = showPreciseFailedCause;
    }

    /**
     * Sets whether TTY is enabled or not.
     * @param isTtyEnabled
     */
    public void setTtyEnabled(boolean isTtyEnabled) {
        mIsTtyEnabled = isTtyEnabled;
        updateConnectionCapabilities();
    }

    /**
     * Whether the original connection is an IMS connection.
     * @return {@code True} if the original connection is an IMS connection, {@code false}
     *     otherwise.
     */
    protected boolean isImsConnection() {
        com.android.internal.telephony.Connection originalConnection = getOriginalConnection();
        return originalConnection != null &&
                originalConnection.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS;
    }

    /**
     * Whether the original connection is an GSM/CDMA connection.
     * @return {@code True} if the original connection is an GSM/CDMA connection, {@code false}
     *     otherwise.
     */
    protected boolean isGsmCdmaConnection() {
        Phone phone = getPhone();
        if (phone != null) {
            switch (phone.getPhoneType()) {
                case PhoneConstants.PHONE_TYPE_GSM:
                case PhoneConstants.PHONE_TYPE_CDMA:
                    return true;
                default:
                    return false;
            }
        }
        return false;
    }

    /**
     * Whether the original connection was ever an IMS connection, either before or now.
     * @return {@code True} if the original connection was ever an IMS connection, {@code false}
     *     otherwise.
     */
    public boolean wasImsConnection() {
        return mWasImsConnection;
    }

    boolean getIsUsingAssistedDialing() {
        return mIsUsingAssistedDialing;
    }

    void setIsUsingAssistedDialing(Boolean isUsingAssistedDialing) {
        mIsUsingAssistedDialing = isUsingAssistedDialing;
        updateConnectionProperties();
    }

    private static Uri getAddressFromNumber(String number) {
        // Address can be null for blocked calls.
        if (number == null) {
            number = "";
        }
        return Uri.fromParts(PhoneAccount.SCHEME_TEL, number, null);
    }

    /**
     * Changes a capabilities bit-mask to add or remove a capability.
     *
     * @param bitmask The bit-mask.
     * @param bitfield The bit-field to change.
     * @param enabled Whether the bit-field should be set or removed.
     * @return The bit-mask with the bit-field changed.
     */
    private int changeBitmask(int bitmask, int bitfield, boolean enabled) {
        if (enabled) {
            return bitmask | bitfield;
        } else {
            return bitmask & ~bitfield;
        }
    }

    private void updateStatusHints() {
        if (isWifi() && getPhone() != null) {
            int labelId = isValidRingingCall()
                    ? R.string.status_hint_label_incoming_wifi_call
                    : R.string.status_hint_label_wifi_call;

            Context context = getPhone().getContext();
            setStatusHints(new StatusHints(
                    context.getString(labelId),
                    Icon.createWithResource(
                            context, R.drawable.ic_signal_wifi_4_bar_24dp),
                    null /* extras */));
        } else {
            setStatusHints(null);
        }
    }

    /**
     * Register a listener for {@link TelephonyConnection} specific triggers.
     * @param l The instance of the listener to add
     * @return The connection being listened to
     */
    public final TelephonyConnection addTelephonyConnectionListener(TelephonyConnectionListener l) {
        mTelephonyListeners.add(l);
        // If we already have an original connection, let's call back immediately.
        // This would be the case for incoming calls.
        if (mOriginalConnection != null) {
            fireOnOriginalConnectionConfigured();
        }
        return this;
    }

    /**
     * Remove a listener for {@link TelephonyConnection} specific triggers.
     * @param l The instance of the listener to remove
     * @return The connection being listened to
     */
    public final TelephonyConnection removeTelephonyConnectionListener(
            TelephonyConnectionListener l) {
        if (l != null) {
            mTelephonyListeners.remove(l);
        }
        return this;
    }

    @Override
    public void setHoldable(boolean isHoldable) {
        mIsHoldable = isHoldable;
        updateConnectionCapabilities();
    }

    @Override
    public boolean isChildHoldable() {
        return getConference() != null;
    }

    public boolean isHoldable() {
        return mIsHoldable;
    }

    /**
     * Fire a callback to the various listeners for when the original connection is
     * set in this {@link TelephonyConnection}
     */
    private final void fireOnOriginalConnectionConfigured() {
        for (TelephonyConnectionListener l : mTelephonyListeners) {
            l.onOriginalConnectionConfigured(this);
        }
    }

    private final void fireOnOriginalConnectionRetryDial(boolean isPermanentFailure) {
        for (TelephonyConnectionListener l : mTelephonyListeners) {
            l.onOriginalConnectionRetry(this, isPermanentFailure);
        }
    }

    /**
     * Handles exiting ECM mode.
     */
    protected void handleExitedEcmMode() {
        updateConnectionProperties();
        /// M: CC: need to update capabilities after exiting ECBM (for mute) [ALPS03266897]
        updateConnectionCapabilities();
    }

    /**
     * Determines whether the connection supports conference calling.  A connection supports
     * conference calling if it:
     * 1. Is not an emergency call.
     * 2. Carrier supports conference calls.
     * 3. If call is a video call, carrier supports video conference calls or the property
     *    "persist.vendor.vt.video_conference_support" equal to 1 (Ex: FTA test).
     * 4. If call is a wifi call and VoWIFI is disabled and carrier supports merging these calls.
     */
    private void refreshConferenceSupported() {
        boolean isVideoCall = VideoProfile.isVideo(getVideoState());
        Phone phone = getPhone();
        if (phone == null) {
            Log.w(this, "refreshConferenceSupported = false; phone is null");
            if (isConferenceSupported()) {
                setConferenceSupported(false);
                notifyConferenceSupportedChanged(false);
            }
            return;
        }

        boolean isIms = phone.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS;
        boolean isVoWifiEnabled = false;
        if (isIms) {
            isVoWifiEnabled = ImsUtil.isWfcEnabled(phone.getContext(), phone.getPhoneId());
        }
        PhoneAccountHandle phoneAccountHandle = isIms ? PhoneUtils
                .makePstnPhoneAccountHandle(phone.getDefaultPhone())
                : PhoneUtils.makePstnPhoneAccountHandle(phone);
        TelecomAccountRegistry telecomAccountRegistry = TelecomAccountRegistry
                .getInstance(getPhone().getContext());
        boolean isConferencingSupported = telecomAccountRegistry
                .isMergeCallSupported(phoneAccountHandle);
        boolean isImsConferencingSupported = telecomAccountRegistry
                .isMergeImsCallSupported(phoneAccountHandle);
        mIsCarrierVideoConferencingSupported = telecomAccountRegistry
                .isVideoConferencingSupported(phoneAccountHandle);
        boolean isMergeOfWifiCallsAllowedWhenVoWifiOff = telecomAccountRegistry
                .isMergeOfWifiCallsAllowedWhenVoWifiOff(phoneAccountHandle);

        Log.v(this, "refreshConferenceSupported : isConfSupp=%b, isImsConfSupp=%b, " +
                "isVidConfSupp=%b, isMergeOfWifiAllowed=%b, " +
                "isWifi=%b, isVoWifiEnabled=%b",
                isConferencingSupported, isImsConferencingSupported,
                mIsCarrierVideoConferencingSupported, isMergeOfWifiCallsAllowedWhenVoWifiOff,
                isWifi(), isVoWifiEnabled);
        boolean isConferenceSupported = true;
        if (mTreatAsEmergencyCall) {
            isConferenceSupported = false;
            Log.d(this, "refreshConferenceSupported = false; emergency call");
        } else if (isRtt() && !isRttCallMergeSupported()) {
            isConferenceSupported = false;
            Log.d(this, "refreshConferenceSupported = false; rtt call");
        } else if (!isConferencingSupported || isIms && !isImsConferencingSupported) {
            isConferenceSupported = false;
            Log.d(this, "refreshConferenceSupported = false; carrier doesn't support conf.");
        } else if (isVideoCall && (!mIsCarrierVideoConferencingSupported
                && !SystemProperties.get("persist.vendor.vt.video_conference_support", "0")
                    .equals("1"))) {
            isConferenceSupported = false;
            Log.d(this, "refreshConferenceSupported = false; video conf not supported.");
        } else if (!isMergeOfWifiCallsAllowedWhenVoWifiOff && isWifi() && !isVoWifiEnabled) {
            isConferenceSupported = false;
            Log.d(this,
                    "refreshConferenceSupported = false; can't merge wifi calls when voWifi off.");
        } else {
            Log.d(this, "refreshConferenceSupported = true.");
        }

        if (isConferenceSupported != isConferenceSupported()) {
            setConferenceSupported(isConferenceSupported);
            notifyConferenceSupportedChanged(isConferenceSupported);
        }
    }
    /**
     * Provides a mapping from extras keys which may be found in the
     * {@link com.android.internal.telephony.Connection} to their equivalents defined in
     * {@link android.telecom.Connection}.
     *
     * @return Map containing key mappings.
     */
    private static Map<String, String> createExtrasMap() {
        Map<String, String> result = new HashMap<String, String>();
        result.put(ImsCallProfile.EXTRA_CHILD_NUMBER,
                android.telecom.Connection.EXTRA_CHILD_ADDRESS);
        result.put(ImsCallProfile.EXTRA_DISPLAY_TEXT,
                android.telecom.Connection.EXTRA_CALL_SUBJECT);
        result.put(ImsCallProfile.EXTRA_ADDITIONAL_SIP_INVITE_FIELDS,
                android.telecom.Connection.EXTRA_SIP_INVITE);
        // RTT for VzW: was video call cannot upgrade to RTT
        result.put(EXTRA_WAS_VIDEO_CALL,
                MtkTelecomManager.EXTRA_WAS_VIDEO_CALL);
        return Collections.unmodifiableMap(result);
    }

    private boolean isShowingOriginalDialString() {
        boolean showOrigDialString = false;
        Phone phone = getPhone();
        if (phone != null && (phone.getPhoneType() == TelephonyManager.PHONE_TYPE_CDMA)
                && !mOriginalConnection.isIncoming()) {
            PersistableBundle pb = getCarrierConfig();
            if (pb != null) {
                showOrigDialString = pb.getBoolean(CarrierConfigManager
                        .KEY_CONFIG_SHOW_ORIG_DIAL_STRING_FOR_CDMA_BOOL);
                Log.d(this, "showOrigDialString: " + showOrigDialString);
            }
        }
        return showOrigDialString;
    }

    /**
     * Creates a string representation of this {@link TelephonyConnection}.  Primarily intended for
     * use in log statements.
     *
     * @return String representation of the connection.
     */
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("[TelephonyConnection objId:");
        sb.append(System.identityHashCode(this));
        sb.append(" telecomCallID:");
        sb.append(getTelecomCallId());
        sb.append(" type:");
        if (isImsConnection()) {
            sb.append("ims");
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        //} else if (this instanceof com.android.services.telephony.GsmConnection) {
        } else if ((this instanceof MtkGsmCdmaConnection) &&
                (((MtkGsmCdmaConnection) this).getPhoneType() == PhoneConstants.PHONE_TYPE_GSM)) {
        /// @}
            sb.append("gsm");
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        //} else if (this instanceof CdmaConnection) {
        } else if ((this instanceof MtkGsmCdmaConnection) &&
                (((MtkGsmCdmaConnection) this).getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA)) {
        /// @}
            sb.append("cdma");
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        } else if (this instanceof MtkGsmCdmaConnection) {
            sb.append("gsmcdma");
        }
        /// @}
        sb.append(" state:");
        sb.append(Connection.stateToString(getState()));
        sb.append(" capabilities:");
        sb.append(MtkConnection.capabilitiesToString(getConnectionCapabilities()));
        sb.append(" properties:");
        sb.append(MtkConnection.propertiesToString(getConnectionProperties()));
        sb.append(" address:");
        sb.append(Log.pii(getAddress()));
        sb.append(" originalConnection:");
        sb.append(mOriginalConnection);
        sb.append(" partOfConf:");
        if (getConference() == null) {
            sb.append("N");
        } else {
            sb.append("Y");
        }
        sb.append(" confSupported:");
        sb.append(mIsConferenceSupported ? "Y" : "N");
        sb.append("]");
        return sb.toString();
    }

    /// M: CC: Set PhoneAccountHandle for ECC @{
    //[ALPS01794357]
    private PhoneAccountHandle mAccountHandle;
    /**
     * @return The PhoneAccountHandle this connection really used.
     */
    public PhoneAccountHandle getAccountHandle() { return mAccountHandle; }

    /**
     *  set the PhoneAccountHandle this connection really used.
     */
    public void setAccountHandle(PhoneAccountHandle handle) {
        mAccountHandle = handle;
    }
    /// @}

    /// M: CC: Modem reset related handling @{
    /**
     * Notify to telecomm if the connection is lost.
     * @hide
     */
    private void notifyConnectionLost() {
        Log.d(this, "notifyConnectionLost");
        sendConnectionEvent(MtkConnection.EVENT_CONNECTION_LOST, null);
        setDisconnected(DisconnectCauseUtil.toTelecomDisconnectCause(
                android.telephony.DisconnectCause.LOCAL,
                "Local Disconnect due to radio off."));
        close();
    }
    /// @}

    /// M: CC: [ALPS03749766] notify phone state "alerting" to Telecom.
    private void notifyPhoneAlertingState() {
        Log.d(this, "notifyPhoneAlertingState");
        sendConnectionEvent(MtkConnection.EVENT_CALL_ALERTING_NOTIFICATION, null);
    }
    /// @}

    /// M: CC: Force updateState for Connection once its ConnectionService is set @{
    /**
     * Base class for forcing call state update after ConnectionService is set,
     * to keep phoneCapabilities up-to-date.
     * see {@link ConnectionService#addConnection}
     * To be overrided by children classes.
     * @hide
     */
    protected void fireOnCallState() {
        updateState();
    }
    /// @}

    /// M: CC: HangupAll for FTA 31.4.4.2 @{
    public void onHangupAll() {
        Log.d(this, "onHangupAll");
        if (mOriginalConnection != null) {
            try {
                Phone phone = getPhone();
                if (phone != null && phone instanceof MtkGsmCdmaPhone) {
                    ((MtkGsmCdmaPhone)phone).hangupAll();
                } else if (phone != null && phone instanceof MtkImsPhone) {
                    ((MtkImsPhone)phone).hangupAll();
                } else {
                    Log.w(this, "Attempting to hangupAll a connection without backing phone.");
                }
            } catch (CallStateException e) {
                Log.e(this, e, "Call to phone.hangupAll() failed with exception");
            }
        }
    }
    /// @}

    /// M: CC: Interface for ECT @{
    /**
     * Handle explicit call transfer.
     * @hide
     */
    public void onExplicitCallTransfer() {
        Log.d(this, "onExplicitCallTransfer");
        Phone phone = mOriginalConnection.getCall().getPhone();
        try {
            phone.explicitCallTransfer();
        } catch (CallStateException e) {
            Log.e(this, e, "Exception occurred while trying to do ECT.");
        }
    }

    /// M: CC: ECC retry @{
    protected void resetTreatAsEmergencyCall() {
        mTreatAsEmergencyCall = false;
    }

    protected void setEmergencyCall(boolean isEmergency) {
        mTreatAsEmergencyCall = isEmergency;
        if (isEmergency) {
            Log.d(this, "ECC retry: set call as emergency call");
        }
    }
    /// @}

    /**
     * M: check if this RTT call can be merged.
     */
    private boolean isRttCallMergeSupported() {
        PersistableBundle b = getCarrierConfig();
        boolean rttCallMergeSupported =
            b.getBoolean(MtkCarrierConfigManager.MTK_KEY_RTT_CALL_MERGE_SUPPORTED_BOOL);

        Log.d(this, "isRttCallMergeSupported: " + rttCallMergeSupported);
        return rttCallMergeSupported;
    }

    /// M: CC: Proprietary CRSS handling @{
    /** @hide */
    public final void notifyActionFailed(int action) {
        Log.i(this, "notifyActionFailed action = " + action);
        sendConnectionEvent(MtkConnection.EVENT_OPERATION_FAILED,
                MtkConnection.ConnectionEventHelper.buildParamsForOperationFailed(action));
    }

    /** @hide */
    public void notifySSNotificationToast(
            int notiType, int type, int code, String number, int index) {
        Log.i(this, "notifySSNotificationToast notiType = " + notiType + " type = " + type
                + " code = " + code + " number = " + Log.pii(number) + " index = " + index);
        sendConnectionEvent(MtkConnection.EVENT_SS_NOTIFICATION,
                MtkConnection.ConnectionEventHelper.buildParamsForSsNotification(notiType, type,
                        code, number, index));
    }

    /** @hide */
    public void notifyNumberUpdate(String number) {
        Log.i(this, "notifyNumberUpdate number = " + Log.pii(number));
        if (!TextUtils.isEmpty(number)) {
            sendConnectionEvent(MtkConnection.EVENT_NUMBER_UPDATED,
                    MtkConnection.ConnectionEventHelper.buildParamsForNumberUpdated(number));
        }
    }

    /** @hide */
    public void notifyIncomingInfoUpdate(int type, String alphaid, int cliValidity) {
        Log.i(this, "notifyIncomingInfoUpdate type = "
                + type + " alphaid = " + alphaid + " cliValidity = " + cliValidity);
        sendConnectionEvent(MtkConnection.EVENT_INCOMING_INFO_UPDATED,
                MtkConnection.ConnectionEventHelper.buildParamsForIncomingInfoUpdated(type,
                        alphaid, cliValidity));
    }

    /// M: For IMS blind/assured ECT @{
    public void onExplicitCallTransfer(String number, int type) {
        Log.v(this, "onExplicitCallTransfer");
        Phone phone = mOriginalConnection.getCall().getPhone();
        if (phone instanceof MtkImsPhone) {
            ((MtkImsPhone)phone).explicitCallTransfer(number, type);
        }
    }

    /// M: For IMS CC @{
    private void trySrvccPendingAction() {
        Log.d(this, "trySrvccPendingAction(): " + mPendingAction);
        switch(mPendingAction) {
            case SRVCC_PENDING_ANSWER_CALL:
                onAnswer(VideoProfile.STATE_AUDIO_ONLY);
                break;
            case SRVCC_PENDING_HOLD_CALL:
                performHold();
                break;
            case SRVCC_PENDING_UNHOLD_CALL:
                performUnhold();
                break;
            case SRVCC_PENDING_HANGUP_CALL:
                hangup(android.telephony.DisconnectCause.LOCAL);
                break;
            default:
                break;
        }
        mPendingAction = SrvccPendingAction.SRVCC_PENDING_NONE;
    }

    /// MTK Property VoLte handling. @{
    private int updatePropertyVoLte(int currentProperties) {
        int newProperties = changeBitmask(
                currentProperties, MtkConnection.PROPERTY_VOLTE, isImsConnection());
        Log.d(this, "updatePropertyVoLte: " + MtkConnection.propertiesToString(newProperties));
        return newProperties;
    }
    /// @}

    /// MTK Property VoLte handling. @{
    private void removePropertyVoLte() {
        int newProperties = getConnectionProperties();
        newProperties = changeBitmask(newProperties, MtkConnection.PROPERTY_VOLTE, false);
        Log.d(this, "removePropertyVoLte: %s", MtkConnection.propertiesToString(newProperties));
        if (getConnectionProperties() != newProperties) {
            setConnectionProperties(newProperties);
        }
    }
    /// @}

    /// Update video ringtone capability. @{
    private int applyVideoRingtoneCapabilities(int originalConnectionCapabilities,
            int currentCapabilities) {
        int capabilities = currentCapabilities;
        if (getState() == STATE_DIALING) {
            boolean supportVideoRingtone = can(originalConnectionCapabilities,
                    MtkImsPhoneConnection.SUPPORTS_VT_RINGTONE);
            capabilities = changeBitmask(capabilities,
                    MtkConnection.CAPABILITY_VIDEO_RINGTONE, supportVideoRingtone);
            Log.d(this, "applyVideoRingtoneCapabilities: " + supportVideoRingtone);
        }
        return capabilities;
    }
    /// @}

    public void performInviteConferenceParticipants(List<String> numbers) {
        if (mOriginalConnection == null) {
            Log.e(this, new CallStateException(), "no orginal connection to inviteParticipants");
            return;
        }
        if (!isImsConnection()) {
            Log.e(this, new CallStateException(), "CS connection doesn't support invite!");
            return;
        }
        ((MtkImsPhoneConnection)mOriginalConnection).inviteConferenceParticipants(numbers);
    }
    /**
     * This function used to notify the onInviteConferenceParticipants() operation is done.
     * @param isSuccess is success or not
     * @hide
     */
    protected void notifyConferenceParticipantsInvited(boolean isSuccess) {
        for (TelephonyConnectionListener l : mTelephonyListeners) {
            l.onConferenceParticipantsInvited(isSuccess);
        }
    }
    /**
     * For conference SRVCC.
     * @param radioConnections new participant connections
     */
    private void notifyConferenceConnectionsConfigured(
            ArrayList<com.android.internal.telephony.Connection> radioConnections) {
        for (TelephonyConnectionListener l : mTelephonyListeners) {
            l.onConferenceConnectionsConfigured(radioConnections);
        }
    }
    ///@}

    /// M: register/unregister ImsPhone to SuppMessageManager @{
    private void registerSuppMessageManager(Phone phone,
                                            com.android.internal.telephony.Connection conn) {
        Log.d(this, "registerSuppMessageManager: " + phone);
        if ((phone instanceof MtkImsPhone) != true) {
            return;
        }
        MtkTelephonyConnectionServiceUtil.getInstance().registerSuppMessageForImsPhone(phone, conn);
    }
    private void unregisterSuppMessageManager(Phone phone,
                                              com.android.internal.telephony.Connection conn) {
        Log.d(this, "unregisterSuppMessageManager: " + phone);
        if ((phone instanceof MtkImsPhone) != true) {
            return;
        }

        MtkTelephonyConnectionServiceUtil.getInstance().unregisterSuppMessageForImsPhone(phone,
                                                                                         conn);
    }
    /// @}

    /**
     * Notify this connection is ECC.
     */
    public void notifyEcc() {
        sendConnectionEvent(MtkConnection.EVENT_VOLTE_MARKED_AS_EMERGENCY, null);
    }

    /**
     * Approve ECC redial.
     */
    public void approveEccRedial(boolean isAprroved) {
        Phone phone = getPhone();
        if (phone == null) {
            return;
        }
        if (phone instanceof ImsPhone) {
            ImsPhone imsPhone = (ImsPhone) phone;
            if (imsPhone.getForegroundCall() != null
                    && imsPhone.getForegroundCall().getImsCall() != null) {
                ImsCall call = imsPhone.getForegroundCall().getImsCall();
                if (call != null && call instanceof MtkImsCall) {
                    ((MtkImsCall)call).approveEccRedial(isAprroved);
                }
            }
        }
    }

    private boolean isCauseMatchEccRetry(int cause) {
        // IMS getPreciseDisconnectCause() is always 0, cannot check
        int callFailCause = mOriginalConnection == null ||
                mOriginalConnection.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS ||
                mOriginalConnection instanceof ImsPhoneConnection
                ? CallFailCause.NOT_VALID
                : mOriginalConnection.getPreciseDisconnectCause();

        Log.d(this, "ECC retry: cause=" + cause + " , callFailCause=" + callFailCause);

        if (cause != android.telephony.DisconnectCause.NOT_DISCONNECTED
                    // [ALPS03363271] Not retry ECC when modem reset
                    && cause != android.telephony.DisconnectCause.LOST_SIGNAL
                    && cause != android.telephony.DisconnectCause.NORMAL
                    && cause != android.telephony.DisconnectCause.NORMAL_UNSPECIFIED
                    && cause != android.telephony.DisconnectCause.LOCAL
                    // [ALPS03869159] [ALPS03866015] No ECC retry if no cause from network
                    && callFailCause != 0
                    // No ECC retry with CM_SER_OPT_UNIMPL(+CEER: 79)
                    && callFailCause != CallFailCause.SERVICE_OR_OPTION_NOT_IMPLEMENTED
                    // [ALPS04980465] No ECC retry if CallFailCause is NORMAL, for it is
                    // converted to DisconnectCause OUT_OF_SERVICE if it is not ECC number
                    && callFailCause != CallFailCause.NORMAL_CLEARING
                    && callFailCause != CallFailCause.NORMAL_UNSPECIFIED
                    // [ALPS02500376][ALPS04930750][ALPS04882587][ALPS04982805]No ECC retry
                    // with IMEI_NOT_ACCEPTED and Sim is absent for GCF TC_26.9.6.2.2
                    && (cause != android.telephony.DisconnectCause.IMEI_NOT_ACCEPTED
                            || (cause == android.telephony.DisconnectCause.IMEI_NOT_ACCEPTED
                            && isAllSimInserted()))) {
            return true;
        }
        return false;
    }

    private static boolean isAllSimInserted() {
        boolean allSimInserted = true;
        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
            if (!TelephonyManager.getDefault().hasIccCard(i)) {
                allSimInserted = false;
                break;
            }
        }
        return allSimInserted;
    }
}

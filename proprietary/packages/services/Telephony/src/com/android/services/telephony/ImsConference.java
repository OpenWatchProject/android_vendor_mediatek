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
 * limitations under the License
 */

package com.android.services.telephony;

import android.content.Context;
import android.graphics.drawable.Icon;
import android.net.Uri;
import android.os.Bundle;
import android.os.PersistableBundle;
/// M: ALPS04234226. For GCF case 15.19. @{
import android.os.SystemProperties;
/// @}
import android.telecom.Conference;
/// M: For multi anchor feature @{
import android.telecom.Conferenceable;
/// @}
import android.telecom.ConferenceParticipant;
import android.telecom.Connection;
import android.telecom.Connection.VideoProvider;
import android.telecom.DisconnectCause;
import android.telecom.Log;
import android.telecom.PhoneAccountHandle;
import android.telecom.StatusHints;
import android.telecom.TelecomManager;
import android.telecom.VideoProfile;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.telephony.PhoneNumberUtils;
import android.util.Pair;
import android.widget.Toast;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.phone.PhoneGlobals;
import com.android.phone.PhoneUtils;
import com.android.phone.R;

import com.mediatek.internal.telephony.imsphone.MtkImsPhone;

import java.util.ArrayList;
import java.util.Arrays;
/// M: For multi anchor feature @{
import java.util.Collections;
/// @}
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Objects;

import com.mediatek.internal.telephony.imsphone.MtkImsPhone;

import mediatek.telecom.MtkConnection;
import mediatek.telephony.MtkCarrierConfigManager;

/**
 * Represents an IMS conference call.
 * <p>
 * An IMS conference call consists of a conference host connection and potentially a list of
 * conference participants.  The conference host connection represents the radio connection to the
 * IMS conference server.  Since it is not a connection to any one individual, it is not represented
 * in Telecom/InCall as a call.  The conference participant information is received via the host
 * connection via a conference event package.  Conference participant connections do not represent
 * actual radio connections to the participants; they act as a virtual representation of the
 * participant, keyed by a unique endpoint {@link android.net.Uri}.
 * <p>
 * The {@link ImsConference} listens for conference event package data received via the host
 * connection and is responsible for managing the conference participant connections which represent
 * the participants.
 */
public class ImsConference extends Conference implements Holdable {

    /**
     * Abstracts out fetching a feature flag.  Makes testing easier.
     */
    public interface FeatureFlagProxy {
        boolean isUsingSinglePartyCallEmulation();
    }

    /**
     * Listener used to respond to changes to conference participants.  At the conference level we
     * are most concerned with handling destruction of a conference participant.
     */
    private final Connection.Listener mParticipantListener = new Connection.Listener() {
        /**
         * Participant has been destroyed.  Remove it from the conference.
         *
         * @param connection The participant which was destroyed.
         */
        @Override
        public void onDestroyed(Connection connection) {
            ConferenceParticipantConnection participant =
                    (ConferenceParticipantConnection) connection;
            removeConferenceParticipant(participant);
            updateManageConference();
        }

    };

    /**
     * Listener used to respond to changes to the underlying radio connection for the conference
     * host connection.  Used to respond to SRVCC changes.
     */
    private final TelephonyConnection.TelephonyConnectionListener mTelephonyConnectionListener =
            new TelephonyConnection.TelephonyConnectionListener() {

        @Override
        public void onOriginalConnectionConfigured(TelephonyConnection c) {
            if (c == mConferenceHost) {
               handleOriginalConnectionChange();
            }
        }
        /// M: VoLTE. @{
        /**
         * For VoLTE enhanced conference call, notify invite conf. participants completed.
         * @param isSuccess is success or not.
         */
        @Override
        public void onConferenceParticipantsInvited(boolean isSuccess) {
            mIsDuringAddingParticipants = false;
        }
        /**
         * For VoLTE conference SRVCC, notify when new participant connections made.
         * @param radioConnections new participant connections.
         */
        @Override
        public void onConferenceConnectionsConfigured(
                ArrayList<com.android.internal.telephony.Connection> radioConnections) {
            handleConferenceSRVCC(radioConnections);
        }
        /// @}
    };

    /**
     * Listener used to respond to changes to the connection to the IMS conference server.
     */
    private final android.telecom.Connection.Listener mConferenceHostListener =
            new android.telecom.Connection.Listener() {

        /**
         * Updates the state of the conference based on the new state of the host.
         *
         * @param c The host connection.
         * @param state The new state
         */
        @Override
        public void onStateChanged(android.telecom.Connection c, int state) {
            setState(state);
        }

        /**
         * Disconnects the conference when its host connection disconnects.
         *
         * @param c The host connection.
         * @param disconnectCause The host connection disconnect cause.
         */
        @Override
        public void onDisconnected(android.telecom.Connection c, DisconnectCause disconnectCause) {
            setDisconnected(disconnectCause);
        }

        /**
         * Handles changes to conference participant data as reported by the conference host
         * connection.
         *
         * @param c The connection.
         * @param participants The participant information.
         */
        @Override
        public void onConferenceParticipantsChanged(android.telecom.Connection c,
                List<ConferenceParticipant> participants) {

            if (c == null || participants == null) {
                return;
            }
            Log.d(this, "onConferenceParticipantsChanged: %d participants", participants.size());
            TelephonyConnection telephonyConnection = (TelephonyConnection) c;
            handleConferenceParticipantsUpdate(telephonyConnection, participants);
        }

        @Override
        public void onVideoStateChanged(android.telecom.Connection c, int videoState) {
            Log.d(this, "onVideoStateChanged video state %d", videoState);
            setVideoState(c, videoState);
        }

        @Override
        public void onVideoProviderChanged(android.telecom.Connection c,
                Connection.VideoProvider videoProvider) {
            Log.d(this, "onVideoProviderChanged: Connection: %s, VideoProvider: %s", c,
                    videoProvider);
            setVideoProvider(c, videoProvider);
        }

        @Override
        public void onConnectionCapabilitiesChanged(Connection c, int connectionCapabilities) {
            Log.d(this, "onConnectionCapabilitiesChanged: Connection: %s," +
                    " connectionCapabilities: %s", c, connectionCapabilities);
            int capabilites = ImsConference.this.getConnectionCapabilities();
            boolean isVideoConferencingSupported = mConferenceHost == null ? false :
                    mConferenceHost.isCarrierVideoConferencingSupported();
            setConnectionCapabilities(applyHostCapabilities(capabilites, connectionCapabilities,
                    isVideoConferencingSupported));
        }

        @Override
        public void onConnectionPropertiesChanged(Connection c, int connectionProperties) {
            Log.d(this, "onConnectionPropertiesChanged: Connection: %s," +
                    " connectionProperties: %s", c, connectionProperties);
            int properties = ImsConference.this.getConnectionProperties();
            setConnectionProperties(applyHostProperties(properties, connectionProperties));
        }

        @Override
        public void onStatusHintsChanged(Connection c, StatusHints statusHints) {
            Log.d(this, "onStatusHintsChanged");
            updateStatusHints();
        }

        @Override
        public void onExtrasChanged(Connection c, Bundle extras) {
            Log.v(this, "onExtrasChanged: c=" + c + " Extras=" + extras);
            putExtras(extras);
        }

        @Override
        public void onExtrasRemoved(Connection c, List<String> keys) {
            Log.v(this, "onExtrasRemoved: c=" + c + " key=" + keys);
            removeExtras(keys);
        }

        @Override
        public void onConnectionEvent(Connection c, String event, Bundle extras) {
            sendConnectionEvent(event, extras);
        }
    };

    /**
     * The telephony connection service; used to add new participant connections to Telecom.
     */
    private TelephonyConnectionServiceProxy mTelephonyConnectionService;

    /**
     * The connection to the conference server which is hosting the conference.
     */
    private TelephonyConnection mConferenceHost;

    /**
     * The PhoneAccountHandle of the conference host.
     */
    private PhoneAccountHandle mConferenceHostPhoneAccountHandle;

    /**
     * The address of the conference host.
     */
    private Uri[] mConferenceHostAddress;

    private TelecomAccountRegistry mTelecomAccountRegistry;

    /**
     * The known conference participant connections.  The HashMap is keyed by a Pair containing
     * the handle and endpoint Uris.
     * Access to the hashmap is protected by the {@link #mUpdateSyncRoot}.
     */
    private final HashMap<Pair<Uri, Uri>, ConferenceParticipantConnection>
            mConferenceParticipantConnections = new HashMap<>();

    /**
     * Sychronization root used to ensure that updates to the
     * {@link #mConferenceParticipantConnections} happen atomically are are not interleaved across
     * threads.  There are some instances where the network will send conference event package
     * data closely spaced.  If that happens, it is possible that the interleaving of the update
     * will cause duplicate participant info to be added.
     */
    private final Object mUpdateSyncRoot = new Object();

    private boolean mIsHoldable;
    private boolean mCouldManageConference;
    private FeatureFlagProxy mFeatureFlagProxy;
    private boolean mIsEmulatingSinglePartyCall = false;
    private boolean mIsUsingSimCallManager = false;

    /**
     * Where {@link #mIsEmulatingSinglePartyCall} is {@code true}, contains the
     * {@link ConferenceParticipantConnection#getUserEntity()} and
     * {@link ConferenceParticipantConnection#getEndpoint()} of the single participant which this
     * conference pretends to be.
     */
    private Pair<Uri, Uri> mLoneParticipantIdentity = null;

    /**
     * The {@link ConferenceParticipantConnection#getUserEntity()} and
     * {@link ConferenceParticipantConnection#getEndpoint()} of the conference host as they appear
     * in the CEP.  This is determined when we scan the first conference event package.
     * It is possible that this will be {@code null} for carriers which do not include the host
     * in the CEP.
     */
    private Pair<Uri, Uri> mHostParticipantIdentity = null;

    /**
     * M: Max. numbers of participants in a IMS conference.
     */
    public static final int IMS_CONFERENCE_MAX_SIZE = 5;
    /// M: ALPS02487069. Keep the state of conference host. @{
    private int mHostCallState = Connection.STATE_NEW;
    /// @}
    /// M: For VoLTE enhanced conference call. @{
    private boolean mIsDuringAddingParticipants = false;
    /// @}
    /// M: For multi anchor feature @{
    private final List<Conferenceable> mConferenceables = new ArrayList<>();
    private final List<Conferenceable> mUnmodifiableConferenceables =
            Collections.unmodifiableList(mConferenceables);

    private final Conference.Listener mConferenceDeathListener = new Conference.Listener() {
        @Override
        public void onDestroyed(Conference c) {
            if (mConferenceables.remove(c)) {
                fireOnConferenceableConnectionsChanged();
            }
        }
    };
    /// @}

    public void updateConferenceParticipantsAfterCreation() {
        if (mConferenceHost != null) {
            Log.d(this, "updateConferenceStateAfterCreation :: process participant update");
            handleConferenceParticipantsUpdate(mConferenceHost,
                    mConferenceHost.getConferenceParticipants());
        } else {
            Log.d(this, "updateConferenceStateAfterCreation :: null mConferenceHost");
        }
    }

    /**
     * Initializes a new {@link ImsConference}.
     *  @param telephonyConnectionService The connection service responsible for adding new
     *                                   conferene participants.
     * @param conferenceHost The telephony connection hosting the conference.
     * @param phoneAccountHandle The phone account handle associated with the conference.
     * @param featureFlagProxy
     */
    public ImsConference(TelecomAccountRegistry telecomAccountRegistry,
            TelephonyConnectionServiceProxy telephonyConnectionService,
            TelephonyConnection conferenceHost, PhoneAccountHandle phoneAccountHandle,
            FeatureFlagProxy featureFlagProxy) {

        super(phoneAccountHandle);

        mTelecomAccountRegistry = telecomAccountRegistry;
        mFeatureFlagProxy = featureFlagProxy;

        // Specify the connection time of the conference to be the connection time of the original
        // connection.
        /// M: ALPS03525429. The original connection might release, should do null check before
        /// access. @{
        //long connectTime = conferenceHost.getOriginalConnection().getConnectTime();
        //long connectElapsedTime = conferenceHost.getOriginalConnection().getConnectTimeReal();
        long connectTime = 0;
        long connectElapsedTime = 0;
        if (conferenceHost.getOriginalConnection() != null) {
            connectTime = conferenceHost.getOriginalConnection().getConnectTime();
            connectElapsedTime = conferenceHost.getOriginalConnection().getConnectTimeReal();
        }
        /// @}
        setConnectionTime(connectTime);
        setConnectionStartElapsedRealTime(connectElapsedTime);
        // Set the connectTime in the connection as well.
        conferenceHost.setConnectTimeMillis(connectTime);
        conferenceHost.setConnectionStartElapsedRealTime(connectElapsedTime);

        mTelephonyConnectionService = telephonyConnectionService;
        setConferenceHost(conferenceHost);

        int capabilities = Connection.CAPABILITY_MUTE |
                Connection.CAPABILITY_CONFERENCE_HAS_NO_CHILDREN;
        if (canHoldImsCalls()) {
            capabilities |= Connection.CAPABILITY_SUPPORT_HOLD | Connection.CAPABILITY_HOLD;
            mIsHoldable = true;
        }
        /// M: For VoLTE conference inviteParticipant. @{
        /// Enable CAPABILITY_INVITE_PARTICIPANTS by checking if current phone
        /// has VOLTE_ENHANCED_CONFERENCE feature.
        if (conferenceHost != null && conferenceHost.getOriginalConnection() != null
                && conferenceHost.getOriginalConnection().getCall().getPhone() != null) {
            Phone phone = conferenceHost.getOriginalConnection().getCall().getPhone();
            if (phone instanceof MtkImsPhone) {
                MtkImsPhone imsPhone = (MtkImsPhone)phone;
                if (imsPhone.isFeatureSupported(MtkImsPhone.FeatureType.VOLTE_ENHANCED_CONFERENCE)){
                    capabilities |= MtkConnection.CAPABILITY_INVITE_PARTICIPANTS;
                    // ALPS04234226. For GCF case 15.19
                    if (isTestSim()) {
                        capabilities |= Connection.CAPABILITY_MANAGE_CONFERENCE;
                    }
                }
            }
        }
        /// @}
        capabilities = applyHostCapabilities(capabilities,
                mConferenceHost.getConnectionCapabilities(),
                mConferenceHost.isCarrierVideoConferencingSupported());
        setConnectionCapabilities(capabilities);

        /// M: Set the properties. @{
        int properties = MtkConnection.PROPERTY_VOLTE;
        properties = applyHostProperties(properties,
                mConferenceHost.getConnectionProperties());
        Log.d(this, "MtkImsConference: properties=" + properties);
        setConnectionProperties(properties);
        /// @}
    }

    /**
     * Transfers capabilities from the conference host to the conference itself.
     *
     * @param conferenceCapabilities The current conference capabilities.
     * @param capabilities The new conference host capabilities.
     * @param isVideoConferencingSupported Whether video conferencing is supported.
     * @return The merged capabilities to be applied to the conference.
     */
    private int applyHostCapabilities(int conferenceCapabilities, int capabilities,
            boolean isVideoConferencingSupported) {

        conferenceCapabilities = changeBitmask(conferenceCapabilities,
                    Connection.CAPABILITY_SUPPORTS_VT_LOCAL_BIDIRECTIONAL,
                    can(capabilities, Connection.CAPABILITY_SUPPORTS_VT_LOCAL_BIDIRECTIONAL));

        if (isVideoConferencingSupported) {
            conferenceCapabilities = changeBitmask(conferenceCapabilities,
                    Connection.CAPABILITY_SUPPORTS_VT_REMOTE_BIDIRECTIONAL,
                    can(capabilities, Connection.CAPABILITY_SUPPORTS_VT_REMOTE_BIDIRECTIONAL));
            conferenceCapabilities = changeBitmask(conferenceCapabilities,
                    Connection.CAPABILITY_CAN_UPGRADE_TO_VIDEO,
                    can(capabilities, Connection.CAPABILITY_CAN_UPGRADE_TO_VIDEO));
        } else {
            // If video conferencing is not supported, explicitly turn off the remote video
            // capability and the ability to upgrade to video.
            Log.d(this, "applyHostCapabilities : video conferencing not supported");
            conferenceCapabilities = changeBitmask(conferenceCapabilities,
                    Connection.CAPABILITY_SUPPORTS_VT_REMOTE_BIDIRECTIONAL, false);
            conferenceCapabilities = changeBitmask(conferenceCapabilities,
                    Connection.CAPABILITY_CAN_UPGRADE_TO_VIDEO, false);
        }

        conferenceCapabilities = changeBitmask(conferenceCapabilities,
                Connection.CAPABILITY_CANNOT_DOWNGRADE_VIDEO_TO_AUDIO,
                can(capabilities, Connection.CAPABILITY_CANNOT_DOWNGRADE_VIDEO_TO_AUDIO));

        conferenceCapabilities = changeBitmask(conferenceCapabilities,
                Connection.CAPABILITY_CAN_PAUSE_VIDEO,
                mConferenceHost.getVideoPauseSupported() && isVideoCapable());
        /// M: apply hold capability. @{
        conferenceCapabilities = changeBitmask(conferenceCapabilities,
                    Connection.CAPABILITY_HOLD,
                    can(capabilities, Connection.CAPABILITY_HOLD));
        /// @}

        return conferenceCapabilities;
    }

    /**
     * Transfers properties from the conference host to the conference itself.
     *
     * @param conferenceProperties The current conference properties.
     * @param properties The new conference host properties.
     * @return The merged properties to be applied to the conference.
     */
    private int applyHostProperties(int conferenceProperties, int properties) {
        conferenceProperties = changeBitmask(conferenceProperties,
                Connection.PROPERTY_HIGH_DEF_AUDIO,
                can(properties, Connection.PROPERTY_HIGH_DEF_AUDIO));

        conferenceProperties = changeBitmask(conferenceProperties,
                Connection.PROPERTY_WIFI,
                can(properties, Connection.PROPERTY_WIFI));

        conferenceProperties = changeBitmask(conferenceProperties,
                Connection.PROPERTY_IS_EXTERNAL_CALL,
                can(properties, Connection.PROPERTY_IS_EXTERNAL_CALL));

        /// M: ALPS04793513, use isConferenceRemoteHosted() instead of !isConferenceHost().
        conferenceProperties = changeBitmask(conferenceProperties,
                Connection.PROPERTY_REMOTELY_HOSTED, isConferenceRemoteHosted());

        /// M: For VoLTE. @{
        conferenceProperties = changeBitmask(conferenceProperties,
                MtkConnection.PROPERTY_VOLTE,
                can(properties, MtkConnection.PROPERTY_VOLTE));
        /// @}

        return conferenceProperties;
    }

    /**
     * Not used by the IMS conference controller.
     *
     * @return {@code Null}.
     */
    @Override
    public android.telecom.Connection getPrimaryConnection() {
        return null;
    }

    /**
     * Returns VideoProvider of the conference. This can be null.
     *
     * @hide
     */
    @Override
    public VideoProvider getVideoProvider() {
        if (mConferenceHost != null) {
            return mConferenceHost.getVideoProvider();
        }
        return null;
    }

    /**
     * Returns video state of conference
     *
     * @hide
     */
    @Override
    public int getVideoState() {
        if (mConferenceHost != null) {
            return mConferenceHost.getVideoState();
        }
        return VideoProfile.STATE_AUDIO_ONLY;
    }

    /**
     * Invoked when the Conference and all its {@link Connection}s should be disconnected.
     * <p>
     * Hangs up the call via the conference host connection.  When the host connection has been
     * successfully disconnected, the {@link #mConferenceHostListener} listener receives an
     * {@code onDestroyed} event, which triggers the conference participant connections to be
     * disconnected.
     */
    @Override
    public void onDisconnect() {
        Log.d(this, "onDisconnect: hanging up conference host.");
        if (mConferenceHost == null) {
            return;
        }

        /// M: can not disconnect participants without remove connection.
        /// Will disconnect particpants when STATE_DISCONNECTED.
        // disconnectConferenceParticipants();

        Call call = mConferenceHost.getCall();
        if (call != null) {
            try {
                call.hangup();
            } catch (CallStateException e) {
                Log.e(this, e, "Exception thrown trying to hangup conference");
            }
        }
    }

    /**
     * Invoked when the specified {@link android.telecom.Connection} should be separated from the
     * conference call.
     * <p>
     * IMS does not support separating connections from the conference.
     *
     * @param connection The connection to separate.
     */
    @Override
    public void onSeparate(android.telecom.Connection connection) {
        Log.wtf(this, "Cannot separate connections from an IMS conference.");
    }

    /**
     * Invoked when the specified {@link android.telecom.Connection} should be merged into the
     * conference call.
     *
     * @param connection The {@code Connection} to merge.
     */
    @Override
    public void onMerge(android.telecom.Connection connection) {
        /// M: For VoLTE enhanced conference call. @{
        Log.d(this, "onMerge()");
        if (mConferenceHost == null) {
            Log.w(this, "mConferenceHost is null!");
            return;
        }
        if (mIsDuringAddingParticipants) {
            toastWhenIsAddingParticipants();
            return;
        }
        /// @}
        /// M: ALPS04057265, Show toast if the conference is full @{
        if (getNumberOfParticipants() >= getMaximumConferenceSize()) {
            Log.d(this, "Conference is full!");
            if (mConferenceHost.getPhone() != null && isShowToastWhenConferenceFull()) {
                toastWhenConferenceIsFull(mConferenceHost.getPhone().getContext());
            }
            if (ignoreAddRequestToFullConference()) {
                Log.d(this, "Ignore the add request");
                return;
            }
        }
        /// @}

        try {
            Phone phone = mConferenceHost.getPhone();
            if (phone != null) {
                phone.conference();
            }
        } catch (CallStateException e) {
            Log.e(this, e, "Exception thrown trying to merge call into a conference");
        }
    }

    /**
     * Invoked when the specified {@link android.telecom.Connection} should be merged into the
     * conference call.
     *
     * @param connection The {@code Connection} to merge.
     */
    @Override
    public void onMerge() {
        /// M: For VoLTE enhanced conference call. @{
        Log.v(this, "onMerge()");
        if (mConferenceHost == null) {
            Log.w(this, "mConferenceHost is null!");
            return;
        }
        if (mIsDuringAddingParticipants) {
            toastWhenIsAddingParticipants();
            return;
        }
        /// @}
        /// M: ALPS04057265, Show toast if the conference is full @{
        if (getNumberOfParticipants() >= getMaximumConferenceSize()) {
            Log.v(this, "Conference is full!");
            if (mConferenceHost.getPhone() != null && isShowToastWhenConferenceFull()) {
                toastWhenConferenceIsFull(mConferenceHost.getPhone().getContext());
            }
            if (ignoreAddRequestToFullConference()) {
                Log.v(this, "Ignore the add request");
                return;
            }
        }
        /// @}

        try {
            Phone phone = mConferenceHost.getPhone();
            if (phone != null) {
                phone.conference();
            }
        } catch (CallStateException e) {
            Log.e(this, e, "Exception thrown trying to merge call into a conference");
        }
    }

    /**
     * Invoked when the conference should be put on hold.
     */
    @Override
    public void onHold() {
        /// M: For VoLTE enhanced conference call. @{
        if (mIsDuringAddingParticipants) {
            toastWhenIsAddingParticipants();
            return;
        }
        /// @}

        if (mConferenceHost == null) {
            return;
        }
        mConferenceHost.performHold();
    }

    /**
     * Invoked when the conference should be moved from hold to active.
     */
    @Override
    public void onUnhold() {
        /// M: For VoLTE enhanced conference call. @{
        if (mIsDuringAddingParticipants) {
            toastWhenIsAddingParticipants();
            return;
        }
        /// @}

        if (mConferenceHost == null) {
            return;
        }
        mConferenceHost.performUnhold();
    }

    /**
     * Invoked to play a DTMF tone.
     *
     * @param c A DTMF character.
     */
    @Override
    public void onPlayDtmfTone(char c) {
        if (mConferenceHost == null) {
            return;
        }
        mConferenceHost.onPlayDtmfTone(c);
    }

    /**
     * Invoked to stop playing a DTMF tone.
     */
    @Override
    public void onStopDtmfTone() {
        if (mConferenceHost == null) {
            return;
        }
        mConferenceHost.onStopDtmfTone();
    }

    /**
     * Handles the addition of connections to the {@link ImsConference}.  The
     * {@link ImsConferenceController} does not add connections to the conference.
     *
     * @param connection The newly added connection.
     */
    @Override
    public void onConnectionAdded(android.telecom.Connection connection) {
        // No-op
        Log.d(this, "connection added: " + connection
                + ", time: " + connection.getConnectTimeMillis());
    }

    @Override
    public void setHoldable(boolean isHoldable) {
        mIsHoldable = isHoldable;
        if (!mIsHoldable) {
            removeCapability(Connection.CAPABILITY_HOLD);
        } else {
            /// M: For VoLTE enhanced conference call.
            /// Should set hold capability only when active or holding.
            if (getState() == Connection.STATE_ACTIVE || getState() == Connection.STATE_HOLDING) {
                addCapability(Connection.CAPABILITY_HOLD);
            }
        }
        /// M: setHoldable to Conference Host Connection to keep consistency.
        if (mConferenceHost != null) {
            mConferenceHost.setHoldable(mIsHoldable);
        }
    }

    @Override
    public boolean isChildHoldable() {
        // The conference should not be a child of other conference.
        return false;
    }

    /**
     * Changes a bit-mask to add or remove a bit-field.
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

    /**
     * Determines if this conference is hosted on the current device or the peer device.
     *
     * @return {@code true} if this conference is hosted on the current device, {@code false} if it
     *      is hosted on the peer device.
     */
    public boolean isConferenceHost() {
        if (mConferenceHost == null) {
            return false;
        }
        com.android.internal.telephony.Connection originalConnection =
                mConferenceHost.getOriginalConnection();

        return originalConnection != null && originalConnection.isMultiparty() &&
                originalConnection.isConferenceHost();
    }

    private boolean isConferenceRemoteHosted() {
        if (mConferenceHost == null) {
            return true;
        }
        com.android.internal.telephony.Connection originalConnection =
                mConferenceHost.getOriginalConnection();

        return originalConnection != null && originalConnection.isMultiparty() &&
                !originalConnection.isConferenceHost();
    }

    /**
     * Updates the manage conference capability of the conference.
     *
     * The following cases are handled:
     * <ul>
     *     <li>There is only a single participant in the conference -- manage conference is
     *     disabled.</li>
     *     <li>There is more than one participant in the conference -- manage conference is
     *     enabled.</li>
     *     <li>No conference event package data is available -- manage conference is disabled.</li>
     * </ul>
     * <p>
     * Note: We add and remove {@link Connection#CAPABILITY_CONFERENCE_HAS_NO_CHILDREN} to ensure
     * that the conference is represented appropriately on Bluetooth devices.
     */
    private void updateManageConference() {
        boolean couldManageConference = can(Connection.CAPABILITY_MANAGE_CONFERENCE);
        boolean canManageConference = mFeatureFlagProxy.isUsingSinglePartyCallEmulation()
                && mIsEmulatingSinglePartyCall
                ? mConferenceParticipantConnections.size() > 1
                : mConferenceParticipantConnections.size() != 0;
        Log.d(this, "updateManageConference was :%s is:%s", couldManageConference ? "Y" : "N",
                canManageConference ? "Y" : "N");

        if (couldManageConference != canManageConference) {
            int capabilities = getConnectionCapabilities();

            if (canManageConference) {
                capabilities |= Connection.CAPABILITY_MANAGE_CONFERENCE;
                capabilities &= ~Connection.CAPABILITY_CONFERENCE_HAS_NO_CHILDREN;
            } else {
                capabilities &= ~Connection.CAPABILITY_MANAGE_CONFERENCE;
                capabilities |= Connection.CAPABILITY_CONFERENCE_HAS_NO_CHILDREN;
            }

            setConnectionCapabilities(capabilities);
        }
    }

    /**
     * Sets the connection hosting the conference and registers for callbacks.
     *
     * @param conferenceHost The connection hosting the conference.
     */
    private void setConferenceHost(TelephonyConnection conferenceHost) {
        if (Log.DEBUG) {
            Log.d(this, "setConferenceHost " + conferenceHost);
        }

        mConferenceHost = conferenceHost;

        // Attempt to get the conference host's address (e.g. the host's own phone number).
        // We need to look at the default phone for the ImsPhone when creating the phone account
        // for the
        if (mConferenceHost.getPhone() != null &&
                mConferenceHost.getPhone().getPhoneType() == PhoneConstants.PHONE_TYPE_IMS) {
            // Look up the conference host's address; we need this later for filtering out the
            // conference host in conference event package data.
            Phone imsPhone = mConferenceHost.getPhone();
            mConferenceHostPhoneAccountHandle =
                    PhoneUtils.makePstnPhoneAccountHandle(imsPhone.getDefaultPhone());
            /// M: The host address from TelecomAccountRegistry is using the API getLine1Number,
            /// getLine1Number is not stable, might get the wrong number.
            //Uri hostAddress = mTelecomAccountRegistry.getAddress(mConferenceHostPhoneAccountHandle);

            ArrayList<Uri> hostAddresses = new ArrayList<>();

            // add address from TelecomAccountRegistry
            //if (hostAddress != null) {
            //    hostAddresses.add(hostAddress);
            //}

            // add addresses from phone
            if (imsPhone.getCurrentSubscriberUris() != null) {
                hostAddresses.addAll(
                        new ArrayList<>(Arrays.asList(imsPhone.getCurrentSubscriberUris())));
            }

            mConferenceHostAddress = new Uri[hostAddresses.size()];
            mConferenceHostAddress = hostAddresses.toArray(mConferenceHostAddress);
            for (Uri host : mConferenceHostAddress) {
                Log.d(this, "hostAddress: " + Log.pii(host));
            }

            mIsUsingSimCallManager = mTelecomAccountRegistry.isUsingSimCallManager(
                    mConferenceHostPhoneAccountHandle);
        }

        // If the conference is not hosted on this device copy over the address and presentation and
        // connect times so that we can log this appropriately in the call log.
        if (!isConferenceHost()) {
            setAddress(mConferenceHost.getAddress(), mConferenceHost.getAddressPresentation());
            setCallerDisplayName(mConferenceHost.getCallerDisplayName(),
                    mConferenceHost.getCallerDisplayNamePresentation());
            setConnectionStartElapsedRealTime(mConferenceHost.getConnectElapsedTimeMillis());
            setConnectionTime(mConferenceHost.getConnectTimeMillis());
        }

        mConferenceHost.addConnectionListener(mConferenceHostListener);
        mConferenceHost.addTelephonyConnectionListener(mTelephonyConnectionListener);
        setConnectionCapabilities(applyHostCapabilities(getConnectionCapabilities(),
                mConferenceHost.getConnectionCapabilities(),
                mConferenceHost.isCarrierVideoConferencingSupported()));
        setConnectionProperties(applyHostProperties(getConnectionProperties(),
                mConferenceHost.getConnectionProperties()));

        setState(mConferenceHost.getState());
        updateStatusHints();
        putExtras(mConferenceHost.getExtras());
    }

    /**
     * Handles state changes for conference participant(s).  The participants data passed in
     *
     * @param parent The connection which was notified of the conference participant.
     * @param participants The conference participant information.
     */
    @VisibleForTesting
    public void handleConferenceParticipantsUpdate(
            TelephonyConnection parent, List<ConferenceParticipant> participants) {

        if (participants == null) {
            return;
        }

        if (parent != null && !parent.isManageImsConferenceCallSupported()) {
            Log.i(this, "handleConferenceParticipantsUpdate: manage conference is disallowed");
            return;
        }

        Log.i(this, "handleConferenceParticipantsUpdate: size=%d", participants.size());

        // Perform the update in a synchronized manner.  It is possible for the IMS framework to
        // trigger two onConferenceParticipantsChanged callbacks in quick succession.  If the first
        // update adds new participants, and the second does something like update the status of one
        // of the participants, we can get into a situation where the participant is added twice.
        synchronized (mUpdateSyncRoot) {
            int oldParticipantCount = mConferenceParticipantConnections.size();
            boolean newParticipantsAdded = false;
            boolean oldParticipantsRemoved = false;
            ArrayList<ConferenceParticipant> newParticipants = new ArrayList<>(participants.size());
            HashSet<Pair<Uri,Uri>> participantUserEntities = new HashSet<>(participants.size());

            // Determine if the conference event package represents a single party conference.
            // A single party conference is one where there is no other participant other than the
            // conference host and one other participant.
            // Note: We consider 0 to still be a single party conference since some carriers will
            // send a conference event package with JUST the host in it when the conference is
            // disconnected.  We don't want to change back to conference mode prior to disconnection
            // or we will not log the call.
            boolean isSinglePartyConference = participants.stream()
                    .filter(p -> {
                        Pair<Uri, Uri> pIdent = new Pair<>(p.getHandle(), p.getEndpoint());
                        return !Objects.equals(mHostParticipantIdentity, pIdent);
                    })
                    .count() <= 1;

            // We will only process the CEP data if:
            // 1. We're not emulating a single party call.
            // 2. We're emulating a single party call and the CEP contains more than just the
            //    single party
            if ((mIsEmulatingSinglePartyCall && !isSinglePartyConference) ||
                !mIsEmulatingSinglePartyCall) {
                // Add any new participants and update existing.
                for (ConferenceParticipant participant : participants) {
                    Pair<Uri, Uri> userEntity = new Pair<>(participant.getHandle(),
                            participant.getEndpoint());

                    participantUserEntities.add(userEntity);
                    if (!mConferenceParticipantConnections.containsKey(userEntity)) {
                        // Some carriers will also include the conference host in the CEP.  We will
                        // filter that out here.
                        if (!isParticipantHost(mConferenceHostAddress, participant.getHandle())) {
                            createConferenceParticipantConnection(parent, participant);
                            newParticipants.add(participant);
                            newParticipantsAdded = true;
                        } else {
                            // Track the identity of the conference host; its useful to know when
                            // we look at the CEP in the future.
                            mHostParticipantIdentity = userEntity;
                        }
                    } else {
                        ConferenceParticipantConnection connection =
                                mConferenceParticipantConnections.get(userEntity);
                        Log.i(this,
                                "handleConferenceParticipantsUpdate: updateState, participant = %s",
                                participant);
                        connection.updateState(participant.getState());
                        connection.setVideoState(parent.getVideoState());
                    }
                }

                // Set state of new participants.
                if (newParticipantsAdded) {
                    // Set the state of the new participants at once and add to the conference
                    for (ConferenceParticipant newParticipant : newParticipants) {
                        ConferenceParticipantConnection connection =
                                mConferenceParticipantConnections.get(new Pair<>(
                                        newParticipant.getHandle(),
                                        newParticipant.getEndpoint()));
                        connection.updateState(newParticipant.getState());
                        connection.setVideoState(parent.getVideoState());
                    }
                }

                // Finally, remove any participants from the conference that no longer exist in the
                // conference event package data.
                Iterator<Map.Entry<Pair<Uri, Uri>, ConferenceParticipantConnection>> entryIterator =
                        mConferenceParticipantConnections.entrySet().iterator();
                while (entryIterator.hasNext()) {
                    Map.Entry<Pair<Uri, Uri>, ConferenceParticipantConnection> entry =
                            entryIterator.next();

                    if (!participantUserEntities.contains(entry.getKey())) {
                        ConferenceParticipantConnection participant = entry.getValue();
                        participant.setDisconnected(new DisconnectCause(DisconnectCause.CANCELED));
                        participant.removeConnectionListener(mParticipantListener);
                        mTelephonyConnectionService.removeConnection(participant);
                        removeConnection(participant);
                        entryIterator.remove();
                        oldParticipantsRemoved = true;
                    }
                }
            }

            int newParticipantCount = mConferenceParticipantConnections.size();
            Log.d(this, "handleConferenceParticipantsUpdate: oldParticipantCount=%d, "
                            + "newParticipantcount=%d", oldParticipantCount, newParticipantCount);
            // If the single party call emulation fature flag is enabled, we can potentially treat
            // the conference as a single party call when there is just one participant.
            if (mFeatureFlagProxy.isUsingSinglePartyCallEmulation()) {
                if (oldParticipantCount > 1 && newParticipantCount == 1) {
                    // If number of participants goes to 1, emulate a single party call.
                    startEmulatingSinglePartyCall();
                } else if (mIsEmulatingSinglePartyCall && !isSinglePartyConference) {
                    // Number of participants increased, so stop emulating a single party call.
                    stopEmulatingSinglePartyCall();
                }
            }

            // If new participants were added or old ones were removed, we need to ensure the state
            // of the manage conference capability is updated.
            if (newParticipantsAdded || oldParticipantsRemoved) {
                updateManageConference();
            }
        }
    }

    /**
     * Called after {@link #startEmulatingSinglePartyCall()} to cause the conference to appear as
     * if it is a conference again.
     * 1. Tell telecom we're a conference again.
     * 2. Restore {@link Connection#CAPABILITY_MANAGE_CONFERENCE} capability.
     * 3. Null out the name/address.
     *
     * Note: Single party call emulation is disabled if the conference is taking place via a
     * sim call manager.  Emulating a single party call requires properties of the conference to be
     * changed (connect time, address, conference state) which cannot be guaranteed to be relayed
     * correctly by the sim call manager to Telecom.
     */
    private void stopEmulatingSinglePartyCall() {
        if (mIsUsingSimCallManager) {
            Log.i(this, "stopEmulatingSinglePartyCall: using sim call manager; skip.");
            return;
        }

        Log.i(this, "stopEmulatingSinglePartyCall: conference now has more than one"
                + " participant; make it look conference-like again.");
        mIsEmulatingSinglePartyCall = false;

        if (mCouldManageConference) {
            int currentCapabilities = getConnectionCapabilities();
            currentCapabilities |= Connection.CAPABILITY_MANAGE_CONFERENCE;
            setConnectionCapabilities(currentCapabilities);
        }

        // Null out the address/name so it doesn't look like a single party call
        setAddress(null, TelecomManager.PRESENTATION_UNKNOWN);
        setCallerDisplayName(null, TelecomManager.PRESENTATION_UNKNOWN);

        // Copy the conference connect time back to the previous lone participant.
        ConferenceParticipantConnection loneParticipant =
                mConferenceParticipantConnections.get(mLoneParticipantIdentity);
        if (loneParticipant != null) {
            Log.d(this,
                    "stopEmulatingSinglePartyCall: restored lone participant connect time");
            loneParticipant.setConnectTimeMillis(getConnectionTime());
            loneParticipant.setConnectionStartElapsedRealTime(getConnectionStartElapsedRealTime());
        }

        // Tell Telecom its a conference again.
        setConferenceState(true);
    }

    /**
     * Called when a conference drops to a single participant. Causes this conference to present
     * itself to Telecom as if it was a single party call.
     * 1. Remove the participant from Telecom and from local tracking; when we get a new CEP in
     *    the future we'll just re-add the participant anyways.
     * 2. Tell telecom we're not a conference.
     * 3. Remove {@link Connection#CAPABILITY_MANAGE_CONFERENCE} capability.
     * 4. Set the name/address to that of the single participant.
     *
     * Note: Single party call emulation is disabled if the conference is taking place via a
     * sim call manager.  Emulating a single party call requires properties of the conference to be
     * changed (connect time, address, conference state) which cannot be guaranteed to be relayed
     * correctly by the sim call manager to Telecom.
     */
    private void startEmulatingSinglePartyCall() {
        if (mIsUsingSimCallManager) {
            Log.i(this, "startEmulatingSinglePartyCall: using sim call manager; skip.");
            return;
        }

        Log.i(this, "startEmulatingSinglePartyCall: conference has a single "
                + "participant; downgrade to single party call.");

        mIsEmulatingSinglePartyCall = true;
        Iterator<ConferenceParticipantConnection> valueIterator =
                mConferenceParticipantConnections.values().iterator();
        if (valueIterator.hasNext()) {
            ConferenceParticipantConnection entry = valueIterator.next();

            // Set the conference name/number to that of the remaining participant.
            setAddress(entry.getAddress(), entry.getAddressPresentation());
            setCallerDisplayName(entry.getCallerDisplayName(),
                    entry.getCallerDisplayNamePresentation());
            setConnectionStartElapsedRealTime(entry.getConnectElapsedTimeMillis());
            setConnectionTime(entry.getConnectTimeMillis());
            mLoneParticipantIdentity = new Pair<>(entry.getUserEntity(), entry.getEndpoint());

            // Remove the participant from Telecom.  It'll get picked up in a future CEP update
            // again anyways.
            entry.setDisconnected(new DisconnectCause(DisconnectCause.CANCELED,
                    DisconnectCause.REASON_EMULATING_SINGLE_CALL));
            entry.removeConnectionListener(mParticipantListener);
            mTelephonyConnectionService.removeConnection(entry);
            removeConnection(entry);
            valueIterator.remove();
        }

        // Have Telecom pretend its not a conference.
        setConferenceState(false);

        // Remove manage conference capability.
        mCouldManageConference = can(Connection.CAPABILITY_MANAGE_CONFERENCE);
        int currentCapabilities = getConnectionCapabilities();
        currentCapabilities &= ~Connection.CAPABILITY_MANAGE_CONFERENCE;
        setConnectionCapabilities(currentCapabilities);
    }

    /**
     * Creates a new {@link ConferenceParticipantConnection} to represent a
     * {@link ConferenceParticipant}.
     * <p>
     * The new connection is added to the conference controller and connection service.
     *
     * @param parent The connection which was notified of the participant change (e.g. the
     *                         parent connection).
     * @param participant The conference participant information.
     */
    private void createConferenceParticipantConnection(
            TelephonyConnection parent, ConferenceParticipant participant) {

        // Create and add the new connection in holding state so that it does not become the
        // active call.
        ConferenceParticipantConnection connection = new ConferenceParticipantConnection(
                parent.getOriginalConnection(), participant,
                !isConferenceHost() /* isRemotelyHosted */);
        connection.addConnectionListener(mParticipantListener);
        if (participant.getConnectTime() == 0) {
            connection.setConnectTimeMillis(parent.getConnectTimeMillis());
            connection.setConnectionStartElapsedRealTime(parent.getConnectElapsedTimeMillis());
        } else {
            connection.setConnectTimeMillis(participant.getConnectTime());
            connection.setConnectionStartElapsedRealTime(participant.getConnectElapsedTime());
        }
        // Indicate whether this is an MT or MO call to Telecom; the participant has the cached
        // data from the time of merge.
        connection.setCallDirection(participant.getCallDirection());

        Log.i(this, "createConferenceParticipantConnection: participant=%s, connection=%s",
                participant, connection);

        synchronized(mUpdateSyncRoot) {
            mConferenceParticipantConnections.put(new Pair<>(participant.getHandle(),
                    participant.getEndpoint()), connection);
        }

        mTelephonyConnectionService.addExistingConnection(mConferenceHostPhoneAccountHandle,
                connection, this);
        addConnection(connection);
    }

    /**
     * Removes a conference participant from the conference.
     *
     * @param participant The participant to remove.
     */
    private void removeConferenceParticipant(ConferenceParticipantConnection participant) {
        Log.i(this, "removeConferenceParticipant: %s", participant);

        participant.removeConnectionListener(mParticipantListener);
        synchronized(mUpdateSyncRoot) {
            mConferenceParticipantConnections.remove(new Pair<>(participant.getUserEntity(),
                    participant.getEndpoint()));
        }
        mTelephonyConnectionService.removeConnection(participant);
    }

    /**
     * Disconnects all conference participants from the conference.
     */
    private void disconnectConferenceParticipants() {
        Log.d(this, "disconnectConferenceParticipants");

        synchronized(mUpdateSyncRoot) {
            for (ConferenceParticipantConnection connection :
                    mConferenceParticipantConnections.values()) {

                connection.removeConnectionListener(mParticipantListener);
                // Mark disconnect cause as cancelled to ensure that the call is not logged in the
                // call log.
                connection.setDisconnected(new DisconnectCause(DisconnectCause.CANCELED));
                mTelephonyConnectionService.removeConnection(connection);
                connection.destroy();
            }
            mConferenceParticipantConnections.clear();
        }
    }

    /**
     * Determines if the passed in participant handle is the same as the conference host's handle.
     * Starts with a simple equality check.  However, the handles from a conference event package
     * will be a SIP uri, so we need to pull that apart to look for the participant's phone number.
     *
     * @param hostHandles The handle(s) of the connection hosting the conference.
     * @param handle The handle of the conference participant.
     * @return {@code true} if the host's handle matches the participant's handle, {@code false}
     *      otherwise.
     */
    private boolean isParticipantHost(Uri[] hostHandles, Uri handle) {
        // If there is no host handle or no participant handle, bail early.
        if (hostHandles == null || hostHandles.length == 0 || handle == null) {
            Log.d(this, "isParticipantHost(N) : host or participant uri null");
            return false;
        }

        // Conference event package participants are identified using SIP URIs (see RFC3261).
        // A valid SIP uri has the format: sip:user:password@host:port;uri-parameters?headers
        // Per RFC3261, the "user" can be a telephone number.
        // For example: sip:1650555121;phone-context=blah.com@host.com
        // In this case, the phone number is in the user field of the URI, and the parameters can be
        // ignored.
        //
        // A SIP URI can also specify a phone number in a format similar to:
        // sip:+1-212-555-1212@something.com;user=phone
        // In this case, the phone number is again in user field and the parameters can be ignored.
        // We can get the user field in these instances by splitting the string on the @, ;, or :
        // and looking at the first found item.

        String number = handle.getSchemeSpecificPart();
        String numberParts[] = number.split("[@;:]");

        if (numberParts.length == 0) {
            Log.d(this, "isParticipantHost(N) : no number in participant handle");
            return false;
        }
        number = numberParts[0];

        for (Uri hostHandle : hostHandles) {
            if (hostHandle == null) {
                continue;
            }
            // The host number will be a tel: uri.  Per RFC3966, the part after tel: is the phone
            // number.
            String hostNumber = hostHandle.getSchemeSpecificPart();

            // Use a loose comparison of the phone numbers.  This ensures that numbers that differ
            // by special characters are counted as equal.
            // E.g. +16505551212 would be the same as 16505551212
            boolean isHost = PhoneNumberUtils.compare(hostNumber, number);

            Log.d(this, "isParticipantHost(%s) : host: %s, participant %s", (isHost ? "Y" : "N"),
                    Log.pii(hostNumber), Log.pii(number));

            if (isHost) {
                return true;
            }
        }
        return false;
    }

    /**
     * Handles a change in the original connection backing the conference host connection.  This can
     * happen if an SRVCC event occurs on the original IMS connection, requiring a fallback to
     * GSM or CDMA.
     * <p>
     * If this happens, we will add the conference host connection to telecom and tear down the
     * conference.
     */
    private void handleOriginalConnectionChange() {
        if (mConferenceHost == null) {
            Log.w(this, "handleOriginalConnectionChange; conference host missing.");
            return;
        }

        com.android.internal.telephony.Connection originalConnection =
                mConferenceHost.getOriginalConnection();

        if (originalConnection != null &&
                originalConnection.getPhoneType() != PhoneConstants.PHONE_TYPE_IMS) {
            Log.i(this,
                    "handleOriginalConnectionChange : handover from IMS connection to " +
                            "new connection: %s", originalConnection);

            PhoneAccountHandle phoneAccountHandle = null;
            if (mConferenceHost.getPhone() != null) {
                if (mConferenceHost.getPhone().getPhoneType() == PhoneConstants.PHONE_TYPE_IMS) {
                    Phone imsPhone = mConferenceHost.getPhone();
                    // The phone account handle for an ImsPhone is based on the default phone (ie
                    // the base GSM or CDMA phone, not on the ImsPhone itself).
                    phoneAccountHandle =
                            PhoneUtils.makePstnPhoneAccountHandle(imsPhone.getDefaultPhone());
                } else {
                    // In the case of SRVCC, we still need a phone account, so use the top level
                    // phone to create a phone account.
                    phoneAccountHandle = PhoneUtils.makePstnPhoneAccountHandle(
                            mConferenceHost.getPhone());
                }
            }

            if (mConferenceHost.getPhone().getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
                Log.i(this,"handleOriginalConnectionChange : SRVCC to GSM");
                GsmConnection c = new GsmConnection(originalConnection, getTelecomCallId(),
                        mConferenceHost.isOutgoingCall());
                // This is a newly created conference connection as a result of SRVCC
                c.setConferenceSupported(true);
                if (singleCallAfterConferenceSRVCC()) {
                    c.setConnectionProperties(c.getConnectionProperties()
                            | Connection.PROPERTY_IS_DOWNGRADED_CONFERENCE);
                }
                c.updateState();
                // Copy the connect time from the conferenceHost
                c.setConnectTimeMillis(mConferenceHost.getConnectTimeMillis());
                c.setConnectionStartElapsedRealTime(mConferenceHost.getConnectElapsedTimeMillis());
                mTelephonyConnectionService.addExistingConnection(phoneAccountHandle, c);
                mTelephonyConnectionService.addConnectionToConferenceController(c);
            } // CDMA case not applicable for SRVCC
            mConferenceHost.removeConnectionListener(mConferenceHostListener);
            mConferenceHost.removeTelephonyConnectionListener(mTelephonyConnectionListener);
            mConferenceHost = null;
            setDisconnected(new DisconnectCause(DisconnectCause.OTHER));
            disconnectConferenceParticipants();
            destroy();
        }

        updateStatusHints();
    }

    /**
     * Changes the state of the Ims conference.
     *
     * @param state the new state.
     */
    public void setState(int state) {
        Log.d(this, "setState %s", Connection.stateToString(state));

        switch (state) {
            case Connection.STATE_INITIALIZING:
            case Connection.STATE_NEW:
            case Connection.STATE_RINGING:
                // No-op -- not applicable.
                break;
            case Connection.STATE_DIALING:
                setDialing();
                break;
            case Connection.STATE_DISCONNECTED:
                DisconnectCause disconnectCause;
                // M: add null check for original connection.
                if (mConferenceHost == null || mConferenceHost.getOriginalConnection() == null) {
                    disconnectCause = new DisconnectCause(DisconnectCause.CANCELED);
                } else {
                    if (mConferenceHost.getPhone() != null) {
                        disconnectCause = DisconnectCauseUtil.toTelecomDisconnectCause(
                                mConferenceHost.getOriginalConnection().getDisconnectCause(),
                                null, mConferenceHost.getPhone().getPhoneId());
                    } else {
                        disconnectCause = DisconnectCauseUtil.toTelecomDisconnectCause(
                                mConferenceHost.getOriginalConnection().getDisconnectCause());
                    }
                }
                setDisconnected(disconnectCause);
                disconnectConferenceParticipants();
                destroy();
                break;
            case Connection.STATE_ACTIVE:
                setActive();
                break;
            case Connection.STATE_HOLDING:
                setOnHold();
                break;
        }
    }

    /**
     * Determines if the host of this conference is capable of video calling.
     * @return {@code true} if video capable, {@code false} otherwise.
     */
    private boolean isVideoCapable() {
        int capabilities = mConferenceHost.getConnectionCapabilities();
        return can(capabilities, Connection.CAPABILITY_SUPPORTS_VT_LOCAL_BIDIRECTIONAL)
                && can(capabilities, Connection.CAPABILITY_SUPPORTS_VT_REMOTE_BIDIRECTIONAL);
    }

    private void updateStatusHints() {
        if (mConferenceHost == null) {
            setStatusHints(null);
            return;
        }

        if (mConferenceHost.isWifi()) {
            Phone phone = mConferenceHost.getPhone();
            if (phone != null) {
                Context context = phone.getContext();
                setStatusHints(new StatusHints(
                        context.getString(R.string.status_hint_label_wifi_call),
                        Icon.createWithResource(
                                context, R.drawable.ic_signal_wifi_4_bar_24dp),
                        null /* extras */));
            }
        } else {
            setStatusHints(null);
        }
    }

    /**
     * Builds a string representation of the {@link ImsConference}.
     *
     * @return String representing the conference.
     */
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("[ImsConference objId:");
        sb.append(System.identityHashCode(this));
        sb.append(" telecomCallID:");
        sb.append(getTelecomCallId());
        sb.append(" state:");
        sb.append(Connection.stateToString(getState()));
        sb.append(" hostConnection:");
        sb.append(mConferenceHost);
        sb.append(" participants:");
        sb.append(mConferenceParticipantConnections.size());
        sb.append("]");
        return sb.toString();
    }

    private boolean canHoldImsCalls() {
        PersistableBundle b = getCarrierConfig();
        // Return true if the CarrierConfig is unavailable
        return b == null || b.getBoolean(CarrierConfigManager.KEY_ALLOW_HOLD_IN_IMS_CALL_BOOL);
    }

    private PersistableBundle getCarrierConfig() {
        if (mConferenceHost == null) {
            return null;
        }

        Phone phone = mConferenceHost.getPhone();
        if (phone == null) {
            return null;
        }
        return PhoneGlobals.getInstance().getCarrierConfigForSubId(phone.getSubId());
    }

    /**
     * @return {@code true} if the carrier associated with the conference requires that the maximum
     *      size of the conference is enforced, {@code false} otherwise.
     */
    public boolean isMaximumConferenceSizeEnforced() {
        PersistableBundle b = getCarrierConfig();
        // Return false if the CarrierConfig is unavailable
        return b != null && b.getBoolean(
                CarrierConfigManager.KEY_IS_IMS_CONFERENCE_SIZE_ENFORCED_BOOL);
    }

    /**
     * @return The maximum size of a conference call where
     * {@link #isMaximumConferenceSizeEnforced()} is true.
     */
    public int getMaximumConferenceSize() {
        PersistableBundle b = getCarrierConfig();

        // If there is no carrier config its really a problem, but we'll still define a sane limit
        // of 5 so that we can still make a conference.
        if (b == null) {
            Log.w(this, "getMaximumConferenceSize - failed to get conference size");
            return 5;
        }
        return b.getInt(CarrierConfigManager.KEY_IMS_CONFERENCE_SIZE_LIMIT_INT);
    }

    /**
     * @return The number of participants in the conference.
     */
    public int getNumberOfParticipants() {
        return mConferenceParticipantConnections.size();
    }

    /**
     * @return {@code True} if the carrier enforces a maximum conference size, and the number of
     *      participants in the conference has reached the limit, {@code false} otherwise.
     */
    public boolean isFullConference() {
        return isMaximumConferenceSizeEnforced()
                && getNumberOfParticipants() >= getMaximumConferenceSize();
    }

    /// M: For VoLTE enhanced conference call. @{
    public void onInviteConferenceParticipants(List<String> numbers) {
        Log.d(this, "onInviteConferenceParticipants()");
        if (mConferenceHost == null) {
            Log.w(this, "mConferenceHost is null!");
            return;
        }
        // Judge whether the invited number has already existed in the conference or self
        Iterator<String> iter = numbers.iterator();
        while (iter.hasNext()) {
            String next = iter.next();
            if (hasExistedInConference(next)
                    || isParticipantHost(mConferenceHostAddress, next)) {
                iter.remove();
            }
        }
        if (numbers.size() == 0) {
            return;
        }
        /// M: ALPS04057265, Show toast if the conference is full @{
        if (getNumberOfParticipants() >= getMaximumConferenceSize()) {
            Log.d(this, "Conference is full!");
            if (mConferenceHost.getPhone() != null && isShowToastWhenConferenceFull()) {
                toastWhenConferenceIsFull(mConferenceHost.getPhone().getContext());
            }
            if (ignoreAddRequestToFullConference()) {
                Log.d(this, "Ignore the add request");
                return;
            }
        }
        /// @}

        ((TelephonyConnection)mConferenceHost).performInviteConferenceParticipants(numbers);
        mIsDuringAddingParticipants = true;
    }

    protected final void setRinging() {
        setState(Connection.STATE_RINGING);
    }

    private boolean hasExistedInConference(String number) {
        Iterator<Map.Entry<Pair<Uri, Uri>, ConferenceParticipantConnection>> entryIterator =
            mConferenceParticipantConnections.entrySet().iterator();
        while (entryIterator.hasNext()) {
            Map.Entry<Pair<Uri, Uri>, ConferenceParticipantConnection> entry =
                entryIterator.next();
            Pair<Uri, Uri> key = entry.getKey();
            Uri userEntity =  key.first;
            String participantNumber = userEntity.getSchemeSpecificPart();
            Log.w(this, "The invited number is %s and participant number is %s",
                    number, participantNumber);
            if (PhoneNumberUtils.compare(number, participantNumber)) {
                Log.d(this, "The invited number has already existed in the conference");
                return true;
            }
        }
        return false;
    }

    private boolean isParticipantHost(Uri[] hostHandles, String number) {
        // If there is no host handle or no participant handle, bail early.
        if (hostHandles == null || hostHandles.length == 0 || number == null) {
            Log.d(this, "isParticipantHost(N) : host or participant uri null");
            return false;
        }

        // Conference event package participants are identified using SIP URIs (see RFC3261).
        // A valid SIP uri has the format: sip:user:password@host:port;uri-parameters?headers
        // Per RFC3261, the "user" can be a telephone number.
        // For example: sip:1650555121;phone-context=blah.com@host.com
        // In this case, the phone number is in the user field of the URI, and the parameters can be
        // ignored.
        //
        // A SIP URI can also specify a phone number in a format similar to:
        // sip:+1-212-555-1212@something.com;user=phone
        // In this case, the phone number is again in user field and the parameters can be ignored.
        // We can get the user field in these instances by splitting the string on the @, ;, or :
        // and looking at the first found item.

        String numberParts[] = number.split("[@;:]");

        if (numberParts.length == 0) {
            Log.d(this, "isParticipantHost(N) : no number in participant handle");
            return false;
        }
        number = numberParts[0];

        for (Uri hostHandle : hostHandles) {
            if (hostHandle == null) {
                continue;
            }
            // The host number will be a tel: uri.  Per RFC3966, the part after tel: is the phone
            // number.
            String hostNumber = hostHandle.getSchemeSpecificPart();

            // Use a loose comparison of the phone numbers.  This ensures that numbers that differ
            // by special characters are counted as equal.
            // E.g. +16505551212 would be the same as 16505551212
            boolean isHost = PhoneNumberUtils.compare(hostNumber, number);

            Log.d(this, "isParticipantHost(%s) : host: %s, participant %s", (isHost ? "Y" : "N"),
                    Log.pii(hostNumber), Log.pii(number));

            if (isHost) {
                return true;
            }
        }
        return false;
    }

    private void toastWhenIsAddingParticipants() {
        if (mConferenceHost == null) {
            return;
        }
        Context context;
        Phone phone = mConferenceHost.getPhone();
        if (phone != null) {
            context = phone.getContext();
            Toast.makeText(context,
                    context.getString(R.string.volte_is_adding_participants), Toast.LENGTH_SHORT).
                        show();
        }
    }

    /// M: ALPS02209724. Popup toast when conference reach the maximum participants. @{
    /**
     * For below cases, PhoneApp popups the toast to hint user the conference is full.
     * case-1: when merging the 6th call into a conference.
     * case-2: when inviting participant to cause conference has more than 6 participants.
     * case-3: when dialing a conference with over 6 participants.
     *
     * @hide
     */
    static void toastWhenConferenceIsFull(Context context) {
        if (context == null) {
            return;
        }
        Toast.makeText(context,
                context.getString(R.string.volte_conf_member_reach_max), Toast.LENGTH_SHORT)
                    .show();
    }
    /// @}

    /// M: For conference SRVCC. @{
    private void handleConferenceSRVCC(
            ArrayList<com.android.internal.telephony.Connection> radioConnections) {
        Log.w(this, "handleConferenceSRVCC");
        if (mConferenceHost == null) {
            Log.w(this, "onConferenceConnectionsConfigured: conference host missing.");
            return;
        }
        if (radioConnections == null || radioConnections.size() < 2) {
            Log.w(this, "onConferenceConnectionsConfigured: failed at radioConnections.");
            return;
        }
        disconnectConferenceParticipants();
        if (mTelephonyConnectionService instanceof TelephonyConnectionServiceProxy) {
            mTelephonyConnectionService.performImsConferenceSRVCC(
                    this, radioConnections, getTelecomCallId());
        }
        mConferenceHost.removeConnectionListener(mConferenceHostListener);
        mConferenceHost.removeTelephonyConnectionListener(mTelephonyConnectionListener);
        mConferenceHost = null;
        destroy();
    }

    /// M: CC: HangupAll for FTA 31.4.4.2 @{
    public void onHangupAll() {
        Log.w(this, "onHangupAll()");
        if (mConferenceHost == null) {
            return;
        }
        try {
            Phone phone = mConferenceHost.getPhone();
            if (phone != null && phone instanceof MtkImsPhone) {
                ((MtkImsPhone)phone).hangupAll();
            } else {
                Log.w(this, "Attempting to hangupAll a conference without backing phone.");
            }
        } catch (CallStateException e) {
            Log.e(this, e, "Call to phone.hangupAll() failed with exception");
        }
    }
    /// @}

    /// M: Customize for specific operator and location.
    private boolean isShowToastWhenConferenceFull() {
        boolean showToast = true;
        PersistableBundle b = getCarrierConfig();
        if (b != null) {
            showToast = b.getBoolean(
                    MtkCarrierConfigManager.MTK_KEY_SHOW_TOAST_WHEN_CONFERENCE_FULL_BOOL);
        }
        Log.d(this, "showToast: %s", showToast);
        return showToast;
    }

    private boolean ignoreAddRequestToFullConference() {
        PersistableBundle b = getCarrierConfig();
        boolean ret = true;
        if (b != null) {
            ret = b.getBoolean(
                MtkCarrierConfigManager.MTK_KEY_IMS_NO_CONF_REQ_AFTER_MAX_CONNECTION_BOOL);
        }
        // Return true if the CarrierConfig is unavailable
        return ret;
    }

    public boolean specialConstraintForVideoConference() {
        PersistableBundle b = getCarrierConfig();
        boolean ret = false;
        if (b != null) {
            ret = b.getBoolean(
                MtkCarrierConfigManager.MTK_KEY_SPECIAL_CONSTRAINT_FOR_IMS_VIDEO_CONFERENCE);
        }
        // Return false if the CarrierConfig is unavailable
        Log.d(this, "specialConstraintForVideoConference: " + ret);
        return ret;
    }

    public int getMaximumVideoConferenceSize() {
        PersistableBundle b = getCarrierConfig();

        // If there is no carrier config its really a problem, but we'll still define a sane limit
        // of 5.
        if (b == null) {
            Log.w(this, "getMaximumVideoConferenceSize - failed to get video conference size");
            return 5;
        }
        return b.getInt(MtkCarrierConfigManager.MTK_KEY_IMS_VIDEO_CONFERENCE_SIZE_LIMIT_INT);
    }

    private boolean singleCallAfterConferenceSRVCC() {
        PersistableBundle b = getCarrierConfig();
        boolean ret = true;
        if (b != null) {
            ret = b.getBoolean(
                MtkCarrierConfigManager.MTK_KEY_SINGLE_CALL_AFTER_CONFERENCE_SRVCC);
        }
        // Return true if the CarrierConfig is unavailable
        Log.d(this, "singleCallAfterConferenceSRVCC: " + ret);
        return ret;
    }

    public boolean allowMergeOnParticipant() {
        PersistableBundle b = getCarrierConfig();
        boolean ret = true;
        if (b != null) {
            ret = b.getBoolean(
                MtkCarrierConfigManager.KEY_SHOW_MERGE_ON_PARTICIPANT);
        }
        // Return true if the CarrierConfig is unavailable
        Log.d(this, "allowMergeOnParticipant: " + ret);
        return ret;
    }
    /// @}

    public TelephonyConnection getHostConnection() {
        return mConferenceHost;
    }

    /// M: ALPS04234226. For GCF case 15.19. @{
    /// TC_15.17 (Creating and leaving a conference)
    /// TC_15.19 (Inviting user to conference by sending a REFER request to the conference focus)
    /// Based on TC_15.17 that will create a empty conference by one-key launched, TC_15.19
    /// needs to invite a member to the conference in conference management screen. So it needs
    /// to show conference management button to enter conference management screen even if there
    /// is no any member in the conference.
    private boolean isTestSim() {
      boolean isTestSim = false;
      isTestSim = SystemProperties.get("vendor.gsm.sim.ril.testsim").equals("1") ||
                  SystemProperties.get("vendor.gsm.sim.ril.testsim.2").equals("1") ||
                  SystemProperties.get("vendor.gsm.sim.ril.testsim.3").equals("1") ||
                  SystemProperties.get("vendor.gsm.sim.ril.testsim.4").equals("1");
      Log.d(this, "isTestSim: %s", isTestSim);
      return isTestSim;
    }
    /// @}

    /// M: For multi anchor feature @{
    /**
     * Similar to {@link #setConferenceableConnections(java.util.List)}, sets a list of connections
     * or conferences with which this connection can be conferenced.
     *
     * @param conferenceables The conferenceables.
     * @hide
     */
    public final void setConferenceables(List<Conferenceable> conferenceables) {
        clearConferenceablesList();
        for (Conferenceable c : conferenceables) {
            // If statement checks for duplicates in input. It makes it N^2 but we're dealing with a
            // small amount of items here.
            if (!mConferenceables.contains(c)) {
                if (c instanceof Connection) {
                    Connection connection = (Connection) c;
                    connection.addConnectionListener(mConnectionDeathListener);
                } else if (c instanceof Conference) {
                    Conference conference = (Conference) c;
                    conference.addListener(mConferenceDeathListener);
                }
                mConferenceables.add(c);
            }
        }
        fireOnConferenceablesChanged();
    }

    /**
     * Returns the connections or conferences with which this connection can be conferenced.
     * @hide
     */
    public final List<Conferenceable> getConferenceables() {
        return mUnmodifiableConferenceables;
    }

    private final void fireOnConferenceablesChanged() {
        for (Conference.Listener l : mListeners) {
            l.onConferenceablesChanged(this, getConferenceables());
        }
    }

    private final void clearConferenceablesList() {
        for (Conferenceable c : mConferenceables) {
            if (c instanceof Connection) {
                Connection connection = (Connection) c;
                connection.removeConnectionListener(mConnectionDeathListener);
            } else if (c instanceof Conference) {
                Conference conference = (Conference) c;
                conference.removeListener(mConferenceDeathListener);
            }
        }
        mConferenceables.clear();
    }
    /// @}
}

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

import android.telecom.Conference;
import android.telecom.Conferenceable;
import android.telecom.Connection;
import android.telecom.ConnectionService;
import android.telecom.DisconnectCause;
import android.telecom.PhoneAccountHandle;
import android.telecom.VideoProfile;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.phone.PhoneUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.stream.Collectors;

import com.mediatek.internal.telephony.imsphone.MtkImsPhoneConnection;
import com.mediatek.phone.ext.ExtensionManager;

/**
 * Manages conferences for IMS connections.
 */
public class ImsConferenceController {

    /**
     * Conference listener; used to receive notification when a conference has been disconnected.
     */
    private final Conference.Listener mConferenceListener = new Conference.Listener() {
        @Override
        public void onDestroyed(Conference conference) {
            if (Log.DEBUG) {
                Log.d(ImsConferenceController.class, "onDestroyed: %s", conference);
            }

            mImsConferences.remove(conference);
        }

        // M: ALPS04414516: recalculate conferenceable for VoLTE enhanced conference call.
        @Override
        public void onStateChanged(Conference conference, int oldState, int newState) {
            Log.d(ImsConferenceController.class, "onStateChanged: %s", conference);
            recalculate();
        }
    };

    /**
     * Ims conference controller connection listener.  Used to respond to changes in state of the
     * Telephony connections the controller is aware of.
     */
    private final Connection.Listener mConnectionListener = new Connection.Listener() {
        @Override
        public void onStateChanged(Connection c, int state) {
            Log.d(this, "onStateChanged: %s", Log.pii(c.getAddress()));
            recalculate();
        }

        @Override
        public void onDisconnected(Connection c, DisconnectCause disconnectCause) {
            Log.d(this, "onDisconnected: %s", Log.pii(c.getAddress()));
            recalculate();
        }

        @Override
        public void onDestroyed(Connection connection) {
            remove(connection);
        }

        @Override
        public void onConferenceStarted() {
            Log.d(this, "onConferenceStarted");
            recalculate();
        }

        @Override
        public void onConferenceSupportedChanged(Connection c, boolean isConferenceSupported) {
            Log.d(this, "onConferenceSupportedChanged");
            recalculate();
        }
    };

    /**
     * The current {@link ConnectionService}.
     */
    private final TelephonyConnectionServiceProxy mConnectionService;

    private final ImsConference.FeatureFlagProxy mFeatureFlagProxy;

    /**
     * List of known {@link TelephonyConnection}s.
     */
    private final ArrayList<TelephonyConnection> mTelephonyConnections = new ArrayList<>();

    /**
     * List of known {@link ImsConference}s.  Realistically there will only ever be a single
     * concurrent IMS conference.
     */
    private final ArrayList<ImsConference> mImsConferences = new ArrayList<>(2);

    private TelecomAccountRegistry mTelecomAccountRegistry;

    // For TMO requirement
    private boolean mSpecialConstraintForVideoConference = false;
    private int mMaximumVideoConferenceSize = 5;
    private final static boolean CAN_AUDIO_CALL_MERGE_FULL_VIDEO_CONFERENCE = true;

    // For Multi-anchor feature in Telephony
    private boolean mAllowMergeOnParticipant = false;

    /**
     * Creates a new instance of the Ims conference controller.
     *
     * @param connectionService The current connection service.
     * @param featureFlagProxy
     */
    public ImsConferenceController(TelecomAccountRegistry telecomAccountRegistry,
            TelephonyConnectionServiceProxy connectionService,
            ImsConference.FeatureFlagProxy featureFlagProxy) {
        mConnectionService = connectionService;
        mTelecomAccountRegistry = telecomAccountRegistry;
        mFeatureFlagProxy = featureFlagProxy;
    }

    /**
     * Adds a new connection to the IMS conference controller.
     *
     * @param connection
     */
    void add(TelephonyConnection connection) {
        // DO NOT add external calls; we don't want to consider them as a potential conference
        // member.
        if ((connection.getConnectionProperties() & Connection.PROPERTY_IS_EXTERNAL_CALL) ==
                Connection.PROPERTY_IS_EXTERNAL_CALL) {
            return;
        }

        if (mTelephonyConnections.contains(connection)) {
            // Adding a duplicate realistically shouldn't happen.
            Log.w(this, "add - connection already tracked; connection=%s", connection);
            return;
        }

        // Note: Wrap in Log.VERBOSE to avoid calling connection.toString if we are not going to be
        // outputting the value.
        if (Log.DEBUG) {
            Log.d(this, "add connection %s", connection);
        }

        mTelephonyConnections.add(connection);
        connection.addConnectionListener(mConnectionListener);
        recalculateConference();
    }

    /**
     * Removes a connection from the IMS conference controller.
     *
     * @param connection
     */
    void remove(Connection connection) {
        // External calls are not part of the conference controller, so don't remove them.
        if ((connection.getConnectionProperties() & Connection.PROPERTY_IS_EXTERNAL_CALL) ==
                Connection.PROPERTY_IS_EXTERNAL_CALL) {
            return;
        }

        if (!mTelephonyConnections.contains(connection)) {
            // Debug only since TelephonyConnectionService tries to clean up the connections tracked
            // when the original connection changes.  It does this proactively.
            Log.d(this, "remove - connection not tracked; connection=%s", connection);
            return;
        }

        if (Log.DEBUG) {
            Log.d(this, "remove connection: %s", connection);
        }

        connection.removeConnectionListener(mConnectionListener);
        mTelephonyConnections.remove(connection);
        recalculateConferenceable();
    }

    /**
     * Triggers both a re-check of conferenceable connections, as well as checking for new
     * conferences.
     */
    private void recalculate() {
        recalculateConferenceable();
        recalculateConference();
    }

    /**
     * Calculates the conference-capable state of all GSM connections in this connection service.
     */
    private void recalculateConferenceable() {
        Log.d(this, "recalculateConferenceable : %d", mTelephonyConnections.size());

        boolean videoConferenceConstraint = false;
        if (mImsConferences.size() > 0) {
            // TMO requirement. There is a special constrain when video conference participant is 3.
            if (mSpecialConstraintForVideoConference) {
                ImsConference conference = mImsConferences.get(0);

                if (VideoProfile.isVideo(conference.getVideoState())
                        && conference.getNumberOfParticipants() >= mMaximumVideoConferenceSize) {
                    videoConferenceConstraint = true;
                }
            }
        }

        HashSet<Conferenceable> conferenceableSet = new HashSet<>(mTelephonyConnections.size() +
                mImsConferences.size());
        HashSet<Conferenceable> conferenceParticipantsSet = new HashSet<>();

        // Loop through and collect all calls which are active or holding
        for (TelephonyConnection connection : mTelephonyConnections) {
            if (Log.DEBUG) {
                Log.d(this, "recalc - %s %s supportsConf? %s", connection.getState(), connection,
                        connection.isConferenceSupported());
            }

            // If this connection is a member of a conference hosted on another device, it is not
            // conferenceable with any other connections.
            if (isMemberOfPeerConference(connection)) {
                if (Log.DEBUG) {
                    Log.d(this, "Skipping connection in peer conference: %s", connection);
                }

                boolean canMergeOnParticipant = checkCanMergeOnParticipant(connection);
                if(!canMergeOnParticipant)
                continue;
            }

            // If this connection does not support being in a conference call, then it is not
            // conferenceable with any other connection.
            if (!connection.isConferenceSupported()) {
                connection.setConferenceables(Collections.<Conferenceable>emptyList());
                continue;
            }

            //For RTT: Some operators ex. TMO, not allow to merge RTT call.
            if (ExtensionManager.getRttUtilExt().isRttCallAndNotAllowMerge(
                        connection.getOriginalConnection())) {
                continue;
            }

            // TMO requirement. When video conference participant is 3,
            // can merge audio call, but video call can't.
            if (videoConferenceConstraint) {
                if (!CAN_AUDIO_CALL_MERGE_FULL_VIDEO_CONFERENCE ||
                        VideoProfile.isVideo(connection.getVideoState())) {
                    continue;
                }
            }

            switch (connection.getState()) {
                case Connection.STATE_ACTIVE:
                    // fall through
                case Connection.STATE_HOLDING:
                    conferenceableSet.add(connection);
                    continue;
                default:
                    break;
            }
            // This connection is not active or holding, so clear all conferencable connections
            connection.setConferenceables(Collections.<Conferenceable>emptyList());
        }
        // Also loop through all active conferences and collect the ones that are ACTIVE or HOLDING.
        for (ImsConference conference : mImsConferences) {
            if (Log.DEBUG) {
                Log.d(this, "recalc - %s %s", conference.getState(), conference);
            }

            if (!conference.isConferenceHost()) {
                if (Log.DEBUG) {
                    Log.d(this, "skipping conference (not hosted on this device): %s", conference);
                }
                boolean canMergeOnParticipant = checkCanMergeOnParticipant(
                        conference.getHostConnection());
                if(!canMergeOnParticipant)
                continue;
            }

            switch (conference.getState()) {
                case Connection.STATE_ACTIVE:
                    //fall through
                case Connection.STATE_HOLDING:
                    if (!conference.isFullConference()) {
                        conferenceParticipantsSet.addAll(conference.getConnections());
                        conferenceableSet.add(conference);
                    }
                    continue;
                default:
                    break;
            }
        }

        Log.d(this, "conferenceableSet size: " + conferenceableSet.size());

        for (Conferenceable c : conferenceableSet) {
            if (c instanceof Connection) {
                // Remove this connection from the Set and add all others
                List<Conferenceable> conferenceables = conferenceableSet
                        .stream()
                        .filter(conferenceable -> c != conferenceable)
                        .collect(Collectors.toList());
                // TODO: Remove this once RemoteConnection#setConferenceableConnections is fixed.
                // Add all conference participant connections as conferenceable with a standalone
                // Connection.  We need to do this to ensure that RemoteConnections work properly.
                // At the current time, a RemoteConnection will not be conferenceable with a
                // Conference, so we need to add its children to ensure the user can merge the call
                // into the conference.
                // We should add support for RemoteConnection#setConferenceables, which accepts a
                // list of remote conferences and connections in the future.
                conferenceables.addAll(conferenceParticipantsSet);

                ((Connection) c).setConferenceables(conferenceables);
            } else if (c instanceof ImsConference) {
                ImsConference imsConference = (ImsConference) c;

                // If the conference is full, don't allow anything to be conferenced with it.
                if (imsConference.isFullConference()) {
                    imsConference.setConferenceableConnections(Collections.<Connection>emptyList());
                }

                boolean canMergeOnParticipant = false;
                if (!imsConference.isConferenceHost()) {
                    canMergeOnParticipant = checkCanMergeOnParticipant(
                            imsConference.getHostConnection());
                }
                if (!canMergeOnParticipant) {
                    // Remove all conferences from the set, since we can not conference a conference
                    // to another conference.
                    List<Connection> connections = conferenceableSet
                            .stream()
                            .filter(conferenceable -> conferenceable instanceof Connection)
                            .map(conferenceable -> (Connection) conferenceable)
                            .collect(Collectors.toList());
                    // Conference equivalent to setConferenceables that only accepts Connections
                    imsConference.setConferenceableConnections(connections);
                } else {
                    List<Conferenceable> conferenceables = conferenceableSet
                          .stream()
                          .filter(conferenceable -> c != conferenceable)
                          .collect(Collectors.toList());
                    Log.v(this, "isMergeOnParticipant make conf conferenceable size" +
                            conferenceables.size());
                    imsConference.setConferenceables(conferenceables);
                }
            }
        }
    }

    /**
     * Determines if a connection is a member of a conference hosted on another device.
     *
     * @param connection The connection.
     * @return {@code true} if the connection is a member of a conference hosted on another device.
     */
    private boolean isMemberOfPeerConference(Connection connection) {
        if (!(connection instanceof TelephonyConnection)) {
            return false;
        }
        TelephonyConnection telephonyConnection = (TelephonyConnection) connection;
        com.android.internal.telephony.Connection originalConnection =
                telephonyConnection.getOriginalConnection();

        return originalConnection != null && originalConnection.isMultiparty() &&
                originalConnection.isMemberOfPeerConference();
    }

    /**
     * Starts a new ImsConference for a connection which just entered a multiparty state.
     */
    private void recalculateConference() {
        Log.d(this, "recalculateConference");

        Iterator<TelephonyConnection> it = mTelephonyConnections.iterator();
        while (it.hasNext()) {
            TelephonyConnection connection = it.next();
            if (connection.isImsConnection() && connection.getOriginalConnection() != null &&
                    connection.getOriginalConnection().isMultiparty() &&
                    /// M: Do NOT create IMS conference if it's a MT call @{
                    // Otherwise this connection will be disconnected and
                    // call duration displays 0s in call log
                    !((MtkImsPhoneConnection)(connection.getOriginalConnection()))
                    .isIncomingCallMultiparty()) {
                    /// @}
                startConference(connection);
                it.remove();
            }
        }
    }

    /**
     * Starts a new {@link ImsConference} for the given IMS connection.
     * <p>
     * Creates a new IMS Conference to manage the conference represented by the connection.
     * Internally the ImsConference wraps the radio connection with a new TelephonyConnection
     * which is NOT reported to the connection service and Telecom.
     * <p>
     * Once the new IMS Conference has been created, the connection passed in is held and removed
     * from the connection service (removing it from Telecom).  The connection is put into a held
     * state to ensure that telecom removes the connection without putting it into a disconnected
     * state first.
     *
     * @param connection The connection to the Ims server.
     */
    private void startConference(TelephonyConnection connection) {
        if (Log.DEBUG) {
            Log.d(this, "Start new ImsConference - connection: %s", connection);
        }

        // Make a clone of the connection which will become the Ims conference host connection.
        // This is necessary since the Connection Service does not support removing a connection
        // from Telecom.  Instead we create a new instance and remove the old one from telecom.
        TelephonyConnection conferenceHostConnection = connection.cloneConnection();
        conferenceHostConnection.setVideoPauseSupported(connection.getVideoPauseSupported());
        conferenceHostConnection.setManageImsConferenceCallSupported(
                connection.isManageImsConferenceCallSupported());

        PhoneAccountHandle phoneAccountHandle = null;

        // Attempt to determine the phone account associated with the conference host connection.
        if (connection.getPhone() != null &&
                connection.getPhone().getPhoneType() == PhoneConstants.PHONE_TYPE_IMS) {
            Phone imsPhone = connection.getPhone();
            // The phone account handle for an ImsPhone is based on the default phone (ie the
            // base GSM or CDMA phone, not on the ImsPhone itself).
            phoneAccountHandle =
                    PhoneUtils.makePstnPhoneAccountHandle(imsPhone.getDefaultPhone());
        }

        /// M: ALPS03865614. Skip adding a disconnected conferencecall.
        /// If the hostconnection is in disconnected state, it is not supposed to do addConference.
        /// When creating ImsConference, it will trigger destroying conference and there will be no
        /// more destroying conference after conference is added to telecom. @{
        ImsConference conference = null;
        if (conferenceHostConnection.getState() == Connection.STATE_DISCONNECTED) {
            Log.d(this, "Skip adding a disconnected conference call");
        } else {
            conference = new ImsConference(mTelecomAccountRegistry, mConnectionService,
                    conferenceHostConnection, phoneAccountHandle, mFeatureFlagProxy);
            conference.setState(conferenceHostConnection.getState());
            conference.addListener(mConferenceListener);
            conference.updateConferenceParticipantsAfterCreation();
            mConnectionService.addConference(conference);
            conferenceHostConnection.setTelecomCallId(conference.getTelecomCallId());
        }
        /// @}

        // Cleanup TelephonyConnection which backed the original connection and remove from telecom.
        // Use the "Other" disconnect cause to ensure the call is logged to the call log but the
        // disconnect tone is not played.
        connection.removeConnectionListener(mConnectionListener);
        connection.clearOriginalConnection();
        connection.setDisconnected(new DisconnectCause(DisconnectCause.OTHER,
                android.telephony.DisconnectCause.toString(
                        android.telephony.DisconnectCause.IMS_MERGED_SUCCESSFULLY)));
        connection.destroy();

        /// M: ALPS03865614. Skip adding a disconnected conferencecall.
        /// Not adding a disconnected connection to the ArrayList. @{
        if ((conferenceHostConnection.getState() != Connection.STATE_DISCONNECTED) &&
                conference != null) {
            mSpecialConstraintForVideoConference = conference.specialConstraintForVideoConference();
            mMaximumVideoConferenceSize = conference.getMaximumVideoConferenceSize();
            mAllowMergeOnParticipant = conference.allowMergeOnParticipant();
            mImsConferences.add(conference);
        }
        /// @}

        // If one of the participants failed to join the conference, recalculate will set the
        // conferenceable connections for the conference to show merge calls option.
        recalculateConferenceable();
    }

    // For VoLTE enhanced conference call.
    /**
     * Creates a new IMS conference directly.
     * @param hostConnection The connection to the conference server.
     */
    public ImsConference createConference(TelephonyConnection hostConnection) {
        Log.d(this, "Create new ImsConference - connection: %s", hostConnection);
        PhoneAccountHandle phoneAccountHandle = null;
        // Attempt to determine the phone account associated with the conference host connection.
        if (hostConnection.getPhone() != null &&
                hostConnection.getPhone().getPhoneType() == PhoneConstants.PHONE_TYPE_IMS) {
            Phone imsPhone = hostConnection.getPhone();
            // The phone account handle for an ImsPhone is based on the default phone (ie the
            // base GSM or CDMA phone, not on the ImsPhone itself).
            phoneAccountHandle =
                    PhoneUtils.makePstnPhoneAccountHandle(imsPhone.getDefaultPhone());
        }
        ImsConference conference = new ImsConference(mTelecomAccountRegistry,
                mConnectionService, hostConnection, phoneAccountHandle, mFeatureFlagProxy);
        int capabilities = conference.getConnectionCapabilities();
        capabilities &= ~Connection.CAPABILITY_HOLD;
        conference.setConnectionCapabilities(capabilities);
        conference.setState(hostConnection.getState());
        conference.addListener(mConferenceListener);
        mImsConferences.add(conference);

        // If one of the participants failed to join the conference, recalculate will set the
        // conferenceable connections for the conference to show merge calls option.
        recalculateConferenceable();
        return conference;
    }
    /// @}

    public boolean isConferenceExist() {
        Log.d(this, "isConferenceExist: %d", mImsConferences.size());
        return (mImsConferences.size() > 0);
    }

    // M: For Multi-anchor feature in Telephony
    private boolean checkCanMergeOnParticipant(TelephonyConnection connection) {
        if (connection == null) {
            return false;
        }
        boolean canMergeOnParticipant = false;
        boolean isCurrentVideoCall = false;
        boolean isCurrentWifiCall = false;
        if (mAllowMergeOnParticipant) {
            isCurrentVideoCall = VideoProfile.isVideo(connection.getVideoState());
            isCurrentWifiCall = connection.isWifi();
            canMergeOnParticipant = !isCurrentVideoCall;
        }

        Log.d(this, "checkCanMergeOnParticipant canMergeOnParticipant "+ canMergeOnParticipant
                + "isCurrentVideoCall " + isCurrentVideoCall
                + "isWifiCall " + isCurrentWifiCall
                + "mAllowMergeOnParticipant " + mAllowMergeOnParticipant);
        return canMergeOnParticipant;
    }
}

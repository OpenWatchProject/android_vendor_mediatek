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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.telecom.Connection;
import android.telecom.DisconnectCause;
import android.telecom.PhoneAccountHandle;

import com.android.internal.telephony.TelephonyIntents;
import com.android.phone.PhoneGlobals;
import com.android.phone.PhoneUtils;

import com.mediatek.services.telephony.MtkGsmCdmaConnection;

import java.util.ArrayList;
import java.util.List;

/**
 * Manages CDMA conference calls. CDMA conference calls are much more limited than GSM conference
 * calls. Two main points of difference:
 * 1) Users cannot manage individual calls within a conference
 * 2) Whether a conference call starts off as a conference or as two distinct calls is a matter of
 *    physical location (some antennas are different than others). Worst still, there's no
 *    indication given to us as to what state they are in.
 *
 * To make life easier on the user we do the following: Whenever there exist 2 or more calls, we
 * say that we are in a conference call with {@link Connection#PROPERTY_GENERIC_CONFERENCE}.
 * Generic indicates that this is a simple conference that doesn't support conference management.
 * The conference call will also support "MERGE" to begin with and stop supporting it the first time
 * we are asked to actually execute a merge. I emphasize when "we are asked" because we get no
 * indication whether the merge succeeds from CDMA, we just assume it does. Thats the best we
 * can do. Also, we do not kill a conference call once it is created unless all underlying
 * connections also go away.
 *
 * Outgoing CDMA calls made while another call exists would normally trigger a conference to be
 * created. To avoid this and make it seem like there is a "dialing" state, we fake it and prevent
 * the conference from being created for 3 seconds. This is a more pleasant experience for the user.
 */
final class CdmaConferenceController {
    private final Connection.Listener mConnectionListener = new Connection.Listener() {
                @Override
                public void onStateChanged(Connection c, int state) {
                    /// M: CC: Don't recalculate conference when connection is disconnected. @{
                    Log.d(this, "onStateChanged, conn=" + c + ", state=" + state);
                    if (state != android.telecom.Connection.STATE_DISCONNECTED) {
                        recalculateConference();
                    }
                    /// @}
                }

                @Override
                public void onDisconnected(Connection c, DisconnectCause disconnectCause) {
                    /// M: CC: Don't recalculate conference when connection is disconnected. @{
                    Log.d(this, "onDisconnected, conn=" + c + ", cause=" + disconnectCause);
                    /// @}
                }

                @Override
                public void onDestroyed(Connection c) {
                    /// M: CC: Merge to MtkGsmCdmaConnection @{
                    //remove((CdmaConnection) c);
                    Log.d(this, "onDestroyed, conn=" + c);
                    remove((MtkGsmCdmaConnection) c);
                    /// @}
                }
            };

    private static final int ADD_OUTGOING_CONNECTION_DELAY_MILLIS = 6000;

    /** The known CDMA connections. */
    /// M: CC: Merge to MtkGsmCdmaConnection @{
    //private final List<CdmaConnection> mCdmaConnections = new ArrayList<>();
    private final List<MtkGsmCdmaConnection> mCdmaConnections = new ArrayList<>();
    /// @}
    /**
     * Newly added connections.  We keep track of newly added outgoing connections because we do not
     * create a conference until a second outgoing call has existing for
     * {@link #ADD_OUTGOING_CONNECTION_DELAY_MILLIS} milliseconds.  This allows the UI to show the
     * call as "Dialing" for a certain amount of seconds.
     */
    /// M: CC: Merge to MtkGsmCdmaConnection @{
    //private final List<CdmaConnection> mPendingOutgoingConnections = new ArrayList<>();
    private final List<MtkGsmCdmaConnection> mPendingOutgoingConnections = new ArrayList<>();
    /// @}

    private final TelephonyConnectionService mConnectionService;

    private final Handler mHandler = new Handler();

    /// M: CC: For CDMA conference @{
    private static final int UPDATE_CALL_CAPABILITIE_DELAY_MILLIS = 200;
    private static final int ADD_WAITING_CONNECTION_DELAY_MILLIS = 1000;
    private CdmaConferenceBroadcastReceiver mReceiver;
    private int mConfConnCount = 0;
    private static final boolean MTK_SVLTE_SUPPORT =
            "1".equals(android.os.SystemProperties.get("ro.boot.opt_c2k_lte_mode"));
    /// @}

    /// M: CC: CDMA second call. @{
    // Set the second MO call to active immediately when receive waiting call,
    // so add new member to record the connection and runnable.
    private MtkGsmCdmaConnection mSecondCall = null;
    private List<MtkGsmCdmaConnection> mConnectionsToReset = new ArrayList<MtkGsmCdmaConnection>();
    private Runnable mDelayRunnable = new Runnable() {
        @Override
        public void run() {
            Log.d(this, "mDelayRunnable, mSecondCall=" + mSecondCall);
            if (mSecondCall != null) {
                mSecondCall.forceAsDialing(false);
                addInternal(mSecondCall);
                mSecondCall = null;
            }
            Log.d(this, "mDelayRunnable, mConnectionsToReset=" + mConnectionsToReset);
            for (MtkGsmCdmaConnection current : mConnectionsToReset) {
                Log.d(this, "mDelayRunnable, reset state for conn=" + current);
                current.resetStateForConference();
            }
            mConnectionsToReset.clear();
        }
    };
    /// @}

    public CdmaConferenceController(TelephonyConnectionService connectionService) {
        mConnectionService = connectionService;
        /// M: CC: Don't allow mute in ECBM and update after exit ECBM. @{
        mReceiver = new CdmaConferenceBroadcastReceiver();
        IntentFilter intentFilter = new IntentFilter(
                TelephonyIntents.ACTION_EMERGENCY_CALLBACK_MODE_CHANGED);
        try {
            PhoneGlobals.getInstance().registerReceiver(mReceiver, intentFilter);
        } catch (IllegalStateException e) {
            Log.e(this, e, "Can't get PhoneGlobals");
        }
        /// @}
    }

    /** The CDMA conference connection object. */
    private CdmaConference mConference;

    /// M: CC: Merge to MtkGsmCdmaConnection @{
    //void add(final CdmaConnection connection) {
    void add(final MtkGsmCdmaConnection connection) {
    /// @}
        Log.d(this, "add, conn=" + connection + ", list=" + mCdmaConnections);

        if (mCdmaConnections.contains(connection)) {
            // Adding a duplicate realistically shouldn't happen.
            Log.w(this, "add - connection already tracked; connection=%s", connection);
            return;
        }

        if (!mCdmaConnections.isEmpty() && connection.isOutgoing()) {
            // There already exists a connection, so this will probably result in a conference once
            // it is added. For outgoing connections which are added while another connection
            // exists, we mark them as "dialing" for a set amount of time to give the user time to
            // see their new call as "Dialing" before it turns into a conference call.
            // During that time, we also mark the other calls as "held" or else it can cause issues
            // due to having an ACTIVE and a DIALING call simultaneously.
            connection.forceAsDialing(true);

            /// M: CC: CDMA second call. @{
            // Set the second MO call to active immediately when receive waiting call
            mSecondCall = connection;
            mConnectionsToReset.clear();
            for (MtkGsmCdmaConnection current : mCdmaConnections) {
                Log.d(this, "current's state=" + current.getState());
                if (current.setHoldingForConference()) {
                    mConnectionsToReset.add(current);
                } else {
                    Log.d(this, "Fail to setHoldingForConference");
                }
            }
            Log.d(this, "Add second connection, mConnectionsToReset:" + mConnectionsToReset);
            mHandler.postDelayed(mDelayRunnable, ADD_OUTGOING_CONNECTION_DELAY_MILLIS);
            /// @}

        /// M: CC: CDMA second hold @{
        // After added the fake HOLDING status, before answer the waiting call,
        // we make sure the HOLDING call can't change to ACTIVE:
        // a) Add listenter to the waiting connection;
        // b) Once the waiting call changes to active, this means the call be answered;
        // c) Reset the holding call to active;
        // d) Add and notify the conference call to telecom.
        } else if (!mCdmaConnections.isEmpty() && connection.isCallWaiting()) {
            Log.d(this, "Waiting call arrives, mSecondCall=" + mSecondCall
                    + ", hasCallbacks=" + mHandler.hasCallbacks(mDelayRunnable));
            if (mSecondCall != null && mHandler.hasCallbacks(mDelayRunnable)) {
                Log.d(this, "Merge the second call now");
                mHandler.removeCallbacks(mDelayRunnable);
                mSecondCall.forceAsDialing(false);
                addInternal(mSecondCall);
                mSecondCall = null;
                for (MtkGsmCdmaConnection current : mConnectionsToReset) {
                    current.resetStateForConference();
                }
                mConnectionsToReset.clear();
            }

            Log.d(this, "add waiting call connection listenner.");
            connection.addConnectionListener(new Connection.Listener() {
                public void onStateChanged(final Connection c, int state) {
                    Log.d(CdmaConferenceController.this, "Waiting call=" + c + ", state=" + state
                            + ", mCdmaConnections.size=" + mCdmaConnections.size());
                    if (state == Connection.STATE_ACTIVE) {
                        c.removeConnectionListener(this);
                        mHandler.postDelayed(new Runnable() {
                            public void run() {
                                addInternal((MtkGsmCdmaConnection) c);
                            }
                        }, ADD_WAITING_CONNECTION_DELAY_MILLIS);
                    } else if (state == Connection.STATE_DISCONNECTED) {
                        c.removeConnectionListener(this);
                    }
                }
            });
        /// @}
        } else {
            // Post the call to addInternal to the handler with no delay.
            // Why you ask?  In TelephonyConnectionService#
            // onCreateIncomingConnection(PhoneAccountHandle, ConnectionRequest) or
            // TelephonyConnectionService#onCreateOutgoingConnection(PhoneAccountHandle,
            // ConnectionRequest) we can create a new connection it will trigger a call to
            // TelephonyConnectionService#addConnectionToConferenceController, which will cause us
            // to get here.  HOWEVER, at this point ConnectionService#addConnection has not yet run,
            // so if we end up calling ConnectionService#addConference, the connection service will
            // not yet know about the new connection, so it won't get added to the conference.
            // Posting to the handler ensures addConnection has a chance to happen before we add the
            // conference.
            mHandler.post(() -> addInternal(connection));
        }
    }

    /// M: CC: Merge to MtkGsmCdmaConnection @{
    //private void addInternal(CdmaConnection connection) {
    private void addInternal(MtkGsmCdmaConnection connection) {
    /// @}
        mCdmaConnections.add(connection);
        connection.addConnectionListener(mConnectionListener);
        recalculateConference();
    }

    /// M: CC: Merge to MtkGsmCdmaConnection @{
    //void remove(CdmaConnection connection) {
    void remove(MtkGsmCdmaConnection connection) {
    /// @}
        if (!mCdmaConnections.contains(connection)) {
            // Debug only since TelephonyConnectionService tries to clean up the connections tracked
            // when the original connection changes.  It does this proactively.
            Log.d(this, "remove - connection not tracked; connection=%s", connection);
            return;
        }

        connection.removeConnectionListener(mConnectionListener);
        mCdmaConnections.remove(connection);
        recalculateConference();
    }

    private void recalculateConference() {
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        //List<CdmaConnection> conferenceConnections = new ArrayList<>(mCdmaConnections.size());
        //for (CdmaConnection connection : mCdmaConnections) {
        List<MtkGsmCdmaConnection> conferenceConnections = new ArrayList<>(mCdmaConnections.size());
        for (MtkGsmCdmaConnection connection : mCdmaConnections) {
        /// @}
            // We do not include call-waiting calls in conferences.
            /// M: CC: Fix timing issue. Don't include ringing calls in conferences.
            if (!connection.isCallWaiting() && connection.getState() != Connection.STATE_RINGING &&
                    connection.getState() != Connection.STATE_DISCONNECTED) {
                conferenceConnections.add(connection);
            }
        }

        Log.d(this, "recalculating conference calls %d", conferenceConnections.size());
        /// M: CC: For CDMA conference
        Log.d(this, "mConfConnCount=" + mConfConnCount);

        if (conferenceConnections.size() >= 2) {
            boolean isNewlyCreated = false;

            /// M: CC: Merge to MtkGsmCdmaConnection @{
            //CdmaConnection newConnection = mCdmaConnections.get(mCdmaConnections.size() - 1);
            MtkGsmCdmaConnection newConnection = mCdmaConnections.get(mCdmaConnections.size() - 1);
            /// }@

            // There are two or more CDMA connections. Do the following:
            // 1) Create a new conference connection if it doesn't exist.
            if (mConference == null) {
                Log.i(this, "Creating new Cdma conference call");
                PhoneAccountHandle phoneAccountHandle =
                        PhoneUtils.makePstnPhoneAccountHandle(newConnection.getPhone());
                mConference = new CdmaConference(phoneAccountHandle);
                isNewlyCreated = true;

                /// M: CC: Set the conference connected time. @{
                Log.d(this, "First conn=" + mCdmaConnections.get(0));
                if (mCdmaConnections.get(0).getOriginalConnection() != null) {
                    mConference.setConnectTimeMillis(
                            mCdmaConnections.get(0).getOriginalConnection().getConnectTime());
                } else {
                    Log.d(this, "First connection's original connection is null!");
                }
                /// @}
            }

            if (newConnection.isOutgoing()) {
                // Only an outgoing call can be merged with an ongoing call.
                /// M: CC: Update the conference's capabilities. @{
                // Only add CAPABILITY_MERGE_CONFERENCE for new created outgoing connection
                if (mConfConnCount == conferenceConnections.size()) {
                    Log.d(this, "The conference call has been merged, so do nothing.");
                } else {
                    mConference.updateCapabilities(Connection.CAPABILITY_MERGE_CONFERENCE);
                    mConference.removeCapabilities(Connection.CAPABILITY_SWAP_CONFERENCE);
                    Log.d(this, "Update merge capability");
                }
                /// @}
            } else {
                // If the most recently added connection was an incoming call, enable
                // swap instead of merge.
                mConference.updateCapabilities(Connection.CAPABILITY_SWAP_CONFERENCE);
                /// M: CC: Update the conference's capabilities. @{
                mConference.removeCapabilities(Connection.CAPABILITY_MERGE_CONFERENCE);
                Log.d(this, "Update swap capability");
                /// @}
            }

            // 2) Add any new connections to the conference
            /// M: CC: Set conference to active when adding new connection.
            boolean addNewConnection = false;
            List<Connection> existingChildConnections =
                    new ArrayList<>(mConference.getConnections());
            /// M: CC: Merge to MtkGsmCdmaConnection @{
            //for (CdmaConnection connection : conferenceConnections) {
            for (MtkGsmCdmaConnection connection : conferenceConnections) {
            /// @}
                if (!existingChildConnections.contains(connection)) {
                    Log.i(this, "Adding connection to conference call: %s", connection);
                    mConference.addConnection(connection);
                    /// M: CC: Set conference to active when adding new connection.
                    addNewConnection = true;
                }
                existingChildConnections.remove(connection);
            }

            // 3) Remove any lingering old/disconnected/destroyed connections
            for (Connection oldConnection : existingChildConnections) {
                mConference.removeConnection(oldConnection);
                Log.i(this, "Removing connection from conference call: %s", oldConnection);
            }

            // 4) Add the conference to the connection service if it is new.
            if (isNewlyCreated) {
                Log.d(this, "Adding the conference call");
                mConference.updateCallRadioTechAfterCreation();
                /// M: CC: Reset connection's state when creating new conference.
                if (MTK_SVLTE_SUPPORT) {
                    mConference.resetConnectionState();
                }
                mConnectionService.addConference(mConference);
            /// M: CC: Set conference to active when adding new connection. @{
            } else if (addNewConnection) {
                mConference.setActive();
                mConference.resetConnectionState();
            /// @}
            }
            /// M: CC: Update the connection capabilities. @{
            if (mConference.getConnectionCapabilities() !=
                     mConference.buildConnectionCapabilities()) {
                // Give some time for telecom knows the conference call.
                mHandler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        if (mConference != null) {
                            mConference.updateConnectionCapabilities();
                        }
                    }
                }, UPDATE_CALL_CAPABILITIE_DELAY_MILLIS);
            }
            /// @}
        } else if (conferenceConnections.isEmpty()) {
            // There are no more connection so if we still have a conference, lets remove it.
            if (mConference != null) {
                Log.i(this, "Destroying the CDMA conference connection.");
                mConference.destroy();
                mConference = null;

                /// M: CC: CDMA second call. @{
                mSecondCall = null;
                mConnectionsToReset.clear();
                /// @}
            }
        }
        /// M: CC: Save the current connection size.
        mConfConnCount = conferenceConnections.size();
    }

    /// M: CC: Don't allow mute in ECBM and update after exit ECBM @{
    /**
     * Receive the ECM change intent.
     */
    private class CdmaConferenceBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (TelephonyIntents.ACTION_EMERGENCY_CALLBACK_MODE_CHANGED.equals(action)) {
                boolean isEcm = intent.getBooleanExtra("phoneinECMState", false);
                Log.d(CdmaConferenceController.this, "Received ECM changed, isEcm=" + isEcm);
                if (mConference != null) {
                    mConference.updateConnectionCapabilities();
                } else {
                    for (MtkGsmCdmaConnection current : mCdmaConnections) {
                        current.updateConnectionCapabilities();
                    }
                }
            }
        }
    }

    public void onDestroy() {
        try {
            PhoneGlobals.getInstance().unregisterReceiver(mReceiver);
        } catch (IllegalStateException e) {
            Log.e(this, e, "onDestroy, can't get PhoneGlobals");
        }
    }
    /// @}

    /// M: CC: To check if a CDMA conference will be created. @{
    boolean hasDelayedConferenceCreation() {
        return mHandler.hasCallbacks(mDelayRunnable);
    }
    /// @}
}

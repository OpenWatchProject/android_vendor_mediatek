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
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.telecom.Conference;
import android.telecom.Connection;
import android.telecom.PhoneAccountHandle;
import android.telephony.CarrierConfigManager;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.Phone;
import com.android.phone.PhoneGlobals;
import com.mediatek.services.telephony.MtkGsmCdmaConnection;

import mediatek.telecom.MtkConnection;

import java.util.List;

/**
 * CDMA-based conference call.
 */
public class CdmaConference extends Conference implements Holdable {
    private int mCapabilities;
    private int mProperties;
    private boolean mIsHoldable;

    public CdmaConference(PhoneAccountHandle phoneAccount) {
        super(phoneAccount);
        setActive();

        mProperties = Connection.PROPERTY_GENERIC_CONFERENCE;
        mProperties |= MtkConnection.PROPERTY_CDMA;
        setConnectionProperties(mProperties);

        mIsHoldable = false;
    }

    public void updateCapabilities(int capabilities) {
        capabilities |= Connection.CAPABILITY_MUTE;
        /// M: CC: Build connection capabilities. @{
        mCapabilities |= capabilities;
        capabilities = buildConnectionCapabilities();
        /// @}
        setConnectionCapabilities(capabilities);
    }

    /**
     * Invoked when the Conference and all it's {@link Connection}s should be disconnected.
     */
    @Override
    public void onDisconnect() {
        Call call = getOriginalCall();
        if (call != null) {
            Log.d(this, "Found multiparty call to hangup for conference.");
            try {
                call.hangup();
            } catch (CallStateException e) {
                Log.e(this, e, "Exception thrown trying to hangup conference");
            }
        }
    }

    @Override
    public void onSeparate(Connection connection) {
        Log.e(this, new Exception(), "Separate not supported for CDMA conference call.");
    }

    @Override
    public void onHold() {
        /// M: CC: CDMA call fake hold handling. @{
        Log.d(this, "onHold, just set the hold status.");
        mHandler.sendMessageDelayed(
                Message.obtain(mHandler, MSG_CDMA_CALL_SWITCH, FAKE_HOLD, 0),
                MSG_CDMA_CALL_SWITCH_DELAY);
        /// @}
    }

    /**
     * Invoked when the conference should be moved from hold to active.
     */
    @Override
    public void onUnhold() {
        /// M: CC: CDMA call fake hold handling. @{
        Log.d(this, "onUnhold, just set the unhold status.");
        mHandler.sendMessageDelayed(
                Message.obtain(mHandler, MSG_CDMA_CALL_SWITCH, FAKE_UNHOLD, 0),
                MSG_CDMA_CALL_SWITCH_DELAY);
        /// @}
    }

    @Override
    public void onMerge() {
        Log.i(this, "Merging CDMA conference call.");
        // Can only merge once
        mCapabilities &= ~Connection.CAPABILITY_MERGE_CONFERENCE;
        // Once merged, swap is enabled.
        if (isSwapSupportedAfterMerge()){
            mCapabilities |= Connection.CAPABILITY_SWAP_CONFERENCE;
        }
        updateCapabilities(mCapabilities);
        sendFlash();

        /// M: CC: CDMA call fake hold handling. @{
        // If the call in HOLDING status before merging, unhold it
        // to avoid the call keeping holding status but no "unhold" button
        // to recover it.
        if (getState() == Connection.STATE_HOLDING) {
            onUnhold();
        }
        /// @}
    }

    @Override
    public void onPlayDtmfTone(char c) {
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        //final CdmaConnection connection = getFirstConnection();
        final MtkGsmCdmaConnection connection = getFirstConnection();
        /// @}
        if (connection != null) {
            connection.onPlayDtmfTone(c);
        } else {
            Log.w(this, "No CDMA connection found while trying to play dtmf tone.");
        }
    }

    @Override
    public void onStopDtmfTone() {
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        //final CdmaConnection connection = getFirstConnection();
        final MtkGsmCdmaConnection connection = getFirstConnection();
        /// @}
        if (connection != null) {
            connection.onStopDtmfTone();
        } else {
            Log.w(this, "No CDMA connection found while trying to stop dtmf tone.");
        }
    }

    @Override
    public void onSwap() {
        Log.i(this, "Swapping CDMA conference call.");
        sendFlash();
    }

    private void sendFlash() {
        Call call = getOriginalCall();
        if (call != null) {
            try {
                // For CDMA calls, this just sends a flash command.
                call.getPhone().switchHoldingAndActive();
            } catch (CallStateException e) {
                Log.e(this, e, "Error while trying to send flash command.");
            }
        }
    }

    private Call getMultipartyCallForConnection(Connection connection) {
        com.android.internal.telephony.Connection radioConnection =
                getOriginalConnection(connection);
        if (radioConnection != null) {
            Call call = radioConnection.getCall();
            if (call != null && call.isMultiparty()) {
                return call;
            }
        }
        return null;
    }

    private Call getOriginalCall() {
        List<Connection> connections = getConnections();
        if (!connections.isEmpty()) {
            com.android.internal.telephony.Connection originalConnection =
                    getOriginalConnection(connections.get(0));
            if (originalConnection != null) {
                return originalConnection.getCall();
            }
        }
        return null;
    }

    /**
     * Return whether network support swap after merge conference call.
     *
     * @return true to support, false not support.
     */
    private final boolean isSwapSupportedAfterMerge()
    {
        boolean supportSwapAfterMerge = true;
        Context context = PhoneGlobals.getInstance();

        if (context != null) {
            CarrierConfigManager configManager = (CarrierConfigManager) context.getSystemService(
                    Context.CARRIER_CONFIG_SERVICE);
            /// M: CC: Get the config of the current phone @{
            //PersistableBundle b = configManager.getConfig();
            PersistableBundle b = null;
            Call call = getOriginalCall();
            if (call != null && call.getPhone() != null) {
                b = configManager.getConfigForSubId(call.getPhone().getSubId());
            }
            /// @}
            if (b != null) {
                supportSwapAfterMerge =
                        b.getBoolean(CarrierConfigManager.KEY_SUPPORT_SWAP_AFTER_MERGE_BOOL);
                Log.d(this, "Current network support swap after call merged capability is "
                        + supportSwapAfterMerge);
            }
        }
        return supportSwapAfterMerge;
    }

    private com.android.internal.telephony.Connection getOriginalConnection(Connection connection) {
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        //if (connection instanceof CdmaConnection) {
        //    return ((CdmaConnection) connection).getOriginalConnection();
        if (connection instanceof MtkGsmCdmaConnection) {
            return ((MtkGsmCdmaConnection) connection).getOriginalConnection();
        /// @}
        } else {
            Log.e(this, null, "Non CDMA connection found in a CDMA conference");
            return null;
        }
    }

    /// M: CC: Merge to MtkGsmCdmaConnection @{
    //private CdmaConnection getFirstConnection() {
    private MtkGsmCdmaConnection getFirstConnection() {
    /// @}
        final List<Connection> connections = getConnections();
        if (connections.isEmpty()) {
            return null;
        }
        /// M: CC: Merge to MtkGsmCdmaConnection @{
        //return (CdmaConnection) connections.get(0);
        return (MtkGsmCdmaConnection) connections.get(0);
        /// @}
    }

    @Override
    public void setHoldable(boolean isHoldable) {
        // Since the CDMA-based conference can't not be held, dont update the capability when this
        // method called.
        mIsHoldable = isHoldable;
    }

    @Override
    public boolean isChildHoldable() {
        // The conference can not be a child of other conference.
        return false;
    }

    /// M: CC: CDMA call fake hold handling. @{
    private static final int MSG_CDMA_CALL_SWITCH = 3;
    private static final int MSG_CDMA_CALL_SWITCH_DELAY = 200;
    private static final int FAKE_HOLD = 1;
    private static final int FAKE_UNHOLD = 0;
    private static final boolean MTK_SVLTE_SUPPORT =
            "1".equals(android.os.SystemProperties.get("ro.vendor.mtk_c2k_lte_mode"));

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_CDMA_CALL_SWITCH:
                    handleFakeHold(msg.arg1);
                    break;
                default:
                    break;
            }
        }
    };

    private void handleFakeHold(int fakeOp) {
        Log.d(this, "handleFakeHold, operation=", fakeOp);
        if (FAKE_HOLD == fakeOp) {
            setOnHold();
        } else if (FAKE_UNHOLD == fakeOp) {
            setActive();
        }
        resetConnectionState();
        updateCapabilities(mCapabilities);
    }

    public void resetConnectionState() {
        int state = getState();
        if (state != Connection.STATE_ACTIVE && state != Connection.STATE_HOLDING) {
            return;
        }

        List<Connection> conns = getConnections();
        for (Connection c : conns) {
            if (c.getState() != state) {
                if (state == Connection.STATE_ACTIVE) {
                    c.setActive();
                } else {
                    c.setOnHold();
                }
                if (c instanceof MtkGsmCdmaConnection) {
                    MtkGsmCdmaConnection cc = (MtkGsmCdmaConnection) c;
                    cc.updateState();
                }
            }
        }
    }
    /// @}

    /// M: CC: Build connection's capabilities. @{
    protected int buildConnectionCapabilities() {
        Log.d(this, "buildConnectionCapabilities");

        if (getConnections() == null || getConnections().size() == 0) {
            Log.d(this, "No connection exist, do nothing.");
            return mCapabilities;
        }

        boolean inEcm = false;
        Call call = getOriginalCall();
        if (call != null) {
            Phone phone = call.getPhone();
            if (phone != null) {
                inEcm = phone.isInEcm();
            }
        }

        // Don't allow mute in ECBM and update after exit ECBM.
        if (!inEcm) {
            mCapabilities |= Connection.CAPABILITY_MUTE;
        } else {
            mCapabilities &= ~Connection.CAPABILITY_MUTE;
        }

        /// M: CC: CDMA call fake hold handling. @{
        if (MTK_SVLTE_SUPPORT) {
            mCapabilities |= Connection.CAPABILITY_SUPPORT_HOLD;
            if (getState() == Connection.STATE_ACTIVE
                    || getState() == Connection.STATE_HOLDING) {
                mCapabilities |= Connection.CAPABILITY_HOLD;
            }
        }
        /// @}

        Log.d(this, MtkConnection.capabilitiesToString(mCapabilities));
        return mCapabilities;
    }
    /// @}

    /// M: CC: Update connection's capabilities. @{
    protected void updateConnectionCapabilities() {
        int newConnectionCapabilities = buildConnectionCapabilities();
        setConnectionCapabilities(newConnectionCapabilities);
    }
    /// @}

    /// M: CC: Remove conference's capabilities. @{
    protected void removeCapabilities(int capabilities) {
        mCapabilities &= ~capabilities;
        setConnectionCapabilities(buildConnectionCapabilities());
    }
    /// @}

    /// M: CC: HangupAll for FTA 31.4.4.2 @{
    /**
     * Hangup all connections in the conference.
     * CDMA hangup all is different to GSM, no CHLD=6 to hang up all calls in a phone
     */
    public void onHangupAll() {
        Log.d(this, "onHangupAll");
        if (getFirstConnection() != null) {
            try {
                Call call = getFirstConnection().getOriginalConnection().getCall();
                if (call != null) {
                    call.hangup();
                } else {
                    Log.w(this, "call is null.");
                }
            } catch (CallStateException e) {
                Log.e(this, e, "Failed to hangup the call.");
            }
        }
    }
    /// @}

}

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

package com.mediatek.op.wifi;

import android.content.Context;
import android.content.pm.PackageManager;
import android.net.NetworkInfo;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Message;
import android.os.Process;
import android.os.UserHandle;

import com.android.internal.util.State;
import com.android.server.wifi.NetworkUpdateResult;
import com.android.server.wifi.ScanDetail;
import com.android.server.wifi.WifiConfigManager;
import com.android.server.wifi.WifiInjector;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.List;

import static android.net.wifi.WifiConfiguration.INVALID_NETWORK_ID;
import static com.android.internal.util.StateMachine.HANDLED;
import static com.android.internal.util.StateMachine.NOT_HANDLED;

public class AutoConnectManager {
    private static final String TAG = "AutoConnectManager";

    private static final String CONNECT_MODE_STATE = "ConnectModeState";

    private static final String WPS_RUNNING_STATE = "WpsRunningState";

    private static final String SYSUI_PACKAGE_NAME = "com.android.systemui";

    private int mSystemUiUid = -1;

    private List<ScanResult> mScanResults = null;

    private List<Integer> mDisconnectNetworks = new ArrayList<>();

    // this flag indicates this scan is triggered by bad RSSI/passive disconnection, it's for
    // network reselection
    private boolean mScanForWeakSignal = false;

    // record contains the last disconnected network (passive disconnection/disconnection due to
    // bad RSSI)
    private int mDisconnectNetworkId = INVALID_NETWORK_ID;

    // indicate whether supplicant is connecting, it's updated in handleSupplicantStateChange()
    private boolean mIsConnecting = false;

    // indicates whether the last disconnection is an active disconnection, will be reset to
    // false when:
    // * wifi is re-enabled
    // * a network is connected
    // * it is consumed in handleScanResults
    private boolean mDisconnectOperation = false;

    private boolean mWaitForScanResult = false;

    // log last weak signal checking time to prevent excessive scan trigger by weak signal
    //  handle codes
    private long mLastCheckWeakSignalTime = 0;

    private int mLastNetworkId = INVALID_NETWORK_ID;

    private Context mContext;

    private Op01WifiService mExt;

    private ClientModeAdapter mClientModeAdapter;

    private NetworkInfo.DetailedState mNetworkState = NetworkInfo.DetailedState.IDLE;

    public AutoConnectManager(Context context, Op01WifiService ext, ClientModeAdapter adapter) {
        mContext = context;
        mExt = ext;
        mClientModeAdapter = adapter;

        try {
            mSystemUiUid = context.getPackageManager().getPackageUidAsUser(
                    SYSUI_PACKAGE_NAME,
                    PackageManager.MATCH_SYSTEM_ONLY,
                    UserHandle.USER_SYSTEM);
        } catch (PackageManager.NameNotFoundException e) {
            log("Unable to resolve SystemUI's UID.");
        }
    }

    public void setLastNetworkId(int id) {
        mLastNetworkId = id;
    }

    public void setNetworkState(NetworkInfo.DetailedState state) {
        mNetworkState = state;
    }

    public NetworkInfo.DetailedState getNetworkState() {
        return mNetworkState;
    }

    public void addDisconnectNetwork(int netId) {
        log("addDisconnectNetwork: " + netId);
        synchronized (mDisconnectNetworks) {
            mDisconnectNetworks.add(netId);
        }
    }

    public void removeDisconnectNetwork(int netId) {
        log("removeDisconnectNetwork: " + netId);
        synchronized (mDisconnectNetworks) {
            mDisconnectNetworks.remove((Integer) netId);
        }
    }

    public void clearDisconnectNetworks() {
        log("clearDisconnectNetworks");
        synchronized (mDisconnectNetworks) {
            mDisconnectNetworks.clear();
        }
    }

    public List<Integer> getDisconnectNetworks() {
        List<Integer> networks = new ArrayList<>();
        synchronized (mDisconnectNetworks) {
            for (Integer netId : mDisconnectNetworks) {
                networks.add(netId);
            }
        }
        return networks;
    }

    private void disableNetwork(int networkId) {
        if (networkId != INVALID_NETWORK_ID) {
            sendDisableMessage(networkId);
        } else {
            log("disableNetwork networkId " + networkId + " invalid, do nothing");
        }
    }

    private void enableNetwork(int networkId) {
        if (networkId != INVALID_NETWORK_ID) {
            sendEnableMessage(networkId, false);
        } else {
            log("enableNetwork networkId " + networkId + " invalid, do nothing");
        }
    }

    public boolean getScanForWeakSignal() {
        return mScanForWeakSignal;
    }

    public void setScanForWeakSignal(boolean value) {
        log("setScanForWeakSignal: " + mScanForWeakSignal + " -> " + value);
        mScanForWeakSignal = value;
    }

    public void setWaitForScanResult(boolean value) {
        log("setWaitForScanResult: " + mWaitForScanResult + " -> " + value);
        mWaitForScanResult = value;
    }

    private boolean getDisconnectOperation() {
        return mDisconnectOperation;
    }

    private void setDisconnectOperation(boolean value) {
        log("setDisconnectOperation: " + mDisconnectOperation + " -> " + value);
        // TODO: debug only, remove ??
        new Throwable().printStackTrace();
        mDisconnectOperation = value;
    }

    public void showReselectionDialog() {
        // Resume the flag
        setScanForWeakSignal(false);
        log("showReselectionDialog mDisconnectNetworkId:" + mDisconnectNetworkId);

        if (isConnectTypeAuto()) {
            log("Auto connect, do nothing");
        } else {
            // Make sure at least one (exclude disconnect) left after passive disconnect
            int networkId = getHighPriorityNetworkId();
            if (networkId == INVALID_NETWORK_ID) {
                log("No valid network exclude disconnect network, do nothing");
                return;
            }

            if (mExt.shouldReselect()) {
                Op01DataUtils.recordLastDataStatus(mContext);
                Op01WifiUtils.showReselectDialog(mContext);
            } else {
                log("Should not show reselect dialog");
            }
        }
    }

    // TODO: priority phaseout, revise??
    private int getHighPriorityNetworkId() {
        WifiConfigManager wcm = WifiInjector.getInstance().getWifiConfigManager();
        List<WifiConfiguration> networks = wcm.getSavedNetworks(Process.WIFI_UID);
        if (networks == null || networks.size() == 0) {
            log("ACM: getHighPriorityNetworkId No configured networks");
            return INVALID_NETWORK_ID;
        }
        LinkedList<WifiConfiguration> found = new LinkedList<>();
        if (mScanResults != null && !mScanResults.isEmpty()) {
            for (WifiConfiguration network : networks) {
                if (network.networkId != mDisconnectNetworkId) {
                    log("ACM: getHighPriorityNetworkId iterate scan result cache");
                    for (ScanResult scanResult : mScanResults) {
                        if ((network.SSID != null) &&
                                (scanResult.SSID != null) &&
                                network.SSID.equals("\"" + scanResult.SSID + "\"") &&
                                Op01WifiUtils.getSecurity(network) == Op01WifiUtils.getSecurity(scanResult) &&
                                scanResult.level > Op01WifiUtils.BEST_SIGNAL_THRESHOLD) {
                            log("ACM: add network to found: " + network.networkId);
                            found.add(network);
                            break;
                        }
                    }
                    log("ACM: getHighPriorityNetworkId iterate scan result cache done");
                } else {
                    log("ACM: ignore disconnect network " + mDisconnectNetworkId);
                }
            }
        }

        log("ACM: found.size() = " + found.size());
        if (found.isEmpty()) {
            log("ACM: getHighPriorityNetworkId Configured networks empty");
            return INVALID_NETWORK_ID;
        }

        WifiConfiguration result = Collections.max(found, new Comparator<Object>() {
            public int compare(Object obj1, Object obj2) {
                WifiConfiguration n1 = (WifiConfiguration) obj1;
                WifiConfiguration n2 = (WifiConfiguration) obj2;
                return n2.priority - n1.priority;
            }
        });

        log("Found the highest priority AP, networkId:" + result.networkId);
        return result.networkId;
    }

    public void clearDisconnectNetworkId() {
        mDisconnectNetworkId = INVALID_NETWORK_ID;
    }

    public void handleScanResults(List<ScanDetail> full, List<ScanDetail> unsaved) {
        log("ACM: handleScanResults enter");
        // update scan results cache
        mScanResults = new ArrayList<>();
        for (ScanDetail detail : full) {
            mScanResults.add(detail.getScanResult());
        }
        log("ACM: handleScanResults scan results cache updated");

        log("ACM: handleScanResults getDisconnectOperation " + getDisconnectOperation());
        if (getDisconnectOperation()) {
            setDisconnectOperation(false);
            if (showDataDialogIfNeed()) {
                log("ACM: handleScanResults exit (show DataDialog)");
                return;
            }
        }

        log("ACM: unsaved size " + unsaved.size());
        if (unsaved.isEmpty()) {
            showDataDialogIfNeed();
        } else {
            if (isWifiConnecting()) {
                return;
            } else {
                showDataDialogIfNeed();
            }
        }
        log("ACM: handleScanResults exit");
    }

    private boolean showDataDialogIfNeed() {
        if (mWaitForScanResult) {
            mWaitForScanResult = false;

            if (!Op01WifiUtils.isTopReselectDialog(mContext) && isDataAvailable()) {
                Op01DataUtils.recordLastDataStatus(mContext);
                turnOffDataConnection();
                sendWifiFailoverGprsDialog();
                return true;
            } else {
                log("[showDataDialogIfNeed] top reselect dialog or data off, do nothing");
            }
        }
        return false;
    }

    private boolean isWifiConnecting() {
        WifiInfo wifiInfo = getClientModeAdapter().getWifiInfo();
        int networkId = mIsConnecting ? wifiInfo.getNetworkId() : INVALID_NETWORK_ID;
        return (mExt.isWifiConnecting(networkId, getDisconnectNetworks()) ||
                isWpsRunning());
    }

    public void setIsConnecting(boolean value) {
        mIsConnecting = value;
    }

    public void resetStates() {
        log("resetStates");
        mDisconnectOperation = false;
        mScanForWeakSignal = false;
        mWaitForScanResult = false;
        mLastCheckWeakSignalTime = 0;
        mIsConnecting = false;
        resetDisconnectNetworkStates();
    }

    public void resetDisconnectNetworkStates() {
        log("resetDisconnectNetworkStates");
        if (!isConnectTypeAuto()) {
            // disable all networks to prevent auto connection in manual/ask mode
            disableAllNetworks(false);
        } else {
            enableNetworks(getDisconnectNetworks());
        }
        clearDisconnectNetworks();
    }

    private void enableNetworks(List<Integer> networks) {
        if (networks != null) {
            for (int netId : networks) {
                log("enableNetwork: " + netId);
                enableNetwork(netId);
            }
        }
    }

    private void disableAllNetworks(boolean exceptLastNetwork) {
        log("disableAllNetworks, exceptLastNetwork:" + exceptLastNetwork);
        WifiConfigManager wcm = WifiInjector.getInstance().getWifiConfigManager();
        List<WifiConfiguration> networks = wcm.getSavedNetworks(Process.WIFI_UID);
        if (exceptLastNetwork) {
            if (null != networks) {
                for (WifiConfiguration network : networks) {
                    int lastNetworkId = getClientModeAdapter().getLastNetworkId();
                    if (network.networkId != lastNetworkId &&
                            network.status != WifiConfiguration.Status.DISABLED) {
                        disableNetwork(network.networkId);
                    }
                }
            }
        } else {
            if (null != networks) {
                for (WifiConfiguration network : networks) {
                    if (network.status != WifiConfiguration.Status.DISABLED) {
                        disableNetwork(network.networkId);
                    }
                }
            }
        }
    }

    public void updateRSSI(Integer newRssi, int ipAddr, int lastNetworkId) {
        if (newRssi != null && newRssi < Op01WifiUtils.WEAK_SIGNAL_THRESHOLD) {
            long time = android.os.SystemClock.elapsedRealtime();
            boolean autoConnect = isConnectTypeAuto();
            log("fetchRssi, ip:" + ipAddr
                    + ", mDisconnectOperation:" + mDisconnectOperation
                    + ", time:" + time + ", lasttime:" + mLastCheckWeakSignalTime);
            final long lastCheckInterval = time - mLastCheckWeakSignalTime;
            if ((ipAddr != 0 &&
                    !mDisconnectOperation &&
                    lastCheckInterval > Op01WifiUtils.MIN_INTERVAL_CHECK_WEAK_SIGNAL_MS) ||
                    (autoConnect &&
                            lastCheckInterval > Op01WifiUtils.MIN_INTERVAL_SCAN_SUPRESSION_MS)) {
                log("Rssi < -85, scan to check signal!");
                mLastCheckWeakSignalTime = time;
                mDisconnectNetworkId = lastNetworkId;
                setScanForWeakSignal(true);
                getClientModeAdapter().startScan();
            }
        }
    }

    public void handleNetworkDisconnect() {
        log("handleNetworkDisconnect, oldState:" +
                mNetworkState + ", mDisconnectOperation:" + getDisconnectOperation());
        if (mNetworkState == NetworkInfo.DetailedState.CONNECTED) {
            // record the disconnected network
            mDisconnectNetworkId = mLastNetworkId;
            if (!getDisconnectOperation()) {
                // this is not an active disconnection, treat this as a weak signal case too
                setScanForWeakSignal(true);
                getClientModeAdapter().startScan();
            }
        }

        // in manual/ask mode, all network should be disabled except the current connected one.
        // disable the disconnected network to prevent auto reconnecting to it
        if (!isConnectTypeAuto()) {
            disableNetwork(mLastNetworkId);
        }

        // reset this timestamp
        mLastCheckWeakSignalTime = 0;
    }


    public void handleWifiDisconnect() {
        log("handleWifiDisconnect");

        int isAsking = Op01WifiUtils.getRemindType(mContext);
        if (isAsking != Op01WifiUtils.WIFI_CONNECT_REMINDER_ALWAYS) {
            log("Not ask mode");
            return;
        }

        boolean dataAvailable = isPsDataAvailable();
        log("dataAvailable: " + dataAvailable);
        if (!dataAvailable) {
            return;
        }

        // Wifi on in settings and enabled in ClientModeImpl
        if (Op01WifiUtils.isWifiOnAndEnabled(mContext, mClientModeAdapter)) {
            log("Start scan and update waitForScanResult flag");
            Op01WifiUtils.startScan(mContext);
            mWaitForScanResult = true;

        } else {
            // Wifi off in settings or not enabled in ClientModeImpl
            Op01DataUtils.recordLastDataStatus(mContext);
            turnOffDataConnection();
            sendWifiFailoverGprsDialog();
        }
    }

    public void updateAutoConnectSettings() {
        int lastConnectId = getClientModeAdapter().getLastNetworkId();
        boolean isConnecting = (mIsConnecting || isWpsRunning());
        log("updateAutoConnectSettings, isConnecting:" + isConnecting);
        List<WifiConfiguration> networks = WifiInjector.getInstance()
                .getWifiConfigManager()
                .getSavedNetworks(Process.WIFI_UID);
        if (null != networks) {
            if (isConnectTypeAuto()) {
                if (!isConnecting) {
                    // TODO: remove due to priority phase out??
                    Collections.sort(networks, new Comparator<WifiConfiguration>() {
                        public int compare(WifiConfiguration obj1, WifiConfiguration obj2) {
                            return obj2.priority - obj1.priority;
                        }
                    });
                    for (WifiConfiguration network : networks) {
                        if (network.networkId != lastConnectId) {
                            enableNetwork(network.networkId);
                        }
                    }
                }
            } else {
                if (!isConnecting) {
                    for (WifiConfiguration network : networks) {
                        if (network.networkId != lastConnectId
                                && network.status != WifiConfiguration.Status.DISABLED) {
                            disableNetwork(network.networkId);
                        }
                    }
                }
            }
        }
    }

    public boolean preProcessMessage(State state, Message msg) {
        int netId;

        if (!state.getName().equals(CONNECT_MODE_STATE)) {
            log("State " + state.getName() + " NOT_HANDLED");
            return NOT_HANDLED;
        }

        switch(msg.what) {
            case ClientModeAdapter.CMD_REMOVE_NETWORK:
                netId = msg.arg1;
                removeDisconnectNetwork(netId);
                if (netId == getNetworkId()) {
                    setDisconnectOperation(true);
                    setScanForWeakSignal(false);
                }
                break;
            case ClientModeAdapter.CMD_ENABLE_NETWORK:
                boolean disableOthers = msg.arg2 == 1;
                if (!disableOthers && !isConnectTypeAuto()) {
                    log("Shouldn't auto connect, ignore enable operation!");
                    replyToMessage(msg);
                    return HANDLED;
                }
                break;
            default:
                return NOT_HANDLED;
        }
        return NOT_HANDLED;
    }

    public boolean postProcessMessage(State state, Message msg, Object... args) {
        if (!state.getName().equals(CONNECT_MODE_STATE)) {
            log("State " + state.getName() + " NOT_HANDLED");
            return NOT_HANDLED;
        }

        int netId;
        WifiConfiguration config;
        switch(msg.what) {
            case ClientModeAdapter.CMD_ENABLE_NETWORK:
                netId = msg.arg1;
                boolean disableOthers = msg.arg2 == 1;
                boolean ok = (Boolean) args[0];
                if (disableOthers && ok) {
                    removeDisconnectNetwork(netId);
                    setDisconnectOperation(true);
                    setScanForWeakSignal(false);
                }
                break;

            case WifiManager.DISABLE_NETWORK:
                // check whether it's successfully disabled
                netId = msg.arg1;
                config = getWifiConfiguration(netId);
                // we need to ensure this network was successfully disabled
                if (config != null && config.getNetworkSelectionStatus() != null
                        && !config.getNetworkSelectionStatus().isNetworkEnabled()) {
                    addDisconnectNetwork(netId);
                    if (netId == getNetworkId()) {
                        setDisconnectOperation(true);
                        setScanForWeakSignal(false);
                    }
                }
                break;

            case WifiManager.CONNECT_NETWORK:
                netId = msg.arg1;
                NetworkUpdateResult result = (NetworkUpdateResult) args[1];
                boolean hasCredentialChanged = false;
                if (result != null) {
                    netId = result.getNetworkId();
                    hasCredentialChanged = result.hasCredentialChanged();
                }
                if (getNetworkId() != netId || hasCredentialChanged) {
                    // set this flag because we will disconnect current network due to
                    // either user specified a different network or the credential change
                    // forced us to do a reconnect
                    setDisconnectOperation(true);
                }
                setScanForWeakSignal(false);
                removeDisconnectNetwork(netId);
                break;

            case WifiManager.FORGET_NETWORK:
                // check whether the network has been deleted
                netId = msg.arg1;
                config = getWifiConfiguration(netId);
                if (config == null) {
                    removeDisconnectNetwork(netId);
                    if (netId == getNetworkId()) {
                        setDisconnectOperation(true);
                        setScanForWeakSignal(false);
                    }
                }
                break;
            default:
                return NOT_HANDLED;
        }
        return NOT_HANDLED;
    }

    public List<ScanResult> getLatestScanResults() {
        return mScanResults;
    }

    public void triggerNetworkEvaluatorCallBack() {
        log("triggerNetworkEvaluatorCallBack");
        // 1. notify user if necessary
        if (getScanForWeakSignal()) {
            // this scan is triggered due to weak signal, auto connect
            // a valid network/show reselection dialog
            showReselectionDialog();
        }
        // reset this record which contains the last disconnected network
        // (aosp disconnection/disconnection due to poor RSSI)
        clearDisconnectNetworkId();
    }

    public void onWifStateChanged(int state) {
        setWaitForScanResult(false);

        if (state == WifiManager.WIFI_STATE_ENABLING) {
            resetDisconnectNetworkStates();

            // WiFi state is enabling, reset this flag as it could be true when WiFi disable
            // and may show reselect dialog unexpectedly if this flag is true
            setScanForWeakSignal(false);
        }
    }

    public void onNetworkStateChanged(NetworkInfo info) {
        NetworkInfo.DetailedState detailedState = info.getDetailedState();
        log("detailed state: " + detailedState);

        if (detailedState == NetworkInfo.DetailedState.CONNECTED) {
            setWaitForScanResult(false);
            setDisconnectOperation(false);

        } else if (detailedState == NetworkInfo.DetailedState.DISCONNECTED) {
            handleNetworkDisconnect();

            if (getNetworkState() == NetworkInfo.DetailedState.CONNECTED) {
                setWaitForScanResult(false);
                handleWifiDisconnect();
            }
        }
        // update states
        setNetworkState(detailedState);
        updateLastnetworkId();
    }

    private WifiConfiguration getWifiConfiguration(int netId) {
        return WifiInjector.getInstance().getWifiConfigManager().getConfiguredNetwork(netId);
    }

    private boolean isDataAvailable() {
        return Op01DataUtils.isDataAvailable(mContext);
    }

    private boolean isPsDataAvailable() {
        return Op01DataUtils.isPsDataAvailable(mContext);
    }

    private void sendWifiFailoverGprsDialog() {
        Op01DataUtils.showDataDialog(mContext);
    }

    public void turnOffDataConnection() {
        Op01DataUtils.turnOffDataConnection(mContext);
    }

    private boolean isConnectTypeAuto() {
        return mExt.isConnectTypeAuto();
    }

    private boolean isWpsRunning() {
        return WPS_RUNNING_STATE.equals(getStateName());
    }

    public void updateLastnetworkId() {
        setLastNetworkId(getClientModeAdapter().getLastNetworkId());
    }

    private int getNetworkId() {
        return getClientModeAdapter().getWifiInfo().getNetworkId();
    }

    private String getStateName() {
        return getClientModeAdapter().getCurrentState().getName();
    }

    private void replyToMessage(Message msg) {
        getClientModeAdapter().replyToMessage(msg, msg.what, ClientModeAdapter.SUCCESS);
    }

    private void sendEnableMessage(int networkId) {
        sendEnableMessage(networkId, true);
    }

    private void sendEnableMessage(int networkId, boolean disableOthers) {
        log("sendEnableMessage " + networkId + ", disableOthers " + disableOthers);
        Message message = getClientModeAdapter().obtainMessage(
                ClientModeAdapter.CMD_ENABLE_NETWORK, networkId, disableOthers ? 1 : 0);
        // must use system uid, otherwise will meet permission check fail
        message.sendingUid = mSystemUiUid;
        getClientModeAdapter().sendMessage(message);
    }

    private void sendDisableMessage(int networkId) {
        log("sendDisableMessage " + networkId);
        Message message = getClientModeAdapter().obtainMessage(
                WifiManager.DISABLE_NETWORK, networkId);
        // must use system uid, otherwise will meet permission check fail
        message.sendingUid = mSystemUiUid;
        getClientModeAdapter().sendMessage(message);
    }

    private ClientModeAdapter getClientModeAdapter() {
        return mClientModeAdapter;
    }

    private void log(String message) {
        Op01WifiUtils.log(TAG, message);
    }
}

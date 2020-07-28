package com.mediatek.op.wifi;

import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.Message;
import android.os.Process;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.util.State;
import com.android.server.wifi.ScanDetail;
import com.android.server.wifi.StateChangeResult;
import com.android.server.wifi.WifiInjector;
import com.android.server.wifi.WifiMonitor;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.server.wifi.WifiOperatorFactoryBase.DefaultMtkWifiServiceExt;

import java.util.List;

/**
 * OP01 plugin for wifi-service.
 */
public class Op01WifiService extends DefaultMtkWifiServiceExt {
    private static final String TAG = "Op01WifiService";

    private long mSwitchSuspendTime = 0;
    private long mReselectSuspendTime = 0;
    private int mConnectType;
    private int mReminderType;

    private ActivityManager mAm;
    private ConnectivityManager mCm;
    private ConnectTypeObserver mConnectTypeObserver;
    private ReminderTypeObserver mReminderTypeObserver;

    private AutoConnectManager mAutoConnectManager;

    // enable access to various internal ClientModeImpl members
    private ClientModeAdapter mClientModeAdapter = null;

    public Op01WifiService(Context context) {
        super(context);
    }

    @Override
    public void init() {
        mCm = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        mAm = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        mConnectType = Op01WifiUtils.getConnectType(mContext);
        mReminderType = Op01WifiUtils.getRemindType(mContext);
        mConnectTypeObserver = new ConnectTypeObserver(new Handler());
        mReminderTypeObserver = new ReminderTypeObserver(new Handler());

        WifiInjector injector = WifiInjector.getInstance();
        mClientModeAdapter = new ClientModeAdapter(injector.getClientModeImpl());
        mAutoConnectManager = new AutoConnectManager(mContext, this, mClientModeAdapter);

        registerWifiOpReceiver();
        registerDataConnectionReceiver();
        registerHandler(injector);

        // Receiver for settings
        registerSettingsReceiver();
    }

    private void registerWifiOpReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        intentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        intentFilter.addAction(Op01WifiUtils.ACTION_SUSPEND_NOTIFICATION_NON_SYS);
        mContext.registerReceiver(new Op01WifiOpReceiver(mAutoConnectManager), intentFilter);
    }

    private void registerDataConnectionReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Op01WifiUtils.ACTION_WIFI_FAILOVER_GPRS_DIALOG);
        mContext.registerReceiver(new DataConnectionReceiver(), intentFilter);
    }

    /**
     * register supplicant state handler
     * NOTE: this method should be invoked in an looper thread
     */
    private void registerHandler(WifiInjector injector) {
        Handler handler = new ConnectHandler(mAutoConnectManager);
        String interfaceName = mClientModeAdapter.getInterfaceName();

        WifiMonitor monitor = injector.getWifiMonitor();
        monitor.registerHandler(interfaceName, WifiMonitor.SUPPLICANT_STATE_CHANGE_EVENT, handler);
        monitor.registerHandler(interfaceName, WifiMonitor.SUP_CONNECTION_EVENT, handler);
    }

    private void registerSettingsReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        intentFilter.addAction(WifiManager.WIFI_AP_STATE_CHANGED_ACTION);
        intentFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mContext.registerReceiver(new WifiSettingsReceiver(), intentFilter);
    }

    private String makeNoise(String origin) {
        if (origin == null) {
            return null;
        }
        char [] org = origin.toCharArray();
        if (org == null) {
            return null;
        }

        String out = new String();
        for(int i=0; i<org.length; i++) {
            out += (org[i] - 'A') + ",";
        }

        return out;
    }

    public boolean isWifiConnecting(int connectingNetworkId, List<Integer> disconnectNetworks) {
        log("isWifiConnecting, mConnectType:" + mConnectType
            + ", connectingNetworkId:" + connectingNetworkId);
        boolean isConnecting = false;
        boolean autoConnect = false;
        NetworkInfo info = mCm.getActiveNetworkInfo();
        if (null == info) {
            log("No active network.");
        } else {
            log("Active network type:" + info.getTypeName());
        }

        String highestPriorityNetworkSSID = null;
        int highestPriority = -1;
        int highestPriorityNetworkId = -1;

        WifiInjector injector = WifiInjector.getInstance();
        List<WifiConfiguration> networks =
                injector.getWifiConfigManager().getSavedNetworks(Process.WIFI_UID);
        log("getSavedNetworks() returned with " + networks.size() + " networks");

        List<ScanResult> scanResults = mAutoConnectManager.getLatestScanResults();
        if (null != networks && null != scanResults) {
            for (WifiConfiguration network : networks) {
                for (ScanResult scanResult : scanResults) {
                    if ((network.SSID != null) && (scanResult.SSID != null)
                        && network.SSID.equals("\"" + scanResult.SSID + "\"")
                        && (Op01WifiUtils.getSecurity(network) == Op01WifiUtils.getSecurity(scanResult))) {
                        if (network.priority > highestPriority) {
                            highestPriority = network.priority;
                            highestPriorityNetworkId = network.networkId;
                            highestPriorityNetworkSSID = network.SSID;
                        }
                        if (network.networkId == connectingNetworkId) {
                            isConnecting = true;
                        }
                    }
                }
            }
        }
        log("highestPriorityNetworkId:" + highestPriorityNetworkId
            + ", highestPriorityNetworkSSID:" + makeNoise(highestPriorityNetworkSSID)
            + ", highestPriority:" + highestPriority
            + ", currentTimeMillis:" + System.currentTimeMillis()
            + ", mSwitchSuspendTime:" + mSwitchSuspendTime);
        if (!isConnecting) {
            if (null != info && info.getType() == ConnectivityManager.TYPE_MOBILE) {

                if (isConnectTypeAsk()) {
                    if (highestPriorityNetworkId != -1
                        && !TextUtils.isEmpty(highestPriorityNetworkSSID)
                        && isSwitchOutOfSuspend()
                        && isRemindAlways()) {

                        // check WifiNotifyDialog
                        if (!Op01WifiUtils.isTopReselectDialog(mContext)) {
                            Op01WifiUtils.showWifiNotifyDialog(mContext,
                                    highestPriorityNetworkSSID, highestPriorityNetworkId);
                        } else {
                            Log.i(TAG, "Ask mode, top is ReselectDialog, do nothing");
                        }
                    }
                } else if (isConnectTypeAuto()) {
                    highestPriorityNetworkSSID = null;
                    highestPriority = -1;
                    highestPriorityNetworkId = -1;
                    if (null != networks && null != scanResults) {
                        for (WifiConfiguration network : networks) {
                            if (!disconnectNetworks.contains(network.networkId)) {
                                for (ScanResult scanresult : scanResults) {
                                    if ((network.SSID != null) && (scanresult.SSID != null)
                                        && network.SSID.equals("\"" + scanresult.SSID + "\"")
                                        && (Op01WifiUtils.getSecurity(network)
                                            == Op01WifiUtils.getSecurity(scanresult))) {
                                        if (network.priority > highestPriority) {
                                            highestPriority = network.priority;
                                            highestPriorityNetworkId = network.networkId;
                                            highestPriorityNetworkSSID = network.SSID;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Log.d(TAG,
                          "Mobile connected, highestPriorityNetworkId:" + highestPriorityNetworkId
                        + ", highestPriorityNetworkSSID:" + makeNoise(highestPriorityNetworkSSID)
                        + ", highestPriority:" + highestPriority);
                    if (highestPriorityNetworkId != -1
                        && !TextUtils.isEmpty(highestPriorityNetworkSSID)) {
                        log("Enable all networks for mobile is connected.");
                        autoConnect = true;
                    }
                } else if (isConnectTypeManual()) {

                    if (! Op01WifiUtils.isTopWifiSettings(mAm)
                        && highestPriorityNetworkId != -1
                        && !TextUtils.isEmpty(highestPriorityNetworkSSID)
                        && isSwitchOutOfSuspend()
                        && isRemindAlways()) {

                        // check WifiNotifyDialog
                        if (!Op01WifiUtils.isTopReselectDialog(mContext)) {
                            Op01WifiUtils.showWifiNotifyDialog(mContext,
                                    highestPriorityNetworkSSID, highestPriorityNetworkId);
                        } else {
                            Log.i(TAG, "Manual mode, top is ReselectDialog, do nothing");
                        }
                    }
                }
            } else {
                if (isConnectTypeAuto()) {
                    highestPriorityNetworkSSID = null;
                    highestPriority = -1;
                    highestPriorityNetworkId = -1;

                    if (null != networks && null != scanResults) {
                        for (WifiConfiguration network : networks) {
                            if (!disconnectNetworks.contains(network.networkId)) {
                                for (ScanResult scanresult : scanResults) {
                                    if ((network.SSID != null) && (scanresult.SSID != null)
                                        && network.SSID.equals("\"" + scanresult.SSID + "\"")
                                        && (Op01WifiUtils.getSecurity(network) ==
                                            Op01WifiUtils.getSecurity(scanresult))) {
                                        if (network.priority > highestPriority) {
                                            highestPriority = network.priority;
                                            highestPriorityNetworkId = network.networkId;
                                            highestPriorityNetworkSSID = network.SSID;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Log.d(TAG,
                          "Mobile isn't connected, highestPriorityNetworkId:"
                          + highestPriorityNetworkId
                          + ", highestPriorityNetworkSSID:" + makeNoise(highestPriorityNetworkSSID)
                          + ", highestPriority:" + highestPriority);
                    if (highestPriorityNetworkId != -1
                        && !TextUtils.isEmpty(highestPriorityNetworkSSID)) {
                        log("Enable all networks for mobile is not connected.");
                        autoConnect = true;
                    }
                }
            }
        }
        Log.d(TAG,"isWifiConnecting, isConnecting:" + isConnecting
                + ", autoConnect:" + autoConnect);
        return (isConnecting || autoConnect);
    }

    public void updateSuspendTime(int type) {
        long timeMillis = System.currentTimeMillis();
        log("updateSuspendTime type " + type + " time to " + timeMillis);

        if (Op01WifiUtils.NOTIFY_TYPE_SWITCH == type) {
            mSwitchSuspendTime = timeMillis;
        } else if (Op01WifiUtils.NOTIFY_TYPE_RESELECT == type) {
            mReselectSuspendTime = timeMillis;
        }
    }

    public boolean shouldReselect() {
        log("shouldReselect mSwitchSuspendTime " + mSwitchSuspendTime
                + ", mReselectSuspendTime " + mReselectSuspendTime
                + ", mReminderType " + mReminderType);
        if (isReselectOutOfSuspend() && isRemindAlways()
                && !Op01WifiUtils.isTopReselectDialog(mAm)) {
            return true;
        }
        return false;
    }

    public boolean isConnectTypeAuto() {
        return mConnectType == MtkSettingsExt.System.WIFI_CONNECT_TYPE_AUTO;
    }

    public boolean isConnectTypeManual() {
        return mConnectType == MtkSettingsExt.System.WIFI_CONNECT_TYPE_MANUL;
    }

    public boolean isConnectTypeAsk() {
        return mConnectType == MtkSettingsExt.System.WIFI_CONNECT_TYPE_ASK;
    }

    private boolean isRemindAlways() {
        return mReminderType == Op01WifiUtils.WIFI_CONNECT_REMINDER_ALWAYS;
    }

    private boolean isSwitchOutOfSuspend() {
        return Op01WifiUtils.isOutOfSuspend(mSwitchSuspendTime);
    }

    private boolean isReselectOutOfSuspend() {
        return Op01WifiUtils.isOutOfSuspend(mReselectSuspendTime);
    }

    private static void log(String message) {
        Op01WifiUtils.log(TAG, message);
    }

    @Override
    public void handleScanResults(List<ScanDetail> full, List<ScanDetail> unsaved) {
        mAutoConnectManager.handleScanResults(full, unsaved);
    }

    @Override
    public void updateRSSI(Integer newRssi, int ipAddr, int lastNetworkId) {
        mAutoConnectManager.updateRSSI(newRssi, ipAddr, lastNetworkId);
    }

    @Override
    public boolean preProcessMessage(State state, Message msg) {
        return mAutoConnectManager.preProcessMessage(state, msg);
    }

    @Override
    public boolean postProcessMessage(State state, Message msg, Object... args) {
        return mAutoConnectManager.postProcessMessage(state, msg, args);
    }

    @Override
    public void triggerNetworkEvaluatorCallBack() {
        mAutoConnectManager.triggerNetworkEvaluatorCallBack();
    }

    @Override
    public boolean needCustomEvaluator() {
        return true;
    }

    private class ConnectTypeObserver extends ContentObserver {

        public ConnectTypeObserver(Handler handler) {
            super(handler);
            Op01WifiUtils.registerTypeObserver(mContext, this);
        }

        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);
            mConnectType = Op01WifiUtils.getConnectType(mContext);
            mSwitchSuspendTime = 0;
            mReselectSuspendTime = 0;
            log("ConnectTypeObserver, mConnectType:" + mConnectType);
            mAutoConnectManager.updateAutoConnectSettings();
        }
    }

    /**
     * Reminder type observer.
     */
    private class ReminderTypeObserver extends ContentObserver {

        public ReminderTypeObserver(Handler handler) {
            super(handler);
            Op01WifiUtils.registerRemindObserver(mContext, this);
        }

        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);
            mReminderType = Op01WifiUtils.getRemindType(mContext);
            log("ReminderTypeObserver, mReminderType:" + mReminderType);
        }
    }

    private class ConnectHandler extends Handler {

        private AutoConnectManager mAutoConnectManager;

        public ConnectHandler(AutoConnectManager autoConnectManager) {
            super();
            mAutoConnectManager = autoConnectManager;
        }

        @Override
        public final void handleMessage(Message msg) {
            log("Supplicant message: " + msg.what);
            switch (msg.what) {
                case WifiMonitor.SUPPLICANT_STATE_CHANGE_EVENT:
                    StateChangeResult stateChangeResult = (StateChangeResult) msg.obj;
                    mAutoConnectManager.setIsConnecting(Op01WifiUtils.
                            isSupplicantConnecting(stateChangeResult));
                    break;

                case WifiMonitor.SUP_CONNECTION_EVENT:
                    mAutoConnectManager.resetStates();
                    break;
                default:
                    log("Invalid message: " + msg.what);
            }
        }
    }

    private class Op01WifiOpReceiver extends BroadcastReceiver {

        private AutoConnectManager mAutoConnectManager;

        private static final int DEFAULT_WIFI_STATE = -1;

        public Op01WifiOpReceiver(AutoConnectManager autoConnectManager) {
            mAutoConnectManager = autoConnectManager;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log("[onReceive] mOperatorReceiver.onReceive: " + action);
            if (action.equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
                final int previousState = intent.getIntExtra(WifiManager.EXTRA_PREVIOUS_WIFI_STATE,
                        DEFAULT_WIFI_STATE);
                final int state = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                        DEFAULT_WIFI_STATE);
                log("[onReceive] previous/current state: " + previousState + "/" + state);

                mAutoConnectManager.onWifStateChanged(state);

            } else if (action.equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
                NetworkInfo info = (NetworkInfo) intent.getExtra(WifiManager.EXTRA_NETWORK_INFO);
                mAutoConnectManager.onNetworkStateChanged(info);

            } else if (action.equals(Op01WifiUtils.ACTION_SUSPEND_NOTIFICATION_NON_SYS)) {
                int type = intent.getIntExtra(Op01WifiUtils.EXTRA_SUSPEND_TYPE, -1);
                updateSuspendTime(type);
            }
        }
    }
}


package com.mediatek.apmonitor.monitor;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.DhcpInfo;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.NetworkUtils;
import android.net.wifi.ScanResult;
import android.net.wifi.ScanResult.InformationElement;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.mediatek.apmonitor.KpiMonitor;
import com.mediatek.apmonitor.ApmMsgDefs;
import com.mediatek.apmonitor.kpi.*;

import java.lang.reflect.Field;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.List;

public class WifiKpiMonitor extends KpiMonitor {
    private static final String TAG = "WifiKpiMonitor";
    private static final int MSG_OFFSET_WIFI            = 400;
    private static final int MSG_MDMI_LOGGING           = MSG_OFFSET_WIFI + 1;
    private static final int MSG_AUTHENTICATION_EVENT   = MSG_OFFSET_WIFI + 2;
    private static final int MSG_ASSOCIATION_EVENT      = MSG_OFFSET_WIFI + 3;
    private static final int MSG_REASSOCIATION_EVENT    = MSG_OFFSET_WIFI + 4;
    private static final int MSG_DISCONNECT_EVENT       = MSG_OFFSET_WIFI + 5;
    private static final int MSG_PS_MODE_CHANGED_EVENT  = MSG_OFFSET_WIFI + 6;
    private static final int MSG_SCAN_RESULTS_EVENT     = MSG_OFFSET_WIFI + 7;
    private static final int MSG_PROVISION_FAIL         = MSG_OFFSET_WIFI + 8;
    private static final int MDMI_LOGGING_INTERVAL = 1000;
    private static final int EID_COUNTRY = 7;
    private static final int EID_QOS_CAP = 46;
    private Handler mHandler = null;
    private WifiManager mWifiManager = null;
    private IntentFilter mFilter;
    private final AtomicBoolean mConnected = new AtomicBoolean(false);
    private int mWifiState = -1;
    private String mAssociatingBssid = null;
    private String mAssociatedBssid = null;
    private String mLastBssid = null;
    private String mLastSsid = null;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive: " + action);

            if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)) {
                mWifiState = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                       WifiManager.WIFI_STATE_UNKNOWN);
            } else if (WifiManager.SCAN_RESULTS_AVAILABLE_ACTION.equals(action)) {
                ScanInfo scanInfo = new ScanInfo();
                java.io.Serializable scanTimeExtra = intent.getSerializableExtra("scan_start");
                if (scanTimeExtra == null) {
                    Log.e(TAG, "Null scanTimeExtra! Fix me.");
                    return;
                }
                scanInfo.mTimeScanStart = (long)scanTimeExtra;

                scanInfo.mAvailableScanChannels = intent.getIntegerArrayListExtra("scan_channels");
                if (intent.getBooleanExtra(WifiManager.EXTRA_RESULTS_UPDATED, false)) {
                    mHandler.sendMessage(Message.obtain(
                            mHandler, MSG_SCAN_RESULTS_EVENT, scanInfo));
                } else {
                    Log.d(TAG, "Receive scan result failed broadcast");
                }
            } else if (WifiManager.NETWORK_STATE_CHANGED_ACTION.equals(action)) {
                NetworkInfo info = intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);

                if (mConnected.get() != info.isConnected()) {
                    mConnected.set(info.isConnected());
                }
            } else if (WifiManager.SUPPLICANT_STATE_CHANGED_ACTION.equals(action)) {
                handleSupplicantStateChanged(
                        (SupplicantState) intent.getParcelableExtra(WifiManager.EXTRA_NEW_STATE),
                        intent.hasExtra(WifiManager.EXTRA_SUPPLICANT_ERROR),
                        intent.getIntExtra(WifiManager.EXTRA_SUPPLICANT_ERROR, 0));
            } else if ("mediatek.intent.action.WIFI_ASSOCIATION_REJECT".equals(action)) {
                String bssid = intent.getStringExtra("bssid");
                if (mLastBssid != null) {
                    mHandler.sendMessage(Message.obtain(mHandler, MSG_REASSOCIATION_EVENT, 1, 0,
                        bssid));
                } else {
                    mHandler.sendMessage(Message.obtain(mHandler, MSG_ASSOCIATION_EVENT, 1, 0,
                        bssid));
                }
                clear();
            } else if ("mediatek.intent.action.WIFI_NETWORK_DISCONNECT".equals(action)) {
                String bssid = intent.getStringExtra("bssid");
                int reason = intent.getIntExtra("reason", -1);
                mHandler.sendMessage(Message.obtain(mHandler, MSG_DISCONNECT_EVENT, reason, 0,
                        bssid));
            } else if ("mediatek.intent.action.WIFI_PS_CHANGED".equals(action)) {
                boolean enabled = intent.getBooleanExtra("ps_mode", false);
                mHandler.sendMessage(Message.obtain(mHandler, MSG_PS_MODE_CHANGED_EVENT, 0, 0,
                        enabled));
            } else if ("mediatek.intent.action.WIFI_PROVISION_FAIL".equals(action)) {
                mHandler.sendMessage(Message.obtain(mHandler, MSG_PROVISION_FAIL));
            }
        }
    };

    private enum SecurityInfo {
        NONE,
        WEP,
        WPA,
        WPA2,
        WPA_CCMP,
        WPA_TKIP,
        WPA2_CCMP,
        WPA2_TKIP,
        WPA2_ENTERPRISE
    }

    public WifiKpiMonitor(Context context, Handler handler) {
        super(context, handler, TAG);
        Log.d(TAG, "WifiKpiMonitor Ctor");

        init(context, handler);
    }

    @Override
    public boolean handleMessage(Message msg) {
        Log.d(TAG, "WifiKpiMonitor handleMessage " + msgToString(msg) + " " + msg);
        switch (msg.what) {
            case MSG_ID_APM_KPI_STATE_CHANGED:
                Short msgId = (Short)msg.obj;
                if (msgId != ApmMsgDefs.APM_MSG_WIFI_INFO) {
                    return false;
                }
                if (msg.arg1 == EVENT_KPI_ADDED) {
                    // To submit when first registration
                    Log.d(TAG, "Subscribe APM_MSG_WIFI_INFO");
                    mHandler.sendMessage(Message.obtain(mHandler, MSG_MDMI_LOGGING));
                } else if (msg.arg1 == EVENT_KPI_REMOVED) {
                    Log.d(TAG, "Unsubscribe APM_MSG_WIFI_INFO");
                    mHandler.removeMessages(MSG_MDMI_LOGGING);
                }
                break;
            case MSG_MDMI_LOGGING:
                WifiInfo wifiInfo = mWifiManager.getConnectionInfo();
                sendWifiInfo(wifiInfo);
                //sendWifiConnectionStatus(wifiInfo);
                mHandler.sendMessageDelayed(Message.obtain(mHandler, MSG_MDMI_LOGGING),
                        MDMI_LOGGING_INTERVAL);
                break;
            case MSG_AUTHENTICATION_EVENT:
                sendWifiAuthenticationEvent((String) msg.obj, msg.arg1);
                break;
            case MSG_ASSOCIATION_EVENT:
                String ssid = getCurrentWifiInfo().getSSID();
                sendWifiAssociationEvent((String) msg.obj, ssid, msg.arg1);
                break;
            case MSG_REASSOCIATION_EVENT:
                sendWifiReAssociationEvent((String) msg.obj, msg.arg1);
                break;
            case MSG_DISCONNECT_EVENT:
                sendWifiDisconnectionEvent((String) msg.obj, (int) msg.arg1);
                sendWifiDeAuthenticationEvent((String) msg.obj, (int) msg.arg1);
                clear();
                break;
            case MSG_PS_MODE_CHANGED_EVENT:
                sendWifiPowerSaveEvent((boolean) msg.obj);
                break;
            case MSG_SCAN_RESULTS_EVENT:
                sendWifiScanEvent((ScanInfo) msg.obj);
                break;
            case MSG_PROVISION_FAIL:
                sendDhcpFailureEvent();
            default:
                return false;
        }
        return true;
    }

    private void init(Context context, Handler handler) {
        mHandler = handler;
        mWifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        // REgister boradcast
        mFilter = new IntentFilter();
        mFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        mFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
        mFilter.addAction(WifiManager.SUPPLICANT_STATE_CHANGED_ACTION);
        mFilter.addAction(WifiManager.CONFIGURED_NETWORKS_CHANGED_ACTION);
        mFilter.addAction(WifiManager.LINK_CONFIGURATION_CHANGED_ACTION);
        mFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        mFilter.addAction("mediatek.intent.action.WIFI_ASSOCIATION_REJECT");
        mFilter.addAction("mediatek.intent.action.WIFI_NETWORK_DISCONNECT");
        mFilter.addAction("mediatek.intent.action.WIFI_PS_CHANGED");
        mFilter.addAction("mediatek.intent.action.WIFI_PROVISION_FAIL");
        context.registerReceiver(mReceiver, mFilter);
    }

    private String msgToString(Message msg) {
        String ret = "";
        switch (msg.what) {
            case MSG_MDMI_LOGGING:
                ret = "MDMI logging";
                break;
            case MSG_AUTHENTICATION_EVENT:
                ret = (msg.arg1 == 1 ? "AUTHENTICATION_FAILURE" : "AUTHENTICATION_COMPLETE");
                break;
            case MSG_ASSOCIATION_EVENT:
                ret = (msg.arg1 == 1 ? "ASSOCIATION_REJECT" : "ASSOCIATION_COMPLETE");
                break;
            case MSG_REASSOCIATION_EVENT:
                ret = (msg.arg1 == 1 ? "REASSOCIATION_REJECT" : "REASSOCIATION_COMPLETE");
                break;
            case MSG_DISCONNECT_EVENT:
                ret = "NETWORK_DISCONNECTIO_EVENT";
                break;
            case MSG_PS_MODE_CHANGED_EVENT:
                ret = "POWER_SAVE_MODE_CHANGED";
                break;
            case MSG_SCAN_RESULTS_EVENT:
                ret = "SCAN_RESULTS_EVENT";
                break;
        }
        return ret;
    }

    private void sendWifiInfo(WifiInfo wifiInfo) {
        if (mWifiManager == null) {
            Log.e(TAG, "mWifiManager is null");
            return;
        }

        // Get some info from WifiInfo
        WifiInfoMessage info = new WifiInfoMessage();
        info.mBssid = wifiInfo.getBSSID();
        info.mSsid = wifiInfo.getSSID();
        info.mRssi = wifiInfo.getRssi();
        info.mDetailState = wifiInfo.getDetailedStateOf(wifiInfo.getSupplicantState()).name();
        info.mSupplicantState = wifiInfo.getSupplicantState().name();
        info.mLinkSpeed = wifiInfo.getLinkSpeed();
        info.mMacAddress = wifiInfo.getMacAddress();
        info.mNetworkID = wifiInfo.getNetworkId();
        info.mHiddenSSID = wifiInfo.getHiddenSSID();
        // Get some info. from DhcpResults
        if (mConnected.get()) {
            DhcpInfo dhcpInfo = mWifiManager.getDhcpInfo();
            info.mIp = NetworkUtils.intToInetAddress(dhcpInfo.ipAddress).getHostAddress();
            info.mGateway = NetworkUtils.intToInetAddress(dhcpInfo.gateway).getHostAddress();
            info.mDns1 = NetworkUtils.intToInetAddress(dhcpInfo.dns1).getHostAddress();
            info.mDns2 = NetworkUtils.intToInetAddress(dhcpInfo.dns2).getHostAddress();
            info.mServerIP = NetworkUtils.intToInetAddress(dhcpInfo.serverAddress).getHostAddress();
            info.mNetMask = NetworkUtils.intToInetAddress(dhcpInfo.netmask).getHostAddress();
        }
        sendKpi(ApmMsgDefs.APM_MSG_WIFI_INFO, info);
    }

/*  apmService handles this KPI
    private void sendWifiConnectionStatus(WifiInfo wifiInfo) {
        ApmWifiInternetConnectionStatus status = mApm.new ApmWifiInternetConnectionStatus();
        status.present = mConnected.get() == true ?
                ApmWifiInternetConnectionStatus.WIFI_CONNECTED :
                ApmWifiInternetConnectionStatus.NO_WIFI_CONNECTED;
        status.linkSpeed = wifiInfo.getLinkSpeed();
        mApm.send(ApmNative.KPI_TYPE_WIFI_INTERNET_CONN_STATUS, status);
    }
*/

    private void handleSupplicantStateChanged(SupplicantState state, boolean hasError, int error) {
        if (hasError) {
            // It means wifi got a authentication failure
            mHandler.sendMessage(Message.obtain(mHandler, MSG_AUTHENTICATION_EVENT, 1, 0,
                    mAssociatedBssid));
        }

        if (state == SupplicantState.ASSOCIATING) {
            mAssociatingBssid = getCurrentWifiInfo().getBSSID();
        } else if (state == SupplicantState.COMPLETED) {
            mLastBssid = getCurrentWifiInfo().getBSSID();
            mLastSsid = getCurrentWifiInfo().getSSID();
            mHandler.sendMessage(Message.obtain(mHandler, MSG_AUTHENTICATION_EVENT, 0, 0,
                    mLastBssid));
        } else if (state == SupplicantState.ASSOCIATED) {
            mAssociatedBssid = getCurrentWifiInfo().getBSSID();
            if (mLastBssid != null) {
                mHandler.sendMessage(Message.obtain(mHandler, MSG_REASSOCIATION_EVENT, 0, 0,
                        mAssociatedBssid));
            } else {
                mHandler.sendMessage(Message.obtain(mHandler, MSG_ASSOCIATION_EVENT, 0, 0,
                        mAssociatedBssid));
            }
        }
        Log.d(TAG, "handleSupplicantStateChanged state:" + state.name() + ", mAssociatingBssid = "
                + mAssociatingBssid + ", mAssociatedBssid = " + mAssociatedBssid + ", hasError = "
                + hasError + ", reason = " + error);
    }

    private void sendWifiAuthenticationEvent(String bssid, int result) {
        WifiConnectionMessage msg = new WifiConnectionMessage();
        msg.mType = 3; /* Authentication */
        msg.mBssid = bssid;
        msg.mCode = result;
        sendKpi(ApmMsgDefs.APM_MSG_WIFI_CONNECTION_INFO, msg);
    }

    private void sendWifiReAssociationEvent(String bssid, int result) {
        WifiConnectionMessage msg = new WifiConnectionMessage();
        msg.mType = 1; /* Reassociation */
        msg.mSsid = mLastSsid;
        msg.mBssid = bssid;
        msg.mCode = result;
        sendKpi(ApmMsgDefs.APM_MSG_WIFI_CONNECTION_INFO, msg);
    }

    private void sendWifiAssociationEvent(String bssid, String ssid, int result) {
        WifiConnectionMessage msg = new WifiConnectionMessage();
        msg.mType = 0; /* Association */
        msg.mSsid = ssid;
        msg.mBssid = bssid;
        msg.mCode = result;
        sendKpi(ApmMsgDefs.APM_MSG_WIFI_CONNECTION_INFO, msg);
    }

    private void sendWifiDisconnectionEvent(String bssid, int disconnectReason) {
        WifiConnectionMessage msg = new WifiConnectionMessage();
        msg.mType = 2; /* Disconnection */
        msg.mSsid = mLastSsid;
        msg.mBssid = bssid;
        msg.mCode = disconnectReason;
        sendKpi(ApmMsgDefs.APM_MSG_WIFI_CONNECTION_INFO, msg);
    }

    private void sendWifiDeAuthenticationEvent(String bssid, int disconnectReason) {
        WifiConnectionMessage msg = new WifiConnectionMessage();
        msg.mType = 4; /* Disconnection */
        msg.mBssid = bssid;
        msg.mCode = disconnectReason;
        sendKpi(ApmMsgDefs.APM_MSG_WIFI_CONNECTION_INFO, msg);
    }

    public void sendWifiPowerSaveEvent(boolean enabled) {
        WifiPSStatusMessage msg = new WifiPSStatusMessage();
        msg.mPSModeOn = enabled;
        sendKpi(ApmMsgDefs.APM_MSG_WIFI_POWER_SAVE, msg);
    }

    private void sendWifiScanEvent(ScanInfo scanInfo) {
        WifiScanMessage msg = new WifiScanMessage();
        msg.mTimeScanStart = scanInfo.mTimeScanStart;
        msg.mTimeScanEnd = System.currentTimeMillis();
        msg.mWifiChannels = getApmWifiChannels(scanInfo.mAvailableScanChannels);
        msg.mAccessPoints = getScannedAccessPoints();
        sendKpi(ApmMsgDefs.APM_MSG_WIFI_SCAN, msg);
    }

    private void sendDhcpFailureEvent() {
        WifiDHCPFailureMessage msg = new WifiDHCPFailureMessage();
        msg.mDHCPFailReason = 0; /* Always be provision fail*/
        sendKpi(ApmMsgDefs.APM_MSG_WIFI_DHCP_FAILURE, msg);
    }

    private WifiInfo getCurrentWifiInfo() {
        return mWifiManager.getConnectionInfo();
    }

    private void clear() {
        mAssociatingBssid = null;
        mAssociatedBssid = null;
        mLastBssid = null;
        mLastSsid = null;
    }

    private int freq2Band(int freq) {
        int band = 0;
        if (freq >= 2412 && freq < 3600) {
            band = 1;
        } else if (freq >= 3600 && freq < 4900) {
            band = 2;
        } else if (freq >= 4900 && freq < 5000) {
            band = 3;
        } else if (freq >= 5000 && freq < 5900) {
            band = 4;
        } else if (freq >= 5900) {
            band = 5;
        }
        return band;
    }

    private ArrayList<WifiChannel> getApmWifiChannels(ArrayList<Integer> reference) {
        ArrayList<WifiChannel> wifiChannels = new ArrayList<WifiChannel>();
        for (int i = 0; i < reference.size(); i++) {
            WifiChannel channel = new WifiChannel();
            channel.mWifiFreq= freq2Band(reference.get(i));
            channel.mChannelNum= convertFrequencyToChannel(reference.get(i));
            wifiChannels.add(channel);
        }
        return wifiChannels;
    }

    public ArrayList<WifiAccessPoint> getScannedAccessPoints() {
        List<ScanResult> scanResults = mWifiManager.getScanResults();
        ArrayList<WifiAccessPoint> wifiAccessPoints = new ArrayList<WifiAccessPoint>();
        for (int i = 0; i < scanResults.size(); i++) {
            WifiAccessPoint ap = new WifiAccessPoint();
            ScanResult accesspoint = scanResults.get(i);
            InformationElement[] ies = getInformationElements(accesspoint);
            ap.mSsid = accesspoint.SSID;
            ap.mBssid = accesspoint.BSSID;
            ap.mWifiChannel = getApmWifiChannel(accesspoint.frequency);
            ap.mSupporedtRates = getSupportedRates(ies);
            ap.mSecurity = getSecurityInfo(accesspoint.capabilities);
            ap.mQosCapability = checkQosCapability(ies);
            ap.mCountryInfo = getCountryInfo(ies) ? 0 : 1; /* 0:US, 1:Others */
            ap.mWifiHTCapability = getHTCapability(ies);
            wifiAccessPoints.add(ap);
        }
        return wifiAccessPoints;
    }

    private WifiChannel getApmWifiChannel(int freq) {
        WifiChannel channel = new WifiChannel();
        channel.mWifiFreq= freq2Band(freq);
        channel.mChannelNum= convertFrequencyToChannel(freq);
        return channel;
    }

    private int convertFrequencyToChannel(int frequency) {
        if (frequency >= 2412 && frequency <= 2472) {
            return (frequency - 2412) / 5 + 1;
        } else if (frequency == 2484) {
            return 14;
        } else if (frequency >= 5170  &&  frequency <= 5825) {
            /* DFS is included. */
            return (frequency - 5170) / 5 + 34;
        }

        return -1;
    }

    private int getSecurityInfo(String capabilities) {
        String _cap = capabilities.replaceAll("^\\[+", "");
        SecurityInfo ret = SecurityInfo.NONE;
        if (_cap.startsWith("WPA2-EAP")) {
            ret = SecurityInfo.WPA2_ENTERPRISE;
        } else if (_cap.startsWith("WPA2-PSK-TKIP")) {
            ret = SecurityInfo.WPA2_TKIP;
        } else if (_cap.startsWith("WPA2-PSK-CCMP")) {
            ret = SecurityInfo.WPA2_CCMP;
        } else if (_cap.startsWith("WPA-PSK-TKIP")) {
            ret = SecurityInfo.WPA_TKIP;
        } else if (_cap.startsWith("WPA-PSK-CCMP")) {
            ret = SecurityInfo.WPA_CCMP;
        } else if (_cap.startsWith("WPA2")) {
            ret = SecurityInfo.WPA2;
        } else if (_cap.startsWith("WPA")) {
            ret = SecurityInfo.WPA;
        } else if (_cap.startsWith("WEP")) {
            ret = SecurityInfo.WEP;
        }
        return ret.ordinal();
    }

    private class ScanInfo {
        private long mTimeScanStart;
        private ArrayList<Integer> mAvailableScanChannels;

        @Override
        public String toString() {
            return "{start scan time: " + mTimeScanStart + ", available scan channels: " +
                    mAvailableScanChannels + "}";
        }
    }

    private InformationElement[] getInformationElements(ScanResult result) {
        try {
            Field ies = result.getClass().getDeclaredField("informationElements");
            ies.setAccessible(true);
            return (InformationElement[]) ies.get(result);
        } catch (ReflectiveOperationException e) {
            e.printStackTrace();
            return null;
        }
    }

    private ArrayList<Integer> getSupportedRates(InformationElement[] ies) {
        SupportedRates supportedRates = new SupportedRates();
        try {
            for (InformationElement ie : ies) {
                if (ie.id == InformationElement.EID_SUPPORTED_RATES) {
                    supportedRates.from(ie);
                    break;
                }
            }
        } catch (IllegalArgumentException | BufferUnderflowException | ArrayIndexOutOfBoundsException e) {
            Log.d(TAG, "parse ies got exception: " + e);
        }

        if (supportedRates.isValid()) {
            return supportedRates.getMbpsRates();
        }
        return null;
    }

    private class SupportedRates {
        public static final int MASK = 0x7F; // 0111 1111
        public boolean mValid = false;
        public ArrayList<Integer> mRates;

        public SupportedRates() {
            mRates = new ArrayList<Integer>();
        }

        /**
         * Is this a valid Supported Rates information element.
         */
        public boolean isValid() {
            return mValid;
        }

        /**
         * get the Rate in bits/s from associated byteval
         */
        public int getRateFromByte(int byteVal) {
            byteVal &= MASK;
            switch(byteVal) {
                case 2:
                    return 1000000;
                case 4:
                    return 2000000;
                case 11:
                    return 5500000;
                case 12:
                    return 6000000;
                case 18:
                    return 9000000;
                case 22:
                    return 11000000;
                case 24:
                    return 12000000;
                case 36:
                    return 18000000;
                case 44:
                    return 22000000;
                case 48:
                    return 24000000;
                case 66:
                    return 33000000;
                case 72:
                    return 36000000;
                case 96:
                    return 48000000;
                case 108:
                    return 54000000;
                default:
                    //ERROR UNKNOWN RATE
                    return -1;
            }
        }

        // Supported Rates format (size unit: byte)
        //
        //| ElementID | Length | Supported Rates  [7 Little Endian Info bits - 1 Flag bit]
        //      1          1          1 - 8
        //
        // Note: InformationElement.bytes has 'Element ID' and 'Length'
        //       stripped off already
        //
        public void from(InformationElement ie) {
            mValid = false;
            if (ie == null || ie.bytes == null || ie.bytes.length > 8 || ie.bytes.length < 1)  {
                return;
            }
            ByteBuffer data = ByteBuffer.wrap(ie.bytes).order(ByteOrder.LITTLE_ENDIAN);
            try {
                for (int i = 0; i < ie.bytes.length; i++) {
                    int rate = getRateFromByte(data.get());
                    if (rate > 0) {
                        mRates.add(rate);
                    } else {
                        return;
                    }
                }
            } catch (BufferUnderflowException e) {
                return;
            }
            mValid = true;
            return;
        }

        /**
         * Lists the rates in a human readable string
         */
        public String toString() {
            StringBuilder sbuf = new StringBuilder();
            for (Integer rate : mRates) {
                sbuf.append(String.format("%.1f", (double) rate / 1000000) + ", ");
            }
            return sbuf.toString();
        }

        public ArrayList<Integer> getMbpsRates() {
            ArrayList<Integer> ret = new ArrayList<Integer>();
            for (int i = 0; i < mRates.size(); i++) {
                // To Mbps
                ret.add(mRates.get(i) / 1000000);
            }
            return ret;
        }
    }

    private boolean checkQosCapability(InformationElement[] ies) {
        try {
            for (InformationElement ie : ies) {
                if (ie.id == EID_QOS_CAP) {
                    ByteBuffer data = ByteBuffer.wrap(ie.bytes).order(ByteOrder.LITTLE_ENDIAN);
                    if (data.remaining() > 1) {
                        return true;
                    }
                    break;
                }
            }
        } catch (IllegalArgumentException | BufferUnderflowException | ArrayIndexOutOfBoundsException e) {
            Log.d(TAG, "parse ies got exception: " + e);
        }
        return false;
    }

    private boolean getCountryInfo(InformationElement[] ies) {
        boolean isUS = false;
        try {
            for (InformationElement ie : ies) {
                if (ie.id == EID_COUNTRY) {
                    ByteBuffer data = ByteBuffer.wrap(ie.bytes).order(ByteOrder.LITTLE_ENDIAN);
                    // First two bytes indicates country code
                    int firstByte = data.get();
                    int secondByte = data.get();
                    if (firstByte == 85 && secondByte == 83) {
                        isUS = true;
                    }
                    break;
                }
            }
        } catch (IllegalArgumentException | BufferUnderflowException | ArrayIndexOutOfBoundsException e) {
            Log.d(TAG, "parse ies got exception: " + e);
        }
        return isUS;
    }

    private WifiHTCapability getHTCapability(InformationElement[] ies) {
        WifiHTCapability htCapability = new WifiHTCapability();
        HtOperation htOperation = new HtOperation();
        try {
            for (InformationElement ie : ies) {
                if (ie.id == InformationElement.EID_HT_OPERATION) {
                    htOperation.from(ie);
                    htCapability.mChannelWidth = htOperation.getChannelWidth();
                    htCapability.mSecondaryChannelOffset = htOperation.secondChannelOffset;
                    break;
                }
            }
        } catch (IllegalArgumentException | BufferUnderflowException | ArrayIndexOutOfBoundsException e) {
            Log.d(TAG, "parse ies got exception: " + e);
        }

        return htCapability;
    }

    private class HtOperation {
        public int secondChannelOffset = 0;

        public int getChannelWidth() {
            if (secondChannelOffset != 0) {
                return 1;
            } else {
                return 0;
            }
        }

        public int getCenterFreq0(int primaryFrequency) {
            //40 MHz
            if (secondChannelOffset != 0) {
                if (secondChannelOffset == 1) {
                    return primaryFrequency + 10;
                } else if (secondChannelOffset == 3) {
                    return primaryFrequency - 10;
                } else {
                    Log.e("HtOperation", "Error on secondChannelOffset: " + secondChannelOffset);
                    return 0;
                }
            } else {
                return 0;
            }
        }

        public void from(InformationElement ie) {
            if (ie.id != InformationElement.EID_HT_OPERATION) {
                throw new IllegalArgumentException("Element id is not HT_OPERATION, : " + ie.id);
            }
            secondChannelOffset = ie.bytes[1] & 0x3;
        }
    }

    private void sendKpi(short type, KpiObjBaseRoot message) {
        byte[] val = message.build();
        int len = message.currentWriteBytes();
        apmSubmitKpi(type, len, val);
    }
}


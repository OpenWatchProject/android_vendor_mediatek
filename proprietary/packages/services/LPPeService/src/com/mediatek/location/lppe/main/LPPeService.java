package com.mediatek.location.lppe.main;

import java.lang.reflect.Method;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.net.LocalSocketAddress.Namespace;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.SystemClock;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.mediatek.location.lppe.bluetooth.BluetoothMeasurement;
import com.mediatek.location.lppe.bluetooth.BluetoothMeasurementRequest;
import com.mediatek.location.lppe.bluetooth.BluetoothMultipleMeasurement;
import com.mediatek.location.lppe.bluetooth.BluetoothProvideCapabilities;
import com.mediatek.location.lppe.bluetooth.BluetotthCategory;
import com.mediatek.location.lppe.bluetooth.LPPeBluetooth;
import com.mediatek.location.lppe.bluetooth.LPPeBluetooth.LPPeBluetoothReceiver;
import com.mediatek.location.lppe.bluetooth.LPPeBluetooth.LPPeBluetoothSender;
import com.mediatek.location.lppe.ipaddr.IpAddressCapabilities;
import com.mediatek.location.lppe.ipaddr.IpAddressInformation;
import com.mediatek.location.lppe.ipaddr.LPPeIpAddress;
import com.mediatek.location.lppe.ipaddr.LPPeIpAddress.LPPeIpAddressReceiver;
import com.mediatek.location.lppe.ipaddr.LPPeIpAddress.LPPeIpAddressSender;
import com.mediatek.location.lppe.lbs.LPPeLbs;
import com.mediatek.location.lppe.lbs.LPPeLbs.LPPeLbsReceiver;
import com.mediatek.location.lppe.lbs.LPPeLbs.LPPeLbsSender;
import com.mediatek.location.lppe.lbs.LbsCapabilities;
import com.mediatek.location.lppe.network.LPPeNetwork;
import com.mediatek.location.lppe.network.LPPeNetwork.LPPeNetworkReceiver;
import com.mediatek.location.lppe.network.LPPeNetwork.LPPeNetworkSender;
import com.mediatek.location.lppe.network.NetworkLocationMeasurement;
import com.mediatek.location.lppe.sensor.LPPeSensor;
import com.mediatek.location.lppe.sensor.LPPeSensor.LPPeSensorReceiver;
import com.mediatek.location.lppe.sensor.LPPeSensor.LPPeSensorSender;
import com.mediatek.location.lppe.sensor.SensorMeasurementRequest;
import com.mediatek.location.lppe.sensor.SensorPressureMeasurement;
import com.mediatek.location.lppe.sensor.SensorProvideCapabilities;
import com.mediatek.location.lppe.wlan.LPPeWlan;
import com.mediatek.location.lppe.wlan.LPPeWlan.LPPeWlanReceiver;
import com.mediatek.location.lppe.wlan.LPPeWlan.LPPeWlanSender;
import com.mediatek.location.lppe.wlan.WlanApMeasurement;
import com.mediatek.location.lppe.wlan.WlanMeasurements;
import com.mediatek.location.lppe.wlan.WlanProvideCapabilities;
import com.mediatek.location.lppe.wlan.WlanProvideMeasurement;
import com.mediatek.socket.base.UdpClient;
import com.mediatek.socket.base.UdpServer;

@SuppressWarnings("unused")
public class LPPeService {
    // Version
    public final static int LPPE_SERVICE_VERSION = 4;
    // 1 = first MP version
    // 2 = support battery info sync
    // 3 = fix timing issue on NLP capability sync
    // 4 = do not query the network location if there is e911 call and network location setting is off

    // Common
    public final static String TAG = "LPPeService";
    private static String LPPE_SOCKET_AGPS_CHANNEL = "mtk_lppe_socket_agps";

    private final static int HANDLER_WIFI_PROVIDE_CAPABILITIES = 0x10;
    private final static int HANDLER_WIFI_START_MEAS = 0x11;
    private final static int HANDLER_WIFI_STOP_MEAS = 0x12;
    private final static int HANDLER_WIFI_DELAY_MEAS = 0x13;
    private final static int HANDLER_BT_PROVIDE_CAPABILITIES = 0x20;
    private final static int HANDLER_BT_START_MEAS = 0x21;
    private final static int HANDLER_BT_STOP_MEAS = 0x22;
    private final static int HANDLER_SENSOR_PROVIDE_CAPABILITIES = 0x30;
    private final static int HANDLER_SENSOR_START_MEAS = 0x31;
    private final static int HANDLER_SENSOR_STOP_MEAS = 0x32;
    private final static int HANDLER_NETWORK_PROVIDE_CAPABILITIES = 0x40;
    private final static int HANDLER_NETWORK_START_MEAS = 0x41;
    private final static int HANDLER_NETWORK_STOP_MEAS = 0x42;
    private final static int HANDLER_NETWORK_RECHECK_CAPABILITIES = 0x43;
    private final static int HANDLER_IPADDR_PROVIDE_CAPABILITIES = 0x50;
    private final static int HANDLER_IPADDR_PROVIDE_INFORMATION = 0x51;
    private final static int HANDLER_LBS_PROVIDE_CAPABILITIES = 0x60;

    private final static int Location_Source_Agnss = 0x1;
    private final static int Location_Source_Sensor = 0x2;
    private final static int Location_Source_EcidLTE = 0x4;
    private final static int Location_Source_EcidGSM = 0x8;
    private final static int Location_Source_EcidUTRA = 0x10;
    private final static int Location_Source_WlanAP = 0x20;
    private final static int Location_Source_Srn = 0x40;

    private static LPPeServiceLogCallback mLogCallback;
    private Context mContext;
    private UdpClient mClient;
    private boolean mDebugEnabled = false;

    // WiFi
    private final static int MAX_WIFI_AP_SIZE = 64;
    private final static int LPPE_SOCKET_WLAN_VERSION = 1;
    private static String LPPE_SOCKET_WLAN_CHANNEL = "mtk_lppe_socket_wlan";
    /**
     * this definition is hidden by Android as systemApi
     * */
    private static String WIFI_AP_STATE_CHANGED_ACTION = "android.net.wifi.WIFI_AP_STATE_CHANGED";
    private static final String EXTRA_WIFI_AP_STATE = "wifi_state";
    private static final int WIFI_AP_STATE_DISABLING = 10;
    private static final int WIFI_AP_STATE_DISABLED = 11;
    private static final int WIFI_AP_STATE_ENABLING = 12;
    private static final int WIFI_AP_STATE_ENABLED = 13;
    private static final int WIFI_AP_STATE_FAILED = 14;

    private final static int WIFI_STATE_IDLE = 0;
    private final static int WIFI_STATE_AP_DISABLING = 1;
    private final static int WIFI_STATE_ENABLING = 2;
    private final static int WIFI_STATE_SCANNING = 3;

    private final static int WIFI_SCAN_MIN_INTERVAL = 2000; //ms
    private WifiManager mWifiManager;
    private LPPeWlanSender mWifiSender = new LPPeWlanSender();
    private UdpServer mWifiServer;
    private boolean mWifiSettingState;
    private int mWifiState = WIFI_STATE_IDLE;
    private long mWifiLastScanTime = 0;

    // BT
    private final static int LPPE_SOCKET_BT_VERSION = 1;
    private static String LPPE_SOCKET_BT_CHANNEL = "mtk_lppe_socket_bt";

    private final static int BT_STATE_IDLE = 0;
    private final static int BT_STATE_ENABLING = 1;
    private final static int BT_STATE_SCANNING = 2;

    private BluetoothManager mBtManager;
    private BluetoothAdapter mBtAdp;
    private LPPeBluetoothSender mBtSender = new LPPeBluetoothSender();
    private UdpServer mBtServer;
    private boolean mBtSettingState;
    private int mBtState = BT_STATE_IDLE;

    // Sensor
    private final static int LPPE_SOCKET_SENSOR_VERSION = 1;
    private static String LPPE_SOCKET_SENSOR_CHANNEL = "mtk_lppe_socket_sensor";
    private final static int LPPE_SENSOR_UPDATE_INTERVAL = 1000; // 1 s
    private final String LPPE_SENSOR_UNCALI_PRESSURE = "UNCALI_PRESSURE";

    private long mSensorLastUpdateTime;
    private SensorManager mSensorManager;
    private boolean mIsUncaliBarometer;
    private Sensor mBarometer;
    private Sensor mTemperature;
    private LPPeSensorSender mSensorSender = new LPPeSensorSender();
    private UdpServer mSensorServer;

    // Network
    private final static int LPPE_SOCKET_NETWORK_VERSION = 1;
    private static String LPPE_SOCKET_NETWORK_CHANNEL = "mtk_lppe_socket_network";
    private static final String LOCATION_PERMISSION_NAME =
            "android.permission.ACCESS_FINE_LOCATION";
    private final static int LPPE_NETWORK_RECHECK_INTERVAL = 10000; // 10 s
    private final static int LPPE_NETWORK_START_MEAS_DELAY = 1000; // 1 s

    private LocationManager mLocationManager;
    private LPPeNetworkSender mNetworkSender = new LPPeNetworkSender();
    private UdpServer mNetworkServer;
    private boolean mNetworkSupported;
    private int mNetworkCapabilityRecheckCount = 6;
    private PackageManager mPackageManager;

    // IP Address
    private final static int LPPE_SOCKET_IPADDR_VERSION = 1;
    private static String LPPE_SOCKET_IPADDR_CHANNEL = "mtk_lppe_socket_ipaddr";

    private LPPeIpAddressSender mIpAddrSender = new LPPeIpAddressSender();
    private UdpServer mIpAddrServer;

    // LBS
    private final static int LPPE_SOCKET_LBS_VERSION = 2;
    private static String LPPE_SOCKET_LBS_CHANNEL = "mtk_lppe_socket_lbs";

    private LPPeLbsSender mLbsSender = new LPPeLbsSender();
    private UdpServer mLbsServer;
    private int mLastBatteryPercentage = -1;
    private TelecomManager mTelecom;

    public LPPeService(Context context) {
        this(context, LPPE_SOCKET_AGPS_CHANNEL, LPPE_SOCKET_WLAN_CHANNEL,
                LPPE_SOCKET_BT_CHANNEL, LPPE_SOCKET_SENSOR_CHANNEL,
                LPPE_SOCKET_NETWORK_CHANNEL, LPPE_SOCKET_IPADDR_CHANNEL);
    }

    public LPPeService(Context context, String agpsChannel, String wlanChannel,
            String btChannel, String sensorChannel, String networkChannel,
            String ipaddrChannel) {
        log("LPPeService() ver=" + LPPE_SERVICE_VERSION);

        mContext = context;
        LPPE_SOCKET_AGPS_CHANNEL = agpsChannel;
        LPPE_SOCKET_WLAN_CHANNEL = wlanChannel;
        LPPE_SOCKET_BT_CHANNEL = btChannel;
        LPPE_SOCKET_SENSOR_CHANNEL = sensorChannel;
        LPPE_SOCKET_NETWORK_CHANNEL = networkChannel;
        LPPE_SOCKET_IPADDR_CHANNEL = ipaddrChannel;

        mWifiManager = (WifiManager) context
                .getSystemService(Context.WIFI_SERVICE);
        if (mWifiManager == null) {
            loge("[wlan] getSystemService(WIFI_SERVICE) is not supported");
        }

        mBtManager = (BluetoothManager) context
                .getSystemService(Context.BLUETOOTH_SERVICE);
        if (mBtManager == null) {
            loge("[bt] getSystemService(BLUETOOTH_SERVICE) is not supported");
        } else {
            mBtAdp = mBtManager.getAdapter();
            if (mBtAdp == null) {
                loge("[bt] BluetoothAdapter.getDefaultAdapter() is not supported");
            }
        }

        mSensorManager = (SensorManager) context
                .getSystemService(Context.SENSOR_SERVICE);
        if (mSensorManager == null) {
            loge("[sensor] getSystemService(Context.SENSOR_SERVICE) is not supported");
        } else {
            mBarometer = mSensorManager.getDefaultSensor(Sensor.TYPE_PRESSURE);
            if (mBarometer == null) {
                loge("[sensor] SensorManager.getDefaultSensor(Sensor.TYPE_PRESSURE) is not supported");
            }

            mTemperature = mSensorManager
                    .getDefaultSensor(Sensor.TYPE_AMBIENT_TEMPERATURE);
            if (mTemperature == null) {
                loge("[sensor] SensorManager.getDefaultSensor(Sensor.TYPE_AMBIENT_TEMPERATURE) is not supported");
            }
        }
        // try to find UNCALI_PRESSURE sensor
        /*
        Sensor uncaliBarometer = null;
        List<Sensor> sensorList = mSensorManager.getSensorList(Sensor.TYPE_ALL);
        for (Sensor sensor : sensorList) {
            if (sensor == null) {
                continue;
            }
            if (sensor.getName().equals(LPPE_SENSOR_UNCALI_PRESSURE)) {
                uncaliBarometer = sensor;
            }
        }
        if (uncaliBarometer == null) {
            loge("[sensor] UNCALI_PRESSURE is not supported");
        } else {
            mIsUncaliBarometer = true;
            mBarometer = uncaliBarometer;
        }
        */

        mPackageManager = context.getPackageManager();
        mLocationManager = (LocationManager) context
                .getSystemService(Context.LOCATION_SERVICE);
        if (mLocationManager == null) {
            loge("[network] getSystemService(Context.LOCATION_SERVICE) is not supported");
        } else {
            mNetworkSupported = mLocationManager
                    .getProvider(LocationManager.NETWORK_PROVIDER) != null;
            if (!mNetworkSupported) {
                loge("[network] NETWORK_PROVIDER is not supported");
                commonSendEventToHandlerWithDelay(HANDLER_NETWORK_RECHECK_CAPABILITIES,
                    LPPE_NETWORK_RECHECK_INTERVAL);
            }
        }

        IntentFilter filter = new IntentFilter();
        if (mWifiManager != null) {
            filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
            filter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
            filter.addAction(WIFI_AP_STATE_CHANGED_ACTION);
        }
        if (mBtAdp != null) {
            filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
            filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_STARTED);
            filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
            filter.addAction(BluetoothDevice.ACTION_FOUND);
        }
        filter.addAction(Intent.ACTION_NEW_OUTGOING_CALL);
        filter.addAction(Intent.ACTION_BATTERY_CHANGED);
        filter.addAction(LocationManager.PROVIDERS_CHANGED_ACTION);

        context.registerReceiver(mBroadcastReceiver, filter);

        mClient = new UdpClient(LPPE_SOCKET_AGPS_CHANNEL, Namespace.ABSTRACT,
                commonGetMaxBuffSize());

        mWifiServer = new UdpServer(LPPE_SOCKET_WLAN_CHANNEL,
                Namespace.ABSTRACT, LPPeWlan.MAX_BUFF_SIZE);
        mBtServer = new UdpServer(LPPE_SOCKET_BT_CHANNEL, Namespace.ABSTRACT,
                LPPeBluetooth.MAX_BUFF_SIZE);
        mSensorServer = new UdpServer(LPPE_SOCKET_SENSOR_CHANNEL,
                Namespace.ABSTRACT, LPPeSensor.MAX_BUFF_SIZE);
        mNetworkServer = new UdpServer(LPPE_SOCKET_NETWORK_CHANNEL,
                Namespace.ABSTRACT, LPPeNetwork.MAX_BUFF_SIZE);
        mIpAddrServer = new UdpServer(LPPE_SOCKET_IPADDR_CHANNEL,
                Namespace.ABSTRACT, LPPeIpAddress.MAX_BUFF_SIZE);
        mLbsServer = new UdpServer(LPPE_SOCKET_LBS_CHANNEL, Namespace.ABSTRACT,
                LPPeLbs.MAX_BUFF_SIZE);

        mWifiServer.setReceiver(mWifiReceiver);
        mBtServer.setReceiver(mBtReceiver);
        mSensorServer.setReceiver(mSensorReceiver);
        mNetworkServer.setReceiver(mNetworkReceiver);
        mIpAddrServer.setReceiver(mIpAddrReceiver);
        mLbsServer.setReceiver(mLbsReceiver);

        // provide the capabilities to AGPSD when LPPe service initial done
        mWifiReceiver.requestCapabilities(0);
        mBtReceiver.requestCapabilities(0);
        mSensorReceiver.requestCapabilities(0);
        mNetworkReceiver.requestCapabilities(0);
        mIpAddrReceiver.requestCapabilities(0);
        mLbsReceiver.requestCapabilities(0);

        mTelecom = (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);
    }

    public void registerLog(LPPeServiceLogCallback callback) {
        mLogCallback = callback;
    }

    public void cleanup() {
        mWifiServer.close();
        mBtServer.close();
        mSensorServer.close();
        mNetworkServer.close();
        mIpAddrServer.close();
        mContext.unregisterReceiver(mBroadcastReceiver);
        if (mWifiState != WIFI_STATE_IDLE) {
            wifiStopMeasurementHandler();
        }
        if (mBtState != BT_STATE_IDLE) {
            btStopMeasurementHandler();
        }
        if (mSensorManager != null) {
            mSensorManager.unregisterListener(mSensorBaroEventListener);
        }
        if (mNetworkSupported) {
            mLocationManager.removeUpdates(mLocationListener);
        }
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            // WIFI
            case HANDLER_WIFI_PROVIDE_CAPABILITIES:
                wifiProvideCapabilitiesHandler();
                break;
            case HANDLER_WIFI_START_MEAS:
                wifiStartMeasurementHandler();
                break;
            case HANDLER_WIFI_STOP_MEAS:
                wifiStopMeasurementHandler();
                break;
            case HANDLER_WIFI_DELAY_MEAS:
                wifiResumeScanHandler();
                break;
            // BT
            case HANDLER_BT_PROVIDE_CAPABILITIES:
                btProvideCapabilitiesHandler();
                break;
            case HANDLER_BT_START_MEAS:
                btStartMeasurementHandler((BluetoothMeasurementRequest) msg.obj);
                break;
            case HANDLER_BT_STOP_MEAS:
                btStopMeasurementHandler();
                break;
            // Sensor
            case HANDLER_SENSOR_PROVIDE_CAPABILITIES:
                sensorProvideCapabilitiesHandler();
                break;
            case HANDLER_SENSOR_START_MEAS:
                sensorStartMeasurementHandler((SensorMeasurementRequest) msg.obj);
                break;
            case HANDLER_SENSOR_STOP_MEAS:
                sensorStopMeasurementHandler();
                break;
            // Network
            case HANDLER_NETWORK_PROVIDE_CAPABILITIES:
                networkProvideCapabilitiesHandler();
                break;
            case HANDLER_NETWORK_START_MEAS:
                networkStartMeasurementHandler();
                break;
            case HANDLER_NETWORK_STOP_MEAS:
                networkStopMeasurementHandler();
                break;
            case HANDLER_NETWORK_RECHECK_CAPABILITIES:
                networkRecheckCapabilitiesHandler();
                break;
            // IP Address
            case HANDLER_IPADDR_PROVIDE_CAPABILITIES:
                ipaddrProvideCapabilitiesHandler();
                break;
            case HANDLER_IPADDR_PROVIDE_INFORMATION:
                ipaddrProvideInformationHandler();
                break;
            // LBS
            case HANDLER_LBS_PROVIDE_CAPABILITIES:
                lbsProvideCapabilitiesHandler();
                break;
            default:
                loge("[common] Handler.handleMessage() unknow what=" + msg.what);
                break;
            }
        }
    };

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context ctx, Intent intent) {
            String action = intent.getAction();
            // WIFI
            if (action.equals(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION)) {
                wifiScanResultHandler(intent);
            } else if (action.equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
                wifiStateChangedHandler(intent);
            } else if (action.equals(WIFI_AP_STATE_CHANGED_ACTION)) {
                wifiApStatChangedHandler(intent);
            }
            // BT
            else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                btStateChangedHandler(intent);
            } else if (action.equals(BluetoothAdapter.ACTION_DISCOVERY_STARTED)) {
                btDiscoveryStartedHandler(intent);
            } else if (action
                    .equals(BluetoothAdapter.ACTION_DISCOVERY_FINISHED)) {
                btDiscoveryFinishedHandler(intent);
            } else if (action.equals(BluetoothDevice.ACTION_FOUND)) {
                btDeviceFoundHandler(intent);
            }
            // LBS
            else if (action.equals(Intent.ACTION_BATTERY_CHANGED)) {
                lbsBatteryChanged(intent);
            } else if (action.equals(LocationManager.PROVIDERS_CHANGED_ACTION)) {
                lbsNlpStatusChanged(intent);
            }
            // Unknown
            else {
                loge("[common] BroadcastReceiver.onReceive() receive an unhandle action=["
                        + action + "]");
            }
        }
    };

    private SensorEventListener mSensorBaroEventListener = new SensorEventListener() {
        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            sensorBaroAccuracyChangedHandler(sensor, accuracy);
        }

        @Override
        public void onSensorChanged(SensorEvent event) {
            sensorBaroSensorChangedHandler(event);
        }
    };

    private LocationListener mLocationListener = new LocationListener() {
        @Override
        public void onLocationChanged(Location location) {
            networkLocationChangedHandler(location);
        }

        @Override
        public void onProviderDisabled(String provider) {
        }

        @Override
        public void onProviderEnabled(String provider) {
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
        }
    };

    private LPPeWlanReceiver mWifiReceiver = new LPPeWlanReceiver() {
        @Override
        public void requestCapabilities(int agpsVersion) {
            log("[wlan] requestCapabilities()" + " agpsVersion=[" + agpsVersion
                    + "]");
            commonSendEventToHandler(HANDLER_WIFI_PROVIDE_CAPABILITIES);
        }

        @Override
        public void provideCapabilities(int wlanVersion,
                WlanProvideCapabilities capabilities) {
            // should not happen
            loge("[wlan] provideCapabilities()" + " wlanVersion=["
                    + wlanVersion + "], this msg is not expected");
        }

        @Override
        public void startMeasurement(boolean hideIcon, WlanMeasurements request) {
            commonSendEventToHandler(HANDLER_WIFI_START_MEAS);
        }

        @Override
        public void stopMeasurement() {
            commonSendEventToHandler(HANDLER_WIFI_STOP_MEAS);
        }

        @Override
        public void provideMeasurement(WlanProvideMeasurement meas) {
            // should not happen
            loge("[wlan] provideMeasurement(), this msg is not expected");
        }
    };

    private LPPeBluetoothReceiver mBtReceiver = new LPPeBluetoothReceiver() {
        @Override
        public void requestCapabilities(int agpsVersion) {
            log("[bt] requestCapabilities()" + " agpsVersion=[" + agpsVersion
                    + "]");
            commonSendEventToHandler(HANDLER_BT_PROVIDE_CAPABILITIES);
        }

        @Override
        public void provideCapabilities(int btVersion,
                BluetoothProvideCapabilities capabilities) {
            // should not happen
            loge("[bt] provideCapabilities()" + " btVersion=[" + btVersion
                    + "]" + " capabilities=[" + capabilities + "]");
        }

        @Override
        public void startMeasurement(boolean hideIcon,
                BluetoothMeasurementRequest request) {
            commonSendEventToHandler(HANDLER_BT_START_MEAS, request);
        }

        @Override
        public void stopMeasurement() {
            commonSendEventToHandler(HANDLER_BT_STOP_MEAS);
        }

        @Override
        public void provideMeasurement(BluetoothMeasurement meas) {
            // should not happen
            log("[bt] provideMeasurement()" + " meas=[" + meas + "]");
        }

        @Override
        public void provideMeasurementFinished() {
            // should not happen
            loge("provideMeasurementFinished()");
        }

    };
    private LPPeSensorReceiver mSensorReceiver = new LPPeSensorReceiver() {
        @Override
        public void requestCapabilities(int agpsVersion) {
            log("[sensor] requestCapabilities()" + " agpsVersion=["
                    + agpsVersion + "]");
            commonSendEventToHandler(HANDLER_SENSOR_PROVIDE_CAPABILITIES);
        }

        @Override
        public void provideCapabilities(int sensorVersion,
                SensorProvideCapabilities capabilities) {
            // should not happen
            loge("[sensor] provideCapabilities()" + " sensorVersion=["
                    + sensorVersion + "]" + " capabilities=[" + capabilities
                    + "]");
        }

        @Override
        public void startMeasurement(SensorMeasurementRequest request) {
            commonSendEventToHandler(HANDLER_SENSOR_START_MEAS, request);
        }

        @Override
        public void stopMeasurement() {
            commonSendEventToHandler(HANDLER_SENSOR_STOP_MEAS);
        }

        @Override
        public void providePressureMeasurement(SensorPressureMeasurement meas) {
            // should not happen
            loge("[sensor] providePressureMeasurement()" + " measurement=["
                    + meas + "]");
        }
    };

    private LPPeNetworkReceiver mNetworkReceiver = new LPPeNetworkReceiver() {
        @Override
        public void requestCapabilities(int agpsVersion) {
            log("[network] requestCapabilities()" + " agpsVersion=["
                    + agpsVersion + "]");
            commonSendEventToHandler(HANDLER_NETWORK_PROVIDE_CAPABILITIES);
        }

        @Override
        public void provideCapabilities(int networkVersion,
                boolean networkLocationSupported) {
            // should not happen
            loge("[network] provideCapabilities()" + " networkVersion=["
                    + networkVersion + "]");
        }

        @Override
        public void startMeasurement() {
            int delayTimeToStart = 0;
            if (mTelecom != null) {
                boolean isInEmergencyCall = mTelecom.isInEmergencyCall();
                if (isInEmergencyCall) {
                    addPackageInLocationSettingsWhitelist(mContext);
                    delayTimeToStart = LPPE_NETWORK_START_MEAS_DELAY;
                }
            }
            commonSendEventToHandlerWithDelay(HANDLER_NETWORK_START_MEAS, delayTimeToStart);
        }

        @Override
        public void stopMeasurement() {
            commonSendEventToHandler(HANDLER_NETWORK_STOP_MEAS);
        }

        @Override
        public void provideMeasurement(NetworkLocationMeasurement meas) {
            // should not happen
            loge("[network] provideMeasurement()" + " meas=[" + meas + "]");
        }

    };

    private LPPeIpAddressReceiver mIpAddrReceiver = new LPPeIpAddressReceiver() {
        @Override
        public void requestCapabilities(int agpsVersion) {
            log("[ip] requestCapabilities()" + " agpsVersion=[" + agpsVersion
                    + "]");
            commonSendEventToHandler(HANDLER_IPADDR_PROVIDE_CAPABILITIES);
        }

        @Override
        public void provideCapabilities(int ipAddrVersion,
                IpAddressCapabilities capabilities) {
            // should not happen
            loge("[ip] provideCapabilities()");
        }

        @Override
        public void requestInformation() {
            commonSendEventToHandler(HANDLER_IPADDR_PROVIDE_INFORMATION);

        }

        @Override
        public void provideInformation(IpAddressInformation info) {
            // should not happen
            loge("[ip] provideInformation()");
        }

    };

    private LPPeLbsReceiver mLbsReceiver = new LPPeLbsReceiver() {
        @Override
        public void requestCapabilities(int agpsVersion) {
            log("[lbs] requestCapabilities()" + " agpsVersion=[" + agpsVersion
                    + "]");
            commonSendEventToHandler(HANDLER_LBS_PROVIDE_CAPABILITIES);
        }

        @Override
        public void provideCapabilities(int lbsVersion,
                LbsCapabilities capabilities) {
            // should not happen
            loge("[lbs] provideCapabilities()");
        }

        @Override
        public void receiveSuplInit(byte[] suplInit) {
            // should not happen
            loge("[lbs] receiveSuplInit()");
        }

        @Override
        public void updateBatteryInfo(int batteryPercentage) {
            // should not happen
            loge("[lbs] updateBatteryInfo()");
        }

        @Override
        public void updateNlpStatus(boolean enabled) {
            // should not happen
            loge("[lbs] updateNlpStatus()");
        }
    };

    private void commonSendEventToHandler(int event) {
        commonSendEventToHandler(event, null);
    }

    private void commonSendEventToHandler(int event, Object obj) {
        Message m = Message.obtain();
        m.what = event;
        m.obj = obj;
        mHandler.sendMessage(m);
    }

    private void commonSendEventToHandlerWithDelay(int event, int delayMillis) {
        Message m = Message.obtain();
        m.what = event;
        m.obj = null;
        mHandler.sendMessageDelayed(m, delayMillis);
    }

    private int commonGetMaxBuffSize() {
        int maxSize = 0;
        if (maxSize < LPPeWlan.MAX_BUFF_SIZE) {
            maxSize = LPPeWlan.MAX_BUFF_SIZE;
        }
        if (maxSize < LPPeBluetooth.MAX_BUFF_SIZE) {
            maxSize = LPPeBluetooth.MAX_BUFF_SIZE;
        }
        if (maxSize < LPPeSensor.MAX_BUFF_SIZE) {
            maxSize = LPPeSensor.MAX_BUFF_SIZE;
        }
        if (maxSize < LPPeNetwork.MAX_BUFF_SIZE) {
            maxSize = LPPeNetwork.MAX_BUFF_SIZE;
        }
        if (maxSize < LPPeIpAddress.MAX_BUFF_SIZE) {
            maxSize = LPPeIpAddress.MAX_BUFF_SIZE;
        }
        return maxSize;
    }

    private static byte[] commonMacStringToByteArray(String mac) {
        String values[] = mac.split(":");
        byte out[] = new byte[6];
        for (int i = 0; i < out.length; i++) {
            out[i] = Integer.decode("0x" + values[i]).byteValue();
        }
        return out;
    }

    private void wifiApStatChangedHandler(Intent intent) {
        int state = intent.getIntExtra(EXTRA_WIFI_AP_STATE,
                WIFI_AP_STATE_FAILED);
        if (state == WIFI_AP_STATE_DISABLED) {
            if (mWifiState == WIFI_STATE_AP_DISABLING) {
                wifiCheckWifiStateAndScan();
            }
        }
    }

    private boolean wifiApSetEnabled(boolean enabled) {
        try {
            Method m;
            Boolean ret;
            m = mWifiManager.getClass().getMethod("setWifiApEnabled",
                    new Class[] { WifiConfiguration.class, boolean.class });
            ret = (Boolean) m.invoke(mWifiManager,
                    new Object[] { null, enabled });
            return ret;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    private boolean wifiApIsEnabled() {
        try {
            Method m;
            Boolean ret;
            m = mWifiManager.getClass().getMethod("isWifiApEnabled",
                    new Class[] {});
            ret = (Boolean) m.invoke(mWifiManager, new Object[] {});
            return ret;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    private int wifiApGetState() {
        try {
            Method m;
            Integer ret;
            m = mWifiManager.getClass().getMethod("getWifiApState",
                    new Class[] {});
            ret = (Integer) m.invoke(mWifiManager, new Object[] {});
            return ret;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return WIFI_AP_STATE_FAILED;
    }

    private void wifiProvideCapabilitiesHandler() {
        WlanProvideCapabilities c = new WlanProvideCapabilities();
        if (mWifiManager != null) {
            c.isWlanSupported = true;
            c.ueMACAddress = commonMacStringToByteArray(wifiGetMacAddr());
            c.measSupported.nonServing = true;
            c.measSupported.apSSID = true;
            c.measSupported.apRSSI = true;
            c.measSupported.apChanFreq = true;
        }
        if (!mWifiSender.provideCapabilities(mClient, LPPE_SOCKET_WLAN_VERSION,
                c)) {
            loge("[wlan] ProvideCapabilities() mWifiSender.provideCapabilities() failed");
        }
    }

    private void wifiCheckWifiStateAndScan() {
        mWifiSettingState = mWifiManager.isWifiEnabled();
        if (mWifiSettingState) {
            mWifiState = WIFI_STATE_SCANNING;
            log("[wlan] StartMeasurement() startScan()");
            mWifiLastScanTime = SystemClock.elapsedRealtime();
            if (!mWifiManager.startScan()) {
                loge("[wlan] StartMeasurement() startScan() failed");
            }
        } else {
            mWifiState = WIFI_STATE_ENABLING;
            log("[wlan] StartMeasurement() setWifiEnabled(true)");
            if (!mWifiManager.setWifiEnabled(true)) {
                loge("[wlan] StartMeasurement() setWifiEnabled(true) failed");
            }
        }
    }

    private void wifiStartMeasurementHandler() {
        if (mWifiManager == null) {
            loge("[wlan] StartMeasurement() WiFi is not supported");
            return;
        }
        if (mWifiState != WIFI_STATE_IDLE) {
            loge("[wlan] StartMeasurement() already started, state="
                    + mWifiState);
            return;
        }

        if (wifiApGetState() != WIFI_AP_STATE_DISABLED) {
            mWifiState = WIFI_STATE_AP_DISABLING;
            log("[wlan] disable WiFi AP");
            wifiApSetEnabled(false);
        } else {
            wifiCheckWifiStateAndScan();
        }
    }

    private void wifiStopMeasurementHandler() {
        if (mWifiManager == null) {
            loge("[wlan] StopMeasurement() WiFi is not supported");
            return;
        }
        if (mWifiState == WIFI_STATE_IDLE) {
            loge("[wlan] StopMeasurement() already stopped");
            return;
        }
        log("[wlan] StopMeasurement()");
        mWifiState = WIFI_STATE_IDLE;
        if (!mWifiSettingState) {
            log("[wlan] StopMeasurement() setWifiEnabled(false) to restore to previous state");
            if (!mWifiManager.setWifiEnabled(false)) {
                loge("[wlan] StopMeasurement() setWifiEnabled(false) failed");
            }
        }
    }

    private void wifiStateChangedHandler(Intent intent) {
        if (mWifiManager == null) {
            loge("[wlan] StateChanged() WiFi is not supported");
            return;
        }
        int state = mWifiManager.getWifiState();
        if (state == WifiManager.WIFI_STATE_ENABLED) {
            if (mWifiState == WIFI_STATE_ENABLING) {
                mWifiState = WIFI_STATE_SCANNING;
                log("[wlan] StateChanged() startScan()");
                mWifiLastScanTime = SystemClock.elapsedRealtime();
                if (!mWifiManager.startScan()) {
                    loge("[wlan] StateChanged() startScan() failed");
                }
            }
        }
    }

    private void wifiScanResultHandler(Intent intent) {
        if (mWifiManager == null) {
            loge("[wlan] ScanResult() WiFi is not supported");
            return;
        }
        if (mWifiState != WIFI_STATE_SCANNING) {
            return;
        }
        List<ScanResult> list = mWifiManager.getScanResults();
        if (list == null) {
            return;
        }
        List<ScanResult> filteredList = wifiFilterScanResult(list,
                MAX_WIFI_AP_SIZE);
        if (mDebugEnabled) {
            wifiScanResultDump(filteredList);
        }
        wifiProvideMeasurementHandler(filteredList);

        long lastScanDuration = SystemClock.elapsedRealtime() - mWifiLastScanTime;
        if ( lastScanDuration < WIFI_SCAN_MIN_INTERVAL) {
            log("[wlan] delay scan after " + (WIFI_SCAN_MIN_INTERVAL - lastScanDuration));
            mHandler.removeMessages(HANDLER_WIFI_DELAY_MEAS);
            sendDelayedMessage(HANDLER_WIFI_DELAY_MEAS, WIFI_SCAN_MIN_INTERVAL - lastScanDuration);
        } else {
            log("[wlan] ScanResult() startScan()");
            mWifiLastScanTime = SystemClock.elapsedRealtime();
            if (!mWifiManager.startScan()) {
                loge("[wlan] ScanResult() startScan() failed");
            }
        }
    }

    private void wifiResumeScanHandler() {
        if (mWifiManager == null) {
            loge("[wlan] wifiResumeScanHandler() WiFi is not supported");
            return;
        }
        if (mWifiState != WIFI_STATE_SCANNING) {
            return;
        }

        log("[wlan] wifiResumeScanHandler() startScan()");
        mWifiLastScanTime = SystemClock.elapsedRealtime();
        if (!mWifiManager.startScan()) {
            loge("[wlan] wifiResumeScanHandler() startScan() failed");
        }
    }

    private String wifiGetServingBSSID() {
        String servingBSSID = null;
        WifiInfo info = mWifiManager.getConnectionInfo();
        if (info != null) {
            servingBSSID = info.getBSSID();
            if (servingBSSID != null
                    && servingBSSID.equals("00:00:00:00:00:00")) {
                servingBSSID = null;
            }
        }
        return servingBSSID;
    }

    private void wifiScanResultDump(List<ScanResult> list) {
        String servingBSSID = wifiGetServingBSSID();
        int i = 1;
        for (ScanResult s : list) {
            String mac = s.BSSID;
            String ssid = s.SSID;
            int rssi = s.level;
            int channelNum = wifiConvertFrequencyToChannelNumber(s.frequency);
            boolean servingFlag = stringCompare(servingBSSID, s.BSSID);
            log("[wlan] " + String.format("%3d", i) + " MAC=[" + mac
                    + "] RSSI=[" + rssi + "] channelNum=[" + channelNum
                    + "] servingFlag=[" + servingFlag + "] SSID=[" + ssid + "]");
            i++;
        }
    }

    private void wifiProvideMeasurementHandler(List<ScanResult> list) {
        WlanProvideMeasurement wlanProvideMeas = new WlanProvideMeasurement();
        wlanProvideMeas.meas = new WlanApMeasurement[list.size()];
        String servingBSSID = wifiGetServingBSSID();
        int i = 0;
        for (ScanResult s : list) {
            wlanProvideMeas.meas[i] = new WlanApMeasurement();
            WlanApMeasurement m = wlanProvideMeas.meas[i];

            m.servingFlag = stringCompare(servingBSSID, s.BSSID);
            m.apMACAddress = commonMacStringToByteArray(s.BSSID);
            m.valid |= 0x1; // apRSSI
            m.valid |= 0x2; // apChanFreq
            m.apRSSI = s.level;
            m.apChanFreq = (short) wifiConvertFrequencyToChannelNumber(s.frequency);
            if (s.SSID.getBytes().length <= 32) {
                m.valid |= 0x8; // apSSID
                m.apSSID = s.SSID.getBytes();
            }
            i++;
        }

        log("[wlan] ProvideMeasurement() update wifi measurement to AGPSD, size=["
                + wlanProvideMeas.meas.length + "]");
        if (!mWifiSender.provideMeasurement(mClient, wlanProvideMeas)) {
            loge("[wlan] ProvideMeasurement() mWifiSender.provideMeasurement() failed");
        }
    }

    // TODO do WiFi scan without enabling WiFi settings
    final String WIFI_SCAN_ALWAYS_AVAILABLE = "wifi_scan_always_enabled";

    // <uses-permission android:name="android.permission.WRITE_SECURE_SETTINGS"
    // /> required
    private int wifiGetWifiScanAlwaysValue(int defaultValue) {
        try {
            return Settings.Global.getInt(mContext.getContentResolver(),
                    WIFI_SCAN_ALWAYS_AVAILABLE);
        } catch (SettingNotFoundException e) {
            e.printStackTrace();
            return -1;
        }
    }

    // <uses-permission android:name="android.permission.WRITE_SECURE_SETTINGS"
    // /> required
    private boolean wifiSetWifiScanAlwaysValue(int value) {
        return Settings.Global.putInt(mContext.getContentResolver(),
                WIFI_SCAN_ALWAYS_AVAILABLE, value);
    }

    private static int wifiConvertFrequencyToChannelNumber(int frequency) {
        if (frequency >= 2412 && frequency <= 2484) {
            return (frequency - 2412) / 5 + 1;
        } else if (frequency >= 5170 && frequency <= 5825) {
            // DFS is included
            return (frequency - 5170) / 5 + 34;
        } else {
            return 0;
        }
    }

    private static List<ScanResult> wifiFilterScanResult(
            List<ScanResult> input, int maxSize) {
        List<ScanResult> out = new ArrayList<ScanResult>();
        for (ScanResult s : input) {
            if (out.size() == maxSize) {
                break;
            }
            if (s.BSSID.equals("00:00:00:00:00:00")) {
                continue;
            }
            out.add(s);
        }
        return out;
    }

    private static String wifiGetMacAddr() {
        try {
            List<NetworkInterface> all = Collections.list(NetworkInterface
                    .getNetworkInterfaces());
            for (NetworkInterface nif : all) {
                if (!nif.getName().equalsIgnoreCase("wlan0"))
                    continue;

                byte[] macBytes = nif.getHardwareAddress();
                if (macBytes == null) {
                    return "02:00:00:00:00:00";
                }

                StringBuilder res1 = new StringBuilder();
                for (byte b : macBytes) {
                    res1.append(String.format("%02X:", b));
                }

                if (res1.length() > 0) {
                    res1.deleteCharAt(res1.length() - 1);
                }
                return res1.toString();
            }
        } catch (Exception ex) {
        }
        return "02:00:00:00:00:00";
    }

    private void btProvideCapabilitiesHandler() {
        BluetoothProvideCapabilities c = new BluetoothProvideCapabilities();
        if (mBtAdp != null) {
            c.btSupported = true;
            c.btRssi = true;
            c.btleSupported = true;
            c.btleRssi = true;
        }
        if (!mBtSender.provideCapabilities(mClient, LPPE_SOCKET_BT_VERSION, c)) {
            loge("[bt] ProvideCapabilities() mBtSender.provideCapabilities() failed");
        }
    }

    private void btStartMeasurementHandler(BluetoothMeasurementRequest request) {
        if (mBtAdp == null) {
            loge("[bt] StartMeasurement() BT is not supported");
            return;
        }
        if (mBtState != BT_STATE_IDLE) {
            loge("[bt] StartMeasurement() already started, state=" + mBtState);
            return;
        }
        mBtSettingState = mBtAdp.isEnabled();
        log("[bt] StartMeasurement()  request=[" + request + "]");
        if (mBtSettingState) {
            mBtState = BT_STATE_SCANNING;
            log("[bt] StartMeasurement() startDiscovery()");
            if (!mBtAdp.startDiscovery()) {
                loge("[bt] StartMeasurement() startDiscovery() failed");
            }
        } else {
            mBtState = BT_STATE_ENABLING;
            log("[bt] StartMeasurement() enable()");
            if (!mBtAdp.enable()) {
                loge("[bt] StartMeasurement() enable() failed");
            }
        }
    }

    private void btStopMeasurementHandler() {
        if (mBtAdp == null) {
            loge("[bt] StopMeasurement() BT is not supported");
            return;
        }
        if (mBtState == BT_STATE_IDLE) {
            loge("[bt] StopMeasurement() already stopped");
            return;
        }
        log("[bt] StopMeasurement()");
        mBtState = BT_STATE_IDLE;
        mBtAdp.cancelDiscovery();
        if (!mBtSettingState) {
            log("[bt] StopMeasurement() disable() to restore to previous state");
            if (!mBtAdp.disable()) {
                loge("[bt] StopMeasurement() disable() failed");
            }
        }
    }

    private void btStateChangedHandler(Intent intent) {
        if (mBtAdp == null) {
            loge("[bt] StateChanged() BT is not supported");
            return;
        }
        if (mBtAdp.isEnabled()) {
            if (mBtState == BT_STATE_ENABLING) {
                mBtState = BT_STATE_SCANNING;
                log("[bt] StateChanged() startDiscovery()");
                if (!mBtAdp.startDiscovery()) {
                    loge("[bt] StateChanged() startDiscovery() failed");
                }
            }
        }
    }

    private void btDiscoveryStartedHandler(Intent intent) {
        // do nothing
    }

    private void btDiscoveryFinishedHandler(Intent intent) {
        if (mBtState != BT_STATE_IDLE) {
            mBtState = BT_STATE_SCANNING;
            if (!mBtSender.provideMeasurementFinished(mClient)) {
                loge("[bt] DiscoveryFinished() mBtSender.provideMeasurementFinished() failed");
            }
            log("[bt] DiscoveryFinished() startDiscovery()");
            if (!mBtAdp.startDiscovery()) {
                loge("[bt] DiscoveryFinished() startDiscovery() failed");
            }
        }
    }

    private void btDeviceFoundHandler(Intent intent) {
        if (mBtState != BT_STATE_IDLE) {
            btDumpDevice(intent);

            mBtState = BT_STATE_SCANNING;
            BluetoothDevice bt = intent
                    .getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

            BluetoothMeasurement meas = new BluetoothMeasurement();

            if (bt != null) {
                meas.mac = commonMacStringToByteArray(bt.getAddress());
                meas.category = btConvertTypeToCategory(bt.getType());
            }
            meas.rssiValid = true;
            meas.rssi = intent.getShortExtra(BluetoothDevice.EXTRA_RSSI,
                    Short.MIN_VALUE);

            if (!mBtSender.provideMeasurement(mClient, meas)) {
                loge("[bt] DeviceFound() mBtSender.provideMeasurement() failed");
            }
        }
    }

    private static BluetotthCategory btConvertTypeToCategory(int type) {
        switch (type) {
        case BluetoothDevice.DEVICE_TYPE_CLASSIC:
            return BluetotthCategory.bt;
        case BluetoothDevice.DEVICE_TYPE_LE:
            return BluetotthCategory.btle;
        case BluetoothDevice.DEVICE_TYPE_DUAL:
            return BluetotthCategory.bt;
        default:
            return BluetotthCategory.unknown;
        }
    }

    private static String btGetDevTypeString(int type) {
        if (type == BluetoothDevice.DEVICE_TYPE_CLASSIC) {
            return "BR/EDR";
        } else if (type == BluetoothDevice.DEVICE_TYPE_LE) {
            return "BLE";
        } else if (type == BluetoothDevice.DEVICE_TYPE_DUAL) {
            return "Dual";
        }
        return "Unknown";
    }

    private static void btDumpDevice(Intent intent) {
        BluetoothDevice bt = intent
                .getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        if (bt != null) {
            String name = bt.getName();
            String mac = bt.getAddress();
            int type = bt.getType();
            int rssi = intent.getShortExtra(BluetoothDevice.EXTRA_RSSI,
                    Short.MIN_VALUE);
            log("[bt] DumpDevice() tick=[" + getTick() + "] mac=[" + mac
                    + "] type=[" + btGetDevTypeString(type) + "] rssi=[" + rssi
                    + "] name=[" + name + "]");
        }
    }

    private void sensorProvideCapabilitiesHandler() {
        SensorProvideCapabilities c = new SensorProvideCapabilities();
        if (mBarometer != null) {
            c.atmosphericPressureSupport = true;
        }
        if (!mSensorSender.provideCapabilities(mClient,
                LPPE_SOCKET_SENSOR_VERSION, c)) {
            loge("[sensor] ProvideCapabilities() mSensorSender.provideCapabilities() failed");
        }
    }

    private void sensorStartMeasurementHandler(SensorMeasurementRequest request) {
        if (request.atmosphericPressureReq) {
            if (mBarometer != null) {
                log("[sensor] StartMeasurement() registerListener() for Barometer");
                mSensorManager.registerListener(mSensorBaroEventListener,
                        mBarometer, SensorManager.SENSOR_DELAY_NORMAL);
            } else {
                loge("[sensor] StartMeasurement() Barometer is not supported");
            }
        }
        if (request.motionStateReq) {
            loge("[sensor] StartMeasurement() Motion State is not supported");
        }
        if (request.secondaryMotionStateRequest) {
            loge("[sensor] StartMeasurement Secondary Motion State is not supported");
        }
    }

    private void sensorStopMeasurementHandler() {
        if(mSensorManager != null) {
            log("[sensor] StopMeasurement() unregisterListener() for Barometer");
            mSensorManager.unregisterListener(mSensorBaroEventListener);
        } else {
            loge("[sensor] StopMeasurement() SensorManager is not supported");
        }
    }

    private void sensorBaroAccuracyChangedHandler(Sensor sensor, int accuracy) {
        log("[sensor] BaroAccuracyChanged() sensor=[" + sensor + "]");
        switch (accuracy) {
        case SensorManager.SENSOR_STATUS_ACCURACY_HIGH:
            log("[sensor] BaroAccuracyChanged() accuracy is high");
            break;
        case SensorManager.SENSOR_STATUS_ACCURACY_MEDIUM:
            log("[sensor] BaroAccuracyChanged() accuracy is medium");
            break;
        case SensorManager.SENSOR_STATUS_ACCURACY_LOW:
            log("[sensor] BaroAccuracyChanged() accuracy is low");
            break;
        case SensorManager.SENSOR_STATUS_UNRELIABLE:
            log("[sensor] BaroAccuracyChanged() accuracy is unreliable");
            break;
        case SensorManager.SENSOR_STATUS_NO_CONTACT:
            log("[sensor] BaroAccuracyChanged() accuracy is no contact");
            break;
        default:
            loge("[sensor] BaroAccuracyChanged() unknown accuracy=[" + accuracy
                    + "]");
            break;
        }
    }

    private void sensorBaroSensorChangedHandler(SensorEvent event) {
        long currTime = getTick();
        if (currTime >= (mSensorLastUpdateTime + LPPE_SENSOR_UPDATE_INTERVAL)) {
            mSensorLastUpdateTime = currTime;
            log("[sensor] BaroSensorChanged()  isUncali=[" + mIsUncaliBarometer
                    + "] accuracy=[" + event.accuracy + "]" + " timestamp=["
                    + event.timestamp + "]" + " value=[" + event.values[0]
                    + "]");
            SensorPressureMeasurement meas = new SensorPressureMeasurement();
            meas.sensorMeasurement = (int) (event.values[0] * 100); // hPa to Pa
            if (!mSensorSender.providePressureMeasurement(mClient, meas)) {
                loge("[sensor] BaroSensorChanged() SensorSender.providePressureMeasurement() failed");
            }
        }
    }

    private void networkProvideCapabilitiesHandler() {
        if (!mNetworkSender.provideCapabilities(mClient,
                LPPE_SOCKET_NETWORK_VERSION, mNetworkSupported)) {
            loge("[network] ProvideCapabilities() mNetworkSender.provideCapabilities() failed");
        }
    }

    private void networkStartMeasurementHandler() {
        if (mNetworkSupported) {
            boolean isInEmergencyCall = false;
            if (mTelecom != null) {
                isInEmergencyCall = mTelecom.isInEmergencyCall();
            }

            if (isInEmergencyCall || checkLocationProxyAppPermission()) {
                LocationRequest request = LocationRequest.createFromDeprecatedProvider(
                        LocationManager.FUSED_PROVIDER,
                        1000 /*minTime*/, 0 /*minDistance*/, false /*oneShot*/);
                request.setHideFromAppOps(true);
                request.setQuality(LocationRequest.ACCURACY_FINE);

                if (isInEmergencyCall) {
                    if(isWhitelistPackageNameSet()) {
                        request.setLocationSettingsIgnored(true);
                    } else {
                        commonSendEventToHandlerWithDelay(HANDLER_NETWORK_START_MEAS,
                                LPPE_NETWORK_START_MEAS_DELAY);
                        log("[network] StartMeasurement() delay for whitelist package name set");
                        return;
                    }
                }
                log("[network] StartMeasurement() requestLocationUpdates()");
                mLocationManager.requestLocationUpdates(
                        request, mLocationListener, mHandler.getLooper());
            } else {
                loge("[network] StartMeasurement() fail due to "
                        + "no E911 call and nonFramework proxy app not granted");
            }
        } else {
            loge("[network] StartMeasurement() network location is not supported");
        }
    }

    private void networkStopMeasurementHandler() {
        if (mNetworkSupported) {
            log("[network] StopMeasurement() removeUpdates()");
            mLocationManager.removeUpdates(mLocationListener);
            removePackageInLocationSettingsWhitelist(mContext);
            mHandler.removeMessages(HANDLER_NETWORK_START_MEAS);
        } else {
            loge("[network] StopMeasurement() network location is not supported");
        }
    }

    private void networkRecheckCapabilitiesHandler() {
        mNetworkCapabilityRecheckCount--;
        log("[network] re-check capability() count=" + mNetworkCapabilityRecheckCount);
        if(mNetworkCapabilityRecheckCount >= 0) {
            mNetworkSupported = mLocationManager
                    .getProvider(LocationManager.NETWORK_PROVIDER) != null;
            if (!mNetworkSupported) {
                loge("[network] NETWORK_PROVIDER is not supported");
                commonSendEventToHandlerWithDelay(HANDLER_NETWORK_RECHECK_CAPABILITIES,
                    LPPE_NETWORK_RECHECK_INTERVAL);
            } else {
                triggerNetworkRequestCapabilities();
            }
        }
    }

    private void networkLocationChangedHandler(Location location) {
        if (!mNetworkSupported) {
            loge("[network] onLocationChanged() network location is not supported");
            return;
        }
        log("[network] onLocationChanged()");
        NetworkLocationMeasurement meas = new NetworkLocationMeasurement();
        meas.time = location.getTime();
        meas.latitude = location.getLatitude();
        meas.longitude = location.getLongitude();
        meas.accuracyValid = location.hasAccuracy();
        meas.accuracy = location.getAccuracy();
        meas.altitudeValid = location.hasAltitude();
        meas.altitude = location.getAltitude();
        meas.confidenceHorizontalValid = location.hasAccuracy();
        meas.confidenceHorizontal = 68;
        meas.locationSourceValid = true;
        meas.locationSource = (Location_Source_EcidLTE | Location_Source_EcidGSM
                | Location_Source_EcidUTRA | Location_Source_WlanAP);
        meas.verticalAccuracyValid = location.hasVerticalAccuracy();
        meas.verticalAccuracy = location.getVerticalAccuracyMeters();
        meas.confidenceVerticalValid = location.hasVerticalAccuracy();
        meas.confidenceVertical = 68;

        if (!mNetworkSender.provideMeasurement(mClient, meas)) {
            loge("[network] onLocationChanged() mNetworkSender.provideMeasurement() failed");
        }
    }

    private void ipaddrProvideCapabilitiesHandler() {
        IpAddressCapabilities capabilities = new IpAddressCapabilities();
        capabilities.ipv4 = true;
        capabilities.ipv6 = true;
        capabilities.nat = false;
        if (!mIpAddrSender.provideCapabilities(mClient,
                LPPE_SOCKET_IPADDR_VERSION, capabilities)) {
            loge("[ip] ipaddrProvideCapabilitiesHandler() mIpAddrSender.provideCapabilities() failed");
        }
    }

    private void lbsProvideCapabilitiesHandler() {
        String imei = "";
        TelephonyManager telephonyMgr
                = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        if (telephonyMgr != null) {
            if(telephonyMgr.getPhoneCount() == 2) {
                imei = telephonyMgr.getImei(0);
            } else {
                imei = telephonyMgr.getImei();
            }
        }
        if (imei == null) {
            imei = "";
            loge("[ip] ipaddrProvideCapabilitiesHandler() getImei failed");
        }

        LbsCapabilities capabilities = new LbsCapabilities();
        capabilities.suplInitOverSms = false;
        capabilities.batteryInfo = true;
        capabilities.imei = imei;
        if (!mLbsSender.provideCapabilities(mClient, LPPE_SOCKET_LBS_VERSION,
                capabilities)) {
            loge("[lbs] lbsProvideCapabilitiesHandler() mLbsSender.provideCapabilities() failed");
        }
        lbsBatteryForceUpdate();
        lbsNlpStatusUpdate();
    }

    private static boolean lbsIsAndroidOAfter() {
        int version = android.os.Build.VERSION.SDK_INT;
        if (version > 25) {
            // android O = 26
            return true;
        } else {
            return false;
        }
    }

    private void lbsBatteryForceUpdate() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_BATTERY_CHANGED);
        Intent intent = mContext.registerReceiver(null, filter);
        if (intent != null) {
            int level = intent.getIntExtra("level", 0);
            int scale = intent.getIntExtra("scale", 1);
            mLastBatteryPercentage = (level * 100) / scale;
            log("[lbs] lbsBatteryForceUpdate() level=[" + level + "] scale=[" + scale +
                "] percentage=[" + mLastBatteryPercentage + "]");
            if (!mLbsSender.updateBatteryInfo(mClient, mLastBatteryPercentage)) {
                loge("[lbs] lbsBatteryForceUpdate() mLbsSender.updateBatteryInfo() failed");
            }
        } else {
            loge("[lbs] lbsBatteryForceUpdate() get sticky intent fail");
        }
    }

    private void lbsBatteryChanged(Intent intent) {
        int level = intent.getIntExtra("level", 0);
        int scale = intent.getIntExtra("scale", 1);
        int batteryPercentage = (level * 100) / scale;
        if(mLastBatteryPercentage != batteryPercentage) {
            mLastBatteryPercentage = batteryPercentage;
            log("[lbs] lbsBatteryChanged() level=[" + level + "] scale=[" + scale +
                "] percentage=[" + mLastBatteryPercentage + "]");
            if(batteryPercentage % 5 == 0) {
                if (!mLbsSender.updateBatteryInfo(mClient, batteryPercentage)) {
                    loge("[lbs] lbsBatteryChanged() mLbsSender.updateBatteryInfo() failed");
                }
            }
        }
    }

    private void lbsNlpStatusUpdate() {
        boolean isEnabled = mLocationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);

        log("[lbs] lbsNlpStatusUpdate() isEnabled=" + isEnabled);
        if (!mLbsSender.updateNlpStatus(mClient, isEnabled)) {
            loge("[lbs] lbsNlpStatusUpdate() mLbsSender.updateNlpStatus() failed");
        }
    }

    private void lbsNlpStatusChanged(Intent intent) {
        lbsNlpStatusUpdate();
    }

    private void ipaddrProvideInformationHandler() {
        try {
            ArrayList<com.mediatek.location.lppe.ipaddr.IpAddress> ipList = new ArrayList<com.mediatek.location.lppe.ipaddr.IpAddress>();
            for (Enumeration<NetworkInterface> list = NetworkInterface
                    .getNetworkInterfaces(); list.hasMoreElements();) {
                NetworkInterface i = list.nextElement();
                // skip loop back
                if (i.getName().equals("lo")) {
                    continue;
                }
                for (Enumeration<InetAddress> list2 = i.getInetAddresses(); list2
                        .hasMoreElements();) {
                    InetAddress addr = list2.nextElement();
                    if (addr instanceof Inet4Address) {
                        com.mediatek.location.lppe.ipaddr.IpAddress ip = new com.mediatek.location.lppe.ipaddr.IpAddress();
                        ip.bearer = com.mediatek.location.lppe.ipaddr.Bearer.unknown;
                        ip.ipv4Valid = true;
                        ip.ipv4 = addr.getAddress();
                        ipList.add(ip);
                    }
                    if (addr instanceof Inet6Address) {
                        com.mediatek.location.lppe.ipaddr.IpAddress ip = new com.mediatek.location.lppe.ipaddr.IpAddress();
                        ip.bearer = com.mediatek.location.lppe.ipaddr.Bearer.unknown;
                        ip.ipv6Valid = true;
                        ip.ipv6 = addr.getAddress();
                        ipList.add(ip);
                    }
                    if (ipList.size() >= 5) {
                        break;
                    }
                }
                if (ipList.size() >= 5) {
                    break;
                }
            }

            log("[ip] ipaddrProvideInformationHandler() " + ipList);
            IpAddressInformation info = new IpAddressInformation();
            info.list = new com.mediatek.location.lppe.ipaddr.IpAddress[ipList
                    .size()];
            for (int i = 0; i < ipList.size(); i++) {
                info.list[i] = ipList.get(i);
            }
            if (!mIpAddrSender.provideInformation(mClient, info)) {
                loge("[ip] ipaddrProvideInformationHandler() mIpAddrSender.provideInformation() failed");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    private static void log(Object msg) {
        Log.d(TAG, "[" + LPPE_SERVICE_VERSION + "]" + msg);
        if (mLogCallback != null) {
            mLogCallback.onLog(false, "" + msg);
        }
    }

    private static void loge(Object msg) {
        Log.e(TAG, "[" + LPPE_SERVICE_VERSION + "]" + msg);
        if (mLogCallback != null) {
            mLogCallback.onLog(true, "" + msg);
        }
    }

    private static boolean stringCompare(String s1, String s2) {
        if (s1 != null) {
            return s1.equals(s2);
        }
        return false;
    }

    private static long getTick() {
        return System.nanoTime() / 1000000;
    }

    public void triggerWifiRequestCapabilities() {
        UdpClient c = new UdpClient(LPPE_SOCKET_WLAN_CHANNEL,
                Namespace.ABSTRACT, LPPeWlan.MAX_BUFF_SIZE);
        mWifiSender.requestCapabilities(c, 101);
    }

    public void triggerWifiStartMeas() {
        UdpClient c = new UdpClient(LPPE_SOCKET_WLAN_CHANNEL,
                Namespace.ABSTRACT, LPPeWlan.MAX_BUFF_SIZE);
        mWifiSender.startMeasurement(c, true, new WlanMeasurements());
    }

    public void triggerWifiStopMeas() {
        UdpClient c = new UdpClient(LPPE_SOCKET_WLAN_CHANNEL,
                Namespace.ABSTRACT, LPPeWlan.MAX_BUFF_SIZE);
        mWifiSender.stopMeasurement(c);
    }

    public void triggerBtRequestCapabilities() {
        UdpClient c = new UdpClient(LPPE_SOCKET_BT_CHANNEL, Namespace.ABSTRACT,
                LPPeBluetooth.MAX_BUFF_SIZE);
        mBtSender.requestCapabilities(c, 102);
    }

    public void triggerBtStartMeas() {
        UdpClient c = new UdpClient(LPPE_SOCKET_BT_CHANNEL, Namespace.ABSTRACT,
                LPPeBluetooth.MAX_BUFF_SIZE);
        BluetoothMeasurementRequest request = new BluetoothMeasurementRequest();
        request.btMultipleMeasurement = BluetoothMultipleMeasurement.allowed;
        request.btRtd = true;

        request.btleMultipleMeasurement = BluetoothMultipleMeasurement.allowed;
        request.btleRtd = true;
        mBtSender.startMeasurement(c, true, request);
    }

    public void triggerBtStopMeas() {
        UdpClient c = new UdpClient(LPPE_SOCKET_BT_CHANNEL, Namespace.ABSTRACT,
                LPPeBluetooth.MAX_BUFF_SIZE);
        mBtSender.stopMeasurement(c);
    }

    public void triggerSensorRequestCapabilities() {
        UdpClient c = new UdpClient(LPPE_SOCKET_SENSOR_CHANNEL,
                Namespace.ABSTRACT, LPPeSensor.MAX_BUFF_SIZE);
        mSensorSender.requestCapabilities(c, 103);
    }

    public void triggerSensorStartMeas() {
        UdpClient c = new UdpClient(LPPE_SOCKET_SENSOR_CHANNEL,
                Namespace.ABSTRACT, LPPeSensor.MAX_BUFF_SIZE);
        SensorMeasurementRequest request = new SensorMeasurementRequest();
        request.atmosphericPressureReq = true;
        request.motionStateReq = true;
        request.secondaryMotionStateRequest = true;
        mSensorSender.startMeasurement(c, request);
    }

    public void triggerSensorStopMeas() {
        UdpClient c = new UdpClient(LPPE_SOCKET_SENSOR_CHANNEL,
                Namespace.ABSTRACT, LPPeSensor.MAX_BUFF_SIZE);
        mSensorSender.stopMeasurement(c);
    }

    public void triggerNetworkRequestCapabilities() {
        UdpClient c = new UdpClient(LPPE_SOCKET_NETWORK_CHANNEL,
                Namespace.ABSTRACT, LPPeNetwork.MAX_BUFF_SIZE);
        mNetworkSender.requestCapabilities(c, 104);
    }

    public void triggerNetworkStartMeas() {
        UdpClient c = new UdpClient(LPPE_SOCKET_NETWORK_CHANNEL,
                Namespace.ABSTRACT, LPPeNetwork.MAX_BUFF_SIZE);
        mNetworkSender.startMeasurement(c);
    }

    public void triggerNetworkStopMeas() {
        UdpClient c = new UdpClient(LPPE_SOCKET_NETWORK_CHANNEL,
                Namespace.ABSTRACT, LPPeNetwork.MAX_BUFF_SIZE);
        mNetworkSender.stopMeasurement(c);
    }

    public void triggerIpAddrRequestCapabilities() {
        UdpClient c = new UdpClient(LPPE_SOCKET_IPADDR_CHANNEL,
                Namespace.ABSTRACT, LPPeIpAddress.MAX_BUFF_SIZE);
        mIpAddrSender.requestCapabilities(c, 105);
    }

    public void triggerIpAddrRequestInformation() {
        UdpClient c = new UdpClient(LPPE_SOCKET_IPADDR_CHANNEL,
                Namespace.ABSTRACT, LPPeIpAddress.MAX_BUFF_SIZE);
        mIpAddrSender.requestInformation(c);
    }

    public void triggerLbsRequestCapabilities() {
        UdpClient c = new UdpClient(LPPE_SOCKET_LBS_CHANNEL,
                Namespace.ABSTRACT, LPPeLbs.MAX_BUFF_SIZE);
        mLbsSender.requestCapabilities(c, 106);
    }

    public void triggerLbsReceiveSuplInit() {
        byte[] suplInit = new byte[] { 0x00, 0x78, 0x02, 0x00, 0x00, 0x40,
                0x00, 0x00, 0x00, 0x46, 0x05, 0x40, 0x01, 0x18, 0x60, 0x03,
                (byte) 0xd2, (byte) 0xdf, (byte) 0xef, (byte) 0xf0, 0x05, 0x30,
                0x07, 0x00, 0x06, (byte) 0x90, 0x07, 0x20, 0x06, 0x50, 0x06,
                (byte) 0xe0, 0x07, 0x40, 0x02, 0x00, 0x04, 0x30, 0x06,
                (byte) 0xf0, 0x06, (byte) 0xd0, 0x06, (byte) 0xd0, 0x07, 0x50,
                0x06, (byte) 0xe0, 0x06, (byte) 0x90, 0x06, 0x30, 0x06, 0x10,
                0x07, 0x40, 0x06, (byte) 0x90, 0x06, (byte) 0xf0, 0x06,
                (byte) 0xe0, 0x07, 0x30, (byte) 0xb7, (byte) 0xfb, (byte) 0xfc,
                0x01, 0x4c, 0x01, (byte) 0xc0, 0x01, (byte) 0xa4, 0x01,
                (byte) 0xc8, 0x01, (byte) 0x94, 0x01, (byte) 0xb8, 0x01,
                (byte) 0xd0, 0x00, (byte) 0x80, 0x01, 0x0c, 0x01, (byte) 0xbc,
                0x01, (byte) 0xb4, 0x01, (byte) 0xb4, 0x01, (byte) 0xd4, 0x01,
                (byte) 0xb8, 0x01, (byte) 0xa4, 0x01, (byte) 0x8c, 0x01,
                (byte) 0x84, 0x01, (byte) 0xd0, 0x01, (byte) 0xa4, 0x01,
                (byte) 0xbc, 0x01, (byte) 0xb8, 0x01, (byte) 0xcc, 0x00, 0x20,
                (byte) 0xc8, 0x60, 0x30, 0x00, (byte) 0x80, (byte) 0x80, 0x00 };
        UdpClient c = new UdpClient(LPPE_SOCKET_LBS_CHANNEL,
                Namespace.ABSTRACT, LPPeLbs.MAX_BUFF_SIZE);
        mLbsSender.receiveSuplInit(mClient, suplInit);
    }

    public interface LPPeServiceLogCallback {
        public void onLog(boolean isError, String msg);
    }

    private void sendDelayedMessage(int what, long delayMillis) {
        mHandler.sendEmptyMessageDelayed(what, delayMillis);
    }

    private void addPackageInLocationSettingsWhitelist(Context context) {
        final String LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST =
                "location_ignore_settings_package_whitelist";
        String packageName = context.getPackageName();

        String whitelist = Settings.Global.getString(
                context.getContentResolver(),
                LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST);
        if (whitelist == null || whitelist.indexOf(packageName) == -1) {
            String outStr = (whitelist == null) ? "" : whitelist + ",";
            outStr += packageName;
            log("Ignore settings outStr = " + outStr);
            Settings.Global.putString(
                    context.getContentResolver(),
                    LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST,
                    outStr);
        }
    }

    private void removePackageInLocationSettingsWhitelist(Context context) {
        final String LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST =
                "location_ignore_settings_package_whitelist";
        String packageName = context.getPackageName();

        String whitelist = Settings.Global.getString(
                context.getContentResolver(),
                LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST);
        int index = -1;
        String outStr = "";
        if (whitelist != null) {
            index = whitelist.indexOf("," + packageName);
            if (index != -1) { /// found ','+package
                outStr = whitelist.replace("," + packageName, "");
            } else { /// not found, try to find package name only
                index = whitelist.indexOf(packageName);
                if(index != -1) {
                    outStr = whitelist.replace(packageName, "");
                }
            }
        }

        if (index != -1) { /// outStr is replaced as new whitelist
            log("Remove ignore settings outStr = " + outStr);
            Settings.Global.putString(
                    context.getContentResolver(),
                    LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST,
                    outStr);
        }
    }

    private boolean isWhitelistPackageNameSet() {
        String packageName = mContext.getPackageName();
        String whitelistPackages[] = mLocationManager.getIgnoreSettingsWhitelist();

        for (String s : whitelistPackages) {
            if (s != null && s.equals(packageName)) {
                return true;
            }
        }
        return false;
    }

    private boolean isPackageInstalled(String packagename) {
        try {
            mPackageManager.getPackageInfo(packagename, 0);
            return true;
        } catch (NameNotFoundException e) {
            return false;
        }
    }

    String loadProxyNameFromCarrierConfig() {
        CarrierConfigManager configManager = (CarrierConfigManager)
                mContext.getSystemService(Context.CARRIER_CONFIG_SERVICE);
        if (configManager == null) {
            return "";
        }

        int ddSubId = SubscriptionManager.getDefaultDataSubscriptionId();
        PersistableBundle configs = SubscriptionManager.isValidSubscriptionId(ddSubId)
                ? configManager.getConfigForSubId(ddSubId) : null;
        if (configs == null) {
            Log.d(TAG, "SIM not ready, use default carrier config.");
            configs = CarrierConfigManager.getDefaultConfig();
        }

        String value = (String) configs.get(CarrierConfigManager.Gps.KEY_NFW_PROXY_APPS_STRING);
        Log.d(TAG, CarrierConfigManager.Gps.KEY_NFW_PROXY_APPS_STRING + ": " + value);
        if (value != null) {
            String[] strings = value.trim().split(" ");
            /// use first one package as proxy package
            return strings[0];
        } else {
            return "";
        }
    }

    private boolean hasLocationPermission(String pkgName) {
        return (mPackageManager.checkPermission(LOCATION_PERMISSION_NAME, pkgName)
                == PackageManager.PERMISSION_GRANTED);
    }

    private boolean checkLocationProxyAppPermission() {
        String mGnssProxyPackageName = loadProxyNameFromCarrierConfig();
        if (isPackageInstalled(mGnssProxyPackageName)) {
            boolean proxyAppLocationGranted = hasLocationPermission(mGnssProxyPackageName);
            log("proxyApp = " + mGnssProxyPackageName
                    + " proxyAppLocationGranted = " + proxyAppLocationGranted);
            return proxyAppLocationGranted;
        } else {
            log("nfw proxy package is not installed: " + mGnssProxyPackageName);
        }
        return false;
    }
}

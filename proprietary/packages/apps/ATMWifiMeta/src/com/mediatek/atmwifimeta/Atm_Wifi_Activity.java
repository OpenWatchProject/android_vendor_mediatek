package com.mediatek.atmwifimeta;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.content.ContentResolver;
import java.util.Map;
import android.content.IntentFilter;
//import android.net.wifi.ScanSettings;
import android.net.wifi.WifiScanner;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import android.os.ServiceManager;
import android.os.IBinder;
import android.os.RemoteException;
import java.util.Collection;
import android.os.Parcel;
import android.os.Parcelable;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiConfiguration.AuthAlgorithm;
import android.net.wifi.WifiConfiguration.KeyMgmt;
import android.net.wifi.SupplicantState;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiInfo;
import android.net.IpConfiguration;
import android.net.IpConfiguration.IpAssignment;
import java.net.InetAddress;
import android.net.StaticIpConfiguration;
import java.net.Inet4Address;
import android.net.NetworkUtils;
import android.net.LinkAddress;
import android.os.SystemProperties;
import java.lang.InterruptedException;
import java.lang.reflect.Method;

import android.net.wifi.WifiConfiguration.KeyMgmt;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.RemoteException;
import android.util.Log;
import vendor.mediatek.hardware.log.V1_0.ILog;
import vendor.mediatek.hardware.log.V1_0.ILogCallback;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.reflect.Method;
import java.util.NoSuchElementException;

import android.os.HwBinder;



public class Atm_Wifi_Activity extends Activity implements View.OnClickListener {

    private static final String TAG = "Atm_Wifi_Activity";

    //default config
    private static final String PATH = "/sdcard/wifi_socket_config.ini";
    private static final String SECTION_NAME = "config_info";
    private String default_target_ssid = "oppometa";
    private String default_target_on_rssi = "-45";
    private String default_target_off_rssi = "-55";
    private String default_target_channel = "6";
    private String default_target_ip_address = "192.168.43.108";
    private String default_target_network_dns = "192.168.43.1";
    private String default_target_network_gateway = "192.168.43.1";
    private String default_target_auto_connect_retry = "1";
    private String default_target_auto_disconnect_retry = "2";
    private	String default_target_network_keyMgmt = "NONE";
    private	String default_target_network_password = null;
    private String default_als_threshold = "5";
    private String default_ps_threshold = "0";
    private String default_use_dhcp = "0";
    private String default_check_alsps = "0";

    //ATM properties
    private static final String META_CONNECT_TYPE = "persist.vendor.meta.connecttype";
    private static final String ATM_IP_ADDRESS = "persist.vendor.atm.ipaddress";
    private static final String ATM_MD_MODE = "persist.vendor.atm.mdmode";


	
 
    //ATM socket and command
    private static final String ATM_COMM_SOCKET = "com.mediatek.atm.communication";
    private static final String ATM_CMD_SWITCH_META_TO_USB = "ATM_SWITCH_META_TO_USB";
    private static final String ATM_CMD_SWITCH_META_TO_WIFI = "ATM_SWITCH_META_TO_WIFI";
	private static final String ATM_CMD_SWITCH_META_TO_WIFI_IP = "ATM_WIFI_INFO_IP_ADDRESS";
    private static final String ATM_CMD_DISCONNECT_WIFI = "ATM_DESTORY_WIFI_SOCKET";
    private static final String ATM_CMD_CONNECT_WIFI = "ATM_NEW_WIFI_SOCKET";
    private static final String ATM_CMD_SWITCH_MODEM_TO_META = "ATM_SWITCH_MODEM_TO_META";
    private static final String ATM_CMD_SWITCH_MODEM_TO_NORMAL = "ATM_SWITCH_MODEM_TO_NORMAL";
    private static final String ATM_CMD_EXIT_ACTIVITY = "ATM_EXIT_WIFI_APP";

    private int connectThreshold = 0;
    private int disconnectThreshold = 0;
    int targetConnectRssi = 0;
    int targetDisonnectRssi = 0;
    String target_network_keygmt = "NONE";
    String target_network_password = null;

    boolean startScanThread = false;

    String connectBSSID = null;
    boolean isFirstTimeRSSI = true;

    String pre_connect_type = "USB";
    String cur_connect_type = "USB";

    private WifiManager mWifiManager;
    NetworkInfo.DetailedState lastState = NetworkInfo.DetailedState.IDLE;

    private int use_dhcp = -1;

    private SensorManager sensorManager;
    private int alsData = -1;
    private int psData = -1;
    private int alsThreshold = -1;
    private int psThreshold = -1;
    private int check_alsps = 0;
    boolean isDutInBox = false;

    //save ini
    Map<String,String> config_ini_map;

    Handler handler;
    private static final int UPDATE_CONNECT_TYPE = 0x01;
    private static final int UPDATE_ALSPS_STATUS = 0x02;
	private static final int MSG_EXIT_ACTIVITY = 0x03;
	
    private static final String ACTION_AUTO_EXIT_ACTIVITY = "Exit_Meta_Info_Activity";
    private static final String ATM_FLAG_PROP = "ro.boot.atm";
    private static final String ATM_ENABLED = "enable";

    WifiConfiguration config = new WifiConfiguration();


	//LOG HIDL service (Client)
	private static ILog mATMWiFiHIDLService = null;


    Handler mHandler;

    //main ui compontent
    TextView ALS_Value;
    TextView PS_Value;
    TextView Target_SSID;
    TextView Auto_Connect_RSSI;
    TextView Auto_Disconnect_RSSI;
    TextView Target_Channel;
    TextView Target_IP;
    TextView Target_Gateway;
    TextView Target_DNS;
    TextView Auto_Connect_Retry;
    TextView Auto_Disconnect_Retry;
    TextView WIFI_Status;
    TextView Current_RSSI;
    TextView MAC_Address;
    TextView Connect_Type;
    Button exit_Button;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if(!getATMEnabled()) //Exit activity if ATM is not enabled
            finish();

        setContentView(R.layout.atm_wifi_activity);
        ALS_Value = (TextView) findViewById(R.id.ALS_Value_value);
        PS_Value = (TextView) findViewById(R.id.PS_Value_value);
        Target_SSID = (TextView) findViewById(R.id.Target_SSID_value);
        Auto_Connect_RSSI = (TextView) findViewById(R.id.Auto_Connect_RSSI_value);
        Auto_Disconnect_RSSI = (TextView) findViewById(R.id.Auto_Disconnect_RSSI_value);
        Target_Channel = (TextView) findViewById(R.id.Target_Channel_value);
        Target_IP = (TextView) findViewById(R.id.Target_IP_value);
        Target_Gateway = (TextView) findViewById(R.id.Target_Gateway_value);
        Target_DNS = (TextView) findViewById(R.id.Target_DNS_value);
        Auto_Connect_Retry = (TextView) findViewById(R.id.Auto_Connect_Retry_value);
        Auto_Disconnect_Retry = (TextView) findViewById(R.id.Auto_Disconnect_Retry_value);
        WIFI_Status = (TextView) findViewById(R.id.WIFI_Status_value);
        Current_RSSI = (TextView) findViewById(R.id.Current_RSSI_value);
        MAC_Address = (TextView) findViewById(R.id.MAC_Address_value);
        Connect_Type = (TextView) findViewById(R.id.Connect_Type_value);
        exit_Button = (Button) findViewById(R.id.Exit_Button);
        exit_Button.setOnClickListener(this);
        mWifiManager = (WifiManager)getSystemService(WIFI_SERVICE);

        //register WifiStateReceiver
        new WifiStateReceiver().register(this) ;

        //register als and ps sensor listener
        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        Sensor als_sensor = sensorManager.getDefaultSensor(Sensor.TYPE_LIGHT);
        Sensor ps_sensor = sensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
        sensorManager.registerListener(alsListener, als_sensor, SensorManager.SENSOR_DELAY_NORMAL);
        sensorManager.registerListener(psListener, ps_sensor, SensorManager.SENSOR_DELAY_NORMAL);

        //register auto exit broadcast receiver
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_AUTO_EXIT_ACTIVITY);

        mHandler = new Handler() {
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case UPDATE_CONNECT_TYPE:
                        if (Connect_Type != null) {
                            Log.d(TAG,"update connect type:" + (String)msg.obj);
                            Connect_Type.setText((String)msg.obj);
                        }
                        break;
                    case UPDATE_ALSPS_STATUS:
                        if (ALS_Value != null && PS_Value != null) {
                            ALS_Value.setText((String)msg.obj);
                            PS_Value.setText((String)msg.obj);
                        }
						break;
 					case MSG_EXIT_ACTIVITY:
						Log.d(TAG,"Exit Activity with msg");
						exitActivity();
						break;
                    default:
                        break;
                }
            }
        };

        Log.d(TAG,"load config_ini_map");
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    config_ini_map = new AtmConfigReader(PATH).getSingleMap(SECTION_NAME);
                    Target_SSID.setText(
                        config_ini_map.getOrDefault(new String("target_ssid"),default_target_ssid) );
                    targetConnectRssi = Integer.parseInt(
                        config_ini_map.getOrDefault(new String("target_on_rssi"),default_target_on_rssi) );
                    Auto_Connect_RSSI.setText(
                        String.valueOf(targetConnectRssi) );
                    targetDisonnectRssi = Integer.parseInt(
                        config_ini_map.getOrDefault(new String("target_off_rssi"),default_target_off_rssi) );
                    Auto_Disconnect_RSSI.setText(
                        String.valueOf(targetDisonnectRssi) );
                    Target_Channel.setText(
                        config_ini_map.getOrDefault(new String("target_channel"),default_target_channel) );
                    Target_IP.setText(
                        config_ini_map.getOrDefault(new String("target_ip_address"),default_target_ip_address) );
                    Target_Gateway.setText(
                        config_ini_map.getOrDefault(new String("target_network_gateway"),default_target_network_gateway) );
                    Target_DNS.setText(
                        config_ini_map.getOrDefault(new String("target_network_dns"),default_target_network_dns) );
                    connectThreshold = Integer.parseInt(
                        config_ini_map.getOrDefault(new String("target_auto_connect_delay"),default_target_auto_connect_retry) );
                    Auto_Connect_Retry.setText(
                        String.valueOf(connectThreshold) );
                    disconnectThreshold = Integer.parseInt(
                        config_ini_map.getOrDefault(new String("target_auto_disconnect_delay"),default_target_auto_disconnect_retry) );
                    Auto_Disconnect_Retry.setText(
                        String.valueOf(disconnectThreshold) );
                    target_network_keygmt = 
                        config_ini_map.getOrDefault( new String("target_network_keyMgmt"),default_target_network_keyMgmt );
                    target_network_password = 
                        config_ini_map.getOrDefault( new String("target_network_password"),default_target_network_password );
                    alsThreshold = Integer.parseInt(
                        config_ini_map.getOrDefault(new String("als_data_threshold"),default_als_threshold) );
                    psThreshold = Integer.parseInt(
                        config_ini_map.getOrDefault(new String("ps_data_threshold"),default_ps_threshold) );
                    use_dhcp = Integer.parseInt(
                        config_ini_map.getOrDefault(new String("use_dhcp"),default_use_dhcp) );
                    check_alsps = Integer.parseInt(
                        config_ini_map.getOrDefault(new String("check_alsps"),default_check_alsps) );
                    Log.d(TAG,"ini load completed");
                } catch (Exception e) {
                    Target_SSID.setText( default_target_ssid );
                    targetConnectRssi = Integer.parseInt( default_target_on_rssi );
                    Auto_Connect_RSSI.setText( String.valueOf(targetConnectRssi) );
                    targetDisonnectRssi = Integer.parseInt( default_target_off_rssi );
                    Auto_Disconnect_RSSI.setText( String.valueOf(targetDisonnectRssi) );
                    Target_Channel.setText( default_target_channel );
                    Target_IP.setText( default_target_ip_address );
                    Target_Gateway.setText( default_target_network_gateway );
                    Target_DNS.setText( default_target_network_dns );
                    connectThreshold = Integer.parseInt( default_target_auto_connect_retry );
                    Auto_Connect_Retry.setText( String.valueOf(connectThreshold) );
                    disconnectThreshold = Integer.parseInt( default_target_auto_disconnect_retry );
                    Auto_Disconnect_Retry.setText( String.valueOf(disconnectThreshold) );
                    target_network_keygmt = default_target_network_keyMgmt;
                    target_network_password = default_target_network_password;
                    alsThreshold = Integer.parseInt( default_als_threshold );
                    psThreshold = Integer.parseInt( default_ps_threshold );
                    use_dhcp = Integer.parseInt( default_use_dhcp );
                    check_alsps = Integer.parseInt( default_check_alsps );
                    Log.d(TAG,"ini not found or io exception happened, loaded the default value");
                    e.printStackTrace();
                }
            }
        }).start();

        //show connect type
        new Thread(new Runnable() {
            @Override
            public void run() {
                Message msg = null;
                while(true) {
                    try{
                        Thread.sleep(50);
                        if (!cur_connect_type.equals(pre_connect_type)) {
                            Log.d(TAG,"connect type changed from " + pre_connect_type + " to " + cur_connect_type);
                            msg = Message.obtain(mHandler,UPDATE_CONNECT_TYPE,cur_connect_type);
                            msg.sendToTarget();
                            pre_connect_type = cur_connect_type;
                        }
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }).start();

        //new thread to read ALSPS status
        new Thread(new Runnable() {
            @Override
            public void run() {
                String alsps_status="";
                Message msg = null;
                if(0 != check_alsps) {
                    int retry_cnt = 0;
                    while(false == isDutInBox) {
                        try {
                            Log.d(TAG, "checkAlsPsData: als_data =" + alsData + ", ps_data = " + psData);
                            if(alsData < alsThreshold && psData > psThreshold) {
                                retry_cnt++;
                                Log.d(TAG, "checkAlsPsData: DUT inside shielding box for " + retry_cnt);
                                if(retry_cnt == 5) {
                                    Log.d(TAG, "checkAlsPsData: confirm DUT inside shielding box");
                                    isDutInBox = true;
                                    alsps_status = "PASS";
                                    msg = Message.obtain(mHandler, UPDATE_ALSPS_STATUS, alsps_status);
                                    msg.sendToTarget();
                                    break;
                                }
                                Thread.sleep(200);
                            } else {
                                retry_cnt = 0;
                                Log.d(TAG, "checkAlsPsData: DUT outside of shielding box");
                                Thread.sleep(200);
                                continue;
                            }
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                }
                else {
                    isDutInBox = true;
                    alsps_status = "NO CHECK";
                    msg = Message.obtain(mHandler, UPDATE_ALSPS_STATUS, alsps_status);
                    msg.sendToTarget();
                }
            }
        }).start();

        //new thread to control scan
        final Thread scanThread = new Thread(new Runnable() {
            @Override
            public void run() {
                while(startScanThread) {
                    startScanFreq();
                }
            }
        });

        //scanThread.start();
        //open wifi and set scanalways to false
        turnOnWifi();
        removeAllConfigNetworks();
        setTargetNetworkConfig();
        startScanFreq();
    }

    private boolean getATMEnabled() {
//        String ATM_flag = systemPropertyGet(ATM_FLAG_PROP,"");
        String ATM_flag = SystemProperties.get(ATM_FLAG_PROP,"");
		
        if(ATM_ENABLED.equals(ATM_flag))
            return true;
        else
            return false;
    }

    
    
    
    
    private void exitActivity() {
        Log.d(TAG,"exit atm_wifi_activity");
        //Check whether modem is in normal mode
        CheckModemMode();
        //Switch meta tst to USB

		SendMessageToMETATst(ATM_CMD_SWITCH_META_TO_USB);
		
 //       systemPropertySet(META_CONNECT_TYPE,"usb");
 //		SystemProperties.set(META_CONNECT_TYPE,"usb");


		
        cur_connect_type = "USB";
        //Close wifi and enable scan always
        mWifiManager.setWifiEnabled(false);
        finish();
    }

    protected void onDestroy() {
        super.onDestroy();
        if(sensorManager != null) {
            sensorManager.unregisterListener(alsListener);
            sensorManager.unregisterListener(psListener);
        }
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == R.id.Exit_Button) {
            exitActivity();
        }
    }

    @Override
    public void onBackPressed() {
            exitActivity();
    }

    private class WifiStateReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
           String action = intent.getAction();
            if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)) {
                Log.d(TAG,"WIFI_STATE_CHANGED_ACTION");
            } else if (WifiManager.SCAN_RESULTS_AVAILABLE_ACTION.equals(action)) {
                Log.d(TAG,"SCAN_RESULTS_AVAILABLE_ACTION");
                String targetBssid = getScanResults();
                if (targetBssid != null) {
                    connectThreshold = Integer.parseInt(Auto_Connect_Retry.getText().toString());
                    handleConnectNetwork(targetBssid);
                }
            } else if (WifiManager.NETWORK_STATE_CHANGED_ACTION.equals(action)) {
                NetworkInfo info = (NetworkInfo) intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
                Log.d(TAG,"NETWORK_STATE_CHANGED_ACTION is " + info.getDetailedState());
                handleNetworkStateChange(info);
            } else if (WifiManager.RSSI_CHANGED_ACTION.equals(action)) {
                Log.d(TAG,"RSSI_CHANGED_ACTION");
                int newRssi = intent.getIntExtra(WifiManager.EXTRA_NEW_RSSI, 0);
                handleRssiChange(newRssi);
            }
        }

        void register(Context context) {
            IntentFilter mIntentFilter = new IntentFilter();
            mIntentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
            mIntentFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
            mIntentFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
            mIntentFilter.addAction(WifiManager.RSSI_CHANGED_ACTION);
            context.registerReceiver(this, mIntentFilter);
        }
    }

    private String getScanResults() {
        if(false == isDutInBox) {
            startScanFreq();
            return null;
        }
        String targetSSID = Target_SSID.getText().toString();

        int countApOverRssi = 0;
        int showRSSI = -999;

        //Log.d(TAG,"targetSSID is "+targetSSID +"targetConnectRssi is "+targetConnectRssi);
        List<ScanResult> newResults = mWifiManager.getScanResults();
        for (ScanResult newResult : newResults) {
            if (newResult.SSID == null || newResult.SSID.isEmpty()) {
                continue;
            }
            String scanSSID = newResult.SSID;
            int scanRSSI = newResult.level;
            String scanBSSID = newResult.BSSID;
            if ((scanRSSI > showRSSI) && (scanSSID.equals(targetSSID))) {
                showRSSI = scanRSSI;
                Log.d(TAG, "update showRSSI :" + showRSSI);
            }
            Log.d(TAG, "[DEBUG] targetConnectRssi :" + targetConnectRssi);
            //only one SSID's rssi is more than connect threshold
            if ((scanRSSI > targetConnectRssi) && (scanSSID.equals(targetSSID))) {
                //Scanned RSSI above threshold
                countApOverRssi++;
                Log.d(TAG,"scaned scanRSSI > targetConnectRssi");
                connectThreshold--;
                Log.d(TAG,"connectThreshold is" + connectThreshold);

                if (connectThreshold < 1) {
                    Log.d(TAG,"targetSSID is "+scanSSID +"targetConnectRssi is "+scanRSSI);
                    connectBSSID = newResult.BSSID;
                    Log.d(TAG,"connectBSSID is "+connectBSSID);
                    connectThreshold = Integer.parseInt(Auto_Connect_Retry.getText().toString());
                } else {
                    startScanFreq();
                }
                break;
            } else if((connectBSSID != null)&&(scanBSSID.equals(connectBSSID))) {
                //Scanned RSSI below disconnect threshold
                Log.d(TAG,"scaned connectedSSID" );
                if (scanRSSI < targetDisonnectRssi) {
                    disconnectThreshold--;
                    Log.d(TAG,"disconnectThreshold is" + disconnectThreshold);
                }
                if (disconnectThreshold <1) {
                    Log.d(TAG,"disconnect "+scanBSSID);
                    mWifiManager.disconnect();
                    isFirstTimeRSSI = true;
                    removeAllConfigNetworks();
                    disconnectThreshold = Integer.parseInt(Auto_Disconnect_Retry.getText().toString());
                }
                break;
            }else
                continue;
        }
        if (countApOverRssi == 0) {
            //Restart scan when no RSSI meets request
            startScanFreq();
        }
        Current_RSSI.setText(String.valueOf(showRSSI));
        return connectBSSID;
    }

    private void handleConnectNetwork(String bssid) {
        config.BSSID = bssid;
        Log.d(TAG,"connect network mac is " + bssid);
        Log.d(TAG,"disable system api ");

        mWifiManager.connect(config,null);
    }

    private void removeAllConfigNetworks() {
        Log.d(TAG,"removeAllConfigNetworks");
        List<WifiConfiguration> networks = mWifiManager.getConfiguredNetworks();
        if (networks != null) {
        //Log.d(TAG,"networks = " + networks );
            int length = networks.size();
            for (int i = 0; i < length; i++) {
                mWifiManager.forget(networks.get(i).networkId ,null);
        }
        Log.d(TAG,"forget all saved network");
       }
    }

    private static String convertToQuotedString(String string) {
       return "\"" + string + "\"";
    }

    private static String quotedString(String s) {
        return String.format("\"%s\"", s);
    }

    private WifiConfiguration setTargetNetworkConfig() {
        //need add a password and kgt
        //set config ssid
        config.SSID = convertToQuotedString(Target_SSID.getText().toString());
        //config.channel = Integer.parseInt(Target_Channel.getText().toString());
        //int key = 0;
        for (int i=0;i<8;i++) {
            if (KeyMgmt.strings[i].equals(target_network_keygmt)) {
                config.allowedKeyManagement.set(i);
                break;
            }
        }
        if (target_network_password != null) {
            config.preSharedKey = quotedString(target_network_password);
        }
        //set ip config
        if(use_dhcp == 0) {
            StaticIpConfiguration staticIpConfig = new StaticIpConfiguration();
            //get ipaddress
            String ipAddr = Target_IP.getText().toString();
           // systemPropertySet(ATM_IP_ADDRESS,ipAddr);
			
			//SystemProperties.set(ATM_IP_ADDRESS,ipAddr);
		   //change to hidl
		    SendMessageToMETATst(ATM_CMD_SWITCH_META_TO_WIFI_IP+ ":" + ipAddr);

            Inet4Address inetAddr = getIPv4Address(ipAddr);
            //get networkPrefixLength default is 24
            int networkPrefixLength = 24;
            staticIpConfig.ipAddress = new LinkAddress(inetAddr, networkPrefixLength);

            //set gateway
            String gateway = Target_Gateway.getText().toString();
            InetAddress gatewayAddr = getIPv4Address(gateway);
            staticIpConfig.gateway = gatewayAddr;

            //set dnsserver
            String dns = Target_DNS.getText().toString();
            InetAddress dnsAddr = getIPv4Address(dns);
            staticIpConfig.dnsServers.add(dnsAddr);
            staticIpConfig.dnsServers.add(dnsAddr);

            config.setIpAssignment(IpAssignment.STATIC);
            config.setStaticIpConfiguration(staticIpConfig);
        }

        Log.d(TAG,"save the target ap  " + config);

        return config;
    }

    private Inet4Address getIPv4Address(String text) {
        try {
            text = text.split("/")[0];
            Log.d(TAG,"Ip address: " + text);
            return (Inet4Address) NetworkUtils.numericToInetAddress(text);
        } catch (IllegalArgumentException | ClassCastException e) {
            return null;
        }
    }

    private void saveTargetNetwork() {
        WifiConfiguration saveConfig = setTargetNetworkConfig();
        //saveConfig.BSSID = bssid;
        mWifiManager.save(saveConfig,null);
        //mWifiManager.disableEphemeralNetwork(saveConfig.SSID);
    }

    private void startScanFreq() {
        Log.d(TAG,"startScanFreq");
        mWifiManager.startScan();
    }

    private void turnOnWifi() {
        //stop reconnect scan
        if(mWifiManager.setWifiEnabled(true)) {
            Log.d(TAG,"setwifienable true");
            final ContentResolver cr = getContentResolver();
            
        }
    }

    private void handleRssiChange(int rssi) {
        Log.d(TAG, "newRssi is " + rssi);
        Current_RSSI.setText(String.valueOf(rssi));

        if (isFirstTimeRSSI&&rssi < targetDisonnectRssi) {
            if (disconnectThreshold>0) {
                //disconnectThreshold--;
                Log.d(TAG, "newRssi below targetDisconnectRSSI ");
                isFirstTimeRSSI = false;
                startScanFreq();
            }
        }
    }

    private void handleNetworkStateChange(NetworkInfo mNetworkInfo) {
        NetworkInfo.DetailedState currentdState = mNetworkInfo.getDetailedState();
        if (currentdState != lastState) {
            lastState = currentdState;
            // reset & clear notification on a network connect & disconnect
            switch (currentdState) {
                case CONNECTED:
                    //Get wifi info
                    final WifiInfo wifiInfo = mWifiManager.getConnectionInfo();
                    if (wifiInfo != null) {
                        int connectedFrequency = wifiInfo.getFrequency();
                        int connectedChannel = checkConnectedChannel(connectedFrequency);
                        Log.d(TAG,"get connected channel: " + connectedChannel);
                        Target_Channel.setText(String.valueOf(connectedChannel));
                        if(use_dhcp == 1) {
                            InetAddress connectedIP = NetworkUtils.intToInetAddress(wifiInfo.getIpAddress());
                            String connectedIpAddress = checkConnectedIp(connectedIP);
                            Target_IP.setText(connectedIpAddress);
                            Log.d(TAG,"get dhcp ip address: " + connectedIpAddress);
                           // systemPropertySet(ATM_IP_ADDRESS,connectedIpAddress);
							
							//SystemProperties.set(ATM_IP_ADDRESS,connectedIpAddress);
							//change to hidl message
							SendMessageToMETATst(ATM_CMD_SWITCH_META_TO_WIFI_IP+ ":" + connectedIpAddress);
                        }
                        MAC_Address.setText(wifiInfo.getBSSID());
                    } else {
                        Log.d(TAG,"get wifiInfo fail");
                    }
                    //Switch meta_tst to wifi
                        
					SendMessageToMETATst(ATM_CMD_SWITCH_META_TO_WIFI);
                    Log.d(TAG,"set persist.meta.connecttype to wifi");
 //                   systemPropertySet(META_CONNECT_TYPE,"wifi");
					
//					SystemProperties.set(META_CONNECT_TYPE,"wifi");
                    //Show UI connect type as WIFI
                    cur_connect_type = "WIFI";
                    WIFI_Status.setText(R.string.Wifi_Connected);
                    break;
                case DISCONNECTED:
                    WIFI_Status.setText(R.string.Wifi_Disconnected);
                    break;
                case OBTAINING_IPADDR:
                    WIFI_Status.setText(R.string.Wifi_OBTAINING_IPADDR);
                    break;
                case SCANNING:
                    WIFI_Status.setText(R.string.Wifi_Scaning);
                    break;
                case CONNECTING:
                    WIFI_Status.setText(R.string.Wifi_Connecting);
                    break;
                case AUTHENTICATING:
                    WIFI_Status.setText(R.string.Wifi_AUTHENTICATING);
                    break;
                default:
                    break;
            }
        }
    }

    private SensorEventListener alsListener = new SensorEventListener() {
        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
        }

        @Override
        public void onSensorChanged(SensorEvent event) {
            if(check_alsps != 0 && isDutInBox == false)
            {
                alsData = (int) event.values[0];
                ALS_Value.setText(String.valueOf(alsData));
            }
        }
    };

    private SensorEventListener psListener = new SensorEventListener() {
        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
        }

        @Override
        public void onSensorChanged(SensorEvent event) {
            if(check_alsps != 0 && isDutInBox == false)
            {
                psData = (int) event.values[0];
                PS_Value.setText(String.valueOf(psData));
            }
        }
    };

    private void CheckModemMode() {
        //Double check modem mode before exit, if tool failed to switch it
       // String modemMode = systemPropertyGet(ATM_MD_MODE,"");
		
        String modemMode = SystemProperties.get(ATM_MD_MODE,"");
        if(modemMode.equals("meta")) {
            Log.d(TAG,"modem is in META mode, switch it to normal before exit");
			SendMessageToMETATst(ATM_CMD_SWITCH_MODEM_TO_NORMAL);
        }
    }

    private String checkConnectedIp(InetAddress connectedIP) {
        Collection<InetAddress> ipAddress = new ArrayList<InetAddress>();
        ipAddress.add(connectedIP);
        String[] connectedIpAddress = new String[1];
        connectedIpAddress = NetworkUtils.makeStrings(ipAddress);
        return connectedIpAddress[0];
    }

    private int checkConnectedChannel(int connectedFrequency) {
        int channel = 0;
        int frequency = connectedFrequency;

        switch(frequency) {
            case 2412:
                channel = 1;
                break;
            case 2417:
                channel = 2;
                break;
            case 2422:
                channel = 3;
                break;
            case 2427:
                channel = 4;
                break;
            case 2432:
                channel = 5;
                break;
            case 2437:
                channel = 6;
                break;
            case 2442:
                channel = 7;
                break;
            case 2447:
                channel = 8;
                break;
            case 2452:
                channel = 9;
                break;
            case 2457:
                channel = 10;
                break;
            case 2462:
                channel = 11;
                break;
            case 2467:
                channel = 12;
                break;
            case 2472:
                channel = 13;
                break;
            case 2484:
                channel = 14;
                break;
            case 5180:
                channel = 36;
                break;
            case 5200:
                channel = 40;
                break;
            default:
                channel = 0;
        }
        return channel;
    }
    
    public static String systemPropertyGet(String key, String def) {
        try {
            Class<?> sp = Class.forName("android.os.SystemProperties");
            Method method = sp.getMethod("get", String.class, String.class);
            return (String) method.invoke(null, key, def);
        } catch (Exception e) {
            //TODO more debug information
            e.printStackTrace();
            return def;
        }
    }
 
    public static void systemPropertySet(String key, String val){
        try {
            Class<?> sp = Class.forName("android.os.SystemProperties");
            Method method = sp.getMethod("set", String.class, String.class);
            method.invoke(null, key, val);
        } catch (Exception e) {
            //TODO more debug information
            e.printStackTrace();
        }
 
    }
    
       public String getErrorInfoFromException(Exception e){
		String retS;
		try {
			StringWriter sw = new StringWriter();
			PrintWriter pw = new PrintWriter(sw);
			e.printStackTrace(pw);
			retS = sw.toString();
			sw.close();
			pw.close();
			}
		catch (Exception e2){
			return "ErrorInfoFromException";
			}
		return retS;
    
		}
    
    public boolean LogHIDLConnection(String serverName) {
		Log.d(TAG, " get hidl" + serverName);

        try {
            mATMWiFiHIDLService = vendor.mediatek.hardware.log.V1_0.ILog.getService(serverName);
            if(mATMWiFiHIDLService == null){
          		 Log.d(TAG, " get hidl failed");
            	return false;
            }
            mATMWiFiHIDLService.setCallback(mLogCallback);
			mATMWiFiHIDLService.linkToDeath(mHidlDeathRecipient, 0);
            Log.d(TAG, "mLogHIDLService.setCallback() done!");
            
        } catch (RemoteException e) {
            e.printStackTrace();
     		 Log.d(TAG, " get hidl exception" +getErrorInfoFromException(e));
			 mATMWiFiHIDLService = null;

        	return false;

        } catch (NoSuchElementException e) {
            e.printStackTrace();
    		 Log.d(TAG, " get hidl exception" +getErrorInfoFromException(e));
			 mATMWiFiHIDLService = null;

        	return false;

        }
        Log.d(TAG, " get hidl done" + serverName);
 
    	return true;

    }
   public boolean sendCmd(String message) {
        if (mATMWiFiHIDLService != null) {
            try {
            	mATMWiFiHIDLService.sendToServer(message);
            } catch (RemoteException e) {
                e.printStackTrace();
        		 Log.d(TAG, " sebd hidl exception" +e.getMessage() );
				 mATMWiFiHIDLService = null;
                return false;
            }
        } else {
        	 Log.d(TAG, "connect() mATMWiFiHIDLService is null!");
            return false;
        }
        return true;
    }
   
    private ILogCallback mLogCallback = new ILogCallback.Stub() {
        @Override
        public boolean callbackToClient(String data) throws RemoteException {

        	Log.d(TAG, "callbackToClient data = " + data);
			if(data.startsWith(ATM_CMD_EXIT_ACTIVITY)) {
				Log.w(TAG, "receive exit command from meta_tst, exit now");
                Message msg = null;
                msg = Message.obtain(mHandler,MSG_EXIT_ACTIVITY,"exit");
                msg.sendToTarget();
	            pre_connect_type = cur_connect_type;
			}
            return true;
        }
    };

    public void SendMessageToMETATst(String type){
		Log.d(TAG, "SendMessageToMETATst info" + type);
		if(mATMWiFiHIDLService == null){//
			Log.d(TAG, "Need to reconnected to the type");
			if(LogHIDLConnection("ATMWiFiHidlServer") == false){
				Log.d(TAG, "Failed to connected with TST");
				return;	
			}
		}
		sendCmd(type); 
		 
		Log.d(TAG, "Cononect to meta begin - 1012 - 1");
		return;

    }
    
    public void disConnect() {
        mATMWiFiHIDLService = null;
		return;
    }

    HidlDeathRecipient mHidlDeathRecipient = new HidlDeathRecipient();

    /**
     * @author MTK81255
     *
     */
    class HidlDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            Log.d(TAG, "serviceDied! cookie = " + cookie);
            disConnect();
        }
    }

}

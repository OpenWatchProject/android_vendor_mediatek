package com.mediatek.lbs.em2.ui;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Date;
import java.util.Enumeration;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.location.Country;
import android.location.CountryDetector;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Process;
import android.provider.Settings;
import android.telephony.CellLocation;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.mediatek.lbs.em2.utils.AgpsConfig;
import com.mediatek.lbs.em2.utils.AgpsInterface;

public class LbsMisc extends Activity {
    private Button          mButton_TIME;
    private Button          mButton_GPS;
    private Button          mButton_WIFI;
    private TextView        mTextViewCellInfo;
    private Spinner         mSpinnerServer;
    private EditText        mEditTextIp;
    private EditText        mEditTextPort;
    private ToggleButton    mButtonConnect;
    private TextView        mTextViewResult;
    private TextView        mTextViewCountryDetectorValue;
    private CheckBox        mCheckBoxE911OpenGpsEnable;
    private CheckBox        mCheckBoxEnableLocationEMService;
    private CheckBox        mCheckBoxEnableLocationSettings;
    private CheckBox        mCheckBoxAddSettingsExemptWhitelist;

    String[] SERVER_NAME    = {"LOCAL", "CMCC", "GOOGLE"};
    String[] SERVER_IP        = {"127.0.0.1", "221.176.0.55", "supl.google.com"};
    String[] SERVER_PORT    = {"7276", "7275", "7275"};

    private int                mSessionId = 0;
    private ConnectionThread mConnectionThread;
    private StringList mStringList = new StringList(12);
    private TelephonyManager    mTelephonyMgr;
    protected AgpsInterface mAgpsInterface;
    private LocationManager mLocationManager = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        log("onCreate");
        super.onCreate(savedInstanceState);

        try {
            initAgpsInterface();
        } catch (RuntimeException e) {
            log("onCreate, Failed to initAgpsInterface");
            return;
        }

        setContentView(R.layout.misc);

        mButton_TIME        = (Button) findViewById(R.id.Button_TIME);
        mButton_GPS         = (Button) findViewById(R.id.Button_GPS);
        mButton_WIFI        = (Button) findViewById(R.id.Button_WIFI);
        mTextViewCellInfo   = (TextView) findViewById(R.id.TextView_CellInfo);
        mSpinnerServer      = (Spinner) findViewById(R.id.Spinner_ServerPrfoile);
        mEditTextIp         = (EditText) findViewById(R.id.EditText_serverIp);
        mEditTextPort       = (EditText) findViewById(R.id.EditText_serverPort);
        mButtonConnect      = (ToggleButton) findViewById(R.id.Button_serverConnect);
        mTextViewResult     = (TextView) findViewById(R.id.TextView_ServerResult);
        mTextViewCountryDetectorValue = (TextView) findViewById(R.id.TextView_CountryDetectorValue);
        mCheckBoxE911OpenGpsEnable = (CheckBox) findViewById(R.id.CheckBox_E911OpenGpsEnable);
        mCheckBoxEnableLocationEMService
                = (CheckBox) findViewById(R.id.CheckBox_EnableLocationEMService);
        mCheckBoxEnableLocationSettings
                = (CheckBox) findViewById(R.id.CheckBox_EnableLocationSettings);
        mCheckBoxAddSettingsExemptWhitelist
                = (CheckBox) findViewById(R.id.CheckBox_AddSettingsExemptWhitelist);

        ArrayAdapter<String> adapter;
        adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, SERVER_NAME);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinnerServer.setAdapter(adapter);

        mTelephonyMgr = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        mTelephonyMgr.listen(new CellStateListener(),
                PhoneStateListener.LISTEN_DATA_CONNECTION_STATE |
                PhoneStateListener.LISTEN_SERVICE_STATE |
                PhoneStateListener.LISTEN_CELL_LOCATION |
                PhoneStateListener.LISTEN_CALL_STATE);
        updateCellInfo();

        /// default enable locationem background service
        SharedPreferences sharedPreferences =
                getSharedPreferences("locationem2.data" , MODE_PRIVATE);
        boolean launchForegroundService =
                sharedPreferences.getBoolean("launchForegroundService" , true);
        if (launchForegroundService) {
            mCheckBoxEnableLocationEMService.setChecked(true);
        } else {
            mCheckBoxEnableLocationEMService.setChecked(false);
        }

        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);
        if (mLocationManager == null) {
            log("ERR: mLocationManager is null");
        }

        if (mLocationManager.isLocationEnabled()) {
            mCheckBoxEnableLocationSettings.setChecked(true);
        } else {
            mCheckBoxEnableLocationSettings.setChecked(false);
        }

        if (isPackageInLocationSettingsWhitelist(getApplicationContext())) {
            mCheckBoxAddSettingsExemptWhitelist.setChecked(true);
        } else {
            mCheckBoxAddSettingsExemptWhitelist.setChecked(false);
        }

        mSpinnerServer.setOnItemSelectedListener(new ListView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                int id = (int) mSpinnerServer.getSelectedItemId();
                mEditTextIp.setText(SERVER_IP[id]);
                mEditTextPort.setText(SERVER_PORT[id]);
            }

            @Override
            public void onNothingSelected(AdapterView<?> arg0) {

            }
        });

        mButton_TIME.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                startActivity(new Intent(android.provider.Settings.ACTION_DATE_SETTINGS));
            }
        });
        mButton_GPS.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                startActivity(new Intent(android.provider.Settings.ACTION_LOCATION_SOURCE_SETTINGS));
            }
        });
        mButton_WIFI.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                startActivity(new Intent(android.provider.Settings.ACTION_WIFI_SETTINGS));
            }
        });
        mButtonConnect.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (mButtonConnect.isChecked()) {
                    try {
                        int inputPort = Integer.valueOf(mEditTextPort.getText().toString());
                        if (mEditTextIp.getText().toString().length() <= 0 ||
                            inputPort < 0 || inputPort > 65535) {
                            throw new NumberFormatException();
                        }
                        mConnectionThread = new ConnectionThread(mEditTextIp.getText().toString(),
                                inputPort, mSessionId);
                        mConnectionThread.start();
                    } catch (NumberFormatException e) {
                        Toast.makeText(getApplicationContext(),
                                "ERR: Parameters error!!", Toast.LENGTH_SHORT).show();
                        mButtonConnect.setChecked(false);

                    }
                    mSessionId++;
                } else {
                    mConnectionThread.stop();
                }
            }
        });
        mCheckBoxE911OpenGpsEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setE911OpenGpsEnable(mCheckBoxE911OpenGpsEnable.isChecked());
            }
        });
        mCheckBoxEnableLocationEMService.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                SharedPreferences sharedPreferences =
                        getSharedPreferences("locationem2.data" , MODE_PRIVATE);
                SharedPreferences.Editor editor = sharedPreferences.edit();
                if (mCheckBoxEnableLocationEMService.isChecked()) {
                    // start locationEM service to avoid process is killed
                    startService(new Intent().setClass(LbsMisc.this, LocationEMService.class));
                    editor.putBoolean("launchForegroundService" , true);
                } else {
                    // stop locationEM service
                    stopService(new Intent().setClass(LbsMisc.this, LocationEMService.class));
                    editor.putBoolean("launchForegroundService" , false);
                }
                editor.commit();
            }
        });
        mCheckBoxEnableLocationSettings.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mLocationManager.setLocationEnabledForUser(
                        mCheckBoxEnableLocationSettings.isChecked(),
                        android.os.Process.myUserHandle());
            }
        });

        mCheckBoxAddSettingsExemptWhitelist.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if(mCheckBoxAddSettingsExemptWhitelist.isChecked()) {
                    addPackageInLocationSettingsWhitelist(getApplicationContext());
                } else {
                    removePackageInLocationSettingsWhitelist(getApplicationContext());
                }
            }
        });
    }

    @Override
    public void onResume() {
        super.onResume();
        applyAgpsConfigToUI();
    }

    @Override
    protected void onDestroy() {
        log("onDestroy");
        super.onDestroy();
    }

    protected void initAgpsInterface() {
        try {
            mAgpsInterface = new AgpsInterface();
        } catch (IOException e) {
            log("agps interface connection failure");
            e.printStackTrace();
        }
    }

    protected void applyAgpsConfigToUI() {
        if (mAgpsInterface == null) {
            log("agpsInterface is null");
            return;
        }
        AgpsConfig config = mAgpsInterface.getAgpsConfigV17();
        mCheckBoxE911OpenGpsEnable.setChecked(config.getAgpsSetting().e911OpenGpsEnable);

        //// Update country detector result
        CountryDetector countryDetector =
                (CountryDetector)getApplicationContext().getSystemService(Context.COUNTRY_DETECTOR);
        if (countryDetector != null) {
            Country country = countryDetector.detectCountry();
            int source = country.getSource();
            String countryIso = country.getCountryIso();
            String outStr = "country="+countryIso+" ,source="+source;
            mTextViewCountryDetectorValue.setText(outStr);
            log("countryDetector result: " + outStr);
        }
    }
    //=================== Test Connection related ========================\\
    class ConnectionThread {
        private Thread mThread;
        private String ip;
        private int port;
        private int sessionId;
        public ConnectionThread(String ip, int port, int sessionId) {
            this.ip = ip;
            this.port = port;
            this.sessionId = sessionId;
            mThread = new Thread() {
                public void run() {
                    runFunction();
                }
            };
        }

        public void start() {
            mThread.start();
        }

        public void stop() {
            mThread.interrupt();
        }

        private void runFunction() {
            try {
                Socket socket = new Socket();
                sendMessage(0, "DNS querying");
                InetSocketAddress isa = new InetSocketAddress(ip, port);
                sendMessage(0, "Name=" + isa.getAddress().getHostName());
                sendMessage(0, "Address=" + isa.getAddress().getHostAddress());
                sendMessage(0, "start connection");
                socket.connect(isa, 10 * 1000);
                sendMessage(0, "connected");
                socket.close();
                sendMessage(0, "disconnected");
            } catch (UnknownHostException e) {
                sendMessage(0, "UnknownHostException " + e.getMessage());
                e.printStackTrace();
            } catch (NullPointerException e) {
                sendMessage(0, "NullPointerException " + e.getMessage());
                e.printStackTrace();
            } catch (IOException e) {
                sendMessage(0, "IOException " + e.getMessage());
                e.printStackTrace();
            }
            sendMessage(1, "");
        }

        private void sendMessage(int what, String msg) {
            Message m = new Message();
            Bundle bundle = new Bundle();
            bundle.putString("msg", getTimeString() + "[id=" + sessionId + "] " + msg + "\n");
            m.what = what;
            m.obj = bundle;
            mHandler.sendMessage(m);
        }

        private String getTimeString() {
            Date date = new Date();
            String str = String.format("[%02d:%02d:%02d] ",
                    date.getHours(),
                    date.getMinutes(),
                    date.getSeconds());
            return str;
        }
    }


    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Bundle bundle = (Bundle) msg.obj;
            switch(msg.what) {
            case 0:
                mStringList.add(bundle.getString("msg"));
                mTextViewResult.setText(mStringList.get());
                break;
            case 1:
                mButtonConnect.setChecked(false);
                break;

            }
        }
    };


  //=================== Phone related ========================\\
    class CellStateListener extends PhoneStateListener {
        @Override
        public void onCellLocationChanged(CellLocation location) {
            log("onCellLocationChanged location=" + location);
            updateCellInfo();
        }
        @Override
        public void onDataConnectionStateChanged(int state, int networkType) {
            log("onDataConnectionStateChanged state=" + state + " networkType=" + networkType);
            updateCellInfo();
        }
        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            log("onServiceStateChanged serviceState=" + serviceState);
            updateCellInfo();
        }
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            log("onCallStateChanged state=" + state + " incomingNumber=" + incomingNumber);
        }
    }


    private String getLocalIpAddress() {
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements(); ) {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements(); ) {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()) {
                        String ip = inetAddress.getHostAddress().toString();
                        if (ip.length() <= 16) {
                            log("IP=" + ip);
                            return ip;
                        }
                    }
                }
            }
        } catch (SocketException e) {
            e.printStackTrace();
        }
        return null;
    }

    private void updateCellInfo() {
        String mccMnc = mTelephonyMgr.getNetworkOperator();
        CellLocation cellLocation = mTelephonyMgr.getCellLocation();
        String ip = getLocalIpAddress();

        if (cellLocation instanceof GsmCellLocation) {
            GsmCellLocation gsmCell = (GsmCellLocation) cellLocation;
            int cid = -1;
            int lac = -1;
            if (gsmCell != null) {
                cid = gsmCell.getCid();
                lac = gsmCell.getLac();
            }
            mTextViewCellInfo.setText("mccMnc=[" + mccMnc + "] lac=[" + lac + "] cid=[" + cid + "]\n" +
                    "IP=" + ip);
        } else if (cellLocation instanceof CdmaCellLocation) {
            CdmaCellLocation cdmaCell = (CdmaCellLocation) cellLocation;
            int networkId = -1;
            int systemId = -1;
            if (cdmaCell != null) {
                networkId = cdmaCell.getNetworkId();
                systemId = cdmaCell.getSystemId();
            }
            mTextViewCellInfo.setText("networkId=[" + networkId + "] systemId=[" + systemId + "] IP=" + ip);
        } else {
            mTextViewCellInfo.setText("Unknown Cell Location IP=" + ip);
        }
    }

    private boolean isPackageInLocationSettingsWhitelist(Context context) {
        final String LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST =
                "location_ignore_settings_package_whitelist";
        String packageName = context.getPackageName();

        String whitelist = Settings.Global.getString(
                this.getContentResolver(),
                LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST);
        if (whitelist != null && whitelist.indexOf(packageName) != -1) {
            return true;
        }
        return false;
    }

    private void addPackageInLocationSettingsWhitelist(Context context) {
        final String LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST =
                "location_ignore_settings_package_whitelist";
        String packageName = context.getPackageName();

        String whitelist = Settings.Global.getString(
                this.getContentResolver(),
                LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST);
        if (whitelist == null || whitelist.indexOf(packageName) == -1) {
            String outStr = (whitelist == null) ? "" : whitelist + ",";
            outStr += packageName;
            log("Add ignore settings outStr = " + outStr);
            Settings.Global.putString(
                    this.getContentResolver(),
                    LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST,
                    outStr);
        }
    }

    private void removePackageInLocationSettingsWhitelist(Context context) {
        final String LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST =
                "location_ignore_settings_package_whitelist";
        String packageName = context.getPackageName();

        String whitelist = Settings.Global.getString(
                this.getContentResolver(),
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
                    this.getContentResolver(),
                    LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST,
                    outStr);
        }
    }


    //=================== basic utility ========================\\
    private void startComponent(String packageName, String fullActivityName) {
        try {
            Intent intent = new Intent();
            intent.setComponent(new ComponentName(packageName, fullActivityName));
            startActivity(intent);
        } catch (ActivityNotFoundException e) {
            e.printStackTrace();
            log("ERR: startComponent failed fullActivityName=" + fullActivityName);
            Toast.makeText(getApplicationContext(), "ERR: startComponent failed!!",
                    Toast.LENGTH_LONG).show();
        }
    }

    private void log(String msg) {
        Log.d("LocationEM_Misc", msg);
    }

}

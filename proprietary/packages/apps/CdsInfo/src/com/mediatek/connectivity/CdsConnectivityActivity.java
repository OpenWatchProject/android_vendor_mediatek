/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.connectivity;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.net.ConnectivityManager;
import android.net.IConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.os.Bundle;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;

import vendor.mediatek.hardware.netdagent.V1_0.INetdagent;

public class CdsConnectivityActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "CDSINFO/CdsConnectivityActivity";
    private Context mContext;
    private Toast mToast;
    private static final boolean DBG = true;

    private ConnectivityManager mConnMgr = null;
    private INetworkManagementService mNwService = null;
    private IConnectivityManager mConnManager = null;
    private Button mDevelopeBtn = null;
    private Button mEnableUdpBtn = null;
    private Button mDisableUdpBtn = null;
    private CheckBox mAvoidBadWifiChkBox = null;
    private CheckBox mMobileOnCheckBox = null;
    private EditText mUdpIpAddr = null;
    private RadioGroup mCaptiveMode = null;

    private static final String ARP_FILE = "/proc/net/arp";

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.cds_connectivity);

        mContext = this.getBaseContext();

        mConnMgr = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNwService = INetworkManagementService.Stub.asInterface(b);
        if (null == mNwService) {
            Log.e(TAG, "onCreate:Failed to get the NetworkManagementService!");
        }
        b = ServiceManager.getService(CONNECTIVITY_SERVICE);
        mConnManager = IConnectivityManager.Stub.asInterface(b);
        if (null == mConnManager) {
            Log.e(TAG, "onCreate:Failed to get the ConnectivityService!");
        }
        mToast = Toast.makeText(this, null, Toast.LENGTH_SHORT);

        mAvoidBadWifiChkBox = (CheckBox) findViewById(R.id.avoid_bad_wifi);
        mAvoidBadWifiChkBox.setOnCheckedChangeListener(new CheckBox.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                updateGlobalSetting(Settings.Global.NETWORK_AVOID_BAD_WIFI,
                                    isChecked);
            }
        });

        mMobileOnCheckBox = (CheckBox) findViewById(R.id.conn_mobile_on);
        mMobileOnCheckBox.setOnCheckedChangeListener(new CheckBox.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                                updateGlobalSetting(Settings.Global.MOBILE_DATA_ALWAYS_ON,
                                    isChecked);
            }
        });

        mCaptiveMode = (RadioGroup) findViewById(R.id.captive_mode);
        mCaptiveMode.setOnCheckedChangeListener(mCaptiveModeListener);

        mDevelopeBtn = (Button) findViewById(R.id.developeBtn);
        mDevelopeBtn.setOnClickListener(this);

        mEnableUdpBtn = (Button) findViewById(R.id.tetherEnableBtn);
        mEnableUdpBtn.setOnClickListener(this);
        mUdpIpAddr = (EditText)  findViewById(R.id.tether_udp_test_ipaddr);
        mUdpIpAddr.setText("");

        mDisableUdpBtn = (Button) findViewById(R.id.tetherDisableBtn);
        mDisableUdpBtn.setOnClickListener(this);

        updateCurrentStatus();
        Log.i(TAG, "CdsConnectivityActivity is started");
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateCurrentStatus();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    private RadioGroup.OnCheckedChangeListener mCaptiveModeListener =
                        new RadioGroup.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(RadioGroup group, int checkedId) {
            int value = Settings.Global.CAPTIVE_PORTAL_MODE_PROMPT;
            switch (checkedId) {
                case R.id.ignore_mode:
                    value = Settings.Global.CAPTIVE_PORTAL_MODE_IGNORE;
                    break;
                case R.id.prompt_mode:
                    value = Settings.Global.CAPTIVE_PORTAL_MODE_PROMPT;
                    break;
                case R.id.avoid_mode:
                    value = Settings.Global.CAPTIVE_PORTAL_MODE_AVOID ;
                    break;
                default:
                    break;
            }
            Settings.Global.putInt(mContext.getContentResolver(),
                        Settings.Global.CAPTIVE_PORTAL_MODE, value);
        }
    };

    private void updateCurrentStatus() {

        initGloablSetting(mMobileOnCheckBox, Settings.Global.MOBILE_DATA_ALWAYS_ON, 0);

        // Default valie is controlled by config_networkAvoidBadWifi
        initGloablSetting(mAvoidBadWifiChkBox, Settings.Global.NETWORK_AVOID_BAD_WIFI, 1);

        initCaptiveSetting(mCaptiveMode, Settings.Global.CAPTIVE_PORTAL_MODE,
                    Settings.Global.CAPTIVE_PORTAL_MODE_PROMPT);

        try {
            String[] tetherInterfaces = mConnManager.getTetheredIfaces();
            if (tetherInterfaces.length != 1) {
               return;
            }
            logd("Tethering iface:" + tetherInterfaces[0]);
            String ipAddr = getClientIp(tetherInterfaces[0]);
            if (ipAddr == null || ipAddr.length() == 0) {
                logd("[NS-IOT]There is no HostPC address!");
                mUdpIpAddr.setText("[NS-IOT]There is no HostPC address!");
            } else {
                mUdpIpAddr.setText(ipAddr);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void initGloablSetting(CheckBox box, String name, int defaultVal) {
        boolean isChecked = (Settings.Global.getInt(mContext.getContentResolver(),
                                name, defaultVal) == 1);
        box.setChecked(isChecked);
    }

    private void initCaptiveSetting(RadioGroup box, String name, int defaultVal) {
        int value = Settings.Global.getInt(mContext.getContentResolver(),
                                name, defaultVal);
        int id = R.id.prompt_mode;
        switch (value) {
            case 0:
                id = R.id.ignore_mode;
                break;
            case 1:
                id = R.id.prompt_mode;
                break;
            case 2:
                id = R.id.avoid_mode;
                break;
            default:
                break;
        }
        box.check(id);
    }

    private void initGlobalStringSetting(EditText txt, String name) {
        String server = Settings.Global.getString(mContext.getContentResolver(),
                                name);

        if (server == null) {
            server = "";
        }
        txt.setText(server);
    }

    private void updateGlobalSetting(String name, boolean enabled) {
        int value = (enabled) ? 1 : 0;
        Log.d(TAG, "updateGlobalSetting:" + name + ":" + value);
        Settings.Global.putInt(mContext.getContentResolver(), name, value);
    }

    private void startDevelopmentSettings() {
        startActivityForResult(new Intent(
                android.provider.Settings.ACTION_APPLICATION_DEVELOPMENT_SETTINGS), 0);
    }

    public void onClick(View v) {
        int buttonId = v.getId();

        switch(buttonId) {
        case R.id.developeBtn:
            Log.i(TAG, "development settings");
            startDevelopmentSettings();
            break;
        case R.id.tetherEnableBtn:
            Log.i(TAG, "configure udp testing");
            setUdpTesting(true);
            break;
        case R.id.tetherDisableBtn:
            setUdpTesting(false);
            break;
        default:
            Log.e(TAG, "Error button");
            break;
        }
    }

    private void setUdpTesting(boolean enabled) {
        try {
            String[] tetherInterfaces = mConnManager.getTetheredIfaces();
            if (tetherInterfaces.length != 1) {
               showToast("Please check tethering statue. Make sure only one tethering is enabled");
               return;
            }

            String extInterface = getUpstreamIface();
            String inInterface = tetherInterfaces[0];

            if (TextUtils.isEmpty(extInterface)) {
                showToast("Please enable Internet connection");
                return;
            }

            Log.d(TAG, "[NS-IOT] setUdpTesting:" + inInterface + ":" + extInterface);

            String ipAddr = mUdpIpAddr.getText().toString();
            if (ipAddr == null || ipAddr.length() == 0) {
                ipAddr = getClientIp(inInterface);
                if (ipAddr == null || ipAddr.length() == 0) {
                    showToast("Please input the destination address");
                    return;
                } else {
                    mUdpIpAddr.setText(ipAddr);
                }
            }
            String info = ((enabled) ? "Enabled" : "Disabled") + " "
                                +  extInterface + "/" + inInterface + ":" + ipAddr;

            INetdagent agent = INetdagent.getService();
            if (agent == null) {
                Log.e(TAG, "agnet is null");
                return;
            }
            String cmd = "";
            if (enabled) {
                //Send command
                cmd = "netdagent firewall set_udp_forwarding " + inInterface + " " +
                        extInterface + " " + ipAddr;
            } else {
                cmd = "netdagent firewall clear_udp_forwarding " + inInterface + " " +
                        extInterface;
            }
            Log.d(TAG, "Send " + cmd);
            agent.dispatchNetdagentCmd(cmd);
            showToast(info);
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "setStartRequest failed!");
        }

    }

    private String getUpstreamIface() {
        // Check DUN PDN firstly
        try {
            Network[] networks = mConnManager.getAllNetworks();
            for (Network nw : networks) {
                NetworkCapabilities cap = mConnManager.getNetworkCapabilities(nw);
                if (cap.hasCapability(NetworkCapabilities.NET_CAPABILITY_DUN)) {
                    LinkProperties prop = mConnManager.getLinkProperties(nw);
                    if (prop != null) {
                        return prop.getInterfaceName();
                    }
                }
            }
            // Check active network connection
            LinkProperties prop = mConnManager.getLinkProperties(mConnManager.getActiveNetwork());
            if (prop != null) {
                return prop.getInterfaceName();
            }
        } catch (RemoteException re) {
            re.printStackTrace();
        }
        return null;
    }

    private ArrayList<String> readClientList(String filename) {
        FileInputStream fstream = null;
        ArrayList<String> list = new ArrayList<String>();
        int i = 0;

        try {
            fstream = new FileInputStream(filename);
            DataInputStream in = new DataInputStream(fstream);
            BufferedReader br = new BufferedReader(new InputStreamReader(in));
            String s;
            // throw away the title line
            while (((s = br.readLine()) != null) && (s.length() != 0)) {
                Log.d(TAG, "line:" + s);
                if (i != 0) {
                    list.add(s);
                }
                i++;
            }
        } catch (IOException ex) {
            // return current list, possibly empty
            Log.e(TAG, "IOException:" + ex);
        } finally {
          if (fstream != null) {
                try {
                    fstream.close();
                } catch (IOException ex) {
                    Log.e(TAG, "IOException:" + ex);
                }
            }
        }
        return list;
    }

    private String getClientIp(String iface) {
        int i = 0;
        String s = null;

        ArrayList<String> clients = readClientList(ARP_FILE);
        if (clients.isEmpty()) {
            Log.i(TAG, "Empty client");
            return null;
        }

        for (i = 0; i < clients.size(); i++) {
            s = (String) clients.get(i);
            logd("getClientIp:" + s);
            if (!TextUtils.isEmpty(s)) {
                String[] fields = s.split(" +");
                logd("fields:" + fields.length);
                if (fields.length == 6) {
                    if (fields[5].equals(iface)) {
                        Log.i(TAG, "Found IP address:" + fields[0]);
                        return fields[0];
                    }
                } else {
                    Log.e(TAG, "The length of fields is wrong:" + fields.length);
                }
            }
        }

        return null;
    }

    private void showToast(String s) {
        mToast.setText(s);
        mToast.show();
    }

    private void logd(String s) {
        if (DBG) {
            Log.d(TAG, s);
        }
    }

}
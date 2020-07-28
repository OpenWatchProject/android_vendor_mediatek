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

/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.mediatek.connectivity;

import static android.net.wifi.ScanResult.InformationElement;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.ScanResult;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.IBinder ;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.util.HexDump;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Random;
import java.util.StringTokenizer;

import vendor.mediatek.hardware.nvram.V1_0.INvram;

/**
 * Show the current status details of Wifi related fields
 */
public class CdsWifiInfoActivity extends Activity {

    private static final String TAG = "CDSINFO/WifiInfo";

    private static final int MAC_ADDRESS_ID = 30;
    private static final int MAC_ADDRESS_OFFSET = 4;
    private static final int MAC_ADDRESS_DIGITS = 6;
    private static final int MAX_ADDRESS_VALUE = 0xff;
    private static final int INVALID_RSSI = -200;

    private static final String MAC_ADDRESS_FILENAME = "/mnt/vendor/nvdata/APCFG/APRDEB/WIFI";

    private Button   mUpdateButton;
    private Button   mScanButton;
    private TextView mWifiState;
    private TextView mNetworkState;
    private TextView mSupplicantState;
    private TextView mRSSI;
    private TextView mBSSID;
    private TextView mSSID;
    private TextView mHiddenSSID;
    private TextView mIPAddr;
    private TextView mMACAddr;
    private TextView mNetworkId;
    private TextView mWifiCapability;
    private TextView mLinkSpeed;
    private TextView mScanList;
    private TextView mSystemProperties;

    private TextView mMacAddrLabel;
    private EditText mMacAddrEdit;
    private Button   mMacAddBtn;

    private Toast mToast;

    private static String MacAddressRandom = "";

    private WifiManager mWifiManager;
    private IntentFilter mWifiStateFilter;
    private short[]  mRandomMacAddr;

    //============================
    // Activity lifecycle
    //============================

    private final static ArrayList<Integer> channels24G = new ArrayList<Integer>(
        Arrays.asList(0, 2412, 2417, 2422, 2427, 2432, 2437, 2442, 2447,
                      2452, 2457, 2462, 2467, 2472, 2484));

    private  int getChannelFromFrequency(int frequency) {
        if (ScanResult.is24GHz(frequency)) {
            return channels24G.indexOf(Integer.valueOf(frequency));
        }
        return -1;
    }

    private String getChannelWidth(int channelWidth) {
        String channelInfo = "Unknown";

        switch (channelWidth) {
            case ScanResult.CHANNEL_WIDTH_20MHZ:
                channelInfo = "20 MHZ";
                break;
            case ScanResult.CHANNEL_WIDTH_40MHZ:
                channelInfo = "40 MHZ";
                break;
            case ScanResult.CHANNEL_WIDTH_80MHZ:
                channelInfo = "80 MHZ";
                break;
            case ScanResult.CHANNEL_WIDTH_160MHZ:
                channelInfo = "160 MHZ";
                break;
            case ScanResult.CHANNEL_WIDTH_80MHZ_PLUS_MHZ:
                channelInfo = "80+ MHZ";
                break;
            default:
                break;
        }
        return channelInfo;
    }

    private final BroadcastReceiver mWifiStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null || intent.getAction() == null) {
                return;
            }

            if (intent.getAction().equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
                handleWifiStateChanged(intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                                       WifiManager.WIFI_STATE_UNKNOWN));
            } else if (intent.getAction().equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
                handleNetworkStateChanged(
                    (NetworkInfo) intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO));
            } else if (intent.getAction().equals(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION)) {
                handleScanResultsAvailable();
            } else if (intent.getAction().equals(
                        WifiManager.SUPPLICANT_CONNECTION_CHANGE_ACTION)) {
                /* TODO: handle supplicant connection change later */
            } else if (intent.getAction().equals(WifiManager.SUPPLICANT_STATE_CHANGED_ACTION)) {
                handleSupplicantStateChanged(
                    (SupplicantState) intent.getParcelableExtra(WifiManager.EXTRA_NEW_STATE),
                    intent.hasExtra(WifiManager.EXTRA_SUPPLICANT_ERROR),
                    intent.getIntExtra(WifiManager.EXTRA_SUPPLICANT_ERROR, 0));
            } else if (intent.getAction().equals(WifiManager.RSSI_CHANGED_ACTION)) {
                handleSignalChanged(intent.getIntExtra(WifiManager.EXTRA_NEW_RSSI, 0));
            } else if (intent.getAction().equals(WifiManager.NETWORK_IDS_CHANGED_ACTION)) {
                /* TODO: handle network id change info later */
            } else {
                Log.e(TAG, "Received an unknown Wifi Intent");
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mWifiManager = (WifiManager) getSystemService(WIFI_SERVICE);

        mWifiStateFilter = new IntentFilter(WifiManager.WIFI_STATE_CHANGED_ACTION);
        mWifiStateFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        mWifiStateFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
        mWifiStateFilter.addAction(WifiManager.SUPPLICANT_STATE_CHANGED_ACTION);
        mWifiStateFilter.addAction(WifiManager.RSSI_CHANGED_ACTION);
        mWifiStateFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);


        setContentView(R.layout.wifi_status_test);

        mUpdateButton = (Button) findViewById(R.id.update);
        mUpdateButton.setOnClickListener(mUpdateButtonHandler);

        mScanButton = (Button) findViewById(R.id.scan);
        mScanButton.setOnClickListener(mScanButtonHandler);

        mWifiState = (TextView) findViewById(R.id.wifi_state);
        mNetworkState = (TextView) findViewById(R.id.network_state);
        mSupplicantState = (TextView) findViewById(R.id.supplicant_state);
        mRSSI = (TextView) findViewById(R.id.rssi);
        mBSSID = (TextView) findViewById(R.id.bssid);
        mSSID = (TextView) findViewById(R.id.ssid);
        mHiddenSSID = (TextView) findViewById(R.id.hidden_ssid);
        mIPAddr = (TextView) findViewById(R.id.ipaddr);
        mMACAddr = (TextView) findViewById(R.id.macaddr);
        mNetworkId = (TextView) findViewById(R.id.networkid);
        mWifiCapability = (TextView) findViewById(R.id.wifi_capability);
        mLinkSpeed = (TextView) findViewById(R.id.link_speed);
        mScanList = (TextView) findViewById(R.id.scan_list);

        mSystemProperties = (TextView) findViewById(R.id.system_property);
        mSystemProperties.setVisibility(View.INVISIBLE);

        mMacAddrLabel = (TextView) findViewById(R.id.mac_label);
        mMacAddrEdit = (EditText) findViewById(R.id.macid);
        mMacAddBtn = (Button) findViewById(R.id.mac_update_btn);
        mMacAddBtn.setOnClickListener(mMacEditButtonHandler);

        mToast = Toast.makeText(this, null, Toast.LENGTH_SHORT);

        getMacAddr();
    }

    @Override
    protected void onResume() {
        super.onResume();
        registerReceiver(mWifiStateReceiver, mWifiStateFilter);
        refreshWifiStatus();
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver(mWifiStateReceiver);
    }

    OnClickListener mScanButtonHandler = new OnClickListener() {
        public void onClick(View v) {
            try {
                mWifiManager.startScan();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    };

    OnClickListener mUpdateButtonHandler = new OnClickListener() {
        public void onClick(View v) {
            refreshWifiStatus();
        }
    };

    OnClickListener mMacEditButtonHandler = new OnClickListener() {
        public void onClick(View v) {
            updateMacAddr();
        }
    };

    private String getWifiCapability() {
        StringBuilder sb = new StringBuilder();
        sb.append("Support for 5 GHz Band: " + mWifiManager.is5GHzBandSupported() + "\n");
        sb.append("Wifi-Direct Support: " + mWifiManager.isP2pSupported() + "\n");
        sb.append("GAS/ANQP Support: "
                + mWifiManager.isPasspointSupported() + "\n");
        sb.append("Soft AP Support: "
                + mWifiManager.isPortableHotspotSupported() + "\n");
        sb.append("WifiScanner APIs Support: "
                + mWifiManager.isWifiScannerSupported() + "\n");
        sb.append("Device-to-device RTT Support: "
                + mWifiManager.isDeviceToDeviceRttSupported() + "\n");
        sb.append("Device-to-AP RTT Support: "
                + mWifiManager.isDeviceToApRttSupported() + "\n");
        sb.append("Preferred network offload Support: "
                + mWifiManager.isPreferredNetworkOffloadSupported() + "\n");
        sb.append("Tunnel directed link setup Support: "
                + mWifiManager.isTdlsSupported() + "\n");
        sb.append("Enhanced power reporting: "
                    + mWifiManager.isEnhancedPowerReportingSupported() + "\n");
        return sb.toString();
    }

    private String getMacAddrFromNvram() {
        StringBuffer nvramBuf = new StringBuffer();
        try {
            int i = 0;
            String buff = null;
            INvram agent = INvram.getService();
            if (agent == null) {
                mToast.setText("No support MAC address writing due to NVRAM");
                mToast.show();
                Log.e(TAG, "NvRAMAgent is null");
                return "";
            }
            try {
                buff = agent.readFileByName(
                        MAC_ADDRESS_FILENAME, MAC_ADDRESS_OFFSET + MAC_ADDRESS_DIGITS);
            } catch (Exception e) {
                e.printStackTrace();
                return "";
            }
            Log.i(TAG, "Raw data:" + buff);
            if (buff.length() < 2 * (MAC_ADDRESS_OFFSET + MAC_ADDRESS_DIGITS)) {
                mToast.setText("The foramt of NVRAM is not correct");
                mToast.show();
                return "";
            }
            // Remove the \0 special character.
            int macLen = buff.length() - 1;
            for (i = MAC_ADDRESS_OFFSET * 2; i < macLen; i += 2) {
                if ((i + 2) < macLen) {
                    nvramBuf.append(buff.substring(i, i + 2));
                    nvramBuf.append(":");
                } else {
                    nvramBuf.append(buff.substring(i));
                }
            }
            Log.d(TAG, "buff:" + nvramBuf.toString());
        } catch (Exception e) {
            e.printStackTrace();
            return "";
        }
        return nvramBuf.toString();
    }

    private void updateMacAddr() {

        try {
            int i = 0;
            INvram agent = INvram.getService();
            byte[] macAddr = new byte[MAC_ADDRESS_DIGITS];
            if (agent == null) {
                mToast.setText("No support MAC address writing due to NVRAM");
                mToast.show();
                Log.e(TAG, "NvRAMAgent is null");
                return;
            }

            //parse mac address firstly
            StringTokenizer txtBuffer = new StringTokenizer(
                                    mMacAddrEdit.getText().toString(), ":");
            while (txtBuffer.hasMoreTokens()) {
                macAddr[i] = (byte) Integer.parseInt(txtBuffer.nextToken(), 16);
                i++;
            }
            if (i != MAC_ADDRESS_DIGITS) {
                Log.e(TAG, "Wrong length of macAddr:" + i);
                mToast.setText("The format of mac address is not correct");
                mToast.show();
                return;
            }

            String buff = null;
            try {
                buff = agent.readFileByName(
                        MAC_ADDRESS_FILENAME, MAC_ADDRESS_OFFSET + MAC_ADDRESS_DIGITS);
            } catch (Exception e) {
                e.printStackTrace();
                return;
            }

            // Remove \0 in the end
            byte[] buffArr = HexDump.hexStringToByteArray(
                                buff.substring(0, buff.length() - 1));

            for (i = 0; i < MAC_ADDRESS_DIGITS; i ++) {
                buffArr[i + 4] = macAddr[i];
            }

            ArrayList<Byte> dataArray = new ArrayList<Byte>(
                                MAC_ADDRESS_OFFSET + MAC_ADDRESS_DIGITS);

            for (i = 0; i < MAC_ADDRESS_OFFSET + MAC_ADDRESS_DIGITS; i++) {
                dataArray.add(i, new Byte(buffArr[i]));
            }

            int flag = 0;
            try {
                flag = agent.writeFileByNamevec(MAC_ADDRESS_FILENAME,
                        MAC_ADDRESS_OFFSET + MAC_ADDRESS_DIGITS, dataArray);
            } catch (Exception e) {
                e.printStackTrace();
                mToast.setText(e.getMessage() + ":" + e.getCause());
                mToast.show();
                return;
            }
            mToast.setText("Update successfully.\r\nPlease reboot this device");
            mToast.show();
        } catch (Exception e) {
            mToast.setText(e.getMessage() + ":" + e.getCause());
            mToast.show();
            e.printStackTrace();
        }
    }

    private void refreshWifiStatus() {
        final WifiInfo wifiInfo = mWifiManager.getConnectionInfo();

        if (wifiInfo == null) {
            return;
        }

        Log.i(TAG, "refreshWifiStatus is called");
        setWifiStateText(mWifiManager.getWifiState());
        mBSSID.setText(wifiInfo.getBSSID());
        try {
            mHiddenSSID.setText(String.valueOf(wifiInfo.getHiddenSSID()));
        } catch (NullPointerException e) {
            e.printStackTrace();
        }

        int ipAddr = wifiInfo.getIpAddress();
        StringBuffer ipBuf = new StringBuffer();
        ipBuf.append(ipAddr  & 0xff).append('.').
        append((ipAddr >>>= 8) & 0xff).append('.').
        append((ipAddr >>>= 8) & 0xff).append('.').
        append((ipAddr >>>= 8) & 0xff);
        mIPAddr.setText(ipBuf);



        if (wifiInfo.getLinkSpeed() > 0) {
            mLinkSpeed.setText(String.valueOf(wifiInfo.getLinkSpeed()) + " Mbps");
        } else {
            mLinkSpeed.setText(R.string.unknown_string);
        }

        mMACAddr.setText(getMacAddrFromNvram());
        mNetworkId.setText(String.valueOf(wifiInfo.getNetworkId()));

        mWifiCapability.setText(String.valueOf(getWifiCapability()));

        if (wifiInfo.getRssi() != INVALID_RSSI) {
            mRSSI.setText(String.valueOf(wifiInfo.getRssi()));
        } else {
            mRSSI.setText(R.string.na_string);
        }

        if (mWifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED) {
            mSSID.setText(wifiInfo.getSSID());
        } else {
            mSSID.setText("");
        }

        SupplicantState supplicantState = wifiInfo.getSupplicantState();
        // setSupplicantStateText(supplicantState);
    }

    private String getSupplicantStateText(SupplicantState supplicantState) {
        String state = "";
        if (SupplicantState.FOUR_WAY_HANDSHAKE.equals(supplicantState)) {
            state = "FOUR WAY HANDSHAKE";
        } else if (SupplicantState.ASSOCIATED.equals(supplicantState)) {
            state = "ASSOCIATED";
        } else if (SupplicantState.ASSOCIATING.equals(supplicantState)) {
            state = "ASSOCIATING";
        } else if (SupplicantState.COMPLETED.equals(supplicantState)) {
            state = "COMPLETED";
        } else if (SupplicantState.DISCONNECTED.equals(supplicantState)) {
            state = "DISCONNECTED";
        } else if (SupplicantState.DORMANT.equals(supplicantState)) {
            state = "DORMANT";
        } else if (SupplicantState.GROUP_HANDSHAKE.equals(supplicantState)) {
            state = "GROUP HANDSHAKE";
        } else if (SupplicantState.INACTIVE.equals(supplicantState)) {
            state = "INACTIVE";
        } else if (SupplicantState.INVALID.equals(supplicantState)) {
            state = "INVALID";
        } else if (SupplicantState.SCANNING.equals(supplicantState)) {
            state = "SCANNING";
        } else if (SupplicantState.UNINITIALIZED.equals(supplicantState)) {
            state = "UNINITIALIZED";
        } else {
            state = "BAD";
            Log.e(TAG, "supplicant state is bad");
        }
        return state;
    }

    private void setWifiStateText(int wifiState) {
        String wifiStateString;

        switch (wifiState) {
        case WifiManager.WIFI_STATE_DISABLING:
            wifiStateString = getString(R.string.wifi_state_disabling);
            break;
        case WifiManager.WIFI_STATE_DISABLED:
            wifiStateString = getString(R.string.wifi_state_disabled);
            break;
        case WifiManager.WIFI_STATE_ENABLING:
            wifiStateString = getString(R.string.wifi_state_enabling);
            break;
        case WifiManager.WIFI_STATE_ENABLED:
            wifiStateString = getString(R.string.wifi_state_enabled);
            break;
        case WifiManager.WIFI_STATE_UNKNOWN:
            wifiStateString = getString(R.string.wifi_state_unknown);
            break;
        default:
            wifiStateString = "BAD";
            Log.e(TAG, "wifi state is bad");
            break;
        }

        if (wifiState == WifiManager.WIFI_STATE_DISABLED) {
            mScanList.setText("");
        }

        mWifiState.setText(wifiStateString);
    }

    private void handleSignalChanged(int rssi) {
        if (rssi != INVALID_RSSI) {
            mRSSI.setText(String.valueOf(rssi));
        } else {
            mRSSI.setText(R.string.na_string);
        }
    }

    private void handleWifiStateChanged(int wifiState) {
        setWifiStateText(wifiState);
    }

    private void handleScanResultsAvailable() {
        List<ScanResult> list = mWifiManager.getScanResults();

        StringBuffer scanList = new StringBuffer();

        try {
            if (list != null) {
                for (int i = list.size() - 1; i >= 0; i--) {
                    final ScanResult scanResult = list.get(i);

                    if (scanResult == null) {
                        continue;
                    }

                    if (TextUtils.isEmpty(scanResult.SSID)) {
                        continue;
                    }

                    scanList.append(scanResult.SSID + "\r\n");
                    scanList.append(scanResult.BSSID + "/"
                                + scanResult.hessid + "/" + scanResult.anqpDomainId);
                    scanList.append("(Ch:" + getChannelFromFrequency(scanResult.frequency)
                                + "/" + scanResult.frequency + "/"
                                +  getChannelWidth(scanResult.channelWidth)
                                + ")-" + scanResult.level + "dBm\r\n");

                    if (scanResult.capabilities.equals("[ESS]")) {
                        scanList.append("[OPEN]\r\n");
                    } else {
                        scanList.append(scanResult.capabilities + " \r\n");
                    }
                    // scanList.append(scanResult.timestamp + "/" + scanResult.seen + "\r\n");

                    if (scanResult.informationElements != null) {
                        scanList.append(getInformationElement(scanResult.informationElements));
                    }
                    scanList.append("\r\n\r\n");
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "handleScanResultsAvailable error:" + e);
        }

        mScanList.setText(scanList);
    }




    private void handleSupplicantStateChanged(SupplicantState state, boolean hasError, int error) {
        if (hasError) {
            mSupplicantState.setText("ERROR AUTHENTICATING");
        } else {
            mSupplicantState.setText(getSupplicantStateText(state));
        }
    }

    private void handleNetworkStateChanged(NetworkInfo networkInfo) {
        if (mWifiManager.isWifiEnabled()) {
            final WifiInfo wifiInfo = mWifiManager.getConnectionInfo();

            if (wifiInfo != null) {
                String summary = getSummary(this, wifiInfo.getSSID(),
                                             networkInfo.getDetailedState());
                mNetworkState.setText(summary);
            }
        }
    }

    private String getInformationElement(InformationElement[] informationElements) {
        StringBuffer ieList = new StringBuffer();
        String ieInfo = "";
        ByteBuffer buffer = null;

        for (InformationElement ie : informationElements) {
            try {
                switch (ie.id) {
                    case InformationElement.EID_SUPPORTED_RATES:
                        buffer = ByteBuffer.wrap(ie.bytes).order(ByteOrder.LITTLE_ENDIAN);
                        break;
                    case InformationElement.EID_BSS_LOAD:
                        buffer = ByteBuffer.wrap(ie.bytes).order(ByteOrder.LITTLE_ENDIAN);
                        short stationCount = buffer.getShort();
                        byte  channelUtilization = buffer.get();
                        short availablAc = buffer.getShort();;
                        ieList.append("SC:" + stationCount
                                + " Util:" + (channelUtilization / 255) + "%"
                                + " availAc:" + availablAc);
                        break;
                    case InformationElement.EID_EXTENDED_SUPPORTED_RATES:
                        break;
                    case InformationElement.EID_EXTENDED_CAPS:
                        break;
                    default:
                        ieInfo = "";
                        break;
                }
                ieList.append(ieInfo);
            } catch (Exception e) {
                Log.e(TAG, "Dump ie err:" + e);
            }
        }
        return ieList.toString();
    }

    private void getMacAddr() {
        mRandomMacAddr = new short[MAC_ADDRESS_DIGITS];
        StringBuilder sb = new StringBuilder();
        Random rand = new Random();
        NumberFormat formatter = new DecimalFormat("00");
        int end1 = rand.nextInt(100);
        int end2 = rand.nextInt(100);
        String num1 = formatter.format(end1);
        String num2 = formatter.format(end2);

        sb.append("00:08:22:11:");
        sb.append(num1).append(":").append(num2);

        mMacAddrLabel.setVisibility(View.VISIBLE);
        mMacAddrEdit.setVisibility(View.VISIBLE);
        mMacAddBtn.setVisibility(View.VISIBLE);
        Log.i(TAG, "string buffer:" + sb);
        mMacAddrEdit.setText(sb);
        MacAddressRandom = sb.toString();
    }

    static String getSummary(Context context, String ssid, DetailedState state) {
            String[] formats = context.getResources().getStringArray((ssid == null)
                               ? R.array.wifi_status : R.array.wifi_status_with_ssid);
            int index = state.ordinal();

            if (index >= formats.length || formats[index].length() == 0) {
                return null;
            }

            return String.format(formats[index], ssid);
    }
}
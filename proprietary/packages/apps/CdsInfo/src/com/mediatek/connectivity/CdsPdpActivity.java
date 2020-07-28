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
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.RouteInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.ServiceManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.text.SimpleDateFormat;
import java.util.Date;

public class CdsPdpActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "CDSINFO/PDP";

    private static final String[] APN_LIST = new String[] {
                        "INTERNET", "MMS", "SUPL", "DUN"};
    private static final String TELEPHONY_CONTACT = "content://telephony/carriers";
    private static final Uri CONTENT_URI =  Uri.parse(TELEPHONY_CONTACT);

    private static final String IP_CMD_STR = "ping -c 100 -s 500 -I ";
    private static final String IPV6_CMD_STR = "ping6 -c 100 -s 500 -I ";
    private static final String HOST_INFO = " [dest. IP]\r\n";

    public static final int MSG_UI_UPDATE = 0;
    public static final int MSG_CONN_UPDATE = 1;
    public static Handler sHandler;

    private static final int[] APN_TYPE_LIST = new int[] {
        ConnectivityManager.TYPE_MOBILE,
        ConnectivityManager.TYPE_MOBILE_MMS,
        ConnectivityManager.TYPE_MOBILE_SUPL,
        ConnectivityManager.TYPE_MOBILE_DUN};

    // Need to sync APN_CAP_LIST in CdsPdpService class
    private static final int[] APN_CAP_LIST = new int[] {
        NetworkCapabilities.NET_CAPABILITY_INTERNET,
        NetworkCapabilities.NET_CAPABILITY_MMS,
        NetworkCapabilities.NET_CAPABILITY_SUPL,
        NetworkCapabilities.NET_CAPABILITY_DUN};

    private static final NetworkRequest ALL_REQUESTS = new NetworkRequest.Builder()
        .removeCapability(NetworkCapabilities.NET_CAPABILITY_NOT_VPN)
        .removeCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED)
        .removeCapability(NetworkCapabilities.NET_CAPABILITY_TRUSTED)
        .build();

    private Toast mToast;
    private Spinner mApnSpinner = null;
    private static ConnectivityManager sConnMgr;
    private TelephonyManager mTelephonyManager;
    private INetworkManagementService mNMService;
    private CdsPdpService.ServiceBinder mBinderService;

    private int mSelectApnPos = 0;

    private Context mContext;
    private Button mAddBtnCmd;
    private Button mRunBtnCmd;
    private Button mStopBtnCmd;
    private Button mCheckBtnCmd;
    private Button mRefreshBtnCmd;
    private Button mResetBtnCmd;
    private Button mApnSettingCmd;
    private TextView mPingTest;
    private TextView mOutputScreen;
    private EditText mHostAddress;

    NetworkCallback mNetworkCallback = new NetworkCallback() {
        @Override
        public void onAvailable(Network network) {
                Log.d(TAG, "onAvailable:" + network);
                sHandler.sendMessage(sHandler.obtainMessage(MSG_CONN_UPDATE));
        }

        @Override
        public void onLost(Network network) {
            Log.d(TAG, "onLost:" + network);
            sHandler.sendMessage(sHandler.obtainMessage(MSG_CONN_UPDATE));
        };
    };

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.cds_pdp);

        mContext = this.getBaseContext();

        mApnSpinner = (Spinner) findViewById(R.id.apnTypeSpinnner);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                                    android.R.layout.simple_spinner_item, APN_LIST);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mApnSpinner.setAdapter(adapter);
        mApnSpinner.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View arg1,
            int position, long arg3) {
                // TODO Auto-generated method stub
                mSelectApnPos     = position;
                mApnSpinner.requestFocus();
                updateConnectButton();
            }
            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
                // TODO Auto-generated method stub

            }
        });
        sHandler = new UiHandler(Looper.getMainLooper());

        sConnMgr = (ConnectivityManager) mContext.getSystemService(
                                            Context.CONNECTIVITY_SERVICE);
        mTelephonyManager = (TelephonyManager) mContext.getSystemService(
                                Context.TELEPHONY_SERVICE);
        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNMService = INetworkManagementService.Stub.asInterface(b);

        mRunBtnCmd = (Button) findViewById(R.id.Start);
        mRunBtnCmd.setOnClickListener(this);
        mAddBtnCmd = (Button) findViewById(R.id.Add);
        mAddBtnCmd.setOnClickListener(this);
        mAddBtnCmd.requestFocus();
        mStopBtnCmd = (Button) findViewById(R.id.Stop);
        mStopBtnCmd.setOnClickListener(this);
        mCheckBtnCmd = (Button) findViewById(R.id.Check);
        mCheckBtnCmd.setOnClickListener(this);
        mRefreshBtnCmd = (Button) findViewById(R.id.Refresh);
        mRefreshBtnCmd.setOnClickListener(this);
        mResetBtnCmd = (Button) findViewById(R.id.Reset);
        mResetBtnCmd.setOnClickListener(this);
        mApnSettingCmd = (Button) findViewById(R.id.ApnSetting);
        mApnSettingCmd.setOnClickListener(this);

        mHostAddress   = (EditText)  findViewById(R.id.HostAddress);
        mOutputScreen = (TextView) findViewById(R.id.outputText);
        mPingTest = (TextView) findViewById(R.id.PingTest);
        mToast = Toast.makeText(this, null, Toast.LENGTH_SHORT);

        Intent serviceIntent = new Intent(CdsPdpActivity.this, CdsPdpService.class);
        startService(serviceIntent);
        bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);

        Log.i(TAG, "CdsPdpActivity is started");
    }

    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();
        refreshConnInfo();
        sConnMgr.registerNetworkCallback(ALL_REQUESTS, mNetworkCallback);
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
        sConnMgr.unregisterNetworkCallback(mNetworkCallback);
    }

    @Override
    protected void onDestroy() {
        Log.i(TAG, "onDestroy");
        super.onDestroy();
    }

    public void onClick(View v) {
        int buttonId = v.getId();

        switch (buttonId) {
        case R.id.Start:
            runNetworkRequest();
            break;
        case R.id.Stop:
            stopNetworkRequest();
            break;
        case R.id.Add:
            handleAddIPAddressToRoute();
            break;
        case R.id.Check:
            handleCheckApn();
            break;
        case R.id.Refresh:
            refreshConnInfo();
            break;
        case R.id.Reset:
            quitApp();
            break;
        case R.id.ApnSetting:
            showApnUi();
            break;
        default:
            break;
        }
    }

    private void showApnUi() {
        int subId = SubscriptionManager.getDefaultSubscriptionId();

        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID)
            return;

        try {
            Intent appIntent = new Intent(android.provider.Settings.ACTION_APN_SETTINGS);
            appIntent.putExtra("sub_id", subId);
            startActivity(appIntent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void quitApp() {
        finish();
        android.os.Process.killProcess(android.os.Process.myPid());
    }

    private void handleCheckApn() {
        StringBuilder sb = new StringBuilder();
        String operator = CdsUtils.cdsGetSimOperator();
        String apnName = null;
        String apn = null;
        int    i = 0;
        if (operator == null) {
            Log.e(TAG, "No operator name");
            return;
        }
        String selection = "numeric = '" + operator + "' and carrier_enabled = 1";
        Cursor cursor = mContext.getContentResolver().query(
                        CONTENT_URI, null, selection, null, "name ASC");

        Log.d(TAG, "APN selection:" + selection);

        if (cursor != null) {
            if (cursor.getCount() > 0) {
                if (cursor.moveToFirst()) {
                    String apnTypeName = getApnName(APN_TYPE_LIST[mSelectApnPos]);
                    do {
                        String[] types = parseTypes(cursor.getString(
                                    cursor.getColumnIndexOrThrow("type")));
                        if (canHandleType(apnTypeName, types)) {
                            i++;
                            apnName = cursor.getString(
                                    cursor.getColumnIndexOrThrow("name"));
                            apn = cursor.getString(
                                    cursor.getColumnIndexOrThrow("apn"));
                            sb.append("\n" + i + "." + apnName + ":" + apn);
                        }
                    } while (cursor.moveToNext());
                }
            }
        }

        if (sb.length() != 0) {
            mToast.setText("APN name is " + sb.toString());
        } else {
            mToast.setText("No APN is found.");
        }
        mToast.show();
    }

    private String getApnName(int apnType) {
        String apnTypeName = "";
        switch(apnType) {
            case ConnectivityManager.TYPE_MOBILE:
                apnTypeName = PhoneConstants.APN_TYPE_DEFAULT;
                break;
            case ConnectivityManager.TYPE_MOBILE_MMS:
                apnTypeName = PhoneConstants.APN_TYPE_MMS;
                break;
            case ConnectivityManager.TYPE_MOBILE_SUPL:
                apnTypeName = PhoneConstants.APN_TYPE_SUPL;
                break;
            case ConnectivityManager.TYPE_MOBILE_IMS:
                apnTypeName = PhoneConstants.APN_TYPE_IMS;
                break;
            case ConnectivityManager.TYPE_MOBILE_DUN:
                apnTypeName = PhoneConstants.APN_TYPE_DUN;
                break;
            default:
                break;
        }
        Log.d(TAG, "Check APN type:" + apnTypeName);
        return apnTypeName;
    }

    private String[] parseTypes(String types) {
        String[] result;

        if (types == null || types.equals("")) {
            result = new String[1];
            result[0] = "";
        } else {
            result = types.split(",");
        }

        return result;
    }

    private boolean canHandleType(String type, String[] types) {
        for (String t : types) {
            if (t.equalsIgnoreCase(type)) {
                return true;
            }
        }

        return false;
    }

    private void runNetworkRequest() {
        Log.i(TAG, "runNetworkRequest:" + mSelectApnPos);
        if (mBinderService == null) return;
        try {
            mBinderService.startNetworkRequest(mSelectApnPos);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void stopNetworkRequest() {
        Log.i(TAG, "stopNetworkRequest" + mSelectApnPos);
        if (mBinderService == null) return;
        try {
            mBinderService.stopNetworkRequest(mSelectApnPos);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void updateConnectButton() {
        Log.i(TAG, "updateConnectButton");
        if (mBinderService == null) return;
        try {
            boolean isConnected = mBinderService.checkNetworkReqeust(mSelectApnPos);
            mRunBtnCmd.setEnabled(!isConnected);
            mStopBtnCmd.setEnabled(isConnected);

            if (isConnected) {
                StringBuilder sb = new StringBuilder();
                Network nw = mBinderService.getNetwork(mSelectApnPos);
                LinkProperties link = sConnMgr.getLinkProperties(nw);
                if (link != null) {
                    String iface = link.getInterfaceName();
                    sb.append(IP_CMD_STR + iface + HOST_INFO);
                    sb.append(IPV6_CMD_STR + iface + HOST_INFO);
                    mPingTest.setText(sb.toString());
                }
            } else {
                mPingTest.setText("");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void handleAddIPAddressToRoute() {
        String ipAddress = mHostAddress.getText().toString();
        int addr = 0;

        Log.i(TAG, "handleAddIPAddressToRoute:" + ipAddress);

        boolean isConnected = false;
        try {
            isConnected = mBinderService.checkNetworkReqeust(mSelectApnPos);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        if (isConnected) {
            try {
                InetAddress inetAddr = InetAddress.getByName(ipAddress);
                Network network = mBinderService.getNetwork(mSelectApnPos);
                LinkProperties linkProperties = sConnMgr.getLinkProperties(network);

                //Support for IPv4 & IPv6 address.
                int addrPrefixLen = (inetAddr instanceof Inet4Address) ? 32 : 128;
                RouteInfo routeInfo = new RouteInfo(new LinkAddress(inetAddr, addrPrefixLen),
                                        null, linkProperties.getInterfaceName());
                mNMService.addLegacyRouteForNetId(network.netId, routeInfo, Process.SYSTEM_UID);
                mHostAddress.setText("");

                mToast.setText("Add host:" + ipAddress + " to " + APN_LIST[mSelectApnPos]);
                mToast.show();
            } catch (Exception e) {
                mToast.setText("Fail to add host address" + e.getMessage());
                mToast.show();
            }
        } else {
            mToast.setText("The connection(" + APN_LIST[mSelectApnPos] + ") is not connected");
            mToast.show();
        }
    }

    private void refreshConnInfo() {
        updateConnectButton();
        updateApnStatus();
    }

    private void updateApnStatus() {
        NetworkInfo    nwInfo;
        LinkProperties nwLink;
        NetworkCapabilities nwCap;
        String sb;
        StringBuilder tb = new StringBuilder();

        Network[] networks = sConnMgr.getAllNetworks();

        SimpleDateFormat sdf = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
        Date dt = new Date();
        sb = sdf.format(dt) + "\r\n\r\n";
        tb.append(sb);

        for (int i = 0; i < networks.length; i ++) {
            nwInfo = sConnMgr.getNetworkInfo(networks[i]);
            if (nwInfo != null && ConnectivityManager.isNetworkTypeMobile(nwInfo.getType())) {
                sb = nwInfo.toString() + "\r\n\r\n";
                if (nwInfo.getState() == NetworkInfo.State.CONNECTED) {
                    nwLink = sConnMgr.getLinkProperties(networks[i]);
                    if (nwLink != null) {
                        sb = nwLink.toString() + "\r\n\r\n";
                        tb.append(sb.replace(',', '\n'));
                    }
                    nwCap = sConnMgr.getNetworkCapabilities(networks[i]);
                    if (nwCap != null) {
                        sb = nwCap.toString() + "\r\n\r\n";
                        tb.append(sb.replace(',', '\n'));
                    }
                }
            }
        }
        updateConnectButton();
        mOutputScreen.setText(tb.toString());
    }

    /**
     *
     * Handler class for handle aysnc tasks or events.
     *
     **/
    final class UiHandler extends Handler {
        public UiHandler(Looper looper) {
            super(looper, null, true);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_CONN_UPDATE:
                    refreshConnInfo();
                    break;
                case MSG_UI_UPDATE:
                    updateConnectButton();
                    break;
                default:
                    break;
            }
        }
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceDisconnected(ComponentName name) {

        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mBinderService = (CdsPdpService.ServiceBinder) service;
            Log.d(TAG, "PDP service is connected");
        }
    };

}
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.location.agps.apn;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.provider.Telephony.Carriers;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.location.agps.apn.AgpsApn.AgpsNetReq;
import com.mediatek.location.agps.apn.Framework2AgpsInterface.Framework2AgpsInterfaceSender;
import com.mediatek.socket.base.UdpClient;

import java.net.InetAddress;
import java.net.UnknownHostException;


public class AgpsApnC2K {
    private static final boolean DEBUG = true;
    private static final String TAG = "AgpsApnC2K";

    private final ConnectivityManager mConnManager;
    private final Context mContext;
    private Framework2AgpsInterfaceSender mToAgpsSender;
    private UdpClient mUdpClient;
    private Handler mHandler;

    private int mAGpsDataConnectionState;
    private InetAddress mAGpsDataConnectionIpAddr;

    private static final int AGPS_DATA_CONNECTION_CLOSED = 0;
    private static final int AGPS_DATA_CONNECTION_OPENING = 1;
    private static final int AGPS_DATA_CONNECTION_OPEN = 2;

    // these must match the definitions in gps.h
    private static final int APN_INVALID = 0;
    private static final int APN_IPV4 = 1;
    private static final int APN_IPV6 = 2;
    private static final int APN_IPV4V6 = 3;
    // these need to match GpsApgsStatusValue defines in gps.h
    /** AGPS status event values. */
    private static final int GPS_REQUEST_AGPS_DATA_CONN = 1;
    private static final int GPS_RELEASE_AGPS_DATA_CONN = 2;
    private static final int GPS_AGPS_DATA_CONNECTED = 3;
    private static final int GPS_AGPS_DATA_CONN_DONE = 4;
    private static final int GPS_AGPS_DATA_CONN_FAILED = 5;

    private static final int NET_STATE_AVAILABLE = 0;
    private static final int NET_STATE_LOST = 1;
    private static final int NET_STATE_UNAVAILABLE = 2;

    private static final String DEFAULT_APN_NAME = "dummy-apn";

    public AgpsApnC2K(Context context, ConnectivityManager manager,
        Framework2AgpsInterfaceSender sender, UdpClient nodeOut, Handler handler) {
        mContext = context;
        mConnManager = manager;
        mToAgpsSender = sender;
        mUdpClient = nodeOut;
        mHandler = handler;

        mToAgpsSender.AirplaneModeChanged(mUdpClient, isAirplaneModeOn());
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mContext.registerReceiver(mBroadcastReceiver, intentFilter, null, mHandler);
    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (DEBUG) Log.d(TAG, "receive broadcast intent, action: " + action);
            if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
                boolean enabled = intent.getBooleanExtra("state", false);
                if (DEBUG) Log.d(TAG, "ACTION_AIRPLANE_MODE_CHANGED enabled =" + enabled);
                mToAgpsSender.AirplaneModeChanged(mUdpClient, enabled);
            }
        }
    };

    private final ConnectivityManager.NetworkCallback mSuplConnectivityCallback =
                new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                Log.d(TAG, "onAvailable");
                sendMessage(AgpsApn.CMD_C2K_APN_CALLBACK, NET_STATE_AVAILABLE, network);
            }

            @Override
            public void onLost(Network network) {
                Log.d(TAG, "onLost");
                sendMessage(AgpsApn.CMD_C2K_APN_CALLBACK, NET_STATE_LOST, network);
            }

            @Override
            public void onUnavailable() {
                Log.d(TAG, "onUnavailable");
                sendMessage(AgpsApn.CMD_C2K_APN_CALLBACK, NET_STATE_UNAVAILABLE, null);
            }
        };

    void handleRequestDataConn(Object obj) {
        Log.d(TAG, "handleRequestDataConn");
        AgpsNetReq req = null;
        if (obj instanceof AgpsNetReq) {
            req = (AgpsNetReq )obj;
        } else {
            Log.e(TAG, "wrong obj, " + obj);
            return;
        }

        if (null != req) {
            byte[] ipaddr = req.mAddr;
            Log.d(TAG, "Received SUPL IP addr[]: " + ipaddr);
            InetAddress connectionIpAddress = null;
            if (ipaddr != null) {
                try {
                    connectionIpAddress = InetAddress.getByAddress(ipaddr);
                    if (DEBUG) Log.d(TAG, "IP address converted to: " + connectionIpAddress);
                } catch (UnknownHostException e) {
                    Log.d(TAG, "Bad IP Address: " + ipaddr);
                }
            }

            if (mAGpsDataConnectionState != AGPS_DATA_CONNECTION_CLOSED) {
                return;
            }
            mAGpsDataConnectionIpAddr = connectionIpAddress;
            mAGpsDataConnectionState = AGPS_DATA_CONNECTION_OPENING;

            NetworkRequest.Builder requestBuilder = new NetworkRequest.Builder();
            requestBuilder.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
            requestBuilder.addCapability(NetworkCapabilities.NET_CAPABILITY_SUPL);
            NetworkRequest request = requestBuilder.build();
            mConnManager.requestNetwork(
                request,
                mSuplConnectivityCallback);
        }
    }

    void handleNetworkCallback(int state, Object obj) {
        Log.d(TAG, "handleNetworkCallback");
        Network network = null;
        if (obj instanceof Network) {
            network = (Network )obj;
        } else {
            Log.e(TAG, "wrong obj, " + obj);
            return;
        }

        switch(state) {
        case NET_STATE_AVAILABLE:
        {
            NetworkInfo info = mConnManager.getNetworkInfo(network);
            TelephonyManager tm = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
            boolean dataEnabled = tm.isDataEnabled();
            boolean networkAvailable = info.isAvailable() && dataEnabled;
            String defaultApn = null;

            if (info.getType() != ConnectivityManager.TYPE_WIFI) {
                defaultApn = getSelectedApn();
            }
            if (defaultApn == null) {
                defaultApn = DEFAULT_APN_NAME;
            }

            String apnName = DEFAULT_APN_NAME;
            int apnIpType = APN_INVALID;
            if (mAGpsDataConnectionState == AGPS_DATA_CONNECTION_OPENING) {
                if (info.isConnected()) {
                    apnName = info.getExtraInfo();
                    if (apnName == null) {
                        // assign a dummy value in the case of C2K as otherwise we will have a
                        // runtime exception in the following call to native_agps_data_conn_open
                        apnName = DEFAULT_APN_NAME;
                    }
                    apnIpType = getApnIpType(apnName);
                    if (DEBUG) {
                        String message = String.format(
                            "native_agps_data_conn_open: mAgpsApn=%s, mApnIpType=%s",
                            apnName,
                            apnIpType);
                        Log.d(TAG, message);
                    }
                    mAGpsDataConnectionState = AGPS_DATA_CONNECTION_OPEN;
                } else {
                    releaseSuplConnection(GPS_AGPS_DATA_CONN_FAILED);
                }
            }

            boolean ret = mToAgpsSender.C2kApnResult(mUdpClient, (int)(network.getNetworkHandle()>>32),
                info.isConnected(), info.getType(), info.isRoaming(), networkAvailable, 
                info.getExtraInfo(), defaultApn, apnName, apnIpType);
            if (DEBUG) {
                Log.d(TAG, "RequestDataConnResult, netId = " + (int)(network.getNetworkHandle()>>32)
                    + ", NetworkInfo = " + info
                    + ", networkAvailable = " + networkAvailable
                    + ", defaultApn = " + defaultApn
                    + ", apnName = " + apnName
                    + ", apnIpType = " + apnIpType
                    + ", ret = " + ret);
            }
        }
        break;
        case NET_STATE_LOST:
            releaseSuplConnection(GPS_RELEASE_AGPS_DATA_CONN);
            break;
        case NET_STATE_UNAVAILABLE:
            // timeout, it was not possible to establish the required connection
            releaseSuplConnection(GPS_AGPS_DATA_CONN_FAILED);
            break;
        default:
            Log.e(TAG, "handleNetworkCallback, error state = " + state);
            break;
        }
    }

    void handleReleaseDataConn() {
        releaseSuplConnection(GPS_RELEASE_AGPS_DATA_CONN);
    }

    private String getSelectedApn() {
        Uri uri = Uri.parse("content://telephony/carriers/preferapn");
        Cursor cursor = null;
        try {
            cursor = mContext.getContentResolver().query(
                    uri,
                    new String[] { "apn" },
                    null /* selection */,
                    null /* selectionArgs */,
                    Carriers.DEFAULT_SORT_ORDER);
            if (cursor != null && cursor.moveToFirst()) {
                return cursor.getString(0);
            } else {
                Log.d(TAG, "No APN found to select.");
            }
        } catch (Exception e) {
            Log.d(TAG, "Error encountered on selecting the APN.", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        return null;
    }

    private int getApnIpType(String apn) {
        if (apn == null) {
            return APN_INVALID;
        }

        String selection = String.format("current = 1 and apn = '%s' and carrier_enabled = 1", apn);
        Cursor cursor = null;
        try {
            cursor = mContext.getContentResolver().query(
                    Carriers.CONTENT_URI,
                    new String[] { Carriers.PROTOCOL },
                    selection,
                    null,
                    Carriers.DEFAULT_SORT_ORDER);

            if (null != cursor && cursor.moveToFirst()) {
                return translateToApnIpType(cursor.getString(0), apn);
            } else {
                Log.e(TAG, "No entry found in query for APN: " + apn);
            }
        } catch (Exception e) {
            Log.e(TAG, "Error encountered on APN query for: " + apn, e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        return APN_INVALID;
    }

    private int translateToApnIpType(String ipProtocol, String apn) {
        if ("IP".equals(ipProtocol)) {
            return APN_IPV4;
        }
        if ("IPV6".equals(ipProtocol)) {
            return APN_IPV6;
        }
        if ("IPV4V6".equals(ipProtocol)) {
            return APN_IPV4V6;
        }

        // we hit the default case so the ipProtocol is not recognized
        String message = String.format("Unknown IP Protocol: %s, for APN: %s", ipProtocol, apn);
        Log.e(TAG, message);
        return APN_INVALID;
    }

    void sendMessage(int what, int state, Object obj) {
        mHandler.obtainMessage(what, state, 0, obj).sendToTarget();
    }

    private void releaseSuplConnection(int agpsDataConnStatus) {
        if (DEBUG) {
            String message = String.format(
                    "releaseSuplConnection, state=%s, status=%s",
                    agpsDataConnStateAsString(),
                    agpsDataConnStatusAsString(agpsDataConnStatus));
            Log.d(TAG, message);
        }

        if (mAGpsDataConnectionState == AGPS_DATA_CONNECTION_CLOSED) {
            return;
        }
        mAGpsDataConnectionState = AGPS_DATA_CONNECTION_CLOSED;

        mConnManager.unregisterNetworkCallback(mSuplConnectivityCallback);
        switch (agpsDataConnStatus) {
            case GPS_AGPS_DATA_CONN_FAILED:
                mToAgpsSender.C2kApnFailed(mUdpClient);
                break;
            case GPS_RELEASE_AGPS_DATA_CONN:
                mToAgpsSender.C2kApnClosed(mUdpClient);
                break;
            default:
                Log.e(TAG, "Invalid status to release SUPL connection: " + agpsDataConnStatus);
        }
    }

   /**
    * @return A string representing the current state stored in {@link #mAGpsDataConnectionState}.
    */
   private String agpsDataConnStateAsString() {
       switch(mAGpsDataConnectionState) {
           case AGPS_DATA_CONNECTION_CLOSED:
               return "CLOSED";
           case AGPS_DATA_CONNECTION_OPEN:
               return "OPEN";
           case AGPS_DATA_CONNECTION_OPENING:
               return "OPENING";
           default:
               return "<Unknown>";
       }
   }

   /**
    * @return A string representing the given GPS_AGPS_DATA status.
    */
   private String agpsDataConnStatusAsString(int agpsDataConnStatus) {
       switch (agpsDataConnStatus) {
           case GPS_AGPS_DATA_CONNECTED:
               return "CONNECTED";
           case GPS_AGPS_DATA_CONN_DONE:
               return "DONE";
           case GPS_AGPS_DATA_CONN_FAILED:
               return "FAILED";
           case GPS_RELEASE_AGPS_DATA_CONN:
               return "RELEASE";
           case GPS_REQUEST_AGPS_DATA_CONN:
               return "REQUEST";
           default:
               return "<Unknown>";
       }
   }

   boolean isAirplaneModeOn() {
       return Settings.System.getInt(mContext.getContentResolver(),
               Settings.System.AIRPLANE_MODE_ON, 0) == 1;
   }

}

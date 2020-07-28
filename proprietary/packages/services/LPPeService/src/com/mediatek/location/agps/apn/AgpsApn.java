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

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.LocalSocketAddress.Namespace;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.PowerManager;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;
import com.mediatek.location.agps.apn.Agps2FrameworkInterface.Agps2FrameworkInterfaceReceiver;
import com.mediatek.location.agps.apn.AgpsApnC2K;
import com.mediatek.location.agps.apn.AgpsNiEncodingType;
import com.mediatek.location.agps.apn.AgpsNiResponseType;
import com.mediatek.location.agps.apn.AgpsNiType;
import com.mediatek.location.agps.apn.AgpsNotifyType;
import com.mediatek.location.agps.apn.Framework2AgpsInterface.Framework2AgpsInterfaceSender;
import com.mediatek.location.agps.apn.GpsNetInitiatedHandler;
import com.mediatek.location.agps.apn.GpsNetInitiatedHandler.GpsNiNotification;
import com.mediatek.location.agps.apn.NetInitiatedListener;

import com.mediatek.socket.base.UdpClient;
import com.mediatek.socket.base.UdpServer;
import java.lang.Thread;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Arrays;

/**
 * A worker which will help mtk_agpsd to call Android APIs
 * for features which are not available in native APIs.
 *
 * {@hide}
 */
public class AgpsApn extends Thread {
    private static final boolean DEBUG = true;

    // Handler messages
    private static final int CMD_REQUEST_NET = 100;
    private static final int CMD_QUERY_DNS   = 101;
    private static final int CMD_NET_TIMEOUT = 102;
    private static final int CMD_RELEASE_NET = 103;
    private static final int CMD_REQUEST_GPS_ICON = 104;
    private static final int CMD_REMOVE_GPS_ICON = 105;
    private static final int CMD_REQUEST_C2K_APN = 106;
    static final int CMD_C2K_APN_CALLBACK = 107;
    private static final int CMD_RELEASE_C2K_APN = 108;
    private static final int CMD_NI_NOTIFY = 109;

    private static final String TAG = "AgpsApn";
    private static final String CHANNEL_OUT = "mtk_framework2agps"; // reply to mtk_agpsd
    private static final String CHANNEL_IN  = "mtk_agps2framework"; // get cmd from mtk_agpsd

    private static final String WAKELOCK_KEY = "MtkAgps";
    private static final long NET_REQ_TIMEOUT = 10000;

    private final ConnectivityManager mConnManager;
    private final Context mContext;

    private final ArrayList<AgpsNetReq> mAgpsNetReqs = new ArrayList<AgpsNetReq>(2);
    private final byte [] mEmptyIpv6 = new byte[16];

    private PowerManager mPowerManager;
    private PowerManager.WakeLock mWakeLock;
    private HandlerThread mHandlerThread;
    private Handler mHandler;
    private UdpServer agpsApnServer;
    private UdpClient mUdpClient;
    private Framework2AgpsInterfaceSender mToAgpsSender;
    private NetworkRequest mNetReqSupl;
    private NetworkRequest mNetReqEmergency;
    private NetworkRequest mNetReqIms;

    private byte mEsuplApnMode;

    private AgpsApnC2K mC2KApn;
    private GpsNetInitiatedHandler mNIHandler;

    public static void log(String msg) {
        Log.d(TAG, msg);
    }

    public AgpsApn(Context context) {
        if (DEBUG) log("AgpsApn constructor");
        mContext = context;
        mConnManager = (ConnectivityManager) mContext.
                getSystemService(Context.CONNECTIVITY_SERVICE);

        setup();

        int buffSizeIn  = Agps2FrameworkInterface.MAX_BUFF_SIZE;
        agpsApnServer = new UdpServer(CHANNEL_IN, Namespace.ABSTRACT, buffSizeIn);
        agpsApnServer.setReceiver(mReceiver);

        int buffSizeOut = Framework2AgpsInterface.MAX_BUFF_SIZE;
        mToAgpsSender = new Framework2AgpsInterfaceSender();
        mUdpClient = new UdpClient(CHANNEL_OUT, Namespace.ABSTRACT, buffSizeOut);
        mC2KApn = new AgpsApnC2K(mContext, mConnManager, mToAgpsSender, mUdpClient,mHandler);

        mNIHandler = new GpsNetInitiatedHandler(mContext,
                mNetInitiatedListener,
                /*mSuplEsEnabled*/ true);
    }

    private Agps2FrameworkInterfaceReceiver mReceiver
            = new Agps2FrameworkInterfaceReceiver() {
        @Override
        public void isExist(byte esuplApnMode) {
            if (DEBUG) log("isExist() " + esuplApnMode);
			mEsuplApnMode = esuplApnMode;
        }

        @Override
        public void acquireWakeLock() {
            if (DEBUG) log("acquireWakeLock()");
            mWakeLock.acquire();
        }

        @Override
        public void releaseWakeLock() {
            if (DEBUG) log("releaseWakeLock()");
            mWakeLock.release();
        }

        @Override
        public void requestDedicatedApnAndDnsQuery(
                String fqdn, boolean isEsupl, boolean isSuplApn) {
            if (DEBUG) {
                    log("requestDedicatedApnAndDnsQuery() fqdn=" + fqdn +
                        " isEsupl=" + isEsupl +
                        " isSuplApn=" + isSuplApn);
            }
            AgpsNetReq agpsNetReq = new AgpsNetReq(fqdn, isEsupl, isSuplApn);
            sendMessage(CMD_REQUEST_NET, agpsNetReq);
        }

        @Override
        public void releaseDedicatedApn() {
            if (DEBUG) log("releaseDedicatedApn()");
            sendMessage(CMD_RELEASE_NET, null);
        }

        @Override
        public void requestGpsIcon(){
            if (DEBUG) log("requestGpsIcon");
            sendMessage(CMD_REQUEST_GPS_ICON, null);
        }

        @Override
        public void removeGpsIcon(){
            if (DEBUG) log("removeGpsIcon()");
            sendMessage(CMD_REMOVE_GPS_ICON, null);
        }

        @Override
        public void requestC2kApn(byte[] addr, boolean isEmergency) {
            if (DEBUG) log("requestC2kApn(), addr = " + addr + ", isE = " + isEmergency);
            AgpsNetReq agpsNetReq = new AgpsNetReq(addr, isEmergency);
            sendMessage(CMD_REQUEST_C2K_APN, agpsNetReq);
        }

        @Override
        public void releaseC2kApn() {
            if (DEBUG) log("releaseC2kApn()");
            sendMessage(CMD_RELEASE_C2K_APN, null);
        }
        @Override
        public void AgpsNiNotify(int sessionId, AgpsNiType niType,
                AgpsNotifyType notifyType, String requestorId, String text,
                AgpsNiEncodingType requestorIdEncoding, AgpsNiEncodingType textEncoding) {
            if (DEBUG) log("AgpsNiNotify(), sessionId= " + sessionId + ", niType= " + niType
                +", notifyType= " + notifyType + ", requestorId= " + requestorId
                +", text= " + text + ", requestorIdEncoding= " + requestorIdEncoding
                +", textEncoding= " + textEncoding);

            GpsNiNotification notification = new GpsNiNotification();

            notification.notificationId = sessionId;
            switch(niType) {
                case AGPS_NI_TYPE_VOICE:
                    notification.niType = GpsNetInitiatedHandler.GPS_NI_TYPE_VOICE;
                    break;
                case AGPS_NI_TYPE_UMTS_SUPL:
                    notification.niType = GpsNetInitiatedHandler.GPS_NI_TYPE_UMTS_SUPL;
                    break;
                case AGPS_NI_TYPE_UMTS_CTRL_PLANE:
                    notification.niType = GpsNetInitiatedHandler.GPS_NI_TYPE_UMTS_CTRL_PLANE;
                    break;
                case AGPS_NI_TYPE_EMERGENCY_SUPL:
                    notification.niType = GpsNetInitiatedHandler.GPS_NI_TYPE_EMERGENCY_SUPL;
                    break;
            }

            notification.needNotify = false;
            notification.needVerify = false;
            notification.privacyOverride = false;

            switch (notifyType) {
                case AGPS_NOTIFY_TYPE_NOTIFY_ONLY:
                    //gnn.notify_flags = GPS_NI_NEED_NOTIFY;
                    notification.needNotify = true;
                    break;
                case AGPS_NOTIFY_TYPE_NOTIFY_ALLOW_NO_ANSWER:
                    //gnn.notify_flags = GPS_NI_NEED_NOTIFY|GPS_NI_NEED_VERIFY;
                    notification.needNotify = true;
                    notification.needVerify = true;
                    break;
                case AGPS_NOTIFY_TYPE_NOTIFY_DENY_NO_ANSWER:
                    //gnn.notify_flags = GPS_NI_NEED_NOTIFY|GPS_NI_NEED_VERIFY;
                    notification.needNotify = true;
                    notification.needVerify = true;
                    break;
                case AGPS_NOTIFY_TYPE_PRIVACY:
                    //gnn.notify_flags = GPS_NI_PRIVACY_OVERRIDE;
                    notification.privacyOverride = true;
                    break;
                default:
                    //gnn.notify_flags = 0;
                    break;
            }

            notification.timeout = 8;
            notification.defaultResponse = GpsNetInitiatedHandler.GPS_NI_RESPONSE_NORESP;
            notification.requestorId = requestorId;
            notification.text = text;
            switch(requestorIdEncoding) {
                case AGPS_NI_ENCODING_TYPE_NONE:
                    notification.requestorIdEncoding = GpsNetInitiatedHandler.GPS_ENC_NONE;
                    break;
                case AGPS_NI_ENCODING_TYPE_GSM7:
                    notification.requestorIdEncoding = GpsNetInitiatedHandler.GPS_ENC_SUPL_GSM_DEFAULT;
                    break;
                case AGPS_NI_ENCODING_TYPE_UTF8:
                    notification.requestorIdEncoding = GpsNetInitiatedHandler.GPS_ENC_SUPL_UTF8;
                    break;
                case AGPS_NI_ENCODING_TYPE_UCS2:
                    notification.requestorIdEncoding = GpsNetInitiatedHandler.GPS_ENC_SUPL_UCS2;
                    break;
                case AGPS_NI_ENCODING_TYPE_ASCII:
                    notification.requestorIdEncoding = GpsNetInitiatedHandler.GPS_ENC_SUPL_ASCII;
                    break;
                default:
                    notification.requestorIdEncoding = GpsNetInitiatedHandler.GPS_ENC_UNKNOWN;
                    break;
            }
            switch(textEncoding) {
                case AGPS_NI_ENCODING_TYPE_NONE:
                    notification.textEncoding = GpsNetInitiatedHandler.GPS_ENC_NONE;
                    break;
                case AGPS_NI_ENCODING_TYPE_GSM7:
                    notification.textEncoding = GpsNetInitiatedHandler.GPS_ENC_SUPL_GSM_DEFAULT;
                    break;
                case AGPS_NI_ENCODING_TYPE_UTF8:
                    notification.textEncoding = GpsNetInitiatedHandler.GPS_ENC_SUPL_UTF8;
                    break;
                case AGPS_NI_ENCODING_TYPE_UCS2:
                    notification.textEncoding = GpsNetInitiatedHandler.GPS_ENC_SUPL_UCS2;
                    break;
                case AGPS_NI_ENCODING_TYPE_ASCII:
                    notification.textEncoding = GpsNetInitiatedHandler.GPS_ENC_SUPL_ASCII;
                    break;
                default:
                    notification.textEncoding = GpsNetInitiatedHandler.GPS_ENC_UNKNOWN;
                    break;
            }
            sendMessage(CMD_NI_NOTIFY, notification);
        }
    };

    protected void setup() {
        mPowerManager = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = mPowerManager
                .newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, WAKELOCK_KEY);
        mWakeLock.setReferenceCounted(true);
        NetworkRequest.Builder nrBuilder = new NetworkRequest.Builder();
        mNetReqEmergency = nrBuilder
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_EIMS)
                .build();
        mNetReqIms = nrBuilder
                .removeCapability(NetworkCapabilities.NET_CAPABILITY_EIMS)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_IMS)
                .build();
        mNetReqSupl = nrBuilder
                .removeCapability(NetworkCapabilities.NET_CAPABILITY_IMS)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_SUPL)
                .build();
        mHandlerThread = new HandlerThread("MtkAgpsHandler");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper()) {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case CMD_REQUEST_NET:
                        handleRequestNet((AgpsNetReq)msg.obj);
                        break;
                    case CMD_QUERY_DNS:
                        handleDnsQuery((AgpsNetReq)msg.obj);
                        break;
                    case CMD_NET_TIMEOUT:
                        handleNetTimeout((AgpsNetReq)msg.obj);
                        break;
                    case CMD_RELEASE_NET:
                        handleReleaseNet((AgpsNetReq)msg.obj);
                        break;
                    case CMD_REQUEST_GPS_ICON:
                        handleRequestGpsIcon();
                        break;
                    case CMD_REMOVE_GPS_ICON:
                        handleRemoveGpsIcon();
                        break;
                    case CMD_REQUEST_C2K_APN:
                        mC2KApn.handleRequestDataConn(msg.obj);
                        break;
                    case CMD_C2K_APN_CALLBACK:
                        mC2KApn.handleNetworkCallback(msg.arg1, msg.obj);
                        break;
                    case CMD_RELEASE_C2K_APN:
                        mC2KApn.handleReleaseDataConn();
                        break;
                    case CMD_NI_NOTIFY:
                        handleNinotify((GpsNiNotification)msg.obj);
                        break;
                }
            }
        };
    }

    void sendMessage(int what, Object obj) {
        mHandler.obtainMessage(what, 0, 0, obj).sendToTarget();
    }

    void sendMessageDelayed(int what, Object obj, long delayMillis) {
        Message msg = mHandler.obtainMessage(what, 0, 0, obj);
        mHandler.sendMessageDelayed(msg, delayMillis);
    }

    void removeMessages(int what, Object obj) {
        mHandler.removeMessages(what, obj);
    }

    void doReleaseNet(AgpsNetReq req) {
        if (DEBUG) log("doReleaseNet");
        mAgpsNetReqs.remove(req);
        req.releaseNet();
    }

    void handleRequestNet(AgpsNetReq req) {
        if (DEBUG) log("handleRequestNet");
        while (mAgpsNetReqs.size() >= 2) {
            // mtk_agpsd may crash due to bug or watch dog
            if (DEBUG) log("remove potential leak of AgpsNetReq");
            doReleaseNet(mAgpsNetReqs.get(0));
        }
        mAgpsNetReqs.add(req);
        req.requestNet();
    }

    void handleDnsQuery(AgpsNetReq req) {
        if (DEBUG) log("handleDnsQuery");
        req.queryDns();
    }

    void handleNetTimeout(AgpsNetReq req) {
        if (DEBUG) log("handleNetTimeout");
        req.queryDns();
    }

    void handleReleaseNet(AgpsNetReq req) {
        if (DEBUG) log("handleReleaseNet");
        if (null != req) {
            doReleaseNet(req);
        } else {
            if (!mAgpsNetReqs.isEmpty()) {
                doReleaseNet(mAgpsNetReqs.get(0));
            }
        }
    }

    void handleRequestGpsIcon() {
    }

    void handleRemoveGpsIcon() {
    }

    void handleNinotify(GpsNiNotification notification) {
        mNIHandler.handleNiNotification(notification);
    }

    private final NetInitiatedListener mNetInitiatedListener = new NetInitiatedListener() {
        // Sends a response for an NI request to HAL.
        @Override
        public boolean sendNiResponse(int notificationId, int userResponse) {
            if (DEBUG) {
                Log.d(TAG, "sendNiResponse, notifId: " + notificationId +
                        ", response: " + userResponse);
            }
            AgpsNiResponseType userNiResponse = AgpsNiResponseType.AGPS_NI_RESPONSE_IGNORE;
            switch(userResponse) {
                case GpsNetInitiatedHandler.GPS_NI_RESPONSE_ACCEPT:
                    userNiResponse = AgpsNiResponseType.AGPS_NI_RESPONSE_ACCEPT;
                    break;
                case GpsNetInitiatedHandler.GPS_NI_RESPONSE_DENY:
                    userNiResponse = AgpsNiResponseType.AGPS_NI_RESPONSE_DENY;
                    break;
                case GpsNetInitiatedHandler.GPS_NI_RESPONSE_NORESP:
                    userNiResponse = AgpsNiResponseType.AGPS_NI_RESPONSE_NORESP;
                    break;
                case GpsNetInitiatedHandler.GPS_NI_RESPONSE_IGNORE:
                    userNiResponse = AgpsNiResponseType.AGPS_NI_RESPONSE_IGNORE;
                    break;
                default:
                    break;
            }
            mToAgpsSender.AgpsNiResponse(mUdpClient, notificationId, userNiResponse);
            return true;
        }
    };

    boolean hasIccCard() {
        TelephonyManager tpMgr = (TelephonyManager)
                mContext.getSystemService(Context.TELEPHONY_SERVICE);
        if (tpMgr != null) {
            return tpMgr.hasIccCard();
        }
        return false;
    }

    boolean isAirplaneModeOn() {
        return Settings.System.getInt(mContext.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) == 1;
    }

    class AgpsNetReq {
        String mFqdn;
        byte[] mAddr;
        boolean mIsEsupl;
        boolean mIsSuplApn;
        boolean mIsQueried = false;
        NetworkRequest mNetReq = null;
        Network mNet = null;
        ConnectivityManager.NetworkCallback mNetworkCallback = null;

        AgpsNetReq(String fqdn, boolean isEsupl, boolean isSuplApn) {
            mFqdn = fqdn;
            mIsEsupl = isEsupl;
            mIsSuplApn = isSuplApn;
        }

        AgpsNetReq(byte[] addr, boolean isEmergency) {
            mAddr = addr;
            mIsEsupl = isEmergency;
        }

        void decideRoute() {
            Network netEmergemcy = null;
            Network netIms = null;
            Network netSupl = null;
            Network [] nets = mConnManager.getAllNetworks();
            if (null != nets) {
                for (Network net : nets) {
                    NetworkCapabilities netCap = mConnManager.getNetworkCapabilities(net);
                    if (DEBUG) log("checking net=" + net + " cap=" + netCap);
                    if (null == netEmergemcy && null != netCap &&
                            netCap.hasCapability(NetworkCapabilities.NET_CAPABILITY_EIMS)) {
                        netEmergemcy = net;
                        if (DEBUG) log("NetEmergemcy");
                    }
                    if (null == netIms && null != netCap &&
                            netCap.hasCapability(NetworkCapabilities.NET_CAPABILITY_IMS)) {
                        netIms = net;
                        if (DEBUG) log("NetIms");
                    }
                    if (null == netSupl && null != netCap &&
                            netCap.hasCapability(NetworkCapabilities.NET_CAPABILITY_SUPL)) {
                        netSupl = net;
                        if (DEBUG) log("NetSupl");
                    }
                }
            }
            if (mIsEsupl) {
                //mEsuplApnMode , bit 0 : disable IMS pdn,  bit 1: disable emergency pdn
                if (null != netEmergemcy && (mEsuplApnMode & 2) == 0) { // allow emergency pdn
                    if (DEBUG) log("to use NetEmergemcy");
                    mNet = netEmergemcy;
                    mNetReq = mNetReqEmergency;
                    return;
                } else if (null != netIms && (mEsuplApnMode & 1) == 0) { // allow IMS pdn
                    if (DEBUG) log("to use NetIms");
                    mNet = netIms;
                    mNetReq = mNetReqIms;
                    return;
                }
            }
            if (mIsSuplApn && hasIccCard() && !isAirplaneModeOn()) {
                if (DEBUG) log("try to use NetSupl");
                mNet = netSupl;
                mNetReq = mNetReqSupl;
            }
        }

        void requestNet() {
            boolean isDirectDns = false;
            decideRoute();
            if (null != mNetReq) {
                mNetworkCallback = new ConnectivityManager.NetworkCallback() {
                    @Override
                    public void onAvailable(Network net) {
                        if (DEBUG) log("onAvailable: network=" + net);
                        synchronized (AgpsNetReq.this) {
                            if (null == mNet) {
                                mNet = net;
                                removeMessages(CMD_NET_TIMEOUT, AgpsNetReq.this);
                                sendMessage(CMD_QUERY_DNS, AgpsNetReq.this);
                            }
                        }
                    }

                    @Override
                    public void onLost(Network net) {
                        if (DEBUG) log("onLost: network=" + net);
                        //sendMessageDelayed(CMD_RELEASE_NET, AgpsNetReq.this, NET_REQ_TIMEOUT);
                    }
                };
                synchronized (this) {
                    if (DEBUG) log("request net:" + mNetReq);
                    mConnManager.requestNetwork(mNetReq, mNetworkCallback);
                    if (null == mNet) {
                        if (DEBUG) log("wait for net callback");
                        sendMessageDelayed(CMD_NET_TIMEOUT, this, NET_REQ_TIMEOUT);
                    } else {
                        //sendMessage(CMD_QUERY_DNS, this);
                        isDirectDns = true;
                    }
                }
            } else {
                //sendMessage(CMD_QUERY_DNS, this);
                isDirectDns = true;
            }
            if (isDirectDns) {
                queryDns();
            }
        }

        void queryDns() {
            if (mIsQueried) return;
            mIsQueried = true;

            boolean hasIpv4 = false;
            boolean hasIpv6 = false;
            int ipv4 = 0;
            byte [] ipv6 = mEmptyIpv6;
            try {
                InetAddress[] ias;
                if (null != mNet) {
                    ias = mNet.getAllByName(mFqdn);
                } else {
                    ias = InetAddress.getAllByName(mFqdn);
                }
                for (InetAddress ia : ias) {
                    byte [] addr = ia.getAddress();
                    log("ia=" + ia.toString() + " bytes=" +
                            Arrays.toString(addr) + " network=" + mNet);
                    if (addr.length == 4 && !hasIpv4) {
                        hasIpv4 = true;
                        ipv4 = addr[3] & 0xFF;
                        ipv4 = (ipv4 << 8) | (addr[2] & 0xFF);
                        ipv4 = (ipv4 << 8) | (addr[1] & 0xFF);
                        ipv4 = (ipv4 << 8) | (addr[0] & 0xFF);
                    } else if (addr.length == 16 && !hasIpv6) {
                        hasIpv6 = true;
                        ipv6 = addr;
                    }
                }
            } catch (UnknownHostException e) {
                log("UnknownHostException for fqdn=" + mFqdn);
            }
            boolean isSuccess = hasIpv4 || hasIpv6;
            boolean hasNetId = (null != mNet);
            int netId = (hasNetId)? (int)(mNet.getNetworkHandle()>>32) : -1;
            boolean ret = mToAgpsSender.DnsQueryResult2(mUdpClient, isSuccess, hasIpv4, ipv4,
                    hasIpv6, ipv6, hasNetId, netId);
            if (DEBUG) log("DnsQueryResult() fqdn=" + mFqdn +
                        " isSuccess=" + isSuccess +
                        " hasIpv4=" + hasIpv4 +
                        " ipv4=" + Integer.toHexString(ipv4) +
                        " hasIpv6=" + hasIpv6 +
                        " ipv6=" + Arrays.toString(ipv6) +
                        " hasNetId=" + hasNetId +
                        " netId=" + netId +
                        " ret=" + ret);
            if (!isSuccess) {
                // mtk_agpsd will not ask to release this failed request
                doReleaseNet(this);
            }
        }

        synchronized void releaseNet() {
            if (DEBUG) log("releaseNet() fqdn=" + mFqdn + " eSupl=" + mIsEsupl +
                    " suplApn=" + mIsSuplApn);
            if (null != mNetworkCallback) {
                if (DEBUG) log("remove net callback");
                mConnManager.unregisterNetworkCallback(mNetworkCallback);
                mNetworkCallback = null;
                removeMessages(CMD_NET_TIMEOUT, AgpsNetReq.this);
                //removeMessages(CMD_RELEASE_NET, this);
            }
            mIsQueried = true;
            mNetReq = null;
            mNet = null;
            mFqdn = null;
        }
    }
}

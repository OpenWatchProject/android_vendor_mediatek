/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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


package com.mediatek.wfo.impl;

import android.content.Context;

import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.system.ErrnoException;
import android.system.Os;
import android.system.OsConstants;
import android.system.StructPollfd;
import android.system.StructTimeval;
import android.util.Log;

import static android.system.OsConstants.*;

import java.io.BufferedWriter;
import java.io.Closeable;
import java.io.File;
import java.io.FileDescriptor;
import java.io.IOException;
import java.io.InterruptedIOException;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

import libcore.io.IoUtils;

public class PingDns {

    private static final String TAG = "PingDns";
    private static final boolean DBG = true;
    private static final boolean VDBG = true;

    // Socket Timeout value for ICMP socket
    private final int PING_TIME_INTERVAL = 100;
    private final int SOCKET_TIMEOUT_MS = 100;
    private final int TIMEOUT_SEND = 100;
    private final int TIMEOUT_RECV = 100;
    private final int MAX_PING_COUNT = 5;
    private final int PACKET_BUFSIZE = 40; // 32 BYTE PAYLOAD + 8 BYTE HEADER
    private final int PORT = 9999;
    //private static final int EVENT_SET_WIFI_PING_RESULT = 151;
    private static final String STATIC_DNS_IPV6 = "2405:200:800:1";
    private static final String STATIC_DNS_IPV4 = "49.45.0.1";

    private double mPingLatency;
    private int mPacketLost;
    //private String mServerInterface = null;
    private Context mContext;
    private Handler mHandler;
    private ConnectivityManager mConnectivityManager;
    private static Integer mInterfaceIndex;
    private int mSimId;
    private int mRat;
    private LinkProperties mWifiLinkProp;
    private InetAddress mDnsAddressIpv6;
    private InetAddress mDnsAddressIpv4;

    public PingDns(Context context, int simId, int rat, Handler hdlr) {
        Log.i(TAG, "PingDns Construct for sim" + simId);
        mContext = context;
        mHandler  = hdlr;
        mRat = rat;
        mSimId = simId;
        mConnectivityManager = (ConnectivityManager)
                context.getSystemService(Context.CONNECTIVITY_SERVICE);
        mPingLatency = 0.0;
        mPacketLost = 0;
        mWifiLinkProp = null;
        mDnsAddressIpv6 = null;
        mDnsAddressIpv4 = null;
    }

    public double getPingLatency() {
        debugInfo("mPingLatency:" + mPingLatency);
        return mPingLatency;
    }

    public int getPacketLoss() {
        debugInfo("mPacketLost:" + mPacketLost);
        return mPacketLost;
    }

    public void pingWifiGateway() {
        debugInfo("pingWifiGateway");
        mWifiLinkProp = getActiveLinkProp();
        if (mWifiLinkProp == null) {
            debugInfo("No Active network");
            return;
        }
        mInterfaceIndex = getInterfaceIndex(mWifiLinkProp.getInterfaceName());
        new NetworkTask().execute();
    }

    private static Integer getInterfaceIndex(String ifname) {
        try {
            NetworkInterface ni = NetworkInterface.getByName(ifname);
            return ni.getIndex();
        } catch (NullPointerException | SocketException e) {
            return null;
        }
    }

    private InetAddress getStaticDnsServer() {
        debugInfo("getStaticDnsServer");
        InetAddress staticDnsAddr = null;
        try {
            debugInfo("Fallback to static IPV6 = " + STATIC_DNS_IPV6);
            try {
                staticDnsAddr = InetAddress.getByName(STATIC_DNS_IPV6);
            } catch (UnknownHostException e) {
                debugInfo("Unable to resolve IPV6 host, Fallback to static IPV4 = "
                        + STATIC_DNS_IPV4);
                staticDnsAddr = InetAddress.getByName(STATIC_DNS_IPV4);
            }
        } catch (Exception e) {
            debugInfo("Exception: " + e);
            e.printStackTrace();
        }
        return staticDnsAddr;
    }

    private InetAddress getImsDnsServer() {
        InetAddress dnsAddress = null;
        debugInfo("getImsDnsServer");
        NetworkInfo nwInfo = mConnectivityManager.getActiveNetworkInfo();
        if (nwInfo == null) {
            debugInfo("no active nwInfo");
            return null;
        }
        debugInfo("nwInfo =" + nwInfo + ", nwInfo.getType = " + nwInfo.getType());

        for (Network network : mConnectivityManager.getAllNetworks()) {
            debugInfo("Checking network:" + network);
            NetworkCapabilities nc = mConnectivityManager.getNetworkCapabilities(network);
            debugInfo("nc = " + nc + ", nwInfo = " + nwInfo);
            if (nc != null && nc.hasCapability(NetworkCapabilities.NET_CAPABILITY_IMS)) {
                LinkProperties imsLinkProp = mConnectivityManager.getLinkProperties(network);

                if (imsLinkProp == null) {
                    debugInfo("imsLinkProp null");
                    break;
                }
                debugInfo("imsLinkProp: " + imsLinkProp.toString());
                for (InetAddress address : imsLinkProp.getDnsServers()) {
                    if ((address instanceof Inet6Address || address instanceof Inet4Address)
                          && !address.isLoopbackAddress()) {
                        mDnsAddressIpv6 = address instanceof Inet6Address ? address :
                                mDnsAddressIpv6;
                        mDnsAddressIpv4 = address instanceof Inet4Address ? address :
                                mDnsAddressIpv4;
                    }
                }
                debugInfo("mDnsAddressIpv6 = " + mDnsAddressIpv6
                            + ", mDnsAddressIpv4 = " + mDnsAddressIpv4);
                break;
            } else {
                debugInfo("No network found with IMS capability");
            }
        }

        dnsAddress = mDnsAddressIpv4;
        if (mDnsAddressIpv4 == null) {
            if (mDnsAddressIpv6 == null) {
                dnsAddress = getStaticDnsServer();
            } else {
                dnsAddress = mDnsAddressIpv6;
            }
        }
        debugInfo("dnsAddress = " + dnsAddress);
        return dnsAddress;
    }

    private class NetworkTask extends AsyncTask<Void, Void, Object> {

        @Override
        protected Object doInBackground(Void... arg0) {
            debugInfo("doInBackground");
            InetAddress dnsAddress = getImsDnsServer();
            if (dnsAddress == null) {
                debugInfo("Can't Process ping request");
                return null;
            }

            try {
                runIcmpCheck(dnsAddress);
            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }
    }

    private LinkProperties getActiveLinkProp() {
        Network network = mConnectivityManager.getActiveNetwork();
        if (network == null) {
            return null;
        }
        NetworkCapabilities nc = mConnectivityManager.getNetworkCapabilities(network);

        if (nc == null || !nc.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)) {
            return null;
        }
        LinkProperties link = mConnectivityManager.getLinkProperties(network);
        debugInfo("getActiveLinkProp: " + link);
        return link;
    }

    private void runIcmpCheck(InetAddress addr) {
        IcmpCheck icmpCheck = new IcmpCheck(addr);
        Thread t = new Thread(icmpCheck);
        t.setName(IcmpCheck.class.getName());
        t.start();
    }

     private class SimpleSocketCheck implements Closeable {
        protected final InetAddress mTarget;
        protected final int mAddressFamily;
        protected FileDescriptor mFileDescriptor;
        protected SocketAddress mSocketAddress = null;

        protected SimpleSocketCheck(InetAddress target) {
            if (target instanceof Inet6Address) {
                Inet6Address targetWithScopeId = null;
                if (target.isLinkLocalAddress() && mInterfaceIndex != null) {
                    try {
                        targetWithScopeId = Inet6Address.getByAddress(
                                null, target.getAddress(), mInterfaceIndex);
                    } catch (UnknownHostException e) {
                        errorInfo("err in SimpleSocketCheck:" + e.toString());
                    }
                }
                mTarget = (targetWithScopeId != null) ? targetWithScopeId : target;
                mAddressFamily = AF_INET6;
            } else {
                mTarget = target;
                mAddressFamily = AF_INET;
            }
            debugInfo("mAddressFamily : " + mAddressFamily);
        }

        protected void setupSocket(
                int sockType, int protocol, long writeTimeout, long readTimeout, int dstPort)
                throws ErrnoException, IOException {

            debugInfo("setupSocket: sockType = " + sockType + ", protocol =" + protocol +
                    ", mAddressFamily" + mAddressFamily);
            mFileDescriptor = Os.socket(mAddressFamily, sockType, protocol);
            // Setting SNDTIMEO is purely for defensive purposes.
            Os.setsockoptTimeval(mFileDescriptor,
                    SOL_SOCKET, SO_SNDTIMEO, StructTimeval.fromMillis(writeTimeout));
            Os.setsockoptTimeval(mFileDescriptor,
                    SOL_SOCKET, SO_RCVTIMEO, StructTimeval.fromMillis(readTimeout));
            // Find out what the kernel set the ICMP ID to.
            InetSocketAddress local = (InetSocketAddress) Os.getsockname(mFileDescriptor);
            debugInfo("setupSocket: sockname = " + local.toString() +
                    ", PORT = " + dstPort + ", fd = " + mFileDescriptor);
            Os.connect(mFileDescriptor, mTarget, dstPort);
            debugInfo("socket connect succssfully");
        }

        protected String getSocketAddressString() {
            // The default toString() implementation is not the prettiest.
            String socAddr = "null";
            if (mSocketAddress != null) {
                InetSocketAddress inetSockAddr = (InetSocketAddress) mSocketAddress;
                InetAddress localAddr = inetSockAddr.getAddress();
                return String.format(
                        (localAddr instanceof Inet6Address ? "[%s]:%d" : "%s:%d"),
                        localAddr.getHostAddress(), inetSockAddr.getPort());
            } else {
                return socAddr;
            }
        }

        @Override
        public void close() {
            IoUtils.closeQuietly(mFileDescriptor);
        }
    }

    private class IcmpCheck extends SimpleSocketCheck implements Runnable {
        private static final int ICMPV4_ECHO_REQUEST = 8;
        private static final int ICMPV6_ECHO_REQUEST = 128;
        private static final int MAX_BUFFER_LOGS = 100;

        private final int mProtocol;
        private final int mIcmpType;

        public IcmpCheck(InetAddress target) {
            super(target);

            //mType = type;
            if (mAddressFamily == AF_INET6) {
                mProtocol = IPPROTO_ICMPV6;
                mIcmpType = ICMPV6_ECHO_REQUEST;
            } else {
                mProtocol = IPPROTO_ICMP;
                mIcmpType = ICMPV4_ECHO_REQUEST;
            }
            debugInfo("IcmpCheck:" + mAddressFamily
                            + ": dst{" + mTarget.getHostAddress() + "}" +
                            ", mProtocol:" + mProtocol);
        }

        private void retrySetupSocket() {
            try {
                if(DBG) debugInfo("Reconnect setup");
                setupSocket(SOCK_DGRAM, mProtocol, TIMEOUT_SEND, TIMEOUT_RECV, PORT + mSimId);
            } catch (ErrnoException | IOException e) {
                errorInfo("icmp setupSocket:" + e.toString() + ":" + mTarget);
            }
        }

        @Override
        public void run() {
            long startTime = 0;
            long elapsedTime = 0;
            long ttlTime;
            File logFile = null;
            BufferedWriter out = null;
            byte[] icmpPacket = new byte[PACKET_BUFSIZE];
            int count = 0;
            ByteBuffer buffer = null;
             byte[] reply = new byte[PACKET_BUFSIZE];
            String msg = null;
            double ttl = 0.0;
            int retryCount = 0;
            boolean socketFlag = false;
            double pingLatency = 0.0;
            int packetLost = 0;

            debugInfo("Run IcmpCheck");

            while (retryCount < 5 && socketFlag == false) {
                try {
                    setupSocket(SOCK_DGRAM, mProtocol, TIMEOUT_SEND, TIMEOUT_RECV, PORT + mSimId);
                    socketFlag = true;
                } catch (ErrnoException | IOException e) {
                    errorInfo("icmp setupSocket:" + e.toString() + ":" + mTarget);
                    if (retryCount < 5) {
                        try {
                            Thread.sleep(1000); // 1 sec
                        } catch (Exception ex1) {
                            errorInfo("Sleep Operation failed");
                        }
                        retryCount++;
                        errorInfo("Retry: icmp setupSocket: retryCount = " + retryCount);
                    } else {
                        return;
                    }
                }
            }

            // Build a trivial ICMP packet.
            final byte[] icmpHeader = {
                    (byte) mIcmpType, 0, 0, 0, 0, 0, 0, 0  // ICMP header
            };
            System.arraycopy(icmpHeader, 0, icmpPacket, 0, icmpHeader.length);

            try {
                debugInfo("Ping " + mTarget);
                int writeFail = 0;
                int readFail = 0;
                int ret = 0;
                while(count++ < MAX_PING_COUNT) {
                    buffer = ByteBuffer.wrap(icmpPacket);
                    buffer.putShort(6, (short) count); // Sequence no. is put on byte 6 in header

                    // SEND PING REQUEST
                    try {
                        startTime = now();
                        //if (VDBG) { debugInfo("write[1]:" + count); }
                        debugInfo("Send request icmp.seq" + count);
                        ret = Os.write(mFileDescriptor, icmpPacket, 0, icmpPacket.length);
                        //if (VDBG) { debugInfo("write[2]:"+ ret); }
                    } catch (ErrnoException | InterruptedIOException e) {
                        errorInfo("icmp :" + e.toString());
                        ret = -1;
                    }

                    if (ret == 0 || ret == -1) {
                        if (writeFail < 5) {
                            writeFail++;
                            continue;
                        } else {
                            debugInfo("Write Failed five times");
                            break;
                        }
                    }

                    // EXPECT A REPLY
                    StructPollfd pollfd = new StructPollfd();
                    pollfd.events = (short) POLLIN;  // "error: possible loss of precision"
                    pollfd.fd = mFileDescriptor;
                    ret = 0;
                    try {
                        ret = Os.poll(new StructPollfd[] { pollfd }, SOCKET_TIMEOUT_MS);
                    } catch (ErrnoException e) {
                        errorInfo("Os.poll error:" + e.toString());
                    }
                    if (ret == 0) {
                        errorInfo("Reply timeout: for icmp.seq " + count + ", ret" + ret);
                        packetLost++;
                        pingLatency += SOCKET_TIMEOUT_MS;
                    } else if (ret == -1) {
                        errorInfo("Reply error: for icmp.seq " + count + ", ret" + ret);
                        packetLost++;
                        pingLatency += SOCKET_TIMEOUT_MS;
                    } else {
                        // RECEIVE PING REPLY
                        try {
                            int readLen = Os.read(mFileDescriptor, reply, 0, PACKET_BUFSIZE);
                            assertEquals(PACKET_BUFSIZE, readLen);
                            if (readLen <= 0) {
                                msg = "icmp.seq=" + count + " no response: packet lost";
                                packetLost++;
                                pingLatency += SOCKET_TIMEOUT_MS;
                                if (DBG) { debugInfo(msg); }
                                if (VDBG) { errorInfo("Empty reply:" + count); }
                                //count++;
                                continue;
                            }
                            elapsedTime = now();
                            ttlTime = elapsedTime - startTime;
                            ttl = ttlTime / 1000000;
                            pingLatency += ttl;
                            msg = "Receive reply icmp.seq=" + count + " TTL:" + ttl + "ms";
                            if (DBG) debugInfo(msg);
                        } catch (ErrnoException e) {
                            errorInfo("replyerror icmp.seq=" + count + ", error: " + e.toString());
                            if (readFail < 5) {
                                readFail++;
                                continue;
                            } else {
                                debugInfo("Read Failed five times");
                                break;
                            }
                        }
                    }
                    //count++;
                    Thread.sleep(PING_TIME_INTERVAL);
                }
                debugInfo("pingResult: writeFail : " + writeFail + ", readFail : " + readFail);
                if (writeFail > 0 || readFail > 0) {
                    packetLost += writeFail + readFail;
                    pingLatency  += packetLost * PING_TIME_INTERVAL;
                }

                pingLatency = pingLatency / MAX_PING_COUNT;
                packetLost = (packetLost * 100) / MAX_PING_COUNT;
                debugInfo("pingResult: mPingLatency = " + pingLatency + ", no. of packet lost = "
                        + packetLost);
            } catch (Exception e) {
                errorInfo("icmp :" + e.toString());
            }
            close();
            debugInfo("End of IcmpCheck");
            Message message = mHandler.obtainMessage(WifiPdnHandler.EVENT_SET_WIFI_PING_RESULT,
                    mSimId, mRat, new PingData(mSimId, pingLatency, packetLost));
            mPingLatency = pingLatency;
            mPacketLost = packetLost;
            mHandler.sendMessage(message);
        }
    }

    // For brevity elsewhere.
    private static final long now() {
        return SystemClock.elapsedRealtimeNanos();
    }

    void assertEquals(int e, int v) {
        if (e != v) {
            debugInfo("Assertion failed! Expected: <" + e + "> Got: <" + v + ">");
        }
    }

    private void debugInfo(String info) {
        Log.i(TAG, "[" + mSimId + "]: " + info);
    }

    private void errorInfo(String info) {
        Log.e(TAG, "[" + mSimId + "]: " + info);
    }
}

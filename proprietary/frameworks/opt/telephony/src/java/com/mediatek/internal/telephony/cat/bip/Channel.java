/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.Inet4Address;
import java.util.Arrays;

import android.net.Network;
import android.os.SystemProperties;

import com.android.internal.telephony.cat.CatService;
import com.android.internal.telephony.cat.CatCmdMessage;
import com.android.internal.telephony.cat.CommandDetails;
import com.android.internal.telephony.cat.CommandParams;

abstract class Channel {
    protected int mChannelId = -1;
    protected int mChannelStatus = BipUtils.CHANNEL_STATUS_UNKNOWN;
    protected int mLinkMode = 0;
    protected int mProtocolType = BipUtils.TRANSPORT_PROTOCOL_UNKNOWN;

    protected InetAddress mAddress = null;
    protected int mPort = 0;
    private MtkCatService mHandler = null;
    protected BipService mBipService = null;
    protected BipChannelManager mBipChannelManager = null;
    protected int mBufferSize = 0;
    protected byte[] mRxBuffer = null;
    protected byte[] mTxBuffer = null;
    protected int mRxBufferCount = 0;
    protected int mRxBufferOffset = 0;
    protected int mTxBufferCount = 0;
    protected int mRxBufferCacheCount = 0;
    protected ReceiveDataResult mRecvDataRet = null;
    protected int needCopy = 0;
    protected boolean isChannelOpened = false;
    protected static final int SOCKET_TIMEOUT = 3000;
    protected ChannelStatus mChannelStatusData = null;
    protected Object mLock;
    protected boolean isReceiveDataTRSent = false;
    protected Network mNetwork = null;

    Channel(int cid, int linkMode, int protocolType, InetAddress address, int port, int bufferSize,
            MtkCatService handler, BipService bipManager) {
        this.mChannelId = cid;
        this.mLinkMode = linkMode;
        this.mProtocolType = protocolType;
        this.mAddress = address;
        this.mPort = port;
        this.mBufferSize = bufferSize;
        this.mLock = new Object();
        this.mHandler = handler;
        this.mBipService = bipManager;
        this.mBipChannelManager = this.mBipService.getBipChannelManager();
        this.mChannelStatusData = new ChannelStatus(cid, ChannelStatus.CHANNEL_STATUS_NO_LINK,
                                                ChannelStatus.CHANNEL_STATUS_INFO_NO_FURTHER_INFO);
    }

    abstract public int openChannel(BipCmdMessage cmdMsg, Network network);

    abstract public int closeChannel();

    abstract public ReceiveDataResult receiveData(int requestSize);

    abstract public int receiveData(int requestSize, ReceiveDataResult rdr);

    abstract public int sendData(byte[] data, int mode);

    abstract public int getTxAvailBufferSize();

    public void dataAvailable(int bufferSize) {
        if (null == mBipService.mCurrentSetupEventCmd) {
            MtkCatLog.e(this, "mCurrentSetupEventCmd is null");
            return;
        }
        if (!mBipService.hasPsEvent(BipUtils.DATA_AVAILABLE_EVENT)) {
            MtkCatLog.d(this, "No need to send data available.");
            return;
        }
        //ToDo: get SetEventListParams and CatCmdMessage by BipService.
        MtkCatResponseMessage resMsg = new MtkCatResponseMessage(MtkCatCmdMessage.getCmdMsg(),
                BipUtils.DATA_AVAILABLE_EVENT);
        byte[] additionalInfo = new byte[7];
        additionalInfo[0] = (byte) 0xB8; // Channel status
        additionalInfo[1] = 0x02;
        additionalInfo[2] = (byte) (getChannelId() | mChannelStatusData.mChannelStatus);
        additionalInfo[3] = ChannelStatus.CHANNEL_STATUS_INFO_NO_FURTHER_INFO;

        additionalInfo[4] = (byte) 0xB7; // Channel data length
        additionalInfo[5] = 0x01;

        if (bufferSize > 0xff) {
            additionalInfo[6] = (byte) 0xff;
        } else {
            additionalInfo[6] = (byte) bufferSize;
        }

        resMsg.setSourceId(0x82);
        resMsg.setDestinationId(0x81);
        //resMsg.setResultCode(ResultCode.OK);
        resMsg.setEventDownload(BipUtils.DATA_AVAILABLE_EVENT, additionalInfo);
        resMsg.setAdditionalInfo(additionalInfo);
        resMsg.setOneShot(false);
        MtkCatLog.d(this, "onEventDownload for dataAvailable");
        mHandler.onEventDownload(resMsg);
        //mHandler.onCmdResponse(resMsg);
    }

    public void changeChannelStatus(byte status) {
        if (null == mBipService.mCurrentSetupEventCmd) {
            MtkCatLog.e(this, "mCurrentSetupEventCmd is null");
            return;
        }
        if (!mBipService.hasPsEvent(BipUtils.CHANNEL_STATUS_EVENT)) {
            MtkCatLog.d(this, "No need to send channel status.");
            return;
        }
        //ToDo: get SetEventListParams and CatCmdMessage by BipService.
        MtkCatResponseMessage resMsg = new MtkCatResponseMessage(
                mBipService.mCurrentSetupEventCmd, BipUtils.CHANNEL_STATUS_EVENT);

        MtkCatLog.d("[BIP]", "[Channel]:changeChannelStatus:" + status);

        byte[] additionalInfo = new byte[4];
        additionalInfo[0] = (byte) 0xB8; // Channel status
        additionalInfo[1] = 0x02;
        additionalInfo[2] = (byte) (getChannelId() | status);
        additionalInfo[3] = ChannelStatus.CHANNEL_STATUS_INFO_NO_FURTHER_INFO;

        resMsg.setSourceId(0x82);
        resMsg.setDestinationId(0x81);
        //resMsg.setResultCode(ResultCode.OK);
        resMsg.setEventDownload(BipUtils.CHANNEL_STATUS_EVENT, additionalInfo);
        resMsg.setAdditionalInfo(additionalInfo);
        resMsg.setOneShot(false);
        mHandler.onEventDownload(resMsg);
        //mHandler.onCmdResponse(resMsg);
    }

    public int getChannelStatus() {
        return mChannelStatus;
    }

    public int getChannelId() {
        return mChannelId;
    }

    public void clearChannelBuffer(boolean resetBuffer) {
        if (true == resetBuffer) {
            Arrays.fill(mRxBuffer, (byte) 0);
            Arrays.fill(mTxBuffer, (byte) 0);
        } else {
            mRxBuffer = null;
            mTxBuffer = null;
        }
        mRxBufferCount = 0;
        mRxBufferOffset = 0;
        mTxBufferCount = 0;
    }
    protected int checkBufferSize() {
        int minBufferSize = 0;
        int maxBufferSize = 0;
        int defaultBufferSize = 0;

        if (mProtocolType == BipUtils.TRANSPORT_PROTOCOL_TCP_LOCAL
            || mProtocolType == BipUtils.TRANSPORT_PROTOCOL_TCP_REMOTE
            || mProtocolType == BipUtils.TRANSPORT_PROTOCOL_SERVER) {
            minBufferSize = BipUtils.MIN_BUFFERSIZE_TCP;
            maxBufferSize = BipUtils.MAX_BUFFERSIZE_TCP;
            defaultBufferSize = BipUtils.DEFAULT_BUFFERSIZE_TCP;
        } else if (mProtocolType == BipUtils.TRANSPORT_PROTOCOL_UDP_LOCAL
                   || mProtocolType == BipUtils.TRANSPORT_PROTOCOL_UDP_REMOTE) {
            minBufferSize = BipUtils.MIN_BUFFERSIZE_UDP;
            maxBufferSize = BipUtils.MAX_BUFFERSIZE_UDP;
            defaultBufferSize = BipUtils.DEFAULT_BUFFERSIZE_UDP;
        }

        MtkCatLog.d("[BIP]", "mBufferSize:" + mBufferSize + " minBufferSize:" + minBufferSize +
                 " maxBufferSize:" + maxBufferSize);

        if (mBufferSize >= minBufferSize && mBufferSize <= maxBufferSize) {
            MtkCatLog.d("[BIP]", "buffer size is normal");
            return ErrorValue.NO_ERROR;
        } else {
            if (mBufferSize > maxBufferSize) {
                MtkCatLog.d("[BIP]", "buffer size is too large, change it to maximum value");
                mBufferSize = maxBufferSize;
            } else {
                MtkCatLog.d("[BIP]", "buffer size is too small, change it to default value");
                mBufferSize = defaultBufferSize;
            }
        }

        if (mBufferSize < BipUtils.MAX_APDU_SIZE) {
            MtkCatLog.d("[BIP]", "buffer size is smaller than 255, change it to MAX_APDU_SIZE");
            mBufferSize = BipUtils.MAX_APDU_SIZE;
        }

        return ErrorValue.COMMAND_PERFORMED_WITH_MODIFICATION;
    }

    private volatile boolean mStop = false;

    protected synchronized void requestStop()
    {
        mStop = true;
        MtkCatLog.d("[BIP]", "requestStop: " + mStop);
    }

    protected class UdpReceiverThread implements Runnable {
        DatagramSocket udpSocket;

        UdpReceiverThread(DatagramSocket s) {
            udpSocket = s;
        }

        @Override
        public void run() {
            byte[] localBuffer = new byte[BipUtils.MAX_BUFFERSIZE_UDP];

            MtkCatLog.d("[BIP]", "[UDP]RecTr run");
            DatagramPacket recvPacket = new DatagramPacket(localBuffer, localBuffer.length);
            try {
                while (!mStop) {
                    int recvLen = 0;
                    MtkCatLog.d("[BIP]", "[UDP]RecTr: Wait data from network");
                    try {
                        Arrays.fill(localBuffer, (byte) 0);
                        udpSocket.receive(recvPacket);
                        recvLen = recvPacket.getLength();
                    } catch (IOException e) {
                        MtkCatLog.e("[BIP]", "[UDP]RecTr:read io exception.");
                        Arrays.fill(localBuffer, (byte) 0);
                        mChannelStatusData.mChannelStatus = ChannelStatus.CHANNEL_STATUS_NO_LINK;
                        clearChannelBuffer(false);
                        break;
                    }
                    MtkCatLog.d("[BIP]", "[UDP]RecTr: recvLen:" + recvLen);
                    if (recvLen >= 0) {
                        synchronized (mLock) {
                            MtkCatLog.d("[BIP]", "[UDP]RecTr: mRxBufferCount:" + mRxBufferCount);
                            if (mRxBufferCount == 0) {
                                if ((recvLen > mBufferSize) &&
                                            (mBufferSize < BipUtils.DEFAULT_BUFFERSIZE_UDP)) {
                                        mRxBuffer = new byte[BipUtils.DEFAULT_BUFFERSIZE_UDP];
                                }
                                System.arraycopy(localBuffer, 0, mRxBuffer, 0, recvLen);
                                mRxBufferCount = recvLen;
                                mRxBufferOffset = 0;
                                dataAvailable(mRxBufferCount);
                                // Wait until the data in mRxBuffer are
                                // received done by SIM
                                try {
                                    mLock.wait();
                                } catch (InterruptedException e) {
                                    MtkCatLog.e("[BIP]", "[UDP]RecTr: InterruptedException !!!");
                                    e.printStackTrace();
                                }
                            } else if (mRxBufferCount > 0) {
                                // This means the mRxBuffer still has data.
                                // Use Data Available envelope command to notify SIM
                                do {
                                    dataAvailable(mRxBufferCount);
                                    try {
                                        mLock.wait();
                                    } catch (InterruptedException e) {
                                        MtkCatLog.e("[BIP]",
                                                "[UDP]RecTr: InterruptedException !!!");
                                        e.printStackTrace();
                                    }
                                } while (mRxBufferCount > 0);

                                // After mRxBuffer is empty, copy data from
                                // localBuffer to mRxBuffer
                                if (recvLen > 0) {
                                    System.arraycopy(localBuffer, 0, mRxBuffer, 0, recvLen);
                                    mRxBufferCount = recvLen;
                                    mRxBufferOffset = 0;
                                    dataAvailable(mRxBufferCount);
                                    try {
                                        mLock.wait();
                                    } catch (InterruptedException e) {
                                        MtkCatLog.e("[BIP]",
                                                "[UDP]RecTr: InterruptedException !!!");
                                        e.printStackTrace();
                                    }
                                }
                            }
                        }
                    } else {
                        MtkCatLog.e("[BIP]", "[UDP]RecTr: end of file or server is disconnected.");
                        break;
                    }
                }
                if (mStop) {
                    MtkCatLog.d("[BIP]", "[UDP]RecTr: stop");
                }
            } catch (Exception e) {
                MtkCatLog.d("[BIP]", "[UDP]RecTr:Error.");
                e.printStackTrace();
            }
        }
    }

    protected class TcpReceiverThread implements Runnable {
        DataInputStream di;

        TcpReceiverThread(DataInputStream s) {
            di = s;
        }

        @Override
        public void run() {
            byte[] localBuffer = new byte[BipUtils.MAX_BUFFERSIZE_TCP];

            MtkCatLog.d("[BIP]", "[TCP]RecTr: run");

            try {
                while (!mStop) {
                    int recvLen = 0;
                    MtkCatLog.d("[BIP]", "[TCP]RecTr: Wait data from network");
                    try {
                        Arrays.fill(localBuffer, (byte) 0);
                        recvLen = di.read(localBuffer);
                    } catch (IOException e) {
                        MtkCatLog.e("[BIP]", "[TCP]RecTr:read io exception.");
                        Arrays.fill(localBuffer, (byte) 0);
                        clearChannelBuffer(false);
                        break;
                    }
                    MtkCatLog.d("[BIP]", "[TCP]RecTr: recvLen:" + recvLen);
                    if (recvLen >= 0) {
                        synchronized (mLock) {
                            MtkCatLog.d("[BIP]", "[TCP]RecTr: mRxBufferCount:" + mRxBufferCount);
                            if (mRxBufferCount == 0) {
                                if ((recvLen > mBufferSize)
                                        && (mBufferSize < BipUtils.DEFAULT_BUFFERSIZE_TCP)) {
                                    mRxBuffer = new byte[BipUtils.DEFAULT_BUFFERSIZE_TCP];
                                }
                                System.arraycopy(localBuffer, 0, mRxBuffer, 0, recvLen);
                                mRxBufferCount = recvLen;
                                mRxBufferOffset = 0;
                                dataAvailable(mRxBufferCount);
                                // Wait until the data in mRxBuffer are
                                // received done by SIM
                                try {
                                    mLock.wait();
                                } catch (InterruptedException e) {
                                    MtkCatLog.e("[BIP]", "[TCP]RecTr: InterruptedException !!!");
                                    e.printStackTrace();
                                }
                            } else if (mRxBufferCount > 0) {
                                // This means the mRxBuffer still has data.
                                // Use Data Available envelope command to notify
                                // SIM
                                do {
                                    dataAvailable(mRxBufferCount);
                                    try {
                                        mLock.wait();
                                    } catch (InterruptedException e) {
                                        MtkCatLog
                                                .e("[BIP]", "[TCP]RecTr: InterruptedException !!!");
                                        e.printStackTrace();
                                    }
                                } while (mRxBufferCount > 0);

                                // After mRxBuffer is empty, copy data from
                                // localBuffer to mRxBuffer
                                if (recvLen > 0) {
                                    System.arraycopy(localBuffer, 0, mRxBuffer, 0, recvLen);
                                    mRxBufferCount = recvLen;
                                    mRxBufferOffset = 0;
                                    dataAvailable(mRxBufferCount);
                                    try {
                                        mLock.wait();
                                    } catch (InterruptedException e) {
                                        MtkCatLog
                                                .e("[BIP]", "[TCP]RecTr: InterruptedException !!!");
                                        e.printStackTrace();
                                    }
                                }

                            }
                        }
                    } else {
                        MtkCatLog.e("[BIP]", "[TCP]RecTr: end of file or server is disconnected.");
                        break;
                    }
                }
                if (mStop) {
                    MtkCatLog.d("[BIP]", "[TCP]RecTr: stop");
                }

            } catch (Exception e) {
                MtkCatLog.d("[BIP]", "[TCP]RecTr:Error");
                e.printStackTrace();
            }
        }
    }
    protected class UICCServerThread implements Runnable {
        TcpServerChannel mTcpServerChannel = null;
        int mReTryCount = 0;
        DataInputStream di = null;
        private static final int RETRY_COUNT = 4;
        private static final int RETRY_ACCEPT_SLEEPTIME = 100;

        UICCServerThread(TcpServerChannel tcpServerChannel)
        {
            MtkCatLog.d("[BIP]", "OpenServerSocketThread Init");
            this.mTcpServerChannel = tcpServerChannel;
        }

        @Override
        public void run()
        {
            byte[] localBuffer = new byte[BipUtils.MAX_BUFFERSIZE_TCP];

            MtkCatLog.d("[BIP]", "[UICC]ServerTr: Run Enter");
            for (;; ) {
                if (mChannelStatus == BipUtils.CHANNEL_STATUS_OPEN) {
                    if (mTcpServerChannel.getTcpStatus() != BipUtils.TCP_STATUS_LISTEN) {
                        mTcpServerChannel.setTcpStatus(BipUtils.TCP_STATUS_LISTEN, true);
                    } else {
                        MtkCatLog.d("[BIP]", "[UICC]ServerTr:TCP status = TCP_STATUS_LISTEN");
                    }
                    try {
                        MtkCatLog.d("[BIP]", "[UICC]ServerTr:Listen to wait client connection...");
                        mTcpServerChannel.mSocket = mTcpServerChannel.mSSocket.accept();
                    } catch (IOException e) {
                        MtkCatLog.e("[BIP]", "[UICC]ServerTr:Fail to accept server socket retry:" +
                                 mReTryCount);
                        if (RETRY_COUNT >= mReTryCount) {
                            mReTryCount++;
                            try {
                                Thread.sleep(RETRY_ACCEPT_SLEEPTIME);
                            } catch (InterruptedException ie) {
                                MtkCatLog.e("[BIP]",
                                        "[UICC]ServerTr:IE: sleep for SS accept retry.");
                            }
                            continue;
                        } else {
                            mReTryCount = 0;
                            try {
                                if (null != mTcpServerChannel.mInput) {
                                    mTcpServerChannel.mInput.close();
                                }
                                if (null != mTcpServerChannel.mOutput) {
                                    mTcpServerChannel.mOutput.close();
                                }
                            } catch (IOException e1) {
                                MtkCatLog.e("[BIP]",
                                        "[UICC]ServerTr:IOE: input/output stream close.");
                            }
                            try {
                                mTcpServerChannel.mSSocket.close();
                            } catch (IOException e2) {
                                MtkCatLog.e("[BIP]", "[UICC]ServerTr:IOE: socket close.");
                            }
                            clearChannelBuffer(false);
                            closeChannel();
                            mBipChannelManager.removeChannel(mChannelId);
                            mTcpServerChannel.setTcpStatus(BipUtils.TCP_STATUS_CLOSE, true);
                            break;
                        }
                    }
                    MtkCatLog.d("[BIP]", "[UICC]ServerTr:Receive a client connection.");
                    mTcpServerChannel.setTcpStatus(BipUtils.TCP_STATUS_ESTABLISHED, true);
                    if (null == mTcpServerChannel.mInput) {
                        try {
                            mTcpServerChannel.mInput = new DataInputStream(
                                    mTcpServerChannel.mSocket.getInputStream());
                        } catch (IOException ioe) {
                            MtkCatLog.e("[BIP]", "[UICC]ServerTr:IOException: getInputStream.");
                            continue;
                        }
                        di = mTcpServerChannel.mInput;
                    }
                    if (null == mTcpServerChannel.mOutput) {
                        try {
                            mTcpServerChannel.mOutput = new BufferedOutputStream(
                                    mTcpServerChannel.mSocket.getOutputStream());
                        } catch (IOException ioe) {
                            MtkCatLog.e("[BIP]", "[UICC]ServerTr:IOException: getOutputStream.");
                            continue;
                        }
                    }
                    while (!mStop) {
                        boolean goOnRead = true;
                        int recvLen = 0;
                        MtkCatLog.d("[BIP]", "[UICC]ServerTr: Start to read data from network");
                        try {
                            Arrays.fill(localBuffer, (byte) 0);
                            recvLen = di.read(localBuffer);
                        } catch (IOException e) {
                            MtkCatLog.e("[BIP]", "[UICC]ServerTr:read io exception.");
                            Arrays.fill(localBuffer, (byte) 0);
                            try {
                                if (null != mTcpServerChannel.mInput) {
                                    mTcpServerChannel.mInput.close();
                                }
                                if (null != mTcpServerChannel.mOutput) {
                                    mTcpServerChannel.mOutput.close();
                                }
                                clearChannelBuffer(true);
                            } catch (IOException e1) {
                                MtkCatLog.e("[BIP]", "[UICC]ServerTr:IOException input stream.");
                            }
                            break;
                        }
                        MtkCatLog.d("[BIP]", "[UICC]ServerTr: Receive data:" + recvLen);
                        if (recvLen >= 0) {
                            int rSize = 0;
                            int localBufferOffset = 0;
                            int localBufferCount = 0;
                            synchronized (mLock) {
                                //byte[] tmpBuffer = new byte[mBufferSize];
                                MtkCatLog.d("[BIP]", "[UICC]ServerTr:mRxBufferCount: " +
                                        mRxBufferCount);
                                if (mRxBufferCount == 0) {
                                    System.arraycopy(localBuffer, 0, mRxBuffer, 0, recvLen);
                                    mRxBufferCount = recvLen;
                                    mRxBufferOffset = 0;
                                    dataAvailable(mRxBufferCount);
                                } else {
                                    System.arraycopy(mRxBuffer, mRxBufferOffset, mRxBuffer, 0,
                                            mRxBufferCount); //shift the data to the head of array.

                                    if (recvLen <= (mBufferSize - mRxBufferCount)) {
                                        rSize = recvLen;
                                    } else { //recvLen > (mBufferSize-mRxBufferCount)
                                        localBufferOffset = rSize = mBufferSize - mRxBufferCount;
                                        mRxBufferCacheCount = recvLen - rSize;
                                    }
                                    System.arraycopy(localBuffer, 0, mRxBuffer, mRxBufferCount,
                                                     rSize);

                                    mRxBufferCount += rSize; //recvLen + mRxBufferCount;
                                    mRxBufferOffset = 0;
                                    MtkCatLog.d("[BIP]", "[UICC]ServerTr:rSize: " + rSize +
                                            ", mRxBufferCacheCount: " + mRxBufferCacheCount);
                                }
                                do {
                                    if (mRxBufferCount >= mBufferSize) {
                                        try {
                                            MtkCatLog.d("[BIP]",
                                                    "[UICC]ServerTr:mRxBuffer is full.");
                                            mLock.wait();
                                        } catch (InterruptedException e) {
                                            MtkCatLog.e("[BIP]", "[UICC]ServerTr:IE :" +
                                                     "mRxBufferCount >= mBufferSize");
                                            if (true ==
                                                    mTcpServerChannel.isCloseBackToTcpListen()) {
                                                clearChannelBuffer(true);
                                                mTcpServerChannel.setCloseBackToTcpListen(false);
                                                goOnRead = false;
                                                break;
                                            }
                                        }
                                        if (0 < mRxBufferCacheCount) {
                                            if (0 < mRxBufferCount) {
                                                System.arraycopy(mRxBuffer, mRxBufferOffset,
                                                                 mRxBuffer, 0, mRxBufferCount);
                                            }
                                            if (mRxBufferCacheCount <=
                                                    (mBufferSize - mRxBufferCount)) {
                                                rSize = mRxBufferCacheCount;
                                            } else {
                                                rSize = mBufferSize - mRxBufferCount;
                                            }
                                            System.arraycopy(localBuffer, localBufferOffset,
                                                             mRxBuffer, mRxBufferCount, rSize);
                                            mRxBufferCount += rSize;
                                            mRxBufferCacheCount -= rSize;
                                            localBufferOffset += rSize;
                                            mRxBufferOffset = 0;
                                        }
                                    } else {
                                        goOnRead = true;
                                        break;
                                    }
                                } while(true);
                            }
                        } else {
                            MtkCatLog.e("[BIP]", "[UICC]ServerTr: client diconnected");
                            try {
                                if (null != mTcpServerChannel.mInput) {
                                    mTcpServerChannel.mInput.close();
                                }
                                if (null != mTcpServerChannel.mOutput) {
                                    mTcpServerChannel.mOutput.close();
                                }
                            } catch (IOException e) {
                                MtkCatLog.e("[BIP]",
                                        "[UICC]ServerTr:len<0,IOException input stream.");
                            }
                            clearChannelBuffer(true);
                            mTcpServerChannel.setTcpStatus(BipUtils.TCP_STATUS_LISTEN, true);
                            break;
                        }
                        if (false == goOnRead) {
                            break;
                        }
                        MtkCatLog.d("[BIP]", "[UICC]ServerTr: buffer data:" + mRxBufferCount);
                    }
                    if (mStop) {
                        MtkCatLog.d("[BIP]", "[UICC]ServerTr: stop");
                    }
                }
            }
        }
    }
}

class TcpServerChannel extends Channel {
    protected ServerSocket mSSocket = null;
    protected Socket mSocket = null;
    protected DataInputStream mInput = null;
    protected BufferedOutputStream mOutput = null;
    private Thread rt = null;
    private boolean mCloseBackToTcpListen = false;

    TcpServerChannel(int cid, int linkMode, int protocolType, int port, int bufferSize,
                     MtkCatService handler, BipService bipManager) {
        super(cid, linkMode, protocolType, null, port, bufferSize, handler, bipManager);
    }

    public int openChannel(BipCmdMessage cmdMsg, Network network) {
        int ret = ErrorValue.NO_ERROR;
        mNetwork = network;

        MtkCatLog.d("[BIP]", "[UICC]openChannel mLinkMode:" + mLinkMode);
        //if (mLinkMode == BipUtils.LINK_ESTABLISHMENT_MODE_IMMEDIATE) {
        try {
            MtkCatLog.d("[BIP]", "[UICC]New server socket.mChannelStatus:" + mChannelStatus +
                     ",port:" + mPort);
            mSSocket = new ServerSocket(mPort, 0, Inet4Address.LOOPBACK);
        } catch (IOException e) {
            MtkCatLog.d("[BIP]", "[UICC]IOEX to create server socket");
            return ErrorValue.BIP_ERROR;
        } catch (Exception e2) {
            MtkCatLog.d("[BIP]", "[UICC]EX to create server socket " + e2);
            return ErrorValue.BIP_ERROR;
        }
        if (mChannelStatus == BipUtils.CHANNEL_STATUS_UNKNOWN ||
            mChannelStatus == BipUtils.CHANNEL_STATUS_CLOSE) {
            setTcpStatus(BipUtils.TCP_STATUS_LISTEN, false);
            mChannelStatus = BipUtils.CHANNEL_STATUS_OPEN;
            rt = new Thread(new UICCServerThread(this));
            rt.start();
        }
        ret = checkBufferSize();
        if (ret == ErrorValue.COMMAND_PERFORMED_WITH_MODIFICATION) {
            MtkCatLog.d("[BIP]", "[UICC]openChannel: buffer size is modified");
            cmdMsg.mBufferSize = mBufferSize;
        }
        cmdMsg.mChannelStatusData.mChannelStatus = getTcpStatus();
        mRxBuffer = new byte[mBufferSize];
        mTxBuffer = new byte[mBufferSize];
        //}
        return ret;
    }

    public int closeChannel() {
        int ret = ErrorValue.NO_ERROR;

        MtkCatLog.d("[BIP]", "[UICC]closeChannel.");
        if (true == mCloseBackToTcpListen) {
            if (BipUtils.TCP_STATUS_ESTABLISHED == mChannelStatusData.mChannelStatus) {
                try {
                    mChannelStatusData.mChannelStatus = BipUtils.TCP_STATUS_LISTEN;
                    if (null != mInput) {
                        mInput.close();
                    }
                    if (null != mOutput) {
                        mOutput.close();
                    }
                    if (null != mSocket) {
                        mSocket.close();
                    }
                    rt.interrupt(); //free the block state of mLock.wait()
                } catch (IOException e) {
                    MtkCatLog.e("[BIP]", "[UICC]IOEX closeChannel back to tcp listen.");
                } finally {
                    mSocket = null;
                    mRxBuffer = null;
                    mTxBuffer = null;
                }
            }
        } else {
            if (rt != null) {
                requestStop();
                rt = null;
            }
            try {
                if (null != mInput)
                    mInput.close();
                if (null != mOutput)
                    mOutput.close();
                if (null != mSocket)
                    mSocket.close();
                if (null != mSSocket)
                    mSSocket.close();
            } catch (IOException e) {
                MtkCatLog.e("[BIP]", "[UICC]IOEX closeChannel");
            } finally {
                mSocket = null;
                mRxBuffer = null;
                mTxBuffer = null;
                /* no need to update channel status, since the channel has been removed from
                   channel pool.
                mChannelStatus = BipUtils.CHANNEL_STATUS_CLOSE;
                mChannelStatusData.mChannelStatus = BipUtils.TCP_STATUS_CLOSE;
                */
            }
        }
        return ret;
    }

    public ReceiveDataResult receiveData(int requestCount) {

        ReceiveDataResult ret = new ReceiveDataResult();

        ret.buffer = new byte[requestCount];
        MtkCatLog.d("[BIP]", "[UICC]receiveData " + mRxBufferCount +
                "/" + requestCount + "/" + mRxBufferOffset);

        if (mRxBufferCount >= requestCount) {
            try {
                MtkCatLog.d("[BIP]", "[UICC]Start to copy data from buffer");
                System.arraycopy(mRxBuffer, mRxBufferOffset, ret.buffer, 0, requestCount);
                mRxBufferCount -= requestCount;
                mRxBufferOffset += requestCount;
                ret.remainingCount = mRxBufferCount;
            } catch (IndexOutOfBoundsException e) {
                MtkCatLog.e("[BIP]", "IOOB-1");
            }
        } else {
            int needCopy = requestCount;
            int canCopy = mRxBufferCount;
            int countCopied = 0;
            boolean canExitLoop = false;

            while (!canExitLoop) {
                if (needCopy > canCopy) {
                    try {
                        System.arraycopy(mRxBuffer, mRxBufferOffset, ret.buffer, countCopied,
                                canCopy);
                        mRxBufferOffset += canCopy;
                        mRxBufferCount -= canCopy;
                        countCopied += canCopy;
                        needCopy -= canCopy;
                    } catch (IndexOutOfBoundsException e) {
                        MtkCatLog.e("[BIP]", "IOOB-2");
                    }
                } else {
                    try {
                        System.arraycopy(mRxBufferCount, mRxBufferOffset, ret.buffer, countCopied,
                                canCopy);
                        mRxBufferOffset += needCopy;
                        countCopied += needCopy;
                        needCopy = 0;
                    } catch (IndexOutOfBoundsException e) {
                        MtkCatLog.e("[BIP]", "IOOB-3");
                    }
                }
                if (needCopy == 0) {
                    canExitLoop = true;
                } else {
                    try {
                        int count = mInput.read(mRxBuffer, 0, mRxBuffer.length);
                        mRxBufferCount = count;
                        mRxBufferOffset = 0;
                    } catch (IOException e) {
                        MtkCatLog.e("[BIP]", "IOException");
                        e.printStackTrace();
                    }
                }
            }
        }
        return ret;
    }

    public int sendData(byte[] data, int mode) {
        int ret = ErrorValue.NO_ERROR;
        byte[] tmpBuffer = null;

        if (null == data) {
            MtkCatLog.e("[BIP]", "[UICC]sendData - data null:");
            return ErrorValue.BIP_ERROR;
        }

        if (null == mTxBuffer) {
            MtkCatLog.e("[BIP]", "[UICC]sendData - mTxBuffer null:");
            return ErrorValue.BIP_ERROR;
        }

        int txRemaining = mTxBuffer.length - mTxBufferCount;

        MtkCatLog.d("[BIP]", "[UICC]sendData: size of buffer:" + data.length + " mode:" + mode);
        MtkCatLog.d("[BIP]", "[UICC]sendData: size of buffer:" + mTxBuffer.length + " count:" +
                 mTxBufferCount);

        if (0 == mTxBufferCount && BipUtils.SEND_DATA_MODE_IMMEDIATE == mode) {
            tmpBuffer = data;
            mTxBufferCount = data.length;
        } else {
            try {
                if (txRemaining >= data.length) {
                    System.arraycopy(data, 0, mTxBuffer, mTxBufferCount, data.length);
                    mTxBufferCount += data.length;
                } else {
                    MtkCatLog.d("[BIP]", "[UICC]sendData - tx buffer is not enough");
                }
            } catch (IndexOutOfBoundsException e) {
                return ErrorValue.BIP_ERROR;
            }
            tmpBuffer = mTxBuffer;
        }
        if (mode == BipUtils.SEND_DATA_MODE_IMMEDIATE &&
            mChannelStatus == BipUtils.CHANNEL_STATUS_OPEN &&
            mChannelStatusData.mChannelStatus == BipUtils.TCP_STATUS_ESTABLISHED) {
            try {
                MtkCatLog.d("[BIP]", "S[UICC]END_DATA_MODE_IMMEDIATE:" + mTxBuffer.length +
                         " count:" + mTxBufferCount);
                mOutput.write(tmpBuffer, 0, mTxBufferCount);
                mOutput.flush();
                mTxBufferCount = 0;
            } catch (IOException e) {
                e.printStackTrace();
                return ErrorValue.BIP_ERROR;
            } catch (NullPointerException e2) {
                e2.printStackTrace();
                return ErrorValue.BIP_ERROR;
            }
        }
        return ret;
    }

    public int getTxAvailBufferSize() {
        if (null == mTxBuffer) {
            MtkCatLog.e("[BIP]", "[UICC]getTxAvailBufferSize - mTxBuffer null:");
            return 0;
        }
        int txRemaining = mTxBuffer.length - mTxBufferCount;
        MtkCatLog.d("[BIP]", "[UICC]available tx buffer size:" + txRemaining);
        return txRemaining;
    }

    public int receiveData(int requestSize, ReceiveDataResult rdr) {
        MtkCatLog.d("[BIP]", "[UICC]new receiveData method");
        int ret = ErrorValue.NO_ERROR;

        if (rdr == null) {
            MtkCatLog.d("[BIP]", "[UICC]rdr is null");
            return ErrorValue.BIP_ERROR;
        }

        MtkCatLog.d("[BIP]", "[UICC]receiveData " + mRxBufferCount + "/" + requestSize +
                 "/" + mRxBufferOffset);

        rdr.buffer = new byte[requestSize];
        synchronized (mLock) {
            if (mRxBufferCount >= requestSize) {
                MtkCatLog.d("[BIP]", "[UICC]rx buffer has enough data");
                try {

                    System.arraycopy(mRxBuffer, mRxBufferOffset, rdr.buffer, 0, requestSize);
                    mRxBufferOffset += requestSize;
                    mRxBufferCount -= requestSize;
                    if (mRxBufferCount == 0) {
                        mRxBufferOffset = 0;
                    }
                    rdr.remainingCount = mRxBufferCount + mRxBufferCacheCount;
                    if (mRxBufferCount < mBufferSize) {
                        MtkCatLog.d("[BIP]", ">= [UICC]notify to read data more to mRxBuffer");
                        mLock.notify();
                    }

                } catch (IndexOutOfBoundsException e) {
                    MtkCatLog.d("[BIP]", "[UICC]fail copy rx buffer out 1");
                    return ErrorValue.BIP_ERROR;
                }
                MtkCatLog.d("[BIP]", "[UICC]rx buffer has enough data - end");
            } else {
                MtkCatLog.d("[BIP]", "[UICC]rx buffer is insufficient - being");
                try {

                    System.arraycopy(mRxBuffer, mRxBufferOffset, rdr.buffer, 0, mRxBufferCount);
                    mRxBufferOffset = 0;
                    mRxBufferCount = 0;

                    if (mRxBufferCount < mBufferSize) {
                        MtkCatLog.d("[BIP]", "< [UICC]notify to read data more to mRxBuffer");
                        mLock.notify();
                    }

                    rdr.remainingCount = 0;
                    ret = ErrorValue.MISSING_DATA;
                } catch (IndexOutOfBoundsException e) {
                    MtkCatLog.d("[BIP]", "[UICC]fail copy rx buffer out 2");
                    return ErrorValue.BIP_ERROR;
                }
                MtkCatLog.d("[BIP]", "[UICC]rx buffer is insufficient - end");
            }
        }
        return ret;
    }
    public void setTcpStatus(byte status, boolean isPackED) {
        if (mChannelStatusData.mChannelStatus == status) {
            return;
        } else {
            MtkCatLog.d("[BIP]", "[UICC][TCPStatus]" + mChannelStatusData.mChannelStatus +
                     "->" + status);
            mChannelStatusData.mChannelStatus = status;
            if (true == isPackED) {
                changeChannelStatus(status);
            }
        }
    }
    public byte getTcpStatus() {
        try {
            return (byte) mChannelStatusData.mChannelStatus;
        } catch (NullPointerException ne) {
            MtkCatLog.e("[BIP]", "[TCP]getTcpStatus");
            return 0;
        }
    }

    public void setCloseBackToTcpListen(boolean isBackToTcpListen) {
        mCloseBackToTcpListen = isBackToTcpListen;
    }
    public boolean isCloseBackToTcpListen() {
        return mCloseBackToTcpListen;
    }
}

class TcpChannel extends Channel {
    private static final int TCP_CONN_TIMEOUT = 15000;
    Socket mSocket = null;
    DataInputStream mInput = null;
    BufferedOutputStream mOutput = null;
    Thread rt;

    TcpChannel(int cid, int linkMode, int protocolType, InetAddress address, int port,
               int bufferSize, MtkCatService handler, BipService bipManager) {
        super(cid, linkMode, protocolType, address, port, bufferSize, handler, bipManager);
    }

    public int openChannel(BipCmdMessage cmdMsg, Network network) {
        int ret = ErrorValue.NO_ERROR;
        Thread t_openChannelThread = null;
        mNetwork = network;

        if (mLinkMode == BipUtils.LINK_ESTABLISHMENT_MODE_IMMEDIATE) {
            t_openChannelThread = new Thread(new Runnable() {
                public synchronized void run() {
                    MtkCatLog.d("[BIP]", "[TCP]running TCP channel thread");
                    InetSocketAddress socketAddress = null;
                    try {
                        mSocket = mNetwork.getSocketFactory().createSocket();
                        //mSocket = new Socket();
                        mSocket.setSoLinger(false, 0);
                        socketAddress = new InetSocketAddress(mAddress, mPort);
                        try {
                            mSocket.connect(socketAddress, TCP_CONN_TIMEOUT);
                        } catch (SocketTimeoutException e3) {
                            MtkCatLog.d("[BIP]", "[TCP]Time out of connect " + e3 + ":" +
                                     TCP_CONN_TIMEOUT + " sec");
                            mChannelStatus = BipUtils.CHANNEL_STATUS_ERROR;
                            if (mBipService.mIsOpenChannelOverWifi) {
                                mBipService.mIsConnectTimeout = true;
                                mBipService.mIsOpenChannelOverWifi = false;
                            }
                        }
                        if (mSocket.isConnected()) {
                            mChannelStatus = BipUtils.CHANNEL_STATUS_OPEN;
                            mChannelStatusData.mChannelStatus = ChannelStatus.CHANNEL_STATUS_LINK;
                        } else {
                            MtkCatLog.e("[BIP]", "[TCP]socket is not connected.");
                            mChannelStatus = BipUtils.CHANNEL_STATUS_ERROR;
                            mSocket.close();
                        }
                    } catch (IOException e) {
                        MtkCatLog.e("[BIP]", "[TCP]Fail to create socket");
                        e.printStackTrace();
                        mChannelStatus = BipUtils.CHANNEL_STATUS_ERROR;
                        if (mBipService.mIsOpenChannelOverWifi) {
                            mBipService.mIsConnectTimeout = true;
                            mBipService.mIsOpenChannelOverWifi = false;
                        }
                    } catch (NullPointerException e2) {
                        MtkCatLog.e("[BIP]", "[TCP]Null pointer tcp socket " + e2);
                        mChannelStatus = BipUtils.CHANNEL_STATUS_ERROR;
                    }
                    onOpenChannelCompleted();
                }
            });
            t_openChannelThread.start();
            ret = ErrorValue.WAIT_OPEN_COMPLETED;
        } else if (mLinkMode == BipUtils.LINK_ESTABLISHMENT_MODE_ONDEMMAND) {
            t_openChannelThread = new Thread(new Runnable() {
                public synchronized void run() {
                    MtkCatLog.d("[BIP]", "[TCP]running TCP channel thread");
                    try {
                        mSocket = new Socket();
                        mSocket.setSoLinger(false, 0);
                        mSocket.setSoTimeout(TCP_CONN_TIMEOUT);
                    } catch (SocketException e) {
                        MtkCatLog.d("[BIP]", "[TCP]Fail to create tcp socket");
                        mChannelStatus = BipUtils.CHANNEL_STATUS_ERROR;
                    }
                }
            });
            t_openChannelThread.start();
            mChannelStatus = BipUtils.CHANNEL_STATUS_OPEN;

            ret = checkBufferSize();
            if (ret == ErrorValue.COMMAND_PERFORMED_WITH_MODIFICATION) {
                MtkCatLog.d("[BIP]", "[TCP]openChannel: buffer size is modified");
                cmdMsg.mBufferSize = mBufferSize;
            }
            mRxBuffer = new byte[mBufferSize];
            mTxBuffer = new byte[mBufferSize];
        }
        return ret;
    }
    private void onOpenChannelCompleted() {
        int ret = ErrorValue.NO_ERROR;
        do {
            if (mChannelStatus == BipUtils.CHANNEL_STATUS_OPEN) {
                try {
                    MtkCatLog.d("[BIP]", "[TCP]stream is open");
                    mInput = new DataInputStream(mSocket.getInputStream());
                    mOutput = new BufferedOutputStream(mSocket.getOutputStream());
                    rt = new Thread(new TcpReceiverThread(mInput));
                    rt.start();
                } catch (IOException e) {
                    MtkCatLog.d("[BIP]", "[TCP]Fail to create data stream");
                    e.printStackTrace();
                    ret = ErrorValue.BIP_ERROR;
                    break;
                }
            } else {
                MtkCatLog.d("[BIP]", "[TCP]socket is not open");
                ret = ErrorValue.BIP_ERROR;
                break;
            }
            ret = checkBufferSize();
            mRxBuffer = new byte[mBufferSize];
            mTxBuffer = new byte[mBufferSize];
        } while(false);

        mBipService.openChannelCompleted(ret, this);
    }
    public int closeChannel() {
        int ret = ErrorValue.NO_ERROR;
        MtkCatLog.d("[BIP]", "[TCP]closeChannel.");
        Thread closeChannelThread = null;

        if (rt != null) {
            requestStop();
            rt = null;
        }
        closeChannelThread = new Thread(new Runnable() {
            public synchronized void run() {
                try {
                    if (null != mInput) {
                        mInput.close();
                    }
                    if (null != mOutput) {
                        mOutput.close();
                    }
                    if (null != mSocket) {
                        mSocket.close();
                    }
                } catch (IOException e) {
                    MtkCatLog.e("[BIP]", "[TCP]closeChannel - IOE");
                } finally {
                    mSocket = null;
                    mRxBuffer = null;
                    mTxBuffer = null;
                    mChannelStatus = BipUtils.CHANNEL_STATUS_CLOSE;
                }
            }
        });
        closeChannelThread.start();
        return ret;
    }

    public ReceiveDataResult receiveData(int requestCount) {
        ReceiveDataResult ret = new ReceiveDataResult();
        ret.buffer = new byte[requestCount];
        MtkCatLog.d("[BIP]", "[TCP]receiveData " + mRxBufferCount + "/" + requestCount +
                 "/" + mRxBufferOffset);

            if (mRxBufferCount >= requestCount) {
                try {
                    MtkCatLog.d("[BIP]", "[TCP]Start to copy data from buffer");

                    System.arraycopy(mRxBuffer, mRxBufferOffset, ret.buffer, 0, requestCount);
                    mRxBufferCount -= requestCount;
                    mRxBufferOffset += requestCount;
                    ret.remainingCount = mRxBufferCount;
                } catch (IndexOutOfBoundsException e) {
                }
            } else {
                int needCopy = requestCount;
                int canCopy = mRxBufferCount;
                int countCopied = 0;
                boolean canExitLoop = false;

                while (!canExitLoop) {
                    if (needCopy > canCopy) {
                        try {
                            System.arraycopy(mRxBuffer, mRxBufferOffset, ret.buffer, countCopied,
                                    canCopy);
                            mRxBufferOffset += canCopy;
                            mRxBufferCount -= canCopy;
                            countCopied += canCopy;
                            needCopy -= canCopy;
                        } catch (IndexOutOfBoundsException e) {
                        }
                    } else {
                        try {
                            System.arraycopy(mRxBufferCount, mRxBufferOffset, ret.buffer,
                                             countCopied, canCopy);
                            mRxBufferOffset += needCopy;
                            countCopied += needCopy;
                            needCopy = 0;
                        } catch (IndexOutOfBoundsException e) {
                        }
                    }

                    if (needCopy == 0) {
                        canExitLoop = true;
                    } else {
                        try {
                            int count = mInput.read(mRxBuffer, 0, mRxBuffer.length);
                            mRxBufferCount = count;
                            mRxBufferOffset = 0;
                        } catch (IOException e) {
                            MtkCatLog.e("[BIP]", "[TCP]receiveData - IOE");
                        }
                    }
                }
            }
        return ret;
    }

    public int sendData(byte[] data, int mode) {
        int ret = ErrorValue.NO_ERROR;
        byte[] tmpBuffer = null;

        if (null == data) {
            MtkCatLog.e("[BIP]", "[TCP]sendData - data null:");
            return ErrorValue.BIP_ERROR;
        }

        if (null == mTxBuffer) {
            MtkCatLog.e("[BIP]", "[TCP]sendData - mTxBuffer null:");
            return ErrorValue.BIP_ERROR;
        }

        int txRemaining = mTxBuffer.length - mTxBufferCount;

        try {
            MtkCatLog.d("[BIP]", "[TCP]sendData: size of data:" + data.length + " mode:" + mode);
            MtkCatLog.d("[BIP]", "[TCP]sendData: size of buffer:" + mTxBuffer.length +
                     " count:" + mTxBufferCount);

            if (0 == mTxBufferCount && BipUtils.SEND_DATA_MODE_IMMEDIATE == mode) {
                tmpBuffer = data;
                mTxBufferCount = data.length;
            } else {
                try {
                    if (txRemaining >= data.length) {
                        System.arraycopy(data, 0, mTxBuffer, mTxBufferCount, data.length);
                        mTxBufferCount += data.length;
                    } else {
                        MtkCatLog.d("[BIP]", "[TCP]sendData - tx buffer is not enough");
                    }
                } catch (IndexOutOfBoundsException e) {
                    return ErrorValue.BIP_ERROR;
                }
                tmpBuffer = mTxBuffer;
            }
            if (mode == BipUtils.SEND_DATA_MODE_IMMEDIATE &&
                    mChannelStatus == BipUtils.CHANNEL_STATUS_OPEN) {
                try {
                    MtkCatLog.d("[BIP]", "[TCP]SEND_DATA_MODE_IMMEDIATE:" + mTxBuffer.length +
                             " count:" + mTxBufferCount);
                    mOutput.write(tmpBuffer, 0, mTxBufferCount);
                    mOutput.flush();
                    mTxBufferCount = 0;
                } catch (IOException e) {
                    MtkCatLog.e("[BIP]", "[TCP]sendData - Exception");
                    e.printStackTrace();
                    return ErrorValue.BIP_ERROR;
                }
            }
        } catch (NullPointerException ne) {
            MtkCatLog.d("[BIP]", "[UDP]sendData NE");
            ne.printStackTrace();
            ret = ErrorValue.BIP_ERROR;
        }
        return ret;
    }

    public int getTxAvailBufferSize() {
        if (null == mTxBuffer) {
            MtkCatLog.e("[BIP]", "[TCP]getTxAvailBufferSize - mTxBuffer null:");
            return 0;
        }
        int txRemaining = mTxBuffer.length - mTxBufferCount;
        MtkCatLog.d("[BIP]", "[TCP]available tx buffer size:" + txRemaining);
        return txRemaining;
    }

    public int receiveData(int requestSize, ReceiveDataResult rdr) {
        int ret = ErrorValue.NO_ERROR;
        MtkCatLog.d("[BIP]", "[TCP]new receiveData method");

        if (rdr == null) {
            MtkCatLog.e("[BIP]", "[TCP]rdr is null");
            return ErrorValue.BIP_ERROR;
        }

        MtkCatLog.d("[BIP]", "[TCP]receiveData mRxBufferCount:" + mRxBufferCount
            + " requestSize: " + requestSize + " mRxBufferOffset:" + mRxBufferOffset);

        rdr.buffer = new byte[requestSize];
        if (mRxBufferCount >= requestSize) {
            try {
                synchronized (mLock) {
                    if (mRxBuffer != null && rdr.buffer != null ) {
                        System.arraycopy(mRxBuffer, mRxBufferOffset, rdr.buffer, 0, requestSize);
                    } else {
                        MtkCatLog.d("[BIP]", "[TCP]mRxBuffer or rdr.buffer is null 1");
                        return ErrorValue.BIP_ERROR;
                    }
                    mRxBufferOffset += requestSize;
                    mRxBufferCount -= requestSize;
                    if (mRxBufferCount == 0) {
                        mRxBufferOffset = 0;
                        // Doesn't receive new data from network until the data
                        // in mRxBuffer are received done by SIM
                        // mLock.notify();
                    }
                    rdr.remainingCount = mRxBufferCount;
                }
            } catch (IndexOutOfBoundsException e) {
                MtkCatLog.e("[BIP]", "[TCP]fail copy rx buffer out 1");
                return ErrorValue.BIP_ERROR;
            }
        } else {
            MtkCatLog.e("[BIP]", "[TCP]rx buffer is insufficient !!!");
            try {
                synchronized (mLock) {
                    if (mRxBuffer != null && rdr.buffer != null) {
                        System.arraycopy(mRxBuffer, mRxBufferOffset, rdr.buffer, 0, mRxBufferCount);
                    } else {
                        MtkCatLog.d("[BIP]", "[TCP]mRxBuffer or rdr.buffer is null 2");
                        return ErrorValue.BIP_ERROR;
                    }

                    mRxBufferOffset = 0;
                    mRxBufferCount = 0;
                    mLock.notify();
                }
                rdr.remainingCount = 0;
                ret = ErrorValue.MISSING_DATA;
            } catch (IndexOutOfBoundsException e) {
                MtkCatLog.e("[BIP]", "[TCP]fail copy rx buffer out 2");
                return ErrorValue.BIP_ERROR;
            }
        }
        return ret;
    }
}

class UdpChannel extends Channel {
    DatagramSocket mSocket = null;
    private static final int UDP_SOCKET_TIMEOUT = 3000;
    Thread rt = null;

    UdpChannel(int cid, int linkMode, int protocolType, InetAddress address, int port,
               int bufferSize, MtkCatService handler, BipService bipManager) {
        super(cid, linkMode, protocolType, address, port, bufferSize, handler, bipManager);
    }

    public int openChannel(BipCmdMessage cmdMsg, Network network) {
        int ret = ErrorValue.NO_ERROR;
        mNetwork = network;

        if (mLinkMode == BipUtils.LINK_ESTABLISHMENT_MODE_IMMEDIATE) {
            try {
                mSocket = new DatagramSocket();
                mNetwork.bindSocket(mSocket);
                mChannelStatus = BipUtils.CHANNEL_STATUS_OPEN;
                mChannelStatusData.mChannelStatus = ChannelStatus.CHANNEL_STATUS_LINK;
                rt = new Thread(new UdpReceiverThread(mSocket));
                rt.start();
                MtkCatLog.d("[BIP]", "[UDP]: sock status:" + mChannelStatus);
            } catch (Exception e) {
                e.printStackTrace();
            }

            ret = checkBufferSize();
            if (ret == ErrorValue.COMMAND_PERFORMED_WITH_MODIFICATION) {
                MtkCatLog.d("[BIP]", "[UDP]openChannel: buffer size is modified");
                cmdMsg.mBufferSize = mBufferSize;
            }
            mRxBuffer = new byte[mBufferSize];
            mTxBuffer = new byte[mBufferSize];
        }

        return ret;
    }

    public int closeChannel() {
        int ret = ErrorValue.NO_ERROR;

        MtkCatLog.d("[BIP]", "[UDP]closeChannel.");

        if (rt != null)
        {
            requestStop();
            rt = null;
        }
        if (mSocket != null) {

            MtkCatLog.d("[BIP]", "[UDP]closeSocket.");

            mSocket.close();
            mChannelStatus = BipUtils.CHANNEL_STATUS_CLOSE;

            mSocket = null;
            mRxBuffer = null;
            mTxBuffer = null;
        }

        return ret;
    }

    public ReceiveDataResult receiveData(int requestCount) {
        ReceiveDataResult ret = new ReceiveDataResult();
        ret.buffer = new byte[requestCount];

        MtkCatLog.d("[BIP]", "[UDP]receiveData " + mRxBufferCount + "/" + requestCount +
                 "/" + mRxBufferOffset);

        if (mRxBufferCount >= requestCount) {
            try {
                System.arraycopy(mRxBuffer, mRxBufferOffset, ret.buffer, 0, requestCount);
                mRxBufferOffset += requestCount;
                mRxBufferCount -= requestCount;
                ret.remainingCount = mRxBufferCount;
            } catch (IndexOutOfBoundsException e) {
            }
        } else {
            int needCopy = requestCount;
            int canCopy = mRxBufferCount;
            int countCopied = 0;
            boolean canExitLoop = false;

            while (!canExitLoop) {
                if (needCopy > canCopy) {
                    try {
                        System.arraycopy(mRxBuffer, mRxBufferOffset, ret.buffer, countCopied,
                                canCopy);
                        countCopied += canCopy;
                        needCopy -= canCopy;
                        mRxBufferOffset += canCopy;
                        mRxBufferCount -= canCopy;
                    } catch (IndexOutOfBoundsException e) {
                    }
                } else {
                    try {
                        System.arraycopy(mRxBuffer, mRxBufferOffset, ret.buffer, countCopied,
                                needCopy);
                        mRxBufferOffset += needCopy;
                        mRxBufferCount -= needCopy;
                        countCopied += needCopy;
                        needCopy = 0;
                    } catch (IndexOutOfBoundsException e) {
                    }
                }

                if (needCopy == 0) {
                    canExitLoop = true;
                } else {
                    try {
                        mSocket.setSoTimeout(UDP_SOCKET_TIMEOUT);
                        DatagramPacket packet = new DatagramPacket(mRxBuffer, mRxBuffer.length);
                        mSocket.receive(packet);
                        mRxBufferOffset = 0;
                        mRxBufferCount = packet.getLength();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        return ret;
    }

    public int sendData(byte[] data, int mode) {
        int ret = ErrorValue.NO_ERROR;
        byte[] tmpBuffer = null;

        if (null == data) {
            MtkCatLog.e("[BIP]", "[UDP]sendData - data null:");
            return ErrorValue.BIP_ERROR;
        }

        if (null == mTxBuffer) {
            MtkCatLog.e("[BIP]", "[UDP]sendData - mTxBuffer null:");
            return ErrorValue.BIP_ERROR;
        }

        int txRemaining = mTxBuffer.length - mTxBufferCount;

        MtkCatLog.d("[BIP]", "[UDP]sendData: size of data:" + data.length + " mode:" + mode);
        MtkCatLog.d("[BIP]", "[UDP]sendData: size of buffer:" + mTxBuffer.length +
                 " count:" + mTxBufferCount);
        try {
        if (0 == mTxBufferCount && BipUtils.SEND_DATA_MODE_IMMEDIATE == mode) {
            tmpBuffer = data;
            mTxBufferCount = data.length;
        } else {
            if (txRemaining >= data.length) {
                try {
                    System.arraycopy(data, 0, mTxBuffer, mTxBufferCount, data.length);
                    mTxBufferCount += data.length;
                } catch (IndexOutOfBoundsException e) {
                    MtkCatLog.e("[BIP]", "[UDP]sendData - IndexOutOfBoundsException");
                }
            } else {
                MtkCatLog.d("[BIP]", "[UDP]sendData - tx buffer is not enough:" + txRemaining);
            }
            tmpBuffer = mTxBuffer;
        }

        if (mode == BipUtils.SEND_DATA_MODE_IMMEDIATE) {
            MtkCatLog.d("[BIP]", "[UDP]Send data(" + mAddress + ":" + mPort + "):" +
                     mTxBuffer.length + " count:" + mTxBufferCount);

            DatagramPacket packet = new DatagramPacket(tmpBuffer, 0, mTxBufferCount, mAddress,
                                                       mPort);
            if (mSocket != null) {
                try {
                    mSocket.send(packet);
                    mTxBufferCount = 0;
                } catch (Exception e) {
                    MtkCatLog.e("[BIP]", "[UDP]sendData - Exception");
                    mChannelStatusData.mChannelStatus = ChannelStatus.CHANNEL_STATUS_NO_LINK;
                    e.printStackTrace();
                    return ErrorValue.BIP_ERROR;
                }
            }
        }
        } catch (NullPointerException ne) {
            MtkCatLog.d("[BIP]", "[UDP]sendData NE");
            ne.printStackTrace();
            ret = ErrorValue.BIP_ERROR;
        }
        return ret;
    }

    public int getTxAvailBufferSize() {
        if (null == mTxBuffer) {
            MtkCatLog.e("[BIP]", "[UDP]getTxAvailBufferSize - mTxBuffer null:");
            return 0;
        }
        int txRemaining = mTxBuffer.length - mTxBufferCount;
        MtkCatLog.d("[BIP]", "[UDP]available tx buffer size:" + txRemaining);
        return txRemaining;
    }

    public int receiveData(int requestSize, ReceiveDataResult rdr) {
        int ret = ErrorValue.NO_ERROR;
        if (rdr == null) {
            MtkCatLog.e("[BIP]", "[UDP]rdr is null");
            return ErrorValue.BIP_ERROR;
        }

        MtkCatLog.d("[BIP]", "[UDP]receiveData mRxBufferCount:" + mRxBufferCount
            + " requestSize: " + requestSize + " mRxBufferOffset:" + mRxBufferOffset);

        rdr.buffer = new byte[requestSize];
        if (mRxBufferCount >= requestSize) {
            try {
                synchronized (mLock) {
                    System.arraycopy(mRxBuffer, mRxBufferOffset, rdr.buffer, 0, requestSize);
                    mRxBufferOffset += requestSize;
                    mRxBufferCount -= requestSize;
                    if (mRxBufferCount == 0) {
                        mRxBufferOffset = 0;
                        // Doesn't receive new data from network until the data
                        // in mRxBuffer are received done by SIM
                        // notify is only sent after terminal response
                        //mLock.notify();
                    }
                    rdr.remainingCount = mRxBufferCount;
                }
            } catch (IndexOutOfBoundsException e) {
                MtkCatLog.e("[BIP]", "[UDP]fail copy rx buffer out 1");
                return ErrorValue.BIP_ERROR;
            }
        } else {
            MtkCatLog.e("[BIP]", "[UDP]rx buffer is insufficient !!!");
            try {
                synchronized (mLock) {
                    System.arraycopy(mRxBuffer, mRxBufferOffset, rdr.buffer, 0, mRxBufferCount);
                    mRxBufferOffset = 0;
                    mRxBufferCount = 0;
                    mLock.notify();
                }
                rdr.remainingCount = 0;
                ret = ErrorValue.MISSING_DATA;
            } catch (IndexOutOfBoundsException e) {
                MtkCatLog.e("[BIP]", "[UDP]fail copy rx buffer out 2");
                return ErrorValue.BIP_ERROR;
            }
        }
        return ret;
    }
}

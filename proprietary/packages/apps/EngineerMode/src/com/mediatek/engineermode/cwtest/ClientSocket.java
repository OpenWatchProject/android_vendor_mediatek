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

package com.mediatek.engineermode.cwtest;

import com.mediatek.engineermode.Elog;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;


/**
 * Client socket, used to connect with GPS server and communicate with PMTK command.
 *
 *
 * @author mtk54046
 * @version 1.0
 */
public class ClientSocket {

    private static final int MASK_8_BIT = 0xFF;
    private static final int SOCKET_TIME_OUT_TIME = 3000;
    private static final int LINE_OUT_SIZE = 1000;
    private static final int BUFFER_SIZE = 2048;
    private static final String TAG = "CwTest";
    private static final int SERVER_PORT = 7000;
    private Socket mClientSocket = null;
    private DataInputStream mDataInput = null;
    private DataOutputStream mDataOutput = null;
    private String mCommand = null;
    private String mResponse = null;
    private BlockingQueue<String> mCommandQueue = null;
    private CWTest mCallBack = null;
    private Thread mSendThread = null;
    private byte[] mDataBuffer = null;

    /**
     * Constructor, initial parameters and start thread to send/receive.
     *
     * @param callBack
     *            Callback when message received
     */
    public ClientSocket(CWTest callBack) {
        this.mCallBack = callBack;
        mCommandQueue = new LinkedBlockingQueue<String>();
        mDataBuffer = new byte[BUFFER_SIZE];


        mSendThread = new Thread(new Runnable() {
            public void run() {
                while (true) {
                    try {
                        mCommand = mCommandQueue.take();
                        Elog.i(TAG, "Queue take command:" + mCommand);
                    } catch (InterruptedException ie) {
                        Elog.w(TAG, "Take command interrupted:" + ie.getMessage());
                        return;
                    }
                    openClient();
                    if (null != mDataOutput && null != mDataInput) {
                        try {
                            mDataOutput.writeBytes(mCommand);
                            mDataOutput.write('\r');
                            mDataOutput.write('\n');
                            mDataOutput.flush();
                            if (mCommand.contains(CWTest.START_CMD)) {
                                String result = null;
                                int line = 0;
                                int count = -1;
                                while ((count = mDataInput.read(mDataBuffer)) != -1) {
                                    line++;
                                    result = new String(mDataBuffer, 0, count);
                                    if (result.contains("PMTK817")) {
                                        mResponse = result;
                                        if (ClientSocket.this.mCallBack != null) {
                                            ClientSocket.this.mCallBack.onResponse(mResponse);
                                        }

                                        break;
                                    }
                                    if (line > LINE_OUT_SIZE) {
                                        break;
                                    }
                                }
                            }
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }

                    closeClient();
                }
            }
        });
        mSendThread.start();
    }

    /**
     * Start client socket and connect with server.
     */
    private void openClient() {
        Elog.i(TAG, "enter startClient");
        if (null != mClientSocket && mClientSocket.isConnected()) {
            Elog.w(TAG, "localSocket has started, return");
            return;
        }
        try {
            mClientSocket = new Socket("127.0.0.1", SERVER_PORT);
            mClientSocket.setSoTimeout(SOCKET_TIME_OUT_TIME);
            mDataOutput = new DataOutputStream(mClientSocket.getOutputStream());
            mDataInput = new DataInputStream(mClientSocket.getInputStream());
        } catch (UnknownHostException e) {
            Elog.e(TAG, e.getMessage());
        } catch (IOException e) {
            Elog.e(TAG, e.getMessage());
        }
    }

    private void closeClient() {
        Elog.v(TAG, "enter closeClient");
        try {
            if (null != mDataInput) {
                mDataInput.close();
            }
            if (null != mDataOutput) {
                mDataOutput.close();
            }
            if (null != mClientSocket) {
                mClientSocket.close();
            }
        } catch (IOException e) {
            Elog.w(TAG, "closeClient IOException: " + e.getMessage());
        } finally {
            mClientSocket = null;
            mDataInput = null;
            mDataOutput = null;
        }
    }

    /**
     * Finalize communicate with socket server.
     */
    void endClient() {
        Elog.v(TAG, "enter endClient");
        mSendThread.interrupt();
        Elog.v(TAG, "Queue remaining:" + mCommandQueue.size());
        closeClient();
        mCallBack = null;
    }


    /**
     * Send command to socket server.
     *
     * @param command
     *            Command need to send
     */
    void sendCommand(String command) {
        String sendComm = "$" + command + "*" + calcCS(command);
        Elog.i(TAG, "Send command: " + sendComm);
        if (!mSendThread.isAlive()) {
            Elog.w(TAG, "sendThread is not alive");
            mSendThread.start();
        }
        if (command.equals(sendComm) || mCommandQueue.contains(sendComm)) {
            Elog.w(TAG, "send command return because of hasn't handle the same");
            return;
        }
        try {
            mCommandQueue.put(sendComm);
        } catch (InterruptedException ie) {
            Elog.w(TAG, "send command interrupted:" + ie.getMessage());
        }
    }

    /**
     * Calculate check sum for PMTK command.
     *
     * @param command
     *            The command need to send
     * @return The check sum string
     */
    private String calcCS(String command) {
        if (null == command || "".equals(command)) {
            return "";
        }
        byte[] ba = command.toUpperCase().getBytes();
        int temp = 0;
        for (byte b : ba) {
            temp ^= b;
        }
        return String.format("%1$02x", temp & MASK_8_BIT).toUpperCase();
    }
}

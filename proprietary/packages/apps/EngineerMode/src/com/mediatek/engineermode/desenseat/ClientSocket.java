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

package com.mediatek.engineermode.desenseat;

import com.mediatek.engineermode.Elog;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.SocketException;
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
    private static final int BUFFER_SIZE = 2048;
    private static final String TAG = "DesenseAT/ClientSocket";
    private static final int SERVER_PORT = 7000;
    private Socket mClientSocket = null;
    private DataInputStream mDataInput = null;
    private DataOutputStream mDataOutput = null;
    private String mCommand = null;
    private String mResponse = null;
    private BlockingQueue<String> mCommandQueue = null;
    private OnResponseListener mCallBack = null;
    private Thread mSendThread = null;
    private byte[] mDataBuffer = null;
    private boolean mAlive = false;
    private static final String PMTK_ACK_HEAD = "PMTK001,828";
    private static final String PMTK_DESENSE_HEAD = "PMTK828";
    private static final int PMTK_DESENSE_HEAD_LENGTH = 7;
    private static final int READ_TIME_OUT = 6000;

    /**
     * Interface to hand data from socket.
     *
     */
    public interface OnResponseListener {
        /**
         * @param response data from socket
         */
        void onResponse(String response);
    }

    boolean sendSetSpecCmd(String cmd, int retrynumber) {

        String sendCmd = "$" + cmd + "*" + calcCS(cmd);
        Elog.v(TAG, "sendSetSpecCmd: " + sendCmd);
        return writeCmdWithRetry(sendCmd, retrynumber);
    }
    private boolean writeCmdWithRetry(String cmd, int retrynumber) {
        for (int k = 0; k < retrynumber; k++) {
            if (mDataOutput != null && mDataInput != null && mAlive)  {
                try {
                    mDataOutput.writeBytes(cmd);
                    mDataOutput.write('\r');
                    mDataOutput.write('\n');
                    mDataOutput.flush();
                    Elog.v(TAG, "writeCmdWithRetry: " + k);

                    String result = null;
                    int count = -1;

                    long startTime = System.currentTimeMillis();
                    while ((mDataInput != null) && ((count = mDataInput.read(mDataBuffer)) != -1)) {

                        result = new String(mDataBuffer, 0, count);
                        if (result.contains(PMTK_ACK_HEAD)) {
                            Elog.v(TAG, "result.contains(PMTK_ACK_HEAD)");
                            int startIndex = result.indexOf(PMTK_ACK_HEAD);
                            if (startIndex != -1) {
                                result = result.substring(startIndex + 4);
                            }

                            int endIndex = result.indexOf("*");
                            if (endIndex != -1) {
                                mResponse = result.substring(0, endIndex);
                            }

                            String[] splited = mResponse.split(",");

                            int ack = Integer.parseInt(splited[2]);
                            Elog.v(TAG, "ack: " + ack);

                            if (ack == 1) {
                                break;
                            } else if (ack == 3) {
                                return true;
                            }
                        } else {
                            if ((System.currentTimeMillis() - startTime) > READ_TIME_OUT) {
                                Elog.e(TAG, "READ_TIME_OUT");
                                break;
                            }
                        }

                    }
                } catch (SocketException e) {
                    Elog.w(TAG, "read data SocketException: " + e.getMessage());
               } catch (IOException e) {
                    e.printStackTrace();
               }
           } else {
               return false;
           }
       }
       return false;

    }

    private void receiveResponse() {

            try {

                String result = null;
                int count = -1;
                long startTime = System.currentTimeMillis();
                while ((mDataInput != null) && ((count = mDataInput.read(mDataBuffer)) != -1)) {
                    if (!mAlive) {
                        mResponse = null;
                        return;
                    }
                    result += new String(mDataBuffer, 0, count);
                    long receiveTime = System.currentTimeMillis();
                    if (!result.contains(PMTK_DESENSE_HEAD)) {
                        if ((receiveTime - startTime) > READ_TIME_OUT) {
                            if (ClientSocket.this.mCallBack != null) {
                                ClientSocket.this.mCallBack.onResponse(null);
                            }
                            return;
                        }
                    } else {
                        startTime = receiveTime;
                    }

                    while (result.contains(PMTK_DESENSE_HEAD)) {
                        int startIndex = result.indexOf(PMTK_DESENSE_HEAD);

                        String strTemp;
                        if (startIndex != -1) {
                            strTemp = result.substring(startIndex + 4);
                        } else {
                            break;
                        }
                        int endIndex = strTemp.indexOf("*");
                        if (endIndex != -1) {
                            mResponse = strTemp.substring(0, endIndex);
                            if (ClientSocket.this.mCallBack != null) {
                                ClientSocket.this.mCallBack.onResponse(mResponse);
                            }
                            result = strTemp;
                        } else {
                            break;
                        }
                    }
                    if (result.length() >= PMTK_DESENSE_HEAD_LENGTH) {
                        result = result.substring(result.length() - PMTK_DESENSE_HEAD_LENGTH);
                    }

                }
                if (!mAlive) {
                    return;
                }
            } catch (SocketException e) {
                Elog.w(TAG,
                        "read data SocketException: "
                                + e.getMessage());
                if (ClientSocket.this.mCallBack != null) {
                    ClientSocket.this.mCallBack.onResponse(null);
                }
                return;
            } catch (IOException e) {
                Elog.w(TAG,
                        "sendCommand IOException: "
                                + e.getMessage());
                if (ClientSocket.this.mCallBack != null) {
                    ClientSocket.this.mCallBack.onResponse(null);
                }
                return;
            }

    }


    /**
     * Constructor, initial parameters and start thread to send/receive.
     *
     * @param callBack
     *            Callback when message received
     */
    public ClientSocket(OnResponseListener callBack) {
        Elog.v(TAG, "ClientSocket constructor");
        this.mCallBack = callBack;
        mCommandQueue = new LinkedBlockingQueue<String>();
        mDataBuffer = new byte[BUFFER_SIZE];


        mSendThread = new Thread(new Runnable() {
            public void run() {
                while (true) {
                    try {
                        mCommand = mCommandQueue.take();
                        Elog.v(TAG, "Queue take command:" + mCommand);
                    } catch (InterruptedException ie) {
                        Elog.w(TAG,
                                "Take command interrupted:" + ie.getMessage());
                        return;
                    }
                    openClient();
                    mResponse = null;
                    if (!writeCmdWithRetry(mCommand, 10)) {
                        mResponse = null;
                        if (ClientSocket.this.mCallBack != null) {
                            ClientSocket.this.mCallBack.onResponse(mResponse);
                        }
                    } else {
                        receiveResponse();
                    }
                    mCommand = null;
                }
            }
        });
        mSendThread.start();
    }

    /**
     * Constructor function.
     */
    public ClientSocket() {
        mDataBuffer = new byte[BUFFER_SIZE];
        openClient();
    }

    /**
     * Start client socket and connect with server.
     */
    private void openClient() {
        Elog.v(TAG, "enter startClient");
        mAlive = true;
        if (null != mClientSocket && mClientSocket.isConnected()) {
            Elog.d(TAG, "localSocket has started, return");
            return;
        }
        try {
            mClientSocket = new Socket("127.0.0.1", SERVER_PORT);
            mDataOutput = new DataOutputStream(mClientSocket.getOutputStream());
            mDataInput = new DataInputStream(mClientSocket.getInputStream());
            mClientSocket.setSoTimeout(READ_TIME_OUT);
        } catch (UnknownHostException e) {
            Elog.w(TAG, e.getMessage());
        } catch (IOException e) {
            Elog.w(TAG, e.getMessage());
        }
    }

    /**
     * Stop client socket and disconnect with server.
     */
    void closeClient() {
        Elog.v(TAG, "enter closeClient");
        mAlive = false;
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
     * Send command to socket server.
     *
     * @param command
     *            Command need to send
     */
    public void sendCommand(String command) {
        String sendComm = "$" + command + "*" + calcCS(command);
        Elog.v(TAG, "Send command: " + sendComm);
        if (!mSendThread.isAlive()) {
            Elog.v(TAG, "sendThread is not alive");
            mSendThread.start();
        }
        if (command.equals(sendComm) || mCommandQueue.contains(sendComm)) {
            Elog.v(TAG, "send command return because of hasn't handle the same");
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

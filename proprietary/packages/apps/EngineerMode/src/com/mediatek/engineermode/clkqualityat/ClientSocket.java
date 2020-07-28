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

package com.mediatek.engineermode.clkqualityat;

import com.mediatek.engineermode.Elog;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;


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
    private static final String TAG = "ClkQualityAt/ClientSocket";
    private static final int SERVER_PORT = 7000;
    private Socket mClientSocket = null;
    private DataInputStream mDataInput = null;
    private DataOutputStream mDataOutput = null;
    private String mResponse = null;
    private OnResponseListener mCallBack = null;
    private byte[] mDataBuffer = null;
    private boolean mAlive = false;
    private static final String PMTK_RES_HEAD = "PMTK856";
    private static final int READY_BIT_START_TIME = 56;
    private static final int READY_BIT_TAEGET = 1;
    private static final int READ_TIME_OUT = 6000;
    private ClkQualityAtActivity.ClockType mClockType;
    private boolean mIsClockTypeCorrect = true;

    /**
     * Interface to hand data from socket.
     *
     */
    public interface OnResponseListener {

        /**
         * @param response data from socket
         */
        public void onResponse(String response);

        /**
         * calculate result after receive all data.
         */
        public void onCalculateResult();
    }

    boolean sendCheckBitCmd(String startCmd, String stopCmd, int time) {
        String sendCmdStart = "$" + startCmd + "*" + calcCS(startCmd);
        String sendCmdStop = "$" + stopCmd + "*" + calcCS(startCmd);
        if (mDataOutput != null && mDataInput != null && mAlive) {
            try {
                Elog.d(TAG, "sendCheckBitCmd: " + sendCmdStart);
                mDataOutput.writeBytes(sendCmdStart);
                mDataOutput.write('\r');
                mDataOutput.write('\n');
                mDataOutput.flush();
                String result = null;
                int count = -1;
                int index = 0;
                String[] splited = null;
                int readyBit = 0;
                while ((mDataInput != null) && ((count = mDataInput.read(mDataBuffer)) != -1)) {
                    result = new String(mDataBuffer, 0, count);
                    if (result.contains(PMTK_RES_HEAD)) {
                        Elog.d(TAG, "@sendCheckBitCmd, result contains:" + PMTK_RES_HEAD + ", len: "
                                    + result.length() + " times:#" + index);
                        int startIndex = result.indexOf(PMTK_RES_HEAD);
                        if (startIndex != -1) {
                            result = result.substring(startIndex + 4);
                        }
                        int endIndex = result.indexOf("*");
                        if (endIndex != -1) {
                            mResponse = result.substring(0, endIndex);
                            splited = mResponse.split(",");
                            readyBit = Integer.parseInt(splited[6]);
                            Elog.d(TAG, "@sendCheckBitCmd, trim response after =  " + mResponse);
                        }
                        ++index;
                        Elog.d(TAG, "index = " + index);
                        // check clock type
                        if (splited != null && index < 5 ) {
                            float clkDrift = Float.valueOf(splited[1]);
                            float clkDriftRate = Float.valueOf(splited[2]);
                            if ( (clkDrift == -100.0f && clkDriftRate == -100.0f
                                 && mClockType == ClkQualityAtActivity.ClockType.Co_TMS)
                                || (clkDrift != -100.0f && clkDriftRate != -100.0f
                                 && mClockType == ClkQualityAtActivity.ClockType.TCXO) ) {
                                Elog.d(TAG, "@sendCheckBitCmd, clock type setting wrong");
                                mIsClockTypeCorrect = false;
                                return false;
                            } else {
                                mIsClockTypeCorrect = true;
                            }
                        }
                        if ((index >= READY_BIT_START_TIME) && (index <= time)) {
                            Elog.d(TAG, "@sendCheckBitCmd, readyBit: " + readyBit);
                            if (readyBit != READY_BIT_TAEGET) {
                                mDataOutput.writeBytes(sendCmdStop);
                                mDataOutput.write('\r');
                                mDataOutput.write('\n');
                                mDataOutput.flush();
                                Elog.d(TAG, "sendCheckBitCmd: " + sendCmdStop);
                                return false;
                            }
                        }
                        if (index == time) {
                            mDataOutput.writeBytes(sendCmdStop);
                            mDataOutput.write('\r');
                            mDataOutput.write('\n');
                            mDataOutput.flush();
                            Elog.d(TAG, "sendCheckBitCmd: " + sendCmdStop);
                            return true;
                        }
                    }
                }
            } catch (SocketException e) {
                Elog.w(TAG, "@sendCheckBitCmd, read data SocketException: " + e.getMessage());
            } catch (IOException e) {
                e.printStackTrace();
            }
       }
       return false;
    }

    /**
     * Constructor, initial parameters and start thread to send/receive.
     *
     * @param callBack
     *            Callback when message received
     */
    public ClientSocket(OnResponseListener callBack) {
        Elog.d(TAG, "ClientSocket constructor");
        this.mCallBack = callBack;
        mDataBuffer = new byte[BUFFER_SIZE];
        openClient();
    }

    /**
     * Start client socket and connect with server.
     */
    private void openClient() {
        Elog.d(TAG, "enter startClient");
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
    public void closeClient() {
        Elog.d(TAG, "enter closeClient");
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

    public void setClockType(ClkQualityAtActivity.ClockType clockType) {
        mClockType = clockType;
    }

    public ClkQualityAtActivity.ClockType getClockType() {
        return mClockType;
    }

    public boolean isClockTypeCorrect() {
        return mIsClockTypeCorrect;
    }

    /**
     * Send command to socket server.
     *
     * @param command
     *            Command need to send
     */
    /**
     * Send command to socket server to start and stop the test.
     * @param startCmd Command to start server to test
     * @param stopCmd Command to stop server to test
     * @param times The number of response expected
     */
    public void sendTestCmd(String startCmd, String stopCmd, int times) {

        String sendCmdStart = "$" + startCmd + "*" + calcCS(startCmd);
        String sendCmdStop = "$" + stopCmd + "*" + calcCS(stopCmd);
        if (mDataOutput != null && mDataInput != null && mAlive) {
            try {
                Elog.d(TAG, "sendStartCmd: " + sendCmdStart);
                mDataOutput.writeBytes(sendCmdStart);
                mDataOutput.write('\r');
                mDataOutput.write('\n');
                mDataOutput.flush();

                String result = null;
                int count = -1;
                int index = 0;
                while ((mDataInput != null) && ((count = mDataInput.read(mDataBuffer)) != -1)) {
                    result = new String(mDataBuffer, 0, count);
                    if (result.contains(PMTK_RES_HEAD)) {
                        Elog.d(TAG, "@sendTestCmd, result.contains(PMTK_RES_HEAD)");
                        int startIndex = result.indexOf(PMTK_RES_HEAD);
                        if (startIndex != -1) {
                            result = result.substring(startIndex + 4);
                        }
                        int endIndex = result.indexOf("*");
                        if (endIndex != -1) {
                            mResponse = result.substring(0, endIndex);
                        }
                        if (ClientSocket.this.mCallBack != null) {
                            ClientSocket.this.mCallBack.onResponse(mResponse);
                        }

                        if (++index == times) {
                            if (ClientSocket.this.mCallBack != null) {
                                Elog.d(TAG, "ClientSocket called onCalculateResult!");
                                ClientSocket.this.mCallBack.onCalculateResult();
                            }
                            Elog.d(TAG, "@sendTestCmd, sendStopCmd: " + sendCmdStop);
                            mDataOutput.writeBytes(sendCmdStop);
                            mDataOutput.write('\r');
                            mDataOutput.write('\n');
                            mDataOutput.flush();
                            return;
                        }
                    }
                }
            } catch (SocketException e) {
                Elog.w(TAG, "read data SocketException: " + e.getMessage());
            } catch (IOException e) {
                e.printStackTrace();
            }
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

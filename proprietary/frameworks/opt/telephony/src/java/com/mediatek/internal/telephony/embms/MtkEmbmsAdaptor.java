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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony;

import android.content.Context;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.SubscriptionController;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import java.nio.charset.StandardCharsets;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * @hide
 */
public class MtkEmbmsAdaptor {
    private static final String TAG = "MtkEmbmsAdaptor";

    private static MtkEmbmsAdaptor sInstance = null;
    private MtkEmbmsAdaptEventHandler mEventHandler = null;
    private SubscriptionController mSubscriptionController;

    private static final int MSG_ID_EVENT_REQUEST   = 0;
    private static final int MSG_ID_EVENT_RESPONSE  = 1;
    private static final int MSG_ID_EVENT_IND  = 2;


    private MtkEmbmsAdaptor(Context c, CommandsInterface[] ci) {
        Rlog.i(TAG, "construtor 2 parameter is called - start");

        mEventHandler = new MtkEmbmsAdaptEventHandler();
        mEventHandler.setRil(c, ci);
        mSubscriptionController = SubscriptionController.getInstance();
        new Thread() {
            public void run() {
                ServerTask server = new ServerTask();
                server.listenConnection(mEventHandler);
            }
        }.start();

        int numPhones = TelephonyManager.getDefault().getPhoneCount();
        MtkRIL mci;
        for (int i = 0; i < numPhones; i++) {
            mci = (MtkRIL) ci[i];
            mci.setAtInfoNotification(mEventHandler, MSG_ID_EVENT_IND, i);
        }

        Rlog.i(TAG, "construtor is called - end");
    }

    /** @hide
     *  @return return the static instance of MtkEmbmsAdaptor
     */
    public static MtkEmbmsAdaptor getDefault(Context context, CommandsInterface[] ci) {
        Rlog.d(TAG, "getDefault()");
        if (sInstance == null) {
            sInstance = new MtkEmbmsAdaptor(context, ci);
        }
        return sInstance;
    }

    private String messageToString(Message msg) {
        switch (msg.what) {
            case MSG_ID_EVENT_REQUEST:
                return "MSG_ID_EVENT_REQUEST";
            case MSG_ID_EVENT_RESPONSE:
                return "MSG_ID_EVENT_RESPONSE";
            case MSG_ID_EVENT_IND:
                return "MSG_ID_EVENT_IND";
            default:
                return "UNKNOWN";
        }
    }

    /**
     * Maintain a server task to provide extenal client to connect to do
     * some external SIM operation.
     *
     */
    public class ServerTask {
        public static final String HOST_NAME = "/dev/socket/embmsd";

        public void listenConnection(MtkEmbmsAdaptEventHandler eventHandler) {
            Rlog.i(TAG, "listenConnection() - start");

            LocalServerSocket serverSocket = null;
            ExecutorService threadExecutor = Executors.newCachedThreadPool();

            try {
                // Create server socket
                serverSocket = new LocalServerSocket(HOST_NAME);

                while (true) {
                    // Allow multiple connection connect to server.
                    LocalSocket socket = serverSocket.accept();
                    Rlog.d(TAG, "There is a client is accepted: " + socket.toString());

                    threadExecutor.execute(new ConnectionHandler(socket, eventHandler));
                }
            } catch (IOException e) {
                Rlog.e(TAG, "listenConnection catch IOException");
                e.printStackTrace();
            } catch (Exception e) {
                Rlog.e(TAG, "listenConnection catch Exception");
                e.printStackTrace();
            } finally {
                Rlog.d(TAG, "listenConnection finally!!");
                if (threadExecutor != null) {
                    threadExecutor.shutdown();
                }
                if (serverSocket != null) {
                    try {
                        serverSocket.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
            Rlog.i(TAG, "listenConnection() - end");
        }
    }

    public class ConnectionHandler implements Runnable {
        private LocalSocket mSocket;
        private MtkEmbmsAdaptEventHandler mEventHandler;

        public ConnectionHandler(LocalSocket clientSocket, MtkEmbmsAdaptEventHandler eventHandler) {
            mSocket = clientSocket;
            mEventHandler = eventHandler;
        }

        /* (non-Javadoc)
         * @see java.lang.Runnable#run()
         */
        @Override
        public void run() {
            Rlog.i(TAG, "New connection: " + mSocket.toString());

            try {
                MtkEmbmsAdaptIoThread ioThread = new MtkEmbmsAdaptIoThread(
                        ServerTask.HOST_NAME,
                        mSocket.getInputStream(),
                        mSocket.getOutputStream(),
                        mEventHandler);
                mEventHandler.setDataStream(ioThread);
                // Start after setDataStream done to avoid null IO thread cause unexcepted behavior.
                if (ioThread != null) {
                    ioThread.start();
                }

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    class MtkEmbmsAdaptIoThread extends Thread {
        private String mName = "";
        private static final int MAX_DATA_LENGTH = (4 * 1024);
        private boolean mIsContinue = true;
        private InputStream mInput = null;
        private OutputStream mOutput = null;
        private MtkEmbmsAdaptEventHandler mEventHandler = null;
        private final Object mOutputLock = new Object();
        private byte[] readBuffer = null;

        public MtkEmbmsAdaptIoThread(
                String name,
                InputStream inputStream,
                OutputStream outputStream,
                MtkEmbmsAdaptEventHandler eventHandler) {
            mName = name;
            mInput = inputStream;
            mOutput = outputStream;
            mEventHandler = eventHandler;
            Rlog.i(TAG, "MtkEmbmsAdaptIoThread constructor is called.");
            readBuffer = new byte[MAX_DATA_LENGTH];
        }

        public void terminate() {
            Rlog.i(TAG, "MtkEmbmsAdaptIoThread terminate.");
            mIsContinue = false;
        }

        public void run() {
            Rlog.i(TAG, "MtkEmbmsAdaptIoThread running.");
            while (mIsContinue) {
                try {
                    int count = mInput.read(readBuffer, 0, MAX_DATA_LENGTH);
                    if (count < 0) {
                        // End-of-stream reached
                        Rlog.e(TAG, "readEvent(), fail to read and throw exception");
                        break;
                    }

                    if (count > 0) {
                        try {
                            handleInput(new String(readBuffer, 0, count));
                        } catch (Exception ee) {
                            ee.printStackTrace();
                        }
                    }
                } catch (IOException e) {
                    Rlog.e(TAG, "MtkEmbmsAdaptIoThread IOException.");
                    e.printStackTrace();

                    // To avoid server socket is closed due to modem reset
                    // Means the client socket has been disconnected.
                    Rlog.e(TAG, "Socket disconnected.");
                    terminate();
                } catch (Exception e) {
                    Rlog.e(TAG, "MtkEmbmsAdaptIoThread Exception.");
                    e.printStackTrace();
                }
            }
        }

        protected void handleInput(String input) {
            Rlog.d(TAG, "process input: RCV <-(" + input + "),length:" + input.length());
            mEventHandler.sendMessage(mEventHandler.obtainMessage(MSG_ID_EVENT_REQUEST,
                input.trim()));
        }

        public void sendCommand(String rawCmd) {
            Rlog.d(TAG, "SND -> (" + rawCmd + ")");

            synchronized (mOutputLock) {
                if (mOutput == null) {
                    Rlog.e(TAG, "missing SIM output stream");
                } else {
                    try {
                        mOutput.write(rawCmd.getBytes(StandardCharsets.UTF_8));
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

    }

    public class MtkEmbmsAdaptEventHandler extends Handler {
        private MtkEmbmsAdaptIoThread mAdaptorIoThread = null;
        private Context mContext;
        private CommandsInterface[] mCis;

        @Override
        public void handleMessage(Message msg) {
            Message onCompleted;
            String data;
            AsyncResult ar;
            int int_data;

            Rlog.d(TAG, "handleMessage: " + messageToString(msg) + " = " + msg);
            switch (msg.what) {
                case MSG_ID_EVENT_REQUEST:
                    data = (String) msg.obj;
                    Rlog.i(TAG, "MSG_ID_EVENT_REQUEST data: " + data);

                    // notice SubscriptionManager.INVALID_SUBSCRIPTION_ID = -1
                    int subId = mSubscriptionController.getDefaultDataSubId();
                    int slotId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
                    if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                        Rlog.e(TAG, "getDefaultDataSubId fail: " + subId);
                    } else {
                        slotId = mSubscriptionController.getSlotIndex(subId);
                    }

                    if (!SubscriptionManager.isValidSlotIndex(slotId)) {
                        Rlog.e(TAG, "inValidSlotIndex:" + slotId);
                        sendFailureCmd();
                        return;
                    }

                    onCompleted = obtainMessage(MSG_ID_EVENT_RESPONSE, slotId);
                    MtkRIL ci = (MtkRIL) mCis[slotId];
                    ci.sendEmbmsAtCommand(data, onCompleted);
                    break;

                case MSG_ID_EVENT_IND:
                    ar = (AsyncResult) msg.obj;
                    int_data = (int) (ar.userObj); //not used
                    data = (String) ar.result;
                    Rlog.i(TAG, "MSG_ID_EVENT_IND data: " + data);
                    sendCommand(data);
                    break;

                case MSG_ID_EVENT_RESPONSE:
                    ar = (AsyncResult) msg.obj;
                    int_data = (int) (ar.userObj); //not used
                    data = (String) ar.result;
                    Rlog.i(TAG, "MSG_ID_EVENT_RESPONSE data: " + data);
                    if (ar.exception instanceof CommandException
                            && ((CommandException) (ar.exception)).getCommandError()
                            == CommandException.Error.RADIO_NOT_AVAILABLE) {
                            Rlog.e(TAG, "MSG_ID_EVENT_RESPONSE exception: " +
                                ((CommandException) (ar.exception)).getCommandError());
                            sendFailureCmd();
                    } else if (data != null) {
                        sendCommand(data);
                    } else {
                        sendFailureCmd();
                    }
                    break;

                default:
                    break;
            }
        }

        private void setDataStream(MtkEmbmsAdaptIoThread adpatorIo) {
            mAdaptorIoThread = adpatorIo;
            Rlog.d(TAG, "MtkEmbmsAdaptEventHandler setDataStream done.");
        }

        private void setRil(Context context, CommandsInterface[] ci) {
            mContext = context;
            mCis = ci;
            Rlog.d(TAG, "MtkEmbmsAdaptEventHandler setRil done.");
        }

        public void sendCommand(String rawCmd) {
            if (mAdaptorIoThread != null) {
                mAdaptorIoThread.sendCommand(rawCmd);
            } else {
                Rlog.e(TAG, "sendCommand fail!! mAdaptorIoThread is null!");
            }
        }

        public void sendFailureCmd() {
            String cmdStr = String.format("ERROR\n");
            sendCommand(cmdStr);
        }
    }

}


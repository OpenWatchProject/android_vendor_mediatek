/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.telephony.internal.telephony.vsim;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;

import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;

import android.telephony.RadioAccessFamily;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.TelephonyManager.MultiSimVariants;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.ProxyController;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccController;

//import com.mediatek.internal.telephony.dataconnection.DataConnectionHelper;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkProxyController;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkSubscriptionController;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.MtkTelephonyProperties;
import com.mediatek.internal.telephony.uicc.MtkUiccController;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.RadioManager;

import mediatek.telephony.MtkServiceState;


/**
 * @hide
 */
public class ExternalSimManager {
    private static final boolean ENG = "eng".equals(Build.TYPE);
    private static final String TAG = "ExternalSimMgr";

    private static final int NO_RESPONSE_TIMEOUT_DURATION = 13 * 1000;
    private static final byte NO_RESPONSE_STATUS_WORD_BYTE1 = 0x00;
    private static final byte NO_RESPONSE_STATUS_WORD_BYTE2 = 0x00;

    private static final int SOCKET_OPEN_RETRY_MILLIS = 4 * 1000;
    // Maximum APDU length is 261 + result in int(4 bytes) + data len is int(4 bytes)
    private static final int MAX_VSIM_UICC_CMD_LEN = 261 + 4 + 4;

    private static final int AUTO_RETRY_DURATION = 2 * 1000;
    private static final int TRY_RESET_MODEM_DURATION = 2 * 1000;
    private static final int SIM_STATE_RETRY_DURATION = 20 * 1000;

    private static final int PLATFORM_READY_CATEGORY_SUB        = 1;
    private static final int PLATFORM_READY_CATEGORY_SIM_SWITCH = 2;
    private static final int PLATFORM_READY_CATEGORY_RADIO      = 3;

    /**
     * [Customization Flag] PLUG_IN_AUTO_RETRY
     *
     * Please revise this flag to decide if need auto retry when fail to plug in VSIM
     * in case of platfrom not ready (e.g. sim switching not completed).
     */
    private static boolean PLUG_IN_AUTO_RETRY = true;
    private static final int PLUG_IN_AUTO_RETRY_TIMEOUT = 40 * 1000;

    /**
     * [Customization Value] RECOVERY_TO_REAL_SIM_TIMEOUT
     *
     * Please revise the value to customized your time out to decide when to recovery real once
     * there is no VSIM inserted event in case of persit vsim is on.
     *
     * Default value: 5 mins.
     */
    private static final int RECOVERY_TO_REAL_SIM_TIMEOUT = 5 * 60 * 1000;
    private Timer mRecoveryTimer = null;

    private static ExternalSimManager sInstance = null;
    private Context mContext = null;
    private CommandsInterface[] mCi = null;
    private VsimIndEventHandler mIndHandler = null;
    private VsimEvenHandler mEventHandler = null;

    private static final String PREFERED_RSIM_SLOT =
            MtkTelephonyProperties.PROPERTY_PREFERED_REMOTE_SIM;
    private static final String PREFERED_AKA_SIM_SLOT =
            MtkTelephonyProperties.PROPERTY_PREFERED_AKA_SIM;
    private final Object mLockForEventReq = new Object();
    private final Object mLock = new Object();

    private static final int SET_CAPABILITY_NONE = 0;
    private static final int SET_CAPABILITY_ONGOING = 1;
    private static final int SET_CAPABILITY_DONE = 2;
    private static final int SET_CAPABILITY_FAILED = 3;

    private static final int EVENT_VSIM_INDICATION = 1;

    private int mSetCapabilityDone = SET_CAPABILITY_NONE;
    // Use to record user original phone id setting.
    // When disable RSIM, will recovery to user original setting.
    private int mUserMainPhoneId = -1;
    // Use to record radio orighal setting.
    // When disable RSIM, will recovery to user original setting.
    private boolean mUserRadioOn = false;

    static final String[] PROPERTY_RIL_FULL_UICC_TYPE = {
        "vendor.gsm.ril.fulluicctype",
        "vendor.gsm.ril.fulluicctype.2",
        "vendor.gsm.ril.fulluicctype.3",
        "vendor.gsm.ril.fulluicctype.4",
    };

    private static int sPreferedRsimSlot = -1;
    private static int sPreferedAkaSlot = -1;

    /*  Construction function for ExternalSimManager */
    public ExternalSimManager() {
        Rlog.d(TAG, "construtor 0 parameter is called - done");
    }

    private ExternalSimManager(Context context, CommandsInterface []ci) {
        Rlog.d(TAG, "construtor 1 parameter is called - start");

        initVsimConfiguration();

        // To avoid persist VSIM didn't work due to VSIM APK didn't reconnect after reboot.
        // We will start timer to check if VSIM APK reconnect. If there is no VSIM APK connection
        // exist before time out (default 5 mins).
        // We will try to disable VSIM automatically.
        startRecoveryTimer();

        mContext = context;
        mCi = ci;
        new Thread() {
            public void run() {
                Looper.prepare();
                mEventHandler = new VsimEvenHandler();
                mIndHandler = new VsimIndEventHandler();

                for (int i = 0; i < mCi.length; i++) {
                    Integer index = new Integer(i);
                    ((MtkRIL)mCi[i]).registerForVsimIndication(
                                mIndHandler, EVENT_VSIM_INDICATION, index);
                }

                Looper.loop();
            }
        }.start();

        new Thread() {
            public void run() {
                while (mEventHandler == null || mIndHandler == null) {
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                //Send an event to rild to indicate ExternalSimManager exists
                sendExternalSimConnectedEvent(0);
                ServerTask server = new ServerTask();
                server.listenConnection();
            }
        }.start();

        // Need to reset system properties when shutdown ipo to avoid receiving unexcepted
        // intetnt in case of IPO boot up.
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        intentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED);
        context.registerReceiver(mReceiver, intentFilter);

        Rlog.d(TAG, "construtor is called - end");
    }

    public static ExternalSimManager make(Context context, CommandsInterface []ci) {
        if (sInstance == null) {
            sInstance = new ExternalSimManager(context, ci);
        }

        return sInstance;
    }

    private static String truncateString(String original) {
        if (original == null || original.length() < 6) {
            return original;
        }
        return original.substring(0, 2) + "***" + original.substring(original.length() - 4);
    }

    private static IMtkTelephonyEx getITelephonyEx() {
        return IMtkTelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"));
    }

    private void sendCapabilityDoneEvent() {
        VsimEvent event = new VsimEvent(
                0,
                ExternalSimConstants.MSG_ID_CAPABILITY_SWITCH_DONE,
                -1);

        Message msg = new Message();
        msg.obj = event;
        mEventHandler.sendMessage(msg);

        Rlog.d(TAG, "sendCapabilityDoneEvent....");
    }

    // Need to reset system properties when shutdown ipo to avoid receiving unexcepted
    // intetnt in case of IPO boot up.
    private final BroadcastReceiver mReceiver = new  BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Rlog.d(TAG,"[Receiver]+");
            String action = intent.getAction();
            Rlog.d(TAG,"Action: " + action);

            if (action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE) &&
                    mSetCapabilityDone == SET_CAPABILITY_ONGOING) {

                if (mEventHandler.getVsimSlotId(ExternalSimConstants.SIM_TYPE_REMOTE_SIM)
                        == RadioCapabilitySwitchUtil.getMainCapabilityPhoneId()) {

                    synchronized (mLock) {
                        mSetCapabilityDone = SET_CAPABILITY_DONE;
                        sendCapabilityDoneEvent();
                    }

                    Rlog.d(TAG,"SET_CAPABILITY_DONE, notify all");
                } else {
                    // Retry switching modem capability
                    if (mEventHandler.switchModemCapability(sPreferedRsimSlot) == false) {
                        synchronized (mLock) {
                            mSetCapabilityDone = SET_CAPABILITY_FAILED;
                            sendCapabilityDoneEvent();
                        }
                    }
                }
            } else if (action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED) &&
                    mSetCapabilityDone == SET_CAPABILITY_ONGOING) {

                synchronized (mLock) {
                    mSetCapabilityDone = SET_CAPABILITY_FAILED;
                    sendCapabilityDoneEvent();
                }

                Rlog.d(TAG,"SET_CAPABILITY_FAILED, notify all");
            }
            Rlog.d(TAG,"[Receiver]-");
        }
    };

    public boolean initializeService(byte[] userData) {
        Rlog.d(TAG, "initializeService() - start");

        if (SystemProperties.getInt("ro.vendor.mtk_external_sim_support", 0) == 0) {
            Rlog.d(TAG, "initializeService() - mtk_external_sim_support didn't support");
            return false;
        }

        SystemProperties.set("ctl.start", "osi");

        Rlog.d(TAG, "initializeService() - end");
        return true;
    }

    public boolean finalizeService(byte[] userData) {
        Rlog.d(TAG, "finalizeService() - start");

        if (SystemProperties.getInt("ro.vendor.mtk_external_sim_support", 0) == 0) {
            Rlog.d(TAG, "initializeService() - mtk_external_sim_support didn't support");
            return false;
        }

        SystemProperties.set("ctl.stop", "osi");

        Rlog.d(TAG, "finalizeService() - end");
        return true;
    }

    public void setVsimProperty(int phoneId, String property, String value) {
        TelephonyManager.getDefault().setTelephonyProperty(phoneId, property, value);

        String newValue = TelephonyManager.getDefault().getTelephonyProperty(
                phoneId, property, "");

        do {
            try {
                Thread.sleep(50);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            newValue = TelephonyManager.getDefault().getTelephonyProperty(
                    phoneId, property, "");
        } while (!newValue.equals(value));
    }

    public void initVsimConfiguration() {
        sPreferedRsimSlot = SystemProperties.getInt(PREFERED_RSIM_SLOT, -1);
        sPreferedAkaSlot = SystemProperties.getInt(PREFERED_AKA_SIM_SLOT, -1);
    }


    public static boolean isNonDsdaRemoteSimSupport() {
        if (/*DataConnectionHelper.isMultiPsAttachSupport() &&*/
                SystemProperties.getInt("ro.vendor.mtk_non_dsda_rsim_support", 0) == 1) {
            return true;
        }

        return false;
    }

    public static boolean isSupportVsimHotPlugOut() {
        for (int i = 0; i < TelephonyManager.getDefault().getSimCount(); i++) {
            String capability = TelephonyManager.getDefault().getTelephonyProperty(
                    i , "vendor.gsm.modem.vsim.capability", "0");
            if (capability != null && capability.length() > 0 && !"0".equals(capability)) {
                if ((Integer.parseInt(capability)
                        & ExternalSimConstants.MODEM_VSIM_CAPABILITYY_HOTSWAP) > 0) {
                    return true;
                }
            }
        }
        return false;
    }

    public static int getPreferedRsimSlot() {
        return sPreferedRsimSlot;
    }

    /**
     * Return true if there are vsim only slots or any vsim is enabled and inserted
     */
    public static boolean isAnyVsimEnabled() {
        int vsimOnly = SystemProperties.getInt("ro.vendor.mtk_external_sim_only_slots", 0);
        if (vsimOnly != 0) {
            return true;
        }
        for (int i = 0; i < TelephonyManager.getDefault().getSimCount(); i++) {
            String enable = TelephonyManager.getDefault().getTelephonyProperty(
                    i , MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_ENABLED, "0");
            String inserted = TelephonyManager.getDefault().getTelephonyProperty(
                    i , MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");
            if (enable != null && enable.length() > 0 && !"0".equals(enable)
                    && inserted != null && inserted.length() > 0 && !"0".equals(inserted)) {
                return true;
            }
        }
        return false;
    }

    public void startRecoveryTimer() {
        for (int i = 0; i < TelephonyManager.getDefault().getSimCount(); i++) {
            String persist = TelephonyManager.getDefault().getTelephonyProperty(
                    i , MtkTelephonyProperties.PROPERTY_PERSIST_EXTERNAL_SIM, "0");
            if (persist != null && persist.length() > 0 && String.valueOf(
                    ExternalSimConstants.PERSIST_TYPE_ENABLED_WITH_TIMEOUT).equals(persist)) {
                if (mRecoveryTimer == null) {
                    mRecoveryTimer = new Timer();

                    int timout = RECOVERY_TO_REAL_SIM_TIMEOUT;

                    String userTimeout = TelephonyManager.getDefault().getTelephonyProperty(
                        i,
                        MtkTelephonyProperties.PROPERTY_PERSIST_EXTERNAL_SIM_TIMEOUT,
                        String.valueOf(RECOVERY_TO_REAL_SIM_TIMEOUT));

                    try {
                        if (Integer.parseInt(userTimeout) > 0) {
                            timout = Integer.parseInt(userTimeout) * 1000;
                        }
                    } catch (NumberFormatException e) {
                        e.printStackTrace();
                    }

                    mRecoveryTimer.schedule(
                            new RecoveryRealSimTask(), timout);
                    Rlog.i(TAG, "startRecoveryTimer: " + timout + " ms.");
                }
                return;
            }
        }
        Rlog.i(TAG, "No need to startRecoveryTimer since didn't set persist VSIM.");
    }

    public void stopRecoveryTimer() {
        if (mRecoveryTimer != null) {
            mRecoveryTimer.cancel();
            mRecoveryTimer.purge();
            mRecoveryTimer = null;
            Rlog.i(TAG, "stopRecoveryTimer.");
        }
    }

    public class RecoveryRealSimTask extends TimerTask {
        public void run() {
            for (int i = 0; i < TelephonyManager.getDefault().getSimCount(); i++) {
                String enabled = TelephonyManager.getDefault().getTelephonyProperty(
                        i , MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_ENABLED, "0");
                if (enabled != null && enabled.length() > 0 && !"0".equals(enabled)) {
                    Rlog.i(TAG, "Auto recovery time out, disable VSIM...");
                    sendDisableEvent(1 << i, ExternalSimConstants.SIM_TYPE_LOCAL_SIM);
                }
            }
            if (!isSupportVsimHotPlugOut()) {
                for (int i = 0; i < TelephonyManager.getDefault().getSimCount();) {
                    String enabled = TelephonyManager.getDefault().getTelephonyProperty(
                            i , MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_ENABLED, "0");
                    if (enabled != null && enabled.length() > 0 && !"0".equals(enabled)) {
                        try {
                            Thread.sleep(100);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    } else {
                        i++;
                    }
                }
                disableAllVsimWithResetModem();
            }
        }
    };

    public void disableAllVsimWithResetModem() {
        // Set system property to note that sim enabled
        for (int i = 0; i < TelephonyManager.getDefault().getSimCount(); i++) {
            waitRildSetDisabledProperty(i);
        }

        if (mEventHandler != null) {
            mEventHandler.retryIfRadioUnavailable(null);
        }

        // TODO: should disable VSIM by hot plug if supported.
        RadioManager.getInstance().setSilentRebootPropertyForAllModem("1");
        MtkUiccController uiccCtrl =
                (MtkUiccController) UiccController.getInstance();
        uiccCtrl.resetRadioForVsim();
        Rlog.i(TAG, "disableAllVsimWithResetModem...");
    }

    public void sendDisableEvent(int slotId, int simType) {
        VsimEvent disableEvent = new VsimEvent(
                0,
                ExternalSimConstants.MSG_ID_EVENT_REQUEST,
                slotId);

        // event id
        disableEvent.putInt(ExternalSimConstants.REQUEST_TYPE_DISABLE_EXTERNAL_SIM);
        // sim type
        disableEvent.putInt(simType);

        Message msg = new Message();
        msg.obj = disableEvent;
        mEventHandler.sendMessage(msg);

        Rlog.i(TAG, "sendDisableEvent[" + slotId + "]....");
    }

    private void sendExternalSimConnectedEvent(int connected) {
        VsimEvent connectedEvent = new VsimEvent(
                0,
                ExternalSimConstants.MSG_ID_EVENT_REQUEST,
                0);
        // event id
        connectedEvent.putInt(ExternalSimConstants.EVENT_TYPE_EXTERNAL_SIM_CONNECTED);
        // reuse sim_type as connected flag
        connectedEvent.putInt(connected);

        Message msg = new Message();
        msg.obj = connectedEvent;
        mEventHandler.sendMessage(msg);

        Rlog.i(TAG, "sendExternalSimConnectedEvent connected=" + connected);
    }

    private void waitRildSetDisabledProperty(int slotId) {
        String enabled = TelephonyManager.getDefault().getTelephonyProperty(
                slotId, MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_ENABLED, "0");
        while (enabled != null && enabled.length() > 0 && !"0".equals(enabled)) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            enabled = TelephonyManager.getDefault().getTelephonyProperty(
                    slotId, MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_ENABLED, "0");
        }
    }

    /**
     * Maintain a server task to provide extenal client to connect to do
     * some external SIM operation.
     *
     */
    public class ServerTask {
        public static final String HOST_NAME = "vsim-adaptor";
        private VsimIoThread ioThread = null;

        public void listenConnection() {
            Rlog.d(TAG, "listenConnection() - start");

            LocalServerSocket serverSocket = null;
            ExecutorService threadExecutor = Executors.newCachedThreadPool();

            try {
                // Create server socket
                serverSocket = new LocalServerSocket(HOST_NAME);

                while(true) {
                    // Allow multiple connection connect to server.
                    LocalSocket socket = serverSocket.accept();
                    Rlog.i(TAG, "There is a client is accpted: " + socket.toString());
                    stopRecoveryTimer();
                    sendExternalSimConnectedEvent(1);
                    threadExecutor.execute(new ConnectionHandler(socket, mEventHandler));
                }
            } catch (IOException e) {
                Rlog.w(TAG, "listenConnection catch IOException");
                e.printStackTrace();
            } catch (Exception e) {
                Rlog.w(TAG, "listenConnection catch Exception");
                e.printStackTrace();
            } finally {
                Rlog.d(TAG, "listenConnection finally!!");
                if (threadExecutor != null )
                    threadExecutor.shutdown();
                if (serverSocket != null) {
                    try {
                        serverSocket.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
            Rlog.d(TAG, "listenConnection() - end");
        }
    }

    public class ConnectionHandler implements Runnable {
        private LocalSocket mSocket;
        private VsimEvenHandler mEventHandler;
        public static final String RILD_SERVER_NAME = "rild-vsim";

        public ConnectionHandler(LocalSocket clientSocket, VsimEvenHandler eventHandler) {
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
                VsimIoThread ioThread = new VsimIoThread(
                        ServerTask.HOST_NAME,
                        mSocket.getInputStream(),
                        mSocket.getOutputStream(),
                        mEventHandler);

                mEventHandler.setDataStream(ioThread, null);

                // Start after setDataStream done to avoid null IO thread cause unexcepted behavior.
                if (ioThread != null) ioThread.start();

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static class VsimEvent {
        public static final int DEFAULT_MAX_DATA_LENGTH = 512;
        private int mTransactionId;
        private int mMessageId;
        private int mSlotId;
        private int mDataLen;
        private int mReadOffset;
        private byte mData[];
        private int mEventMaxDataLen = DEFAULT_MAX_DATA_LENGTH;

        /**
         * The VsimEvent constructor with specified phone Id.
         *
         * @param transactionId event serial number, use to determine a pair of request & response.
         * @param messageId message event id
         */
        public VsimEvent(int transactionId, int messageId) {
            this(transactionId, messageId, 0);
        }

        /**
         * The VsimEvent constructor with specified phone Id.
         *
         * @param transactionId event serial number, use to determine a pair of request & response.
         * @param messageId message event id
         * @param slotId the indicated slotId
         */
        public VsimEvent(int transactionId, int messageId, int slotId) {
            this(transactionId, messageId, DEFAULT_MAX_DATA_LENGTH, slotId);
        }

        /**
         * The VsimEvent constructor with specified phone Id.
         *
         * @param transactionId event serial number, use to determine a pair of request & response.
         * @param messageId message event id
         * @param length the max data length of the event
         * @param slotId the indicated slotId
         */
        public VsimEvent(int transactionId, int messageId, int length, int slotId) {
            mTransactionId = transactionId;
            mMessageId = messageId;
            mSlotId = slotId;
            mEventMaxDataLen = length;
            mData = new byte[mEventMaxDataLen];
            mDataLen = 0;
            mReadOffset = 0;
        }

        public void resetOffset() {
            synchronized (this) {
                mReadOffset = 0;
            }
        }

        public int putInt(int value) {
            synchronized (this) {
                if (mDataLen > mEventMaxDataLen - 4) {
                    return -1;
                }

                for (int i = 0 ; i < 4 ; ++i) {
                    mData[mDataLen] = (byte) ((value >> (8 * i)) & 0xFF);
                    mDataLen++;
                }
            }
            return 0;
        }

        public int putShort(int value) {
            synchronized (this) {
                if (mDataLen > mEventMaxDataLen - 2) {
                    return -1;
                }

                for (int i = 0 ; i < 2 ; ++i) {
                    mData[mDataLen] = (byte) ((value >> (8 * i)) & 0xFF);
                    mDataLen++;
                }
            }
            return 0;
        }

        public int putByte(int value) {
            synchronized (this) {
                if (mDataLen > mEventMaxDataLen - 1) {
                    return -1;
                }

                mData[mDataLen] = (byte) (value & 0xFF);
                mDataLen++;
            }
            return 0;
        }

        public int putString(String str, int len) {
            synchronized (this) {
                if (mDataLen > mEventMaxDataLen - len) {
                    return -1;
                }

                byte s[] = str.getBytes();
                if (len < str.length()) {
                    System.arraycopy(s, 0, mData, mDataLen, len);
                    mDataLen += len;
                } else {
                    int remain = len - str.length();
                    System.arraycopy(s, 0, mData, mDataLen, str.length());
                    mDataLen += str.length();
                    for (int i = 0 ; i < remain ; i++) {
                        mData[mDataLen] = 0;
                        mDataLen++;
                    }
                }
            }
            return 0;
        }

        public int putBytes(byte [] value) {
            synchronized (this) {
                int len = value.length;

                if (len > mEventMaxDataLen) {
                    return -1;
                }

                System.arraycopy(value, 0, mData, mDataLen, len);
                mDataLen += len;
            }
            return 0;
        }

        public byte [] getData() {
            synchronized (this) {
                byte tempData[] = new byte[mDataLen];
                System.arraycopy(mData, 0, tempData, 0, mDataLen);
                return tempData;
            }
        }

        public int getDataLen() {
            synchronized (this) {
                return mDataLen;
            }
        }

        public byte [] getDataByReadOffest() {
            synchronized (this) {
                byte tempData[] = new byte[mDataLen - mReadOffset];
                System.arraycopy(mData, mReadOffset, tempData, 0, mDataLen - mReadOffset);
                return tempData;
            }
        }

        public int getMessageId() {
            return mMessageId;
        }

        /*
         * Return slot bit mask.
         * 1 means slot0,
         * 2 means slot1,
         * 3 means slot 0 and slot 1.
         */
        public int getSlotBitMask() {
            return mSlotId;
        }

        /*
         * Return the first mapping slot of slot bit mask value.
         */
        public int getFirstSlotId() {
            int simCount = TelephonyManager.getDefault().getSimCount();

            if (getSlotBitMask() > (1 << simCount -1 )) {
                Rlog.w(TAG, "getFirstSlotId, invalid slot id: " + getSlotBitMask());
                return 0;
            }

            for (int i = 0; i < simCount; i++) {
                if ((getSlotBitMask() & (1 << i)) != 0) {
                    return i;
                }
            }
            Rlog.w(TAG, "getFirstSlotId, invalid slot id: " + getSlotBitMask());
            return 0;
        }

        public int getTransactionId() {
            return mTransactionId;
        }

        public int getInt() {
            int ret = 0;
            synchronized (this) {
                if (mData.length >= 4) {
                    ret = ((mData[mReadOffset + 3] & 0xff) << 24 |
                           (mData[mReadOffset + 2] & 0xff) << 16 |
                        (mData[mReadOffset + 1] & 0xff) << 8 |
                        (mData[mReadOffset] & 0xff));
                    mReadOffset += 4;
                }
            }
            return ret;
        }

        public int getShort() {
            int ret = 0;
            synchronized (this) {
                ret =  ((mData[mReadOffset + 1] & 0xff) << 8 | (mData[mReadOffset] & 0xff));
                mReadOffset += 2;
            }
            return ret;
        }

        // Notice: getByte is to get int8 type from VA, not get one byte.
        public int getByte() {
            int ret = 0;
            synchronized (this) {
                ret = (mData[mReadOffset] & 0xff);
                mReadOffset += 1;
            }
            return ret;
        }

        public byte[] getBytes(int length) {
            synchronized (this) {
                if (length > mDataLen - mReadOffset) {
                    return null;
                }

                byte[] ret = new byte[length];

                for (int i = 0 ; i < length ; i++) {
                    ret[i] = mData[mReadOffset];
                    mReadOffset++;
                }
                return ret;
            }
        }

        public String getString(int len) {
            byte buf [] = new byte[len];

            synchronized (this) {
                System.arraycopy(mData, mReadOffset, buf, 0, len);
                mReadOffset += len;
            }

            return (new String(buf)).trim();
        }

        public String toString() {
            return new String("dumpEvent: transaction_id: " + getTransactionId()
                    + ", message_id:" + getMessageId()
                    + ", slot_id:" + getSlotBitMask()
                    + ", data_len:" + getDataLen()
                    + ", event:" + truncateString(IccUtils.bytesToHexString(getData())));
        }
    }


    class VsimIoThread extends Thread {
        private String mName = "";
        private static final int MAX_DATA_LENGTH = (20 * 1024);
        private boolean mIsContinue = true;
        private DataInputStream mInput = null;
        private DataOutputStream mOutput = null;
        private VsimEvenHandler mEventHandler = null;
        private String mServerName = "";

        private byte[] readBuffer = null;

        public VsimIoThread(
                String name,
                InputStream inputStream,
                OutputStream outputStream,
                VsimEvenHandler eventHandler) {
            mName = name;
            mInput = new DataInputStream(inputStream);
            mOutput = new DataOutputStream(outputStream);
            mEventHandler = eventHandler;
            logd("VsimIoThread constructor is called.");
        }

        public void terminate() {
            logd("VsimIoThread terminate.");
            mIsContinue = false;
        }

        public void run() {
            logd("VsimIoThread running.");
            while (mIsContinue) {
                try {
                    //if (mInput.available() > 0) {
                        VsimEvent event = readEvent();
                        // Need to handle the event
                        if (event != null) {
                            Message msg = new Message();
                            msg.obj = event;
                            mEventHandler.sendMessage(msg);
                        }
                    //} else if (!mSocket.isConnected()){
                    //    log("VsimIoThread mSocket is disconnected!");
                    //}
                } catch (IOException e) {
                    logw("VsimIoThread IOException.");
                    e.printStackTrace();

                    if (!mServerName.equals("")) {
                        //createClientSocket(mServerName);
                    } else {
                        // Means the client socket has been disconnected.
                        for (int i = 0; i < TelephonyManager.getDefault().getSimCount(); i++) {
                            String enabled = TelephonyManager.getDefault().getTelephonyProperty(
                                    i, MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_ENABLED,
                                    "0");
                            String insert = TelephonyManager.getDefault().getTelephonyProperty(
                                    i, MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED,
                                    "0");

                            if (enabled != null && enabled.length() > 0 &&
                                    !"0".equals(enabled)) {

                                if (insert == null || insert.length() <= 0) {
                                    insert = "0";
                                }

                                sendDisableEvent(1 << i, Integer.valueOf(insert));

                                logi("Disable VSIM and reset modem since socket disconnected.");
                            }
                        }
                        sendExternalSimConnectedEvent(0);
                        logw("Socket disconnected and vsim is disabled.");
                        terminate();
                    }
                } catch (Exception e) {
                    logw("VsimIoThread Exception.");
                    e.printStackTrace();
                }
            }
        }

        private void writeBytes(byte [] value, int len) throws IOException {
            mOutput.write(value, 0, len);
        }

        private void writeInt(int value) throws IOException {
            for (int i = 0 ; i < 4 ; ++i) {
                mOutput.write((value >> (8 * i)) & 0xff);
            }
        }

        public int writeEvent(VsimEvent event) {
            return writeEvent(event, false);
        }

        public int writeEvent(VsimEvent event, boolean isBigEndian) {
            logd("writeEvent Enter, isBigEndian:" + isBigEndian);
            int ret = -1;
            try {
                synchronized (this) {
                    if (mOutput != null) {
                        dumpEvent(event);

                        writeInt(event.getTransactionId());
                        writeInt(event.getMessageId());
                        // Platfrom slot id start from 0, so need to add 1.
                        writeInt(event.getSlotBitMask());
                        writeInt(event.getDataLen());
                        writeBytes(event.getData(), event.getDataLen());
                        mOutput.flush();
                        ret = 0;
                    } else {
                        loge("mOut is null, socket is not setup");
                    }
                }
            } catch (Exception e) {
                loge("writeEvent Exception");
                e.printStackTrace();
                return -1;
            }

            return ret;
        }

        /**
         * DataInputStream's readInt is Big-Endian method.
         */
        private int readInt() throws IOException {
            byte[] tempBuf = new byte[8];
            int readCount = mInput.read(tempBuf, 0, 4);
            if (readCount < 0) {
                loge("readInt(), fail to read and throw exception");
                throw new IOException("fail to read");
            }

            return ((tempBuf[3]) << 24 |
                    (tempBuf[2] & 0xff) << 16 |
                    (tempBuf[1] & 0xff) << 8 |
                    (tempBuf[0] & 0xff));
        }

        private VsimEvent readEvent() throws IOException {
            logd("readEvent Enter");

            int transaction_id = readInt();
            int msg_id = readInt();
            int slot_id = readInt();
            int data_len = readInt();
            logd("readEvent transaction_id: " + transaction_id +
                    ", msgId: " + msg_id + ", slot_id: " + slot_id + ", len: " + data_len);

            if (data_len > MAX_VSIM_UICC_CMD_LEN) {
                loge("readEvent(), data_len large than " + MAX_VSIM_UICC_CMD_LEN);
                throw new IOException("unreasonable data length");
            }

            readBuffer = new byte[data_len];

            int offset = 0;
            int remaining = data_len;

            do {
                int countRead = mInput.read(readBuffer, offset, remaining);

                if (countRead < 0) {
                    loge("readEvent(), fail to read and throw exception");
                    throw new IOException("fail to read");
                }

                offset += countRead;
                remaining -= countRead;
            } while (remaining > 0);

            VsimEvent event = new VsimEvent(transaction_id, msg_id, data_len, slot_id);
            event.putBytes(readBuffer);

            dumpEvent(event);
            return event;
        }

        private void dumpEvent(VsimEvent event) {
            if (ENG) {
                logd("dumpEvent: transaction_id: " + event.getTransactionId()
                        + ", message_id:" + event.getMessageId()
                        + ", slot_id:" + event.getSlotBitMask()
                        + ", data_len:" + event.getDataLen()
                        + ", event:" + truncateString(IccUtils.bytesToHexString(event.getData())));
            } else {
                logd("dumpEvent: transaction_id: " + event.getTransactionId()
                        + ", message_id:" + event.getMessageId()
                        + ", slot_id:" + event.getSlotBitMask()
                        + ", data_len:" + event.getDataLen());
            }
        }

        private void logd(String s) {
            Rlog.d(TAG, "[" + mName + "] " + s);
        }

        private void logi(String s) {
            Rlog.i(TAG, "[" + mName + "] " + s);
        }

        private void logw(String s) {
            Rlog.w(TAG, "[" + mName + "] " + s);
        }

        private void loge(String s) {
            Rlog.e(TAG, "[" + mName + "] " + s);
        }
    }

    public class VsimIndEventHandler extends Handler {
        /*public VsimIndEventHandler() {

        }*/

        protected Integer getCiIndex(Message msg) {
            AsyncResult ar;
            Integer index = new Integer(PhoneConstants.DEFAULT_CARD_INDEX);

            /*
             * The events can be come in two ways. By explicitly sending it using
             * sendMessage, in this case the user object passed is msg.obj and from
             * the CommandsInterface, in this case the user object is msg.obj.userObj
             */
            if (msg != null) {
                if (msg.obj != null && msg.obj instanceof Integer) {
                    index = (Integer)msg.obj;
                } else if(msg.obj != null && msg.obj instanceof AsyncResult) {
                    ar = (AsyncResult)msg.obj;
                    if (ar.userObj != null && ar.userObj instanceof Integer) {
                        index = (Integer)ar.userObj;
                    }
                }
            }
            return index;
        }

        @Override
        public void handleMessage(Message msg) {
             Integer index = getCiIndex(msg);

             if (index < 0 || index >= mCi.length) {
                 Rlog.e(TAG, "Invalid index : " + index + " received with event " + msg.what);
                 return;
             }

             AsyncResult ar = (AsyncResult)msg.obj;
             switch (msg.what) {
                 case EVENT_VSIM_INDICATION:
                     if (ENG) Rlog.d(TAG, "Received EVENT_VSIM_INDICATION...");
                     VsimEvent indicationEvent = (VsimEvent)ar.result;
                     dumpEvent(indicationEvent);

                     Message vsimMsg = new Message();
                     vsimMsg.obj = indicationEvent;
                     mEventHandler.sendMessage(vsimMsg);
                     break;
                 default:
                     Rlog.e(TAG, " Unknown Event " + msg.what);
             }
        }

        private void dumpEvent(VsimEvent event) {
            if (ENG) {
                Rlog.d(TAG, "dumpEvent: transaction_id: " + event.getTransactionId()
                        + ", message_id:" + event.getMessageId()
                        + ", slot_id:" + event.getSlotBitMask()
                        + ", data_len:" + event.getDataLen()
                        + ", event:" + truncateString(IccUtils.bytesToHexString(event.getData())));
            } else {
                Rlog.d(TAG, "dumpEvent: transaction_id: " + event.getTransactionId()
                        + ", message_id:" + event.getMessageId()
                        + ", slot_id:" + event.getSlotBitMask()
                        + ", data_len:" + event.getDataLen());
            }
        }
    }

    public class VsimEvenHandler extends Handler {
        private VsimIoThread mVsimAdaptorIo = null;
        private VsimIoThread mVsimRilIo = null;
        private boolean mHasNotifyEnableEvnetToModem = false;
        private boolean mIsSwitchRfSuccessful = false;
        private boolean mIsAkaOccupyRf = false;
        private boolean[] mIsMdWaitingResponse = null;
        private VsimEvent[] mWaitingEvent = null;
        private Timer[] mNoResponseTimer = null;
        private boolean[] mIsWaitingAuthRsp = null;
        private int[] mNoResponseTimeOut = null;
        private eventHandlerTread[] mEventHandlingThread = null;
        private long mLastDisableEventTime = 0;

        public VsimEvenHandler() {
            int simCount = TelephonyManager.getDefault().getSimCount();
            mIsMdWaitingResponse = new boolean[simCount];
            mNoResponseTimer = new Timer[simCount];
            mWaitingEvent = new VsimEvent[simCount];
            mIsWaitingAuthRsp = new boolean[simCount];
            mNoResponseTimeOut = new int[simCount];
            mEventHandlingThread = new eventHandlerTread[simCount];

            for (int i = 0; i < simCount; i++) {
                mIsMdWaitingResponse[i] = false;
                mNoResponseTimer[i] = null;
                mWaitingEvent[i] = null;
                mIsWaitingAuthRsp[i] = false;
                mNoResponseTimeOut[i] = NO_RESPONSE_TIMEOUT_DURATION;
                mEventHandlingThread[i] = null;
            }
        }

        @Override
        public void handleMessage(Message msg) {
            VsimEvent event = null;
            if (msg.obj instanceof AsyncResult) {
                AsyncResult ar = (AsyncResult)msg.obj;
                event = (VsimEvent) ar.userObj;
            } else {
                event = (VsimEvent) msg.obj;
            }

            int slotId = event.getFirstSlotId();

            if (slotId >=0 && slotId < TelephonyManager.getDefault().getSimCount()) {
                while (mEventHandlingThread[slotId] != null
                        && mEventHandlingThread[slotId].isWaiting()) {

                    Rlog.d(TAG, "handleMessage[" + slotId +"] thread running, delay 100 ms...");

                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                mEventHandlingThread[slotId] = new eventHandlerTread((VsimEvent) event);
                mEventHandlingThread[slotId].start();
            } else {
                // If there is no invalid slot id, always new thread to handle it.
                new eventHandlerTread((VsimEvent) msg.obj).start();
            }
        }

        private void setDataStream(VsimIoThread vsimAdpatorIo, VsimIoThread vsimRilIo) {
            mVsimAdaptorIo = vsimAdpatorIo;
            mVsimRilIo = vsimRilIo;
            Rlog.d(TAG, "VsimEvenHandler setDataStream done.");
        }

        private int getRspMessageId(int requestMsgId) {
            switch (requestMsgId) {
                case ExternalSimConstants.MSG_ID_INITIALIZATION_REQUEST:
                    return ExternalSimConstants.MSG_ID_INITIALIZATION_RESPONSE;
                case ExternalSimConstants.MSG_ID_EVENT_REQUEST:
                    return ExternalSimConstants.MSG_ID_EVENT_RESPONSE;
                case ExternalSimConstants.MSG_ID_GET_PLATFORM_CAPABILITY_REQUEST:
                    return ExternalSimConstants.MSG_ID_GET_PLATFORM_CAPABILITY_RESPONSE;
                case ExternalSimConstants.MSG_ID_UICC_RESET_REQUEST:
                    return ExternalSimConstants.MSG_ID_UICC_RESET_RESPONSE;
                case ExternalSimConstants.MSG_ID_UICC_APDU_REQUEST:
                case ExternalSimConstants.MSG_ID_UICC_AUTHENTICATION_REQUEST_IND:
                    return ExternalSimConstants.MSG_ID_UICC_APDU_RESPONSE;
                case ExternalSimConstants.MSG_ID_UICC_POWER_DOWN_REQUEST:
                    return ExternalSimConstants.MSG_ID_UICC_POWER_DOWN_RESPONSE;
                case ExternalSimConstants.MSG_ID_GET_SERVICE_STATE_REQUEST:
                    return ExternalSimConstants.MSG_ID_GET_SERVICE_STATE_RESPONSE;
                case ExternalSimConstants.MSG_ID_FINALIZATION_REQUEST:
                    return ExternalSimConstants.MSG_ID_FINALIZATION_RESPONSE;
                default:
                    Rlog.d(TAG, "getRspMessageId: " + requestMsgId + "no support.");

            }
            return -1;
        }

        public class eventHandlerTread extends Thread {
            VsimEvent mEvent = null;
            boolean isWaiting = true;

            public eventHandlerTread(VsimEvent event) {
                mEvent = event;
            }

            public boolean isWaiting() {
                return isWaiting;
            }

            public void setWaiting(boolean waiting) {
                isWaiting = waiting;
            }

            public void run() {
                Rlog.d(TAG, "eventHandlerTread[ " + mEvent.getFirstSlotId() + "]: run...");
                dispatchCallback(mEvent);
                isWaiting = false;
            }
        }

        private Runnable mTryResetModemRunnable = new Runnable() {
            public void run() {
                MtkUiccController uiccCtrl = (MtkUiccController) UiccController.getInstance();
                if (uiccCtrl.isAllRadioAvailable()) {
                    RadioManager.getInstance().setSilentRebootPropertyForAllModem("1");
                    uiccCtrl.resetRadioForVsim();

                    Rlog.i(TAG, "mTryResetModemRunnable reset modem done.");
                } else {
                    postDelayed(mTryResetModemRunnable, TRY_RESET_MODEM_DURATION);
                }
            }
        };

        public class TimeOutTimerTask extends TimerTask {
            int mSlotId = 0;
            public TimeOutTimerTask (int slotId) {
                mSlotId = slotId;
            }

            public void run() {
                synchronized (mLock) {
                    // send no response result to modem.
                    if (mWaitingEvent[mSlotId] != null) {
                        sendNoResponseError(mWaitingEvent[mSlotId]);
                    }
                    Rlog.i(TAG, "TimeOutTimerTask[" + mSlotId +
                            "] time out and send response to modem directly.");
                }
            }
        };

        private void sendNoResponseError(VsimEvent event) {
            // [VSIM 3.0] If isWaitingAuthRsp flg is true, means that we should
            // release RF occupy before send auth response to mode.
            if (mIsWaitingAuthRsp[event.getFirstSlotId()] == true) {
                mIsWaitingAuthRsp[event.getFirstSlotId()] = false;
                sendRsimAuthProgressEvent(
                        ExternalSimConstants.EVENT_TYPE_RECEIVE_RSIM_AUTH_RSP);
            }

            // Set flag first to avoid timing issue between modem and AP event handling
            if (getMdWaitingFlag(event.getFirstSlotId()) == true) {
                setMdWaitingFlag(false, event.getFirstSlotId());

                VsimEvent response = new VsimEvent(
                        event.getTransactionId(),
                        getRspMessageId(event.getMessageId()),
                        event.getSlotBitMask());

                // Set result to invalid status
                response.putInt(ExternalSimConstants.RESPONSE_RESULT_GENERIC_ERROR);

                // Set no response status word
                response.putInt(2); // length
                response.putByte(NO_RESPONSE_STATUS_WORD_BYTE1);
                response.putByte(NO_RESPONSE_STATUS_WORD_BYTE2);

                ((MtkRIL) mCi[event.getFirstSlotId()]).sendVsimOperation(
                        response.getTransactionId(),
                        response.getMessageId(),
                        response.getInt(),
                        response.getInt(),
                        response.getDataByReadOffest(), null);
            }
        }

        private void sendVsimNotification(
            int slotId, int transactionId, int eventId, int simType, Message message) {

            boolean result = ((MtkRIL) mCi[slotId]).sendVsimNotification(
                    transactionId, eventId, simType, message);

            Rlog.d(TAG, "sendVsimNotification result = " + result);

            if (message == null) {
                int timeOut = 0;
                while (!result && timeOut < PLUG_IN_AUTO_RETRY_TIMEOUT) {
                    try {
                        Thread.sleep(AUTO_RETRY_DURATION);
                        timeOut += AUTO_RETRY_DURATION;
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }

                    result = ((MtkRIL) mCi[slotId]).sendVsimNotification(
                            transactionId, eventId, simType, message);
                }
            }

            if (!result) {
                Rlog.e(TAG, "sendVsimNotification fail until " + PLUG_IN_AUTO_RETRY_TIMEOUT);
            }
        }

        private int sendSetRsimMappingInfoSync(int slotId, int transactionId)
                throws InterruptedException {
            VsimEvent event = new VsimEvent(
                    0,
                    ExternalSimConstants.MSG_ID_EVENT_RESPONSE,
                    1 << slotId);
            // event id
            event.putInt(ExternalSimConstants.REQUEST_TYPE_SET_MAPPING_INFO);
            // sim type
            event.putInt(ExternalSimConstants.SIM_TYPE_REMOTE_SIM);

            Message msg = new Message();
            msg.obj = event;
            msg.setTarget(mEventHandler);
            boolean result = ((MtkRIL)mCi[slotId]).sendVsimNotification(
                    transactionId, ExternalSimConstants.REQUEST_TYPE_SET_MAPPING_INFO,
                    ExternalSimConstants.SIM_TYPE_REMOTE_SIM, msg);
            if (result == true) {
                Rlog.d(TAG, "sendSetRsimMappingInfoSync before mLock.wait");
                mLock.wait();
                return ExternalSimConstants.RESPONSE_RESULT_OK;
            } else {
                Rlog.e(TAG, "sendSetRsimMappingInfoSync fail.");
                return ExternalSimConstants.RESPONSE_RESULT_PLATFORM_NOT_READY;
            }
        }

        private void sendPlugOutEvent(VsimEvent event) {
            String isInserted = TelephonyManager.getDefault().getTelephonyProperty(
                      event.getFirstSlotId(),
                      MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");

            if ("0".equals(isInserted)) {
                Rlog.d(TAG, "sendPlugOutEvent: " + isInserted);
            } else {
                VsimEvent plugOutEvent = new VsimEvent(
                        event.getTransactionId(),
                        ExternalSimConstants.MSG_ID_EVENT_REQUEST,
                        event.getSlotBitMask());

                // event id
                plugOutEvent.putInt(ExternalSimConstants.REQUEST_TYPE_PLUG_OUT);
                // sim type
                plugOutEvent.putInt(ExternalSimConstants.SIM_TYPE_LOCAL_SIM);

                setMdWaitingFlag(false, event.getFirstSlotId());

                /*if (mVsimRilIo != null) {
                    mVsimRilIo.writeEvent(plugOutEvent);
                }*/

                //((MtkRIL) mCi[event.getFirstSlotId()]).sendVsimNotification(
                sendVsimNotification(event.getFirstSlotId(),
                        plugOutEvent.mTransactionId,
                        ExternalSimConstants.REQUEST_TYPE_PLUG_OUT,
                        ExternalSimConstants.SIM_TYPE_LOCAL_SIM,
                        null);
            }
        }

        private void sendHotPlugEvent(VsimEvent event, boolean plugIn) {
            int eventId = ExternalSimConstants.REQUEST_TYPE_PLUG_IN;
            if (!plugIn) {
                eventId = ExternalSimConstants.REQUEST_TYPE_PLUG_OUT;
            }

            //((MtkRIL) mCi[event.getFirstSlotId()]).sendVsimNotification(
            sendVsimNotification(event.getFirstSlotId(),
                    event.getTransactionId(),
                    eventId,
                    ExternalSimConstants.SIM_TYPE_LOCAL_SIM,
                    null);
        }

        private int getVsimSlotId(int simType) {
            switch (simType) {
                case ExternalSimConstants.SIM_TYPE_AKA_SIM: {
                    int akaSim = SystemProperties.getInt(PREFERED_AKA_SIM_SLOT, -1);
                    if (akaSim == -1) akaSim = sPreferedAkaSlot; //The property may not be set yet
                    return akaSim;
                }
                case ExternalSimConstants.SIM_TYPE_REMOTE_SIM: {
                    int rSim = SystemProperties.getInt(PREFERED_RSIM_SLOT, -1);
                    if (rSim == -1) rSim = sPreferedRsimSlot; //The property may not be set yet
                    return rSim;
                }
                default: {
                    for (int i = 0; i < TelephonyManager.getDefault().getSimCount(); i++) {
                        String enable = TelephonyManager.getDefault().getTelephonyProperty(
                                i , MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_ENABLED, "0");
                        if (enable != null && enable.length() > 0 && !"0".equals(enable)) {
                            String inserted = TelephonyManager.getDefault().getTelephonyProperty(
                                    i , MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");
                            if (inserted != null && inserted.length() > 0 &&
                                    String.valueOf(simType).equals(inserted)) {
                                return i;
                            }
                        }
                    }
                }
            }

            return -1;
        }

        // FIXME: We can't support RSIM in case of using physical SIM to be AKA-SIM.
        private void sendRsimAuthProgressEvent(int eventId) {
            mIsSwitchRfSuccessful = false;

            int akaSim = getVsimSlotId(ExternalSimConstants.SIM_TYPE_AKA_SIM);
            int rSim = getVsimSlotId(ExternalSimConstants.SIM_TYPE_REMOTE_SIM);

            if ((akaSim < 0 || akaSim > TelephonyManager.getDefault().getSimCount()) ||
                    (rSim < 0 || rSim > TelephonyManager.getDefault().getSimCount())) {
                Rlog.d(TAG, "sendRsimAuthProgressEvent aka sim: " + akaSim + ", rsim: " + rSim);
                mIsSwitchRfSuccessful = true;
                return ;
            }

            // Set waiting auth flag by event id
            if (eventId == ExternalSimConstants.EVENT_TYPE_SEND_RSIM_AUTH_IND) {
                mIsAkaOccupyRf = true;
            } else if (eventId == ExternalSimConstants.EVENT_TYPE_RECEIVE_RSIM_AUTH_RSP) {
                if (mIsAkaOccupyRf == false) {
                    Rlog.d(TAG, "sendRsimAuthProgressEvent, aka didn't occupy rf");
                    return;
                }
                mIsAkaOccupyRf = false;
            }
            Rlog.d(TAG, "sendRsimAuthProgressEvent mIsWaitingAuthRsp[" + rSim +"]: "
                    + mIsWaitingAuthRsp[rSim]);

            VsimEvent event = new VsimEvent(
                    0,
                    ExternalSimConstants.MSG_ID_EVENT_RESPONSE,
                    1 << akaSim);

            // event id
            event.putInt(eventId);
            // sim type
            event.putInt(ExternalSimConstants.SIM_TYPE_LOCAL_SIM);

            Message msg = new Message();
            msg.obj = event;
            msg.setTarget(mEventHandler);

            //((MtkRIL) mCi[event.getFirstSlotId()]).sendVsimNotification(
            sendVsimNotification(event.getFirstSlotId(),
                    event.mTransactionId, eventId,
                    ExternalSimConstants.SIM_TYPE_LOCAL_SIM,
                    msg);

            Rlog.d(TAG, "sendRsimAuthProgressEvent eventId: " + eventId);

            try {
                // Waiting for get response from vsim-rild.
                mLock.wait();
            } catch (InterruptedException e) {
                Rlog.w(TAG, "sendRsimAuthProgressEvent InterruptedException.");
            }
        }

        private void sendActiveAkaSimEvent(int slotId, boolean turnOn) {
            Rlog.d(TAG, "sendActiveAkaSimEvent[" + slotId + "]: " + turnOn);

            // Release RF before turn off AKA SIM stack under auth waiting status
            int rsimSlot = getVsimSlotId(ExternalSimConstants.SIM_TYPE_REMOTE_SIM);
            if (rsimSlot >=0 && mIsWaitingAuthRsp[rsimSlot] == true) {
                if (turnOn == false) {
                    sendRsimAuthProgressEvent(
                            ExternalSimConstants.EVENT_TYPE_RECEIVE_RSIM_AUTH_RSP);
                }
            }

            VsimEvent akaEvent = new VsimEvent(
                    0,
                    ExternalSimConstants.MSG_ID_EVENT_REQUEST,
                    1 << slotId);
            int eventId = 0;

            // event id
            if (turnOn) {
                sPreferedAkaSlot = akaEvent.getFirstSlotId();
                eventId = ExternalSimConstants.REQUEST_TYPE_SET_MAPPING_INFO;
                akaEvent.putInt(ExternalSimConstants.REQUEST_TYPE_SET_MAPPING_INFO);
            } else {
                sPreferedAkaSlot = -1;
                eventId = ExternalSimConstants.EVENT_TYPE_RSIM_AUTH_DONE;
                akaEvent.putInt(ExternalSimConstants.EVENT_TYPE_RSIM_AUTH_DONE);
            }
            // sim type
            akaEvent.putInt(ExternalSimConstants.SIM_TYPE_AKA_SIM);

            //((MtkRIL) mCi[akaEvent.getFirstSlotId()]).sendVsimNotification(
            sendVsimNotification(akaEvent.getFirstSlotId(),
                    akaEvent.mTransactionId, eventId,
                    ExternalSimConstants.SIM_TYPE_AKA_SIM, null);

            // Occupy RF after turn on AKA SIM stack under auth waiting status
            if (rsimSlot >= 0 && mIsWaitingAuthRsp[rsimSlot] == true) {
                if (turnOn == true) {
                    sendRsimAuthProgressEvent(
                            ExternalSimConstants.EVENT_TYPE_SEND_RSIM_AUTH_IND);
                }
            }

        }

        private void setMdWaitingFlag(boolean isWaiting, int slotId) {
            setMdWaitingFlag(isWaiting, null, slotId);
        }

        private void setMdWaitingFlag(boolean isWaiting, VsimEvent event, int slotId) {
            Rlog.d(TAG, "setMdWaitingFlag[" + slotId + "]: " + isWaiting);
            mIsMdWaitingResponse[slotId] = isWaiting;

            if (isWaiting == true) {
                mWaitingEvent[slotId] = event;

                if (mNoResponseTimer[slotId] == null) {
                    mNoResponseTimer[slotId] = new Timer(true);
                }

                String isVsimEnabled =
                        TelephonyManager.getDefault().getTelephonyProperty(
                        ((event != null) ? event.getFirstSlotId() : -1),
                        MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_ENABLED, "0");

                // If received modem event under vsim disabled state,
                // we send error response directly.
                if (("".equals(isVsimEnabled) || "0".equals(isVsimEnabled))) {
                    mNoResponseTimer[slotId].schedule(new TimeOutTimerTask(slotId), 500);

                    // If it is not under IPO shutdown stage, means we should
                    // reset mode to notify VSIM disabled event.
                    // Notes. try reset after TRY_RESET_MODEM_DURATION delay.
                    // Only reset if still receive URC after disabled for 5 seconds
                    if (System.currentTimeMillis() > (mLastDisableEventTime + 5000)) {
                        postDelayed(mTryResetModemRunnable, TRY_RESET_MODEM_DURATION);
                    }

                    Rlog.i(TAG, "recevice modem event under vsim disabled state. lastDisableTime:" +
                            mLastDisableEventTime);
                } else {
                    mNoResponseTimer[slotId].schedule(
                            new TimeOutTimerTask(slotId), mNoResponseTimeOut[slotId]);
                }

            } else {
                if (mNoResponseTimer[slotId] != null) {
                    mNoResponseTimer[slotId].cancel();
                    mNoResponseTimer[slotId].purge();
                    mNoResponseTimer[slotId] = null;
                }
                mWaitingEvent[slotId] = null;
            }
        }

        private boolean getMdWaitingFlag(int slotId) {
            Rlog.d(TAG, "getMdWaitingFlag[" + slotId + "]: " + mIsMdWaitingResponse[slotId]);
            return mIsMdWaitingResponse[slotId];
        }

        private boolean isRsimDataConnected() {
            if (sPreferedRsimSlot < 0) return false;
            PhoneConstants.DataState dataState =
                    PhoneFactory.getPhone(sPreferedRsimSlot).getDataConnectionState();
            Rlog.d(TAG, "rsim data state[" + sPreferedRsimSlot + "]: " + dataState);
            return (dataState == PhoneConstants.DataState.CONNECTED);
        }

        private boolean isPlatformReady(int category) {
            boolean isReady = true;

            switch (category) {
                case PLATFORM_READY_CATEGORY_SUB: {
                    MtkSubscriptionController ctrl =
                            (MtkSubscriptionController)SubscriptionController.getInstance();
                    isReady = ctrl.isReady();
                    break;
                }
                case PLATFORM_READY_CATEGORY_SIM_SWITCH: {
                    MtkProxyController ctrl =
                            (MtkProxyController) ProxyController.getInstance();
                    isReady = !ctrl.isCapabilitySwitching();
                    break;
                }
                case PLATFORM_READY_CATEGORY_RADIO: {
                    MtkUiccController ctrl = (MtkUiccController)UiccController.getInstance();
                    isReady = ctrl.isAllRadioAvailable();
                    break;
                }
                default:
                    Rlog.d(TAG, "isPlatformReady invalid category: " + category);
            }

            return isReady;
        }

        private int retryIfPlatformNotReady(VsimEvent event, int category) {
            int result = ExternalSimConstants.RESPONSE_RESULT_OK;

            boolean isReady = isPlatformReady(category);

            Rlog.d(TAG, "retryIfPlatformNotReady category= " + category + ", isReady= " + isReady);

            if (!isReady) {
                int timeOut = 0;
                do {
                    try {
                        Thread.sleep(AUTO_RETRY_DURATION);
                        timeOut += AUTO_RETRY_DURATION;
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    isReady = isPlatformReady(category);
                } while (!isReady && timeOut < PLUG_IN_AUTO_RETRY_TIMEOUT);
            }

            if (!isReady) {
                result = ExternalSimConstants.RESPONSE_RESULT_PLATFORM_NOT_READY;
                Rlog.w(TAG, "retryIfPlatformNotReady return not ready");
            }

            return result;

        }

        private boolean switchModemCapability(int rsimSlot) {
            MtkProxyController ctrl = (MtkProxyController) ProxyController.getInstance();
            try {
                if (ctrl != null) {
                    int len = TelephonyManager.getDefault().getPhoneCount();
                    RadioAccessFamily[] rafs = new RadioAccessFamily[len];
                    boolean atLeastOneMatch = false;
                    for (int phoneId = 0; phoneId < len; phoneId++) {
                        int raf;
                        if (phoneId == rsimSlot) {
                            raf = ctrl.getMaxRafSupported();
                            atLeastOneMatch = true;
                        } else {
                            raf = ctrl.getMinRafSupported();
                        }
                        Rlog.d(TAG, "[switchModemCapability] raf[" + phoneId + "]=" + raf);
                        rafs[phoneId] = new RadioAccessFamily(phoneId, raf);
                    }
                    if (atLeastOneMatch) {
                        ctrl.setRadioCapability(rafs);
                        return true;
                    } else {
                        Rlog.e(TAG, "[switchModemCapability] rsim error:" + rsimSlot);
                    }
                }
            } catch (RuntimeException e) {
                Rlog.e(TAG, "[switchModemCapability] setRadioCapability: Runtime Exception");
                e.printStackTrace();
            }
            return false;
        }

        private int retryIfRadioUnavailable(VsimEvent event) {
            return retryIfPlatformNotReady(event, PLATFORM_READY_CATEGORY_RADIO);
        }

        private int retryIfSubNotReady(VsimEvent event) {
            return retryIfPlatformNotReady(event, PLATFORM_READY_CATEGORY_SUB);
        }

        private int retryIfCapabilitySwitching(VsimEvent event) {
            return retryIfPlatformNotReady(event, PLATFORM_READY_CATEGORY_SIM_SWITCH);
        }

        private void changeRadioSetting(boolean turnOn) {
            // VSIM 3.0
            // Turn off/on the order SIM slot radio
            int simCount = TelephonyManager.getDefault().getSimCount();
            if (isNonDsdaRemoteSimSupport() && (simCount > 2)) {
                int rsim = SystemProperties.getInt(PREFERED_RSIM_SLOT, -1);
                int akaSim = SystemProperties.getInt(PREFERED_AKA_SIM_SLOT, -1);
                for (int i = 0; i < simCount; i++) {
                    if ((-1 != rsim && i != rsim) && (-1 != akaSim && i != akaSim)) {
                        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(i);

                        ITelephony telephony = ITelephony.Stub.asInterface(
                                ServiceManager.getService(Context.TELEPHONY_SERVICE));

                        try {
                            if (telephony != null) {
                                if (false == turnOn &&telephony.isRadioOnForSubscriber(
                                        subId, mContext.getOpPackageName())) {
                                    mUserRadioOn = true;
                                    telephony.setRadioForSubscriber(subId, false);
                                    Rlog.i(TAG, "changeRadioSetting trun off radio subId:" + subId);
                                } else if (true == turnOn && mUserRadioOn == true) {
                                    mUserRadioOn = false;
                                    telephony.setRadioForSubscriber(subId, true);
                                    Rlog.i(TAG, "changeRadioSetting trun on radio subId:" + subId);
                                }
                            } else {
                                Rlog.d(TAG, "telephony is null");
                            }
                        } catch (RemoteException e) {
                            e.printStackTrace();
                        }
                    }
                }
            }
        }

        private void handleEventRequest(int type, VsimEvent event) {
            Rlog.i(TAG, "VsimEvenHandler eventHandlerByType: type[" + type + "] start");

            // Get external SIM slot id
            int slotId = event.getFirstSlotId();
            // Get if a local SIM (local SIM mean no need to download SIM data from server)
            int simType = event.getInt();
            // Response result
            int result = ExternalSimConstants.RESPONSE_RESULT_OK;
            // Might need to replace the slot id value when special request handling
            int newSlotId = -1;

            Rlog.d(TAG, "VsimEvenHandler First slotId:" + slotId + ", simType:" + simType);

            if ((result == ExternalSimConstants.RESPONSE_RESULT_OK) &&
                    (slotId >= 0 && slotId < TelephonyManager.getDefault().getSimCount()) &&
                    !((MtkUiccController)UiccController.getInstance()).ignoreGetSimStatus()) {
                switch (type) {
                    case ExternalSimConstants.REQUEST_TYPE_ENABLE_EXTERNAL_SIM: {
                        //((MtkRIL)mCi[slotId]).registerForVsimIndication(this, );

                        result = retryIfRadioUnavailable(event);
                        if (result < 0) break;
                        // 0. set result according to sub ready state
                        result = retryIfSubNotReady(event);
                        // Fail on retry, sent error response to vsim-adaptor directly
                        if (result < 0) break;
                        result = retryIfCapabilitySwitching(event);
                        // Fail on retry, sent error response to vsim-adaptor directly
                        if (result < 0) break;

                        // 1.set default data sub id without capablity swtich to VSIM slot
                        MtkSubscriptionController ctrl =
                                (MtkSubscriptionController)SubscriptionController.getInstance();
                        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(slotId);

                        if (simType == ExternalSimConstants.SIM_TYPE_REMOTE_SIM) {
                            ctrl.setDefaultDataSubIdWithoutCapabilitySwitch(subId);
                            // Set preferred RSIM slot
                            if (isNonDsdaRemoteSimSupport()) {
                                sPreferedRsimSlot = slotId;
                            }

                            Rlog.d(TAG, "VsimEvenHandler set default data to subId: " + subId);
                        }

                        // 2. Support hot plug, need to plug out original real sim first
                        if (isSupportVsimHotPlugOut() && !isNonDsdaRemoteSimSupport()) {
                            sendHotPlugEvent(event, false);
                        }

                        // 3. send event to modem side, will set rsim target slot to vsim only
                        sendVsimNotification(slotId,
                                event.mTransactionId, type, simType, null);
                        break;
                    }
                    case ExternalSimConstants.REQUEST_TYPE_DISABLE_EXTERNAL_SIM: {
                        // If didn't enable vsim, ignore directly.
                        String enabled = TelephonyManager.getDefault().getTelephonyProperty(
                                slotId,
                                MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_ENABLED, "0");
                        if (enabled == null || enabled.length() == 0 || "0".equals(enabled)) {
                            Rlog.w(TAG, "VsimEvenHandler didn't not enabled before.");
                            break;
                        }

                        // 0. Check if platform not ready due to sim switching is not completed.
                        result = retryIfCapabilitySwitching(event);
                        // Fail on retry, sent error response to vsim-adaptor directly
                        if (result < 0) break;

                        // 1. set modem waiting flag to false to drop the following uncompleted
                        //    APDU or RESET request.
                        if (mWaitingEvent[slotId] != null) {
                            sendNoResponseError(mWaitingEvent[slotId]);
                        }

                        // 2. send plug out event
                        sendPlugOutEvent(event);

                        // 2.1 Special handling: to ensure plug out done before disable VSIM
                        if (isNonDsdaRemoteSimSupport() || isSupportVsimHotPlugOut()) {
                            SubscriptionController ctrl = SubscriptionController.getInstance();
                            IccCardConstants.State state = IccCardConstants.State.NOT_READY;
                            List<SubscriptionInfo> subInfos = null;
                            int timeOut = 0;
                            do {
                                try {
                                    Thread.sleep(200);
                                    timeOut += 200;
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                                state = IccCardConstants.State.intToState(
                                        ctrl.getSimStateForSlotIndex(slotId));
                                // Wait until subInfos be null which means the subscription
                                // controller has handled the plug out event and it will be safe
                                // to disable VSIM and upload +ESIMS:1,12 by modem
                                subInfos = ctrl.getSubInfoUsingSlotIndexPrivileged(slotId);
                            } while (((state != IccCardConstants.State.ABSENT
                                    && state != IccCardConstants.State.NOT_READY
                                    && state != IccCardConstants.State.UNKNOWN)
                                    || subInfos != null)
                                    && timeOut < SIM_STATE_RETRY_DURATION);
                            Rlog.i(TAG, "VsimEvenHandler DISABLE_EXTERNAL_SIM state: " + state);
                        }
                        if (getVsimSlotId(ExternalSimConstants.SIM_TYPE_REMOTE_SIM) == slotId) {
                            sPreferedRsimSlot = -1;
                            sPreferedAkaSlot = -1;
                            SystemProperties.set(
                                    MtkTelephonyProperties.PROPERTY_EXTERNAL_DISABLE_SIM_DIALOG,
                                    "0");
                        }
                        // 3. send event to modem side, will reset in rild side
                        sendVsimNotification(slotId,
                                event.mTransactionId, type, simType, null);
                        mLastDisableEventTime = System.currentTimeMillis();

                        if (isNonDsdaRemoteSimSupport() || isSupportVsimHotPlugOut()) {
                            // Set main protocol again to avoid default data sub and main protocol
                            // didn't sync after VSIM disabled.
                            MtkSubscriptionController ctrl =
                                    (MtkSubscriptionController)SubscriptionController.getInstance();

                            int defaultDataSub = SubscriptionManager.getDefaultDataSubscriptionId();
                            boolean success = ctrl.setDefaultDataSubIdWithResult(defaultDataSub);

                            // In this case, VSIM allow to enable without reset modem.
                            Rlog.d(TAG, "Disable VSIM without reset modem, sim switch:" + success);
                        } else {
                            waitRildSetDisabledProperty(slotId);
                            RadioManager.getInstance().setSilentRebootPropertyForAllModem("1");
                            MtkUiccController uiccCtrl =
                                    (MtkUiccController) UiccController.getInstance();
                            uiccCtrl.resetRadioForVsim();
                        }

                        // 4. Support hot plug, need to plug in original real sim in the end
                        if (isSupportVsimHotPlugOut() && !isNonDsdaRemoteSimSupport()) {
                            sendHotPlugEvent(event, true);
                        }
                        break;
                    }
                    case ExternalSimConstants.REQUEST_TYPE_PLUG_IN: {
                        // 0. Check if platform not ready due to sim switching is not completed.
                        result = retryIfCapabilitySwitching(event);
                        // Fail on retry, sent error response to vsim-adaptor directly
                        if (result < 0) break;

                        // [VSIM 3.0]
                        // If RSIM enabled, then we need to turn radio off for non-RSIM,
                        // non-AKA-SIM slot.
                        // TODO: how to get aka sim slot? if it is physical sim?

                        // 1. Special handling: to ensure plug out done before plug in VSIM
                        if (isNonDsdaRemoteSimSupport() || isSupportVsimHotPlugOut()) {
                            SubscriptionController ctrl = SubscriptionController.getInstance();
                            IccCardConstants.State state = IccCardConstants.State.NOT_READY;
                            int timeOut = 0;
                            do {
                                try {
                                    Thread.sleep(200);
                                    timeOut += 200;
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                                state = IccCardConstants.State.intToState(
                                        ctrl.getSimStateForSlotIndex(slotId));
                            } while (state != IccCardConstants.State.ABSENT
                                    && state != IccCardConstants.State.NOT_READY
                                    && state != IccCardConstants.State.UNKNOWN
                                    && timeOut < SIM_STATE_RETRY_DURATION);
                            Rlog.d(TAG, "VsimEvenHandler REQUEST_TYPE_PLUG_IN state: " + state);
                        }

                        // 2.capability switch or reset modem with set VSIM on
                        MtkSubscriptionController ctrl =
                                (MtkSubscriptionController)SubscriptionController.getInstance();
                        int mCPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();

                        // Local sim, will not set capability switch
                        if (slotId == mCPhoneId
                                || (simType == ExternalSimConstants.SIM_TYPE_LOCAL_SIM)) {
                            // Send event to rild which will reset modem in rild side.
                            Rlog.d(TAG, "VsimEvenHandler no need to do capablity switch");
                            //mVsimRilIo.writeEvent(event);

                            //((MtkRIL) mCi[slotId]).sendVsimNotification(
                            sendVsimNotification(slotId,
                                    event.mTransactionId, type, simType, null);

                            if (isNonDsdaRemoteSimSupport() || isSupportVsimHotPlugOut()) {
                                // In this case, VSIM allow to enable without reset modem.
                                Rlog.d(TAG, "VSIM allow to enable without reset modem");
                            } else {
                                RadioManager.getInstance().setSilentRebootPropertyForAllModem(
                                        "1");
                                MtkUiccController uiccCtrl =
                                        (MtkUiccController) UiccController.getInstance();
                                uiccCtrl.resetRadioForVsim();
                            }
                        } else {
                            // FIXME: capability switch might not trigger modem reset in some
                            // project.
                            // set result according to sub ready state
                            Rlog.d(TAG, "VsimEvenHandler need to do capablity switch");
                            if (ctrl.isReady()) {
                                // Capbility switch will reset modem
                                int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(slotId);

                                boolean success = ctrl.setDefaultDataSubIdWithResult(subId);

                                if (success == true) {
                                    result = ExternalSimConstants.RESPONSE_RESULT_OK;
                                } else {
                                    result = ExternalSimConstants.RESPONSE_RESULT_GENERIC_ERROR;
                                }
                            } else {
                                result= ExternalSimConstants.RESPONSE_RESULT_PLATFORM_NOT_READY;
                            }
                        }
                        break;
                    }
                    case ExternalSimConstants.REQUEST_TYPE_PLUG_OUT: {
                        // 1.[VSIM 3.0]If isWaitingAuthRsp flg is true, means that we should
                        //   release RFoccupy before send auth response to mode.
                        if (mWaitingEvent[slotId] != null) {
                            sendNoResponseError(mWaitingEvent[slotId]);
                        }

                        // 2.send event to modem side
                        //mVsimRilIo.writeEvent(event);
                        //((MtkRIL) mCi[slotId]).sendVsimNotification(
                        sendVsimNotification(slotId, event.mTransactionId, type, simType, null);
                        break;
                    }
                    case ExternalSimConstants.REQUEST_TYPE_SET_PERSIST_TYPE: {
                        sendVsimNotification(slotId, event.mTransactionId, type, simType, null);
                        break;
                    }
                    case ExternalSimConstants.REQUEST_TYPE_SET_PERSIST_TIMEOUT: {
                        sendVsimNotification(slotId, event.mTransactionId, type, simType, null);
                        break;
                    }
                    case ExternalSimConstants.REQUEST_TYPE_SET_MAPPING_INFO: {
                        if (simType == ExternalSimConstants.SIM_TYPE_REMOTE_SIM) {
                            result = retryIfCapabilitySwitching(event);
                            // Fail on retry, sent error response to vsim-adaptor directly
                            if (result < 0) break;

                            try {
                                Rlog.d(TAG, "VsimEvenHandler isCapabilitySwitching: false.");
                                sPreferedRsimSlot = slotId;
                                result = sendSetRsimMappingInfoSync(slotId, event.mTransactionId);
                                int mainPhoneId =
                                        RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
                                Rlog.i(TAG, "VsimEvenHandler sendSetRsimMappingInfoSync result:" +
                                        result + "mainPhoneId:" + mainPhoneId);
                                if (result < 0 || mainPhoneId == slotId) break;

                                mSetCapabilityDone = SET_CAPABILITY_ONGOING;
                                if (switchModemCapability(slotId) == false) {
                                    mSetCapabilityDone = SET_CAPABILITY_NONE;
                                    sPreferedRsimSlot = -1;
                                    sendVsimNotification(slotId, event.mTransactionId,
                                            ExternalSimConstants.REQUEST_TYPE_RESET_MAPPING_INFO,
                                            simType, null);
                                    result =
                                        ExternalSimConstants
                                        .RESPONSE_RESULT_PLATFORM_NOT_READY;
                                    break;
                                }

                                if (mSetCapabilityDone == SET_CAPABILITY_ONGOING) {
                                    Rlog.d(TAG, "VsimEvenHandler before mLock.wait");
                                    mLock.wait();
                                }

                                Rlog.d(TAG, "VsimEvenHandler after mLock.wait");

                                if (mSetCapabilityDone == SET_CAPABILITY_FAILED) {
                                    sPreferedRsimSlot = -1;
                                    sendVsimNotification(slotId, event.mTransactionId,
                                            ExternalSimConstants.REQUEST_TYPE_RESET_MAPPING_INFO,
                                            simType, null);
                                    result =
                                        ExternalSimConstants
                                        .RESPONSE_RESULT_PLATFORM_NOT_READY;
                                }
                                mSetCapabilityDone = SET_CAPABILITY_NONE;
                            } catch (InterruptedException e) {
                                Rlog.w(TAG, "VsimEvenHandler InterruptedException.");
                            }
                        } else if (simType == ExternalSimConstants.SIM_TYPE_AKA_SIM) {
                            if (slotId >= 0
                                    && slotId < TelephonyManager.getDefault().getSimCount()) {
                                // Case 1. vaild slot id, Told Modem there is a AKA-SIM stack
                                sendActiveAkaSimEvent(slotId, true);
                            } else {
                                // Case 2. invalid slot id, told modem end of AKA-SIM stack
                                int akaSim = SystemProperties.getInt(PREFERED_AKA_SIM_SLOT, -1);
                                if (akaSim != -1) {
                                    newSlotId = akaSim;
                                    sendActiveAkaSimEvent(akaSim, false);
                                }
                                // Reset AKA SIM slot system properties
                                Rlog.d(TAG, "Reset PREFERED_AKA_SIM_SLOT");
                            }
                        } else {
                            result = ExternalSimConstants.RESPONSE_RESULT_GENERIC_ERROR;
                        }
                        break;
                    }
                    case ExternalSimConstants.REQUEST_TYPE_RESET_MAPPING_INFO: {
                        // Notify modem stop see this stack as AKA-SIM stack
                        if (simType == ExternalSimConstants.SIM_TYPE_AKA_SIM) {
                            int akaSim = SystemProperties.getInt(PREFERED_AKA_SIM_SLOT, -1);
                            if (akaSim != -1) {
                                sendActiveAkaSimEvent(akaSim, false);
                            }

                            // Reset AKA SIM slot system properties
                            Rlog.d(TAG, "Reset PREFERED_AKA_SIM_SLOT");
                        } else if (simType == ExternalSimConstants.SIM_TYPE_REMOTE_SIM) {
                            int rSim = SystemProperties.getInt(PREFERED_RSIM_SLOT, -1);
                            if (rSim != -1) {
                                sendVsimNotification(slotId, event.mTransactionId, type, simType,
                                        null);
                            }
                        }
                        break;
                    }
                    case ExternalSimConstants.REQUEST_TYPE_SET_TIMEOUT_TIMER: {
                        // In this case, simType will be time out duration
                        mNoResponseTimeOut[slotId] = simType * 1000;
                        sendVsimNotification(slotId, event.mTransactionId, type, simType, null);
                        break;
                    }
                    case ExternalSimConstants.REQUEST_TYPE_ENABLE_SIM_DIALOG: {
                        SystemProperties.set(
                                MtkTelephonyProperties.PROPERTY_EXTERNAL_DISABLE_SIM_DIALOG,
                                "0");
                        break;
                    }
                    case ExternalSimConstants.REQUEST_TYPE_DISABLE_SIM_DIALOG: {
                        SystemProperties.set(
                                MtkTelephonyProperties.PROPERTY_EXTERNAL_DISABLE_SIM_DIALOG,
                                "1");
                        break;
                    }
                    case ExternalSimConstants.EVENT_TYPE_EXTERNAL_SIM_CONNECTED: {
                        sendVsimNotification(slotId, event.mTransactionId, type, simType, null);
                        Rlog.i(TAG, "VsimEvenHandler eventHandlerByType: type[" + type + "] end");
                        return; //Internal event, don't send to apk
                    }
                    default: {
                        result = ExternalSimConstants.RESPONSE_RESULT_GENERIC_ERROR;
                        Rlog.d(TAG, "VsimEvenHandler invalid event id.");
                        break;
                    }
                }
            } else {
                if (result == ExternalSimConstants.RESPONSE_RESULT_OK) {
                    result = ExternalSimConstants.RESPONSE_RESULT_GENERIC_ERROR;
                }
            }

            VsimEvent eventResponse = new VsimEvent(
                    event.getTransactionId(),
                    ExternalSimConstants.MSG_ID_EVENT_RESPONSE,
                    (newSlotId == -1) ? event.getSlotBitMask() : (1 << newSlotId));
            eventResponse.putInt(result);  //result

            if (mVsimAdaptorIo != null) {
                mVsimAdaptorIo.writeEvent(eventResponse);
            }

            Rlog.i(TAG, "VsimEvenHandler eventHandlerByType: type[" + type + "] end");
        }

        private void handleGetPlatformCapability(VsimEvent event) {
            int eventId = event.getInt();   //no-used
            int simType = event.getInt();

            VsimEvent response = new VsimEvent(
                    event.getTransactionId(),
                    ExternalSimConstants.MSG_ID_GET_PLATFORM_CAPABILITY_RESPONSE,
                    event.getSlotBitMask());
            // 1. Put result value to check platform ready
            MtkSubscriptionController ctrl =
                    (MtkSubscriptionController) SubscriptionController.getInstance();
            if (ctrl.isReady()) {
                response.putInt(ExternalSimConstants.RESPONSE_RESULT_OK);
            } else {
                response.putInt(ExternalSimConstants.RESPONSE_RESULT_PLATFORM_NOT_READY);
            }

            // 2.1 Return multi-phone type, such as dsds or dsda.
            MultiSimVariants config = TelephonyManager.getDefault().getMultiSimConfiguration();
            if (config == MultiSimVariants.DSDS) {
                response.putInt(ExternalSimConstants.MULTISIM_CONFIG_DSDS);
            } else if (config == MultiSimVariants.DSDA) {
                response.putInt(ExternalSimConstants.MULTISIM_CONFIG_DSDA);
            } else if (config == MultiSimVariants.TSTS) {
                response.putInt(ExternalSimConstants.MULTISIM_CONFIG_TSTS);
            } else {
                response.putInt(ExternalSimConstants.MULTISIM_CONFIG_UNKNOWN);
            }

            // 2.2 Return external SIM support flag (refer to feature option)
            if (SystemProperties.getInt("ro.vendor.mtk_external_sim_support", 0) > 0) {
                int mDefaultSupportVersion =
                        ExternalSimConstants.CAPABILITY_SUPPORT_EXTERNAL_SIM |
                        ExternalSimConstants.CAPABILITY_SUPPORT_PERSIST_EXTERNAL_SIM;

                if (isNonDsdaRemoteSimSupport()) {
                    mDefaultSupportVersion |=
                            ExternalSimConstants.CAPABILITY_SUPPORT_NON_DSDA_REMOTE_SIM;
                }
                response.putInt(mDefaultSupportVersion);
            } else {
                response.putInt(ExternalSimConstants.CAPABILITY_NOT_SUPPORT);
            }

            // 2.3 Return slots allow to enable external SIM.
            // The value is bit-mask, bit X means the slot (X - 1) is allowed to use external
            // SIM. For an example, value 3 is bit 1 and 2 is 1 means external SIM is allowed
            // to enable on slot 1 and slot 0.
            int simCount = TelephonyManager.getDefault().getSimCount();

            Rlog.d(TAG, "handleGetPlatformCapability simType: " + simType
                    + ", simCount: " + simCount);

            if (simType == ExternalSimConstants.SIM_TYPE_LOCAL_SIM) {
                // [Case 1] Soft SIM / AKA-SIM
                int rsimSlot = SystemProperties.getInt(PREFERED_RSIM_SLOT, -1);
                if (rsimSlot == -1) {
                    // Case 1.1 Soft SIM, all SIM slots are allowed.
                    response.putInt((1 << simCount) - 1);
                } else {
                    // Case 1.2 AKA SIM in case of RSIM slot has been indicated.
                    // RSIM always be protocol 1 and AKA-SIM always be protocol 2
                    // Slot1: Protocol1 (RSIM), Slot2: Protocol2 (AKA-SIM), Slot3: Protocol3
                    // Slot1: Protocol2 (AKA-SIM), Slot2: Protocol1 (RSIM), Slot3: Protocol3
                    // Slot1: Protocol3 , Slot2: Protocol2 (AKA-SIM), Slot3: Protocol1(RSIM)
                    if (rsimSlot == 1 || rsimSlot == 4) {
                        response.putInt(2);
                    } else if (rsimSlot == 2) {
                        response.putInt(1);
                    }
                }
            } else {
                // [Case 2] Remote SIM
                if (config == MultiSimVariants.DSDA) {
                    // Case 2.1 Remote SIM, DSDA is needed
                    int isCdmaCard = 0;
                    int isHasCard = 0;

                    for (int i = 0; i < simCount; i++) {
                        String cardType = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[i], "");
                        if (!cardType.equals("")) {
                            isHasCard |= (1 << i);
                        }

                        if (cardType.contains("CSIM")
                                || cardType.contains("RUIM") || cardType.contains("UIM")) {
                            isCdmaCard |= (1 << i);
                        }
                    }

                    Rlog.d(TAG, "handleGetPlatformCapability isCdmaCard: " + isCdmaCard
                            + ", isHasCard: " + isHasCard);

                    if (isHasCard == 0) {
                        // DSDA project and there is no card is inserted.
                        response.putInt(0);
                    } else if (isCdmaCard == 0) {
                        // DSDA project and there is no C card is inserted.
                        response.putInt(0);
                    } else {
                        // DSDA project and there is a C card is inserted.
                        response.putInt(isCdmaCard ^ ((1 << simCount) - 1));
                    }

                } else if (isNonDsdaRemoteSimSupport()) {
                    // Case 2.2 Remote SIM, MPS is supported. (VSIM 3.0)
                    //SystemProperties.set(PREFERED_RSIM_SLOT,
                    //        String.valueOf(event.getFirstSlotId()));
                    if (config == MultiSimVariants.DSDS) {
                        // DSDS
                        response.putInt((1 << simCount) - 1);
                    } else if (config == MultiSimVariants.TSTS) {
                        // TSTS, RSIM always be protocol 1 and AKA-SIM always be protocol 2
                        // FIXME: only return RSIM slot currently.
                        int vsimOnly = SystemProperties.getInt(
                                "ro.vendor.mtk_external_sim_only_slots", 0);
                        if (vsimOnly != 0) {
                            response.putInt(vsimOnly);
                        } else {
                            response.putInt((1 << simCount) - 1);
                        }
                    }
                } else {
                    // Case 2.3 Remote SIM, DSDA and MPS is not supported.
                    // In this case, we couldn't enable external SIM.
                    response.putInt(0);
                }
            }

            // Write response event by socket
            mVsimAdaptorIo.writeEvent(response);
        }

        private void handleServiceStateRequest(VsimEvent event) {
            int result = ExternalSimConstants.RESPONSE_RESULT_OK;
            int voiceRejectCause = -1;
            int dataRejectCause = -1;

            VsimEvent response = new VsimEvent(
                    event.getTransactionId(),
                    ExternalSimConstants.MSG_ID_GET_SERVICE_STATE_RESPONSE,
                    event.getSlotBitMask());
            MtkSubscriptionController ctrl =
                    (MtkSubscriptionController) SubscriptionController.getInstance();
            if (ctrl.isReady()) {
                int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(
                        event.getFirstSlotId());

                ServiceState ss = TelephonyManager.getDefault().getServiceStateForSubscriber(subId);

                if (ss != null) {
                    MtkServiceState mtkSs = (MtkServiceState) ss;
                    Rlog.d(TAG, "handleServiceStateRequest subId: " + subId +
                            ", ss = " + mtkSs.toString());
                    voiceRejectCause = mtkSs.getVoiceRejectCause();
                    dataRejectCause = mtkSs.getDataRejectCause();
                }
            } else {
                result = ExternalSimConstants.RESPONSE_RESULT_PLATFORM_NOT_READY;
            }

            //Put response result
            response.putInt(result);
            //Put voice reject cause
            response.putInt(voiceRejectCause);
            //Put data reject cause
            response.putInt(dataRejectCause);

            mVsimAdaptorIo.writeEvent(response);
        }

        private Object getLock(int msgId) {
            switch(msgId) {
                case ExternalSimConstants.MSG_ID_EVENT_REQUEST:
                case ExternalSimConstants.MSG_ID_EVENT_RESPONSE:
                case ExternalSimConstants.MSG_ID_UICC_AUTHENTICATION_REQUEST_IND:
                case ExternalSimConstants.MSG_ID_UICC_AUTHENTICATION_DONE_IND:
                case ExternalSimConstants.MSG_ID_UICC_AUTHENTICATION_ABORT_IND:
                case ExternalSimConstants.MSG_ID_CAPABILITY_SWITCH_DONE:
                    return mLock;
                default:
                    return mLock;
            }
        }

        /* dispatch Callback */
        private void dispatchCallback(VsimEvent event) {
            synchronized (getLock(event.getMessageId())) {
                // To record the running thread is get lock successfully.
                // Allow to start next thread.
                if (mEventHandlingThread[event.getFirstSlotId()] != null) {
                    mEventHandlingThread[event.getFirstSlotId()].setWaiting(false);
                }

                // Handler events
                int msgId = event.getMessageId();

                Rlog.d(TAG, "VsimEvenHandler handleMessage[" + event.getFirstSlotId()
                        + "]: msgId[" + msgId + "] start");

                switch (msgId) {
                    case ExternalSimConstants.MSG_ID_INITIALIZATION_REQUEST:
                        // Customized:allow to do neccessary initialization related to external SIM.
                        // For an example, start an indicated service or set some configuration.
                        break;

                    case ExternalSimConstants.MSG_ID_FINALIZATION_REQUEST:
                        // Customized:allow to do neccessary finalization related to external SIM.
                        // For an example, stop an indicated service or set some configuration.
                        break;

                    case ExternalSimConstants.MSG_ID_GET_PLATFORM_CAPABILITY_REQUEST:
                        handleGetPlatformCapability(event);
                        break;

                    case ExternalSimConstants.MSG_ID_EVENT_REQUEST:
                        handleEventRequest(event.getInt(), event);
                        break;

                    case ExternalSimConstants.MSG_ID_EVENT_RESPONSE:
                        // FIXME: Is need to return from modem???
                        // Occupy/Release RF command need to wait response.
                        int type = event.getInt();
                        if (type == ExternalSimConstants.EVENT_TYPE_SEND_RSIM_AUTH_IND
                                || type == ExternalSimConstants.EVENT_TYPE_RECEIVE_RSIM_AUTH_RSP) {

                            mIsSwitchRfSuccessful = (event.getInt() < 0) ? false : true;

                            mLock.notifyAll();
                        } else if (type == ExternalSimConstants.REQUEST_TYPE_SET_MAPPING_INFO) {
                            // Notify sendSetRsimMappingInfoSync
                            mLock.notifyAll();
                        }
                        break;

                    case ExternalSimConstants.MSG_ID_UICC_APDU_REQUEST: {
                        setMdWaitingFlag(true, event, event.getFirstSlotId());
                        // Request from modem side, just adjust format and dispatch the event

                        // get system property to check if vsim started
                        String inserted =  TelephonyManager.getDefault().getTelephonyProperty(
                                event.getFirstSlotId(),
                                MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");

                        if (mVsimAdaptorIo != null
                                && inserted != null && inserted.length() > 0
                                && !"0".equals(inserted)) {
                            mVsimAdaptorIo.writeEvent(event);
                        } else {
                            Rlog.d(TAG, "ignore UICC_APDU_REQUEST since vsim plug out.");
                            sendNoResponseError(event);
                        }
                        break;
                    }
                    case ExternalSimConstants.MSG_ID_UICC_APDU_RESPONSE:
                        // If modem waiting flag is set to flag, mean that, there might
                        // be a plug out event during waiting response.
                        // In this case, AP should drop this event to avoid modem receive
                        // unexcepted event.

                        // If isWaitingAuthRsp flg is true, means that we should release RF
                        // occupy before send auth response to mode.
                        if (mIsWaitingAuthRsp[event.getFirstSlotId()] == true) {
                            mIsWaitingAuthRsp[event.getFirstSlotId()] = false;
                            sendRsimAuthProgressEvent(
                                    ExternalSimConstants.EVENT_TYPE_RECEIVE_RSIM_AUTH_RSP);
                        }

                        if (getMdWaitingFlag(event.getFirstSlotId())) {

                            // Get resposne, so reset waiting flag.
                            setMdWaitingFlag(false, event.getFirstSlotId());

                            // Otherwise, just send to modem side without parsing data.
                            // get system property to check if vsim started
                            String inserted =  TelephonyManager.getDefault().getTelephonyProperty(
                                    event.getFirstSlotId(),
                                    MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");

                            if (inserted != null && inserted.length() > 0
                                    && !"0".equals(inserted)) {
                                //mVsimRilIo.writeEvent(event);
                                ((MtkRIL) mCi[event.getFirstSlotId()]).sendVsimOperation(
                                         event.getTransactionId(), event.getMessageId(),
                                         event.getInt(), event.getInt(),
                                         event.getDataByReadOffest(), null);
                            } else {
                                Rlog.d(TAG, "ignore UICC_APDU_RESPONSE since vsim plug out.");
                            }
                        }
                        break;

                    case ExternalSimConstants.MSG_ID_UICC_RESET_REQUEST: {
                        setMdWaitingFlag(true, event, event.getFirstSlotId());
                        // Reguest from modem side, just adjust format and dispatch the event
                        // get system property to check if vsim started
                        String inserted =  TelephonyManager.getDefault().getTelephonyProperty(
                                event.getFirstSlotId(),
                                MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");

                        if (mVsimAdaptorIo != null
                                && inserted != null && inserted.length() > 0
                                && !"0".equals(inserted)) {
                            mVsimAdaptorIo.writeEvent(event);
                        }
                        break;
                    }
                    case ExternalSimConstants.MSG_ID_UICC_RESET_RESPONSE:
                        if (getMdWaitingFlag(event.getFirstSlotId())) {
                            // If modem waiting flag is set to flag, mean that, there might
                            // be a plug out event during waiting response.
                            // In this case, AP should drop this event to avoid modem receive
                            // unexcepted event.
                            // If the waiting flag is true,
                            // just send to modem side without parsing data.
                            setMdWaitingFlag(false, event.getFirstSlotId());
                            //mVsimRilIo.writeEvent(event);
                            // TODO: how to decode the result?
                            ((MtkRIL) mCi[event.getFirstSlotId()]).sendVsimOperation(
                                     event.getTransactionId(), event.getMessageId(),
                                     event.getInt(), event.getInt(),
                                     event.getDataByReadOffest(), null);
                        }
                        break;

                    case ExternalSimConstants.MSG_ID_UICC_POWER_DOWN_REQUEST: {
                        // get system property to check if vsim started
                        String inserted =  TelephonyManager.getDefault().getTelephonyProperty(
                                event.getFirstSlotId(),
                                MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");

                        if (mVsimAdaptorIo != null
                                && inserted != null && inserted.length() > 0
                                && !"0".equals(inserted)) {
                            mVsimAdaptorIo.writeEvent(event);
                        }
                        break;
                    }
                    case ExternalSimConstants.MSG_ID_UICC_POWER_DOWN_RESPONSE:
                        //mVsimRilIo.writeEvent(event);
                        break;

                    case ExternalSimConstants.MSG_ID_GET_SERVICE_STATE_REQUEST:
                        handleServiceStateRequest(event);
                        break;

                    case ExternalSimConstants.MSG_ID_UICC_AUTHENTICATION_REQUEST_IND: {
                        // Send to vsim-rild to occpy RF before notify vsim adaptor.
                        setMdWaitingFlag(true, event, event.getFirstSlotId());
                        mIsWaitingAuthRsp[event.getFirstSlotId()] = true;
                        sendRsimAuthProgressEvent(
                                ExternalSimConstants.EVENT_TYPE_SEND_RSIM_AUTH_IND);

                        // Change message id to apdu request
                        event.mMessageId = ExternalSimConstants.MSG_ID_UICC_APDU_REQUEST;

                        // Reguest from modem side, just adjust format and dispatch the event
                        // get system property to check if vsim started
                        String inserted =  TelephonyManager.getDefault().getTelephonyProperty(
                                event.getFirstSlotId(),
                                MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");

                        // If fail to occupy RF, send auth. error resposne to modem directly.
                        if (mIsSwitchRfSuccessful == true) {
                            if (inserted != null &&
                                    inserted.length() > 0 && !"0".equals(inserted)) {
                                mVsimAdaptorIo.writeEvent(event);
                            }
                        } else {
                            sendNoResponseError(event);
                        }

                        break;
                    }

                    case ExternalSimConstants.MSG_ID_UICC_AUTHENTICATION_DONE_IND: {
                        // Notify modem to turn off AKA SIM stack
                        //int akaSim = SystemProperties.getInt(PREFERED_AKA_SIM_SLOT, -1);
                        //if (akaSim != -1) {
                        //    sendActiveAkaSimEvent(akaSim, false);
                        //}

                        // Send to vsim adaptor directly.
                        // get system property to check if vsim started
                        String inserted =  TelephonyManager.getDefault().getTelephonyProperty(
                                event.getFirstSlotId(),
                                MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");

                        if (inserted != null && inserted.length() > 0 && !"0".equals(inserted)) {
                            mVsimAdaptorIo.writeEvent(event);
                        }
                        break;
                    }
                    case ExternalSimConstants.MSG_ID_UICC_AUTHENTICATION_ABORT_IND: {
                        // Send error response to RSIM slot
                        VsimEvent abortEvent = new VsimEvent(
                                0,
                                ExternalSimConstants.MSG_ID_UICC_APDU_REQUEST,
                                1 << (getVsimSlotId(ExternalSimConstants.SIM_TYPE_REMOTE_SIM)));
                        sendNoResponseError(abortEvent);

                        // Send to vsim adaptor directly.
                        // get system property to check if vsim started
                        String inserted =  TelephonyManager.getDefault().getTelephonyProperty(
                                1 << (getVsimSlotId(ExternalSimConstants.SIM_TYPE_REMOTE_SIM)),
                                MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");

                        if (inserted != null && inserted.length() > 0 && !"0".equals(inserted)) {
                            mVsimAdaptorIo.writeEvent(event);
                        }
                        break;
                    }
                    case ExternalSimConstants.MSG_ID_CAPABILITY_SWITCH_DONE: {
                        mLock.notifyAll();
                        break;
                    }
                    case ExternalSimConstants.MSG_ID_UICC_TEST_MODE_REQUEST: {
                        //mVsimRilIo.writeEvent(event);
                        ((MtkRIL) mCi[event.getFirstSlotId()]).sendVsimOperation(
                                 event.getTransactionId(), event.getMessageId(),
                                 event.getInt(), event.getInt(),
                                 event.getDataByReadOffest(), null);
                        break;
                    }
                    default:
                        Rlog.d(TAG, "VsimEvenHandler handleMessage: default");
                }

                Rlog.d(TAG, "VsimEvenHandler handleMessage[" + event.getFirstSlotId()
                        + "]: msgId[" + msgId + "] end");
            }
        }
    }
}


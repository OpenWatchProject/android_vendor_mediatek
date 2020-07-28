/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.ct.deviceregister.dm;

import android.app.Activity;
import android.app.AlarmManager.OnAlarmListener;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.util.Log;

import com.ct.deviceregister.dm.utils.AgentProxy;
import com.ct.deviceregister.dm.utils.EsnWrapper;
import com.ct.deviceregister.dm.utils.PlatformManager;
import com.ct.deviceregister.dm.utils.Utils;
import com.ct.selfregister.dm.Op09SelfRegister;
import com.mediatek.internal.telephony.devreg.DefaultDeviceRegisterExt;
import com.mediatek.internal.telephony.devreg.DeviceRegisterController;

public class Op09DeviceRegisterExt extends DefaultDeviceRegisterExt {

    private static final String TAG = Const.TAG_PREFIX + "RegisterService";

    private static final int MSG_BOOT_COMPLETED = 1;
    private static final int MSG_SIM_STATE_CHANGED = 2;
    private static final int MSG_SIM_INSERVICE = 3;
    private static final int MSG_MESSAGE_SEND = 4;
    private static final int MSG_DEFAULT_DATA_SUBSCRIPTION_CHANGED = 5;
    private static final int MSG_RETRY_REGISTER = 6;

    private static final int TIMES_MAX_WAIT = 3;
    private static final int TIMES_MAX_RETRY = 3;

    private AgentProxy mAgentProxy;
    private PlatformManager mPlatformManager;
    private SmsSendReceiver mSmsSendReceiver;
    private StartupReceiver mStartupReceiver;

    private String mMeid = null;
    private String mPreviousEsnOrMeid = null;

    private boolean mHasInServiceAlarm = false;
    private boolean mHasSimLoadedIntent = false;

    private int mUimSlot = Const.SLOT_ID_INVALID;
    private int mDataSub = Const.SUB_ID_INVALID;
    private int mWaitTimes = 0;
    private int mRetryTimes = 0;

    private int[] mSlotList;
    private String[] mImsiOnSim;
    private int[] mSubIdSnapShot;
    private CustomizedPhoneStateListener[] mPhoneStateListener;

    private OnAlarmListener mRetryAlarmListener;

    private Handler mHandler;
    private HandlerThread mHandlerThread;

    private String mVersion = "";
    private Op09ImsRegister mImsRegister;

    private boolean mWaitForResponse = false;
    private long mSendMessageTime = 0;
    private String mRegisterImsi = "";

    public Op09DeviceRegisterExt(Context context, DeviceRegisterController controller) {
        super(context, controller);
        if (PlatformManager.isFeatureSupported()) {
            init();

            mImsRegister = new Op09ImsRegister(context, controller);

            // Register after ImsRegister init, to avoid broadcast comes before
            // ImsRegister finish init
            registerStartupReceiver();
        }
        Op09SelfRegister.newSelfRegister(context);
    }

    private void init() {
        Log.i(TAG, "init at " + System.currentTimeMillis());
        mPlatformManager = new PlatformManager(mContext);
        mAgentProxy = new AgentProxy(mContext);

        if (mPlatformManager.isSingleLoad()) {
            mSlotList = Const.SINGLE_SIM_SLOT;
        } else {
            mSlotList = Const.DUAL_SIM_SLOTS;
        }

        mPhoneStateListener = new CustomizedPhoneStateListener[mSlotList.length];
        mSubIdSnapShot = new int[mSlotList.length];

        for (int i = 0; i < mSlotList.length; ++i) {
            mPhoneStateListener[i] = null;
            mSubIdSnapShot[i] = Const.SUB_ID_INVALID;
        }
        initHandler();
        mVersion = PlatformManager.getVersion();
    }

    private void resetParameters() {
        Log.i(TAG, "resetParameters at " + System.currentTimeMillis());

        unRegisterSendReceiver();
        unRegisterPhoneListeners();

        // not reset mDataSub & mRetryTimes, mWaitForResponse
        mWaitTimes = 0;
        mHasSimLoadedIntent = false;
    }

    /*
     * rawValue: "oldEsn,newEsn"
     */
    @Override
    public void setCdmaCardEsnOrMeid(String rawValue){
        if (rawValue == null || rawValue.length() == 0) {
            Log.i(TAG, "setCdmaCardEsnOrMeid, invalid raw value");
            return;
        }

        String temp[] = rawValue.split(",");
        if (temp.length >= 1) {
            String esnOrMeid = temp[0].trim();

            if (mPreviousEsnOrMeid == null || !mPreviousEsnOrMeid.equals(esnOrMeid)) {
                Log.i(TAG, "Update previous esn/meid from "
                        + PlatformManager.encryptMessage(mPreviousEsnOrMeid)
                        + " to " + PlatformManager.encryptMessage(esnOrMeid));
                mPreviousEsnOrMeid = esnOrMeid;

                if (mHasSimLoadedIntent) {
                    mHandler.sendEmptyMessage(MSG_SIM_STATE_CHANGED);
                }
            } else {
                Log.i(TAG, "No need to update previous esn/meid");
            }
        }
    }

    /**
     * Handle the auto register message reply (for CDMA)
     * @param pdu: server SMS pdu response
     */
    @Override
    public void handleAutoRegMessage(byte[] pdu){
        Log.i(TAG, "handleAutoRegMessage");

        if (mWaitForResponse) {
            boolean result = PlatformManager.SmsWrapper.checkRegisterResult(pdu);
            mAgentProxy.setRegisterFlag(result);
            if (result) {
                Log.i(TAG, "Register success!");
            } else {
                Log.i(TAG, "Register failed!");
            }

            mWaitForResponse = false;
            mSendMessageTime = 0;
            mRegisterImsi = "";

            // Cancel retry alarm if any, after a complete register flow
            cancelRetryAlarm();
            resetParameters();
        } else {
            Log.e(TAG, "Not wait for response, ignore this response");
        }
    }

    /**
     * Handle the auto register message reply (for IMS)
     * @param subId: sub id of SMS response
     * @param format: format of message, like SmsMessage.FORMAT_3GPP2
     * @param pdu: server SMS pdu response
     */
    @Override
    public void handleAutoRegMessage(int subId, String format, byte[] pdu) {
        Log.i(TAG, "handleAutoRegMessage subId " + subId + ", format " + format);
        if (isVersion2019V1() && mImsRegister != null) {
            mImsRegister.onAutoRegMessage(subId, format, pdu);
        }
    }

    private void onSimStateChanged() {
        Log.i(TAG, "onSimStateChanged");
        mHasSimLoadedIntent = true;

        // start service if MEID/ESN info also ready
        if (mPreviousEsnOrMeid != null) {
            mHandler.sendEmptyMessage(MSG_SIM_STATE_CHANGED);
        }

        if (isVersion2019V1() && mImsRegister != null) {
            mImsRegister.onSimStateChanged();
        }
    }

    private void onDataSubChanged(int subId) {
        Log.i(TAG, "onDataSubChanged Old/now are " + mDataSub + "/" + subId);

        if (subId >= 0 && subId != mDataSub) {
            if (mDataSub >= 0 ) {
                mHandler.sendEmptyMessage(MSG_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
            } else {
                Log.i(TAG, "Old sub invalid, do nothing");
            }
            mDataSub = subId;
        }
    }

    private void initHandler() {
        mHandlerThread = new HandlerThread("DeviceRegister");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper()) {

            @Override
            public void handleMessage(Message msg) {

                switch (msg.what) {
                case MSG_BOOT_COMPLETED:
                    doAfterBoot();
                    break;

                case MSG_SIM_STATE_CHANGED:
                case MSG_DEFAULT_DATA_SUBSCRIPTION_CHANGED:
                    registerPhoneListeners();
                    break;

                case MSG_SIM_INSERVICE:
                    doAfterSimInService();
                    break;

                case MSG_MESSAGE_SEND:
                    doAfterMessageSend();
                    break;

                case MSG_RETRY_REGISTER:
                    doAfterRetry();

                default:
                    break;
                }
            }
        };
    }

    private void doAfterBoot() {
        checkFingerPrint();
        if (isVersion2019V1() && mImsRegister != null) {
            mImsRegister.onBootUp();
        }
    }

    /*
     * Check fingerprint and reset register if needed
     */
    private void checkFingerPrint() {
        String savedFingerPrint = mAgentProxy.getSavedFingerPrint();
        String currentFingerPrint = mAgentProxy.getCurrentFingerPrint();
        Log.i(TAG, "savedFingerPrint " + savedFingerPrint);
        Log.i(TAG, "currentFingerPrint " + currentFingerPrint);

        // If finger print not the same (first time boot, factory reset or system update), reset
        // register flag
        if (!savedFingerPrint.equals(currentFingerPrint)) {
            Log.i(TAG, "System fingerprint changes, reset register flag");
            mAgentProxy.setSavedFingerPrint(currentFingerPrint);
            mAgentProxy.resetRegisterFlag();

            if (isVersion2019V1() && mImsRegister != null) {
                // reset flag for Ims register
                mImsRegister.resetRegisterFlag();
            }
        } else {
            Log.i(TAG, "Same fingerprint, do nothing");
        }
    }

    private void doAfterSimInService() {
        mHasInServiceAlarm = false;

        Log.i(TAG, "doAfterSimInService, wait times " + mWaitTimes);
        if (!mPlatformManager.areSlotsInfoReady(mSlotList) && mWaitTimes < TIMES_MAX_WAIT) {
            long second = (mWaitTimes % 3 + 1) * 10 * Const.ONE_SECOND;
            setInServiceDelayAlarm(second);
            mWaitTimes++;
            return;
        }

        initIdentityInfo();

        // Scenario need to handle
        // 1. CT SIM's network change from CDMA to LTE
        // 2. CT SIM removed after IN_SERVICE & register CDMA network
        if (!isUimSlotValid()) {
            Log.i(TAG, "Can't find a valid UIM due to network change, wait");
            return;
        }

        if (needRegister()) {
            registerSendReceiver();
            sendRegisterMessage();
        } else {
            if (mWaitForResponse) {
                unRegisterPhoneListeners();
                Log.i(TAG, "Still wait for response.");
            } else {
                Log.d(TAG, "Phone no need to register.");
                resetParameters();
            }
        }
    }

    private void doAfterMessageSend() {
        Log.i(TAG, "doAfterMessageSend " + System.currentTimeMillis());

        // Update to current device MEID
        mPreviousEsnOrMeid = mMeid;

        mWaitForResponse = true;
        mSendMessageTime = System.currentTimeMillis();
        mRegisterImsi = getCurrentCDMAImsi();

        unRegisterPhoneListeners();

        mAgentProxy.resetRegisterFlag();
        mAgentProxy.setSavedImsi(mImsiOnSim);
        mAgentProxy.setRegisterImsi(mRegisterImsi);
    }

    private void doAfterRetry() {
        Log.i(TAG, "doAfterRetry");
        mRetryAlarmListener = null;
        if (!mHasInServiceAlarm) {
            setInServiceDelayAlarm(Const.ONE_MINUTE);
        }
    }

    private void setRetryAlarm() {
        Log.i(TAG, "setRetryAlarm");
        if (mRetryTimes < TIMES_MAX_RETRY) {
            mRetryTimes++;
            Log.i(TAG, "[setRetryAlarm] retry after 10 minutes ");
            mRetryAlarmListener = new CustomizedAlarmListener(MSG_RETRY_REGISTER);
            PlatformManager.setElapsedAlarm(mContext, mRetryAlarmListener, Const.TEN_MINUTES);
        } else {
            Log.i(TAG, "[setRetryAlarm] reach limit " + mRetryTimes);
        }
        resetParameters();
    }

    private void cancelRetryAlarm() {
        if (mRetryAlarmListener != null) {
            Log.i(TAG, "cancelRetryAlarm");
            PlatformManager.cancelAlarm(mContext, mRetryAlarmListener);
            mRetryAlarmListener = null;
        }
        mRetryTimes = 0;
    }

    private void setInServiceDelayAlarm() {
        setInServiceDelayAlarm(Const.ONE_MINUTE);
    }

    private void setInServiceDelayAlarm(long delay) {
        Log.i(TAG, "setInServiceDelayAlarm " + (delay / Const.ONE_SECOND) + "s");
        OnAlarmListener listener = new CustomizedAlarmListener(MSG_SIM_INSERVICE);
        PlatformManager.setElapsedAlarm(mContext, listener, delay);
        mHasInServiceAlarm = true;
    }

    private boolean needRegister() {

        if (mUimSlot == Const.SLOT_ID_INVALID) {
            Log.w(TAG, "[needRegister] no valid UIM");
            return false;
        }

        if (mPlatformManager.isNetworkRoaming(mUimSlot)) {
            Log.w(TAG, "[needRegister] uim " + mUimSlot + " is roaming");
            return false;
        }

        // Already send a message but not get response
        if (mWaitForResponse) {
            return isUimChangeOrTimeout();
        } else {
            Log.i(TAG, "[needRegister] not wait for response");
        }

        if (mAgentProxy.isRegistered()) {
            Log.d(TAG, "[needRegister] registered before, check imsi and meid.");

            if (isRegisterImsiSame() && isImsiSame() && isEsnOrMeidSame()) {
                Log.d(TAG, "[needRegister] same imsi and meid");
                return false;
            }
        }

        return true;
    }

    /**
     * Already send a message, whether uim changes or time out
     * @return true if Uim changes or last message sent > 5 minutes;
     *         false: same Uim and
     */
    private boolean isUimChangeOrTimeout() {
        // Uim changes, do register
        String currentImsi = getCurrentCDMAImsi();
        if (!currentImsi.equals(mRegisterImsi)) {
            Log.i(TAG, "[needRegister] Uim changes, do register");
            return true;
        }

        // Uim not change, but not get response > 5 minutes, do register
        long interval = System.currentTimeMillis() - mSendMessageTime;
        Log.i(TAG, "Last register/current is " + mSendMessageTime + "/"
                + System.currentTimeMillis() + ", interval is " + interval);
        if (interval >= Const.FIVE_MINUTES) {
            Log.i(TAG, "[needRegister] same Uim, no response > 5 minutes, do register");
            return true;
        } else {
            Log.i(TAG, "[needRegister] Uim not change, wait for response");
            return false;
        }
    }

    private void initIdentityInfo() {
        initUimSlot();

        mImsiOnSim = new String[mSlotList.length];
        for (int i = 0; i < mSlotList.length; ++i) {
            mImsiOnSim[i] = mPlatformManager.getImsiInfo(mSlotList[i]);
        }

        mMeid = mPlatformManager.getDeviceMeid(mSlotList);
    }

    private void initUimSlot() {
        mUimSlot = Const.SLOT_ID_INVALID;

        // 1. try mobile data slot
        int masterSlot = mPlatformManager.getDefaultSim();

        // 2. If master valid UIM, use this; else, first valid UIM
        if (masterSlot != Const.SLOT_ID_INVALID
                && mPlatformManager.isValidCdmaUim(masterSlot)
                && mPlatformManager.isValidCdmaNetwork(masterSlot)) {
            mUimSlot = masterSlot;

        } else {
            for (int slotId : mSlotList) {
                if (slotId != masterSlot && mPlatformManager.isValidCdmaUim(slotId)
                        && mPlatformManager.isValidCdmaNetwork(slotId)) {
                    mUimSlot = slotId;
                }
            }
        }

        Log.i(TAG, "[initUimSlot] Uim slot is " + mUimSlot);
    }

    private boolean isUimSlotValid() {
        return mUimSlot != Const.SLOT_ID_INVALID;
    }

    private boolean isRegisterImsiSame() {
        String registeredImsi = mAgentProxy.getRegisterImsi();
        String currentCdmaImsi = getCurrentCDMAImsi();

        Log.i(TAG, "Registered/current imsi " + PlatformManager.encryptMessage(registeredImsi)
                + "/" + PlatformManager.encryptMessage(currentCdmaImsi));

        return registeredImsi.equals(currentCdmaImsi);
    }

    private boolean isImsiSame() {
        String[] imsiSaved = mAgentProxy.getSavedImsi();

        for (int i = 0; i < imsiSaved.length; ++i) {
            imsiSaved[i] = mPlatformManager.getProcessedImsi(imsiSaved[i]);
        }

        return Utils.compareUnsortArray(mImsiOnSim, imsiSaved);
    }

    private boolean isEsnOrMeidSame() {
        // 1. mPreviousEsnOrMeid set from "doAfterMessageSend"
        if (mPreviousEsnOrMeid != null && mPreviousEsnOrMeid.equals(mMeid)) {
            Log.i(TAG, "Previous MEID same to device, not from FW, return true");
            return true;
        }

        // 2. mPreviousEsnOrMeid set from framework
        String rawEsnOrMeid = mPreviousEsnOrMeid;
        String esnOrMeid = EsnWrapper.processResponseFromFW(rawEsnOrMeid);

        Log.i(TAG, "device meid is " + PlatformManager.encryptMessage(mMeid));

        if (mMeid.length() == esnOrMeid.length()) {
            return mMeid.equals(esnOrMeid);
        }

        String deviceEsn = EsnWrapper.convertToEsn(mMeid);
        String savedEsn = EsnWrapper.convertToEsn(esnOrMeid);

        Log.i(TAG, "ESN device/saved are " + PlatformManager.encryptMessage(deviceEsn)
                + "/" + PlatformManager.encryptMessage(savedEsn));

        return deviceEsn.equals(savedEsn);
    }

    private void sendRegisterMessage() {
        RegisterMessage registerMessage = new RegisterMessage(this);
        byte[] data = registerMessage.getRegisterMessage();

        Intent intent = new Intent(Const.ACTION_MESSAGE_SEND);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);

        Log.i(TAG, "[sendRegisterMessage] slot: " + mUimSlot + ", len: " + data.length
                + ", content (hide sensitive): " + registerMessage.encryptMeidImsi());

        try {
            PlatformManager.SmsWrapper.sendRegisterMessage(mDeviceRegisterController,
                    data, pendingIntent, mUimSlot);
        } catch (UnsupportedOperationException | SecurityException  e) {
            Log.i(TAG, "Exception " + e);
            e.printStackTrace();
            setRetryAlarm();
        }
    }

    public String getImei(int slotId) {
        return mPlatformManager.getImei(slotId);
    }

    public String getDeviceMeid() {
        return mMeid;
    }

    public String getCurrentCDMAImsi() {
        return mPlatformManager.getCdmaImsiForCT(mUimSlot);
    }

    private void registerPhoneListeners() {

        for (int i = 0; i < mSlotList.length; ++i) {

            int[] subId = PlatformManager.getSubId(mSlotList[i]);

            if (PlatformManager.isSubIdsValid(subId)) {

                if (subId[0] == mSubIdSnapShot[i]) {
                    Log.i(TAG, "[registerPhoneListeners] Slot " + i + " already" +
                            " registered.");
                } else {
                    Log.i(TAG, "[registerPhoneListeners] Slot " + i + " subId changed," +
                            " refresh");
                    unRegisterPhoneListener(mPhoneStateListener[i]);

                    mSubIdSnapShot[i] = subId[0];
                    mPhoneStateListener[i] = new CustomizedPhoneStateListener(i, subId[0]);
                    registerPhoneListener(mPhoneStateListener[i]);
                }

            } else {
                if (mPhoneStateListener[i] != null) {
                    Log.i(TAG, "[registerPhoneListeners] Slot " + i + " unplug, unregister");
                    unRegisterPhoneListener(mPhoneStateListener[i]);
                    mSubIdSnapShot[i] = Const.SUB_ID_INVALID;
                    mPhoneStateListener[i] = null;
                } else {
                    Log.i(TAG, "[registerPhoneListeners] No need for slot " + i);
                }

            }
        }
    }

    private void unRegisterPhoneListeners() {
        Log.i(TAG, "unRegisterPhoneListeners");

        for (int i = 0; i < mPhoneStateListener.length; ++i) {
            if (mPhoneStateListener[i] != null) {
                unRegisterPhoneListener(mPhoneStateListener[i]);
                mSubIdSnapShot[i] = Const.SUB_ID_INVALID;
                mPhoneStateListener[i] = null;
            }
        }
    }

    private void registerPhoneListener(CustomizedPhoneStateListener listener) {
        if (listener != null) {
            mPlatformManager.registerPhoneListener(listener, listener.mSubId);
        }
    }

    private void unRegisterPhoneListener(CustomizedPhoneStateListener listener) {
        if (listener != null) {
            mPlatformManager.unRegisterPhoneListener(listener, listener.mSubId);
        }
    }

    private void registerStartupReceiver() {
        Log.i(TAG, "registerStartupReceiver");
        if (mStartupReceiver == null) {
            mStartupReceiver = new StartupReceiver();
            IntentFilter filter = new IntentFilter();
            filter.addAction(Const.ACTION_BOOT_COMPLETED);
            filter.addAction(Const.ACTION_SIM_STATE_CHANGED);
            filter.addAction(Const.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
            mContext.registerReceiver(mStartupReceiver, filter);
        }
    }

    private void unRegisterStartupReceiver() {
        Log.i(TAG, "unRegisterStartupReceiver");
        if (mStartupReceiver != null) {
            mContext.unregisterReceiver(mStartupReceiver);
            mStartupReceiver = null;
        }
    }

    private void registerSendReceiver() {
        if (mSmsSendReceiver == null) {
            mSmsSendReceiver = new SmsSendReceiver();
            IntentFilter filter = new IntentFilter();
            filter.addAction(Const.ACTION_MESSAGE_SEND);
            mContext.registerReceiver(mSmsSendReceiver, filter);
        }
    }

    private void unRegisterSendReceiver() {
        if (mSmsSendReceiver != null) {
            mContext.unregisterReceiver(mSmsSendReceiver);
            mSmsSendReceiver = null;
        }
    }

    private boolean isVersion2019V1() {
        return mVersion.equals(Const.VERSION_2019V1);
    }

    private class CustomizedAlarmListener implements OnAlarmListener {

        private int mMsgType;

        public CustomizedAlarmListener(int type) {
            mMsgType = type;
        }

        @Override
        public void onAlarm() {
            mHandler.sendEmptyMessage(mMsgType);
        }
    }

    private class CustomizedPhoneStateListener extends PhoneStateListener {

        private int mSlotId;
        private int mSubId;

        public CustomizedPhoneStateListener(int slotId, int subId) {
            mSlotId = slotId;
            mSubId = subId;
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            super.onServiceStateChanged(serviceState);

            if (mPlatformManager.isValidCdmaNetwork(serviceState)) {
                Log.i(TAG, "[onService " + mSlotId + "] voice/data in service");

                // As SIM info may not ready, can't check whether it's UIM here
                if (!mHasInServiceAlarm) {
                    setInServiceDelayAlarm();
                }

            } else {
                Log.i(TAG, "[onService " + mSlotId + "] not in service");
            }
        }
    }

    private class SmsSendReceiver extends BroadcastReceiver {
        private static final String TAG = Const.TAG_PREFIX + "SmsSendReceiver";

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "onReceive " + intent);

            String action = intent.getAction();
            if (action.equalsIgnoreCase(Const.ACTION_MESSAGE_SEND)) {
                int resultCode = getResultCode();

                if (resultCode == Activity.RESULT_OK) {
                    mHandler.sendEmptyMessage(MSG_MESSAGE_SEND);

                } else {
                    Log.i(TAG, "ResultCode: " + resultCode + ", failed.");
                    setRetryAlarm();
                }

            } else {
                Log.i(TAG, "action is not valid." + action);
            }
        }
    }

    private class StartupReceiver extends BroadcastReceiver {

        private static final String TAG = Const.TAG_PREFIX + "StartupReceiver";

        @Override
        public void onReceive(Context context, Intent intent) {

            boolean isSystem = android.os.Process.myUserHandle().isSystem();
            if (!isSystem) {
                Log.d(TAG, "This is not system user, return.");
                return;
            }

            if (!isSwitchOpen()) {
                Log.i(TAG, "Feature is not enabled, do nothing");
                return;
            }

            String action = intent.getAction();
            if (action.equalsIgnoreCase(Const.ACTION_BOOT_COMPLETED)) {
                Log.d(TAG, "onReceive " + intent);
                mHandler.sendEmptyMessage(MSG_BOOT_COMPLETED);

            } else if (action.equalsIgnoreCase(Const.ACTION_SIM_STATE_CHANGED)) {
                String state = intent.getStringExtra(Const.EXTRA_ICC_STATE);
                int phoneId = intent.getIntExtra(Const.PHONE_KEY, Const.SLOT_ID_INVALID);

                Log.i(TAG, "SIM_STATE_CHANGED phoneId " + phoneId + ", state is " + state);
                if (state.equals(Const.VALUE_ICC_LOADED) || state.equals(Const.VALUE_ICC_ABSENT)) {
                    onSimStateChanged();
                }
            } else if (action.equals(Const.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED)) {
                int subId = intent.getIntExtra(Const.SUBSCRIPTION_KEY, Const.SUB_ID_INVALID);

                onDataSubChanged(subId);
            } else {
                Log.i(TAG, "Invalid action " + action);
            }
        }

        private boolean isSwitchOpen() {
            return PlatformManager.isFeatureEnabled();
        }
    }

}

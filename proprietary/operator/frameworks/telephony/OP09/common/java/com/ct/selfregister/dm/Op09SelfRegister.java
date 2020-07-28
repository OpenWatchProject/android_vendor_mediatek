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

package com.ct.selfregister.dm;

import android.app.AlarmManager.OnAlarmListener;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.telephony.CellLocation;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.util.Log;

import com.ct.selfregister.dm.utils.AgentProxy;
import com.ct.selfregister.dm.utils.PlatformManager;
import com.ct.selfregister.dm.utils.Utils;

import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Iterator;

/**
 * Service which process the main logic of registration.
 */
public class Op09SelfRegister {

    public static final String TAG = Const.TAG_PREFIX + "RegisterService";

    // Messages related to android intent
    private static final int MSG_BOOT_COMPLETED = 1;
    private static final int MSG_SIM_STATE_CHANGED = 2;
    private static final int MSG_DEFAULT_DATA_SUBSCRIPTION_CHANGED = 3;

    // Messages related to flow control
    private static final int MSG_WATCH_DOG = 21;
    private static final int MSG_SIM_INSERVICE = 22;
    private static final int MSG_SERVER_RESPONSE = 23;
    private static final int MSG_PERIOD_REGISTER = 24;

    // Messages related to retry
    private static final int MSG_RETRY = 31;
    private static final int MSG_RETRY_DATASUB = 32;
    private static final int MSG_RETRY_PERIOD = 33;

    private static final int LIMIT_MESSAGE_LIST = 30;

    private static final int TIMES_MAX_WAIT = 3;
    private static final int TIMES_MAX_RETRY = 10;

    private static final int TYPE_INVALID = -1;
    private static final int TYPE_COMPARE_ICCID = 1;
    private static final int TYPE_NO_COMPARE_DATASUB_CHANGE = 2;
    private static final int TYPE_NO_COMPARE_PERIOD = 3;

    private int[] mSlotList;
    private String[] mIccIdList;
    private int[] mSubIdSnapShot;
    private CustomizedPhoneStateListener[] mPhoneStateListener;

    private Context mContext;
    private AgentProxy mAgentProxy;
    private PlatformManager mPlatformManager;

    private int mMasterSlot = Const.SLOT_ID_INVALID;
    private int mDataSub = Const.SUB_ID_INVALID;

    private int mWaitTimes = 0;
    private int mRetryTimes = 0;

    private int mRegisterType = -1;
    private boolean mHasInServiceAlarm = false;
    private boolean mAfterBootup = false;

    private ArrayList<Integer> mMessageList = new ArrayList<Integer>();

    private OnAlarmListener mWatchDogAlarmListener;
    private OnAlarmListener mRetryAlarmListener;
    private OnAlarmListener mPeriodicAlarmListener;

    private StartupReceiver mStartupReceiver;

    private Handler mHandler;
    private HandlerThread mHandlerThread;

    private static Op09SelfRegister sInstance = null;
    public static void newSelfRegister(Context context) {
        if (sInstance == null) {
            sInstance = new Op09SelfRegister(context);
        }
    }

    public Op09SelfRegister(Context context) {
        mContext = context;
        if (PlatformManager.isFeatureSupported()) {
            init();
        }
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
        mIccIdList = new String[mSlotList.length];
        mSubIdSnapShot = new int[mSlotList.length];

        for (int i = 0; i < mSlotList.length; ++i) {
            mPhoneStateListener[i] = null;
            mIccIdList = null;
            mSubIdSnapShot[i] = Const.SUB_ID_INVALID;
        }
        registerStartupReceiver();

        initHandler();
        mPeriodicAlarmListener = new CustomizedAlarmListener(MSG_PERIOD_REGISTER);
    }

    private void resetParameters() {
        Log.i(TAG, "resetParameters at " + System.currentTimeMillis());

        // not reset mDataSub & mRetryTimes
        mWaitTimes = 0;
        mHasInServiceAlarm = false;
        mAfterBootup = false;
        unRegisterPhoneListeners();
        mMessageList.clear();
    }

    private void initHandler() {

        mHandlerThread = new HandlerThread("SelfRegister");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper()) {

            @Override
            public void handleMessage(Message msg) {
                Log.i(TAG, "[handleMessage] msg is " + msg);

                switch (msg.what) {
                case MSG_BOOT_COMPLETED:
                    doAfterBoot();
                    break;

                case MSG_SIM_STATE_CHANGED:
                case MSG_DEFAULT_DATA_SUBSCRIPTION_CHANGED:
                    addMessageToList(msg.what);
                    registerPhoneListeners();
                    break;

                case MSG_PERIOD_REGISTER:
                    doForPeriod();
                    addMessageToList(msg.what);
                    break;

                case MSG_SIM_INSERVICE:
                    processActionList();
                    doAfterSimInservice();
                    break;

                case MSG_RETRY:
                case MSG_RETRY_DATASUB:
                case MSG_RETRY_PERIOD:
                    addMessageToList(msg.what);
                    doAfterRetry();
                    break;

                case MSG_WATCH_DOG:
                    doForWatchDog();
                    break;

                case MSG_SERVER_RESPONSE:
                    handleResponse(msg);
                    break;

                default:
                    break;
                }
            }
        };
    }

    private void doAfterBoot() {
        mAfterBootup = true;

        // boot complete may come later than SIM IN_SERVICE
        if (!mHasInServiceAlarm) {
            setWatchDogAlarm();

            if (!mMessageList.contains(MSG_SIM_STATE_CHANGED)) {
                startServiceIfSimLoaded();
            }
        }

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
        } else {
            Log.i(TAG, "Same fingerprint, do nothing");
        }
    }

    private void startServiceIfSimLoaded() {
        Intent intent = mContext.registerReceiver(null,
                new IntentFilter(Const.ACTION_SIM_STATE_CHANGED));

        // If SIM_STATE_CHANGED all received when device locked, not register
        // PhoneStateListener and register will fail. Start service here if has previous
        // SIM_STATE_CHANGED (absent/loaded) intent.
        if (intent != null) {
            String state = intent.getStringExtra(Const.EXTRA_ICC_STATE);
            int phoneId = intent.getIntExtra(Const.PHONE_KEY, Const.SLOT_ID_INVALID);

            if (state.equals(Const.VALUE_ICC_LOADED) || state.equals(Const.VALUE_ICC_ABSENT)) {
                Log.i(TAG, "PhoneId " + phoneId + ", state is " + state + ", startService");
                mHandler.sendEmptyMessage(MSG_SIM_STATE_CHANGED);
            }
        }
    }

    private void doAfterRetry() {
        Log.i(TAG, "doAfterRetry");
        mRetryAlarmListener = null;
        if (!mHasInServiceAlarm) {
            setInServiceDelayAlarm(Const.ONE_MINUTE);
        }
    }

    /*
     * trigger a register and set next periodic register
     */
    private void doForPeriod() {
        if (!mHasInServiceAlarm) {
            setInServiceDelayAlarm(Const.ONE_MINUTE);
        }

        // next register after 30 days
        setPeriodicAlarm(Const.PERIOD_INTERVAL_NEXT);
    }

    /*
     * Continue period register triggered in last boot up. Scenarios:
     *  1. Boot up with same SIM cards
     *  2. Boot up but no SIM becomes IN_SERVICE in 10 minutes
     */
    private void continuePeriodRegister() {
        if (mAfterBootup) {
            long triggerTime = mAgentProxy.getPeriodTriggerTime();
            if (triggerTime > 0) {
                long interval = triggerTime - System.currentTimeMillis();
                Log.i(TAG, "continuePeriodRegister: interval is " + interval);

                if (interval > 0) {
                    setPeriodicAlarm(interval);
                } else {
                    setPeriodicAlarm(Const.ONE_MINUTE);
                }
            } else {
                Log.i(TAG, "No period register in last boot up");
            }
        }
    }

    private void addMessageToList(int type) {
        // In auto test, lots of SIM/data sub intent may happen. Ignore if reach to limit
        if (mMessageList.size() > LIMIT_MESSAGE_LIST) {
            return;
        }
        mMessageList.add((Integer) type);
    }

    private void processActionList() {
        if (mWaitTimes > 0) {
            return ;
        }

        // Update register type (remove header), and refresh alarm if needed
        if (mMessageList.size() > 0) {
            int message = mMessageList.get(0);
            mRegisterType = getActionType(message);

            // Refresh if triggered by condition change
            if (message == MSG_SIM_STATE_CHANGED
                    || message == MSG_DEFAULT_DATA_SUBSCRIPTION_CHANGED) {
                cancelRetryAlarm();
            }
        }
        dumpActionList("After process");
    }

    private int getActionType(int type) {
        if (type == MSG_SIM_STATE_CHANGED || type == MSG_RETRY) {
            return TYPE_COMPARE_ICCID;

        } else if (type == MSG_DEFAULT_DATA_SUBSCRIPTION_CHANGED
                || type == MSG_RETRY_DATASUB) {
            return TYPE_NO_COMPARE_DATASUB_CHANGE;

        } else if (type == MSG_PERIOD_REGISTER
                || type == MSG_RETRY_PERIOD) {
            return TYPE_NO_COMPARE_PERIOD;
        }
        return TYPE_INVALID;
    }

    private boolean isTypeValid(int type) {
        if (type == TYPE_COMPARE_ICCID
                || type == TYPE_NO_COMPARE_DATASUB_CHANGE
                || type == TYPE_NO_COMPARE_PERIOD) {
            return true;
        } else {
            return false;
        }
    }

    private void dumpActionList(String prefix) {
        Log.i(TAG, prefix + ", list size " + mMessageList.size());
        for (int message: mMessageList) {
            String action = "";
            switch (message) {
            case MSG_SIM_STATE_CHANGED:
                action = "SIM_STATE_CHANGED";
                break;

            case MSG_DEFAULT_DATA_SUBSCRIPTION_CHANGED:
                action = "DEFAULT_DATA_SUBSCRIPTION_CHANGED";
                break;

            case MSG_PERIOD_REGISTER:
                action = "PERIOD_REGISTER";
                break;

            case MSG_RETRY:
                action = "RETRY";
                break;

            case MSG_RETRY_DATASUB:
                action = "RETRY_DATA_SUB";
                break;

            case MSG_RETRY_PERIOD:
                action = "RETRY_PERIOD";
                break;

            default:
                break;
            }
            Log.i(TAG, "Action is " + action);
        }
    }

    private void doAfterSimInservice() {
        Log.i(TAG, "doAfterSimInservice, type is " + mRegisterType
                + ", wait time " + mWaitTimes);

        if (!isTypeValid(mRegisterType)) {
            Log.e(TAG, "Invalid register type " + mRegisterType);
            return;
        }

        if (!mPlatformManager.areSlotsInfoReady(mSlotList) && mWaitTimes < TIMES_MAX_WAIT) {
            long second = (mWaitTimes % 3 + 1) * 10 * Const.ONE_SECOND;
            setInServiceDelayAlarm(second);
            mWaitTimes++;
            return;
        }

        if (!mPlatformManager.areValidOperatorSim(mSlotList)) {
            Log.e(TAG, "SIM operator not valid, do nothing.");
            return;
        }

        initIdentityInfo();

        if (!needRegister()) {
            checkNextAction();
            return;
        }

        if (isLocationValid() && isNetworkTypeValid()) {
            RegisterMessage registerMessage = new RegisterMessage(this);

            // save real message to shared preference
            mAgentProxy.setRegisterMessage(registerMessage.getRealMessage());

            final String message = registerMessage.getBase64Message();
            new Thread(new Runnable() {

                @Override
                public void run() {
                    JSONObject response = Utils.httpSend(message);
                    sendResponseToHandler(response);
                }

            }).start();
        } else {
            doContiditionFail();
        }
    }

    /*
     * Continue to wait or retry if condition check fail
     */
    private void doContiditionFail() {
        if (mWaitTimes < TIMES_MAX_WAIT) {
            mWaitTimes++;
            long second = (mWaitTimes % 3 + 1) * 10 * Const.ONE_SECOND;
            setInServiceDelayAlarm(second);

        } else {
            setRetryAlarm();
            resetParameters();
        }
    }

    /*
     * If no SIM IN_SERVICE in 10 times after boot up, set retry and continue the period register
     * in last boot up.
     */
    private void doForWatchDog() {
        setRetryAlarm();
        continuePeriodRegister();
        resetParameters();
    }

    /*
     * If not register to network in 10 minutes, set a retry alarm.
     */
    private void setWatchDogAlarm() {
        Log.i(TAG, "setWatchDogAlarm for 10 minutes");
        mWatchDogAlarmListener = new CustomizedAlarmListener(MSG_WATCH_DOG);
        PlatformManager.setElapsedAlarm(mContext, mWatchDogAlarmListener, 10 * Const.ONE_MINUTE);
    }

    private void cancelWatchDogAlarm() {
        if (mWatchDogAlarmListener != null) {
            Log.i(TAG, "cancelWatchDogAlarm");
            PlatformManager.cancelAlarm(mContext, mWatchDogAlarmListener);
            mWatchDogAlarmListener = null;
        }
    }

    /*
     * Trigger when SIM IN_SERVICE, unregister listen to avoid further state
     * change
     */
    private void setInServiceDelayAlarm() {
        setInServiceDelayAlarm(Const.ONE_MINUTE);
        cancelWatchDogAlarm();
    }

    private void setInServiceDelayAlarm(long delay) {
        Log.i(TAG, "setInServiceDelayAlarm " + (delay / Const.ONE_SECOND) + "s");
        OnAlarmListener listener = new CustomizedAlarmListener(MSG_SIM_INSERVICE);
        PlatformManager.setElapsedAlarm(mContext, listener, delay);
        mHasInServiceAlarm = true;
    }

    private void setRetryAlarm() {
        if (!isTypeValid(mRegisterType)) {
            // If type not valid and message list empty, return without retry
            if (mMessageList.size() == 0) {
                Log.e(TAG, "Sim in unknown/lock state, just return.");
                return;
            }

            // If SIM never register to network, mRegisterType is not inited
            // and need to update here
            mRegisterType = getActionType(mMessageList.get(0));
        }

        if (mRetryTimes < TIMES_MAX_RETRY) {
            int retryType = MSG_RETRY;
            if (mRegisterType == TYPE_NO_COMPARE_DATASUB_CHANGE) {
                retryType = MSG_RETRY_DATASUB;
            } else if (mRegisterType == TYPE_NO_COMPARE_PERIOD) {
                retryType = MSG_RETRY_PERIOD;
            }

            mRetryTimes++;
            Log.i(TAG, "Retry after 1h, times " + mRetryTimes + ", type " + retryType);

            mRetryAlarmListener = new CustomizedAlarmListener(retryType);
            PlatformManager.setRtcAlarm(mContext, mRetryAlarmListener, Const.ONE_HOUR);
        } else {
            Log.i(TAG, "Already retried " + mRetryTimes + " times");
        }
    }

    private void cancelRetryAlarm() {
        if (mRetryAlarmListener != null) {
            Log.i(TAG, "cancelRetryAlarm");
            PlatformManager.cancelAlarm(mContext, mRetryAlarmListener);
            mRetryAlarmListener = null;
        }
        mRetryTimes = 0;
    }

    private void triggerFirstPeriodAlarm() {
        if (mMessageList.size() > 0) {
            int message = mMessageList.get(0);
            // Refresh if triggered by condition change
            if (message == MSG_SIM_STATE_CHANGED
                    || message == MSG_DEFAULT_DATA_SUBSCRIPTION_CHANGED) {

                // Cancel former and set a new with 30.5 days
                cancelPeriodicAlarm();
                setPeriodicAlarm(Const.PERIOD_INTERVAL_FIRST);
            }
        }
    }

    private void setPeriodicAlarm(long interval) {
        Log.i(TAG, "setPeriodicAlarm with " + (1.0 * interval / Const.ONE_DAY) + " days");
        PlatformManager.setRtcAlarm(mContext, mPeriodicAlarmListener, interval);
        mAgentProxy.setPeriodTriggerTime(interval);
    }

    private void cancelPeriodicAlarm() {
        PlatformManager.cancelAlarm(mContext, mPeriodicAlarmListener);
    }

    /*
     * Send the response to handler (process on main thread)
     */
    private void sendResponseToHandler(JSONObject response) {
        mHandler.sendMessage(mHandler.obtainMessage(MSG_SERVER_RESPONSE, response));
    }

    private void handleResponse(Message message) {
        JSONObject response = (JSONObject) message.obj;
        boolean result = Utils.checkRegisterResult(response);

        if (result) {
            Log.i(TAG, "analyseResponse(), resultCode:0 resultDesc:Success");

            mAgentProxy.setSavedIccId(mIccIdList);
            mAgentProxy.setRegisterFlag(true);

            triggerFirstPeriodAlarm();
        } else {
            Log.e(TAG, "Register fail!");
            setRetryAlarm();
        }
        resetParameters();
    }

    /*
     * If not register in last round (no need), check the action list to find a different type
     * and trigger register flow with new type. If All action have same type to last round, just
     * ignore and stop.
     *
     * Example 1:
     *   Last action is SIM_STATE_CHANGED (TYPE_COMPARE_ICCID), and no need to register.
     *   Action list is {
     *      1. SIM_STATE_CHANGED,
     *      2. SIM_STATE_CHANGED,
     *      3. DEFAULT_DATA_SUBSCRIPTION_CHANGED,
     *      4. SIM_STATE_CHANGED,
     *      5. DEFAULT_DATA_SUBSCRIPTION_CHANGED,}
     *   Find action 3 has different type (TYPE_NO_COMPARE_DATA_SUB), and trigger a new register flow.
     *
     * Example 2:
     *   Last action is SIM_STATE_CHANGED (TYPE_COMPARE_ICCID), and no need to register.
     *   Action list is {
     *      1. SIM_STATE_CHANGED,
     *      2. SIM_STATE_CHANGED,
     *      3. RETRY}
     *   All action is list have same type to last round, just quit.
     */
    private void checkNextAction() {
        dumpActionList("Last register type: " + mRegisterType);

        removeType(mRegisterType);

        if (mMessageList.size() > 0) {
            mRegisterType = getActionType(mMessageList.get(0));
            doAfterSimInservice();
            return;
        }

        resetParameters();
    }

    private void removeType(int type) {
        Iterator<Integer> iterator = mMessageList.iterator();
        while (iterator.hasNext()) {
            int itemType = getActionType(iterator.next());
            if (type == itemType) {
                iterator.remove();
            }
        }
    }

    private boolean needRegister() {
        // 1. master/slave switch only occur when there're two cards
        if (mRegisterType == TYPE_NO_COMPARE_DATASUB_CHANGE) {
            if (mPlatformManager.isSingleLoad()) {
                Log.i(TAG, "[needRegister] single load can't master/slave switch");
                return false;
            }

            for (int slotId: mSlotList) {
                if (!mPlatformManager.hasIccCard(slotId)) {
                    Log.i(TAG, "[needRegister] slot " + slotId + " no SIM");
                    return false;
                }
            }
        }

        // 2. No need if roaming
        for (int i = 0; i < mSlotList.length; ++i) {
            if (mPlatformManager.isNetworkRoaming(mSlotList[i])) {
                Log.i(TAG, "[needRegister] Sim " + i + " roaming, no need");
                return false;
            }
        }

        if (mRegisterType == TYPE_COMPARE_ICCID) {
            // 3. If already registered and iccIds not change, no need
            if (mAgentProxy.isRegistered()) {

                if (isIccIdSame()) {
                    Log.i(TAG, "[needRegister] same iccid, no need");

                    // Boot up with same SIM cards, continue period register
                    continuePeriodRegister();
                    return false;
                }
            }
        }

        // 4. period register
        if (mRegisterType == TYPE_NO_COMPARE_PERIOD) {
            Log.i(TAG, "Period register, return true");
        }
        return true;
    }

    private boolean isIccIdSame() {
        String[] iccidSaved = mAgentProxy.getSavedIccId();

        for (int i = 0; i < iccidSaved.length; ++i) {
            iccidSaved[i] = mPlatformManager.getProcessedIccid(iccidSaved[i]);
        }

        return Utils.compareUnsortArray(mIccIdList, iccidSaved);
    }

    private void initIdentityInfo() {
        mMasterSlot = getMasterSlot();

        mIccIdList = new String[mSlotList.length];
        for (int i = 0; i < mIccIdList.length; ++i) {
            mIccIdList[i] = mPlatformManager.getIccId(mSlotList[i]);
        }
    }

    private int getMasterSlot() {
        // 1. try mobile data slot
        int slotId = PlatformManager.getDefaultSim();
        if (slotId != Const.SLOT_ID_INVALID && mPlatformManager.hasIccCard(slotId)) {
            return slotId;
        }

        // 2. Use first slot not empty
        for (int i : mSlotList) {
            if (mPlatformManager.hasIccCard(i)) {
                slotId = i;
                break;
            }
        }
        return slotId;
    }

    /*
     * Whether network is value: should be Wi-Fi on data link on a CT card
     */
    private boolean isNetworkTypeValid() {
        if (hasWiFiConnection()) {
            Log.i(TAG, "[isNetworkTypeValid] find Wi-Fi, network valid");
            return true;
        }

        if (hasMobileLinkConnection()) {
            int slotId = PlatformManager.getDefaultSim();

            if (mPlatformManager.isValidUim(slotId)) {
                return true;
            }
        }
        Log.i(TAG, "[isNetworkTypeValid] no Wi-Fi or invalid data link");
        return false;
    }

    private boolean hasWiFiConnection() {
        return mPlatformManager.hasNetworkConnection(ConnectivityManager.TYPE_WIFI);

    }

    private boolean hasMobileLinkConnection() {
        return mPlatformManager.hasNetworkConnection(ConnectivityManager.TYPE_MOBILE);
    }

    public int getDataSim() {
        return mMasterSlot;
    }

    // ------------------------------------------------------
    // Wrapper of PlatformManager
    // ------------------------------------------------------

    public PlatformManager getPlatformManager() {
        return mPlatformManager;
    }

    public String getIccIdFromCard(int slotId) {
        return mIccIdList[slotId];
    }

    public String[] getComplexImsi(int slotId) {
        String imsiArray[] = mPlatformManager.getComplexImsi(slotId);
        return imsiArray;
    }

    public String getMeid() {
        String meid = mPlatformManager.getDeviceMeid(mSlotList);
        Log.i(TAG, "meid is " + PlatformManager.encryptMessage(meid));
        return meid;
    }

    public String getImei(int slotId) {
        String imei = mPlatformManager.getImei(slotId);
        Log.i(TAG, "imei " + slotId + " is " + PlatformManager.encryptMessage(imei));
        return imei;
    }

    public String getMacId() {
        return mPlatformManager.getMacAddress();
    }

    // ------------------------------------------------------
    // Location info
    // ------------------------------------------------------

    private boolean isLocationValid() {
        for (int i=0; i < mSlotList.length; ++i) {
            if (getCellId(i) > 0) {
                return true;
            }
        }
        return false;
    }

    public int getCellId(int slot) {
        // if not IN_service (register to network)
        if (!mPlatformManager.isInService(slot)) {
            return Const.VALUE_INVALID_INT;
        }

        int value = mPlatformManager.getCellId(slot);
        if (value > 0) {
            return value;
        }
        // No cell id found, try base id
        return getCdmaBaseId(slot);
    }

    public int getCdmaBaseId(int slot) {
        int baseId = Const.VALUE_INVALID_INT;

        CellLocation location = mPlatformManager.getCellLocation(slot);
        if (location instanceof CdmaCellLocation) {
            baseId = ((CdmaCellLocation) location).getBaseStationId();
        } else if (location instanceof GsmCellLocation) {
            baseId = ((GsmCellLocation) location).getCid();
        }
        Log.i(TAG, "[getCdmaBaseId " + slot + "] is " + PlatformManager.encryptMessage(baseId));
        return baseId;
    }

    private void registerStartupReceiver() {
        if (mStartupReceiver == null) {
            mStartupReceiver = new StartupReceiver();
            IntentFilter filter = new IntentFilter();
            filter.addAction(Const.ACTION_BOOT_COMPLETED);
            filter.addAction(Const.ACTION_SIM_STATE_CHANGED);
            filter.addAction(Const.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
            mContext.registerReceiver(mStartupReceiver, filter);
        }
    }

    private void registerPhoneListeners() {

        for (int i = 0; i < mSlotList.length; ++i) {

            int[] subId = PlatformManager.getSubId(mSlotList[i]);

            if (PlatformManager.isSubIdsValid(subId)) {

                if (subId[0] == mSubIdSnapShot[i]) {
                    Log.i(TAG, "[registerPhoneListeners] Slot " + i + " already registered.");
                } else {
                    Log.i(TAG, "[registerPhoneListeners] Slot " + i + " subId changed, refresh");
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

            if (mPlatformManager.isInService(serviceState)) {
                Log.i(TAG, "[onService " + mSlotId + "] voice/data in service");

                if (!mHasInServiceAlarm) {
                    setInServiceDelayAlarm();
                }
            } else {
                Log.i(TAG, "[onService " + mSlotId + "] not in service ");
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

            if(!isSwitchOpen()) {
                Log.i(TAG, "Feature is not enabled, do nothing");
                return;
            }

            String action = intent.getAction();

            if (action.equalsIgnoreCase(Const.ACTION_SIM_STATE_CHANGED)) {
                String state = intent.getStringExtra(Const.EXTRA_ICC_STATE);
                int phoneId = intent.getIntExtra(Const.PHONE_KEY, Const.SLOT_ID_INVALID);

                if (state.equals(Const.VALUE_ICC_LOADED) || state.equals(Const.VALUE_ICC_ABSENT)) {
                    Log.i(TAG, "PhoneId " + phoneId + ", state is " + state + ", go to service");
                    mHandler.sendEmptyMessage(MSG_SIM_STATE_CHANGED);

                } else {
                    Log.i(TAG, "PhoneId " + phoneId + ", state is " + state + ", ignore");
                }

            } else if (action.equals(Const.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED)) {
                int subId = intent.getIntExtra(Const.SUBSCRIPTION_KEY, Const.SUB_ID_INVALID);

                Log.i(TAG, "Old/now data sub are " + mDataSub + "/" + subId);

                if (subId >= 0 && subId != mDataSub) {
                    if (mDataSub >= 0 ) {
                        mHandler.sendEmptyMessage(MSG_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
                    }else {
                        Log.i(TAG, "Old sub invalid, do nothing");
                    }
                    mDataSub = subId;
                }

            } else if (action.equals(Const.ACTION_BOOT_COMPLETED)) {
                Log.i(TAG, "onReceive " + action);
                mHandler.sendEmptyMessage(MSG_BOOT_COMPLETED);
            }
        }

        private boolean isSwitchOpen() {
            return PlatformManager.isFeatureEnabled();
        }
    }
}

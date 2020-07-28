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

package com.mediatek.ims;

import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.SystemProperties;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.telephony.PhoneStateListener;
import android.telephony.Rlog;
import android.text.TextUtils;

import com.android.ims.ImsConnectionStateListener;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsServiceClass;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.UiccController;

import com.mediatek.ims.OperatorUtils;
import com.mediatek.ims.OperatorUtils.OPID;
import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.ImsSSOemPlugin;
import com.mediatek.ims.ril.ImsCommandsInterface.RadioState;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;
import java.util.ArrayList;

import java.lang.NumberFormatException;

import com.mediatek.ims.ril.ImsCommandsInterface;

public class MtkSuppServExt extends Handler {
    private final static String LOG_TAG = "SuppServExt";

    private static final boolean DBG = true;
    private static final boolean VDBG = SystemProperties.get("ro.build.type").
            equals("eng") ? true : false; // STOPSHIP if true

    static private final boolean SDBG = SystemProperties.get("ro.build.type").equals("user")
            ? false : true;

    private static final String SYS_PROP_QUERY_VOLTE_SUB =
            "persist.vendor.suppserv.query_volte_sub";

    private Context mContext;

    private int mPhoneId = 1;

    private ImsService mImsService;

    private ImsCommandsInterface mImsRILAdapter;

    private ImsManager mImsManager = null;

    private MtkSuppServExt mSuppServExt = null;

    private SuppServTaskDriven mSuppServTaskDriven = null;

    private PhoneStateListener mPhoneStateListener;

    private boolean mSimLoaded = false;

    private boolean mQueryXcapDone = false;

    private String mOldIccId = "";

    private boolean mSimIsChangedAfterBoot = false;

    private ImsSSOemPlugin mPluginBase;

    private static final int TASK_QUERY_XCAP = 0;
    private static final int TASK_RESET_AND_QUERY_XCAP = 1;
    private static final int TASK_SET_VOLTE_SUBSCRIPTION_DIRECLY = 2;
    private static final int TASK_SET_UT_CAPABILITY = 3;

    private static final int EVENT_IMS_UT_EVENT_QUERY_XCAP = 0;
    private static final int EVENT_IMS_REGISTRATION_INFO = 1;
    private static final int EVENT_RADIO_NOT_AVAILABLE = 2;
    private static final int EVENT_RADIO_OFF = 3;
    private static final int EVENT_RADIO_ON = 4;
    private static final int EVENT_ON_VOLTE_SUBSCRIPTION = 5;

    private static final int UT_CAPABILITY_UNKNOWN = 0;
    private static final int UT_CAPABILITY_ENABLE  = 1;
    private static final int UT_CAPABILITY_DISABLE = 2;
    private static final String SETTING_UT_CAPABILITY = "ut_capability";

    private static final String ICCID_KEY = "iccid_key";

    private int mRadioState = RadioState.RADIO_UNAVAILABLE.ordinal();


    private class Task {
        private int mTaskId        = -1;
        private boolean mExtraBool = false;
        private int mExtraInt      = -1;
        private String mExtraMsg   = "";

        public Task(int taskId, boolean b, String extraMsg) {
            mTaskId   = taskId;
            mExtraBool = b;
            mExtraMsg = extraMsg;
        }
        public Task(int taskId, String extraMsg) {
            mTaskId   = taskId;
            mExtraMsg = extraMsg;
        }
        public Task(int taskId, int extraInt, String extraMsg) {
            mTaskId   = taskId;
            mExtraInt = extraInt;
            mExtraMsg = extraMsg;
        }
        public int getTaskId() {
            return mTaskId;
        }
        public int getExtraInt() {
            return mExtraInt;
        }
        public boolean getExtraBoolean() {
            return mExtraBool;
        }
        public String getExtraMsg() {
            return mExtraMsg;
        }
        public String toString() {
            return "Task ID: "     + mTaskId +
                 ", ExtraBool: " + mExtraBool +
                 ", ExtraInt: "  + mExtraInt +
                 ", ExtraMsg: " + mExtraMsg;
        }
    }

    private class SuppServTaskDriven extends Handler {
        private ArrayList <Task> mPendingTask = new ArrayList<>();

        private Object mTaskLock = new Object();
        private Object mStateLock = new Object();

        private final static int EVENT_DONE = 0;
        private final static int EVENT_EXEC_NEXT  = 1;

        private final static int STATE_NO_PENDING = 0;
        private final static int STATE_DOING      = 1;
        private final static int STATE_DONE       = 2;

        private int mState = STATE_NO_PENDING;

        public SuppServTaskDriven() {
        }

        public SuppServTaskDriven(Looper looper) {
            super(looper);
        }

        public void appendTask(Task task) {
            synchronized (mTaskLock) {
                // logd("appendTask: " + task.toString());
                mPendingTask.add(task);
            }

            Message msg = obtainMessage(EVENT_EXEC_NEXT);
            msg.sendToTarget();
        }

        private int getState() {
            synchronized (mStateLock) {
                // logd("Now state: " + stateToString(mState));
                return mState;
            }
        }

        private void setState(int state) {
            synchronized (mStateLock) {
                // logd("Set state: " + stateToString(state));
                mState = state;
            }
        }

        private Task getCurrentPendingTask() {
            synchronized (mTaskLock) {
                // logd("getCurrentPendingTask mPendingTask.size(): " + mPendingTask.size());
                if (mPendingTask.size() == 0) {
                    return null;
                }

                return mPendingTask.get(0);
            }
        }

        private void removePendingTask(int index) {
            synchronized (mTaskLock) {
                if (mPendingTask.size() > 0) {
                    mPendingTask.remove(index);
                    // logd("removePendingTask remain mPendingTask: " + mPendingTask.size());
                }
            }
        }

        public void clearPendingTask() {
            synchronized (mTaskLock) {
                mPendingTask.clear();
            }
        }

        public void exec() {
            Task task = getCurrentPendingTask();
            if (task == null) {
                setState(STATE_NO_PENDING);
                // logd("Set STATE_NO_PENDING.");
                return;
            }

            if (getState() == STATE_DOING) {
                return;
            }

            setState(STATE_DOING);
            int taskId = task.getTaskId();
            if (VDBG) {
                logd(task.toString());
            }

            switch (taskId) {
                case TASK_RESET_AND_QUERY_XCAP:{
                    mQueryXcapDone = false;    // no need to break, go to TASK_QUERY_XCAP directly.
                }
                case TASK_QUERY_XCAP: {
                    boolean force = task.getExtraBoolean();
                    String extraMsg = task.getExtraMsg();
                    startHandleXcapQueryProcess(force, extraMsg);
                    break;
                }
                case TASK_SET_VOLTE_SUBSCRIPTION_DIRECLY: {
                    // set volte subscription status
                    int currentVolteStatus = getVolteSubscriptionFromSettings();
                    int newVolteStatus = task.getExtraInt();
                    logd("TASK_SET_VOLTE_SUBSCRIPTION_DIRECLY, currentVolteStatus: " +
                            currentVolteStatus + " newVolteStatus: " + newVolteStatus);

                    if (currentVolteStatus != newVolteStatus) {
                        setVolteSubscriptionToSettings(newVolteStatus);
                    }

                    mQueryXcapDone = true;

                    taskDone();
                    break;
                }
                case TASK_SET_UT_CAPABILITY: {
                    // set ut capability status
                    int currentUtStatus = getUtCapabilityFromSettings();
                    int newUtStatus = task.getExtraInt();
                    logd("TASK_SET_UT_CAPABILITY, currentUtStatus: " + currentUtStatus +
                            " newUtStatus: " + newUtStatus);

                    if (currentUtStatus != newUtStatus) {
                        setUtCapabilityToSettings(newUtStatus);
                    }

                    mQueryXcapDone = true;

                    taskDone();
                    break;
                }
                default: {
                    taskDone();
                    break;
                }
            }
        }

        @Override
        public void handleMessage(Message msg) {
            // logd("handleMessage msg: " + eventToString(msg.what));
            AsyncResult ar;
            switch (msg.what) {
                case EVENT_DONE:
                    removePendingTask(0);  // Do next task directly.
                    setState(STATE_DONE);
                case EVENT_EXEC_NEXT:
                    exec();
                    break;
            }
        }

        private String stateToString(int state) {
            switch (state) {
                case STATE_NO_PENDING:
                    return "STATE_NO_PENDING";
                case STATE_DOING:
                    return "STATE_DOING";
                case STATE_DONE:
                    return "STATE_DONE";
            }
            return "UNKNOWN_STATE";
        }

        private String eventToString(int event) {
            switch (event) {
                case EVENT_DONE:
                    return "EVENT_DONE";
                case EVENT_EXEC_NEXT:
                    return "EVENT_EXEC_NEXT";
            }
            return "UNKNOWN_EVENT";
        }
    }

    private boolean checkNeedQueryXcap() {
        // Need to set default value to "1", then it will query volte subscription to DB.
        if ("0".equals(SystemProperties.get(SYS_PROP_QUERY_VOLTE_SUB, "0"))) {
            return false;
        }

        // Check carrier config to determine if need to do internal XCAP query for this operator
        // If the carrier config is not loaded, return false by default
        CarrierConfigManager configManager = (CarrierConfigManager) mContext.getSystemService(
                Context.CARRIER_CONFIG_SERVICE);
        int subId = getSubIdUsingPhoneId(mPhoneId);

        PersistableBundle b = null;
        if (configManager != null) {
            b = configManager.getConfigForSubId(subId);
        }

        if (b != null) {
            logd("checkNeedQueryXcap: carrier config is ready, config = " + b.getBoolean(
                    mPluginBase.getXcapQueryCarrierConfigKey(), false));
            return b.getBoolean(mPluginBase.getXcapQueryCarrierConfigKey(), false);
        } else {
            logd("checkNeedQueryXcap: carrier config not ready, return false");
            return false;
        }
    }

    private boolean isOp(OPID id) {
        return OperatorUtils.isOperator(OperatorUtils.getSimOperatorNumericForPhone(mPhoneId), id);
    }

    public MtkSuppServExt(Context context, int phoneId, ImsService imsService, Looper looper) {
        super(looper);

        mContext = context;
        // mImsManager = ImsManager.getInstance(context, mPhone.getPhoneId());

        mPhoneId = phoneId;
        mImsService = imsService;
        mImsRILAdapter = mImsService.getImsRILAdapter(phoneId);

        mSuppServTaskDriven = new SuppServTaskDriven(looper);

        mImsManager = ImsManager.getInstance(context, phoneId);

        mPluginBase = ExtensionFactory.makeOemPluginFactory(mContext).makeImsSSOemPlugin(mContext);

        checkImsInService();

        registerBroadcastReceiver();
        registerEvent();

        logd("MtkSuppServExt init done.");
    }

    private void checkImsInService() {
        if (mImsService.getImsServiceState(mPhoneId) == ServiceState.STATE_IN_SERVICE) {
            mQueryXcapDone = true;
            setVolteSubscriptionToSettings(mPluginBase.getVolteSubEnableConstant());
        }
    }

    private void initPhoneStateListener(Looper looper) {
        TelephonyManager tm = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);
        mPhoneStateListener = new PhoneStateListener(looper) {
            @Override
            public void onServiceStateChanged(ServiceState serviceState) {
                switch(serviceState.getDataRegState()) {
                    case ServiceState.STATE_IN_SERVICE:
                        Task task = new Task(TASK_QUERY_XCAP, false, "Data reg state in service.");
                        mSuppServTaskDriven.appendTask(task);
                        break;
                    default:
                        break;
                }
            }
        };

        tm.listen(mPhoneStateListener, PhoneStateListener.LISTEN_SERVICE_STATE);
    }

    private void registerBroadcastReceiver() {
        final IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        filter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        filter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        filter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
        mContext.registerReceiver(mBroadcastReceiver, filter);
    }

    private void registerEvent() {
        mImsRILAdapter.registerForImsRegistrationInfo(this, EVENT_IMS_REGISTRATION_INFO, null);
        mImsRILAdapter.registerForNotAvailable(this, EVENT_RADIO_NOT_AVAILABLE, null);
        mImsRILAdapter.registerForOff(this, EVENT_RADIO_OFF, null);
        mImsRILAdapter.registerForOn(this, EVENT_RADIO_ON, null);
        mImsRILAdapter.registerForVolteSubscription(this, EVENT_ON_VOLTE_SUBSCRIPTION, null);
    }


    private void unRegisterEvent() {
        mImsRILAdapter.unregisterForImsRegistrationInfo(this);
        mImsRILAdapter.unregisterForNotAvailable(this);
        mImsRILAdapter.unregisterForOff(this);
        mImsRILAdapter.unregisterForOn(this);
        mImsRILAdapter.unregisterForVolteSubscription(this);
    }

    private void unRegisterBroadReceiver() {
        mContext.unregisterReceiver(mBroadcastReceiver);
    }

    public void dispose() {
        unRegisterBroadReceiver();
    }

    private boolean checkInitCriteria(StringBuilder criteriaFailReason) {
        if (!checkNeedQueryXcap()) {
            criteriaFailReason.append(
                    "No need to support for this operator OR carrier config not ready, ");
            return false;
        }

        if (!isDataEnabled()) {
            criteriaFailReason.append("Data is not enabled, ");
            return false;
        }

        if (!isDataEnabled()) {
            criteriaFailReason.append("Data is not enabled, ");
            return false;
        }

        if (!isSubInfoReady()) {
            criteriaFailReason.append("SubInfo not ready, ");
            return false;
        }

        if (!getSimLoaded()) {
            criteriaFailReason.append("Sim not loaded, ");
            return false;
        }

        if (!isDataRegStateInService()) {
            criteriaFailReason.append("Data reg state is not in service, ");
            return false;
        }

        if (mQueryXcapDone) {
            criteriaFailReason.append("No need query, ");
            return false;
        }

        if (!mSimIsChangedAfterBoot) {
            criteriaFailReason.append("Sim not changed, ");
            return false;
        }

        if (mRadioState != RadioState.RADIO_ON.ordinal()) {
            criteriaFailReason.append("radio not available, ");
            return false;
        }

        criteriaFailReason.append("All Criteria ready.");
        return true;
    }

    private void startHandleXcapQueryProcess(boolean forceQuery, String reason) {
        StringBuilder criteriaFailReason = new StringBuilder();
        boolean checkCriteria = checkInitCriteria(criteriaFailReason);
        logd("startHandleXcapQueryProcess(), forceQuery: " + forceQuery +
             ", reason: " + reason +
             ", checkCriteria: " + checkCriteria +
             ", criteriaFailReason: " + criteriaFailReason.toString());
        if (!checkCriteria) {
            taskDone();
            return;
        }

        mSimIsChangedAfterBoot = false;

        startXcapQuery();
    }

    private void taskDone() {
        Message ssmsg = mSuppServTaskDriven.obtainMessage(SuppServTaskDriven.EVENT_DONE);
        ssmsg.sendToTarget();
    }

    private boolean isSubInfoReady() {
        TelephonyManager tm = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        int subId = getSubIdUsingPhoneId(mPhoneId);
        String iccid = tm.getSimSerialNumber(subId);

        if (!TextUtils.isEmpty(iccid)) {
            return true;
        }

        return false;
    }

    private boolean isDataEnabled() {
        TelephonyManager tm = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        int subId = getSubIdUsingPhoneId(mPhoneId);

        return tm.getDataEnabled(subId);
    }

    private boolean isDataRegStateInService() {
        TelephonyManager tm = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        int subId = getSubIdUsingPhoneId(mPhoneId);
        ServiceState state = tm.getServiceStateForSubscriber(subId);

        return state.getDataRegState() == ServiceState.STATE_IN_SERVICE;
    }

    // Not support legacy MD currently.
    private void startXcapQuery() {
        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            Message msg = obtainMessage(EVENT_IMS_UT_EVENT_QUERY_XCAP, null);
            mImsRILAdapter.getXcapStatus(msg);
        } else {
            taskDone();
        }
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                post(new Runnable() {
                    @Override
                    public void run() {
                        handleSubinfoUpdate();
                    }
                });
            } else if (action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE)) {
                mQueryXcapDone = false;
                // startHandleCFUQueryProcess(false, "Radio capability done");
                Task task = new Task(TASK_QUERY_XCAP, false, "Radio capability done");
                mSuppServTaskDriven.appendTask(task);
            } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                boolean bAirplaneModeOn = intent.getBooleanExtra("state", false);
                logd("ACTION_AIRPLANE_MODE_CHANGED, bAirplaneModeOn = " +
                       bAirplaneModeOn);
                if (bAirplaneModeOn) {
                    mQueryXcapDone = false;
                }
            } else if (action.equals(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)) {
                int simStatus = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                        TelephonyManager.SIM_STATE_UNKNOWN);
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);

                logd("ACTION_SIM_APPLICATION_STATE_CHANGED: " + simStatus +
                        ", subId: " + subId);

                if (subId != getSubIdUsingPhoneId(mPhoneId)) {
                    return;
                }

                if (TelephonyManager.SIM_STATE_LOADED != simStatus) {
                    setSimLoaded(false);
                    return;
                }

                setSimLoaded(true);
                Task task = new Task(TASK_QUERY_XCAP, false, "SIM loaded.");
                mSuppServTaskDriven.appendTask(task);
                if (isOp(OPID.OP09) &&
                        (SystemProperties.getInt("persist.vendor.mtk_ct_volte_support", 0) != 0)) {
                    mImsService.notifyUtCapabilityChange(mPhoneId);
                }
            } else if (action.equals(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)) {
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);

                if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                    logd("ACTION_CARRIER_CONFIG_CHANGED: not loaded, subId: " + subId);
                } else if (subId != getSubIdUsingPhoneId(mPhoneId)) {
                    logd("ACTION_CARRIER_CONFIG_CHANGED: not for this phone, subId: " + subId);
                } else {
                    logd("ACTION_CARRIER_CONFIG_CHANGED: loaded, subId: " + subId);
                    Task task = new Task(TASK_QUERY_XCAP, false, "Carrier config changed");
                    mSuppServTaskDriven.appendTask(task);
                }
            }
        }
    };

    private boolean getSimLoaded() {
        logi("mSimLoaded: " + mSimLoaded);
        return mSimLoaded;
    }

    private void setSimLoaded(boolean value) {
        logi("Set mSimLoaded: " + value);
        mSimLoaded = value;
    }

    private void handleSubinfoUpdate() {
        if (!isSubInfoReady()) {
            return;
        }

        // Only subinfo update need to handle SS init flow.
        handleSuppServInit();
    }

    private void handleSuppServInit() {
        TelephonyManager tm = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        int subId = getSubIdUsingPhoneId(mPhoneId);
        String iccid = tm.getSimSerialNumber(subId);

        if (TextUtils.isEmpty(iccid)) {
            return;
        }

        // If we found reboot occurs or SIM is changed, then trigger a XCAP query
        handleXcapQueryIfSimChangedOrBootup(iccid);
    }

    private void handleXcapQueryIfSimChangedOrBootup(String iccid) {
        logw("handleXcapQueryIfSimChangedOrBootup mySubId "
                + getSubIdUsingPhoneId(mPhoneId) + " old iccid : "
                + Rlog.pii(LOG_TAG, mOldIccId) + " new iccid : "
                + Rlog.pii(LOG_TAG, iccid));

        // Becuase mOldIccId is not persistent, if the current iccid not equal to mOldIccId
        // it means a boot-up occurs or SIM is changed
        if (iccid.equals(mOldIccId)) {
            if (VDBG) {
                logd("handleXcapQueryIfSimChangedOrBootup: Same SIM.");
            }
            return;
        }

        mOldIccId = iccid;
        mSimIsChangedAfterBoot = true;

        // Clean all the pending task first
        mSuppServTaskDriven.clearPendingTask();

        // Reset status in DB
        setVolteSubscriptionDirectly(mPluginBase.getVolteSubUnknownConstant(), "Reset VoLTE subscription status");
        setUtCapabilityDirectly(UT_CAPABILITY_UNKNOWN, "Reset Ut capabatility status");
        Task task = new Task(TASK_RESET_AND_QUERY_XCAP, false, "Sim Changed or Bootup");
        mSuppServTaskDriven.appendTask(task);
    }

    private void setVolteSubscriptionDirectly(int status, String msgStr) {
        Task task = new Task(TASK_SET_VOLTE_SUBSCRIPTION_DIRECLY, status, msgStr);
        mSuppServTaskDriven.appendTask(task);
    }

    private void setUtCapabilityDirectly(int status, String msgStr) {
        Task task = new Task(TASK_SET_UT_CAPABILITY, status, msgStr);
        mSuppServTaskDriven.appendTask(task);
    }

    @Override
    public void handleMessage(Message msg) {
        logd("handleMessage: " + toEventString(msg.what) + "(" + msg.what + ")");
        AsyncResult ar = (AsyncResult) msg.obj;
        switch (msg.what) {
            case EVENT_IMS_UT_EVENT_QUERY_XCAP: {
                mQueryXcapDone = true;
                taskDone();
                break;
            }

            case EVENT_IMS_REGISTRATION_INFO: {
                int status = ((int[]) ar.result)[0];
                if (DBG) logd("EVENT_IMS_REGISTRATION_INFO: " + status);
                if (status == 1) {  // In service
                    setVolteSubscriptionDirectly(mPluginBase.getVolteSubEnableConstant(), "Ims registered.");
                }
                break;
            }

            case EVENT_RADIO_NOT_AVAILABLE: {
                mRadioState = RadioState.RADIO_UNAVAILABLE.ordinal();
                break;
            }

            case EVENT_RADIO_OFF: {
                mRadioState = RadioState.RADIO_OFF.ordinal();
                break;
            }

            case EVENT_RADIO_ON: {
                mRadioState = RadioState.RADIO_ON.ordinal();
                Task task = new Task(TASK_QUERY_XCAP, false, "Radio on");
                mSuppServTaskDriven.appendTask(task);
                break;
            }

            case EVENT_ON_VOLTE_SUBSCRIPTION: {
                // Write the status of VoLTE Supscription into provider
                int volteSubstatus = ((int[]) ar.result)[0];
                logd(" EVENT_ON_VOLTE_SUBSCRIPTION, volteSubstatus = " + volteSubstatus);
                if (volteSubstatus == 1) {
                    setVolteSubscriptionDirectly(mPluginBase.getVolteSubEnableConstant(),
                            "Receive VoLTE Subscription URC");
                    setUtCapabilityDirectly(UT_CAPABILITY_ENABLE,
                            "Receive VoLTE Subscription URC");
                } else if (volteSubstatus == 2) {
                    setVolteSubscriptionDirectly(mPluginBase.getVolteSubDisableConstant(),
                            "Receive VoLTE Subscription URC");
                    setUtCapabilityDirectly(UT_CAPABILITY_DISABLE,
                            "Receive VoLTE Subscription URC");
                }
                break;
            }

            default: {
                logd("Unhandled msg: " + msg.what);
                break;
            }
        }
    }

    private int commandExceptionToVolteServiceStatus(CommandException commandException) {
        CommandException.Error err = null;
        int status = mPluginBase.getVolteSubUnknownConstant();

        err = commandException.getCommandError();

        logd("commandException: " + err);

        if (err == CommandException.Error.OEM_ERROR_2) {
            status = mPluginBase.getVolteSubDisableConstant();
        } else if (err == CommandException.Error.OEM_ERROR_4) {
            status = mPluginBase.getVolteSubEnableConstant();
        } else if (err == CommandException.Error.OEM_ERROR_25) {
            status = mPluginBase.getVolteSubEnableConstant();
        } else if (err == CommandException.Error.REQUEST_NOT_SUPPORTED) {
            status = mPluginBase.getVolteSubDisableConstant();
        }

        return status;
    }

    public boolean isSupportCFT() {
        int status = getVolteSubscriptionFromSettings();
        boolean bStatus = false;

        logd("isSupportCFT: getVolteSubscriptionFromSettings = " + status);

        if (isOp(OPID.OP01)) {
            bStatus = true;
        }
        // if (!isOp(OPID.OP01)) {
        //     return false;
        // }

        // if (status == mPluginBase.getVolteSubEnableConstant()) {
        //     bStatus = true;
        // }

        return bStatus;
    }

    private int getVolteSubscriptionFromSettings() {
        int status = android.provider.Settings.Global.getInt(
                mContext.getContentResolver(),
                mPluginBase.getVolteSubscriptionKey() + mPhoneId,
                mPluginBase.getVolteSubUnknownConstant());
        return status;
    }

    private void setVolteSubscriptionToSettings(int status) {
        logd("setVolteSubscriptionToSettings: " + status);
        android.provider.Settings.Global.putInt(
                mContext.getContentResolver(),
                mPluginBase.getVolteSubscriptionKey() + mPhoneId,
                status);
    }

    public int getUtCapabilityFromSettings() {
        int status = android.provider.Settings.Global.getInt(
                mContext.getContentResolver(),
                SETTING_UT_CAPABILITY + mPhoneId,
                UT_CAPABILITY_UNKNOWN);
        return status;
    }

    private void setUtCapabilityToSettings(int status) {
        logd("setUtCapabilityToSettings: " + status);
        android.provider.Settings.Global.putInt(
                mContext.getContentResolver(),
                SETTING_UT_CAPABILITY + mPhoneId,
                status);
        mImsService.notifyUtCapabilityChange(mPhoneId);
    }

    private static int getSubIdUsingPhoneId(int phoneId) {
        int [] values = SubscriptionManager.getSubId(phoneId);
        if(values == null || values.length <= 0) {
            return SubscriptionManager.getDefaultSubscriptionId();
        }
        else {
            return values[0];
        }
    }

    private String getIccIdFromSp() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mContext);
        String iccid = sp.getString(ICCID_KEY + mPhoneId, "");
        logd("getIccIdFromSp: " + iccid);
        return iccid;
    }

    private String toEventString(int event) {
        switch (event) {
            case EVENT_IMS_UT_EVENT_QUERY_XCAP:
                return "EVENT_IMS_UT_EVENT_QUERY_XCAP";
            case EVENT_IMS_REGISTRATION_INFO:
                return "EVENT_IMS_REGISTRATION_INFO";
            case EVENT_RADIO_NOT_AVAILABLE:
                return "EVENT_RADIO_NOT_AVAILABLE";
            case EVENT_RADIO_ON:
                return "EVENT_RADIO_ON";
            case EVENT_RADIO_OFF:
                return "EVENT_RADIO_OFF";
            case EVENT_ON_VOLTE_SUBSCRIPTION:
                return "EVENT_ON_VOLTE_SUBSCRIPTION";
            default:
        }
        return "UNKNOWN_IMS_EVENT_ID";
    }

    private void loge(String s) {
        Rlog.e(LOG_TAG, "[" + mPhoneId + "]" + s);
    }

    private void logw(String s) {
        Rlog.w(LOG_TAG, "[" + mPhoneId + "]" + s);
    }

    private void logi(String s) {
        // default user/userdebug debug level set as INFO
        Rlog.i(LOG_TAG, "[" + mPhoneId + "]" + s);
    }

    private void logd(String s) {
        // default eng debug level set as DEBUG
        Rlog.d(LOG_TAG, "[" + mPhoneId + "]" + s);
    }

    private void logv(String s) {
        Rlog.v(LOG_TAG, "[" + mPhoneId + "]" + s);
    }
}

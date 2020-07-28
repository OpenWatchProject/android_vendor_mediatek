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
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.SystemProperties;
import android.telephony.AccessNetworkConstants;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.Rlog;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import com.android.ims.ImsConnectionStateListener;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsUtInterface;
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


// import com.mediatek.ims.internal.MtkImsManager;

import com.mediatek.internal.telephony.imsphone.MtkImsPhone;

import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkOperatorUtils;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkServiceStateTracker;
import com.mediatek.internal.telephony.MtkSSRequestDecisionMaker;
import com.mediatek.internal.telephony.MtkSuppServUtTest;
import com.mediatek.internal.telephony.uicc.MtkSIMRecords;

import java.util.Base64;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;
import java.util.ArrayList;

import static com.android.internal.telephony.CommandsInterface.CF_REASON_UNCONDITIONAL;
import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_VOICE;

import static com.mediatek.internal.telephony.MtkTelephonyProperties.CFU_QUERY_SIM_CHANGED_PROP;
import static
        com.mediatek.internal.telephony.MtkTelephonyProperties.PROPERTY_ERROR_MESSAGE_FROM_XCAP;

import static
com.mediatek.internal.telephony.MtkTelephonyProperties.PROPERTY_TBCW_MODE;
import static
com.mediatek.internal.telephony.MtkTelephonyProperties.TBCW_DISABLED;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.CFU_QUERY_TYPE_DEF_VALUE;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.CFU_QUERY_ICCID_PROP;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.CFU_QUERY_TYPE_PROP;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.CFU_QUERY_OVER_IMS;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.CFU_STATUS_SYNC_FOR_OTA;

import static com.mediatek.internal.telephony.MtkGsmCdmaPhone.TBCW_UNKNOWN;
import static com.mediatek.internal.telephony.MtkGsmCdmaPhone.TBCW_VOLTE_USER;
import static com.mediatek.internal.telephony.MtkGsmCdmaPhone.TBCW_NOT_VOLTE_USER;
import static com.mediatek.internal.telephony.MtkGsmCdmaPhone.TBCW_WITH_CS;


// MtkSuppServHelper only support to query the CFU value.
// MtkSIMRecords would handle the CFU initial status.
// MtkSuppServHelper use the same thread with GsmCdmaPhone.
// SuppServTaskDriven use another thread.

public class MtkSuppServHelper extends Handler {
    private final static String LOG_TAG = "SuppServHelper";

    private static final boolean DBG = true;
    private static final boolean VDBG = SystemProperties.get("ro.build.type").
            equals("eng") ? true : false; // STOPSHIP if true

    static private final boolean SDBG = SystemProperties.get("ro.build.type").equals("user")
            ? false : true;

    private Context mContext;

    private MtkGsmCdmaPhone mPhone = null;

    private UiccController mUiccController = null;

    private final AtomicReference<IccRecords> mIccRecords = new AtomicReference<IccRecords>();

    private AtomicBoolean mAttached = new AtomicBoolean(false);

    private MtkSuppServHelper mMtkSuppServHelper = null;

    private SuppServTaskDriven mSuppServTaskDriven = null;

    private boolean mNeedGetCFU        = true;
    private boolean mNeedGetCFUOverIms = false;

    private boolean mSimRecordsLoaded = false;
    private boolean mCarrierConfigLoaded = false;

    private ImsManager mImsManager = null;
    private final ImsManager.Connector mImsManagerConnector;

    private int mCFUStatusFromMD = -1;  // -1: not set
                                        //  0: disable
                                        //  1: enable

    private static final int TIMER_FOR_SKIP_WAITING_CFU_STATUS_FROM_MD = 20000;
    private boolean mSkipCFUStatusFromMD = false;  // TODO: to avoid MD not send URC. (No need now)

    private static final int EVENT_REGISTERED_TO_NETWORK                     = 0;
    private static final int EVENT_ICCRECORDS_READY                          = 1;
    private static final int EVENT_DATA_CONNECTION_ATTACHED                  = 2;
    private static final int EVENT_DATA_CONNECTION_DETACHED                  = 3;
    private static final int EVENT_GET_CALL_FORWARD_BY_GSM_DONE              = 4;
    private static final int EVENT_GET_CALL_FORWARD_BY_IMS_DONE              = 5;
    private static final int EVENT_CALL_FORWARDING_STATUS_FROM_MD            = 6;
    private static final int EVENT_QUERY_CFU_OVER_CS                         = 7;
    private static final int EVENT_CFU_STATUS_FROM_MD                        = 8;
    private static final int EVENT_SS_RESET                                  = 9;
    private static final int EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_GSM_DONE    = 10;
    private static final int EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_IMS_DONE    = 11;
    private static final int EVENT_SIM_RECORDS_LOADED                        = 12;
    private static final int EVENT_ICC_CHANGED                               = 13;
    private static final int EVENT_QUERY_CFU_OVER_CS_AFTER_DATA_NOT_ATTACHED = 14;
    private static final int EVENT_CARRIER_CONFIG_LOADED                     = 15;
    public  static final int EVENT_CLEAN_CFU_STATUS                          = 16;

    private static final int TIMER_FOR_WAIT_DATA_ATTACHED = 20000;
    private static final int TIMER_FOR_RETRY_QUERY_CFU    = 20000; // TODO

    private static final int TASK_QUERY_CFU          = 0;
    private static final int TASK_QUERY_CFU_OVER_GSM = 1;   // 91, 92: CS only
                                                            // 93: CS or xcap 2G/3G
    private static final int TASK_QUERY_CFU_OVER_IMS = 2;
    private static final int TASK_TIME_SLOT_FAILED   = 3;
    private static final int TASK_CLEAN_CFU_STATUS   = 4;
    private static final int TASK_SET_CW_STATUS   = 5;

    private static final int QUERY_OVER_GSM         = 0;
    private static final int QUERY_OVER_GSM_OVER_UT = 1;
    private static final int QUERY_OVER_IMS         = 2;

    // Default is disable.
    private static final String CFU_SETTING_DEFAULT                 = CFU_QUERY_TYPE_DEF_VALUE;
    private static final String CFU_SETTING_ALWAYS_NOT_QUERY        = "1";
    private static final String CFU_SETTING_ALWAYS_QUERY            = "2";
    private static final String CFU_SETTING_QUERY_IF_EFCFIS_INVALID = "3";

    private static final int EFCFIS_STATUS_NOT_READY     = 0;
    private static final int EFCFIS_STATUS_VALID         = 2;
    private static final int EFCFIS_STATUS_INVALID       = 3;

    /* Query CFU over IMS if IMS registered is a new feature in O, however,
     * it hits some SQC or machine test cases. So, we close this feature by default.
     */
    private static final boolean CFU_QUERY_WHEN_IMS_REGISTERED_DEFAULT = false;

    private static final String SIM_NO_CHANGED = "0";
    private static final String SIM_CHANGED    = "1";

    private static final String IMS_NO_NEED_QUERY = "0";
    private static final String IMS_NOT_QUERY_YET = "1";
    private static final String IMS_QUERY_DONE    = "2";

    private static final String ACTION_SYSTEM_UPDATE_SUCCESSFUL =
            "com.mediatek.systemupdate.UPDATE_SUCCESSFUL";

    private int mNeeedSyncForOTA = -1;   // -1: Default
                                             //  0: Not change
                                             //  1: Change


    private boolean mAlwaysQueryDone = false;

    private static final boolean WITH_TIME_SLOT = true;
    private static final boolean WITHOUT_TIME_SLOT = false;

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
                    logd("removePendingTask remain mPendingTask: " + mPendingTask.size());
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
            logd(task.toString());

            switch (taskId) {
                case TASK_QUERY_CFU: {
                    boolean force = task.getExtraBoolean();
                    String extraMsg = task.getExtraMsg();
                    startHandleCFUQueryProcess(force, extraMsg);
                    break;
                }
                case TASK_QUERY_CFU_OVER_GSM: {
                    queryCallForwardStatusOverGSM();
                    break;
                }
                case TASK_QUERY_CFU_OVER_IMS: {
                    queryCallForwardStatusOverIMS();
                    break;
                }
                case TASK_TIME_SLOT_FAILED: {
                    startCFUQuery(true);
                    break;
                }
                case TASK_CLEAN_CFU_STATUS: {
                    cleanCFUStatus();
                    break;
                }
                case TASK_SET_CW_STATUS: {
                    if (!isMDSupportIMSSuppServ()) {
                        mPhone.setTbcwMode(TBCW_UNKNOWN);  //reset to unknow due to sim change.
                        mPhone.setSSPropertyThroughHidl(mPhone.getPhoneId(),
                                                      PROPERTY_TBCW_MODE, TBCW_DISABLED);
                    }
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

    public MtkSuppServHelper(Context context, Phone phone) {
        mContext = context;
        mPhone   = (MtkGsmCdmaPhone) phone;
        mImsManagerConnector = new ImsManager.Connector(mPhone.getContext(), mPhone.getPhoneId(),
                new ImsManager.Connector.Listener() {
                    @Override
                    public void connectionReady(ImsManager manager) throws ImsException {
                        mImsManager = manager;
                        setImsCallback();
                    }

                    @Override
                    public void connectionUnavailable() {
                        unSetImsCallback();
                    }
                });
        mImsManagerConnector.connect();
        registerEvent();
        registerBroadcastReceiver();

        logd("MtkSuppServHelper init done.");
    }

    public void init(Looper looper){
        mSuppServTaskDriven = new SuppServTaskDriven(looper);
    }

    private boolean checkInitCriteria(StringBuilder criteriaFailReason){
        String cfuSetting = getCfuSetting();

        if (!getNeedGetCFU()) {
            criteriaFailReason.append("No need to get CFU. (flag is false), ");
            return false;
        }

        if (!isSubInfoReady()) {
            criteriaFailReason.append("SubInfo not ready, ");
            return false;
        }

        if (!isIccCardMncMccAvailable(mPhone.getPhoneId())) {
            criteriaFailReason.append("MCC MNC not ready, ");
            return false;
        }

        if (!isIccRecordsAvailable()) {
            criteriaFailReason.append("Icc record available, ");
            return false;
        }

        if (!isVoiceInService()) {
            criteriaFailReason.append("Network is not registered, ");
            return false;
        }

        if (!getSimRecordsLoaded()) {
            criteriaFailReason.append("Sim not loaded, ");
            return false;
        }

        // Must put this criteria after sim records loaded.
        if (cfuSetting.equals(CFU_SETTING_QUERY_IF_EFCFIS_INVALID)) {
            int efcfisStatus = checkEfCfis();
            logd("efcfisStatus: " + efcfisStatus);
            if (efcfisStatus == EFCFIS_STATUS_VALID ||
                    efcfisStatus == EFCFIS_STATUS_NOT_READY) {
                criteriaFailReason.append(
                        "EfCfis in SIM is valid, no need to check or SIMRecords not ready.");
                return false;
            }
        }

        criteriaFailReason.append("All Criteria ready.");
        return true;
    }

    private void onUpdateIcc() {
        if (mUiccController == null) {
            return;
        }

        IccRecords newIccRecords = getUiccRecords(UiccController.APP_FAM_3GPP);

        if (newIccRecords == null && mPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
            // M:  CDMALTEPhone gets 3GPP above, pure CDMA card gets 3GPP2 here.
            newIccRecords = getUiccRecords(UiccController.APP_FAM_3GPP2);
        }

        IccRecords r = mIccRecords.get();

        if (newIccRecords == null) {
            logd("onUpdateIcc: newIccRecords is null");
        } else {
            logd("onUpdateIcc: newIccRecords is not null");
        }

        if (r != newIccRecords) {
            setSimRecordsLoaded(false);

            if (r != null) {
                logi("Removing stale icc objects.");
                r.unregisterForRecordsLoaded(this);
                mIccRecords.set(null);
            } else {
                logd("onUpdateIcc: mIccRecords is not null");
            }
            if (newIccRecords != null) {
                if (SubscriptionManager.isValidSubscriptionId(mPhone.getSubId())) {
                    logi("New records found.");
                    mIccRecords.set(newIccRecords);
                    newIccRecords.registerForRecordsLoaded(
                            this, EVENT_SIM_RECORDS_LOADED, null);
                }
            } else {
                logd("onUpdateIcc: Sim not ready.");
            }
        }
    }

    private IccRecords getUiccRecords(int appFamily) {
        return mUiccController.getIccRecords(mPhone.getPhoneId(), appFamily);
    }

    private boolean getSimRecordsLoaded() {
        logi("mSimRecordsLoaded: " + mSimRecordsLoaded);
        return mSimRecordsLoaded;
    }

    private void setSimRecordsLoaded(boolean value) {
        logi("Set mSimRecordsLoaded: " + value);
        mSimRecordsLoaded = value;
    }

    private boolean getCarrierConfigLoaded() {
        logi("mCarrierConfigLoaded: " + mCarrierConfigLoaded);
        return mCarrierConfigLoaded;
    }

    private void setCarrierConfigLoaded(boolean value) {
        logi("Set mCarrierConfigLoaded: " + value);
        mCarrierConfigLoaded = value;
    }

    private void handleSubinfoUpdate() {
        if (!isSubInfoReady()) {
            return;
        }

        // Only subinfo update need to handle SS init flow.
        handleSuppServInit();

        if (!isIccRecordsAvailable()) {
            return;
        }
    }

    private String getCfuSetting() {
        String cfuSetting;

        // Default disable this feature. (2017.9.8)
        String defaultQueryCfuMode = getCFUQueryDefault();
        if (!TelephonyManager.from(mContext).isVoiceCapable()) {
            // disable CFU query for non voice capable devices (i.e. tablet devices)
            cfuSetting = SystemProperties.get(CFU_QUERY_TYPE_PROP, CFU_SETTING_ALWAYS_NOT_QUERY);
        } else {
            cfuSetting = SystemProperties.get(CFU_QUERY_TYPE_PROP,
                    defaultQueryCfuMode);
        }

        return cfuSetting;
    }

    private String getCFUQueryDefault() {
        // Default is disable this feature. (2017.9.8)
        // It can be customize here.
        return CFU_SETTING_DEFAULT;
    }

    private boolean isNotMachineTest() {
        String isTestSim = "0";
        /// M: Add for CMCC RRM test. @{
        boolean isRRMEnv = false;
        String operatorNumeric = null;
        /// @}
        if (mPhone.getPhoneId() == PhoneConstants.SIM_ID_1) {
            isTestSim = SystemProperties.get("vendor.gsm.sim.ril.testsim", "0");
        }
        else if (mPhone.getPhoneId() == PhoneConstants.SIM_ID_2) {
            isTestSim = SystemProperties.get("vendor.gsm.sim.ril.testsim.2", "0");
        }

        /// M: Add for CMCC RRM test. @{
        // RRM test use 46602 as PLMN, which will not appear in the actual network
        // Note that this should be modified when the PLMN for RRM test is changed
        operatorNumeric = mPhone.getServiceState().getOperatorNumeric();
        if (operatorNumeric != null && operatorNumeric.equals("46602")) {
            isRRMEnv = true;
        }
        /// @}

        logd("isTestSIM : " + isTestSim + " isRRMEnv : " + isRRMEnv);

        return isTestSim.equals("0") && (isRRMEnv == false);
    }

    private void startHandleCFUQueryProcess(boolean forceQuery, String reason) {
        StringBuilder criteriaFailReason = new StringBuilder();
        boolean checkCriteria = checkInitCriteria(criteriaFailReason);
        String cfuSetting = getCfuSetting();  // Handle query CFU flow after init.
        logd("startHandleCFUQueryProcess(), forceQuery: " + forceQuery +
             ", CFU_KEY = " + cfuSetting +
             ", reason: " + reason +
             ", checkCriteria: " + checkCriteria +
             ", criteriaFailReason: " + criteriaFailReason.toString());
        if (!checkCriteria) {
            taskDone();
            return;
        }

        if (isNotMachineTest()) { /// M: Add for CMCC RRM test.
            // 0 : default
            // 1 : OFF
            // 2 : ON
            if ((cfuSetting.equals(CFU_SETTING_QUERY_IF_EFCFIS_INVALID)
                    && getSIMChangedRecordFromSystemProp())) {
                /* For solving ALPS01023811 */
                String isChangedProp = CFU_QUERY_SIM_CHANGED_PROP + mPhone.getPhoneId();
                SystemProperties.set(isChangedProp, SIM_NO_CHANGED);

                startCFUQuery();
            } else if (cfuSetting.equals(CFU_SETTING_ALWAYS_QUERY)) {
                if (!mAlwaysQueryDone) {
                    logd("Always query done: " + mAlwaysQueryDone);
                    startCFUQuery();
                    mAlwaysQueryDone = true;
                } else {
                    taskDone();
                }
            } else {
                taskDone();
            }
        } else {
            taskDone();
        }
        setNeedGetCFU(false);
    }

    public void setAlwaysQueryDoneFlag(boolean flag) {
        logd("setAlwaysQueryDoneFlag: flag = " + flag);
        mAlwaysQueryDone = flag;
    }

    private void setNeedGetCFU(boolean bNeed) {
        logd("setNeedGetCFU: " + bNeed);
        mNeedGetCFU = bNeed;
    }

    private boolean getNeedGetCFU() {
        return mNeedGetCFU;
    }

    private void taskDone() {
        Message ssmsg = mSuppServTaskDriven.obtainMessage(SuppServTaskDriven.EVENT_DONE);
        ssmsg.sendToTarget();
    }

    private boolean isIccCardMncMccAvailable(int phoneId) {
        UiccController uiccCtl = UiccController.getInstance();
        IccRecords iccRecords = uiccCtl.getIccRecords(phoneId, UiccController.APP_FAM_3GPP);
        if (iccRecords != null) {
            String mccMnc = iccRecords.getOperatorNumeric();
            return (mccMnc != null);
        }
        return false;
    }

    // No need now.
    private boolean isReceiveCFUStatusFromMD() {
        if (mSkipCFUStatusFromMD) {
            return true;
        }

        if (mCFUStatusFromMD != -1) {
            return true;
        }

        // return false;   //TBD
        return true;
    }

    private boolean isIccRecordsAvailable() {
        IccRecords r = mPhone.getIccRecords();
        if (r != null) {
            return true;
        }
        return false;
    }

    private boolean isVoiceInService() {
        if (mPhone.mSST != null && mPhone.mSST.mSS != null
                    && (mPhone.mSST.mSS.getState() == ServiceState.STATE_IN_SERVICE)) {
            return true;
        }
        return false;
    }

    private boolean isSubInfoReady() {
        SubscriptionManager subMgr = SubscriptionManager.from(mContext);
        SubscriptionInfo mySubInfo = null;
        if (subMgr != null) {
            mySubInfo = subMgr.getActiveSubscriptionInfo(mPhone.getSubId());
        }
        if ((mySubInfo != null) && (mySubInfo.getIccId() != null)) {
            return true;
        }
        return false;
    }

    private void handleSuppServInit() {
        String mySettingName = CFU_QUERY_ICCID_PROP + mPhone.getPhoneId();
        String oldIccId = SystemProperties.get(mySettingName, "");

        String cfuSetting = getCfuSetting();

        SubscriptionManager subMgr = SubscriptionManager.from(mContext);
        SubscriptionInfo mySubInfo = null;
        if (subMgr != null) {
            mySubInfo = subMgr.getActiveSubscriptionInfo(mPhone.getSubId());
        }

        /* If the SIM card has been changed, need to Initialize
         * some SS varibles and query CFU status
         */
        if (mySubInfo == null) {
            mNeeedSyncForOTA = -1;
        }

        String newIccId = encryptString(mySubInfo.getIccId());

        if (mySubInfo != null && !newIccId.equals(oldIccId)) {
            logw("mySubId " + mPhone.getSubId() + " mySettingName "
                    + Rlog.pii(SDBG, mySettingName) + " old iccid : "
                    + oldIccId + " new iccid : " + newIccId);

            SystemProperties.set(mySettingName, newIccId);
            String isChanged = CFU_QUERY_SIM_CHANGED_PROP + mPhone.getPhoneId();
            SystemProperties.set(isChanged, SIM_CHANGED);

            if (isNeedSyncSysPropToSIMforOTA()) {
                setNeedSyncSysPropToSIMforOTA(false);
                mNeeedSyncForOTA = 0;
            }

            handleSuppServIfSimChanged();
        } else if (mySubInfo != null && newIccId.equals(oldIccId)) {
            mNeeedSyncForOTA = 1;
            if (isNeedSyncSysPropToSIMforOTA()) {
                logd("ICC are the sames and trigger CFU status sync for OTA.");
                if (syncSysPropToSIMforOTA()) {
                    setNeedSyncSysPropToSIMforOTA(false);
                    mNeeedSyncForOTA = 0;
                }
            }
        } else if (cfuSetting.equals(CFU_SETTING_ALWAYS_QUERY)) {
            Task task = new Task(TASK_QUERY_CFU, false, "Always query CFU");
            mSuppServTaskDriven.appendTask(task);
        }
    }

    private void handleSuppServIfSimChanged() {
        if (getSIMChangedRecordFromSystemProp()) {
            reset();

            mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);

            mSuppServTaskDriven.appendTask(new Task(TASK_SET_CW_STATUS, false, "Sim Changed"));

            mAlwaysQueryDone = false;
            /// M: SS OP01 Ut

            mPhone.saveTimeSlot(null);

            // Remove the CLIR setting for new SIM
            SharedPreferences sp =
                    PreferenceManager.getDefaultSharedPreferences(mContext);
            int clirSetting = sp.getInt(MtkGsmCdmaPhone.CLIR_KEY + mPhone.getPhoneId(), -1);
            if (clirSetting != -1) {
                SharedPreferences.Editor editor = sp.edit();
                editor.remove(MtkGsmCdmaPhone.CLIR_KEY + mPhone.getPhoneId());
                if (!editor.commit()) {
                    loge("failed to commit the removal of CLIR preference");
                }
            }

            if (needQueryCFUOverIms()) {
                TelephonyManager.setTelephonyProperty(mPhone.getPhoneId(),
                        CFU_QUERY_OVER_IMS, IMS_NOT_QUERY_YET);
            } else {
                TelephonyManager.setTelephonyProperty(mPhone.getPhoneId(),
                        CFU_QUERY_OVER_IMS, IMS_NO_NEED_QUERY);
            }

            Task task = new Task(TASK_QUERY_CFU, false, "Sim Changed");
            mSuppServTaskDriven.appendTask(task);
        }
    }

    private boolean getIMSQueryStatus() {
        String status = IMS_NO_NEED_QUERY;

        if (needQueryCFUOverIms()) {
            status = mPhone.getSystemProperty(CFU_QUERY_OVER_IMS, IMS_NO_NEED_QUERY);
        }

        if (IMS_QUERY_DONE.equals(status)
            || IMS_NO_NEED_QUERY.equals(status)) {
            return false;
        } else if (IMS_NOT_QUERY_YET.equals(status)) {
            return true;
        }

        return false;
    }

    private boolean getSIMChangedRecordFromSystemProp() {
        String isChangedProp = CFU_QUERY_SIM_CHANGED_PROP + mPhone.getPhoneId();
        String isChanged = SystemProperties.get(isChangedProp, SIM_NO_CHANGED);

        logd("getSIMChangedRecordFromSystemProp: " + isChanged);

        if (isChanged.equals(SIM_CHANGED)) {
            return true;
        }
        return false;
    }

    private int getCallForwardingFromSimRecords() {
        IccRecords r = mPhone.getIccRecords();
        if (r != null) {
            return r.getVoiceCallForwardingFlag();
        }
        return -1;
    }

    private void startCFUQuery() {
        startCFUQuery(false);
    }

    private void startCFUQuery(boolean bForceNoTimeSlot) {
        if (isIMSRegistered()) {
            Phone imsPhone = mPhone.getImsPhone();
            if(isSupportCFUTimeSlot() && !bForceNoTimeSlot) {
                getCallForwardingOption(QUERY_OVER_IMS, WITH_TIME_SLOT);
            } else {
                getCallForwardingOption(QUERY_OVER_IMS, WITHOUT_TIME_SLOT);
            }
            return;
        }

        // TODO: Need to handle bDataEnable for op01 and op02 because
        // we don't have to handle the CS domain due to VOLTE card.
        boolean bDataEnable = mPhone.getDataEnabledSettings().isDataEnabled();
        if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                && mPhone.isGsmUtSupport() && bDataEnable) {

            logd("startCFUQuery, get data attached state : " + mAttached.get());
            boolean attachedState = mAttached.get();
            if (!attachedState) {
                Message msg = obtainMessage(EVENT_QUERY_CFU_OVER_CS_AFTER_DATA_NOT_ATTACHED);
                sendMessageDelayed(msg, TIMER_FOR_WAIT_DATA_ATTACHED);
                taskDone();
                return;
            }
            if(isSupportCFUTimeSlot() && !bForceNoTimeSlot) {
                getCallForwardingOption(QUERY_OVER_GSM_OVER_UT, WITH_TIME_SLOT);
            } else {
                getCallForwardingOption(QUERY_OVER_GSM_OVER_UT, WITHOUT_TIME_SLOT);
            }
        } else {
            // TODO: Do we need to recover it?
            if (mPhone.isDuringVoLteCall()|| mPhone.isDuringImsEccCall()) {
                logi("No need query CFU in CS domain due to during volte call and ims ecc call!");
                taskDone();
                return;
            } else {
                if (isIMSRegistered() && isNoNeedToCSFBWhenIMSRegistered()) {
                    taskDone();
                    return;
                }

                if (isNotSupportUtToCS()) {
                    taskDone();
                    return;
                }
                getCallForwardingOption(QUERY_OVER_GSM, WITHOUT_TIME_SLOT);
            }
        }
    }

    private boolean isIMSRegistered() {
        Phone imsPhone = mPhone.getImsPhone();
        if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                && (imsPhone != null)
                && (imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)) {
            return true;
        }
        return false;
    }

      public boolean getIMSRegistered() {
        return isIMSRegistered();
    }

    private void setImsCallback() throws ImsException {
        try {
            mImsManager.addRegistrationCallback(mImsRegistrationCallback);
        } catch (ImsException ie) {
            logd("ImsManager addRegistrationCallback failed, " + ie.toString());
        }
    }

    private void unSetImsCallback() {
        if (mImsManager != null) {
            mImsManager.removeRegistrationListener(mImsRegistrationCallback);
        }
    }

    private void registerEvent() {
        mPhone.getServiceStateTracker().registerForDataConnectionAttached(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN, this,
                EVENT_DATA_CONNECTION_ATTACHED, null);
        mPhone.getServiceStateTracker().registerForDataConnectionDetached(
                AccessNetworkConstants.TRANSPORT_TYPE_WWAN, this,
                EVENT_DATA_CONNECTION_DETACHED, null);
        mPhone.getServiceStateTracker().registerForNetworkAttached(this,
                EVENT_REGISTERED_TO_NETWORK, null);
        mPhone.registerForSimRecordsLoaded(this, EVENT_SIM_RECORDS_LOADED, null);
        mUiccController = UiccController.getInstance();
        mUiccController.registerForIccChanged(this, EVENT_ICC_CHANGED, null);
        ((MtkRIL) mPhone.mCi).registerForCallForwardingInfo(this,
                EVENT_CFU_STATUS_FROM_MD, null);
    }

    private void unRegisterEvent() {
        mPhone.getServiceStateTracker()
                .unregisterForDataConnectionAttached(
                    AccessNetworkConstants.TRANSPORT_TYPE_WWAN, this);
        mPhone.getServiceStateTracker()
                .unregisterForDataConnectionDetached(
                    AccessNetworkConstants.TRANSPORT_TYPE_WWAN, this);
        mPhone.getServiceStateTracker().unregisterForNetworkAttached(this);
        mPhone.unregisterForSimRecordsLoaded(this);
        mUiccController = UiccController.getInstance();
        mUiccController.unregisterForIccChanged(this);
        ((MtkRIL) mPhone.mCi).unregisterForCallForwardingInfo(this);
    }

    private void registerBroadcastReceiver() {
        final IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        // filter.addAction(MtkImsManager.ACTION_IMS_STATE_CHANGED);
        filter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        filter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        //filter.addAction(MtkSuppServUtTest.ACTION_SUPPLEMENTARY_SERVICE_UT_TEST);

        filter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
        filter.addAction(ACTION_SYSTEM_UPDATE_SUCCESSFUL);
        mContext.registerReceiver(mBroadcastReceiver, filter);
        final IntentFilter utTestFilter = new IntentFilter();
        utTestFilter.addAction(MtkSuppServUtTest.ACTION_SUPPLEMENTARY_SERVICE_UT_TEST);
        mContext.registerReceiver(mBroadcastReceiver, utTestFilter,
                MtkSuppServUtTest.SUPPLEMENTARY_SERVICE_PERMISSION, null);
    }

    private void unRegisterBroadReceiver() {
        mContext.unregisterReceiver(mBroadcastReceiver);
    }

    public void dispose() {
        unRegisterEvent();
        unRegisterBroadReceiver();
        mImsManagerConnector.disconnect();
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                handleSubinfoUpdate();
            } else if (action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE)) {
                setNeedGetCFU(true);
                // startHandleCFUQueryProcess(false, "Radio capability done");
                Task task = new Task(TASK_QUERY_CFU, false, "Radio capability done");
                mSuppServTaskDriven.appendTask(task);
            } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                boolean bAirplaneModeOn = intent.getBooleanExtra("state", false);
                logd("ACTION_AIRPLANE_MODE_CHANGED, bAirplaneModeOn = " +
                       bAirplaneModeOn);
                if (bAirplaneModeOn) {
                    setNeedGetCFU(true);

                    /// M: add for CU VOLTE SS:AIRPLANE_MODE restore to PS domain @{
                    if (isResetCSFBStatusAfterFlightMode()) {
                        mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
                    }
                    /// @}
                }
            } else if (action.equals(
                    MtkSuppServUtTest.ACTION_SUPPLEMENTARY_SERVICE_UT_TEST)) {
                logd("ACTION_SUPPLEMENTARY_SERVICE_UT_TEST");
                if (!isSupportSuppServUTTest()) {
                    return;
                }
                MtkSuppServUtTest ssUtTest = makeMtkSuppServUtTest(intent);
                ssUtTest.run();
            } else if (action.equals(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)) {
                int simStatus = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                        TelephonyManager.SIM_STATE_UNKNOWN);
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                logd("ACTION_SIM_APPLICATION_STATE_CHANGED: " + simStatus +
                        ", subId: " + subId +
                        ", CallForwardingFromSimRecords: " + getCallForwardingFromSimRecords());
                if (TelephonyManager.SIM_STATE_LOADED != simStatus) {
                    return;
                }
                if (subId == mPhone.getSubId()) {
                    if (getCallForwardingFromSimRecords() ==
                                IccRecords.CALL_FORWARDING_STATUS_ENABLED) {
                        logd("ACTION_SIM_APPLICATION_STATE_CHANGED, refresh CFU info.");
                        mPhone.notifyCallForwardingIndicator();
                    }
                }
            } else if (action.equals(ACTION_SYSTEM_UPDATE_SUCCESSFUL)) {
                logd("ACTION_SYSTEM_UPDATE_SUCCESSFUL, sync CFU info.");
                setNeedSyncSysPropToSIMforOTA(true);
                if (syncSysPropToSIMforOTA()) {
                    setNeedSyncSysPropToSIMforOTA(false);
                    mNeeedSyncForOTA = 0;
                }
            }
        }
    };

    @Override
    public void handleMessage(Message msg) {
        logd("handleMessage: " + toEventString(msg.what) + "(" + msg.what + ")");
        AsyncResult ar = (AsyncResult) msg.obj;
        switch (msg.what) {
            case EVENT_GET_CALL_FORWARD_BY_GSM_DONE: {
                // SSRequestDecisionMaker would handle CSFB by itself
                if (ar.exception != null) {

                } else {
                    Message ret = mPhone.getCFCallbackMessage();
                    AsyncResult.forMessage(ret, ar.result, ar.exception);
                    ret.sendToTarget();
                }
                taskDone();
                break;
            }
            case EVENT_GET_CALL_FORWARD_BY_IMS_DONE: {
                // Imsphone would not handle CSFB by itself, so need to handle CSFB here.
                // This is not so good to copy the same logic from MtkGsmCdmaPhone,
                // might need to enhance in the future.

                // ==== 91:92 Over imsphone ERROR code ====
                // [OPERATION_NOT_ALLOWED]:
                //     CSFB when !isNotSupportUtToCS() && !isNoNeedToCSFBWhenIMSRegistered()
                // [NO_NETWORK_FOUND]:
                //     CSFB when !isNotSupportUtToCS() && !isNoNeedToCSFBWhenIMSRegistered()

                // ==== 93 Over imsphone ====
                // No need to handle CSFB

                if (isMDSupportIMSSuppServ()) {
                    Message ret = mPhone.getCFCallbackMessage();
                    AsyncResult.forMessage(ret, ar.result, ar.exception);
                    ret.sendToTarget();

                    mPhone.setSystemProperty(CFU_QUERY_OVER_IMS, IMS_QUERY_DONE);
                    taskDone();

                    return;
                }

                CommandException cmdException = null;
                if ((ar.exception != null) && (ar.exception instanceof CommandException)) {
                    cmdException = (CommandException) ar.exception;
                    logd("cmdException error:" + cmdException.getCommandError());
                }

                if ((cmdException != null) && ((cmdException.getCommandError()
                        == CommandException.Error.OPERATION_NOT_ALLOWED)
                        || (cmdException != null) && (cmdException.getCommandError()
                            == CommandException.Error.OEM_ERROR_3))) {
                    if (!isNotSupportUtToCS() && !isNoNeedToCSFBWhenIMSRegistered()) {
                        Task task = new Task(TASK_QUERY_CFU_OVER_GSM, false,
                                toReasonString(msg.what));
                        mSuppServTaskDriven.appendTask(task);
                    } else {

                    }
                } else if (cmdException != null) {

                } else {
                    Message ret = mPhone.getCFCallbackMessage();
                    AsyncResult.forMessage(ret, ar.result, ar.exception);
                    ret.sendToTarget();
                }

                mPhone.setSystemProperty(CFU_QUERY_OVER_IMS, IMS_QUERY_DONE);
                taskDone();
                break;
            }
            case EVENT_QUERY_CFU_OVER_CS_AFTER_DATA_NOT_ATTACHED : {
                logd("Receive the event for query CFU over CS after data not attached");
                if (isNotSupportUtToCS()) {
                    // getAndSetCFUStatusFromLocal();
                    break;
                }
                Task task = new Task(TASK_QUERY_CFU_OVER_GSM, "Query Cfu over CS");
                mSuppServTaskDriven.appendTask(task);
                break;
            }
            case EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_GSM_DONE: {
                // 91, 92, 93 error code:
                // [REQUEST_NOT_SUPPORTED]:  Query without time slot

                CommandException cmdException = null;
                if ((ar.exception != null) && (ar.exception instanceof CommandException)) {
                    cmdException = (CommandException) ar.exception;
                    logd("cmdException error:" + cmdException.getCommandError());
                }

                if ((msg.arg1 == 1) && (cmdException != null) &&
                        (cmdException.getCommandError() ==
                                CommandException.Error.REQUEST_NOT_SUPPORTED)) {
                    Task task = new Task(TASK_TIME_SLOT_FAILED, false, toReasonString(msg.what));
                    mSuppServTaskDriven.appendTask(task);
                } else if ((msg.arg1 == 1) && (cmdException != null)) {

                } else {
                    Message ret = mPhone.getCFTimeSlotCallbackMessage();
                    AsyncResult.forMessage(ret, ar.result, ar.exception);
                    ret.sendToTarget();
                }
                taskDone();
                break;
            }
            case EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_IMS_DONE: {
                // ==== 91, 92, 93 Over imsphone ERROR code ====
                // [REQUEST_NOT_SUPPORTED]:
                //     Query without time slot. (startCFUQuery() would decide CSFB or over XCAP.)
                // Note: MtkGsmCdmaPhone does not handle failed case over IMS, TBD this part.

                CommandException cmdException = null;
                if ((ar.exception != null) && (ar.exception instanceof CommandException)) {
                    cmdException = (CommandException) ar.exception;
                    logd("cmdException error:" + cmdException.getCommandError());
                }

                if ((msg.arg1 == 1) && (cmdException != null) &&
                            (cmdException.getCommandError() ==
                            CommandException.Error.REQUEST_NOT_SUPPORTED)) {

                    // Use genaral flow.
                    Task task = new Task(TASK_TIME_SLOT_FAILED, false, toReasonString(msg.what));
                    mSuppServTaskDriven.appendTask(task);
                } else if ((msg.arg1 == 1) && (cmdException != null)) {

                } else {
                    Message ret = mPhone.getCFTimeSlotCallbackMessage();
                    AsyncResult.forMessage(ret, ar.result, ar.exception);
                    ret.sendToTarget();
                }

                mPhone.setSystemProperty(CFU_QUERY_OVER_IMS, IMS_QUERY_DONE);
                taskDone();
                break;
            }
            case EVENT_DATA_CONNECTION_ATTACHED: {
                mAttached.set(true);
                /* If there is the kind of message exist in the queue, it means
                 * it has ever sent a delay message to query cfu but the data is not attached
                 * at that time. We need to do CFU query again after data attached.
                 */
                if (hasMessages(EVENT_QUERY_CFU_OVER_CS_AFTER_DATA_NOT_ATTACHED)) {
                    logd("remove EVENT_QUERY_CFU_OVER_CS_AFTER_DATA_NOT_ATTACHED" +
                            ", and then start CFU query again");
                    removeMessages(EVENT_QUERY_CFU_OVER_CS_AFTER_DATA_NOT_ATTACHED);
                    startCFUQuery();
                    break;
                }
                Task task = new Task(TASK_QUERY_CFU, false, toReasonString(msg.what));
                mSuppServTaskDriven.appendTask(task);
                break;
            }
            case EVENT_DATA_CONNECTION_DETACHED:
                mAttached.set(false);
                break;
            case EVENT_REGISTERED_TO_NETWORK:
            case EVENT_ICCRECORDS_READY: {
                Task task = new Task(TASK_QUERY_CFU, false, toReasonString(msg.what));
                mSuppServTaskDriven.appendTask(task);
                break;
            }
            case EVENT_QUERY_CFU_OVER_CS: {
                if (isNotSupportUtToCS()) {
                    break;
                }
                Task task = new Task(TASK_QUERY_CFU_OVER_GSM, "Query Cfu over CS");
                mSuppServTaskDriven.appendTask(task);
                break;
            }
            case EVENT_CFU_STATUS_FROM_MD: {
                ar = (AsyncResult) msg.obj;
                /* Line1 is enabled or disabled while reveiving this EVENT */
                if (ar != null && ar.exception == null && ar.result != null) {
                   /* Line1 is enabled or disabled while reveiving this EVENT */
                   int[] cfuResult = (int[]) ar.result;
                   logd("handle EVENT_CFU_STATUS_FROM_MD:" + cfuResult[0]);
                   mCFUStatusFromMD = cfuResult[0];
                }
                break;
            }
            case EVENT_SIM_RECORDS_LOADED: {
                setSimRecordsLoaded(true);
                if (isNeedSyncSysPropToSIMforOTA()) {
                    if (syncSysPropToSIMforOTA()) {
                        setNeedSyncSysPropToSIMforOTA(false);
                        mNeeedSyncForOTA = 0;
                    }
                }
                /// M: Add for CT volte card when the phone type is cdma @{
                notifyCdmaCallForwardingIndicator();
                /// @}

                Task task = new Task(TASK_QUERY_CFU, false, toReasonString(msg.what));
                mSuppServTaskDriven.appendTask(task);
                break;
            }
            case EVENT_ICC_CHANGED: {
                onUpdateIcc();
                break;
            }
            case EVENT_CARRIER_CONFIG_LOADED: {
                setCarrierConfigLoaded(true);
                Task task = new Task(TASK_QUERY_CFU, false, toReasonString(msg.what));
                mSuppServTaskDriven.appendTask(task);
                break;
            }
            case EVENT_CLEAN_CFU_STATUS: {
                logd("Receive EVENT_CLEAN_CFU_STATUS, SIM has disposed");
                Task task = new Task(TASK_CLEAN_CFU_STATUS, toReasonString(msg.what));
                mSuppServTaskDriven.appendTask(task);
                break;
            }
            default: {
                logd("Unhandled msg: " + msg.what);
                break;
            }
        }
    }

    private void queryCallForwardStatusOverGSM() {
        getCallForwardingOption(QUERY_OVER_GSM, WITHOUT_TIME_SLOT);
    }

    private void queryCallForwardStatusOverIMS() {
        if (isIMSRegistered()) {
            if(isSupportCFUTimeSlot()) {
                getCallForwardingOption(QUERY_OVER_IMS, WITH_TIME_SLOT);
            } else {
                getCallForwardingOption(QUERY_OVER_IMS, WITHOUT_TIME_SLOT);
            }
            return;
        }
        taskDone();
    }

    // If SIM CFU status invalid, then query.
    private int checkEfCfis() {
        IccRecords r = mPhone.getIccRecords();
        if (r != null &&
                r instanceof MtkSIMRecords) {
            if (((MtkSIMRecords) r).checkEfCfis()) {
                return EFCFIS_STATUS_VALID;
            } else {
                return EFCFIS_STATUS_INVALID;
            }
        }

        // Default is true to available the API is not available.
        return EFCFIS_STATUS_NOT_READY;
    }

    private void setNeedSyncSysPropToSIMforOTA(boolean value) {
        String sValue = value ? "1" : "0";
        TelephonyManager.setTelephonyProperty(mPhone.getPhoneId(), CFU_STATUS_SYNC_FOR_OTA, sValue);
    }

    private boolean isNeedSyncSysPropToSIMforOTA() {
        String need = TelephonyManager.getTelephonyProperty(
                mPhone.getPhoneId(), CFU_STATUS_SYNC_FOR_OTA, "0");

        if (need.equals("1")) {
            return true;
        }

        return false;
    }

    private boolean syncSysPropToSIMforOTA() {
        if (mNeeedSyncForOTA == 0) {
            logd("syncSysPropToSIMforOTA: No need to sync (sim change): " +
                    mNeeedSyncForOTA);
            return true;
        } else if (mNeeedSyncForOTA == -1) {
            logd("syncSysPropToSIMforOTA: No need to sync (unknown): " +
                    mNeeedSyncForOTA);
            return false;
        }

        if (!getSimRecordsLoaded()) {
            logd("syncSysPropToSIMforOTA: SIM not loaded.");
            return false;
        }

        if (mCFUStatusFromMD == -1) {
            logd("syncSysPropToSIMforOTA: ECFU not receive yet.");
            return false;
        }

        int checkEfCfis = checkEfCfis();
        logd("syncSysPropToSIMforOTA: checkEfCfis = " + checkEfCfis);
        if (checkEfCfis == EFCFIS_STATUS_NOT_READY) {
            return false;
        } else if (checkEfCfis == EFCFIS_STATUS_VALID) {
            boolean cfuStatus = mPhone.getCallForwardingIndicator();
            if (cfuStatus) {
                logd("syncSysPropToSIMforOTA: true from system preference.");
                mPhone.setVoiceCallForwardingFlag(1, cfuStatus, "");
            } else if (mCFUStatusFromMD == 1) {
                cfuStatus = true;
                logd("syncSysPropToSIMforOTA: from MD.");
                mPhone.setVoiceCallForwardingFlag(1, cfuStatus, "");
            }
            return true;
        } else if (checkEfCfis == EFCFIS_STATUS_INVALID) {
            return true;
        }

        return true;
    }

    // Query CFU over IMS no matter get the result from GSM or not.
    private boolean needQueryCFUOverIms() {
        return CFU_QUERY_WHEN_IMS_REGISTERED_DEFAULT;
    }

    public void setIccRecordsReady() {
        Message msg = obtainMessage(EVENT_ICCRECORDS_READY);
        msg.sendToTarget();
    }

    private void reset() {
        mNeedGetCFU      = true;
        mSuppServTaskDriven.clearPendingTask();
    }

    private final ImsMmTelManager.RegistrationCallback mImsRegistrationCallback =
            new ImsMmTelManager.RegistrationCallback() {

        @Override
        public void onRegistered(
                @ImsRegistrationImplBase.ImsRegistrationTech int imsRadioTech) {
            if (DBG) logd("onImsRegistered imsRadioTech=" + imsRadioTech);

            if (!isMDSupportIMSSuppServ()) {
                if (mPhone.isOpTbcwWithCS()) {
                    mPhone.setTbcwMode(TBCW_WITH_CS);
                    mPhone.setTbcwToEnabledOnIfDisabled();
                } else {
                    // TBCW for VoLTE user
                    mPhone.setTbcwMode(TBCW_VOLTE_USER);
                    mPhone.setTbcwToEnabledOnIfDisabled();
                }
            }

            String cfuSetting = getCfuSetting();
            if (cfuSetting.equals(CFU_SETTING_ALWAYS_NOT_QUERY)) {
                logd("onImsRegistered, no need to query CFU over IMS due to ALWAYS_NOT_QUERY");
                return;
            }

            if (!isNotMachineTest()) {
                logd("onImsRegistered, no need to query CFU over IMS due to machine test");
                return;
            }

            setNeedGetCFU(true);
            if (getIMSQueryStatus()) {
                Task task = new Task(TASK_QUERY_CFU_OVER_IMS, "IMS state in service");
                mSuppServTaskDriven.appendTask(task);
            }
        }
    };

    private void getCallForwardingOption(int reason, boolean withTimeSlot) {
        MtkSuppServQueueHelper suppServQueueHelper = MtkSuppServManager.getSuppServQueueHelper();
        if (suppServQueueHelper == null) {
            switch (reason) {
                case QUERY_OVER_IMS:
                    if (withTimeSlot) {
                        queryCallForwardingOption(reason, withTimeSlot,
                                obtainMessage(EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_IMS_DONE,
                                1, 0, null));
                    } else {
                        queryCallForwardingOption(reason, withTimeSlot,
                                obtainMessage(EVENT_GET_CALL_FORWARD_BY_IMS_DONE, null));
                    }
                    break;
                case QUERY_OVER_GSM_OVER_UT:
                    if (withTimeSlot) {
                        if (isMDSupportIMSSuppServ()) {
                            queryCallForwardingOption(reason, withTimeSlot,
                                    obtainMessage(EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_GSM_DONE));
                        } else {
                            queryCallForwardingOption(reason, withTimeSlot,
                                    obtainMessage(EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_GSM_DONE,
                                    1, 0, null));
                        }
                    } else {
                        if (isMDSupportIMSSuppServ()) {
                            queryCallForwardingOption(reason, withTimeSlot,
                                    obtainMessage(EVENT_GET_CALL_FORWARD_BY_GSM_DONE));
                        } else {
                            queryCallForwardingOption(reason, withTimeSlot,
                                    obtainMessage(EVENT_GET_CALL_FORWARD_BY_GSM_DONE, null));
                        }
                    }
                    break;
                case QUERY_OVER_GSM:
                    queryCallForwardingOption(reason, withTimeSlot,
                                obtainMessage(EVENT_GET_CALL_FORWARD_BY_GSM_DONE));
                    break;
            }
        } else {
            switch (reason) {
                case QUERY_OVER_IMS:
                    if (withTimeSlot) {
                        suppServQueueHelper.getCallForwardingOption(reason,
                                withTimeSlot ? 1 : 0,
                                obtainMessage(EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_IMS_DONE,
                                        1, 0, null),
                                mPhone.getPhoneId());
                    } else {
                        suppServQueueHelper.getCallForwardingOption(reason,
                                withTimeSlot ? 1 : 0,
                                obtainMessage(EVENT_GET_CALL_FORWARD_BY_IMS_DONE, null),
                                mPhone.getPhoneId());
                    }
                    break;
                case QUERY_OVER_GSM_OVER_UT:
                    if (withTimeSlot) {
                        if (isMDSupportIMSSuppServ()) {
                            suppServQueueHelper.getCallForwardingOption(reason,
                                withTimeSlot ? 1 : 0,
                                obtainMessage(EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_GSM_DONE),
                                mPhone.getPhoneId());
                        } else {
                            suppServQueueHelper.getCallForwardingOption(reason,
                                withTimeSlot ? 1 : 0,
                                obtainMessage(EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_GSM_DONE,
                                        1, 0, null),
                                mPhone.getPhoneId());
                        }
                    } else {
                        if (isMDSupportIMSSuppServ()) {
                            suppServQueueHelper.getCallForwardingOption(reason,
                                withTimeSlot ? 1 : 0,
                                obtainMessage(EVENT_GET_CALL_FORWARD_BY_GSM_DONE),
                                mPhone.getPhoneId());
                        } else {
                            suppServQueueHelper.getCallForwardingOption(reason,
                                withTimeSlot ? 1 : 0,
                                obtainMessage(EVENT_GET_CALL_FORWARD_BY_GSM_DONE,null),
                                mPhone.getPhoneId());
                        }
                    }
                    break;
                case QUERY_OVER_GSM:
                    suppServQueueHelper.getCallForwardingOption(reason,
                                withTimeSlot ? 1 : 0,
                                obtainMessage(EVENT_GET_CALL_FORWARD_BY_GSM_DONE),
                                mPhone.getPhoneId());
                    break;
            }
        }
    }

    public void queryCallForwardingOption(int reason, boolean withTimeSlot, Message respCallback) {
        logd("queryCallForwardingOption, reason: " + reason + ", withTimeSlot: " + withTimeSlot);
        switch (reason) {
            case QUERY_OVER_IMS:
                Phone imsPhone = mPhone.getImsPhone();
                if (withTimeSlot) {
                    ((MtkImsPhone)imsPhone).getCallForwardInTimeSlot(CF_REASON_UNCONDITIONAL,
                        respCallback);
                } else {
                    ((MtkImsPhone)imsPhone).getCallForwardingOption(CF_REASON_UNCONDITIONAL,
                        respCallback);
                }
                break;
            case QUERY_OVER_GSM_OVER_UT:
                if (withTimeSlot) {
                    if (isMDSupportIMSSuppServ()) {
                        mPhone.mMtkCi.queryCallForwardInTimeSlotStatus(
                                CF_REASON_UNCONDITIONAL, SERVICE_CLASS_VOICE,
                                respCallback);
                    } else {
                        mPhone.getMtkSSRequestDecisionMaker().queryCallForwardInTimeSlotStatus(
                            CF_REASON_UNCONDITIONAL,
                            SERVICE_CLASS_VOICE,
                            respCallback);
                    }
                } else {
                    if (isMDSupportIMSSuppServ()) {
                        mPhone.mCi.queryCallForwardStatus(
                            CF_REASON_UNCONDITIONAL, SERVICE_CLASS_VOICE,
                            null, respCallback);
                    } else {
                        mPhone.getMtkSSRequestDecisionMaker().queryCallForwardStatus(
                            CF_REASON_UNCONDITIONAL,
                            SERVICE_CLASS_VOICE, null,
                            respCallback);
                    }
                }
                break;
            case QUERY_OVER_GSM:
                if (isVoiceInService()) {
                    mPhone.mCi.queryCallForwardStatus(CF_REASON_UNCONDITIONAL, SERVICE_CLASS_VOICE,
                            null, respCallback);
                } else {
                    AsyncResult.forMessage(respCallback, null,
                            new CommandException(CommandException.Error.GENERIC_FAILURE));
                    respCallback.sendToTarget();
                    taskDone();
                }
                break;
        }
    }

    public void notifyCarrierConfigLoaded() {
        Message msg = obtainMessage(EVENT_CARRIER_CONFIG_LOADED);
        msg.sendToTarget();
    }

    private boolean isSupportSuppServUTTest() {
        /* Run SS UT test only when it supports VoLTE and phone ID = 0 */
        return SystemProperties.get("persist.vendor.ims_support").equals("1") &&
                    SystemProperties.get("persist.vendor.volte_support").equals("1") &&
                    mPhone.getPhoneId() == 0;
    }

    // Need to consider to part if MD support IMS SS.:
    // 1. Request handle.
    // 2. Response handle.
    private boolean isMDSupportIMSSuppServ() {
        boolean r = false;
        if (SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1")) {
            r = true;
        }
        return r;
    }

    private boolean isNotSupportUtToCS() {
        return mPhone.isNotSupportUtToCSforCFUQuery();
    }

    private boolean isNoNeedToCSFBWhenIMSRegistered() {
        return mPhone.isNoNeedToCSFBWhenIMSRegistered();
    }

    private boolean isSupportCFUTimeSlot() {
        return mPhone.isSupportCFUTimeSlot();
    }

    private boolean isResetCSFBStatusAfterFlightMode() {
        return mPhone.isResetCSFBStatusAfterFlightMode();
    }

    private MtkSuppServUtTest makeMtkSuppServUtTest(Intent intent) {
        return new MtkSuppServUtTest(mContext, intent, (Phone) mPhone);
    }

    private void notifyCdmaCallForwardingIndicator() {
        if (mPhone.isGsmSsPrefer() &&
                mPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
            mPhone.notifyCallForwardingIndicator();
        }
    }

    public String getXCAPErrorMessageFromSysProp(CommandException.Error error) {
        String propNamePrefix = PROPERTY_ERROR_MESSAGE_FROM_XCAP + "." + mPhone.getPhoneId();

        String fullErrorMsg = "";
        String errorMsg = null;

        int idx = 0;
        String propName = propNamePrefix +"." + idx;
        String propValue = "";

        propValue = SystemProperties.get(propName, "");


        while (!propValue.equals("")) {
            fullErrorMsg += propValue;

            idx++;
            propName = propNamePrefix +"." + idx;
            propValue = SystemProperties.get(propName, "");
        }

        logd("fullErrorMsg: " + fullErrorMsg);

        String errorCode = "";
        switch (error) {
            case OEM_ERROR_25:
                errorCode = "409";
                break;
            default:
                return errorMsg;
        }

        if (!fullErrorMsg.startsWith(errorCode)) {
            return errorMsg;
        }

        errorMsg = fullErrorMsg.substring(errorCode.length() + 1);
        logd("errorMsg: " + errorMsg);

        return errorMsg;
    }

    private void cleanCFUStatus() {
        if (mPhone != null) {
            setAlwaysQueryDoneFlag(false);
            mPhone.cleanCallForwardingIndicatorFromSharedPref();
            mPhone.notifyCallForwardingIndicatorWithoutCheckSimState();
        }
        taskDone();
    }

    private String toReasonString(int event) {
        switch (event) {
            case EVENT_DATA_CONNECTION_ATTACHED:
                return "Data Attached";
            case EVENT_REGISTERED_TO_NETWORK:
                return "CS in service";
            case EVENT_ICCRECORDS_READY:
                return "ICCRecords ready";
            case EVENT_SIM_RECORDS_LOADED:
                return "SIM records loaded";
            case EVENT_CARRIER_CONFIG_LOADED:
                return "Carrier config loaded";
            case EVENT_CLEAN_CFU_STATUS:
                return "Clean CFU status";
            default:
        }
        return "Unknown reason, should not be here.";
    }

    private String toEventString(int event) {
        switch (event) {
            case EVENT_REGISTERED_TO_NETWORK:
                return "EVENT_REGISTERED_TO_NETWORK";
            case EVENT_ICCRECORDS_READY:
                return "EVENT_ICCRECORDS_READY";
            case EVENT_DATA_CONNECTION_ATTACHED:
                return "EVENT_DATA_CONNECTION_ATTACHED";
            case EVENT_DATA_CONNECTION_DETACHED:
                return "EVENT_DATA_CONNECTION_DETACHED";
            case EVENT_GET_CALL_FORWARD_BY_GSM_DONE:
                return "EVENT_GET_CALL_FORWARD_BY_GSM_DONE";
            case EVENT_GET_CALL_FORWARD_BY_IMS_DONE:
                return "EVENT_GET_CALL_FORWARD_BY_IMS_DONE";
            case EVENT_CALL_FORWARDING_STATUS_FROM_MD:
                return "EVENT_CALL_FORWARDING_STATUS_FROM_MD";
            case EVENT_QUERY_CFU_OVER_CS:
                return "EVENT_QUERY_CFU_OVER_CS";
            case EVENT_CFU_STATUS_FROM_MD:
                return "EVENT_CFU_STATUS_FROM_MD";
            case EVENT_SS_RESET:
                return "EVENT_SS_RESET";
            case EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_GSM_DONE:
                return "EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_GSM_DONE";
            case EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_IMS_DONE:
                return "EVENT_GET_CALL_FORWARD_TIME_SLOT_BY_IMS_DONE";
            case EVENT_SIM_RECORDS_LOADED:
                return "EVENT_SIM_RECORDS_LOADED";
            case EVENT_ICC_CHANGED:
                return "EVENT_ICC_CHANGED";
            case EVENT_CARRIER_CONFIG_LOADED:
                return "EVENT_CARRIER_CONFIG_LOADED";
            case EVENT_QUERY_CFU_OVER_CS_AFTER_DATA_NOT_ATTACHED:
                return "EVENT_QUERY_CFU_OVER_CS_AFTER_DATA_NOT_ATTACHED";
            case EVENT_CLEAN_CFU_STATUS:
                return "EVENT_CLEAN_CFU_STATUS";
            default:
        }
        return "UNKNOWN_EVENT_ID";
    }

    private void loge(String s) {
        Rlog.e(LOG_TAG, "[" + mPhone.getPhoneId() + "]" + s);
    }

    private void logw(String s) {
        Rlog.w(LOG_TAG, "[" + mPhone.getPhoneId() + "]" + s);
    }

    private void logi(String s) {
        // default user/userdebug debug level set as INFO
        Rlog.i(LOG_TAG, "[" + mPhone.getPhoneId() + "]" + s);
    }

    private void logd(String s) {
        // default eng debug level set as DEBUG
        Rlog.d(LOG_TAG, "[" + mPhone.getPhoneId() + "]" + s);
    }

    private void logv(String s) {
        Rlog.v(LOG_TAG, "[" + mPhone.getPhoneId() + "]" + s);
    }

    public static String encryptString(String message) {
        Base64.Encoder encoder = Base64.getEncoder();
        byte[] textByte = null;
        try {
            textByte = message.getBytes("UTF-8");
        } catch (Exception e) {
            e.printStackTrace();
            textByte = null;
        }

        if (textByte == null) return "";

        String encryptedString = encoder.encodeToString(textByte);
        return encryptedString;
    }
}

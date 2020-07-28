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

package com.mediatek.op12.telephony.selfactivation;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Looper;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.PcoData;
import android.telephony.PhoneStateListener;
import android.telephony.Rlog;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.widget.Toast;

import java.util.HashMap;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;

import com.mediatek.internal.telephony.selfactivation.ISelfActivation;
import com.mediatek.internal.telephony.selfactivation.SelfActivationDefault;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.RadioManager;


public class SelfActivationPcoBasedSm extends SelfActivationDefault {
    private static final boolean DBG = true;
    private static final String TAG = "SelfActivationPcoBasedSm";

    // Project configuration constants
    private static final String PROP_VZW_DEVICE_TYPE = "persist.vendor.vzw_device_type";
    private static final String PROPERTY_OP12_DEVICE_MODEL = "persist.vendor.op12.model";
    private static final String OP12_BRANDED_DEVICE = "0";

    // PCO value definition
    public static final int PCO_VALUE_DEFAULT = 0;
    public static final int PCO_VALUE_SELF_ACT = 5;
    public static final int PCO_VALUE_INVALID = -1;
    // OP12 PCO reserved index
    private static final int PCO_ID_FF00H = 65280;

    // Internal debug usage
    public static final String ACTION_DEBUG_SELFACTIVATION =
            "mediatek.intent.action.DEBUG_SELFACTIVATION";
    public static final String KEY_DBG_ACTION = "dbg_action";
    public static final String KEY_DBG_PARAMETER = "dbg_param";
    public static final int DEBUG_ACTION_NONE = 0;
    public static final int DEBUG_ACTION_RESET = 1;
    public static final int DEBUG_ACTION_DUMP = 2;
    public static final int DEBUG_ACTION_RADIO_CTRL = 3;
    public static final int DEBUG_ACTION_PCO_EVENT = 4;
    public static final String PROPERTY_RADIO_CONTROL = "persist.vendor.pco5.radio.ctrl";

    // 5 minutes timer to disable radio after call end.
    private static final int PCO_DISABLE_RADIO_TIMER_MS = 5*60*1000;

    private int mSubId = 0;
    private boolean mInitialized = false;
    private PcoStateMachine mStateMachine = null;
    private SaRadioEnabledCause mRadioEnabledCause = null;
    private SaCallState mCallState = null;
    private SelfActivateState mSelfActivateState = null;
    private PCO520State m520State = null;
    private RadioManager mRadioManager = null;
    private boolean mEnableRadioControl = false;
    private boolean mInitRadioControl = false;

    // listeners
    private SubscriptionManager mSubscriptionManager = null;
    private TelephonyManager mTelephonyManager = null;
    private DebugReceiver mDebugReceiver = new DebugReceiver(this);
    private CallStateListener mCallStateListener = null;

    // self activation operation
    private static final int OPERATION_ADD_DATA_AGREE = 0;
    private static final int OPERATION_ADD_DATA_DECLINE = 1;
    private static final int OPERATION_MAKE_CALL = 2;

    // M: ALPS03224621 workaround patch to pass script issue
    // TODO: Remove it!
    private static final int PCO_DISABLE_RADIO_DELAY_MS =
            SystemProperties.getInt("persist.vendor.pco.delay", 0);

    // Public APIs @{
    @Override
    public int selfActivationAction(int action, Bundle param) {
        int ret = 0;
        Rlogd("selfActivationAction action: " + action);

        if (isInitialized()) {
            Message msg = mStateMachine.getHandler().obtainMessage(SaEvents.EVENT_SELF_ACT);
            switch (action) {
                case ISelfActivation.ACTION_ADD_DATA_SERVICE:
                    handleActionAddDataService(param, msg);
                    break;
                case ISelfActivation.ACTION_MO_CALL:
                    handleActionMoCall(param, msg);
                    break;
                case ISelfActivation.ACTION_RESET_520_STATE:
                    updatePCO520State(false);
                    break;
                default:
                    throw new IllegalArgumentException("Invalid action: " + action);
            }
            mStateMachine.getHandler().sendMessage(msg);
        } else {
            ret = -1;
            Rloge("selfActivationAction not ready! action: " + action);
        }
        return ret;
    }

    @Override
    public int getSelfActivateState() {
        int state = ISelfActivation.STATE_NONE;
        if (isInitialized()) {
            state = mSelfActivateState.getState();
        } else {
            Rloge("getSelfActivateState not ready!");
        }
        return state;
    }

    @Override
    public int getPCO520State() {
        int state = ISelfActivation.STATE_520_NONE;
        if (isInitialized()) {
            state = m520State.getState();
        } else {
            Rloge("getPCO520State not ready!");
        }
        return state;
    }
    @Override
    public ISelfActivation buildParams() {
        synchronized(this) {
            if (mContext == null || mCi == null) {
                throw new RuntimeException("SelfActivationPcoBasedSm is not initialized!");
            }
            init();
        }
        return this;
    }
    // Public APIs @}

    // Internal APIs
    private void init() {
        mStateMachine = new PcoStateMachine(mPhoneId);
        mSubId = MtkSubscriptionManager.getSubIdUsingPhoneId(mPhoneId);
        mRadioEnabledCause = new SaRadioEnabledCause();
        mCallState = new SaCallState(mPhoneId);
        mSelfActivateState = new SelfActivateState(mPhoneId);
        m520State = new PCO520State(mPhoneId);
        mSubscriptionManager = SubscriptionManager.from(mContext);

        // TODO: Shall reference to the Verizon device type definition
        // Customer can configure default value in optr_package_config.mk
        // Moreover, we also provide DEBUG_ACTION_RADIO_CTRL broadcast
        // to modify this behavior in runtime.
        mEnableRadioControl = true;
        if ("1".equals(SystemProperties.get(PROPERTY_RADIO_CONTROL, "0"))) {
            mEnableRadioControl = true;
        } else {
            mEnableRadioControl = false;
        }

        mStateMachine.start();
        mInitialized = true;
        Rlog.d(TAG, this.toString());
    }

    private void handleActionAddDataService(Bundle param, Message msg) {
        if (param != null) {
            int operation = param.getInt(ISelfActivation.EXTRA_KEY_ADD_DATA_OP);
            switch (operation) {
                case ISelfActivation.ADD_DATA_DECLINE:
                    msg.arg1 = OPERATION_ADD_DATA_DECLINE;
                    break;
                case ISelfActivation.ADD_DATA_AGREE:
                    msg.arg1 = OPERATION_ADD_DATA_AGREE;
                    break;
                default:
                    throw new IllegalArgumentException("Invalid operation: " + operation);
            }
        } else {
            throw new IllegalArgumentException("Null param");
        }
    }

    private void handleActionMoCall(Bundle param, Message msg) {
        if (param != null) {
            msg.arg1 = OPERATION_MAKE_CALL;
            int callType = param.getInt(ISelfActivation.EXTRA_KEY_MO_CALL_TYPE);
            switch (callType) {
                case ISelfActivation.CALL_TYPE_NORMAL:
                    msg.arg2 = SaRadioEnabledCause.CAUSE_NORMAL_CALL;
                    break;
                case ISelfActivation.CALL_TYPE_EMERGENCY:
                    msg.arg2 = SaRadioEnabledCause.CAUSE_EMERGENCY_CALL;
                    break;
                default:
                    throw new IllegalArgumentException("Invalid callType: " + callType);
            }
        } else {
            throw new IllegalArgumentException("Null param");
        }
    }

    private void handlePcoEvent(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        if (ar.exception != null) {
            Rloge("PCO_DATA exception: " + ar.exception);
            return;
        }

        PcoData pcoData = (PcoData)(ar.result);
        if (pcoData == null) {
            Rloge("NULL PCO_DATA");
            return;
        }

        if (PCO_ID_FF00H == pcoData.pcoId) {
            // Verizon FF00H contain only 1 byte data
            // Convert ascii char to integer
            int newPcoValue = pcoData.contents[0] - '0';
            int currPcoValue = getPcoValue();
            Rlogd("handlePcoEvent(): " +
                    "PCO ID: " + pcoData.pcoId +
                    ", newPcoValue: " + newPcoValue +
                    ", currPcoValue: " + currPcoValue);
            if (newPcoValue != currPcoValue) {
                updatePcoValue(newPcoValue);
                // Update self activation state
                updateSelfActivateState(newPcoValue);
                if(newPcoValue == 0 && currPcoValue == 5) updatePCO520State(true);
            }
        }
    }

    private void handleNetworkRejectEvent(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        if (ar.exception != null || ar.result == null) {
            Rloge("[handleNetworkRejectEvent] exception happeneds");
            return;
        }

        // result[0]: <emm_cause>
        // result[1]: <esm_cause>
        // result[2]: <event_type>
        //            0 EMM_CAUSE_SOURCE_OTHER
        //            1 EMM_CAUSE_SOURCE_ATTACH_REJECT
        //            2 EMM_CAUSE_SOURCE_TAU_REJECT
        //            3 EMM_CAUSE_SOURCE_NW_DETAC
        //
        int[] ints = (int[]) ar.result;
        if (ints.length < 3) {
           Rloge("[handleNetworkRejectEvent] urc format error");
           return;
        }
        int emm_cause = ints[0];
        int esm_cause = ints[1];
        int event_type = ints[2];
        Rlogd("[handleNetworkRejectEvent] emm_cause:" + emm_cause + ", esm_cause:"
                + esm_cause + ", event_type:" + event_type);
        // VZ_REQ_LTEPCO_34859 reqquests that device shall follow PCO = 5 behavior
        // if device recevies EMM casue code 8 when attaching fail.
        if (emm_cause == 8 && event_type == 1) {
            updatePcoValue(PCO_VALUE_SELF_ACT);
        }

    }

    private boolean isInitialized() {
        return mInitialized;
    }

    private boolean isSelfActivatedPco() {
        int currPcoValue = getPcoValue();
        return (PCO_VALUE_SELF_ACT == currPcoValue) ? true : false;
    }

    private RadioManager getRadioManager() {
        if (mRadioManager == null) {
            mRadioManager = RadioManager.getInstance();
        }
        return mRadioManager;
    }

    public SelfActivationPcoBasedSm(int phoneId) {
        super(phoneId);
    }

    private void registerEvents() {
        mCi.registerForAvailable(mStateMachine.getHandler(), SaEvents.EVENT_RM_RADIO_AVAILABLE, null);

        // register radio state
        mCi.registerForRadioStateChanged(mStateMachine.getHandler(), SaEvents.EVENT_RM_RADIO_CHANGED, null);

        // PCO event
        mCi.registerForPcoData(mStateMachine.getHandler(), SaEvents.EVENT_PCO_STATE, null);
        ((MtkRIL) mCi).registerForPcoDataAfterAttached(mStateMachine.getHandler(),
                SaEvents.EVENT_PCO_STATE, null);

        // register network rejecting event
        ((MtkRIL) mCi).registerForNetworkReject(mStateMachine.getHandler(),
                SaEvents.EVENT_NETWORK_REJECT, null);

        IntentFilter debugIntentFilter =
                new IntentFilter(ACTION_DEBUG_SELFACTIVATION);
        mContext.registerReceiver(mDebugReceiver, debugIntentFilter);

        // register call state
        mTelephonyManager = new TelephonyManager(mContext, mSubId);
        if (mTelephonyManager != null) {
            regCallStateListener(mSubId);
        }
        // register SIM state
        if (mSubscriptionManager != null) {
            mSubscriptionManager.addOnSubscriptionsChangedListener(mOnSubInfoChangeListener);
            Rlogd("registerEvents: Subscription info");
        }
    }

    private void regCallStateListener(int subId) {
        // Not to check isValidSubscriptionId() for case of ECC w/o SIM.
        // The radio shall be disabled after the call with PCO 5.
        if (mCallStateListener == null) {
            mCallStateListener = new CallStateListener(subId,
                    mStateMachine.getHandler().getLooper());
            mTelephonyManager.listen(mCallStateListener, PhoneStateListener.LISTEN_CALL_STATE);
            Rlogd("regCallStateListener: create mCallStateListener with subId: " + subId);
        } else if (mCallStateListener.getSubId() != subId) {
            Rlogd("regCallStateListener: update mCallStateListener with subId old: " +
                    mCallStateListener.getSubId() + " new: " + subId);
            mTelephonyManager.listen(mCallStateListener, PhoneStateListener.LISTEN_NONE);
            mTelephonyManager = new TelephonyManager(mContext, subId);
            mCallStateListener = new CallStateListener(subId,
                    mStateMachine.getHandler().getLooper());
            mTelephonyManager.listen(mCallStateListener, PhoneStateListener.LISTEN_CALL_STATE);
        } else {
            // The subId is the same, do nothing!
        }
    }

    private void deRegisterEvents() {
        mCi.unregisterForAvailable(mStateMachine.getHandler());
        mCi.unregisterForRadioStateChanged(mStateMachine.getHandler());
        mCi.unregisterForPcoData(mStateMachine.getHandler());
        ((MtkRIL) mCi).unregisterForPcoDataAfterAttached(mStateMachine.getHandler());
        ((MtkRIL) mCi).unregisterForNetworkReject(mStateMachine.getHandler());
        mContext.unregisterReceiver(mDebugReceiver);
        mTelephonyManager.listen(mCallStateListener, PhoneStateListener.LISTEN_NONE);
        mSubscriptionManager.removeOnSubscriptionsChangedListener(mOnSubInfoChangeListener);
    }

    class PcoStateMachine extends StateMachine {
        int mPhoneId = 0;
        // States
        private SaBaseState mBaseState = new SaBaseState();
        private SaIdleState mIdleState = new SaIdleState();
        private SaRadioDisabledState mRadioDisabledState = new SaRadioDisabledState();
        private SaRadioEnablingState mRadioEnablingState = new SaRadioEnablingState();
        private SaRadioEnabledState mRadioEnabledState = new SaRadioEnabledState();
        private SaRadioDisablingState mRadioDisablingState = new SaRadioDisablingState();

        public PcoStateMachine(int phoneId) {
            super("PcoStateMachine[" + phoneId +"]");
            mPhoneId = phoneId;
            addState(mBaseState);
                addState(mIdleState, mBaseState);
                addState(mRadioDisabledState, mBaseState);
                addState(mRadioEnablingState, mBaseState);
                addState(mRadioEnabledState, mBaseState);
                addState(mRadioDisablingState, mBaseState);
            setInitialState(mIdleState);
        }

        // The parent state for all other states.
        private class SaBaseState extends State {
            @Override
            public void enter() {
                Rlogd("SaBaseState: enter");
                registerEvents();

            }
            @Override
            public void exit() {
                Rlogd("SaBaseState: exit");
                deRegisterEvents();
            }
            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;
                Rlogd(getName() + ": processMessage msg = " + SaEvents.print(msg.what));
                switch (msg.what) {
                    case SaEvents.EVENT_RM_RADIO_AVAILABLE:
                        if (!mInitRadioControl) {
                            syncRadioControlToMd(mEnableRadioControl, "radio available");
                            mInitRadioControl = true;
                        }
                        break;
                    case SaEvents.EVENT_SYNC_RADIO_CTRL_DONE:
                        // Enable radio control if disable fail for legacy MD.
                        AsyncResult asyncResult = (AsyncResult) msg.obj;
                        if (asyncResult != null && asyncResult.exception == null) {
                            int enable = (int) asyncResult.userObj;
                            Rlogd("Sync radio control: " + enable + " success");
                        } else {
                            Rloge("Sync radio control failed, forece enable!");
                            mEnableRadioControl = true;
                        }
                        break;
                    case SaEvents.EVENT_SIM_STATE_CHANGED:
                        // Only re-enable radio if PCO is 5
                        if (isSelfActivatedPco()) {
                            Rlogd("SIM CHANGED with !PCO_VALUE_SELF_ACT, try to enable radio");
                            mRadioEnabledCause.addCause(SaRadioEnabledCause.CAUSE_SIM_CHANGED);
                            mStateMachine.transitionTo(mRadioEnablingState);
                        }
                        resetPcoValue("SaBaseState: EVENT_SIM_STATE_CHANGED");
                        break;
                    case SaEvents.EVENT_SELF_ACT:
                        int operation = msg.arg1;
                        Rlogd("SaBaseState: EVENT_SELF_ACT with operation: " + operation);
                        switch (operation) {
                            case OPERATION_ADD_DATA_AGREE:
                                mRadioEnabledCause.addCause(
                                        SaRadioEnabledCause.CAUSE_ADD_DATA_SERVICE);
                                Rlogd("Remove SaEvents.EVENT_DISABLE_RADIO_PCO because user agree"
                                        + " to add data service");
                                getHandler().removeMessages(SaEvents.EVENT_DISABLE_RADIO_PCO);
                                break;
                            case OPERATION_ADD_DATA_DECLINE:
                                mRadioEnabledCause.removeCause(
                                        SaRadioEnabledCause.CAUSE_ADD_DATA_SERVICE);
                                break;
                            case OPERATION_MAKE_CALL:
                                int cause = msg.arg2;
                                mRadioEnabledCause.addCause(cause);
                                break;
                            default:
                                Rloge("Unexpected operation: " + operation);
                                break;
                        }
                        break;
                    case SaEvents.EVENT_RM_RADIO_CHANGED:
                        int radioState = mCi.getRadioState();
                        Rlogd("EVENT_RM_RADIO_CHANGED, state = " + radioState);
                        if (TelephonyManager.RADIO_POWER_ON == radioState) {
                            sendEvent(SaEvents.EVENT_RM_RADIO_ON);
                        } else if (TelephonyManager.RADIO_POWER_OFF == radioState) {
                            sendEvent(SaEvents.EVENT_RM_RADIO_OFF);
                        }
                        break;
                    default:
                        Rlogd("not-handled msg.what: " + SaEvents.print(msg.what));
                        retVal = NOT_HANDLED;
                }
                return retVal;
            }
        }

        private class SaIdleState extends State {
            @Override
            public void enter() {
                Rlogd("SaIdleState: enter");
                mRadioEnabledCause.setCause(SaRadioEnabledCause.CAUSE_NONE);
            }
            @Override
            public void exit() {
                Rlogd("SaIdleState: exit");
            }
            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;
                Rlogd(getName() + ": processMessage msg = " + SaEvents.print(msg.what));
                switch (msg.what) {
                    case SaEvents.EVENT_RM_RADIO_ON:
                        transitionTo(mRadioEnabledState);
                        break;
                    case SaEvents.EVENT_RM_RADIO_OFF:
                        transitionTo(mRadioDisabledState);
                        break;
                    default:
                        Rlogd("not-handled msg.what: " + SaEvents.print(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }
                return retVal;
            }
        }

        private class SaRadioDisabledState extends State {
            @Override
            public void enter() {
                Rlogd("SaRadioDisabledState: enter");
                mRadioEnabledCause.setCause(SaRadioEnabledCause.CAUSE_NONE);
            }
            @Override
            public void exit() {
                Rlogd("SaRadioDisabledState: exit");
            }
            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;
                Rlogd(getName() + ": processMessage msg = " + SaEvents.print(msg.what));
                switch (msg.what) {
                    case SaEvents.EVENT_RM_RADIO_ON:
                        transitionTo(mRadioEnabledState);
                        break;
                    case SaEvents.EVENT_SELF_ACT:
                        int operation = msg.arg1;
                        Rlogd("SaRadioDisabledState: EVENT_SELF_ACT with operation: " + operation);
                        switch (operation) {
                            case OPERATION_ADD_DATA_AGREE:
                                mRadioEnabledCause.addCause(
                                        SaRadioEnabledCause.CAUSE_ADD_DATA_SERVICE);
                                transitionTo(mRadioEnablingState);
                                Rlogd("Enable radio to add data service");
                                break;
                            case OPERATION_ADD_DATA_DECLINE:
                                mRadioEnabledCause.removeCause(
                                        SaRadioEnabledCause.CAUSE_ADD_DATA_SERVICE);
                                break;
                            case OPERATION_MAKE_CALL:
                                int cause = msg.arg2;
                                mRadioEnabledCause.addCause(cause);
                                // For case emergency call in radio disabled state.
                                // common flow will enable radio and go to
                                // RadioEnabledState directly without SelfActivationPcoBasedSm involved.
                                if (SaRadioEnabledCause.CAUSE_NORMAL_CALL == cause) {
                                    transitionTo(mRadioEnablingState);
                                    Rlogd("Enable radio to make call");
                                }
                                break;
                            default:
                                Rloge("Unexpected operation: " + operation);
                                break;
                        }
                        break;
                    // This only happen when MO with flight mode ON & PCO 5
                    case SaEvents.EVENT_CALL_STATE_START:
                    case SaEvents.EVENT_CALL_STATE_IDLE:
                        Rlogd("defer call state message");
                        deferMessage(msg);
                        break;
                    case SaEvents.EVENT_DBG_RESET:
                        Rlogd("Reset! try to re-enable radio");
                        transitionTo(mRadioEnablingState);
                        break;

                    default:
                        Rlogd("not-handled msg.what: " + SaEvents.print(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }
                return retVal;
            }
        }

        private class SaRadioEnablingState extends State {
            @Override
            public void enter() {
                Rlogd("SaRadioEnablingState: enter");
                int ret = setRadioPower(true, RadioManager.REASON_PCO_ON);
                if (RadioManager.ERROR_AIRPLANE_MODE == ret) {
                    transitionTo(mRadioDisabledState);
                }
            }
            @Override
            public void exit() {
                Rlogd("SaRadioEnablingState: exit");
            }
            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;
                Rlogd(getName() + ": processMessage msg = " + SaEvents.print(msg.what));
                switch (msg.what) {
                    case SaEvents.EVENT_RM_RADIO_ON:
                        transitionTo(mRadioEnabledState);
                        break;
                    case SaEvents.EVENT_RM_RADIO_OFF:
                        transitionTo(mRadioDisabledState);
                        break;
                    case SaEvents.EVENT_SIM_STATE_CHANGED:
                        mRadioEnabledCause.addCause(SaRadioEnabledCause.CAUSE_SIM_CHANGED);
                        break;
                    case SaEvents.EVENT_CALL_STATE_START:
                    case SaEvents.EVENT_CALL_STATE_IDLE:
                        Rlogd("defer call state message");
                        deferMessage(msg);
                        break;
                    default:
                        Rlogd("not-handled msg.what: " + SaEvents.print(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }
                return retVal;
            }
        }

        private class SaRadioEnabledState extends State {
            @Override
            public void enter() {
                Rlogd("SaRadioEnabledState: enter");
                disableRadioIfNeed();
            }
            @Override
            public void exit() {
                Rlogd("SaRadioEnabledState: exit");
            }
            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;
                Rlogd(getName() + ": processMessage msg = " + SaEvents.print(msg.what));
                switch (msg.what) {
                    case SaEvents.EVENT_RM_RADIO_OFF:
                        transitionTo(mRadioDisabledState);
                        break;
                    case SaEvents.EVENT_PCO_STATE:
                        handlePcoEvent(msg);
                        disableRadioIfNeed();
                        break;
                    case SaEvents.EVENT_CALL_STATE_START:
                        mCallState.incCallRefCount();
                        if (isStartCallEndTimer()) {
                            Rlogd("Stop timer by call start");
                            getHandler().removeMessages(SaEvents.EVENT_TIMER_RADIO_OFF);
                        }
                        break;
                    case SaEvents.EVENT_CALL_STATE_IDLE:
                        boolean lastCall = mCallState.decCallRefCount();
                        if (isSelfActivatedPco()) {
                            if (lastCall) {
                                Rlogd("Start timer by call end");
                                getHandler().sendEmptyMessageDelayed(SaEvents.EVENT_TIMER_RADIO_OFF,
                                        PCO_DISABLE_RADIO_TIMER_MS);
                            }
                        }
                        break;
                    case SaEvents.EVENT_TIMER_RADIO_OFF:
                        disableRadioCallEndTimer();
                        break;
                    case SaEvents.EVENT_DISABLE_RADIO_PCO:
                        transitionTo(mRadioDisablingState);
                        break;
                    case SaEvents.EVENT_SIM_STATE_CHANGED:
                        Rlogd("Stay in radio enabled state");
                        break;
                    case SaEvents.EVENT_SELF_ACT:
                        int operation = msg.arg1;
                        if (OPERATION_ADD_DATA_DECLINE == operation) {
                            mRadioEnabledCause.removeCause(
                                    SaRadioEnabledCause.CAUSE_ADD_DATA_SERVICE);
                            Rlogd("Disable radio because user decline to add data service");
                            sendEvent(SaEvents.EVENT_DISABLE_RADIO_PCO);
                        } else if (OPERATION_ADD_DATA_AGREE == operation) {
                            mRadioEnabledCause.addCause(
                                    SaRadioEnabledCause.CAUSE_ADD_DATA_SERVICE);
                            Rlogd("Remove SaEvents.EVENT_DISABLE_RADIO_PCO because user agree"
                                    + " to add data service");
                            getHandler().removeMessages(SaEvents.EVENT_DISABLE_RADIO_PCO);
                        }
                        break;
                    case SaEvents.EVENT_NETWORK_REJECT:
                        handleNetworkRejectEvent(msg);
                        disableRadioIfNeed();
                        break;
                    default:
                        Rlogd("not-handled msg.what: " + SaEvents.print(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }
                return retVal;
            }
        }

        private class SaRadioDisablingState extends State {
            @Override
            public void enter() {
                Rlogd("SaRadioDisablingState: enter");
                int ret = setRadioPower(false, RadioManager.REASON_PCO_OFF);
                if (RadioManager.ERROR_PCO_ALREADY_OFF == ret) {
                    transitionTo(mRadioDisabledState);
                    Rlogd("radio already off!");
                }
            }
            @Override
            public void exit() {
                Rlogd("SaRadioDisablingState: exit");
            }
            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;
                Rlogd(getName() + ": processMessage msg = " + SaEvents.print(msg.what));
                switch (msg.what) {
                    case SaEvents.EVENT_RM_RADIO_OFF:
                        transitionTo(mRadioDisabledState);
                        break;
                    default:
                        Rlogd("not-handled msg.what: " + SaEvents.print(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }
                return retVal;
            }
        }
    }

    class CallStateListener extends PhoneStateListener {
        private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

        public CallStateListener(int subId, Looper looper) {
            super(looper);
            mSubId = subId;
        }

        public int getSubId() {
            return mSubId;
        }

        @Override
        public void onCallStateChanged(int state, String phoneNumber) {
            Rlogd("onCallStateChanged: " + state);
            switch (state) {
                case TelephonyManager.CALL_STATE_OFFHOOK:
                    sendEvent(SaEvents.EVENT_CALL_STATE_START);
                    break;
                case TelephonyManager.CALL_STATE_IDLE:
                    sendEvent(SaEvents.EVENT_CALL_STATE_IDLE);
                    break;
                default:
                    break;
            }
        }
    }

    private final SubscriptionManager.OnSubscriptionsChangedListener mOnSubInfoChangeListener
            = new SubscriptionManager.OnSubscriptionsChangedListener() {
        @Override
        public void onSubscriptionsChanged() {
            mSubId = MtkSubscriptionManager.getSubIdUsingPhoneId(mPhoneId);
            regCallStateListener(mSubId);
            SubscriptionInfo info = mSubscriptionManager.getActiveSubscriptionInfo(mSubId);
            if (info != null) {
                String newIccId = info.getIccId();
                if (!newIccId.isEmpty()) {
                    String currIccId =
                            SaOp12PersistDataHelper.getStringData(mContext, mPhoneId,
                                    SaOp12PersistDataHelper.DATA_KEY_ICCID, "");
                    Rlogd("onSubscriptionsChanged currIccid: " + currIccId + " newIccId: " + newIccId);
                    if (!newIccId.equalsIgnoreCase(currIccId)) {
                        SaOp12PersistDataHelper.putStringData(mContext, mPhoneId,
                                SaOp12PersistDataHelper.DATA_KEY_ICCID, newIccId);
                        sendEvent(SaEvents.EVENT_SIM_STATE_CHANGED);
                    }
                }
            } else {
                // Rloge("onSubscriptionsChanged: null sub info!");
            }
        }
    };

    private class DebugReceiver extends BroadcastReceiver {
        private SelfActivationPcoBasedSm mSaSmInstance = null;

        public DebugReceiver(SelfActivationPcoBasedSm saSmInstance) {
            mSaSmInstance = saSmInstance;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(ACTION_DEBUG_SELFACTIVATION)) {
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                int debugAction = intent.getIntExtra(KEY_DBG_ACTION, DEBUG_ACTION_NONE);
                switch (debugAction) {
                    case DEBUG_ACTION_RESET:
                        if (SubscriptionManager.INVALID_SUBSCRIPTION_ID == subId) {
                            Rlogd("Reset selfactivation state");
                            resetPcoValue("ACTION_RESET_SELFACTIVATION");
                            sendEvent(SaEvents.EVENT_DBG_RESET);
                        } else if (mSubId == subId) {
                            Rlogd("Reset selfactivation state with subId: " + subId);
                            resetPcoValue("ACTION_RESET_SELFACTIVATION");
                            sendEvent(SaEvents.EVENT_DBG_RESET);
                        }
                        break;
                    case DEBUG_ACTION_DUMP:
                        Rlogd(mSaSmInstance.toString());
                        Toast.makeText(context,
                                mStateMachine.getCurrentState().getName(),
                                Toast.LENGTH_SHORT).show();
                        break;
                    case DEBUG_ACTION_RADIO_CTRL:
                        int ctrl = intent.getIntExtra(KEY_DBG_PARAMETER, 0);
                        mEnableRadioControl = (ctrl == 1) ? true : false;
                        Rlogd("Enable radio control: " + mEnableRadioControl);
                        syncRadioControlToMd(mEnableRadioControl, "debug");
                        break;
                    case DEBUG_ACTION_PCO_EVENT:
                        int pco = intent.getIntExtra(KEY_DBG_PARAMETER, PCO_VALUE_INVALID);
                        byte[] content = new byte[1];
                        content[0] = (byte) pco;
                        PcoData pcoData = new PcoData(0, "IPV4", PCO_ID_FF00H, content);
                        AsyncResult ar = new AsyncResult(null, (Object) pcoData, null);
                        Message msg = mStateMachine.getHandler().obtainMessage(SaEvents.EVENT_PCO_STATE);
                        msg.obj = (Object) ar;
                        Rlogd("DEBUG_ACTION_PCO_EVENT with PCO FF00H: " + pco);
                        mStateMachine.getHandler().sendMessage(msg);
                        break;
                    default:
                        Rloge("Unknow debug action: " + debugAction);
                        break;
                }
            }
        }
    }

    private void updateSelfActivateState(int pcoValue) {
        switch (pcoValue) {
            case PCO_VALUE_SELF_ACT:
                mSelfActivateState.setState(ISelfActivation.STATE_NOT_ACTIVATED);
                break;
            case PCO_VALUE_DEFAULT:
                mSelfActivateState.setState(ISelfActivation.STATE_NONE);
                break;
            // TODO: How to identify STATE_ACTIVATED ?
            default:
                mSelfActivateState.setState(ISelfActivation.STATE_UNKNOWN);
                Rloge("Unknown PCO value: " + pcoValue);
                break;
        }
        Rlogd("updateSelfActivateState: pcoValue = " + pcoValue +
                ", state = " + mSelfActivateState);

    }

    private void updatePCO520State(boolean flag) {
        if(flag)
            m520State.setState(ISelfActivation.STATE_520_ACTIVATED);
        else
            m520State.setState(ISelfActivation.STATE_520_NONE);
        Rlogd("updatePCO520State: flag = " + flag +
                ", state = " + m520State);

    }
    private int getPcoValue() {
        return SaOp12PersistDataHelper.getIntData(mContext, mPhoneId,
                SaOp12PersistDataHelper.DATA_KEY_PCO, PCO_VALUE_DEFAULT);
    }

    private void updatePcoValue(int value) {
        SaOp12PersistDataHelper.putIntData(mContext, mPhoneId,
                SaOp12PersistDataHelper.DATA_KEY_PCO, value);
    }

    private void resetPcoValue(String cause) {
        Rlogd("resetPcoValue cause: " + cause);
        SaOp12PersistDataHelper.putIntData(mContext, mPhoneId,
                SaOp12PersistDataHelper.DATA_KEY_PCO, PCO_VALUE_DEFAULT);
        updateSelfActivateState(PCO_VALUE_DEFAULT);
    }

    private int setRadioPower(boolean power, int reason) {
        int retVal = RadioManager.SUCCESS;
        retVal = getRadioManager().setRadioPower(power, mPhoneId, reason);
        Rlogd("setRadioPower(" + power + "), ret = " + retVal);
        return retVal;
    }

    @Override
    public String toString() {
        StringBuilder s = new StringBuilder();
        s.append("SelfActivationPcoBasedSm{");
        s.append(" mPhoneId = " + mPhoneId);
        s.append(", mSubId = " + mSubId);
        s.append(", curState = " + mStateMachine.getCurrentState().getName());
        s.append(", mRadioEnabledCause = " + mRadioEnabledCause);
        s.append(", PersistDataHelper = " + SaOp12PersistDataHelper.toString(mContext, mPhoneId));
        s.append(", mCallState = " + mCallState);
        s.append(", mSelfActivateState = " + mSelfActivateState);
        s.append(", m520State = " + m520State);
        s.append("}");

        return s.toString();
    }

    private void disableRadioIfNeed() {
        int currPcoValue = getPcoValue();
        boolean isSelfActPco = (PCO_VALUE_SELF_ACT == currPcoValue) ? true : false;
        boolean isOp12BrandedDevice = isOp12BrandedDevice();
        Rlogd("disableRadioIfNeed: isSelfActPco = " + isSelfActPco +
                ", getCallRefCount = " + mCallState.getCallRefCount() +
                ", isStartCallEndTimer = " + isStartCallEndTimer() +
                ", " + mRadioEnabledCause +
                ", mEnableRadioControl = " + mEnableRadioControl);
        if (isSelfActPco &&
                !isStartCallEndTimer() && // not disable radio during the timer
                (mCallState.getCallRefCount() == 0) && // Not disable radio during the call
                mEnableRadioControl &&
                !mRadioEnabledCause.hasCause(SaRadioEnabledCause.CAUSE_ADD_DATA_SERVICE) &&
                !mRadioEnabledCause.hasCause(SaRadioEnabledCause.CAUSE_NORMAL_CALL) &&
                !mRadioEnabledCause.hasCause(SaRadioEnabledCause.CAUSE_EMERGENCY_CALL)) {
            // M: ALPS03224621 workaround patch to pass script issue
            sendEventDelayed(SaEvents.EVENT_DISABLE_RADIO_PCO, PCO_DISABLE_RADIO_DELAY_MS);
            Rlogd("disableRadioIfNeed: true");
        }
    }

    private boolean isStartCallEndTimer() {
        if (mStateMachine.getHandler().hasMessages(SaEvents.EVENT_TIMER_RADIO_OFF)) {
            return true;
        }
        return false;
    }

    private void disableRadioCallEndTimer() {
        int currPcoValue = getPcoValue();
        boolean isSelfActPco = (PCO_VALUE_SELF_ACT == currPcoValue) ? true : false;
        Rlogd("disableRadioCallEndTimer: isSelfActPco = " + isSelfActPco +
                ", mRadioEnabledCause = " + mRadioEnabledCause +
                ", mEnableRadioControl = " + mEnableRadioControl);
        if (isSelfActPco &&
                mEnableRadioControl &&
                !mRadioEnabledCause.hasCause(SaRadioEnabledCause.CAUSE_ADD_DATA_SERVICE)) {
            sendEvent(SaEvents.EVENT_DISABLE_RADIO_PCO);
            Rlogd("disableRadioCallEndTimer: true");
        }
    }

    protected void Rloge(String s) {
        Rlog.e(TAG + "[" + mPhoneId + "]", s);
    }

    protected void Rlogd(String s) {
        if (DBG) {
            Rlog.d(TAG + "[" + mPhoneId + "]", s);
        }
    }

    private void sendEvent(int event, int arg1) {
        Rlogd("sendEvent: " + SaEvents.print(event));
        Message msg = Message.obtain(mStateMachine.getHandler(), event);
        msg.arg1 = arg1;
        mStateMachine.getHandler().sendMessage(msg);
    }

    private void sendEvent(int event) {
        Rlogd("sendEvent: " + SaEvents.print(event));
        Message msg = Message.obtain(mStateMachine.getHandler(), event);
        mStateMachine.getHandler().sendMessage(msg);
    }

    private void sendEventDelayed(int event, long delayMillis) {
        Rlogd("sendEventDelayed: " + SaEvents.print(event) + " with delay: " + delayMillis);
        Message msg = Message.obtain(mStateMachine.getHandler(), event);
        mStateMachine.getHandler().sendMessageDelayed(msg, delayMillis);
    }
    private void syncRadioControlToMd(boolean enableRadioControl, String cause) {
        Rlogd("syncRadioControlToMd() enableRadioControl: " +
                enableRadioControl + " cause: " + cause);
        String[] cmd = new String[2];
        int enable = (enableRadioControl == true)? 2 : 1;
        cmd[0] = "AT+ESELFACT=\"radioctrl\"" +
                ",\"" + enable + "\"";
        cmd[1] = "";
        Message result = mStateMachine.getHandler().obtainMessage(
                SaEvents.EVENT_SYNC_RADIO_CTRL_DONE, enable);
        ((MtkRIL) mCi).invokeOemRilRequestStrings(cmd, result);
    }

    /* Class to maintain the call state.
     * Cancel the timer when a call start, and start timer when the call end.
     *
     * Note S: call start / E: call end
     * Case 1: E1E2 => E1 no action, E2 no action.
     * Case 2. S1E1 => S1 cancel timer, E1 start timer.
     * Case 3. S1E1E2 => S1 cancel timer, E1 start timer, E2 no action.
     * Case 4. S1E1S2E2 => S1 cancel timer, E1 start timer, S2 cancel timer, E2 start timer.
     * Case 5: S1S2E1E2E3 => S1 cancel timer, S2 no action,
     *         E1 no action, E2 cancel timer, E3 no action.
     */
    class SaCallState {
        private int mPhoneId = 0;
        private int mCallRefCount = 0;

        SaCallState(int phoneId) {
            mPhoneId = phoneId;
        }

        int getCallRefCount() {
            return mCallRefCount;
        }

        // Return true if the fisrt call
        boolean incCallRefCount() {
            Rlogd("incCallRefCount: " + mCallRefCount + " > " + (mCallRefCount + 1));
            mCallRefCount++;
            return (mCallRefCount == 1) ? true : false;
        }

        // Return true if the last call end
        boolean decCallRefCount() {
            if (mCallRefCount > 0) {
                Rlogd("decCallRefCount: " + mCallRefCount + " > " + (mCallRefCount - 1));
                mCallRefCount--;
                if (mCallRefCount == 0) {
                    return true;
                }
            }
            return false;
        }

        void resetCallRefCount() {
            mCallRefCount = 0;
        }

        @Override
        public String toString() {
            return "SaCallState{ mPhoneId = " + mPhoneId +
                    " mCallRefCount = " + mCallRefCount + "}";
        }
    }

    class SelfActivateState {
        private int mPhoneId;
        private int mState;

        public SelfActivateState(int phoneId) {
            mPhoneId = phoneId;
            int saved = SaOp12PersistDataHelper.getIntData(mContext, mPhoneId,
                    SaOp12PersistDataHelper.DATA_KEY_SA_STATE, ISelfActivation.STATE_UNKNOWN);
            if (saved != ISelfActivation.STATE_UNKNOWN) {
                mState = saved;
            } else {
                setState(ISelfActivation.STATE_NONE);
            }
        }

        void setState(int state) {
            mState = state;
            SaOp12PersistDataHelper.putIntData(mContext, mPhoneId,
                    SaOp12PersistDataHelper.DATA_KEY_SA_STATE, mState);
            Rlogd("setState: " + print(mState));
        }

        int getState() {
            return mState;
        }

        String print(int state) {
            String outString = "";
            switch (state) {
                case ISelfActivation.STATE_NONE: outString = "STATE_NONE"; break;
                case ISelfActivation.STATE_NOT_ACTIVATED: outString = "STATE_NOT_ACTIVATED"; break;
                case ISelfActivation.STATE_ACTIVATED: outString = "STATE_ACTIVATED"; break;
                case ISelfActivation.STATE_UNKNOWN:
                default:
                    outString = "STATE_UNKNOWN"; break;
            }
            return outString;
        }

        @Override
        public String toString() {
            return "SelfActivateState{ mPhoneId = " + mPhoneId +
                    " mState = " + print(mState) +
                    " mEnableRadioControl = " + mEnableRadioControl + "}";
        }
    }

    class PCO520State {
        private int mPhoneId;
        private int m520State;

        public PCO520State(int phoneId) {
            mPhoneId = phoneId;
            int saved = SaOp12PersistDataHelper.getIntData(mContext, mPhoneId,
                    SaOp12PersistDataHelper.DATA_KEY_SA_STATE, ISelfActivation.STATE_520_UNKNOWN);
            if (saved != ISelfActivation.STATE_520_UNKNOWN) {
                m520State = saved;
            } else {
                setState(ISelfActivation.STATE_520_NONE);
            }
        }

        void setState(int state) {
            m520State = state;
            SaOp12PersistDataHelper.putIntData(mContext, mPhoneId,
                    SaOp12PersistDataHelper.DATA_KEY_SA_STATE, m520State);
            Rlogd("set520State: " + print(m520State));
        }

        int getState() {
            return m520State;
        }

        String print(int state) {
            String outString = "";
            switch (state) {
                case ISelfActivation.STATE_520_NONE: outString = "520_NONE"; break;
                case ISelfActivation.STATE_520_ACTIVATED: outString = "520_ACTIVATED"; break;
                case ISelfActivation.STATE_520_UNKNOWN:
                default:
                    outString = "520_UNKNOWN"; break;
            }
            return outString;
        }

        @Override
        public String toString() {
            return "SelfActivateState{ mPhoneId = " + mPhoneId +
                    " m520State = " + print(m520State) +
                    " mEnableRadioControl = " + mEnableRadioControl + "}";
        }
    }

    // Event codes for driving the state machine
    static class SaEvents {
        // External events
        static final int EVENT_EXTERNAL_START = 0;
        static final int EVENT_RM_RADIO_CHANGED = EVENT_EXTERNAL_START;
        static final int EVENT_RM_RADIO_OFF = EVENT_EXTERNAL_START + 1;
        static final int EVENT_RM_RADIO_ON = EVENT_EXTERNAL_START + 2;
        static final int EVENT_RM_RADIO_AVAILABLE = EVENT_EXTERNAL_START + 3;
        static final int EVENT_SELF_ACT = EVENT_EXTERNAL_START + 4;
        static final int EVENT_PCO_STATE = EVENT_EXTERNAL_START + 5;
        static final int EVENT_CALL_STATE_START = EVENT_EXTERNAL_START + 7;
        static final int EVENT_CALL_STATE_IDLE = EVENT_EXTERNAL_START + 8;
        static final int EVENT_SIM_STATE_CHANGED = EVENT_EXTERNAL_START + 9;
        static final int EVENT_NETWORK_REJECT = EVENT_EXTERNAL_START + 10;

        // Internal events
        static final int EVENT_INTERNAL_START = EVENT_EXTERNAL_START + 100;
        static final int EVENT_DISABLE_RADIO_PCO = EVENT_INTERNAL_START + 1;
        static final int EVENT_TIMER_RADIO_OFF = EVENT_INTERNAL_START + 2;
        static final int EVENT_SYNC_RADIO_CTRL_DONE = EVENT_INTERNAL_START + 3;
        // Debug usage
        static final int EVENT_DBG_RESET = EVENT_INTERNAL_START + 4;

        public static String print(int eventCode) {
            String outString = "";
            switch (eventCode) {
                case EVENT_RM_RADIO_CHANGED: outString = "EVENT_RM_RADIO_CHANGED"; break;
                case EVENT_RM_RADIO_OFF: outString = "EVENT_RM_RADIO_OFF"; break;
                case EVENT_RM_RADIO_ON: outString = "EVENT_RM_RADIO_ON"; break;
                case EVENT_RM_RADIO_AVAILABLE: outString = "EVENT_RM_RADIO_AVAILABLE"; break;
                case EVENT_SELF_ACT: outString = "EVENT_SELF_ACT"; break;
                case EVENT_PCO_STATE: outString = "EVENT_PCO_STATE"; break;
                case EVENT_CALL_STATE_START: outString = "EVENT_CALL_STATE_START"; break;
                case EVENT_CALL_STATE_IDLE: outString = "EVENT_CALL_STATE_IDLE"; break;
                case EVENT_SIM_STATE_CHANGED: outString = "EVENT_SIM_STATE_CHANGED"; break;
                case EVENT_DISABLE_RADIO_PCO: outString = "EVENT_DISABLE_RADIO_PCO"; break;
                case EVENT_TIMER_RADIO_OFF: outString = "EVENT_TIMER_RADIO_OFF"; break;
                case EVENT_SYNC_RADIO_CTRL_DONE: outString = "EVENT_SYNC_RADIO_CTRL_DONE"; break;
                case EVENT_DBG_RESET: outString = "EVENT_DBG_RESET"; break;
                case EVENT_NETWORK_REJECT: outString = "EVENT_NETWORK_REJECT"; break;
                default:
                    throw new IllegalArgumentException("Invalid eventCode: " + eventCode);
            }
            return outString;
        }
    }

    public static boolean isOp12BrandedDevice() {
        return (OP12_BRANDED_DEVICE.equalsIgnoreCase(
                SystemProperties.get(PROPERTY_OP12_DEVICE_MODEL))) ? true : false;
    }
}

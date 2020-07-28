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

package com.mediatek.internal.telephony.dataconnection;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.AccessNetworkConstants;
import android.telephony.CarrierConfigManager;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.OnSubscriptionsChangedListener;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneSwitcher;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyDevController;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.IccRecords;

import com.mediatek.internal.telephony.datasub.SmartDataSwitchAssistant;
import com.mediatek.internal.telephony.imsphone.MtkImsPhoneCallTracker;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkGsmCdmaCallTracker;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkHardwareConfig;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkSubscriptionController;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkPhoneSwitcher;
import com.mediatek.internal.telephony.MtkSubscriptionController;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.MtkRIL;

import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class MtkDcHelper extends Handler {
    private static final String LOG_TAG = "DcHelper";
    private static final boolean DBG = true;
    private static final boolean VDBG = SystemProperties.get("ro.build.type").
            equals("eng") ? true : false; // STOPSHIP if true

    // M: PS/CS concurrent
    private static MtkDcHelper sMtkDcHelper = null;
    protected Phone[] mPhones;
    protected int mPhoneNum;
    private Context mContext;

    // M: Query modem hardware capability
    private TelephonyDevController mTelDevController = TelephonyDevController.getInstance();

    // Multi-PS Attach
    private static final String PROP_DATA_CONFIG = "ro.vendor.mtk_data_config";
    private static final int DATA_CONFIG_MULTI_PS = 0x1;
    private boolean mHasFetchMpsAttachSupport = false;
    private boolean mMpsAttachSupport = false;

    // event id must be multiple of EVENT_ID_INTVL
    private static final int EVENT_ID_INTVL = 10;
    private final static int EVENT_SUBSCRIPTION_CHANGED    = EVENT_ID_INTVL * 0;
    private static final int EVENT_RIL_CONNECTED = EVENT_ID_INTVL * 1;
    // M: PS/CS concurrent @{
    private static final int EVENT_VOICE_CALL_STARTED = EVENT_ID_INTVL * 2;
    private static final int EVENT_VOICE_CALL_ENDED = EVENT_ID_INTVL * 3;
    private static final int EVENT_NO_CS_CALL_AFTER_SRVCC = EVENT_ID_INTVL * 4;
    // M: PS/CS concurrent @}

    private static final int EVENT_DSDA_STATE_CHANGED = EVENT_ID_INTVL * 5;
    private static final int EVENT_VOICE_CALL_OFFHOOK = EVENT_ID_INTVL * 6;
    private static final int EVENT_CALL_ADDITIONAL_INFO = EVENT_ID_INTVL * 7;

    // M: GwsdDualSim @{
    private static final int MT_CALL_REJECTED = 1;
    private static final int MT_CALL_MISSED = 2;
    private static final int MT_CALL_NUMREDIRECT = 3;
    private static final int MT_CALL_RQ = 4;
    // M: GwsdDualSim @}

    // M: PS/CS concurrent @{
    private int mCallingPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
    // M: svlte denali can support two phone calls.
    private int mPrevCallingPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
    private static final String PROP_MTK_CDMA_LTE_MODE = "ro.boot.opt_c2k_lte_mode";
    public static final boolean MTK_SVLTE_SUPPORT = (SystemProperties.getInt(
            PROP_MTK_CDMA_LTE_MODE, 0) == 1);
    public static final boolean MTK_SRLTE_SUPPORT = (SystemProperties.getInt(
            PROP_MTK_CDMA_LTE_MODE, 0) == 2);
    // M: PS/CS concurrent @}

    private int mDsdaMode = 0;

    // M: To get ICCID info.
    static private String[] PROPERTY_ICCID_SIM = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4",
    };

    private static final String[] PROPERTY_RIL_TEST_SIM = {
        "vendor.gsm.sim.ril.testsim",
        "vendor.gsm.sim.ril.testsim.2",
        "vendor.gsm.sim.ril.testsim.3",
        "vendor.gsm.sim.ril.testsim.4",
    };

    private static final String[]  PROPERTY_RIL_FULL_UICC_TYPE = {
        "vendor.gsm.ril.fulluicctype",
        "vendor.gsm.ril.fulluicctype.2",
        "vendor.gsm.ril.fulluicctype.3",
        "vendor.gsm.ril.fulluicctype.4",
    };

    private static final String INVALID_ICCID = "N/A";

    private Call.SrvccState mSrvccState = Call.SrvccState.NONE;

    // M: Data on domestic roaming. @{
    public enum Operator {
        OP129, //KDDI
        OP156, // Telenor
    }

    private static final Map<Operator, List> mOperatorMap = new HashMap<Operator, List>() {
        {
            put(Operator.OP129, Arrays.asList("44007", "44008", "44050", "44051", "44052",
                    "44053", "44054", "44055", "44056", "44070",
                    "44071", "44072", "44073", "44074", "44075",
                    "44076", "44077", "44078", "44079", "44088",
                    "44089", "44170"));
            put(Operator.OP156, Arrays.asList("23802", "23877"));
        }
    };
    // M: Data on domestic roaming. @}

    /// M: CDMA dual activation.
    private static final String RIL_CDMA_DUALACT_SUPPORT = "vendor.ril.cdma.3g.dualact";

    // M: GwsdDualSim @{
    private boolean mGwsdDualSimStatus = false;
    private boolean mIsPhoneOffhook = false;
    protected final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                return;
            }
            if (action.equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                String phoneState = intent.getStringExtra(TelephonyManager.EXTRA_STATE);
                if (phoneState.equals(TelephonyManager.EXTRA_STATE_IDLE)) {
                    logd("onPhoneStateChanged: phone is IDLE");
                    mIsPhoneOffhook = false;
                }
                // M: no need to handle when EXTRA_STATE_RINGING
                else if (phoneState.equals(TelephonyManager.EXTRA_STATE_OFFHOOK)) {
                    logd("onPhoneStateChanged: phone is OFFHOOK");
                    Message msg = mRspHandler.obtainMessage(EVENT_VOICE_CALL_OFFHOOK);
                    msg.sendToTarget();
                }
            } else {
                // unknown action
            }
        }
    };
    // M: GwsdDualSim @}

    protected MtkDcHelper(Context context, Phone[] phones) {
        mContext = context;
        mPhones = phones;
        mPhoneNum = phones.length;
        registerEvents();
    }

    public void dispose() {
        logd("MtkDcHelper.dispose");
        unregisterEvents();
    }

    public static MtkDcHelper makeMtkDcHelper(Context context, Phone[] phones) {

        if (context == null || phones == null) {
            throw new RuntimeException("param is null");
        }

        if (sMtkDcHelper == null) {
            logd("makeMtkDcHelper: phones.length=" + phones.length);
            sMtkDcHelper = new MtkDcHelper(context, phones);
        }

        logd("makesMtkDcHelper: X sMtkDcHelper =" + sMtkDcHelper);
        return sMtkDcHelper;
    }

    private Handler mRspHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            // msg_id = n * EVENT_ID_INTVL + phone_id, use mod operator to get phone_id
            // event_id must be multiple of EVENT_ID_INTVL => n * EVENT_ID_INTVL
            int phoneId = msg.what % EVENT_ID_INTVL;
            int eventId = msg.what - phoneId;
            int restCallingPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
            AsyncResult ar;
            switch (eventId) {
                case EVENT_RIL_CONNECTED:
                    logd("EVENT_PHONE" + phoneId + "_EVENT_RIL_CONNECTED");
                    onCheckIfRetriggerDataAllowed(phoneId);
                    break;
                // M: PS/CS concurrent @{
                case EVENT_VOICE_CALL_STARTED:
                    if (!isInSRVCC()) {
                        if (MTK_SVLTE_SUPPORT &&
                            mCallingPhoneId != SubscriptionManager.INVALID_PHONE_INDEX) {
                            mPrevCallingPhoneId = mCallingPhoneId;
                            logd("SVLTE Voice Call2 Started, save first mPrevCallingPhoneId = "
                                    + mPrevCallingPhoneId);
                        }
                        mCallingPhoneId = phoneId;
                        logd("Voice Call Started, mCallingPhoneId = " + mCallingPhoneId);
                        onVoiceCallStarted();
                    }
                    break;
                case EVENT_NO_CS_CALL_AFTER_SRVCC:
                    logd("Got 'no CS calls after SRVCC' notification, tunnel it to VOICE_CALL_END");
                    mSrvccState = Call.SrvccState.NONE;
                    // intentionally fall-througth
                case EVENT_VOICE_CALL_ENDED:
                    mIsPhoneOffhook = false;
                    Call.SrvccState preSrvccState = mSrvccState;
                    ar = (AsyncResult) msg.obj;
                    if (ar  != null && ar.result != null) {
                        mSrvccState = (Call.SrvccState) ar.result;
                    } else {
                        mSrvccState = Call.SrvccState.NONE;
                    }
                    logd("mSrvccState = " + mSrvccState);
                    if (!isInSRVCC() || (preSrvccState == Call.SrvccState.NONE)) {
                        logd("Voice Call Ended, mCallingPhoneId = " + mCallingPhoneId);
                        if (MTK_SVLTE_SUPPORT &&
                            mPrevCallingPhoneId != SubscriptionManager.INVALID_PHONE_INDEX) {
                            if (phoneId == mCallingPhoneId) {
                                restCallingPhoneId = mPrevCallingPhoneId;
                            } else {
                                restCallingPhoneId = mCallingPhoneId;
                            }
                            // record the left one call.
                            mCallingPhoneId = restCallingPhoneId;
                            mPrevCallingPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
                            logd("SVLTE Voice Call2 Ended, mCallingPhoneId = " + mCallingPhoneId);
                        }
                        onVoiceCallEnded();
                        mCallingPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
                        if (MTK_SVLTE_SUPPORT &&
                            restCallingPhoneId != SubscriptionManager.INVALID_PHONE_INDEX) {
                            mCallingPhoneId = restCallingPhoneId;
                            logd("SVLTE Voice Call Ended, restore first mCallingPhoneId = "
                                    + mCallingPhoneId);
                        }
                    }
                    break;
                // M: PS/CS concurrent @}
                case EVENT_DSDA_STATE_CHANGED:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.result != null) {
                        mDsdaMode = (int) ar.result;
                        logd("mDsdaMode = " + mDsdaMode);
                        if (mCallingPhoneId != SubscriptionManager.INVALID_PHONE_INDEX) {
                            for (int i = 0; i < mPhoneNum; i++) {
                                ((MtkDcTracker) mPhones[i].getDcTracker(
                                    AccessNetworkConstants.TRANSPORT_TYPE_WWAN))
                                    .onDsdaStateChanged();
                            }
                        }
                        SmartDataSwitchAssistant mSmartDataSwitchAssistant
                                = SmartDataSwitchAssistant.getInstance();
                        if (mSmartDataSwitchAssistant != null) {
                            mSmartDataSwitchAssistant.onDsdaStateChanged();
                        }
                    }
                    break;
                case EVENT_CALL_ADDITIONAL_INFO:
                    ar = (AsyncResult) msg.obj;
                    String[] callAdditionalInfo = (String[]) ar.result;
                    int type = Integer.parseInt(callAdditionalInfo[0]);
                    if (type == MT_CALL_RQ) {
                        int mtCallRq = Integer.parseInt(callAdditionalInfo[1]);
                        mGwsdDualSimStatus = (mtCallRq == 1) ? true : false;
                        logd("MT_CALL_RQ, mtCallRq = " + mtCallRq + ", mGwsdDualSimStatus = "
                                + mGwsdDualSimStatus);
                    }
                    break;
                case EVENT_VOICE_CALL_OFFHOOK:
                    if (mCallingPhoneId != SubscriptionManager.INVALID_PHONE_INDEX) {
                        mIsPhoneOffhook = true;
                        logd("Voice Call OffHook, re-evaluate call start");
                        onVoiceCallStarted();
                    }
                    break;
                default:
                    logd("Unhandled message with number: " + msg.what);
                    break;
            }
        }
    };

    public static MtkDcHelper getInstance() {
        if (sMtkDcHelper == null) {
            throw new RuntimeException("Should not be called before makesMtkDcHelper");
        }
        return sMtkDcHelper;
    }

    // M: Retrigger data allow to phone switcher
    private void onCheckIfRetriggerDataAllowed(int phoneId) {
        logd("onCheckIfRetriggerDataAllowed: retriggerDataAllowed: mPhone[" + phoneId +"]");
        if (MtkPhoneSwitcher.getInstance() != null) {
            MtkPhoneSwitcher.getInstance().onRadioCapChanged(phoneId);
        }
    }

    // PS Attach enhancement End

    public boolean isOperatorMccMnc(Operator opt, int phoneId) {
        String mccMnc = TelephonyManager.getDefault().getSimOperatorNumericForPhone(phoneId);
        boolean bMatched = mOperatorMap.get(opt).contains(mccMnc);

        logd("isOperatorMccMnc: mccmnc=" + mccMnc
                 + ", bMatched=" + bMatched);

        return bMatched;
    }

    // M: PS/CS concurrent feature start
    private void registerEvents() {
        logd("registerEvents");
        for (int i = 0; i < mPhoneNum; i++) {
            // Register event for radio capability change
            mPhones[i].mCi.registerForRilConnected(mRspHandler,
                    EVENT_RIL_CONNECTED + i, null);
            // M: PS/CS concurrent @{
            // Register events for GSM/CDMA call state.
            mPhones[i].getCallTracker().registerForVoiceCallStarted (mRspHandler,
                    EVENT_VOICE_CALL_STARTED + i, null);
            mPhones[i].getCallTracker().registerForVoiceCallEnded (mRspHandler,
                    EVENT_VOICE_CALL_ENDED + i, null);
            // M: PS/CS concurrent @}
            ((MtkRIL) mPhones[i].mCi).registerForDsdaStateChanged(mRspHandler,
                    EVENT_DSDA_STATE_CHANGED + i, null);
            // Register for call information for GwsdDualSim Feature.
            ((MtkRIL) mPhones[i].mCi).registerForCallAdditionalInfo(mRspHandler,
                    EVENT_CALL_ADDITIONAL_INFO + i, null);
        }

        /// M: CDMA dual activation, register phone change event. @{
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED);
        mContext.registerReceiver(mPhoneSwitchReceiver, filter);

        filter = new IntentFilter();
        filter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        mContext.registerReceiver(mBroadcastReceiver, filter);
        logd("registered phone change event.");
        /// @}
    }

    // M: PS/CS concurrent @{
    public void registerImsEvents(int phoneId) {
        if (!SubscriptionManager.isValidPhoneId(phoneId)) {
            logd("registerImsEvents, invalid phoneId");
            return;
        }

        logd("registerImsEvents, phoneId = " + phoneId);
        Phone imsPhone = mPhones[phoneId].getImsPhone();
        // Register events for IMS call state.
        // In multi PS project,  it allows VoLTE call in SIM1 while data on SIM2.
        // Therefore, it needs to handle the call state of IMS phone as well.
        // Only register events if IMS phone not null in the case of IMS service down.
        if (imsPhone != null) {
            MtkImsPhoneCallTracker imsCt = (MtkImsPhoneCallTracker) imsPhone.getCallTracker();
            imsCt.registerForVoiceCallStarted(mRspHandler,
                    EVENT_VOICE_CALL_STARTED + phoneId, null);
            imsCt.registerForVoiceCallEnded(mRspHandler,
                    EVENT_VOICE_CALL_ENDED + phoneId, null);
            imsCt.registerForCallsDisconnectedDuringSrvcc(mRspHandler,
                    EVENT_NO_CS_CALL_AFTER_SRVCC + phoneId, null);
        } else {
            logd("Not register IMS phone calling state yet.");
        }
    }
    // M: PS/CS concurrent @}

    private void unregisterEvents() {
        logd("unregisterEvents");
        // M: PS/CS concurrent @{
        for (int i = 0; i < mPhoneNum; i++) {
            mPhones[i].getCallTracker().unregisterForVoiceCallStarted(mRspHandler);
            mPhones[i].getCallTracker().unregisterForVoiceCallEnded(mRspHandler);
            ((MtkRIL) mPhones[i].mCi).unregisterForDsdaStateChanged(mRspHandler);
        }
        // M: PS/CS concurrent @}
    }

    // M: PS/CS concurrent @{
    public void unregisterImsEvents(int phoneId) {
        if (!SubscriptionManager.isValidPhoneId(phoneId)) {
            logd("unregisterImsEvents, invalid phoneId");
            return;
        }

        // Unregister events for IMS call state.
        logd("unregisterImsEvents, phoneId = " + phoneId);
        Phone imsPhone = mPhones[phoneId].getImsPhone();
        if (imsPhone != null) {
            MtkImsPhoneCallTracker imsCt = (MtkImsPhoneCallTracker) imsPhone.getCallTracker();
            imsCt.unregisterForVoiceCallStarted(mRspHandler);
            imsCt.unregisterForVoiceCallEnded(mRspHandler);
            imsCt.unregisterForCallsDisconnectedDuringSrvcc(mRspHandler);
        } else {
            logd("Not unregister IMS phone calling state yet.");
        }
    }

    private void onVoiceCallStarted() {
        for (int i = 0; i < mPhoneNum; i++) {
            logd("onVoiceCallStarted: mPhone[ " + i +"]");
            ((MtkDcTracker) mPhones[i].getDcTracker(AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).
                 onVoiceCallStartedEx();
        }
    }

    private void onVoiceCallEnded() {
        for (int i = 0; i < mPhoneNum; i++) {
            logd("onVoiceCallEnded: mPhone[ " + i +"]");
            ((MtkDcTracker) mPhones[i].getDcTracker(AccessNetworkConstants.TRANSPORT_TYPE_WWAN)).
                 onVoiceCallEndedEx();
        }
    }

    public boolean isDataSupportConcurrent(int phoneId) {
        ArrayList<Integer> callingPhoneIdList = new ArrayList<Integer>();
        for (int i = 0; i < mPhoneNum; i++) {
            if (mPhones[i].getState() != PhoneConstants.State.IDLE) {
                callingPhoneIdList.add(i);
            }
        }
        if (callingPhoneIdList.size() == 0) {
            logd("isDataSupportConcurrent: no calling phone!");
            return true;
        }
        if (MTK_SVLTE_SUPPORT && callingPhoneIdList.size() > 1) {
            logd("isDataSupportConcurrent: SVLTE and >1 calling phone.");
            return mPhones[phoneId].getServiceStateTracker().isConcurrentVoiceAndDataAllowed();
        }

        // voice/data on the same phone
        if (phoneId == callingPhoneIdList.get(0)) {
            boolean isConcurrent = false;
            boolean inSrvcc = false;
            MtkGsmCdmaCallTracker ct = (MtkGsmCdmaCallTracker) mPhones[phoneId].getCallTracker();
            Phone imsPhone = mPhones[phoneId].getImsPhone();

            boolean inPsEcc = (imsPhone == null) ?
                    false : imsPhone.isInEmergencyCall();

            PhoneConstants.State csCallState = PhoneConstants.State.IDLE;
            if (ct != null) {
                inSrvcc = ct.getHandoverConnectionSize() != 0;
                csCallState = ct.getState();
            }

            if (inPsEcc || inSrvcc
                    || (csCallState != PhoneConstants.State.IDLE)) {
                isConcurrent = mPhones[phoneId].getServiceStateTracker().
                        isConcurrentVoiceAndDataAllowed();
                boolean mShouldAutoAttach = shouldAutoAttachForCall(phoneId);
                if (mShouldAutoAttach) {
                    isConcurrent = true;
                }
            } else {
                isConcurrent = true;
            }

            logd("isDataSupportConcurrent: (voice/data on the same phone) isConcurrent = "
                    + isConcurrent + ", phoneId = " + phoneId + ", callingPhoneId = "
                    + callingPhoneIdList.get(0) + ", inPsEcc = " + inPsEcc
                    + ", inSrvcc = " + inSrvcc + ", csCallState = " + csCallState);
            return isConcurrent;
        } else {
            // voice/data not on the same phone
            if (mDsdaMode == 1) {
                logd("DSDA mode, support concurrent");
                return true;
            }

            // GwsdDualSim:
            MtkTelephonyManagerEx tmEx = MtkTelephonyManagerEx.getDefault();
            boolean isDataAvailable = false;
            if(tmEx != null) {
                isDataAvailable = tmEx.isDataAvailableForGwsdDualSim(mGwsdDualSimStatus);
                if (isDataAvailable) {
                    logd("isDataAvailable: " + isDataAvailable +
                            ", mGwsdDualSimStatus: " + mGwsdDualSimStatus);
                    return true;
                }
            }

            if (MTK_SRLTE_SUPPORT) {
                //  For SRLTE, return false directly since DSDS.
                logd("isDataSupportConcurrent: support SRLTE ");
                return false;
            } else if (MTK_SVLTE_SUPPORT) {
                //  For SVLTE, need to check more conditions since DSDA.
                int phoneType = mPhones[callingPhoneIdList.get(0)].getPhoneType();

                if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                    // If the calling phone is CDMA type(PS on the other phone is GSM), return true.
                    return true;
                } else {
                    // If the calling phone is GSM type, need to check the other phone's PS Rat.
                    // If the other phone's PS type is CDMA, return true, else, return false.
                    int rilRat = mPhones[phoneId].getServiceState().getRilDataRadioTechnology();
                    logd("isDataSupportConcurrent: support SVLTE RilRat = " + rilRat
                            + "calling phoneType: " + phoneType);

                    return (ServiceState.isCdma(rilRat));
                }
            } else {
                logd("isDataSupportConcurrent: not SRLTE or SVLTE ");
                return false;
            }
        }
    }

    public boolean isAllCallingStateIdle() {
        PhoneConstants.State [] state = new PhoneConstants.State[mPhoneNum];
        boolean allCallingState = false;
        for (int i = 0; i < mPhoneNum; i++) {
            state[i] = mPhones[i].getState();

            if (state[i] != null && state[i] == PhoneConstants.State.IDLE) {
                allCallingState = true;
            } else {
                allCallingState = false;
                break;
            }
        }

        if (!allCallingState && VDBG) {
            // For log reduction, only log if call state not IDLE and not shown in user load.
            for (int i = 0; i < mPhoneNum; i++) {
                logd("isAllCallingStateIdle: state[" + i + "]=" + state[i] +
                        " allCallingState = " + allCallingState);
            }
        }
        return allCallingState;
    }

    public boolean isWifiCallingEnabled() {
        boolean isWifiCallingEnabled = false;
        // in case the calling phone id being changed by other thread
        int callingPhoneId = mCallingPhoneId;
        int callingPhoneId2 = mPrevCallingPhoneId;

        IMtkTelephonyEx telephonyEx = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx"));
        if (telephonyEx != null) {
            try {
                if (SubscriptionManager.isValidPhoneId(callingPhoneId)) {
                    isWifiCallingEnabled = telephonyEx.isWifiCallingEnabled(
                            mPhones[callingPhoneId].getSubId());
                }
                if (MTK_SVLTE_SUPPORT && !isWifiCallingEnabled) {
                    if (SubscriptionManager.isValidPhoneId(callingPhoneId2)) {
                        isWifiCallingEnabled = telephonyEx.isWifiCallingEnabled(
                                mPhones[callingPhoneId2].getSubId());
                    }
                }
            } catch (RemoteException ex) {
                ex.printStackTrace();
            }
        }

        return isWifiCallingEnabled;
    }

    public static boolean isImsOrEmergencyApn(String[] apnTypes) {
        boolean isImsApn = true;
        if (apnTypes == null) {
            loge("isImsOrEmergencyApn: apnTypes is null");
            return false;
        }
        if (apnTypes.length == 0) {
            return false;
        }
        for (String type : apnTypes) {
            if (!PhoneConstants.APN_TYPE_IMS.equals(type) &&
                    !PhoneConstants.APN_TYPE_EMERGENCY.equals(type)) {
                isImsApn = false;
                break;
            }
        }
        return isImsApn;
    }

    /**
    * M: Check if data allowed for concurrent.
    *
    * @param phoneId Identifier of phone object
    * @return boolean True if support concurrent
    */
    public boolean isDataAllowedForConcurrent(int phoneId) {
        if (!SubscriptionManager.isValidPhoneId(phoneId)) {
            logd("isDataAllowedForConcurrent: invalid calling phone id");
            return false;
        }
        if (isAllCallingStateIdle() || isDataSupportConcurrent(phoneId)
                || (isWifiCallingEnabled() && !mPhones[phoneId].isInEmergencyCall())) {
            return true;
        }
        return false;
    }

    public static boolean hasVsimApn(String[] apnTypes) {
        boolean hasVsimApn = false;
        if (apnTypes == null) {
            loge("hasVsimApn: apnTypes is null");
            return false;
        }
        if (apnTypes.length == 0) {
            return false;
        }
        for (String type : apnTypes) {
            if (TextUtils.equals(MtkPhoneConstants.APN_TYPE_VSIM, type)) {
                hasVsimApn = true;
                break;
            }
        }
        return hasVsimApn;
    }
    // M: PS/CS concurrent @}

    public boolean isSimInserted(int phoneId) {
        logd("isSimInserted:phoneId =" + phoneId);
        String iccid = SystemProperties.get(PROPERTY_ICCID_SIM[phoneId], "");
        return !TextUtils.isEmpty(iccid) && !INVALID_ICCID.equals(iccid);
    }

    public boolean isTestIccCard(int phoneId) {
        String testCard = null;

        testCard = SystemProperties.get(PROPERTY_RIL_TEST_SIM[phoneId], "");
        if (VDBG) logd("isTestIccCard: phoneId id = " + phoneId + ", iccType = " + testCard);
        return (testCard != null && testCard.equals("1"));
    }

    /**
    * M: Multi-PS attach support or not.
    * @return boolean true if support Multi-PS attach
    */
    public boolean isMultiPsAttachSupport() {
        if (!mHasFetchMpsAttachSupport) {
            int config = SystemProperties.getInt(PROP_DATA_CONFIG, 0);

            if ((config & DATA_CONFIG_MULTI_PS) == DATA_CONFIG_MULTI_PS) {
                mMpsAttachSupport = true;
            }
            mHasFetchMpsAttachSupport = true;
        }
        return mMpsAttachSupport;
    }

    public boolean hasMdAutoSetupImsCapability() {
        if (mTelDevController != null && mTelDevController.getModem(0) != null
                && ((MtkHardwareConfig) mTelDevController.getModem(0))
                .hasMdAutoSetupImsCapability() == true) {
            logd("hasMdAutoSetupImsCapability: true");
            return true;
        }
        logd("hasMdAutoSetupImsCapability: false");
        return false;
    }

    private boolean isInSRVCC() {
        return (mSrvccState == Call.SrvccState.COMPLETED);
    }

    // For CDMA 3G dual mode card, in some special cases, it is not enable
    // to get available IccRecords and register for SIM records loaded event
    // when icc changed, becuase of card type and phone type are mismatched.

    // For example, SIM1 is GSM card, and SIM2 is CDMA 3G dual mode card.
    // SIM2 will work at CDMA mode. Phone2 type is GSM and will transfer to
    // CDMA later. Before phone type change, ICC change happens and mobile
    // data service can not get available IccRecords of 3GPP type because
    // card type is CDMA. And it has no chance to get IccRecords of 3GPP2
    // type because phone type is GSM. Even phone type changed to CDMA later,
    // mobile data service has no chance to get IccRecords because no more
    // ICC changed event.
    private final BroadcastReceiver mPhoneSwitchReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED.equals(action)) {
                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
                logd("mPhoneSwitchReceiver: phoneId = " + phoneId);
                if (isCdma4GDualModeCard(phoneId) || isCdma3GDualModeCard(phoneId)) {
                    mPhones[phoneId].getDcTracker(AccessNetworkConstants.TRANSPORT_TYPE_WWAN).
                        update();
                }
            }
        }
    };

    /** Check whether support cdma dual activation feature. */
    public static boolean isCdmaDualActivationSupport() {
        return SystemProperties.get(RIL_CDMA_DUALACT_SUPPORT).equals("1");
    }

    public static boolean isCdma4GDualModeCard(int phoneId) {
        if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
            logd("isCdma4GDualModeCard invalid phoneId = " + phoneId);
            return false;
        }

        MtkIccCardConstants.CardType cardType =
                MtkTelephonyManagerEx.getDefault().getCdmaCardType(phoneId);

        return (cardType == MtkIccCardConstants.CardType.CT_4G_UICC_CARD
                || cardType == MtkIccCardConstants.CardType.NOT_CT_UICC_CARD);
    }

    public static boolean isCdma3GDualModeCard(int phoneId) {
        if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
            logd("isCdma3GDualModeCard invalid phoneId = " + phoneId);
            return false;
        }

        MtkIccCardConstants.CardType cardType =
                MtkTelephonyManagerEx.getDefault().getCdmaCardType(phoneId);

        return (cardType == MtkIccCardConstants.CardType.UIM_SIM_CARD
                || cardType == MtkIccCardConstants.CardType.CT_UIM_SIM_CARD);
    }

    public static boolean isCdma3GCard(int phoneId) {
        if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
            logd("isCdma3GCard invalid phoneId = " + phoneId);
            return false;
        }

        MtkIccCardConstants.CardType cardType =
                MtkTelephonyManagerEx.getDefault().getCdmaCardType(phoneId);

        return (cardType == MtkIccCardConstants.CardType.UIM_CARD
                || cardType == MtkIccCardConstants.CardType.CT_3G_UIM_CARD);
    }

    // M: [OD over ePDG] @{
    public static int decodeRat(int param) {
        if (param < 0) {
            return -1;
        }
        return (param / MtkPhoneConstants.RAT_TYPE_KEY) + 1;
    }
    // M: [OD over ePDG] @}

    public boolean isSimMeLockAllowed(int phoneId) {
        if (MtkPhoneSwitcher.getInstance() != null) {
            if (MtkPhoneSwitcher.getInstance().getSimLockMode()) {
                if (!(MtkPhoneSwitcher.getInstance().getPsAllowedByPhoneId(phoneId))) {
                    return false;
                }
            }
        }
        return true;
    }

    private boolean shouldAutoAttachForCall(int phoneId) {
        ServiceState serviceState = mPhones[phoneId].getServiceState();
        PhoneSwitcher phoneSwitcher = PhoneSwitcher.getInstance();
        boolean mAutoAttach = (serviceState != null)
                && (phoneId != phoneSwitcher.getPreferredDataPhoneId())
                && (serviceState.getVoiceNetworkType() == TelephonyManager.NETWORK_TYPE_UMTS);
        if (serviceState != null) {
            logd("shouldAutoAttachForCall=" + mAutoAttach + ", phoneId:" + phoneId +
                ", getVoiceNetworkType=" + serviceState.getVoiceNetworkType());
        }
        return mAutoAttach;
    }

    // M: Return if the device supports Dsda mode now
    public int getDsdaMode() {
        return mDsdaMode;
    }

    protected static void logv(String s) {
        if (DBG) {
            Rlog.v(LOG_TAG, s);
        }
    }

    protected static void logd(String s) {
        if (DBG) {
            Rlog.d(LOG_TAG, s);
        }
    }

    protected static void loge(String s) {
        if (DBG) {
            Rlog.e(LOG_TAG, s);
        }
    }

    protected static void logi(String s) {
        if (DBG) {
            Rlog.i(LOG_TAG, s);
        }
    }
}

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

import static android.telephony.SubscriptionManager.INVALID_PHONE_INDEX;

import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.os.Looper;
import android.os.Handler;
import android.os.Message;
import android.telephony.data.ApnSetting;
import android.telephony.PreciseCallState;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.ITelephonyRegistry;
import com.android.internal.telephony.dataconnection.ApnContext;
import com.android.internal.telephony.dataconnection.DcRequest;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.PhoneSwitcher;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.internal.telephony.dataconnection.MtkDcHelper;
import com.mediatek.internal.telephony.datasub.SmartDataSwitchAssistant;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.telephony.MtkTelephonyManagerEx;


import java.util.List;

public class MtkPhoneSwitcher extends PhoneSwitcher {
    private final static String LOG_TAG = "MtkPhoneSwitcher";
    private final static boolean VDBG = true;
    private static MtkPhoneSwitcher sInstance = null;
    private boolean[] mPhoneStateIsSet;
    private boolean mIsInCall = false;
    private static final int EVENT_SIMLOCK_INFO_CHANGED          = 1000;
    private static final int EVENT_CALL_EVALUATE                 = 1001;

    static final int SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE =
        MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE;

    public MtkPhoneSwitcher(int maxActivePhones, int numPhones, Context context,
            SubscriptionController subscriptionController, Looper looper, ITelephonyRegistry tr,
            CommandsInterface[] cis, Phone[] phones) {
        super(maxActivePhones, numPhones, context, subscriptionController, looper, tr, cis, phones);
        sInstance = this;
        mPhoneStateIsSet = new boolean[numPhones];

        updateHalCommandToUse();
        log("updateHalCommandToUse done");

        for (int i = 0; i < numPhones; i++) {
            mPhoneStateIsSet[i] = false;
        }

        if (MtkTelephonyManagerEx.getDefault() !=null){
            log("getSimLockPolicy:" + MtkTelephonyManagerEx.getDefault().getSimLockPolicy());
        }
    }

    @Override
    public void onRadioCapChanged(int phoneId) {
        if (mHalCommandToUse == HAL_COMMAND_ALLOW_DATA || mHalCommandToUse == HAL_COMMAND_UNKNOWN) {
            // used when the modem may have been rebooted and we want to resend
            // setDataAllowed
            // If PhoneSwitcher is not decided which Phone should be attached, then AP should not
            // resend data allowed flag to Modem.
            log("onRadioCapChanged: mPhoneStateIsSet[" + phoneId + "] =" +
                mPhoneStateIsSet[phoneId]);
            if (mPhoneStateIsSet[phoneId]) {
                super.onRadioCapChanged(phoneId);
            }
        } else {
            log("onRadioCapChanged: preferred data");
            super.onRadioCapChanged(phoneId);
        }
    }

    /**
     * Return if PhoneSwitcher accepts this network request or not.
     * If the apnId is invalid, this request will not be processed.
     *
     * @param request the NetworkRequest to evaluate.
     * @param score the score of this network factory.
     * @return a boolean indicates whether to process this request or not.
     */
    public static boolean acceptRequest(NetworkRequest request, int score) {
        if (ApnContext.getApnTypeFromNetworkRequest(request) == ApnSetting.TYPE_NONE) {
            log("[acceptRequest] Invalid APN ID request: " + request);
            return false;
        }
        return true;
    }

    @Override
    protected NetworkCapabilities makeNetworkFilter() {
        NetworkCapabilities netCap = super.makeNetworkFilter();
        netCap.addCapability(NetworkCapabilities.NET_CAPABILITY_BIP);
        return netCap;
    }

    public static MtkPhoneSwitcher getInstance() {
        return sInstance;
    }

     // This updates mPreferredDataPhoneId which decides which phone should handle default network
     // requests.
    @Override
    protected void updatePreferredDataPhoneId() {
        SmartDataSwitchAssistant mSmartDataSwitchAssistant = SmartDataSwitchAssistant.getInstance();
        Phone voicePhone = findPhoneById(mPhoneIdInVoiceCall);

        // M: Temp data Switch
        if (voicePhone != null && voicePhone.isUserDataEnabled() &&
                mIsInCall == false) {
            log("set mIsInCall to true");
            mIsInCall = true;
            mSmartDataSwitchAssistant.registerReEvaluateEvent(this, EVENT_CALL_EVALUATE,
                    null, mPhoneIdInVoiceCall);
        }
        // M: Temp data Switch
        if (voicePhone == null && mIsInCall == true) {
            log("set mIsInCall to false");
            mIsInCall = false;
            mSmartDataSwitchAssistant.unregisterReEvaluateEvent(this);
        }

        if (mEmergencyOverride != null && findPhoneById(mEmergencyOverride.mPhoneId) != null) {
            // Override DDS for emergency even if user data is not enabled, since it is an
            // emergency.
            // TODO: Provide a notification to the user that metered data is currently being
            // used during this period.
            log("updatePreferredDataPhoneId: preferred data overridden for emergency."
                    + " phoneId = " + mEmergencyOverride.mPhoneId);
            mPreferredDataPhoneId = mEmergencyOverride.mPhoneId;
        } else if (voicePhone != null && voicePhone.getDataEnabledSettings().isDataEnabled(
                ApnSetting.TYPE_DEFAULT)
                // M: Temp data Switch
                && mSmartDataSwitchAssistant.checkIsSwitchAvailable(mPhoneIdInVoiceCall)) {

            // If a phone is in call and user enabled its mobile data, we
            // should switch internet connection to it. Because the other modem
            // will lose data connection anyway.
            // TODO: validate network first.

            mPreferredDataPhoneId = mPhoneIdInVoiceCall;
        } else {
            int subId = getSubIdForDefaultNetworkRequests();
            int phoneId = SubscriptionManager.INVALID_PHONE_INDEX;

            if (SubscriptionManager.isUsableSubIdValue(subId)) {
                for (int i = 0; i < mNumPhones; i++) {
                    if (mPhoneSubscriptions[i] == subId) {
                        phoneId = i;
                        break;
                    }
                }
            }

            mPreferredDataPhoneId = phoneId;
        }
        mPreferredDataSubId = mSubscriptionController.getSubIdUsingPhoneId(mPreferredDataPhoneId);
    }

    @Override
    protected void updateHalCommandToUse() {
        super.updateHalCommandToUse();
        log("updateHalCommandToUse");
        // M: SIM ME Lock, Reveive intent for SIM ME Lock permission check.
        if (mHalCommandToUse == HAL_COMMAND_ALLOW_DATA || mHalCommandToUse == HAL_COMMAND_UNKNOWN) {
            log("updateHalCommandToUse: The HIDL preferred data not exist, use ALLOW_DATA");
            if (getSimLockMode()) {
                mContext.registerReceiver(mSimLockChangedReceiver,
                        new IntentFilter(TelephonyIntents.ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION));
            }
        }
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case EVENT_SIMLOCK_INFO_CHANGED: {
                StringBuilder sb = new StringBuilder("simLockChange");
                for (int i = 0; i < mNumPhones; i++) {
                    int cap = MtkTelephonyManagerEx.getDefault().getShouldServiceCapability(i);
                    sb.append(" phone[").append(i).append("],Capability=").append(cap);
                }
                onEvaluate(REQUESTS_CHANGED, sb.toString());
                break;
            }
            case EVENT_CALL_EVALUATE: {
                log("EVENT_CALL_EVALUATE");
                onEvaluate(REQUESTS_UNCHANGED, "CALL_EVALUATE");
                break;
            }
            default:
                super.handleMessage(msg);
                break;
        }
    }

    @Override
    protected boolean isCallActive(Phone phone) {
        if (phone == null) {
            return false;
        }

        log("ForegroundCall:" + phone.getForegroundCall().getState()+
            ", RingingCall:" + phone.getRingingCall().getState() +
            ", BackgroundCall: " + phone.getBackgroundCall().getState());
        return (phone.getForegroundCall().getState() == Call.State.ACTIVE
                || phone.getForegroundCall().getState() == Call.State.ALERTING
                || phone.getForegroundCall().getState() == Call.State.DIALING
                || phone.getRingingCall().getState().isRinging()
                || phone.getBackgroundCall().getState().isAlive());
    }

    @Override
    protected void deactivate(int phoneId) {
        super.deactivate(phoneId);
        mPhoneStateIsSet[phoneId] = true;
    }

    @Override
    protected void activate(int phoneId) {
        super.activate(phoneId);
        mPhoneStateIsSet[phoneId] = true;
    }

    /// M: ECC w/o SIM  @{
    private boolean isEimsAllowed(NetworkRequest networkRequest) {
        if (networkRequest.networkCapabilities.hasCapability(
                NetworkCapabilities.NET_CAPABILITY_EIMS)) {
            //Check if sim inserted
            for (int i = 0; i < mNumPhones; i++) {
                if (MtkDcHelper.getInstance().isSimInserted(i)) {
                    loge("isAllowEims, sim is not null");
                    return false;
                }
            }
            return true;
        }
        loge("isAllowEims, NetworkRequest not include EIMS capability");
        return false;
    }
    /// @}

    @Override
    protected void suggestDefaultActivePhone(List<Integer> newActivePhones) {
        // Scenario:
        //    1. Default data unset and camp on 4G at SIM1.
        //    2. IMS request and release after at SIM1.
        //    3. 4G been detached and not recover ever.
        //  The reason for that is because default data unset and after detach,
        //  there's no more active phone to attach.
        // Solution:
        //    In the case of no active phone, add one with following condition
        //    1. Get main capability (4G) phone id
        //    2. Add it into activie phone if SIM inserted.
        MtkDcHelper dcHelper = MtkDcHelper.getInstance();
        int mainCapPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        if (newActivePhones.isEmpty()) {
            log("newActivePhones is empty");
            if (dcHelper.isSimInserted(mainCapPhoneId)) {
                // M: For SIM ME lock feature,
                // need to check the valid card before add it to new active phone.
                // Always add new active phones if SMLFeature is off.
                if(!getSimLockMode() || getPsAllowedByPhoneId(mainCapPhoneId)){
                    log("newActivePhones mainCapPhoneId=" + mainCapPhoneId);
                    newActivePhones.add(mainCapPhoneId);
                }
            }
        }

        logv("mPrioritizedDcRequests" + this.mPrioritizedDcRequests.toString());

        /// M: ECC w/o SIM  @{
        if (newActivePhones.isEmpty()) {
            log("ECC w/o SIM");
            for (DcRequest dcRequest : mPrioritizedDcRequests) {
                if (isEimsAllowed(dcRequest.networkRequest) == true) {
                    log("newActivePhones mainCapPhoneId=" + mainCapPhoneId);
                    newActivePhones.add(mainCapPhoneId);
                }
            }
        }
        /// @}
    }

    /***************  M: SIM ME Lock Section Start ***************/
    // M: Only for HAL_COMMAND_ALLOW_DATA
    private final BroadcastReceiver mSimLockChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Message msg = MtkPhoneSwitcher.this.obtainMessage(EVENT_SIMLOCK_INFO_CHANGED);
            //log("EVENT_SIMLOCK_INFO_CHANGED");
            msg.sendToTarget();
        }
    };

    @Override
    protected int phoneIdForRequest(NetworkRequest netRequest) {
        // M: SIM ME Lock, Do SIM ME Lock permission check for Activate/Deactivate.
        // M: Only for HAL_COMMAND_ALLOW_DATA
        int phoneId = super.phoneIdForRequest(netRequest);
        if (mHalCommandToUse == HAL_COMMAND_ALLOW_DATA || mHalCommandToUse == HAL_COMMAND_UNKNOWN) {
            if(getSimLockMode()) {
                if(!getPsAllowedByPhoneId(phoneId)){
                    return INVALID_PHONE_INDEX;
                }
            }
        }
        return phoneId;
    }

    /// M: Get the SIM lock Feature On/Off. @{
    /**
     * @return the true for feature on.
     */
    public boolean getSimLockMode(){
        int policy = MtkTelephonyManagerEx.getDefault().getSimLockPolicy();
        return (policy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ONLY_SLOT1 ||
                policy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ONLY_SLOT2 ||
                policy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL ||
                policy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOT1 ||
                policy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOT2 ||
                policy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA ||
                policy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_CS ||
                policy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_REVERSE ||
                policy == SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE);
    }

    /// M: Check the PhoneId has Ps capability. @{
    /**
     * @param phoneId.
     * @return true for Ps allowed.
     */
    public boolean getPsAllowedByPhoneId(int phoneId){
        int cap = MtkTelephonyManagerEx.getDefault().getShouldServiceCapability(phoneId);
        int policy = MtkTelephonyManagerEx.getDefault().getSimLockPolicy();
        switch (policy) {
            ///simmelock request 1:
            case MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ONLY_SLOT1:
            ///simmelock request 2:
            case MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ONLY_SLOT2:
            ///simmelock request 3:
            case MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL:
            ///simmelock request 4:
            case MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOT1:
            ///simmelock request 5:
            case MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOT2:
            ///simmelock request 6:
            case MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA:
            ///simmelock request 7:
            case MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_CS:
            ///simmelock request 8:
            case MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_REVERSE:
            ///simmelock request 9:
            case SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE:
            {
                return (cap == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_FULL ||
                    cap == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_PS_ONLY);
            }
            default:
                return true;
        }
    }
    /*************** SIM ME Lock Section End ***************/

    private static void log(String l) {
        Rlog.d(LOG_TAG, l);
    }

    private static void loge(String l) {
        Rlog.e(LOG_TAG, l);
    }

    private static void logv(String l) {
        Rlog.v(LOG_TAG, l);
    }
}

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

package com.mediatek.internal.telephony.imsphone;

import android.app.Activity;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.PowerManager;
import android.os.ResultReceiver;
import android.os.SystemProperties;

import android.telephony.CarrierConfigManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsInfo;
import android.telephony.Rlog;

import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsUtInterface;
import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;

import static com.android.internal.telephony.CommandsInterface.CF_ACTION_DISABLE;
import static com.android.internal.telephony.CommandsInterface.CF_ACTION_ENABLE;
import static com.android.internal.telephony.CommandsInterface.CF_ACTION_ERASURE;
import static com.android.internal.telephony.CommandsInterface.CF_ACTION_REGISTRATION;
import static com.android.internal.telephony.CommandsInterface.CF_REASON_ALL;
import static com.android.internal.telephony.CommandsInterface.CF_REASON_ALL_CONDITIONAL;
import static com.android.internal.telephony.CommandsInterface.CF_REASON_NO_REPLY;
import static com.android.internal.telephony.CommandsInterface.CF_REASON_NOT_REACHABLE;
import static com.android.internal.telephony.CommandsInterface.CF_REASON_BUSY;
import static com.android.internal.telephony.CommandsInterface.CF_REASON_UNCONDITIONAL;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneNotifier;
import com.android.internal.telephony.TelephonyComponentFactory;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.imsphone.ImsPhoneCallTracker;
import com.android.internal.telephony.imsphone.ImsPhoneMmiCode;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccController;

import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.ims.MtkImsCallForwardInfo;
import com.mediatek.ims.MtkImsReasonInfo;
import com.mediatek.ims.MtkImsUt;
import com.mediatek.internal.telephony.digits.DigitsUssdManager;
import com.mediatek.internal.telephony.MtkCallForwardInfo;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;
import com.mediatek.internal.telephony.MtkSubscriptionManager;

import static com.mediatek.internal.telephony.MtkGsmCdmaPhone.EVENT_IMS_UT_CSFB;

import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkSuppSrvRequest;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.lang.Boolean;
import java.util.Arrays;
import java.util.Calendar;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.TimeZone;

import mediatek.telephony.MtkCarrierConfigManager;
import mediatek.telephony.MtkServiceState;

/**
 * {@hide}
 */
public class MtkImsPhone extends ImsPhone {
    private static final String LOG_TAG = "MtkImsPhone";

    public enum FeatureType {
        VOLTE_ENHANCED_CONFERENCE,
        VIDEO_RESTRICTION,
        BLINDASSURED_ECT,
        CONSULTATIVE_ECT,
    }

    private String mDialString;

    private boolean mIsDigitsSupported = MtkTelephonyManagerEx.getDefault().isDigitsSupported();
    private boolean mIsBlindAssuredEctSupported;
    private boolean mIsConsultativeEctSupported;
    private boolean mIsDeviceSwitchSupported;

    // WFC
    private boolean mIsWfcModeHomeForDomRoaming;

    public static final String UT_BUNDLE_KEY_CLIR = "queryClir";

    private static final String CFU_TIME_SLOT = "persist.vendor.radio.cfu.timeslot.";

    public static final int EVENT_GET_CALL_FORWARD_TIME_SLOT_DONE = 109;
    public static final int EVENT_SET_CALL_FORWARD_TIME_SLOT_DONE = 110;

    {
        mSS = new MtkServiceState();
    }

    protected BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)) {
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                if (subId == getSubId()) {
                    logd("Receive carrierConfig changed: " + mPhoneId);
                    cacheCarrierConfiguration();
                }
            }
        }
    };

    // Constructors
    public MtkImsPhone(Context context, PhoneNotifier notifier, Phone defaultPhone) {
        this(context, notifier, defaultPhone, false);
    }

    @VisibleForTesting
    public MtkImsPhone(Context context, PhoneNotifier notifier, Phone defaultPhone,
                    boolean unitTestMode) {
        super(context, notifier, defaultPhone, unitTestMode);
        logd("Start to create MtkImsPhone.");
        setPhoneName("MtkImsPhone");

        // Force initial roaming state update later, on EVENT_CARRIER_CONFIG_CHANGED.
        // Settings provider or CarrierConfig may not be loaded now.
        cacheCarrierConfiguration();
        registerForListenCarrierConfigChanged();
    }

    @Override
    public void dispose() {
        super.dispose();
        if (mContext != null) {
            mContext.unregisterReceiver(mReceiver);
        }
    }

    @Override
    protected Connection dialInternal(String dialString, DialArgs dialArgs,
                                      ResultReceiver wrappedCallback)
            throws CallStateException {
        // Need to make sure dialString gets parsed properly
        /// M: Ignore stripping for VoLTE SIP uri. @{
        // String newDialString = PhoneNumberUtils.stripSeparators(dialString);
        boolean isUriNumber = PhoneNumberUtils.isUriNumber(dialString);
        String newDialString = dialString;
        if (!isUriNumber) {
            newDialString = PhoneNumberUtils.stripSeparators(dialString);
        }
        /// @}

        // handle in-call MMI first if applicable
        if (handleInCallMmiCommands(newDialString)) {
            return null;
        }

        ImsDialArgs.Builder imsDialArgsBuilder;
        // Get the CLIR info if needed
        if (!(dialArgs instanceof ImsDialArgs)) {
            imsDialArgsBuilder = ImsDialArgs.Builder.from(dialArgs);
        } else {
            imsDialArgsBuilder = ImsDialArgs.Builder.from((ImsDialArgs) dialArgs);
        }
        imsDialArgsBuilder.setClirMode(mCT.getClirMode());

        if (mDefaultPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
            return mCT.dial(dialString, imsDialArgsBuilder.build());
        }

        // Only look at the Network portion for mmi
        /// M: Ignore extracting for VoLTE SIP uri. @{
        // String networkPortion = PhoneNumberUtils.extractNetworkPortionAlt(newDialString);
        String networkPortion = dialString;
        if (!isUriNumber) {
            networkPortion = PhoneNumberUtils.extractNetworkPortionAlt(newDialString);
        }
        /// @}

        // for URI dial string, it must not be MMI code
        // Because we skip stripSeparators() / extractNetworkPortionAlt()
        //
        // it will cause some string like "@" will be viewed as short code MMI
        // if we pass into stripSeparators(), it will be changed to "" and then it will not be viewed as MMI
        //
        //MtkImsPhoneMmiCode mmi =
        //        MtkImsPhoneMmiCode.newFromDialString(networkPortion, this);
        MtkImsPhoneMmiCode mmi = null;
        if (!PhoneNumberUtils.isUriNumber(dialString)) {
            mmi = MtkImsPhoneMmiCode.newFromDialString(networkPortion, this, wrappedCallback);
        } else {
            if (DBG) logd("dialInternal: url dial string, it must not be MMI");
        }

        boolean isEcc = MtkLocalPhoneNumberUtils.getIsEmergencyNumber();
            //PhoneNumberUtils.isEmergencyNumber(getSubId(), dialString);

        if (DBG) logd("dialInternal: dialing w/ mmi [" + mmi + "] isEcc: " + isEcc);

        /// M:  CS Fall back to GsmCdmaPhone for MMI code. @{
        mDialString = dialString;
        /// @}
        if (mmi == null || isEcc) {
            return mCT.dial(dialString, imsDialArgsBuilder.build());
        } else if (mmi.isTemporaryModeCLIR()) {
            imsDialArgsBuilder.setClirMode(mmi.getCLIRMode());
            return mCT.dial(mmi.getDialingNumber(), imsDialArgsBuilder.build());
        } else if (!mmi.isSupportedOverImsPhone()) {
            // If the mmi is not supported by IMS service,
            // try to initiate dialing with default phone
            // Note: This code is never reached; there is a bug in isSupportedOverImsPhone which
            // causes it to return true even though the "processCode" method ultimately throws the
            // exception.
            logi("dialInternal: USSD not supported by IMS; fallback to CS.");
            throw new CallStateException(CS_FALLBACK);
        } else {
            mPendingMMIs.add(mmi);
            mMmiRegistrants.notifyRegistrants(new AsyncResult(null, mmi, null));

            try {
                DigitsUssdManager digitsUssdManager = OpTelephonyCustomizationUtils
                        .getOpFactory(this.getContext()).makeDigitsUssdManager();
                digitsUssdManager.setUssdExtra(dialArgs.intentExtras);
                mmi.processCode();
            } catch (CallStateException cse) {
                if (CS_FALLBACK.equals(cse.getMessage())) {
                    logi("dialInternal: fallback to GSM required.");
                    // Make sure we remove from the list of pending MMIs since it will handover to
                    // GSM.
                    mPendingMMIs.remove(mmi);
                    throw cse;
                }
            }

            return null;
        }
    }

    public void explicitCallTransfer(String number, int type) {
        ((MtkImsPhoneCallTracker)mCT).unattendedCallTransfer(number, type);
    }

    public void deviceSwitch(String number, String deviceId) {
        ((MtkImsPhoneCallTracker)mCT).deviceSwitch(number, deviceId);
    }

    public void cancelDeviceSwitch() {
        ((MtkImsPhoneCallTracker)mCT).cancelDeviceSwitch();
    }

    @Override
    public void setImsRegistered(boolean value) {
        mImsRegistered = value;
        /// M: ALPS02494504. Remove notification when registering on IMS. @{
        if (mImsRegistered) {
            final String notificationTag = "wifi_calling";
            final int notificationId = 1;

            NotificationManager notificationManager =
                    (NotificationManager) mContext.getSystemService(
                            Context.NOTIFICATION_SERVICE);
            notificationManager.cancel(notificationTag, notificationId);
        }
        /// @}
    }

    /// M: @{
    /* package */
    @Override
    protected void onIncomingUSSD(int ussdMode, String ussdMessage) {
        if (DBG) logd("onIncomingUSSD ussdMode=" + ussdMode);

        boolean isUssdError;
        boolean isUssdRequest;

        isUssdRequest
            = (ussdMode == CommandsInterface.USSD_MODE_REQUEST);

        isUssdError
            = (ussdMode != CommandsInterface.USSD_MODE_NOTIFY
                && ussdMode != CommandsInterface.USSD_MODE_REQUEST);

        ImsPhoneMmiCode found = null;
        for (int i = 0, s = mPendingMMIs.size() ; i < s; i++) {
            if(mPendingMMIs.get(i).isPendingUSSD()) {
                found = mPendingMMIs.get(i);
                break;
            }
        }

        if (found != null) {
            // Complete pending USSD
            if (isUssdError) {
                found.onUssdFinishedError();
            } else {
                found.onUssdFinished(ussdMessage, isUssdRequest);
            }
        } else { // pending USSD not found
            // The network may initiate its own USSD request

            // ignore everything that isnt a Notify or a Request
            // also, discard if there is no message to present
            if (!isUssdError && ussdMessage != null) {
                ImsPhoneMmiCode mmi;
                mmi = ImsPhoneMmiCode.newNetworkInitiatedUssd(ussdMessage,
                        isUssdRequest,
                        this);
                onNetworkInitiatedUssd(mmi);
            } else if (isUssdError) {
                MtkImsPhoneMmiCode mmi;
                mmi = MtkImsPhoneMmiCode.newNetworkInitiatedUssdError(ussdMessage, this);
                onNetworkInitiatedUssd(mmi);
            }
        }
    }

    // For VoLTE enhanced conference call.
    public Connection dial(List<String> numbers, int videoState)
            throws CallStateException {
        return ((MtkImsPhoneCallTracker)mCT).dial(numbers, videoState);
    }

    public void hangupAll() throws CallStateException {
        if (DBG) {
            logd("hangupAll");
        }
        ((MtkImsPhoneCallTracker)mCT).hangupAll();
    }
    /// @}

    public void removeMmi(ImsPhoneMmiCode mmi) {
        logd("removeMmi: " + mmi);
        dumpPendingMmi();
        mPendingMMIs.remove(mmi);
    }

    public void dumpPendingMmi() {
        int size = mPendingMMIs.size();
        if (size == 0) {
            logd("dumpPendingMmi: none");
            return;
        }
        for (int i=0; i<size; i++) {
            logd("dumpPendingMmi: " + mPendingMMIs.get(i));
        }
    }

    /**
     * CS Fall back to GsmCdmaPhone for MMI code.
     *
     *@param reason the reason for CS fallback
     */
    public void handleMmiCodeCsfb(int reason, MtkImsPhoneMmiCode mmi) {
        if (DBG) {
            logd("handleMmiCodeCsfb: reason = " + reason + ", mDialString = "
                    + mDialString + ", mmi=" + mmi);
        }
        removeMmi(mmi);

        if (reason == MtkImsReasonInfo.CODE_UT_XCAP_403_FORBIDDEN) {
            ((MtkGsmCdmaPhone)mDefaultPhone).setCsFallbackStatus(
                    MtkPhoneConstants.UT_CSFB_UNTIL_NEXT_BOOT);
        } else if (reason == MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST) {
            ((MtkGsmCdmaPhone)mDefaultPhone).setCsFallbackStatus(
                    MtkPhoneConstants.UT_CSFB_ONCE);
        }
        MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(
                MtkSuppSrvRequest.SUPP_SRV_REQ_MMI_CODE, null);
        ss.mParcel.writeString(mDialString);
        Message msgCSFB = mDefaultPhone.obtainMessage(EVENT_IMS_UT_CSFB, ss);

        mDefaultPhone.sendMessage(msgCSFB);
    }

    @Override
    protected boolean isValidCommandInterfaceCFReason (int commandInterfaceCFReason) {
        switch (commandInterfaceCFReason) {
        case CF_REASON_UNCONDITIONAL:
        case CF_REASON_BUSY:
        case CF_REASON_NO_REPLY:
        case CF_REASON_NOT_REACHABLE:
        case CF_REASON_ALL:
        case CF_REASON_ALL_CONDITIONAL:
        case MtkRIL.CF_REASON_NOT_REGISTERED:
            return true;
        default:
            return false;
        }
    }

    @Override
    protected int getConditionFromCFReason(int reason) {
        switch(reason) {
            case CF_REASON_UNCONDITIONAL: return ImsUtInterface.CDIV_CF_UNCONDITIONAL;
            case CF_REASON_BUSY: return ImsUtInterface.CDIV_CF_BUSY;
            case CF_REASON_NO_REPLY: return ImsUtInterface.CDIV_CF_NO_REPLY;
            case CF_REASON_NOT_REACHABLE: return ImsUtInterface.CDIV_CF_NOT_REACHABLE;
            case CF_REASON_ALL: return ImsUtInterface.CDIV_CF_ALL;
            case CF_REASON_ALL_CONDITIONAL: return ImsUtInterface.CDIV_CF_ALL_CONDITIONAL;
            case MtkRIL.CF_REASON_NOT_REGISTERED: return ImsUtInterface.CDIV_CF_NOT_LOGGED_IN;
            default:
                break;
        }

        return ImsUtInterface.INVALID;
    }

    @Override
    protected int getCFReasonFromCondition(int condition) {
        switch(condition) {
            case ImsUtInterface.CDIV_CF_UNCONDITIONAL: return CF_REASON_UNCONDITIONAL;
            case ImsUtInterface.CDIV_CF_BUSY: return CF_REASON_BUSY;
            case ImsUtInterface.CDIV_CF_NO_REPLY: return CF_REASON_NO_REPLY;
            case ImsUtInterface.CDIV_CF_NOT_REACHABLE: return CF_REASON_NOT_REACHABLE;
            case ImsUtInterface.CDIV_CF_ALL: return CF_REASON_ALL;
            case ImsUtInterface.CDIV_CF_ALL_CONDITIONAL: return CF_REASON_ALL_CONDITIONAL;
            case ImsUtInterface.CDIV_CF_NOT_LOGGED_IN: return MtkRIL.CF_REASON_NOT_REGISTERED;
            default:
                break;
        }

        return CF_REASON_NOT_REACHABLE;
    }

    // Create Cf (Call forward) so that dialling number &
    // mIsCfu (true if reason is call forward unconditional)
    // mOnComplete (Message object passed by client) can be packed &
    // given as a single Cf object as user data to UtInterface.
    private static class Cf {
        public final String mSetCfNumber;
        public final Message mOnComplete;
        public final boolean mIsCfu;
        public final int mServiceClass;

        public Cf(String cfNumber, boolean isCfu, Message onComplete, int serviceClass) {
            mSetCfNumber = cfNumber;
            mIsCfu = isCfu;
            mOnComplete = onComplete;
            mServiceClass = serviceClass;
        }
    }

    @Override
    public void getCallForwardingOption(int commandInterfaceCFReason,
            Message onComplete) {
        if (DBG) logd("getCallForwardingOption reason=" + commandInterfaceCFReason);
        if (isValidCommandInterfaceCFReason(commandInterfaceCFReason)) {
            if (DBG) logd("requesting call forwarding query.");

            Message resp;
            resp = obtainMessage(EVENT_GET_CALL_FORWARD_DONE, onComplete);

            try {
                ImsUtInterface ut = mCT.getUtInterface();
                ut.queryCallForward(getConditionFromCFReason(
                        commandInterfaceCFReason), null, resp);
            } catch (ImsException e) {
                sendErrorResponse(onComplete, e);
            }
        } else if (onComplete != null) {
            sendErrorResponse(onComplete);
        }
    }

    @Override
    public void setCallForwardingOption(int commandInterfaceCFAction,
            int commandInterfaceCFReason,
            String dialingNumber,
            int serviceClass,
            int timerSeconds,
            Message onComplete) {
        if (DBG) logd("setCallForwardingOption action=" + commandInterfaceCFAction
                + ", reason=" + commandInterfaceCFReason + " serviceClass=" + serviceClass);
        if ((isValidCommandInterfaceCFAction(commandInterfaceCFAction)) &&
                (isValidCommandInterfaceCFReason(commandInterfaceCFReason))) {
            Message resp;

            // + [ALPS02301009]
            if (dialingNumber == null || dialingNumber.isEmpty()) {
                if (mDefaultPhone != null &&
                    mDefaultPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
                    if ((mDefaultPhone instanceof MtkGsmCdmaPhone)
                        && ((MtkGsmCdmaPhone) mDefaultPhone).isSupportSaveCFNumber()) {
                        if (isCfEnable(commandInterfaceCFAction)) {
                            String getNumber =
                                ((MtkGsmCdmaPhone) mDefaultPhone).getCFPreviousDialNumber(
                                    commandInterfaceCFReason);

                            if (getNumber != null && !getNumber.isEmpty()) {
                                dialingNumber = getNumber;
                            }
                        }
                    }
                }
            }
            // - [ALPS02301009]
            Cf cf = new Cf(dialingNumber,
                    (commandInterfaceCFReason == CF_REASON_UNCONDITIONAL ? true : false),
                    onComplete, serviceClass);
            resp = obtainMessage(EVENT_SET_CALL_FORWARD_DONE,
                    isCfEnable(commandInterfaceCFAction) ? 1 : 0, 0, cf);

            try {
                ImsUtInterface ut = mCT.getUtInterface();
                ut.updateCallForward(getActionFromCFAction(commandInterfaceCFAction),
                        getConditionFromCFReason(commandInterfaceCFReason),
                        dialingNumber,
                        serviceClass,
                        timerSeconds,
                        // M:
                        resp);
                        /// @}
            } catch (ImsException e) {
                sendErrorResponse(onComplete, e);
            }
        } else if (onComplete != null) {
            sendErrorResponse(onComplete);
        }
    }

    protected int getCBTypeFromFacility(String facility) {
        if (MtkRIL.CB_FACILITY_BA_ACR.equals(facility)) {
            return ImsUtInterface.CB_BIC_ACR;
        }

        return super.getCBTypeFromFacility(facility);
    }

    @Override
    public void setCallBarring(String facility, boolean lockState, String password,
            Message onComplete,  int serviceClass) {
        if (DBG) {
            logd("setCallBarring facility=" + facility
                    + ", lockState=" + lockState + ", serviceClass = " + serviceClass);
        }
        Message resp;
        resp = obtainMessage(EVENT_SET_CALL_BARRING_DONE, onComplete);

        int action;
        if (lockState) {
            action = CommandsInterface.CF_ACTION_ENABLE;
        }
        else {
            action = CommandsInterface.CF_ACTION_DISABLE;
        }

        try {
            ImsUtInterface ut = mCT.getUtInterface();
            // password is not required with Ut interface, but would be needed when CSFB occurs
            ((MtkImsUt)ut).updateCallBarring(password, getCBTypeFromFacility(facility), action,
                    resp, null,  serviceClass);
        } catch (ImsException e) {
            sendErrorResponse(onComplete, e);
        }
    }

    private static class CfEx {
        final String mSetCfNumber;
        final long[] mSetTimeSlot;
        final Message mOnComplete;
        final boolean mIsCfu;

        CfEx(String cfNumber, long[] cfTimeSlot, boolean isCfu, Message onComplete) {
            mSetCfNumber = cfNumber;
            mSetTimeSlot = cfTimeSlot;
            mIsCfu = isCfu;
            mOnComplete = onComplete;
        }
    }

    public void saveTimeSlot(long[] timeSlot) {
        String timeSlotKey = CFU_TIME_SLOT + mPhoneId;
        String timeSlotString = "";
        if (timeSlot != null && timeSlot.length == 2) {
            timeSlotString = Long.toString(timeSlot[0]) + "," + Long.toString(timeSlot[1]);
        }
        SystemProperties.set(timeSlotKey, timeSlotString);
        logd("timeSlotString = " + timeSlotString);
    }

    public long[] getTimeSlot() {
        String timeSlotKey = CFU_TIME_SLOT + mPhoneId;
        String timeSlotString = SystemProperties.get(timeSlotKey, "");
        long[] timeSlot = null;
        if (timeSlotString != null && !timeSlotString.equals("")) {
            String[] timeArray = timeSlotString.split(",");
            if (timeArray.length == 2) {
                timeSlot = new long[2];
                for (int i = 0; i < 2; i++) {
                    timeSlot[i] = Long.parseLong(timeArray[i]);
                    Calendar calenar = Calendar.getInstance(TimeZone.getDefault());
                    calenar.setTimeInMillis(timeSlot[i]);
                    int hour = calenar.get(Calendar.HOUR_OF_DAY);
                    int min = calenar.get(Calendar.MINUTE);
                    Calendar calenar2 = Calendar.getInstance(TimeZone.getDefault());
                    calenar2.set(Calendar.HOUR_OF_DAY, hour);
                    calenar2.set(Calendar.MINUTE, min);
                    timeSlot[i] = calenar2.getTimeInMillis();
                }
            }
        }
        logd("timeSlot = " + Arrays.toString(timeSlot));
        return timeSlot;
    }

    public void getCallForwardInTimeSlot(int commandInterfaceCFReason,
            Message onComplete) {
        if (DBG) {
            logd("getCallForwardInTimeSlot reason = " + commandInterfaceCFReason);
        }
        if (commandInterfaceCFReason == CF_REASON_UNCONDITIONAL) {
            if (DBG) {
                logd("requesting call forwarding in a time slot query.");
            }

            Message resp;
            resp = obtainMessage(EVENT_GET_CALL_FORWARD_TIME_SLOT_DONE, onComplete);

            try {
                ImsUtInterface ut = mCT.getUtInterface();
                ((MtkImsUt)ut).queryCallForwardInTimeSlot(
                        getConditionFromCFReason(commandInterfaceCFReason),
                        resp);
            } catch (ImsException e) {
                sendErrorResponse(onComplete, e);
            }
        } else if (onComplete != null) {
            sendErrorResponse(onComplete);
        }
    }

    public void setCallForwardInTimeSlot(int commandInterfaceCFAction,
            int commandInterfaceCFReason,
            String dialingNumber,
            int timerSeconds,
            long[] timeSlot,
            Message onComplete) {
        if (DBG) {
            logd("setCallForwardInTimeSlot action = " + commandInterfaceCFAction
                    + ", reason = " + commandInterfaceCFReason);
        }
        if ((isValidCommandInterfaceCFAction(commandInterfaceCFAction)) &&
                (commandInterfaceCFReason == CF_REASON_UNCONDITIONAL)) {
            Message resp;
            CfEx cfEx = new CfEx(dialingNumber, timeSlot, true, onComplete);
            resp = obtainMessage(EVENT_SET_CALL_FORWARD_TIME_SLOT_DONE,
                    commandInterfaceCFAction, 0, cfEx);

            try {
                ImsUtInterface ut = mCT.getUtInterface();
                ((MtkImsUt)ut).updateCallForwardInTimeSlot(
                        getActionFromCFAction(commandInterfaceCFAction),
                        getConditionFromCFReason(commandInterfaceCFReason),
                        dialingNumber,
                        timerSeconds,
                        timeSlot,
                        resp);
             } catch (ImsException e) {
                sendErrorResponse(onComplete, e);
             }
        } else if (onComplete != null) {
            sendErrorResponse(onComplete);
        }
    }

    public void getCallForwardingOptionForServiceClass(int commandInterfaceCFReason,
                                                       int serviceClass, Message onComplete) {
        if (DBG) logd("getCallForwardingOptionForServiceClass reason=" + commandInterfaceCFReason
                + ", service class= " + serviceClass);
        if (isValidCommandInterfaceCFReason(commandInterfaceCFReason)) {
            if (DBG) logd("requesting call forwarding query.");
            Message resp;
            resp = obtainMessage(EVENT_GET_CALL_FORWARD_DONE, onComplete);

            try {
                ImsUtInterface ut = mCT.getUtInterface();
                ((MtkImsUt)ut).queryCFForServiceClass(
                        getConditionFromCFReason(commandInterfaceCFReason),
                        null, serviceClass, resp);
            } catch (ImsException e) {
                sendErrorResponse(onComplete, e);
            }
        } else if (onComplete != null) {
            sendErrorResponse(onComplete);
        }
    }

    private MtkCallForwardInfo[] handleCfInTimeSlotQueryResult(MtkImsCallForwardInfo[] infos) {
        MtkCallForwardInfo[] cfInfos = null;

        if (supportMdAutoSetupIms()) {
            if (infos != null && infos.length != 0) {
                cfInfos = new MtkCallForwardInfo[infos.length];
            }
        } else {
            // ALPS03047471: Fix JE when query CF
            if (infos != null) {
                cfInfos = new MtkCallForwardInfo[infos.length];
            }
        }

        IccRecords r = mDefaultPhone.getIccRecords();
        if (infos == null || infos.length == 0) {
            if (r != null) {
                // Assume the default is not active
                // Set unconditional CFF in SIM to false
                setVoiceCallForwardingFlag(r, 1, false, null);
            }
        } else {
            for (int i = 0, s = infos.length; i < s; i++) {
                if (infos[i].mCondition == ImsUtInterface.CDIV_CF_UNCONDITIONAL &&
                        (infos[i].mServiceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) {
                    if (r != null) {
                        setVoiceCallForwardingFlag(r, 1, (infos[i].mStatus == 1),
                            infos[i].mNumber);
                        saveTimeSlot(infos[i].mTimeSlot);
                    }
                }
                cfInfos[i] = getMtkCallForwardInfo(infos[i]);
            }
        }

        return cfInfos;
    }

    private MtkCallForwardInfo getMtkCallForwardInfo(MtkImsCallForwardInfo info) {
        MtkCallForwardInfo cfInfo = new MtkCallForwardInfo();
        cfInfo.status = info.mStatus;
        cfInfo.reason = getCFReasonFromCondition(info.mCondition);
        cfInfo.serviceClass = info.mServiceClass;
        cfInfo.toa = info.mToA;
        cfInfo.number = info.mNumber;
        cfInfo.timeSeconds = info.mTimeSeconds;
        cfInfo.timeSlot = info.mTimeSlot;
        return cfInfo;
    }


    @Override
    public void sendUssdResponse(String ussdMessge) {
        dumpPendingMmi();
        super.sendUssdResponse(ussdMessge);
    }

    @Override
    protected CommandException getCommandException(int code, String errorString) {
        logd("getCommandException code= " + code
                + ", errorString= " + errorString);
        CommandException.Error error = CommandException.Error.GENERIC_FAILURE;

        switch(code) {
            case ImsReasonInfo.CODE_UT_NOT_SUPPORTED:
                error = CommandException.Error.REQUEST_NOT_SUPPORTED;
                break;
            case ImsReasonInfo.CODE_UT_CB_PASSWORD_MISMATCH:
                error = CommandException.Error.PASSWORD_INCORRECT;
                break;
            case ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE:
                error = CommandException.Error.RADIO_NOT_AVAILABLE;
                break;
            case ImsReasonInfo.CODE_FDN_BLOCKED:
                error = CommandException.Error.FDN_CHECK_FAILURE;
                break;
            case ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_DIAL:
                error = CommandException.Error.SS_MODIFIED_TO_DIAL;
                break;
            case ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_USSD:
                error = CommandException.Error.SS_MODIFIED_TO_USSD;
                break;
            case ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_SS:
                error = CommandException.Error.SS_MODIFIED_TO_SS;
                break;
            case ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_DIAL_VIDEO:
                error = CommandException.Error.SS_MODIFIED_TO_DIAL_VIDEO;
                break;
            case MtkImsReasonInfo.CODE_UT_XCAP_403_FORBIDDEN:
                error = CommandException.Error.OPERATION_NOT_ALLOWED;
                break;
            case MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST:
                error = CommandException.Error.OEM_ERROR_3;
                break;
            case MtkImsReasonInfo.CODE_UT_XCAP_409_CONFLICT:
                // 409 CONFLICT
                error = CommandException.Error.OEM_ERROR_25;
                break;
            case MtkImsReasonInfo.CODE_UT_XCAP_832_TERMINAL_BASE_SOLUTION:
                error = CommandException.Error.OEM_ERROR_7;
                break;
            default:
                break;
        }

        return new CommandException(error, errorString);
    }

    @Override
    protected CallForwardInfo getCallForwardInfo(ImsCallForwardInfo info) {
        CallForwardInfo cfInfo = new CallForwardInfo();
        cfInfo.status = info.mStatus;
        cfInfo.reason = getCFReasonFromCondition(info.mCondition);
        cfInfo.serviceClass = info.mServiceClass;
        cfInfo.toa = info.mToA;
        cfInfo.number = info.mNumber;
        cfInfo.timeSeconds = info.mTimeSeconds;
        return cfInfo;
    }

    @Override
    public CallForwardInfo[] handleCfQueryResult(ImsCallForwardInfo[] infos) {
        CallForwardInfo[] cfInfos = null;

        if (supportMdAutoSetupIms()) {
            // AOSP logic for 93
            if (infos != null && infos.length != 0) {
                cfInfos = new CallForwardInfo[infos.length];
            }
        } else {
            // ALPS03047471: Fix JE when query CF
            if (infos != null) {
                cfInfos = new CallForwardInfo[infos.length];
            }
        }

        IccRecords r = mDefaultPhone.getIccRecords();
        if (infos == null || infos.length == 0) {
            if (r != null) {
                // Assume the default is not active
                // Set unconditional CFF in SIM to false
                setVoiceCallForwardingFlag(r, 1, false, null);
            }
        } else {
            for (int i = 0, s = infos.length; i < s; i++) {
                if (infos[i].mCondition == ImsUtInterface.CDIV_CF_UNCONDITIONAL &&
                        (infos[i].mServiceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) {
                    if (r != null) {
                        setVoiceCallForwardingFlag(r, 1, (infos[i].mStatus == 1),
                            infos[i].mNumber);
                    }
                }
                cfInfos[i] = getCallForwardInfo(infos[i]);
            }
        }

        return cfInfos;
    }

    @Override
    protected int[] handleCbQueryResult(ImsSsInfo[] infos) {
        int[] cbInfos = new int[1];
        // cbInfos[0] = SERVICE_CLASS_NONE;

        // if (infos[0].mStatus == 1) {
        //     cbInfos[0] = SERVICE_CLASS_VOICE;
        // }

        // M:
        cbInfos[0] = infos[0].mStatus;
        // @}
        return cbInfos;
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;

        if (DBG) logd("Mtk handleMessage what=" + msg.what);

        switch (msg.what) {
            case EVENT_SET_CALL_FORWARD_DONE:
                {
                    IccRecords r = mDefaultPhone.getIccRecords();
                    Cf cf = (Cf) ar.userObj;
                    int cfAction = msg.arg1;
                    int cfReason = msg.arg2;

                    int cfEnable = isCfEnable(cfAction) ? 1 : 0;

                    if (cf.mIsCfu && ar.exception == null && r != null) {
                        if (((MtkGsmCdmaPhone) mDefaultPhone).queryCFUAgainAfterSet()
                            && cfReason == CF_REASON_UNCONDITIONAL) {
                            if (ar.result == null) {
                                logi("arResult is null.");
                            } else {
                                logd("[EVENT_SET_CALL_FORWARD_DONE check cfinfo.");
                            }
                        } else {
                            if ((cf.mServiceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) {
                                setVoiceCallForwardingFlag(r, 1, cfEnable == 1, cf.mSetCfNumber);
                            }
                        }
                    }

                    // + [ALPS02301009]
                    if (mDefaultPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
                        if ((mDefaultPhone instanceof MtkGsmCdmaPhone)
                            && ((MtkGsmCdmaPhone) mDefaultPhone).isSupportSaveCFNumber()) {
                            if (ar.exception == null) {
                                if (cfEnable == 1) {
                                    boolean ret =
                                        ((MtkGsmCdmaPhone) mDefaultPhone).applyCFSharePreference(
                                            cfReason, cf.mSetCfNumber);
                                    if (!ret) {
                                        logd("applySharePreference false.");
                                    }
                                }

                                if (cfAction == CF_ACTION_ERASURE) {
                                    ((MtkGsmCdmaPhone) mDefaultPhone).clearCFSharePreference(
                                            cfReason);
                                }
                            }
                        }
                    }
                    // - [ALPS02301009]
                    sendResponse(cf.mOnComplete, null, ar.exception);
                }
                break;

            case EVENT_GET_CALL_FORWARD_TIME_SLOT_DONE:
                MtkCallForwardInfo[] mtkCfInfos = null;
                if (ar.exception == null) {
                    mtkCfInfos = handleCfInTimeSlotQueryResult(
                           (MtkImsCallForwardInfo[]) ar.result);
                }

                if ((ar.exception != null) && (ar.exception instanceof ImsException)) {
                    ImsException imsException = (ImsException) ar.exception;
                    if ((imsException != null) && (imsException.getCode() ==
                           MtkImsReasonInfo.CODE_UT_XCAP_403_FORBIDDEN)) {
                        ((MtkGsmCdmaPhone)mDefaultPhone).setCsFallbackStatus(
                                MtkPhoneConstants.UT_CSFB_UNTIL_NEXT_BOOT);
                        Message resp = (Message) ar.userObj;
                        if (resp != null) {
                           AsyncResult.forMessage(resp, mtkCfInfos, new CommandException(
                                   CommandException.Error.REQUEST_NOT_SUPPORTED));
                           resp.sendToTarget();
                           return;
                       }
                    }
                }
                sendResponse((Message) ar.userObj, mtkCfInfos, ar.exception);
                break;

            case EVENT_SET_CALL_FORWARD_TIME_SLOT_DONE: {
                IccRecords records = mDefaultPhone.getIccRecords();
                CfEx cfEx = (CfEx) ar.userObj;
                if (cfEx.mIsCfu && ar.exception == null && records != null) {
                    int cfAction = msg.arg1;
                    int cfEnable = isCfEnable(cfAction) ? 1 : 0;

                    setVoiceCallForwardingFlag(records, 1, cfEnable == 1, cfEx.mSetCfNumber);
                    saveTimeSlot(cfEx.mSetTimeSlot);
                }

                if ((ar.exception != null) && (ar.exception instanceof ImsException)) {
                    ImsException imsException = (ImsException) ar.exception;
                    if ((imsException != null) && (imsException.getCode() ==
                           MtkImsReasonInfo.CODE_UT_XCAP_403_FORBIDDEN)) {
                        ((MtkGsmCdmaPhone)mDefaultPhone).setCsFallbackStatus(
                                MtkPhoneConstants.UT_CSFB_UNTIL_NEXT_BOOT);
                        Message resp = cfEx.mOnComplete;
                        if (resp != null) {
                           AsyncResult.forMessage(resp, null, new CommandException(
                                   CommandException.Error.REQUEST_NOT_SUPPORTED));
                           resp.sendToTarget();
                           return;
                       }
                    }
                }
                sendResponse(cfEx.mOnComplete, null, ar.exception);
                break;
            }

            case EVENT_GET_CALL_BARRING_DONE:
                {
                    // Do not do in 93 IMS SS logic
                    if (supportMdAutoSetupIms() == false) {
                        if (((MtkGsmCdmaPhone) mDefaultPhone).isOpTransferXcap404() &&
                            (ar.exception != null) && (ar.exception instanceof ImsException)) {
                             ImsException imsException = (ImsException) ar.exception;
                             if ((imsException != null) && (imsException.getCode() ==
                                    MtkImsReasonInfo.CODE_UT_XCAP_404_NOT_FOUND)) {
                                 Message resp = (Message) ar.userObj;
                                 if (resp != null) {
                                    AsyncResult.forMessage(resp, null, new CommandException(
                                            CommandException.Error.NO_SUCH_ELEMENT));
                                    resp.sendToTarget();
                                    return;
                                }
                            }
                        }
                    }
                    int[] ssInfos = null;
                    if (ar.exception == null) {
                        ssInfos = handleCbQueryResult((ImsSsInfo[])ar.result);
                    }
                    sendResponse((Message) ar.userObj, ssInfos, ar.exception);
                }
                break;

            case EVENT_SET_CALL_BARRING_DONE:
                {
                    // Do not do in 93 IMS SS logic
                    if (supportMdAutoSetupIms() == false) {
                        if (((MtkGsmCdmaPhone) mDefaultPhone).isOpTransferXcap404()
                            && (ar.exception != null)
                            && (ar.exception instanceof ImsException)) {
                             ImsException imsException = (ImsException) ar.exception;
                             if ((imsException != null) && (imsException.getCode() ==
                                    MtkImsReasonInfo.CODE_UT_XCAP_404_NOT_FOUND)) {
                                 Message resp = (Message) ar.userObj;
                                 if (resp != null) {
                                    AsyncResult.forMessage(resp, null, new CommandException(
                                            CommandException.Error.NO_SUCH_ELEMENT));
                                    resp.sendToTarget();
                                    return;
                                }
                             }
                        }
                    }
                    sendResponse((Message) ar.userObj, null, ar.exception);
                }
                break;

            default:
                super.handleMessage(msg);
                break;
        }
    }
    /// @} // MTK SS end

    /**
     * Update roaming state and WFC mode in the following situations:
     *     1) voice is in service.
     *     2) data is in service and it is not IWLAN (if in legacy mode).
     * @param ss non-null ServiceState
     */
    @Override
    protected void updateRoamingState(ServiceState ss) {
        if (ss == null) {
            loge("updateRoamingState: null ServiceState!");
            return;
        }
        boolean newRoamingState = ss.getRoaming();
        // Do not recalculate if there is no change to state.
        if (mRoaming == newRoamingState) {
            return;
        }
        boolean isInService = (ss.getVoiceRegState() == ServiceState.STATE_IN_SERVICE
                || ss.getDataRegState() == ServiceState.STATE_IN_SERVICE);
        // If we are not IN_SERVICE for voice or data, ignore change roaming state, as we always
        // move to home in this case.
        if (!isInService) {
            logi("updateRoamingState: we are OUT_OF_SERVICE, ignoring roaming change.");
            return;
        }
        // We ignore roaming changes when moving to IWLAN because it always sets the roaming
        // mode to home and masks the actual cellular roaming status if voice is not registered. If
        // we just moved to IWLAN because WFC roaming mode is IWLAN preferred and WFC home mode is
        // cell preferred, we can get into a condition where the modem keeps bouncing between
        // IWLAN->cell->IWLAN->cell...
        if (isCsNotInServiceAndPsWwanReportingWlan(ss)) {
            logi("updateRoamingState: IWLAN masking roaming, ignore roaming change.");
            return;
        }
        if (mCT.getState() == PhoneConstants.State.IDLE) {
            if (DBG) logd("updateRoamingState now: " + newRoamingState);
            mRoaming = newRoamingState;
            ImsManager imsManager = ImsManager.getInstance(mContext, mPhoneId);

            if (mIsWfcModeHomeForDomRoaming) {
                int voiceRoamingType = ss.getVoiceRoamingType();
                int dataRoamingType = ss.getDataRoamingType();

                if (mRoaming && (voiceRoamingType == ServiceState.ROAMING_TYPE_DOMESTIC ||
                        dataRoamingType == ServiceState.ROAMING_TYPE_DOMESTIC)) {
                    logd("Convert new roaming to HOME if it's domestic roaming, " +
                        " voiceRoamingType: " + voiceRoamingType +
                        " dataRoamingType: " + dataRoamingType);

                    imsManager.setWfcMode(imsManager.getWfcMode(false), false);
                    return;
                }
            }

            imsManager.setWfcMode(imsManager.getWfcMode(newRoamingState), newRoamingState);
        } else {
            if (DBG) logd("updateRoamingState postponed: " + newRoamingState);
            mCT.registerForVoiceCallEnded(this, EVENT_VOICE_CALL_ENDED, null);
        }
    }

    @Override
    @VisibleForTesting
    public void setServiceState(int state) {
        super.setServiceState(state);
        updateIsEmergencyOnly();
    }

    private boolean isSupportImsEcc() {
        return ((MtkImsPhoneCallTracker)mCT).isSupportImsEcc();
    }

    public void updateIsEmergencyOnly() {
        ServiceState ss = getServiceState();
        logd("updateIsEmergencyOnly() sst: " + ss.getState() +
                " supportImsEcc: " + isSupportImsEcc());
        if (ss.getState() == ServiceState.STATE_OUT_OF_SERVICE && isSupportImsEcc()) {
            mSS.setEmergencyOnly(true);
        } else {
            mSS.setEmergencyOnly(false);
        }
    }

    private boolean supportMdAutoSetupIms() {
        boolean r = false;
        if (SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1")) {
            r = true;
        }
        return r;
    }

    private void registerForListenCarrierConfigChanged() {
        if (mContext == null) {
            logd("registerForListenCarrierConfigChanged failed");
            return;
        }
        IntentFilter intentfilter = new IntentFilter();
        intentfilter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        mContext.registerReceiver(mReceiver, intentfilter);
    }

    private void cacheCarrierConfiguration() {
        int subId = getSubId();
        CarrierConfigManager configMgr = (CarrierConfigManager) mContext.
                getSystemService(Context.CARRIER_CONFIG_SERVICE);
        if (configMgr == null) {
            logd("cacheCarrierConfiguration failed: config mgr access failed");
            return;
        }

        PersistableBundle carrierConfig = configMgr.getConfigForSubId(subId);

        if (carrierConfig == null) {
            logd("cacheCarrierConfiguration failed: carrier config access failed");
            return;
        }

        mIsConsultativeEctSupported = carrierConfig.getBoolean(
                MtkCarrierConfigManager.MTK_KEY_CONSULTATIVE_ECT_SUPPORTED);

        mIsBlindAssuredEctSupported = carrierConfig.getBoolean(
                MtkCarrierConfigManager.MTK_KEY_BLIND_ASSURED_ECT_SUPPORTED);

        mIsDeviceSwitchSupported = carrierConfig.getBoolean(
                    MtkCarrierConfigManager.MTK_KEY_DEVICE_SWITCH_SUPPORTED);

        mIsWfcModeHomeForDomRoaming = carrierConfig.getBoolean(
                    MtkCarrierConfigManager.MTK_KEY_WFC_MODE_DOMESTIC_ROMAING_TO_HOME);

        logd("cacheCarrierConfiguration, "
            + "blindAssureEctSupported: " + mIsBlindAssuredEctSupported
            + " deviceSwitchSupported: " + mIsDeviceSwitchSupported
            + " WfcModeHomeForDomRoaming: " + mIsWfcModeHomeForDomRoaming);
    }

    public boolean isFeatureSupported(FeatureType feature) {
        if (feature == FeatureType.VOLTE_ENHANCED_CONFERENCE
                || feature == FeatureType.VIDEO_RESTRICTION
                || feature == FeatureType.BLINDASSURED_ECT) {
            final List<String> voLteEnhancedConfMccMncList = Arrays.asList(
                    // 1. CMCC:
                    "46000", "46002", "46004", "46007", "46008",
                    // 2. Test SIM:
                    "00101");

            IccRecords iccRecords = mDefaultPhone.getIccRecords();
            if (iccRecords == null) {
                logd("isFeatureSupported(" + feature + ") no iccRecords");
                return false;
            }

            String mccMnc = iccRecords.getOperatorNumeric();
            if (feature == FeatureType.BLINDASSURED_ECT) {
                if (mIsBlindAssuredEctSupported) {
                    logd("isFeatureSupported(" + feature + "): true"
                            + " current mccMnc = " + mccMnc);
                    return true;
                }
                if (mIsDigitsSupported && mIsDeviceSwitchSupported) {
                    logd("Digits device and TMO card, ECT supported: " + mccMnc);
                    return true;
                }
            } else {
                boolean ret = voLteEnhancedConfMccMncList.contains(mccMnc);
                logd("isFeatureSupported(" + feature + "): ret = " + ret
                        + " current mccMnc = " + mccMnc);
                return ret;
            }
        } else if (feature == FeatureType.CONSULTATIVE_ECT) {
            if (mIsConsultativeEctSupported) {
                logd("isFeatureSupported(" + feature + "): true");
                return true;
            } else {
                logd("isFeatureSupported(" + feature + "): false");
                return false;
            }
        }
        return false;
    }

    public boolean isWifiPdnOutOfService() {
        return ((MtkImsPhoneCallTracker)mCT).isWifiPdnOutOfService();
    }

    @Override
    public void setVoiceCallForwardingFlag(IccRecords r, int line, boolean enable, String number) {
        super.setVoiceCallForwardingFlag(r, line, enable, number);
        if (mDefaultPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA
                && (mDefaultPhone instanceof MtkGsmCdmaPhone)
                && ((MtkGsmCdmaPhone)mDefaultPhone).isGsmSsPrefer()) {
            UiccController uiccCtl = UiccController.getInstance();
            if (uiccCtl != null) {
                IccRecords record = uiccCtl.getIccRecords(mPhoneId, UiccController.APP_FAM_3GPP);
                if (record != null) {
                    record.setVoiceCallForwardingFlag(line, enable, number);
                }
            }
            mDefaultPhone.notifyCallForwardingIndicator();
        }
    }

    @Override
    protected boolean handleEctIncallSupplementaryService(String dialString) {

        int len = dialString.length();

        if (len != 1) {
            return false;
        }

        if (isFeatureSupported(FeatureType.CONSULTATIVE_ECT)) {
            if (DBG) Rlog.d(LOG_TAG, "supports explicit call transfer");
            mCT.explicitCallTransfer();
            return true;
        }

        return super.handleEctIncallSupplementaryService(dialString);
    }
}

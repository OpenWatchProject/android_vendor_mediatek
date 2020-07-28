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
import android.content.Intent;
import com.android.internal.telephony.RadioIndication;
import com.android.internal.telephony.RIL;

import android.os.RemoteException;

import android.hardware.radio.V1_0.CdmaCallWaiting;
import android.hardware.radio.V1_0.CdmaInformationRecord;
import android.hardware.radio.V1_0.CdmaLineControlInfoRecord;
import android.hardware.radio.V1_0.CdmaNumberInfoRecord;
import android.hardware.radio.V1_0.CdmaRedirectingNumberInfoRecord;
import android.hardware.radio.V1_0.CdmaSignalInfoRecord;
import android.hardware.radio.V1_0.CdmaSmsMessage;
import android.hardware.radio.V1_0.CdmaT53AudioControlInfoRecord;
import android.hardware.radio.V1_0.CellInfoType;
import android.hardware.radio.V1_0.CfData;
import android.hardware.radio.V1_0.LceDataInfo;
import android.hardware.radio.V1_0.PcoDataInfo;
import android.hardware.radio.V1_0.SetupDataCallResult;
import android.hardware.radio.V1_0.SimRefreshResult;
import android.hardware.radio.V1_0.SsInfoData;
import android.hardware.radio.V1_0.StkCcUnsolSsResult;
import android.hardware.radio.V1_0.SuppSvcNotification;

import android.os.AsyncResult;
import android.os.Build;
import android.os.SystemProperties;
import android.os.UserHandle;

import android.util.Log;
import java.util.ArrayList;
import java.util.Iterator;

import com.android.internal.telephony.cdma.CdmaInformationRecords;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.internal.telephony.gsm.MtkSuppServiceNotification;
import com.mediatek.internal.telephony.gsm.MtkSuppCrssNotification;
import com.mediatek.internal.telephony.worldphone.WorldMode;
import static com.android.internal.telephony.RILConstants.*;
import static com.mediatek.internal.telephony.MtkRILConstants.*;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.android.internal.telephony.CommandsInterface;

import android.os.Build;

// MTK-START, SMS part
import vendor.mediatek.hardware.mtkradioex.V1_0.EtwsNotification;
import com.mediatek.internal.telephony.MtkEtwsNotification;
// MTK-END, SMS part

/// CC: M: call control part @{
import vendor.mediatek.hardware.mtkradioex.V1_0.IncomingCallNotification;
import vendor.mediatek.hardware.mtkradioex.V1_0.CipherNotification;
import vendor.mediatek.hardware.mtkradioex.V1_0.CrssNotification;
/// @}

// SS
import vendor.mediatek.hardware.mtkradioex.V1_0.CfuStatusNotification;

// M: [VzW] Data Framework @{
import vendor.mediatek.hardware.mtkradioex.V1_0.PcoDataAttachedInfo;
import com.mediatek.internal.telephony.dataconnection.PcoDataAfterAttached;
// M: [VzW] Data Framework @}

// External SIM [Start]
import com.android.internal.telephony.uicc.IccUtils;
import vendor.mediatek.hardware.mtkradioex.V1_0.VsimOperationEvent;
import com.mediatek.telephony.internal.telephony.vsim.ExternalSimManager.VsimEvent;
// External SIM [End]

/// Ims Data Framework @{
import com.mediatek.internal.telephony.ims.MtkDedicateDataCallResponse;
import vendor.mediatek.hardware.mtkradioex.V1_0.DedicateDataCall;
/// @}

/// [NW] @{
import android.telephony.SignalStrength;
import vendor.mediatek.hardware.mtkradioex.V1_0.SignalStrengthWithWcdmaEcio;
import android.telephony.SubscriptionManager;
/// @}

// DSBP enhancement
import vendor.mediatek.hardware.mtkradioex.V1_0.DsbpState;

// SIM
import vendor.mediatek.hardware.mtkradioex.V1_0.SimHotSwap;

import vendor.mediatek.hardware.mtkradioex.V1_0.CallInfoType;

public class MtkRadioExIndication extends MtkRadioExIndicationBase {
    private static final boolean ENG = "eng".equals(Build.TYPE);

    // TAG
    private static final String TAG = "MtkRadioInd";
    private MtkRIL mMtkRil;

    MtkRadioExIndication(RIL ril) {
        super(ril);
        mMtkRil = (MtkRIL) ril;
    }

    /* MTK SS */
    public void cfuStatusNotify(int indicationType, CfuStatusNotification cfuStatus) {
        mMtkRil.processIndication(indicationType);

        int[] notification = new int[2];
        notification[0] = cfuStatus.status;
        notification[1] = cfuStatus.lineId;

        if (MtkRIL.MTK_RILJ_LOGD) mMtkRil.unsljLogRet(RIL_UNSOL_CALL_FORWARDING, notification);

        if (notification[1] == 1) {
            mMtkRil.mCfuReturnValue = notification;
        }

        /* ONLY notify for Line 1 */
        if (mMtkRil.mCallForwardingInfoRegistrants.size() != 0 && notification[1] == 1) {
            /* Update mCfuReturnValue first */
            mMtkRil.mCallForwardingInfoRegistrants
                    .notifyRegistrants(new AsyncResult(null, notification, null));
        }
    }

    /// M: CC: call control related @{
    /// M: CC: incoming call notification handling
    public void incomingCallIndication(int indicationType, IncomingCallNotification inCallNotify) {
        mMtkRil.processIndication(indicationType);

        String[] notification = new String[7];
        notification[0] = inCallNotify.callId;
        notification[1] = inCallNotify.number;
        notification[2] = inCallNotify.type;
        notification[3] = inCallNotify.callMode;
        notification[4] = inCallNotify.seqNo;
        notification[5] = inCallNotify.redirectNumber;

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(RIL_UNSOL_INCOMING_CALL_INDICATION, null);
        }
        if (mMtkRil.mIncomingCallIndicationRegistrant != null) {
            mMtkRil.mIncomingCallIndicationRegistrant
                    .notifyRegistrant(new AsyncResult(null, notification, null));
        }

    }

    public void callAdditionalInfoInd(int indicationType,
            int ciType,
            ArrayList<String> info) {
        mMtkRil.processIndication(indicationType);

        String[] notification = new String[info.size() + 1];
        notification[0] = Integer.toString(ciType);
        for (int i = 0; i < info.size(); i++) {
            notification[i + 1] = info.get(i);
        }

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(RIL_UNSOL_CALL_ADDITIONAL_INFO, null);
        }


        if (mMtkRil.mCallAdditionalInfoRegistrants !=  null) {
            mMtkRil.mCallAdditionalInfoRegistrants
                    .notifyRegistrants(new AsyncResult(null, notification, null));
        }
    }

    /// M: CC: ciphering support notification
    public void cipherIndication(int indicationType, CipherNotification cipherNotify) {
        mMtkRil.processIndication(indicationType);

        String[] notification = new String[4];
        notification[0] = cipherNotify.simCipherStatus;
        notification[1] = cipherNotify.sessionStatus;
        notification[2] = cipherNotify.csStatus;
        notification[3] = cipherNotify.psStatus;

        if (MtkRIL.MTK_RILJ_LOGD) mMtkRil.unsljLogRet(RIL_UNSOL_CIPHER_INDICATION, notification);

        if (mMtkRil.mCipherIndicationRegistrants != null) {
            mMtkRil.mCipherIndicationRegistrants
                    .notifyRegistrants(new AsyncResult(null, notification, null));
        }

    }

    /// M: CC: CRSS notification handling
    public void suppSvcNotifyEx(int indicationType, SuppSvcNotification suppSvcNotification) {
        mMtkRil.processIndication(indicationType);

        MtkSuppServiceNotification notification = new MtkSuppServiceNotification();
        notification.notificationType = suppSvcNotification.isMT ? 1 : 0;
        notification.code = suppSvcNotification.code;
        notification.index = suppSvcNotification.index;
        notification.type = suppSvcNotification.type;
        notification.number = suppSvcNotification.number;

        if (MtkRIL.MTK_RILJ_LOGD) mMtkRil.unsljLogRet(
                RIL_UNSOL_SUPP_SVC_NOTIFICATION_EX, null);

        if (mMtkRil.mSsnExRegistrant != null) {
            mMtkRil.mSsnExRegistrant.notifyRegistrant(new AsyncResult(null, notification, null));
        }
    }

    public void crssIndication(int indicationType, CrssNotification crssNotification) {
        mMtkRil.processIndication(indicationType);

        MtkSuppCrssNotification notification = new MtkSuppCrssNotification();
        notification.code = crssNotification.code;
        notification.type = crssNotification.type;
        notification.alphaid = crssNotification.alphaid;
        notification.number = crssNotification.number;
        notification.cli_validity = crssNotification.cli_validity;

        if (MtkRIL.MTK_RILJ_LOGD) mMtkRil.unsljLogRet(RIL_UNSOL_CRSS_NOTIFICATION, null);

        if (mMtkRil.mCallRelatedSuppSvcRegistrant != null) {
            mMtkRil.mCallRelatedSuppSvcRegistrant
                    .notifyRegistrant(new AsyncResult(null, notification, null));
        }
    }

    public void eccNumIndication(int indicationType, String eccListWithCard, String eccListNoCard) {
    }

    private int getSubId(int phoneId) {
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        int[] subIds = SubscriptionManager.getSubId(phoneId);
        if (subIds != null && subIds.length > 0) {
            subId = subIds[0];
        }
        return subId;
    }

   // NW-START
    public void responseCsNetworkStateChangeInd(int indicationType,
            ArrayList<String> state) {
        mMtkRil.processIndication(indicationType);

        mMtkRil.riljLog("[UNSL]< " + "UNSOL_RESPONSE_CS_NETWORK_STATE_CHANGED");

        if (mMtkRil.mCsNetworkStateRegistrants.size() != 0) {
            mMtkRil.mCsNetworkStateRegistrants.notifyRegistrants(
                    new AsyncResult(null, state.toArray(new String[state.size()]), null));
        }
    }

    public void responsePsNetworkStateChangeInd(int indicationType,
            ArrayList<Integer> state) {
        mMtkRil.processIndication(indicationType);

        mMtkRil.riljLog("[UNSL]< " + "UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED");

        Object ret = null;
        int[] response = new int[state.size()];
        for (int i = 0; i < state.size(); i++) {
            response[i] = state.get(i);
        }
        ret = response;
        if (mMtkRil.mPsNetworkStateRegistrants.size() != 0) {
            mMtkRil.mPsNetworkStateRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    public void responseNetworkEventInd(int indicationType,
            ArrayList<Integer> event) {
        mMtkRil.processIndication(indicationType);

        Object ret = null;
        int[] response= new int[event.size()];

        for (int i = 0; i < event.size(); i++) {
             response[i] = event.get(i);
        }
        ret = response;

        mMtkRil.unsljLogRet(RIL_UNSOL_NETWORK_EVENT, ret);

        if (mMtkRil.mNetworkEventRegistrants.size()  !=  0) {
            mMtkRil.mNetworkEventRegistrants.notifyRegistrants(new AsyncResult(null, ret, null));
        }

    }

    public void networkRejectCauseInd(int indicationType, ArrayList<Integer> event) {
        mMtkRil.processIndication(indicationType);

        Object ret = null;
        int[] response= new int[event.size()];

        for (int i = 0; i < event.size(); i++) {
             response[i] = event.get(i);
        }
        ret = response;

        mMtkRil.unsljLogRet(RIL_UNSOL_NETWORK_REJECT_CAUSE, ret);

        if (mMtkRil.mNetworkRejectRegistrants.size() != 0) {
            mMtkRil.mNetworkRejectRegistrants.notifyRegistrants(new AsyncResult(null, ret, null));
        }

    }

    public void responseModulationInfoInd(int indicationType,
            ArrayList<Integer> data) {
        mMtkRil.processIndication(indicationType);

        Object ret = null;
        int[] response= new int[data.size()];

        for (int i = 0; i < data.size(); i++) {
             response[i] = data.get(i);
        }
        ret = response;

        mMtkRil.unsljLogRet(RIL_UNSOL_MODULATION_INFO, ret);

        if (mMtkRil.mModulationRegistrants.size()  !=  0) {
            mMtkRil.mModulationRegistrants.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    public void responseInvalidSimInd(int indicationType,
            ArrayList<String> state) {
        mMtkRil.processIndication(indicationType);
        String [] ret = state.toArray(new String[state.size()]);
        mMtkRil.unsljLogRet(RIL_UNSOL_INVALID_SIM, ret);

        if (mMtkRil.mInvalidSimInfoRegistrant.size()  !=  0) {
            mMtkRil.mInvalidSimInfoRegistrant.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * Indicates when radio state changes.
     * @param indicationType RadioIndicationType
     * @param info femtocell information
     */
    public void responseFemtocellInfo(int indicationType, ArrayList<String> info) {
        mMtkRil.processIndication(indicationType);

        Object ret = null;
        String[] response = info.toArray(new String[info.size()]);
        ret = response;

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(RIL_UNSOL_FEMTOCELL_INFO, ret);
        }

        if (mMtkRil.mFemtoCellInfoRegistrants.size() !=  0) {
            mMtkRil.mFemtoCellInfoRegistrants.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * Indicates the current signal strength of the camped or primary serving cell.
     */
    public void currentSignalStrengthWithWcdmaEcioInd(int indicationType,
            SignalStrengthWithWcdmaEcio signalStrength) {
        mMtkRil.processIndication(indicationType);
        // Todo: here should change to the signalStrength with Wcdma Ecio
        SignalStrength ss = new SignalStrength ();
        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(RIL_UNSOL_SIGNAL_STRENGTH_WITH_WCDMA_ECIO, ss);
            mMtkRil.riljLog("currentSignalStrengthWithWcdmaEcioInd SignalStrength=" + ss);
        }
        if (mMtkRil.mSignalStrengthWithWcdmaEcioRegistrants.size() != 0) {
            mMtkRil.mSignalStrengthWithWcdmaEcioRegistrants
                    .notifyRegistrants(new AsyncResult(null, ss, null));
        }
    }

    public void responseLteNetworkInfo(int indicationType, int info) {
        mMtkRil.riljLog("[UNSL]< " + "RIL_UNSOL_LTE_NETWORK_INFO " + info);
    }

    /**
     * Indicates when current resident network mccmnc changes.
     * @param indicationType RadioIndicationType
     * @param mccmnc current resident network mcc & mnc.
     */
    public void onMccMncChanged(int indicationType, String mccmnc) {
        mMtkRil.processIndication(indicationType);

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(RIL_UNSOL_MCCMNC_CHANGED, mccmnc);
        }

        if (mMtkRil.mMccMncRegistrants.size() != 0) {
            mMtkRil.mMccMncRegistrants.notifyRegistrants(new AsyncResult(null, mccmnc, null));
        }
    }
    // NW-END

    // MTK-START: SIM
    public void onVirtualSimOn(int indicationType, int simInserted) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_VIRTUAL_SIM_ON);

        if (mMtkRil.mVirtualSimOn != null) {
            mMtkRil.mVirtualSimOn.notifyRegistrants(
                                new AsyncResult(null, simInserted, null));
        }
    }

    public void onVirtualSimOff(int indicationType, int simInserted) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_VIRTUAL_SIM_OFF);

        if (mMtkRil.mVirtualSimOff != null) {
            mMtkRil.mVirtualSimOn.notifyRegistrants(
                                new AsyncResult(null, simInserted, null));
        }
    }

    public void onVirtualSimStatusChanged(int indicationType, int simInserted) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED);

        if (simInserted == 0 && mMtkRil.mVirtualSimOff != null) {
            mMtkRil.mVirtualSimOff.notifyRegistrants(
                                new AsyncResult(null, simInserted, null));
        } else if (simInserted == 1 && mMtkRil.mVirtualSimOn != null) {
            mMtkRil.mVirtualSimOn.notifyRegistrants(
                                new AsyncResult(null, simInserted, null));
        }
    }

    public void onImeiLock(int indicationType) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_IMEI_LOCK);

        if (mMtkRil.mImeiLockRegistrant != null) {
            mMtkRil.mImeiLockRegistrant.notifyRegistrants(
                                new AsyncResult(null, null, null));
        }
    }

    public void onImsiRefreshDone(int indicationType) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_IMSI_REFRESH_DONE);

        if (mMtkRil.mImsiRefreshDoneRegistrant != null) {
            mMtkRil.mImsiRefreshDoneRegistrant.notifyRegistrants(
                                new AsyncResult(null, null, null));
        }
    }

    public void onCardDetectedInd(int indicationType) {
        mMtkRil.processIndication(indicationType);

        if (ENG) {
            mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_CARD_DETECTED_IND);
        }

        if (mMtkRil.mCardDetectedIndRegistrant.size() != 0) {
            mMtkRil.mCardDetectedIndRegistrant.notifyRegistrants(new AsyncResult(
                    null, null, null));
        } else {
            // Phone process is not ready and cache it then wait register to notify
            if (ENG) {
                mMtkRil.riljLog("Cache card detected event");
            }
            mMtkRil.mIsCardDetected = true;
        }
    }
    // MTK-END:

    // SMS-START
    public void newEtwsInd(int indicationType, EtwsNotification etws) {
        mMtkRil.processIndication(indicationType);

        MtkEtwsNotification response = new MtkEtwsNotification();
        response.messageId = etws.messageId;
        response.serialNumber = etws.serialNumber;
        response.warningType = etws.warningType;
        response.plmnId = etws.plmnId;
        response.securityInfo = etws.securityInfo;

        if (ENG) mMtkRil.unsljLogRet(
                MtkRILConstants.RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION, response);

        if (mMtkRil.mEtwsNotificationRegistrant != null) {
            mMtkRil.mEtwsNotificationRegistrant.notifyRegistrant(
                    new AsyncResult(null, response, null));
        }
    }

    public void meSmsStorageFullInd(int indicationType) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_ME_SMS_STORAGE_FULL);

        if (mMtkRil.mMeSmsFullRegistrant != null) {
            mMtkRil.mMeSmsFullRegistrant.notifyRegistrant();
        }
    }

    public void smsReadyInd(int indicationType) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_SMS_READY_NOTIFICATION);

        if (mMtkRil.mSmsReadyRegistrants.size() != 0) {
            mMtkRil.mSmsReadyRegistrants.notifyRegistrants();
        } else {
            // Phone process is not ready and cache it then wait register to notify
            if (ENG) mMtkRil.riljLog("Cache sms ready event");
            mMtkRil.mIsSmsReady = true;
        }
    }
    // SMS-END

    // DATA
    public void dataAllowedNotification(int indicationType, int isAllowed) {
        mMtkRil.processIndication(indicationType);

        int response[] = new int[1];
        response[0] = isAllowed;

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogMore(RIL_UNSOL_DATA_ALLOWED, (isAllowed == 1) ? "true" : "false");
        }

        if (mMtkRil.mDataAllowedRegistrants != null) {
            mMtkRil.mDataAllowedRegistrants.notifyRegistrants(
                    new AsyncResult(null, response, null));
        }
    }

    // APC URC
    public void onPseudoCellInfoInd(int indicationType, ArrayList<Integer> info) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_PSEUDO_CELL_INFO);

        int[] response = new int[info.size()];
        for (int i = 0; i < info.size(); i++) {
            response[i] = info.get(i);
        }

        PseudoCellInfo cellInfo;
        String property = String.format("persist.vendor.radio.apc.mode%d", mMtkRil.mInstanceId);
        String propStr = SystemProperties.get(property, "0");
        int index = propStr.indexOf("=");
        if (index != -1){
            String subStr = propStr.substring(index + 1);
            String[] settings = subStr.split(",");
            int mode = Integer.parseInt(settings[0]);
            int report = Integer.parseInt(settings[1]);
            boolean enable = (report == 1) ? true : false;
            int interval = Integer.parseInt(settings[2]);
            cellInfo = new PseudoCellInfo(mode, enable, interval, response);
        } else {
            cellInfo = new PseudoCellInfo(0, false, 0, response);
        }

        if (mMtkRil.mPseudoCellInfoRegistrants != null) {
            mMtkRil.mPseudoCellInfoRegistrants.notifyRegistrants(
                    new AsyncResult(null, cellInfo, null));
        }
        //sendBroadcast apc infos
        Intent intent = new Intent(TelephonyIntents.ACTION_APC_INFO_NOTIFY);
        intent.putExtra(TelephonyIntents.EXTRA_APC_PHONE, mMtkRil.mInstanceId);
        intent.putExtra(TelephonyIntents.EXTRA_APC_INFO, cellInfo);
        mMtkRil.mMtkContext.sendBroadcast(intent);
        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.riljLog("Broadcast for APC info:cellInfo=" + cellInfo.toString());
        }
    }
    // M: eMBMS feature
    /*
     * Indicates of eMBMS session activate status
     *
     * @param indicationType RadioIndicationType
     * @param status Activated session:1, else 0
     */
    public void eMBMSSessionStatusIndication(int indicationType, int status) {
        mMtkRil.processIndication(indicationType);
        Object ret = null;
        int response[] = new int[1];
        response[0] = status;
        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(MtkRILConstants.RIL_UNSOL_EMBMS_SESSION_STATUS, ret);
        }

        if (mMtkRil.mEmbmsSessionStatusNotificationRegistrant.size() > 0) {
            if (MtkRIL.MTK_RILJ_LOGD) {
                mMtkRil.riljLog("Notify mEmbmsSessionStatusNotificationRegistrant");
            }
            mMtkRil.mEmbmsSessionStatusNotificationRegistrant.notifyRegistrants(
                new AsyncResult(null, response, null));
        } else {
            if (MtkRIL.MTK_RILJ_LOGD) {
                mMtkRil.riljLog("No mEmbmsSessionStatusNotificationRegistrant exist");
            }
        }

        Intent intent = new Intent(
            TelephonyIntents.ACTION_EMBMS_SESSION_STATUS_CHANGED);
        intent.putExtra(TelephonyIntents.EXTRA_IS_ACTIVE, status);
        mMtkRil.mMtkContext.sendBroadcast(intent);
    }

    // MTK-START: SIM HOT SWAP
    /**
     * URC for SIM hot swap indication.
     * @param indicationType RadioIndicationType.
     * @param event SIM hot swap type.
     * @param info SIM hot swap info.
     */
    public void onSimHotSwapInd(int indicationType, int event, String info) {
        mMtkRil.processIndication(indicationType);

        if (ENG) {
            mMtkRil.riljLog("onSimHotSwapInd event: " + event + " info: " + info);
        }

        switch (event) {
            case SimHotSwap.SIM_HOTSWAP_PLUG_IN:
                if (mMtkRil.mSimPlugIn != null) {
                    mMtkRil.mSimPlugIn.notifyRegistrants(new AsyncResult(null, null, null));
                }
            break;

            case SimHotSwap.SIM_HOTSWAP_PLUG_OUT:
                if (mMtkRil.mSimPlugOut != null) {
                    mMtkRil.mSimPlugOut.notifyRegistrants(new AsyncResult(null, null, null));
                }
            break;

            case SimHotSwap.SIM_HOTSWAP_RECOVERY:
                if (mMtkRil.mSimRecovery != null) {
                    mMtkRil.mSimRecovery.notifyRegistrants(new AsyncResult(null, null, null));
                }
            break;

            case SimHotSwap.SIM_HOTSWAP_MISSING:
                if (mMtkRil.mSimMissing != null) {
                    mMtkRil.mSimMissing.notifyRegistrants(new AsyncResult(null, null, null));
                }
            break;

            case SimHotSwap.SIM_HOTSWAP_TRAY_PLUG_IN:
                if (mMtkRil.mSimTrayPlugIn != null) {
                    mMtkRil.mSimTrayPlugIn.notifyRegistrants(new AsyncResult(null, null, null));
                }
            break;

            case SimHotSwap.SIM_HOTSWAP_COMMONSLOT_NO_CHANGED:
                if (mMtkRil.mSimCommonSlotNoChanged != null) {
                    mMtkRil.mSimCommonSlotNoChanged.notifyRegistrants(
                            new AsyncResult(null, null, null));
                }
            break;

            default :
                mMtkRil.riljLog("onSimHotSwapInd Invalid event!");
            break;
        }
    }
    // MTK-END

    // MTK-START: SIM POWER
    /**
     * URC for SIM power indication.
     * @param indicationType RadioIndicationType.
     * @param info SIM power state.
     */
    public void onSimPowerChangedInd(int indicationType, ArrayList<Integer> info) {
        mMtkRil.processIndication(indicationType);

        if (ENG) {
            mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_SIM_POWER_CHANGED);
        }

        Object ret = null;
        int[] response = new int[info.size()];
        for (int i = 0; i < info.size(); i++) {
            response[i] = info.get(i);
        }
        ret = response;

        mMtkRil.mSimPowerInfo = ret;
        if (mMtkRil.mSimPowerChanged.size() != 0) {
            mMtkRil.mSimPowerChanged.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }
    // MTK-END
    // MTK-START: SIM ME LOCK
    public void smlSlotLockInfoChangedInd(int indicationType, ArrayList<Integer> info) {
        mMtkRil.processIndication(indicationType);

        if (ENG) {
            mMtkRil.unsljLog(
                    MtkRILConstants.RIL_UNSOL_SIM_SLOT_LOCK_POLICY_NOTIFY);
        }

        Object ret = null;
        int[] response = new int[info.size()];
        for (int i = 0; i < info.size(); i++) {
            response[i] = info.get(i);
        }
        ret = response;

        mMtkRil.mSmlSlotLockInfo = ret;
        if (mMtkRil.mSmlSlotLockInfoChanged.size() != 0) {
            mMtkRil.mSmlSlotLockInfoChanged.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }
    // MTK-END
    // MTK-START: SIM RSU
    /**
     * Indicates when modem report event.
     * @param indicationType RadioIndicationType
     * @param eventId event identification
     */
    public void onRsuSimLockEvent(int indicationType, int eventId) {
        mMtkRil.processIndication(indicationType);
        mMtkRil.riljLog("[RSU-SIMLOCK] onRsuSimLockEvent eventId " + eventId);
        if (ENG) {
            mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_RSU_SIM_LOCK_NOTIFICATION);
        }

        int response[] = new int[1];
        response[0] = eventId;

        if (mMtkRil.mRsuSimlockRegistrants != null) {
            mMtkRil.mRsuSimlockRegistrants.notifyRegistrants(
                    new AsyncResult(null, response, null));
        }
    }
    // MTK-END

    /*
     * Indicates of eMBMS AT command event
     *
     * @param indicationType RadioIndicationType
     * @param info Information AT command string
     */
    public void eMBMSAtInfoIndication(int indicationType, String info) {
        mMtkRil.processIndication(indicationType);
        Object ret = null;
        String response = new String(info);
        ret = response;
        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(MtkRILConstants.RIL_UNSOL_EMBMS_AT_INFO, ret);
        }

        if (mMtkRil.mEmbmsAtInfoNotificationRegistrant.size() > 0) {
            if (MtkRIL.MTK_RILJ_LOGD) {
                mMtkRil.riljLog("Notify mEmbmsAtInfoNotificationRegistrant");
            }
            mMtkRil.mEmbmsAtInfoNotificationRegistrant.notifyRegistrants(
                new AsyncResult(null, ret, null));
        } else {
            if (MtkRIL.MTK_RILJ_LOGD) {
                mMtkRil.riljLog("No mEmbmsAtInfoNotificationRegistrant exist");
            }
        }
    }
    /// M: eMBMS end

    /**
     * Indicates when PLMN Changed.
     * @param indicationType RadioIndicationType
     * @param plmns ArrayList<String>
     */
    public void plmnChangedIndication(int indicationType, ArrayList<String> plmns) {
        mMtkRil.processIndication(indicationType);
        Object ret = null;
        String[] response = new String[plmns.size()];
        for (int i = 0; i < plmns.size(); i++) {
            response[i] = plmns.get(i);
        }
        ret = response;
        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(RIL_UNSOL_RESPONSE_PLMN_CHANGED, ret);
        }
        synchronized (mMtkRil.mWPMonitor) {
            if (mMtkRil.mPlmnChangeNotificationRegistrant.size() > 0) {
                if (MtkRIL.MTK_RILJ_LOGD) {
                    mMtkRil.riljLog("ECOPS,notify mPlmnChangeNotificationRegistrant");
                }
                mMtkRil.mPlmnChangeNotificationRegistrant.notifyRegistrants(
                    new AsyncResult(null, ret, null));
            } else {
                mMtkRil.mEcopsReturnValue = ret;
            }
        }
    }

    /**
     * Indicates when need to registrtion.
     * @param indicationType RadioIndicationType
     * @param sessionIds ArrayList<Integer>
     */
    public void registrationSuspendedIndication(int indicationType, ArrayList<Integer> sessionIds) {
        mMtkRil.processIndication(indicationType);
        Object ret = null;
        int[] response = new int[sessionIds.size()];
        for (int i = 0; i < sessionIds.size(); i++) {
            response[i] = sessionIds.get(i);
        }
        ret = response;
        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED, ret);
        }
        synchronized (mMtkRil.mWPMonitor) {
            if (mMtkRil.mRegistrationSuspendedRegistrant != null) {
                if (MtkRIL.MTK_RILJ_LOGD) {
                    mMtkRil.riljLog("EMSR, notify mRegistrationSuspendedRegistrant");
                }
                mMtkRil.mRegistrationSuspendedRegistrant.notifyRegistrant(
                    new AsyncResult(null, ret, null));
            } else {
                mMtkRil.mEmsrReturnValue = ret;
            }
        }
    }

    /**
     * Indicates when GMSS Rat changed.
     * @param indicationType RadioIndicationType
     * @param gmsss ArrayList<Integer>
     */
    public void gmssRatChangedIndication(int indicationType, ArrayList<Integer> gmsss) {
        mMtkRil.processIndication(indicationType);
        Object ret = null;
        int[] response = new int[gmsss.size()];
        for (int i = 0; i < gmsss.size(); i++) {
            response[i] = gmsss.get(i);
        }
        ret = response;
        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(RIL_UNSOL_GMSS_RAT_CHANGED, ret);
        }
        int[] rat = (int[]) ret;
        if (mMtkRil.mGmssRatChangedRegistrant != null) {
            mMtkRil.mGmssRatChangedRegistrant.notifyRegistrants(new AsyncResult(null, rat, null));
        }
    }

    /**
     * Indicates when modem trigger world mode.
     * @param indicationType RadioIndicationType
     * @param modes ArrayList<Integer>
     */
    public void worldModeChangedIndication(int indicationType, ArrayList<Integer> modes) {
        mMtkRil.processIndication(indicationType);
        Object ret = null;
        int[] response = new int[modes.size()];
        for (int i = 0; i < modes.size(); i++) {
            response[i] = modes.get(i);
        }
        ret = response;
        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(RIL_UNSOL_WORLD_MODE_CHANGED, ret);
        }
        int state = 1;
        boolean retvalue = false;
        if (ret != null) {
            state = ((int[]) ret)[0];
            //update switching state
            if (state == 2) { //rild init send end urc
                retvalue = WorldMode.resetSwitchingState(state);
                state = 1;
            } else if (state == 0) {
                retvalue = WorldMode.updateSwitchingState(true);
            } else {
                retvalue = WorldMode.updateSwitchingState(false);
            }
            if (false == retvalue) {
                return;
            }
            //sendBroadcast with state
            Intent intent = new Intent(WorldMode.ACTION_WORLD_MODE_CHANGED);
            intent.putExtra(WorldMode.EXTRA_WORLD_MODE_CHANGE_STATE, (Integer) state);
            mMtkRil.mMtkContext.sendBroadcast(intent);
            if (MtkRIL.MTK_RILJ_LOGD) {
                mMtkRil.riljLog("Broadcast for WorldModeChanged: state=" + state);
            }
        }
    }

    /**
     * Indicates when reset attach APN
     * @param indicationType RadioIndicationType
     */
    public void resetAttachApnInd(int indicationType) {
        mMtkRil.processIndication(indicationType);


        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_RESET_ATTACH_APN);

        if (mMtkRil.mResetAttachApnRegistrants != null) {
            mMtkRil.mResetAttachApnRegistrants.notifyRegistrants(
                                new AsyncResult(null, null, null));
        }
    }

    /**
     * Indicates when modem changes attach APN
     * @param indicationType RadioIndicationType
     * @param apnClassType class type for APN
     */
    public void mdChangedApnInd(int indicationType, int apnClassType) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_DATA_ATTACH_APN_CHANGED);

        if (mMtkRil.mAttachApnChangedRegistrants != null) {
            mMtkRil.mAttachApnChangedRegistrants.notifyRegistrants(
                                new AsyncResult(null, apnClassType, null));
        }
    }

    @Override
    public void esnMeidChangeInd(int indicationType, String esnMeid) {
        mMtkRil.processIndication(indicationType);

        if (ENG) {
            mMtkRil.unsljLog(
                    MtkRILConstants.RIL_UNSOL_CDMA_CARD_INITIAL_ESN_OR_MEID);
        }

        if (mMtkRil.mCDMACardEsnMeidRegistrant != null) {
            mMtkRil.mCDMACardEsnMeidRegistrant.notifyRegistrant(
                    new AsyncResult(null, esnMeid, null));
        } else {
            if (ENG) {
                mMtkRil.riljLog("Cache esnMeidChangeInd");
            }
            mMtkRil.mEspOrMeid = (Object) esnMeid;
        }
    }

    public void phbReadyNotification(int indicationType, int isPhbReady) {
        mMtkRil.processIndication(indicationType);

        int response[] = new int[1];
        response[0] = isPhbReady;

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogMore(RIL_UNSOL_PHB_READY_NOTIFICATION,
                    "phbReadyNotification: " + isPhbReady);
        }

        if (((MtkRIL) mMtkRil).mPhbReadyRegistrants != null) {
            ((MtkRIL) mMtkRil).mPhbReadyRegistrants.notifyRegistrants(
                    new AsyncResult(null, response, null));
        }
    }

    // / M: BIP {
    public void bipProactiveCommand(int indicationType, String cmd) {
        mMtkRil.processIndication(indicationType);

        if (ENG) {
            mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND);
        }

        if (mMtkRil.mBipProCmdRegistrant != null) {
            mMtkRil.mBipProCmdRegistrant.notifyRegistrants(new AsyncResult (null, cmd, null));
        }
    }
    // / M: BIP }
    // / M: OTASP {
    public void triggerOtaSP(int indicationType) {
        String[] testTriggerOtasp = new String[3];
        testTriggerOtasp[0] = "AT+CDV=*22899";
        testTriggerOtasp[1] = "";
        testTriggerOtasp[2] = "DESTRILD:C2K";
        mMtkRil.invokeOemRilRequestStrings(testTriggerOtasp, null);
    }
    // M: OTASP }

    // M: [VzW] Data Framework @{
    public void pcoDataAfterAttached(int indicationType, PcoDataAttachedInfo pco) {
        mMtkRil.processIndication(indicationType);

        PcoDataAfterAttached response = new PcoDataAfterAttached(pco.cid,
                pco.apnName,
                pco.bearerProto,
                pco.pcoId,
                RIL.arrayListToPrimitiveArray(pco.contents));

        if (MtkRIL.MTK_RILJ_LOGD) mMtkRil.unsljLogRet(RIL_UNSOL_PCO_DATA_AFTER_ATTACHED, response);

        mMtkRil.mPcoDataAfterAttachedRegistrants.notifyRegistrants(
                new AsyncResult(null, response, null));
    }
    // M: [VzW] Data Framework @}

    // / M: STK {
    public void onStkMenuReset(int indicationType) {
        mMtkRil.processIndication(indicationType);

        if (ENG) {
            mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_STK_SETUP_MENU_RESET);
        }

        if (mMtkRil.mStkSetupMenuResetRegistrant != null) {
            mMtkRil.mStkSetupMenuResetRegistrant.notifyRegistrants(
                    new AsyncResult (null, null, null));
        }
    }
    // / M: STK }

    // M: [LTE][Low Power][UL traffic shaping] @{
    public void onLteAccessStratumStateChanged(int indicationType, ArrayList<Integer> state) {
        mMtkRil.processIndication(indicationType);

        int[] response = new int[state.size()];
        for (int i = 0; i < state.size(); i++) {
            response[i] = state.get(i);
        }

        if (MtkRIL.MTK_RILJ_LOGD) mMtkRil.unsljLogRet(
                RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE, response);

        if (mMtkRil.mLteAccessStratumStateRegistrants != null) {
            mMtkRil.mLteAccessStratumStateRegistrants.notifyRegistrants(
                    new AsyncResult (null, response, null));
        }
    }
    // M: [LTE][Low Power][UL traffic shaping] @}

    public void networkInfoInd(int indicationType, ArrayList<String> networkinfo) {
        mMtkRil.processIndication(indicationType);

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogMore(RIL_UNSOL_NETWORK_INFO, "networkInfo: " +
                    networkinfo);
        }

        String [] ret = networkinfo.toArray(new String[networkinfo.size()]);

        if (mMtkRil.mNetworkInfoRegistrant.size() !=  0) {
            mMtkRil.mNetworkInfoRegistrant.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    // M: Data Framework - Data Retry enhancement
    public void onMdDataRetryCountReset(int indicationType) {
        mMtkRil.processIndication(indicationType);

        if (ENG) {
            mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_MD_DATA_RETRY_COUNT_RESET);
        }

        if (mMtkRil.mMdDataRetryCountResetRegistrants != null) {
            mMtkRil.mMdDataRetryCountResetRegistrants.notifyRegistrants(
                                new AsyncResult(null, null, null));
        }
    }

    // M: Data Framework - CC 33
    public void onRemoveRestrictEutran(int indicationType) {
        mMtkRil.processIndication(indicationType);

        if (ENG) {
            mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_REMOVE_RESTRICT_EUTRAN);
        }

        if (mMtkRil.mRemoveRestrictEutranRegistrants!= null) {
            mMtkRil.mRemoveRestrictEutranRegistrants.notifyRegistrants(
                                new AsyncResult(null, null, null));
        }
    }

    @Override
    public void confSRVCC(int indicationType, ArrayList<Integer> callIds) {
        mMtkRil.processIndication(indicationType);

        if (MtkRIL.MTK_RILJ_LOGD) mMtkRil.unsljLog(RIL_UNSOL_ECONF_SRVCC_INDICATION);

        int[] response = new int[callIds.size()];
        for (int i = 0; i < callIds.size(); i++) {
            response[i] = callIds.get(i);
        }
        mMtkRil.mEconfSrvccRegistrants.notifyRegistrants(new AsyncResult(null, response, null));
    }

    // External SIM [Start]
    public void onVsimEventIndication(int indicationType, VsimOperationEvent event) {
        mMtkRil.processIndication(indicationType);

        if (MtkRIL.MTK_RILJ_LOGV) {
            mMtkRil.unsljLogRet(
                MtkRILConstants.RIL_UNSOL_VSIM_OPERATION_INDICATION,
                "len=" + new Integer(event.dataLength));
        }

        int length = ((event.dataLength > 0) ? (event.dataLength / 2 + 4) : 0);

        VsimEvent indicationEvent = new VsimEvent(
                event.transactionId, event.eventId, length,
                1 << mMtkRil.mInstanceId);
        if (length > 0) {
            indicationEvent.putInt(event.dataLength / 2);
            indicationEvent.putBytes(IccUtils.hexStringToBytes(event.data));
        }

        if (ENG) mMtkRil.unsljLogRet(
                MtkRILConstants.RIL_UNSOL_VSIM_OPERATION_INDICATION, indicationEvent.toString());

        if (mMtkRil.mVsimIndicationRegistrants != null) {
            mMtkRil.mVsimIndicationRegistrants.notifyRegistrants(
                    new AsyncResult(null, indicationEvent, null));
        }
    }
    // External SIM [End]

    /// Ims Data Framework {@
    public void dedicatedBearerActivationInd(int indicationType, DedicateDataCall ddcResult) {
        mMtkRil.processIndication(indicationType);

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLog(RIL_UNSOL_DEDICATE_BEARER_ACTIVATED);
        }

        MtkDedicateDataCallResponse ret = mMtkRil.convertDedicatedDataCallResult(ddcResult);
        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.riljLog(ret.toString());
        }

        if (mMtkRil.mDedicatedBearerActivedRegistrants != null) {
            mMtkRil.mDedicatedBearerActivedRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    public void dedicatedBearerModificationInd(int indicationType, DedicateDataCall ddcResult) {
        mMtkRil.processIndication(indicationType);

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLog(RIL_UNSOL_DEDICATE_BEARER_MODIFIED);
        }

        MtkDedicateDataCallResponse ret = mMtkRil.convertDedicatedDataCallResult(ddcResult);
        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.riljLog(ret.toString());
        }

        if (mMtkRil.mDedicatedBearerModifiedRegistrants != null) {
            mMtkRil.mDedicatedBearerModifiedRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    public void dedicatedBearerDeactivationInd(int indicationType, int cid) {
        mMtkRil.processIndication(indicationType);

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLog(RIL_UNSOL_DEDICATE_BEARER_DEACTIVATED);
        }

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.riljLog("dedicatedBearerDeactivationInd, cid: " + cid);
        }

        if (mMtkRil.mDedicatedBearerDeactivatedRegistrants != null) {
            mMtkRil.mDedicatedBearerDeactivatedRegistrants.notifyRegistrants(
                    new AsyncResult(null, cid, null));
        }
    }
    /// @}

    /**
     * @param indicationType RadioIndicationType
     * @param data Data sent by oem
     */
    public void oemHookRaw(int indicationType, ArrayList<Byte> data) {
        mMtkRil.processIndication(indicationType);

        byte[] response = RIL.arrayListToPrimitiveArray(data);
        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogvRet(RIL_UNSOL_OEM_HOOK_RAW,
                    com.android.internal.telephony.uicc.IccUtils.bytesToHexString(response));
        }

        if (mMtkRil.mUnsolOemHookRegistrant != null) {
            mMtkRil.mUnsolOemHookRegistrant.notifyRegistrant(
                    new AsyncResult(null, response, null));
        }
    }

    /**
     * Indicates TX power
     * @param indicationType RadioIndicationType
     * @param txPower ArrayList<Integer>
     */
    public void onTxPowerIndication(int indicationType, ArrayList<Integer> txPower) {
        mMtkRil.processIndication(indicationType);
        int[] response = new int[txPower.size()];
        for (int i = 0; i < txPower.size(); i++) {
            response[i] = txPower.get(i);
        }

        if (((MtkRIL) mMtkRil).mTxPowerRegistrant != null) {
            ((MtkRIL) mMtkRil).mTxPowerRegistrant.notifyRegistrants(
                    new AsyncResult(null, response, null));
        }
    }

    /**
     * URC for Tx power reduction
     * @param indicationType RadioIndicationType
     * @param txPower ArrayList<Integer>
     */
    public void onTxPowerStatusIndication(int indicationType, ArrayList<Integer> txPower) {
        mMtkRil.processIndication(indicationType);
        int[] response = new int[txPower.size()];
        for (int i = 0; i < txPower.size(); i++) {
            response[i] = txPower.get(i);
        }

        if (((MtkRIL) mMtkRil).mTxPowerStatusRegistrant != null) {
            ((MtkRIL) mMtkRil).mTxPowerStatusRegistrant.notifyRegistrants(
                    new AsyncResult(null, response, null));
        }
    }

    /**
     * Indicates when dsbp changes.
     * @param indicationType RadioIndicationType
     * @param DsbpState android.hardware.mtkradioex.V1_0.DsbpState
     */
    public void dsbpStateChanged(int indicationType, int dsbpState) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_DSBP_STATE_CHANGED);

        mMtkRil.riljLog("dsbpStateChanged state: " + dsbpState);
        if (mMtkRil.mDsbpStateRegistrant != null) {
            mMtkRil.mDsbpStateRegistrant.notifyRegistrants(
                    new AsyncResult(null, dsbpState, null));
        }
    }

    public void onDsdaChangedInd(int indicationType, int mode) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_ON_DSDA_CHANGED);

        mMtkRil.riljLog("onDsdaChangedInd: mode=" + mode);
        if (mMtkRil.mDsdaStateRegistrant != null) {
            mMtkRil.mDsdaStateRegistrant.notifyRegistrants(
                    new AsyncResult(null, mode, null));
        }
    }

    public void qualifiedNetworkTypesChangedInd(int indicationType, ArrayList<Integer> data) {
        mMtkRil.processIndication(indicationType);

        int[] response = new int[data.size()];
        for (int i = 0; i < data.size(); i++) {
            response[i] = data.get(i);
        }

        if (MtkRIL.MTK_RILJ_LOGD) {
            mMtkRil.unsljLogRet(MtkRILConstants.RIL_UNSOL_QUALIFIED_NETWORK_TYPES_CHANGED,
                    response);
        }

        if (mMtkRil.mQualifiedNetworkTypesRegistrant != null) {
            mMtkRil.mQualifiedNetworkTypesRegistrant.notifyRegistrants(
                    new AsyncResult(null, response, null));
        }
    }

    /**
     * <signal type>:  integer
     * 0    LTE RSRP
     * 1    LTE RS SNR
     * 2    UMTS RSCP
     * 3    UMTS ECNO
     *
     * CELLULAR_QUALITY_LTE_RSRP = 0;
     * CELLULAR_QUALITY_UMTS_RSSI = 1;
     * CELLULAR_QUALITY_UMTS_RSCP = 2;
     * CELLULAR_QUALITY_GSM_RSSI = 3;
     * CELLULAR_QUALITY_CDMA1X_ECIO = 4;
     * CELLULAR_QUALITY_LTE_SNR = 5;
     * CELLULAR_QUALITY_LTE_ECIO = 6;
     * CELLULAR_QUALITY_LTE_RSRQ = 7;
     *
     */
    public void onCellularQualityChangedInd(int indicationType, ArrayList<Integer> indStgs) {
        mMtkRil.processIndication(indicationType);

        if (ENG) mMtkRil.unsljLog(
                MtkRILConstants.RIL_UNSOL_IWLAN_CELLULAR_QUALITY_CHANGED_IND);

        int[] data = new int[indStgs.size()];
        for (int i = 0; i < indStgs.size(); i++) {
            data[i] = indStgs.get(i);
        }

        int CellularQualityType;
        switch (data[0]) {
            case 0:
                CellularQualityType = 0; // CELLULAR_QUALITY_LTE_RSRP
                break;
            case 1:
                CellularQualityType = 5; // CELLULAR_QUALITY_LTE_SNR
                break;
            case 2:
                CellularQualityType = 2; // CELLULAR_QUALITY_UMTS_RSCP
                break;
            case 3:
                CellularQualityType = 6; // CELLULAR_QUALITY_LTE_ECIO
                break;
            case 4:
                CellularQualityType = 7; // CELLULAR_QUALITY_LTE_RSRQ
                break;
            default:
                CellularQualityType = 0;
                break;
        }

        mMtkRil.riljLog("RIL_UNSOL_IWLAN_CELLULAR_QUALITY_CHANGED_IND type:" + CellularQualityType
                + " , value = " + data[1]);
    }

    public void mobileDataUsageInd(int indicationType, ArrayList<Integer> data) {
        mMtkRil.processIndication(indicationType);

        int[] response = new int[data.size()];
        for (int i = 0; i < data.size(); i++) {
            response[i] = data.get(i);
        }

        if (MtkRIL.MTK_RILJ_LOGD) mMtkRil.unsljLogRet(RIL_UNSOL_MOBILE_DATA_USAGE, response);

        if (mMtkRil.mMobileDataUsageRegistrants != null) {
            mMtkRil.mMobileDataUsageRegistrants.notifyRegistrants(
                    new AsyncResult(null, response, null));
        }
    }

    public void onNwLimitInd(int indicationType, ArrayList<Integer> state) {
        mMtkRil.processIndication(indicationType);

        int[] response = new int[state.size()];
        for (int i = 0; i < state.size(); i++) {
            response[i] = state.get(i);
        }

        if (MtkRIL.MTK_RILJ_LOGD) mMtkRil.unsljLogRet(RIL_UNSOL_NW_LIMIT, response);

        if (mMtkRil.mNwLimitRegistrants != null) {
            mMtkRil.mNwLimitRegistrants.notifyRegistrants(
                    new AsyncResult (null, response, null));
        }
    }

    public void onRsuEvent(int type, int eventId, String eventString) {
        mMtkRil.processIndication(type);
        mMtkRil.riljLog("[RSU-SIMLOCK] onRsuEvent eventId: " + eventId
                + ", eventString: " + eventString);
        if (ENG) {
            mMtkRil.unsljLog(MtkRILConstants.RIL_UNSOL_RSU_SIM_LOCK_NOTIFICATION);
        }

        int response[] = new int[1];
        response[0] = eventId;

        if (mMtkRil.mRsuSimlockRegistrants != null) {
            mMtkRil.mRsuSimlockRegistrants.notifyRegistrants(
                    new AsyncResult(null, response, null));
        }
    }
}

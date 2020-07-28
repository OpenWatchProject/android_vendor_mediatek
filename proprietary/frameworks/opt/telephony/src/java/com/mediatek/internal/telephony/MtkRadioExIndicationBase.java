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

import com.android.internal.telephony.RIL;

import vendor.mediatek.hardware.mtkradioex.V1_9.IMtkRadioExIndication;

import android.hardware.radio.V1_0.CdmaCallWaiting;
import android.hardware.radio.V1_0.CdmaSignalInfoRecord;
import android.hardware.radio.V1_0.CdmaSmsMessage;
import android.hardware.radio.V1_0.LceDataInfo;
import android.hardware.radio.V1_0.PcoDataInfo;
import android.hardware.radio.V1_0.SetupDataCallResult;
import android.hardware.radio.V1_0.SimRefreshResult;
import android.hardware.radio.V1_0.StkCcUnsolSsResult;
import android.hardware.radio.V1_0.SuppSvcNotification;
import android.hardware.radio.V1_1.KeepaliveStatus;

import java.util.ArrayList;

// MTK-START, SMS part
import vendor.mediatek.hardware.mtkradioex.V1_0.EtwsNotification;
// MTK-END, SMS part

/// CC: M: call control part @{
import vendor.mediatek.hardware.mtkradioex.V1_0.IncomingCallNotification;
import vendor.mediatek.hardware.mtkradioex.V1_0.CipherNotification;
import vendor.mediatek.hardware.mtkradioex.V1_0.CrssNotification;
/// @}

//SS
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

// M: Ims Data Framework
import vendor.mediatek.hardware.mtkradioex.V1_0.DedicateDataCall;

// NW
import vendor.mediatek.hardware.mtkradioex.V1_0.SignalStrengthWithWcdmaEcio;

// DSBP enhancement
import vendor.mediatek.hardware.mtkradioex.V1_0.DsbpState;

import vendor.mediatek.hardware.mtkradioex.V1_0.CallInfoType;

public class MtkRadioExIndicationBase extends IMtkRadioExIndication.Stub  {

    MtkRadioExIndicationBase(RIL ril) {
    }

    public void currentSignalStrengthWithWcdmaEcioInd(int indicationType,
            SignalStrengthWithWcdmaEcio signalStrength) {
    }

    /* MTK SS */
    public void cfuStatusNotify(int indicationType, CfuStatusNotification cfuStatus) {
    }

    /// M: CC: call control related @{
    /// M: CC: incoming call notification handling
    public void incomingCallIndication(int indicationType, IncomingCallNotification inCallNotify) {
    }

    public void callAdditionalInfoInd(int indicationType,
           int ciType, ArrayList<String> info) {
    }

    /// M: CC: ciphering support notification
    public void cipherIndication(int indicationType, CipherNotification cipherNotify) {
    }

    /// M: CC: CRSS notification handling
    public void suppSvcNotifyEx(int indicationType, SuppSvcNotification suppSvc) {
    }

    public void crssIndication(int indicationType, CrssNotification crssNotification) {
    }

    /// M: CC: CDMA call accepted notification handling @{
    public void cdmaCallAccepted(int indicationType) {
    }
    /// @}

    public void eccNumIndication(int indicationType, String eccListWithCard, String eccListNoCard) {
    }

   // NW-START
    public void responseCsNetworkStateChangeInd(int indicationType,
            ArrayList<String> state) {
    }

    public void responsePsNetworkStateChangeInd(int indicationType,
            ArrayList<Integer> state) {
    }

    public void responseNetworkEventInd(int indicationType,
            ArrayList<Integer> event) {
    }

    public void responseModulationInfoInd(int indicationType,
            ArrayList<Integer> data) {
    }

    public void responseInvalidSimInd(int indicationType,
            ArrayList<String> state) {
    }

    public void responseFemtocellInfo(int indicationType, ArrayList<String> info) {
    }

    public void responseLteNetworkInfo(int indicationType, int info) {
    }

    public void onMccMncChanged(int indicationType, String mccmnc) {
    }
    // NW-END

    // MTK-START: SIM
    public void onVirtualSimOn(int indicationType, int simInserted) {
    }

    public void onVirtualSimOff(int indicationType, int simInserted) {
    }

    public void onVirtualSimStatusChanged(int indicationType, int simInserted) {
    }

    public void onImeiLock(int indicationType) {
    }

    public void onImsiRefreshDone(int indicationType) {
    }

    public void onCardDetectedInd(int indicationType) {
    }
    // MTK-END:

    // SMS-START
    public void newEtwsInd(int indicationType, EtwsNotification etws) {
    }

    public void meSmsStorageFullInd(int indicationType) {
    }

    public void smsReadyInd(int indicationType) {
    }
    // SMS-END

    // DATA
    public void dataAllowedNotification(int indicationType, int isAllowed) {
    }

    // APC URC
    public void onPseudoCellInfoInd(int indicationType, ArrayList<Integer> info) {
    }
    // M: eMBMS feature
    /*
     * Indicates of eMBMS session activate status
     *
     * @param indicationType RadioIndicationType
     * @param status Activated session:1, else 0
     */
    public void eMBMSSessionStatusIndication(int indicationType, int status) {
    }

    /*
     * Indicates of eMBMS AT command event
     *
     * @param indicationType RadioIndicationType
     * @param info Information AT command string
     */
    public void eMBMSAtInfoIndication(int indicationType, String info) {
    }
    /// M: eMBMS end


    /**
     * Indicates when PLMN Changed.
     * @param indicationType RadioIndicationType
     * @param plmns ArrayList<String>
     */
    public void plmnChangedIndication(int indicationType, ArrayList<String> plmns) {
    }

    /**
     * Indicates when need to registrtion.
     * @param indicationType RadioIndicationType
     * @param sessionIds ArrayList<Integer>
     */
    public void registrationSuspendedIndication(int indicationType, ArrayList<Integer> sessionIds) {
    }

    /**
     * Indicates when GMSS Rat changed.
     * @param indicationType RadioIndicationType
     * @param gmsss ArrayList<Integer>
     */
    public void gmssRatChangedIndication(int indicationType, ArrayList<Integer> gmsss) {
    }

    /**
     * Indicates when modem trigger world mode.
     * @param indicationType RadioIndicationType
     * @param modes ArrayList<Integer>
     */
    public void worldModeChangedIndication(int indicationType, ArrayList<Integer> modes) {
    }

    /**
     * Indicates when reset attach APN
     * @param indicationType RadioIndicationType
     */
    public void resetAttachApnInd(int indicationType) {
    }

    /**
     * Indicates when modem changes attach APN
     * @param indicationType RadioIndicationType
     * @param apnClassType class type for APN
     */
    public void mdChangedApnInd(int indicationType, int apnClassType) {
    }

    /**
     * ESN or MEID change indication.
     *
     * @param indicationType the Radio Indication Type
     * @param esnMeid the ESN or MEID string, format is "old,new"
     */
    public void esnMeidChangeInd(int indicationType, String esnMeid) {
    }

    public void phbReadyNotification(int indicationType, int isPhbReady) {
    }

    // / M: BIP {
    public void bipProactiveCommand(int indicationType, String cmd) {
    }
    // / M: BIP }
    // / M: OTASP {
    public void triggerOtaSP(int indicationType) {
    }
    // / M: OTASP }

    // / M: STK {
    public void onStkMenuReset(int indicationType) {
    }
    // / M: STK }

    public void onMdDataRetryCountReset(int indicationType) {}

    public void onRemoveRestrictEutran(int indicationType) {}

    // M: [LTE][Low Power][UL traffic shaping] @{
    public void onLteAccessStratumStateChanged(int indicationType, ArrayList<Integer> state) {}
    // M: [LTE][Low Power][UL traffic shaping] @}

    // MTK-START: SIM HOT SWAP
    /**
     * URC for SIM hot swap indication.
     * @param indicationType RadioIndicationType.
     * @param event SIM hot swap type.
     * @param info SIM hot swap info.
     */
    public void onSimHotSwapInd(int indicationType, int event, String info) {}

    // MTK-START: SIM POWER
    /**
     * URC for SIM power indication.
     * @param indicationType RadioIndicationType.
     * @param info SIM power state.
     */
    public void onSimPowerChangedInd(int indicationType, ArrayList<Integer> info) {
    }
    // MTK-END
    // MTK-START: SIM RSU
    /**
     * Indicates when modem report event.
     * @param indicationType RadioIndicationType
     * @param eventId event identification
     */
    public void onRsuSimLockEvent(int indicationType, int eventId) {
    }
    // MTK-END
    // MTK-START: SIM ME LOCK
    public void smlSlotLockInfoChangedInd(int indicationType, ArrayList<Integer> info) {
    }
    // MTK-END

    public void networkInfoInd(int indicationType, ArrayList<String> networkinfo) {
    }
    // MTK-START: SIM TMO RSU
    public void onSimMeLockEvent(int indicationType) {
    }
    // MTK-END

    // M: [VzW] Data Framework
    public void pcoDataAfterAttached(int indicationType, PcoDataAttachedInfo pco) {
    }

    /*
     * Indicates the orders of participants call ids when IMS conference SRVCC.
     */
    public void confSRVCC(int indicationType, ArrayList<Integer> callIds) {

    }

    // External SIM [Start]
    public void onVsimEventIndication(int indicationType, VsimOperationEvent event) {
    }
    // External SIM [End]

    /// Ims Data Framework {@
    public void dedicatedBearerActivationInd(int indicationType, DedicateDataCall ddcResult) {
    }
    public void dedicatedBearerModificationInd(int indicationType, DedicateDataCall ddcResult) {
    }
    public void dedicatedBearerDeactivationInd(int indicationType, int ddcResult) {
    }
    /// @}

    public void oemHookRaw(int indicationType, ArrayList<Byte> data) {
    }

    /**
     * Indicates TX power
     * @param indicationType RadioIndicationType
     * @param txPower ArrayList<Integer>
     */
    public void onTxPowerIndication(int indicationType, ArrayList<Integer> txPower) {
    }

    public void onTxPowerStatusIndication(int indicationType, ArrayList<Integer> indPower) {
    }

    public void networkRejectCauseInd(int indicationType, ArrayList<Integer> data) {
    }

    /**
     * Indicates when dsbp changes.
     * @param indicationType RadioIndicationType
     * @param DsbpState android.hardware.mtkradioex.V1_0.DsbpState
     */
    public void dsbpStateChanged(int indicationType, int dsbpState) {
    }

    public void networkBandInfoInd(int indicationType, ArrayList<Integer> info) {
    }

    public void smsInfoExtInd(int indicationType, String info) {
    }

    public void onDsdaChangedInd(int indicationType, int mode) {
    }

    public void qualifiedNetworkTypesChangedInd(int indicationType, ArrayList<Integer> data) {
    }

    public void onCellularQualityChangedInd(int indicationType, ArrayList<Integer> indStgs) {
    }

    public void mobileDataUsageInd(int indicationType, ArrayList<Integer> data) {
    }

    public void onNwLimitInd(int indicationType, ArrayList<Integer> state) {
    }

    public void onRsuEvent(int type, int eventId, String eventString) {
    }
}

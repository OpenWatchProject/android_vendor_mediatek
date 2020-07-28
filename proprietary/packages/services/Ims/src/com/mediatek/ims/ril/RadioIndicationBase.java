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

package com.mediatek.ims.ril;

import java.util.ArrayList;

import android.hardware.radio.V1_4.IRadioIndication;

import android.hardware.radio.V1_1.NetworkScanResult;
import android.hardware.radio.V1_1.KeepaliveStatus;

import android.hardware.radio.V1_0.CdmaCallWaiting;
import android.hardware.radio.V1_0.CdmaInformationRecords;
import android.hardware.radio.V1_0.CdmaSignalInfoRecord;
import android.hardware.radio.V1_0.CdmaSmsMessage;
import android.hardware.radio.V1_0.CellInfo;
import android.hardware.radio.V1_0.HardwareConfig;
import android.hardware.radio.V1_0.LceDataInfo;
import android.hardware.radio.V1_0.PcoDataInfo;
import android.hardware.radio.V1_0.RadioCapability;
import android.hardware.radio.V1_0.SetupDataCallResult;
import android.hardware.radio.V1_0.SignalStrength;
import android.hardware.radio.V1_0.SimRefreshResult;
import android.hardware.radio.V1_0.StkCcUnsolSsResult;
import android.hardware.radio.V1_0.SuppSvcNotification;
import android.hidl.base.V1_0.DebugInfo;
import android.os.HwParcel;
import android.os.IHwBinder;
import android.os.IHwBinder.DeathRecipient;
import android.os.RemoteException;

public class RadioIndicationBase extends IRadioIndication.Stub {

    @Override
    public void callRing(int type, boolean data, CdmaSignalInfoRecord arg2) {
        riljLoge("No implementation in callRing");
    }

    @Override
    public void callStateChanged(int type) {
        riljLoge("No implementation in callStateChanged");
    }

    @Override
    public void cdmaCallWaiting(int type, CdmaCallWaiting data) {
        riljLoge("No implementation in cdmaCallWaiting");
    }

    @Override
    public void cdmaInfoRec(int type, CdmaInformationRecords data) {
        riljLoge("No implementation in cdmaInfoRec");
    }

    @Override
    public void cdmaNewSms(int type, CdmaSmsMessage data) {
        riljLoge("No implementation in cdmaNewSms");
    }

    @Override
    public void cdmaOtaProvisionStatus(int type, int data) {
        riljLoge("No implementation in cdmaOtaProvisionStatus");
    }

    @Override
    public void cdmaPrlChanged(int type, int data) {
        riljLoge("No implementation in cdmaPrlChanged");
    }

    @Override
    public void cdmaRuimSmsStorageFull(int type) {
        riljLoge("No implementation in cdmaRuimSmsStorageFull");
    }

    @Override
    public void cdmaSubscriptionSourceChanged(int type, int data) {
        riljLoge("No implementation in cdmaSubscriptionSourceChanged");
    }

    @Override
    public void cellInfoList(int type, ArrayList<CellInfo> data) {
        riljLoge("No implementation in cellInfoList");
    }

    @Override
    public void currentSignalStrength(int type, SignalStrength data) {
        riljLoge("No implementation in currentSignalStrength");
    }

    @Override
    public void dataCallListChanged(int type,
            ArrayList<SetupDataCallResult> data) {

        riljLoge("No implementation in dataCallListChanged");
    }

    @Override
    public void enterEmergencyCallbackMode(int type) {
        riljLoge("No implementation in enterEmergencyCallbackMode");
    }

    @Override
    public void exitEmergencyCallbackMode(int type) {
        riljLoge("No implementation in exitEmergencyCallbackMode");
    }

    @Override
    public void hardwareConfigChanged(int type, ArrayList<HardwareConfig> data) {
        riljLoge("No implementation in hardwareConfigChanged");
    }

    @Override
    public void imsNetworkStateChanged(int type) {
        riljLoge("No implementation in imsNetworkStateChanged");
    }

    @Override
    public void indicateRingbackTone(int type, boolean data) {
        riljLoge("No implementation in indicateRingbackTone");
    }

    @Override
    public void lceData(int type, LceDataInfo data) {
        riljLoge("No implementation in lceData");
    }

    @Override
    public void modemReset(int type, String data) {
        riljLoge("No implementation in modemReset");
    }

    @Override
    public void networkStateChanged(int type) {
        riljLoge("No implementation in networkStateChanged");
    }

    @Override
    public void newBroadcastSms(int type, ArrayList<Byte> data) {
        riljLoge("No implementation in newBroadcastSms");
    }

    @Override
    public void newSms(int type, ArrayList<Byte> data) {
        riljLoge("No implementation in newSms");
    }

    @Override
    public void newSmsOnSim(int type, int data) {
        riljLoge("No implementation in newSmsOnSim");
    }

    @Override
    public void newSmsStatusReport(int type, ArrayList<Byte> data) {
        riljLoge("No implementation in newSmsStatusReport");
    }

    @Override
    public void nitzTimeReceived(int type, String data, long arg2) {
        riljLoge("No implementation in nitzTimeReceived");
    }

    @Override
    public void onSupplementaryServiceIndication(int type,
            StkCcUnsolSsResult data) {

        riljLoge("No implementation in onSupplementaryServiceIndication");
    }

    @Override
    public void onUssd(int type, int data, String arg2) {
        riljLoge("No implementation in onUssd");
    }

    @Override
    public void pcoData(int type, PcoDataInfo data) {
        riljLoge("No implementation in pcoData");
    }

    @Override
    public void radioCapabilityIndication(int type, RadioCapability data) {
        riljLoge("No implementation in radioCapabilityIndication");
    }

    @Override
    public void radioStateChanged(int type, int data) {
        riljLoge("No implementation in radioStateChanged");
    }

    @Override
    public void resendIncallMute(int type) {
        riljLoge("No implementation in resendIncallMute");
    }

    @Override
    public void restrictedStateChanged(int type, int data) {
        riljLoge("No implementation in restrictedStateChanged");
    }

    @Override
    public void rilConnected(int type) {
        riljLoge("No implementation in rilConnected");
    }

    @Override
    public void simRefresh(int type, SimRefreshResult data) {
        riljLoge("No implementation in simRefresh");
    }

    @Override
    public void simSmsStorageFull(int type) {
        riljLoge("No implementation in simSmsStorageFull");
    }

    @Override
    public void simStatusChanged(int type) {
        riljLoge("No implementation in simStatusChanged");
    }

    @Override
    public void srvccStateNotify(int type, int data) {
        riljLoge("No implementation in srvccStateNotify");
    }

    @Override
    public void stkCallControlAlphaNotify(int type, String data) {
        riljLoge("No implementation in stkCallControlAlphaNotify");
    }

    @Override
    public void stkCallSetup(int type, long data) {
        riljLoge("No implementation in stkCallSetup");
    }

    @Override
    public void stkEventNotify(int type, String data) {
        riljLoge("No implementation in stkEventNotify");
    }

    @Override
    public void stkProactiveCommand(int type, String data) {
        riljLoge("No implementation in stkProactiveCommand");
    }

    @Override
    public void stkSessionEnd(int type) {
        riljLoge("No implementation in stkSessionEnd");
    }

    @Override
    public void subscriptionStatusChanged(int type, boolean data) {
        riljLoge("No implementation in subscriptionStatusChanged");
    }

    @Override
    public void suppSvcNotify(int type, SuppSvcNotification data) {
        riljLoge("No implementation in suppSvcNotify");
    }

    @Override
    public void voiceRadioTechChanged(int type, int data) {
        riljLoge("No implementation in voiceRadioTechChanged");
    }

    @Override
    public void keepaliveStatus(int type, KeepaliveStatus status) {
        riljLoge("No implementation in keepaliveStatus");
    }

    @Override
    public void carrierInfoForImsiEncryption(int type) {
        riljLoge("No implementation in carrierInfoForImsiEncryption");
    }

    @Override
    public void networkScanResult(int type, NetworkScanResult result) {
        riljLoge("No implementation in networkScanResult");
    }

    /**
     * Indicates current signal strength of the radio.
     * AOSP Radio 1.2 Interfaces
     *
     * @param type Type of radio indication
     * @param signalStrength SignalStrength information
     */
    @Override
    public void currentSignalStrength_1_2(int type,
                android.hardware.radio.V1_2.SignalStrength signalStrength) {
        riljLoge("No implementation in currentSignalStrength_1_2");
    }

    /**
     * AOSP Radio 1.2 Interfaces
     *
     * @param type Type of radio indication
     * @param signalStrength SignalStrength information
     */
    @Override
    public void currentPhysicalChannelConfigs(int type,
                ArrayList<android.hardware.radio.V1_2.PhysicalChannelConfig> configs) {
        riljLoge("No implementation in currentPhysicalChannelConfigs");
    }

    /**
     * AOSP Radio 1.2 Interfaces
     *
     * @param type Type of radio indication
     * @param lce LinkCapacityEstimate information
     */
    @Override
    public void currentLinkCapacityEstimate(int type,
                android.hardware.radio.V1_2.LinkCapacityEstimate lce) {
        riljLoge("No implementation in currentLinkCapacityEstimate");
    }

    /**
     * AOSP Radio 1.2 Interfaces
     *
     * @param type Type of radio indication
     * @param lce LinkCapacityEstimate information
     */
    @Override
    public void cellInfoList_1_2(int type,
                                 ArrayList<android.hardware.radio.V1_2.CellInfo> records) {
        riljLoge("No implementation in cellInfoList_1_2");
    }

    /**
     * AOSP Radio 1.2 Interfaces
     *
     * @param type Type of radio indication
     * @param result NetworkScanResult information
     */
    @Override
    public void networkScanResult_1_2(int type,
                                      android.hardware.radio.V1_2.NetworkScanResult result) {
        riljLoge("No implementation in networkScanResult_1_2");
    }

    public void currentEmergencyNumberList(int indicationType,
            ArrayList<android.hardware.radio.V1_4.EmergencyNumber> emergencyNumberList) {
        riljLoge("No implementation in currentEmergencyNumberList");
    }

    public void cellInfoList_1_4(int indicationType,
                                 ArrayList<android.hardware.radio.V1_4.CellInfo> records) {
        riljLoge("No implementation in cellInfoList_1_4");
    }

    public void networkScanResult_1_4(int indicationType,
                                      android.hardware.radio.V1_4.NetworkScanResult result) {
        riljLoge("No implementation in networkScanResult_1_4");
    }

    public void currentPhysicalChannelConfigs_1_4(int indicationType,
            ArrayList<android.hardware.radio.V1_4.PhysicalChannelConfig> configs) {
        riljLoge("No implementation in currentPhysicalChannelConfigs_1_4");
    }

    public void dataCallListChanged_1_4(int indicationType,
            ArrayList<android.hardware.radio.V1_4.SetupDataCallResult> dcList) {
        riljLoge("No implementation in dataCallListChanged_1_4");
    }

    public void currentSignalStrength_1_4(int indicationType,
            android.hardware.radio.V1_4.SignalStrength signalStrength) {
        riljLoge("No implementation in currentSignalStrength_1_4");
    }

    /**
     * Log for error
     * @param msg
     */
    protected void riljLoge(String msg) {}
}

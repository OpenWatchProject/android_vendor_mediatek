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

import vendor.mediatek.hardware.mtkradioex.V1_0.EtwsNotification;
import vendor.mediatek.hardware.mtkradioex.V1_6.IImsRadioIndication;
import vendor.mediatek.hardware.mtkradioex.V1_0.IncomingCallNotification;
import vendor.mediatek.hardware.mtkradioex.V1_0.ImsConfParticipant;
import vendor.mediatek.hardware.mtkradioex.V1_0.ImsRegStatusInfo;
import vendor.mediatek.hardware.mtkradioex.V1_0.CallInfoType;

import android.hardware.radio.V1_0.CdmaSmsMessage;
import android.hardware.radio.V1_0.SuppSvcNotification;
import android.hidl.base.V1_0.DebugInfo;
import android.os.HwParcel;
import android.os.IHwBinder;
import android.os.IHwBinder.DeathRecipient;
import android.os.RemoteException;

import vendor.mediatek.hardware.mtkradioex.V1_0.Dialog;

public class ImsRadioIndicationBase extends IImsRadioIndication.Stub {


    @Override
    public void ectIndication(int arg0, int arg1, int arg2, int arg3)
            {

        riljLoge("No implementation in ectIndication");
    }

    @Override
    public void volteSetting(int arg0, boolean arg1) {
        riljLoge("No implementation in volteSetting");
    }

    @Override
    public void callInfoIndication(int type, ArrayList<String> data) {
        riljLoge("No implementation in callInfoIndication");
    }

    @Override
    public void callmodChangeIndicator(int type, String callId, String callMode,
                                       String videoState, String audioDirection,
                                       String pau) {

        riljLoge("No implementation in callmodChangeIndicator");
    }

    @Override
    public void econfResultIndication(int type, String confCallId,
                                      String op, String num, String result,
                                      String cause, String joinedCallId) {

        riljLoge("No implementation in econfResultIndication");
    }

    @Override
    public void getProvisionDone(int type, String data, String arg2) {
        riljLoge("No implementation in getProvisionDone");
    }

    @Override
    public void imsBearerStateNotify(int type, int data, int action, String arg2) {
        riljLoge("No implementation in imsBearerStateNotify");
    }

    @Override
    public void imsBearerInit(int type) {
        riljLoge("No implementation in imsBearerInit");
    }

    @Override
    public void imsDataInfoNotify(int type, String arg1, String arg2, String arg3) {
        riljLoge("No implementation in imsDataInfoNotify");
    }

    @Override
    public void imsDisableDone(int type) {
        riljLoge("No implementation in imsDisableDone");
    }

    @Override
    public void imsDisableStart(int type) {
        riljLoge("No implementation in imsDisableStart");
    }

    @Override
    public void imsEnableDone(int type) {
        riljLoge("No implementation in imsEnableDone");
    }

    @Override
    public void imsEnableStart(int type) {
        riljLoge("No implementation in imsEnableStart");
    }

    @Override
    public void imsRegistrationInfo(int type, int data, int arg2) {

        riljLoge("No implementation in imsRegistrationInfo");
    }

    @Override
    public void incomingCallIndication(int type, IncomingCallNotification data) {
        riljLoge("No implementation in incomingCallIndication");
    }

    @Override
    public void onUssi(int type, int ussdModeType, String msg) {

        riljLoge("No implementation in onUssi");
    }

    @Override
    public void onXui(int type, String accountId, String broadcastFlag,
                      String xuiInfo) {

        riljLoge("No implementation in onXui");
    }

    @Override
    public void onVolteSubscription(int type, int status) {

        riljLoge("No implementation in onVolteSubscription");
    }

    @Override
    public void suppSvcNotify(int type, SuppSvcNotification data) {

        riljLoge("No implementation in suppSvcNotify");
    }

    @Override
    public void sipCallProgressIndicator(int type, String callId, String dir,
                                         String sipMsgType, String method,
                                         String responseCode, String reasonText) {

        riljLoge("No implementation in sipCallProgressIndicator");
    }

    @Override
    public void videoCapabilityIndicator(int type, String callId,
                                         String localVideoCap, String remoteVideoCap) {

        riljLoge("No implementation in videoCapabilityIndicator");
    }

    @Override
    public void imsConferenceInfoIndication(int type,
            ArrayList<ImsConfParticipant> participants) {
        riljLoge("No implementation in imsConferenceInfoIndication");
    }

    @Override
    public void lteMessageWaitingIndication(int type,
                                          String callId, String pType, String urcIdx,
                                          String totalUrcCount, String rawData) {
        riljLoge("No implementation in lteMessageWaitingIndication");
    }

    /**
     * [IMS] IMS Dialog Event Package Indiciation
     * @param type Type of radio indication
     * @param dialogList the dialog info list
     */
    @Override
    public void imsDialogIndication(int type, ArrayList<Dialog> dialogList) {
        riljLoge("No implementation in imsDialogIndication");
    }

    @Override
    public void imsCfgDynamicImsSwitchComplete(int type) {
        riljLoge("No implementation in imsCfgDynamicImsSwitchComplete");
    }

    @Override
    public void imsCfgFeatureChanged(int type, int phoneId, int featureId, int value) {
        riljLoge("No implementation in imsCfgFeatureChanged");
    }

    @Override
    public void imsCfgConfigChanged(int type, int phoneId, String configId, String value) {
        riljLoge("No implementation in imsCfgConfigChanged");
    }

    @Override
    public void imsCfgConfigLoaded(int type) {
        riljLoge("No implementation in imsCfgConfigLoaded");
    }

    @Override
    public void newSmsStatusReportEx(int indicationType, ArrayList<Byte> pdu) {
        riljLoge("No implementation in newSmsStatusReportEx");
    }

    @Override
    public void newSmsEx(int indicationType, ArrayList<Byte> pdu) {
        riljLoge("No implementation in newSmsEx");
    }

    @Override
    public void cdmaNewSmsEx(int indicationType, CdmaSmsMessage msg) {
        riljLoge("No implementation in cdmaNewSmsEx");
    }

    @Override
    public void noEmergencyCallbackMode(int indicationType) {
        riljLoge("No implementation in noEmergencyCallbackMode");
    }

    @Override
    public void imsRtpInfo(int type, String pdnId, String networkId, String timer,
                           String sendPktLost, String recvPktLost, String jitter, String delay) {

        riljLoge("No implementation in imsRtpInfoReport");
    }

    @Override
    public void imsRedialEmergencyIndication(int type, String callId) {
        riljLoge("No implementation in imsRedialEmergencyIndication");
    }

    /**
    * Reports speech codec information
    *
    * @param type Type of radio indication
    * @param info integer type speech codec info
    */
    @Override
    public void speechCodecInfoIndication(int type, int info) {
        riljLoge("No implementation in speechCodecInfoIndication");
    }

    @Override
    public void imsRadioInfoChange(int type, String iid, String info) {
        riljLoge("No implementation in imsRadioInfoChange");
    }

    @Override
    public void imsSupportEcc(int type, int supportLteEcc) {
        riljLoge("No implementation in isSupportLteEcc");
    }

    @Override
    public void multiImsCount(int type, int count) {
        riljLoge("No implementation in multiImsCount");
    }

    @Override
    public void imsEventPackageIndication(int type,
                                          String callId, String ptype, String urcIdx,
                                          String totalUrcCount, String rawData) {
        riljLoge("No implementation in imsEventPackageIndication");
    }

    @Override
    public void imsDeregDone(int type) {
        riljLoge("No implementation in imsDeregDone");
    }

    // M: RTT @{
    @Override
    public void rttModifyResponse(int indicationType, int callid, int result) {
        riljLoge("No implementation in rttModifyResponse");
    }

    @Override
    public void rttTextReceive(int indicationType, int callid, int length, String text) {
        riljLoge("No implementation in rttTextReceive");
    }

    @Override
    public void rttModifyRequestReceive(int indicationType, int callid, int rttType) {
        riljLoge("No implementation in rttModifyRequestReceive");
    }

    @Override
    public void rttCapabilityIndication(int indicationType, int callid, int localCapability,
            int remoteCapability, int localStatus, int remoteStatus) {
        riljLoge("No implementation in rttCapabilityIndication");
    }

    @Override
    public void audioIndication(int indicationType, int callId, int audio) {
        riljLoge("No implementation in audioIndication");
    }
    // @}

    @Override
    public void sendVopsIndication(int indicationType, int vops) {
        riljLoge("No implementation in sendVopsIndication");
    }

    @Override
    public void sipHeaderReport(int indicationType, ArrayList<String> data) {
        riljLoge("No implementation in sipHeaderReport");
    }

    @Override
    public void callAdditionalInfoInd(int indicationType,
            int ciType,
            ArrayList<String> info) {
        riljLoge("No implementation in callAdditionalInfoInd");
    }

    @Override
    public void callRatIndication(int indicationType, int domain, int rat) {
        riljLoge("No implementation in callRatIndication");
    }

    @Override
    public void sipRegInfoInd(int indicationType, int account_id, int response_code,
            ArrayList<String>  info) {
        riljLoge("No implementation in sipRegInfoInd");
    }

    @Override
    public void imsRegStatusReport(int type, ImsRegStatusInfo report) {
        riljLoge("No implementation in imsRegStatusReport");
    }

    @Override
    public void imsRegInfoInd(int indicationType, ArrayList<Integer>  info) {
        riljLoge("No implementation in imsRegInfoInd");
    }

    @Override
    public void onSsacStatus(int type, ArrayList<Integer> status) {
        riljLoge("No implementation in onSsacStatus");
    }

    /**
     * Log for error
     * @param msg
     */
    protected void riljLoge(String msg) {}
}

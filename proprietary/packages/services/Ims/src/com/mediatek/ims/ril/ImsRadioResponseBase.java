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

import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.SendSmsResult;

import vendor.mediatek.hardware.mtkradioex.V1_6.IImsRadioResponse;
import vendor.mediatek.hardware.mtkradioex.V1_0.CallForwardInfoEx;

import android.os.IHwBinder;
import android.os.IHwBinder.DeathRecipient;
import android.os.RemoteException;

public class ImsRadioResponseBase extends IImsRadioResponse.Stub {

    @Override
    public void acknowledgeLastIncomingGsmSmsExResponse(RadioResponseInfo info) {
        riljLoge("No implementation in acknowledgeLastIncomingGsmSmsExResponse");
    }

    @Override
    public void acknowledgeLastIncomingCdmaSmsExResponse(RadioResponseInfo info) {
        riljLoge("No implementation in acknowledgeLastIncomingCdmaSmsExResponse");
    }



    @Override
    public void sendImsSmsExResponse(RadioResponseInfo responseInfo, SendSmsResult sms) {
        riljLoge("No implementation in sendImsSmsExResponse");
    }

    @Override
    public void cancelUssiResponse(RadioResponseInfo info) {
        riljLoge("No implementation in cancelUssiResponse");
    }

    @Override
    public void getXcapStatusResponse(RadioResponseInfo info) {
        riljLoge("No implementation in getXcapStatusResponse");
    }

    @Override
    public void resetSuppServResponse(RadioResponseInfo info) {
        riljLoge("No implementation in resetSuppServResponse");
    }

    @Override
    public void setupXcapUserAgentStringResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setupXcapUserAgentStringResponse");
    }

    @Override
    public void conferenceDialResponse(RadioResponseInfo info) {
        riljLoge("No implementation in conferenceDialResponse");
    }

    @Override
    public void imsDeregNotificationResponse(RadioResponseInfo info) {
        riljLoge("No implementation in deregisterImsResponse");
    }

    @Override
    public void dialWithSipUriResponse(RadioResponseInfo info) {
        riljLoge("No implementation in dialWithSipUriResponse");
    }

    @Override
    public void forceReleaseCallResponse(RadioResponseInfo info) {
        riljLoge("No implementation in forceReleaseCallResponse");
    }

    @Override
    public void getProvisionValueResponse(RadioResponseInfo info) {
        riljLoge("No implementation in getProvisionValueResponse");
    }

    @Override
    public void hangupAllResponse(RadioResponseInfo info) {
        riljLoge("No implementation in hangupAllResponse");
    }

    @Override
    public void controlCallResponse(RadioResponseInfo info) {
        riljLoge("No implementation in controlCallResponse");
    }

    @Override
    public void imsBearerStateConfirmResponse(RadioResponseInfo info) {
        riljLoge("No implementation in imsBearerStateConfirm");
    }

    @Override
    public void setImsBearerNotificationResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setImsBearerNotificationResponse");
    }

    @Override
    public void imsEctCommandResponse(RadioResponseInfo info) {
        riljLoge("No implementation in imsEctCommandResponse");
    }

    @Override
    public void controlImsConferenceCallMemberResponse(RadioResponseInfo info) {
        riljLoge("No implementation in controlImsConferenceCallMemberResponse");
    }

    @Override
    public void sendUssiResponse(RadioResponseInfo info) {
        riljLoge("No implementation in sendUssiResponse");
    }

    @Override
    public void setCallIndicationResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setCallIndicationResponse");
    }

    @Override
    public void setImsCallStatusResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setImsCallStatusResponse");
    }

    @Override
    public void setImsEnableResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setImsEnableResponse");
    }

    @Override
    public void setImsRtpReportResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setImsRtpReportResponse");
    }

    @Override
    public void setImsVideoEnableResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setImsVideoEnableResponse");
    }

    @Override
    public void setImscfgResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setImscfgResponse");
    }

    @Override
    public void setModemImsCfgResponse(RadioResponseInfo info, String results) {
        riljLoge("No implementation in setModemImsCfgResponse");
    }

    @Override
    public void pullCallResponse(RadioResponseInfo info) {
        riljLoge("No implementation in pullCallResponse");
    }

    @Override
    public void setProvisionValueResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setProvisionValueResponse");
    }

    @Override
    public void setImsCfgFeatureValueResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setImsCfgFeatureValueResponse");
    }

    @Override
    public void getImsCfgFeatureValueResponse(RadioResponseInfo info, int value) {
        riljLoge("No implementation in getImsCfgFeatureValueResponse");
    }

    @Override
    public void setImsCfgProvisionValueResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setImsCfgProvisionValueResponse");
    }

    @Override
    public void getImsCfgProvisionValueResponse(RadioResponseInfo info, String value) {
        riljLoge("No implementation in getImsCfgProvisionValueResponse");
    }

    @Override
    public void getImsCfgResourceCapValueResponse(RadioResponseInfo info, int value) {
        riljLoge("No implementation in getImsCfgResourceCapValueResponse");
    }

    @Override
    public void setWfcProfileResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setWfcProfileResponse");
    }

    @Override
    public void updateImsRegistrationStatusResponse(RadioResponseInfo info) {
        riljLoge("No implementation in updateImsRegistrationStatusResponse");
    }

    @Override
    public void videoCallAcceptResponse(RadioResponseInfo info) {
        riljLoge("No implementation in videoCallAcceptResponse");
    }

    @Override
    public void eccRedialApproveResponse(RadioResponseInfo info) {
        riljLoge("No implementation in eccRedialApproveResponse");
    }

    @Override
    public void vtDialResponse(RadioResponseInfo info) {
        riljLoge("No implementation in vtDialResponse");
    }

    @Override
    public void vtDialWithSipUriResponse(RadioResponseInfo info) {
        riljLoge("No implementation in vtDialWithSipUriResponse");
    }

    @Override
    public void setImsRegistrationReportResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setImsRegistrationReportResponse");
    }

    @Override
    public void setVoiceDomainPreferenceResponse(RadioResponseInfo info) {
       riljLoge("No implementation in setImsVoiceDomainPreferenceResponse");
    }

    @Override
    public void getVoiceDomainPreferenceResponse(RadioResponseInfo info, int vdp) {
       riljLoge("No implementation in getImsVoiceDomainPreferenceResponse");
    }

    @Override
    public void setCallForwardInTimeSlotResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setCallForwardInTimeSlotResponse");
    }

    @Override
    public void runGbaAuthenticationResponse(RadioResponseInfo info, ArrayList<String> resList) {
        riljLoge("No implementation in runGbaAuthenticationResponse");
    }

    @Override
    public void queryCallForwardInTimeSlotStatusResponse(RadioResponseInfo info,
            ArrayList<CallForwardInfoEx> callForwardInfoExs) {

        riljLoge("No implementation in queryCallForwardInTimeSlotStatusResponse");
    }

    @Override
    public void setColrResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setColrResponse");
    }

    @Override
    public void setColpResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setColpResponse");
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param status indicates COLR status. "0" means not provisioned, "1" means provisioned,
     *        "2" means unknown
     */
    @Override
    public void getColrResponse(RadioResponseInfo responseInfo, int status) {
        riljLoge("No implementation in getColrResponse");
    }

    @Override
    public void getColpResponse(RadioResponseInfo responseInfo, int n, int m) {
        riljLoge("No implementation in getColpResponse");
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    @Override
    public void setClipResponse(RadioResponseInfo responseInfo) {
        riljLoge("No implementation in setClipResponse");
    }

    @Override
    public void hangupWithReasonResponse(RadioResponseInfo responseInfo) {
        riljLoge("No implementation in hangupWithReasonResponse");
    }

    // M: RTT @{
    @Override
    public void setRttModeResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setRttModeResponse");
    }

    @Override
    public void rttModifyRequestResponseResponse(RadioResponseInfo info) {
        riljLoge("No implementation in rttModifyRequestResponseResponse");
    }

    @Override
    public void sendRttTextResponse(RadioResponseInfo info) {
        riljLoge("No implementation in sendRttTextResponse");
    }

    @Override
    public void sendRttModifyRequestResponse(RadioResponseInfo info) {
        riljLoge("No implementation in sendRttModifyRequestResponse");
    }
    // @}

    public void queryVopsStatusResponse(RadioResponseInfo responseInfo, int vops) {
        riljLoge("No implementation in queryVopsStatusResponse");
    }

    @Override
    public void setSipHeaderResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setSipHeaderResponse");
    }

    @Override
    public void setSipHeaderReportResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setSipHeaderReportResponse");
    }

    @Override
    public void setImsCallModeResponse(RadioResponseInfo info) {
        riljLoge("No implementation in setImsCallModeResponse");
    }

    @Override
    public void setVendorSettingResponse(RadioResponseInfo responseInfo) {
    }

    public void querySsacStatusResponse(RadioResponseInfo responseInfo, ArrayList<Integer> status) {
        riljLoge("No implementation in querySsacStatusResponse");
    }

    @Override
    public void toggleRttAudioIndicationResponse(RadioResponseInfo responseInfo) {
        riljLoge("No implementation in toggleRttAudioIndicationResponse");
    }

    @Override
    public void setCallAdditionalInfoResponse(RadioResponseInfo responseInfo) {
        riljLoge("No implementation in setCallAdditionalInfoResponse");
    }
    /**
     * Log for error
     * @param msg
     */
    protected void riljLoge(String msg) {}

}

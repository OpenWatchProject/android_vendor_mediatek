/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

import com.android.internal.telephony.OperatorInfo;
import com.android.internal.telephony.RadioResponse;
import com.android.internal.telephony.UUSInfo;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.RILRequest;

import vendor.mediatek.hardware.mtkradioex.V1_0.PhbEntryExt;
import vendor.mediatek.hardware.mtkradioex.V1_0.PhbEntryStructure;
import vendor.mediatek.hardware.mtkradioex.V1_0.PhbMemStorageResponse;
import vendor.mediatek.hardware.mtkradioex.V1_0.CallForwardInfoEx;

import android.content.Context;
import android.hardware.radio.V1_0.ActivityStatsInfo;
import android.hardware.radio.V1_0.AppStatus;
import android.hardware.radio.V1_0.CardStatus;
import android.hardware.radio.V1_0.CarrierRestrictions;
import android.hardware.radio.V1_0.CdmaBroadcastSmsConfigInfo;
import android.hardware.radio.V1_0.DataRegStateResult;
import android.hardware.radio.V1_0.GsmBroadcastSmsConfigInfo;
import android.hardware.radio.V1_0.LastCallFailCauseInfo;
import android.hardware.radio.V1_0.LceDataInfo;
import android.hardware.radio.V1_0.LceStatusInfo;
import android.hardware.radio.V1_0.NeighboringCell;
import android.hardware.radio.V1_0.RadioError;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.SendSmsResult;
import android.hardware.radio.V1_0.SetupDataCallResult;
import android.hardware.radio.V1_0.VoiceRegStateResult;
import android.os.AsyncResult;
import android.os.RemoteException;
import android.os.Message;

import android.os.SystemProperties;
import android.text.TextUtils;

import android.util.Log;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

// SMS-START
import vendor.mediatek.hardware.mtkradioex.V1_0.SmsParams;
import vendor.mediatek.hardware.mtkradioex.V1_0.SmsMemStatus;
import mediatek.telephony.MtkSmsParameters;
import com.mediatek.internal.telephony.MtkIccSmsStorageStatus;
// SMS-END
// NW-START
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.ServiceStateTracker;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import java.io.UnsupportedEncodingException;
import android.telephony.SignalStrength;
import vendor.mediatek.hardware.mtkradioex.V1_0.SignalStrengthWithWcdmaEcio;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
// NW END
// PHB START
import com.mediatek.internal.telephony.phb.PBEntry;
import com.mediatek.internal.telephony.phb.PBMemStorage;
import com.mediatek.internal.telephony.phb.PhbEntry;
// PHB END

import android.telephony.PhoneNumberUtils;
import com.android.internal.telephony.uicc.IccUtils;

// External SIM [Start]
import vendor.mediatek.hardware.mtkradioex.V1_0.VsimEvent;
// External SIM [End]
import com.mediatek.internal.telephony.MtkMessageBoost;
//RSU
import vendor.mediatek.hardware.mtkradioex.V1_9.RsuResponseInfo;

public class MtkRadioExResponse extends MtkRadioExResponseBase {

    // TAG
    private static final String TAG = "MtkRadioRespEx";
    private static final boolean isUserLoad = SystemProperties.get("ro.build.type").equals("user");

    MtkRIL mMtkRil;
    MtkMessageBoost mMtkMessageBoost;

    public MtkRadioExResponse(RIL ril) {
        super(ril);
        mMtkRil = (MtkRIL)ril;
        mMtkMessageBoost = MtkMessageBoost.init(mMtkRil);
    }

    /**
     * Helper function to send response msg
     * @param msg Response message to be sent
     * @param ret Return object to be included in the response message
     */
    static void sendMessageResponse(Message msg, Object ret) {
        if (msg != null) {
            AsyncResult.forMessage(msg, ret, null);
            msg.sendToTarget();
        }
    }

    /**
     * Acknowledge the receipt of radio request sent to the vendor. This must be sent only for
     * radio request which take long time to respond.
     * For more details, refer https://source.android.com/devices/tech/connect/ril.html
     *
     * @param serial Serial no. of the request whose acknowledgement is sent.
     */
    public void acknowledgeRequest(int serial) {
        mMtkRil.processRequestAck(serial);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param retriesRemaining Number of retries remaining.
     */
    public void supplyDepersonalizationResponse(RadioResponseInfo responseInfo,
            int retriesRemaining) {
        mMtkRil.getMtkRadioResponse().supplyNetworkDepersonalizationResponse(responseInfo, retriesRemaining);
    }

    /* MTK SS Feature : Start */
    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setClipResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param n Colp status in network, "0" means disabled, "1" means enabled
     * @param m Service status, "0" means not provisioned, "1" means provisioned in permanent mode
     */
    public void getColpResponse(RadioResponseInfo responseInfo, int n, int m) {
        mMtkRil.getMtkRadioResponse().responseInts(responseInfo, n, m);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param status indicates COLR status. "0" means not provisioned, "1" means provisioned,
     *        "2" means unknown
     */
    public void getColrResponse(RadioResponseInfo responseInfo, int status) {
        mMtkRil.getMtkRadioResponse().responseInts(responseInfo, status);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param n CNAP status, "0" means disabled, "1" means enabled.
     * @param m Service status, "0" means not provisioned, "1" means provisioned, "2" means unknown
     */
    public void sendCnapResponse(RadioResponseInfo responseInfo, int n, int m) {
        mMtkRil.getMtkRadioResponse().responseInts(responseInfo, n, m);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    @Override
    public void setColpResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    @Override
    public void setColrResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param callForwardInfoExs points to a vector of CallForwardInfoEx, one for
     *        each distinct registered phone number.
     */
    @Override
    public void queryCallForwardInTimeSlotStatusResponse(RadioResponseInfo responseInfo,
            ArrayList<CallForwardInfoEx> callForwardInfoExs) {
        responseCallForwardInfoEx(responseInfo, callForwardInfoExs);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     *
     */
    @Override
    public void setCallForwardInTimeSlotResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param resList points to a vector of String.
     */
    @Override
    public void runGbaAuthenticationResponse(RadioResponseInfo responseInfo,
                                             ArrayList<String> resList) {
        mMtkRil.getMtkRadioResponse().responseStringArrayList(mMtkRil.getMtkRadioResponse().mRil,
                responseInfo, resList);
    }
    /* MTK SS Feature : End */

    public void sendOemRilRequestRawResponse(RadioResponseInfo responseInfo,
                                             ArrayList<Byte> var2) {}

    public void setTrmResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);
        if (rr != null) {
            if (responseInfo.error == RadioError.NONE) {
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
            }
            mMtkRil.processResponseDone(rr, responseInfo, null);
        }
    }

    // MTK-START: SIM
    /**
     * @param info Response info struct containing response type, serial no. and error.
     * @param response Response string of getATRResponse.
     */
    public void getATRResponse(RadioResponseInfo info, String response) {
        mMtkRil.getMtkRadioResponse().responseString(info, response);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error.
     * @param response Response string of getIccidResponse.
     */
    public void getIccidResponse(RadioResponseInfo info, String response) {
        mMtkRil.getMtkRadioResponse().responseString(info, response);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error.
     */
    public void setSimPowerResponse(RadioResponseInfo info) {
        mMtkRil.getMtkRadioResponse().responseVoid(info);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error.
     * @param simPowerOnOffResponse result of activating sim power.
     */
    public void activateUiccCardRsp(RadioResponseInfo info, int simPowerOnOffResponse) {
        mMtkRil.getMtkRadioResponse().responseInts(info, simPowerOnOffResponse);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error.
     * @param simPowerOnOffResponse result of deactivating sim power.
     */
    public void deactivateUiccCardRsp(RadioResponseInfo info, int simPowerOnOffResponse) {
        mMtkRil.getMtkRadioResponse().responseInts(info, simPowerOnOffResponse);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error.
     * @param simPowerOnOffStatus status of sim power state.
     */
    public void getCurrentUiccCardProvisioningStatusRsp(RadioResponseInfo info,
            int simPowerOnOffStatus) {
        mMtkRil.getMtkRadioResponse().responseInts(info, simPowerOnOffStatus);
    }
    // MTK-END

    // MTK-START: NW
    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setNetworkSelectionModeManualWithActResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().setNetworkSelectionModeManualResponse(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param networkInfos List of network operator information as OperatorInfoWithAct defined in
     *      types.hal
     */
    public void getAvailableNetworksWithActResponse(RadioResponseInfo responseInfo,
                                            ArrayList<vendor.mediatek.hardware.
                                            mtkradioex.V1_0.OperatorInfoWithAct> networkInfos) {
        responseOperatorInfosWithAct(responseInfo, networkInfos);
    }

    /**
       * @param responseInfo Response info struct containing response type, serial no. and error
       * @param customerSignalStrength list of signalstrength with wcdma ecio
       */
    public void getSignalStrengthWithWcdmaEcioResponse(RadioResponseInfo responseInfo,
            SignalStrengthWithWcdmaEcio signalStrength){
        responseGetSignalStrengthWithWcdmaEcio(responseInfo, signalStrength);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void cancelAvailableNetworksResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void cfgA2offsetResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void cfgB1offsetResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void enableSCGfailureResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void disableNRResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setTxPowerResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setSearchStoredFreqInfoResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setSearchRatResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setBgsrchDeltaSleepTimerResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    private int getSubId(int phoneId) {
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        int[] subIds = SubscriptionManager.getSubId(phoneId);
        if (subIds != null && subIds.length > 0) {
            subId = subIds[0];
        }
        return subId;
    }

    private void responseOperatorInfosWithAct(RadioResponseInfo responseInfo,
                                            ArrayList<vendor.mediatek.hardware.
                                            mtkradioex.V1_0.OperatorInfoWithAct> networkInfos) {
        RILRequest rr = mMtkRil.getMtkRadioResponse().mRil.processResponse(responseInfo);

        if (rr != null) {
            ArrayList<OperatorInfo> ret = null;
            if (responseInfo.error == RadioError.NONE) {
                ret =  new ArrayList<OperatorInfo>();
                for (int i = 0; i < networkInfos.size(); i++) {
                    int nLac = -1;
                    mMtkRil.riljLog("responseOperatorInfosWithAct: act:" + networkInfos.get(i).act);
                    mMtkRil.riljLog("responseOperatorInfosWithAct: lac:" + networkInfos.get(i).lac);
                    if (networkInfos.get(i).lac.length() > 0) {
                        nLac = Integer.parseInt(networkInfos.get(i).lac, 16);
                    }
                    networkInfos.get(i).base.alphaLong = mMtkRil.lookupOperatorName(
                            getSubId(mMtkRil.mInstanceId),
                            networkInfos.get(i).base.operatorNumeric, true, nLac);
                    networkInfos.get(i).base.alphaShort = mMtkRil.lookupOperatorName(
                            getSubId(mMtkRil.mInstanceId),
                            networkInfos.get(i).base.operatorNumeric, false, nLac);
                    /* Show Act info(ex: "2G","3G","4G") for PLMN list result */
                    networkInfos.get(i).base.alphaLong = networkInfos.get(i).base.
                            alphaLong.concat(" " + networkInfos.get(i).act);
                    networkInfos.get(i).base.alphaShort = networkInfos.get(i).base.
                            alphaShort.concat(" " + networkInfos.get(i).act);

                    if (!mMtkRil.hidePLMN(networkInfos.get(i).base.operatorNumeric)) {
                        ret.add(new OperatorInfo(networkInfos.get(i).base.alphaLong,
                                networkInfos.get(i).base.alphaShort,
                                networkInfos.get(i).base.operatorNumeric,
                                mMtkRil.getMtkRadioResponse().convertOpertatorInfoToString(
                                        networkInfos.get(i).base.status)));
                    } else {
                        mMtkRil.riljLog("remove this one " +
                                networkInfos.get(i).base.operatorNumeric);
                    }
                }
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, ret);
            }
            mMtkRil.getMtkRadioResponse().mRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    private void responseGetSignalStrengthWithWcdmaEcio(
            RadioResponseInfo responseInfo,
            SignalStrengthWithWcdmaEcio signalStrength) {
        RILRequest rr = mMtkRil.getMtkRadioResponse().mRil.processResponse(responseInfo);

        if (rr != null) {
            // To Do: this SignalStrength should be customer signalStrength class or struct
            SignalStrength ret = new SignalStrength ();
            if (responseInfo.error == RadioError.NONE) {
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, ret);
            }
            mMtkRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    // MTK-END: NW

    /*
    * @param responseInfo Response info struct containing response type, serial no. and error
    */
    public void setModemPowerResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    // SMS-START
    public void getSmsParametersResponse(
            RadioResponseInfo responseInfo, SmsParams params) {
        responseSmsParams(responseInfo, params);
    }

    private void responseSmsParams(RadioResponseInfo responseInfo, SmsParams params) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            Object ret = null;
            if (responseInfo.error == RadioError.NONE) {
                MtkSmsParameters smsp = new MtkSmsParameters(
                        params.format, params.vp, params.pid, params.dcs);
                mMtkRil.riljLog("responseSmsParams: from HIDL: " + smsp);
                ret = smsp;
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, ret);
            }
            mMtkRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    public void setSmsParametersResponse(
            RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }
    public void setEtwsResponse(
            RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void removeCbMsgResponse(
            RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }


    public void getSmsMemStatusResponse(
            RadioResponseInfo responseInfo, SmsMemStatus params) {
        responseSmsMemStatus(responseInfo, params);
    }

    private void responseSmsMemStatus(RadioResponseInfo responseInfo, SmsMemStatus params) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            Object ret = null;
            if (responseInfo.error == RadioError.NONE) {
                MtkIccSmsStorageStatus status = new MtkIccSmsStorageStatus(
                        params.used, params.total);
                mMtkRil.riljLog("responseSmsMemStatus: from HIDL: " + status);
                ret = status;
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, ret);
            }
            mMtkRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    public void setGsmBroadcastLangsResponse(
            RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void getGsmBroadcastLangsResponse(
            RadioResponseInfo responseInfo, String langs) {
        mMtkRil.getMtkRadioResponse().responseString(responseInfo, langs);
    }

    public void getGsmBroadcastActivationRsp(RadioResponseInfo responseInfo,
            int activation) {
        mMtkRil.getMtkRadioResponse().responseInts(responseInfo, activation);
    }
    // SMS-END

    /// M: eMBMS feature.
    /**
     * The response of sendEmbmsAtCommand.
     *
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param result response string
     */
    public void sendEmbmsAtCommandResponse(RadioResponseInfo responseInfo, String result) {
        mMtkRil.getMtkRadioResponse().responseString(responseInfo, result);
    }
    /// M: eMBMS end

    /// M: CC: call control part @{
    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void hangupAllResponse(RadioResponseInfo responseInfo)
            throws RemoteException {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setCallIndicationResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setVoicePreferStatusResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setEccNumResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void getEccNumResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }
    /// M: CC: @}

    /// M: CC: Emergency mode for Fusion RIL @{
    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setEccModeResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }
    /// @}

    /// M: CC: Vzw/CTVolte ECC for Fusion RIL@{
    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void eccPreferredRatResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }
    /// @}

    // APC-Start
    public void setApcModeResponse(
            RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void getApcInfoResponse(
            RadioResponseInfo responseInfo, ArrayList<Integer> cellInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            int[] response = new int[cellInfo.size()];
            if (responseInfo.error == RadioError.NONE) {
                for (int i = 0; i < cellInfo.size(); i++) {
                    response[i] = cellInfo.get(i);
                }
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, response);
            }
            mMtkRil.processResponseDone(rr, responseInfo, response);
        }
    }
    // APC-End

    public void triggerModeSwitchByEccResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);
        if (rr != null) {
            if (responseInfo.error == RadioError.NONE) {
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
            }
            mMtkRil.processResponseDone(rr, responseInfo, null);
        }
    }

    @Override
    public void getSmsRuimMemoryStatusResponse(RadioResponseInfo responseInfo,
            SmsMemStatus memStatus) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            MtkIccSmsStorageStatus ret = null;
            if (responseInfo.error == RadioError.NONE) {
                ret = new MtkIccSmsStorageStatus(memStatus.used, memStatus.total);
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, ret);
            }
            mMtkRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    @Override
    public void setFdModeResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setResumeRegistrationResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);
        if (rr != null) {
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
        }
        mMtkRil.processResponseDone(rr, responseInfo, null);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param applyType response reload/store type
     */
    public void modifyModemTypeResponse(RadioResponseInfo responseInfo, int applyType) {
        mMtkRil.getMtkRadioResponse().responseInts(responseInfo, applyType);
    }

    public void handleStkCallSetupRequestFromSimWithResCodeResponse(
            RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    // PHB START
    public void queryPhbStorageInfoResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> storageInfo) {
        mMtkRil.getMtkRadioResponse().responseIntArrayList(responseInfo, storageInfo);
    }

    public void writePhbEntryResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void readPhbEntryResponse(RadioResponseInfo responseInfo,
            ArrayList<PhbEntryStructure> phbEntry) {
        responsePhbEntries(responseInfo, phbEntry);
    }

    private void
    responsePhbEntries(RadioResponseInfo responseInfo, ArrayList<PhbEntryStructure> phbEntry) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            PhbEntry[] ret = null;
            if (responseInfo.error == RadioError.NONE) {
                ret = new PhbEntry[phbEntry.size()];
                for (int i = 0; i < phbEntry.size(); i++) {
                    ret[i] = new PhbEntry();
                    ret[i].type = phbEntry.get(i).type;
                    ret[i].index = phbEntry.get(i).index;
                    ret[i].number = phbEntry.get(i).number;
                    ret[i].ton = phbEntry.get(i).ton;
                    ret[i].alphaId = phbEntry.get(i).alphaId;
                }
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, ret);
            }
            mMtkRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    public void queryUPBCapabilityResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> upbCapability) {
        mMtkRil.getMtkRadioResponse().responseIntArrayList(responseInfo, upbCapability);
    }

    public void editUPBEntryResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void deleteUPBEntryResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void readUPBGasListResponse(RadioResponseInfo responseInfo, ArrayList<String> gasList) {
        mMtkRil.getMtkRadioResponse().responseStringArrayList(mMtkRil.getMtkRadioResponse().mRil,
                responseInfo, gasList);
    }

    public void readUPBGrpEntryResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> grpEntries) {
        mMtkRil.getMtkRadioResponse().responseIntArrayList(responseInfo, grpEntries);
    }

    public void writeUPBGrpEntryResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void getPhoneBookStringsLengthResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> stringLength) {
        mMtkRil.getMtkRadioResponse().responseIntArrayList(responseInfo, stringLength);
    }

    public void getPhoneBookMemStorageResponse(RadioResponseInfo responseInfo,
            PhbMemStorageResponse phbMemStorage) {
        responseGetPhbMemStorage(responseInfo, phbMemStorage);
    }

    private void
    responseGetPhbMemStorage(RadioResponseInfo responseInfo, PhbMemStorageResponse phbMemStorage) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            PBMemStorage ret = new PBMemStorage();
            if (responseInfo.error == RadioError.NONE) {
                ret.setStorage(phbMemStorage.storage);
                ret.setUsed(phbMemStorage.used);
                ret.setTotal(phbMemStorage.total);
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, ret);
            }
            mMtkRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    public void setPhoneBookMemStorageResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void readPhoneBookEntryExtResponse(RadioResponseInfo responseInfo,
            ArrayList<PhbEntryExt> phbEntryExts) {
        responseReadPhbEntryExt(responseInfo, phbEntryExts);
    }

    private void responseCallForwardInfoEx(RadioResponseInfo responseInfo,
            ArrayList<CallForwardInfoEx> callForwardInfoExs) {
        long[] timeSlot;
        String[] timeSlotStr;
        RILRequest rr = mMtkRil.processResponse(responseInfo);
        if (rr != null) {
            MtkCallForwardInfo[] ret = new MtkCallForwardInfo[callForwardInfoExs.size()];
            for (int i = 0; i < callForwardInfoExs.size(); i++) {
                timeSlot = new long[2];
                timeSlotStr = new String[2];

                ret[i] = new MtkCallForwardInfo();
                ret[i].status = callForwardInfoExs.get(i).status;
                ret[i].reason = callForwardInfoExs.get(i).reason;
                ret[i].serviceClass = callForwardInfoExs.get(i).serviceClass;
                ret[i].toa = callForwardInfoExs.get(i).toa;
                ret[i].number = callForwardInfoExs.get(i).number;
                ret[i].timeSeconds = callForwardInfoExs.get(i).timeSeconds;
                timeSlotStr[0] = callForwardInfoExs.get(i).timeSlotBegin;
                timeSlotStr[1] = callForwardInfoExs.get(i).timeSlotEnd;

                if (timeSlotStr[0] == null || timeSlotStr[1] == null) {
                    ret[i].timeSlot = null;
                } else {
                    // convert to local time
                    for (int j = 0; j < 2; j++) {
                        SimpleDateFormat dateFormat = new SimpleDateFormat("HH:mm");
                        dateFormat.setTimeZone(TimeZone.getTimeZone("GMT+8"));
                        try {
                            Date date = dateFormat.parse(timeSlotStr[j]);
                            timeSlot[j] = date.getTime();
                        } catch (ParseException e) {
                            e.printStackTrace();
                            timeSlot = null;
                            break;
                        }
                    }
                    ret[i].timeSlot = timeSlot;
                }
            }
            if (responseInfo.error == RadioError.NONE) {
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, ret);
            }
            mMtkRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    private void
    responseReadPhbEntryExt(RadioResponseInfo responseInfo, ArrayList<PhbEntryExt> phbEntryExts) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            PBEntry[] ret = null;
            if (responseInfo.error == RadioError.NONE) {
                ret = new PBEntry[phbEntryExts.size()];
                for (int i = 0; i < phbEntryExts.size(); i++) {
                    ret[i] = new PBEntry();
                    ret[i].setIndex1(phbEntryExts.get(i).type);
                    ret[i].setNumber(phbEntryExts.get(i).number);
                    ret[i].setType(phbEntryExts.get(i).type);
                    ret[i].setText(phbEntryExts.get(i).text);
                    ret[i].setHidden(phbEntryExts.get(i).hidden);
                    ret[i].setGroup(phbEntryExts.get(i).group);
                    ret[i].setAdnumber(phbEntryExts.get(i).adnumber);
                    ret[i].setAdtype(phbEntryExts.get(i).adtype);
                    ret[i].setSecondtext(phbEntryExts.get(i).secondtext);
                    ret[i].setEmail(phbEntryExts.get(i).email);
                }
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, ret);
            }
            mMtkRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    public void writePhoneBookEntryExtResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void queryUPBAvailableResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> upbAvailable) {
        mMtkRil.getMtkRadioResponse().responseIntArrayList(responseInfo, upbAvailable);
    }

    public void readUPBEmailEntryResponse(RadioResponseInfo responseInfo, String email) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);
        if (rr != null) {
            if (responseInfo.error == RadioError.NONE) {
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, email);
            }
            String str = "xxx@email.com";
            mMtkRil.processResponseDone(rr, responseInfo, str);
        }
    }

    public void readUPBSneEntryResponse(RadioResponseInfo responseInfo, String sne) {
        mMtkRil.getMtkRadioResponse().responseString(responseInfo, sne);
    }

    public void readUPBAnrEntryResponse(RadioResponseInfo responseInfo,
            ArrayList<PhbEntryStructure> anrs) {
        responsePhbEntries(responseInfo, anrs);
    }

    public void readUPBAasListResponse(RadioResponseInfo responseInfo, ArrayList<String> aasList) {
        mMtkRil.getMtkRadioResponse().responseStringArrayList(mMtkRil.getMtkRadioResponse().mRil,
                responseInfo, aasList);
    }

    public void setPhonebookReadyResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }
    // PHB END

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void restartRILDResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    //MTK-START Femtocell (CSG)
    /**
     * Femtocell list response.
     * @param responseInfo Response info struct containing response type, serial no. and error.
     * @param femtoList response femtocell list.
     */
    public void getFemtocellListResponse(RadioResponseInfo responseInfo,
            ArrayList<String> femtoList) {
        responseFemtoCellInfos(responseInfo, femtoList);
    }

    /**
     * Cancel femtocell list response.
     * @param responseInfo Response info struct containing response type, serial no. and error.
     */
    public void abortFemtocellListResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * Select femtocell list response.
     * @param responseInfo Response info struct containing response type, serial no. and error.
     */
    public void selectFemtocellResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * Query femtocell system selection mode response.
     * @param responseInfo Response info struct containing response type, serial no. and error.
     * @param mode response femtocell system selection mode.
     */
    public void queryFemtoCellSystemSelectionModeResponse(RadioResponseInfo responseInfo,
            int mode) {
        mMtkRil.getMtkRadioResponse().responseInts(responseInfo, mode);
    }

    // MTK-START: SIM ME LOCK
    /**
     * @param info Response info struct containing response type, serial no. and error.
     */
    public void queryNetworkLockResponse(RadioResponseInfo info, int catagory,
            int state, int retry_cnt, int autolock_cnt, int num_set, int total_set,
            int key_state) {
        mMtkRil.getMtkRadioResponse().responseInts(info, catagory, state, retry_cnt, autolock_cnt,
                num_set, total_set, key_state);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error.
     */
    public void setNetworkLockResponse(RadioResponseInfo info) {
        mMtkRil.getMtkRadioResponse().responseVoid(info);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param remainingAttempts Number of retries remaining, must be equal to -1 if unknown.
     */
    public void supplyDeviceNetworkDepersonalizationResponse(RadioResponseInfo responseInfo,
            int retriesRemaining) {
        mMtkRil.getMtkRadioResponse().responseInts(responseInfo, retriesRemaining);
    }
    // MTK-END

    /**
     * Set femtocell system selection mode response.
     * @param responseInfo Response info struct containing response type, serial no. and error.
     */
    public void setFemtoCellSystemSelectionModeResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    private void
    responseFemtoCellInfos(RadioResponseInfo responseInfo,
            ArrayList<String> info) {
        RILRequest rr = mMtkRil.getMtkRadioResponse().mRil.processResponse(responseInfo);
        ArrayList<FemtoCellInfo> femtoInfos = null;
        int size_femto = 7;

        String fPlmn = null;
        String fAct = null;
        String fCsgId = null;
        Phone phone = PhoneFactory.getPhone(mMtkRil.mInstanceId);
        if (phone != null) {
            ServiceStateTracker sst = phone.getServiceStateTracker();
            ServiceState ss = sst.mSS;
            fPlmn = ((MtkServiceStateTracker) sst).getFemtoPlmn();
            fAct = ((MtkServiceStateTracker) sst).getFemtoAct();
            fCsgId = ((MtkServiceStateTracker) sst).getFemtoCsgId();
        }

        if (rr != null && responseInfo.error == RadioError.NONE) {
            String[] strings = null;

            strings = new String[info.size()];
            for (int i = 0; i < info.size(); i++) {
                strings[i] = info.get(i);
            }

            if (strings.length % size_femto != 0) {
                throw new RuntimeException(
                    "responseFemtoCellInfos: invalid response. Got "
                    + strings.length + " strings, expected multible of " + size_femto);
            }
            femtoInfos = new ArrayList<FemtoCellInfo>(strings.length / size_femto);

            /* <plmn numeric>,<act>,<plmn long alpha name>,<csgId>,,csgIconType>,<hnbName>,<sig> */
            for (int i = 0 ; i < strings.length ; i += size_femto) {
                String actStr;
                String hnbName;
                int rat;
                int sig = 0;
                boolean con = false; // plmn, act, csgId are the same.

                /* ALPS00273663 handle UCS2 format : prefix + hex string ex: "uCs2806F767C79D1" */
                if ((strings[i + 1] != null) && (strings[i + 1].startsWith("uCs2") == true))
                {
                    mMtkRil.riljLog("responseFemtoCellInfos handling UCS2 format name");

                    try {
                        strings[i + 0] = new String(
                                IccUtils.hexStringToBytes(strings[i + 1].substring(4)), "UTF-16");
                    } catch (UnsupportedEncodingException ex) {
                        mMtkRil.riljLog("responseFemtoCellInfos UnsupportedEncodingException");
                    }
                }

                if (strings[i + 1] != null
                        && (strings[i + 1].equals("") || strings[i + 1].equals(strings[i + 0]))) {
                    mMtkRil.riljLog(
                            "lookup RIL responseFemtoCellInfos() for plmn id= " + strings[i + 0]);
                    strings[i + 1] = mMtkRil.lookupOperatorName(getSubId(mMtkRil.mInstanceId),
                            strings[i + 0], true, -1);
                }

                sig = Integer.valueOf(strings[i + 6]).intValue();

                if (strings[i + 2].equals("7")) {
                    actStr = "4G";
                    rat = ServiceState.RIL_RADIO_TECHNOLOGY_LTE;
                } else if (strings[i + 2].equals("2")) {
                    actStr = "3G";
                    rat = ServiceState.RIL_RADIO_TECHNOLOGY_UMTS;
                } else {
                    actStr = "2G";
                    rat = ServiceState.RIL_RADIO_TECHNOLOGY_GPRS;
                }
                strings[i + 1] = strings[i + 1].concat(" " + actStr);

                hnbName = new String(IccUtils.hexStringToBytes(strings[i + 5]));

                if ((strings[i + 0] != null && strings[i + 0].equals(fPlmn)) &&  // Plmn
                    (strings[i + 2] != null && strings[i + 2].equals(fAct)) &&  // Act
                    (strings[i + 3] != null && strings[i + 3].equals(fCsgId))) {  // CsgId
                    con = true;
                }
                mMtkRil.riljLog("FemtoCellInfo(" + strings[i + 3] + "," + strings[i + 4] + ","
                                + strings[i + 5] + "," + strings[i + 0] + "," + strings[i + 1]
                                + "," + rat + ")" + " hnbName=" + hnbName + ",sig=" + sig
                                + ",con=" + con);
                femtoInfos.add(
                    new FemtoCellInfo(
                        Integer.parseInt(strings[i + 3]),
                        Integer.parseInt(strings[i + 4]),
                        hnbName,
                        strings[i + 0],
                        strings[i + 1],
                        rat,
                        con,
                        sig));
            }
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, femtoInfos);
        }
        mMtkRil.processResponseDone(rr, responseInfo, femtoInfos);
    }
    //MTK-END Femtocell (CSG)

    // M: [LTE][Low Power][UL traffic shaping] @{
    public void setLteAccessStratumReportResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);
        if (rr != null) {
            if (responseInfo.error == RadioError.NONE) {
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
            }
            mMtkRil.processResponseDone(rr, responseInfo, null);
        }
    }

    public void setLteUplinkDataTransferResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);
        if (rr != null) {
            if (responseInfo.error == RadioError.NONE) {
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
            }
            mMtkRil.processResponseDone(rr, responseInfo, null);
        }
    }
    // M: [LTE][Low Power][UL traffic shaping] @}

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param respAntConf Ant configuration
     */
    public void setRxTestConfigResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> respAntConf) {
        mMtkRil.getMtkRadioResponse().responseIntArrayList(responseInfo, respAntConf);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param respAntInfo Ant Info
     */
    public void getRxTestResultResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> respAntInfo) {
        mMtkRil.getMtkRadioResponse().responseIntArrayList(responseInfo, respAntInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param polCapability the order number of operator in the SIM/USIM preferred operator list
     */
    public void getPOLCapabilityResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> polCapability) {
        mMtkRil.getMtkRadioResponse().responseIntArrayList(responseInfo, polCapability);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param polList the SIM/USIM preferred operator list
     */
    public void getCurrentPOLListResponse(RadioResponseInfo responseInfo,
            ArrayList<String> polList) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);
        ArrayList<NetworkInfoWithAcT> NetworkInfos = null;
        if (rr != null) {
            if (responseInfo.error == RadioError.NONE) {
                if (polList.size() % 4 != 0) {
                    mMtkRil.riljLog("RIL_REQUEST_GET_POL_LIST: invalid response. Got "
                        + polList.size() + " strings, expected multible of 4");
                } else {
                    NetworkInfos = new ArrayList<NetworkInfoWithAcT>(polList.size() / 4);
                    String strOperName = null;
                    String strOperNumeric = null;
                    int nAct = 0;
                    int nIndex = 0;
                    for (int i = 0; i < polList.size(); i+=4) {
                        strOperName = null;
                        strOperNumeric = null;
                        nAct = 0;
                        nIndex = 0;
                        if (polList.get(i) != null) nIndex = Integer.parseInt(polList.get(i));
                        if (polList.get(i+1) != null) {
                            int format = Integer.parseInt(polList.get(i+1));
                            switch (format) {
                                case 0:
                                case 1:
                                    strOperName = polList.get(i+2);
                                    break;
                                case 2:
                                    if (polList.get(i+2) != null) {
                                        strOperNumeric = polList.get(i+2);
                                        strOperName = mMtkRil.lookupOperatorName(
                                                getSubId(mMtkRil.mInstanceId),
                                                strOperNumeric, true, -1);
                                    }
                                    break;
                                default:
                                    break;
                            }
                        }
                        if (polList.get(i+3) != null) nAct = Integer.parseInt(polList.get(i+3));
                        if (strOperNumeric != null && !strOperNumeric.equals("?????")) {
                            NetworkInfos.add(
                                new NetworkInfoWithAcT(
                                    strOperName,
                                    strOperNumeric,
                                    nAct,
                                    nIndex));
                        }
                    }
                    mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, NetworkInfos);
                }
            }
            mMtkRil.processResponseDone(rr, responseInfo, NetworkInfos);
        }
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setPOLEntryResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    // M: Data Framework - common part enhancement
    public void syncDataSettingsToMdResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    // M: Data Framework - Data Retry enhancement
    public void resetMdDataRetryCountResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }
    // M: Data Framework - CC 33
    public void setRemoveRestrictEutranModeResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /// M: [Network][C2K] Sprint roaming control @{
    public void setRoamingEnableResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void getRoamingEnableResponse(RadioResponseInfo responseInfo, ArrayList<Integer> data) {
        mMtkRil.getMtkRadioResponse().responseIntArrayList(responseInfo, data);
    }
    /// @}

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setLteReleaseVersionResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param mode setting mode of lte release version.
     */
    public void getLteReleaseVersionResponse(RadioResponseInfo responseInfo, int mode) {
        mMtkRil.getMtkRadioResponse().responseInts(responseInfo, mode);
    }

    // External SIM [Start]
    public void vsimNotificationResponse(RadioResponseInfo info, VsimEvent event) {
        RILRequest rr = mMtkRil.processResponse(info);

        if (rr != null) {
            Object ret = null;
            if (info.error == RadioError.NONE) {
                ret = (Object) event.transactionId;
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, ret);
            }
            mMtkRil.processResponseDone(rr, info, ret);
        }
    }

    public void vsimOperationResponse(RadioResponseInfo info) {
        mMtkRil.getMtkRadioResponse().responseVoid(info);
    }
    // External SIM [End]

    public void setWifiEnabledResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
        }
        mMtkRil.processResponseDone(rr, responseInfo, null);
    }

    public void setWifiAssociatedResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
        }
        mMtkRil.processResponseDone(rr, responseInfo, null);
    }

    public void setWifiSignalLevelResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
        }
        mMtkRil.processResponseDone(rr, responseInfo, null);
    }

    public void setWifiIpAddressResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
        }
        mMtkRil.processResponseDone(rr, responseInfo, null);
    }

    public void setLocationInfoResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
        }
        mMtkRil.processResponseDone(rr, responseInfo, null);
    }

    public void setEmergencyAddressIdResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
        }
        mMtkRil.processResponseDone(rr, responseInfo, null);
    }

    public void setNattKeepAliveStatusResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
        }
        mMtkRil.processResponseDone(rr, responseInfo, null);
    }

    public void setWifiPingResultResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
        }
        mMtkRil.processResponseDone(rr, responseInfo, null);
    }

     ///M: Notify ePDG screen state
    public void notifyEPDGScreenStateResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
        }
        mMtkRil.processResponseDone(rr, responseInfo, null);
    }

    public void setServiceStateToModemResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param data Data returned by oem
     */
    public void sendRequestRawResponse(RadioResponseInfo responseInfo, ArrayList<Byte> data) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            byte[] ret = null;
            if (responseInfo.error == RadioError.NONE) {
                ret = RIL.arrayListToPrimitiveArray(data);
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, ret);
            }
            mMtkRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    public void setTxPowerStatusResponse(RadioResponseInfo responseInfo,
            ArrayList<Byte> data) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);
        if (rr != null) {
            if (responseInfo.error == RadioError.NONE) {
                mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
            }
            mMtkRil.processResponseDone(rr, responseInfo, null);
        } else {
            mMtkRil.riljLog("setTxPowerStatusResponse, rr is null");
        }
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param data Data returned by oem
     */
    public void sendRequestStringsResponse(RadioResponseInfo responseInfo, ArrayList<String> data) {
        mMtkRil.getMtkRadioResponse().responseStringArrayList(mMtkRil, responseInfo, data);
    }

    public void dataConnectionAttachResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void dataConnectionDetachResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void resetAllConnectionsResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void setSuppServPropertyResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void setVendorSettingResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    // M: GWSDS @{
    public void setGwsdModeResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void setCallValidTimerResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void setIgnoreSameNumberIntervalResponse(
            RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void setKeepAliveByPDCPCtrlPDUResponse(
            RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    public void setKeepAliveByIpDataResponse(
            RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }
    // @}

    public void hangupWithReasonResponse(RadioResponseInfo responseInfo) {
        mMtkRil.getMtkRadioResponse().responseVoid(responseInfo);
    }

    /**
     * Response for request 'registerCellQltyReport'
     * @param responseInfo Response info containing response type, serial no. and error
     */
    public void registerCellQltyReportResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
        }
        mMtkRil.processResponseDone(rr, responseInfo, null);
    }

    public void getSuggestedPlmnListResponse(RadioResponseInfo responseInfo,
            ArrayList<String> data) {
        mMtkRil.getMtkRadioResponse().responseStringArrayList(mMtkRil, responseInfo, data);
    }

    public void sendSarIndicatorResponse(RadioResponseInfo info) {
        RILRequest rr = mMtkRil.processResponse(info);
        if (rr != null) {
            if (info.error == RadioError.NONE) {
                 mMtkRil.getMtkRadioResponse().sendMessageResponse(rr.mResult, null);
            }
            mMtkRil.processResponseDone(rr, info, null);
        } else {
            mMtkRil.riljLog("sendSarIndicatorResponse, rr is null");
        }
    }

    public void sendRsuRequestResponse(RadioResponseInfo responseInfo,
                                       RsuResponseInfo data) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);
        if (rr != null) {
            if (responseInfo.error == RadioError.NONE) {
                AsyncResult.forMessage(rr.mResult, data, null);
                rr.mResult.sendToTarget();
            }
            mMtkRil.processResponseDone(rr, responseInfo, data);
        } else {
            mMtkRil.riljLog("sendRsuRequestResponse, rr is null");
        }
    }

}

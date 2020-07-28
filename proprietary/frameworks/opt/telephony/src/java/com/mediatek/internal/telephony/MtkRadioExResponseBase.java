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

import vendor.mediatek.hardware.mtkradioex.V1_9.IMtkRadioExResponse;
import vendor.mediatek.hardware.mtkradioex.V1_0.PhbEntryExt;
import vendor.mediatek.hardware.mtkradioex.V1_0.PhbEntryStructure;
import vendor.mediatek.hardware.mtkradioex.V1_0.PhbMemStorageResponse;
import vendor.mediatek.hardware.mtkradioex.V1_0.CallForwardInfoEx;

import android.hardware.radio.V1_0.ActivityStatsInfo;
import android.hardware.radio.V1_0.CardStatus;
import android.hardware.radio.V1_0.CarrierRestrictions;
import android.hardware.radio.V1_0.CdmaBroadcastSmsConfigInfo;
import android.hardware.radio.V1_0.DataRegStateResult;
import android.hardware.radio.V1_0.GsmBroadcastSmsConfigInfo;
import android.hardware.radio.V1_0.LastCallFailCauseInfo;
import android.hardware.radio.V1_0.LceDataInfo;
import android.hardware.radio.V1_0.LceStatusInfo;
import android.hardware.radio.V1_0.NeighboringCell;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.SendSmsResult;
import android.hardware.radio.V1_0.SetupDataCallResult;
import android.hardware.radio.V1_0.VoiceRegStateResult;
import android.hardware.radio.V1_1.KeepaliveStatus;
import android.os.RemoteException;
import java.util.ArrayList;

// SMS-START
import vendor.mediatek.hardware.mtkradioex.V1_0.SmsParams;
import vendor.mediatek.hardware.mtkradioex.V1_0.SmsMemStatus;
// SMS-END

// External SIM [Start]
import vendor.mediatek.hardware.mtkradioex.V1_0.VsimEvent;
// External SIM [End]

// NW
import vendor.mediatek.hardware.mtkradioex.V1_0.SignalStrengthWithWcdmaEcio;
//RSU
import vendor.mediatek.hardware.mtkradioex.V1_9.RsuResponseInfo;

public class MtkRadioExResponseBase extends IMtkRadioExResponse.Stub {

    public MtkRadioExResponseBase(RIL ril) {
    }

    /**
     * Acknowledge the receipt of radio request sent to the vendor. This must be sent only for
     * radio request which take long time to respond.
     * For more details, refer https://source.android.com/devices/tech/connect/ril.html
     *
     * @param serial Serial no. of the request whose acknowledgement is sent.
     */
    public void acknowledgeRequest(int serial) {
    }

    /* MTK SS Feature : Start */
    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setClipResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param n Colp status in network, "0" means disabled, "1" means enabled
     * @param m Service status, "0" means not provisioned, "1" means provisioned in permanent mode
     */
    public void getColpResponse(RadioResponseInfo responseInfo, int n, int m) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param status indicates COLR status. "0" means not provisioned, "1" means provisioned,
     *        "2" means unknown
     */
    public void getColrResponse(RadioResponseInfo responseInfo, int status) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param n CNAP status, "0" means disabled, "1" means enabled.
     * @param m Service status, "0" means not provisioned, "1" means provisioned, "2" means unknown
     */
    public void sendCnapResponse(RadioResponseInfo responseInfo, int n, int m) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setColpResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setColrResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param callForwardInfoExs points to a vector of CallForwardInfoEx, one for
     *        each distinct registered phone number.
     */
    public void queryCallForwardInTimeSlotStatusResponse(RadioResponseInfo responseInfo,
            ArrayList<CallForwardInfoEx> callForwardInfoExs) {
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     *
     */
    public void setCallForwardInTimeSlotResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param resList points to a vector of String.
     */
    public void runGbaAuthenticationResponse(RadioResponseInfo responseInfo,
                                             ArrayList<String> resList) {
    }
    /* MTK SS Feature : End */

    public void sendOemRilRequestRawResponse(RadioResponseInfo responseInfo,
                                             ArrayList<Byte> var2) {}

    public void setTrmResponse(RadioResponseInfo responseInfo) {
    }

    // MTK-START: SIM
    /**
     * @param info Response info struct containing response type, serial no. and error.
     * @param response Response string of getATRResponse.
     */
    public void getATRResponse(RadioResponseInfo info, String response) {
    }

    /**
     * @param info Response info struct containing response type, serial no. and error.
     * @param response Response string of getIccidResponse.
     */
    public void getIccidResponse(RadioResponseInfo info, String response) {
    }

    /**
     * @param info Response info struct containing response type, serial no. and error.
     */
    public void setSimPowerResponse(RadioResponseInfo info) {
    }

    /**
     * @param info Response info struct containing response type, serial no. and error.
     * @param simPowerOnOffResponse result of activating sim power.
     */
    public void activateUiccCardRsp(RadioResponseInfo info, int simPowerOnOffResponse) {
    }

    /**
     * @param info Response info struct containing response type, serial no. and error.
     * @param simPowerOnOffResponse result of deactivating sim power.
     */
    public void deactivateUiccCardRsp(RadioResponseInfo info, int simPowerOnOffResponse) {
    }

    /**
     * @param info Response info struct containing response type, serial no. and error.
     * @param simPowerOnOffStatus status of sim power state.
     */
    public void getCurrentUiccCardProvisioningStatusRsp(RadioResponseInfo info,
            int simPowerOnOffStatus) {
    }
    // MTK-END

    // MTK-START: NW
    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setNetworkSelectionModeManualWithActResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param networkInfos List of network operator information as OperatorInfoWithAct defined in
     *                     types.hal
     */
    public void getAvailableNetworksWithActResponse(RadioResponseInfo responseInfo,
                                            ArrayList<vendor.mediatek.hardware.
                                            mtkradioex.V1_0.OperatorInfoWithAct> networkInfos) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param customerSignalStrength list of signalstrength with wcdma ecio
     */
    public void getSignalStrengthWithWcdmaEcioResponse(RadioResponseInfo responseInfo,
            SignalStrengthWithWcdmaEcio signalStrength) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void cancelAvailableNetworksResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void cfgA2offsetResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void cfgB1offsetResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void enableSCGfailureResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void disableNRResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setTxPowerResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setSearchStoredFreqInfoResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setSearchRatResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setBgsrchDeltaSleepTimerResponse(RadioResponseInfo responseInfo) {
    }

    /*
    * @param responseInfo Response info struct containing response type, serial no. and error
    */
    public void setModemPowerResponse(RadioResponseInfo responseInfo) {
    }

    // SMS-START
    public void getSmsParametersResponse(
            RadioResponseInfo responseInfo, SmsParams params) {
    }

    public void setSmsParametersResponse(
            RadioResponseInfo responseInfo) {
    }
    public void setEtwsResponse(
            RadioResponseInfo responseInfo) {
    }

    public void removeCbMsgResponse(
            RadioResponseInfo responseInfo) {
    }


    public void getSmsMemStatusResponse(
            RadioResponseInfo responseInfo, SmsMemStatus params) {
    }

    public void setGsmBroadcastLangsResponse(
            RadioResponseInfo responseInfo) {
    }

    public void getGsmBroadcastLangsResponse(
            RadioResponseInfo responseInfo, String langs) {
    }

    public void getGsmBroadcastActivationRsp(RadioResponseInfo responseInfo,
            int activation) {
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
    }
    /// M: eMBMS end

    /// M: CC: call control part @{
    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void hangupAllResponse(RadioResponseInfo responseInfo)
            throws RemoteException {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setCallIndicationResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setVoicePreferStatusResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setEccNumResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void getEccNumResponse(RadioResponseInfo responseInfo) {
    }
    /// M: CC: @}

    /// M: CC: Emergency mode for Fusion RIL @{
    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setEccModeResponse(RadioResponseInfo responseInfo) {
    }
    /// @}

    /// M: CC: Vzw/CTVolte ECC for Fusion RIL@{
    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void eccPreferredRatResponse(RadioResponseInfo responseInfo) {
    }
    /// @}

    /// [Softbank requirement]: Enable or disable call sub address feature. @{
    public void setCallSubAddressResponse(RadioResponseInfo responseInfo) {
    }

    public void getCallSubAddressResponse(RadioResponseInfo info, int enable) {
    }
    /// @}

    // APC-Start
    public void setApcModeResponse(
            RadioResponseInfo responseInfo) {
    }

    public void getApcInfoResponse(
            RadioResponseInfo responseInfo, ArrayList<Integer> cellInfo) {
    }
    // APC-End

    public void triggerModeSwitchByEccResponse(RadioResponseInfo responseInfo) {
    }

    @Override
    public void getSmsRuimMemoryStatusResponse(RadioResponseInfo responseInfo,
            SmsMemStatus memStatus) {
    }

    @Override
    public void setFdModeResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setResumeRegistrationResponse(RadioResponseInfo responseInfo) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param applyType response reload/store type
     */
    public void modifyModemTypeResponse(RadioResponseInfo responseInfo, int applyType) {
    }

    public void handleStkCallSetupRequestFromSimWithResCodeResponse(
            RadioResponseInfo responseInfo) {
    }

    // PHB START, interface only currently.
    public void queryPhbStorageInfoResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> storageInfo) {
    }

    public void writePhbEntryResponse(RadioResponseInfo responseInfo) {
    }

    public void readPhbEntryResponse(RadioResponseInfo responseInfo,
            ArrayList<PhbEntryStructure> phbEntry) {
    }

    public void queryUPBCapabilityResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> upbCapability) {
    }

    public void editUPBEntryResponse(RadioResponseInfo responseInfo) {
    }

    public void deleteUPBEntryResponse(RadioResponseInfo responseInfo) {
    }

    public void readUPBGasListResponse(RadioResponseInfo responseInfo, ArrayList<String> gasList) {
    }

    public void readUPBGrpEntryResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> grpEntries) {
    }

    public void writeUPBGrpEntryResponse(RadioResponseInfo responseInfo) {
    }

    public void getPhoneBookStringsLengthResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> stringLength) {
    }

    public void getPhoneBookMemStorageResponse(RadioResponseInfo responseInfo,
            PhbMemStorageResponse phbMemStorage) {
    }

    public void setPhoneBookMemStorageResponse(RadioResponseInfo responseInfo) {
    }

    public void readPhoneBookEntryExtResponse(RadioResponseInfo responseInfo,
            ArrayList<PhbEntryExt> phbEntryExts) {
    }

    public void writePhoneBookEntryExtResponse(RadioResponseInfo responseInfo) {
    }

    public void queryUPBAvailableResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> upbAvailable) {
    }

    public void readUPBEmailEntryResponse(RadioResponseInfo responseInfo, String email) {
    }

    public void readUPBSneEntryResponse(RadioResponseInfo responseInfo, String sne) {
    }

    public void readUPBAnrEntryResponse(RadioResponseInfo responseInfo,
            ArrayList<PhbEntryStructure> anrs) {
    }

    public void readUPBAasListResponse(RadioResponseInfo responseInfo, ArrayList<String> aasList) {
    }

    public void setPhonebookReadyResponse(RadioResponseInfo responseInfo) {
    }
    // PHB END

    public void restartRILDResponse(RadioResponseInfo responseInfo) {
    }

    //Femtocell (CSG)-START
    public void getFemtocellListResponse(RadioResponseInfo responseInfo,
            ArrayList<String> femtoList) {
    }

    public void abortFemtocellListResponse(RadioResponseInfo responseInfo) {
    }

    public void selectFemtocellResponse(RadioResponseInfo responseInfo) {
    }

    public void queryFemtoCellSystemSelectionModeResponse(RadioResponseInfo responseInfo,
            int mode) {
    }

    public void setFemtoCellSystemSelectionModeResponse(RadioResponseInfo responseInfo) {
    }
    //Femtocell (CSG)-END

    // M: Data Framework - common part enhancement
    public void syncDataSettingsToMdResponse(RadioResponseInfo responseInfo) {}

    // M: Data Framework - Data Retry enhancement
    public void resetMdDataRetryCountResponse(RadioResponseInfo responseInfo) {}

    // M: Data Framework - CC 33
    public void setRemoveRestrictEutranModeResponse(RadioResponseInfo responseInfo) {}

    // M: [LTE][Low Power][UL traffic shaping] @{
    public void setLteAccessStratumReportResponse(RadioResponseInfo responseInfo) {}
    public void setLteUplinkDataTransferResponse(RadioResponseInfo responseInfo) {}
    // M: [LTE][Low Power][UL traffic shaping] @}
    // MTK-START: SIM ME LOCK
    public void queryNetworkLockResponse(RadioResponseInfo info, int catagory,
            int state, int retry_cnt, int autolock_cnt, int num_set, int total_set,
            int key_state) {
    }
    public void setNetworkLockResponse(RadioResponseInfo info) {
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param retriesRemaining Number of retries remaining.
     */
    public void supplyDepersonalizationResponse(RadioResponseInfo responseInfo,
            int retriesRemaining) {
    }

    public void supplyDeviceNetworkDepersonalizationResponse(RadioResponseInfo responseInfo,
            int remainingAttempts) {
    }
    // MTK-END

    public void setRxTestConfigResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> respAntConf) {
    }

    public void getRxTestResultResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> respAntInfo) {
    }

    public void getPOLCapabilityResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> polCapability) {
    }

    public void getCurrentPOLListResponse(RadioResponseInfo responseInfo,
            ArrayList<String> polList) {
    }

    public void setPOLEntryResponse(RadioResponseInfo responseInfo) {
    }

    /// M: [Network][C2K] Sprint roaming control @{
    public void setRoamingEnableResponse(RadioResponseInfo responseInfo) {
    }

    public void getRoamingEnableResponse(RadioResponseInfo responseInfo, ArrayList<Integer> data) {
    }
    /// @}

    public void setLteReleaseVersionResponse(RadioResponseInfo responseInfo) {
    }

    public void getLteReleaseVersionResponse(RadioResponseInfo responseInfo, int mode) {
    }

    // External SIM [Start]
    public void vsimNotificationResponse(RadioResponseInfo info, VsimEvent event) {
    }

    public void vsimOperationResponse(RadioResponseInfo info) {
    }
    // External SIM [End]

    public void setWifiEnabledResponse(RadioResponseInfo responseInfo) {
    }

    public void setWifiAssociatedResponse(RadioResponseInfo responseInfo) {
    }

    public void setWifiSignalLevelResponse(RadioResponseInfo responseInfo) {
    }

    public void setWifiIpAddressResponse(RadioResponseInfo responseInfo) {
    }

    public void setLocationInfoResponse(RadioResponseInfo responseInfo) {
    }

    public void setEmergencyAddressIdResponse(RadioResponseInfo responseInfo) {
    }

    public void setNattKeepAliveStatusResponse(RadioResponseInfo responseInfo) {
    }

    public void setWifiPingResultResponse(RadioResponseInfo responseInfo) {
    }

    /// M: Notify ePDG screen state
    public void notifyEPDGScreenStateResponse(RadioResponseInfo responseInfo) {
    }

    public void setServiceStateToModemResponse(RadioResponseInfo responseInfo) {
    }

    public void sendRequestRawResponse(RadioResponseInfo responseInfo, ArrayList<Byte> data) {
    }

    public void sendRequestStringsResponse(RadioResponseInfo responseInfo, ArrayList<String> data) {
    }

    public void dataConnectionAttachResponse(RadioResponseInfo responseInfo) {
    }

    public void dataConnectionDetachResponse(RadioResponseInfo responseInfo) {
    }

    public void resetAllConnectionsResponse(RadioResponseInfo responseInfo) {
    }

    public void setTxPowerStatusResponse(RadioResponseInfo responseInfo) {
    }

    public void setSuppServPropertyResponse(RadioResponseInfo responseInfo) {
    }

    public void hangupWithReasonResponse(RadioResponseInfo responseInfo) {
    }

    public void setVendorSettingResponse(RadioResponseInfo responseInfo) {
    }

    public void getPlmnNameFromSE13TableResponse(RadioResponseInfo info, String name) {
    }

    public void enableCAPlusBandWidthFilterResponse(RadioResponseInfo info) {
    }

    // M: GWSDS @{
    public void setGwsdModeResponse(RadioResponseInfo responseInfo) {
    }

    public void setCallValidTimerResponse(RadioResponseInfo responseInfo) {
    }

    public void setIgnoreSameNumberIntervalResponse(RadioResponseInfo responseInfo) {
    }

    public void setKeepAliveByPDCPCtrlPDUResponse(RadioResponseInfo responseInfo) {
    }

    public void setKeepAliveByIpDataResponse(RadioResponseInfo responseInfo) {
    }
    // @}

    public void enableDsdaIndicationResponse(RadioResponseInfo responseInfo) {
    }

    public void getDsdaStatusResponse(RadioResponseInfo responseInfo, int mode) {
    }

    public void registerCellQltyReportResponse(RadioResponseInfo responseInfo) {
    }

    public void getSuggestedPlmnListResponse(RadioResponseInfo responseInfo,
            ArrayList<String> data) {
    }

    public void setMaxUlSpeedResponse(RadioResponseInfo responseInfo) {
    }

    public void deactivateNrScgCommunicationResponse(RadioResponseInfo responseInfo) {
    }

    public void getDeactivateNrScgCommunicationResponse(RadioResponseInfo responseInfo,
            int deactivate, int allowSCGAdd) {
    }

    public void sendSarIndicatorResponse(RadioResponseInfo info) {
    }

    public void sendRsuRequestResponse(RadioResponseInfo responseInfo, RsuResponseInfo data) {
    }
}

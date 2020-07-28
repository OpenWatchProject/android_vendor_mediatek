/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __RIL_CLIENT_COMMAND_H__
#define __RIL_CLIENT_COMMAND_H__
    // AOSP request
    {RIL_REQUEST_GET_SIM_STATUS, dispatchVoid, responseSimStatus },
    {RIL_REQUEST_ENTER_SIM_PIN, dispatchStrings, responseInts },
    {RIL_REQUEST_ENTER_SIM_PUK, dispatchStrings, responseInts },
    {RIL_REQUEST_ENTER_SIM_PIN2, dispatchStrings, responseInts },
    {RIL_REQUEST_ENTER_SIM_PUK2, dispatchStrings, responseInts },
    {RIL_REQUEST_CHANGE_SIM_PIN, dispatchStrings, responseInts },
    {RIL_REQUEST_CHANGE_SIM_PIN2, dispatchStrings, responseInts },
    {RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION, dispatchStrings, responseInts },
    {RIL_REQUEST_GET_CURRENT_CALLS, dispatchVoid, responseCallList },
    {RIL_REQUEST_DIAL, dispatchDial, responseVoid },
    {RIL_REQUEST_EMERGENCY_DIAL, dispatchEmergencyDial, responseVoid },
    {RIL_REQUEST_GET_IMSI, dispatchStrings, responseString },
    {RIL_REQUEST_HANGUP, dispatchInts, responseVoid },
    {RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND, dispatchVoid, responseVoid },
    {RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND, dispatchVoid, responseVoid },
    {RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, dispatchVoid, responseVoid },
    {RIL_REQUEST_CONFERENCE, dispatchVoid, responseVoid },
    {RIL_REQUEST_UDUB, dispatchVoid, responseVoid },
    {RIL_REQUEST_LAST_CALL_FAIL_CAUSE, dispatchVoid, responseFailCause },
    {RIL_REQUEST_SIGNAL_STRENGTH, dispatchVoid, responseRilSignalStrength },
    {RIL_REQUEST_VOICE_REGISTRATION_STATE, dispatchVoid, responseVoiceRegistrationState },
    {RIL_REQUEST_DATA_REGISTRATION_STATE, dispatchVoid, responseDataRegistrationState },
    {RIL_REQUEST_OPERATOR, dispatchVoid, responseStrings },
    {RIL_REQUEST_RADIO_POWER, dispatchInts, responseVoid },
    {RIL_REQUEST_DTMF, dispatchString, responseVoid },
    {RIL_REQUEST_SEND_SMS, dispatchStrings, responseSMS },
    {RIL_REQUEST_SEND_SMS_EXPECT_MORE, dispatchStrings, responseSMS },
    {RIL_REQUEST_SETUP_DATA_CALL, dispatchDataCall, responseSetupDataCall },
    {RIL_REQUEST_SIM_IO, dispatchSIM_IO, responseSIM_IO },
    {RIL_REQUEST_SEND_USSD, dispatchString, responseVoid },
    {RIL_REQUEST_CANCEL_USSD, dispatchVoid, responseVoid },
    {RIL_REQUEST_GET_CLIR, dispatchVoid, responseInts },
    {RIL_REQUEST_SET_CLIR, dispatchInts, responseVoid },
    {RIL_REQUEST_QUERY_CALL_FORWARD_STATUS, dispatchCallForward, responseCallForwards },
    {RIL_REQUEST_SET_CALL_FORWARD, dispatchCallForward, responseVoid },
    {RIL_REQUEST_QUERY_CALL_WAITING, dispatchInts, responseInts },   /* Solve [ALPS00284553] Change to 1, mtk04070, 20120516 */
    {RIL_REQUEST_SET_CALL_WAITING, dispatchInts, responseVoid },
    {RIL_REQUEST_SMS_ACKNOWLEDGE, dispatchInts, responseVoid },
    {RIL_REQUEST_GET_IMEI, dispatchVoid, responseString },
    {RIL_REQUEST_GET_IMEISV, dispatchVoid, responseString },
    {RIL_REQUEST_ANSWER,dispatchVoid, responseVoid },
    {RIL_REQUEST_DEACTIVATE_DATA_CALL, dispatchStrings, responseVoid },
    {RIL_REQUEST_QUERY_FACILITY_LOCK, dispatchStrings, responseInts },
    {RIL_REQUEST_SET_FACILITY_LOCK, dispatchStrings, responseInts },
    {RIL_REQUEST_CHANGE_BARRING_PASSWORD, dispatchStrings, responseVoid },
    {RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE, dispatchVoid, responseInts },
    {RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, dispatchVoid, responseVoid },
    {RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL, dispatchString, responseVoid },
    {RIL_REQUEST_QUERY_AVAILABLE_NETWORKS , dispatchVoid, responseStrings },
    {RIL_REQUEST_DTMF_START, dispatchString, responseVoid },
    {RIL_REQUEST_DTMF_STOP, dispatchVoid, responseVoid },
    {RIL_REQUEST_BASEBAND_VERSION, dispatchVoid, responseString },
    {RIL_REQUEST_SEPARATE_CONNECTION, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_MUTE, dispatchInts, responseVoid },
    {RIL_REQUEST_GET_MUTE, dispatchVoid, responseInts },
    {RIL_REQUEST_QUERY_CLIP, dispatchVoid, responseInts },
    {RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE, dispatchVoid, responseInts },
    {RIL_REQUEST_DATA_CALL_LIST, dispatchVoid, responseDataCallList },
    {RIL_REQUEST_RESET_RADIO, dispatchVoid, responseVoid },
    {RIL_REQUEST_RESTART_RILD, dispatchVoid, responseVoid },
    {RIL_REQUEST_OEM_HOOK_RAW, dispatchRaw, responseRaw },
    {RIL_REQUEST_OEM_HOOK_STRINGS, dispatchStrings, responseStrings },
    {RIL_REQUEST_SCREEN_STATE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION, dispatchInts, responseVoid },
    {RIL_REQUEST_WRITE_SMS_TO_SIM, dispatchSmsWrite, responseInts },
    {RIL_REQUEST_DELETE_SMS_ON_SIM, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_BAND_MODE, dispatchInts, responseVoid },
    {RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE, dispatchVoid, responseInts },
    {RIL_REQUEST_STK_GET_PROFILE, dispatchVoid, responseString },
    {RIL_REQUEST_STK_SET_PROFILE, dispatchString, responseVoid },
    {RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND, dispatchString, responseString },
    {RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE, dispatchString, responseVoid },
    {RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM, dispatchInts, responseVoid },
    {RIL_REQUEST_EXPLICIT_CALL_TRANSFER, dispatchVoid, responseVoid },
    {RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, dispatchInts, responseVoid },
    {RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE, dispatchVoid, responseInts },
    {RIL_REQUEST_GET_NEIGHBORING_CELL_IDS, dispatchVoid, responseCellList },
    {RIL_REQUEST_SET_LOCATION_UPDATES, dispatchInts, responseVoid },
    {RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, dispatchInts, responseVoid },
    {RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, dispatchInts, responseVoid },
    {RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE, dispatchVoid, responseInts },
    {RIL_REQUEST_SET_TTY_MODE, dispatchInts, responseVoid },
    {RIL_REQUEST_QUERY_TTY_MODE, dispatchVoid, responseInts },
    {RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE, dispatchInts, responseVoid },
    {RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE, dispatchVoid, responseInts },
    {RIL_REQUEST_CDMA_FLASH, dispatchString, responseVoid },
    {RIL_REQUEST_CDMA_BURST_DTMF, dispatchStrings, responseVoid },
    {RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY, dispatchString, responseVoid },
    {RIL_REQUEST_CDMA_SEND_SMS, dispatchCdmaSms, responseSMS },
    {RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, dispatchCdmaSmsAck, responseVoid },
    {RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG, dispatchVoid, responseGsmBrSmsCnf },
    {RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG, dispatchGsmBrSmsCnf, responseVoid },
    {RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION, dispatchInts, responseVoid },
    {RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, dispatchVoid, responseCdmaBrSmsCnf },
    {RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, dispatchCdmaBrSmsCnf, responseVoid },
    {RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, dispatchInts, responseVoid },
    {RIL_REQUEST_CDMA_SUBSCRIPTION, dispatchVoid, responseStrings },
    {RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM, dispatchRilCdmaSmsWriteArgs, responseInts },
    {RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, dispatchInts, responseVoid },
    {RIL_REQUEST_DEVICE_IDENTITY, dispatchVoid, responseStrings },
    {RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE, dispatchVoid, responseVoid },
    {RIL_REQUEST_GET_SMSC_ADDRESS, dispatchVoid, responseString },
    {RIL_REQUEST_SET_SMSC_ADDRESS, dispatchString, responseVoid },
    {RIL_REQUEST_REPORT_SMS_MEMORY_STATUS, dispatchInts, responseVoid },
    {RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING, dispatchVoid, responseVoid },
    {RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE, dispatchVoid, responseInts },
    {RIL_REQUEST_ISIM_AUTHENTICATION, dispatchString, responseString },
    {RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU, dispatchStrings, responseVoid },
    {RIL_REQUEST_SET_SMS_FWK_READY, dispatchVoid, responseVoid },
    {RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS, dispatchString, responseSIM_IO },
    {RIL_REQUEST_VOICE_RADIO_TECH, dispatchVoid, responseInts },
    {RIL_REQUEST_GET_CELL_INFO_LIST, dispatchVoid, responseCellInfoList }, // ALPS01286560: getallcellinfo might be pending by PLMN list such long time request and case SWT .
    {RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_INITIAL_ATTACH_APN, dispatchSetInitialAttachApn, responseVoid }, // GSM CH should go with RIL_REQUEST_RADIO_POWER, C2K CH dont care
    {RIL_REQUEST_IMS_REGISTRATION_STATE, dispatchVoid, responseInts },
    {RIL_REQUEST_IMS_SEND_SMS, dispatchImsSms, responseSMS },
    {RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC, dispatchSIM_APDU, responseSIM_IO },
    //{RIL_REQUEST_SIM_OPEN_CHANNEL, dispatchOpenChannelParams, responseInts },
    {RIL_REQUEST_SIM_CLOSE_CHANNEL, dispatchInts, responseVoid },
    {RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL, dispatchSIM_APDU, responseSIM_IO },
    {RIL_REQUEST_NV_READ_ITEM, dispatchNVReadItem, responseString },
    {RIL_REQUEST_NV_WRITE_ITEM, dispatchNVWriteItem, responseVoid },
    {RIL_REQUEST_NV_WRITE_CDMA_PRL, dispatchRaw, responseVoid },
    {RIL_REQUEST_NV_RESET_CONFIG, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_UICC_SUBSCRIPTION, dispatchUiccSubscripton, responseVoid },
    {RIL_REQUEST_ALLOW_DATA, dispatchInts, responseVoid },
    {RIL_REQUEST_GET_HARDWARE_CONFIG, dispatchVoid, responseHardwareConfig },
    {RIL_REQUEST_SIM_AUTHENTICATION, dispatchSimAuthentication, responseSIM_IO },
    {RIL_REQUEST_GET_DC_RT_INFO, dispatchVoid, responseDcRtInfo },
    {RIL_REQUEST_SET_DC_RT_INFO_RATE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_DATA_PROFILE, dispatchDataProfile, responseVoid },
    {RIL_REQUEST_SHUTDOWN, dispatchVoid, responseVoid },
    {RIL_REQUEST_GET_RADIO_CAPABILITY, dispatchVoid, responseRadioCapability },
    {RIL_REQUEST_SET_RADIO_CAPABILITY, dispatchRadioCapability, responseRadioCapability },
    {RIL_REQUEST_START_LCE, dispatchInts, responseLceStatus },
    {RIL_REQUEST_STOP_LCE, dispatchVoid, responseLceStatus },
    {RIL_REQUEST_PULL_LCEDATA, dispatchVoid, responseLceData },
    {RIL_REQUEST_GET_ACTIVITY_INFO, dispatchVoid, responseActivityData },
    {RIL_REQUEST_SET_ALLOWED_CARRIERS, dispatchCarrierRestrictions, responseInts },
    {RIL_REQUEST_GET_ALLOWED_CARRIERS, dispatchVoid, responseCarrierRestrictions },
    {RIL_REQUEST_SEND_DEVICE_STATE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_SIM_CARD_POWER, dispatchInts, responseVoid },
    //{RIL_REQUEST_START_NETWORK_SCAN, dispatchNetworkScan, responseVoid },
    {RIL_REQUEST_STOP_NETWORK_SCAN, dispatchVoid, responseVoid },
    {RIL_REQUEST_START_KEEPALIVE, dispatchVoid, responseVoid },
    {RIL_REQUEST_STOP_KEEPALIVE, dispatchVoid, responseVoid },
    {RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION, dispatchVoid, responseVoid },
    {RIL_REQUEST_SET_PREFERRED_DATA_MODEM, dispatchInts, responseVoid},

    // MTK request
    {RIL_REQUEST_RESUME_REGISTRATION, dispatchInts, responseVoid },  // [C2K][IR] Support SVLTE IR feature

    /// M: eMBMS feature
    {RIL_REQUEST_EMBMS_AT_CMD, dispatchString, responseString },
    /// M: eMBMS end
    // MTK-START: SIM
    {RIL_REQUEST_SIM_GET_ATR, dispatchVoid, responseString },
    {RIL_REQUEST_SIM_GET_ICCID, dispatchVoid, responseString },
    {RIL_REQUEST_SET_SIM_POWER, dispatchInts, responseVoid },
    // MTK-END
    // MTK-START: SIM GBA
    //{RIL_REQUEST_GENERAL_SIM_AUTH, dispatchSimAuth, responseSIM_IO },
    // MTK-END

    // modem power
    {RIL_REQUEST_MODEM_POWERON, dispatchVoid, responseVoid },
    {RIL_REQUEST_MODEM_POWEROFF, dispatchVoid, responseVoid },

    // MTK-START: NW
    {RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT, dispatchStrings, responseVoid },
    {RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT, dispatchVoid, responseStrings },
    {RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS, dispatchVoid, responseVoid },//for PLMN List abort
    {RIL_REQUEST_SET_LTE_RELEASE_VERSION, dispatchInts, responseVoid },
    {RIL_REQUEST_GET_LTE_RELEASE_VERSION, dispatchVoid, responseInts },
    {RIL_REQUEST_SIGNAL_STRENGTH_WITH_WCDMA_ECIO, dispatchVoid, responseInts },
    // ATCI
    {RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL, dispatchRaw, responseRaw },
    // M: To set language configuration for GSM cell broadcast
    {RIL_REQUEST_GSM_SET_BROADCAST_LANGUAGE, dispatchString, responseVoid },
    // M: To get language configuration for GSM cell broadcast
    {RIL_REQUEST_GSM_GET_BROADCAST_LANGUAGE, dispatchVoid, responseString },
    {RIL_REQUEST_GET_SMS_SIM_MEM_STATUS, dispatchVoid, responseGetSmsSimMemStatusCnf },
    //{RIL_REQUEST_GET_SMS_PARAMS, dispatchVoid, responseSmsParams },
    //{RIL_REQUEST_SET_SMS_PARAMS, dispatchSmsParams, responseVoid },
    {RIL_REQUEST_SET_ETWS, dispatchInts, responseVoid },
    {RIL_REQUEST_REMOVE_CB_MESSAGE, dispatchInts, responseVoid },
    /// M: CC: Proprietary incoming call indication
    {RIL_REQUEST_SET_CALL_INDICATION, dispatchInts, responseVoid },
    /// M: CC: Proprietary ECC handling @{
    {RIL_REQUEST_SET_ECC_LIST, dispatchStrings, responseVoid },
    /// @}
    /// M: CC: Proprietary call control hangup all
    {RIL_REQUEST_HANGUP_ALL, dispatchVoid, responseVoid },
    /// M: CC: Emergency mode for Fusion RIL
    {RIL_REQUEST_SET_ECC_MODE, dispatchStrings, responseVoid },
    /// M: CC: Vzw/CTVolte ECC for Fusion RIL
    {RIL_REQUEST_ECC_PREFERRED_RAT, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_PS_REGISTRATION, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_PSEUDO_CELL_MODE, dispatchInts, responseVoid },
    {RIL_REQUEST_GET_PSEUDO_CELL_INFO, dispatchVoid, responseInts },
    {RIL_REQUEST_SWITCH_MODE_FOR_ECC, dispatchInts, responseVoid },
    {RIL_REQUEST_GET_SMS_RUIM_MEM_STATUS, dispatchVoid, responseGetSmsSimMemStatusCnf },
    // FastDormancy
    //{RIL_REQUEST_SET_FD_MODE, dispatchFdMode, responseVoid },
    {RIL_REQUEST_MODIFY_MODEM_TYPE, dispatchInts, responseInts },
    {RIL_REQUEST_SET_TRM, dispatchInts, responseVoid },
    //STK
    {RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE, dispatchInts, responseVoid },
    //SS
    {RIL_REQUEST_SET_CLIP, dispatchInts, responseVoid },
    {RIL_REQUEST_GET_COLP, dispatchVoid, responseInts },
    {RIL_REQUEST_GET_COLR, dispatchVoid, responseInts },
    {RIL_REQUEST_SEND_CNAP, dispatchString, responseInts },

    {RIL_REQUEST_SET_COLP, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_COLR, dispatchInts, responseVoid },
    {RIL_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT, dispatchCallForwardEx, responseCallForwardsEx },
    {RIL_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT, dispatchCallForwardEx, responseVoid },
    {RIL_REQUEST_RUN_GBA, dispatchStrings, responseStrings },

    // PHB START
    {RIL_REQUEST_QUERY_PHB_STORAGE_INFO, dispatchInts, responseInts },
    {RIL_REQUEST_WRITE_PHB_ENTRY, dispatchPhbEntry, responseVoid },
    {RIL_REQUEST_READ_PHB_ENTRY, dispatchInts, responsePhbEntries },
    {RIL_REQUEST_QUERY_UPB_CAPABILITY, dispatchVoid, responseInts },
    {RIL_REQUEST_EDIT_UPB_ENTRY, dispatchStrings, responseVoid },
    {RIL_REQUEST_DELETE_UPB_ENTRY, dispatchInts, responseVoid },
    {RIL_REQUEST_READ_UPB_GAS_LIST, dispatchInts, responseStrings },
    {RIL_REQUEST_READ_UPB_GRP, dispatchInts, responseInts },
    {RIL_REQUEST_WRITE_UPB_GRP, dispatchInts, responseVoid },
    {RIL_REQUEST_GET_PHB_STRING_LENGTH, dispatchVoid, responseInts },
    //{RIL_REQUEST_GET_PHB_MEM_STORAGE, dispatchVoid, responseGetPhbMemStorage },
    {RIL_REQUEST_SET_PHB_MEM_STORAGE, dispatchStrings, responseVoid },
    //{RIL_REQUEST_READ_PHB_ENTRY_EXT, dispatchInts, responseReadPhbEntryExt },
    //{RIL_REQUEST_WRITE_PHB_ENTRY_EXT, dispatchWritePhbEntryExt, responseVoid },
    {RIL_REQUEST_QUERY_UPB_AVAILABLE, dispatchInts, responseInts },
    {RIL_REQUEST_READ_EMAIL_ENTRY, dispatchInts, responseString },
    {RIL_REQUEST_READ_SNE_ENTRY, dispatchInts, responseString },
    //{RIL_REQUEST_READ_ANR_ENTRY, dispatchInts, responsePhbEntries },
    {RIL_REQUEST_READ_UPB_AAS_LIST, dispatchInts, responseStrings },
    // PHB END
    //Femtocell (CSG) feature
    {RIL_REQUEST_GET_FEMTOCELL_LIST, dispatchVoid, responseStrings },
    // Femtocell (CSG) : abort command shall be sent in differenent channel
    {RIL_REQUEST_ABORT_FEMTOCELL_LIST, dispatchVoid, responseVoid },
    {RIL_REQUEST_SELECT_FEMTOCELL, dispatchStrings, responseVoid },
    {RIL_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE, dispatchVoid, responseInts },
    {RIL_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE, dispatchInts, responseVoid },
     // Data
    {RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD, dispatchInts, responseVoid },
    {RIL_REQUEST_RESET_MD_DATA_RETRY_COUNT, dispatchStrings, responseVoid },
    // M: Data Framework - CC 33
    {RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE, dispatchInts, responseVoid },
    // M: [LTE][Low Power][UL traffic shaping] @{
    {RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT, dispatchInts, responseVoid},
    {RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER, dispatchInts, responseVoid},
    // M: [LTE][Low Power][UL traffic shaping] @}
    // MTK-START: SIM ME LOCK
    {RIL_REQUEST_QUERY_SIM_NETWORK_LOCK, dispatchInts, responseInts },
    {RIL_REQUEST_SET_SIM_NETWORK_LOCK, dispatchStrings, responseVoid },
    {RIL_REQUEST_ENTER_DEPERSONALIZATION, dispatchStrings, responseInts },
    // MTK-END
    {RIL_REQUEST_SET_IMS_ENABLE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_VOLTE_ENABLE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_WFC_ENABLE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_VILTE_ENABLE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_VIWIFI_ENABLE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_IMS_VIDEO_ENABLE, dispatchInts, responseVoid },
    {RIL_REQUEST_VIDEO_CALL_ACCEPT, dispatchInts, responseVoid },
    {RIL_REQUEST_ECC_REDIAL_APPROVE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_IMSCFG, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_MD_IMSCFG, dispatchStrings, responseString },
    {RIL_REQUEST_GET_PROVISION_VALUE, dispatchString, responseStrings },
    {RIL_REQUEST_SET_PROVISION_VALUE, dispatchStrings, responseVoid },
    {RIL_REQUEST_IMS_BEARER_STATE_CONFIRM, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_IMS_BEARER_NOTIFICATION, dispatchInts, responseVoid },
    {RIL_REQUEST_IMS_DEREG_NOTIFICATION, dispatchInts, responseVoid },
    {RIL_REQUEST_IMS_ECT, dispatchStrings, responseVoid },
    {RIL_REQUEST_HOLD_CALL, dispatchInts, responseVoid },
    {RIL_REQUEST_RESUME_CALL, dispatchInts, responseVoid },
    {RIL_REQUEST_DIAL_WITH_SIP_URI, dispatchString, responseVoid },
    {RIL_REQUEST_FORCE_RELEASE_CALL, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_IMS_RTP_REPORT, dispatchInts, responseVoid },
    {RIL_REQUEST_CONFERENCE_DIAL, dispatchStrings, responseVoid },
    {RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER, dispatchStrings, responseVoid },
    {RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER, dispatchStrings, responseVoid },
    {RIL_REQUEST_VT_DIAL_WITH_SIP_URI, dispatchString, responseVoid },
    {RIL_REQUEST_SEND_USSI, dispatchString, responseVoid },
    {RIL_REQUEST_CANCEL_USSI, dispatchVoid, responseVoid },
    {RIL_REQUEST_SET_WFC_PROFILE, dispatchInts, responseVoid },
    {RIL_REQUEST_PULL_CALL, dispatchStrings, responseVoid },
    {RIL_REQUEST_SET_IMS_REGISTRATION_REPORT, dispatchVoid, responseVoid },
    {RIL_REQUEST_IMS_DIAL, dispatchDial, responseVoid },
    {RIL_REQUEST_IMS_VT_DIAL, dispatchDial, responseVoid },
    {RIL_REQUEST_IMS_EMERGENCY_DIAL, dispatchDial, responseVoid },
    {RIL_REQUEST_IMS_GET_CURRENT_CALLS, dispatchVoid, responseCallList },
    {RIL_REQUEST_IMS_HANGUP_WAITING_OR_BACKGROUND, dispatchVoid, responseVoid },
    {RIL_REQUEST_IMS_HANGUP_FOREGROUND_RESUME_BACKGROUND, dispatchVoid, responseVoid },
    {RIL_REQUEST_IMS_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, dispatchVoid, responseVoid },
    // MTK_TC1_FEATURE for Antenna Testing start
    {RIL_REQUEST_VSS_ANTENNA_CONF, dispatchInts, responseInts }, // Antenna Testing
    {RIL_REQUEST_VSS_ANTENNA_INFO, dispatchInts, responseInts }, // Antenna Testing
    // MTK_TC1_FEATURE for Antenna Testing end
    // Preferred Operator List
    {RIL_REQUEST_GET_POL_CAPABILITY, dispatchVoid, responseInts },
    {RIL_REQUEST_GET_POL_LIST, dispatchVoid, responseStrings },
    {RIL_REQUEST_SET_POL_ENTRY, dispatchStrings, responseVoid },
    /// M: [Network][C2K] Sprint roaming control @{
    {RIL_REQUEST_SET_ROAMING_ENABLE, dispatchInts, responseVoid },
    {RIL_REQUEST_GET_ROAMING_ENABLE, dispatchInts, responseInts },
    /// @}
    // External SIM [START]
    //{RIL_REQUEST_VSIM_NOTIFICATION, dispatchVsimEvent, responseInts },
    //{RIL_REQUEST_VSIM_OPERATION, dispatchVsimOperationEvent, responseInts },
    // External SIM [END]
    {RIL_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION, dispatchVoid, responseInts },
    {RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_E911_STATE, dispatchInts, responseVoid },
    /// M: MwiService @{
    {RIL_REQUEST_SET_WIFI_ENABLED, dispatchStrings, responseVoid },
    {RIL_REQUEST_SET_WIFI_ASSOCIATED, dispatchStrings, responseVoid },
    {RIL_REQUEST_SET_WFC_CONFIG, dispatchStrings, responseVoid },
    {RIL_REQUEST_SET_WIFI_SIGNAL_LEVEL, dispatchStrings, responseVoid },
    {RIL_REQUEST_SET_WIFI_IP_ADDRESS, dispatchStrings, responseVoid },
    {RIL_REQUEST_SET_GEO_LOCATION, dispatchStrings, responseVoid },
    {RIL_REQUEST_SET_EMERGENCY_ADDRESS_ID, dispatchStrings, responseVoid },
    {RIL_REQUEST_SET_NATT_KEEP_ALIVE_STATUS, dispatchStrings, responseVoid },
    {RIL_REQUEST_NOTIFY_EPDG_SCREEN_STATE, dispatchStrings, responseVoid },
    {RIL_REQUEST_QUERY_SSAC_STATUS, dispatchVoid, responseInts},
    ///@}
    /// M: Network @{
    {RIL_REQUEST_SET_SERVICE_STATE, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_VOICE_PREFER_STATUS, dispatchInts, responseVoid },
    {RIL_REQUEST_SET_ECC_NUM, dispatchStrings, responseVoid },
    {RIL_REQUEST_GET_ECC_NUM, dispatchVoid, responseVoid },
    {RIL_REQUEST_IMS_CONFIG_SET_FEATURE, dispatchInts, responseVoid},
    /// OEM data ril request. @{
    {RIL_REQUEST_DATA_CONNECTION_ATTACH, dispatchInts, responseVoid },
    {RIL_REQUEST_DATA_CONNECTION_DETACH, dispatchInts, responseVoid },
    {RIL_REQUEST_RESET_ALL_CONNECTIONS, dispatchVoid, responseVoid },
    /// @}
    {RIL_REQUEST_SET_TX_POWER_STATUS, dispatchInts, responseVoid},
    {RIL_REQUEST_SET_SYSTEM_SELECTION_CHANNELS, dispatchSetSystemSelectionChannels, responseVoid},
    {RIL_REQUEST_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA, dispatchSetSignalStrengthReportingCriteria, responseVoid},
    {RIL_REQUEST_ENABLE_MODEM, dispatchInts, responseVoid},
#endif /* __RIL_CLIENT_COMMAND_H__ */

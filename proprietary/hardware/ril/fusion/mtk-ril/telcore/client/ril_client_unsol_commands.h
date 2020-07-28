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

#ifndef __RIL_CLIENT_UNSOL_COMMAND_H__
#define __RIL_CLIENT_UNSOL_COMMAND_H__

// AOSP URC
    {RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, responseVoid},
    {RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, responseVoid},
    {RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, responseStrings},
    {RIL_UNSOL_RESPONSE_NEW_SMS, responseString},
    {RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT, responseString},
    {RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM, responseInts},
    {RIL_UNSOL_ON_USSD, responseStrings},
    {RIL_UNSOL_ON_USSD_REQUEST, responseVoid},
    {RIL_UNSOL_NITZ_TIME_RECEIVED, responseString},
    {RIL_UNSOL_SIGNAL_STRENGTH, responseRilSignalStrength},
    {RIL_UNSOL_DATA_CALL_LIST_CHANGED, responseDataCallList},
    {RIL_UNSOL_SUPP_SVC_NOTIFICATION, responseSsn},
    {RIL_UNSOL_STK_SESSION_END, responseVoid},
    {RIL_UNSOL_STK_PROACTIVE_COMMAND, responseString},
    {RIL_UNSOL_STK_EVENT_NOTIFY, responseString},
    {RIL_UNSOL_STK_CALL_SETUP, responseInts},
    {RIL_UNSOL_SIM_SMS_STORAGE_FULL, responseVoid},
    {RIL_UNSOL_SIM_REFRESH, responseSimRefresh},
    {RIL_UNSOL_CALL_RING, responseCallRing},
    {RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, responseVoid},
    {RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, responseCdmaSms},
    {RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS, responseRaw},
    {RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL, responseVoid},
    {RIL_UNSOL_RESTRICTED_STATE_CHANGED, responseInts},
    {RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE, responseVoid},
    {RIL_UNSOL_CDMA_CALL_WAITING, responseCdmaCallWaiting},
    {RIL_UNSOL_CDMA_OTA_PROVISION_STATUS, responseInts},
    {RIL_UNSOL_CDMA_INFO_REC, responseCdmaInformationRecords},
    {RIL_UNSOL_OEM_HOOK_RAW, responseRaw},
    {RIL_UNSOL_RINGBACK_TONE, responseInts},
    {RIL_UNSOL_RESEND_INCALL_MUTE, responseVoid},
    {RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED, responseInts},
    {RIL_UNSOL_CDMA_PRL_CHANGED, responseInts},
    {RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE, responseVoid},
    {RIL_UNSOL_RIL_CONNECTED, responseInts},
    {RIL_UNSOL_VOICE_RADIO_TECH_CHANGED, responseInts},
    {RIL_UNSOL_CELL_INFO_LIST, responseCellInfoList},
    {RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED, responseVoid},
    {RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED, responseInts},
    {RIL_UNSOL_SRVCC_STATE_NOTIFY, responseInts},
    {RIL_UNSOL_HARDWARE_CONFIG_CHANGED, responseHardwareConfig},
    {RIL_UNSOL_DC_RT_INFO_CHANGED, responseDcRtInfo},
    {RIL_UNSOL_RADIO_CAPABILITY, responseRadioCapability},
    {RIL_UNSOL_ON_SS, responseSSData},
    {RIL_UNSOL_STK_CC_ALPHA_NOTIFY, responseStrings},
    {RIL_UNSOL_LCEDATA_RECV, responseLceData},
    {RIL_UNSOL_PCO_DATA, responsePcoData},
    {RIL_UNSOL_MODEM_RESTART, responseString},
    //{RIL_UNSOL_NETWORK_SCAN_RESULT, responseNetworkScanResult},

// MTK URC
    {RIL_UNSOL_RESPONSE_PLMN_CHANGED, responseStrings},
    {RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED, responseInts},
/// M: [C2K 6M][NW] add for Iwlan @{
    {RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED, responseInts},
/// M: [C2K 6M][NW] add for Iwlan @{
    {RIL_UNSOL_GMSS_RAT_CHANGED, responseInts},
    {RIL_UNSOL_CDMA_PLMN_CHANGED, responseStrings},
    {RIL_UNSOL_RESPONSE_CS_NETWORK_STATE_CHANGED, responseStrings},
    {RIL_UNSOL_INVALID_SIM, responseStrings},
    {RIL_UNSOL_NETWORK_EVENT, responseInts},
    {RIL_UNSOL_MODULATION_INFO, responseInts},
// MTK-START: SIM
    {RIL_UNSOL_VIRTUAL_SIM_ON,responseInts},
    {RIL_UNSOL_VIRTUAL_SIM_OFF,responseInts},
    {RIL_UNSOL_IMEI_LOCK, responseVoid},
    {RIL_UNSOL_IMSI_REFRESH_DONE, responseVoid},
// MTK-END
/// M: eMBMS feature
// only used in ril-proxy, and which use rilproxy\libril\ril.cpp use the same name file under libril
    {RIL_UNSOL_EMBMS_SESSION_STATUS, responseInts},
    {RIL_UNSOL_EMBMS_AT_INFO, responseString},
/// M: eMBMS end

// ATCI
    {RIL_UNSOL_ATCI_RESPONSE, responseRaw},
    {RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION, responseEtwsNotification},
    {RIL_UNSOL_ME_SMS_STORAGE_FULL, responseVoid},
    {RIL_UNSOL_SMS_READY_NOTIFICATION, responseVoid},
    {RIL_UNSOL_DATA_ALLOWED, responseInts},
/// M: CC: Proprietary incoming call indication
    {RIL_UNSOL_INCOMING_CALL_INDICATION, responseStrings},
    {RIL_UNSOL_CALL_ADDITIONAL_INFO, responseStrings},
    {RIL_UNSOL_PSEUDO_CELL_INFO, responseInts},
//Reset Attach APN
    {RIL_UNSOL_RESET_ATTACH_APN, responseVoid},
// M: IA-change attach APN
    {RIL_UNSOL_DATA_ATTACH_APN_CHANGED, responseInts},
    {RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION, responseEtwsNotification},
/// M: CC: Cipher indication support
    {RIL_UNSOL_CIPHER_INDICATION, responseStrings},
/// M: CC: Call control CRSS handling
    {RIL_UNSOL_CRSS_NOTIFICATION, responseCrssN},
    /// M: CC: GSA HD Voice for 2/3G network support
    {RIL_UNSOL_SPEECH_CODEC_INFO, responseInts},
// World Phone
    {RIL_UNSOL_WORLD_MODE_CHANGED, responseInts},
    {RIL_UNSOL_CDMA_CARD_INITIAL_ESN_OR_MEID, responseString},
    {RIL_UNSOL_FEMTOCELL_INFO, responseStrings},
// / M: BIP {
    {RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND, responseString},
// / M: BIP }
// SS
    {RIL_UNSOL_CALL_FORWARDING, responseInts},
// PHB
    {RIL_UNSOL_PHB_READY_NOTIFICATION, responseInts},
// / M: OTASP {
    {RIL_UNSOL_TRIGGER_OTASP, responseVoid},
// / M: OTASP }
    {RIL_UNSOL_MD_DATA_RETRY_COUNT_RESET, responseVoid},
    {RIL_UNSOL_REMOVE_RESTRICT_EUTRAN, responseVoid},
    {RIL_UNSOL_PCO_STATUS, responseInts},
// M: [LTE][Low Power][UL traffic shaping] @{
    {RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE, responseInts},
// M: [LTE][Low Power][UL traffic shaping] @}
// MTK-START: SIM HOT SWAP
    {RIL_UNSOL_SIM_PLUG_IN, responseVoid},
    {RIL_UNSOL_SIM_PLUG_OUT, responseVoid},
    {RIL_UNSOL_SIM_MISSING, responseInts},
    {RIL_UNSOL_SIM_RECOVERY, responseInts},
// MTK-END
// MTK-START: SIM COMMON SLOT
    {RIL_UNSOL_TRAY_PLUG_IN, responseVoid},
    {RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED, responseVoid},
// MTK-END
    /// [IMS] IMS Indication ================================================================
    {RIL_UNSOL_CALL_INFO_INDICATION, responseStrings},
    {RIL_UNSOL_ECONF_RESULT_INDICATION, responseStrings},
    {RIL_UNSOL_SIP_CALL_PROGRESS_INDICATOR, responseStrings},
    {RIL_UNSOL_CALLMOD_CHANGE_INDICATOR, responseStrings},
    {RIL_UNSOL_VIDEO_CAPABILITY_INDICATOR, responseStrings},
    {RIL_UNSOL_ON_USSI, responseStrings},
    {RIL_UNSOL_GET_PROVISION_DONE, responseStrings},
    {RIL_UNSOL_IMS_RTP_INFO, responseStrings},
    {RIL_UNSOL_ON_XUI, responseStrings},
    {RIL_UNSOL_IMS_EVENT_PACKAGE_INDICATION, responseStrings},
    {RIL_UNSOL_ECONF_SRVCC_INDICATION, responseInts},
    {RIL_UNSOL_IMS_REGISTRATION_INFO, responseInts},
    {RIL_UNSOL_IMS_ENABLE_DONE, responseVoid},
    {RIL_UNSOL_IMS_DISABLE_DONE, responseVoid},
    {RIL_UNSOL_IMS_ENABLE_START, responseVoid},
    {RIL_UNSOL_IMS_DISABLE_START, responseVoid},
    {RIL_UNSOL_ECT_INDICATION, responseInts},
    {RIL_UNSOL_VOLTE_SETTING, responseInts},
    {RIL_UNSOL_IMS_BEARER_STATE_NOTIFY, responseInts},
    {RIL_UNSOL_IMS_BEARER_INIT, responseInts},
    {RIL_UNSOL_IMS_DEREG_DONE, responseVoid},
    {RIL_UNSOL_ON_VOLTE_SUBSCRIPTION, responseInts},
    {RIL_UNSOL_IMS_RESPONSE_CALL_STATE_CHANGED, responseVoid},
    /// [IMS] IMS Indication ================================================================
    // M: [VzW] Data Framework
    //{RIL_UNSOL_PCO_DATA_AFTER_ATTACHED, responsePcoDataAfterAttached},
    {RIL_UNSOL_NETWORK_INFO, responseStrings},
// MTK-START: SIM TMO RSU
    {RIL_UNSOL_MELOCK_NOTIFICATION,responseInts},
// MTK-END
/// M: CC: CDMA call accepted indication
    {RIL_UNSOL_CDMA_CALL_ACCEPTED, responseVoid},
// /M: STK{
    {RIL_UNSOL_STK_SETUP_MENU_RESET, responseVoid},
// /M: STK}

    // External SIM [Start]
    //{RIL_UNSOL_VSIM_OPERATION_INDICATION, responseVsimOperationEvent},
    // External SIM [End]

// M: [VzW] Data Framework
    {RIL_UNSOL_VOLTE_LTE_CONNECTION_STATUS, responseInts},
    {RIL_UNSOL_IMS_MULTIIMS_COUNT, responseInts},

///M: MwiService @{
    {RIL_UNSOL_MOBILE_WIFI_ROVEOUT, responseStrings},
    {RIL_UNSOL_MOBILE_WIFI_HANDOVER, responseInts},
    {RIL_UNSOL_ACTIVE_WIFI_PDN_COUNT, responseInts},
    {RIL_UNSOL_WIFI_RSSI_MONITORING_CONFIG, responseInts},
    {RIL_UNSOL_WIFI_PDN_ERROR, responseInts},
    {RIL_UNSOL_REQUEST_GEO_LOCATION, responseStrings},
    {RIL_UNSOL_WFC_PDN_STATE, responseInts},
    {RIL_UNSOL_NATT_KEEP_ALIVE_CHANGED, responseStrings},
    {RIL_UNSOL_SSAC_STATUS, responseInts},
///@}
    {RIL_UNSOL_IMS_SUPPORT_ECC, responseInts},
    {RIL_UNSOL_TX_POWER, responseInts},

    {RIL_UNSOL_ECC_NUM, responseString},
    {RIL_UNSOL_EMERGENCY_NUMBER_LIST, responseEmergencyList},
    {RIL_UNSOL_SUPP_SVC_NOTIFICATION_EX, responseSsn},

    {RIL_UNSOL_MCCMNC_CHANGED, responseString},
    {RIL_UNSOL_SIGNAL_STRENGTH_WITH_WCDMA_ECIO, responseInts},
    {RIL_UNSOL_TX_POWER_STATUS, responseInts},

#endif /* __RIL_CLIENT_UNSOL_COMMAND_H__ */

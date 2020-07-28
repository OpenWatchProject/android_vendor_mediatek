/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#ifdef MTK_USE_HIDL
    {RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED, mtkRadioEx::responsePsNetworkStateChangeInd, WAKE_PARTIAL},
/// M: eMBMS feature
    {RIL_UNSOL_EMBMS_SESSION_STATUS, mtkRadioEx::embmsSessionStatusInd, WAKE_PARTIAL},
    {RIL_UNSOL_EMBMS_AT_INFO, mtkRadioEx::embmsAtInfoInd, WAKE_PARTIAL},
/// M: eMBMS end
    {RIL_UNSOL_RESPONSE_CS_NETWORK_STATE_CHANGED, mtkRadioEx::responseCsNetworkStateChangeInd, WAKE_PARTIAL},
    {RIL_UNSOL_INVALID_SIM, mtkRadioEx::responseInvalidSimInd, WAKE_PARTIAL},
    {RIL_UNSOL_NETWORK_EVENT, mtkRadioEx::responseNetworkEventInd, WAKE_PARTIAL},
// MTK-START: SIM
    {RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED,mtkRadioEx::onVirtualSimStatusChanged, WAKE_PARTIAL},
    {RIL_UNSOL_IMEI_LOCK,mtkRadioEx::onImeiLock, WAKE_PARTIAL},
    {RIL_UNSOL_IMSI_REFRESH_DONE, mtkRadioEx::onImsiRefreshDone, WAKE_PARTIAL},
// MTK-END
// ATCI
    {RIL_UNSOL_ATCI_RESPONSE, mtkRadioEx::atciInd, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION, mtkRadioEx::newEtwsInd, WAKE_PARTIAL},
    {RIL_UNSOL_ME_SMS_STORAGE_FULL, mtkRadioEx::meSmsStorageFullInd, WAKE_PARTIAL},
    {RIL_UNSOL_SMS_READY_NOTIFICATION, mtkRadioEx::smsReadyInd, WAKE_PARTIAL},
    {RIL_UNSOL_DATA_ALLOWED, mtkRadioEx::dataAllowedNotificationInd, WAKE_PARTIAL},
    /// M: CC: Proprietary incoming call indication ([IMS] common flow)
    {RIL_UNSOL_INCOMING_CALL_INDICATION, mtkRadioEx::incomingCallIndicationInd, WAKE_PARTIAL},
    /// M: CC: Proprietary incoming call indication ([IMS] common flow)
    {RIL_UNSOL_CALL_ADDITIONAL_INFO, mtkRadioEx::callAdditionalInfoInd, WAKE_PARTIAL},
    /// M: CC: Cipher indication support ([IMS] common flow)
    {RIL_UNSOL_CIPHER_INDICATION, mtkRadioEx::cipherIndicationInd, WAKE_PARTIAL},
    /// M: CC: CRSS notification indication
    {RIL_UNSOL_CRSS_NOTIFICATION, mtkRadioEx::crssNotifyInd, WAKE_PARTIAL},
    /// M: CC: GSA HD Voice for 2/3G network support
    {RIL_UNSOL_SPEECH_CODEC_INFO, mtkRadioEx::speechCodecInfoInd, WAKE_PARTIAL},
    {RIL_UNSOL_PSEUDO_CELL_INFO, mtkRadioEx::onPseudoCellInfoInd, WAKE_PARTIAL},
    {RIL_UNSOL_RESET_ATTACH_APN, mtkRadioEx::resetAttachApnInd, WAKE_PARTIAL},
    {RIL_UNSOL_DATA_ATTACH_APN_CHANGED, mtkRadioEx::mdChangeApnInd, WAKE_PARTIAL},
// World Phone
    {RIL_UNSOL_WORLD_MODE_CHANGED, mtkRadioEx::worldModeChangedIndication, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_PLMN_CHANGED, mtkRadioEx::plmnChangedIndication, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED, mtkRadioEx::registrationSuspendedIndication, WAKE_PARTIAL},
    {RIL_UNSOL_GMSS_RAT_CHANGED, mtkRadioEx::gmssRatChangedIndication, WAKE_PARTIAL},
    {RIL_UNSOL_CDMA_CARD_INITIAL_ESN_OR_MEID, mtkRadioEx::esnMeidChangeInd, WAKE_PARTIAL},
    {RIL_UNSOL_FEMTOCELL_INFO, mtkRadioEx::responseFemtocellInfo, WAKE_PARTIAL},
    {RIL_UNSOL_SIGNAL_STRENGTH_WITH_WCDMA_ECIO, mtkRadioEx::currentSignalStrengthWithWcdmaEcioInd, DONT_WAKE},
// / M: BIP {
    {RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND, mtkRadioEx::bipProactiveCommandInd, WAKE_PARTIAL},
// / M: BIP }
// SS
    {RIL_UNSOL_CALL_FORWARDING, mtkRadioEx::cfuStatusNotifyInd, WAKE_PARTIAL},
// PHB
    {RIL_UNSOL_PHB_READY_NOTIFICATION, mtkRadioEx::phbReadyNotificationInd, WAKE_PARTIAL},
// / M: OTASP {
    {RIL_UNSOL_TRIGGER_OTASP, mtkRadioEx::triggerOtaSPInd, WAKE_PARTIAL},
// / M: OTASP }
    {RIL_UNSOL_MD_DATA_RETRY_COUNT_RESET, mtkRadioEx::onMdDataRetryCountReset, WAKE_PARTIAL},
    {RIL_UNSOL_REMOVE_RESTRICT_EUTRAN, mtkRadioEx::onRemoveRestrictEutran, WAKE_PARTIAL},
// M: [LTE][Low Power][UL traffic shaping] @{
    {RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE, mtkRadioEx::onLteAccessStratumStateChanged, WAKE_PARTIAL},
// M: [LTE][Low Power][UL traffic shaping] @}
// MTK-START: SIM HOT SWAP
    {RIL_UNSOL_SIM_PLUG_IN, mtkRadioEx::onSimPlugIn, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_PLUG_OUT, mtkRadioEx::onSimPlugOut, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_MISSING, mtkRadioEx::onSimMissing, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_RECOVERY, mtkRadioEx::onSimRecovery, WAKE_PARTIAL},
// MTK-END
// MTK-START: SIM COMMON SLOT
    {RIL_UNSOL_TRAY_PLUG_IN, mtkRadioEx::onSimTrayPlugIn, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED, mtkRadioEx::onSimCommonSlotNoChanged, WAKE_PARTIAL},
// MTK-END
    {RIL_UNSOL_CALL_INFO_INDICATION, mtkRadioEx::callInfoIndicationInd, WAKE_PARTIAL},
    {RIL_UNSOL_ECONF_RESULT_INDICATION, mtkRadioEx::econfResultIndicationInd, WAKE_PARTIAL},
    {RIL_UNSOL_SIP_CALL_PROGRESS_INDICATOR, mtkRadioEx::sipCallProgressIndicatorInd, WAKE_PARTIAL},
    {RIL_UNSOL_CALLMOD_CHANGE_INDICATOR, mtkRadioEx::callmodChangeIndicatorInd, WAKE_PARTIAL},
    {RIL_UNSOL_VIDEO_CAPABILITY_INDICATOR, mtkRadioEx::videoCapabilityIndicatorInd, WAKE_PARTIAL},
    {RIL_UNSOL_ON_USSI, mtkRadioEx::onUssiInd, WAKE_PARTIAL},
    {RIL_UNSOL_ON_VOLTE_SUBSCRIPTION, mtkRadioEx::onVolteSubscription, WAKE_PARTIAL},
    {RIL_UNSOL_GET_PROVISION_DONE, mtkRadioEx::getProvisionDoneInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_RTP_INFO, mtkRadioEx::imsRtpInfoInd, WAKE_PARTIAL},
    {RIL_UNSOL_ON_XUI, mtkRadioEx::onXuiInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_EVENT_PACKAGE_INDICATION, mtkRadioEx::imsEventPackageIndicationInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_REGISTRATION_INFO, mtkRadioEx::imsRegistrationInfoInd, WAKE_PARTIAL},
    {RIL_UNSOL_ECONF_SRVCC_INDICATION, mtkRadioEx::confSRVCCInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_ENABLE_DONE, mtkRadioEx::imsEnableDoneInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_DISABLE_DONE, mtkRadioEx::imsDisableDoneInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_ENABLE_START, mtkRadioEx::imsEnableStartInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_DISABLE_START, mtkRadioEx::imsDisableStartInd, WAKE_PARTIAL},
    {RIL_UNSOL_ECT_INDICATION, mtkRadioEx::ectIndicationInd, WAKE_PARTIAL},
    {RIL_UNSOL_VOLTE_SETTING, mtkRadioEx::volteSettingInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_BEARER_STATE_NOTIFY, mtkRadioEx::imsBearerStateNotifyInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_BEARER_INIT, mtkRadioEx::imsBearerInitInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_DEREG_DONE, mtkRadioEx::imsDeregDoneInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_CONFIG_DYNAMIC_IMS_SWITCH_COMPLETE, mtkRadioEx::imsCfgDynamicImsSwitchCompleteInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_CONFIG_FEATURE_CHANGED, mtkRadioEx::imsCfgFeatureChangedInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_CONFIG_CONFIG_CHANGED, mtkRadioEx::imsCfgConfigChangedInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_CONFIG_CONFIG_LOADED, mtkRadioEx::imsCfgConfigLoadedInd, WAKE_PARTIAL},

    // M: [VzW] Data Framework
    {RIL_UNSOL_PCO_DATA_AFTER_ATTACHED, mtkRadioEx::pcoDataAfterAttachedInd, WAKE_PARTIAL},
    {RIL_UNSOL_NETWORK_INFO, mtkRadioEx::networkInfoInd, WAKE_PARTIAL},
/// M: CC: CDMA call accepted indication
    {RIL_UNSOL_CDMA_CALL_ACCEPTED, mtkRadioEx::cdmaCallAcceptedInd, WAKE_PARTIAL},
// / M:STK {
    {RIL_UNSOL_STK_SETUP_MENU_RESET, mtkRadioEx::onStkMenuResetInd,  WAKE_PARTIAL},
// / M:STK }

    // External SIM [Start]
    {RIL_UNSOL_VSIM_OPERATION_INDICATION, mtkRadioEx::onVsimEventIndication, WAKE_PARTIAL},
    // External SIM [End]

/// Ims Data Framework
    {RIL_UNSOL_DEDICATE_BEARER_ACTIVATED, mtkRadioEx::dedicatedBearerActivationInd, WAKE_PARTIAL},
    {RIL_UNSOL_DEDICATE_BEARER_MODIFIED, mtkRadioEx::dedicatedBearerModificationInd, WAKE_PARTIAL},
    {RIL_UNSOL_DEDICATE_BEARER_DEACTIVATED, mtkRadioEx::dedicatedBearerDeactivationInd, WAKE_PARTIAL},
/// @}
    {RIL_UNSOL_IMS_MULTIIMS_COUNT, mtkRadioEx::multiImsCountInd, WAKE_PARTIAL},
///M: MwiService @{
    {RIL_UNSOL_MOBILE_WIFI_ROVEOUT, mtkRadioEx::onWifiRoveout, WAKE_PARTIAL},
    {RIL_UNSOL_MOBILE_WIFI_HANDOVER, mtkRadioEx::onPdnHandover, WAKE_PARTIAL},
    {RIL_UNSOL_ACTIVE_WIFI_PDN_COUNT, mtkRadioEx::onWifiPdnActivate, WAKE_PARTIAL},
    {RIL_UNSOL_WIFI_RSSI_MONITORING_CONFIG, mtkRadioEx::onWifiMonitoringThreshouldChanged, WAKE_PARTIAL},
    {RIL_UNSOL_WIFI_PDN_ERROR, mtkRadioEx::onWfcPdnError, WAKE_PARTIAL},
    {RIL_UNSOL_REQUEST_GEO_LOCATION, mtkRadioEx::onLocationRequest, WAKE_PARTIAL},
    {RIL_UNSOL_WFC_PDN_STATE, mtkRadioEx::onWfcPdnStateChanged, WAKE_PARTIAL},
    {RIL_UNSOL_NATT_KEEP_ALIVE_CHANGED, mtkRadioEx::onNattKeepAliveChanged, WAKE_PARTIAL},
    {RIL_UNSOL_WIFI_PING_REQUEST, mtkRadioEx::onWifiPingRequest, WAKE_PARTIAL},
    {RIL_UNSOL_WIFI_PDN_OOS, mtkRadioEx::onWifiPdnOOS, WAKE_PARTIAL},
    {RIL_UNSOL_WIFI_LOCK, mtkRadioEx::onWifiLock, WAKE_PARTIAL},
    {RIL_UNSOL_SSAC_STATUS,  mtkRadioEx::onSsacStatus, WAKE_PARTIAL},
///@}
    {RIL_UNSOL_IMS_SUPPORT_ECC, mtkRadioEx::imsSupportEccInd, WAKE_PARTIAL},
    {RIL_UNSOL_REDIAL_EMERGENCY_INDICATION, mtkRadioEx::redialEmergencyIndication, WAKE_PARTIAL},
    {RIL_UNSOL_EMERGENCY_BEARER_SUPPORT_NOTIFY, mtkRadioEx::emergencyBearerInfoInd, WAKE_PARTIAL},
    {RIL_UNSOL_TX_POWER, mtkRadioEx::onTxPowerIndication, WAKE_PARTIAL},
    {RIL_UNSOL_TX_POWER_STATUS, mtkRadioEx::onTxPowerStatusIndication, WAKE_PARTIAL},
    {RIL_UNSOL_LTE_NETWORK_INFO, mtkRadioEx::responseLteNetworkInfo, WAKE_PARTIAL},
    {RIL_UNSOL_PHYSICAL_CHANNEL_CONFIGS_MTK, radio::currentPhysicalChannelConfigs, WAKE_PARTIAL},
    {RIL_UNSOL_LINK_CAPACITY_ESTIMATE, radio::currentLinkCapacityEstimate, WAKE_PARTIAL},

    {RIL_UNSOL_IMS_CONFERENCE_INFO_INDICATION, mtkRadioEx::onImsConferenceInfoIndication, WAKE_PARTIAL},
    {RIL_UNSOL_LTE_MESSAGE_WAITING_INDICATION, mtkRadioEx::onLteMessageWaitingIndication, WAKE_PARTIAL},

///M:Dialog Event Package Info @{
    {RIL_UNSOL_IMS_DIALOG_INDICATION, mtkRadioEx::imsDialogIndicationInd, WAKE_PARTIAL},
///@}
    {RIL_UNSOL_ECC_NUM, mtkRadioEx::eccNumIndication, WAKE_PARTIAL},

    {RIL_UNSOL_MCCMNC_CHANGED, mtkRadioEx::onMccMncChangedInd, WAKE_PARTIAL},
    {RIL_UNSOL_NETWORK_REJECT_CAUSE, mtkRadioEx::networkRejectCauseInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_DATA_INFO_NOTIFY, mtkRadioEx::imsDataInfoNotifyInd, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT_EX, mtkRadioEx::newSmsStatusReportIndEx, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_NEW_SMS_EX, mtkRadioEx::newSmsIndEx, WAKE_PARTIAL},
    {RIL_UNSOL_DSBP_STATE_CHANGED, mtkRadioEx::onDsbpStateChanged, WAKE_PARTIAL},
    // MTK-START: SIM SLOT LOCK
    {RIL_UNSOL_SIM_SLOT_LOCK_POLICY_NOTIFY, mtkRadioEx::smlSlotLockInfoChangedInd, WAKE_PARTIAL},
    // MTK-END
    // MTK-START: SIM ATT RSU
    {RIL_UNSOL_ATT_SIM_LOCK_NOTIFICATION, mtkRadioEx::onRsuSimLockEvent, WAKE_PARTIAL},
    // MTK-END
    {RIL_UNSOL_RESPONSE_CDMA_NEW_SMS_EX, mtkRadioEx::cdmaNewSmsIndEx, WAKE_PARTIAL},
    {RIL_UNSOL_NO_EMERGENCY_CALLBACK_MODE, mtkRadioEx::noEmergencyCallbackModeInd, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_POWER_CHANGED, mtkRadioEx::onSimPowerChangedInd, WAKE_PARTIAL},
    {RIL_UNSOL_CARD_DETECTED_IND, mtkRadioEx::onCardDetectedInd, WAKE_PARTIAL},
    {RIL_UNSOL_SUPP_SVC_NOTIFICATION_EX, mtkRadioEx::suppSvcNotifyExInd, WAKE_PARTIAL},
// M: RTT {
    {RIL_UNSOL_RTT_MODIFY_RESPONSE, mtkRadioEx::rttModifyResponseInd, WAKE_PARTIAL},
    {RIL_UNSOL_RTT_TEXT_RECEIVE, mtkRadioEx::rttTextReceiveInd, WAKE_PARTIAL},
    {RIL_UNSOL_RTT_CAPABILITY_INDICATION, mtkRadioEx::rttCapabilityIndicationInd, WAKE_PARTIAL},
    {RIL_UNSOL_RTT_MODIFY_REQUEST_RECEIVE, mtkRadioEx::rttModifyRequestReceiveInd, WAKE_PARTIAL},
    {RIL_UNSOL_AUDIO_INDICATION, mtkRadioEx::audioIndicationInd, WAKE_PARTIAL},
// @}
    {RIL_UNSOL_VOPS_INDICATION, mtkRadioEx::sendVopsIndication, WAKE_PARTIAL},
    {RIL_UNSOL_SIP_REG_INFO, mtkRadioEx::sipRegInfoInd, WAKE_PARTIAL},
    {RIL_UNSOL_ON_DSDA_CHANGED, mtkRadioEx::onDsdaChangedInd, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_REGISTRATION_STATE_IND, mtkRadioEx::imsRegistrationStateInd, WAKE_PARTIAL},
    {RIL_UNSOL_SIP_HEADER, mtkRadioEx::sipHeaderReportInd, WAKE_PARTIAL},
    {RIL_UNSOL_CALL_RAT_INDICATION, mtkRadioEx::callRatIndication, WAKE_PARTIAL},
    {RIL_UNSOL_QUALIFIED_NETWORK_TYPES_CHANGED, mtkRadioEx::qualifiedNetworkTypesChangedInd, WAKE_PARTIAL},
    {RIL_UNSOL_EIREG_INFO_IND, mtkRadioEx::eiregDataInd, WAKE_PARTIAL},
    // MUSE WFC requirement start
    {RIL_UNSOL_IWLAN_CELLULAR_QUALITY_CHANGED_IND, mtkRadioEx::onCellularQualityChangedInd, WAKE_PARTIAL},
    // MUSE WFC requirement end
    {RIL_UNSOL_MOBILE_DATA_USAGE, mtkRadioEx::mobileDataUsageInd, WAKE_PARTIAL},
    {RIL_UNSOL_NW_LIMIT, mtkRadioEx::onNwLimitInd, WAKE_PARTIAL},
    {RIL_UNSOL_SML_RSU_EVENT, mtkRadioEx::onRsuEvent, WAKE_PARTIAL},
#else
    {RIL_UNSOL_RESPONSE_PLMN_CHANGED, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED, responseInts, WAKE_PARTIAL},
/// M: [C2K 6M][NW] add for Iwlan @{
    {RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED, responseInts, WAKE_PARTIAL},
/// M: [C2K 6M][NW] add for Iwlan @{
    {RIL_UNSOL_GMSS_RAT_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_CDMA_PLMN_CHANGED, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_CS_NETWORK_STATE_CHANGED, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_INVALID_SIM, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_NETWORK_EVENT, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_MODULATION_INFO, responseInts, WAKE_PARTIAL},
// MTK-START: SIM
    {RIL_UNSOL_VIRTUAL_SIM_ON,responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_VIRTUAL_SIM_OFF,responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_IMEI_LOCK, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_IMSI_REFRESH_DONE, responseVoid, WAKE_PARTIAL},
// MTK-END
/// M: eMBMS feature
// only used in ril-proxy, and which use rilproxy\libril\ril.cpp use the same name file under libril
    {RIL_UNSOL_EMBMS_SESSION_STATUS, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_EMBMS_AT_INFO, responseString, WAKE_PARTIAL},
/// M: eMBMS end

// ATCI
    {RIL_UNSOL_ATCI_RESPONSE, responseRaw, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION, responseEtwsNotification, WAKE_PARTIAL},
    {RIL_UNSOL_ME_SMS_STORAGE_FULL, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_SMS_READY_NOTIFICATION, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_DATA_ALLOWED, responseInts, WAKE_PARTIAL},
/// M: CC: Proprietary incoming call indication
    {RIL_UNSOL_INCOMING_CALL_INDICATION, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_CALL_ADDITIONAL_INFO, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_PSEUDO_CELL_INFO, responseInts, WAKE_PARTIAL},
//Reset Attach APN
    {RIL_UNSOL_RESET_ATTACH_APN, responseVoid, WAKE_PARTIAL},
// M: IA-change attach APN
    {RIL_UNSOL_DATA_ATTACH_APN_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION, responseEtwsNotification, WAKE_PARTIAL},
/// M: CC: Cipher indication support
    {RIL_UNSOL_CIPHER_INDICATION, responseStrings, WAKE_PARTIAL},
/// M: CC: Call control CRSS handling
    {RIL_UNSOL_CRSS_NOTIFICATION, responseCrssN, WAKE_PARTIAL},
    /// M: CC: GSA HD Voice for 2/3G network support
    {RIL_UNSOL_SPEECH_CODEC_INFO, responseInts, WAKE_PARTIAL},
// World Phone
    {RIL_UNSOL_WORLD_MODE_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_CDMA_CARD_INITIAL_ESN_OR_MEID, responseString, WAKE_PARTIAL},
    {RIL_UNSOL_FEMTOCELL_INFO, responseStrings, WAKE_PARTIAL},
// / M: BIP {
    {RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND, responseString, WAKE_PARTIAL},
// / M: BIP }
// SS
    {RIL_UNSOL_CALL_FORWARDING, responseInts, WAKE_PARTIAL},
// PHB
    {RIL_UNSOL_PHB_READY_NOTIFICATION, responseInts, WAKE_PARTIAL},
// / M: OTASP {
    {RIL_UNSOL_TRIGGER_OTASP, responseVoid, WAKE_PARTIAL},
// / M: OTASP }
    {RIL_UNSOL_MD_DATA_RETRY_COUNT_RESET, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_REMOVE_RESTRICT_EUTRAN, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_PCO_STATUS, responseInts, WAKE_PARTIAL},
// M: [LTE][Low Power][UL traffic shaping] @{
    {RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE, responseInts, WAKE_PARTIAL},
// M: [LTE][Low Power][UL traffic shaping] @}
// MTK-START: SIM HOT SWAP
    {RIL_UNSOL_SIM_PLUG_IN, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_PLUG_OUT, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_MISSING, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_RECOVERY, responseInts, WAKE_PARTIAL},
// MTK-END
// MTK-START: SIM COMMON SLOT
    {RIL_UNSOL_TRAY_PLUG_IN, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED, responseVoid, WAKE_PARTIAL},
// MTK-END
    /// [IMS] IMS Indication ================================================================
    {RIL_UNSOL_CALL_INFO_INDICATION, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_ECONF_RESULT_INDICATION, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_SIP_CALL_PROGRESS_INDICATOR, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_CALLMOD_CHANGE_INDICATOR, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_VIDEO_CAPABILITY_INDICATOR, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_ON_USSI, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_ON_VOLTE_SUBSCRIPTION, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_GET_PROVISION_DONE, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_RTP_INFO, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_ON_XUI, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_EVENT_PACKAGE_INDICATION, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_ECONF_SRVCC_INDICATION, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_REGISTRATION_INFO, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_ENABLE_DONE, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_DISABLE_DONE, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_ENABLE_START, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_DISABLE_START, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_ECT_INDICATION, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_VOLTE_SETTING, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_BEARER_STATE_NOTIFY, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_BEARER_INIT, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_DEREG_DONE, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_CONFIG_DYNAMIC_IMS_SWITCH_COMPLETE, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_CONFIG_FEATURE_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_CONFIG_CONFIG_CHANGED, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_CONFIG_CONFIG_LOADED, responseVoid, WAKE_PARTIAL},

    /// [IMS] IMS Indication ================================================================
    // M: [VzW] Data Framework
    {RIL_UNSOL_PCO_DATA_AFTER_ATTACHED, responsePcoDataAfterAttached, WAKE_PARTIAL},
    {RIL_UNSOL_NETWORK_INFO, responseStrings, WAKE_PARTIAL},
// MTK-START: SIM TMO RSU
    {RIL_UNSOL_MELOCK_NOTIFICATION,responseInts, WAKE_PARTIAL},
// MTK-END
// MTK-START: SIM ATT RSU
    {RIL_UNSOL_ATT_SIM_LOCK_NOTIFICATION, responseInts, WAKE_PARTIAL},
// MTK-END
/// M: CC: CDMA call accepted indication
    {RIL_UNSOL_CDMA_CALL_ACCEPTED, responseVoid, WAKE_PARTIAL},
// /M: STK{
    {RIL_UNSOL_STK_SETUP_MENU_RESET, responseVoid, WAKE_PARTIAL},
// /M: STK}

    // External SIM [Start]
    {RIL_UNSOL_VSIM_OPERATION_INDICATION, responseVsimOperationEvent, WAKE_PARTIAL},
    // External SIM [End]

// M: [VzW] Data Framework
    {RIL_UNSOL_VOLTE_LTE_CONNECTION_STATUS, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_MULTIIMS_COUNT, responseInts, WAKE_PARTIAL},

///M: MwiService @{
    {RIL_UNSOL_MOBILE_WIFI_ROVEOUT, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_MOBILE_WIFI_HANDOVER, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_ACTIVE_WIFI_PDN_COUNT, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_WIFI_RSSI_MONITORING_CONFIG, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_WIFI_PDN_ERROR, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_REQUEST_GEO_LOCATION, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_WFC_PDN_STATE, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_NATT_KEEP_ALIVE_CHANGED, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_WIFI_PDN_OOS, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_SSAC_STATUS,  responseInts, WAKE_PARTIAL},
///@}
    {RIL_UNSOL_IMS_SUPPORT_ECC, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_REDIAL_EMERGENCY_INDICATION, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_EMERGENCY_BEARER_SUPPORT_NOTIFY, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_TX_POWER, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_LTE_NETWORK_INFO, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_CONFERENCE_INFO_INDICATION, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_LTE_MESSAGE_WAITING_INDICATION, responseStrings, WAKE_PARTIAL},

    ///M:Dialog Event Package Info @{
    {RIL_UNSOL_IMS_DIALOG_INDICATION, responseStrings, WAKE_PARTIAL},
    ///@}

    {RIL_UNSOL_LINK_CAPACITY_ESTIMATE, responseLinkCapacityEstimate, WAKE_PARTIAL},
// MTK-START: SIM SLOT LOCK
    {RIL_UNSOL_SIM_SLOT_LOCK_POLICY_NOTIFY, responseInts, WAKE_PARTIAL},
// MTK-END
    {RIL_UNSOL_NO_EMERGENCY_CALLBACK_MODE, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_POWER_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_CARD_DETECTED_IND, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_VOPS_INDICATION, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_SIP_HEADER, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_SUPP_SVC_NOTIFICATION_EX, responseSsn, WAKE_PARTIAL},
    {RIL_UNSOL_CALL_RAT_INDICATION, responseInts, WAKE_PARTIAL},
    #endif // MTK_USE_HIDL

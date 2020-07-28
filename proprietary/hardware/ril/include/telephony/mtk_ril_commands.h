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
 // MTK-START: SIM
 {RIL_REQUEST_SIM_GET_ATR, mtkRadioEx::getATRResponse},
 {RIL_REQUEST_SIM_GET_ICCID, mtkRadioEx::getIccidResponse},
 {RIL_REQUEST_SET_SIM_POWER, mtkRadioEx::setSimPowerResponse},
 // MTK-END
 // modem power
 {RIL_REQUEST_MODEM_POWERON, mtkRadioEx::setModemPowerResponse},
 {RIL_REQUEST_MODEM_POWEROFF, mtkRadioEx::setModemPowerResponse},
 // MTK-START: NW
 {RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT, mtkRadioEx::setNetworkSelectionModeManualWithActResponse},
 {RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT, mtkRadioEx::getAvailableNetworksWithActResponse},
 {RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS, mtkRadioEx::cancelAvailableNetworksResponse},
 {RIL_REQUEST_SIGNAL_STRENGTH_WITH_WCDMA_ECIO, mtkRadioEx::getSignalStrengthWithWcdmaEcioResponse},
 // ATCI
 {RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL, mtkRadioEx::sendAtciResponse},
 // M: To set language configuration for GSM cell broadcast
 {RIL_REQUEST_GSM_SET_BROADCAST_LANGUAGE, mtkRadioEx::setGsmBroadcastLangsResponse},
 // M: To get language configuration for GSM cell broadcast
 {RIL_REQUEST_GSM_GET_BROADCAST_LANGUAGE, mtkRadioEx::getGsmBroadcastLangsResponse},
 {RIL_REQUEST_GET_SMS_SIM_MEM_STATUS, mtkRadioEx::getSmsMemStatusResponse},
 {RIL_REQUEST_GET_SMS_PARAMS, mtkRadioEx::getSmsParametersResponse},
 {RIL_REQUEST_SET_SMS_PARAMS, mtkRadioEx::setSmsParametersResponse},
 {RIL_REQUEST_SET_ETWS, mtkRadioEx::setEtwsResponse},
 {RIL_REQUEST_REMOVE_CB_MESSAGE, mtkRadioEx::removeCbMsgResponse},
 /// M: CC: Proprietary incoming call indication
 {RIL_REQUEST_SET_CALL_INDICATION, mtkRadioEx::setCallIndicationResponse},
 /// M: CC: Proprietary ECC handling @{
 /// M: eMBMS feature
 {RIL_REQUEST_EMBMS_AT_CMD, mtkRadioEx::sendEmbmsAtCommandResponse},
 /// M: eMBMS end
 /// @}
 /// M: CC: Proprietary call control hangup all
 {RIL_REQUEST_HANGUP_ALL, mtkRadioEx::hangupAllResponse},                                             /// M: CC: Proprietary call control hangup all ([IMS] common flow)
 // M: CC: Proprietary call control hangupWithReason
 {RIL_REQUEST_HANGUP_WITH_REASON, mtkRadioEx::hangupWithReasonResponse},
 /// M: CC: Emergency mode for Fusion RIL
 {RIL_REQUEST_SET_ECC_MODE, mtkRadioEx::setEccModeResponse},
 /// M: CC: Vzw/CTVolte ECC for Fusion RIL
 {RIL_REQUEST_ECC_PREFERRED_RAT, mtkRadioEx::eccPreferredRatResponse},                                /// M: CC: CtVolte/Vzw E911
 {RIL_REQUEST_SET_PSEUDO_CELL_MODE, mtkRadioEx::setApcModeResponse},
 {RIL_REQUEST_GET_PSEUDO_CELL_INFO, mtkRadioEx::getApcInfoResponse},
 {RIL_REQUEST_SWITCH_MODE_FOR_ECC, mtkRadioEx::triggerModeSwitchByEccResponse},
 // MTK-START: SIM GBA
 {RIL_REQUEST_GENERAL_SIM_AUTH, radio::iccIOForAppResponse},
 // MTK-END
 {RIL_REQUEST_GET_SMS_RUIM_MEM_STATUS, mtkRadioEx::getSmsRuimMemoryStatusResponse},
 // FastDormancy
 {RIL_REQUEST_SET_FD_MODE, mtkRadioEx::setFdModeResponse},
 {RIL_REQUEST_MODIFY_MODEM_TYPE, mtkRadioEx::modifyModemTypeResponse},
 {RIL_REQUEST_RESUME_REGISTRATION, mtkRadioEx::setResumeRegistrationResponse},
 {RIL_REQUEST_SET_TRM, mtkRadioEx::setTrmResponse},
 {RIL_REQUEST_RESTART_RILD, mtkRadioEx::restartRILDResponse},
 //STK
 {RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE, mtkRadioEx::handleStkCallSetupRequestFromSimWithResCodeResponse},
 //SS
 {RIL_REQUEST_SET_CLIP, mtkRadioEx::setClipResponse},
 {RIL_REQUEST_GET_COLP, mtkRadioEx::getColpResponse},
 {RIL_REQUEST_GET_COLR, mtkRadioEx::getColrResponse},
 {RIL_REQUEST_SEND_CNAP, mtkRadioEx::sendCnapResponse},
 {RIL_REQUEST_SET_COLP, mtkRadioEx::setColpResponse},
 {RIL_REQUEST_SET_COLR, mtkRadioEx::setColrResponse},
 {RIL_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT, mtkRadioEx::queryCallForwardInTimeSlotStatusResponse},
 {RIL_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT, mtkRadioEx::setCallForwardInTimeSlotResponse},
 {RIL_REQUEST_RUN_GBA, mtkRadioEx::runGbaAuthenticationResponse},
 {RIL_REQUEST_GET_XCAP_STATUS, mtkRadioEx::getXcapStatusResponse},
 {RIL_REQUEST_RESET_SUPP_SERV, mtkRadioEx::resetSuppServResponse},
 {RIL_REQUEST_SETUP_XCAP_USER_AGENT_STRING, mtkRadioEx::setupXcapUserAgentStringResponse},
 {RIL_REQUEST_SET_SS_PROPERTY, mtkRadioEx::setSuppServPropertyResponse},
 // PHB START
 {RIL_REQUEST_QUERY_PHB_STORAGE_INFO, mtkRadioEx::queryPhbStorageInfoResponse},
 {RIL_REQUEST_WRITE_PHB_ENTRY, mtkRadioEx::writePhbEntryResponse},
 {RIL_REQUEST_READ_PHB_ENTRY, mtkRadioEx::readPhbEntryResponse},
 {RIL_REQUEST_QUERY_UPB_CAPABILITY, mtkRadioEx::queryUPBCapabilityResponse},
 {RIL_REQUEST_EDIT_UPB_ENTRY, mtkRadioEx::editUPBEntryResponse},
 {RIL_REQUEST_DELETE_UPB_ENTRY, mtkRadioEx::deleteUPBEntryResponse},
 {RIL_REQUEST_READ_UPB_GAS_LIST, mtkRadioEx::readUPBGasListResponse},
 {RIL_REQUEST_READ_UPB_GRP, mtkRadioEx::readUPBGrpEntryResponse},
 {RIL_REQUEST_WRITE_UPB_GRP, mtkRadioEx::writeUPBGrpEntryResponse},
 {RIL_REQUEST_GET_PHB_STRING_LENGTH, mtkRadioEx::getPhoneBookStringsLengthResponse},
 {RIL_REQUEST_GET_PHB_MEM_STORAGE, mtkRadioEx::getPhoneBookMemStorageResponse},
 {RIL_REQUEST_SET_PHB_MEM_STORAGE, mtkRadioEx::setPhoneBookMemStorageResponse},
 {RIL_REQUEST_READ_PHB_ENTRY_EXT, mtkRadioEx::readPhoneBookEntryExtResponse},
 {RIL_REQUEST_WRITE_PHB_ENTRY_EXT, mtkRadioEx::writePhoneBookEntryExtResponse},
 {RIL_REQUEST_QUERY_UPB_AVAILABLE, mtkRadioEx::queryUPBAvailableResponse},
 {RIL_REQUEST_READ_EMAIL_ENTRY, mtkRadioEx::readUPBEmailEntryResponse},
 {RIL_REQUEST_READ_SNE_ENTRY, mtkRadioEx::readUPBSneEntryResponse},
 {RIL_REQUEST_READ_ANR_ENTRY, mtkRadioEx::readUPBAnrEntryResponse},
 {RIL_REQUEST_READ_UPB_AAS_LIST, mtkRadioEx::readUPBAasListResponse},
 {RIL_REQUEST_SET_PHONEBOOK_READY, mtkRadioEx::setPhonebookReadyResponse},
 // PHB END
 //Femtocell (CSG) feature
 {RIL_REQUEST_GET_FEMTOCELL_LIST, mtkRadioEx::getFemtocellListResponse},
 // Femtocell (CSG) : abort command shall be sent in differenent channel
 {RIL_REQUEST_ABORT_FEMTOCELL_LIST, mtkRadioEx::abortFemtocellListResponse},
 {RIL_REQUEST_SELECT_FEMTOCELL, mtkRadioEx::selectFemtocellResponse},
 {RIL_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE, mtkRadioEx::queryFemtoCellSystemSelectionModeResponse},
 {RIL_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE, mtkRadioEx::setFemtoCellSystemSelectionModeResponse},
 // Data
 // M: Data Framework - common part enhancement
 {RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD, mtkRadioEx::syncDataSettingsToMdResponse},
 // M: Data Framework - Data Retry enhancement
 {RIL_REQUEST_RESET_MD_DATA_RETRY_COUNT, mtkRadioEx::resetMdDataRetryCountResponse},
 // M: Data Framework - CC 33
 {RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE, mtkRadioEx::setRemoveRestrictEutranModeResponse},
 // M: [LTE][Low Power][UL traffic shaping] @{
 {RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT, mtkRadioEx::setLteAccessStratumReportResponse},
 {RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER, mtkRadioEx::setLteUplinkDataTransferResponse},
 // M: [LTE][Low Power][UL traffic shaping] @}
 // MTK-START: SIM ME LOCK
 {RIL_REQUEST_QUERY_SIM_NETWORK_LOCK, mtkRadioEx::queryNetworkLockResponse},
 {RIL_REQUEST_SET_SIM_NETWORK_LOCK, mtkRadioEx::setNetworkLockResponse},
 {RIL_REQUEST_ENTER_DEPERSONALIZATION, mtkRadioEx::supplyDepersonalizationResponse},
 // MTK-END
 // [IMS] Enable/Disable IMS
 {RIL_REQUEST_SET_IMS_ENABLE, mtkRadioEx::setImsEnableResponse},
 // [IMS] Enable/Disable IMS Video
 {RIL_REQUEST_SET_IMS_VIDEO_ENABLE, mtkRadioEx::setImsVideoEnableResponse},
 // [IMS] Accept Video Call
 {RIL_REQUEST_VIDEO_CALL_ACCEPT, mtkRadioEx::videoCallAcceptResponse},
 // [IMS] Approve ECC Redial
 {RIL_REQUEST_ECC_REDIAL_APPROVE, mtkRadioEx::eccRedialApproveResponse},
 // [IMS] Enable/Disable IMS Features
 {RIL_REQUEST_SET_IMSCFG, mtkRadioEx::setImscfgResponse},
 // [IMS] Set IMS configuration to modem
 {RIL_REQUEST_SET_MD_IMSCFG, mtkRadioEx::setModemImsCfgResponse},
 // [IMS] Get Provision Value
 {RIL_REQUEST_GET_PROVISION_VALUE, mtkRadioEx::getProvisionValueResponse},
 // [IMS] Set Provision Value
 {RIL_REQUEST_SET_PROVISION_VALUE, mtkRadioEx::setProvisionValueResponse},
 // [IMS][Telephonyware] Set IMS Config Feature Value
 {RIL_REQUEST_IMS_CONFIG_SET_FEATURE, mtkRadioEx::setImsCfgFeatureValueResponse},
 // [IMS][Telephonyware] Set IMS Config Feature Value
 {RIL_REQUEST_IMS_CONFIG_GET_FEATURE, mtkRadioEx::getImsCfgFeatureValueResponse},
 // [IMS][Telephonyware] Set IMS Config Provision Value
 {RIL_REQUEST_IMS_CONFIG_SET_PROVISION, mtkRadioEx::setImsCfgProvisionValueResponse},
 // [IMS][Telephonyware] Get IMS Config Provision Value
 {RIL_REQUEST_IMS_CONFIG_GET_PROVISION, mtkRadioEx::getImsCfgProvisionValueResponse},
 // [IMS][Telephonyware] Get IMS Config Resource Cap Value
 {RIL_REQUEST_IMS_CONFIG_GET_RESOURCE_CAP, mtkRadioEx::getImsCfgResourceCapValueResponse},
 // [IMS] Set Provision Value
 {RIL_REQUEST_IMS_BEARER_STATE_CONFIRM, mtkRadioEx::imsBearerStateConfirmResponse},
 // [IMS] IMS Bearer Activiation Done
{RIL_REQUEST_SET_IMS_BEARER_NOTIFICATION, mtkRadioEx::setImsBearerNotificationResponse},
 // [IMS] Set IMS Bearer Notification
 {RIL_REQUEST_IMS_DEREG_NOTIFICATION, mtkRadioEx::imsDeregNotificationResponse},
 // [IMS] IMS Deregister Notification
 {RIL_REQUEST_IMS_ECT, mtkRadioEx::imsEctCommandResponse},
 // [IMS] Hold Call
 {RIL_REQUEST_HOLD_CALL, mtkRadioEx::controlCallResponse},
 // [IMS] Resume Call
 {RIL_REQUEST_RESUME_CALL, mtkRadioEx::controlCallResponse},
 // [IMS] Dial with SIP URI
 {RIL_REQUEST_DIAL_WITH_SIP_URI, mtkRadioEx::dialWithSipUriResponse},
 // [IMS] Force Release Call
 {RIL_REQUEST_FORCE_RELEASE_CALL, mtkRadioEx::forceReleaseCallResponse},
 // [IMS] SET IMS RTP Report
 {RIL_REQUEST_SET_IMS_RTP_REPORT, mtkRadioEx::setImsRtpReportResponse},
 // [IMS] Conference Dial
 {RIL_REQUEST_CONFERENCE_DIAL, mtkRadioEx::conferenceDialResponse},
 // [IMS] Add Member to Conference
 {RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER, mtkRadioEx::controlImsConferenceCallMemberResponse},
 // [IMS] Remove Member to Conference
 {RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER, mtkRadioEx::controlImsConferenceCallMemberResponse},
 // [IMS] Dial a VT Call with SIP URI
 {RIL_REQUEST_VT_DIAL_WITH_SIP_URI, mtkRadioEx::vtDialWithSipUriResponse},
 // [IMS] Send USSI
 {RIL_REQUEST_SEND_USSI, mtkRadioEx::sendUssiResponse},
 // [IMS] Cancel USSI
 {RIL_REQUEST_CANCEL_USSI, mtkRadioEx::cancelUssiResponse},
 // [IMS] Set WFC Profile
 {RIL_REQUEST_SET_WFC_PROFILE, mtkRadioEx::setWfcProfileResponse},
 // [IMS] Pull CALL
 {RIL_REQUEST_PULL_CALL, mtkRadioEx::pullCallResponse},
 // [IMS] Set IMS Registrtion Report Enable
 {RIL_REQUEST_SET_IMS_REGISTRATION_REPORT, mtkRadioEx::setImsRegistrationReportResponse},
 // [IMS] Dial
 {RIL_REQUEST_IMS_DIAL, mtkRadioEx::imsDialResponse},
 // [IMS] VT Dial
 {RIL_REQUEST_IMS_VT_DIAL, mtkRadioEx::imsVtDialResponse},
 // [IMS] Emergency Dial
 {RIL_REQUEST_IMS_EMERGENCY_DIAL, mtkRadioEx::imsEmergencyDialResponse},
 {RIL_REQUEST_GET_POL_CAPABILITY, mtkRadioEx::getPOLCapabilityResponse},
 {RIL_REQUEST_GET_POL_LIST, mtkRadioEx::getCurrentPOLListResponse},
 {RIL_REQUEST_SET_POL_ENTRY, mtkRadioEx::setPOLEntryResponse},
 /// M: [Network][C2K] Sprint roaming control @{
 {RIL_REQUEST_SET_ROAMING_ENABLE, mtkRadioEx::setRoamingEnableResponse},
 {RIL_REQUEST_GET_ROAMING_ENABLE, mtkRadioEx::getRoamingEnableResponse},
 /// @}
 // External SIM [START]
 {RIL_REQUEST_VSIM_NOTIFICATION, mtkRadioEx::vsimNotificationResponse},
 {RIL_REQUEST_VSIM_OPERATION, mtkRadioEx::vsimOperationResponse},
 // External SIM [END]
 {RIL_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION, mtkRadioEx::getGsmBroadcastActivationRsp},
 {RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE, mtkRadioEx::setVoiceDomainPreferenceResponse},
 {RIL_REQUEST_GET_VOICE_DOMAIN_PREFERENCE, mtkRadioEx::getVoiceDomainPreferenceResponse},
 /// M: MwiService @{
 {RIL_REQUEST_SET_WIFI_ENABLED, mtkRadioEx::setWifiEnabledResponse},
 {RIL_REQUEST_SET_WIFI_ASSOCIATED, mtkRadioEx::setWifiAssociatedResponse},
 {RIL_REQUEST_SET_WFC_CONFIG, mtkRadioEx::setWfcConfigResponse},
 {RIL_REQUEST_SET_WIFI_SIGNAL_LEVEL, mtkRadioEx::setWifiSignalLevelResponse},
 {RIL_REQUEST_SET_WIFI_IP_ADDRESS, mtkRadioEx::setWifiIpAddressResponse},
 {RIL_REQUEST_SET_GEO_LOCATION, mtkRadioEx::setLocationInfoResponse},
 {RIL_REQUEST_SET_EMERGENCY_ADDRESS_ID, mtkRadioEx::setEmergencyAddressIdResponse},
 {RIL_REQUEST_SET_SERVICE_STATE, mtkRadioEx::setServiceStateToModemResponse},
 {RIL_REQUEST_SET_NATT_KEEP_ALIVE_STATUS, mtkRadioEx::setNattKeepAliveStatusResponse},
 {RIL_REQUEST_SET_WIFI_PING_RESULT, mtkRadioEx::setWifiPingResultResponse},
 {RIL_REQUEST_NOTIFY_EPDG_SCREEN_STATE, mtkRadioEx::notifyEPDGScreenStateResponse},
 {RIL_REQUEST_QUERY_SSAC_STATUS, mtkRadioEx::querySsacStatusResponse},

 ///@}
 {RIL_REQUEST_UPDATE_SUBLOCK_SETTINGS, mtkRadioEx::sendSubsidyLockResponse},
 {RIL_REQUEST_GET_SUBLOCK_MODEM_STATUS, mtkRadioEx::sendSubsidyLockResponse},
 {RIL_REQUEST_IMS_SEND_SMS_EX, mtkRadioEx::sendImsSmsExResponse},
 {RIL_REQUEST_SMS_ACKNOWLEDGE_EX, mtkRadioEx::acknowledgeLastIncomingGsmSmsExResponse},
 {RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX, mtkRadioEx::acknowledgeLastIncomingCdmaSmsExResponse},
 // PS/CS attach
 {RIL_REQUEST_DATA_CONNECTION_ATTACH, mtkRadioEx::dataConnectionAttachResponse},
 // PS/CS detach
 {RIL_REQUEST_DATA_CONNECTION_DETACH, mtkRadioEx::dataConnectionDetachResponse},
 // Cleanup all connections
 {RIL_REQUEST_RESET_ALL_CONNECTIONS, mtkRadioEx::resetAllConnectionsResponse},
 {RIL_REQUEST_SET_VOICE_PREFER_STATUS, mtkRadioEx::setVoicePreferStatusResponse},
 {RIL_REQUEST_SET_ECC_NUM, mtkRadioEx::setEccNumResponse},
 {RIL_REQUEST_GET_ECC_NUM, mtkRadioEx::getEccNumResponse},
 {RIL_REQUEST_SET_LTE_RELEASE_VERSION, mtkRadioEx::setLteReleaseVersionResponse},
 {RIL_REQUEST_GET_LTE_RELEASE_VERSION, mtkRadioEx::getLteReleaseVersionResponse},
 {RIL_REQUEST_SET_TX_POWER_STATUS, mtkRadioEx::setTxPowerStatusResponse},
 /// M: SIM SLOT LOCK @{
 {RIL_REQUEST_ENTER_DEVICE_NETWORK_DEPERSONALIZATION, mtkRadioEx::supplyDeviceNetworkDepersonalizationResponse},
 {RIL_REQUEST_GET_TS25_NAME, mtkRadioEx::getPlmnNameFromSE13TableResponse},
 {RIL_REQUEST_ENABLE_CA_PLUS_FILTER, mtkRadioEx::enableCAPlusBandWidthFilterResponse},
 ///@}

 // M: RTT @{
 {RIL_REQUEST_SET_RTT_MODE, mtkRadioEx::setRttModeResponse},
 {RIL_REQUEST_SEND_RTT_MODIFY_REQUEST, mtkRadioEx::sendRttModifyRequestResponse},
 {RIL_REQUEST_SEND_RTT_TEXT, mtkRadioEx::sendRttTextResponse},
 {RIL_REQUEST_RTT_MODIFY_REQUST_RESPONSE, mtkRadioEx::rttModifyRequestResponseResponse},
 {RIL_REQUEST_TOGGLE_RTT_AUDIO_INDICATION, mtkRadioEx::toggleRttAudioIndicationResponse},
 // @}
 {RIL_REQUEST_QUERY_VOPS_STATUS, mtkRadioEx::queryVopsStatusResponse},
 // M: GWSD @{
 {RIL_REQUEST_SET_GWSD_MODE, mtkRadioEx::setGwsdModeResponse},
 {RIL_REQUEST_SET_GWSD_CALL_VALID, mtkRadioEx::setCallValidTimerResponse},
 {RIL_REQUEST_SET_GWSD_IGNORE_CALL_INTERVAL, mtkRadioEx::setIgnoreSameNumberIntervalResponse},
 {RIL_REQUEST_SET_GWSD_KEEP_ALIVE_PDCP, mtkRadioEx::setKeepAliveByPDCPCtrlPDUResponse},
 {RIL_REQUEST_SET_GWSD_KEEP_ALIVE_IPDATA, mtkRadioEx::setKeepAliveByIpDataResponse},
 {RIL_REQUEST_ENABLE_DSDA_INDICATION, mtkRadioEx::enableDsdaIndicationResponse},
 {RIL_REQUEST_GET_DSDA_STATUS, mtkRadioEx::getDsdaStatusResponse},
 // @}
 {RIL_REQUEST_SET_SIP_HEADER, mtkRadioEx::setSipHeaderResponse},
 {RIL_REQUEST_SIP_HEADER_REPORT, mtkRadioEx::setSipHeaderReportResponse},
 {RIL_REQUEST_SET_IMS_CALL_MODE, mtkRadioEx::setImsCallModeResponse},
 {RIL_REQUEST_ACTIVATE_UICC_CARD, mtkRadioEx::activateUiccCardRsp},
 {RIL_REQUEST_DEACTIVATE_UICC_CARD, mtkRadioEx::deactivateUiccCardRsp},
 {RIL_REQUEST_GET_CURRENT_UICC_CARD_PROVISIONING_STATUS, mtkRadioEx::getCurrentUiccCardProvisioningStatusRsp},
 {RIL_REQUEST_IWLAN_REGISTER_CELLULAR_QUALITY_REPORT, mtkRadioEx::registerCellQltyReportResponse},
 {RIL_REQUEST_GET_SUGGESTED_PLMN_LIST, mtkRadioEx::getSuggestedPlmnListResponse},
 {RIL_REQUEST_CONFIG_A2_OFFSET, mtkRadioEx::cfgA2offsetResponse},
 {RIL_REQUEST_CONFIG_B1_OFFSET, mtkRadioEx::cfgB1offsetResponse},
 {RIL_REQUEST_ENABLE_SCG_FAILURE, mtkRadioEx::enableSCGfailureResponse},
 {RIL_REQUEST_DISABLE_NR, mtkRadioEx::disableNRResponse},
 {RIL_REQUEST_SET_TX_POWER, mtkRadioEx::setTxPowerResponse},
 {RIL_REQUEST_SEARCH_STORED_FREQUENCY_INFO, mtkRadioEx::setSearchStoredFreqInfoResponse},
 {RIL_REQUEST_SEARCH_RAT, mtkRadioEx::setSearchRatResponse},
 {RIL_REQUEST_SET_BACKGROUND_SEARCH_TIMER, mtkRadioEx::setBgsrchDeltaSleepTimerResponse},
 {RIL_REQUEST_ROUTE_CERTIFICATE, mtkRadioEx::routeCertificateResponse},
 {RIL_REQUEST_ROUTE_AUTH, mtkRadioEx::routeAuthMessageResponse},
 {RIL_REQUEST_ENABLE_CAPABILITY, mtkRadioEx::enableCapabilityResponse},
 {RIL_REQUEST_ABORT_CERTIFICATE, mtkRadioEx::abortCertificateResponse},
 {RIL_REQUEST_SEND_SAR_IND, mtkRadioEx::sendSarIndicatorResponse},
 {RIL_REQUEST_SET_CALL_ADDITIONAL_INFO, mtkRadioEx::setCallAdditionalInfoResponse},
 {RIL_REQUEST_SML_RSU_REQUEST, mtkRadioEx::sendRsuRequestResponse},
#else

#ifdef C2K_RIL
  // Define GSM RILD channel id to avoid build error
  #define RIL_CMD_PROXY_1 RIL_CHANNEL_1
  #define RIL_CMD_PROXY_2 RIL_CHANNEL_1
  #define RIL_CMD_PROXY_3 RIL_CHANNEL_1
  #define RIL_CMD_PROXY_4 RIL_CHANNEL_1
  #define RIL_CMD_PROXY_5 RIL_CHANNEL_1
  #define RIL_CMD_PROXY_6 RIL_CHANNEL_1
#else
  // Define C2K RILD channel id to avoid build error
  #define RIL_CHANNEL_1 RIL_CMD_PROXY_1
  #define RIL_CHANNEL_2 RIL_CMD_PROXY_1
  #define RIL_CHANNEL_3 RIL_CMD_PROXY_1
  #define RIL_CHANNEL_4 RIL_CMD_PROXY_1
  #define RIL_CHANNEL_5 RIL_CMD_PROXY_1
  #define RIL_CHANNEL_6 RIL_CMD_PROXY_1
  #define RIL_CHANNEL_7 RIL_CMD_PROXY_1
#endif // C2K_RIL

// External SIM [Start]
#ifdef MTK_MUX_CHANNEL_64
    #define MTK_RIL_CHANNEL_VSIM RIL_CMD_PROXY_11
#else
    #define MTK_RIL_CHANNEL_VSIM RIL_CMD_PROXY_2
#endif
// External SIM [End]

#ifdef MTK_MUX_CHANNEL_64
    // To send GSM SMS through non-realtime channel if the platform support 64 MUX channels
    #define RIL_CHANNEL_GSM_SEND_SMS RIL_CMD_PROXY_8
#else
    #define RIL_CHANNEL_GSM_SEND_SMS RIL_CMD_PROXY_1
#endif

{RIL_REQUEST_RESUME_REGISTRATION, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_3},  // [C2K][IR] Support SVLTE IR feature

/// M: eMBMS feature
{RIL_REQUEST_EMBMS_AT_CMD, dispatchString, responseString, RIL_CMD_PROXY_6, RIL_CHANNEL_1},
/// M: eMBMS end
// MTK-START: SIM
{RIL_REQUEST_SIM_GET_ATR, dispatchVoid, responseString, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SIM_GET_ICCID, dispatchVoid, responseString, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_SIM_POWER, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
// MTK-END
// MTK-START: SIM GBA
{RIL_REQUEST_GENERAL_SIM_AUTH, dispatchSimAuth, responseSIM_IO, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
// MTK-END

// modem power
{RIL_REQUEST_MODEM_POWERON, dispatchVoid, responseVoid,RIL_CMD_PROXY_1, RIL_CHANNEL_3},
{RIL_REQUEST_MODEM_POWEROFF, dispatchVoid, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_3},

// MTK-START: NW
{RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT, dispatchStrings, responseVoid,RIL_CMD_PROXY_3, RIL_CHANNEL_3},
{RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT, dispatchVoid, responseStrings, RIL_CMD_PROXY_3, RIL_CHANNEL_3},
{RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS, dispatchVoid, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},//for PLMN List abort

// ATCI
{RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL, dispatchRaw, responseRaw, RIL_CMD_PROXY_6, RIL_CHANNEL_6},
// M: To set language configuration for GSM cell broadcast
{RIL_REQUEST_GSM_SET_BROADCAST_LANGUAGE, dispatchString, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
// M: To get language configuration for GSM cell broadcast
{RIL_REQUEST_GSM_GET_BROADCAST_LANGUAGE, dispatchVoid, responseString, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
{RIL_REQUEST_GET_SMS_SIM_MEM_STATUS, dispatchVoid, responseGetSmsSimMemStatusCnf, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
{RIL_REQUEST_GET_SMS_PARAMS, dispatchVoid, responseSmsParams, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_SMS_PARAMS, dispatchSmsParams, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_ETWS, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_REMOVE_CB_MESSAGE, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
/// M: CC: Proprietary incoming call indication
{RIL_REQUEST_SET_CALL_INDICATION, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
/// M: CC: Proprietary ECC handling @{
{RIL_REQUEST_SET_ECC_LIST, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
/// @}
/// M: CC: Proprietary call control hangup all
{RIL_REQUEST_HANGUP_ALL, dispatchVoid, responseVoid, RIL_CMD_PROXY_4, RIL_CHANNEL_2},
{RIL_REQUEST_HANGUP_WITH_REASON, dispatchInts, responseVoid, RIL_CMD_PROXY_4, RIL_CHANNEL_2},
// Verizon E911
{RIL_REQUEST_SET_ECC_MODE, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_3},
{RIL_REQUEST_ECC_PREFERRED_RAT, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_2},
{RIL_REQUEST_SET_PS_REGISTRATION, dispatchInts, responseVoid, RIL_CMD_PROXY_5, RIL_CHANNEL_1},
{RIL_REQUEST_SET_PSEUDO_CELL_MODE, dispatchInts, responseVoid, RIL_CMD_PROXY_3, RIL_CHANNEL_3},
{RIL_REQUEST_GET_PSEUDO_CELL_INFO, dispatchVoid, responseInts, RIL_CMD_PROXY_3, RIL_CHANNEL_3},
{RIL_REQUEST_SWITCH_MODE_FOR_ECC, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_GET_SMS_RUIM_MEM_STATUS, dispatchVoid, responseGetSmsSimMemStatusCnf, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
// FastDormancy
{RIL_REQUEST_SET_FD_MODE, dispatchFdMode, responseVoid, RIL_CMD_PROXY_3, RIL_CHANNEL_3},
{RIL_REQUEST_MODIFY_MODEM_TYPE, dispatchInts, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_TRM, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_3},
{RIL_REQUEST_RESTART_RILD, dispatchVoid, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
//STK
{RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_3},
//SS
{RIL_REQUEST_SET_CLIP, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_GET_COLP, dispatchVoid, responseInts, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_GET_COLR, dispatchVoid, responseInts, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SEND_CNAP, dispatchString, responseInts, RIL_CMD_PROXY_2, RIL_CHANNEL_1},

{RIL_REQUEST_SET_COLP, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SET_COLR, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT, dispatchCallForwardEx, responseCallForwardsEx, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT, dispatchCallForwardEx, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_RUN_GBA, dispatchStrings, responseStrings, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_GET_XCAP_STATUS, dispatchVoid, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_RESET_SUPP_SERV, dispatchVoid, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SETUP_XCAP_USER_AGENT_STRING, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SET_SS_PROPERTY, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},

// PHB START
{RIL_REQUEST_QUERY_PHB_STORAGE_INFO, dispatchInts, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_WRITE_PHB_ENTRY, dispatchPhbEntry, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_READ_PHB_ENTRY, dispatchInts, responsePhbEntries, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_QUERY_UPB_CAPABILITY, dispatchVoid, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_EDIT_UPB_ENTRY, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_DELETE_UPB_ENTRY, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_READ_UPB_GAS_LIST, dispatchInts, responseStrings, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_READ_UPB_GRP, dispatchInts, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_WRITE_UPB_GRP, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_GET_PHB_STRING_LENGTH, dispatchVoid, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_GET_PHB_MEM_STORAGE, dispatchVoid, responseGetPhbMemStorage, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_PHB_MEM_STORAGE, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_READ_PHB_ENTRY_EXT, dispatchInts, responseReadPhbEntryExt, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_WRITE_PHB_ENTRY_EXT, dispatchWritePhbEntryExt, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_QUERY_UPB_AVAILABLE, dispatchInts, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_READ_EMAIL_ENTRY, dispatchInts, responseString, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_READ_SNE_ENTRY, dispatchInts, responseString, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_READ_ANR_ENTRY, dispatchInts, responsePhbEntries, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_READ_UPB_AAS_LIST, dispatchInts, responseStrings, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_PHONEBOOK_READY, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
// PHB END
//Femtocell (CSG) feature
{RIL_REQUEST_GET_FEMTOCELL_LIST, dispatchVoid, responseStrings, RIL_CMD_PROXY_3, RIL_CHANNEL_1},
// Femtocell (CSG) : abort command shall be sent in differenent channel
{RIL_REQUEST_ABORT_FEMTOCELL_LIST, dispatchVoid, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SELECT_FEMTOCELL, dispatchStrings, responseVoid, RIL_CMD_PROXY_3, RIL_CHANNEL_1},
{RIL_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE, dispatchVoid, responseInts, RIL_CMD_PROXY_3, RIL_CHANNEL_1},
{RIL_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE, dispatchInts, responseVoid, RIL_CMD_PROXY_3, RIL_CHANNEL_1},
 // Data
{RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD, dispatchInts, responseVoid, RIL_CMD_PROXY_5, RIL_CHANNEL_4},
{RIL_REQUEST_RESET_MD_DATA_RETRY_COUNT, dispatchStrings, responseVoid, RIL_CMD_PROXY_5, RIL_CHANNEL_4},
// M: Data Framework - CC 33
{RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE, dispatchInts, responseVoid, RIL_CMD_PROXY_5, RIL_CHANNEL_4},
// M: [LTE][Low Power][UL traffic shaping] @{
{RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT, dispatchInts, responseVoid, RIL_CMD_PROXY_5, RIL_CHANNEL_4},
{RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER, dispatchInts, responseVoid, RIL_CMD_PROXY_5, RIL_CHANNEL_4},
// M: [LTE][Low Power][UL traffic shaping] @}
// MTK-START: SIM ME LOCK
{RIL_REQUEST_QUERY_SIM_NETWORK_LOCK, dispatchInts, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_SIM_NETWORK_LOCK, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_ENTER_DEPERSONALIZATION, dispatchStrings, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
// MTK-END
{RIL_REQUEST_SET_IMS_ENABLE, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_WFC_ENABLE, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_IMS_VIDEO_ENABLE, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_VIDEO_CALL_ACCEPT, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_ECC_REDIAL_APPROVE, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SET_IMSCFG, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_MD_IMSCFG, dispatchStrings, responseString, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_GET_PROVISION_VALUE, dispatchString, responseStrings, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_PROVISION_VALUE, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_IMS_CONFIG_SET_FEATURE, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_IMS_CONFIG_GET_FEATURE, dispatchInts, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_IMS_CONFIG_SET_PROVISION, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_IMS_CONFIG_GET_PROVISION, dispatchInts, responseString, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_IMS_CONFIG_GET_RESOURCE_CAP, dispatchInts, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_IMS_BEARER_STATE_CONFIRM, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_IMS_BEARER_NOTIFICATION, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_IMS_DEREG_NOTIFICATION, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_IMS_ECT, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_HOLD_CALL, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_RESUME_CALL, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_DIAL_WITH_SIP_URI, dispatchString, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_FORCE_RELEASE_CALL, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_IMS_RTP_REPORT, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_CONFERENCE_DIAL, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_VT_DIAL_WITH_SIP_URI, dispatchString, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SEND_USSI, dispatchString, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_CANCEL_USSI, dispatchVoid, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SET_WFC_PROFILE, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_PULL_CALL, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SET_IMS_REGISTRATION_REPORT, dispatchVoid, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_IMS_DIAL, dispatchDial, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_2},
{RIL_REQUEST_IMS_VT_DIAL, dispatchDial, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_IMS_EMERGENCY_DIAL, dispatchEmergencyDial, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_2},
// MTK_TC1_FEATURE for Antenna Testing start
{RIL_REQUEST_VSS_ANTENNA_CONF, dispatchInts, responseInts, RIL_CMD_PROXY_3, RIL_CHANNEL_1}, // Antenna Testing
{RIL_REQUEST_VSS_ANTENNA_INFO, dispatchInts, responseInts, RIL_CMD_PROXY_3, RIL_CHANNEL_1}, // Antenna Testing
// MTK_TC1_FEATURE for Antenna Testing end
// Preferred Operator List
{RIL_REQUEST_GET_POL_CAPABILITY, dispatchVoid, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_GET_POL_LIST, dispatchVoid, responseStrings, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_POL_ENTRY, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
/// M: [Network][C2K] Sprint roaming control @{
{RIL_REQUEST_SET_ROAMING_ENABLE, dispatchInts, responseVoid, RIL_CMD_PROXY_3, RIL_CHANNEL_3},
{RIL_REQUEST_GET_ROAMING_ENABLE, dispatchInts, responseInts, RIL_CMD_PROXY_3, RIL_CHANNEL_3},
/// @}
// External SIM [START]
{RIL_REQUEST_VSIM_NOTIFICATION, dispatchVsimEvent, responseInts, MTK_RIL_CHANNEL_VSIM, RIL_CHANNEL_1},
{RIL_REQUEST_VSIM_OPERATION, dispatchVsimOperationEvent, responseInts, MTK_RIL_CHANNEL_VSIM, RIL_CHANNEL_1},
// External SIM [END]
{RIL_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION, dispatchVoid, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
{RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_GET_VOICE_DOMAIN_PREFERENCE, dispatchVoid, responseInts, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SET_E911_STATE, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
/// M: MwiService @{
{RIL_REQUEST_SET_WIFI_ENABLED, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_WIFI_ASSOCIATED, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_WFC_CONFIG, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_WIFI_SIGNAL_LEVEL, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_WIFI_IP_ADDRESS, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_GEO_LOCATION, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_EMERGENCY_ADDRESS_ID, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_NATT_KEEP_ALIVE_STATUS, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_WIFI_PING_RESULT, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_NOTIFY_EPDG_SCREEN_STATE, dispatchInts, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_QUERY_SSAC_STATUS, dispatchVoid, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},

///@}
/// M: Network @{
{RIL_REQUEST_SET_SERVICE_STATE, dispatchInts, responseVoid, RIL_CMD_PROXY_3, RIL_CHANNEL_1},
///@}

/// M: Subsidylock @{
{RIL_REQUEST_UPDATE_SUBLOCK_SETTINGS, dispatchRaw, responseRaw, RIL_CMD_PROXY_3, RIL_CHANNEL_1},
{RIL_REQUEST_GET_SUBLOCK_MODEM_STATUS, dispatchRaw, responseRaw, RIL_CMD_PROXY_3, RIL_CHANNEL_1},
///@}
{RIL_REQUEST_IMS_SEND_SMS_EX, dispatchImsSms, responseSMS, RIL_CHANNEL_GSM_SEND_SMS, RIL_CHANNEL_5},
{RIL_REQUEST_SMS_ACKNOWLEDGE_EX, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_5},
/// M: SIM SLOT LOCK @{
{RIL_REQUEST_ENTER_DEVICE_NETWORK_DEPERSONALIZATION, dispatchStrings, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
///@}
{RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX, dispatchCdmaSmsAck, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
{RIL_REQUEST_QUERY_VOPS_STATUS, dispatchVoid, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_SIP_HEADER, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SIP_HEADER_REPORT, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_SET_IMS_CALL_MODE, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
{RIL_REQUEST_ACTIVATE_UICC_CARD, dispatchVoid, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_DEACTIVATE_UICC_CARD, dispatchVoid, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_GET_CURRENT_UICC_CARD_PROVISIONING_STATUS, dispatchVoid, responseInts, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_IWLAN_REGISTER_CELLULAR_QUALITY_REPORT, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
{RIL_REQUEST_SET_CALL_ADDITIONAL_INFO, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
#endif // MTK_USE_HIDL

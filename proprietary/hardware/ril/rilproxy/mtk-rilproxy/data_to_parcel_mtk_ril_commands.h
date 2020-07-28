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

{RIL_REQUEST_RESUME_REGISTRATION, intsToParcel, parcelToVoid},
{RIL_REQUEST_SIM_GET_ATR, voidToParcel, parcelToString},
{RIL_REQUEST_SET_SIM_POWER, intsToParcel, parcelToVoid},
{RIL_REQUEST_MODEM_POWERON, voidToParcel, parcelToVoid},
{RIL_REQUEST_MODEM_POWEROFF, voidToParcel, parcelToVoid},
{RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT, stringsToParcel, parcelToVoid},
{RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT, voidToParcel, parcelToStrings},
{RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS, voidToParcel, parcelToVoid},
{RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL, rawToParcel, parcelToRaw},
/// M: eMBMS feature
{RIL_REQUEST_EMBMS_AT_CMD, stringToParcel, parcelToString},
/// M: eMBMS end
// M: To set language configuration for GSM cell broadcast
{RIL_REQUEST_GSM_SET_BROADCAST_LANGUAGE, stringToParcel, parcelToVoid},
// M: To get language configuration for GSM cell broadcast
{RIL_REQUEST_GSM_GET_BROADCAST_LANGUAGE, voidToParcel, parcelToString},
{RIL_REQUEST_GET_SMS_SIM_MEM_STATUS, voidToParcel, parcelToGetSmsSimMemStatusCnf},
{RIL_REQUEST_GET_SMS_PARAMS, voidToParcel, parcelToSmsParams},
{RIL_REQUEST_SET_SMS_PARAMS, smsParamsToParcel, parcelToVoid},
{RIL_REQUEST_SET_ETWS, intsToParcel, parcelToVoid},
{RIL_REQUEST_REMOVE_CB_MESSAGE, intsToParcel, parcelToVoid},
/// M: CC: Proprietary incoming call indication
{RIL_REQUEST_SET_CALL_INDICATION, intsToParcel, parcelToVoid},
/// M: CC: Proprietary ECC enhancement @{
{RIL_REQUEST_SET_ECC_LIST, stringsToParcel, parcelToVoid},
/// @}
/// M: CC: Proprietary call control hangup all
{RIL_REQUEST_HANGUP_ALL, voidToParcel, parcelToVoid},
{RIL_REQUEST_SET_ECC_MODE, stringsToParcel, parcelToVoid}, // Verizon E911
{RIL_REQUEST_SET_PS_REGISTRATION, intsToParcel, parcelToVoid},
{RIL_REQUEST_SET_PSEUDO_CELL_MODE, intsToParcel, parcelToVoid},
{RIL_REQUEST_GET_PSEUDO_CELL_INFO, voidToParcel, parcelToInts},
{RIL_REQUEST_SWITCH_MODE_FOR_ECC, intsToParcel, parcelToVoid},
{RIL_REQUEST_GET_SMS_RUIM_MEM_STATUS, voidToParcel, parcelToGetSmsSimMemStatusCnf},
/// M: Ims Data Framework
{RIL_LOCAL_REQUEST_PCSCF_DISCOVERY_PCO, intsToParcel, parcelToString},
{RIL_LOCAL_REQUEST_WIFI_CONNECT_IND, rawToParcel, parcelToVoid},
/// @}
// FastDormancy
{RIL_REQUEST_SET_FD_MODE, fdModeToParcel, parcelToVoid},
{RIL_REQUEST_MODIFY_MODEM_TYPE, intsToParcel, parcelToInts},
{RIL_REQUEST_SET_TRM, intsToParcel, parcelToVoid},
/// M: STK @{
{RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE, intsToParcel, parcelToVoid},
/// @}
// SS
{RIL_REQUEST_SET_CLIP, intsToParcel, parcelToVoid},
{RIL_REQUEST_GET_COLP, voidToParcel, parcelToInts},
{RIL_REQUEST_GET_COLR, voidToParcel, parcelToInts},
{RIL_REQUEST_SEND_CNAP, voidToParcel, parcelToInts},
// PHB START
{RIL_REQUEST_QUERY_PHB_STORAGE_INFO, intsToParcel, parcelToInts},
{RIL_REQUEST_WRITE_PHB_ENTRY, phbEntryToParcel, parcelToVoid},
{RIL_REQUEST_READ_PHB_ENTRY, intsToParcel, parcelToPhbEntries},
{RIL_REQUEST_QUERY_UPB_CAPABILITY, voidToParcel, parcelToInts},
{RIL_REQUEST_EDIT_UPB_ENTRY, stringsToParcel, parcelToVoid},
{RIL_REQUEST_DELETE_UPB_ENTRY, intsToParcel, parcelToVoid},
{RIL_REQUEST_READ_UPB_GAS_LIST, intsToParcel, parcelToStrings},
{RIL_REQUEST_READ_UPB_GRP, intsToParcel, parcelToInts},
{RIL_REQUEST_WRITE_UPB_GRP, intsToParcel, parcelToVoid},
{RIL_REQUEST_GET_PHB_STRING_LENGTH, voidToParcel, parcelToInts},
{RIL_REQUEST_GET_PHB_MEM_STORAGE, voidToParcel, parcelToGetPhbMemStorage},
{RIL_REQUEST_SET_PHB_MEM_STORAGE, stringsToParcel, parcelToVoid},
{RIL_REQUEST_READ_PHB_ENTRY_EXT, intsToParcel, parcelToReadPhbEntryExt},
{RIL_REQUEST_WRITE_PHB_ENTRY_EXT, writePhbEntryExtToParcel, parcelToVoid},
{RIL_REQUEST_QUERY_UPB_AVAILABLE, intsToParcel, parcelToInts},
{RIL_REQUEST_READ_EMAIL_ENTRY, intsToParcel, parcelToString},
{RIL_REQUEST_READ_SNE_ENTRY, intsToParcel, parcelToString},
{RIL_REQUEST_READ_ANR_ENTRY, intsToParcel, parcelToPhbEntries},
{RIL_REQUEST_READ_UPB_AAS_LIST, intsToParcel, parcelToStrings},
{RIL_REQUEST_SET_PHONEBOOK_READY, intsToParcel, parcelToVoid},
// PHB END
{RIL_REQUEST_GET_FEMTOCELL_LIST, voidToParcel, parcelToStrings},
{RIL_REQUEST_ABORT_FEMTOCELL_LIST, voidToParcel, parcelToVoid},
{RIL_REQUEST_SELECT_FEMTOCELL, stringsToParcel, parcelToVoid},
{RIL_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE, voidToParcel, parcelToInts},
{RIL_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE, intsToParcel, parcelToVoid},
{RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD, intsToParcel, parcelToVoid},
{RIL_REQUEST_RESET_MD_DATA_RETRY_COUNT, stringToParcel, parcelToVoid},
// M: Data Framework - CC 33
{RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE, intsToParcel, parcelToVoid},
// MTK-START: SIM GBA
{RIL_REQUEST_GENERAL_SIM_AUTH, simAuthToParcel, parcelToSIM_IO},
// MTK-END
// M: [LTE][Low Power][UL traffic shaping] @{
{RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT, intsToParcel, parcelToVoid},
{RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER, intsToParcel, parcelToVoid},
// M: [LTE][Low Power][UL traffic shaping] @}
// MTK-START: SIM ME LOCK
{RIL_REQUEST_QUERY_SIM_NETWORK_LOCK, intsToParcel, parcelToInts},
{RIL_REQUEST_SET_SIM_NETWORK_LOCK, stringsToParcel, parcelToVoid},
// MTK-END
{RIL_REQUEST_SET_IMS_ENABLE, intsToParcel, parcelToVoid},
{RIL_REQUEST_SET_VOLTE_ENABLE, intsToParcel, parcelToVoid},
{RIL_REQUEST_SET_WFC_ENABLE, intsToParcel, parcelToVoid},
{RIL_REQUEST_SET_VILTE_ENABLE, intsToParcel, parcelToVoid},
{RIL_REQUEST_SET_VIWIFI_ENABLE, intsToParcel, parcelToVoid},
{RIL_REQUEST_SET_IMS_VIDEO_ENABLE, intsToParcel, parcelToVoid},
{RIL_REQUEST_VIDEO_CALL_ACCEPT, intsToParcel, parcelToVoid},
{RIL_REQUEST_ECC_REDIAL_APPROVE, intsToParcel, parcelToVoid},
{RIL_REQUEST_SET_IMSCFG, intsToParcel, parcelToVoid},
{RIL_REQUEST_SET_MD_IMSCFG, stringsToParcel, parcelToString},
{RIL_REQUEST_GET_PROVISION_VALUE, stringToParcel, parcelToStrings},
{RIL_REQUEST_SET_PROVISION_VALUE, stringsToParcel, parcelToVoid},
{RIL_REQUEST_IMS_BEARER_STATE_CONFIRM, intsToParcel, parcelToVoid},
{RIL_REQUEST_SET_IMS_BEARER_NOTIFICATION, stringsToParcel, parcelToVoid},
{RIL_REQUEST_IMS_DEREG_NOTIFICATION, intsToParcel, parcelToVoid},
{RIL_REQUEST_IMS_ECT, stringsToParcel, parcelToVoid},
{RIL_REQUEST_HOLD_CALL, intsToParcel, parcelToVoid},
{RIL_REQUEST_RESUME_CALL, intsToParcel, parcelToVoid},
{RIL_REQUEST_DIAL_WITH_SIP_URI, stringToParcel, parcelToVoid},
{RIL_REQUEST_FORCE_RELEASE_CALL, intsToParcel, parcelToVoid},
{RIL_REQUEST_SET_IMS_RTP_REPORT, intsToParcel, parcelToVoid},
{RIL_REQUEST_CONFERENCE_DIAL, stringsToParcel, parcelToVoid},
{RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER, stringsToParcel, parcelToVoid},
{RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER, stringsToParcel, parcelToVoid},
{RIL_REQUEST_SET_ECC_MODE, stringsToParcel, parcelToVoid}, // Verizon E911
{RIL_REQUEST_VT_DIAL_WITH_SIP_URI, stringToParcel, parcelToVoid},
{RIL_REQUEST_SEND_USSI, stringToParcel, parcelToVoid},
{RIL_REQUEST_CANCEL_USSI, voidToParcel, parcelToVoid},
{RIL_REQUEST_SET_WFC_PROFILE, intsToParcel, parcelToVoid},
{RIL_REQUEST_VSS_ANTENNA_CONF, intsToParcel, parcelToInts}, // Antenna Testing
{RIL_REQUEST_VSS_ANTENNA_INFO, intsToParcel, parcelToInts}, // Antenna Testing
{RIL_REQUEST_GET_POL_CAPABILITY, voidToParcel, parcelToInts},
{RIL_REQUEST_GET_POL_LIST, voidToParcel, parcelToStrings},
{RIL_REQUEST_SET_POL_ENTRY, stringsToParcel, parcelToVoid},
{RIL_REQUEST_SET_IMS_REGISTRATION_REPORT, voidToParcel, parcelToVoid},
/// M: [Network][C2K] Sprint roaming control @{
{RIL_REQUEST_SET_ROAMING_ENABLE, intsToParcel, parcelToVoid},
{RIL_REQUEST_GET_ROAMING_ENABLE, intsToParcel, parcelToInts},
/// @}
// Verizon IMS Call pull
{RIL_REQUEST_PULL_CALL, stringsToParcel, parcelToVoid},
// [IMS] Dial
{RIL_REQUEST_IMS_DIAL, dialToParcel, parcelToVoid},
// [IMS] VT Dial
{RIL_REQUEST_IMS_VT_DIAL, dialToParcel, parcelToVoid},
// [IMS] Emergency Dial
{RIL_REQUEST_IMS_EMERGENCY_DIAL, emergencyDialToParcel, parcelToVoid},
{RIL_REQUEST_SET_IMS_REGISTRATION_REPORT, voidToParcel, parcelToVoid},
// External SIM [START]
{RIL_REQUEST_VSIM_NOTIFICATION, vsimEventToParcel, parcelToInts},
{RIL_REQUEST_VSIM_OPERATION, vsimOperationEventToParcel, parcelToInts},
// External SIM [END]
{RIL_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION, voidToParcel, parcelToInts},
{RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE, intsToParcel, parcelToVoid},

// M: E911
{RIL_REQUEST_SET_E911_STATE, intsToParcel, parcelToVoid},
// [SIM] SubsidyLock
{RIL_REQUEST_GET_SUBLOCK_MODEM_STATUS, rawToParcel, parcelToRaw},
{RIL_REQUEST_UPDATE_SUBLOCK_SETTINGS, rawToParcel, parcelToRaw},
{RIL_REQUEST_IMS_SEND_SMS_EX, imsSmsToParcel, parcelToSMS},
//SMS
{RIL_REQUEST_SET_SMS_FWK_READY, voidToParcel, parcelToVoid},
{RIL_REQUEST_SMS_ACKNOWLEDGE_EX, intsToParcel, parcelToVoid},
{RIL_REQUEST_SETPROP_IMS_HANDOVER, stringsToParcel, parcelToVoid},
// SIM SLOT LOCK
{RIL_REQUEST_ENTER_DEVICE_NETWORK_DEPERSONALIZATION, stringsToParcel, parcelToInts},
{RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX, cdmaSmsAckToParcel, parcelToVoid},
{RIL_REQUEST_QUERY_VOPS_STATUS, voidToParcel, parcelToInts},
{RIL_REQUEST_RESTART_RILD, voidToParcel, parcelToVoid},


/*
*  Copyright (C) 2014 MediaTek Inc.
*
*  Modification based on code covered by the below mentioned copyright
*  and/or permission notice(s).
*/


/*
**
** Copyright 2014, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
    {RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, parcelToVoid},
    {RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, parcelToVoid},
    {RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, parcelToStrings},
    {RIL_UNSOL_RESPONSE_NEW_SMS, parcelToString},
    {RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT, parcelToString},
    {RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM, parcelToInts},
    {RIL_UNSOL_ON_USSD, parcelToStrings},
    {RIL_UNSOL_ON_USSD_REQUEST, parcelToVoid},
    {RIL_UNSOL_NITZ_TIME_RECEIVED, parcelToString},
    {RIL_UNSOL_SIGNAL_STRENGTH, parcelToInts},
    {RIL_UNSOL_DATA_CALL_LIST_CHANGED, parcelToDataCallList},
    {RIL_UNSOL_SUPP_SVC_NOTIFICATION, parcelToSsn},
    {RIL_UNSOL_STK_SESSION_END, parcelToVoid},
    {RIL_UNSOL_STK_PROACTIVE_COMMAND, parcelToString},
    {RIL_UNSOL_STK_EVENT_NOTIFY, parcelToString},
    {RIL_UNSOL_STK_CALL_SETUP, parcelToInts},
    {RIL_UNSOL_SIM_SMS_STORAGE_FULL, parcelToVoid},
    {RIL_UNSOL_SIM_REFRESH, parcelToSimRefresh},
    {RIL_UNSOL_CALL_RING, parcelToCallRing},
    {RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, parcelToVoid},
    {RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, parcelToCdmaSms},
    {RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS, parcelToRaw},
    {RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL, parcelToVoid},
    {RIL_UNSOL_RESTRICTED_STATE_CHANGED, parcelToInts},
    {RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE, parcelToVoid},
    {RIL_UNSOL_CDMA_CALL_WAITING, parcelToCdmaCallWaiting},
    {RIL_UNSOL_CDMA_OTA_PROVISION_STATUS, parcelToInts},
    {RIL_UNSOL_CDMA_INFO_REC, parcelToCdmaInformationRecords},
    {RIL_UNSOL_OEM_HOOK_RAW, parcelToRaw},
    {RIL_UNSOL_RINGBACK_TONE, parcelToInts},
    {RIL_UNSOL_RESEND_INCALL_MUTE, parcelToVoid},
    {RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED, parcelToInts},
    {RIL_UNSOL_CDMA_PRL_CHANGED, parcelToInts},
    {RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE, parcelToVoid},
    {RIL_UNSOL_RIL_CONNECTED, parcelToInts},
    {RIL_UNSOL_VOICE_RADIO_TECH_CHANGED, parcelToInts},
    {RIL_UNSOL_CELL_INFO_LIST, parcelToCellInfoList},
    {RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED, parcelToVoid},
    {RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED, parcelToInts},
    {RIL_UNSOL_SRVCC_STATE_NOTIFY, parcelToInts},
    {RIL_UNSOL_HARDWARE_CONFIG_CHANGED, parcelToHardwareConfig},
    {RIL_UNSOL_DC_RT_INFO_CHANGED, parcelToDcRtInfo},
    {RIL_UNSOL_RADIO_CAPABILITY, parcelToRadioCapability},
    {RIL_UNSOL_ON_SS, parcelToSSData},
    {RIL_UNSOL_STK_CC_ALPHA_NOTIFY, parcelToStrings},
    {RIL_UNSOL_LCEDATA_RECV, parcelToLceData},
    {RIL_UNSOL_PCO_DATA, parcelToPcoData},
    {RIL_UNSOL_MODEM_RESTART, parcelToString},
    {RIL_UNSOL_NETWORK_SCAN_RESULT, parcelToNetworkScanResult},
    {RIL_UNSOL_EMERGENCY_NUMBER_LIST, parcelToEmergencyNumberList},
    {RIL_UNSOL_KEEPALIVE_STATUS, parcelToKeepaliveStatus},

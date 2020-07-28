/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.ims.rcsua.service;

/**
 * The Class RcsEventSettingTLV
 */
public class RcsEventSettingTLV {
    // Copy from ims_interface_ap.h + volte_event.h
    /* ------------------------------------------------ */
    /*  System                                          */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_System_Start_Code = 0;
    public static final int VoLTE_Setting_System_Operator_ID            = VoLTE_Setting_System_Start_Code + 1;                  ///< data type int
    public static final int VoLTE_Setting_System_GENERAL_SETTING        = VoLTE_Setting_System_Start_Code + 2;
    public static final int VoLTE_Setting_System_Test_Sim_Status        = VoLTE_Setting_System_Start_Code + 3;
    /* ------------------------------------------------ */
    /*  Network Information                             */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_Network_Information_Start_Code = 1000;
    public static final int VoLTE_Setting_Net_Local_Address             = VoLTE_Setting_Network_Information_Start_Code + 1;     ///< data type is string
    public static final int VoLTE_Setting_Net_Local_Port                = VoLTE_Setting_Network_Information_Start_Code + 2;     ///< data type int
    public static final int VoLTE_Setting_Net_Local_Protocol_Type       = VoLTE_Setting_Network_Information_Start_Code + 3;     ///< UDP: 0; TCP: 1
    public static final int VoLTE_Setting_Net_Local_Protocol_Version    = VoLTE_Setting_Network_Information_Start_Code + 4;     ///< IPv4: 1; IPv6: 2
    public static final int VoLTE_Setting_Net_Local_IPSec_Port_Start    = VoLTE_Setting_Network_Information_Start_Code + 5;
    public static final int VoLTE_Setting_Net_Local_IPSec_Port_Range    = VoLTE_Setting_Network_Information_Start_Code + 6;
    public static final int VoLTE_Setting_Net_Local_RTP_RTCP_Port_Start = VoLTE_Setting_Network_Information_Start_Code + 7;
    public static final int VoLTE_Setting_Net_Local_RTP_RTCP_Port_Range = VoLTE_Setting_Network_Information_Start_Code + 8;
    public static final int VoLTE_Setting_Net_IPSec                     = VoLTE_Setting_Network_Information_Start_Code + 9;     ///< Disable: 0; Enable: 1
    public static final int VoLTE_Setting_Net_Cell_ID                   = VoLTE_Setting_Network_Information_Start_Code + 10;    ///< data type is string
    public static final int VoLTE_Setting_Net_RTP_DSCP                  = VoLTE_Setting_Network_Information_Start_Code + 11;    /// For build pass; need to remove
    public static final int VoLTE_Setting_Net_IF_Name                   = VoLTE_Setting_Network_Information_Start_Code + 12;    ///< data type is string
    public static final int VoLTE_Setting_Net_Network_Id                = VoLTE_Setting_Network_Information_Start_Code + 13;    ///< data type is integer
    public static final int VoLTE_Setting_Net_SIP_Dscp                  = VoLTE_Setting_Network_Information_Start_Code + 14;    ///< data type is integer
    public static final int VoLTE_Setting_Net_RTP_Voice_Dscp            = VoLTE_Setting_Network_Information_Start_Code + 15;    ///< data type is integer
    public static final int VoLTE_Setting_Net_RTP_Video_Dscp            = VoLTE_Setting_Network_Information_Start_Code + 16;    ///< data type is integer
    public static final int VoLTE_Setting_Net_SIP_Soc_Priority          = VoLTE_Setting_Network_Information_Start_Code + 17;    ///< data type is integer
    public static final int VoLTE_Setting_Net_SIP_Soc_Tcp_Mss           = VoLTE_Setting_Network_Information_Start_Code + 18;    ///< data type is integer
    public static final int VoLTE_Setting_Net_RTP_Voice_Soc_Priority    = VoLTE_Setting_Network_Information_Start_Code + 19;    ///< data type is integer
    public static final int VoLTE_Setting_Net_RTP_Video_Soc_Priority    = VoLTE_Setting_Network_Information_Start_Code + 20;    ///< data type is integer
    public static final int VoLTE_Setting_Net_LBS_Location_Info         = VoLTE_Setting_Network_Information_Start_Code + 21;    ///< data type is integer
    public static final int VoLTE_Setting_Net_Security                  = VoLTE_Setting_Network_Information_Start_Code + 22;
    public static final int VoLTE_Setting_Net_Authentication            = VoLTE_Setting_Network_Information_Start_Code + 23;
    public static final int VoLTE_Setting_Net_PCSCF_Port                = VoLTE_Setting_Network_Information_Start_Code + 24;
    public static final int VoLTE_Setting_Net_Emergency_AID             = VoLTE_Setting_Network_Information_Start_Code + 25;
    public static final int VoLTE_Setting_Net_PCSCF_Number              = VoLTE_Setting_Network_Information_Start_Code + 26;
    public static final int VoLTE_Setting_Net_Access_Type               = VoLTE_Setting_Network_Information_Start_Code + 27;
    /* ------------------------------------------------ */
    /*  Account Settings                                */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_Account_Settings_Start_Code = 2000;
    public static final int VoLTE_Setting_Account_Public_UID        = VoLTE_Setting_Account_Settings_Start_Code + 1;        ///< data type is string
    public static final int VoLTE_Setting_Account_Private_UID       = VoLTE_Setting_Account_Settings_Start_Code + 2;        ///< data type is string
    public static final int VoLTE_Setting_Account_Home_URI          = VoLTE_Setting_Account_Settings_Start_Code + 3;        ///< data type is string
    public static final int VoLTE_Setting_Account_IMEI              = VoLTE_Setting_Account_Settings_Start_Code + 4;        ///< data type is string
    public static final int VoLTE_Setting_Account_ConfFactoryURI    = VoLTE_Setting_Account_Settings_Start_Code + 5;
    public static final int VoLTE_Setting_Account_IMSI_MNC          = VoLTE_Setting_Account_Settings_Start_Code + 6;
    public static final int VoLTE_Setting_Account_IMSI_MCC          = VoLTE_Setting_Account_Settings_Start_Code + 7;
    public static final int VoLTE_Setting_Account_MSISDN            = VoLTE_Setting_Account_Settings_Start_Code + 8;

    /* ------------------------------------------------ */
    /*  Server Settings                                 */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_Server_Settings_Start_Code = 3000;
    public static final int VoLTE_Setting_Server_PCSCF_List         = VoLTE_Setting_Server_Settings_Start_Code + 1;         ///< data type is string

    /* ------------------------------------------------ */
    /*  VoLTE Service Settings                          */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_VoLTE_Service_Start_Code  = 10000;

    /* ------------------------------------------------ */
    /*  VoLTE Call Settings                             */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_VoLTE_Call_Start_Code     = 11000;
    public static final int VoLTE_Setting_VoLTE_Call_Session_Flag   = VoLTE_Setting_VoLTE_Call_Start_Code + 1;
    public static final int VoLTE_Setting_VoLTE_Call_Session_Timer  = VoLTE_Setting_VoLTE_Call_Start_Code + 2;              ///< in seconds
    public static final int VoLTE_Setting_VoLTE_Call_Session_MinSE  = VoLTE_Setting_VoLTE_Call_Start_Code + 3;              ///< in seconds
    public static final int VoLTE_Setting_VoLTE_Call_Privacy        = VoLTE_Setting_VoLTE_Call_Start_Code + 4;
    public static final int VoLTE_Setting_VoLTE_Call_Capability     = VoLTE_Setting_VoLTE_Call_Start_Code + 5;              ///< data type is ::VoLTE_Call_Capability_Type_e
    public static final int VoLTE_Setting_VoLTE_Call_RTCP_Interval  = VoLTE_Setting_VoLTE_Call_Start_Code + 6;              ///< in ms
    public static final int VoLTE_Setting_VoLTE_Early_Media         = VoLTE_Setting_VoLTE_Call_Start_Code + 7;              ///< If the incoming call support Early Media; shall we require it to play?
    public static final int VoLTE_Setting_VoLTE_Call_CodecOrder1    = VoLTE_Setting_VoLTE_Call_Start_Code + 8;              ///< voip_codec_enum
    public static final int VoLTE_Setting_VoLTE_Call_CodecOrder2    = VoLTE_Setting_VoLTE_Call_Start_Code + 9;              ///< voip_codec_enum
    public static final int VoLTE_Setting_VoLTE_Call_CodecOrder3    = VoLTE_Setting_VoLTE_Call_Start_Code + 10;              ///< voip_codec_enum
    public static final int VoLTE_Setting_VoLTE_Call_def_max_ptime  = VoLTE_Setting_VoLTE_Call_Start_Code + 11;
    public static final int VoLTE_Setting_VoLTE_Call_def_ptime      = VoLTE_Setting_VoLTE_Call_Start_Code + 12;
    public static final int VoLTE_Setting_VoLTE_Call_G711_ptime     = VoLTE_Setting_VoLTE_Call_Start_Code + 13;
    public static final int VoLTE_Setting_VoLTE_Call_G726_ptime     = VoLTE_Setting_VoLTE_Call_Start_Code + 14;
    public static final int VoLTE_Setting_VoLTE_Call_G729_ptime     = VoLTE_Setting_VoLTE_Call_Start_Code + 15;
    public static final int VoLTE_Setting_VoLTE_Call_amr_pt         = VoLTE_Setting_VoLTE_Call_Start_Code + 16;
    public static final int VoLTE_Setting_VoLTE_Call_amr_wb_pt      = VoLTE_Setting_VoLTE_Call_Start_Code + 17;
    public static final int VoLTE_Setting_VoLTE_Call_h264_pt        = VoLTE_Setting_VoLTE_Call_Start_Code + 18;
    public static final int VoLTE_Setting_VoLTE_Call_G729_annexB    = VoLTE_Setting_VoLTE_Call_Start_Code + 19;
    public static final int VoLTE_Setting_VoLTE_Call_TelEvt         = VoLTE_Setting_VoLTE_Call_Start_Code + 20;
    public static final int VoLTE_Setting_VoLTE_Call_TelEvt_pt      = VoLTE_Setting_VoLTE_Call_Start_Code + 21;
    public static final int VoLTE_Setting_VoLTE_Call_Priority       = VoLTE_Setting_VoLTE_Call_Start_Code + 22;
    public static final int VoLTE_Setting_VoLTE_Call_UserAgent      = VoLTE_Setting_VoLTE_Call_Start_Code + 23;             ///< VOLTE_MAX_ADDRESS_LENGTH
    public static final int VoLTE_Setting_VoLTE_Call_RejCode        = VoLTE_Setting_VoLTE_Call_Start_Code + 24;
    public static final int VoLTE_Setting_VoLTE_Call_noResourceCode = VoLTE_Setting_VoLTE_Call_Start_Code + 25;
    public static final int VoLTE_Setting_VoLTE_Call_RejMediaCode   = VoLTE_Setting_VoLTE_Call_Start_Code + 26;
    public static final int VoLTE_Setting_VoLTE_Call_RejbyUserCode  = VoLTE_Setting_VoLTE_Call_Start_Code + 27;
    public static final int VoLTE_Setting_VoLTE_Call_Conf_SubTimer  = VoLTE_Setting_VoLTE_Call_Start_Code + 28;
    public static final int VoLTE_Setting_VoLTE_Call_amr_mode_set   = VoLTE_Setting_VoLTE_Call_Start_Code + 29;
    public static final int VoLTE_Setting_VoLTE_Call_amr_wb_mode_set   = VoLTE_Setting_VoLTE_Call_Start_Code + 30;
    public static final int VoLTE_Setting_VoLTE_Call_amr_fmt_variant   = VoLTE_Setting_VoLTE_Call_Start_Code + 31;
    public static final int VoLTE_Setting_VoLTE_Call_precondition   = VoLTE_Setting_VoLTE_Call_Start_Code + 32;
    public static final int VoLTE_Setting_VoLTE_Call_MO_Invite_To_BWCNF_Time = VoLTE_Setting_VoLTE_Call_Start_Code + 33;
    public static final int VoLTE_Setting_VoLTE_Call_h264_pt2       = VoLTE_Setting_VoLTE_Call_Start_Code + 34;
    public static final int VoLTE_Setting_VoLTE_Call_h265_pt        = VoLTE_Setting_VoLTE_Call_Start_Code + 35;
    public static final int VoLTE_Setting_VoLTE_Call_h265_pt2       = VoLTE_Setting_VoLTE_Call_Start_Code + 36;
    public static final int VoLTE_Setting_VoLTE_Call_amr_oct_pt     = VoLTE_Setting_VoLTE_Call_Start_Code + 37;
    public static final int VoLTE_Setting_VoLTE_Call_amr_open_pt    = VoLTE_Setting_VoLTE_Call_Start_Code + 38;
    public static final int VoLTE_Setting_VoLTE_Call_amr_wb_oct_pt  = VoLTE_Setting_VoLTE_Call_Start_Code + 39;
    public static final int VoLTE_Setting_VoLTE_Call_TelEvt_wb_pt   = VoLTE_Setting_VoLTE_Call_Start_Code + 40;
    public static final int VoLTE_Setting_VoLTE_Call_CONTACT_WITH_USERNAME_INCALL          = VoLTE_Setting_VoLTE_Call_Start_Code + 41;
    public static final int VoLTE_Setting_VoLTE_Call_SENDRECV_BEFORE_PRCD                  = VoLTE_Setting_VoLTE_Call_Start_Code + 42;
    public static final int VoLTE_Setting_VoLTE_Call_ALWAYS_USE_UPDATE_FOR_PRCD            = VoLTE_Setting_VoLTE_Call_Start_Code + 43;
    public static final int VoLTE_Setting_VoLTE_Call_MERGE_NEED_SWAP                       = VoLTE_Setting_VoLTE_Call_Start_Code + 44;
    public static final int VoLTE_Setting_VoLTE_Call_ALWAYS_USE_SIP_URI_FOR_MO_CALL        = VoLTE_Setting_VoLTE_Call_Start_Code + 45;
    public static final int VoLTE_Setting_VoLTE_Call_SET_STRENGTH_MANDATORY                = VoLTE_Setting_VoLTE_Call_Start_Code + 46;
    public static final int VoLTE_Setting_VoLTE_Call_SEND_REFER_TO_PEER                    = VoLTE_Setting_VoLTE_Call_Start_Code + 47;
    public static final int VoLTE_Setting_VoLTE_Call_ADD_3GPP_IMS_IN_ACCEPT                = VoLTE_Setting_VoLTE_Call_Start_Code + 48;
    public static final int VoLTE_Setting_VoLTE_Call_TERMINAL_BASED_CW                     = VoLTE_Setting_VoLTE_Call_Start_Code + 49;
    public static final int VoLTE_Setting_VoLTE_Call_REFER_DIALOG_TO_SERVER                = VoLTE_Setting_VoLTE_Call_Start_Code + 50;
    public static final int VoLTE_Setting_VoLTE_Call_CHECK_REASON_PHRASE                   = VoLTE_Setting_VoLTE_Call_Start_Code + 51;
    public static final int VoLTE_Setting_VoLTE_Call_MERGE_SEND_BYE                        = VoLTE_Setting_VoLTE_Call_Start_Code + 52;
    public static final int VoLTE_Setting_VoLTE_Call_WHEN_STOP_TCALL                       = VoLTE_Setting_VoLTE_Call_Start_Code + 53;
    public static final int VoLTE_Setting_VoLTE_Call_SET_RTCP_0                            = VoLTE_Setting_VoLTE_Call_Start_Code + 54;
    public static final int VoLTE_Setting_VoLTE_Call_CHECK_CONTACT_380                     = VoLTE_Setting_VoLTE_Call_Start_Code + 55;
    public static final int VoLTE_Setting_VoLTE_Call_CONF_PARTICIPANT_NOT_SUBSCRIBE        = VoLTE_Setting_VoLTE_Call_Start_Code + 56;
    public static final int VoLTE_Setting_VoLTE_Call_ADD_SDP_IN_180_FOR_NON_PRECOND        = VoLTE_Setting_VoLTE_Call_Start_Code + 57;
    public static final int VoLTE_Setting_VoLTE_Call_ADD_PRECONDITION_IN_HOLD_UNHOLD_SDP   = VoLTE_Setting_VoLTE_Call_Start_Code + 58;
    public static final int VoLTE_Setting_VoLTE_Call_USE_ORG_SDP_FOR_INVITE_WITHOUT_SDP    = VoLTE_Setting_VoLTE_Call_Start_Code + 59;
    public static final int VoLTE_Setting_VoLTE_Call_OIR_PRIVACY_TO_HEADER                 = VoLTE_Setting_VoLTE_Call_Start_Code + 60;
    public static final int VoLTE_Setting_VoLTE_Call_REGISTER_CAP_WHENEVER                 = VoLTE_Setting_VoLTE_Call_Start_Code + 61;
    public static final int VoLTE_Setting_VoLTE_Call_NO_AUTO_RETRY_FOR_MO_CALL             = VoLTE_Setting_VoLTE_Call_Start_Code + 62;
    public static final int VoLTE_Setting_VoLTE_Call_USE_EPS_PREFIX_IN_PHONE_CONTEXT       = VoLTE_Setting_VoLTE_Call_Start_Code + 63;
    public static final int VoLTE_Setting_VoLTE_Call_FOLLOW_NW_IMS_VOPS_SUPPORT            = VoLTE_Setting_VoLTE_Call_Start_Code + 64;
    public static final int VoLTE_Setting_VoLTE_Call_ATTEMPT_REG_WHEN_RECEIVE403           = VoLTE_Setting_VoLTE_Call_Start_Code + 65;
    public static final int VoLTE_Setting_VoLTE_Call_WAITING_487_FOR_CANCEL_TIMER_TIMEOUT  = VoLTE_Setting_VoLTE_Call_Start_Code + 66;
    public static final int VoLTE_Setting_VoLTE_Call_BCSFB_FEATURE_ENABLE                  = VoLTE_Setting_VoLTE_Call_Start_Code + 67;
    public static final int VoLTE_Setting_VoLTE_Call_SEND_183_WHEN_PRCD_NONE               = VoLTE_Setting_VoLTE_Call_Start_Code + 68;
    public static final int VoLTE_Setting_VoLTE_Call_CALL_ID_WITH_HOST_INCALL              = VoLTE_Setting_VoLTE_Call_Start_Code + 69;
    public static final int VoLTE_Setting_VoLTE_Call_ADD_RTCP_PORT_INTO_SDP                = VoLTE_Setting_VoLTE_Call_Start_Code + 70;
    public static final int VoLTE_Setting_VoLTE_Call_ECC_SESSION_TIMER_ENABLE              = VoLTE_Setting_VoLTE_Call_Start_Code + 71;
    public static final int VoLTE_Setting_VoLTE_Call_SUBSCRIBE_DIALOG_TO_SERVER            = VoLTE_Setting_VoLTE_Call_Start_Code + 72;
    public static final int VoLTE_Setting_VoLTE_Call_CHECK_CONFERENCE_SUBSCRIBE            = VoLTE_Setting_VoLTE_Call_Start_Code + 73;
    public static final int VoLTE_Setting_VoLTE_Call_CONFERENCE_SUBSCRIBE_TIMING           = VoLTE_Setting_VoLTE_Call_Start_Code + 74;
    public static final int VoLTE_Setting_VoLTE_Call_GENERAL_AMR_NB_WB                     = VoLTE_Setting_VoLTE_Call_Start_Code + 75;
    public static final int VoLTE_Setting_VoLTE_Call_DISABLE_CONF_CALL_PRCD                = VoLTE_Setting_VoLTE_Call_Start_Code + 76;
    public static final int VoLTE_Setting_VoLTE_Call_WAIT_CANCEL_IN_SRVCC_END              = VoLTE_Setting_VoLTE_Call_Start_Code + 77;
    public static final int VoLTE_Setting_VoLTE_Call_REJECT_1W_IF_1A1H                     = VoLTE_Setting_VoLTE_Call_Start_Code + 78;
    public static final int VoLTE_Setting_VoLTE_Call_NO_AUTO_RETRY_FOR_DCM_MO_CALL         = VoLTE_Setting_VoLTE_Call_Start_Code + 79;
    public static final int VoLTE_Setting_VoLTE_Call_JOIN_SEND_BYE                         = VoLTE_Setting_VoLTE_Call_Start_Code + 80;
    public static final int VoLTE_Setting_VoLTE_Call_SET_ZERO_CELL_ID_IN_WIFI              = VoLTE_Setting_VoLTE_Call_Start_Code + 81;
    public static final int VoLTE_Setting_VoLTE_Call_REJECT_MTCALL_BY_VOPS                 = VoLTE_Setting_VoLTE_Call_Start_Code + 82;
    public static final int VoLTE_Setting_VoLTE_Call_REJECT_MTCALL_WHEN_VILTE              = VoLTE_Setting_VoLTE_Call_Start_Code + 83;
    public static final int VoLTE_Setting_VoLTE_Call_NOT_ADD_SDP_IN_OPTIONS                = VoLTE_Setting_VoLTE_Call_Start_Code + 84;
    public static final int VoLTE_Setting_VoLTE_Call_PCSCF_ERROR_WHEN663                   = VoLTE_Setting_VoLTE_Call_Start_Code + 85;
    public static final int VoLTE_Setting_VoLTE_Call_REL_CONF_IF_NO_PARTICIPANT            = VoLTE_Setting_VoLTE_Call_Start_Code + 86;
    public static final int VoLTE_Setting_VoLTE_Call_ADD_NO_FORK                           = VoLTE_Setting_VoLTE_Call_Start_Code + 87;
    public static final int VoLTE_Setting_VoLTE_Call_TMO_SPECIFIC_SDP                      = VoLTE_Setting_VoLTE_Call_Start_Code + 88;
    public static final int VoLTE_Setting_VoLTE_Call_CALL_RINGING_TIMER_TIMEOUT            = VoLTE_Setting_VoLTE_Call_Start_Code + 89;
    public static final int VoLTE_Setting_VoLTE_Call_CALL_RINGBACK_TIMER_TIMEOUT           = VoLTE_Setting_VoLTE_Call_Start_Code + 90;
    public static final int VoLTE_Setting_VoLTE_Call_CALL_TCALL_TIMER_TIMEOUT              = VoLTE_Setting_VoLTE_Call_Start_Code + 91;
    public static final int VoLTE_Setting_VoLTE_Call_REJECT_CODE_IN23G                     = VoLTE_Setting_VoLTE_Call_Start_Code + 92;
    public static final int VoLTE_Setting_VoLTE_Call_CALL_REFRESHCALL_TIMER_EXPIRE         = VoLTE_Setting_VoLTE_Call_Start_Code + 93;
    public static final int VoLTE_Setting_VoLTE_Call_CALL_CALLEND_TIMER_TIMEOUT            = VoLTE_Setting_VoLTE_Call_Start_Code + 94;
    public static final int VoLTE_Setting_VoLTE_Call_WAITING_MT_CNF_TIMER_TIMEOUT          = VoLTE_Setting_VoLTE_Call_Start_Code + 95;
    public static final int VoLTE_Setting_VoLTE_Call_CONF_SUB_TIMER_TIMEOUT                = VoLTE_Setting_VoLTE_Call_Start_Code + 96;
    public static final int VoLTE_Setting_VoLTE_Call_MO_INVITE_TO_BW_CNF_TIMER_TIMEOUT     = VoLTE_Setting_VoLTE_Call_Start_Code + 97;
    public static final int VoLTE_Setting_VoLTE_Call_SRVCC_CAPABILITY                      = VoLTE_Setting_VoLTE_Call_Start_Code + 98;
    public static final int VoLTE_Setting_VoLTE_Call_EVS_SUPPORT                           = VoLTE_Setting_VoLTE_Call_Start_Code + 99;
    public static final int VoLTE_Setting_VoLTE_Call_EVS_MODE_SWITCH                       = VoLTE_Setting_VoLTE_Call_Start_Code + 100;
    public static final int VoLTE_Setting_VoLTE_Call_HF_ONLY                               = VoLTE_Setting_VoLTE_Call_Start_Code + 101;
    public static final int VoLTE_Setting_VoLTE_Call_DTX                                   = VoLTE_Setting_VoLTE_Call_Start_Code + 102;
    public static final int VoLTE_Setting_VoLTE_Call_DTX_RECV                              = VoLTE_Setting_VoLTE_Call_Start_Code + 103;
    public static final int VoLTE_Setting_VoLTE_Call_BR_BEGIN                              = VoLTE_Setting_VoLTE_Call_Start_Code + 104;
    public static final int VoLTE_Setting_VoLTE_Call_BR_END                                = VoLTE_Setting_VoLTE_Call_Start_Code + 105;
    public static final int VoLTE_Setting_VoLTE_Call_BR_SEND_BEGIN                         = VoLTE_Setting_VoLTE_Call_Start_Code + 106;
    public static final int VoLTE_Setting_VoLTE_Call_BR_SEND_END                           = VoLTE_Setting_VoLTE_Call_Start_Code + 107;
    public static final int VoLTE_Setting_VoLTE_Call_BR_RECV_BEGIN                         = VoLTE_Setting_VoLTE_Call_Start_Code + 108;
    public static final int VoLTE_Setting_VoLTE_Call_BR_RECV_END                           = VoLTE_Setting_VoLTE_Call_Start_Code + 109;
    public static final int VoLTE_Setting_VoLTE_Call_BW_BEGIN                              = VoLTE_Setting_VoLTE_Call_Start_Code + 110;
    public static final int VoLTE_Setting_VoLTE_Call_BW_END                                = VoLTE_Setting_VoLTE_Call_Start_Code + 111;
    public static final int VoLTE_Setting_VoLTE_Call_BW_SEND_BEGIN                         = VoLTE_Setting_VoLTE_Call_Start_Code + 112;
    public static final int VoLTE_Setting_VoLTE_Call_BW_SEND_END                           = VoLTE_Setting_VoLTE_Call_Start_Code + 113;
    public static final int VoLTE_Setting_VoLTE_Call_BW_RECV_BEGIN                         = VoLTE_Setting_VoLTE_Call_Start_Code + 114;
    public static final int VoLTE_Setting_VoLTE_Call_BW_RECV_END                           = VoLTE_Setting_VoLTE_Call_Start_Code + 115;
    public static final int VoLTE_Setting_VoLTE_Call_CH_SEND                               = VoLTE_Setting_VoLTE_Call_Start_Code + 116;
    public static final int VoLTE_Setting_VoLTE_Call_CH_RECV                               = VoLTE_Setting_VoLTE_Call_Start_Code + 117;
    public static final int VoLTE_Setting_VoLTE_Call_CH_AW_RECV                            = VoLTE_Setting_VoLTE_Call_Start_Code + 118;
    public static final int VoLTE_Setting_VoLTE_Call_CMR                                   = VoLTE_Setting_VoLTE_Call_Start_Code + 119;
    public static final int VoLTE_Setting_Call_evs_pt                          = VoLTE_Setting_VoLTE_Call_Start_Code + 120;
    public static final int VoLTE_Setting_Call_short_code_support              = VoLTE_Setting_VoLTE_Call_Start_Code + 121;
    public static final int VoLTE_Setting_Call_early_media_when_rtp_coming     = VoLTE_Setting_VoLTE_Call_Start_Code + 122;
    public static final int VoLTE_Setting_Call_session_refresher_in_req        = VoLTE_Setting_VoLTE_Call_Start_Code + 123;
    public static final int VoLTE_Setting_Call_session_refresher_in_resp       = VoLTE_Setting_VoLTE_Call_Start_Code + 124;
    public static final int VoLTE_Setting_Call_wfc_emerg_pidf_country          = VoLTE_Setting_VoLTE_Call_Start_Code + 125;
    public static final int VoLTE_Setting_Call_set_des_strength_optional       = VoLTE_Setting_VoLTE_Call_Start_Code + 126;
    public static final int VoLTE_Setting_Call_conf_user_count_includes_host   = VoLTE_Setting_VoLTE_Call_Start_Code + 127;
    public static final int VoLTE_Setting_Call_ecn_init_method                 = VoLTE_Setting_VoLTE_Call_Start_Code + 128;
    public static final int VoLTE_Setting_Call_histinfo_in_supported           = VoLTE_Setting_VoLTE_Call_Start_Code + 129;
    public static final int VoLTE_Setting_Call_delay_unhold_call               = VoLTE_Setting_VoLTE_Call_Start_Code + 130;
    public static final int VoLTE_Setting_Call_use_pub_uid_in_ref_to           = VoLTE_Setting_VoLTE_Call_Start_Code + 131;
    public static final int VoLTE_Setting_Call_prefer_original_codec           = VoLTE_Setting_VoLTE_Call_Start_Code + 132;
    public static final int VoLTE_Setting_Call_add_audio_video_in_delay_media  = VoLTE_Setting_VoLTE_Call_Start_Code + 133;
    public static final int VoLTE_Setting_Call_add_contact_in_bye              = VoLTE_Setting_VoLTE_Call_Start_Code + 134;
    public static final int VoLTE_Setting_Call_resp_precondition_without_bw_cnf= VoLTE_Setting_VoLTE_Call_Start_Code + 135;
    public static final int VoLTE_Setting_Call_wait_second_invite_for_hold     = VoLTE_Setting_VoLTE_Call_Start_Code + 136;
    public static final int VoLTE_Setting_Call_no_feature_caps_process         = VoLTE_Setting_VoLTE_Call_Start_Code + 137;
    public static final int VoLTE_Setting_Call_delay_ringing                   = VoLTE_Setting_VoLTE_Call_Start_Code + 138;
    public static final int VoLTE_Setting_Call_add_framerate                   = VoLTE_Setting_VoLTE_Call_Start_Code + 139;
    public static final int VoLTE_Setting_Call_add_framesize                   = VoLTE_Setting_VoLTE_Call_Start_Code + 140;
    public static final int VoLTE_Setting_Call_add_imageattr                   = VoLTE_Setting_VoLTE_Call_Start_Code + 141;
    public static final int VoLTE_Setting_VoLTE_Call_allow_non_sendrecv_rtcp_0 = VoLTE_Setting_VoLTE_Call_Start_Code + 142;
    public static final int VoLTE_Setting_VoLTE_Call_RTCP_XR_SUPPORT           = VoLTE_Setting_VoLTE_Call_Start_Code + 143;
    public static final int VoLTE_Setting_VoLTE_Call_RTCP_XR_FORMAT            = VoLTE_Setting_VoLTE_Call_Start_Code + 144;
    public static final int VoLTE_Setting_VoLTE_Call_RTCP_XR_LOSS_MAXSIZE      = VoLTE_Setting_VoLTE_Call_Start_Code + 145;
    public static final int VoLTE_Setting_VoLTE_Call_RTCP_XR_DUP_MAXSIZE       = VoLTE_Setting_VoLTE_Call_Start_Code + 146;
    public static final int VoLTE_Setting_VoLTE_Call_RTCP_XR_RCPT_MAXSIZE      = VoLTE_Setting_VoLTE_Call_Start_Code + 147;
    public static final int VoLTE_Setting_VoLTE_Call_RTCP_XR_RTT_MAXSIZE       = VoLTE_Setting_VoLTE_Call_Start_Code + 148;
    public static final int VoLTE_Setting_VoLTE_Call_RTCP_XR_RTT_MODE          = VoLTE_Setting_VoLTE_Call_Start_Code + 149;
    public static final int VoLTE_Setting_VoLTE_Call_RTCP_XR_STAT_FLAGS        = VoLTE_Setting_VoLTE_Call_Start_Code + 150;
    /* bitmask */
    public static final int VoLTE_Setting_add_country_to_pani                  = VoLTE_Setting_VoLTE_Call_Start_Code + 151;
    public static final int VoLTE_Setting_Call_force_srvcc_transfer            = VoLTE_Setting_VoLTE_Call_Start_Code + 152;
    public static final int VoLTE_Setting_Call_reject_dlg_more_than_max        = VoLTE_Setting_VoLTE_Call_Start_Code + 153;
    public static final int VoLTE_Setting_Call_send_bye_when_term_dlg          = VoLTE_Setting_VoLTE_Call_Start_Code + 154;
    public static final int VoLTE_Setting_Call_gtt_support                     = VoLTE_Setting_VoLTE_Call_Start_Code + 155;
    public static final int VoLTE_Setting_Call_gtt_t140_pt                     = VoLTE_Setting_VoLTE_Call_Start_Code + 156;
    public static final int VoLTE_Setting_Call_gtt_red_pt                      = VoLTE_Setting_VoLTE_Call_Start_Code + 157;
    public static final int VoLTE_Setting_Call_gtt_red_level                   = VoLTE_Setting_VoLTE_Call_Start_Code + 158;
    public static final int VoLTE_Setting_Call_gtt_cps                         = VoLTE_Setting_VoLTE_Call_Start_Code + 159;
    public static final int VoLTE_Setting_Call_srvcc_feature_enable            = VoLTE_Setting_VoLTE_Call_Start_Code + 160;
    public static final int VoLTE_Setting_Call_cs_call_status                  = VoLTE_Setting_VoLTE_Call_Start_Code + 162;
    public static final int VoLTE_Setting_Call_show_octet_align                = VoLTE_Setting_VoLTE_Call_Start_Code + 164;
    public static final int VoLTE_Setting_Call_video_conf_if_one_is_video      = VoLTE_Setting_VoLTE_Call_Start_Code + 165;
    public static final int VoLTE_Setting_Call_use_from_to_append_pau          = VoLTE_Setting_VoLTE_Call_Start_Code + 166;
    public static final int VoLTE_Setting_Call_rel_holdcall_in_midcall_SRVCC   = VoLTE_Setting_VoLTE_Call_Start_Code + 167;
    public static final int VoLTE_Setting_Call_need_revise_rtp_dir             = VoLTE_Setting_VoLTE_Call_Start_Code + 168;
    public static final int VoLTE_Setting_Call_calc_participant_count_from_notify            = VoLTE_Setting_VoLTE_Call_Start_Code + 169;
    public static final int VoLTE_Setting_Call_dereg_send_bye                  = VoLTE_Setting_VoLTE_Call_Start_Code + 170;
    public static final int VoLTE_Setting_VoLTE_Call_CALL_LTE_EMC_TIMER_TIMEOUT= VoLTE_Setting_VoLTE_Call_Start_Code + 171;
    public static final int VoLTE_Setting_Call_pull_service                    = VoLTE_Setting_VoLTE_Call_Start_Code + 172;
    public static final int VoLTE_Setting_Call_always100rel_in_resp            = VoLTE_Setting_VoLTE_Call_Start_Code + 173;
    public static final int VoLTE_Setting_Call_use_183_for_early_media         = VoLTE_Setting_VoLTE_Call_Start_Code + 174;
    public static final int VoLTE_Setting_Call_pull_sub_expire                 = VoLTE_Setting_VoLTE_Call_Start_Code + 175;
    public static final int VoLTE_Setting_Call_gtt_mode                        = VoLTE_Setting_VoLTE_Call_Start_Code + 176;
    public static final int VoLTE_Setting_Call_ect_assured_notify_timer_timeout  = VoLTE_Setting_VoLTE_Call_Start_Code + 177;
    public static final int VoLTE_Setting_Call_ect_new_call_type                 = VoLTE_Setting_VoLTE_Call_Start_Code + 178;
    public static final int VoLTE_Setting_Call_ect_refer_in_new_dialog           = VoLTE_Setting_VoLTE_Call_Start_Code + 179;
    public static final int VoLTE_Setting_Call_ect_without_target_hold           = VoLTE_Setting_VoLTE_Call_Start_Code + 180;
    public static final int VoLTE_Setting_Call_ect_default_type                  = VoLTE_Setting_VoLTE_Call_Start_Code + 181;
    public static final int VoLTE_Setting_Call_ect_refer_to_tag_swap             = VoLTE_Setting_VoLTE_Call_Start_Code + 182;
    public static final int VoLTE_Setting_Call_ssac_on                         = VoLTE_Setting_VoLTE_Call_Start_Code + 183;
    public static final int VoLTE_Setting_Call_1x_call_status                  = VoLTE_Setting_VoLTE_Call_Start_Code + 184;
    public static final int VoLTE_Setting_Call_subscriber_provisioned_for_volte= VoLTE_Setting_VoLTE_Call_Start_Code + 185;
    public static final int VoLTE_Setting_Call_timer_b_restore_reg             = VoLTE_Setting_VoLTE_Call_Start_Code + 186;
    public static final int VoLTE_Setting_VoLTE_Call_sdp_group_rtpmap_fmtp     = VoLTE_Setting_VoLTE_Call_Start_Code + 187;
    public static final int VoLTE_Setting_VoLTE_Call_sdp_group_rtpmap_ptime    = VoLTE_Setting_VoLTE_Call_Start_Code + 188;
    public static final int VoLTE_Setting_VoLTE_Call_transfer_conf_call_as_1to1 = VoLTE_Setting_VoLTE_Call_Start_Code + 189;
    public static final int VoLTE_Setting_Call_conf_amr_wb_mode_set            = VoLTE_Setting_VoLTE_Call_Start_Code + 190;
    public static final int VoLTE_Setting_Call_video_action_prcd               = VoLTE_Setting_VoLTE_Call_Start_Code + 192;
    public static final int VoLTE_Setting_Call_bypass_conf_qos_check_in_delay_media = VoLTE_Setting_VoLTE_Call_Start_Code + 194;
    public static final int VoLTE_Setting_Call_auto_unhold_when_rcv_recvonly    = VoLTE_Setting_VoLTE_Call_Start_Code + 195;
    public static final int VoLTE_Setting_VoLTE_Call_check_curr_qos            = VoLTE_Setting_VoLTE_Call_Start_Code + 196;
    public static final int VoLTE_Setting_Call_err_handling_for_video_via_update    = VoLTE_Setting_VoLTE_Call_Start_Code + 198;
    public static final int VoLTE_Setting_Call_sendrecv_before_conf_call_prcd    = VoLTE_Setting_VoLTE_Call_Start_Code + 200;
    public static final int VoLTE_Setting_VoLTE_Call_ringing_timeout_code           = VoLTE_Setting_VoLTE_Call_Start_Code + 201;
    public static final int VoLTE_Setting_Call_keep_orginal_refresher_in_reinvite   = VoLTE_Setting_VoLTE_Call_Start_Code + 202;
    public static final int VoLTE_Setting_Call_pem_use_sdp_rtp_drct            = VoLTE_Setting_VoLTE_Call_Start_Code + 203;
    public static final int VoLTE_Setting_Call_RTT_enable                      = VoLTE_Setting_VoLTE_Call_Start_Code + 204;
    public static final int VoLTE_Setting_VoLTE_Call_MOD_SESSION_CNF_TIMER_TIMEROUT  = VoLTE_Setting_VoLTE_Call_Start_Code + 205;
    public static final int VoLTE_Setting_VoLTE_Call_FORCE_CSFB_WHEN663             = VoLTE_Setting_VoLTE_Call_Start_Code + 207;
    public static final int VoLTE_Setting_Call_decouple_conf_srvcc_mid_call_ue_cap  = VoLTE_Setting_VoLTE_Call_Start_Code + 208;
    public static final int VoLTE_Setting_Call_disable_ul_rtp_in_early_state   = VoLTE_Setting_VoLTE_Call_Start_Code + 209;
    public static final int VoLTE_Setting_Call_pqos_timer_timeout              = VoLTE_Setting_VoLTE_Call_Start_Code + 210;
    public static final int VoLTE_Setting_Call_support_early_upgrade           = VoLTE_Setting_VoLTE_Call_Start_Code + 211;
    public static final int VoLTE_Setting_Call_lte_attach_type                 = VoLTE_Setting_VoLTE_Call_Start_Code + 212;
    public static final int VoLTE_Setting_Call_TEXT_feature_tag                = VoLTE_Setting_VoLTE_Call_Start_Code + 213;
    public static final int VoLTE_Setting_Call_EMC_RTT_CB_TIMER_TIMEOUT        = VoLTE_Setting_VoLTE_Call_Start_Code + 214;
    public static final int VoLTE_Setting_VoLTE_Call_SEND_REFER_TO_CONF_URI         = VoLTE_Setting_VoLTE_Call_Start_Code + 215;
    public static final int VoLTE_Setting_Call_disable_amr_params_in_wfc_call = VoLTE_Setting_VoLTE_Call_Start_Code + 216;
    public static final int VoLTE_Setting_Call_Barring_In_Warning_Header            = VoLTE_Setting_VoLTE_Call_Start_Code + 219;
    public static final int VoLTE_Setting_Call_EMC_Ringback_Timer_Timeout      = VoLTE_Setting_VoLTE_Call_Start_Code + 220;
    public static final int VoLTE_Setting_Call_Add_PCNI_In_VoWiFi                    = VoLTE_Setting_VoLTE_Call_Start_Code + 221;
    public static final int VoLTE_Setting_Call_conference_desub_time                = VoLTE_Setting_VoLTE_Call_Start_Code + 222;
    public static final int VoLTE_Setting_Call_use_amr_bit_rate_downgrade      = VoLTE_Setting_VoLTE_Call_Start_Code + 223;
    public static final int VoLTE_Setting_Call_re_calc_rr_rs_in_offer_answer   = VoLTE_Setting_VoLTE_Call_Start_Code + 224;
    public static final int VoLTE_Setting_Call_max_hold_unhold_retry_cnt       = VoLTE_Setting_VoLTE_Call_Start_Code + 229;
    public static final int VoLTE_Setting_Call_ect_enable                      = VoLTE_Setting_VoLTE_Call_Start_Code + 230;
    public static final int VoLTE_Setting_Call_upgrade_cancel_feature_enable   = VoLTE_Setting_VoLTE_Call_Start_Code + 231;
    public static final int VoLTE_Setting_Call_upgrade_with_Tupo               = VoLTE_Setting_VoLTE_Call_Start_Code + 232;
    public static final int VoLTE_Setting_Call_record_route_in_update_resp     = VoLTE_Setting_VoLTE_Call_Start_Code + 233;
    public static final int VoLTE_Setting_Call_mo_pidf_usage                   = VoLTE_Setting_VoLTE_Call_Start_Code + 235;
    public static final int VoLTE_Setting_Call_send_bw_ind_when_recv_183_prack = VoLTE_Setting_VoLTE_Call_Start_Code + 237;
    public static final int VoLTE_Setting_Call_refer_delay_timer               = VoLTE_Setting_VoLTE_Call_Start_Code + 239;
    public static final int VoLTE_Setting_Call_video_offer_rtp_profile         = VoLTE_Setting_VoLTE_Call_Start_Code + 240;

    /* ------------------------------------------------ */
    /*  VoLTE MWI Settings                              */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_VoLTE_MWI_Start_Code      = VoLTE_Setting_VoLTE_Call_Start_Code + 900;
    public static final int VoLTE_Setting_VoLTE_MWI_License         = VoLTE_Setting_VoLTE_MWI_Start_Code  + 1;           ///< Disable: 0; Enable: 1

    /* ------------------------------------------------ */
    /*  VoLTE SMS Settings                              */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_VoLTE_SMS_Start_Code      = 12000;
    public static final int VoLTE_Setting_VoLTE_SMS_Capability      = VoLTE_Setting_VoLTE_SMS_Start_Code + 1;               ///< data type is boolean (1 or 0) */
    public static final int VoLTE_Setting_VoLTE_SMS_3GPP_SMS_PSI    = VoLTE_Setting_VoLTE_SMS_Start_Code + 2;               ///< data type is string
    public static final int VoLTE_Setting_VoLTE_SMS_PSISMSC_FROM_USER          = VoLTE_Setting_VoLTE_SMS_Start_Code + 3;
    public static final int VoLTE_Setting_VoLTE_SMS_FORCE_PSI_SCHEME_TO_TEL    = VoLTE_Setting_VoLTE_SMS_Start_Code + 4;
    public static final int VoLTE_Setting_VoLTE_SMS_ADD_USER_PHONE             = VoLTE_Setting_VoLTE_SMS_Start_Code + 5;
    public static final int VoLTE_Setting_VoLTE_SMS_UPDATE_CALL_ID_WITH_HOST   = VoLTE_Setting_VoLTE_SMS_Start_Code + 6;
    public static final int VoLTE_Setting_VoLTE_SMS_ADD_NO_FORK_IN_SMS         = VoLTE_Setting_VoLTE_SMS_Start_Code + 7;
    public static final int VoLTE_Setting_VoLTE_SMS_NSN_SPECIFIC               = VoLTE_Setting_VoLTE_SMS_Start_Code + 8;
    public static final int VoLTE_Setting_VoLTE_SMS_MO_RETRY_AFTER_504         = VoLTE_Setting_VoLTE_SMS_Start_Code + 9;
    public static final int VoLTE_Setting_VoLTE_SMS_SEND_TIMERF_EXPIRY         = VoLTE_Setting_VoLTE_SMS_Start_Code + 10;
    public static final int VoLTE_Setting_VoLTE_SMS_IMSI_REGED_DISABLE_SMS     = VoLTE_Setting_VoLTE_SMS_Start_Code + 11;
    public static final int VoLTE_Setting_VoLTE_SMS_network_types              = VoLTE_Setting_VoLTE_SMS_Start_Code + 12;
    public static final int VoLTE_Setting_VoLTE_SMS_restore_in_3gpp_abnormal   = VoLTE_Setting_VoLTE_SMS_Start_Code + 13;
    public static final int VoLTE_Setting_VoLTE_SMS_support_in_23g             = VoLTE_Setting_VoLTE_SMS_Start_Code + 14;
    public static final int VoLTE_Setting_VoLTE_SMS_FOLLOW_NW_IMS_VOPS_SUPPORT = VoLTE_Setting_VoLTE_SMS_Start_Code + 15;
    /* ------------------------------------------------ */
    /*  VoLTE XDM Settings                              */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_VoLTE_XDM_Start_Code      = 13000;

    /* ------------------------------------------------ */
    /*  VoLTE REG Settings                              */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_VoLTE_REG_Start_Code      = 14000;
    public static final int VoLTE_Setting_Reg_Register_Expiry       = VoLTE_Setting_VoLTE_REG_Start_Code + 1;               ///< data type is int
    public static final int VoLTE_Setting_Reg_Event_Subscribe       = VoLTE_Setting_VoLTE_REG_Start_Code + 2;               ///< Disable: 0; Enable: 1
    public static final int VoLTE_Setting_Reg_Emergency             = VoLTE_Setting_VoLTE_REG_Start_Code + 3;               ///< Disable: 0; Enable: 1
    public static final int VoLTE_Setting_Reg_Retry_First_Time      = VoLTE_Setting_VoLTE_REG_Start_Code + 4;               ///< data type is int
    public static final int VoLTE_Setting_Reg_Retry_Base_Time       = VoLTE_Setting_VoLTE_REG_Start_Code + 5;               ///< data type is int
    public static final int VoLTE_Setting_Reg_Retry_Max_Time        = VoLTE_Setting_VoLTE_REG_Start_Code + 6;               ///< data type is int
    public static final int VoLTE_Setting_Reg_Contact_With_UserName = VoLTE_Setting_VoLTE_REG_Start_Code + 7;               ///< Disable: 0; Enable: 1
    public static final int VoLTE_Setting_Reg_URI_With_Port         = VoLTE_Setting_VoLTE_REG_Start_Code + 8;               ///< Disable: 0; Enable: 1
    public static final int VoLTE_Setting_Reg_IPSec_Algo_Set        = VoLTE_Setting_VoLTE_REG_Start_Code + 9;               ///< data type is int
    public static final int VoLTE_Setting_Reg_Enable_HTTP_Digest    = VoLTE_Setting_VoLTE_REG_Start_Code + 10;              ///< Disable: 0; Enable: 1
    public static final int VoLTE_Setting_Reg_Auth_Name            = VoLTE_Setting_VoLTE_REG_Start_Code + 11;              ///< data type is string
    public static final int VoLTE_Setting_Reg_Auth_Password        = VoLTE_Setting_VoLTE_REG_Start_Code + 12;              ///< data type is string
    public static final int VoLTE_Setting_Reg_Specific_IPSec_Algo  = VoLTE_Setting_VoLTE_REG_Start_Code + 13;                ///< data type is int
    public static final int VoLTE_Setting_Reg_CONTACT_WITH_TRANSPORT                         = VoLTE_Setting_VoLTE_REG_Start_Code + 14;
    public static final int VoLTE_Setting_Reg_CONTACT_WITH_REGID                             = VoLTE_Setting_VoLTE_REG_Start_Code + 15;
    public static final int VoLTE_Setting_Reg_CONTACT_WITH_MOBILITY                          = VoLTE_Setting_VoLTE_REG_Start_Code + 16;
    public static final int VoLTE_Setting_Reg_CONTACT_WITH_EXPIRES                           = VoLTE_Setting_VoLTE_REG_Start_Code + 17;
    public static final int VoLTE_Setting_Reg_AUTHORIZATION_WITH_ALGO                        = VoLTE_Setting_VoLTE_REG_Start_Code + 18;
    public static final int VoLTE_Setting_Reg_REREG_IN_RAT_CHANGE                            = VoLTE_Setting_VoLTE_REG_Start_Code + 19;
    public static final int VoLTE_Setting_Reg_REREG_IN_OOS_END                               = VoLTE_Setting_VoLTE_REG_Start_Code + 20;
    public static final int VoLTE_Setting_Reg_DE_SUBSCRIBE                                   = VoLTE_Setting_VoLTE_REG_Start_Code + 21;
    public static final int VoLTE_Setting_Reg_USE_SPECIFIC_IPSEC_ALGO                        = VoLTE_Setting_VoLTE_REG_Start_Code + 22;
    public static final int VoLTE_Setting_Reg_TRY_NEXT_PCSCF                                 = VoLTE_Setting_VoLTE_REG_Start_Code + 23;
    public static final int VoLTE_Setting_Reg_DEREG_CLEAR_IPSEC                              = VoLTE_Setting_VoLTE_REG_Start_Code + 24;
    public static final int VoLTE_Setting_Reg_INITIAL_REG_WITHOUT_PANI                       = VoLTE_Setting_VoLTE_REG_Start_Code + 25;
    public static final int VoLTE_Setting_Reg_DEREG_RESET_TCP_CLIENT                         = VoLTE_Setting_VoLTE_REG_Start_Code + 26;
    public static final int VoLTE_Setting_Reg_TREG                                           = VoLTE_Setting_VoLTE_REG_Start_Code + 27;
    public static final int VoLTE_Setting_Reg_REREG_23G4                                     = VoLTE_Setting_VoLTE_REG_Start_Code + 28;
    public static final int VoLTE_Setting_Reg_RESUB_23G4                                     = VoLTE_Setting_VoLTE_REG_Start_Code + 29;
    public static final int VoLTE_Setting_Reg_NOT_AUTO_REG_403                               = VoLTE_Setting_VoLTE_REG_Start_Code + 30;
    public static final int VoLTE_Setting_Reg_CALL_ID_WITH_HOST_INREG                        = VoLTE_Setting_VoLTE_REG_Start_Code + 31;
    public static final int VoLTE_Setting_Reg_KEEP_ALIVE_MODE                                = VoLTE_Setting_VoLTE_REG_Start_Code + 32;
    public static final int VoLTE_Setting_Reg_TCP_CONNECT_MAX_TIME_INVITE                    = VoLTE_Setting_VoLTE_REG_Start_Code + 33;
    public static final int VoLTE_Setting_Reg_EMS_MODE_IND                                   = VoLTE_Setting_VoLTE_REG_Start_Code + 34;
    public static final int VoLTE_Setting_Reg_CONTACT_WITH_ACCESSTYPE                        = VoLTE_Setting_VoLTE_REG_Start_Code + 35;
    public static final int VoLTE_Setting_Reg_WFC_WITH_PLANI                                 = VoLTE_Setting_VoLTE_REG_Start_Code + 36;
    public static final int VoLTE_Setting_Reg_USE_UDP_ON_TCP_FAIL                            = VoLTE_Setting_VoLTE_REG_Start_Code + 37;
    public static final int VoLTE_Setting_Reg_IPSEC_FAIL_ALLOWED                             = VoLTE_Setting_VoLTE_REG_Start_Code + 38;
    public static final int VoLTE_Setting_Reg_CONTACT_WITH_VIDEO_FEATURE_TAG_IN_SUBSCRIBE    = VoLTE_Setting_VoLTE_REG_Start_Code + 39;
    public static final int VoLTE_Setting_Reg_VIA_WITHOUT_RPORT                              = VoLTE_Setting_VoLTE_REG_Start_Code + 40;
    public static final int VoLTE_Setting_Reg_REG_ROUTE_HDR                                  = VoLTE_Setting_VoLTE_REG_Start_Code + 41;
    public static final int VoLTE_Setting_Reg_VIA_URI_WITH_DEFAULT_PORT                      = VoLTE_Setting_VoLTE_REG_Start_Code + 42;
    public static final int VoLTE_Setting_Reg_NOTIFY_SMS_NOTIFY_DONE                         = VoLTE_Setting_VoLTE_REG_Start_Code + 43;
    public static final int VoLTE_Setting_Reg_EMERGENCY_USE_IMSI                             = VoLTE_Setting_VoLTE_REG_Start_Code + 44;
    public static final int VoLTE_Setting_Reg_CHECK_MSISDN                                   = VoLTE_Setting_VoLTE_REG_Start_Code + 45;
    public static final int VoLTE_Setting_Reg_RETRY_INTERVAL_AFTER_403                       = VoLTE_Setting_VoLTE_REG_Start_Code + 46;
    public static final int VoLTE_Setting_Reg_SUPPORT_THROTTLING_ALGO                        = VoLTE_Setting_VoLTE_REG_Start_Code + 47;
    public static final int VoLTE_Setting_Reg_REG_AFTER_NW_DEREG_60S                         = VoLTE_Setting_VoLTE_REG_Start_Code + 48;
    public static final int VoLTE_Setting_Reg_SUB_CONTACT_WITH_SIP_INSTANCE                  = VoLTE_Setting_VoLTE_REG_Start_Code + 49;
    public static final int VoLTE_Setting_Reg_STOP_REG_MD_LOWER_LAYER_ERR                    = VoLTE_Setting_VoLTE_REG_Start_Code + 50;
    public static final int VoLTE_Setting_Reg_REG_GRUU_SUPPORT                               = VoLTE_Setting_VoLTE_REG_Start_Code + 51;
    public static final int VoLTE_Setting_Reg_oos_end_reset_tcp_client          = VoLTE_Setting_VoLTE_REG_Start_Code + 53;
    public static final int VoLTE_Setting_Reg_pidf_country                      = VoLTE_Setting_VoLTE_REG_Start_Code + 55;
    public static final int VoLTE_Setting_Reg_REG_SUPPORT_MAX_RETRY_ALGO        = VoLTE_Setting_VoLTE_REG_Start_Code + 56;
    public static final int VoLTE_Setting_SIP_A_Timer                           = VoLTE_Setting_VoLTE_REG_Start_Code + 57;
    public static final int VoLTE_Setting_SIP_B_Timer                           = VoLTE_Setting_VoLTE_REG_Start_Code + 58;
    public static final int VoLTE_Setting_SIP_C_Timer                           = VoLTE_Setting_VoLTE_REG_Start_Code + 59;
    public static final int VoLTE_Setting_SIP_D_Timer                           = VoLTE_Setting_VoLTE_REG_Start_Code + 60;
    public static final int VoLTE_Setting_SIP_E_Timer                           = VoLTE_Setting_VoLTE_REG_Start_Code + 61;
    public static final int VoLTE_Setting_SIP_G_Timer                           = VoLTE_Setting_VoLTE_REG_Start_Code + 62;
    public static final int VoLTE_Setting_SIP_H_Timer                           = VoLTE_Setting_VoLTE_REG_Start_Code + 63;
    public static final int VoLTE_Setting_SIP_I_Timer                           = VoLTE_Setting_VoLTE_REG_Start_Code + 64;
    public static final int VoLTE_Setting_SIP_J_Timer                           = VoLTE_Setting_VoLTE_REG_Start_Code + 65;
    public static final int VoLTE_Setting_SIP_K_Timer                           = VoLTE_Setting_VoLTE_REG_Start_Code + 66;
    public static final int VoLTE_Setting_Reg_FAIL_NOT_NEED_REMOVE_BINDING      = VoLTE_Setting_VoLTE_REG_Start_Code + 67;
    public static final int VoLTE_Setting_Reg_REG_RCS_State                     = VoLTE_Setting_VoLTE_REG_Start_Code + 68;
    public static final int VoLTE_Setting_Reg_PEND_DEREG_IN_INITIAL_REG         = VoLTE_Setting_VoLTE_REG_Start_Code + 69;
    public static final int VoLTE_Setting_Reg_TRY_NEXT_PCSCF_5626               = VoLTE_Setting_VoLTE_REG_Start_Code + 70;
    public static final int VoLTE_Setting_Reg_TRY_SAME_PCSCF_REREG              = VoLTE_Setting_VoLTE_REG_Start_Code + 71;
    public static final int VoLTE_Setting_Reg_PUBLIC_UIDS                       = VoLTE_Setting_VoLTE_REG_Start_Code + 72;
    public static final int VoLTE_Setting_Reg_5626_BASE_TIME                    = VoLTE_Setting_VoLTE_REG_Start_Code + 73;
    public static final int VoLTE_Setting_Reg_5626_MAX_TIME                     = VoLTE_Setting_VoLTE_REG_Start_Code + 74;
    public static final int VoLTE_Setting_Reg_TRY_ALL_PCSCF_5626                = VoLTE_Setting_VoLTE_REG_Start_Code + 75;
    public static final int VoLTE_Setting_Reg_REG_Call_Domain                   = VoLTE_Setting_VoLTE_REG_Start_Code + 78;
    public static final int VoLTE_Setting_Reg_REG_Silent_Redial                 = VoLTE_Setting_VoLTE_REG_Start_Code + 79;
    public static final int VoLTE_Setting_Reg_REG_Tdelay                        = VoLTE_Setting_VoLTE_REG_Start_Code + 80;
    public static final int VoLTE_Setting_Reg_REG_ECBM_State                    = VoLTE_Setting_VoLTE_REG_Start_Code + 81;
    public static final int VoLTE_Setting_Reg_Try_N_Next_Pcscf_5626             = VoLTE_Setting_VoLTE_REG_Start_Code + 84;
    public static final int VoLTE_Setting_Reg_Dereg_Delay_Time                  = VoLTE_Setting_VoLTE_REG_Start_Code + 85;
    public static final int VoLTE_Setting_Reg_try_same_pcscf_if_retry_after     = VoLTE_Setting_VoLTE_REG_Start_Code + 86;
    public static final int VoLTE_Setting_Reg_CHECK_NOTIFY_INSTANCE_ID          = VoLTE_Setting_VoLTE_REG_Start_Code + 88;
    public static final int VoLTE_Setting_Reg_ADD_CS_VOLTE_FEATURE_TAG          = VoLTE_Setting_VoLTE_REG_Start_Code + 89;
    public static final int VoLTE_Setting_Reg_CHECK_CALL_DOMAIN                 = VoLTE_Setting_VoLTE_REG_Start_Code + 90;
    public static final int VoLTE_Setting_Reg_SUPPORT_SIP_BLOCK                 = VoLTE_Setting_VoLTE_REG_Start_Code + 91;
    public static final int VoLTE_Setting_Reg_Reuse_Transport_Methods           = VoLTE_Setting_VoLTE_REG_Start_Code + 92;
    public static final int VoLTE_Setting_SIP_WHEN_STOP_B_TIMER                 = VoLTE_Setting_VoLTE_REG_Start_Code + 93;
    public static final int VoLTE_Setting_Reg_RCS_ADD_MOBILITY_TAG              = VoLTE_Setting_VoLTE_REG_Start_Code + 95;
    public static final int VoLTE_Setting_Reg_support_resub                     = VoLTE_Setting_VoLTE_REG_Start_Code + 96;
    public static final int VoLTE_Setting_Reg_Expire_Time_By_User               = VoLTE_Setting_VoLTE_REG_Start_Code + 97;
    public static final int VoLTE_Setting_Add_CNI_in_WIFI                       = VoLTE_Setting_VoLTE_REG_Start_Code + 98;
    public static final int VoLTE_Setting_Reg_Reg_Over_Tcp                      = VoLTE_Setting_VoLTE_REG_Start_Code + 99;
    public static final int VoLTE_Setting_Reg_NOT_AUTO_REG_500                  = VoLTE_Setting_VoLTE_REG_Start_Code + 100;
    public static final int VoLTE_Setting_SIP_EMC_CITS_Protect_Timer            = VoLTE_Setting_VoLTE_REG_Start_Code + 102;
    public static final int VoLTE_Setting_Reg_REG_Call_Domain_Cause             = VoLTE_Setting_VoLTE_REG_Start_Code + 103;
    public static final int VoLTE_Setting_Reg_RANDOM_INIT_IPSEC_PORT            = VoLTE_Setting_VoLTE_REG_Start_Code + 104;
    public static final int VoLTE_Setting_Add_cell_info_age_to_cni              = VoLTE_Setting_VoLTE_REG_Start_Code + 105;
    public static final int VoLTE_Setting_SIP_check_reg_contact                 = VoLTE_Setting_VoLTE_REG_Start_Code + 106;
    public static final int VoLTE_Setting_REG_Geolocation_Type                  = VoLTE_Setting_VoLTE_REG_Start_Code + 107;
    public static final int VoLTE_Setting_Reg_Init_Reg_Delay_Time               = VoLTE_Setting_VoLTE_REG_Start_Code + 108;
    public static final int VoLTE_Setting_Reg_Update_IPSec_Port_494             = VoLTE_Setting_VoLTE_REG_Start_Code + 112;

    /* ------------------------------------------------ */
    /*  VoLTE SIP Settings                              */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_VoLTE_SIP_Start_Code      = 15000;
    public static final int VoLTE_Setting_SIP_T1_Timer              = VoLTE_Setting_VoLTE_SIP_Start_Code + 1;               ///< data type is int
    public static final int VoLTE_Setting_SIP_T2_Timer              = VoLTE_Setting_VoLTE_SIP_Start_Code + 2;               ///< data type is int
    public static final int VoLTE_Setting_SIP_T4_Timer              = VoLTE_Setting_VoLTE_SIP_Start_Code + 3;               ///< data type is int
    public static final int VoLTE_Setting_SIP_Keep_Alive            = VoLTE_Setting_VoLTE_SIP_Start_Code + 4;               ///< data type is int
    public static final int VoLTE_Setting_SIP_Force_Use_UDP         = VoLTE_Setting_VoLTE_SIP_Start_Code + 5;               ///< Disable: 0; Enable: 1
    public static final int VoLTE_Setting_SIP_TCP_On_Demand         = VoLTE_Setting_VoLTE_SIP_Start_Code + 6;               ///< Disable: 0; Enable: 1
    public static final int VoLTE_Setting_SIP_TCP_MTU_Size          = VoLTE_Setting_VoLTE_SIP_Start_Code + 7;               ///< data type is int
    public static final int VoLTE_Setting_SIP_Reg_Timestamp         = VoLTE_Setting_VoLTE_SIP_Start_Code + 8;               ///< data type is string; for WFC PLANI header
    public static final int VoLTE_Setting_SIP_Transaction_Timer     = VoLTE_Setting_VoLTE_SIP_Start_Code + 9;               ///< data type is int

    /* ------------------------------------------------ */
    /*  VoLTE USSD Settings                             */
    /* ------------------------------------------------ */
    public static final int VoLTE_Setting_VoLTE_USSD_Start_Code      = 16000;
    public static final int VoLTE_Setting_USSD_NW_Timeout_Timer      = VoLTE_Setting_VoLTE_USSD_Start_Code + 1;               ///< data type is uint8 */
    public static final int VoLTE_Setting_USSD_Setup_Timeout_Timer   = VoLTE_Setting_VoLTE_USSD_Start_Code + 2;               ///< data type is uint8 */
    public static final int VoLTE_Setting_USSD_Capability            = VoLTE_Setting_VoLTE_USSD_Start_Code + 3;               ///< data type is uint8 */


    public int mType;
    public int mDatalen;
    public byte mData[];

    public RcsEventSettingTLV(int type, int len, byte[] data) {
        mType = type;
        mDatalen = len;
        mData = data;
    }
}

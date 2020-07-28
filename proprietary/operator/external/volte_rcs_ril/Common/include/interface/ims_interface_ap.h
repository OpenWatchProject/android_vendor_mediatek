/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef _IMS_INTERFACE_AP_H
#define _IMS_INTERFACE_AP_H

/*****imc_general_def.h START*****/
#define IMC_MAX_IMPU_LEN                256
#define IMC_MAX_DOMAIN_NAME_LEN         256
#define IMC_MAX_URI_LENGTH              128
#define IMC_MAX_IF_NAME_LENGTH          (16)
/*****imc_general_def.h END*****/

/*****ims_msgid.h START*****/
#define IMCB_IMC_VOLTE_EVENT_EXTERNAL_IMC_START_CODE (100000)
/*****ims_msgid.h END*****/

/**************************/
/***** UA PART START ******/
/**************************/

/*****volte_def.h START*****/
#define VOLTE_MAX_CELL_ID_LENGTH                        (64)
#define VOLTE_MAX_CALL_ID_LENGTH                        (64)
#define VOLTE_MAX_URI_LENGTH                            (IMC_MAX_URI_LENGTH)
#define VOLTE_MAX_DOMAIN_NAME_LENGTH                    IMC_MAX_DOMAIN_NAME_LEN
#define VOLTE_MAX_USER_AGENT_LENGTH                     (128)
#define VOLTE_MAX_GRUU_LENGTH                           (128)
#define VOLTE_MAX_ASSOCIATED_URI                        (VOLTE_MAX_URI_LENGTH << 2)
#define VOLTE_MAX_ADDRESS_LENGTH                        (64)
#define VOLTE_MAX_ADDRESS_LIST_LENGTH                   (256)
#define VOLTE_MAX_REG_UID_LENGTH                        IMC_MAX_IMPU_LEN
#define VOLTE_MAX_IMEI_LENGTH                           (20)
#define VOLTE_MAX_REG_CAPABILITY_LENGTH                 (256)
#define VOLTE_MAX_TIMESTAMP_LENGTH                      (256)
#define VOLTE_MAX_IF_NAME_LENGTH                        (IMC_MAX_IF_NAME_LENGTH)
#define VOLTE_MAX_SECURIT_VERIFY_LENGTH                 (768) //6 algo combo
#define VOLTE_MAX_TIME_STAMP_LENGTH                     (32)
#define VOLTE_MAX_PLANI_LENGTH                          (256)
/*****volte_def.h END*****/

/*****volte_type.h START*****/
typedef unsigned int UINT32;
typedef int INT32;
typedef unsigned short UINT16;
typedef short INT16;
typedef unsigned char UINT8;
typedef char INT8;
/*****volte_type.h END*****/

/* location info */
typedef struct _VoLTE_Event_LBS_Location_ {
    INT32               lat[2];       /* [0..8388607], 23 bits field */
    INT32               lng[2];       /* [-8388608..8388607], 24 bits field */
    INT32               acc;
    UINT8               timestamp[VOLTE_MAX_TIME_STAMP_LENGTH];   // VOLTE_MAX_TIME_STAMP_LENGTH =32
} VoLTE_Event_LBS_Location_t;

enum VoLTE_Event_SIP_Cause_e {
    VoLTE_Event_SIP_Cause_Unspecified                               = 0,
    VoLTE_Event_SIP_Cause_300_Multiple_Choices                      = 300,
    VoLTE_Event_SIP_Cause_301_Moved_Permanently                     = 301,
    VoLTE_Event_SIP_Cause_302_Moved_Temporarily                     = 302,
    VoLTE_Event_SIP_Cause_305_Use_Proxy                             = 305,
    VoLTE_Event_SIP_Cause_380_Alternative_Service                   = 380,
    VoLTE_Event_SIP_Cause_400_Bad_Request                           = 400,
    VoLTE_Event_SIP_Cause_401_Unauthorized                          = 401,
    VoLTE_Event_SIP_Cause_402_Payment_Required                      = 402,
    VoLTE_Event_SIP_Cause_403_Forbidden                             = 403,
    VoLTE_Event_SIP_Cause_404_Not_Found                             = 404,
    VoLTE_Event_SIP_Cause_405_Method_Not_Allowed                    = 405,
    VoLTE_Event_SIP_Cause_406_Not_Acceptable                        = 406,
    VoLTE_Event_SIP_Cause_407_Proxy_Authentication_Required         = 407,
    VoLTE_Event_SIP_Cause_408_Request_Timeout                       = 408,
    VoLTE_Event_SIP_Cause_410_Gone                                  = 410,
    VoLTE_Event_SIP_Cause_413_Request_Entity_Too_Large              = 413,
    VoLTE_Event_SIP_Cause_414_Request_URI_Too_Large                 = 414,
    VoLTE_Event_SIP_Cause_415_Unsupported_Media_Type                = 415,
    VoLTE_Event_SIP_Cause_416_Unsupported_URI_Scheme                = 416,
    VoLTE_Event_SIP_Cause_420_Bad_Extension                         = 420,
    VoLTE_Event_SIP_Cause_421_Extension_Required                    = 421,
    VoLTE_Event_SIP_Cause_423_Interval_Too_Brief                    = 423,
    VoLTE_Event_SIP_Cause_480_Temporarily_Not_Available             = 480,
    VoLTE_Event_SIP_Cause_481_Call_Leg_Transaction_Does_Not_Exist   = 481,
    VoLTE_Event_SIP_Cause_482_Loop_Detected                         = 482,
    VoLTE_Event_SIP_Cause_483_Too_Many_Hops                         = 483,
    VoLTE_Event_SIP_Cause_484_Address_Incomplete                    = 484,
    VoLTE_Event_SIP_Cause_485_Ambiguous                             = 485,
    VoLTE_Event_SIP_Cause_486_Busy_Here                             = 486,
    VoLTE_Event_SIP_Cause_487_Request_Terminated                    = 487,
    VoLTE_Event_SIP_Cause_488_Not_Acceptable_Here                   = 488,
    VoLTE_Event_SIP_Cause_491_Request_Pending                       = 491,
    VoLTE_Event_SIP_Cause_493_Undecipherable                        = 493,
    VoLTE_Event_SIP_Cause_494_Security_Agreement_Required           = 494,
    VoLTE_Event_SIP_Cause_500_SERVER_INT_ERROR                      = 500,
    VoLTE_Event_SIP_Cause_501_Not_Implemented                       = 501,
    VoLTE_Event_SIP_Cause_502_Bad_Gateway                           = 502,
    VoLTE_Event_SIP_Cause_503_Service_Unavailable                   = 503,
    VoLTE_Event_SIP_Cause_504_Server_Time_Out                       = 504,
    VoLTE_Event_SIP_Cause_505_SIP_Version_Not_Supported             = 505,
    VoLTE_Event_SIP_Cause_513_Message_Too_Large                     = 513,
    VoLTE_Event_SIP_Cause_580_Precondition_Failure                  = 580,
    VoLTE_Event_SIP_Cause_600_Busy_Everywhere                       = 600,
    VoLTE_Event_SIP_Cause_603_Decline                               = 603,
    VoLTE_Event_SIP_Cause_604_Does_Not_Exist_Anywhere               = 604,
    VoLTE_Event_SIP_Cause_606_Not_Acceptable                        = 606,

    VoLTE_Event_SIP_Cause_UNKNOWN                                   = 700,
    VoLTE_Event_SIP_Cause_Max                                       = VoLTE_Event_SIP_Cause_UNKNOWN,
};


enum VoLTE_Event_Reg_Cause_e {
    VoLTE_Event_Reg_Cause_Normal                                = 0,
    VoLTE_Event_Reg_Cause_Internal_Error                        = 1,
    VoLTE_Event_Reg_Cause_Stack_Error                           = 2,
    VoLTE_Event_Reg_Cause_Timer_Error                           = 3,
    VoLTE_Event_Reg_Cause_Try_Next_PCSCF                        = 4,
    VoLTE_Event_Reg_Cause_PCSCF_All_Failed                      = 5,
    VoLTE_Event_Reg_Cause_Auth_Error                            = 6,
    VoLTE_Event_Reg_Cause_Reset                                 = 7,
    VoLTE_Event_Reg_Cause_AssURI_Changed                        = 8,
    VoLTE_Event_Reg_Cause_Network_initiated                     = 10,
    VoLTE_Event_Reg_Cause_Channel_Bind                          = 11,
    VoLTE_Event_Reg_Cause_OOS                                   = 12,
    VoLTE_Event_Reg_Cause_SIP_Error                             = 13,
    VoLTE_Event_Reg_Cause_Parameter_Error                       = 14,
    VoLTE_Event_Reg_Cause_Not_Bind                              = 15,
    VoLTE_Event_Reg_Cause_Not_Auto_Re_Reg                       = 16,
    VoLTE_Event_Reg_Cause_Retry_After                           = 17,
    VoLTE_Event_Reg_Cause_IMS_PDN_Fatal_Fail                    = 18,
    VoLTE_Event_Reg_Cause_MD_Lower_Layer_Err                    = 19,
    VoLTE_Event_Reg_Cause_C2K_Fail                              = 20,
    VoLTE_Event_Reg_Cause_Retry_By_RFC5626                      = 21,
    /*add here*/
    VoLTE_Event_Reg_Cause_UNKNOWN                               = 99,
    VoLTE_Event_Reg_Cause_Max                                   = VoLTE_Event_Reg_Cause_UNKNOWN,
};

enum VoLTE_Event_Ems_Mode_e {
    VoLTE_Event_Ems_Mode_None = 0,
    VoLTE_Event_Ems_Mode_Nospec,
    VoLTE_Event_Ems_Mode_3gpp,
    VoLTE_Event_Ems_Mode_Wlan,
    VoLTE_Event_Ems_Mode_eHRPD,
    VoLTE_Event_Ems_Mode_Max
};

#endif //_IMS_INTERFACE_AP_H


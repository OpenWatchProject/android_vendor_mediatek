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

#ifndef RIL_SS_H
#define RIL_SS_H 1

/* USSD messages using the default alphabet are coded with the GSM 7-bit default alphabet         *
 * given in clause 6.2.1. The message can then consist of up to 182 user characters (3GPP 23.038) */
#define MAX_RIL_USSD_STRING_LENGTH 255

//BEGIN mtk08470 [20130109][ALPS00436983]
// number string cannot more than MAX_RIL_USSD_NUMBER_LENGTH digits
#define MAX_RIL_USSD_NUMBER_LENGTH 160
//END mtk08470 [20130109][ALPS00436983]

#define SETTING_QUERY_CFU_TYPE "persist.vendor.radio.cfu.querytype"
#define BUF_MAX_LEN 40
#define UNUSED(x) (x)   //eliminate "warning: unused parameter"

/*
 * <classx> is a sum of integers each representing a class of information (default 7):
 * 1    voice (telephony)
 * 2    data (refers to all bearer services; with <mode>=2 this may refer only to some bearer service if TA does not support values 16, 32, 64 and 128)
 * 4    fax (facsimile services)
 * 8    short message service
 * 16   data circuit sync
 * 32   data circuit async
 * 64   dedicated packet access
 * 128  dedicated PAD access
 */
typedef enum {
    CLASS_NONE                      = 0,
    CLASS_VOICE                     = 1,
    CLASS_DATA                      = 2,
    CLASS_FAX                       = 4,
    CLASS_DEFAULT                   = 7,
    CLASS_SMS                       = 8,
    CLASS_DATA_SYNC                 = 16,
    CLASS_DATA_ASYNC                = 32,
    CLASS_DEDICATED_PACKET_ACCESS   = 64,
    CLASS_DEDICATED_PAD_ACCESS      = 128,
    CLASS_MTK_LINE2                 = 256,
    CLASS_MTK_VIDEO                 = 512
} AtInfoClassE;

typedef enum {
    BS_ALL_E                        = 0,
    BS_TELE_ALL_E                   = 10,
    BS_TELEPHONY_E                  = 11,
    BS_TELE_DATA_ALL_E              = 12,
    BS_TELE_FAX_E                   = 13,
    BS_TELE_SMS_E                   = 16,
    BS_TELE_VGCS_E                  = 17, /* Not supported by framework */
    BS_TELE_VBS_E                   = 18, /* Not supported by framework */
    BS_TELE_ALL_EXCEPT_SMS_E        = 19,
    BS_DATA_ALL_E                   = 20,
    BS_DATA_ASYNC_ALL_E             = 21,
    BS_DATA_SYNC_ALL_E              = 22,
    BS_DATA_CIRCUIT_SYNC_E          = 24,
    BS_DATA_CIRCUIT_ASYNC_E         = 25,
    BS_DATA_SYNC_TELE_E             = 26, /* Supported by framework */
    BS_GPRS_ALL_E                   = 99
} BsCodeE;

/***
 * "AO"    BAOC (Barr All Outgoing Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OI"    BOIC (Barr Outgoing International Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OX"    BOIC exHC (Barr Outgoing International Calls except to Home Country) (refer 3GPP TS 22.088 [6] clause 1)
 * "AI"    BAIC (Barr All Incoming Calls) (refer 3GPP TS 22.088 [6] clause 2)
 * "IR"    BIC Roam (Barr Incoming Calls when Roaming outside the home country) (refer 3GPP TS 22.088 [6] clause 2)
 * "AB"    All Barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AG"    All outGoing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AC"    All inComing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 */

typedef enum {
    CB_BAOC,
    CB_BOIC,
    CB_BOIC_EXHC,
    CB_BAIC,
    CB_BIC_ROAM,
    CB_ABS,
    CB_AOBS,
    CB_AIBS,
    CB_SUPPORT_NUM
} CallBarServicesE;

/*
 * 0   unconditional call forwarding is active
 * 1   some of the conditional call forwardings are active
 * 2   call has been forwarded
 * 3   call is waiting
 * 4   this is a CUG call (also <index> present)
 * 5   outgoing calls are barred
 * 6   incoming calls are barred
 * 7   CLIR suppression rejected
 * 8   call has been deflected
*/
typedef enum {
    CODE_IMS_MO_UNCONDITIONAL_CF_ACTIVE = 100,
    CODE_IMS_MO_SOME_CF_ACTIVE,              //101
    CODE_IMS_MO_CALL_FORWARDED,              //102
    CODE_IMS_MO_CALL_IS_WAITING,             //103
    CODE_IMS_MO_CUG_CALL,                    //104
    CODE_IMS_MO_OUTGOING_CALLS_BARRED,       //105
    CODE_IMS_MO_INCOMING_CALLS_BARRED,       //106
    CODE_IMS_MO_CLIR_SUPPRESSION_REJECTED,   //107
    CODE_IMS_MO_CALL_DEFLECTED               //108
} NotificationCodeImsMoE;

/*
 * 0   this is a forwarded call (MT call setup)
 * 1   this is a CUG call (also <index> present) (MT call setup)
 * 2   call has been put on hold (during a voice call)
 * 3   call has been retrieved (during a voice call)
 * 4   multiparty call entered (during a voice call)
 * 5   call on hold has been released (this is not a SS notification) (during a voice call)
 * 6   forward check SS message received (can be received whenever)
 * 7   call is being connected (alerting) with the remote party in alerting state in explicit call transfer operation (during a voice call)
 * 8   call has been connected with the other remote party in explicit call transfer operation (also number and subaddress parameters may be present) (during a voice call or MT call setup)
 * 9   this is a deflected call (MT call setup)
 * 10  additional incoming call forwarded
 * 11  MT is a forwarded call (CF)
 * 12  MT is a forwarded call (CFU)
 * 13  MT is a forwarded call (CFC)
 * 14  MT is a forwarded call (CFB)
 * 15  MT is a forwarded call (CFNRy)
 * 16  MT is a forwarded call (CFNRc)
*/
typedef enum {
    CODE_IMS_MT_FORWARDED_CALL = 100,
    CODE_IMS_MT_CUG_CALL,                   //101
    CODE_IMS_MT_CALL_ON_HOLD,               //102
    CODE_IMS_MT_CALL_RETRIEVED,             //103
    CODE_IMS_MT_MULTI_PARTY_CALL,           //104
    CODE_IMS_MT_ON_HOLD_CALL_RELEASED,      //105
    CODE_IMS_MT_FORWARD_CHECK_RECEIVED,     //106
    CODE_IMS_MT_CALL_CONNECTING_ECT,        //107
    CODE_IMS_MT_CALL_CONNECTED_ECT,         //108
    CODE_IMS_MT_DEFLECTED_CALL,             //109
    CODE_IMS_MT_ADDITIONAL_CALL_FORWARDED,  //110
    CODE_IMS_MT_FORWARDED_CF,               //111
    CODE_IMS_MT_FORWARDED_CF_UNCOND,        //112
    CODE_IMS_MT_FORWARDED_CF_COND,          //113
    CODE_IMS_MT_FORWARDED_CF_BUSY,          //114
    CODE_IMS_MT_FORWARDED_CF_NO_REPLY,      //115
    CODE_IMS_MT_FORWARDED_CF_NOT_REACHABLE  //116
} NotificationCodeImsMtE;

int rilSsMain(int request, void *data, size_t datalen, RIL_Token t);
int rilSsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx *p_channel);
void requestCallBarring(void *data, size_t datalen, RIL_Token t);
void onLteSuppSvcNotification(char *s, RIL_SOCKET_ID rid);
void retrieveCallForwardedToNumber(char *rawString, char *number);
void updateCFUQueryType(const char *cmd);
void onCRINGReceived();
void onECPI133Received();
const char *InfoClassToMmiBSCodeString (int infoClass);
const char *callBarFacToServiceCodeStrings(const char *fac);
extern RIL_RadioState getRadioState(RIL_SOCKET_ID rid);
extern SIM_Status getSIMStatus(RIL_SOCKET_ID rid);
extern int checkUserLoad();

#endif /* RIL_SS_H */
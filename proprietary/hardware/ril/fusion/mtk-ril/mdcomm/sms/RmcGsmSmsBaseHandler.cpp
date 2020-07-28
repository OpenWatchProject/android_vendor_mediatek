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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RmcGsmSmsBaseHandler.h"
#include <telephony/mtk_ril.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include "RmcGsmSmsDefs.h"
#include "RfxRilUtils.h"

using ::android::String8;

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RmcGsmSmsBaseHandler::RmcGsmSmsBaseHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    setTag(String8("RmcGsmSmsBase"));
}

int RmcGsmSmsBaseHandler::smsPackPdu(const char* smsc, const char *tpdu,
        char *pdu, bool check) {
    int len=0;
    TPDU_ERROR_CAUSE_ENUM error_cause = TPDU_NO_ERROR;
    int i=0;

    //Check SMSC
    if (smsc == NULL) {
        // "NULL for default SMSC"
        smsc= "00";
    } else {
        len = strlen(smsc);
        if ( (len%2) != 0 || len<2 || (len/2-1) > MAX_SMSC_LENGTH) {
            logE(mTag, "The length of smsc is not valid: %d", len);
            return -1;
        }
    }

    // Check PDU
    if (tpdu == NULL) {
        logE(mTag, "PDU should not be NULL");
        return -1;
    }
    len = strlen(tpdu);
    if ( (len%2) != 0) {
        logE(mTag, "The length of TPDU is not valid: %d", len);
        return -1;
    }
    len /= 2;
    if ( len > MAX_TPDU_LENGTH) {
        logE(mTag, "The length of TPDU is too long: %d", len);
        return -1;
    }

    sprintf(pdu, "%s%s", smsc, tpdu);

    logD(mTag, "pdu len=%d", (int)strlen(pdu));

    if (check) {
        if (smsSubmitPduCheck(strlen(pdu), pdu, &error_cause) == false) {
            logE(mTag, "smsPackPdu error, error_cause = %d", error_cause);
            return -1;
        }
    }

    return len;
}

/*****************************************************************************
* FUNCTION
*  smsFoCheck
* DESCRIPTION
*   This function check the Fist Octet.
*
* PARAMETERS
*  a  IN       fo (first octet)
* RETURNS
*  true:  fo is valid
*  false: fo is invalid
* GLOBALS AFFECTED
*  none
*****************************************************************************/
bool RmcGsmSmsBaseHandler::smsFoCheck(int fo) {
    /* Check Message Type Identifier */
    if ((fo & TPDU_MTI_BITS) >= TPDU_MTI_RESERVED)
        return false;

    /* Check Validity Period , currently only relative format supported */
    if ((((fo & TPDU_VPF_BITS) >> 3) != TPDU_VPF_NOT_PRESENT) &&
        (((fo & TPDU_VPF_BITS) >> 3) != TPDU_VPF_RELATIVE))
        return false;

    return true;
}

/*****************************************************************************
* FUNCTION
*  smsPidCheck
* DESCRIPTION
*   This function checks the Protocol IDentifier.
*
* PARAMETERS
*  a  IN       pid
*  b  IN/OUT   *fail_cause_ptr
*  c  IN/OUT   *aux_info, useful for caller
* RETURNS
*  true:  pid is valid
*  false: pid is invalid
* GLOBALS AFFECTED
*  none
*****************************************************************************/
bool RmcGsmSmsBaseHandler::smsPidCheck(int pid) {
    bool ret_val = true;
    logD(mTag, "smsPidCheck, pid=%d", pid);

    if (pid != TPDU_PID_DEFAULT_PID) {
        /*
         * Check for reserved and SC specific use of
         * * PID value
         */
        if (((pid & TPDU_PID_CHECK) == TPDU_PID_CHECK) ||
            ((pid & TPDU_PID_CHECK) == TPDU_PID_RESERVED)) {
            ret_val = false;
        }
    }

    return ret_val;
}

/*****************************************************************************
* FUNCTION
*  smsDcsCheck
* DESCRIPTION
*   This function checks the data coding scheme.
*
* PARAMETERS
*  a  IN       dcs
*  b  IN/OUT   *fail_cause_ptr
* RETURNS
*  KAL_TRUE:  dcs is valid
*  KAL_FALSE: dcs is invalid
* GLOBALS AFFECTED
*  none
*****************************************************************************/
bool RmcGsmSmsBaseHandler::smsDcsCheck(int dcs) {
    bool ret_val = true;
    unsigned char coding_group;
    logD(mTag, "smsDcsCheck, dcs=%d", dcs);

    if (dcs == 0)                       /* Default DCS */
        return true;

    coding_group = dcs & 0xf0;

    /* bit7..bit4=1111: Data coding/message class */
    if ((dcs & TPDU_DCS_CODING2) == TPDU_DCS_CODING2) {
        /* Bit3 is reserved, shall equals to zero */
        if ((dcs & TPDU_DCS_RESERVE_BIT) == TPDU_DCS_RESERVE_BIT) {
            ret_val = false;
        }
    } else if ((dcs & TPDU_DCS_CODING1) == 0x00) {
        /* bit7..bit4=00xx: General Data coding indication */
        /* bit 3 and bit 2 are reserved */
        if ((dcs & TPDU_DCS_ALPHABET_CHECK) == TPDU_DCS_ALPHABET_CHECK) {
            ret_val = false;
        } else if ((dcs & TPDU_DCS_COMPRESS_CHECK) == TPDU_DCS_COMPRESS_CHECK) {
            /* Currently, not support compression */
            ret_val = false;
        }
    } else if ((coding_group & 0xc0) == 0x80) {
        ret_val = false;
    } else if ((coding_group == 0xc0) || (coding_group == 0xd0) || (coding_group == 0xe0)) {
        /* Bit2 is reserved, shall equals to zero */
        if ((dcs & 0x04) == 0x04) {
            ret_val = false;
        }
    }

    return ret_val;
}

/*****************************************************************************
* FUNCTION
*  smsDecodeDcs
* DESCRIPTION
*   This function decodes the data coding scheme.
*
* PARAMETERS
*  a  IN       dcs
*  b  IN/OUT   alphabet_type
*  c  IN/OUT   msg_class
*  d  IN/OUT   compress
* RETURNS
*  none
* GLOBALS AFFECTED
*  none
*****************************************************************************/
void RmcGsmSmsBaseHandler::smsDecodeDcs(int dcs, SMS_ENCODING_ENUM *alphabet_type,
        SMS_MESSAGE_CLASS_ENUM *msg_class, bool *is_compress) {
    unsigned char coding_group;

    /* Default DCS value */
    *alphabet_type = SMS_ENCODING_7BIT;
    *msg_class = SMS_MESSAGE_CLASS_UNSPECIFIED;
    *is_compress = false;

    if (dcs == 0)
        return;

    if (dcs == 0x84) {
        *alphabet_type = SMS_ENCODING_16BIT;
        return;
    }

    coding_group = dcs >> 4;

    if (coding_group == 0x0f) {
        /* Data Coding/Message Class */
        /*
         * SCR: 2001,
         * * ensure the reserved bit is zero
         */
        if ((dcs & 0x08) == 0x08) {
            *alphabet_type = SMS_ENCODING_UNKNOWN;
        } else {
            *msg_class = (SMS_MESSAGE_CLASS_ENUM) (dcs & 0x03);
            *alphabet_type = (SMS_ENCODING_ENUM) ((dcs & 0x04) >> 2);
        }
    } else if ((coding_group | 0x07) == 0x07) {     /* General Data Coding Scheme */
        if ((dcs & 0x10) == 0x10)
            *msg_class = (SMS_MESSAGE_CLASS_ENUM) (dcs & 0x03);
        else
            *msg_class = SMS_MESSAGE_CLASS_UNSPECIFIED;

        *alphabet_type = (SMS_ENCODING_ENUM) ((dcs >> 2) & 0x03);
        if (((coding_group & 0x02) >> 1) == 1)
            *is_compress = true;
    } else if ((coding_group & 0x0c) == 0x08) {
        /*
         * according GSM 23.038 clause 4:
         * "Any reserved codings shall be assumed to be the GSM 7 bit default alphabet."
         */
    } else if (((coding_group & 0x0f) == 0x0c) || /* discard */
             ((coding_group & 0x0f) == 0x0d) || /* store, gsm-7 */
             ((coding_group & 0x0f) == 0x0e))   /* store, ucs2 */ {
        /* 1110: msg wait ind (store, ucs2) */
        if ((coding_group & 0x0f) == 0x0e)
            *alphabet_type = SMS_ENCODING_16BIT;
    }

    /*
     * if the reserved bit been set or the alphabet_type uses the reserved one,
     * then according GSM 23.038 clause 4:
     * "Any reserved codings shall be assumed to be the GSM default alphabet."
     * we change it as SMSAL_GSM7_BIT
     */

    if (*alphabet_type == SMS_ENCODING_UNKNOWN) {
        *alphabet_type = SMS_ENCODING_7BIT;
    }

}

/*****************************************************************************
* FUNCTION
*  smsIsLenIn8bit
* DESCRIPTION
*   This function determines the unit of TP-User-Data.
*
* PARAMETERS
*  a  IN          dcs
* RETURNS
*  KAL_TRUE: length in octet
*  KAL_TRUE: length in septet
* GLOBALS AFFECTED
*  none
*****************************************************************************/
bool RmcGsmSmsBaseHandler::smsIsLenIn8bit(int dcs) {
    bool is_compress;
    SMS_ENCODING_ENUM alphabet_type;
    SMS_MESSAGE_CLASS_ENUM mclass;

    smsDecodeDcs(dcs, &alphabet_type, &mclass, &is_compress);

    if ((is_compress == true) ||
        (alphabet_type == SMS_ENCODING_8BIT) ||
        (alphabet_type == SMS_ENCODING_16BIT)) {
        return true;
    } else {
        return false;
    }

}

unsigned int RmcGsmSmsBaseHandler::smsMsgLenInOctet(int dcs, int len) {
    if (smsIsLenIn8bit(dcs)) {
        return len;
    } else {
        return (len * 7 + 7) / 8;
    }
}

int RmcGsmSmsBaseHandler::smsHexCharToDecInt(char *hex, int length) {
    int i = 0;
    int value, digit;

    for (i = 0, value = 0; i < length && hex[i] != '\0'; i++) {
        if (hex[i]>='0' && hex[i]<='9') {
            digit = hex[i] - '0';
        } else if ( hex[i]>='A' && hex[i] <= 'F') {
            digit = hex[i] - 'A' + 10;
        } else if ( hex[i]>='a' && hex[i] <= 'f') {
            digit = hex[i] - 'a' + 10;
        } else {
            return -1;
        }
        value = value*16 + digit;
    }

    return value;
}

bool RmcGsmSmsBaseHandler::smsSubmitPduCheck(int pdu_len, char *pdu_ptr,
        TPDU_ERROR_CAUSE_ENUM  *error_cause) {
    unsigned char off = 0;                  /* offset */
    int dcs;
    int fo;
    unsigned char udl, user_data_len;
    int i=0;
    int pdu_array[pdu_len/2];

    *error_cause = TPDU_NO_ERROR;

    /* check length */
    if (pdu_len/2 > TPDU_MAX_TPDU_SIZE || pdu_len == 0) {
        logD(mTag, "smsSubmitPduCheck pdu length check fail, pdu_len=%d", pdu_len);
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    for (i = 0; i < pdu_len/2 ;i ++) {
        pdu_array[i] = smsHexCharToDecInt((pdu_ptr+2*i), 2);
    }
    /* check sca */
    if (pdu_array[0] > TPDU_MAX_ADDR_LEN) {
        logD(mTag, "SMSC address length check fail, len =%d", pdu_array[0]);
        return false;
    }

    off += (1 + pdu_array[0]);              /* move to head of TPDU (skip sca_len, sca) */

    /* check this pdu is SMS-SUBMIT or not */
    if ((pdu_array[off] & TPDU_MTI_BITS) != TPDU_MTI_SUBMIT) {
        logE(mTag, "TPDU_MTI_SUBMIT fail, First Octet =%d, off=%d", pdu_array[off], off);
        return false;
    }

    /* check fo */
    fo = pdu_array[off];
    if (smsFoCheck(pdu_array[off]) == false) {
        logE(mTag, "smsFoCheck fail");
        return false;
    }

    off += 2;                           /* move to da, (skip fo and msg_ref) */

    /* check da */
    if (pdu_array[off] > (TPDU_MAX_ADDR_LEN - 1) * 2) {
        logE(mTag, "DA length check fail, DA length =%d, off=%d", pdu_array[off], off);
        return false;
    }

    off += 1 + 1 + ((pdu_array[off]+1)/2);    /* skip addr_len, addr digits & addr type */

    /* check pid */
    if (smsPidCheck(pdu_array[off]) == false) {
        logE(mTag, "smsPidCheck check fail, pid =%d, off=%d", pdu_array[off], off);
        return false;
    }

    off++;                              /* move to dcs */

    /* check dcs */
    dcs = pdu_array[off];
    if (smsDcsCheck(dcs) == false) {
        logE(mTag, "rild_sms_dcs_check check fail, dcs =%d, off=%d", dcs, off);
        return false;
    }

    /* move to udl */
    if (((fo & TPDU_VPF_BITS) >> 3) == TPDU_VPF_RELATIVE)
        off += 2;                       /* VP is relative format (1 byte) */
    else
        off++;                          /* VP not present */


    /* udl */
    logD(mTag, "udl=%d, off=%d", pdu_array[off], off);
    udl = pdu_array[off];

    /* move to user data */
    off++;

    user_data_len = (unsigned char) smsMsgLenInOctet(dcs, udl);

    if (user_data_len > TPDU_ONE_MSG_OCTET || (user_data_len != (pdu_len/2 - off))) {
        logD(mTag, "user data length check fail, user_data_len=%d", user_data_len);
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    return true;

}

bool RmcGsmSmsBaseHandler::smsDeliverPduCheck(int pdu_len, char *pdu,
        TPDU_ERROR_CAUSE_ENUM *error_cause) {
    int off = 0;                  /* offset */
    int dcs;
    unsigned char udl, user_data_len;
    int pdu_array[pdu_len/2];
    int i=0;

    *error_cause = TPDU_NO_ERROR;

    /* check length */
    if (pdu_len/2 > TPDU_MAX_TPDU_SIZE || pdu_len == 0) {
        logD(mTag, "smsDeliverPduCheck pdu length fail, pdu_len=%d", pdu_len);
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    for (i = 0; i < pdu_len/2 ;i ++) {
        pdu_array[i] = smsHexCharToDecInt((pdu+2*i), 2);
    }

    if (pdu_array[0] > TPDU_MAX_ADDR_LEN) {
        logD(mTag, "SMSC address length check fail, len =%d", pdu_array[0]);
        return false;
    }

    off += (1 + pdu_array[0]);              /* move to head of TPDU (skip sca_len, sca) */

    /* check this pdu is SMS-DELIVER or not */
    if (((pdu_array[off] & TPDU_MTI_BITS) != TPDU_MTI_DELIVER) &&
        ((pdu_array[off] & TPDU_MTI_BITS) != TPDU_MTI_UNSPECIFIED)) {
        logE(mTag, "first octet check fail, first octet =%d", pdu_array[off]);
        return false;
    }

    off += 1;                           /* move to oa, (skip fo ) */

    /* check oa */
    if (pdu_array[off] > (TPDU_MAX_ADDR_LEN - 1) * 2) {
        logE(mTag, "OA length check fail, OA length =%d", pdu_array[off]);
        return false;
    }

    if ((pdu_array[off] % 2) >0)
        off += 1 + 1 + ((pdu_array[off]+1)/2);    /* skip addr_len, addr digits & addr type */
    else
        off += 1 + 1 + (pdu_array[off]/2);    /* skip addr_len, addr digits & addr type */

    off++;                              /* skip pid, move to dcs */

    /* check dcs */
    dcs = pdu_array[off];
    logD(mTag, "dcs =%d", pdu_array[off]);

    off += (1 + 7);                     /* skip dcs, move to uhl */

    /* udl */
    logD(mTag, "udl =%d", pdu_array[off]);
    udl = pdu_array[off];

    /* move to user data */
    off++;

    user_data_len = (unsigned char) smsMsgLenInOctet(dcs, udl);

    if (user_data_len > TPDU_ONE_MSG_OCTET || (user_data_len != (pdu_len/2 - off))) {
        logD(mTag, "user data length check fail, user_data_len=%d", user_data_len);
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    return true;

}

void RmcGsmSmsBaseHandler::showCurrIncomingSmsType() {
    int smsType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE,
            SMS_INBOUND_NONE);

    if (smsType == SMS_INBOUND_NONE) {
        logD(mTag, "current sms_type = SMS_INBOUND_NONE");
    } else if (smsType == SMS_INBOUND_3GPP_CMT) {
        logD(mTag, "current sms_type = SMS_INBOUND_3GPP_CMT");
    } else if (smsType == SMS_INBOUND_3GPP_CMTI) {
        logD(mTag, "current sms_type = SMS_INBOUND_3GPP_CMTI");
    } else if (smsType == SMS_INBOUND_3GPP_CDS) {
        logD(mTag, "current sms_type = SMS_INBOUND_3GPP_CDS");
    } else {
        logD(mTag, "current sms_type = UNKNOWN");
    }
}

int RmcGsmSmsBaseHandler::atGetCmsError(const sp<RfxAtResponse> p_response) {
    int ret;
    int err;
    RfxAtLine *p_cur;

    if (p_response->getSuccess() > 0) {
        return CMS_SUCCESS;
    }

    if (p_response->getFinalResponse() == NULL) {
        return CMS_ERROR_NON_CMS;
    }

    if( strStartsWith(p_response->getFinalResponse()->getLine(), "ERROR" ) ) {
        return CMS_UNKNOWN;
    }

    if( !strStartsWith(p_response->getFinalResponse()->getLine(), "+CMS ERROR:" ) ) {
        return CMS_ERROR_NON_CMS;
    }

    p_cur = p_response->getFinalResponse();
    p_cur->atTokStart(&err);

    if (err < 0) {
        return CMS_ERROR_NON_CMS;
    }

    ret = p_cur->atTokNextint(&err);

    if (err < 0) {
        return CMS_ERROR_NON_CMS;
    }

    return ret;
}

int RmcGsmSmsBaseHandler::smsCheckReceivedPdu(int length, char* pdu, bool check) {
    int pdu_length;
    int sca_length;
    TPDU_ERROR_CAUSE_ENUM  error_cause = TPDU_NO_ERROR;
    int i=0;

    pdu_length = strlen(pdu);
    if (pdu_length < 2) {
        logW(mTag, "The PDU structure might be wrong");
        return -1;
    }

    sca_length = smsHexCharToDecInt(pdu, 2);
    if (sca_length < 0) {
        logW(mTag, "The PDU structure might be wrong with invalid digits: %d", sca_length);
        return -1;
    }

    if (check) {
        if (smsDeliverPduCheck(pdu_length, pdu, &error_cause) == false) {
            logE(mTag, "rild_sms_deliver_pdu_check error, error_cause = %d", error_cause);
            return -1;
        }
    }

    /***************************************************
     * It's does not need to check this part due to ..
     * 1. rild_sms_deliver_pdu_check has already checked
     *    the legnth of this pdu
     * 2. Use emulator to receive a SMS, the length is
     *    hardcord and wrong on +CMT commands.
     *    It will never receive any MT SMS from emulator.
     *    It needs to fix on emulator coding and takes
     *    time.
     ***************************************************/
    pdu_length -= (sca_length + 1)*2;
    if (pdu_length != length*2) {
        // Still print the log for debug usage
        logW(mTag, "The specified TPDU length is not matched to the TPDU data:  %d", length);
    }

    return 0;
}

int RmcGsmSmsBaseHandler::hexCharToInt(char c) {
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);

    return 0;
}

void RmcGsmSmsBaseHandler::hexStringToBytes(const char *in, int inLength, char *out,
        int outLength) {
    int i;

    if (in == NULL || out == NULL) {
        return;
    }

    if (inLength != outLength * 2) {
        return;
    }

    for (i = 0 ; i < inLength ; i += 2) {
        out[i/2] = (char)((hexCharToInt(in[i]) << 4) | hexCharToInt(in[i+1]));
    }
}


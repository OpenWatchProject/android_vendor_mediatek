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

#include <telephony/mtk_ril.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include <cutils/properties.h>

#include <ril_callbacks.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "RIL-SMS"
#endif


#include <log/log.h>

enum{
    SMS_NONE,
    SMS_IMS,
    SMS_3G
}SMS_TYPE;

/* TODO
* The following constant variables also defined in the ril_callbacks.c
* We should figure out a method to merge them
*/
#ifdef MTK_RIL
static const RIL_SOCKET_ID s_pollSimId = RIL_SOCKET_1;
static const RIL_SOCKET_ID s_pollSimId2 = RIL_SOCKET_2;
static const RIL_SOCKET_ID s_pollSimId3 = RIL_SOCKET_3;
static const RIL_SOCKET_ID s_pollSimId4 = RIL_SOCKET_4;
#endif /* MTK_RIL */
static const struct timeval TIMEVAL_0 = {2,0};

// default set as available
int phone_storage_status = 0;
int sms_type = SMS_NONE;
/*
 * This is defined on RILD and use to know MD is off or on.
 * In flight mode, it will turns off MD if feature option is ON
*/
extern int s_md_off;

static int checkSimPresent(RIL_Token t) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int present = isSimInserted(rid);
    RLOGD("checkSimPresent %d", present);
    return present;
}

void showCurrIncomingSmsType() {
    if (sms_type == SMS_NONE) {
        LOGD("current sms_type = SMS_NONE");
    } else if (sms_type == SMS_IMS) {
        LOGD("current sms_type = SMS_IMS");
    } else if (sms_type == SMS_3G) {
        LOGD("current sms_type = SMS_3G");
    } else {
        LOGD("current sms_type = UNKNOWN");
    }
}

/****************************************
* Temporaily, we didn't return a enum for cms ERROR
* because there are too much error casue we can't expected from modem
*
****************************************/
static int at_get_cms_error(const ATResponse *p_response) {
    int ret;
    int err;
    char *p_cur;

    if (p_response->success > 0) {
        return CMS_SUCCESS;
    }

    if (p_response->finalResponse == NULL) {
        return CMS_ERROR_NON_CMS;
    }

    if( strStartsWith(p_response->finalResponse, "ERROR" ) ) {
        return CMS_UNKNOWN;
    }

    if( !strStartsWith(p_response->finalResponse, "+CMS ERROR:" ) ) {
        return CMS_ERROR_NON_CMS;
    }

    p_cur = p_response->finalResponse;
    err = at_tok_start(&p_cur);

    if (err < 0) {
        return CMS_ERROR_NON_CMS;
    }

    err = at_tok_nextint(&p_cur, &ret);

    if (err < 0) {
        return CMS_ERROR_NON_CMS;
    }

    return ret;
}


int rild_sms_hexCharToDecInt(char *hex, int length) {
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

/*****************************************************************************
* FUNCTION
*  rild_sms_fo_check
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
bool rild_sms_fo_check(int fo) {
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
*  rild_sms_pid_check
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
bool rild_sms_pid_check(int pid) {
    bool ret_val = true;
    LOGD("rild_sms_submit_pdu_check, pid=%d", pid);

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
*  rild_sms_dcs_check
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
bool rild_sms_dcs_check(int dcs) {
    bool ret_val = true;
    unsigned char coding_group;
    LOGD("rild_sms_submit_pdu_check, dcs=%d", dcs);

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
*  rild_sms_decode_dcs
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
void rild_sms_decode_dcs(int             dcs,
                      SMS_ENCODING_ENUM         *alphabet_type,
                      SMS_MESSAGE_CLASS_ENUM    *msg_class,
                      bool                   *is_compress) {
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
    } else if ((coding_group | 0x07) == 0x07) {
        /* General Data Coding Scheme */
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
             ((coding_group & 0x0f) == 0x0e))   /* store, ucs2 */{
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
*  rild_sms_is_len_in8bit
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
bool rild_sms_is_len_in8bit(int dcs) {
    bool is_compress;
    SMS_ENCODING_ENUM alphabet_type;
    SMS_MESSAGE_CLASS_ENUM mclass;

    rild_sms_decode_dcs(dcs, &alphabet_type, &mclass, &is_compress);

    if ((is_compress == true) ||
        (alphabet_type == SMS_ENCODING_8BIT) ||
        (alphabet_type == SMS_ENCODING_16BIT)) {
        return true;
    } else {
        return false;
    }

}

unsigned int rild_sms_msg_len_in_octet(int dcs, int len) {
    if (rild_sms_is_len_in8bit(dcs)) {
        return len;
    } else {
        return (len * 7 + 7) / 8;
    }
}

bool rild_sms_submit_pdu_check(int pdu_len, char *pdu_ptr, TPDU_ERROR_CAUSE_ENUM  *error_cause) {
    unsigned char off = 0;                  /* offset */
    int dcs;
    int fo;
    unsigned char udl, user_data_len;
    int i=0;
    int pdu_array[pdu_len/2];

    *error_cause = TPDU_NO_ERROR;
    LOGD("rild_sms_submit_pdu_check, pdu_len=%d", pdu_len);

    /* check length */
    if (pdu_len/2 > TPDU_MAX_TPDU_SIZE || pdu_len == 0) {
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    for (i = 0; i < pdu_len/2 ;i ++) {
        pdu_array[i] = rild_sms_hexCharToDecInt((pdu_ptr+2*i), 2);
    }
    /* check sca */
    LOGD("SMSC address len =%d", pdu_array[0]);
    if (pdu_array[0] > TPDU_MAX_ADDR_LEN)
        return false;

    off += (1 + pdu_array[0]);              /* move to head of TPDU (skip sca_len, sca) */

    /* check this pdu is SMS-SUBMIT or not */
    LOGD("First Octet =%d, off=%d", pdu_array[off], off);
    if ((pdu_array[off] & TPDU_MTI_BITS) != TPDU_MTI_SUBMIT)
    {
        LOGD("TPDU_MTI_SUBMIT fail");
        return false;
    }

    /* check fo */
    fo = pdu_array[off];
    if (rild_sms_fo_check(pdu_array[off]) == false) {
        LOGD("rild_sms_fo_check fail");
        return false;
    }

    off += 2;                           /* move to da, (skip fo and msg_ref) */

    /* check da */
    LOGD("DA length =%d, off=%d", pdu_array[off], off);
    if (pdu_array[off] > (TPDU_MAX_ADDR_LEN - 1) * 2) {
        LOGD("DA length check fail");
        return false;
    }

    off += 1 + 1 + ((pdu_array[off]+1)/2);    /* skip addr_len, addr digits & addr type */

    /* check pid */
    LOGD("pid =%d, off=%d", pdu_array[off], off);
    if (rild_sms_pid_check(pdu_array[off]) == false) {
        LOGD("rild_sms_pid_check check fail");
        return false;
    }

    off++;                              /* move to dcs */

    /* check dcs */
    LOGD("dcs =%d, off=%d", pdu_array[off], off);
    dcs = pdu_array[off];
    if (rild_sms_dcs_check(dcs) == false) {
        LOGD("rild_sms_dcs_check check fail");
        return false;
    }

    /* move to udl */
    if (((fo & TPDU_VPF_BITS) >> 3) == TPDU_VPF_RELATIVE)
        off += 2;                       /* VP is relative format (1 byte) */
    else
        off++;                          /* VP not present */


    /* udl */
    LOGD("udl=%d, off=%d", pdu_array[off], off);
    udl = pdu_array[off];

    /* move to user data */
    off++;

    user_data_len = (unsigned char) rild_sms_msg_len_in_octet(dcs, udl);
    LOGD("user_data_len=%d", user_data_len);

    if (user_data_len > TPDU_ONE_MSG_OCTET || (user_data_len != (pdu_len/2 - off))) {
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    return true;

}

bool rild_sms_deliver_pdu_check(int pdu_len, char *pdu, TPDU_ERROR_CAUSE_ENUM *error_cause) {
    int off = 0;                  /* offset */
    int dcs;
    unsigned char udl, user_data_len;
    int pdu_array[pdu_len/2];
    int i=0;

    LOGD("rild_sms_deliver_pdu_check, pdu_len=%d", pdu_len);

    *error_cause = TPDU_NO_ERROR;

    /* check length */
    if (pdu_len/2 > TPDU_MAX_TPDU_SIZE || pdu_len == 0) {
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    for (i = 0; i < pdu_len/2 ;i ++) {
        pdu_array[i] = rild_sms_hexCharToDecInt((pdu+2*i), 2);
    }

    LOGD("SMSC address len =%d", pdu_array[0]);
    if (pdu_array[0] > TPDU_MAX_ADDR_LEN)
        return false;

    off += (1 + pdu_array[0]);              /* move to head of TPDU (skip sca_len, sca) */

    /* check this pdu is SMS-DELIVER or not */
    LOGD("first octet =%d", pdu_array[off]);
    if (((pdu_array[off] & TPDU_MTI_BITS) != TPDU_MTI_DELIVER) &&
        ((pdu_array[off] & TPDU_MTI_BITS) != TPDU_MTI_UNSPECIFIED)) {
        LOGD("first octet check fail");
        return false;
    }

    off += 1;                           /* move to oa, (skip fo ) */

    /* check oa */
    LOGD("OA length =%d", pdu_array[off]);
    if (pdu_array[off] > (TPDU_MAX_ADDR_LEN - 1) * 2) {
        LOGD("OA length check fail");
        return false;
    }

    if ((pdu_array[off] % 2) >0)
        off += 1 + 1 + ((pdu_array[off]+1)/2);    /* skip addr_len, addr digits & addr type */
    else
        off += 1 + 1 + (pdu_array[off]/2);    /* skip addr_len, addr digits & addr type */

    off++;                              /* skip pid, move to dcs */

    /* check dcs */
    dcs = pdu_array[off];
    LOGD("dcs =%d", pdu_array[off]);

    off += (1 + 7);                     /* skip dcs, move to uhl */

    /* udl */
    LOGD("udl =%d", pdu_array[off]);
    udl = pdu_array[off];

    /* move to user data */
    off++;

    user_data_len = (unsigned char) rild_sms_msg_len_in_octet(dcs, udl);
    LOGD("user_data_len=%d", user_data_len);

    if (user_data_len > TPDU_ONE_MSG_OCTET || (user_data_len != (pdu_len/2 - off))) {
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    return true;

}

int rild_sms_queryCBInfo(
        int *mode, char **ch_str, char **dcs_str, int *alllanguageon, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    char *line, *chIDs, *dcsIDs;

    *ch_str = *dcs_str = NULL;

    err = at_send_command_singleline("AT+CSCB?", "+CSCB:", &p_response, SMS_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || p_response->success == 0)
        goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) goto error;

    // get mode
    err = at_tok_nextint(&line, mode);
    if(err < 0 || (*mode) < 0 || (*mode) > 1) goto error;

    // get <mids> ID string
    err = at_tok_nextstr(&line, &chIDs);
    if(err < 0) goto error;
    asprintf(ch_str, "%s", chIDs);

    /*******************************************
    * Please remember to free the buffer ch_str and dcs_str
    * after using this function
    *******************************************/

    // get <dcss> ID string
    err = at_tok_nextstr(&line, &dcsIDs);
    if(err < 0) goto error;
    asprintf(dcs_str, "%s", dcsIDs);

    /*******************************************
    * Please remember to free the buffer ch_str and dcs_str
    * after using this function
    *******************************************/

    // get alllanguageon
    err = at_tok_nextint(&line, alllanguageon);
    if(err < 0)
        *alllanguageon = 0;

    at_response_free(p_response);
    return 0;
error:
    at_response_free(p_response);
    return -1;
}

int rild_sms_getIDTable(char *table, char *id_string, int maximum_id) {
    int len, i, j;
    int value, begin;

    memset(table, 0x00, maximum_id);
    len = strlen(id_string);

    if (len == 0)
        return 0;

    for(i = 0, begin = -1, value = 0; i <= len; i++, id_string++) {
        //LOGD("%s", id_string);
        if ( (*id_string) == ' ' )
            continue;
        else if ( (*id_string) == ',' || (*id_string == '\0')) {
            if (begin == -1) {
                /* %d, */
                begin = value;
            } else {
                /* %d-%d,*/
                begin = value + 1; /* ignore MIR value*/
            }

            for (j = begin; j <= value; j++) {
                if (j < maximum_id) {
                    table[j] = 1;
                    //LOGD("Set table valud %d", j);
                }
                else {
                    ;/* ignore */
                }
            }

            begin = -1;
            value = 0;
        } else if ( (*id_string) == '-' ) {
            begin = value;
            value = 0;
        } else if ( (*id_string) >= '0' && (*id_string) <= '9' ) {
            value = value*10 + (*id_string) - '0';
        } else {
            LOGE("There is some invalid characters: %x", (*id_string) );
            return -1;
        }
    }

    return 0;
}

static int  rild_sms_packPDU(const char* smsc, const char *tpdu, char *pdu, bool check) {
    int len=0;
    TPDU_ERROR_CAUSE_ENUM  error_cause = 0;
    int i=0;

    //Check SMSC
    if (smsc == NULL) {
        // "NULL for default SMSC"
        smsc= "00";
    } else {
        len = strlen(smsc);
        if ( (len%2) != 0 || len<2 || (len/2-1) > MAX_SMSC_LENGTH) {
            LOGE("The length of smsc is not valid: %d", len);
            return -1;
        }
    }

    // Check PDU
    if (tpdu == NULL) {
        LOGE("PDU should not be NULL");
        return -1;
    }
    len = strlen(tpdu);
    if ( (len%2) != 0) {
        LOGE("The length of TPDU is not valid: %d", len);
        return -1;
    }
    len /= 2;
    if ( len > MAX_TPDU_LENGTH) {
        LOGE("The length of TPDU is too long: %d", len);
        return -1;
    }

    sprintf(pdu, "%s%s", smsc, tpdu);

    RLOGD("pdu len=%zu", strlen(pdu));

    if (check) {
        if (rild_sms_submit_pdu_check(strlen(pdu), pdu, &error_cause) == false) {
            LOGE("rild_sms_submit_pdu_check error, error_cause = %d", error_cause);
            return -1;
        } else {
            LOGD("rild_sms_submit_pdu_check pass");
        }
    }

    return len;
}

static int rild_sms_checkReceivedPDU(int length, char* pdu, bool check) {
    int pdu_length;
    int sca_length;
    TPDU_ERROR_CAUSE_ENUM  error_cause = 0;
    int i=0;

    pdu_length = strlen(pdu);
    if (pdu_length < 2) {
        LOGW("The PDU structure might be wrong");
        return -1;
    }

    sca_length = rild_sms_hexCharToDecInt(pdu, 2);
    if (sca_length < 0) {
        LOGW("The PDU structure might be wrong with invalid digits: %d", sca_length);
        return -1;
    }

    if (check) {
        if (rild_sms_deliver_pdu_check(pdu_length, pdu, &error_cause) == false) {
            LOGE("rild_sms_deliver_pdu_check error, error_cause = %d", error_cause);
            return -1;
        } else {
            LOGD("rild_sms_deliver_pdu_check pass");
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
        LOGW("The specified TPDU length is not matched to the TPDU data:  %d", length);
    }

    return 0;
}

static void rild_sms_sendSMS(int request, void * data, size_t datalen, RIL_Token t) {
    int err, len=0, cause;
    const char *smsc;
    const char *pdu;
    char *cmd1 = NULL, *line = NULL;
    char buffer[(MAX_SMSC_LENGTH+MAX_TPDU_LENGTH+1)*2+1];   // +2: one for <length of SMSC>, one for \0
    RIL_SMS_Response response;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    SMS_UNUSED(datalen);

    if (checkSimPresent(t) == 0) {
        RIL_onRequestComplete(t, RIL_E_SIM_ABSENT, NULL, 0);
        return;
    }

    smsc = ((const char **)data)[0];
    pdu = ((const char **)data)[1];

    /* fill error code first */
    response.errorCode = ril_errno;

    len = rild_sms_packPDU(smsc, pdu, buffer, true);
    if(len < 0) goto error;

    /********************************************
    * if the request is RIL_REQUEST_SEND_SMS_EXPECT_MORE
    * We should send AT+CMMS=1 to enable keeping relay protocol link
    *********************************************/
    if (request == RIL_REQUEST_SEND_SMS_EXPECT_MORE) {
        err = at_send_command("AT+CMMS=1", &p_response, SMS_CHANNEL_CTX);
        if (err < 0 || p_response == NULL || p_response->success == 0) {
            LOGE("Error occurs while executing AT+CMMS=1");
            goto error;
        }
        at_response_free(p_response);
        p_response = NULL;
    }


    asprintf(&cmd1, "AT+CMGS=%d, \"%s\"", len, buffer);
    if (cmd1 == NULL) {
        goto error;
    }
    err = at_send_command_singleline(cmd1, "+CMGS:", &p_response, SMS_CHANNEL_CTX);
    free(cmd1);
    if (err < 0 || p_response == NULL) {
        LOGE("Error occurs while executing AT+CMGS=%d", len);
        goto error;
    }

    if (p_response->success == 0) {
        /****************************************
        * FIXME
        * Need to check what cause should be retry latter.
        ****************************************/
        cause = at_get_cms_error(p_response);
        if (cause == CMS_MTK_FDN_CHECK_FAILURE) {
            ril_errno = RIL_E_FDN_CHECK_FAILURE;
        } else if (cause == CMS_MTK_REQ_RETRY) {
            ril_errno = RIL_E_SMS_SEND_FAIL_RETRY;
        }

        /********************************************
        * free because it needs to send another AT
        * commands again
        ********************************************/
        at_response_free(p_response);
        p_response = NULL;
        /********************************************
        * if the request is RIL_REQUEST_SEND_SMS_EXPECT_MORE
        * We should send AT+CMMS=0 to disable keeping relay protocol link
        *********************************************/
        if (request == RIL_REQUEST_SEND_SMS_EXPECT_MORE) {
            err = at_send_command("AT+CMMS=0", &p_response, SMS_CHANNEL_CTX);
            if (err != 0 || p_response == NULL || p_response->success == 0) {
                LOGW("Warning: error occurs while executing AT+CMMS=0");
            }
        }

        response.errorCode = ril_errno;

        goto error;
    }

    // Get message reference and ackPDU
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);

    err = at_tok_nextint(&line, &(response.messageRef));
    LOGD("message reference number: %d ", response.messageRef);
    if (err < 0) goto error;

    if ( at_tok_hasmore(&line) ) {
        err = at_tok_nextstr(&line, &(response.ackPDU));
        if (err < 0) goto error;
    } else {
        response.ackPDU = NULL;
    }

    /* fill success code */
    response.errorCode = RIL_E_SUCCESS;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    at_response_free(p_response);

    return;
error:
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    if (p_response != NULL) {
        at_response_free(p_response);
    }
}

void requestSendSMS(void * data, size_t datalen, RIL_Token t) {
    rild_sms_sendSMS(RIL_REQUEST_SEND_SMS, data, datalen, t);
}

void requestSendSmsExpectMore(void * data, size_t datalen, RIL_Token t) {
    rild_sms_sendSMS(RIL_REQUEST_SEND_SMS_EXPECT_MORE, data, datalen, t);
}

void requestImsSendSms(int request, void * data, size_t datalen, RIL_Token t) {
    RIL_IMS_SMS_Message *msg = data;

    if (msg->tech == RADIO_TECH_3GPP) {
        LOGD("Start to send SMS over IMS");
        rild_sms_sendSMS(
                request,
                msg->message.gsmMessage,
                datalen - sizeof(RIL_RadioTechnologyFamily)+sizeof(uint8_t)+sizeof(int32_t),
                t);
    }
}

void requestSMSAcknowledge(void * data, size_t datalen, RIL_Token t) {
    int ackSuccess, cause;
    int err = 0;
    ATResponse *p_response = NULL;
    char *cmd = NULL;

    SMS_UNUSED(datalen);

    ackSuccess = ((int *)data)[0];
    cause = ((int *)data)[1] & 0xFF;

    showCurrIncomingSmsType();
    if (ackSuccess == 1) {
        if (sms_type == SMS_3G) {
            err = at_send_command("AT+CNMA=1, 2, \"0000\"", &p_response, SMS_CHANNEL_CTX);
        } else if (sms_type == SMS_IMS) {
            err = at_send_command("AT+EIMSCNMA=1, 2, \"0000\"", &p_response, SMS_CHANNEL_CTX);
        } else {
            LOGE("SMS type is SMS_NONE");
            goto error;
        }
    } else if (ackSuccess == 0) {
        if (sms_type == SMS_3G) {
            asprintf(&cmd, "AT+CNMA=2, 3, \"00%02X00\"", cause);
        } else if (sms_type == SMS_IMS) {
            asprintf(&cmd, "AT+EIMSCNMA=2, 3, \"00%02X00\"", cause);
        } else {
            LOGE("SMS type is SMS_NONE");
            goto error;
        }
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);
    } else {
        LOGE("unsupported arg to RIL_REQUEST_SMS_ACKNOWLEDGE\n");
        goto error;
    }

    if (err < 0 || p_response == NULL || p_response->success == 0)
        goto error;

    sms_type = SMS_NONE;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;
error:
    sms_type = SMS_NONE;
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    return;
}

void requestSMSAcknowledgeWithPdu(void * data, size_t datalen, RIL_Token t)
{
    const char *ackSuccess;
    const char *pdu;
    int err;
    ATResponse *p_response = NULL;
    char *cmd = NULL;

    SMS_UNUSED(datalen);

    ackSuccess = ((const char **)data)[0];
    pdu = ((const char **)data)[1];

    showCurrIncomingSmsType();
    if (*ackSuccess == '1') {
        if (sms_type == SMS_3G) {
            asprintf(&cmd, "AT+CNMA=1, 2, \"%s\"", pdu);
        } else if (sms_type == SMS_IMS) {
            asprintf(&cmd, "AT+EIMSCNMA=1, 2, \"%s\"", pdu);
        } else {
            LOGE("SMS type is SMS_NONE");
            goto error;
        }
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);
    } else if (*ackSuccess == '0') {
        if (sms_type == SMS_3G) {
            asprintf(&cmd, "AT+CNMA=2, 3, \"%s\"", pdu);
        } else if (sms_type == SMS_IMS) {
            asprintf(&cmd, "AT+EIMSCNMA=2, 3, \"%s\"", pdu);
        } else {
            LOGE("SMS type is SMS_NONE");
            goto error;
        }
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);
    } else {
        LOGE("unsupported arg to RIL_REQUEST_SMS_ACKNOWLEDGE\n");
        goto error;
    }

    if (err < 0 || p_response == NULL || p_response->success == 0)
        goto error;

    sms_type = SMS_NONE;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;
error:
    sms_type = SMS_NONE;
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    return;
}

void requestWriteSmsToSim(void * data, size_t datalen, RIL_Token t)
{
    RIL_SMS_WriteArgs *p_args;
    char *cmd, buffer[(MAX_SMSC_LENGTH+MAX_TPDU_LENGTH+1)*2+1];   // +2: one for <length of SMSC>, one for \0;
    int length, err, response[1], ret = 0;
    ATResponse *p_response = NULL;
    char *line;

    SMS_UNUSED(datalen);

    if (checkSimPresent(t) == 0) {
        RIL_onRequestComplete(t, RIL_E_SIM_ABSENT, NULL, 0);
        return;
    }

    p_args = (RIL_SMS_WriteArgs *)data;

    //check stat
    if (p_args->status < RIL_SMS_REC_UNREAD || p_args->status >= RIL_SMS_MESSAGE_MAX) {
        LOGE("The status is invalid: %d", p_args->status);
        goto error;
    }

    // pack PDU with SMSC
    length = rild_sms_packPDU(p_args->smsc, p_args->pdu, buffer, false);
    if (length < 0) goto error;

    asprintf(&cmd, "AT+CMGW=%d,%d, \"%s\"", length, p_args->status, buffer);
    err = at_send_command_singleline(cmd, "+CMGW:", &p_response, SMS_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response == NULL) {
        goto error;
    }

    if (p_response->success == 0) {
        ret = at_get_cms_error(p_response);
        goto error;
    }

    // get +CMGW: <index>
    line = p_response->p_intermediates->line;

    err = at_tok_start( &line);
    if( err < 0) goto error;

    err = at_tok_nextint(&line, &response[0]);
    if( err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    at_response_free(p_response);
    return;
error:
    if(CMS_CM_SIM_IS_FULL == ret) {
        RIL_onRequestComplete(t, RIL_E_SIM_FULL, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    at_response_free(p_response);
}

void requestDeleteSmsOnSim(void *data, size_t datalen, RIL_Token t)
{
    char * cmd = NULL;
    ATResponse *p_response = NULL;
    int err, index;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    SMS_UNUSED(datalen);

    if (checkSimPresent(t) == 0) {
        RIL_onRequestComplete(t, RIL_E_SIM_ABSENT, NULL, 0);
        return;
    }

    index = ((int *)data)[0];

    if(index > 0) {
        asprintf(&cmd, "AT+CMGD=%d", index);
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);

        if ( !(err < 0 || p_response == NULL || p_response->success == 0) ) {
            ril_errno = RIL_E_SUCCESS;
        }
        at_response_free(p_response);
    } else if(-1 == index) {
        // delete all sms on SIM
        asprintf(&cmd, "AT+CMGD=0,4");
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);

        if ( !(err < 0 || p_response == NULL || p_response->success == 0) ) {
            ril_errno = RIL_E_SUCCESS;
        }
        at_response_free(p_response);
    }
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
}

void requestGSMSMSBroadcastActivation(void *data, size_t datalen, RIL_Token t)
{
    int activation;
    int err;
    char *line;
    int mode, allLanguageOn;
    char *ch_string = NULL, *dcs_string = NULL;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_SUCCESS;

    SMS_UNUSED(datalen);

    activation = ((int *)data)[0];
    LOGD("activation: %d", activation);

    // asprintf(&line, "AT+CSCB=%d", activation);
    if(1 == activation) {
        asprintf(&line, "AT+CSCB=1");
    } else if(0 == activation){
        LOGD("query previous config info");
        err = rild_sms_queryCBInfo(&mode, &ch_string, &dcs_string, &allLanguageOn, t);
        if (err < 0) goto error;
        LOGD("Current setting: %s, %s", ch_string, dcs_string);

        asprintf(&line, "AT+CSCB=0, \"%s\", \"%s\"", ch_string, dcs_string);
        if(NULL != ch_string) {
            free(ch_string);
        }
        if(NULL != dcs_string) {
            free(dcs_string);
        }
    } else {
        LOGD("unhandled mode fot AT+CSCB");
        ril_errno = RIL_E_GENERIC_FAILURE;
        goto error;
    }
    err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);
    if (err < 0 || p_response == NULL || p_response->success <= 0) {
        ril_errno = RIL_E_GENERIC_FAILURE;
    }
    at_response_free(p_response);
    p_response = NULL;

    asprintf(&line, "AT+ETWS=%d", ((activation == 0)? 5 : 0));
    err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);
    if (err < 0 || p_response == NULL || p_response->success == 0) {
        ril_errno = RIL_E_GENERIC_FAILURE;
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);

    return;

error:
    if(NULL != ch_string) {
        free(ch_string);
    }
    if(NULL != dcs_string) {
        free(dcs_string);
    }

    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestGetSMSCAddress(void *data, size_t datalen, RIL_Token t)
{
    int err, tosca;
    char *line, *sca;
    ATResponse *p_response = NULL;

    SMS_UNUSED(data);
    SMS_UNUSED(datalen);

    if (checkSimPresent(t) == 0) {
        RIL_onRequestComplete(t, RIL_E_SIM_ABSENT, NULL, 0);
        return;
    }

    err = at_send_command_singleline("AT+CSCA?", "+CSCA:", &p_response, SMS_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || p_response->success <= 0) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    // get <SCA>
    err = at_tok_nextstr(&line, &sca);
    if (err < 0) goto error;

    // get <TOSCA>
    err = at_tok_nextint(&line, &tosca);
    if (err < 0) goto error;

    if (tosca == 0x91 && sca[0] != '+') {
        // add '+' in front of the sca
        sca--;
        sca[0] = '+';
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, sca, sizeof(char*)*strlen(sca));
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestSetSMSCAddress(void *data, size_t datalen, RIL_Token t)
{
    int err = -1, tosca = 0x81, len = -1;
    char *line = NULL, *sca = NULL;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    char *empty = "";

    SMS_UNUSED(datalen);

    if (checkSimPresent(t) == 0) {
        RIL_onRequestComplete(t, RIL_E_SIM_ABSENT, NULL, 0);
        return;
    }

    sca = ((data != NULL)? ((char*) data) : empty);

    // get <tosca>
    if (sca[0] == '+') {
        tosca = 0x91;
        sca++;
    }

    len = strlen(sca);
    if (len > (MAX_SMSC_LENGTH-1)*2) {
        LOGE("The input length of the SCA is too long: %d", len);
    } else {
        asprintf(&line, "AT+CSCA=\"%s\", %d", sca, tosca);
        if (line == NULL) {
            goto error;
        }
        err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
        free(line);
        if ( !(err < 0 || p_response == NULL || p_response->success <= 0) ) {
            /********************************************
            * free because it needs to send another AT
            * commands again
            ********************************************/
            at_response_free(p_response);
            p_response = NULL;
            // ril_errno = RIL_E_SUCCESS;
            asprintf(&line, "AT+CSAS");
            if (line == NULL) {
                goto error;
            }
            err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
            free(line);
            if ( !(err < 0 || p_response == NULL || p_response->success <= 0) ) {
                ril_errno = RIL_E_SUCCESS;
            }
        }
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void sendNewSmsErrorAck(void *param) {
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *)param);

    LOGD("sendNewSmsErroAck, rid = %d", rid);
    at_send_command("AT+CNMA=2, 3, \"00FF00\"", NULL, getChannelCtxbyProxy());
}

void sendNewSmsErrorAckAndReset(void *param) {
    sendNewSmsErrorAck(param);
    showCurrIncomingSmsType();
    sms_type = SMS_NONE;
}

void sendNewImsSmsErrorAck(void *param) {
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *)param);

    LOGD("sendNewImsSmsErroAck, rid = %d", rid);
    at_send_command("AT+EIMSCNMA=2, 3, \"00FF00\"", NULL, getChannelCtxbyProxy());
}

void sendNewImsSmsErrorAckAndReset(void *param) {
    sendNewImsSmsErrorAck(param);
    showCurrIncomingSmsType();
    sms_type = SMS_NONE;
}

void sendNewSmsErroAckWithTimerCallback(
        RIL_SOCKET_ID socketId, int newSmsDomain, bool reset) {
    RIL_SOCKET_ID *pollSimId;

    if (socketId == RIL_SOCKET_1) {
        pollSimId = &s_pollSimId;
    } else if (socketId == RIL_SOCKET_2) {
        pollSimId = &s_pollSimId2;
    } else if (socketId == RIL_SOCKET_3) {
        pollSimId = &s_pollSimId3;
    } else if (socketId == RIL_SOCKET_4) {
        pollSimId = &s_pollSimId4;
    }

    if (newSmsDomain == SMS_3G) {
        if (reset) {
            RIL_requestProxyTimedCallback(sendNewSmsErrorAckAndReset, pollSimId, &TIMEVAL_0,
                    getRILChannelId(RIL_SMS, socketId), "sendNewSmsErrorAckAndReset");
        } else {
            RIL_requestProxyTimedCallback(sendNewSmsErrorAck, pollSimId, &TIMEVAL_0,
                    getRILChannelId(RIL_SMS, socketId), "sendNewSmsErrorAck");
        }
    } else if (newSmsDomain == SMS_IMS) {
        if (reset) {
            RIL_requestProxyTimedCallback(sendNewImsSmsErrorAckAndReset, pollSimId, &TIMEVAL_0,
                    getRILChannelId(RIL_SMS, socketId), "sendNewImsSmsErrorAckAndReset");
        } else {
            RIL_requestProxyTimedCallback(sendNewImsSmsErrorAck, pollSimId, &TIMEVAL_0,
                    getRILChannelId(RIL_SMS, socketId), "sendNewImsSmsErrorAck");
        }
    } else {
        LOGD("sendNewSmsErroAckWithTimerCallback, Type is not reasonable, newSmsDomain = %d",
                newSmsDomain);
    }
}

void onNewSms(const char *urc, const char *smspdu, RIL_SOCKET_ID rid) {
    int length = -1;
    int err;
    char *pUrc = (char *)urc;

    err = at_tok_start(&pUrc);

    // skip <alpha>
    err = at_tok_nextint(&pUrc, &length);

    // get <length>
    err = at_tok_nextint(&pUrc, &length);

    err = rild_sms_checkReceivedPDU(length,(char *) smspdu, true);
    if (err < 0) goto error;

    RIL_UNSOL_RESPONSE (
        RIL_UNSOL_RESPONSE_NEW_SMS,
        smspdu, sizeof(char*)*length*2,
        rid);

    return;

error:
    // Use the timer callback to send CNMA fail to modem and will reset the
    // sms_type value while the CNMA is sent
    sendNewSmsErroAckWithTimerCallback(rid, sms_type, true);

    LOGE("onNewSms check fail");
}

void onNewSmsStatusReport(const char *urc, const char *smspdu, RIL_SOCKET_ID rid) {
    int length = -1;
    int err;
    char *pUrc = (char *)urc;

    err = at_tok_start(&pUrc);

    err = at_tok_nextint(&pUrc, &length);

    err = rild_sms_checkReceivedPDU(length,(char *) smspdu, false);
    if (err < 0) goto error;

    RIL_UNSOL_RESPONSE (
        RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT,
        smspdu, sizeof(char*)*length*2,
        rid);

    return;

error:
    // Use the timer callback to send CNMA fail to modem and will reset the
    // sms_type value while the CNMA is sent
    sendNewSmsErroAckWithTimerCallback(rid, sms_type, true);

    LOGE("onNewSmsStatusReport check fail");
}

void onNewSmsOnSim(const char *urc, RIL_SOCKET_ID rid) {
    int err, index[1];
    char *mem = NULL;
    char *pUrc = (char *)urc;

    err = at_tok_start(&pUrc);
    if (err < 0) goto error;

    err = at_tok_nextstr(&pUrc, &mem);
    if (err < 0) goto error;

    if (strncmp(mem, "SM", 2) != 0) {
        LOGE("The specified storage is not SIM storage");
        return;
    }

    err = at_tok_nextint(&pUrc, &index[0]);
    if (err < 0) goto error;

    RIL_UNSOL_RESPONSE (
        RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM,
        index, sizeof(index),
        rid);

    return;

error:
    LOGE("There is something wrong with the input URC");
}

void onSimSmsStorageStatus(const char *line, RIL_SOCKET_ID rid) {
    int err;
    int status, ciev_id;
    int urc_id = -1;
    char *pUrc = (char *)line;

    err = at_tok_start(&pUrc);

    err = at_tok_nextint(&pUrc, &ciev_id);
    if (err < 0) return;

    err = at_tok_nextint(&pUrc, &status);
    if (err < 0) return;

    if (err < 0) {
        LOGE("There is something wrong with the URC: +CIEV:7, <status>");
    } else {
        switch(status) {
            case 0: // available
                break;
            case 1: // full
            case 2: // SIM exceed
                /* for mem1, mem2 and mem3, all are SIM card storage due to set as AT+CPMS="SM", "SM", "SM" */
                urc_id = RIL_UNSOL_SIM_SMS_STORAGE_FULL;
                break;
            case 3: // ME exceed
                LOGD("Phone storage status: %d", phone_storage_status);
                if (phone_storage_status == 1)
                {
                    /*********************************************************
                     * It is a workaround solution here.
                     * for mem1, mem2 and mem3, even if all are SIM card
                     * storage due to set as AT+CPMS="SM", "SM", "SM", we still
                     * need to check the phone_storage_status.
                     * It is the current limitation on modem, it should
                     * upgrate the +CIEV for better usage.
                     ********************************************************/
                    urc_id = RIL_UNSOL_ME_SMS_STORAGE_FULL;
                }
                else
                {
                    /* for mem1, mem2 and mem3, all are SIM card storage due to set as AT+CPMS="SM", "SM", "SM" */
                    urc_id = RIL_UNSOL_SIM_SMS_STORAGE_FULL;
                }
                break;
            default:
                LOGW("The received <status> is invalid: %d", status);
                break;
        }
        if (urc_id >= 0) {
            RIL_UNSOL_RESPONSE (urc_id, NULL, 0, rid);
        }
    }
}

int hexCharToInt(char c) {
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);

    return 0;
}

void hexStringToBytes(const char *in, int inLength, char *out, int outLength) {
    int i;

    if (in == NULL || out == NULL) {
        return;
    }

    if (inLength != outLength * 2) {
        return;
    }

    for (i = 0 ; i < inLength ; i += 2) {
        out[i/2] = (char)((hexCharToInt(in[i]) << 4)
                           | hexCharToInt(in[i+1]));
    }
}

void onNewBroadcastSms(const char *urc, const char *smspdu, RIL_SOCKET_ID rid) {
    int length = -1;
    int err;
    char *line = NULL;
    char *pUrc = (char *)urc;
    char *byteSmsPdu = NULL;

    err = at_tok_start(&pUrc);

    err = at_tok_nextint(&pUrc, &length);

    asprintf(&line, "00%s", smspdu);
    err = rild_sms_checkReceivedPDU(length, line, false);
    free(line);
    if (err < 0) goto error;
    /* Transfer to byte array for responseRaw */
    byteSmsPdu = calloc(1, sizeof(char)*length);
    if (byteSmsPdu == NULL) {
        RLOGE("onNewBroadcastSms, fail to allocate memory");
        goto error;
    }
    hexStringToBytes(smspdu, length*2, byteSmsPdu, length);

    RIL_UNSOL_RESPONSE (
        RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS ,
        byteSmsPdu, sizeof(char)*length,
        rid);

    free(byteSmsPdu);
    return;

error:
    sms_type = SMS_NONE;
    LOGE("onNewBroadcastSms check fail");
}

void onSmsReady(void *param) {
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *)param);

    LOGD("On Sms Ready rid: %d", rid);

    /*
     * If modem turns off, RILD should not send any AT command to modem.
     * If RILD sends the at commands to Modem(off), it will block the
     * MUXD/CCCI channels
     */
    if (s_md_off) {
        LOGD("Modem turns off");
        return;
    }
    LOGD("Modem turns on");

    RIL_UNSOL_RESPONSE(
                RIL_UNSOL_SMS_READY_NOTIFICATION,
                NULL, 0, rid);

    /*
     * Always send SMS messages directly to the TE
     *
     * mode = 1 // discard when link is reserved (link should never be
     *             reserved)
     * mt = 2   // most messages routed to TE
     * bm = 2   // new cell BM's routed to TE
     * ds = 1   // Status reports routed to TE
     * bfr = 1  // flush buffer
     */
    at_send_command_singleline("AT+CSMS=1", "+CSMS:", NULL,getChannelCtxbyProxy());

    at_send_command("AT+CNMI=1,2,2,1,1", NULL,getChannelCtxbyProxy());

    at_send_command("AT+CPMS=\"SM\", \"SM\", \"SM\"", NULL, getChannelCtxbyProxy());
}

extern int rilSmsMain(int request, void *data, size_t datalen, RIL_Token t) {
    switch (request) {
        case RIL_REQUEST_SEND_SMS:
            requestSendSMS(data, datalen, t);
            break;
        case RIL_REQUEST_SMS_ACKNOWLEDGE:
        case RIL_REQUEST_SMS_ACKNOWLEDGE_EX:
            requestSMSAcknowledge(data, datalen, t);
            break;
        case RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU:
            requestSMSAcknowledgeWithPdu(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_SMS_TO_SIM:
            requestWriteSmsToSim(data, datalen, t);
            break;
        case RIL_REQUEST_DELETE_SMS_ON_SIM:
            requestDeleteSmsOnSim(data, datalen, t);
            break;
        case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
            requestSendSmsExpectMore(data, datalen, t);
            break;
        case RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG:
            //for channel over 1000 issue
            //requestGSMGetBroadcastSMSConfigEx(data, datalen, t);
            requestGsmGetBroadcastSmsConfig(data, datalen, t, GSM_CB_QUERY_CHANNEL);
            break;
        case RIL_REQUEST_GSM_SET_BROADCAST_LANGUAGE:
            requestGsmSetBroadcastLanguage(data, datalen, t);
            break;
        case RIL_REQUEST_GSM_GET_BROADCAST_LANGUAGE:
            //for channel over 1000 issue
            //requestGSMGetBroadcastSMSConfigEx(data, datalen, t);
            requestGsmGetBroadcastSmsConfig(data, datalen, t, GSM_CB_QUERY_LANGUAGE);
            break;
        case RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:
            //for channel over 1000 issue
            //requestGSMSetBroadcastSMSConfigEx(data, datalen, t);
            requestGsmSetBroadcastSmsConfig(data, datalen, t);
            break;
        case RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION:
            requestGSMSMSBroadcastActivation(data, datalen, t);
            break;
        case RIL_REQUEST_GET_SMSC_ADDRESS:
            requestGetSMSCAddress(data, datalen, t);
            break;
        case RIL_REQUEST_SET_SMSC_ADDRESS:
            requestSetSMSCAddress(data, datalen, t);
            break;
        case RIL_REQUEST_REPORT_SMS_MEMORY_STATUS:
            requestReportSMSMemoryStatus(data, datalen, t);
            break;

        case RIL_REQUEST_IMS_SEND_SMS:
        case RIL_REQUEST_IMS_SEND_SMS_EX:
            requestImsSendSms(request, data, datalen, t);
            break;

        case RIL_REQUEST_GET_SMS_SIM_MEM_STATUS:
            requestGetSmsSimMemoryStatus(data, datalen, t);
            break;

        case RIL_REQUEST_GET_SMS_PARAMS:
            requestGetSmsParams(data, datalen, t);
            break;

        case RIL_REQUEST_SET_SMS_PARAMS:
            requestSetSmsParams(data, datalen, t);
            break;

        case RIL_REQUEST_SET_ETWS:
            requestSetEtws(data, datalen, t);
            break;
        case RIL_REQUEST_REMOVE_CB_MESSAGE:
            requestRemoveCbMsg(data, datalen, t);
            break;
        case RIL_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION:
            requestGetGsmSmsBroadcastActivation(data, datalen, t);
            break;
        default:
            return 0; /* no matched request */
        break;
     }

    return 1; /* request found and handled */

}

extern int rilSmsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    // During phone mode chip certification process, sms rild will not handle any URC from modem
    char chipCertification[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.op12.ccp.mode", chipCertification, "0");
    if (atoi(chipCertification) == 1) {
        // Return directly and not handling anything
        return 0;
    }

    if (strStartsWith(s, "+CIEV: 7")) {
        onSimSmsStorageStatus(s,rid);
    } else if (strStartsWith(s, "+CMT:")) {
        showCurrIncomingSmsType();
        if (sms_type == SMS_NONE) {
            sms_type = SMS_3G;
            onNewSms(s, sms_pdu,rid);
        } else {
            LOGE("One 3G or IMS SMS on AP, reject");
            // Use the timer callback to send CNMA fail to modem and will reset the
            // sms_type value while the CNMA is sent
            sendNewSmsErroAckWithTimerCallback(rid, SMS_3G, false);
        }
    } else if (strStartsWith(s, "+EIMSCMT:")) {
        showCurrIncomingSmsType();
        if (sms_type == SMS_NONE) {
            sms_type = SMS_IMS;
            onNewSms(s, sms_pdu,rid);
        } else {
            LOGE("One 3G or IMS SMS on AP, reject");
            // Use the timer callback to send CNMA fail to modem and will reset the
            // sms_type value while the CNMA is sent
            sendNewSmsErroAckWithTimerCallback(rid, SMS_IMS, false);
        }
    } else if (strStartsWith(s, "+CMTI:")) {
        onNewSmsOnSim(s,rid);
    } else if (strStartsWith(s, "+CDS:")) {
        showCurrIncomingSmsType();
        if (sms_type == SMS_NONE) {
            sms_type = SMS_3G;
            onNewSmsStatusReport(s, sms_pdu,rid);
        } else {
            LOGE("One 3G or IMS SMS report on AP, reject");
            // Use the timer callback to send CNMA fail to modem and will reset the
            // sms_type value while the CNMA is sent
            sendNewSmsErroAckWithTimerCallback(rid, SMS_3G, false);
        }
    } else if (strStartsWith(s, "+EIMSCDS:")) {
        showCurrIncomingSmsType();
        if (sms_type == SMS_NONE) {
            sms_type = SMS_IMS;
            onNewSmsStatusReport(s, sms_pdu,rid);
        } else {
            LOGE("One 3G or IMS SMS report on AP, reject");
            // Use the timer callback to send CNMA fail to modem and will reset the
            // sms_type value while the CNMA is sent
            sendNewSmsErroAckWithTimerCallback(rid, SMS_IMS, false);
        }
    } else if (strStartsWith(s, "+CBM:")) {
        onNewBroadcastSms(s, sms_pdu,rid);
    } else if (strStartsWith(s, "+EIND: 1")) {
        if (rid == RIL_SOCKET_1) {
            RIL_requestProxyTimedCallback(onSmsReady, (void*)&s_pollSimId, &TIMEVAL_0,
                    getRILChannelId(RIL_SMS, rid), "onSmsReady");
        } else if (rid == RIL_SOCKET_2) {
            RIL_requestProxyTimedCallback(onSmsReady, (void*)&s_pollSimId2, &TIMEVAL_0,
                    getRILChannelId(RIL_SMS, rid), "onSmsReady");
        } else if (rid == RIL_SOCKET_3) {
            RIL_requestProxyTimedCallback(onSmsReady, (void*)&s_pollSimId3, &TIMEVAL_0,
                    getRILChannelId(RIL_SMS, rid), "onSmsReady");
        } else if (rid == RIL_SOCKET_4) {
            RIL_requestProxyTimedCallback(onSmsReady, (void*)&s_pollSimId4, &TIMEVAL_0,
                    getRILChannelId(RIL_SMS, rid), "onSmsReady");
        }
    }
    else if (strStartsWith(s, "+ETWS:")) {
        onNewEtwsNotification(s, rid);
    } else {
        return 0;
    }

    return 1;
}

void requestGsmSetBroadcastSmsConfig(void *data, size_t datalen, RIL_Token t) {
    int num = datalen/sizeof(RIL_GSM_BroadcastSmsConfigInfo *);
    int start = -1, end = -1, mode = -1;
    int err = RIL_E_GENERIC_FAILURE;
    char *line = NULL;
    ATResponse *p_response = NULL;
    RIL_GSM_BroadcastSmsConfigInfo **info = (RIL_GSM_BroadcastSmsConfigInfo**)data;
    char* settings = NULL;
    char* desStr = NULL;

    RLOGD("Set CB channel array num %d", num);

    // 1. Convert RIL_GSM_BroadcastSmsConfigInfo to string
    for (int i = 0; i < num ; i++) {
        start = info[i]->fromServiceId;
        end = info[i]->toServiceId;

        if (start == end) {
            // Single channel id
            if ((i+1) == num) {
                // The last one
                asprintf(&desStr, "%s%d", ((settings != NULL)? settings : ""), start);
            } else {
                asprintf(&desStr, "%s%d,", ((settings != NULL)? settings : ""), start);
            }
        } else {
            // A range of channel id
            if ((i + 1) == num) {
                // The last one
                asprintf(&desStr, "%s%d-%d", ((settings != NULL)? settings : ""), start, end);
            } else {
                asprintf(&desStr, "%s%d-%d,", ((settings != NULL)? settings : ""), start, end);
            }
        }
        if (settings != NULL) {
            free(settings);
            settings = NULL;
        }
        asprintf(&settings, "%s", desStr);
        if (desStr != NULL) {
            // release desStr and ready to append new setting
            free(desStr);
            desStr = NULL;
        }
    }

    RLOGD("Set CB channels %s", settings);

    // 2. Overwrite the channel configuration
    if (settings != NULL) {
        asprintf(&line, "AT+CSCB=0, \"%s\", \"\"", settings);
    } else {
        asprintf(&line, "AT+CSCB=0, \"\", \"\"");
    }

    if (settings != NULL) {
        free(settings);
        settings = NULL;
    }
    err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);
    line = NULL;
    if(err < 0 || p_response == NULL || p_response->success == 0) {
        LOGE("Fail to set channel");
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    p_response = NULL;

    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static RIL_GSM_BroadcastSmsConfigInfo **convertChannelStringToGsmCbConfigInfo(
        char* channels, int *count)
{
    int num = 0, i = 0;
    char key[] = ",-";
    char *pHead = channels, *pCh = NULL, *tmpChStr = NULL;
    RIL_SMS_GSM_CB_CHANNEL_LIST *pCbListHead = NULL, *pCbList = NULL;
    RIL_GSM_BroadcastSmsConfigInfo **gsmBciPtrs = NULL;
    char ch[50];

    asprintf(&tmpChStr, "%s", channels);

    pHead = pCh = tmpChStr;

    // 1. Split string
    pCh = strpbrk(tmpChStr, key);
    while (pCh != NULL) {
        // New a RIL_SMS_GSM_CB_CHANNEL_list object
        if (pCbListHead == NULL) {
            // Point to the first element
            pCbListHead = calloc(1, sizeof(RIL_SMS_GSM_CB_CHANNEL_LIST));
            if (pCbListHead == NULL) {
                RLOGE("Failed to allocate memory for pCbListHead");
                break;
            }
            pCbList = pCbListHead;
        } else {
            pCbList->pNext = calloc(1, sizeof(RIL_SMS_GSM_CB_CHANNEL_LIST));
            if (pCbList->pNext == NULL) {
                RLOGE("Failed to allocate memory for pCbList->pNext");
                break;
            }
            pCbList = pCbList->pNext;
        }

        memset(ch, 0, sizeof(char)*50);
        strncpy(ch, pHead, (pCh - pHead));
        // Move pHead to next position
        pHead = pCh + 1;
        if ((*pCh) == ',') {
            // Match the key comma
            num++;
            pCbList->end = pCbList->start = atoi(ch);
            RLOGD("Single channel id, Start %d, End %d, num %d", pCbList->start, pCbList->end,
                    num);
        } else {
            // Match the key desh
            num++;
            pCbList->start = atoi(ch);
            pCh = strpbrk(pHead, key);
            memset(ch, 0, sizeof(char)*50);
            if (pCh != NULL) {
                strncpy(ch, pHead, (pCh - pHead));
            } else {
                // This must be the last channel id
                strncpy(ch, pHead, 49);
            }
            pCbList->end = atoi(ch);
            // Move pHead to next position
            pHead = ((pCh != NULL)? pCh + 1 : pCh);
            RLOGD("Range channel id, Start %d, End %d, num %d", pCbList->start, pCbList->end,
                    num);
        }

        if (pHead != NULL) {
            pCh = strpbrk(pHead, key);
        }
    }

    if (pHead != NULL) {
        // The case is the last channel id. e.g. "...,1234". Or only one channel id. e.g. "1234"
        if (pCbListHead == NULL) {
            pCbListHead = calloc(1, sizeof(RIL_SMS_GSM_CB_CHANNEL_LIST));
            // Keep the first element
            pCbList = pCbListHead;
        } else {
            pCbList->pNext = calloc(1, sizeof(RIL_SMS_GSM_CB_CHANNEL_LIST));
            pCbList = pCbList->pNext;
        }

        if (pCbList != NULL) {
            num++;
            memset(ch, 0, sizeof(char)*50);
            strncpy(ch, pHead, 49);
            pCbList->end = pCbList->start = atoi(ch);
            RLOGD("The last channel id, Start %d, End %d, num %d", pCbList->start, pCbList->end,
                    num);
        } else {
            RLOGE("Failed to allocate memory for the last channe id");
        }
    }

    // 2. Create RIL_GSM_BroadcastSmsConfigInfo and set value
    gsmBciPtrs = calloc(1, sizeof(RIL_GSM_BroadcastSmsConfigInfo *)*num);
    if (gsmBciPtrs != NULL) {
        pCbList = pCbListHead;
        while (pCbList != NULL) {
            gsmBciPtrs[i] = calloc(1, sizeof(RIL_GSM_BroadcastSmsConfigInfo));
            if (gsmBciPtrs[i] == NULL) {
                RLOGE("Failed to allocate memory for gsmBciPtrs[%d]", i);
                break;
            }
            gsmBciPtrs[i]->fromServiceId = pCbList->start;
            gsmBciPtrs[i]->toServiceId = pCbList->end;
            gsmBciPtrs[i]->selected = 1;
            i++;
            pCbList = pCbList->pNext;
        }
        RLOGD("gsmBciPtrs count %d", i);
    } else {
        num = 0;
    }

    // 3. Release
    pCbList = pCbListHead;
    while (pCbList != NULL) {
        pCbListHead = pCbList->pNext;
        free(pCbList);
        pCbList = pCbListHead;
    }

    free(tmpChStr);

    (*count) = i;
    return gsmBciPtrs;
}


void requestGsmGetBroadcastSmsConfig(void *data, size_t datalen, RIL_Token t,
        GSM_CB_QUERY_MODE query) {
    int err = RIL_E_GENERIC_FAILURE;
    int mode = -1;
    char *line = NULL, *channels = NULL, *langs = NULL, *settings = NULL;
    ATResponse *p_response = NULL;

    SMS_UNUSED(data);
    SMS_UNUSED(datalen);

    // 1. Query the configuration from modem
    err = at_send_command_singleline("AT+CSCB?", "+CSCB:", &p_response, SMS_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || p_response->success == 0)
        goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) goto error;

    // get mode
    err = at_tok_nextint(&line, &mode);
    if(err < 0 || mode < 0 || mode > 1) goto error;

    // get <mids> ID string
    err = at_tok_nextstr(&line, &channels);
    if(err < 0) goto error;

    // get <dcss> ID string
    err = at_tok_nextstr(&line, &langs);
    if(err < 0) goto error;

    if (query == GSM_CB_QUERY_CHANNEL) {
        asprintf(&settings, "%s", channels);
        if (strlen(channels) > 0) {
            // At least one channel id

            // 2. Convert string to RIL_GSM_BroadcastSmsConfigInfo
            int count = 0, i = 0;
            RIL_GSM_BroadcastSmsConfigInfo **gsmBciPtrs = NULL;

            gsmBciPtrs = convertChannelStringToGsmCbConfigInfo(settings, &count);

            // 3. Send AP channel configuration
            RIL_onRequestComplete(t, RIL_E_SUCCESS, gsmBciPtrs,
                    sizeof(RIL_GSM_BroadcastSmsConfigInfo*)*count);

            // free the output data
            for (i = 0; i < count ; i++) {
                if (gsmBciPtrs[i] != NULL) {
                    free(gsmBciPtrs[i]);
                }
            }
            if (gsmBciPtrs != NULL) {
                free(gsmBciPtrs);
            }
        } else {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        }

    } else {
        // 2. Send AP languages configuration
        asprintf(&settings, "%s", langs);
        if (strlen(langs) > 0) {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, settings, strlen(settings));
        } else {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        }
    }
    if (settings != NULL) {
        free(settings);
    }
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);

}

void requestGsmSetBroadcastLanguage(void *data, size_t datalen, RIL_Token t)
{
    int err = RIL_E_GENERIC_FAILURE;
    int mode = -1;
    char *line = NULL, *langs = (char*)data, *channels = NULL, *oldLangs = NULL, *cmd = NULL;
    char *chStr = NULL, *laStr = NULL;
    ATResponse *p_response = NULL;

    SMS_UNUSED(datalen);

    RLOGD("Set CB langs %s", langs);

    asprintf(&laStr, "%s", ((strlen(langs) > 0)? langs : ""));

    // 1. Query old settings
    err = at_send_command_singleline("AT+CSCB?", "+CSCB:", &p_response, SMS_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || p_response->success == 0)
        goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) goto error;

    // get mode
    err = at_tok_nextint(&line, &mode);
    if(err < 0 || mode < 0 || mode > 1) goto error;

    // get <mids> ID string
    err = at_tok_nextstr(&line, &channels);
    if(err < 0) goto error;

    asprintf(&chStr, "%s", ((strlen(channels) > 0)? channels : ""));

    // get <dcss> ID string
    err = at_tok_nextstr(&line, &oldLangs);
    if(err < 0) goto error;

    // 2. Clear language settings
    asprintf(&cmd, "AT+CSCB=1, \"\", \"%s\"", oldLangs);
    at_response_free(p_response);
    p_response = NULL;
    err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        RLOGE("Fail to clear language configuration");
        goto error;
    }

    at_response_free(p_response);
    p_response = NULL;

    // 3. Set new language configuration
    asprintf(&line, "AT+CSCB=0, \"%s\", \"%s\"", chStr, laStr);

    err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);
    line = NULL;

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        RLOGE("Fail to set language configuration");
        goto error;
    }

    at_response_free(p_response);
    p_response = NULL;


    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

    if (chStr != NULL) {
        free(chStr);
    }
    if (laStr != NULL) {
        free(laStr);
    }
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    if (chStr != NULL) {
        free(chStr);
    }
    if (laStr != NULL) {
        free(laStr);
    }
}

void requestGetSmsSimMemoryStatus(void *data, size_t datalen, RIL_Token t) {
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    ATResponse *p_response = NULL;
    RIL_SMS_Memory_Status mem_status = {0, 0};
    char *line;
    int err;
    int i;

    SMS_UNUSED(data);
    SMS_UNUSED(datalen);

    err = at_send_command_singleline("AT+CPMS?", "+CPMS:", &p_response, SMS_CHANNEL_CTX);

    if (err < 0 || p_response == NULL || p_response->success <= 0)
        goto done;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    RIL_SMS_GOTO_DONE_CHECK(err);

    for (i = 0; i < RIL_SMS_MEM_TYPE_TOTAL; i++) {
        char *mem_type_str;

        err = at_tok_nextstr(&line, &mem_type_str);

        RIL_SMS_GOTO_DONE_CHECK(err);

        if (strcmp(mem_type_str, "SM") == 0 || strcmp(mem_type_str, "SM_P") == 0) {
            // err = at_tok_nextint(&line, &mem_status->used);
            err = at_tok_nextint(&line, &(mem_status.used));

            RIL_SMS_GOTO_DONE_CHECK(err);

            // err = at_tok_nextint(&line, &mem_status->total);
            err = at_tok_nextint(&line, &(mem_status.total));

            RIL_SMS_GOTO_DONE_CHECK(err);

            ril_errno = RIL_E_SUCCESS;

            break;
        }
    }

done:
    RIL_onRequestComplete(t, ril_errno, &mem_status, sizeof(RIL_SMS_Memory_Status));
    at_response_free(p_response);
}

void requestReportSMSMemoryStatus(void *data, size_t datalen, RIL_Token t) {
    int available;
    int err;
    char *line = NULL;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    SMS_UNUSED(datalen);

    if (checkSimPresent(t) == 0) {
        RIL_onRequestComplete(t, RIL_E_SIM_ABSENT, NULL, 0);
        return;
    }

    available = ((int *)data)[0];
    if (available == 1 || available == 0) {

        asprintf(&line, "AT+EMEMS=1, %d", available==1 ? 0 : 1);
        err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
        free(line);
        if (! (err < 0 || p_response == NULL || p_response->success <= 0) ) {
            ril_errno = RIL_E_SUCCESS;
            phone_storage_status = (available==1 ? 0 : 1);
        }
        RLOGD("Phone storage status: %d", phone_storage_status);
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);

}

void requestGetSmsParams(void *data, size_t datalen, RIL_Token t) {
    int err;
    char *line;
    RIL_SmsParams smsParams;
    ATResponse *p_response = NULL;

    SMS_UNUSED(data);
    SMS_UNUSED(datalen);

    err = at_send_command_singleline("AT+CSMP?", "+CSMP:", &p_response, SMS_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || p_response->success <= 0) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(smsParams.format));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(smsParams.vp));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(smsParams.pid));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(smsParams.dcs));
    if (err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &smsParams, sizeof(smsParams));
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestSetSmsParams(void *data, size_t datalen, RIL_Token t) {
    RIL_SmsParams *p_smsParams;
    char *line = NULL;
    ATResponse *p_response = NULL;
    int err;

    SMS_UNUSED(datalen);

    p_smsParams = (RIL_SmsParams *)data;
    if(p_smsParams->format != 0x00 && p_smsParams->format != 0x02) {
        RLOGD("invalid validity period format %d", p_smsParams->format);
        goto error;
    }

    asprintf(&line, "AT+CSMP=%d,%d,%d,%d", p_smsParams->format, p_smsParams->vp,
            p_smsParams->pid, p_smsParams->dcs);
    err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);
    if (err < 0 || p_response == NULL || p_response->success <= 0) goto error;
    /********************************************
    * free because it needs to send another AT
    * commands again
    ********************************************/
    at_response_free(p_response);
    p_response = NULL;
    // confirm modification
    asprintf(&line, "AT+CSAS");
    err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);
    if (err < 0 || p_response == NULL || p_response->success <= 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);

    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    return;
}

void requestSetEtws(void *data, size_t datalen, RIL_Token t) {
    int ret;
    int setting = ((int *)data)[0];
    ATResponse *p_response = NULL;
    char *line = NULL;

    SMS_UNUSED(datalen);

    asprintf(&line, "AT+ETWS=%d", setting);
    ret = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);

    if (ret < 0 || p_response == NULL || p_response->success == 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void onNewEtwsNotification(const char *line, RIL_SOCKET_ID rid) {
    RLOGD("[ETWS: enter onNewEtwsNotification");
    int err;
    char *p_cur = (char *)line;

    RIL_CBEtwsNotification response;

    /*
     * +ETWS: <warning type>,<message ID>,<serial number>,<PLMN_ID>[,<security info>]
     */

    err = at_tok_start(&p_cur);
    if (err < 0) goto error;

    err = at_tok_nextint(&p_cur, &(response.warningType));
    if(err < 0) goto error;

    err = at_tok_nextint(&p_cur, &(response.messageId));
    if(err < 0) goto error;

    err = at_tok_nextint(&p_cur, &(response.serialNumber));
    if(err < 0) goto error;

    err = at_tok_nextstr(&p_cur, &(response.plmnId));
    if(err < 0) goto error;

    if (at_tok_hasmore(&p_cur)) {
        err = at_tok_nextstr(&p_cur, &(response.securityInfo));
        if (err < 0) goto error;
    } else {
        response.securityInfo = NULL;
    }

    RIL_UNSOL_RESPONSE(
            RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION,
            &response, sizeof(RIL_CBEtwsNotification),
            rid);

    return;

error:
    RLOGE("[ETWS: There is something wrong with ETWS URC");
}

int rild_sms_handleCBConfig(CB_CONFIG_NODE **head, char *id_string, int selected) {
    RLOGD("rild_sms_handleCBConfig, id_string=%s,selected=%d", id_string, selected);
    int len, i, j;
    int value, begin;
    len = strlen(id_string);
    CB_CONFIG_NODE *node = (*head)->next, *pre = *head, *tem = NULL;
    if (len == 0)
        return 0;

    for(i = 0, begin = -1, value = -1; i <= len; i++, id_string++) {
        //RLOGD("%s", id_string);
        if ( (*id_string) == ' ' )
            continue;
        else if ( (*id_string) == ',' || (*id_string == '\0')) {
            RLOGD("rild_sms_handleCBConfig, id_string=%s", id_string);
            if (begin == -1) {
                /* %d, */
                begin = value;
            }
            RLOGD("rild_sms_handleCBConfig, id_string=%s,begin=%d,value=%d", id_string, begin, value);
            if (value >= 0){
                for (j = begin; j <= value; j++) {
                    while (node != NULL) {
                        if (node->value >= j) {
                            break;
                        } else {
                            pre = node;
                            node = node->next;
                        }
                    }
                    if (node == NULL) {
                        tem = calloc(1, sizeof(CB_CONFIG_NODE));
                        if (tem == NULL) {
                            RLOGE("rild_sms_handleCBConfig, failed to allocate memory");
                            break;
                        }
                        tem->value = j;
                        tem->selected = selected;
                        tem->next = NULL;
                        pre->next = tem;
                        node = pre->next;
                    } else if (node->value == j) {
                        node->selected = selected;
                    } else if (node->value > j){
                        tem = calloc(1, sizeof(CB_CONFIG_NODE));
                        if (tem == NULL) {
                            RLOGE("rild_sms_handleCBConfig, failed to allocate memory");
                            break;
                        }
                        tem->value = j;
                        tem->selected = selected;
                        tem->next = node;
                        pre->next = tem;
                        pre = pre->next;
                    }

                }

            }
            node = (*head)->next;
            pre = *head;
            begin = -1;
            value = -1;
        }
        else if ( (*id_string) == '-' )
        {
            begin = value;
            value = -1;
        }
        else if ( (*id_string) >= '0' && (*id_string) <= '9' )
        {
            if (value == -1) {
                value = 0;
            }
            value = value*10 + (*id_string) - '0';
        }
        else
        {
            RLOGE("There is some invalid characters: %x", (*id_string) );
            return -1;
        }
    }

    return 0;
}

void requestRemoveCbMsg(void *data, size_t datalen, RIL_Token t) {
    int ret;
    int channelId = ((int *)data)[0];
    int serailId = ((int *)data)[1];
    ATResponse *p_response = NULL;
    char *line = NULL;

    SMS_UNUSED(datalen);

    asprintf(&line, "AT+ECBMR=%d,%d", channelId, serailId);
    ret = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);

    if (ret < 0 || p_response == NULL || p_response->success == 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestGetGsmSmsBroadcastActivation(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    char *line, *chIDs, *dcsIDs;
    int mode[1] = {0};

    err = at_send_command_singleline("AT+CSCB?", "+CSCB:", &p_response, SMS_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || p_response->success == 0)
        goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) goto error;

    // get mode, +CSCB: <mode>. 0 denotes on and 1 denotes off
    err = at_tok_nextint(&line, &mode[0]);
    if(err < 0 || mode[0] < 0 || mode[0] > 1) goto error;

    // But 0 means off and 1 means on in the framework
    mode[0] = ((mode[0] == 0)? 1 : 0);

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, mode, sizeof(int));
    return;
error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, mode, sizeof(int));
}


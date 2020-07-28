/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include "sysenv_utils.h"
#include "MetaPub.h"
#include "meta_rat_para.h"

#undef LOG_TAG
#define LOG_TAG "RATMETA"

#define DBG(f, ...)   ALOGD("%s: " f, __func__, ##__VA_ARGS__)

#define TRUE 1
#define FALSE 0

#define FAIL 0
#define SUCCESS_REBOOT 1
#define SUCCESS_NO_REBOOT 2

/** the block of defination
    must be sync with RatConfiguration.java and ratconfig.c **/
#define PROPERTY_BUILD_RAT_CONFIG           "ro.vendor.mtk_protocol1_rat_config"
#define PROPERTY_RAT_CONFIG                 "ro.vendor.mtk_ps1_rat"
#define PROPERTY_IS_USING_DEFAULT_CONFIG    "ro.boot.opt_using_default"
#define PROPERTY_OPT_C2K_SUPPORT            "ro.vendor.mtk_c2k_support"
#define PROPERTY_OPT_LTE_SUPPORT            "ro.vendor.mtk_lte_support"

#define CDMA "C"
#define LteFdd "Lf"
#define LteTdd "Lt"
#define WCDMA "W"
#define TDSCDMA "T"
#define GSM "G"
#define DELIMITER "/"

#define MASK_CDMA      (1 << 5)
#define MASK_LteFdd    (1 << 4)
#define MASK_LteTdd    (1 << 3)
#define MASK_WCDMA     (1 << 2)
#define MASK_TDSCDMA   (1 << 1)
#define MASK_GSM       (1)
/*************************/
//check sum
#define MD_RAT_CS        "md_rat_cs"
#define OPT_MD1_SUPPORT  "opt_md1_support"
#define OPT_MD3_SUPPORT  "opt_md3_support"
#define OPT_IRAT_SUPPORT "opt_irat_support"
#define OPT_C2K_LTE_MODE "opt_c2k_lte_mode"
#define OPT_LTE_SUPPORT  "opt_lte_support"
#define OPT_PS1_RAT      "opt_ps1_rat"

#define MD_MODE_UNKNOWN    0
#define MD_MODE_LTG    8   // uLTG
#define MD_MODE_LWG    9   // uLWG
#define MD_MODE_LWTG   10  // uLWTG
#define MD_MODE_LWCG   11  // uLWCG
#define MD_MODE_LWCTG  12  // uLWTCG(Auto mode)
#define MD_MODE_LTTG   13  // LtTG
#define MD_MODE_LFWG   14  // LfWG
#define MD_MODE_LFWCG  15  // uLfWCG
#define MD_MODE_LCTG   16  // uLCTG
#define MD_MODE_LTCTG  17  // uLtCTG

#define MAX_RAT_STRING_LEN 64
#define METARAT_UNUSED(x) (void)(x)

static RAT_CNF rat_cnf;

static unsigned int max_rat = 0x0;
static int max_rat_initialized = FALSE;
static unsigned int actived_rat = 0x0;
static int is_default_config = 1;

static unsigned int ratToBitmask(const char* rat);
static char * ratToString(unsigned int iRat, char *out) ;
static unsigned int getMaxRat();
static int checkRatConfig(unsigned int iRat);
static unsigned int getRatConfig();

/*
 * transfer the rat from "L/T/G" to bitmask.
 * @return unsigned int, rat in bitmask.
 */
static unsigned int ratToBitmask(const char* rat) {
    unsigned int iRat = 0x0;
    if (strstr(rat, CDMA)  !=  NULL) {
        iRat = iRat | MASK_CDMA;
    }
    if (strstr(rat, LteFdd)  !=  NULL) {
        iRat = iRat | MASK_LteFdd;
    }
    if (strstr(rat, LteTdd)  != NULL) {
        iRat = iRat | MASK_LteTdd;
    }
    if (strstr(rat, WCDMA)  !=  NULL) {
        iRat = iRat | MASK_WCDMA;
    }
    if (strstr(rat, TDSCDMA) != NULL) {
        iRat = iRat | MASK_TDSCDMA;
    }
    if (strstr(rat, GSM) != NULL) {
        iRat = iRat | MASK_GSM;
    }
    return iRat;
}

static char * ratToString(unsigned int iRat, char *out) {
    char buffer[MAX_RAT_STRING_LEN] = { 0 };
    int len=0;
    if ((iRat & MASK_CDMA) == MASK_CDMA) {
        len += sprintf(buffer+len,"%s",DELIMITER);
        len += sprintf(buffer+len,"%s",CDMA);
    }
    if ((iRat & MASK_LteFdd) == MASK_LteFdd) {
      len += sprintf(buffer+len,"%s",DELIMITER);
      len += sprintf(buffer+len,"%s",LteFdd);
    }
    if ((iRat & MASK_LteTdd) == MASK_LteTdd) {
      len += sprintf(buffer+len,"%s",DELIMITER);
      len += sprintf(buffer+len,"%s",LteTdd);
    }
    if ((iRat & MASK_WCDMA) == MASK_WCDMA) {
      len += sprintf(buffer+len,"%s",DELIMITER);
      len += sprintf(buffer+len,"%s",WCDMA);
    }
    if ((iRat & MASK_TDSCDMA) == MASK_TDSCDMA) {
      len += sprintf(buffer+len,"%s",DELIMITER);
      len += sprintf(buffer+len,"%s",TDSCDMA);
    }
    if ((iRat & MASK_GSM) == MASK_GSM) {
      len += sprintf(buffer+len,"%s",DELIMITER);
      len += sprintf(buffer+len,"%s",GSM);
    }
    if (strlen(buffer)> 0) {
        // for remove the delimiter at rat[0]
        strncpy (out, &buffer[1], sizeof(buffer) - 1);
    }
    return out;
}

static int ratToModemMode(int iRat) {
    int modemMode = MD_MODE_UNKNOWN;
    if (iRat == (MASK_LteFdd | MASK_LteTdd | MASK_TDSCDMA | MASK_GSM)) {
        modemMode = MD_MODE_LTG;
    } else if (iRat == (MASK_LteTdd | MASK_TDSCDMA | MASK_GSM)) {
        modemMode = MD_MODE_LTTG;
    } else if (iRat == (MASK_LteFdd | MASK_LteTdd | MASK_TDSCDMA | MASK_WCDMA | MASK_GSM)) {
        modemMode = MD_MODE_LWTG;
    } else if (iRat == (MASK_LteFdd | MASK_WCDMA | MASK_GSM)) {
        modemMode = MD_MODE_LFWG;
    } else if (iRat == (MASK_LteFdd | MASK_LteTdd | MASK_WCDMA | MASK_GSM)) {
        modemMode = MD_MODE_LWG;
    } else if (iRat == (MASK_CDMA | MASK_LteFdd | MASK_LteTdd | MASK_TDSCDMA | MASK_WCDMA | MASK_GSM)) {
        modemMode = MD_MODE_LWCTG;
    } else if (iRat == (MASK_CDMA | MASK_LteFdd | MASK_LteTdd | MASK_TDSCDMA | MASK_GSM)) {
        modemMode = MD_MODE_LCTG;
    } else if (iRat == (MASK_CDMA | MASK_LteTdd | MASK_TDSCDMA | MASK_GSM)) {
        modemMode = MD_MODE_LTCTG;
    } else if (iRat == (MASK_CDMA | MASK_LteFdd | MASK_WCDMA | MASK_GSM)) {
        modemMode = MD_MODE_LFWCG;
    } else if (iRat == (MASK_CDMA | MASK_LteFdd | MASK_LteTdd | MASK_WCDMA | MASK_GSM)) {
        modemMode = MD_MODE_LWCG;
    } else if (iRat == (MASK_CDMA | MASK_WCDMA | MASK_GSM)) {
        modemMode = MD_MODE_LWCG;
    } else if (iRat == (MASK_CDMA | MASK_LteFdd | MASK_LteTdd | MASK_GSM)) {
        modemMode = MD_MODE_LWCG;
    } else if (iRat == (MASK_LteFdd | MASK_LteTdd)) {
        modemMode = MD_MODE_LWG;
    } else if (iRat == (MASK_LteFdd)) {
        modemMode = MD_MODE_LWG;
    }
    DBG("ratToModemMode:%d", modemMode);
    return modemMode;
}

/*
 * get the rat of project config
 * @return int, the rat of project config in bitmask.
 */
static unsigned int getMaxRat() {
    if ( !max_rat_initialized ) {
        char rat[PROPERTY_VALUE_MAX];
        int mIsDefault = 1;
        property_get(PROPERTY_BUILD_RAT_CONFIG, rat, "");
        DBG("getMaxRat: initial %s", rat);
        max_rat = ratToBitmask(rat);
        property_get(PROPERTY_IS_USING_DEFAULT_CONFIG, rat, "1");
        mIsDefault = atoi(rat);
        if (mIsDefault != 0) {
            // is_using_default is not 0, using default config.
            is_default_config = 1;
        } else {
            is_default_config = 0;
        }
        max_rat_initialized = TRUE;
    }
    //DBG("getMaxRat: %d", max_rat);
    return max_rat;
}

/*
 * check the iRat is supported by project config
 * @params unsinged int iRat, the rat to be check.
 * @return int,
 *           1, supported.
 *           0, not supported.
 */
static int checkRatConfig(unsigned int iRat) {
    unsigned int maxrat = 0x0;
    maxrat = getMaxRat();
    if ((iRat | maxrat) == maxrat) {
        //DBG("checkRatConfig: supported (%X/%X)", iRat, maxrat);
        return TRUE;
    } else {
        DBG("checkRatConfig: not supported (%X/%X)", iRat, maxrat);
        return FALSE;
    }
}

/*
 * get the active rat with the bitmask format
 * @return unsigned int, the active rat in bitmask.
 */
static unsigned int getRatConfig() {
    char rat[PROPERTY_VALUE_MAX];
    unsigned int default_rat_config = getMaxRat();
    if (default_rat_config == 0) {
        actived_rat = 0;
        return actived_rat;
    }
    if (is_default_config) {
        actived_rat = default_rat_config;
        return actived_rat;
    }
    property_get(PROPERTY_RAT_CONFIG, rat, "");
    if (strlen(rat) > 0) {
        actived_rat = ratToBitmask(rat);
        if (checkRatConfig(actived_rat) == FALSE) {
            DBG("getRatConfig: illegal value of PROPERTY_RAT_CONFIG, set max");
            actived_rat = getMaxRat();
        }
    } else {
        DBG("getRatConfig: active rat is not exist, set max");
        actived_rat = getMaxRat();
    }
    return actived_rat;
}

int SetModeToKernal(int modemMode, int iRat) {
    int i;
    char *name = NULL;
    char *value = NULL;
    int isCdmaSupport = (iRat& MASK_CDMA) == MASK_CDMA ? TRUE : FALSE;
    int isLteFddSupport = (iRat& MASK_LteFdd) == MASK_LteFdd ? TRUE : FALSE;
    int isLteTddSupport = (iRat& MASK_LteTdd) == MASK_LteTdd ? TRUE : FALSE;
    int isWcdmaSupport = (iRat& MASK_WCDMA) == MASK_WCDMA ? TRUE : FALSE;
    int isTdscdmaSupport = (iRat& MASK_TDSCDMA) == MASK_TDSCDMA ? TRUE : FALSE;
    int isGsmSupport = (iRat& MASK_GSM) == MASK_GSM ? TRUE : FALSE;
    char rat[PROPERTY_VALUE_MAX] = { 0 };
    int c6m_1rild = 0;
    int isDynamicSwitch;
    char property_value[PROPERTY_VALUE_MAX] = {0};
    property_get("ro.vendor.mtk_telephony_switch", property_value, "0");
    isDynamicSwitch = atoi(property_value);
    if (isDynamicSwitch == 0) {
        DBG("SetModeToKernal return in non-dynamic project");
        return TRUE;
    }
    property_get("ro.vendor.mtk_ril_mode", property_value, "0");
    if (strcmp(property_value, "c6m_1rild") == 0) {
        c6m_1rild = 1;
    }

    ratToString(iRat,rat);
    DBG("SetModeToKernal enter, modem=%d, iRat=%d, Rat=%s", modemMode, iRat, rat);
    DBG("SetModeToKernal rat - %d/%d/%d/%d/%d/%d",
            isCdmaSupport,isLteFddSupport,isLteTddSupport,
            isWcdmaSupport,isTdscdmaSupport,isGsmSupport);

    for (i = 0 ; i < 7 ; i++) {
        switch (i) {
            case 0:
                name = MD_RAT_CS;
                //Use rat bitmask as checksum, this MUST be the first item.
                //The idea is the 1st item and the last item must wirte to lk_env saftly
                asprintf(&value, "%d", iRat);
                break;
            case 1:
                name = OPT_MD1_SUPPORT;
                asprintf(&value, "%d", modemMode);
                break;
            case 2:
                name = OPT_MD3_SUPPORT;
                if (c6m_1rild == 0 && (modemMode == MD_MODE_LWCG ||
                        modemMode == MD_MODE_LWCTG)) {
                    asprintf(&value, "%d", 2);
                } else {
                    asprintf(&value, "%d", 0);
                }
                break;
            case 3:
                name = OPT_C2K_LTE_MODE;
                if (modemMode == MD_MODE_LWCG ||
                        modemMode == MD_MODE_LWCTG) {
                    asprintf(&value, "%d", 2);
                } else {
                    asprintf(&value, "%d", 0);
                }
                break;
            case 4:
                name = OPT_IRAT_SUPPORT;
                if (modemMode == MD_MODE_LWCTG ||
                        (modemMode == MD_MODE_LWCG &&
                        (isLteFddSupport && isLteTddSupport))) {
                    asprintf(&value, "%d", 1);
                } else {
                    asprintf(&value, "%d", 0);
                }
                break;
            case 5:
                name = OPT_LTE_SUPPORT;
                if (isLteFddSupport ||isLteTddSupport) {
                    asprintf(&value, "%d", 1);
                } else {
                    asprintf(&value, "%d", 0);
                }
                break;
            case 6:
                name = OPT_PS1_RAT;
                //To calculate the check sum, this MUST be the last item
                asprintf(&value, "%s", rat);
                break;
            default:
                break;
        }

        DBG("updateMode write %s = %s\n", name, value);
        if (name && value) {
            sysenv_set_static(name, value);
            DBG("updateMode read  %s = %s\n", name, sysenv_get_static(name));
        }
        if (value) {
            free (value);
            value = NULL;
        }
    }
    return TRUE;
}

/*
 * set current active rat.
 * @param String rat format like C/Lf/Lt/W/T/G
 * @return RatConfiguration.Result as following
 *	   0 as RatConfiguration.Result.FAIL :
		   fail
 *	   1 as RatConfiguration.Result.SUCCESS_REBOOT:
		   success, and reboot is necessary
 *	   2 as RatConfiguration.Result.SUCCESS_NO_REBOOT:
		   success, no need to reboot
 */
static unsigned int setRatConfig(unsigned char* rat) {
    int iRat = ratToBitmask((const char*)rat) ;
    DBG("setRatConfig:%d", iRat);
    if (checkRatConfig(iRat) == false) {
        return 0;
    }
    if (iRat == (int)getRatConfig()) {
        DBG("setRatConfig SUCCESS_NO_REBOOT, already this rat:%s", rat);
        return 2;
    }
    if (SetModeToKernal(ratToModemMode(iRat), iRat) == TRUE) {
        DBG("setRatConfig SUCCESS_REBOOT");
        return 1;
    } else {
        DBG("setRat FAIL");
        return 0;
    }
}

bool META_RAT_init() {
    DBG("start");

    char temp[128]={ 0 };
    property_get("ro.vendor.mtk_protocol1_rat_config", temp, "");
    DBG("ro.vendor.mtk_protocol1_rat_config:%s\n", temp);

    return true;
}

void META_RAT_deinit() {
}

void META_RAT_OP(RAT_REQ *req, char *peer_buff, unsigned short peer_len) {

    char temp[PROPERTY_VALUE_MAX]={ 0 };
    memset(&rat_cnf, 0, sizeof(RAT_CNF));
    rat_cnf.header.id = FT_RATCONFIG_CNF_ID;
    rat_cnf.header.token = req->header.token;
    rat_cnf.op = req->op;

    METARAT_UNUSED(peer_buff);
    METARAT_UNUSED(peer_len);

    DBG("req->op:%d\n", req->op);
    switch(req->op) {
        case RAT_OP_READ_OPTR:
            property_get("persist.vendor.operator.optr", temp, "");
            memcpy(rat_cnf.RAT_result.m_op.m_optr, &temp, MAX_RAT_STRING_LEN);
            rat_cnf.drv_status = TRUE;
            rat_cnf.status = META_SUCCESS;
            DBG("optr:%s - %s\n", temp, rat_cnf.RAT_result.m_op.m_optr);
            WriteDataToPC(&rat_cnf, sizeof(RAT_CNF), NULL, 0);
            break;

        case RAT_OP_READ_OPTRSEG:
            property_get("persist.vendor.operator.seg", temp, "");
            memcpy(rat_cnf.RAT_result.m_seg.m_optrseg, &temp, MAX_RAT_STRING_LEN);
            rat_cnf.drv_status = TRUE;
            rat_cnf.status = META_SUCCESS;
            DBG("optrseg:%s - %s\n", temp, rat_cnf.RAT_result.m_seg.m_optrseg);
            WriteDataToPC(&rat_cnf, sizeof(RAT_CNF), NULL, 0);
            break;

        case RAT_OP_GET_CURRENT_RAT:
            ratToString(getRatConfig(), temp);
            memcpy(rat_cnf.RAT_result.m_rat.m_RatValue, &temp, MAX_RAT_STRING_LEN);
            rat_cnf.drv_status = TRUE;
            rat_cnf.status = META_SUCCESS;
            DBG("current rat:%s - %s\n", temp, rat_cnf.RAT_result.m_rat.m_RatValue);
            WriteDataToPC(&rat_cnf, sizeof(RAT_CNF), NULL, 0);
            break;

        case RAT_OP_SET_NEW_RAT:
            rat_cnf.RAT_result.m_set.m_Ret = setRatConfig(req->cmd.m_rat.m_setRatValue);
            rat_cnf.drv_status = TRUE;
            rat_cnf.status = META_SUCCESS;
            WriteDataToPC(&rat_cnf, sizeof(RAT_CNF), NULL, 0);
            break;

        default:
            DBG("Not support OPCODE:req->op [%d]\n", req->op);
            rat_cnf.status = META_FAILED;
            WriteDataToPC(&rat_cnf, sizeof(RAT_CNF), NULL, 0);
            break;
    }
}

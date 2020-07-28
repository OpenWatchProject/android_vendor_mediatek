/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#include <libmtkrilutils.h>
#include <telephony/mtk_ril.h>
/// M: [Network][C2K] add for rat config in 3/4/5M. @{
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/base64.h>
#include <openssl/mem.h>
#include <mtk_log.h>
#include <mtk_properties.h>
#include <mtkconfigutils.h>
/// M: [Network][C2K] start @{
#include <ratconfig.h>
/// @}

#define NUM_ELEMS(a)    (sizeof (a) / sizeof (a)[0])
#define LOG_TAG "RilUtils"

static const char* VZW_MCC_MNC[] =
{
    "310004", "310005", "310006", "310010", "310012", "310013",
    "310350", "310590", "310820", "310890", "310910", "311012",
    "311110", "311270", "311271", "311272", "311273", "311274",
    "311275", "311276", "311277", "311278", "311279", "311280",
    "311281", "311282", "311283", "311284", "311285", "311286",
    "311287", "311288", "311289", "311390", "311480", "311481",
    "311482", "311483", "311484", "311485", "311486", "311487",
    "311488", "311489", "311590", "312770"
};

static const char *sAtCmdWithoutContent[] = {
    "+ICCID",
    "AT+CMGS", // Send SMS
    "+CMT",    // New SMS
    "AT+CMGW", // Write SMS to SIM
    "AT+C2KCMGS", // Send 3GPP2 SMS over IMS
    "+C2KCMT",  // New 3GPP2 SMS over IMS
    "AT+EC2KCMGS", // Send 3GPP2 SMS
    "+EC2KCMT",    // New 3GPP2 SMS
    "AT+EC2KCMGW", // Write SMS to RUIM/CSIM
    "AT+EC2KCNMA",
    "AT+CGLA",
    "+CGLA",
    "+ESLOTSINFO",
    "AT+ECLCK=",
// External SIM [Start]
    "+ERSAIND",
    "AT+ERSA",
    "+ERSAAUTH",
    "AT+ERSIMATO",
// External SIM [End]
    "+CPBR",
    "AT+EPBW",
    "AT+EPBUM",
    "+EPBUM",
    "AT+CPIN=", //Verify PIN
    "AT+CPIN2=",
    "AT+EPIN1=",
    "AT+EPIN2=",
    "AT+CPUK=",
    "AT+CPUK2=",
    "AT+ESLBLOB=3,",
    "AT+CPWD=",
    "AT+CLCK=",
    "AT+EAUTH", //SIM AUTH
    "AT+ESIMAUTH", //SIM AUTH
    "+CRSM", //Read EFiccid
    "+ESIMIND: 3",  // SIM cache file
    "+CNUM",
    "+STKPCI: 1",
    "+STKCTRL:",
    "AT+STKTR=\"81030123",
    "AT+EGMR",
/// M: CC: sensitive info hidden settings -start
    "ATDE",
    "ATD",
    "+CCWA",
    "+CDIP",
    "+CLIP",
    "+COLP",
    "+CNAP",
    "+ECPI",
    "+EAIC",
    "+EICPGU",
    "+CSSU",
    "+CLCC",
    "ATDER",
/// M: CC: sensitive info hidden settings -end
/// M: Ims CC -start
    "AT+EDCONF",
    "AT+ECONF",
    "+EDCONF",
    "+CDU",
    "+ECONF",
    "+ECALLPULL",
    "+ECT",
    "+EIMSEVTPKG",
/// M: Ims CC -end
/// M: SS sensitive info hidden settings -start
    "AT+ECUSD",  // SS
    "+CCFC",     // CF
    "+ECFU",     // CFU from MD
    "+EIMSXUI",
    "AT+EGBA",
    "+EGBA",
    "AT+EIUSD",
    "+ECMCCSS",
/// M: SS -end
    "+REDIRNUM",
    "+CEXTD",
    "+CFNM",
    "AT+ECRLA",
    "+ECRLA",
    "AT+EAPNSET",
    "AT+EIAAPN",
    "AT+CLCC",
    "AT+VTS",
    "AT+EVTS",
    "AT+CFSH",
    "+CFSH", // Unused URC, but contains sensitive data, so need to filter
    "+CORG", // Unused URC, but contains sensitive data, so need to filter
// MWIS [Start]
    "AT+EIMSGEO",
    "+EIMSGEO",
    "AT+EWIFIADDR",
    "AT+EWIFIASC",
    "AT+EWIFINATT",
    "+EWOKEEPALIVE",
// MWIS [End]
    "+EDMFAPP",
    "+EIMSCMODE",
// Data - Start
    "AT+EPDN",
    "+EPDN",
    "+CGCONTRDP:",
    "+CGDCONT:",
    "+CGPADDR:",
    "+CGPRCO:",
    "+EGCONTRDP:",
// Data - End
// NW [Start]
    "+VLOCINFO",  // sid,nid,bs_id,bs_lat
    "+EAPC",  // PseudoCell lac,cid
    "+EREG",  // lac, cid
    "+EGREG", // lac, cid
    "+ECELL",  // lac, cid
    "+CREG",
    "+CGREG",
    "+CEREG",
// NW [END]
// RTT [Start]
    "AT+ERTTSTR",  //sned RTT text
    "+ERTTSTR",  //receive RTT text
// RTT [END]
    "+EIMSREGURI"  // receive IMS registration URI information
};

static const char *s_atcmd_log_reduction[] = {
    "AT+ESRVSTATE",
    "+ECBDINFO",
    "AT+ECBDINFO",
    "AT+EDEFROAM",
    "+EDEFROAM",
    "AT+EIPRL",
    "+EIPRL",
};

int isLteSupport() {
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("ro.vendor.mtk_lte_support", property_value, "0");
    return atoi(property_value);
}

int getSimCount() {
    int simCount = 1;
#ifdef FUSION_RIL
    FeatureValue featurevalue;
    memset(featurevalue.value, 0, sizeof(featurevalue.value));
    mtkGetFeature(CONFIG_SIM_MODE, &featurevalue);
    if (strcmp(featurevalue.value, "ss") == 0) {
        simCount = 1;
    } else if (strcmp(featurevalue.value, "dsds") == 0 ||
            strcmp(featurevalue.value, "dsda") == 0) {
        simCount = 2;
    } else if (strcmp(featurevalue.value, "tsts") == 0) {
        simCount = 3;
    } else if (strcmp(featurevalue.value, "qsqs") == 0) {
        simCount = 4;
    }
#else
    // GSM RILD refer this property to decide SIM count
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("persist.vendor.radio.msimmode", property_value, "0");
    if (strcmp(property_value, "ss") == 0) {
        simCount = 1;
    } else if (strcmp(property_value, "dsds") == 0 ||
            strcmp(property_value, "dsda") == 0) {
        simCount = 2;
    } else if (strcmp(property_value, "tsts") == 0) {
        simCount = 3;
    } else if (strcmp(property_value, "qsqs") == 0) {
        simCount = 4;
    }
#endif
    return simCount;
}

int getMdMaxSimCount() {
#if defined(MD_PS_COUNT_2)
#define MD_PS_NUM 2
#elif defined(MD_PS_COUNT_3)
#define MD_PS_NUM 3
#elif defined(MD_PS_COUNT_4)
#define MD_PS_NUM 4
#else
#define MD_PS_NUM 1
#endif
    return MD_PS_NUM;
}

int isCCCIPowerOffModem() {
    /* Return 1 if CCCI support power-off modem completely and deeply.
       If not support, we switch on/off modem by AT+EPON and AT+EPOF */
    return 1;
}

int isSupportCommonSlot() {
    /* Return 1 if support SIM Hot Swap with Common Slot Feature */
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("ro.vendor.mtk_sim_hot_swap_common_slot", property_value, "0");
    mtkLogD(LOG_TAG, "ro.vendor.mtk_sim_hot_swap_common_slot:[%d]", atoi(property_value));
    if(strcmp("1", property_value) == 0) {
        return 1;
    } else {
        return 0;
    }
}

int RIL_get3GSIM() {
    char tmp[MTK_PROPERTY_VALUE_MAX] = { 0 };
    int simId = 0;

    mtk_property_get(PROPERTY_3G_SIM, tmp, "1");
    simId = atoi(tmp);
    return simId;
}

int isEmulatorRunning() {
    int isEmulatorRunning = 0;
    char *qemu = malloc(sizeof(char) * MTK_PROPERTY_VALUE_MAX);
    if (qemu == NULL) {
        return 0;
    }
    memset(qemu, 0, sizeof(char) * MTK_PROPERTY_VALUE_MAX);
    mtk_property_get("ro.kernel.qemu", qemu, "");
    if (strcmp(qemu, "1") == 0)
        isEmulatorRunning = 1;

    free(qemu);
    return isEmulatorRunning;
}

int isInternalLoad() {
    #ifdef __PRODUCTION_RELEASE__
        return 0;
    #else
        char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
        mtk_property_get("vendor.ril.emulation.production", property_value, "0");
        return (strcmp("1", property_value) != 0);
    #endif /* __PRODUCTION_RELEASE__ */
}

int isEngLoad() {
    int isEngLoad = 0;
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("ro.build.type", property_value, "");
    isEngLoad = (strcmp("eng", property_value) == 0);
    return isEngLoad;
}

int isUserLoad() {
    int isUserLoad = 0;
    char property_value_emulation[MTK_PROPERTY_VALUE_MAX] = { 0 };
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("vendor.ril.emulation.userload", property_value_emulation, "0");
    if(strcmp("1", property_value_emulation) == 0) {
        return 1;
    }
    mtk_property_get("ro.build.type", property_value, "");
    isUserLoad = (strcmp("user", property_value) == 0);
    return isUserLoad;
}

void setMSimProperty(int phoneId, char *pPropertyName, char *pUpdateValue) {
    #define MAX_PHONE_NUM 4
    #define MIN(a,b) ((a)<(b) ? (a) : (b))

    char oldItemValue[MTK_PROPERTY_VALUE_MAX] = {0};
    char newPropertyValue[MTK_PROPERTY_VALUE_MAX] = {0};
    int i = 0;
    int strLen = 0;

    if (pUpdateValue != NULL && strchr(pUpdateValue, ',') != NULL) {
        mtkLogD(LOG_TAG, "setMSimProperty ERROR: ',' is found");
    }
    for (i = 0; i < MAX_PHONE_NUM; i++) {
        if (i == phoneId) {
            // use new value
            strncat(newPropertyValue, pUpdateValue, MTK_PROPERTY_VALUE_MAX - strlen(newPropertyValue) -1);
        } else {
            getMSimProperty(i, pPropertyName, oldItemValue);
            strncat(newPropertyValue, oldItemValue, MTK_PROPERTY_VALUE_MAX - strlen(newPropertyValue) -1);
        }
        if (i != MAX_PHONE_NUM-1 && (strlen(newPropertyValue) < MTK_PROPERTY_VALUE_MAX -2)) {
            strncat(newPropertyValue, ",", 1);
        }
        memset(oldItemValue, 0, MTK_PROPERTY_VALUE_MAX);
    }
    //RLOGI("setMSimProperty phoneId=%d, newPropertyValue=%s", phoneId, newPropertyValue);
    // remove no use ','
    strLen = strlen(newPropertyValue);
    for (i = (strLen-1); i >= 0; i--) {
        if (newPropertyValue[i] == ',') {
            // remove
            newPropertyValue[i] = '\0';
        } else {
            break;
        }
    }
    //RLOGI("newPropertyValue %s\n", newPropertyValue);
    mtk_property_set(pPropertyName, newPropertyValue);
}

void getMSimProperty(int phoneId, char *pPropertyName,char *pPropertyValue) {
    char prop[MTK_PROPERTY_VALUE_MAX] = {0};
    char value[MTK_PROPERTY_VALUE_MAX] = {0};
    int count= 0;
    int propLen = 0;
    int i = 0;
    int j = 0;

    mtk_property_get(pPropertyName, prop, "");
    //RLOGI("getMSimProperty pPropertyName=%s, prop=%s", pPropertyName, prop);
    propLen = strlen(prop);
    for (i = 0; i < propLen; i++) {
        if(prop[i] == ',') {
            count++;
            if((count-1) == phoneId) {
                // return current buffer
                //RLOGI("getMSimProperty found! phoneId=%d, value =%s", phoneId, value);
                strncpy(pPropertyValue, value, strlen(value));
                return;
            } else {
                // clear current buffer
                j = 0;
                memset(value, 0, sizeof(char) * MTK_PROPERTY_VALUE_MAX);
            }
        } else {
            value[j] = prop[i];
            j++;
        }
    }
    if (count == phoneId) {
        strncpy(pPropertyValue, value, strlen(value));
        //RLOGI("getMSimProperty found at end! phoneId=%d, value =%s", phoneId, value);
    }
}

/*
 * @src: the source string that needed to be masked
 * @dst: caller must allocate enough memory space(larger than src string length)
 * @maskNum: specifies the num chars to be masked; 7 means mask the first 7 chars, -7 means mask
 *           the last 7 chars. Mask half string when maskNum > src string length
 * @maskPtn: the pattern used to mask src string(ex: 'X', '*'...)
 */
void maskString(char *src, char *dst, int maskNum, char maskPtn) {
    int stgLen = -1;
    int maskFromBegin = 1;

    if (src == NULL || dst == NULL) {
        goto error;
    }
    stgLen = strlen(src);
    if (stgLen == 0) {
        goto error;
    } else if (maskNum < 0) {
        maskFromBegin = 0;
        maskNum = -maskNum;
    }
    if (maskNum > stgLen) {
        maskNum = stgLen >> 1;
    }
    if (maskFromBegin) {
        strncpy(dst, src, stgLen);
        memset(dst, maskPtn, maskNum * sizeof(char));
    } else {
        memset(dst, maskPtn, stgLen * sizeof(char));
        strncpy(dst, src, stgLen - maskNum);
    }
    dst[stgLen] = '\0';
    return;
error:
    if (dst != NULL) {
        dst[0] = '\0';
    }
    mtkLogD(LOG_TAG, "[maskString] error[%d]", stgLen);
    return;
}

void givePrintableIccid(char *iccid, char *maskIccid) {
    if (maskIccid == NULL) {
        return;
    }
    maskString(iccid, maskIccid, 16, 'X');
}

// Notice: caller should free the return string by himself.
unsigned char* pii(char* input) {
#ifdef MTK_SHA_SUPPORT
    if (input == NULL) {
        return NULL;
    }

    // according to Rlog, input should executes sha1 and then use base64 to encode.
    unsigned char sha1Result[20] = {0};
    unsigned char *outText = NULL;
    size_t outLength = 0;

    SHA1((const uint8_t *)input, strlen(input), sha1Result);
    if (!EVP_EncodedLength(&outLength, sizeof(sha1Result))) {
        mtkLogD(LOG_TAG, "pii: can not get length of base64");
        return NULL;
    }
    outText = (unsigned char *) OPENSSL_malloc(outLength);
    if (outText == NULL) {
        mtkLogD(LOG_TAG, "pii: malloc fail");
        return NULL;
    }
    EVP_EncodeBlock((uint8_t *) outText, sha1Result, sizeof(sha1Result));

    // telephony java uses URL_SAFE, NO_PADDING, NO_WRAP
    // URL_SAFE => replace '+' with '-', '/' with '_'
    // NO_PADDING => without '='
    // NO_WRAP => use BIO_set_flags to achieve
    size_t i;
    for (i = 0; i < outLength; i++) {
        if (outText[i] == '+') {
            outText[i] = '-';
        } else if (outText[i] == '/') {
            outText[i] = '_';
        } else if (outText[i] == '=') {
            outText[i] = '\0';
        }
    }
    return outText;
#else
    mtkLogD(LOG_TAG, "pii: not support SHA, return NULL");
    return NULL;
#endif
}

int isSrlteSupport() {
    int isSrlteSupport = 0;
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("ro.vendor.mtk_c2k_lte_mode", property_value, "0");
    isSrlteSupport = atoi(property_value);
    return (isSrlteSupport == 2);
}

int isSvlteSupport() {
    int isSvlteSupport = 0;
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("ro.vendor.mtk_c2k_lte_mode", property_value, "0");
    isSvlteSupport = atoi(property_value);
    return (isSvlteSupport == 1);
}

int isSupportC2kSim() {
    if (isSrlteSupport() || isSvlteSupport()) {
        return 1;
    } else {
        return 0;
    }
}

int readMccMnc(RIL_SOCKET_ID rid, char *mccmnc) {
    char* prop = NULL;
    int ret = READ_MCC_MNC_ERR_SUCCESS;
    if (mccmnc == NULL || rid < RIL_SOCKET_1 || rid >= RIL_SOCKET_NUM) {
        return READ_MCC_MNC_ERR_NULL_MEM;
    }
    if (rid == RIL_SOCKET_1) {
        asprintf(&prop, "%s", PROPERTY_MCC_MNC);
    } else {
        asprintf(&prop, "%s.%c", PROPERTY_MCC_MNC, rid + '0');
    }
    if (prop != NULL) {
        mtk_property_get(prop, mccmnc, "0");
    } else {
        return READ_MCC_MNC_ERR_NOT_READY;
    }

    if (strcmp("N/A", mccmnc) == 0) {
        ret = READ_MCC_MNC_ERR_NOT_READY;
    }
    mtkLogD(LOG_TAG, "prop %s, readMccMnc %s, ret %d", prop, mccmnc, ret);
    free(prop);
    return ret;
}

int isCdmaLteDcSupport() {
    int isCdmaLteSupport = 0;
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("ro.vendor.mtk_c2k_lte_mode", property_value, "0");
    isCdmaLteSupport = atoi(property_value);
    return isCdmaLteSupport ? 1 : 0;
}

int isDisableCapabilitySwitch() {
    int ret = 0;
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("ro.vendor.mtk_disable_cap_switch", property_value, "0");
    ret = atoi(property_value);
    if (ret == 0) {
        mtk_property_get("persist.vendor.radio.simswitch.emmode", property_value, "1");
        ret = atoi(property_value);
        if (ret == 0) {
            ret = 1;
        } else {
            ret = 0;
        }
    }

    return ret;
}

int isDualTalkMode() {
    // DT support is phase-out
    return 0;
}

// Multi-PS Attach Start
int isMultiPsAttachSupport() {
    static int support = -1;
    if (support == -1) {
        char value[MTK_PROPERTY_VALUE_MAX] = {0};
        mtk_property_get("ro.vendor.mtk_data_config", value, "0");

        if ((atoi(value) & 1) == 1) {
            support = 1;
        } else {
            support = 0;
        }
    }
    return support;
}
// Multi-PS attach End

/**
 * Get the SVLTE slot id.
 * @return SVLTE slot id.
 *         1 : svlte in slot1
 *         2 : svlte in slot2
 */
int getActiveSvlteModeSlotId() {
    int i = 1;
    char tempstr[MTK_PROPERTY_VALUE_MAX];
    char *tok;

    memset(tempstr, 0, sizeof(tempstr));
    // 3 means SVLTE mode, 2 is CSFB mode in this persist.
    mtk_property_get("persist.vendor.radio.svlte_slot", tempstr, "2,2");
    tok = strtok(tempstr, ",");
    while(tok != NULL)
    {
        if (3 == atoi(tok)) {
            return i;
        }
        i++;
        tok = strtok(NULL, ",");
    }
    return -1;
}

/**
 * Return cdma modem`s slot
 * For E+G double talk
 * <return>
 *         1 : cdma in slot1
 *         2 : cdma in slot2
 */
int getCdmaModemSlot(void) {
    if (isCdmaLteDcSupport()) {
        return getCdmaSocketSlotId();
    }

    int ret = 0;
    char tempstr[MTK_PROPERTY_VALUE_MAX];

    memset(tempstr, 0, sizeof(tempstr));
    mtk_property_get("ril.external.md", tempstr, "2");

    ret = atoi(tempstr);
    mtkLogD(LOG_TAG, "via cdma modem is in slot%d", ret);
    return ret;
}

/**
 * Get slot id which connect to c2k rild socket.
 * @return slot id which connect to c2k rild socket
 *         1 : slot1 connect to c2k rild socket
 *         2 : slot2 connect to c2k rild socket
 */
int getCdmaSocketSlotId() {
    int ret = 0;
    char tempstr[MTK_PROPERTY_VALUE_MAX];

    memset(tempstr, 0, sizeof(tempstr));
    mtk_property_get("persist.vendor.radio.cdma_slot", tempstr, "1");

    ret = atoi(tempstr);
    return ret;
}

GEMINI_MODE getGeminiMode() {
    static GEMINI_MODE sProjType = GEMINI_MODE_UNKNOWN;
    if (sProjType == GEMINI_MODE_UNKNOWN) {
        char propValue[MTK_PROPERTY_VALUE_MAX] = {0};

        mtk_property_get("persist.vendor.radio.mtk_ps2_rat", propValue, "G");

        if (isLteSupport()) {
            if (strchr(propValue, 'L') != NULL) {
                sProjType = GEMINI_MODE_L_AND_L;
            } else if (strchr(propValue, 'W') != NULL) {
                sProjType = GEMINI_MODE_L_AND_W;
            } else {
                sProjType = GEMINI_MODE_L_AND_G;
            }
       } else {
            if (strchr(propValue, 'L') != NULL) {
                mtkLogD(LOG_TAG, "getProjectType, project config error.");
            } else if (strchr(propValue, 'W') != NULL) {
                sProjType = GEMINI_MODE_W_AND_W;
            } else {
                sProjType = GEMINI_MODE_W_AND_G;
            }
        }
    }
    mtkLogD(LOG_TAG, "getProjectType = %d", sProjType);
    return sProjType;
}

// Femtocell support Start
int isFemtocellSupport() {
    int isFemtocellSupport = 0;
    char optr[MTK_PROPERTY_VALUE_MAX] = {0};

    mtk_property_get("ro.vendor.mtk_femto_cell_support", optr, "0");
    isFemtocellSupport = atoi(optr);

    return isFemtocellSupport;
}
// Femtocell support End

int isEnableModulationReport()
{
    char optr[MTK_PROPERTY_VALUE_MAX] = {0};

    mtk_property_get("persist.vendor.operator.optr", optr, "");
    //RLOGI("[isEnableModulationReport] optr:%s", optr);

    return (strcmp("OP08", optr) == 0) ? 1 : 0;
}

int isDisable2G() {
    int ret = 0;
    char property_value[MTK_PROPERTY_VALUE_MAX] = {0};
    char optr[MTK_PROPERTY_VALUE_MAX] = {0};

    mtk_property_get("persist.vendor.operator.optr", optr, "");
    if (strcmp("OP07", optr) == 0) {
        mtk_property_get("persist.vendor.radio.disable.2g", property_value, "0");
        mtkLogD(LOG_TAG, "%s = %s", "persist.vendor.radio.disable.2g", property_value);
        if (atoi(property_value) == 1) {
            ret = 1;
        } else {
            ret = 0;
        }
    } else {
        ret = 0;
    }

    return ret;
}

const char* mtkRequestToString(int request) {
    switch(request) {
        /// M: [C2K 6M][NW] Set the SVLTE RAT mode@{
        case RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE: return "LOCAL_REQUEST_SET_SVLTE_RAT_MODE";
        /// M: [C2K 6M][NW] Set the SVLTE RAT mode@}
        /// M: [C2K 6M][NW]  @{
        case RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED: return "UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED";
        /// M: [C2K 6M][NW]  @}
        /* M: network part start */
        case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT : return "QUERY_AVAILABLE_NETWORKS_WITH_ACT";
        case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT: return "SET_NETWORK_SELECTION_MANUAL_WIT_ACT";
        case RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS : return "ABORT_QUERY_AVAILABLE_NETWORKS";
        case RIL_LOCAL_REQUEST_ABORT_AVAILABLE_NETWORK : return "LOCAL_ABORT_AVAILABLE_NETWORKS";
        case RIL_REQUEST_SIGNAL_STRENGTH_WITH_WCDMA_ECIO : return "GET_SIGNAL_STRENGTH_WITH_WCDMA_ECIO";
        case RIL_UNSOL_SIGNAL_STRENGTH_WITH_WCDMA_ECIO : return "UNSOL_SIGNAL_STRENGTH_WITH_WCDMA_ECIO";
        //Femtocell (CSG) feature START
        case RIL_REQUEST_GET_FEMTOCELL_LIST: return "RIL_REQUEST_GET_FEMTOCELL_LIST";
        case RIL_REQUEST_ABORT_FEMTOCELL_LIST: return "RIL_REQUEST_ABORT_FEMTOCELL_LIST";
        case RIL_REQUEST_SELECT_FEMTOCELL: return "RIL_REQUEST_SELECT_FEMTOCELL";
        case RIL_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE:
            return "RIL_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE";
        case RIL_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE:
            return "RIL_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE";
        //Femtocell (CSG) feature END
        case RIL_UNSOL_RESPONSE_CS_NETWORK_STATE_CHANGED: return "UNSOL_RESPONSE_CS_NETWORK_STATE_CHANGED";
        case RIL_UNSOL_INVALID_SIM: return "UNSOL_INVALID_SIM";
        case RIL_UNSOL_NETWORK_EVENT: return "RIL_UNSOL_NETWORK_EVENT";
        case RIL_UNSOL_MODULATION_INFO: return "RIL_UNSOL_MODULATION_INFO";
        /* M: network part end */
        case RIL_LOCAL_GSM_UNSOL_CARD_TYPE_NOTIFY: return "LOCAL_GSM_UNSOL_CARD_TYPE_NOTIFY";
        case RIL_LOCAL_GSM_UNSOL_CT3G_DUALMODE_CARD: return "LOCAL_GSM_UNSOL_CT3G_DUALMODE_CARD";
        case RIL_LOCAL_GSM_REQUEST_SWITCH_CARD_TYPE: return "RIL_LOCAL_GSM_REQUEST_SWITCH_CARD_TYPE";
         /// M: [C2K]Dynamic switch support. @{
        case RIL_REQUEST_ENTER_RESTRICT_MODEM: return "RIL_REQUEST_ENTER_RESTRICT_MODEM";
        case RIL_REQUEST_LEAVE_RESTRICT_MODEM: return "RIL_REQUEST_LEAVE_RESTRICT_MODEM";
        /// @}
        case RIL_REQUEST_GSM_DEVICE_IDENTITY: return "RIL_REQUEST_GSM_DEVICE_IDENTITY";
        case RIL_LOCAL_REQUEST_MODE_SWITCH_GSM_SET_TRM: return "MODE_SWITCH_GSM_SET_TRM";
        case RIL_LOCAL_REQUEST_MODE_SWITCH_C2K_SET_TRM: return "MODE_SWITCH_C2K_SET_TRM";
        case RIL_LOCAL_C2K_REQUEST_AT_COMMAND_WITH_PROXY_CDMA:
            return "AT_COMMAND_WITH_PROXY_CDMA";
        case RIL_REQUEST_RESUME_REGISTRATION: return "RIL_REQUEST_RESUME_REGISTRATION";
        case RIL_REQUEST_SET_REG_SUSPEND_ENABLED: return "RIL_REQUEST_SET_REG_SUSPEND_ENABLED";
        case RIL_UNSOL_RESPONSE_PLMN_CHANGED: return "RIL_UNSOL_RESPONSE_PLMN_CHANGED";
        case RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED: return "RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED";
        /// [C2K] IRAT feature code start.
        case RIL_LOCAL_GSM_REQUEST_SET_ACTIVE_PS_SLOT: return "RIL_LOCAL_GSM_REQUEST_SET_ACTIVE_PS_SLOT";
        case RIL_LOCAL_GSM_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE: return "RIL_LOCAL_GSM_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE";
        case RIL_LOCAL_GSM_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE: return "RIL_LOCAL_GSM_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE";
        /// [C2K] IRAT feature code finish.
        case RIL_REQUEST_SET_STK_UTK_MODE: return "RIL_REQUEST_SET_STK_UTK_MODE";
        // FastDormancy
        case RIL_REQUEST_SET_FD_MODE: return "RIL_REQUEST_SET_FD_MODE";
        case RIL_LOCAL_REQUEST_CDMA_SMS_SPECIFIC_TO_GSM: return "CDMA_SMS_SPECIFIC_TO_GSM";
        case RIL_LOCAL_GSM_UNSOL_CDMA_SMS_SPECIFIC_FROM_GSM: return "CDMA_SMS_SPECIFIC_FROM_GSM";
        /// M: [Network][C2K] Config EVDO mode. @{
        case RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE: return "RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE";
        /// @}
        /// M: [Network][C2K] Set the SVLTE RAT mode. @{
        case RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE: return "LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE";
        /// @}
        /// M: [Network][C2K] Set the FDD TDD mode @{
        case RIL_LOCAL_REQUEST_SET_FDD_TDD_MODE: return "LOCAL_REQUEST_SET_FDD_TDD_MODE";
        /// M: [Network][C2K] Set the FDD TDD mode @}
        /// M: [Network][C2K] get eng info. @{
        case RIL_LOCAL_C2K_UNSOL_ENG_MODE_NETWORK_INFO: return "RIL_LOCAL_C2K_UNSOL_ENG_MODE_NETWORK_INFO";
        /// @}
        case RIL_LOCAL_C2K_UNSOL_CDMA_CARD_READY: return "RIL_LOCAL_C2K_UNSOL_CDMA_CARD_READY";
        case RIL_LOCAL_C2K_UNSOL_CDMA_CARD_TYPE_NOTIFY: return "RIL_LOCAL_C2K_UNSOL_CDMA_CARD_TYPE_NOTIFY";
        case RIL_LOCAL_C2K_UNSOL_CDMA_IMSI_READY: return "RIL_LOCAL_C2K_UNSOL_CDMA_IMSI_READY";
        case RIL_LOCAL_C2K_REQUEST_SWITCH_CARD_TYPE: return "RIL_LOCAL_C2K_REQUEST_SWITCH_CARD_TYPE";
        /// M: [C2K]Dynamic switch support. @{
        case RIL_REQUEST_ENTER_RESTRICT_MODEM_C2K: return "RIL_REQUEST_ENTER_RESTRICT_MODEM_C2K";
        case RIL_REQUEST_LEAVE_RESTRICT_MODEM_C2K: return "RIL_REQUEST_LEAVE_RESTRICT_MODEM_C2K";
        // MTK-START: SIM
        case RIL_REQUEST_SIM_GET_ATR: return "SIM_GET_ATR";
        case RIL_REQUEST_SIM_GET_ICCID: return "RIL_REQUEST_SIM_GET_ICCID";
        case RIL_REQUEST_SET_SIM_POWER: return "RIL_REQUEST_SET_SIM_POWER";
        case RIL_LOCAL_GSM_UNSOL_EF_ECC: return "LOCAL_GSM_UNSOL_EF_ECC";
        case RIL_UNSOL_VIRTUAL_SIM_ON: return "RIL_UNSOL_VIRTUAL_SIM_ON";
        case RIL_UNSOL_VIRTUAL_SIM_OFF: return "RIL_UNSOL_VIRTUAL_SIM_OFF";
        case RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED: return "RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED";
        case RIL_UNSOL_IMEI_LOCK: return "RIL_UNSOL_IMEI_LOCK";
        case RIL_UNSOL_IMSI_REFRESH_DONE: return "RIL_UNSOL_IMSI_REFRESH_DONE";
        // MTK-END
        /// @}
        /// [C2K] IRAT feature code start.
        case RIL_LOCAL_C2K_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE: return "RIL_LOCAL_C2K_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE";
        case RIL_LOCAL_C2K_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE: return "RIL_LOCAL_C2K_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE";
        /// [C2K] IRAT feature code finish.
        case RIL_REQUEST_RESUME_REGISTRATION_CDMA: return "RIL_REQUEST_RESUME_REGISTRATION_CDMA";
        case RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA: return "RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA";
        case RIL_UNSOL_CDMA_PLMN_CHANGED: return "RIL_UNSOL_CDMA_PLMN_CHANGED";
        // ATCI start
        case RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL: return "RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL";
        case RIL_UNSOL_ATCI_RESPONSE: return "RIL_UNSOL_ATCI_RESPONSE";
        // ATCI end
        case RIL_REQUEST_MODEM_POWERON: return "RIL_REQUEST_MODEM_POWERON";
        case RIL_REQUEST_MODEM_POWEROFF: return "RIL_REQUEST_MODEM_POWEROFF";
        // SMS-START
        // SMS ready event
        case RIL_UNSOL_SMS_READY_NOTIFICATION: return "RIL_UNSOL_SMS_READY_NOTIFICATION";
        // Memory storage full
        case RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION: return "RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION";
        case RIL_UNSOL_ME_SMS_STORAGE_FULL: return "RIL_UNSOL_ME_SMS_STORAGE_FULL";
        case RIL_REQUEST_GET_SMS_SIM_MEM_STATUS: return "RIL_REQUEST_GET_SMS_SIM_MEM_STATUS";
        case RIL_REQUEST_GET_SMS_PARAMS: return "RIL_REQUEST_GET_SMS_PARAMS";
        case RIL_REQUEST_SET_SMS_PARAMS: return "RIL_REQUEST_SET_SMS_PARAMS";
        case RIL_REQUEST_GSM_SET_BROADCAST_LANGUAGE: return "GSM_SET_BROADCAST_LANGUAGE";
        case RIL_REQUEST_GSM_GET_BROADCAST_LANGUAGE: return "GSM_GET_BROADCAST_LANGUAGE";
        case RIL_REQUEST_SET_ETWS: return "SET_ETWS";
        case RIL_REQUEST_REMOVE_CB_MESSAGE: return "REMOVE_CB_MESSAGE";
        case RIL_REQUEST_IMS_SEND_SMS_EX: return "RIL_REQUEST_IMS_SEND_SMS_EX";
        case RIL_REQUEST_SMS_ACKNOWLEDGE_EX: return "RIL_REQUEST_SMS_ACKNOWLEDGE_EX";
        case RIL_REQUEST_SET_SMS_FWK_READY: return "SET_SMS_FWK_READY";
        case RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT_EX: return "RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT_EX";
        case RIL_UNSOL_RESPONSE_NEW_SMS_EX: return "RIL_UNSOL_RESPONSE_NEW_SMS_EX";
        // SMS-END
        /// M: eMBMS feature
        case RIL_REQUEST_EMBMS_AT_CMD: return "RIL_REQUEST_EMBMS_AT_CMD";

        case RIL_LOCAL_REQUEST_EMBMS_AT_CMD: return "RIL_LOCAL_REQUEST_EMBMS_AT_CMD";
        case RIL_LOCAL_REQUEST_EMBMS_ENABLE: return "RIL_LOCAL_REQUEST_EMBMS_ENABLE";
        case RIL_LOCAL_REQUEST_EMBMS_DISABLE: return "RIL_LOCAL_REQUEST_EMBMS_DISABLE";
        case RIL_LOCAL_REQUEST_EMBMS_START_SESSION: return "RIL_LOCAL_REQUEST_EMBMS_START_SESSION";
        case RIL_LOCAL_REQUEST_EMBMS_STOP_SESSION: return "RIL_LOCAL_REQUEST_EMBMS_STOP_SESSION";
        case RIL_LOCAL_REQUEST_EMBMS_GET_TIME: return "RIL_LOCAL_REQUEST_EMBMS_GET_TIME";
        case RIL_LOCAL_REQUEST_EMBMS_GET_COVERAGE_STATE: return "RIL_REQUEST_EMBMS_GET_COVERAGE_STATE";
        case RIL_LOCAL_REQUEST_EMBMS_SET_E911: return "RIL_LOCAL_REQUEST_EMBMS_SET_E911";
        case RIL_LOCAL_REQUEST_EMBMS_TRIGGER_CELL_INFO_NOTIFY: return "RIL_LOCAL_REQUEST_EMBMS_TRIGGER_CELL_INFO_NOTIFY";
        case RIL_LOCAL_REQUEST_SETUP_DATA_CALL_ALT: return "RIL_LOCAL_REQUEST_SETUP_DATA_CALL_ALT";
        case RIL_LOCAL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE_ALT: return "RIL_LOCAL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE_ALT";
        case RIL_LOCAL_REQUEST_PCSCF_DISCOVERY_PCO_ALT: return "RIL_LOCAL_REQUEST_PCSCF_DISCOVERY_PCO_ALT";
        case RIL_UNSOL_EMBMS_AT_INFO: return "RIL_UNSOL_EMBMS_AT_INFO";

        case RIL_LOCAL_GSM_UNSOL_EMBMS_START_SESSION_RESPONSE: return "RIL_LOCAL_GSM_UNSOL_EMBMS_START_SESSION_RESPONSE";
        case RIL_LOCAL_GSM_UNSOL_EMBMS_CELL_INFO_NOTIFICATION: return "RIL_LOCAL_GSM_UNSOL_EMBMS_CELL_INFO_NOTIFICATION";
        case RIL_LOCAL_GSM_UNSOL_EMBMS_COVERAGE_STATE: return "RIL_LOCAL_GSM_UNSOL_EMBMS_COVERAGE_STATE";
        case RIL_LOCAL_GSM_UNSOL_EMBMS_ACTIVE_SESSION: return "RIL_LOCAL_GSM_UNSOL_EMBMS_ACTIVE_SESSION";
        case RIL_LOCAL_GSM_UNSOL_EMBMS_AVAILABLE_SESSION: return "RIL_LOCAL_GSM_UNSOL_EMBMS_AVAILABLE_SESSION";
        case RIL_LOCAL_GSM_UNSOL_EMBMS_SAI_LIST_NOTIFICATION: return "RIL_LOCAL_GSM_UNSOL_EMBMS_SAI_LIST_NOTIFICATION";
        case RIL_LOCAL_GSM_UNSOL_EMBMS_OOS_NOTIFICATION: return "RIL_LOCAL_GSM_UNSOL_EMBMS_OOS_NOTIFICATION";
        /// M: eMBMS end
        case RIL_UNSOL_DATA_ALLOWED: return "RIL_UNSOL_DATA_ALLOWED";
        /// M: CC: Proprietary incoming call handling @{
        case RIL_REQUEST_SET_CALL_INDICATION: return "RIL_REQUEST_SET_CALL_INDICATION";
        case RIL_UNSOL_INCOMING_CALL_INDICATION: return "RIL_UNSOL_INCOMING_CALL_INDICATION";
        case RIL_UNSOL_CALL_ADDITIONAL_INFO: return "RIL_UNSOL_CALL_ADDITIONAL_INFO";
        /// @}
        /// M: CC: Proprietary ECC enhancement @{
        case RIL_REQUEST_SET_ECC_LIST: return "RIL_REQUEST_SET_ECC_LIST";
        /// @}
        /// M: CC: Proprietary call control hangup all
        case RIL_REQUEST_HANGUP_ALL: return "RIL_REQUEST_HANGUP_ALL";
        /// M: CC: Proprietary call control hangup all
        case RIL_REQUEST_HANGUP_WITH_REASON: return "RIL_REQUEST_HANGUP_WITH_REASON";
        case RIL_LOCAL_C2K_UNSOL_EF_ECC: return "LOCAL_C2K_UNSOL_EF_ECC";
        case RIL_REQUEST_SET_PS_REGISTRATION: return "RIL_REQUEST_SET_PS_REGISTRATION";
        /// M: CC: call control CRSS handling
        case RIL_UNSOL_CRSS_NOTIFICATION: return "RIL_UNSOL_CRSS_NOTIFICATION";
        /// M: CC: GSA HD Voice for 2/3G network support
        case RIL_UNSOL_SPEECH_CODEC_INFO: return "RIL_UNSOL_SPEECH_CODEC_INFO";
        /// M: CC: Call control force release call
        case RIL_REQUEST_FORCE_RELEASE_CALL: return "RIL_REQUEST_FORCE_RELEASE_CALL";
        /// M: CC: Normal/Emergency call redial
        case RIL_LOCAL_REQUEST_EMERGENCY_REDIAL: return "RIL_LOCAL_REQUEST_EMERGENCY_REDIAL";
        case RIL_LOCAL_REQUEST_EMERGENCY_SESSION_BEGIN: return "RIL_LOCAL_REQUEST_EMERGENCY_SESSION_BEGIN";
        case RIL_LOCAL_REQUEST_EMERGENCY_SESSION_END: return "RIL_LOCAL_REQUEST_EMERGENCY_SESSION_END";
        // Verizon hVoLTE/E911 interface
        case RIL_LOCAL_REQUEST_SET_HVOLTE_MODE: return "RIL_LOCAL_REQUEST_SET_HVOLTE_MODE";

        // APC
        case RIL_REQUEST_SET_PSEUDO_CELL_MODE: return "RIL_REQUEST_SET_PSEUDO_CELL_MODE";
        case RIL_REQUEST_GET_PSEUDO_CELL_INFO: return "RIL_REQUEST_GET_PSEUDO_CELL_INFO";
        case RIL_UNSOL_PSEUDO_CELL_INFO: return "RIL_UNSOL_PSEUDO_CELL_INFO";
        case RIL_REQUEST_GET_SMS_RUIM_MEM_STATUS: return "RIL_REQUEST_GET_SMS_RUIM_MEM_STATUS";
        //Reset Attach APN
        case RIL_UNSOL_RESET_ATTACH_APN: return "RIL_UNSOL_RESET_ATTACH_APN";
        // M: IA- change attach APN
        case RIL_UNSOL_DATA_ATTACH_APN_CHANGED: return "RIL_UNSOL_DATA_ATTACH_APN_CHANGED";
        // World Phone
        case RIL_REQUEST_MODIFY_MODEM_TYPE: return "RIL_REQUEST_MODIFY_MODEM_TYPE";
        case RIL_UNSOL_WORLD_MODE_CHANGED: return "RIL_UNSOL_WORLD_MODE_CHANGED";
        case RIL_UNSOL_GMSS_RAT_CHANGED: return "RIL_UNSOL_GMSS_RAT_CHANGED";
        // MTK-START: SIM GBA
        case RIL_REQUEST_GENERAL_SIM_AUTH: return "RIL_REQUEST_GENERAL_SIM_AUTH";
        // MTK-START: SIM HOT SWAP / SIM RECOVERY
        case RIL_UNSOL_SIM_PLUG_IN: return "RIL_UNSOL_SIM_PLUG_IN";
        case RIL_UNSOL_SIM_PLUG_OUT: return "RIL_UNSOL_SIM_PLUG_OUT";
        case RIL_UNSOL_SIM_MISSING: return "RIL_UNSOL_SIM_MISSING";
        case RIL_UNSOL_SIM_RECOVERY: return "RIL_UNSOL_SIM_RECOVERY";
        // MTK-START: SIM COMMON SLOT
        case RIL_UNSOL_TRAY_PLUG_IN: return "RIL_UNSOL_TRAY_PLUG_IN";
        case RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED: return "RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED";
        // MTK-END
        case RIL_UNSOL_SIM_POWER_CHANGED: return "RIL_UNSOL_SIM_POWER_CHANGED";
        case RIL_UNSOL_CARD_DETECTED_IND: return "RIL_UNSOL_CARD_DETECTED_IND";
        // MTK-START: SIM ME LOCK
        case RIL_REQUEST_QUERY_SIM_NETWORK_LOCK: return "RIL_REQUEST_QUERY_SIM_NETWORK_LOCK";
        case RIL_REQUEST_SET_SIM_NETWORK_LOCK: return "RIL_REQUEST_SET_SIM_NETWORK_LOCK";
        case RIL_REQUEST_ENTER_DEPERSONALIZATION: return "RIL_REQUEST_ENTER_DEPERSONALIZATION";
        // MTK-END
        // MTK-START: SIM SLOT LOCK
        case RIL_UNSOL_SIM_SLOT_LOCK_POLICY_NOTIFY: return "RIL_UNSOL_SIM_SLOT_LOCK_POLICY_NOTIFY";
        case RIL_REQUEST_ENTER_DEVICE_NETWORK_DEPERSONALIZATION:
            return "ENTER_DEVICE_NETWORK_DEPERSONALIZATION";
        // MTK-END
        // MTK-START: SIM TMO RSU
        case RIL_LOCAL_REQUEST_GET_SHARED_KEY:
            return "RIL_LOCAL_REQUEST_GET_SHARED_KEY";
        case RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS:
            return "RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS";
        case RIL_LOCAL_REQUEST_GET_SIM_LOCK_INFO:
            return "RIL_LOCAL_REQUEST_GET_SIM_LOCK_INFO";
        case RIL_LOCAL_REQUEST_RESET_SIM_LOCK_SETTINGS:
            return "RIL_LOCAL_REQUEST_RESET_SIM_LOCK_SETTINGS";
        case RIL_LOCAL_REQUEST_GET_MODEM_STATUS:
            return "RIL_LOCAL_REQUEST_GET_MODEM_STATUS";
        case RIL_UNSOL_MELOCK_NOTIFICATION: return "RIL_UNSOL_MELOCK_NOTIFICATION";
        // SIM ATT RSU
        case RIL_UNSOL_ATT_SIM_LOCK_NOTIFICATION: return "RIL_UNSOL_ATT_SIM_LOCK_NOTIFICATION";
        // MTK-END
        // M: MPS feature
        case RIL_LOCAL_REQUEST_DEACTIVATE_ALL_DATA_CALL: return "RIL_LOCAL_REQUEST_DEACTIVATE_ALL_DATA_CALL";
        case RIL_LOCAL_C2K_REQUEST_DEACTIVATE_ALL_DATA_CALL: return "RIL_LOCAL_C2K_REQUEST_DEACTIVATE_ALL_DATA_CALL";
        case RIL_REQUEST_SET_TRM: return "RIL_REQUEST_SET_TRM";
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE: return "STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE";
        case RIL_UNSOL_CDMA_CARD_INITIAL_ESN_OR_MEID: return "CDMA_CARD_INITIAL_ESN_OR_MEID";
        case RIL_LOCAL_REQUEST_AT_COMMAND_WITH_PROXY: return "RIL_LOCAL_REQUEST_AT_COMMAND_WITH_PROXY";
        // / M: BIP {
        case RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND: return "RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND";
        // / M: BIP }
        // SS : Start
        case RIL_REQUEST_SET_CLIP: return "RIL_REQUEST_SET_CLIP";
        case RIL_REQUEST_GET_COLP: return "RIL_REQUEST_GET_COLP";
        case RIL_REQUEST_GET_COLR: return "RIL_REQUEST_GET_COLR";
        case RIL_REQUEST_SEND_CNAP: return "RIL_REQUEST_SEND_CNAP";
        case RIL_UNSOL_CALL_FORWARDING: return "RIL_UNSOL_CALL_FORWARDING";
        // SS : End
        // IMS SS : Start
        case RIL_REQUEST_SET_COLP: return "RIL_REQUEST_SET_COLP";
        case RIL_REQUEST_SET_COLR: return "RIL_REQUEST_SET_COLR";
        case RIL_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT: return "RIL_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT";
        case RIL_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT: return "RIL_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT";
        case RIL_REQUEST_RUN_GBA: return "RIL_REQUEST_RUN_GBA";
        // IMS SS : End
        // PHB START
        case RIL_REQUEST_QUERY_PHB_STORAGE_INFO: return "RIL_REQUEST_QUERY_PHB_STORAGE_INFO";
        case RIL_REQUEST_WRITE_PHB_ENTRY: return "RIL_REQUEST_WRITE_PHB_ENTRY";
        case RIL_REQUEST_READ_PHB_ENTRY: return "RIL_REQUEST_READ_PHB_ENTRY";
        case RIL_REQUEST_QUERY_UPB_CAPABILITY: return "RIL_REQUEST_QUERY_UPB_CAPABILITY";
        case RIL_REQUEST_EDIT_UPB_ENTRY: return "RIL_REQUEST_EDIT_UPB_ENTRY";
        case RIL_REQUEST_DELETE_UPB_ENTRY: return "RIL_REQUEST_DELETE_UPB_ENTRY";
        case RIL_REQUEST_READ_UPB_GAS_LIST: return "RIL_REQUEST_READ_UPB_GAS_LIST";
        case RIL_REQUEST_READ_UPB_GRP: return "RIL_REQUEST_READ_UPB_GRP";
        case RIL_REQUEST_WRITE_UPB_GRP: return "RIL_REQUEST_WRITE_UPB_GRP";
        case RIL_REQUEST_GET_PHB_STRING_LENGTH: return "RIL_REQUEST_GET_PHB_STRING_LENGTH";
        case RIL_REQUEST_GET_PHB_MEM_STORAGE: return "RIL_REQUEST_GET_PHB_MEM_STORAGE";
        case RIL_REQUEST_SET_PHB_MEM_STORAGE: return "RIL_REQUEST_SET_PHB_MEM_STORAGE";
        case RIL_REQUEST_READ_PHB_ENTRY_EXT: return "RIL_REQUEST_READ_PHB_ENTRY_EXT";
        case RIL_REQUEST_WRITE_PHB_ENTRY_EXT: return "RIL_REQUEST_WRITE_PHB_ENTRY_EXT";
        case RIL_REQUEST_QUERY_UPB_AVAILABLE: return "RIL_REQUEST_QUERY_UPB_AVAILABLE";
        case RIL_REQUEST_READ_EMAIL_ENTRY: return "RIL_REQUEST_READ_EMAIL_ENTRY";
        case RIL_REQUEST_READ_SNE_ENTRY: return "RIL_REQUEST_READ_SNE_ENTRY";
        case RIL_REQUEST_READ_ANR_ENTRY: return "RIL_REQUEST_READ_ANR_ENTRY";
        case RIL_REQUEST_READ_UPB_AAS_LIST: return "RIL_REQUEST_READ_UPB_AAS_LIST";
        case RIL_UNSOL_PHB_READY_NOTIFICATION: return "RIL_UNSOL_PHB_READY_NOTIFICATION";
        case RIL_REQUEST_SET_PHONEBOOK_READY: return "RIL_REQUEST_SET_PHONEBOOK_READY";
        // PHB END
        /// M:[Network] add for Wifi calling Service. @{
        case RIL_LOCAL_GSM_UNSOL_MAL_PS_RGEGISTRATION_STATE: return "RIL_LOCAL_GSM_UNSOL_MAL_PS_RGEGISTRATION_STATE";
        /// @}
        /// M: EPDG feature. Update PS state from MAL @{
        case RIL_LOCAL_REQUEST_MAL_PS_RGEGISTRATION_STATE: return "RIL_LOCAL_REQUEST_MAL_PS_RGEGISTRATION_STATE";
        /// @}
        // / M: OTASP {
        case RIL_UNSOL_TRIGGER_OTASP: return "RIL_UNSOL_TRIGGER_OTASP";
        // / M: OTASP }
        // M: [LTE][Low Power][UL traffic shaping] @{
        case RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT: return "RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT";
        case RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER: return "RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER";
        case RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE: return "RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE";
        // M: [LTE][Low Power][UL traffic shaping] @}
        case RIL_UNSOL_NETWORK_INFO: return "UNSOL_NETWORK_INFO";
        /// [IMS][framework] {@
        case RIL_REQUEST_SET_IMS_ENABLE: return "RIL_REQUEST_SET_IMS_ENABLE";
        case RIL_REQUEST_SET_VOLTE_ENABLE: return "RIL_REQUEST_SET_VOLTE_ENABLE";
        case RIL_REQUEST_SET_WFC_ENABLE: return "RIL_REQUEST_SET_WFC_ENABLE";
        case RIL_REQUEST_SET_IMS_VIDEO_ENABLE: return "RIL_REQUEST_SET_IMS_VIDEO_ENABLE";
        case RIL_REQUEST_IMS_DEREG_NOTIFICATION: return "RIL_REQUEST_IMS_DEREG_NOTIFICATION";
        case RIL_REQUEST_GET_PROVISION_VALUE: return "RIL_REQUEST_GET_PROVISION_VALUE";
        case RIL_REQUEST_SET_PROVISION_VALUE: return "RIL_REQUEST_SET_PROVISION_VALUE";
        case RIL_REQUEST_SET_IMSCFG: return "RIL_REQUEST_SET_IMSCFG";
        case RIL_REQUEST_SET_MD_IMSCFG: return "RIL_REQUEST_SET_MD_IMSCFG";
        case RIL_REQUEST_SET_WFC_PROFILE: return "RIL_REQUEST_SET_WFC_PROFILE";
        case RIL_REQUEST_SET_IMS_REGISTRATION_REPORT: return "RIL_REQUEST_SET_IMS_REGISTRATION_REPORT";
        case RIL_REQUEST_SET_IMS_RTP_REPORT: return "RIL_REQUEST_SET_IMS_RTP_REPORT";
        case RIL_REQUEST_QUERY_VOPS_STATUS: return "RIL_REQUEST_QUERY_VOPS_STATUS";
        case RIL_UNSOL_VOPS_INDICATION: return "RIL_UNSOL_VOPS_INDICATION";
        case RIL_UNSOL_IMS_REGISTRATION_INFO: return "RIL_UNSOL_IMS_REGISTRATION_INFO";
        case RIL_UNSOL_IMS_ENABLE_DONE: return "RIL_UNSOL_IMS_ENABLE_DONE";
        case RIL_UNSOL_IMS_DISABLE_DONE: return "RIL_UNSOL_IMS_DISABLE_DONE";
        case RIL_UNSOL_IMS_ENABLE_START: return "RIL_UNSOL_IMS_ENABLE_START";
        case RIL_UNSOL_IMS_DISABLE_START: return "RIL_UNSOL_IMS_DISABLE_START";
        case RIL_UNSOL_IMS_RTP_INFO: return "RIL_UNSOL_IMS_RTP_INFO";
        case RIL_UNSOL_IMS_DEREG_DONE: return "RIL_UNSOL_IMS_DEREG_DONE";
        case RIL_UNSOL_GET_PROVISION_DONE: return "RIL_UNSOL_GET_PROVISION_DONE";
        case RIL_UNSOL_ECONF_SRVCC_INDICATION: return "RIL_UNSOL_ECONF_SRVCC_INDICATION";
        case RIL_UNSOL_VOLTE_SETTING: return "RIL_UNSOL_VOLTE_SETTING";
        /// [IMS][framework] @}
        case RIL_LOCAL_REQUEST_HANDOVER_IND: return "RIL_LOCAL_REQUEST_HANDOVER_IND";
        case RIL_LOCAL_REQUEST_WIFI_DISCONNECT_IND: return "RIL_LOCAL_REQUEST_WIFI_DISCONNECT_IND";
        case RIL_LOCAL_REQUEST_PCSCF_DISCOVERY_PCO: return "RIL_REQUEST_PCSCF_DISCOVERY_PCO";
        case RIL_LOCAL_REQUEST_WIFI_CONNECT_IND: return "RIL_REQUEST_WIFI_CONNECT_IND";
        case RIL_LOCAL_GSM_UNSOL_DEDICATE_BEARER_ACTIVATED: return "RIL_LOCAL_GSM_UNSOL_DEDICATE_BEARER_ACTIVATED";
        case RIL_LOCAL_GSM_UNSOL_DEDICATE_BEARER_MODIFIED: return "RIL_LOCAL_GSM_UNSOL_DEDICATE_BEARER_MODIFIED";
        case RIL_LOCAL_GSM_UNSOL_DEDICATE_BEARER_DEACTIVATED: return "RIL_LOCAL_GSM_UNSOL_DEDICATE_BEARER_DEACTIVATED";
        case RIL_LOCAL_GSM_UNSOL_MAL_DATA_CALL_LIST_CHANGED: return "RIL_LOCAL_GSM_UNSOL_MAL_DATA_CALL_LIST_CHANGED";
        /// @}
        // MTK_TC1_FEATURE for Antenna Testing start
        case RIL_REQUEST_VSS_ANTENNA_CONF: return "RIL_REQUEST_VSS_ANTENNA_CONF";
        case RIL_REQUEST_VSS_ANTENNA_INFO: return "RIL_REQUEST_VSS_ANTENNA_INFO";
        // MTK_TC1_FEATURE for Antenna Testing end
        case RIL_REQUEST_GET_POL_CAPABILITY: return "RIL_REQUEST_GET_POL_CAPABILITY";
        case RIL_REQUEST_GET_POL_LIST: return "RIL_REQUEST_GET_POL_LIST";
        case RIL_REQUEST_SET_POL_ENTRY: return "RIL_REQUEST_SET_POL_ENTRY";
        // /M: STK {
        case RIL_UNSOL_STK_SETUP_MENU_RESET:return "RIL_UNSOL_STK_SETUP_MENU_RESET";
        // /M: STK }
        // M: [VzW] Data Framework
        case RIL_UNSOL_PCO_DATA_AFTER_ATTACHED: return "RIL_UNSOL_PCO_DATA_AFTER_ATTACHED";
        // M: Data Framework - common part enhancement
        case RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD: return "RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD";
        // M: Data Framework - Data Retry enhancement
        case RIL_REQUEST_RESET_MD_DATA_RETRY_COUNT: return "RIL_REQUEST_RESET_MD_DATA_RETRY_COUNT";
        case RIL_UNSOL_MD_DATA_RETRY_COUNT_RESET: return "RIL_UNSOL_MD_DATA_RETRY_COUNT_RESET";
        // M: Data Framework - CC 33
        case RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE: return "RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE";
        case RIL_UNSOL_REMOVE_RESTRICT_EUTRAN: return "RIL_UNSOL_REMOVE_RESTRICT_EUTRAN";
        /// M: CC: Switch HPF
        case RIL_LOCAL_C2K_REQUEST_SWITCH_HPF: return "RIL_LOCAL_C2K_REQUEST_SWITCH_HPF";
        /// M: CC: Switch antenna
        case RIL_LOCAL_REQUEST_SWITCH_ANTENNA: return "RIL_LOCAL_REQUEST_SWITCH_ANTENNA";
        /// M: CC: Normal/Emergency call redial @{
        case RIL_LOCAL_C2K_REQUEST_REDIAL: return "RIL_LOCAL_C2K_REQUEST_REDIAL";
        case RIL_LOCAL_C2K_REQUEST_EMERGENCY_REDIAL: return "RIL_LOCAL_C2K_REQUEST_EMERGENCY_REDIAL";
        /// @}
        ///M: [Network] Femtocell URC log print. @{
        case RIL_UNSOL_FEMTOCELL_INFO: return "UNSOL_FEMTOCELL_INFO";
        /// @}
        /// M: [Network][C2K] Sprint roaming control @{
        case RIL_REQUEST_SET_ROAMING_ENABLE: return "SET_ROAMING_ENABLE";
        case RIL_REQUEST_GET_ROAMING_ENABLE: return "GET_ROAMING_ENABLE";
        /// @}
        //M: AGPSD command
        case RIL_LOCAL_C2K_REQUEST_AGPS_TCP_CONNIND : return "RIL_REQUEST_AGPS_TCP_CONNIND";
        case RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT : return "RIL_UNSOL_VIA_GPS_EVENT";
        // External SIM [START]
        case RIL_REQUEST_VSIM_NOTIFICATION: return "RIL_REQUEST_VSIM_NOTIFICATION";
        case RIL_REQUEST_VSIM_OPERATION: return "RIL_REQUEST_VSIM_OPERATION";
        case RIL_UNSOL_VSIM_OPERATION_INDICATION: return "RIL_UNSOL_VSIM_OPERATION_INDICATION";
        // External SIM [END]
        case RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL: return "RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL";
        case RIL_LOCAL_REQUEST_SET_MODEM_THERMAL: return "RIL_LOCAL_REQUEST_SET_MODEM_THERMAL";
        case RIL_REQUEST_CONFIG_MODEM_STATUS: return "RIL_REQUEST_CONFIG_MODEM_STATUS";
        // M: PDN deactivation failure info to RILP
        case RIL_LOCAL_GSM_UNSOL_PDN_DEACT_FAILURE_IND: return "RIL_LOCAL_GSM_UNSOL_PDN_DEACT_FAILURE_IND";
        case RIL_LOCAL_REQUEST_MODEM_POWEROFF_SYNC_C2K: return "RIL_LOCAL_REQUEST_MODEM_POWEROFF_SYNC_C2K";
        case RIL_LOCAL_GSM_UNSOL_ENHANCED_MODEM_POWER: return "RIL_LOCAL_GSM_UNSOL_ENHANCED_MODEM_POWER";
        case RIL_REQUEST_RESTART_RILD: return "RIL_REQUEST_RESTART_RILD";
        case RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL: return "RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL";
        case RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO: return "RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO";
        /// M: IMS SS @{
        case RIL_REQUEST_SEND_USSI: return "RIL_REQUEST_SEND_USSI";
        case RIL_REQUEST_CANCEL_USSI: return "RIL_REQUEST_CANCEL_USSI";
        case RIL_UNSOL_ON_USSI: return "RIL_UNSOL_ON_USSI";
        /// @}
        case RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE: return "RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE";
        /// M: Ims Data Framework @{
        case RIL_UNSOL_DEDICATE_BEARER_ACTIVATED: return "RIL_UNSOL_DEDICATE_BEARER_ACTIVATED";
        case RIL_UNSOL_DEDICATE_BEARER_MODIFIED: return "RIL_UNSOL_DEDICATE_BEARER_MODIFIED";
        case RIL_UNSOL_DEDICATE_BEARER_DEACTIVATED: return "RIL_UNSOL_DEDICATE_BEARER_DEACTIVATED";
        /// @}
        case RIL_LOCAL_REQUEST_RESUME_WORLD_MODE: return "RIL_LOCAL_REQUEST_RESUME_WORLD_MODE";
        case RIL_UNSOL_IMS_MULTIIMS_COUNT: return "RIL_UNSOL_IMS_MULTIIMS_COUNT";
        case RIL_REQUEST_SET_E911_STATE: return "RIL_REQUEST_SET_E911_STATE";
        case RIL_UNSOL_IMS_SUPPORT_ECC: return "RIL_UNSOL_IMS_SUPPORT_ECC";
        case RIL_LOCAL_REQUEST_SIM_GET_EFDIR: return "RIL_LOCAL_REQUEST_SIM_GET_EFDIR";
        case RIL_LOCAL_GSM_UNSOL_ESIMIND_APPLIST: return "RIL_LOCAL_GSM_UNSOL_ESIMIND_APPLIST";
        case RIL_UNSOL_PHYSICAL_CHANNEL_CONFIGS_MTK: return "RIL_UNSOL_PHYSICAL_CHANNEL_CONFIGS_MTK";
        case RIL_REQUEST_DATA_CONNECTION_ATTACH: return "RIL_REQUEST_DATA_CONNECTION_ATTACH";
        case RIL_REQUEST_DATA_CONNECTION_DETACH: return "RIL_REQUEST_DATA_CONNECTION_DETACH";
        case RIL_REQUEST_RESET_ALL_CONNECTIONS: return "RIL_REQUEST_RESET_ALL_CONNECTIONS";
        case RIL_REQUEST_SET_LTE_RELEASE_VERSION: return "RIL_REQUEST_SET_LTE_RELEASE_VERSION";
        case RIL_REQUEST_GET_LTE_RELEASE_VERSION: return "RIL_REQUEST_GET_LTE_RELEASE_VERSION";
        case RIL_UNSOL_MCCMNC_CHANGED: return "RIL_UNSOL_MCCMNC_CHANGED";
        /// M: [IR][C2K] Reset Suspend Mode @{
        case RIL_LOCAL_REQUEST_RESET_SUSPEND: return "RIL_LOCAL_REQUEST_RESET_SUSPEND";
        /// @}
        case RIL_REQUEST_SETPROP_IMS_HANDOVER: return "RIL_REQUEST_SETPROP_IMS_HANDOVER";
        case RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA: return "RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA";
        case RIL_REQUEST_SET_PREFERRED_DATA_MODEM: return "RIL_REQUEST_SET_PREFERRED_DATA_MODEM";
        case RIL_UNSOL_IMS_BEARER_STATE_NOTIFY: return "RIL_UNSOL_IMS_BEARER_STATE_NOTIFY";
        case RIL_UNSOL_IMS_DATA_INFO_NOTIFY: return "RIL_UNSOL_IMS_DATA_INFO_NOTIFY";
        case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX: return "CDMA_SMS_ACKNOWLEDGE_EX";
        case RIL_UNSOL_RESPONSE_CDMA_NEW_SMS_EX: return "UNSOL_CDMA_NEW_SMS_EX";
        /// IMS CC @{
        case RIL_REQUEST_IMS_GET_CURRENT_CALLS: return "RIL_REQUEST_IMS_GET_CURRENT_CALLS";
        case RIL_REQUEST_IMS_HANGUP_WAITING_OR_BACKGROUND: return "RIL_REQUEST_IMS_HANGUP_WAITING_OR_BACKGROUND";
        case RIL_REQUEST_IMS_HANGUP_FOREGROUND_RESUME_BACKGROUND: return "RIL_REQUEST_IMS_HANGUP_FOREGROUND_RESUME_BACKGROUND";
        case RIL_REQUEST_IMS_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE: return "RIL_REQUEST_IMS_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE";
        case RIL_UNSOL_IMS_RESPONSE_CALL_STATE_CHANGED: return "RIL_UNSOL_IMS_RESPONSE_CALL_STATE_CHANGED";
        case RIL_REQUEST_CONFERENCE_DIAL: return "RIL_REQUEST_CONFERENCE_DIAL";
        case RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER: return "RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER";
        case RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER: return "RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER";
        case RIL_UNSOL_ECC_NUM: return "RIL_UNSOL_ECC_NUM";
        case RIL_UNSOL_IMS_BEARER_INIT: return "RIL_UNSOL_IMS_BEARER_INIT";
        /// @}
        case RIL_REQUEST_ACTIVATE_UICC_CARD: return "RIL_REQUEST_ACTIVATE_UICC_CARD";
        case RIL_REQUEST_DEACTIVATE_UICC_CARD: return "RIL_REQUEST_DEACTIVATE_UICC_CARD";
        case RIL_REQUEST_GET_CURRENT_UICC_CARD_PROVISIONING_STATUS: return "GET_CURRENT_UICC_CARD_PROVISIONING_STATUS";
        case RIL_REQUEST_START_KEEPALIVE: return "RIL_REQUEST_START_KEEPALIVE";
        case RIL_REQUEST_STOP_KEEPALIVE: return "RIL_REQUEST_STOP_KEEPALIVE";
        case RIL_UNSOL_KEEPALIVE_STATUS: return "RIL_UNSOL_KEEPALIVE_STATUS";
        case RIL_LOCAL_C2K_REQUEST_SET_UTK_MODE: return "RIL_LOCAL_C2K_REQUEST_SET_UTK_MODE";
        default: return "<unknown request>";
    }
}

///M: [Network][C2K] add for engineer rat switch. @{
int isSvlteCdmaOnlySetFromEngMode()
{
    int isSetFromEngMode = 0;
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("persist.vendor.radio.ct.ir.engmode", property_value, "0");
    isSetFromEngMode = atoi(property_value);
    return isSetFromEngMode;
}
/// @}

// M: [Network][C2K ]India 3M/5M. @{
int getSvlteProjectType() {
    int type = -1;

    if (RatConfig_isLteFddSupported() && RatConfig_isLteTddSupported()
            && RatConfig_isC2kSupported() && RatConfig_isWcdmaSupported()
            && RatConfig_isTdscdmaSupported()
            && RatConfig_isGsmSupported()) {
        type = SVLTE_PROJ_DC_6M;
    } else if (RatConfig_isLteFddSupported()
            && RatConfig_isLteTddSupported() && RatConfig_isC2kSupported()
            && RatConfig_isWcdmaSupported() && RatConfig_isGsmSupported()) {
        type = SVLTE_PROJ_DC_5M;
    } else if (RatConfig_isLteFddSupported()
            && RatConfig_isLteTddSupported() && RatConfig_isC2kSupported()
            && RatConfig_isGsmSupported()) {
        type = SVLTE_PROJ_DC_4M;
    } else if (RatConfig_isC2kSupported() && RatConfig_isGsmSupported()
            && RatConfig_isWcdmaSupported()) {
        type = SVLTE_PROJ_DC_3M;
    }

    mtkLogI(LOG_TAG, "getSvlteWwopType(): %d", type);
    return type;
}

int isSvlteLcgSupport()
{
    int isSvlteLcgSupport = 0;
    if (RatConfig_isLteFddSupported() == 1 && RatConfig_isLteTddSupported() == 1
            && RatConfig_isC2kSupported() == 1 && RatConfig_isGsmSupported() ==1
            && RatConfig_isWcdmaSupported() == 0 && RatConfig_isTdscdmaSupported() == 0) {
        isSvlteLcgSupport = 1;
    }
    return isSvlteLcgSupport;
}
///end @}

/// [IMS][framework] {@
int isImsSupport() {
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("persist.vendor.ims_support", property_value, "0");
    return atoi(property_value);
}

int isImsAndVolteSupport() {
    FeatureValue feature;
    char ims_prop[MTK_PROPERTY_VALUE_MAX] = {0};

    memset(feature.value, 0, sizeof(feature.value));
    mtkGetFeature(CONFIG_VOLTE, &feature);
    mtk_property_get("persist.vendor.ims_support", ims_prop, "");

    if (!strcmp(feature.value, "1") && !strcmp(ims_prop, "1")) {
        return 1;
    }

    return 0;
}

/// M: ViLTE @{
/**
 * Checks if ViLTE is supported.
 * @return true if ViLTE is supported
 */
bool isVilteSupport() {
    FeatureValue feature;
    memset(feature.value, 0, sizeof(feature.value));
    mtkGetFeature(CONFIG_VILTE, &feature);
    return strcmp(feature.value, "1") == 0;
}
/// @}

/// M: Wfc @{
/**
 * Checks if Wfc is supported.
 * @return true if wfc is supported
 */
bool isWfcSupport() {
    FeatureValue feature;
    memset(feature.value, 0, sizeof(feature.value));
    mtkGetFeature(CONFIG_WFC, &feature);
    return strcmp(feature.value, "1") == 0;
}
/// @}

int isEpdgSupport() {
    return isWfcSupport();
}

int getMultiImsSupportValue() {
    char multiImsSupport[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("persist.vendor.mims_support", multiImsSupport, "1");
    return atoi(multiImsSupport);
}

/// M: RCS Proxy @{
/**
 * Checks if RCS UA Proxy is supported.
 * @return true if UA Proxy is enabled
 */
bool isRcsUaProxySupport() {
    int isRcsUaProxySupport = 0;

    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("persist.vendor.mtk_rcs_ua_support", property_value, "0");

    isRcsUaProxySupport = atoi(property_value);
    mtkLogI(LOG_TAG, "isRcsUaProxySupport: %d", isRcsUaProxySupport);
    return (isRcsUaProxySupport == 1) ? true : false;
}
/// @}

/// M: RCS Service @{
/**
 * Checks if RCS Service is supported.
 * @return true if RCS Service is enabled
 */
bool isRcsServiceEnabled() {
    int isRcsServiceEnabled = 1;

    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("persist.vendor.service.rcs", property_value, "1");

    isRcsServiceEnabled = atoi(property_value);
    mtkLogI(LOG_TAG, "isRcsServiceEnabled: %d", isRcsServiceEnabled);
    return (isRcsServiceEnabled == 1) ? true : false;
}
/// @}

/// M: RCS Service Tag  @{
/**
 * Checks which RCS service tag was supported.
 * @return RCS service tag bit-map:
 *         0x01  session
 *         0x02  filetransfer
 *         0x04  msg
 *         0x08  largemsg
 *         0x10  geopush
 *         0x20  geopull
 *         0x40  geopullft
 *         0x80  imdn aggregation
 *        0x100  geosms
 *        0x200  fthttp
 *         0x00  RCS service tag was removed
 */
int getRcsSrvTag() {
    int rcsSrvTags = 0, rcsCustomized = 0;

    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("persist.vendor.service.tag.rcs", property_value, "0");

    rcsSrvTags = atoi(property_value);
    memset(property_value, 0, sizeof(property_value));
    mtk_property_get("persist.vendor.customized.rcs", property_value, "0");
    rcsCustomized = atoi(property_value);
    mtkLogI(LOG_TAG, "rcsCustomized: %d, rcsSrvTags: %d", rcsCustomized, rcsSrvTags);
    if (rcsSrvTags <= 0x00 || rcsSrvTags > 0xFFFF) {
        if (rcsCustomized == 0) {
            // MTK internal: session + filetransfer + msg + largemsg
            return 0x0F;
        } else {
            // default tags: session + filetransfer + geopush
            return 0x13;
        }
    }
    return rcsSrvTags;
}
/// @}

/// [IMS][framework]@}

int isOp12Support() {
    int ret = 0;
    char optr_value[MTK_PROPERTY_VALUE_MAX] = {0};
    mtk_property_get("persist.vendor.operator.optr", optr_value, "0");
    if (strcmp(optr_value, OPERATOR_OP12) == 0) {
        ret = 1;
    }
    return ret;
}

int isOp01Support() {
    int ret = 0;
    char optr_value[MTK_PROPERTY_VALUE_MAX] = {0};
    mtk_property_get("persist.vendor.operator.optr", optr_value, "0");
    if (strcmp(optr_value, OPERATOR_OP01) == 0) {
        ret = 1;
    }
    return ret;
}

int isOp07Support() {
    int ret = 0;
    char optr_value[MTK_PROPERTY_VALUE_MAX] = {0};
    mtk_property_get("persist.vendor.operator.optr", optr_value, "0");
    if (strcmp(optr_value, OPERATOR_OP07) == 0) {
        ret = 1;
    }
    return ret;
}

int isOp08Support() {
    int ret = 0;
    char optr_value[MTK_PROPERTY_VALUE_MAX] = {0};
    mtk_property_get("persist.vendor.operator.optr", optr_value, "0");
    if (strcmp(optr_value, OPERATOR_OP08) == 0) {
        ret = 1;
    }
    return ret;
}

int isOp12MccMnc(char *mccmnc) {
    unsigned int i;
    for (i = 0; i < sizeof(VZW_MCC_MNC) / sizeof(VZW_MCC_MNC[0]); i++) {
        if (0 == strcmp(mccmnc, VZW_MCC_MNC[i])) {
            mtkLogD(LOG_TAG, "isOp12MccMnc: true.");
            return 1;
        }
    }
    mtkLogD(LOG_TAG, "isOp12MccMnc: false");
    return 0;
};

int isOp16Support() {
    int isSupport = 0;
    char dsbp_enable[MTK_PROPERTY_VALUE_MAX] = {0};
    char optr[MTK_PROPERTY_VALUE_MAX] = {0};

    mtk_property_get("persist.vendor.operator.optr", optr, "");

    // Check D-SBP enabled or not
    mtk_property_get("persist.vendor.radio.mtk_dsbp_support", dsbp_enable, "");
    // Support D-SBP
    if (!strcmp(dsbp_enable, "1")) {
        // get dynamic SBP ID by SIM
        char dsbp[MTK_PROPERTY_VALUE_MAX] = {0};
        mtk_property_get("persist.vendor.radio.sim.sbp", dsbp, "0");
        isSupport = (strtol(dsbp, NULL, 0) == 16) ? 1 : 0;
    } else if (strcmp(optr, "OP16") == 0) { // Check OPTR
        isSupport = 1;
    }
    return isSupport;
}


int isOp17MccMnc(char *mccmnc) {
    return (strcmp(mccmnc, "44010") == 0);
}

int isOp129MccMnc(char *mccmnc) {
    return (strcmp(mccmnc, "44051") == 0 ||
        strcmp(mccmnc, "44054") == 0);
}

int isOp01Volte() {
    char sbp_value[MTK_PROPERTY_VALUE_MAX] = {0};
    int isOp01Volte = 0;

    mtk_property_get("ro.vendor.mtk_md_sbp_custom_value", sbp_value, "");
    if (strlen(sbp_value) > 0) {
        int tmpValue = (int)strtol(sbp_value, NULL, 0);
        char optr[MTK_PROPERTY_VALUE_MAX] = {0};

        mtk_property_get("persist.vendor.operator.optr", optr, "");
        if (tmpValue == 1) {    // CMCC customize load
            isOp01Volte = 1;
        } else if ((tmpValue == 0) && !strcmp("", optr)) { // OM load
            char dsbp_prop[MTK_PROPERTY_VALUE_MAX] = {0};

            mtk_property_get("persist.vendor.radio.mtk_dsbp_support", dsbp_prop, "");
            if (!strcmp(dsbp_prop, "1")) { // support D-SBP
                // get dynamic SBP ID by SIM
                mtk_property_get("persist.vendor.radio.sim.sbp", sbp_value, "0");
                isOp01Volte = (strtol(sbp_value, NULL, 0) == 1) ? 1 : 0;
            }
        }
    }

    return isOp01Volte;
}

int isApFallbackSupport() {
    char value[MTK_PROPERTY_VALUE_MAX] = {0};
    int ret = 0;
    int default_value = 100; // Default value.
    mtk_property_get("persist.vendor.ap.fallback.support", value, "100");
    ret = atoi(value);
    if (default_value == ret) {
        if (isOp07Support()) {
            ret = 0;
        } else { // Default is on.
            ret = 1;
        }
    }
    return ret;
}

int isMdFallbackSupport() {
    char value[MTK_PROPERTY_VALUE_MAX] = {0};
    int ret = 0;
    int default_value = 100; // Default value.
    mtk_property_get("persist.vendor.md.fallback.support", value, "100");
    ret = atoi(value);
    if (default_value == ret) {
        if (isOp07Support()) {
            ret = 1;
        } else { // Default is off.
            ret = 0;
        }
    }
    return ret;
}

// M: [VzW] Data Framework @{
bool isFallbackNotSupportByOp() {
    int i = 0;
    int length = sizeof(VZW_MCC_MNC) / sizeof(VZW_MCC_MNC[0]);
    char operatorNumeric[MTK_PROPERTY_VALUE_MAX] = {0};
    bool ret = false;

    mtk_property_get("gsm.operator.numeric", operatorNumeric, "");

    for (i = 0; i < length; i++) {
        if (0 == strcmp(operatorNumeric, VZW_MCC_MNC[i])) {
            mtkLogD(LOG_TAG, "isFallbackNotSupportByOp: true");
            ret = true;
            return ret;
        }
    }
    return ret;
}
// M: [VzW] Data Framework @}

int isInternationalRoamingEnabled() {
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("vendor.ril.evdo.irsupport", property_value, "0");
    return atoi(property_value);
}

static const char PROPERTY_SIM_SWITCH_FULL_UICC_TYPE[2][35] = {
    "vendor.gsm.ril.fulluicctype",
    "vendor.gsm.ril.fulluicctype.2",
};

static const char PROPERTY_SIM_SWITCH_CT3G[2][25] = {
    "gsm.ril.ct3g",
    "gsm.ril.ct3g.2",
};

static bool isCTCardType(int slot_id) {
    char tmp[MTK_PROPERTY_VALUE_MAX] = { 0 };
    char tmp2[MTK_PROPERTY_VALUE_MAX] = { 0 };
    char *value1, *value2, *value3;
    mtk_property_get(PROPERTY_SIM_SWITCH_FULL_UICC_TYPE[slot_id], tmp, "");
    value1 = strstr(tmp, "UIM");
    value2 = strstr(tmp, "CSIM");
    value3 = strstr(tmp, "SIM");

    if (value1 == NULL && value2 == NULL) {
        if (value3 != NULL) {
            mtk_property_get(PROPERTY_SIM_SWITCH_CT3G[slot_id], tmp2, "");

            if ((strcmp("1", tmp2) == 0)) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        return true;
    }
}

bool isDualCTCard() {
    int i;
    bool ret = false;

    for (i=0; i < 2; i++) {
        ret = isCTCardType(i);
        if (!ret) {
            break;
        }
    }

    return ret;
}

bool hasCdmaCard() {
    for (int i = 0; i < getSimCount(); i++) {
        if (isCTCardType(i)) {
            return true;
        }
    }
    return false;
}

bool hasVsimCard() {
    for (int rid = RIL_SOCKET_1; rid < RIL_SOCKET_NUM; rid++) {
        char persist_vsim_prop[MTK_PROPERTY_VALUE_MAX] = {0};
        char vsim_enabled_prop[MTK_PROPERTY_VALUE_MAX] = {0};
        char vsim_inserted_prop[MTK_PROPERTY_VALUE_MAX] = {0};
        getMSimProperty(rid, "persist.vendor.radio.external.sim", persist_vsim_prop);
        getMSimProperty(rid, "vendor.gsm.external.sim.enabled", vsim_enabled_prop);
        getMSimProperty(rid, "vendor.gsm.external.sim.inserted", vsim_inserted_prop);
        if (isExternalSimOnlySlot(rid) || atoi(persist_vsim_prop) > 0 ||
            (atoi(vsim_enabled_prop) > 0 && atoi(vsim_inserted_prop) > 0)) {
            return true;
        }
    }
    return false;
}

void setRildInterfaceCtrlSupport() {
    if (isEpdgSupport()) {
        mtkLogI(LOG_TAG, "set vendor.ril.epdg.interface.ctrl 1");
        mtk_property_set("vendor.ril.epdg.interface.ctrl", "1");
    }
}

int getRildInterfaceCtrlSupport() {
    char value[MTK_PROPERTY_VALUE_MAX] = {0};
    mtk_property_get("vendor.ril.epdg.interface.ctrl", value, "0");
    return (strtol(value, NULL, 0) == 1) ? 1 : 0;
}
/// @}

int isMultipleImsSupport() {
    static int ims_num = -1;
    if (ims_num == -1) {
        char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
        mtk_property_get("persist.vendor.mims_support", property_value, "0");
        ims_num = atoi(property_value);
    }
    return (ims_num > 1) ? 1 : 0;
}

int isMalSupported() {
    static int is_mal_supported = -1;
    if (is_mal_supported == -1) {
        char ims_prop[MTK_PROPERTY_VALUE_MAX] = {0};
        //MAL is depended on ims or epdg
        mtk_property_get("persist.vendor.ims_support", ims_prop, "0");
        bool wfcSupport = isWfcSupport();
        if (!strcmp(ims_prop, "1") || wfcSupport) {
            is_mal_supported = 1;
        } else {
            is_mal_supported = 0;
        }
    }
    return is_mal_supported;
}

// M: VzW
int getClassType(char *apn, int supportedApnTypesBitmask) {
    int classType = APN_CLASS_3;

    if ((supportedApnTypesBitmask & RIL_APN_TYPE_EMERGENCY) || strcasecmp(VZW_EMERGENCY_NI, apn) == 0) {
        classType = APN_CLASS_0;
    } else if ((supportedApnTypesBitmask & RIL_APN_TYPE_IMS) || strcasecmp(VZW_IMS_NI, apn) == 0) {
        classType = APN_CLASS_1;
    } else if (strcasecmp(VZW_ADMIN_NI, apn) == 0) {
        classType = APN_CLASS_2;
    } else if (strcasecmp(VZW_APP_NI, apn) == 0) {
        classType = APN_CLASS_4;
    } else if (strcasecmp(VZW_800_NI, apn) == 0) {
        classType = APN_CLASS_5;
    } else if (supportedApnTypesBitmask & RIL_APN_TYPE_DEFAULT) {
        classType = APN_CLASS_3;
    } else {
        mtkLogD(LOG_TAG, "getClassType: set to default class 3");
    }

    mtkLogD(LOG_TAG, "[%s] classType = %d", __FUNCTION__, classType);
    return classType;
}

// External SIM [Start]
int isExternalSimSupport() {
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("ro.vendor.mtk_external_sim_support", property_value, "0");
    mtkLogI(LOG_TAG, "[isExternalSimSupport] vsim support:%d", atoi(property_value));
    return atoi(property_value);
}

int isExternalSimOnlySlot(RIL_SOCKET_ID rid) {
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("ro.vendor.mtk_external_sim_only_slots", property_value, "0");
    int supported = atoi(property_value) & (1 << rid);

    mtkLogI(LOG_TAG, "[isExternalSimOnlySlot] vsimOnlySlots:%d, supported:%d",
            atoi(property_value), supported);
    return ((supported > 0) ? 1 : 0);
}

int isPersistExternalSimDisabled() {
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("ro.vendor.mtk_persist_vsim_disabled", property_value, "0");
    return atoi(property_value);
}

static pthread_mutex_t sExternalSimPropertyMutex = PTHREAD_MUTEX_INITIALIZER;
void setExternalSimProperty(int phoneId, char *pPropertyName, char *pUpdateValue) {
    pthread_mutex_lock(&sExternalSimPropertyMutex);
    setMSimProperty(phoneId, pPropertyName, pUpdateValue);
    pthread_mutex_unlock(&sExternalSimPropertyMutex);
}
// External SIM [End]

bool isDssNoResetSupport() {
    char tmp[MTK_PROPERTY_VALUE_MAX] = { 0 };

    mtk_property_get("ril.simswitch.no_reset_support", tmp, "0");
    if (strcmp(tmp, "1") == 0) {
        return true;
    }
    return false;
}

/**
 * returns 1 if line starts with prefix, 0 if it does not
 */
int strStartsWith(const char *line, const char *prefix)
{
    for ( ; *line != '\0' && *prefix != '\0' ; line++, prefix++) {
        if (*line != *prefix) {
            return 0;
        }
    }

    return *prefix == '\0';
}

/**
 * return 1 for hiden content of log, 0 is not
*/
int needToHidenLog(const char *target) {
    for (unsigned int i = 0; i < NUM_ELEMS(sAtCmdWithoutContent); i++) {
        if (strStartsWith(target, sAtCmdWithoutContent[i])) {
            return i;
        }
    }
    return -1;
}

const char* getHidenLogPreFix(int i) {
    if (i < 0) {
        return "index error";
    }
    return sAtCmdWithoutContent[i];
}

bool isRedialFeatureEnabled() {
    char optr[MTK_PROPERTY_VALUE_MAX] = {0};

    mtk_property_get("persist.vendor.operator.optr", optr, "");
    mtkLogD(LOG_TAG, "[isRedialFeatureEnabled] optr:%s", optr);

    // should not enable for OP12X or OP20X
    if (strlen(optr) > 4) {
        return false;
    }

    if (strncmp(optr, "OP12", 4) == 0) {
        return true;
    } else if (strncmp(optr, "OP20", 4) == 0) {
        return true;
    }

    return false;
}

/**
 * Return 1 for OM project
**/
int isOMSupport() {
    int ret = 0;
    char optr_value[MTK_PROPERTY_VALUE_MAX] = {0};
    mtk_property_get("persist.vendor.operator.optr", optr_value, "0");
    if (strcmp(optr_value, OPERATOR_OM_ID) == 0) {
        ret = 1;
    }
    return ret;
}

/**
 * Check if the AT command is in the log reduction list
*/
int isLogReductionCmd(const char *target) {
    size_t i;
    for (i = 0; i < NUM_ELEMS(s_atcmd_log_reduction); i++) {
        if (strStartsWith(target, s_atcmd_log_reduction[i])) {
            return 1;
        }
    }
    return 0;
}

/**
 * add lock mutex parameter for set Sim property for thread safe
*/
void setMSimPropertyThreadSafe(
        int phoneId, char *pPropertyName, char *pUpdateValue, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    setMSimProperty(phoneId, pPropertyName, pUpdateValue);
    pthread_mutex_unlock(mutex);
}

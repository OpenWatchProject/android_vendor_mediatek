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

#ifndef LIBMTKRILUTILS_H
#define LIBMTKRILUTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <telephony/mtk_ril.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HIDEN_LOG_LEN 20

#define OPERATOR_OM_ID "0"
// OPxx define @{
#define OPERATOR_OP01 "OP01"
#define OPERATOR_OP12 "OP12"
#define OPERATOR_OP07 "OP07"
#define OPERATOR_OP08 "OP08"
#define OPERATOR_OP16 "OP16"

// OPxx define @}

#define PROPERTY_MCC_MNC "vendor.gsm.ril.uicc.mccmnc"
#define PROPERTY_MCC_MNC_CDMA "vendor.cdma.ril.uicc.mccmnc"

#define READ_MCC_MNC_ERR_SUCCESS 0
#define READ_MCC_MNC_ERR_NULL_MEM 1
#define READ_MCC_MNC_ERR_NOT_READY 2

// OP12 related @{
#define OP12_PLMN "311480"
#define OP12_PCO_ID "FF00"
#define OP12_MCC 311
#define OP12_MNC 480
#define VZW_IMS_NI "VZWIMS"
#define VZW_EMERGENCY_NI "VZWEMERGENCY"
#define VZW_ADMIN_NI "VZWADMIN"
#define VZW_INTERNET_NI "VZWINTERNET"
#define VZW_APP_NI "VZWAPP"
#define VZW_800_NI "VZW800"
#define APN_CLASS_0 0
#define APN_CLASS_1 1
#define APN_CLASS_2 2
#define APN_CLASS_3 3
#define APN_CLASS_4 4
#define APN_CLASS_5 5
// OP12 related @}

// Vzw APN
#define VZW_IMS_NI "VZWIMS"
#define VZW_EMERGENCY_NI "VZWEMERGENCY"
#define VZW_ADMIN_NI "VZWADMIN"
#define VZW_INTERNET_NI "VZWINTERNET"
#define VZW_APP_NI "VZWAPP"
#define VZW_800_NI "VZW800"
#define APN_CLASS_0 0
#define APN_CLASS_1 1
#define APN_CLASS_2 2
#define APN_CLASS_3 3
#define APN_CLASS_4 4
#define APN_CLASS_5 5

// structure
typedef enum {
    GEMINI_MODE_UNKNOWN = -1,
    GEMINI_MODE_L_AND_L,
    GEMINI_MODE_L_AND_W,
    GEMINI_MODE_L_AND_G,
    GEMINI_MODE_W_AND_W,
    GEMINI_MODE_W_AND_G,
} GEMINI_MODE;

/// M: [Network][C2K] for india 3M/5M. @{
typedef enum {
    SVLTE_PROJ_INVALID = -1,
    SVLTE_PROJ_DC_3M = 3,
    SVLTE_PROJ_DC_4M = 4,
    SVLTE_PROJ_DC_5M = 5,
    SVLTE_PROJ_DC_6M = 6,
    SVLTE_PROJ_SC_3M = 103,
    SVLTE_PROJ_SC_4M = 104,
    SVLTE_PROJ_SC_5M = 105,
    SVLTE_PROJ_SC_6M = 106,
} SVLTE_PROJ_TYPE;
/// @}

int isLteSupport();
int getSimCount();
int getMdMaxSimCount();
int isCCCIPowerOffModem();
int isSupportCommonSlot();
int RIL_get3GSIM();
int isEmulatorRunning();
int isInternalLoad();
int isEngLoad();
int isUserLoad();
void setMSimProperty(int phoneId, char *pPropertyName, char *pUpdateValue);
void getMSimProperty(int phoneId, char *pPropertyName,char *pPropertyValue);
void maskString(char *src, char *dst, int maskNum, char maskPtn);
void givePrintableIccid(char *iccid, char *maskIccid);
// Notice: caller should free the return string by himself.
unsigned char* pii(char* input);
int isSrlteSupport();
int isSvlteSupport();
int isSupportC2kSim();
int isCdmaLteDcSupport();
int isDisableCapabilitySwitch();
int isDualTalkMode();
int getActiveSvlteModeSlotId();
/// M: C2K RIL START
int getCdmaModemSlot(void);
int getCdmaSocketSlotId();
int getActiveSvlteModeSlotId();

/// M: C2K RIL END
GEMINI_MODE getGeminiMode();
int isDisable2G();
int isEnableModulationReport();
int isFemtocellSupport();
int isMultiPsAttachSupport();
const char* mtkRequestToString(int request);
/// M: [Network][C2K] add for rat mode initiallize. @{
int isSvlteCdmaOnlySetFromEngMode();
int getSvlteProjectType();
int isSvlteLcgSupport();
/// @}

/// [IMS][framework] {@
int isImsSupport();
int isImsAndVolteSupport();
bool isVilteSupport();
bool isWfcSupport();
int isEpdgSupport();
int getMultiImsSupportValue();
bool isRcsUaProxySupport();
bool isRcsServiceEnabled();
int getRcsSrvTag();
/// [IMS][framework] @}
int isOp01Support();
int isOp07Support();
int isOp08Support();
int isOp12Support();
int isOp16Support();
int getClassType(char *apn, int supportedApnTypesBitmask);

/// M: [SIM Switch] {@
// return true if both slots are cdma cards
bool isDualCTCard();
// return true if there is any cdma card
bool hasCdmaCard();
// return true if vsim is inserted
bool hasVsimCard();
/// M: [SIM Switch] @}
int isOp17MccMnc(char *mccmnc);
int isOp129MccMnc(char *mccmnc);
int isOp01Volte();
int isApFallbackSupport();
int isMdFallbackSupport();
int isInternationalRoamingEnabled();
int readMccMnc(RIL_SOCKET_ID rid, char *mccmnc);
int isMultipleImsSupport();
int isMalSupported();

/// M: Ims Data Framework {@
void setRildInterfaceCtrlSupport();
int getRildInterfaceCtrlSupport();
/// @}

// External SIM [Start]
int isExternalSimSupport();
int isExternalSimOnlySlot(RIL_SOCKET_ID rid);
int isPersistExternalSimDisabled();
void setExternalSimProperty(int phoneId, char *pPropertyName, char *pUpdateValue);
// External SIM [End]

bool isDssNoResetSupport();

int strStartsWith(const char *line, const char *prefix);
int needToHidenLog(const char *prefix);
const char* getHidenLogPreFix(int i);

// hVoLTE/E911 feature
bool isRedialFeatureEnabled();
int isOMSupport();
int isLogReductionCmd(const char *target);
void setMSimPropertyThreadSafe(
        int phoneId, char *pPropertyName, char *pUpdateValue, pthread_mutex_t* mutex);

#ifdef __cplusplus
}
#endif

#endif // LIBMTKRILUTILS_H

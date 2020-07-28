/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef __RP_NW_DEFS_H__
#define __RP_NW_DEFS_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include <telephony/mtk_ril.h>
#include <libmtkrilutils.h>

/*****************************************************************************
 * Define
 *****************************************************************************/

/* Define SIM application family type. */
typedef enum {
    APP_FAM_UNKNOWN = -1, // for Unknown SIMs
    APP_FAM_3GPP    = 0, // for GSM SIMs
    APP_FAM_3GPP2   = 1, // for CDMA SIMs
} AppFamilyType;

/* Define the network selecting mode. */
typedef enum {
    NWS_MODE_UNKNOWN    = -1, // for CSFB (GSM) network selecting mode
    NWS_MODE_CDMALTE    = 0,  // for CDMALTE (CDMA) network selecting mode
    NWS_MODE_CSFB       = 1,  // for CSFB (GSM) network selecting mode
    NWS_MODE_LTEONLY    = 2   // for VoLTE on (LTE Only) network selecting mode
} NwsMode;

typedef enum {
    EVDO_MODE_NORMAL = 0, // EHRPD disabled
    EVDO_MODE_EHRPD = 1, // EHRPD enabled
    EVDO_MODE_INVALID = -1, //EHRPD no need config
} EvdoMode;

/* Define the combined preferred network type. */
typedef struct {
    int gsm_preferred_nw_type;
    int c2k_preferred_nw_type;
} CombinedPreferredNwType;

typedef enum {
    RAT_SWITCH_FAIL = -1,  // RAT switch failed
    RAT_SWITCH_SUCC = 0,  // RAT switch success
} RatSwitchResult;

/**************************
 * For NW module used only!
 **************************/
typedef enum {
    SVLTE_RAT_MODE_INVALID = -1,
    SVLTE_RAT_MODE_4G = 0,
    SVLTE_RAT_MODE_3G = 1,
    SVLTE_RAT_MODE_4G_DATA_ONLY = 2,
    SVLTE_RAT_MODE_LTE_C2K = 3,
    SVLTE_RAT_MODE_STK_SWITCH = 4,
    SVLTE_RAT_MODE_IR_SUSPEND = 5,
    SVLTE_RAT_MODE_LTE_ONLY = 6
} SVLTE_RAT_MODE;

/**************************
 * For NW module used only!
 **************************/
typedef enum EngineerMode {
    ENGINEER_MODE_AUTO  = 0,
    ENGINEER_MODE_CDMA  = 1,
    ENGINEER_MODE_CSFB = 2,
    ENGINEER_MODE_LTE = 3
} Engineer_Mode;

typedef enum {
    SVLTE_FDD_ONLY_MODE = 0,
    SVLTE_FDD_TDD_MODE = 1,
} LteBandMode;

typedef enum {
    PLMN_SEARCH_START = 0,
    PLMN_SEARCH_COMPLETE = 1,
}PlmnSearchStage;

/**************************
 * For NW module used only!
 **************************/
typedef enum {
    ACT_RAT_SWITCH_BEGIN = 0,
    ACT_CONFIG_RADIO_OFF,
    ACT_CONFIG_EVDO_MODE,
    ACT_CONFIG_SVLTE_MODE,
    ACT_CONFIG_RAT_MODE,
    ACT_CONFIG_PHONE_MODE,
    ACT_CONFIG_RADIO_ON,
    ACT_RAT_SWITCH_DONE,
    ACT_RAT_SWITCH_FAIL
} RAT_CTRL_ACTION;

typedef enum {
    PS_RAT_FAMILY_UNKNOWN = 0,
    PS_RAT_FAMILY_GSM = 1,
    PS_RAT_FAMILY_CDMA = 2,
    PS_RAT_FAMILY_IWLAN = 4,
} PsRatFamily;

typedef enum {
    VOLTE_OFF = 0,
    VOLTE_ON = 1,
} VolteState;

#endif /* __RP_NW_DEFS_H__ */

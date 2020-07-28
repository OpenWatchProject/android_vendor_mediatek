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

#ifndef __RP_IR_UTILS_H__
#define __RP_IR_UTILS_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <telephony/mtk_ril.h>
#include "cutils/properties.h"
#include "RfxController.h"
#include "RpNwDefs.h"

using namespace std;

typedef enum TagServiceType {
    OUT_OF_SERVICE,
    IN_SEARCHING,
    IN_SERVICE,
} ServiceType;

/* Define the network selecting mode. */
typedef enum {
    GMSS_RAT_3GPP2,    // Any RAT in 3GPP2 RAT group depend on other information.
    GMSS_RAT_3GPP,     // Any RAT in 3GPP RAT group depend on other information.
    GMSS_RAT_C2K1X,    // CDMA2000 1x
    GMSS_RAT_C2KHRPD,  // CDMA2000 HRPD
    GMSS_RAT_GSM,      // GERAN (GSM)
    GMSS_RAT_WCDMA,    // UTRAN (WCDMA)
    GMSS_RAT_LTE       // EUTRAN (LTE)
} GmssRat;

/* Define the GMSS status. */
typedef enum {
    GMSS_STATUS_SELECT,   // GMSS select.
    GMSS_STATUS_ATTACHED, // GMSS attached.
    GMSS_STATUS_ECC       // GMSS ECC.
} GmssStatus;

/*  <MSPL_set> integer type;
 *  The set(no priority) of Multimode System Selection (MMSS) System Priority List.
 *  Enum specified as:
 *  RAT_NONE              = 0,
 *  RAT_GSM               = 1,
 *  RAT_UMTS              = 2,
 *  RAT_GSM_UMTS          = RAT_GSM | RAT_UMTS,
 *  RAT_LTE               = 4,
 *  RAT_GSM_LTE           = RAT_GSM | RAT_LTE,
 *  RAT_UMTS_LTE          = RAT_UMTS | RAT_LTE,
 *  RAT_GSM_UMTS_LTE      = RAT_GSM | RAT_UMTS | RAT_LTE,
 *  RAT_COM_GSM           = 8,
 *  RAT_C2K               = 16,
 *  RAT_GSM_C2K           = RAT_GSM | RAT_C2K,
 *  RAT_UMTS_C2K          = RAT_UMTS | RAT_C2K,
 *  RAT_GSM_UMTS_C2K      = RAT_GSM | RAT_UMTS | RAT_C2K,
 *  RAT_LTE_C2K           = RAT_LTE | RAT_C2K,
 *  RAT_GSM_LTE_C2K       = RAT_GSM | RAT_LTE | RAT_C2K,
 *  RAT_UMTS_LTE_C2K      = RAT_UMTS | RAT_LTE | RAT_C2K,
 *  RAT_GSM_UMTS_LTE_C2K  = RAT_GSM | RAT_UMTS | RAT_LTE | RAT_C2K
 */
typedef enum {
    RAT_NONE     = 0,
    RAT_GSM      = 1,
    RAT_UMTS     = 2,
    RAT_LTE      = 4,
    RAT_COM_GSM  = 8,
    RAT_C2K      = 16
} GmssMspl;

/* Define the pending EGMSS information. */
typedef struct {
    int32_t count;
    int32_t rat;
    int32_t mcc;
    int32_t status;
    int32_t mspl;
    int32_t isHome;
    int32_t isSrlte;
} GmssInfo;

const char PROPERTY_ICCID_SIM[4][25] = {
    "vendor.ril.iccid.sim1",
    "vendor.ril.iccid.sim2",
    "vendor.ril.iccid.sim3",
    "vendor.ril.iccid.sim4",
};

#define INVALID_VALUE -1

#define NO_SERVICE_DELAY_TIME (15*1000)

#define Bool2Str(value) RpIrUtils::boolToString(value)
#define Nws2Str(value) RpIrUtils::nwsModeToString(value)
#define VolteState2Str(value) RpIrUtils::volteStateToString(value)
#define ServiceType2Str(value) RpIrUtils::serviceTypeToString(value)

extern "C" int strStartsWith(const char *line, const char *prefix);

/*****************************************************************************
 * Class RpIrUtils
 *****************************************************************************/

class RpIrUtils {

public:
    RpIrUtils();
    virtual ~RpIrUtils();

    static const char* boolToString(bool value);
    static const char* nwsModeToString(NwsMode mode);
    static const char* volteStateToString(VolteState volteState);
    static const char* serviceTypeToString(ServiceType serviceType);
    static const char* prefNwType2Str(int prefNwType);
    static bool is3GPrefNwType(int curPrefNwType);
    static bool isLcgSupport();
    static bool isWorldPhoneSupport();
    static bool isWorldModeSupport();
    static bool isViceSimSupportLte();
    static bool isSimReady(int slot_id);
    static bool isSimLocked(int slot_id);
    static bool isCdma3GDualModeCard(int slot_id);
    static bool isCdma4GDualModeCard(int slot_id);
    static bool isCdmaDualModeSimCard(int slot_id);
    static bool isSupportRoaming(int slot_id);
    static bool isViceOfTwoCtDualVolte(int slot_id);
    static int getCtCardType(int slot_id);
    static int getMainCapSlot();
    static int getCdmaSlotId();
    static int getIccAppFamily(int slot_id);
    static int getDefaultDataSim();
    static int isCdmaLteDcSupport();
};

#endif /* __RP_IR_UTILS_H__ */


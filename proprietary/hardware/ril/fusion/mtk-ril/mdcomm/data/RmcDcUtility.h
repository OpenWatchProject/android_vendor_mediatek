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

#ifndef __RMC_DC_UTILITY_H__
#define __RMC_DC_UTILITY_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RmcDataDefs.h"
#include "RfxLog.h"
#include <string>
#include <stdlib.h>
#include "rfx_properties.h"
#include <telephony/mtk_ril.h>

#define APN_TYPE_INVALID -1

/*****************************************************************************
 * Class RmcDcUtility
 *****************************************************************************/
class RmcDcUtility {
    public:
        static int isOp07Support();
        static int isOp12Support();
        static int isOp16Support();
        static int isOp20Support();
        static int getAddressType(char* addr);
        static const char* getProfileType(const int profileType);
        static const char* getMtkProfileType(const int profileType);
        static int getApnTypeId(const int profileType);
        static int getMtkApnTypeId(const int profileType);
        static const char* getApnType(int apnTypeId);
        static int getProfileID(int apnTypeId);
        static int getProtocolType(const char* protocol);
        static const char* getProtocolName(int protocol);
        static int getProtocolClassBitmap(int protocol);
        static int getAuthType(int authType);
        static int stringToBinaryBase(char *str, int base, int *err);
        static int isImsSupport();
        static const char* addrTypeToString(ADDRESS_TYPE addrType);
        static const char* pdnTypeToString(int pdnType);
        static void getPropertyBySlot(int slotId, const char *pPropertyName, char *pPropertyValue);
        static const char* checkRcsSupportPcscf();
        static int isOp12MccMnc(const char *mccmnc);
        static bool isUserBuild();
        static bool isSupportOverrideModemDefaultApn();
        static bool isSupportDefaultImsApnSettings();
        static bool isWifiApnExplicitlyConfigured();
        static bool isSupportSscMode(int desireMode);
};
#endif /* __RMC_DC_UTILITY_H__ */

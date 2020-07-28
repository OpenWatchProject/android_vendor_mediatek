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

#ifndef _IMSCONFIGUTILS_H_
#define _IMSCONFIGUTILS_H_
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <string>
#include <String8.h>
#include "RfxDefs.h"

using ::android::String8;
/*****************************************************************************
 * Class RtcImsConfigUtils
 *****************************************************************************/

typedef struct {
    int mccmnc;
    int opId;
} ConfigOperatorTable;

class ImsConfigUtils{

public:
    static int getSystemPropValue(const char* propName);
    static std::string getSystemPropStringValue(const char* propName);
    static int getFeaturePropValue(const char* propName, int slot_id);
    static void setFeaturePropValue(const char* propName, char* enabled, int slot_id);
    static int setBitForPhone(int featureValue, int enabled, int slot_id);

    static bool isMultiImsSupport();
    static bool checkIsPhoneIdValid(int slot_id);

    static bool isTestSim(int slot_id);
    static bool isCtVolteDisabled(int slot_id);
    static bool isPhoneIdSupportIms(int slot_id);
    static int getOperatorId(int mccmnc);
    static bool isAllowForceNotify(int slot_id, int value);
    static void setAllowForceNotify(int slot_id, bool allow, int value);

private:
    static void printLog(int level, String8 log, int slot_id);

public:
    static const char* PROPERTY_IMS_SUPPORT;
    static const char* PROPERTY_VOLTE_ENALBE;
    static const char* PROPERTY_WFC_ENALBE;
    static const char* PROPERTY_VILTE_ENALBE;
    static const char* PROPERTY_VIWIFI_ENALBE;
    static const char* PROPERTY_VONR_ENALBE;
    static const char* PROPERTY_VINR_ENALBE;
    static const char* PROPERTY_IMS_VIDEO_ENALBE;
    static const char* PROPERTY_MULTI_IMS_SUPPORT;
    static const char* PROPERTY_DYNAMIC_IMS_SWITCH_SUPPORT;
    static const char* PROPERTY_CTVOLTE_ENABLE;

    static const char PROPERTY_ICCID_SIM[4][25];
    static const char PROPERTY_TEST_SIM[4][30];

private :
    static const bool DEBUG_ENABLED;
    static bool mForceNotify[4];
    static int mForceValue[4];
};

#endif
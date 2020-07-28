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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "camera_custom_3dnr.h"
#include <stdlib.h>  // For atio().
#include <cutils/properties.h>  // For property_get().
#include <isp_tuning/isp_tuning.h>
using namespace NSIspTuning;


// E.g. 600 means that THRESHOLD_LOW is ISO600.
#define ISO_ENABLE_THRESHOLD_LOW        600

// E.g. 800 means that THRESHOLD_HIGH is ISO600.
#define ISO_ENABLE_THRESHOLD_HIGH       800

#if 0   // Obsolete
// E.g. 60 means that use 60% of Max Current ISO as THRESHOLD_LOW.
#define ISO_ENABLE_THRESHOLD_LOW_PERCENTAGE        60

// E.g. 80 means that use 80% of Max Current ISO as THRESHOLD_HIGH.
#define ISO_ENABLE_THRESHOLD_HIGH_PERCENTAGE       80
#endif  // Obsolete

// E.g. 130 means thatrRaise max ISO limitation to 130% when 3DNR on.
// When set to 100, 3DNR is noise improvement priority.
// When set to higher than 100, 3DNR is frame rate improvement priority.
#define MAX_ISO_INCREASE_PERCENTAGE     100

// How many frames should 3DNR HW be turned off (for power saving) if it
// stays at inactive state. (Note: inactive state means ISO is lower than
// ISO_ENABLE_THRESHOLD_LOW).
#define HW_POWER_OFF_THRESHOLD          60

// How many frames should 3DNR HW be turned on again if it returns from
// inactive state and stays at active state. (Note: active state means
// ISO is higher than ISO_ENABLE_THRESHOLD_LOW).
#define HW_POWER_REOPEN_DELAY           4

// ISO value must higher then threshold to turn on 3DNR
#define NR3D_OFF_ISO_THRESHOLD          400
#define VHDR_NR3D_OFF_ISO_THRESHOLD     400

// GMV value must lower then threshold to turn on 3DNR
#define NR3D_GMV_THRESHOLD              28

int get_3dnr_iso_enable_threshold_low(void)
{
    // Force change ISO Limit.
    unsigned int IsoEnableThresholdLowTemp = 0;
    unsigned int i4TempInputValue = ::property_get_int32("vendor.camera.3dnr.lowiso", 0);

    if (i4TempInputValue != 0)  // Raise AE ISO limit to 130%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 130: Raise Increase ISO Limit to 130% (increase 30%). 100: it means don't need to increase.
    {
        IsoEnableThresholdLowTemp = i4TempInputValue;
    }
    else
    {
        IsoEnableThresholdLowTemp = ISO_ENABLE_THRESHOLD_LOW;
    }

    return IsoEnableThresholdLowTemp;
}

int get_3dnr_iso_enable_threshold_high(void)
{
    // Force change ISO Limit.
    unsigned int IsoEnableThresholdHighTemp = 0;
    unsigned int i4TempInputValue = ::property_get_int32("vendor.camera.3dnr.highiso", 0);

    if (i4TempInputValue != 0)  // Raise AE ISO limit to 130%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 130: Raise Increase ISO Limit to 130% (increase 30%). 100: it means don't need to increase.
    {
        IsoEnableThresholdHighTemp = i4TempInputValue;
    }
    else
    {
        IsoEnableThresholdHighTemp = ISO_ENABLE_THRESHOLD_HIGH;
    }

    return IsoEnableThresholdHighTemp;
}

#if 0   // Obsolete
int get_3dnr_iso_enable_threshold_low_percentage(void)
{
    // Force change ISO Limit.
    unsigned int IsoEnableThresholdLowPercentageTemp = 0;
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.3dnr.lowpercent", InputValue, "0");
    unsigned int i4TempInputValue = atoi(InputValue);
    if (i4TempInputValue != 0)  // Raise AE ISO limit to 130%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 130: Raise Increase ISO Limit to 130% (increase 30%). 100: it means don't need to increase.
    {
        IsoEnableThresholdLowPercentageTemp = i4TempInputValue;
    }
    else
    {
        IsoEnableThresholdLowPercentageTemp = ISO_ENABLE_THRESHOLD_LOW_PERCENTAGE;
    }

    return IsoEnableThresholdLowPercentageTemp;
}

int get_3dnr_iso_enable_threshold_high_percentage(void)
{
    // Force change ISO Limit.
    unsigned int IsoEnableThresholdHighPercentageTemp = 0;
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.3dnr.highpercent", InputValue, "0");
    unsigned int i4TempInputValue = atoi(InputValue);
    if (i4TempInputValue != 0)  // Raise AE ISO limit to 130%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 130: Raise Increase ISO Limit to 130% (increase 30%). 100: it means don't need to increase.
    {
        IsoEnableThresholdHighPercentageTemp = i4TempInputValue;
    }
    else
    {
        IsoEnableThresholdHighPercentageTemp = ISO_ENABLE_THRESHOLD_HIGH_PERCENTAGE;
    }

    return IsoEnableThresholdHighPercentageTemp;
}
#endif  // Obsolete

int get_3dnr_max_iso_increase_percentage(void)
{
    // Force change ISO Limit.
    unsigned int MaxIsoIncreasePercentageTemp = 0;
    unsigned int i4TempInputValue = ::property_get_int32("vendor.camera.3dnr.forceisolimit", 0);

    if (i4TempInputValue != 0)  // Raise AE ISO limit to 130%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 130: Raise Increase ISO Limit to 130% (increase 30%). 100: it means don't need to increase.
    {
        MaxIsoIncreasePercentageTemp = i4TempInputValue;
    }
    else
    {
        MaxIsoIncreasePercentageTemp = MAX_ISO_INCREASE_PERCENTAGE;
    }

    return MaxIsoIncreasePercentageTemp;
}

int get_3dnr_hw_power_off_threshold(void)
{
    return HW_POWER_OFF_THRESHOLD;
}

int get_3dnr_hw_power_reopen_delay(void)
{
    return HW_POWER_REOPEN_DELAY;
}

int get_3dnr_gmv_threshold(int force3DNR)
{
    int i4GmvThreshold = NR3D_GMV_THRESHOLD;

    if (force3DNR)
    {
        i4GmvThreshold = ::property_get_int32("vendor.debug.3dnr.gmv.threshold", NR3D_GMV_THRESHOLD);
    }
    return i4GmvThreshold;
}

/*******************************************************************************
* is VHDR case
********************************************************************************/
MBOOL is_vhdr_profile(MUINT8 ispProfile)
{

#if 0
    switch (ispProfile)
    {
        case EIspProfile_iHDR_Preview:
        case EIspProfile_zHDR_Preview:
        case EIspProfile_mHDR_Preview:
        case EIspProfile_iHDR_Video:
        case EIspProfile_zHDR_Video:
        case EIspProfile_mHDR_Video:
        case EIspProfile_iHDR_Preview_VSS:
        case EIspProfile_zHDR_Preview_VSS:
        case EIspProfile_mHDR_Preview_VSS:
        case EIspProfile_iHDR_Video_VSS:
        case EIspProfile_zHDR_Video_VSS:
        case EIspProfile_mHDR_Video_VSS:
        case EIspProfile_zHDR_Capture:
        case EIspProfile_mHDR_Capture:
        case EIspProfile_Auto_iHDR_Preview:
        case EIspProfile_Auto_zHDR_Preview:
        case EIspProfile_Auto_mHDR_Preview:
        case EIspProfile_Auto_iHDR_Video:
        case EIspProfile_Auto_zHDR_Video:
        case EIspProfile_Auto_mHDR_Video:
        case EIspProfile_Auto_iHDR_Preview_VSS:
        case EIspProfile_Auto_zHDR_Preview_VSS:
        case EIspProfile_Auto_mHDR_Preview_VSS:
        case EIspProfile_Auto_iHDR_Video_VSS:
        case EIspProfile_Auto_zHDR_Video_VSS:
        case EIspProfile_Auto_mHDR_Video_VSS:
        case EIspProfile_Auto_zHDR_Capture:
        case EIspProfile_Auto_mHDR_Capture:
        case EIspProfile_EIS_iHDR_Preview:
        case EIspProfile_EIS_zHDR_Preview:
        case EIspProfile_EIS_mHDR_Preview:
        case EIspProfile_EIS_iHDR_Video:
        case EIspProfile_EIS_zHDR_Video:
        case EIspProfile_EIS_mHDR_Video:
        case EIspProfile_EIS_Auto_iHDR_Preview:
        case EIspProfile_EIS_Auto_zHDR_Preview:
        case EIspProfile_EIS_Auto_mHDR_Preview:
        case EIspProfile_EIS_Auto_iHDR_Video:
        case EIspProfile_EIS_Auto_zHDR_Video:
        case EIspProfile_EIS_Auto_mHDR_Video:
            //VHDR case
            return true;
        default:
            //not VHDR case
            return false;
    }
#else
    return false;
#endif
}

/*******************************************************************************
* get 3dnr iso threshold ( default / property force case / feature case)
********************************************************************************/
MINT32 NR3DCustom::get_3dnr_off_iso_threshold(MUINT8 ispProfile, MBOOL useAdbValue)
{
    int i4IsoThreshold = 0;

    if(is_vhdr_profile(ispProfile))
    {
        //set custom threshold by vhdr scenario
        i4IsoThreshold = VHDR_NR3D_OFF_ISO_THRESHOLD;
    }
    else
    {
        //set default 3dnr iso threshold
        i4IsoThreshold = NR3D_OFF_ISO_THRESHOLD;
    }

    if (useAdbValue)
    {
        //property force set case
        i4IsoThreshold = ::property_get_int32("vendor.debug.3dnr.iso.threshold", i4IsoThreshold);
    }

    return i4IsoThreshold;
}


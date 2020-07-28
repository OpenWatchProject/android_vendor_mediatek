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



int get_3dnr_iso_enable_threshold_low(void)
{
    // Force change ISO Limit.
    unsigned int IsoEnableThresholdLowTemp = 0;
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.3dnr.lowiso", InputValue, "0"); 
    unsigned int i4TempInputValue = atoi(InputValue);
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
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.3dnr.highiso", InputValue, "0"); 
    unsigned int i4TempInputValue = atoi(InputValue);
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
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.3dnr.forceisolimit", InputValue, "0"); 
    unsigned int i4TempInputValue = atoi(InputValue);
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


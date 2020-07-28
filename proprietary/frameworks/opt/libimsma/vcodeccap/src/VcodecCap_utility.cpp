/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "[VT]VcodecCap"

#include <utils/KeyedVector.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <stdlib.h>

#include "IVcodecCap.h"
#include "VcodecCap.h"

using namespace android;


char *toString(video_profile_t profile)
{
    if (profile & VIDEO_PROFILE_HIGH)
    {
        return (char *)&("VIDEO_PROFILE_HIGH");
    }
    else if (profile & VIDEO_PROFILE_MAIN)
    {
        return (char *)&("VIDEO_PROFILE_MAIN");
    }
    else if (profile & VIDEO_PROFILE_BASELINE)
    {
        return (char *)&("VIDEO_PROFILE_BASELINE");
    }
    return (char *)&("VIDEO_PROFILE_UNKNOWN");
}

char *toString(video_level_t level)
{
    switch (level)
    {
        case VIDEO_LEVEL_0:
            return (char *)&("VIDEO_LEVEL_0");
        case VIDEO_LEVEL_1:
            return (char *)&("VIDEO_LEVEL_1");
        case VIDEO_HIGH_TIER_LEVEL_1:
            return (char *)&("VIDEO_HIGH_TIER_LEVEL_1");
        case VIDEO_LEVEL_1b:
            return (char *)&("VIDEO_LEVEL_1b");
        case VIDEO_LEVEL_1_1:
            return (char *)&("VIDEO_LEVEL_1_1");
        case VIDEO_LEVEL_1_2:
            return (char *)&("VIDEO_LEVEL_1_2");
        case VIDEO_LEVEL_1_3:
            return (char *)&("VIDEO_LEVEL_1_3");
        case VIDEO_LEVEL_2:
            return (char *)&("VIDEO_LEVEL_2");
        case VIDEO_HIGH_TIER_LEVEL_2:
            return (char *)&("VIDEO_HIGH_TIER_LEVEL_2");
        case VIDEO_LEVEL_2_1:
            return (char *)&("VIDEO_LEVEL_2_1");
        case VIDEO_HIGH_TIER_LEVEL_2_1:
            return (char *)&("VIDEO_HIGH_TIER_LEVEL_2_1");
        case VIDEO_LEVEL_2_2:
            return (char *)&("VIDEO_LEVEL_2_2");
        case VIDEO_LEVEL_3:
            return (char *)&("VIDEO_LEVEL_3");
        case VIDEO_HIGH_TIER_LEVEL_3:
            return (char *)&("VIDEO_HIGH_TIER_LEVEL_3");
        case VIDEO_LEVEL_3_1:
            return (char *)&("VIDEO_LEVEL_3_1");
        case VIDEO_HIGH_TIER_LEVEL_3_1:
            return (char *)&("VIDEO_HIGH_TIER_LEVEL_3_1");
        case VIDEO_LEVEL_3_2:
            return (char *)&("VIDEO_LEVEL_3_2");
        case VIDEO_LEVEL_4:
            return (char *)&("VIDEO_LEVEL_4");
        case VIDEO_LEVEL_4_1:
            return (char *)&("VIDEO_LEVEL_4_1");
        case VIDEO_LEVEL_4_2:
            return (char *)&("VIDEO_LEVEL_4_2");
        case VIDEO_LEVEL_5:
            return (char *)&("VIDEO_LEVEL_5");
        default:
            break;
    }
    return (char *)&("VIDEO_LEVEL_UNKNOWN");
}

void printBinary(unsigned char *ptrBS, int iLen)
{
    int i;
    char tmp[1024];
    char *ptr = tmp;
    for (i = 0; i < iLen; ++i)
    {
        sprintf(ptr, "%02x", ptrBS[i]);
        ptr += 2;
    }
    *ptr = '\0';
    ALOGI("[genParameterSets] bssize(%d), bs(%s)", iLen, tmp);
}

uint32_t getLevel(video_format_t format, video_level_t level)
{
    if (format == VIDEO_H264)
    {
        switch (level)
        {
            case VIDEO_LEVEL_1:
                return 10;
            case VIDEO_LEVEL_1_1:
                return 11;
            case VIDEO_LEVEL_1_2:
                return 12;
            case VIDEO_LEVEL_1_3:
                return 13;
            case VIDEO_LEVEL_2:
                return 20;
            case VIDEO_LEVEL_2_1:
                return 21;
            case VIDEO_LEVEL_2_2:
                return 22;
            case VIDEO_LEVEL_3:
                return 30;
            case VIDEO_LEVEL_3_1:
                return 31;
            case VIDEO_LEVEL_3_2:
                return 32;
            case VIDEO_LEVEL_4:
                return 40;
            case VIDEO_LEVEL_4_1:
                return 41;
            case VIDEO_LEVEL_4_2:
                return 42;
            case VIDEO_LEVEL_5:
                return 50;
            case VIDEO_LEVEL_5_1:
                return 51;
            default:
                break;
        }
    }
    else if (format == VIDEO_HEVC)
    {
        switch (level)
        {
            case VIDEO_LEVEL_1:
                return 30;
            case VIDEO_LEVEL_2:
                return 60;
            case VIDEO_LEVEL_2_1:
                return 63;
            case VIDEO_LEVEL_3:
                return 90;
            case VIDEO_LEVEL_3_1:
                return 93;
            case VIDEO_LEVEL_4:
                return 120;
            default:
                break;
        }
    }
    return 30;
}

video_level_t getLevel(video_format_t format, uint32_t level)
{
    if (format == VIDEO_H264)
    {
        switch (level)
        {
            case 10:
                return VIDEO_LEVEL_1;
            case 11:
                return VIDEO_LEVEL_1_1;
            case 12:
                return VIDEO_LEVEL_1_2;
            case 13:
                return VIDEO_LEVEL_1_3;
            case 20:
                return VIDEO_LEVEL_2;
            case 21:
                return VIDEO_LEVEL_2_1;
            case 22:
                return VIDEO_LEVEL_2_2;
            case 30:
                return VIDEO_LEVEL_3;
            case 31:
                return VIDEO_LEVEL_3_1;
            case 32:
                return VIDEO_LEVEL_3_2;
            case 40:
                return VIDEO_LEVEL_4;
            case 41:
                return VIDEO_LEVEL_4_1;
            case 42:
                return VIDEO_LEVEL_4_2;
            case 50:
                return VIDEO_LEVEL_5;
            case 51:
                return VIDEO_LEVEL_5_1;
            default:
                break;
        }
    }
    else if (format == VIDEO_HEVC)
    {
        switch (level)
        {
            case 30:
                return VIDEO_LEVEL_1;
            case 60:
                return VIDEO_LEVEL_2;
            case 63:
                return VIDEO_LEVEL_2_1;
            case 90:
                return VIDEO_LEVEL_3;
            case 93:
                return VIDEO_LEVEL_3_1;
            case 120:
                return VIDEO_LEVEL_4;
            default:
                break;
        }
    }
    return VIDEO_LEVEL_UNKNOWN;
}


uint32_t getProfile(video_format_t format, video_profile_t profile)
{
    if (format == VIDEO_H264)
    {
        // (Baseline=66, Main=77, High=100)
        switch (profile)
        {
            case VIDEO_PROFILE_BASELINE:
                return 66;
                break;
            case VIDEO_PROFILE_MAIN:
                return 77;
                break;
            case VIDEO_PROFILE_HIGH:
                return 100;
                break;
            default:
                break;
        };
        return 66;
    }
    else if (format == VIDEO_HEVC)
    {
        return 1;
    }
    return 0;
}

video_profile_t getProfile(video_format_t format, uint32_t u4Profile)
{
    if (format == VIDEO_H264)
    {
        // (Baseline=66, Main=77, High=100)
        switch (u4Profile)
        {
            case 66:
                return VIDEO_PROFILE_BASELINE;
                break;
            case 77:
                return VIDEO_PROFILE_MAIN;
                break;
            case 100:
                return VIDEO_PROFILE_HIGH;
                break;
            default:
                break;
        };
    }
    else if (format == VIDEO_HEVC)
    {
        return VIDEO_PROFILE_MAIN;
    }

    return VIDEO_PROFILE_UNKNOWN;
}

char *getOperatorName(int opid)
{
    switch (opid)
    {
        case 1:     return (char *)&("CMCC");
        case 2:     return (char *)&("CU");
        case 3:     return (char *)&("Orange");
        case 5:     return (char *)&("TMO EU");
        case 6:     return (char *)&("Vodafone");
        case 7:     return (char *)&("AT&T");
        case 8:     return (char *)&("TMO US");
        case 9:     return (char *)&("CT");
        case 10:    return (char *)&("Tier-2");
        case 11:    return (char *)&("H3G");
        case 12:    return (char *)&("Verizon");
        case 15:    return (char *)&("Telefonica");
        case 16:    return (char *)&("EE");
        case 17:    return (char *)&("DoCoMo");
        case 18:    return (char *)&("Reliance");
        case 19:    return (char *)&("Telstra");
        case 20:    return (char *)&("Sprint");
        case 50:    return (char *)&("Softbank");
        case 100:   return (char *)&("CSL");
        case 101:   return (char *)&("PCCW ");
        case 102:   return (char *)&("SMT");
        case 103:   return (char *)&("SingTel");
        case 104:   return (char *)&("Starhub");
        case 105:   return (char *)&("AMX");
        case 106:   return (char *)&("3HK");
        case 107:   return (char *)&("SFR");
        case 108:   return (char *)&("TWN");
        case 109:   return (char *)&("CHT");
        case 110:   return (char *)&("FET");
        case 111:   return (char *)&("India Operator");
        case 112:   return (char *)&("Telcel");
        case 113:   return (char *)&("Beeline");
        case 114:   return (char *)&("KT (KOREA TELECOME)");
        case 115:   return (char *)&("SKT (SK Telecome)");
        case 116:   return (char *)&("U+ (LG UPLUS)");
        case 117:   return (char *)&("Smartfren");
        case 118:   return (char *)&("YTL");
        case 119:   return (char *)&("Natcom");
        case 120:   return (char *)&("Claro(COLOMBIA)");
        case 121:   return (char *)&("Bell");
        case 122:   return (char *)&("AIS");
        case 124:   return (char *)&("APTG");
        case 125:   return (char *)&("DTAC");
        case 126:   return (char *)&("Avea");
        case 127:   return (char *)&("Megafon");
        case 128:   return (char *)&("DNA");
        case 129:   return (char *)&("KDDI");
        case 130:   return (char *)&("TIM");
        case 131:   return (char *)&("TrueMove");
        case 132:   return (char *)&("Movistar");
        case 135:   return (char *)&("MTS");
        case 143:   return (char *)&("Turkcell");
        case 147:   return (char *)&("Airtel");
        case 153:   return (char *)&("VHA");
        case 154:   return (char *)&("Telia");
        case 156:   return (char *)&("Telenor");
        case 161:   return (char *)&("Play");
        case 163:   return (char *)&("Sri-Lanka");
        case 165:   return (char *)&("Sunrise");
        case 178:   return (char *)&("Smart");
        case 182:   return (char *)&("Proximus");
        case 187:   return (char *)&("Safaricom");
        case 202:   return (char *)&("Spark");
        case 221:   return (char *)&("TPG");
        case 223:   return (char *)&("Ooredoo");
        case 231:   return (char *)&("Maxis");
        case 234:   return (char *)&("Plus");
        case 1002:  return (char *)&("Nokia");
        case 1004:  return (char *)&("Huawei");
        default:
            return (char *)&("NONE");
    }
}

video_property_t* getVideoProperty()
{
    static int property_init = 0;
    static video_property_t video_property;
    if (property_init == 0)
    {
        char value[PROPERTY_VALUE_MAX];
        property_get("persist.vendor.vt.vformat", value, "1");
        video_property.format = (video_format_t)atoi(value);

        property_get("persist.vendor.vt.vprofile", value, "0");
        video_property.profile = (video_profile_t)atoi(value);

        property_get("persist.vendor.vt.vlevel", value, "0");
        video_property.level = getLevel(VIDEO_H264, atoi(value)); //FIXME

        property_get("persist.vendor.vt.vfps", value, "0");
        video_property.fps = atoi(value);

        property_get("persist.vendor.vt.vbitrate", value, "0");
        video_property.bitrate = atoi(value) * 1000;

        property_get("persist.vendor.vt.viperiod", value, "0");
        video_property.Iinterval = atoi(value);

        ALOGI("[getVideoProperty] f:%d,P:%d,L:%d,F:%d,B:%d,I:%d",
                video_property.format,
                video_property.profile,
                video_property.level,
                video_property.fps,
                video_property.bitrate,
                video_property.Iinterval);
        property_init = 1;
    }
    return &video_property;
}


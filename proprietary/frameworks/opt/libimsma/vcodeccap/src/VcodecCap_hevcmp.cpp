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

#include "IVcodecCap.h"
#include "VcodecCap.h"

using namespace android;

static const int MAX_HEVC_MEDIA_PROFILE_NUM  = 5;
static video_media_profile_t rmedia_profile[MAX_HEVC_MEDIA_PROFILE_NUM] =
{
    {VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_1,  176,  144, 15,  145 * 1000, 270 * 1000, 1},
    {VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_2,  352,  288, 30,  360 * 1000, 526 * 1000, 1},
    {VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_2_1,  480,  320, 30,  495 * 1000, 654 * 1000, 1},
    {VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_3,    640,  480, 30,  742 * 1000, 974 * 1000, 1},
    {VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_3_1,  1280,  720, 30,  1314 * 1000, 1714 * 1000, 1},
};
///////////////////////////////////////////////////////////////////////////////////
// OP01 (CMCC)

static const int MAX_HEVC_MEDIA_PROFILE_NUM_OP01  = 4;
static video_media_profile_t rmedia_profile_op01[MAX_HEVC_MEDIA_PROFILE_NUM_OP01] =
{
    {VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_1,    320,  240, 10,  145 * 1000, 270 * 1000, 1},
    {VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_2,    320,  240, 20,  327 * 1000, 462 * 1000, 1},
    {VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_2_1,  480,  320, 30,  450 * 1000, 594 * 1000, 1},
    {VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_3,    640,  480, 30,  450 * 1000, 670 * 1000, 1},
};
static const int MAX_QUALITY_NUM  = 4;
static QualityInfo_t rQualityInfo_OP01[MAX_QUALITY_NUM] =
{
    {VIDEO_QUALITY_FINE,   VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_3},
    {VIDEO_QUALITY_HIGH,   VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_3},
    {VIDEO_QUALITY_MEDIUM, VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_2_1},
    {VIDEO_QUALITY_LOW,    VIDEO_HEVC, VIDEO_PROFILE_MAIN, VIDEO_LEVEL_2},
};

///////////////////////////////////////////////////////////////////////////////////
void getDefaultHEVCMediaProfileByOperator(int opid, video_media_profile_t **ppvideo_media_profile_t, int *pCount)
{
    switch(opid)
    {
        case 1: // CMCC
            *ppvideo_media_profile_t = rmedia_profile_op01;
            *pCount = MAX_HEVC_MEDIA_PROFILE_NUM_OP01;
            break;
        default:
            *ppvideo_media_profile_t = rmedia_profile;
            *pCount = MAX_HEVC_MEDIA_PROFILE_NUM;
            break;
    }
}

void createHEVCQualityMediaProfileByOperator(int opid, QualityInfo_t *pQualityInfo)
{
    switch (opid)
    {
        case 1: // CMCC
            {
                memcpy(pQualityInfo, rQualityInfo_OP01, sizeof(QualityInfo_t) * MAX_QUALITY_NUM);
            }
            break;
        default:
            {
                memcpy(pQualityInfo, rQualityInfo_OP01, sizeof(QualityInfo_t) * MAX_QUALITY_NUM);
            }
            break;
    }
}


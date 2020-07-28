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

#ifndef VCODECCAP_H
#define VCODECCAP_H

#include <utils/KeyedVector.h>
#include <utils/Vector.h>

using namespace android;

typedef struct sensor_level_info
{
    video_level_t level;
    uint32_t width;
    uint32_t height;
} sensor_level_info_t;

typedef struct vcodec_level_property_value
{
    uint32_t    max_mbps;
    uint32_t    max_smbps;
    uint32_t    max_fs;
    uint32_t    max_cpb;
    uint32_t    max_dpb;
    uint32_t    max_br;
} vcodec_level_property_value_t;

typedef struct _vdec_cap
{
    video_profile_t    profile;
    video_level_t      max_level;

    _vdec_cap()
    {
        profile = VIDEO_PROFILE_UNKNOWN;
        max_level = VIDEO_LEVEL_UNKNOWN;
    }
} vdec_cap_t;

typedef struct _venc_cap
{
    video_profile_t    profile;
    video_level_t      max_level;

    _venc_cap()
    {
        profile = VIDEO_PROFILE_UNKNOWN;
        max_level = VIDEO_LEVEL_UNKNOWN;
    }
} venc_cap_t;

typedef struct _video_default_cap
{
    video_profile_t    profile;
    video_level_t      level;

    _video_default_cap()
    {
        profile = VIDEO_PROFILE_UNKNOWN;
        level = VIDEO_LEVEL_UNKNOWN;
    }
} video_default_cap_t;

typedef struct _video_property
{
    video_format_t  format;
    video_profile_t	profile;
    video_level_t	level;
    uint32_t		fps;
    uint32_t		bitrate;	// in kbps
    uint32_t		Iinterval;

  _video_property()
  {
    format = VIDEO_H264;
    profile = VIDEO_PROFILE_UNKNOWN;
    level = VIDEO_LEVEL_UNKNOWN;
    fps = 0;
    bitrate = 0;
    Iinterval = 0;
  }
} video_property_t;

typedef struct QualityInfo
{
    video_quality_t    quality;
    video_format_t     format;
    video_profile_t    profile;
    video_level_t      level;
} QualityInfo_t;

typedef struct MediaProfileList
{
    int mOPID;
    video_quality_t mDefault_quality;
    Vector<video_format_t> mVideoFormatList;
    Vector<QualityInfo_t> mQualityList[VIDEO_FORMAT_NUM];
    Vector<video_media_profile_t *> mMediaProfile[VIDEO_FORMAT_NUM];
    KeyedVector<video_quality_t, video_media_profile_t> mQualityMediaProfile[VIDEO_FORMAT_NUM];
} MediaProfileList_t;

/**
 * @par Enumeration
 *   VENC_DRV_MRESULT_T
 * @par Description
 *   This is the return value for eVEncDrvXXX()
 */
typedef enum __VENC_DRV_MRESULT_T {
    VENC_DRV_MRESULT_OK = 0,                    /* /< Return Success */
    VENC_DRV_MRESULT_FAIL,                      /* /< Return Fail */
    VENC_DRV_MRESULT_MAX = 0x0FFFFFFF           /* /< Max VENC_DRV_MRESULT_T value */
} VENC_DRV_MRESULT_T;


void getDefaultH264MediaProfileByOperator(int opid, video_media_profile_t **ppvideo_media_profile_t, int *pCount);
void getDefaultHEVCMediaProfileByOperator(int opid, video_media_profile_t **ppvideo_media_profile_t, int *pCount);
void createH264QualityMediaProfileByOperator(int opid, QualityInfo_t *pQualityInfo);
void createHEVCQualityMediaProfileByOperator(int opid, QualityInfo_t *pQualityInfo);
MediaProfileList_t *getMediaProfileListInst(uint32_t opID);
video_media_profile_t *getMediaProfileEntry(MediaProfileList_t *pMediaProfileList_t, video_format_t format, video_profile_t profile, video_level_t level);

char *toString(video_profile_t profile);
char *toString(video_level_t level);
void printBinary(unsigned char *ptrBS, int iLen);
uint32_t getLevel(video_format_t format, video_level_t level);
video_level_t getLevel(video_format_t format, uint32_t level);
uint32_t getProfile(video_format_t format, video_profile_t profile);
video_profile_t getProfile(video_format_t format, uint32_t u4Profile);
char *getOperatorName(int opid);
video_property_t* getVideoProperty();

#endif


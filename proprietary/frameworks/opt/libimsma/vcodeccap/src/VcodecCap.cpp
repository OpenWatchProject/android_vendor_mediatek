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

#include <utils/Log.h>
#include <sys/types.h>
#include <cutils/properties.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <unistd.h>
#include "IVcodecCap.h"
#include "VcodecCap.h"

#include "EncodeCap.h"
#include "VcodecCap_genHeader.h"

#include <media/stagefright/omx/OMXUtils.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/OMXClient.h>

#define USE_OMX_NODE 1
#ifdef USE_OMX_NODE

//#define _DEBUG_

using namespace android;
#define VENC_UNUSED(x) (void)x

#define VENC_ROUND_N(X, N)   (((X) + ((N)-1)) & (~((N)-1)))    //only for N is exponential of 2
#define CHECK_IF_BREAK(fun, error, errorstr)    \
    if (VENC_DRV_MRESULT_FAIL == fun)  \
    {   \
        ALOGE("[ERROR] %s",errorstr);   \
        error = -1; \
        break;  \
    }
#define CHECK_IF_WARN(fun, errorstr)    \
    if (VENC_DRV_MRESULT_FAIL == fun && errorstr != NULL)  \
    {   \
        ALOGW("[WARN] %s",errorstr);   \
    }

static video_level_t VIDEO_LEVEL[2][26] =
{
    {
        // Decoder
        VIDEO_LEVEL_UNKNOWN,
        VIDEO_LEVEL_0,
        VIDEO_LEVEL_1,
        VIDEO_HIGH_TIER_LEVEL_1,    //< Specified by HEVC
        VIDEO_LEVEL_1b,
        VIDEO_LEVEL_1_1,
        VIDEO_LEVEL_1_2,
        VIDEO_LEVEL_1_3,
        VIDEO_LEVEL_2,
        VIDEO_HIGH_TIER_LEVEL_2,    ///< Specified by HEVC
        VIDEO_LEVEL_2_1,
        VIDEO_HIGH_TIER_LEVEL_2_1,  ///< Specified by HEVC
        VIDEO_LEVEL_2_2,
        VIDEO_LEVEL_3,
        VIDEO_HIGH_TIER_LEVEL_3,    ///< Specified by HEVC
        VIDEO_LEVEL_3_1,
        VIDEO_HIGH_TIER_LEVEL_3_1,   ///< Specified by HEVC
        VIDEO_LEVEL_3_2,
        VIDEO_LEVEL_4,
        VIDEO_HIGH_TIER_LEVEL_4,     ///< Specified by HEVC
        VIDEO_LEVEL_4_1,
        VIDEO_HIGH_TIER_LEVEL_4_1,   ///< Specified by HEVC
        VIDEO_LEVEL_4_2,
        VIDEO_LEVEL_5,
        VIDEO_HIGH_TIER_LEVEL_5,     ///< Specified by HEVC
        VIDEO_LEVEL_5_1,
    },
    {
        // Encoder
        VIDEO_LEVEL_UNKNOWN,
        VIDEO_LEVEL_0,
        VIDEO_LEVEL_1,
        VIDEO_LEVEL_1b,
        VIDEO_LEVEL_1_1,
        VIDEO_LEVEL_1_2,
        VIDEO_LEVEL_1_3,
        VIDEO_LEVEL_2,
        VIDEO_LEVEL_2_1,
        VIDEO_LEVEL_2_2,
        VIDEO_LEVEL_3,
        VIDEO_LEVEL_3_1,
        VIDEO_LEVEL_3_2,
        VIDEO_LEVEL_4,
        VIDEO_LEVEL_4_1,
        VIDEO_LEVEL_4_2,
        VIDEO_LEVEL_5,
        VIDEO_LEVEL_5_1,
        VIDEO_LEVEL_UNKNOWN,
        VIDEO_LEVEL_UNKNOWN,
        VIDEO_LEVEL_UNKNOWN,
        VIDEO_LEVEL_UNKNOWN,
        VIDEO_LEVEL_UNKNOWN,
        VIDEO_LEVEL_UNKNOWN,
        VIDEO_LEVEL_UNKNOWN,
        VIDEO_LEVEL_UNKNOWN
    }
};

static sensor_level_info_t sensorLevel[14] =
{
    {VIDEO_LEVEL_1_1, 176, 144},
    {VIDEO_LEVEL_1_2, 320, 240},
    {VIDEO_LEVEL_1_3, 352, 288},
    {VIDEO_LEVEL_2_1, 352, 480},
    {VIDEO_LEVEL_2_2, 352, 480},
    {VIDEO_LEVEL_3, 640, 480},
    {VIDEO_LEVEL_3, 720, 480},
    {VIDEO_LEVEL_3_1, 1280, 720},
    {VIDEO_LEVEL_3_2, 1280, 720},
    {VIDEO_LEVEL_4, 1920, 1080},
    {VIDEO_LEVEL_4_1, 1920, 1080},
    {VIDEO_LEVEL_4_2, 1920, 1080},
    {VIDEO_LEVEL_5, 2560, 1920},
    {VIDEO_LEVEL_5_1, 4096, 2048}
};

video_profile_t getProfilefromOMXAVC(OMX_U32 eProfile){
    switch(eProfile){
        case OMX_VIDEO_AVCProfileBaseline:
            return VIDEO_PROFILE_BASELINE;
            break;
        case OMX_VIDEO_AVCProfileMain:
            return VIDEO_PROFILE_MAIN;
            break;
        case OMX_VIDEO_AVCProfileHigh:
            return VIDEO_PROFILE_HIGH;
            break;
        default:
            return VIDEO_PROFILE_UNKNOWN;
            break;
    }
}

video_profile_t getProfilefromOMXHEVC(OMX_U32 eProfile){
    switch(eProfile){
        case OMX_VIDEO_HEVCProfileMain:
            return VIDEO_PROFILE_MAIN;
            break;
        default:
            return VIDEO_PROFILE_UNKNOWN;
            break;
    }
}

video_level_t getLevelfromOMXAVC(OMX_U32 eLevel){
    switch(eLevel){
        case OMX_VIDEO_AVCLevel1:
            return VIDEO_LEVEL_1;
            break;
        case OMX_VIDEO_AVCLevel1b:
            return VIDEO_LEVEL_1b;
            break;
        case OMX_VIDEO_AVCLevel11:
            return VIDEO_LEVEL_1_1;
            break;
        case OMX_VIDEO_AVCLevel12:
            return VIDEO_LEVEL_1_2;
            break;
        case OMX_VIDEO_AVCLevel13:
            return VIDEO_LEVEL_1_3;
            break;
        case OMX_VIDEO_AVCLevel2:
            return VIDEO_LEVEL_2;
            break;
        case OMX_VIDEO_AVCLevel21:
            return VIDEO_LEVEL_2_1;
            break;
        case OMX_VIDEO_AVCLevel22:
            return VIDEO_LEVEL_2_2;
            break;
        case OMX_VIDEO_AVCLevel3:
            return VIDEO_LEVEL_3;
            break;
        case OMX_VIDEO_AVCLevel31:
            return VIDEO_LEVEL_3_1;
            break;
        case OMX_VIDEO_AVCLevel32:
            return VIDEO_LEVEL_3_2;
            break;
        case OMX_VIDEO_AVCLevel4:
            return VIDEO_LEVEL_4;
            break;
        case OMX_VIDEO_AVCLevel41:
            return VIDEO_LEVEL_4_1;
            break;
        case OMX_VIDEO_AVCLevel42:
            return VIDEO_LEVEL_4_2;
            break;
        case OMX_VIDEO_AVCLevel5:
            return VIDEO_LEVEL_5;
            break;
        case OMX_VIDEO_AVCLevel51:
            return VIDEO_LEVEL_5_1;
            break;
        default:
            return VIDEO_LEVEL_UNKNOWN;
            break;
    }
}

video_level_t getLevelfromOMXHEVC(OMX_U32 eLevel){
    switch(eLevel){
        case OMX_VIDEO_HEVCMainTierLevel1:
            return VIDEO_LEVEL_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel1:
            return VIDEO_HIGH_TIER_LEVEL_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel2:
            return VIDEO_LEVEL_2;
            break;
        case OMX_VIDEO_HEVCHighTierLevel2:
            return VIDEO_HIGH_TIER_LEVEL_2;
            break;
        case OMX_VIDEO_HEVCMainTierLevel21:
            return VIDEO_LEVEL_2_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel21:
            return VIDEO_HIGH_TIER_LEVEL_2_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel3:
            return VIDEO_LEVEL_3;
            break;
        case OMX_VIDEO_HEVCHighTierLevel3:
            return VIDEO_HIGH_TIER_LEVEL_3;
            break;
        case OMX_VIDEO_HEVCMainTierLevel31:
            return VIDEO_LEVEL_3_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel31:
            return VIDEO_HIGH_TIER_LEVEL_3_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel4:
            return VIDEO_LEVEL_4;
            break;
        case OMX_VIDEO_HEVCHighTierLevel4:
            return VIDEO_HIGH_TIER_LEVEL_4;
            break;
        case OMX_VIDEO_HEVCMainTierLevel41:
            return VIDEO_LEVEL_4_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel41:
            return VIDEO_HIGH_TIER_LEVEL_4_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel5:
            return VIDEO_LEVEL_5;
            break;
        case OMX_VIDEO_HEVCHighTierLevel5:
            return VIDEO_HIGH_TIER_LEVEL_5;
            break;
        default:
            return VIDEO_LEVEL_UNKNOWN;
            break;
    }
}
namespace android {
    static int32_t getVEncCapability_1(video_format_t format, venc_cap_t *pvenc_cap_t);
    static int32_t getVDecCapability_1(video_format_t format, vdec_cap_t *pvdec_cap_t);
}

static video_media_profile_t *getMediaProfileEntryBySIM(uint32_t opID, video_format_t format, video_profile_t profile, video_level_t level);

static void decodeBase64(const void *_data, size_t size, uint8_t *out)
{
    const uint8_t *data = (const uint8_t *)_data;

    if ((size % 4) != 0)
    {
        return;
    }

    size_t padding = 0;
    if (size >= 1 && data[size - 1] == '=')
    {
        padding = 1;

        if (size >= 2 && data[size - 2] == '=')
        {
            padding = 2;
        }
    }

    size_t outLen = 3 * size / 4 - padding;

    size_t j = 0;
    uint32_t accum = 0;
    for (size_t i = 0; i < size; ++i)
    {
        char c = data[i];
        unsigned value;
        if (c >= 'A' && c <= 'Z')
        {
            value = c - 'A';
        }
        else if (c >= 'a' && c <= 'z')
        {
            value = 26 + c - 'a';
        }
        else if (c >= '0' && c <= '9')
        {
            value = 52 + c - '0';
        }
        else if (c == '+')
        {
            value = 62;
        }
        else if (c == '/')
        {
            value = 63;
        }
        else if (c != '=')
        {
            return;
        }
        else
        {
            if (i < size - padding)
            {
                return;
            }

            value = 0;
        }

        accum = (accum << 6) | value;

        if (((i + 1) % 4) == 0)
        {
            out[j++] = (accum >> 16);

            if (j < outLen)
            {
                out[j++] = (accum >> 8) & 0xff;
            }
            if (j < outLen)
            {
                out[j++] = accum & 0xff;
            }

            accum = 0;
        }
    }
}

static char encode6Bit(unsigned x)
{
    if (x <= 25)
    {
        return 'A' + x;
    }
    else if (x <= 51)
    {
        return 'a' + x - 26;
    }
    else if (x <= 61)
    {
        return '0' + x - 52;
    }
    else if (x == 62)
    {
        return '+';
    }
    else
    {
        return '/';
    }
}

static int32_t encodeBase64(
    const void *_data, size_t size, uint8_t *out)
{
    int32_t pos = 0;
    const uint8_t *data = (const uint8_t *)_data;

    size_t i;
    for (i = 0; i < (size / 3) * 3; i += 3)
    {
        uint8_t x1 = data[i];
        uint8_t x2 = data[i + 1];
        uint8_t x3 = data[i + 2];

        out[pos++] = (encode6Bit(x1 >> 2));
        out[pos++] = (encode6Bit((x1 << 4 | x2 >> 4) & 0x3f));
        out[pos++] = (encode6Bit((x2 << 2 | x3 >> 6) & 0x3f));
        out[pos++] = (encode6Bit(x3 & 0x3f));
    }
    switch (size % 3)
    {
        case 0:
            break;
        case 2:
        {
            uint8_t x1 = data[i];
            uint8_t x2 = data[i + 1];
            out[pos++] = (encode6Bit(x1 >> 2));
            out[pos++] = (encode6Bit((x1 << 4 | x2 >> 4) & 0x3f));
            out[pos++] = (encode6Bit((x2 << 2) & 0x3f));
            out[pos++] = ('=');
            break;
        }
        default:
        {
            uint8_t x1 = data[i];
            out[pos++] = (encode6Bit(x1 >> 2));
            out[pos++] = (encode6Bit((x1 << 4) & 0x3f));
            out[pos++] = ('=');
            out[pos++] = ('=');
            break;
        }
    }

    return pos;
}

static video_profile_t getMaxProfile(video_profile_t profile)
{
    if (profile & VIDEO_PROFILE_HIGH)
    {
        return VIDEO_PROFILE_HIGH;
    }
    else if (profile & VIDEO_PROFILE_MAIN)
    {
        return VIDEO_PROFILE_MAIN;
    }
    else if (profile & VIDEO_PROFILE_BASELINE)
    {
        return VIDEO_PROFILE_BASELINE;
    }

    return VIDEO_PROFILE_UNKNOWN;
}

static void getMorePropertyValue(video_level_t level, vcodec_level_property_value_t *pvcodec_level_property_value_t)
{
    memset(pvcodec_level_property_value_t, 0, sizeof(vcodec_level_property_value_t));
    switch (level)
    {
        case VIDEO_LEVEL_4: // 1080p
        case VIDEO_LEVEL_4_1:
        {
            pvcodec_level_property_value_t->max_br = 40 * 1024; // in kbps
            pvcodec_level_property_value_t->max_cpb = 0;
            pvcodec_level_property_value_t->max_dpb = 0;
            pvcodec_level_property_value_t->max_fs = 8192;
            pvcodec_level_property_value_t->max_mbps = 245760;
            pvcodec_level_property_value_t->max_smbps = 245760;
        }
        break;
        case VIDEO_LEVEL_3_1:   // 720p
        case VIDEO_LEVEL_3_2:
        {
            pvcodec_level_property_value_t->max_br = 40 * 1024; // in kbps
            pvcodec_level_property_value_t->max_cpb = 0;
            pvcodec_level_property_value_t->max_dpb = 0;
            pvcodec_level_property_value_t->max_fs = 5120;
            pvcodec_level_property_value_t->max_mbps = 216000;
            pvcodec_level_property_value_t->max_smbps = 216000;
        }
        break;
        default:
        {
            pvcodec_level_property_value_t->max_br = 0; // in kbps
            pvcodec_level_property_value_t->max_cpb = 0;
            pvcodec_level_property_value_t->max_dpb = 0;
            pvcodec_level_property_value_t->max_fs = 0;
            pvcodec_level_property_value_t->max_mbps = 0;
            pvcodec_level_property_value_t->max_smbps = 0;
        }
        break;
    }
}

static video_level_t getSensorLevel(uint32_t max_width, uint32_t max_height)
{
    uint32_t i = 0;
    uint32_t count = sizeof(sensorLevel) / sizeof(sensor_level_info_t);
    video_level_t level = VIDEO_LEVEL_0;

    for (; i < count; i++)
    {
        if (max_width * max_height >= sensorLevel[i].width * sensorLevel[i].height)
        {
            //ALOGI("%d, %d, %d, %d, %d", max_width, max_height, sensorLevel[i].width, sensorLevel[i].height, sensorLevel[i].level);
            level = sensorLevel[i].level;
        }
        else
        {
            break;
        }
    }
    ALOGI("[getSensorLevel] resolution = %d x %d, level = %d", max_width, max_height, level);
    return level;
}

static int32_t genH264ParameterSets(VENC_DRV_H264_VIDEO_PROFILE_T eProfile,
                                        VENC_DRV_VIDEO_LEVEL_T eLevel,
                                        uint32_t width,
                                        uint32_t height,
                                        uint32_t prefer_WH_ratio,
                                        uint32_t bitrate,
                                        uint32_t *profile_level_id,
                                        uint8_t *pParameterSets)
{
    int32_t iError = 0;
    const int32_t BUFFER_SIZE = 64;
    char spsBuffer[BUFFER_SIZE], ppsBuffer[BUFFER_SIZE];
    VENC_DRV_PARAM_BS_BUF_T sSPSBitStreamBuf, sPPSBitStreamBuf;
    int32_t u4SPSSize, u4PPSSize;

    ALOGI("[genParameterSets] genH264PS, eProfile(%d), eLevel(%d), width(%d), height(%d), prefer_WH_ratio(%d), bitrate(%d)", eProfile, eLevel, width, height, prefer_WH_ratio, bitrate);

    do {
        sSPSBitStreamBuf.rBSAddr.u4PA = sSPSBitStreamBuf.rBSAddr.u4VA = sSPSBitStreamBuf.u4BSStartVA = (VAL_ULONG_T)spsBuffer;
        sSPSBitStreamBuf.rBSAddr.u4Size = sSPSBitStreamBuf.u4BSSize = BUFFER_SIZE;

        CHECK_IF_BREAK(!H264_EncodeSPS_SW(eProfile, eLevel, width, height, prefer_WH_ratio,
                                    &sSPSBitStreamBuf, &u4SPSSize),
                    iError, "cannot encode SPS header");
        // debug
        printBinary((unsigned char *)sSPSBitStreamBuf.rBSAddr.u4VA, u4SPSSize);

        sPPSBitStreamBuf.rBSAddr.u4PA = sPPSBitStreamBuf.rBSAddr.u4VA = sPPSBitStreamBuf.u4BSStartVA = (VAL_ULONG_T)ppsBuffer;
        sPPSBitStreamBuf.rBSAddr.u4Size = sPPSBitStreamBuf.u4BSSize = BUFFER_SIZE;

        CHECK_IF_BREAK(!H264_EncodePPS_SW(eProfile, &sPPSBitStreamBuf, &u4PPSSize),
                    iError, "cannot encode PPS header");
        // debug
        printBinary((unsigned char *)sPPSBitStreamBuf.rBSAddr.u4VA, u4PPSSize);

        unsigned char *pSPS = (unsigned char *)(sSPSBitStreamBuf.rBSAddr.u4VA + 4);
        *profile_level_id = (pSPS[1] << 16) + (pSPS[2] << 8) + pSPS[3];

        // encode as base64, ignore start code "00000001"

        int32_t pos = encodeBase64((unsigned char *)(sSPSBitStreamBuf.rBSAddr.u4VA + 4), u4SPSSize - 4, pParameterSets);
        //memset(spsBuffer, 0, BUFFER_SIZE);
        //memset(ppsBuffer, 0, BUFFER_SIZE);
        //decodeBase64(pParameterSets, pos, (uint8_t *)spsBuffer);
        //printBinary((unsigned char *)spsBuffer, 16);
        pParameterSets[pos++] = ',';
        pos += encodeBase64((unsigned char *)(sPPSBitStreamBuf.rBSAddr.u4VA + 4), u4PPSSize - 4, pParameterSets + pos);
        pParameterSets[pos] = '\0';
    }
    while(0);

    return iError;
}

static int32_t getH264CodecParam(video_profile_t profile,
                                 video_level_t level,
                                 video_level_t max_level,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t prefer_WH_ratio,
                                 uint32_t bitrate,
                                 video_codec_fmtp_t *pVideo_codec_fmtp_t)
{
    VENC_DRV_H264_VIDEO_PROFILE_T eProfile = (VENC_DRV_H264_VIDEO_PROFILE_T)profile;
    VENC_DRV_VIDEO_LEVEL_T eLevel = (VENC_DRV_VIDEO_LEVEL_T)0;

    uint32_t index = 0;
    for (; index < 26; index++)
    {
        if (VIDEO_LEVEL[1][index] == level)
        {
            eLevel = (VENC_DRV_VIDEO_LEVEL_T)index;
            break;
        }
    }

    //ALOGI("[getVCodecParam] in:format=%d", format);
    ALOGI("[getH264CodecParam] in:profile=%d(%s), level=%d(%s), max_level=%d(%s)",
          profile, toString((video_profile_t)profile),
          level, toString((video_level_t)level),
          max_level, toString((video_level_t)max_level));

    if (prefer_WH_ratio == 1) //  Vertical =1, W<H
    {
        pVideo_codec_fmtp_t->width = height;
        pVideo_codec_fmtp_t->height = width;
    }
    else
    {
        pVideo_codec_fmtp_t->width = width;
        pVideo_codec_fmtp_t->height = height;
    }

    // image info
    pVideo_codec_fmtp_t->image_send[0].valid = 1;
    pVideo_codec_fmtp_t->image_send[0].x = pVideo_codec_fmtp_t->width;
    pVideo_codec_fmtp_t->image_send[0].y = pVideo_codec_fmtp_t->height;
    pVideo_codec_fmtp_t->image_recv[0].valid = 1;
    pVideo_codec_fmtp_t->image_recv[0].x = pVideo_codec_fmtp_t->width;
    pVideo_codec_fmtp_t->image_recv[0].y = pVideo_codec_fmtp_t->height;
    pVideo_codec_fmtp_t->image_send[1].valid = 1;
    pVideo_codec_fmtp_t->image_send[1].x = pVideo_codec_fmtp_t->height;
    pVideo_codec_fmtp_t->image_send[1].y = pVideo_codec_fmtp_t->width;
    pVideo_codec_fmtp_t->image_recv[1].valid = 1;
    pVideo_codec_fmtp_t->image_recv[1].x = pVideo_codec_fmtp_t->height;
    pVideo_codec_fmtp_t->image_recv[1].y = pVideo_codec_fmtp_t->width;

    // Generate H.264 SPS/PPS
    h264_codec_fmtp_t *ph264_codec_fmtp = &pVideo_codec_fmtp_t->codec_fmtp.h264_codec_fmtp;
    genH264ParameterSets(eProfile, eLevel, width, height, prefer_WH_ratio, bitrate,
                            &ph264_codec_fmtp->profile_level_id,
                            ph264_codec_fmtp->sprop_parameter_sets);

    ph264_codec_fmtp->max_recv_level = getLevel(VIDEO_H264, max_level);
    ph264_codec_fmtp->redundant_pic_cap = 0;
    {
        vcodec_level_property_value_t property_value;
        getMorePropertyValue(max_level, &property_value);
        ph264_codec_fmtp->max_mbps = property_value.max_mbps;        //245760
        ph264_codec_fmtp->max_smbps = property_value.max_smbps;      //245760
        ph264_codec_fmtp->max_fs = property_value.max_fs;            //8192
        ph264_codec_fmtp->max_cpb = property_value.max_cpb;          // 2
        ph264_codec_fmtp->max_dpb = property_value.max_dpb;          //21
        ph264_codec_fmtp->max_br = property_value.max_br;            //40 * 1024 * 1024
    }
    ph264_codec_fmtp->max_rcmd_nalu_size = 1024 * 1024;
    ph264_codec_fmtp->sar_understood = 0;
    ph264_codec_fmtp->sar_supported = 0;


    ALOGI("[getH264CodecParam] out:profile_level_id=0x%x, max_recv_level=%d, \
          redundant_pic_cap=0x%x, sprop_parameter_sets=%s, \
          sprop_level_parameter_sets=%s, framesize=%d-%d",
          ph264_codec_fmtp->profile_level_id,
          ph264_codec_fmtp->max_recv_level,
          ph264_codec_fmtp->redundant_pic_cap,
          ph264_codec_fmtp->sprop_parameter_sets,
          ph264_codec_fmtp->sprop_level_parameter_sets,
          pVideo_codec_fmtp_t->width, pVideo_codec_fmtp_t->height);
#ifdef _DEBUG_
    ALOGI("[getH264CodecParam] out:max_mbps=%d",                    ph264_codec_fmtp->max_mbps);
    ALOGI("[getH264CodecParam] out:max_smbps=%d",                   ph264_codec_fmtp->max_smbps);
    ALOGI("[getH264CodecParam] out:max_fs=%d",                      ph264_codec_fmtp->max_fs);
    ALOGI("[getH264CodecParam] out:max_cpb=%d",                     ph264_codec_fmtp->max_cpb);
    ALOGI("[getH264CodecParam] out:max_dpb=%d",                     ph264_codec_fmtp->max_dpb);
    ALOGI("[getH264CodecParam] out:max_br=%d",                      ph264_codec_fmtp->max_br);
    ALOGI("[getH264CodecParam] out:max_rcmd_nalu_size=%d",          ph264_codec_fmtp->max_rcmd_nalu_size);
    ALOGI("[getH264CodecParam] out:sar_understood=%d",              ph264_codec_fmtp->sar_understood);
    ALOGI("[getH264CodecParam] out:sar_supported=%d",               ph264_codec_fmtp->sar_supported);
    ALOGI("[getH264CodecParam] out:sar_understood=%d",              ph264_codec_fmtp->sar_understood);
#endif

    return 0;
}

int32_t searchStartCode(int32_t startpos, VAL_ULONG_T u4BSStartVA, VAL_ULONG_T u4BSSize)
{
    uint32_t pos = startpos;
    char *pBSStartVA = (char*)u4BSStartVA;
    for(; pos < u4BSSize - 5; pos++)
    {
        if (pBSStartVA[pos] == 0x00 &&
            pBSStartVA[pos + 1] == 0x00 &&
            pBSStartVA[pos + 2] == 0x00 &&
            pBSStartVA[pos + 3] == 0x01)
        {
            //VDO_LOGI("%d,%d,%d,%d", pBSStartVA[pos], pBSStartVA[pos + 1], pBSStartVA[pos + 2], pBSStartVA[pos + 3]);
            return pos;
        }
    }
    return 0;
}

void removePreventionByte(uint8_t *profile_tier_level, uint8_t size)
{
    int32_t pos = 0;
    for(; pos < size - 2;)
    {
        if (profile_tier_level[pos] == 0x00 &&
            profile_tier_level[pos + 1] == 0x00 &&
            profile_tier_level[pos + 2] == 0x03)
        {
            memmove(profile_tier_level + pos + 2, profile_tier_level + pos + 3, size - (pos + 3));
            pos += 2;
        }
        else
        {
            pos++;
        }
    }
}

static int32_t genHEVCParameterSets(VENC_DRV_HEVC_VIDEO_PROFILE_T eProfile,
                                        VENC_DRV_VIDEO_LEVEL_T eLevel,
                                        uint32_t width,
                                        uint32_t height,
                                        uint32_t prefer_WH_ratio,
                                        uint32_t bitrate,
                                        hevc_codec_fmtp_t *phevc_codec_fmtp)
{
    int32_t iError = 0;
    const int32_t BUFFER_SIZE = 64;
    char vpsBuffer[BUFFER_SIZE], spsBuffer[BUFFER_SIZE], ppsBuffer[BUFFER_SIZE];
    VENC_DRV_PARAM_BS_BUF_T sVPSBitStreamBuf, sSPSBitStreamBuf, sPPSBitStreamBuf;
    int32_t u4VPSSize, u4SPSSize, u4PPSSize, u4Profile_tier_level_size;

    ALOGI("[genParameterSets] genHEVCPS, eProfile(%d), eLevel(%d), width(%d), height(%d), prefer_WH_ratio(%d), bitrate(%d)", eProfile, eLevel, width, height, prefer_WH_ratio, bitrate);

    do
    {

        // encode VPS
        sVPSBitStreamBuf.rBSAddr.u4PA = sVPSBitStreamBuf.rBSAddr.u4VA = sVPSBitStreamBuf.u4BSStartVA = (VAL_ULONG_T)vpsBuffer;
        sVPSBitStreamBuf.rBSAddr.u4Size = sVPSBitStreamBuf.u4BSSize = BUFFER_SIZE;
        CHECK_IF_BREAK(!HEVC_EncodeVPS_SW(eLevel, &sVPSBitStreamBuf, &u4VPSSize),
                    iError, "cannot encode VPS header");
        // debug
        printBinary((unsigned char *)sVPSBitStreamBuf.rBSAddr.u4VA, u4VPSSize);

        // encode SPS
        sSPSBitStreamBuf.rBSAddr.u4PA = sSPSBitStreamBuf.rBSAddr.u4VA = sSPSBitStreamBuf.u4BSStartVA = (VAL_ULONG_T)spsBuffer;
        sSPSBitStreamBuf.rBSAddr.u4Size = sSPSBitStreamBuf.u4BSSize = BUFFER_SIZE;
        CHECK_IF_BREAK(!HEVC_EncodeSPS_SW(eLevel, width, height, prefer_WH_ratio, &sSPSBitStreamBuf, &u4SPSSize),
                    iError, "cannot encode SPS header");
        // debug
        printBinary((unsigned char *)sSPSBitStreamBuf.rBSAddr.u4VA, u4SPSSize);

        // encode PPS
        sPPSBitStreamBuf.rBSAddr.u4PA = sPPSBitStreamBuf.rBSAddr.u4VA = sPPSBitStreamBuf.u4BSStartVA = (VAL_ULONG_T)ppsBuffer;
        sPPSBitStreamBuf.rBSAddr.u4Size = sPPSBitStreamBuf.u4BSSize = BUFFER_SIZE;
        CHECK_IF_BREAK(!HEVC_EncodePPS_SW(&sPPSBitStreamBuf, &u4PPSSize),
                    iError, "cannot encode PPS header");
        // debug
        printBinary((unsigned char *)sPPSBitStreamBuf.rBSAddr.u4VA, u4PPSSize);

        {
            uint8_t profile_tier_level[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];

            u4Profile_tier_level_size = u4VPSSize - 10;
            if ((VOLTE_MAX_SDP_PARAMETER_SET_LENGTH - 10) < u4Profile_tier_level_size || 0 >= u4Profile_tier_level_size)
            {
                ALOGE("[ERROR] u4Profile_tier_level_size error: %d\n", u4Profile_tier_level_size);
                break;
            }
            else
            {
                memcpy(profile_tier_level, ((unsigned char *)sVPSBitStreamBuf.rBSAddr.u4VA) + 4 + 2 + 4, u4Profile_tier_level_size); //start code + nal type + 4 bytes
            }
            // VPS
            u4VPSSize = encodeBase64((unsigned char *)(sVPSBitStreamBuf.rBSAddr.u4VA + 4), u4VPSSize - 4, phevc_codec_fmtp->sprop_vps);
            phevc_codec_fmtp->sprop_vps[u4VPSSize] = '\0';

            // SPS
            u4SPSSize = encodeBase64((unsigned char *)(sSPSBitStreamBuf.rBSAddr.u4VA + 4), u4SPSSize - 4, phevc_codec_fmtp->sprop_sps);
            phevc_codec_fmtp->sprop_sps[u4SPSSize] = '\0';

            // PPS
            u4PPSSize = encodeBase64((unsigned char *)(sPPSBitStreamBuf.rBSAddr.u4VA + 4), u4PPSSize - 4, phevc_codec_fmtp->sprop_pps);
            phevc_codec_fmtp->sprop_pps[u4PPSSize] = '\0';

            ALOGI("%s\n%s\n%s\n", phevc_codec_fmtp->sprop_vps, phevc_codec_fmtp->sprop_sps, phevc_codec_fmtp->sprop_pps);

            printBinary((unsigned char *)profile_tier_level, u4Profile_tier_level_size);
            removePreventionByte(profile_tier_level, u4Profile_tier_level_size);
            printBinary((unsigned char *)profile_tier_level, u4Profile_tier_level_size);
            phevc_codec_fmtp->profile_space = (profile_tier_level[0] & 0xC0) >> 6;
            phevc_codec_fmtp->tier_flag = (profile_tier_level[0] & 0x20) >> 5;
            phevc_codec_fmtp->profile_id = (profile_tier_level[0] & 0x1F);
            memcpy((unsigned char *)phevc_codec_fmtp->profile_comp_ind, &(profile_tier_level[1]), 4);
            memcpy((unsigned char *)phevc_codec_fmtp->interop_constraints, &(profile_tier_level[5]), 6);
            phevc_codec_fmtp->level_id = profile_tier_level[11];
        }

    }
    while (0);

    return iError;

}

static int32_t getHEVCCodecParam(video_profile_t profile,
                                     video_level_t level,
                                     video_level_t max_level,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t prefer_WH_ratio,
                                     uint32_t bitrate,
                                     video_codec_fmtp_t *pVideo_codec_fmtp_t)
{
    VENC_UNUSED(max_level);
    VENC_DRV_HEVC_VIDEO_PROFILE_T eProfile = (VENC_DRV_HEVC_VIDEO_PROFILE_T)profile;
    VENC_DRV_VIDEO_LEVEL_T eLevel = (VENC_DRV_VIDEO_LEVEL_T)0;

    uint32_t index = 0;
    for (; index < 26; index++)
    {
        if (VIDEO_LEVEL[1][index] == level)
        {
            eLevel = (VENC_DRV_VIDEO_LEVEL_T)index;
            break;
        }
    }

#ifdef _DEBUG_
    //ALOGI("[getVCodecParam] in:format=%d", format);
    ALOGI("[getHEVCCodecParam] in:profile=%d(%s)", profile, toString((video_profile_t)profile));
    ALOGI("[getHEVCCodecParam] in:level=%d(%s)", level, toString((video_level_t)level));
    ALOGI("[getHEVCCodecParam] in:max_level=%d(%s)", max_level, toString((video_level_t)max_level));
#endif

    if (prefer_WH_ratio == 1) //  Vertical =1, W<H
    {
        pVideo_codec_fmtp_t->width = height;
        pVideo_codec_fmtp_t->height = width;
    }
    else
    {
        pVideo_codec_fmtp_t->width = width;
        pVideo_codec_fmtp_t->height = height;
    }

    // image info
    // image info
    pVideo_codec_fmtp_t->image_send[0].valid = 1;
    pVideo_codec_fmtp_t->image_send[0].x = pVideo_codec_fmtp_t->width;
    pVideo_codec_fmtp_t->image_send[0].y = pVideo_codec_fmtp_t->height;
    pVideo_codec_fmtp_t->image_recv[0].valid = 1;
    pVideo_codec_fmtp_t->image_recv[0].x = pVideo_codec_fmtp_t->width;
    pVideo_codec_fmtp_t->image_recv[0].y = pVideo_codec_fmtp_t->height;
    pVideo_codec_fmtp_t->image_send[1].valid = 1;
    pVideo_codec_fmtp_t->image_send[1].x = pVideo_codec_fmtp_t->height;
    pVideo_codec_fmtp_t->image_send[1].y = pVideo_codec_fmtp_t->width;
    pVideo_codec_fmtp_t->image_recv[1].valid = 1;
    pVideo_codec_fmtp_t->image_recv[1].x = pVideo_codec_fmtp_t->height;
    pVideo_codec_fmtp_t->image_recv[1].y = pVideo_codec_fmtp_t->width;


    // Generate HEVC SPS/PPS
    hevc_codec_fmtp_t *phevc_codec_fmtp = &pVideo_codec_fmtp_t->codec_fmtp.hevc_codec_fmtp;

    genHEVCParameterSets(eProfile, eLevel, width, height, prefer_WH_ratio, bitrate, phevc_codec_fmtp);

#ifdef _DEBUG_
    ALOGI("[getHEVCCodecParam] out:profile_space=%d",             phevc_codec_fmtp->profile_space);
    ALOGI("[getHEVCCodecParam] out:profile_id=%d",                phevc_codec_fmtp->profile_id);
    ALOGI("[getHEVCCodecParam] out:tier_flag=%d",                 phevc_codec_fmtp->tier_flag);
    ALOGI("[getHEVCCodecParam] out:level_id=%d",                  phevc_codec_fmtp->level_id);
    ALOGI("[getHEVCCodecParam] out:interop_constraints=0x%02x%02x%02x%02x%02x%02x",
                                            phevc_codec_fmtp->interop_constraints[0], phevc_codec_fmtp->interop_constraints[1],
                                            phevc_codec_fmtp->interop_constraints[2], phevc_codec_fmtp->interop_constraints[3],
                                            phevc_codec_fmtp->interop_constraints[4], phevc_codec_fmtp->interop_constraints[5]);
    ALOGI("[getHEVCCodecParam] out:profile_comp_ind=0x%02x%02x%02x%02x",
                                            phevc_codec_fmtp->profile_comp_ind[0], phevc_codec_fmtp->profile_comp_ind[1],
                                            phevc_codec_fmtp->profile_comp_ind[2], phevc_codec_fmtp->profile_comp_ind[3]);
#endif

    return 0;
}

static int32_t getVCodecParam(video_format_t format,
                              video_profile_t profile,
                              video_level_t level,
                              video_level_t max_level,
                              int fps,
                              uint32_t width,
                              uint32_t height,
                              uint32_t prefer_WH_ratio,
                              uint32_t bitrate,
                              video_codec_fmtp_t *pVideo_codec_fmtp_t)
{
    memset(pVideo_codec_fmtp_t, 0, sizeof(video_codec_fmtp_t));

    pVideo_codec_fmtp_t->format = format;
    pVideo_codec_fmtp_t->fps = fps;

    if (format == VIDEO_H264)
    {
        return getH264CodecParam(profile, level, max_level, width, height, prefer_WH_ratio, bitrate, pVideo_codec_fmtp_t);
    }
    else if (format == VIDEO_HEVC)
    {
        return getHEVCCodecParam(profile, level, max_level, width, height, prefer_WH_ratio, bitrate, pVideo_codec_fmtp_t);
    }
    return 0;
}

static int32_t getVDecCapability(video_format_t format, vdec_cap_t *pvdec_cap_t)
{
    return android::getVDecCapability_1(format, pvdec_cap_t);
}

static int32_t getVEncCapability(video_format_t format, venc_cap_t *pvenc_cap_t)
{
    return android::getVEncCapability_1(format, pvenc_cap_t);
}

static bool isVEncCapSupported(video_format_t format, uint32_t profile_level_id, sensor_resolution_t *psensor_resolution_t, venc_cap_t *pvenc_cap_t)
{
    bool isSupported = false;

    if (format == VIDEO_H264)
    {
        video_profile_t profile = getProfile(format, profile_level_id >> 16);
        video_level_t level = getLevel(format, (uint32_t)(profile_level_id & 0xFF));
        video_level_t sensor_level = getSensorLevel(psensor_resolution_t->sensor_max_width, psensor_resolution_t->sensor_max_height);

        if (!(profile & pvenc_cap_t->profile)) { isSupported = false; }
        if (level > sensor_level || level > pvenc_cap_t->max_level) { isSupported = false; }
    }

    ALOGI("[isVEncCapSupported] out:%s", isSupported == true ? "true" : "false");
    return isSupported;
}

static int32_t getDefaultCapability(vdec_cap_t *pvdec_cap_t, venc_cap_t *pvenc_cap_t, video_default_cap_t *pvideo_default_cap_t)
{
    video_profile_t profile = (pvdec_cap_t->profile < pvenc_cap_t->profile) ? pvdec_cap_t->profile : pvenc_cap_t->profile;
    pvideo_default_cap_t->profile = getMaxProfile(profile);
    pvideo_default_cap_t->level = (pvenc_cap_t->max_level < pvdec_cap_t->max_level) ? pvenc_cap_t->max_level : pvdec_cap_t->max_level;

    ALOGI("[getDefaultCapability] out:default_profile=%d(%s), default_level=%d(%s)",
          pvideo_default_cap_t->profile, toString(pvideo_default_cap_t->profile),
          pvideo_default_cap_t->level, toString(pvideo_default_cap_t->level));
    return 0;
}

static void copyCodecLevelftmp(video_format_t format, video_codec_fmtp_t *codeccap, video_codec_level_fmtp_t *codeclevelcap)
{
    codeclevelcap->format = codeccap->format;
    codeclevelcap->fps = codeccap->fps;
    codeclevelcap->width = codeccap->width;
    codeclevelcap->height = codeccap->height;

    if (format == VIDEO_H264)
    {
        h264_codec_level_fmtp_t *ph264_codec_level_fmtp = &codeclevelcap->codec_level_fmtp.h264_codec_level_fmtp;
        h264_codec_fmtp_t *ph264_codec_fmtp = &codeccap->codec_fmtp.h264_codec_fmtp;
        ph264_codec_level_fmtp->profile_level_id = ph264_codec_fmtp->profile_level_id;
        snprintf((char *)ph264_codec_level_fmtp->sprop_parameter_sets, sizeof(ph264_codec_level_fmtp->sprop_parameter_sets),"%s", (char*)ph264_codec_fmtp->sprop_parameter_sets);
    }
    else if (format == VIDEO_HEVC)
    {
        hevc_codec_level_fmtp_t *phevc_codec_level_fmtp_t = &codeclevelcap->codec_level_fmtp.hevc_codec_level_fmtp;
        hevc_codec_fmtp_t *phevc_codec_fmtp = &codeccap->codec_fmtp.hevc_codec_fmtp;
        phevc_codec_level_fmtp_t->profile_space = phevc_codec_fmtp->profile_space;
        phevc_codec_level_fmtp_t->tier_flag = phevc_codec_fmtp->tier_flag;
        phevc_codec_level_fmtp_t->profile_id = phevc_codec_fmtp->profile_id;
        phevc_codec_level_fmtp_t->level_id = phevc_codec_fmtp->level_id;
        memcpy(phevc_codec_level_fmtp_t->interop_constraints, phevc_codec_fmtp->interop_constraints, 6);
        memcpy(phevc_codec_level_fmtp_t->profile_comp_ind, phevc_codec_fmtp->profile_comp_ind, 4);
        phevc_codec_level_fmtp_t->sprop_sub_layer_id = phevc_codec_fmtp->sprop_sub_layer_id;
        phevc_codec_level_fmtp_t->recv_sub_layer_id = phevc_codec_fmtp->recv_sub_layer_id;
        phevc_codec_level_fmtp_t->max_recv_level_id = phevc_codec_fmtp->max_recv_level_id;
        snprintf((char *)phevc_codec_level_fmtp_t->sprop_vps, sizeof(phevc_codec_level_fmtp_t->sprop_vps),"%s", (char*)phevc_codec_fmtp->sprop_vps);
        snprintf((char *)phevc_codec_level_fmtp_t->sprop_sps, sizeof(phevc_codec_level_fmtp_t->sprop_sps),"%s", (char*)phevc_codec_fmtp->sprop_vps);
        snprintf((char *)phevc_codec_level_fmtp_t->sprop_pps, sizeof(phevc_codec_level_fmtp_t->sprop_pps),"%s", (char*)phevc_codec_fmtp->sprop_vps);
        snprintf((char *)phevc_codec_level_fmtp_t->sprop_sei, sizeof(phevc_codec_level_fmtp_t->sprop_sei),"%s", (char*)phevc_codec_fmtp->sprop_vps);
    }
}

static video_media_profile_t *getMediaProfileEntryBySIM(uint32_t opID, video_format_t format, video_profile_t profile, video_level_t level)
{
    MediaProfileList_t *pMediaProfileList_t = getMediaProfileListInst(opID);
    return getMediaProfileEntry(pMediaProfileList_t, format, profile, level);
}

static int32_t getDefaultMediaProfileBySensorLevel(uint32_t opID, video_level_t sensor_level,
                                                   Vector<video_media_profile_t> &targetMediaProfile)
{
    // find the max spec.
    MediaProfileList_t *pMediaProfileList_t = getMediaProfileListInst(opID);
    uint32_t fgFormatGot = 0;
    uint32_t index = pMediaProfileList_t->mDefault_quality;

    for (; index <= VIDEO_QUALITY_END; index <<= 1)
    {
        video_quality_t target_quality = (video_quality_t)index;
        uint32_t profile_count = 0;
        getMediaProfileByQuality(opID, target_quality, &profile_count, NULL);

        video_media_profile_t *prvideo_media_profile_t = new video_media_profile_t[profile_count];
        getMediaProfileByQuality(opID, target_quality, &profile_count, prvideo_media_profile_t);

        for (uint32_t count = 0; count < profile_count; count++)
        {
            video_format_t format = prvideo_media_profile_t[count].format;
            video_level_t level = prvideo_media_profile_t[count].level;
            if (sensor_level >= level)
            {
                if ((fgFormatGot & format) == 0)
                {
                    targetMediaProfile.push_back(prvideo_media_profile_t[count]);
                    fgFormatGot |= format;
                }
            }
        }
        delete [] prvideo_media_profile_t;
    }
    return 0;
}

/**************************************************************************************/
/* Interface                                                                                                                                                          */
/**************************************************************************************/
int32_t getMediaProfile(video_format_t format, uint32_t profile, uint32_t level, video_media_profile_t *pmedia_profile)
{
    return getMediaProfile(0, format, profile, level, pmedia_profile);
}

int32_t getCodecCapabiltiy(uint32_t quality,
                           sensor_resolution_t *resolution,
                           uint32_t *capNumbers,
                           video_codec_fmtp_t *codeccap)
{
    return getCodecCapabiltiy(0, quality, resolution, capNumbers, codeccap);
}

int32_t getAvailableBitrateInfo(uint32_t *pcount, video_media_bitrate_t *pvideo_media_bitrate)
{
    return getAvailableBitrateInfo(0, pcount, pvideo_media_bitrate);
}

int32_t getAdaptativeFrameRateInfo(video_format_t format, uint32_t profile, uint32_t level,
                                                 uint32_t expected_bitrate,
                                                 uint32_t *target_bitrate, uint32_t *target_framerate)
{
    return getAdaptativeFrameRateInfo(0, format, profile, level, expected_bitrate, target_bitrate, target_framerate);
}

int32_t getCodecLevelParameterSets(uint32_t quality,
                                            sensor_resolution_t *resolution,
                                            video_format_t informat,
                                            uint32_t *levelCapNumbers,
                                            video_codec_level_fmtp_t *codeclevelcap)
{
    return getCodecLevelParameterSets(0, quality, resolution, informat, levelCapNumbers, codeclevelcap);
}

int32_t getMediaProfile(uint32_t opID, video_format_t format, uint32_t profile, uint32_t level, video_media_profile_t *pmedia_profile)
{
    video_media_profile_t *pMPEntry = getMediaProfileEntryBySIM(opID, format, getProfile(format, profile), getLevel(format, level));
    if (pMPEntry)
    {
        *pmedia_profile = *pMPEntry;
        return 0;
    }
    return -1;
}

int32_t getMediaProfileByQuality(uint32_t opID, video_quality_t quality, uint32_t *pprofile_count, video_media_profile_t *pmedia_profile)
{
    MediaProfileList_t *pMediaProfileList_t = getMediaProfileListInst(opID);
    video_property_t* pvideo_property_t = getVideoProperty();

    if (quality == VIDEO_QUALITY_DEFAULT)
    {
        quality = pMediaProfileList_t->mDefault_quality;
    }

    *pprofile_count = 0;

    for (Vector<video_format_t>::iterator it = pMediaProfileList_t->mVideoFormatList.begin() ; it != pMediaProfileList_t->mVideoFormatList.end(); ++it)
    {
        if (pMediaProfileList_t->mQualityMediaProfile[(*it)].indexOfKey(quality) >= 0)
        {
            if (pmedia_profile)
            {
                pmedia_profile[*pprofile_count] = pMediaProfileList_t->mQualityMediaProfile[(*it)].valueFor(quality);
            }
            *pprofile_count += 1;
        }
    }
    return 0;
}

int32_t getCodecCapabiltiy(uint32_t opID, uint32_t quality,
                           sensor_resolution_t *resolution,
                           uint32_t *capNumbers,
                           video_codec_fmtp_t *codeccap)
{
    Vector<video_media_profile_t> targetMediaProfile;
    video_level_t sensor_level = getSensorLevel(resolution->sensor_max_width, resolution->sensor_max_height);

    if (quality == VIDEO_QUALITY_DEFAULT)
    {
        getDefaultMediaProfileBySensorLevel(opID, sensor_level, targetMediaProfile);
    }
    else
    {
        video_quality_t target_quality = (video_quality_t)quality;
        uint32_t profile_count = 0;
        getMediaProfileByQuality(opID, target_quality, &profile_count, NULL);

        video_media_profile_t *prvideo_media_profile_t = new video_media_profile_t[profile_count];
        getMediaProfileByQuality(opID, target_quality, &profile_count, prvideo_media_profile_t);

        for (uint32_t count = 0; count < profile_count; count++)
        {
            video_format_t format = prvideo_media_profile_t[count].format;
            video_level_t level = prvideo_media_profile_t[count].level;
            if (sensor_level >= level)
            {
                targetMediaProfile.push_back(prvideo_media_profile_t[count]);
            }
        }
        delete [] prvideo_media_profile_t;
    }

    *capNumbers = 0;

    // Packetization mode = 0
    for (Vector<video_media_profile_t>::iterator it = targetMediaProfile.begin() ; it != targetMediaProfile.end(); ++it)
    {
        if (codeccap)
        {
            video_property_t* pvideo_property_t = getVideoProperty();
            video_profile_t profile = pvideo_property_t->profile != VIDEO_PROFILE_UNKNOWN?pvideo_property_t->profile:(*it).profile;
            video_level_t level = pvideo_property_t->level != VIDEO_LEVEL_UNKNOWN?pvideo_property_t->level:(*it).level;
            uint32_t fps = pvideo_property_t->fps != 0?pvideo_property_t->fps:(*it).fps;

            getVCodecParam((*it).format,
                           profile,
                           level,
                           level,
                           fps,
                           (*it).width,
                           (*it).height,
                           resolution->prefer_WH_ratio,
                           (*it).bitrate,
                           &codeccap[*capNumbers]);

            if ((*it).format == VIDEO_H264)
            {
                codeccap[*capNumbers].codec_fmtp.h264_codec_fmtp.packetization_mode = 1;
            }
            else if ((*it).format == VIDEO_HEVC)
            {
                codeccap[*capNumbers].codec_fmtp.hevc_codec_fmtp.packetization_mode = 1;
            }
        }
        *capNumbers += 1;
    }

    // Packetization mode = 1, sw codec
#if 0
    if (codeccap)
    {
        getVCodecParam(VIDEO_H264,
                       VIDEO_PROFILE_BASELINE,
                       VIDEO_LEVEL_1_2,
                       VIDEO_LEVEL_1_2,
                       &codeccap[*capNumbers]);
        codeccap[*capNumbers].codec_fmtp.h264_codec_fmtp.packetization_mode = 0;
    }
    *capNumbers += 1;
#endif
    return 0;
}

int32_t getAvailableBitrateInfo(uint32_t opID, uint32_t *pcount,
                                        video_media_bitrate_t *pvideo_media_bitrate)
{
    MediaProfileList_t *pMediaProfileList_t = getMediaProfileListInst(opID);
    video_property_t* pvideo_property_t = getVideoProperty();
    int32_t u4Index = 0;

    *pcount = 0;
    for (Vector<video_format_t>::iterator it = pMediaProfileList_t->mVideoFormatList.begin() ; it != pMediaProfileList_t->mVideoFormatList.end(); ++it)
    {
        Vector<video_media_profile_t *> *pMediaProfile = &pMediaProfileList_t->mMediaProfile[(*it)];

        *pcount += pMediaProfile->size();
        if (pvideo_media_bitrate)
        {
            for (Vector<video_media_profile_t *>::iterator it2 = pMediaProfile->begin() ; it2 != pMediaProfile->end(); ++it2)
            {
                video_media_profile_t *pMPEntry = (*it2);
                pvideo_media_bitrate[u4Index].format = pMPEntry->format;
                pvideo_media_bitrate[u4Index].profile = getProfile(pMPEntry->format, pMPEntry->profile);
                pvideo_media_bitrate[u4Index].level = getLevel(pMPEntry->format, pMPEntry->level);
                pvideo_media_bitrate[u4Index].bitrate = pMPEntry->bitrate / 1000;  // in kbps
                pvideo_media_bitrate[u4Index].minbitrate = pMPEntry->minbitrate / 1000; // in kbps
                u4Index++;
            }
        }
    }
    return 0;
}

uint32_t getResolutionChangeCapabiltiy(video_format_t format)
{
    uint32_t u4Capability = 0;

    switch(format)
    {
        case VIDEO_H264:
            u4Capability = getH264ResolutionChangeCapabiltiy();
            break;
        case VIDEO_HEVC:
            u4Capability = getHEVCResolutionChangeCapabiltiy();
            break;
        default:
            ALOGE("[ERROR] getResolutionChangeCapabiltiy error format");
            break;
    }

    return u4Capability;
}

uint32_t getAdaptiveDropFrameCapabiltiy(video_format_t format)
{
    uint32_t u4Capability = 0;

    switch(format)
    {
        case VIDEO_H264:
            u4Capability = getH264AdaptiveDropFrameCapabiltiy();
            break;
        case VIDEO_HEVC:
            u4Capability = getHEVCAdaptiveDropFrameCapabiltiy();
            break;
        default:
            ALOGE("[ERROR] getAdaptiveDropFrameCapabiltiy2 error format");
            break;
    }

    return u4Capability;
}

int32_t getAdaptativeFrameRateInfo(uint32_t opID, video_format_t format, uint32_t profile, uint32_t level,
                                   uint32_t expected_bitrate,
                                   uint32_t *target_bitrate, uint32_t *target_framerate)
{
    MediaProfileList_t *pMediaProfileList_t = getMediaProfileListInst(opID);
    Vector<video_media_profile_t *> *pMediaProfile = &pMediaProfileList_t->mMediaProfile[format];
    uint32_t minbitrate = 0xFFFFFFFF;
    uint32_t initial_frame_rate = 0xFFFFFFFF;

    for (Vector<video_media_profile_t *>::iterator it = pMediaProfile->begin() ; it != pMediaProfile->end(); ++it)
    {
        video_media_profile_t *pMPEntry = (*it);
        if (profile == getProfile(pMPEntry->format, pMPEntry->profile) && level == getLevel(pMPEntry->format, pMPEntry->level))
        {
            minbitrate = pMPEntry->minbitrate;
            initial_frame_rate = pMPEntry->fps;
            break;
        }
    }

    if (format == VIDEO_H264)
        getH264AdaptativeEncodeInfo(expected_bitrate, target_bitrate, minbitrate, initial_frame_rate, target_framerate);
    else if (format == VIDEO_HEVC)
        getHEVCAdaptativeEncodeInfo(expected_bitrate, target_bitrate, minbitrate, initial_frame_rate, target_framerate);
    else
        ALOGD("[getAdaptativeFrameRateInfo] error format: %d", format);

    return 0;
}

int32_t getAdaptativeFrameRateInfo(uint32_t opID, video_format_t format, uint32_t profile, uint32_t level,
                                   uint32_t expected_bitrate,
                                   uint32_t *target_bitrate, uint32_t *target_framerate,
                                   uint32_t width, uint32_t height,
                                   uint32_t *target_width, uint32_t *target_height)
{
    uint32_t minbitrate = 0xFFFFFFFF;
    uint32_t initial_frame_rate = 0xFFFFFFFF;

    *target_width = 0;
    *target_height = 0;

    char value[PROPERTY_VALUE_MAX];
    uint32_t AdaptiveResolution = 0;
    property_get("persist.vendor.vt.adaptiveRes", value, "2");
    AdaptiveResolution = atoi(value);


    if (AdaptiveResolution == 0 || format != VIDEO_H264 )
       return getAdaptativeFrameRateInfo(opID, format, profile, level, expected_bitrate, target_bitrate, target_framerate);
    else
    {
        MediaProfileList_t *pMediaProfileList_t = getMediaProfileListInst(opID);
        Vector<video_media_profile_t *> *pMediaProfile = &pMediaProfileList_t->mMediaProfile[format];
        uint32_t resolution = Resolution_OTHERS;

        ALOGD("[getAdaptativeFrameRateInfo] expected_bitrate: %d, resolution: %d, %d", expected_bitrate, width, height);


        for (Vector<video_media_profile_t *>::iterator it = pMediaProfile->begin() ; it != pMediaProfile->end(); ++it)
        {
            video_media_profile_t *pMPEntry = (*it);
            if (profile == getProfile(pMPEntry->format, pMPEntry->profile) && level == getLevel(pMPEntry->format, pMPEntry->level))
            {
                minbitrate = pMPEntry->minbitrate;
                initial_frame_rate = pMPEntry->fps;

                if (pMPEntry->width == 320 && pMPEntry->height == 240)
                    resolution = Resolution_QVGA;
                else if (pMPEntry->width == 640 && pMPEntry->height == 480)
                    resolution = Resolution_VGA;

                ALOGD("[getAdaptativeFrameRateInfo] init resolution: %d, %d, %d", pMPEntry->width, pMPEntry->height, resolution);

                break;
            }
        }
        getH264AdaptativeEncodeInfo(expected_bitrate, target_bitrate, minbitrate, initial_frame_rate, target_framerate, AdaptiveResolution, resolution, width, height, target_width, target_height);
    }

    ALOGD("[getAdaptativeFrameRateInfo] target framerate: %d, resolution: %d, %d", *target_framerate, *target_width, *target_height);

    return 0;
}


int32_t getCodecLevelParameterSets(uint32_t opID, uint32_t quality,
                                            sensor_resolution_t *resolution,
                                            video_format_t informat,
                                            uint32_t *levelCapNumbers,
                                            video_codec_level_fmtp_t *codeclevelcap)
{
    MediaProfileList_t *pMediaProfileList_t = getMediaProfileListInst(opID);
    Vector<video_media_profile_t> targetMediaProfile;
    video_profile_t vprofile = VIDEO_PROFILE_UNKNOWN;
    video_level_t vlevel = VIDEO_LEVEL_UNKNOWN;

    ALOGD("[getCodecLevelParameterSets] quality: %d opID %d\n", quality , opID);


    if (quality == VIDEO_QUALITY_DEFAULT)
    {
        video_quality_t target_quality = (video_quality_t)pMediaProfileList_t->mDefault_quality;

        if (pMediaProfileList_t->mQualityMediaProfile[informat].indexOfKey(target_quality) >= 0)
        {
            video_media_profile_t rvideo_media_profile = pMediaProfileList_t->mQualityMediaProfile[informat].valueFor(target_quality);

            vprofile = rvideo_media_profile.profile;
            vlevel = rvideo_media_profile.level;
            ALOGD("[getCodecLevelParameterSets] quality: %d, vprofile:%s, vlevel:%s",
                            target_quality,
                            toString(vprofile),
                            toString(vlevel));
        }
    }
    else
    {

        video_quality_t target_quality = (video_quality_t)quality;

        if (pMediaProfileList_t->mQualityMediaProfile[informat].indexOfKey(target_quality) >= 0)
        {
            video_media_profile_t rvideo_media_profile = pMediaProfileList_t->mQualityMediaProfile[informat].valueFor(target_quality);

            vprofile = rvideo_media_profile.profile;
            vlevel = rvideo_media_profile.level;
            ALOGD("[getCodecLevelParameterSets] quality: %d, vprofile:%s, vlevel:%s",
                            target_quality,
                            toString(vprofile),
                            toString(vlevel));
        }

    }

    // Search mediaprofile entry
    Vector<video_media_profile_t *> *pMediaProfile = &pMediaProfileList_t->mMediaProfile[informat];
    for (Vector<video_media_profile_t *>::iterator it = pMediaProfile->begin() ; it != pMediaProfile->end(); ++it)
    {
        video_media_profile_t *pMPEntry = (*it);
        if (pMPEntry->profile == vprofile && pMPEntry->level <= vlevel)
        {
            targetMediaProfile.push_back(*pMPEntry);
        }
    }

    *levelCapNumbers = 0;

    // Packetization mode = 0
    video_codec_fmtp_t *pVideo_codec_fmtp_t = new video_codec_fmtp_t;
    for (Vector<video_media_profile_t>::iterator it = targetMediaProfile.begin() ; it != targetMediaProfile.end(); ++it)
    {
        if (codeclevelcap)
        {
            video_profile_t profile = (*it).profile;
            video_level_t level = (*it).level;
            uint32_t fps = (*it).fps;

            getVCodecParam((*it).format,
                           profile,
                           level,
                           level,
                           fps,
                           (*it).width,
                           (*it).height,
                           resolution->prefer_WH_ratio,
                           (*it).bitrate,
                           pVideo_codec_fmtp_t);

            copyCodecLevelftmp((*it).format, pVideo_codec_fmtp_t, &codeclevelcap[*levelCapNumbers]);
        }
        *levelCapNumbers += 1;
    }
    delete pVideo_codec_fmtp_t;

    return 0;
}

int32_t configLowPowerMode(low_power_mode_t lpMode, int32_t enable)
{
    SetcEncodeconfig(CONFIG_VCODECCAPTYPE_GED_VILTE_MODE, enable);

    return 0;
}

namespace android {
using android::status_t;

struct MessageList : public RefBase {
    MessageList() {
    }
    virtual ~MessageList() {
    }
    std::list<sp<AMessage> > &getList() { return mList; }
private:
    std::list<sp<AMessage> > mList;

    DISALLOW_EVIL_CONSTRUCTORS(MessageList);
};

struct CodecObserver : public BnOMXObserver {
    CodecObserver() {}

    void setNotificationMessage(const sp<AMessage> &msg) {
        mNotify = msg;
    }

    // from IOMXObserver
    virtual void onMessages(const std::list<omx_message> &messages) {
        if (messages.empty()) {
            return;
        }

        sp<AMessage> notify = mNotify->dup();
        sp<MessageList> msgList = new MessageList();
        for (std::list<omx_message>::const_iterator it = messages.cbegin();
              it != messages.cend(); ++it) {
            const omx_message &omx_msg = *it;

            sp<AMessage> msg = new AMessage;
            msg->setInt32("type", omx_msg.type);
            switch (omx_msg.type) {
                case omx_message::EVENT:
                {
                    msg->setInt32("event", omx_msg.u.event_data.event);
                    msg->setInt32("data1", omx_msg.u.event_data.data1);
                    msg->setInt32("data2", omx_msg.u.event_data.data2);
                    break;
                }

                case omx_message::EMPTY_BUFFER_DONE:
                {
                    msg->setInt32("buffer", omx_msg.u.buffer_data.buffer);
                    msg->setInt32("fence_fd", omx_msg.fenceFd);
                    break;
                }

                case omx_message::FILL_BUFFER_DONE:
                {
                    msg->setInt32(
                            "buffer", omx_msg.u.extended_buffer_data.buffer);
                    msg->setInt32(
                            "range_offset",
                            omx_msg.u.extended_buffer_data.range_offset);
                    msg->setInt32(
                            "range_length",
                            omx_msg.u.extended_buffer_data.range_length);
                    msg->setInt32(
                            "flags",
                            omx_msg.u.extended_buffer_data.flags);
                    msg->setInt64(
                            "timestamp",
                            omx_msg.u.extended_buffer_data.timestamp);
                    msg->setInt32(
                            "fence_fd", omx_msg.fenceFd);
                    break;
                }

                case omx_message::FRAME_RENDERED:
                {
                    msg->setInt64(
                            "media_time_us", omx_msg.u.render_data.timestamp);
                    msg->setInt64(
                            "system_nano", omx_msg.u.render_data.nanoTime);
                    break;
                }

                default:
                    ALOGE("Unrecognized message type: %d", omx_msg.type);
                    break;
            }
            msgList->getList().push_back(msg);
        }
        notify->setObject("messages", msgList);
        notify->post();
    }

protected:
    virtual ~CodecObserver() {}

private:
    sp<AMessage> mNotify;

    DISALLOW_EVIL_CONSTRUCTORS(CodecObserver);
};

enum {
    kMaxIndicesToCheck = 128, // used when enumerating supported formats and profiles
};

static int32_t getVEncCapability_1(video_format_t format, venc_cap_t *pvenc_cap_t)
{
    //const char* name = "OMX.MTK.VIDEO.ENCODER.AVC"; // "OMX.MTK.VIDEO.ENCODER.HEVC" "OMX.MTK.VIDEO.DECODER.HEVC"  "OMX.MTK.VIDEO.DECODER.AVC"
    //const char* mime = "video/avc"; // mime = "video/hevc";
    //const char *role = "video_encoder.avc"; // "video_decoder.avc" "video_encoder.hevc"  "video_decoder.hevc"

    bool isEncoder = 1;
    OMXClient client;
    const char* owner = "default";

    status_t err = client.connect(owner);
    if (err != OK) {
        return -1;
    }

    sp<IOMX> omx = client.interface();
    sp<CodecObserver> observer = new CodecObserver;
    sp<IOMXNode> omxNode;

    if (format == VIDEO_H264)
    {
        const char* name = "OMX.MTK.VIDEO.ENCODER.AVC";
        const char *role = "video_encoder.avc";

        err = omx->allocateNode(name, observer, &omxNode);
        if (err != OK) {
            client.disconnect();
            return -1;
        }

        err = SetComponentRole(omxNode, role);
        if (err != OK) {
            omxNode->freeNode();
            client.disconnect();
            return -1;
        }
    }
    else if (format == VIDEO_HEVC)
    {
        const char* name = "OMX.MTK.VIDEO.ENCODER.HEVC";
        const char *role = "video_encoder.hevc";

        err = omx->allocateNode(name, observer, &omxNode);
        if (err != OK) {
            client.disconnect();
            return -1;
        }

        err = SetComponentRole(omxNode, role);
        if (err != OK) {
            omxNode->freeNode();
            client.disconnect();
            return -1;
        }
    }
    if (omxNode == NULL)
    {
        return -1;
    }
    OMX_VIDEO_PARAM_PROFILELEVELTYPE param;
    InitOMXParams(&param);
    param.nPortIndex = isEncoder ? 1 : 0;   //isEncoder ? kPortIndexOutput : kPortIndexInput

    for (OMX_U32 index = 0; index <= kMaxIndicesToCheck; ++index) {
        param.nProfileIndex = index;
        status_t err = omxNode->getParameter(
                OMX_IndexParamVideoProfileLevelQuerySupported,
                &param, sizeof(param));
        if (err != OK) {
           break;
        }

        //ALOGD("Terry eProfile = %x, eLevel = %x", param.eProfile, param.eLevel);
    }

    if (format == VIDEO_H264)
    {
        pvenc_cap_t->profile = getProfilefromOMXAVC(param.eProfile);
        pvenc_cap_t->max_level = getLevelfromOMXAVC(param.eLevel);
    }
    else if (format == VIDEO_HEVC)
    {
        pvenc_cap_t->profile = getProfilefromOMXHEVC(param.eProfile);
        pvenc_cap_t->max_level = getLevelfromOMXHEVC(param.eLevel);
    }

    //ALOGD("Terry eProfile = %d, max_Level = %d", pvenc_cap_t->profile, pvenc_cap_t->max_level);

    omxNode->freeNode();
    client.disconnect();

    return 0;
}

static int32_t getVDecCapability_1(video_format_t format, vdec_cap_t *pvdec_cap_t)
{
    //const char* name = "OMX.MTK.VIDEO.ENCODER.AVC"; // "OMX.MTK.VIDEO.ENCODER.HEVC" "OMX.MTK.VIDEO.DECODER.HEVC"  "OMX.MTK.VIDEO.DECODER.AVC"
    //const char* mime = "video/avc"; // mime = "video/hevc";
    //const char *role = "video_encoder.avc"; // "video_decoder.avc" "video_encoder.hevc"  "video_decoder.hevc"

    bool isEncoder = 0;
    OMXClient client;
    const char* owner = "default";

    status_t err = client.connect(owner);
    if (err != OK) {
        return -1;
    }

    sp<IOMX> omx = client.interface();
    sp<CodecObserver> observer = new CodecObserver;
    sp<IOMXNode> omxNode;

    if (format == VIDEO_H264)
    {
        const char* name = "OMX.MTK.VIDEO.DECODER.AVC";
        const char *role = "video_decoder.avc";

        err = omx->allocateNode(name, observer, &omxNode);
        if (err != OK) {
            client.disconnect();
            return -1;
        }

        err = SetComponentRole(omxNode, role);
        if (err != OK) {
            omxNode->freeNode();
            client.disconnect();
            return -1;
        }
    }
    else if (format == VIDEO_HEVC)
    {
        const char* name = "OMX.MTK.VIDEO.DECODER.HEVC";
        const char *role = "video_decoder.hevc";

        err = omx->allocateNode(name, observer, &omxNode);
        if (err != OK) {
            client.disconnect();
            return -1;
        }

        err = SetComponentRole(omxNode, role);
        if (err != OK) {
            omxNode->freeNode();
            client.disconnect();
            return -1;
        }
    }
    if (omxNode == NULL)
    {
        return -1;
    }
    OMX_VIDEO_PARAM_PROFILELEVELTYPE param;
    InitOMXParams(&param);
    param.nPortIndex = isEncoder ? 1 : 0;   //isEncoder ? kPortIndexOutput : kPortIndexInput

    for (OMX_U32 index = 0; index <= kMaxIndicesToCheck; ++index) {
        param.nProfileIndex = index;
        status_t err = omxNode->getParameter(
                OMX_IndexParamVideoProfileLevelQuerySupported,
                &param, sizeof(param));
        if (err != OK) {
           break;
        }

        //ALOGD("Terry eProfile = %x, eLevel = %x", param.eProfile, param.eLevel);
    }

    if (format == VIDEO_H264)
    {
        pvdec_cap_t->profile = VIDEO_PROFILE_HIGH; // getProfilefromOMXAVC(param.eProfile)
        pvdec_cap_t->max_level = getLevelfromOMXAVC(param.eLevel);
    }
    else if (format == VIDEO_HEVC)
    {
        pvdec_cap_t->profile = VIDEO_PROFILE_HIGH; // getProfilefromOMXHEVC(param.eProfile)
        pvdec_cap_t->max_level = getLevelfromOMXHEVC(param.eLevel);
    }

    //ALOGD("Terry eProfile = %d, max_Level = %d", pvdec_cap_t->profile, pvdec_cap_t->max_level);

    omxNode->freeNode();
    client.disconnect();

    return 0;
}
}

#else
int32_t getMediaProfile(video_format_t format, uint32_t profile, uint32_t level, video_media_profile_t *pmedia_profile)
{
    return 0;
}

int32_t getCodecCapabiltiy(uint32_t quality,
                           sensor_resolution_t *resolution,
                           uint32_t *capNumbers,
                           video_codec_fmtp_t *codeccap)
{
    return 0;
}

int32_t getAvailableBitrateInfo(uint32_t *pcount,
                                        video_media_bitrate_t *pvideo_media_bitrate)
{
    return 0;
}

int32_t getAdaptativeFrameRateInfo(video_format_t format, uint32_t profile, uint32_t level,
                                   uint32_t expected_bitrate,
                                   uint32_t *target_bitrate, uint32_t *target_framerate)
{
    return 0;
}

int32_t getCodecLevelParameterSets(uint32_t quality,
                                            sensor_resolution_t *resolution,
                                            video_format_t informat,
                                            uint32_t *levelCapNumbers,
                                            video_codec_level_fmtp_t *codeclevelcap)
{
    return 0;
}

int32_t getMediaProfile(uint32_t opID, video_format_t format,
                                uint32_t profile,
                                uint32_t level,
                                video_media_profile_t *pmedia_profile)
{
    return 0;
}

int32_t getMediaProfileByQuality(uint32_t opID, video_quality_t quality,
                                        uint32_t *pprofile_count,
                                        video_media_profile_t *pmedia_profile)
{
    return 0;
}

int32_t getCodecCapabiltiy(uint32_t opID, uint32_t quality,
                           sensor_resolution_t *resolution,
                           uint32_t *capNumbers,
                           video_codec_fmtp_t *codeccap)
{
    return 0;
}

int32_t getAvailableBitrateInfo(uint32_t opID, uint32_t *pcount,
                                        video_media_bitrate_t *pvideo_media_bitrate)
{
    return 0;
}

int32_t getAdaptativeFrameRateInfo(uint32_t opID, video_format_t format, uint32_t profile, uint32_t level,
                                                 uint32_t expected_bitrate,
                                                 uint32_t *target_bitrate, uint32_t *target_framerate)
{
   return 0;
}

int32_t getCodecLevelParameterSets(uint32_t opID, uint32_t quality,
                                            sensor_resolution_t *resolution,
                                            video_format_t informat,
                                            uint32_t *levelCapNumbers,
                                            video_codec_level_fmtp_t *codeclevelcap)
{
    return 0;
}


int32_t configLowPowerMode(low_power_mode_t lpMode, int32_t enable)
{
    return 0;
}

uint32_t getAdaptiveDropFrameCapabiltiy(video_format_t format)
{
    return 0;
}

int32_t getAdaptativeFrameRateInfo(uint32_t opID, video_format_t format, uint32_t profile, uint32_t level,
                                   uint32_t expected_bitrate,
                                   uint32_t *target_bitrate, uint32_t *target_framerate,
                                   uint32_t width, uint32_t height,
                                   uint32_t *target_width, uint32_t *target_height)
{
   return 0;
}



#endif

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

#ifndef IVCODECCAP_H
#define IVCODECCAP_H

#define VOLTE_MAX_SDP_PARAMETER_SET_LENGTH (256)
#define VCODEC_VOLTE_MAX_SDP_EGBYTE_LENGTH (8)

typedef enum _video_format
{
    VIDEO_H264 = 1,    ///< H.264 video format
    VIDEO_H263 = 2,
    VIDEO_MPEG4 = 3,
    VIDEO_HEVC = 4,    ///< HEVC video format
    VIDEO_FORMAT_NUM = 5,
    VIDEO_FORMAT_MAX = 0xFFFFFFFF      ///< Max VENC_DRV_VIDEO_FORMAT_T value
} video_format_t;

typedef enum _video_quality
{
    VIDEO_QUALITY_DEFAULT = 0,
    VIDEO_QUALITY_FINE = (1 << 0),
    VIDEO_QUALITY_HIGH = (1 << 1),
    VIDEO_QUALITY_MEDIUM = (1 << 2),
    VIDEO_QUALITY_LOW = (1 << 3),
    VIDEO_QUALITY_END = VIDEO_QUALITY_LOW,
    VIDEO_QUALITY_MAX = 0xFFFFFFFF
} video_quality_t;


typedef enum _video_profile
{
    VIDEO_PROFILE_UNKNOWN                   = 0,          ///< Unknown video profile
    VIDEO_PROFILE_BASELINE               = (1 << 0),   ///< H264 baseline profile
    VIDEO_PROFILE_CONSTRAINED_BASELINE  = (1 << 1),   ///< H264 constrained baseline profile
    VIDEO_PROFILE_MAIN                  = (1 << 2),   ///< H264 main profile
    VIDEO_PROFILE_EXTENDED              = (1 << 3),   ///< H264 extended profile
    VIDEO_PROFILE_HIGH                  = (1 << 4),   ///< H264 high profile
    VIDEO_PROFILE_HIGH_10                   = (1 << 5),   ///< H264 high 10 profile
    VIDEO_PROFILE_HIGH422                   = (1 << 6),   ///< H264 high 422 profile
    VIDEO_PROFILE_HIGH444                   = (1 << 7),   ///< H264 high 444 profile
    VIDEO_PROFILE_HIGH_10_INTRA         = (1 << 8),   ///< H264 high 10 intra profile in Amendment 2
    VIDEO_PROFILE_HIGH422_INTRA         = (1 << 9),   ///< H264 high 422 intra profile in Amendment 2
    VIDEO_PROFILE_HIGH444_INTRA         = (1 << 10),  ///< H264 high 444 intra profile in Amendment 2
    VIDEO_PROFILE_CAVLC444_INTRA            = (1 << 11),  ///< H264 CAVLC 444 intra profile in Amendment 2
    VIDEO_PROFILE_HIGH444_PREDICTIVE    = (1 << 12),  ///< H264 high 444 predictive profile in Amendment 2
    VIDEO_PROFILE_SCALABLE_BASELINE     = (1 << 13),  ///< H264 scalable baseline profile in Amendment 3
    VIDEO_PROFILE_SCALABLE_HIGH         = (1 << 14),  ///< H264 scalable high profile in Amendment 3
    VIDEO_PROFILE_SCALABLE_HIGH_INTRA   = (1 << 15),  ///< H264 scalable high intra profile in Amendment 3
    VIDEO_PROFILE_MULTIVIEW_HIGH            = (1 << 16)   ///< Corrigendum 1 (2009)
} video_profile_t;

typedef enum _video_level
{
    VIDEO_LEVEL_UNKNOWN = 0,
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
} video_level_t;

typedef enum low_power_mode
{
	CONFIG_GED_VILTE_MODE = (1 << 0),
} low_power_mode_t;

typedef struct
{
    uint8_t         valid;
    uint32_t        x;    ///< RFC6236 image attributes
    uint32_t        y;
    uint8_t         sar[VCODEC_VOLTE_MAX_SDP_EGBYTE_LENGTH];    //sample aspect ratio
    uint8_t         par[VCODEC_VOLTE_MAX_SDP_EGBYTE_LENGTH];    //allowed range in picture aspect ratio
    uint8_t         q[VCODEC_VOLTE_MAX_SDP_EGBYTE_LENGTH];    //a given parameter set over another
} image_info;

typedef struct h264_codec_fmtp
{
    /*@ profile-level-id */
    uint32_t  profile_level_id; ///< H.264 profile level id. construct by 3 parts: profile_idc, profile_iop, level
    uint32_t  max_recv_level;
    uint8_t   redundant_pic_cap;  ///< 0, 1. 0: we do not support yet.

    /*@ sprop-parameter-sets, in NAL units*/
    uint8_t     sprop_parameter_sets[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    /*@ sprop-level-parameter-sets*/
    uint8_t     sprop_level_parameter_sets[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];

    // NOTE: if the below value is 0, please ignore it and don't add to SDP
    uint32_t    max_mbps;     ///< reserved. VT service provides default value
    uint32_t    max_smbps;    ///< reserved. VT service provides default value
    uint32_t    max_fs;       ///< reserved. VT service provides default value
    uint32_t    max_cpb;      ///< reserved. VT service provides default value
    uint32_t    max_dpb;      ///< reserved. VT service provides default value
    uint32_t    max_br;       ///< reserved. VT service provides default value
    uint32_t    max_rcmd_nalu_size;     ///< reserved. VT service provides default value
    uint8_t     sar_understood;         ///< 0~255, reserved. VT service provides default value
    uint8_t     sar_supported;          ///< 0~255, reserved. VT service provides default value

    uint32_t    video_b_as; // video encode bitrate
    uint8_t     packetization_mode;
} h264_codec_fmtp_t;

typedef struct hevc_codec_fmtp
{
    uint8_t profile_space;
    uint8_t tier_flag;
    uint8_t profile_id;
    uint8_t level_id;
    uint8_t interop_constraints[6];
    uint8_t profile_comp_ind[4];
    uint8_t sprop_sub_layer_id;
    uint8_t recv_sub_layer_id;
    uint8_t max_recv_level_id;
    uint8_t tx_mode;  //"SRST", "MRST" or "MRMT"
    uint8_t sprop_vps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    uint8_t sprop_sps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    uint8_t sprop_pps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    uint8_t sprop_sei[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    uint32_t max_lsr;
    uint32_t max_lps;
    uint32_t max_cpb;
    uint32_t max_dpb;
    uint32_t max_br;
    uint32_t max_tr;
    uint32_t max_tc;
    uint32_t max_fps;
    uint32_t sprop_max_don_diff;
    uint32_t sprop_depack_buf_nalus;
    uint32_t sprop_depack_buf_bytes;
    uint32_t depack_buf_cap;
    uint32_t sprop_seg_id;
    uint32_t sprop_spatial_seg_idc;
    uint8_t dec_parallel_cap[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    uint8_t include_dph[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    uint32_t    video_b_as; // video encode bitrate
    uint8_t     packetization_mode;
} hevc_codec_fmtp_t;

typedef union codec_fmtp
{
    h264_codec_fmtp_t h264_codec_fmtp;
    hevc_codec_fmtp_t hevc_codec_fmtp;
} codec_fmtp_t;

typedef struct video_codec_fmtp
{
    video_format_t format;
    uint32_t    fps;
    uint32_t    width;
    uint32_t    height;
    image_info  image_send[4];
    image_info  image_recv[4];
    codec_fmtp_t codec_fmtp;
} video_codec_fmtp_t;

typedef struct h264_codec_level_fmtp
{
    /*@ profile-level-id */
    uint32_t    profile_level_id; ///< H.264 profile level id. construct by 3 parts: profile_idc, profile_iop, level
    uint8_t     sprop_parameter_sets[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
} h264_codec_level_fmtp_t;

typedef struct hevc_codec_level_fmtp
{
    uint8_t profile_space;
    uint8_t tier_flag;
    uint8_t profile_id;
    uint8_t level_id;
    uint8_t interop_constraints[6];
    uint8_t profile_comp_ind[4];
    uint8_t sprop_sub_layer_id;
    uint8_t recv_sub_layer_id;
    uint8_t max_recv_level_id;
    uint8_t sprop_vps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    uint8_t sprop_sps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    uint8_t sprop_pps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    uint8_t sprop_sei[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
} hevc_codec_level_fmtp_t;

typedef union codec_level_fmtp
{
    h264_codec_level_fmtp_t h264_codec_level_fmtp;
    hevc_codec_level_fmtp_t hevc_codec_level_fmtp;
} codec_level_fmtp_t;

typedef struct video_codec_level_fmtp
{
    video_format_t format;
    uint32_t    fps;
    uint32_t    width;
    uint32_t    height;
    codec_level_fmtp_t codec_level_fmtp;
} video_codec_level_fmtp_t;

typedef struct _sensor_resolution
{
    uint32_t sensor_max_width;
    uint32_t sensor_max_height;
    /*
	  NotSure = -1,
	  Horizontal = 0, W>H
	  Vertical =1, W<H
    */
    uint32_t prefer_WH_ratio;
} sensor_resolution_t;

typedef struct _video_media_profile
{
    video_format_t format;
    video_profile_t profile;
    video_level_t level;
    uint32_t width;
    uint32_t height;
    uint32_t fps;
    uint32_t minbitrate;
    uint32_t bitrate;
    uint32_t Iinterval;
} video_media_profile_t;

typedef struct _video_media_bitrate
{
    video_format_t format;
    uint32_t profile;
    uint32_t level;
    uint32_t minbitrate;
    uint32_t bitrate;
} video_media_bitrate_t;

/* deprecated
[in] format: codec format
[in] profile
[in] level
[out] video_media_profile_t
*/
int32_t getMediaProfile(video_format_t format,
                                uint32_t profile,
                                uint32_t level,
                                video_media_profile_t *pmedia_profile);

/* deprecated
[in]video_quality_t: tell the UI setting quality
[in]sensor_resolution_t: :tell the sensor common resolution
[output]capNumbers:
*/
int32_t getCodecCapabiltiy(uint32_t quality,
                           sensor_resolution_t *resolution,
                           uint32_t *capNumbers,
                           video_codec_fmtp_t *codeccap);

/* deprecated */
int32_t getAvailableBitrateInfo(uint32_t *pcount,
                                        video_media_bitrate_t *pvideo_media_bitrate);

/* deprecated */
int32_t getAdaptativeFrameRateInfo(video_format_t format, uint32_t profile, uint32_t level,
                                   uint32_t expected_bitrate,
                                   uint32_t *target_bitrate, uint32_t *target_framerate);

/* deprecated */
int32_t getCodecLevelParameterSets(uint32_t quality,
                                            sensor_resolution_t *resolution,
                                            video_format_t informat,
                                            uint32_t *levelCapNumbers,
                                            video_codec_level_fmtp_t *codeclevelcap);

// For dual sim
int32_t getMediaProfile(uint32_t opID, video_format_t format,
                                uint32_t profile,
                                uint32_t level,
                                video_media_profile_t *pmedia_profile);

int32_t getMediaProfileByQuality(uint32_t opID, video_quality_t quality,
                                        uint32_t *pprofile_count,
                                        video_media_profile_t *pmedia_profile);

int32_t getCodecCapabiltiy(uint32_t opID, uint32_t quality,
                           sensor_resolution_t *resolution,
                           uint32_t *capNumbers,
                           video_codec_fmtp_t *codeccap);

int32_t getAvailableBitrateInfo(uint32_t opID, uint32_t *pcount,
                                        video_media_bitrate_t *pvideo_media_bitrate);

int32_t getAdaptativeFrameRateInfo(uint32_t opID, video_format_t format, uint32_t profile, uint32_t level,
                                   uint32_t expected_bitrate,
                                   uint32_t *target_bitrate, uint32_t *target_framerate);


int32_t getAdaptativeFrameRateInfo(uint32_t opID, video_format_t format, uint32_t profile, uint32_t level,
                                   uint32_t expected_bitrate,
                                   uint32_t *target_bitrate, uint32_t *target_framerate, uint32_t width, uint32_t height,
                                   uint32_t *target_width, uint32_t *target_height);

int32_t getCodecLevelParameterSets(uint32_t opID, uint32_t quality,
                                            sensor_resolution_t *resolution,
                                            video_format_t informat,
                                            uint32_t *levelCapNumbers,
                                            video_codec_level_fmtp_t *codeclevelcap);

int32_t configLowPowerMode(low_power_mode_t lpMode, int32_t enable);

uint32_t getAdaptiveDropFrameCapabiltiy(video_format_t format);


#endif

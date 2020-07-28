/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "[VT]VcodecCap_main"

#include <sys/types.h>
#include <unistd.h>
#include "IVcodecCap.h"
#include <utils/Log.h>

#define VCODEC_UNUSED(x) (void)x

const int opID = 8;

void verifyBitrateInfo()
{
    uint32_t count = 0;
    ALOGI("<<<<< getAvailableBitrateInfo");
    getAvailableBitrateInfo(opID, &count, NULL);
    ALOGI(">>>>> getAvailableBitrateInfo: count: %d", count);

    video_media_bitrate_t *prvideo_media_bitrate_t = new video_media_bitrate_t[count];

    ALOGI("<<<<< getAvailableBitrateInfo");
    getAvailableBitrateInfo(opID, &count, prvideo_media_bitrate_t);
    ALOGI(">>>>> getAvailableBitrateInfo: count: %d", count);

    for (uint32_t i = 0; i < count; i++)
    {
        ALOGI("[%d] format:%d, profile:%d, level:%d = (%d,%d)",
              i,
              prvideo_media_bitrate_t[i].format,
              prvideo_media_bitrate_t[i].profile,
              prvideo_media_bitrate_t[i].level,
              prvideo_media_bitrate_t[i].minbitrate,
              prvideo_media_bitrate_t[i].bitrate);
    }
    delete []prvideo_media_bitrate_t;
}

void verfiyMediaProfile(video_quality_t quality, int sensor_max_width, int sensor_max_height)
{
    // getMediaProfile testcase
    uint32_t profile_count = 0;

    ALOGI("<<<<< getMediaProfileByQuality");
    getMediaProfileByQuality(opID, quality, &profile_count, NULL);
    ALOGI(">>>>> getMediaProfileByQuality: profile_count: %d", profile_count);

    video_media_profile_t *prvideo_media_profile_t_2 = new video_media_profile_t[profile_count];
    ALOGI("<<<<< getMediaProfileByQuality");
    getMediaProfileByQuality(opID, quality, &profile_count, prvideo_media_profile_t_2);
    ALOGI(">>>>> getMediaProfileByQuality: profile_count: %d", profile_count);
    for (uint32_t i = 0; i < profile_count; i++)
    {
        ALOGI("[%d] getMediaProfileByQuality by quality(%d): %d x %d",
              i,
              quality,
              prvideo_media_profile_t_2[i].width,
              prvideo_media_profile_t_2[i].height);
    }

    // getCodecCapabiltiy testcase
    sensor_resolution_t resolution;
    resolution.sensor_max_width = sensor_max_width;
    resolution.sensor_max_height = sensor_max_height;
    resolution.prefer_WH_ratio = 1;
    uint32_t capNumbers = 0;

    ALOGI("<<<<< getCodecCapabiltiy");
    getCodecCapabiltiy(opID, quality, &resolution, &capNumbers, NULL);
    ALOGI(">>>>> getCodecCapabiltiy: capNumbers: %d", capNumbers);

    video_codec_fmtp_t *prvideo_codec_fmtp_t = new video_codec_fmtp_t[capNumbers];
    ALOGI("<<<<< getCodecCapabiltiy");
    getCodecCapabiltiy(opID, quality, &resolution, &capNumbers, prvideo_codec_fmtp_t);
    ALOGI(">>>>> getCodecCapabiltiy: capNumbers: %d", capNumbers);
    for (uint32_t i = 0; i < capNumbers; i++)
    {
        if (prvideo_codec_fmtp_t[i].format == VIDEO_H264)
        {
            h264_codec_fmtp_t *ph264_codec_fmtp = &prvideo_codec_fmtp_t[i].codec_fmtp.h264_codec_fmtp;
            ALOGI("[%d] getMediaProfile: 0x%x", i, ph264_codec_fmtp->profile_level_id);
            if (getMediaProfile(opID, prvideo_codec_fmtp_t[i].format,
                                ph264_codec_fmtp->profile_level_id >> 16,
                                ph264_codec_fmtp->profile_level_id & 0xFF, prvideo_media_profile_t_2) != -1)
            {
                ALOGI("[%d][%d] getMediaProfile by level: %dx%d@%dfps, %dbps",
                      i,
                      prvideo_codec_fmtp_t[i].codec_fmtp.h264_codec_fmtp.packetization_mode,
                      prvideo_media_profile_t_2[0].width,
                      prvideo_media_profile_t_2[0].height,
                      prvideo_media_profile_t_2[0].fps,
                      prvideo_media_profile_t_2[0].bitrate);
            }
        }
        else if (prvideo_codec_fmtp_t[i].format == VIDEO_HEVC)
        {
            hevc_codec_fmtp *phevc_codec_fmtp = &prvideo_codec_fmtp_t[i].codec_fmtp.hevc_codec_fmtp;
            ALOGI("[%d] getMediaProfile: %d, %d", i, phevc_codec_fmtp->profile_id, phevc_codec_fmtp->level_id);
            if (getMediaProfile(opID, prvideo_codec_fmtp_t[i].format,
                                phevc_codec_fmtp->profile_id,
                                phevc_codec_fmtp->level_id, prvideo_media_profile_t_2) != -1)
            {
                ALOGI("[%d] getMediaProfile by level: %dx%d@%dfps, %dbps",
                      i,
                      prvideo_media_profile_t_2[0].width,
                      prvideo_media_profile_t_2[0].height,
                      prvideo_media_profile_t_2[0].fps,
                      prvideo_media_profile_t_2[0].bitrate);
            }
        }
    }

    delete []prvideo_media_profile_t_2;
    delete []prvideo_codec_fmtp_t;
}

void verifyAdaptativeFrameRateInfo()
{
    uint32_t target_bitrate;
    uint32_t target_framerate;
    int index = 0;
    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 30, 1200*1000, &target_bitrate, &target_framerate);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d", index++, target_bitrate, target_framerate);

    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 30, 1000*1000, &target_bitrate, &target_framerate);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d", index++, target_bitrate, target_framerate);

    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 30, 900*1000, &target_bitrate, &target_framerate);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d", index++, target_bitrate, target_framerate);

    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 30, 500*1000, &target_bitrate, &target_framerate);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d", index++, target_bitrate, target_framerate);

    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 30, 200*1000, &target_bitrate, &target_framerate);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d", index++, target_bitrate, target_framerate);

    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 31, 190*1000, &target_bitrate, &target_framerate);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d", index++, target_bitrate, target_framerate);
}

void verifyAdaptativeFrameRateInfo_Resolution()
{
    uint32_t target_bitrate;
    uint32_t target_framerate;
    int index = 0;

    uint32_t target_width = 480;
    uint32_t target_height = 640;

    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 30, 50*1000, &target_bitrate, &target_framerate, target_width, target_height, &target_width, &target_height);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d, target_resolution: %d, %d", index++, target_bitrate, target_framerate, target_width, target_height);

    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 30, 500*1000, &target_bitrate, &target_framerate, target_width, target_height, &target_width, &target_height);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d, target_resolution: %d, %d", index++, target_bitrate, target_framerate, target_width, target_height);

    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 30, 200*1000, &target_bitrate, &target_framerate, target_width, target_height, &target_width, &target_height);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d, target_resolution: %d, %d", index++, target_bitrate, target_framerate, target_width, target_height);

    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 30, 50*1000, &target_bitrate, &target_framerate, target_width, target_height, &target_width, &target_height);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d, target_resolution: %d, %d", index++, target_bitrate, target_framerate, target_width, target_height);

    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 30, 400*1000, &target_bitrate, &target_framerate, target_width, target_height, &target_width, &target_height);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d, target_resolution: %d, %d", index++, target_bitrate, target_framerate, target_width, target_height);

    getAdaptativeFrameRateInfo(opID, VIDEO_H264, 66, 30, 800*1000, &target_bitrate, &target_framerate, target_width, target_height, &target_width, &target_height);
    ALOGI("[%d] target_bitrate: %d, target_framerate: %d, target_resolution: %d, %d", index++, target_bitrate, target_framerate, target_width, target_height);

}


void verifyCodecLevel(video_quality_t quality, int sensor_max_width, int sensor_max_height)
{
    VCODEC_UNUSED(quality);
    uint32_t levelCapNumbers;
    video_codec_level_fmtp_t *codeclevelcap;

    sensor_resolution_t resolution;
    resolution.sensor_max_width = sensor_max_width;
    resolution.sensor_max_height = sensor_max_height;
    resolution.prefer_WH_ratio = 1;

    getCodecLevelParameterSets(opID, VIDEO_QUALITY_DEFAULT, &resolution, VIDEO_H264, &levelCapNumbers, NULL);

    codeclevelcap = new video_codec_level_fmtp_t[levelCapNumbers];
    getCodecLevelParameterSets(opID, VIDEO_QUALITY_DEFAULT, &resolution, VIDEO_H264, &levelCapNumbers, codeclevelcap);
    for(uint32_t i = 0; i < levelCapNumbers; i++)
    {
        ALOGI("[getCodecLevelParameterSets][%d] %dx%d@%dfps, sprop_parameter_sets: %s", i,
            codeclevelcap[i].width, codeclevelcap[i].height, codeclevelcap[i].fps,
            codeclevelcap[i].codec_level_fmtp.h264_codec_level_fmtp.sprop_parameter_sets);
    }
    delete []codeclevelcap;
}


int main()
{
    ALOGI("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    //verifyBitrateInfo();

    ALOGI("getAdaptiveDropFrameCapabiltiy: %d\n", getAdaptiveDropFrameCapabiltiy(VIDEO_H264));

    verfiyMediaProfile(VIDEO_QUALITY_DEFAULT, 1280, 720);

    verfiyMediaProfile(VIDEO_QUALITY_FINE, 1280, 720);
    verfiyMediaProfile(VIDEO_QUALITY_HIGH, 1280, 720);
    verfiyMediaProfile(VIDEO_QUALITY_MEDIUM, 1280, 720);
    verfiyMediaProfile(VIDEO_QUALITY_LOW, 1280, 720);

    //verfiyMediaProfile(VIDEO_QUALITY_DEFAULT, 640, 480);
    //verfiyMediaProfile(VIDEO_QUALITY_FINE, 640, 480);
    //verfiyMediaProfile(VIDEO_QUALITY_HIGH, 640, 480);
    //verfiyMediaProfile(VIDEO_QUALITY_MEDIUM, 640, 480);
    //verfiyMediaProfile(VIDEO_QUALITY_LOW, 640, 480);

    verifyAdaptativeFrameRateInfo();

    verifyAdaptativeFrameRateInfo_Resolution();

    verifyCodecLevel(VIDEO_QUALITY_DEFAULT, 640, 480);
    //configLowPowerMode(CONFIG_GED_VILTE_MODE, 1);
    //configLowPowerMode(CONFIG_GED_VILTE_MODE, 0);

    ALOGI(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    return 0;
}


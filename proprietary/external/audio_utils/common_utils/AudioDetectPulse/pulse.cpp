/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
**
** Copyright 2012, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/* #define LOG_NDEBUG 0 */
#define LOG_TAG "audio_utils_pulse"

#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <audio_utils/format.h>
#include <pulse.h>
#include "AudioDetectPulse.h"
#include <utils/Timers.h>

namespace android {

#ifdef MTK_LATENCY_DETECT_PULSE

#define MAX_FRAMECOUNT  2048
#define MAX_CHANNEL     2

short *SaveBuffer[TAG_MAX];

static const char *TagString[] = {
    "CaptureDataProvider",
    "FastCapture",
    "NormalCapture",
    "AudioRecord",
    "AudioTrack",
    "FastMixer",
    "NormalMixer",
    "PlaybackHandler",
    "StreamIn",
    "StreamOut",
    "CaptureHandler",
    "CaptureDataClient1",
    "CaptureDataClient2",
    "CaptureDataClient3",
    "CaptureDataClient4",
    "CaptureDataClient5",
    "Aurisys1",
    "Aurisys2",
    "Aurisys3",
    "Aurisys4",
    "Aurisys5",
    "Aurisys6",
    "Aurisys7",
    "Aurisys8",
    "Aurisys9",
    "SpeechIn",
    "SpeechOut",
    "Unknow"
};

static void dumpPCMData(const char *filepath, void *buffer, int count) {
    FILE *fp = fopen(filepath, "ab+");
    if (fp != NULL) {
        fwrite(buffer, 1, count, fp);
        fclose(fp);
    } else {
        ALOGE("open file fail");
    }
}

const char *Tag2String(const int TagNum) {
    return TagString[TagNum];
}

void detectPulse_(const int TagNum, const int pulseLevel, short *ptr, const size_t desiredFrames, const int channels,
           const int sampleRate, const int precision) {
    int partFrames = (int)desiredFrames / precision;
    int baseFrames = 0;
    int count = 0;
    int sum = 0;
    unsigned int bufVal = 0;

    nsecs_t frameTimeUs = systemTime(SYSTEM_TIME_MONOTONIC) / 1000;

    for (int i = 0; i < precision; i++) {
        count = (i == precision - 1) ? (int)(desiredFrames - baseFrames) : partFrames;
        for (int j = 0; j < count; j++) {
            bufVal = (unsigned int)abs((int)ptr[(baseFrames + j) * channels]);
            bufVal >>= 5;
            sum += bufVal * bufVal;
        }

        nsecs_t time = frameTimeUs - (int64_t)((desiredFrames - baseFrames - count) * 1000000 / (int64_t)sampleRate);
        if (sum >= pulseLevel) {
            ALOGD("TagNum %d - %s, sum %d, time %1.3f, detect pulse", TagNum, Tag2String(TagNum), sum, (double)time * 1e-3);
        } else {
            ALOGD("TagNum %d - %s, sum %d, time %1.3f", TagNum, Tag2String(TagNum), sum, (double)time * 1e-3);
        }
        baseFrames += partFrames;
        sum = 0;
    }
}

void detectPulse(const int TagNum, const int pulseLevel, const int dump, void *ptr,
                 const size_t desiredFrames, const audio_format_t format,
                 const int channels, const int sampleRate, const int precision) {
    //ALOGD("%s, TagNum %d, pulseLevel %d, ptr %x, format %d, frames %d, channels %d",
    //            __FUNCTION__, TagNum, pulseLevel, (int)ptr, format, (int)desiredFrames, channels);

    if (TagNum >= TAG_MAX) {
        ALOGE("%s, TagNum %d is not support!!", __FUNCTION__, TagNum);
        return;
    }
    if ((format != AUDIO_FORMAT_PCM_16_BIT) && (format != AUDIO_FORMAT_PCM_32_BIT) &&
        (format != AUDIO_FORMAT_PCM_8_24_BIT) && (format != AUDIO_FORMAT_PCM_FLOAT) &&
        (format != AUDIO_FORMAT_PCM_24_BIT_PACKED)) {
        ALOGE("%s, %s, format(%d) is not support!!", __FUNCTION__, Tag2String(TagNum), format);
        return;
    }
    if (channels > MAX_CHANNEL) {
        ALOGE("%s, %s, channel(%d) is not support!!", __FUNCTION__, Tag2String(TagNum), channels);
        return;
    }
    if (desiredFrames > MAX_FRAMECOUNT || desiredFrames <= 0) {
        ALOGE("%s, %s, frames(%d) is not support!!", __FUNCTION__, Tag2String(TagNum), (int)desiredFrames);
        return;
    }
    if (ptr == NULL) {
        ALOGE("%s, %s, ptr is NULL!!", __FUNCTION__, Tag2String(TagNum));
        return ;
    }

    if (SaveBuffer[TagNum] == NULL) {
        SaveBuffer[TagNum] = (short *)malloc(sizeof(short) * MAX_FRAMECOUNT * MAX_CHANNEL);
        ALOGD("%s, %s, malloc %p", __FUNCTION__, Tag2String(TagNum), SaveBuffer[TagNum]);
        if (SaveBuffer[TagNum] == NULL) {
            ALOGE("%s, %s, malloc fail!!", __FUNCTION__, Tag2String(TagNum));
            return;
        }
    }

    memcpy_by_audio_format(SaveBuffer[TagNum], AUDIO_FORMAT_PCM_16_BIT, ptr, format, desiredFrames * channels);

    if (dump) {
        // dump pcm
        char  fileName[80] = {0};
        sprintf(fileName, "%s.%s.pcm", "/sdcard/mtklog/audio_dump/detectPulse_16bit_Tag", Tag2String(TagNum));
        dumpPCMData(fileName, SaveBuffer[TagNum], desiredFrames * channels * audio_bytes_per_sample(AUDIO_FORMAT_PCM_16_BIT));
    }

    detectPulse_(TagNum, pulseLevel, SaveBuffer[TagNum], desiredFrames, channels, sampleRate, precision);
}

#else
void detectPulse(__attribute__((unused)) const int TagNum, __attribute__((unused)) const int pulseLevel,
                 __attribute__((unused)) const int dump, __attribute__((unused)) void *ptr,
                 __attribute__((unused)) const size_t desiredFrames, __attribute__((unused)) const audio_format_t format,
                 __attribute__((unused)) const int channels, __attribute__((unused)) const int sampleRate,
                 __attribute__((unused)) const int precision) {
}
#endif
}


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

#ifndef ANDROID_AUDIO_PULSE_H
#define ANDROID_AUDIO_PULSE_H

#include <system/audio.h>


typedef enum {
    TAG_CAPTURE_DATA_PROVIDER = 0,
    TAG_FAST_CATTURE,
    TAG_NORMAL_CATTURE,
    TAG_AUDIO_RECORD,
    TAG_AUDIO_TRACK,
    TAG_FAST_MIXER,
    TAG_NORMAL_MIXER,
    TAG_PLAYERBACK_HANDLER,
    TAG_STREAMIN,
    TAG_STREAMOUT,
    TAG_CAPTURE_HANDLER,
    TAG_CAPTURE_DATA_CLIENT1,
    TAG_CAPTURE_DATA_CLIENT2,
    TAG_CAPTURE_DATA_CLIENT3,
    TAG_CAPTURE_DATA_CLIENT4,
    TAG_CAPTURE_DATA_CLIENT5,
    TAG_AURISYS1,
    TAG_AURISYS2,
    TAG_AURISYS3,
    TAG_AURISYS4,
    TAG_AURISYS5,
    TAG_AURISYS6,
    TAG_AURISYS7,
    TAG_AURISYS8,
    TAG_AURISYS9,
    TAG_Speech_IN,
    TAG_Speech_OUT,
    TAG_MAX
} PULSE_TAG;


namespace android {

void detectPulse(const int TagNum, const int pulseLevel, const int dump, void *ptr,
                 const size_t desiredFrames, const audio_format_t format,
                 const int channels, const int sampleRate, const int precision);
}

#endif


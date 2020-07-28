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


#ifndef ANDROID_AUDIO_HAL_INTERFACE_MTK_H
#define ANDROID_AUDIO_HAL_INTERFACE_MTK_H

#define AUDIO_PARAMETER_KEY_TIME_STRETCH "time_stretch"
#define AUDIO_PARAMETER_KEY_HDMI_BITWIDCH "HDMI_bitwidth"
#define AUDIO_PARAMETER_KEY_HDMI_CHANNEL "HDMI_channel"
#define AUDIO_PARAMETER_KEY_HDMI_MAXSAMPLERATE "HDMI_maxsamplingrate"
#define AUDIO_PARAMETER_KEY_BESSURROUND_ONOFF "BesSurround_OnOff"
#define AUDIO_PARAMETER_KEY_BESSURROUND_MODE "BesSurround_Mode"

#define AUDIO_PARAMETER_KEY_HDMI_MAXSAMPLERATE "HDMI_maxsamplingrate"
#define AUDIO_PARAMETER_KEY_BESSURROUND_ONOFF "BesSurround_OnOff"
#define AUDIO_PARAMETER_KEY_BESSURROUND_MODE "BesSurround_Mode"
#define AUDIO_PARAMETER_KEY_BESAUDEN_ONOFF "SetMusicPlusStatus"

#define AUDIO_PARAMETER_KEY_ROUTING_TO_NONE "ROUTING_TO_NONE"
#define AUDIO_PARAMETER_KEY_FM_DIRECT_CONTROL "FM_DIRECT_CONTROL"

#define AUDIO_PARAMETER_KEY_OFFLOAD_AUDIO_STANDBY_WHEN_MUTE "OffloadAudio_Do_Standby_When_Mute"
#define AUDIO_PARAMETER_KEY_OFFLOAD_AUDIO_CHECK_SUPPORT "OffloadAudio_Check_Support"


#include <hardware/audio.h>

typedef enum {
    DEVICE_CBK_EVENT_SETPARAMETERS,
} device_parameters_callback_event_t;

typedef struct audio_hw_device_set_parameters_callback {
    int paramchar_len;
    char paramchar[1024];
} audio_hw_device_set_parameters_callback_t;


typedef int (*device_parameters_callback_t)(device_parameters_callback_event_t event, audio_hw_device_set_parameters_callback_t *param, void *cookie);
typedef int (*device_audio_parameter_changed_callback_t)(const char *param, void *cookie);


struct audio_hw_device_mtk: audio_hw_device {

    int (*xway_play_start)(struct audio_hw_device *dev, int sample_rate);
    int (*xway_play_stop)(struct audio_hw_device *dev);
    int (*xway_play_write)(struct audio_hw_device *dev, void *buffer, int size_bytes);
    int (*xway_getfreebuffercount)(struct audio_hw_device *dev);
    int (*xway_rec_start)(struct audio_hw_device *dev, int smple_rate);
    int (*xway_rec_stop)(struct audio_hw_device *dev);
    int (*xway_rec_read)(struct audio_hw_device *dev, void *buffer, int size_bytes);

    int (*setup_parameters_callback)(struct audio_hw_device *dev, device_parameters_callback_t callback, void *cookie);
    int (*set_audio_parameter_changed_callback)(struct audio_hw_device *dev, device_audio_parameter_changed_callback_t callback, void *cookie);
    int (*clear_audio_parameter_changed_callback)(struct audio_hw_device *dev, void *cookie);
};
typedef struct audio_hw_device_mtk audio_hw_device_mtk_t;

#endif  // ANDROID_AUDIO_HAL_INTERFACE_MTK_H

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

#define LOG_TAG "mtk_audio_hw_hal"
//#define LOG_NDEBUG 0

#include <stdint.h>
#include <inttypes.h>

#include <hardware/hardware.h>
#include <system/audio.h>
//#include <hardware/audio.h>
#include <hardware/audio_mtk.h>
#include <media/AudioParameter.h>

#include <hardware_legacy/AudioMTKHardwareInterface.h>

#include "AudioType.h"
#include "AudioDeviceInt.h"
#include "AudioTypeExt.h"
#include <sys/time.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <utils/String16.h>
#include <utils/threads.h>
#include <pthread.h>
#include <string>
#include <libladder.h>

namespace android {

#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
#define CALLSTACK() \
    { \
        ALOGD("CALL STACK : - %s", __FUNCTION__); \
        std::string stackString; \
        UnwindCurThreadBT(&stackString); \
        ALOGD("\t%s", stackString.c_str()); \
    }
#define AUDIO_HAL_FUNCTION_MIN_NS   (10000000) // 10 ms
#define AUDIO_HAL_FUNCTION_READ_NS   (80000000) // 80 ms
#define AUDIO_HAL_FUNCTION_WRITE_NS   (80000000) // 80 ms
#define AUDIO_HAL_FUNCTION_CREATEPATCH_NS   (80000000) // 80 ms
#define AUDIO_HAL_FUNCTION_SETMODE_NS   (80000000) // 80 ms
class AudioAutoTimeProfile {
public:
    inline AudioAutoTimeProfile(const char *kFuncName): mBeginTime(0), mEndTime(0), mTotalTime(0), mThresholdTime(AUDIO_HAL_FUNCTION_MIN_NS) {
        s8 = String8(kFuncName);
        mBeginTime = systemTime();
    }
    inline AudioAutoTimeProfile(const char *kFuncName, nsecs_t WarningNsec): mBeginTime(0), mEndTime(0), mTotalTime(0), mThresholdTime(AUDIO_HAL_FUNCTION_MIN_NS) {
        if (WarningNsec > mThresholdTime) {
            mThresholdTime = WarningNsec;
        }
        s8 = String8(kFuncName);
        mBeginTime = systemTime();
    }
    inline ~AudioAutoTimeProfile() {
        mEndTime = systemTime();
        mTotalTime = mEndTime - mBeginTime;
        if (mTotalTime >= mThresholdTime) {
            ALOGD("mtkAudProfile %" PRId64 " ms @ %s", mTotalTime / 1000000, s8.string());
            if (mDumpStack > 0) {
                CALLSTACK();
            }
        }
    }
    static int mDumpStack;
private:
    nsecs_t mBeginTime;
    nsecs_t mEndTime;
    nsecs_t mTotalTime;
    nsecs_t mThresholdTime;;
    String8 s8;
};

int AudioAutoTimeProfile::mDumpStack = 0;
#endif

/** HIDL Client usage and singleton Audio HAL Instance**/
static uint32_t gAudioHALRefCountByClient = 0;
static pthread_mutex_t gHwInstanceLock = PTHREAD_MUTEX_INITIALIZER;

extern "C" {

    struct legacy_audio_module {
        struct audio_module module;
    };

    struct legacy_audio_device {
        struct audio_hw_device_mtk device;

        AudioMTKHardwareInterface *hwif;
    };

    struct legacy_stream_out {
        struct audio_stream_out stream;

        AudioMTKStreamOutInterface *legacy_out;
    };

    struct legacy_stream_in {
        struct audio_stream_in stream;

        AudioMTKStreamInInterface *legacy_in;
    };

    /** audio_stream_out implementation **/
    static uint32_t out_get_sample_rate(const struct audio_stream *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->sampleRate();
    }

    static int out_set_sample_rate(struct audio_stream *stream, uint32_t rate __unused) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_out *out =
                reinterpret_cast<struct legacy_stream_out *>(stream);

        ALOGE("(%s:%d) %s: Implement me!", __FILE__, __LINE__, __func__);
        /* TODO: implement this */
        return 0;
    }

    static size_t out_get_buffer_size(const struct audio_stream *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->bufferSize();
    }

    static audio_channel_mask_t out_get_channels(const struct audio_stream *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return (audio_channel_mask_t) out->legacy_out->channels();
    }

    static audio_format_t out_get_format(const struct audio_stream *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        // legacy API, don't change return type
        return (audio_format_t) out->legacy_out->format();
    }

    static int out_set_format(struct audio_stream *stream, audio_format_t format __unused) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_out *out =
                reinterpret_cast<struct legacy_stream_out *>(stream);
        ALOGE("(%s:%d) %s: Implement me!", __FILE__, __LINE__, __func__);
        /* TODO: implement me */
        return 0;
    }

    static int out_standby(struct audio_stream *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_out *out =
                reinterpret_cast<struct legacy_stream_out *>(stream);
        return out->legacy_out->standby(false);
    }

    static int out_dump(const struct audio_stream *stream, int fd) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        Vector<String16> args;
        return out->legacy_out->dump(fd, args);
    }

    static int out_set_parameters(struct audio_stream *stream, const char *kvpairs) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_out *out =
                reinterpret_cast<struct legacy_stream_out *>(stream);
        int val;
        String8 s8 = String8(kvpairs);
        AudioParameter parms = AudioParameter(String8(kvpairs));

        return out->legacy_out->setParameters(s8);
    }

    static char *out_get_parameters(const struct audio_stream *stream, const char *keys) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        String8 s8;
        int val;

        s8 = out->legacy_out->getParameters(String8(keys));

        AudioParameter parms = AudioParameter(s8);

        return strdup(s8.string());
    }

    static uint32_t out_get_latency(const struct audio_stream_out *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->latency();
    }

    static int out_set_volume(struct audio_stream_out *stream, float left,
                              float right) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_out *out =
                reinterpret_cast<struct legacy_stream_out *>(stream);
        return out->legacy_out->setVolume(left, right);
    }

    static ssize_t out_write(struct audio_stream_out *stream, const void *buffer,
                             size_t bytes) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__, AUDIO_HAL_FUNCTION_WRITE_NS);
#endif
        struct legacy_stream_out *out =
                reinterpret_cast<struct legacy_stream_out *>(stream);
        return out->legacy_out->write(buffer, bytes);
    }

    static int out_drain(struct audio_stream_out *stream, audio_drain_type_t type) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_out *out =
                reinterpret_cast<struct legacy_stream_out *>(stream);
        return out->legacy_out->drain(type);
    }

    static int out_pause(struct audio_stream_out *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_out *out =
                reinterpret_cast<struct legacy_stream_out *>(stream);
        return out->legacy_out->pause();
    }

    static int out_resume(struct audio_stream_out *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_out *out =
                reinterpret_cast<struct legacy_stream_out *>(stream);
        return out->legacy_out->resume();
    }

    static int out_flush(struct audio_stream_out *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_out *out =
                reinterpret_cast<struct legacy_stream_out *>(stream);
        return out->legacy_out->flush();
    }

    static int out_get_render_position(const struct audio_stream_out *stream,
                                       uint32_t *dsp_frames) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->getRenderPosition(dsp_frames);
    }

    static int out_get_next_write_timestamp(const struct audio_stream_out *stream,
                                            int64_t *timestamp) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->getNextWriteTimestamp(timestamp);
    }

    static int out_set_callback(struct audio_stream_out *stream,
                                stream_callback_t callback, void *cookie) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
#if 1
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->setCallBack(callback, cookie);
#else
        return -EINVAL;
#endif
    }

    static int out_get_presentation_position(const struct audio_stream_out *stream,
                                             uint64_t *frames, struct timespec *timestamp) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
#if 1
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->getPresentationPosition(frames, timestamp);
#else
        return -EINVAL;
#endif
    }

    static void out_update_source_metadata(struct audio_stream_out *stream,
                                   const struct source_metadata* source_metadata) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->updateSourceMetadata(source_metadata);
    }

    static int out_add_audio_effect(const struct audio_stream *stream __unused, effect_handle_t effect __unused) {
        return -EINVAL;
    }

    static int out_remove_audio_effect(const struct audio_stream *stream __unused, effect_handle_t effect __unused) {
        return -EINVAL;
    }

    static int out_start(const struct audio_stream_out *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->start();
    }

    static int out_stop(const struct audio_stream_out *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->stop();
    }

    static int out_create_mmap_buffer(const struct audio_stream_out *stream,
                                     int32_t min_size_frames, struct audio_mmap_buffer_info *info) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->createMmapBuffer(min_size_frames, info);
    }

    static int out_get_mmap_position(const struct audio_stream_out *stream,
                                    struct audio_mmap_position *position) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_out *out =
                reinterpret_cast<const struct legacy_stream_out *>(stream);
        return out->legacy_out->getMmapPosition(position);
    }

    /** audio_stream_in implementation **/
    static uint32_t in_get_sample_rate(const struct audio_stream *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->sampleRate();
    }

    static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate __unused) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_in *in =
                reinterpret_cast<struct legacy_stream_in *>(stream);

        ALOGE("(%s:%d) %s: Implement me!", __FILE__, __LINE__, __func__);
        /* TODO: implement this */
        return 0;
    }

    static size_t in_get_buffer_size(const struct audio_stream *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->bufferSize();
    }

    static audio_channel_mask_t in_get_channels(const struct audio_stream *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return (audio_channel_mask_t) in->legacy_in->channels();
    }

    static audio_format_t in_get_format(const struct audio_stream *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        // legacy API, don't change return type
        return (audio_format_t) in->legacy_in->format();
    }

    static int in_set_format(struct audio_stream *stream, audio_format_t format  __unused) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_in *in =
                reinterpret_cast<struct legacy_stream_in *>(stream);
        ALOGE("(%s:%d) %s: Implement me!", __FILE__, __LINE__, __func__);
        /* TODO: implement me */
        return 0;
    }

    static int in_standby(struct audio_stream *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_in *in = reinterpret_cast<struct legacy_stream_in *>(stream);
        return in->legacy_in->standby(false);
    }

    static int in_dump(const struct audio_stream *stream, int fd) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        Vector<String16> args;
        return in->legacy_in->dump(fd, args);
    }

    static int in_set_parameters(struct audio_stream *stream, const char *kvpairs) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_in *in =
                reinterpret_cast<struct legacy_stream_in *>(stream);
        int val;
        AudioParameter parms = AudioParameter(String8(kvpairs));
        String8 s8 = String8(kvpairs);

        return in->legacy_in->setParameters(s8);
    }

    static char *in_get_parameters(const struct audio_stream *stream,
                                   const char *keys) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        String8 s8;
        int val;

        s8 = in->legacy_in->getParameters(String8(keys));

        AudioParameter parms = AudioParameter(s8);

        return strdup(s8.string());
    }

    static int in_set_gain(struct audio_stream_in *stream, float gain) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_in *in =
                reinterpret_cast<struct legacy_stream_in *>(stream);
        return in->legacy_in->setGain(gain);
    }

    static ssize_t in_read(struct audio_stream_in *stream, void *buffer,
                           size_t bytes) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__, AUDIO_HAL_FUNCTION_READ_NS);
#endif
        struct legacy_stream_in *in =
                reinterpret_cast<struct legacy_stream_in *>(stream);
        return in->legacy_in->read(buffer, bytes);
    }

    static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_stream_in *in =
                reinterpret_cast<struct legacy_stream_in *>(stream);
        return in->legacy_in->getInputFramesLost();
    }

    static int in_get_capture_position(const struct audio_stream_in *stream,
                                       int64_t *frames, int64_t *time) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->getCapturePosition(frames, time);
    }

    static int in_get_active_microphones(const struct audio_stream_in *stream,
                                  struct audio_microphone_characteristic_t *mic_array,
                                  size_t *mic_count) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->getActiveMicrophones(mic_array, mic_count);
    }

    static int in_set_microphone_direction(const struct audio_stream_in *stream,
                                  audio_microphone_direction_t direction) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->setMicrophoneDirection(direction);
    }

    static int in_set_microphone_field_dimension(const struct audio_stream_in *stream, float zoom) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->setMicrophoneFieldDimension(zoom);
    }

    static void in_update_sink_metadata(struct audio_stream_in *stream,
                                 const struct sink_metadata *sink_metadata) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->updateSinkMetadata(sink_metadata);
    }

    static int in_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->addAudioEffect(effect);
    }

    static int in_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->removeAudioEffect(effect);
    }

    static int in_start(const struct audio_stream_in *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->start();
    }

    static int in_stop(const struct audio_stream_in *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->stop();
    }

    static int in_create_mmap_buffer(const struct audio_stream_in *stream,
                                     int32_t min_size_frames, struct audio_mmap_buffer_info *info) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->createMmapBuffer(min_size_frames, info);
    }

    static int in_get_mmap_position(const struct audio_stream_in *stream,
                                    struct audio_mmap_position *position) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_stream_in *in =
                reinterpret_cast<const struct legacy_stream_in *>(stream);
        return in->legacy_in->getMmapPosition(position);
    }

    /** audio_hw_device implementation **/
    static inline struct legacy_audio_device *to_ladev(struct audio_hw_device *dev) {
        return reinterpret_cast<struct legacy_audio_device *>(dev);
    }

    static inline const struct legacy_audio_device *to_cladev(const struct audio_hw_device *dev) {
        return reinterpret_cast<const struct legacy_audio_device *>(dev);
    }

    static uint32_t adev_get_supported_devices(const struct audio_hw_device *dev  __unused) {
        /* XXX: The old AudioHardwareInterface interface is not smart enough to
         * tell us this, so we'll lie and basically tell AF that we support the
         * below input/output devices and cross our fingers. To do things properly,
         * audio hardware interfaces that need advanced features (like this) should
         * convert to the new HAL interface and not use this wrapper. */

        return (/* OUT */
                   AUDIO_DEVICE_OUT_EARPIECE |
                   AUDIO_DEVICE_OUT_SPEAKER |
                   AUDIO_DEVICE_OUT_WIRED_HEADSET |
                   AUDIO_DEVICE_OUT_WIRED_HEADPHONE |
                   AUDIO_DEVICE_OUT_AUX_DIGITAL |
                   AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET |
                   AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET |
                   AUDIO_DEVICE_OUT_ALL_SCO |
                   AUDIO_DEVICE_OUT_FM |
                   AUDIO_DEVICE_OUT_DEFAULT |
                   /* IN */
                   AUDIO_DEVICE_IN_COMMUNICATION |
                   AUDIO_DEVICE_IN_AMBIENT |
                   AUDIO_DEVICE_IN_BUILTIN_MIC |
                   AUDIO_DEVICE_IN_WIRED_HEADSET |
                   AUDIO_DEVICE_IN_AUX_DIGITAL |
                   AUDIO_DEVICE_IN_VOICE_CALL |
                   AUDIO_DEVICE_IN_BACK_MIC |
                   AUDIO_DEVICE_IN_ALL_SCO |
                   AUDIO_DEVICE_IN_FM_TUNER |
                   AUDIO_DEVICE_IN_DEFAULT);
    }

    static int adev_init_check(const struct audio_hw_device *dev) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_audio_device *ladev = to_cladev(dev);

        return ladev->hwif->initCheck();
    }

    static int adev_set_voice_volume(struct audio_hw_device *dev, float volume) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->setVoiceVolume(volume);
    }

    static int adev_set_master_volume(struct audio_hw_device *dev, float volume) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->setMasterVolume(volume);
    }

    static int adev_get_master_volume(struct audio_hw_device *dev, float *volume) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->getMasterVolume(volume);
    }

    static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__, AUDIO_HAL_FUNCTION_SETMODE_NS);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        // as this is the legacy API, don't change it to use audio_mode_t instead of int
        return ladev->hwif->setMode((int) mode);
    }

    static int adev_set_mic_mute(struct audio_hw_device *dev, bool state) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->setMicMute(state);
    }

    static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_audio_device *ladev = to_cladev(dev);
        return ladev->hwif->getMicMute(state);
    }

    static int adev_set_parameters(struct audio_hw_device *dev, const char *kvpairs) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->setParameters(String8(kvpairs));
    }

    static char *adev_get_parameters(const struct audio_hw_device *dev,
                                     const char *keys) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_audio_device *ladev = to_cladev(dev);
        String8 s8;

        s8 = ladev->hwif->getParameters(String8(keys));
        return strdup(s8.string());
    }

    static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev,
                                             const struct audio_config *config) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_audio_device *ladev = to_cladev(dev);
        return ladev->hwif->getInputBufferSize(config->sample_rate, (int) config->format,
                                               popcount(config->channel_mask));
    }

    static int adev_open_output_stream(struct audio_hw_device *dev,
                                       audio_io_handle_t handle,
                                       audio_devices_t devices,
                                       audio_output_flags_t flags,
                                       struct audio_config *config,
                                       struct audio_stream_out **stream_out,
                                       const char *address __unused) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        status_t status = (status_t) handle;
        struct legacy_stream_out *out;
        int ret;

        out = (struct legacy_stream_out *)calloc(1, sizeof(*out));
        if (!out) {
            return -ENOMEM;
        }

        out->legacy_out = ladev->hwif->openOutputStreamWithFlags(devices, flags,
                                                                 (int *) &config->format,
                                                                 &config->channel_mask,
                                                                 &config->sample_rate, &status);
        if (!out->legacy_out) {
            ret = status;
            goto err_open;
        }

        out->stream.common.get_sample_rate = out_get_sample_rate;
        out->stream.common.set_sample_rate = out_set_sample_rate;
        out->stream.common.get_buffer_size = out_get_buffer_size;
        out->stream.common.get_channels = out_get_channels;
        out->stream.common.get_format = out_get_format;
        out->stream.common.set_format = out_set_format;
        out->stream.common.standby = out_standby;
        out->stream.common.dump = out_dump;
        out->stream.common.set_parameters = out_set_parameters;
        out->stream.common.get_parameters = out_get_parameters;
        out->stream.common.add_audio_effect = out_add_audio_effect;
        out->stream.common.remove_audio_effect = out_remove_audio_effect;
        out->stream.get_latency = out_get_latency;
        out->stream.set_volume = out_set_volume;
        out->stream.write = out_write;
        out->stream.get_render_position = out_get_render_position;
        out->stream.get_next_write_timestamp = out_get_next_write_timestamp;

        out->stream.set_callback = out_set_callback;
        out->stream.get_presentation_position = out_get_presentation_position;

        out->stream.update_source_metadata = out_update_source_metadata;

        out->stream.pause = out_pause;
        out->stream.resume = out_resume;
        out->stream.drain = out_drain;
        out->stream.flush = out_flush;

        // AAudio MMAP
        if (flags & AUDIO_OUTPUT_FLAG_MMAP_NOIRQ) {
            out->stream.start = out_start;
            out->stream.stop = out_stop;
            out->stream.create_mmap_buffer = out_create_mmap_buffer;
            out->stream.get_mmap_position = out_get_mmap_position;
        }

        *stream_out = &out->stream;
        return 0;

err_open:
        free(out);
        *stream_out = NULL;
        return ret;
    }

    static void adev_close_output_stream(struct audio_hw_device *dev,
                                         struct audio_stream_out *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        struct legacy_stream_out *out = reinterpret_cast<struct legacy_stream_out *>(stream);

        ladev->hwif->closeOutputStream(out->legacy_out);
        free(out);
    }

    /** This method creates and opens the audio hardware input stream */
    static int adev_open_input_stream(struct audio_hw_device *dev,
                                      audio_io_handle_t handle,
                                      audio_devices_t devices,
                                      struct audio_config *config,
                                      struct audio_stream_in **stream_in,
                                      audio_input_flags_t flags /*__unused*/,
                                      const char *address __unused,
                                      audio_source_t source /*__unused*/) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        status_t status = (status_t) handle;;
        struct legacy_stream_in *in;
        int ret;
        AudioParameter param = AudioParameter();
        param.addInt(String8(AudioParameter::keyInputSource), source);
        in = (struct legacy_stream_in *)calloc(1, sizeof(*in));
        if (!in) {
            return -ENOMEM;
        }

#ifdef UPLINK_LOW_LATENCY
        in->legacy_in = ladev->hwif->openInputStreamWithFlags(devices, (int *) &config->format,
                                                              &config->channel_mask, &config->sample_rate,
                                                              &status, (audio_in_acoustics_t)0, flags);
#else
        in->legacy_in = ladev->hwif->openInputStream(devices, (int *) &config->format,
                                                     &config->channel_mask, &config->sample_rate,
                                                     &status, (audio_in_acoustics_t)0);
#endif
        if (!in->legacy_in) {
            ret = status;
            goto err_open;
        }
        in->legacy_in->setParameters(param.toString());
        in->stream.common.get_sample_rate = in_get_sample_rate;
        in->stream.common.set_sample_rate = in_set_sample_rate;
        in->stream.common.get_buffer_size = in_get_buffer_size;
        in->stream.common.get_channels = in_get_channels;
        in->stream.common.get_format = in_get_format;
        in->stream.common.set_format = in_set_format;
        in->stream.common.standby = in_standby;
        in->stream.common.dump = in_dump;
        in->stream.common.set_parameters = in_set_parameters;
        in->stream.common.get_parameters = in_get_parameters;
        in->stream.common.add_audio_effect = in_add_audio_effect;
        in->stream.common.remove_audio_effect = in_remove_audio_effect;
        in->stream.set_gain = in_set_gain;
        in->stream.read = in_read;
        in->stream.get_input_frames_lost = in_get_input_frames_lost;
        in->stream.get_capture_position = in_get_capture_position;
        in->stream.get_active_microphones = in_get_active_microphones;
        in->stream.set_microphone_direction = in_set_microphone_direction;
        in->stream.set_microphone_field_dimension = in_set_microphone_field_dimension;
        in->stream.update_sink_metadata = in_update_sink_metadata;

        // AAudio MMAP
        if (flags & AUDIO_INPUT_FLAG_MMAP_NOIRQ) {
            in->stream.start = in_start;
            in->stream.stop = in_stop;
            in->stream.create_mmap_buffer = in_create_mmap_buffer;
            in->stream.get_mmap_position = in_get_mmap_position;
        }

        *stream_in = &in->stream;
        return 0;

err_open:
        free(in);
        *stream_in = NULL;
        return ret;
    }

    //-----------------------------------------------------------------
    static int adev_set_emparameter(struct audio_hw_device *dev, void *ptr, int len) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->SetEMParameter(ptr, len);
    }

    static int adev_get_emparameter(struct audio_hw_device *dev, void *ptr, int len) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->GetEMParameter(ptr, len);
    }

    static int adev_set_audiocommand(struct audio_hw_device *dev, int par1, int par2) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->SetAudioCommand(par1, par2);
    }

    static int adev_get_audiocommand(struct audio_hw_device *dev, int par1) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->GetAudioCommand(par1);
    }

    static int adev_set_audiodata(struct audio_hw_device *dev, int par1, size_t len, void *ptr) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->SetAudioData(par1, len, ptr);
    }

    static int adev_get_audiodata(struct audio_hw_device *dev, int par1, size_t len, void *ptr) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->GetAudioData(par1, len, ptr);
    }

#if 0
    static int adev_set_acf_previewparameter(struct audio_hw_device *dev, void *ptr, int len) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->SetACFPreviewParameter(ptr, len);
    }

    static int adev_set_hcf_previewparameter(struct audio_hw_device *dev, void *ptr, int len) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->SetHCFPreviewParameter(ptr, len);
    }
#endif

    static int adev_xway_play_start(struct audio_hw_device *dev, int sample_rate) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayPlay_Start(sample_rate);
    }

    static int adev_xway_play_stop(struct audio_hw_device *dev) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayPlay_Stop();
    }

    static int adev_xway_play_write(struct audio_hw_device *dev, void *buffer, int size_bytes) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayPlay_Write(buffer, size_bytes);
    }

    static int adev_xway_getfreebuffercount(struct audio_hw_device *dev) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayPlay_GetFreeBufferCount();
    }

    static int adev_xway_rec_start(struct audio_hw_device *dev, int smple_rate) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayRec_Start(smple_rate);
    }

    static int adev_xway_rec_stop(struct audio_hw_device *dev) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayRec_Stop();
    }

    static int adev_xway_rec_read(struct audio_hw_device *dev, void *buffer, int size_bytes) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayRec_Read(buffer, size_bytes);
    }
    //-------------------------------------------------------------------------

    static void adev_close_input_stream(struct audio_hw_device *dev,
                                        struct audio_stream_in *stream) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        struct legacy_stream_in *in =
                reinterpret_cast<struct legacy_stream_in *>(stream);

        ladev->hwif->closeInputStream(in->legacy_in);
        free(in);
    }

    static int adev_get_microphones(const struct audio_hw_device *dev,
                           struct audio_microphone_characteristic_t *mic_array,
                           size_t *mic_count) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_audio_device *ladev = to_cladev(dev);
        return ladev->hwif->getMicrophones(mic_array, mic_count);
    }

    static int adev_dump(const struct audio_hw_device *dev, int fd) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        const struct legacy_audio_device *ladev = to_cladev(dev);
        Vector<String16> args;

        return ladev->hwif->dumpState(fd, args);
    }

    static int adev_create_audio_patch(struct audio_hw_device *dev,
                                       unsigned int num_sources,
                                       const struct audio_port_config *sources,
                                       unsigned int num_sinks,
                                       const struct audio_port_config *sinks,
                                       audio_patch_handle_t *handle) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__, AUDIO_HAL_FUNCTION_CREATEPATCH_NS);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->createAudioPatch(num_sources, sources, num_sinks, sinks, handle);
    }

    static int adev_release_audio_patch(struct audio_hw_device *dev,
                                        audio_patch_handle_t handle) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__, AUDIO_HAL_FUNCTION_CREATEPATCH_NS);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->releaseAudioPatch(handle);
    }

    static int adev_get_audio_port(struct audio_hw_device *dev,
                                   struct audio_port *port) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->getAudioPort(port);
    }

    static int adev_set_audio_port_config(struct audio_hw_device *dev,
                                          const struct audio_port_config *config) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->setAudioPortConfig(config);
    }

    static int adev_setup_parameters_callback(struct audio_hw_device *dev,
                                              device_parameters_callback_t callback, void *cookie) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->setupParametersCallback(callback, cookie);
    }

    static int adev_set_audio_parameters_changed_callback(struct audio_hw_device *dev,
                                                          device_audio_parameter_changed_callback_t callback, void *cookie) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->setAudioParameterChangedCallback(callback, cookie);
    }

    static int adev_clear_audio_parameters_changed_callback(struct audio_hw_device *dev, void *cookie) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->clearAudioParameterChangedCallback(cookie);
    }

    static int legacy_adev_close(hw_device_t *device) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        AudioAutoTimeProfile _p(__func__);
#endif
        struct audio_hw_device *hwdev =
                reinterpret_cast<struct audio_hw_device *>(device);
        struct legacy_audio_device *ladev = to_ladev(hwdev);

        if (!ladev) {
            return 0;
        }

        if (ladev->hwif) {
            uint32_t tempcount;
            pthread_mutex_lock(&gHwInstanceLock);
            gAudioHALRefCountByClient--;
            ALOGD("gAudioHALRefCountByClient - %d", gAudioHALRefCountByClient);
            tempcount = gAudioHALRefCountByClient;
            if (tempcount == 0) {
                delete ladev->hwif;
            }
            ladev->hwif = NULL;
            pthread_mutex_unlock(&gHwInstanceLock);
        }

        free(ladev);
        return 0;
    }

    static int legacy_adev_open(const hw_module_t *module, const char *name,
                                hw_device_t **device) {
#ifdef AUDIO_HAL_PROFILE_ENTRY_FUNCTION
        char value[PROPERTY_VALUE_MAX];
        property_get("vendor.audio.hal.callstack", value, "0");
        AudioAutoTimeProfile::mDumpStack = atoi(value);
        AudioAutoTimeProfile _p(__func__);
#endif
        struct legacy_audio_device *ladev;
        int ret;

        if (strncmp(name, AUDIO_HARDWARE_INTERFACE, strlen(AUDIO_HARDWARE_INTERFACE) + 1) != 0) {
            return -EINVAL;
        }

        ladev = (struct legacy_audio_device *)calloc(1, sizeof(*ladev));
        if (!ladev) {
            return -ENOMEM;
        }

        ladev->device.common.tag = HARDWARE_DEVICE_TAG;
#ifdef MTK_SUPPORT_AUDIO_DEVICE_API3
        ladev->device.common.version = AUDIO_DEVICE_API_VERSION_3_0;
#else
        ladev->device.common.version = AUDIO_DEVICE_API_VERSION_2_0;
#endif
        ladev->device.common.module = const_cast<hw_module_t *>(module);
        ladev->device.common.close = legacy_adev_close;

        ladev->device.get_supported_devices = adev_get_supported_devices;
        ladev->device.init_check = adev_init_check;
        ladev->device.set_voice_volume = adev_set_voice_volume;
        ladev->device.set_master_volume = adev_set_master_volume;
        ladev->device.get_master_volume = adev_get_master_volume;
        ladev->device.set_mode = adev_set_mode;
        ladev->device.set_mic_mute = adev_set_mic_mute;
        ladev->device.get_mic_mute = adev_get_mic_mute;
        ladev->device.set_parameters = adev_set_parameters;
        ladev->device.get_parameters = adev_get_parameters;
        ladev->device.get_input_buffer_size = adev_get_input_buffer_size;
        ladev->device.open_output_stream = adev_open_output_stream;
        ladev->device.close_output_stream = adev_close_output_stream;
        ladev->device.open_input_stream = adev_open_input_stream;
        ladev->device.close_input_stream = adev_close_input_stream;

        ladev->device.get_microphones = adev_get_microphones;

        ladev->device.dump = adev_dump;

        ladev->device.create_audio_patch = adev_create_audio_patch;
        ladev->device.release_audio_patch = adev_release_audio_patch;
        ladev->device.get_audio_port = adev_get_audio_port;
        ladev->device.set_audio_port_config = adev_set_audio_port_config;

#if 0   // Don't use them from Android N
        ladev->device.SetEMParameter = adev_set_emparameter;
        ladev->device.GetEMParameter = adev_get_emparameter;
        ladev->device.SetAudioCommand = adev_set_audiocommand;
        ladev->device.GetAudioCommand = adev_get_audiocommand;
        ladev->device.SetAudioData = adev_set_audiodata;
        ladev->device.GetAudioData = adev_get_audiodata;

        ladev->device.SetACFPreviewParameter = adev_set_acf_previewparameter;
        ladev->device.SetHCFPreviewParameter = adev_set_hcf_previewparameter;
#endif
        ladev->device.xway_play_start = adev_xway_play_start;
        ladev->device.xway_play_stop = adev_xway_play_stop;
        ladev->device.xway_play_write = adev_xway_play_write;
        ladev->device.xway_getfreebuffercount = adev_xway_getfreebuffercount;
        ladev->device.xway_rec_start = adev_xway_rec_start;
        ladev->device.xway_rec_stop = adev_xway_rec_stop;
        ladev->device.xway_rec_read = adev_xway_rec_read;

        // added for HIDL extend
        ladev->device.setup_parameters_callback = adev_setup_parameters_callback;
        ladev->device.set_audio_parameter_changed_callback = adev_set_audio_parameters_changed_callback;
        ladev->device.clear_audio_parameter_changed_callback = adev_clear_audio_parameters_changed_callback;

        pthread_mutex_lock(&gHwInstanceLock);
        ladev->hwif = createMTKAudioHardware();
        if (!ladev->hwif) {
            pthread_mutex_unlock(&gHwInstanceLock);
            ret = -EIO;
            free(ladev);
            return ret;
        } else {
            gAudioHALRefCountByClient++;
            ALOGD("gAudioHALRefCountByClient + %d", gAudioHALRefCountByClient);
            pthread_mutex_unlock(&gHwInstanceLock);
        }

        *device = &ladev->device.common;

        return 0;
    }

    static struct hw_module_methods_t legacy_audio_module_methods = {
        .open = legacy_adev_open
    };

    struct legacy_audio_module HAL_MODULE_INFO_SYM = {
        .module = {
            .common = {
                .tag = HARDWARE_MODULE_TAG,
                .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
                .hal_api_version = HARDWARE_HAL_API_VERSION,
                .id = AUDIO_HARDWARE_MODULE_ID,
                .name = "MTK Audio HW HAL",
                .author = "MTK",
                .methods = &legacy_audio_module_methods,
                .dso = NULL,
                .reserved = {0},
            },
        },
    };

}; // extern "C"

};


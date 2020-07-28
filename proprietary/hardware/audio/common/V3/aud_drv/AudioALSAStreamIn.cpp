#include "AudioALSAStreamIn.h"

#include <inttypes.h>
#include <cutils/atomic.h>
#include "AudioType.h"
#include "AudioALSAStreamManager.h"
#include "AudioALSACaptureHandlerBase.h"
#include "AudioUtility.h"

#include "AudioALSAFMController.h"

#include <audio_effects/effect_aec.h>
#include <audio_effects/effect_ns.h>
#include <audio_effects/effect_agc.h>
#include "WCNChipController.h"
#include <math.h>
#include "AudioDeviceInt.h"
#include <media/TypeConverter.h>

#ifdef MTK_LATENCY_DETECT_PULSE
#include "AudioDetectPulse.h"
#endif

#if defined(MTK_AUDIO_DSP_RECOVERY_SUPPORT)
#include <audio_dsp_controller.h>
#endif




#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSAStreamIn"

#define NORMAL_BUFFER_TIME_MS 20 //ms

namespace android {

int AudioALSAStreamIn::mDumpFileNum = 0;

// TODO(Harvey): Query this
static const audio_format_t       kDefaultInputSourceFormat      = AUDIO_FORMAT_PCM_16_BIT;
static const audio_channel_mask_t kDefaultInputSourceChannelMask = AUDIO_CHANNEL_IN_STEREO;
static const audio_channel_mask_t kDefaultVoiceInputSourceChannelMask = AUDIO_CHANNEL_IN_VOICE_UPLINK | AUDIO_CHANNEL_IN_VOICE_DNLINK;
static const uint32_t             kDefaultInputSourceSampleRate  = 48000;

static bool tempDebugflag = false;
//uint32_t AudioALSAStreamIn::mSuspendCount = 0;

static uint32_t get_latency_ms(uint32_t bytes, stream_attribute_t *attribute) {
    uint32_t sleep_ms = NORMAL_BUFFER_TIME_MS; // default 20 ms
    uint32_t word_size = audio_bytes_per_sample(attribute->audio_format);

    if (word_size != 0) {
        sleep_ms = (bytes * 1000) / (attribute->sample_rate * attribute->num_channels * word_size);
    } else if (attribute->mAudioInputFlags & AUDIO_INPUT_FLAG_FAST) {
        sleep_ms = UPLINK_LOW_LATENCY_MS;
    }
    return sleep_ms;
}

int AudioALSAStreamIn::calculateCapturePosition(int64_t *frames, int64_t *time) {
    int ret = mCaptureHandler->getCapturePosition(frames, time);
    *frames += mStandbyFrameCount;

    // For concurrent case, we have to calculate their frames by getRawStartFrameCount()
    *frames = *frames - mCaptureHandler->getRawStartFrameCount();
    return ret;
}

void AudioALSAStreamIn::updateStandByFrameCount() {
    /* Standby state will close data provider that will reset the capture position,
       therefore, we keep the frame counts before close */
    int64_t time;
    int64_t frames;
    int64_t oldStandbyFrameCount = mStandbyFrameCount;

    calculateCapturePosition(&frames, &time);
    mStandbyFrameCount = frames;

    ALOGD("%s(), update the mStandbyFrameCount = %" PRIu64 "-> %" PRIu64 , __FUNCTION__, oldStandbyFrameCount, mStandbyFrameCount);
}

AudioALSAStreamIn::AudioALSAStreamIn() :
    mStreamManager(AudioALSAStreamManager::getInstance()),
    mCaptureHandler(NULL),
    mLockCount(0),
    mSuspendLockCount(0),
    mIdentity(0xFFFFFFFF),
    mSuspendCount(0),
    mStandbyFrameCount(0),
    mStandby(true),
    mStreamInReopen(false),
    mUpdateStandbyFrameCount(false),
    mNewInputDevice(AUDIO_DEVICE_NONE),
    mLowLatencyMode(false),
    mKernelBuffersize(KERNEL_BUFFER_SIZE_UL1_NORMAL),
    mReduceBuffersize(0),
    mReadBytesSum(0),
    mOldReadBytesSum(0),
    mOldTime(0),
    mOldFrames(0),
    mOldAudioInputFlags(AUDIO_INPUT_FLAG_NONE),
    mOldBuffersize(0),
    mAudioSpeechEnhanceInfoInstance(AudioSpeechEnhanceInfo::getInstance()),
    mAvailableInputDevices(AUDIO_DEVICE_IN_BUILTIN_MIC),
    mStart(false),
    mDestroy(false) {
    ALOGD("%s()", __FUNCTION__);

#ifdef CONFIG_MT_ENG_BUILD
    mLogEnable = 1;
#else
    mLogEnable = __android_log_is_loggable(ANDROID_LOG_DEBUG, LOG_TAG, ANDROID_LOG_INFO);
#endif

    memset(&mStreamAttributeTarget, 0, sizeof(mStreamAttributeTarget));

    for (int i = 0; i < MAX_PREPROCESSORS; i++) {
        mPreProcessEffectBackup[i] = {0};
    }
    mPreProcessEffectBackupCount = 0;
    mPCMDumpFile = NULL;
}


AudioALSAStreamIn::~AudioALSAStreamIn() {
    ALOGD("%s()", __FUNCTION__);

    mDestroy = true;

    if (!mStandby) {
        ALOGW("%s(), not standby, mStandby %d, mCaptureHandler 0x%p",
              __FUNCTION__, mStandby, mCaptureHandler);
        standby();
    }
}


bool AudioALSAStreamIn::checkOpenStreamFormat(int *format) {
    if (*format != kDefaultInputSourceFormat) {
        ALOGE("%s(), wrong format 0x%x, use 0x%x instead.", __FUNCTION__, *format, kDefaultInputSourceFormat);
        *format = kDefaultInputSourceFormat;
        return false;
    } else {
        return true;
    }
}

bool AudioALSAStreamIn::checkOpenStreamChannels(uint32_t *channels) {
    if (*channels != kDefaultInputSourceChannelMask) {
        // Check channel mask for voice recording
        if (*channels & kDefaultVoiceInputSourceChannelMask && !(*channels & ~kDefaultVoiceInputSourceChannelMask)) {
            return true;
        }

        if (*channels & AUDIO_CHANNEL_IN_MONO) {
            return true;
        }

        ALOGE("%s(), wrong channels 0x%x, use 0x%x instead.", __FUNCTION__, *channels, kDefaultInputSourceChannelMask);
        *channels = kDefaultInputSourceChannelMask;
        return false;
    } else {
        return true;
    }

}


bool AudioALSAStreamIn::checkOpenStreamSampleRate(const audio_devices_t devices, uint32_t *sampleRate) {
    if (devices == AUDIO_DEVICE_IN_FM_TUNER) { // FM
        const uint32_t fm_uplink_sampling_rate = AudioALSAFMController::getInstance()->getFmUplinkSamplingRate();
        if (*sampleRate != fm_uplink_sampling_rate) {
            ALOGE("%s(), AUDIO_DEVICE_IN_FM_TUNER, wrong sampleRate %d, use %d instead.", __FUNCTION__, *sampleRate, fm_uplink_sampling_rate);
            *sampleRate = fm_uplink_sampling_rate;
            return false;
        } else {
            return true;
        }
    } else if (devices == AUDIO_DEVICE_IN_SPK_FEED) {
        if (*sampleRate != 16000) {
            ALOGE("%s(), AUDIO_DEVICE_IN_SPK_FEED, wrong sampleRate %d, use %d instead.", __FUNCTION__, *sampleRate, 16000);
            *sampleRate = 16000;
            return false;
        } else {
            return true;
        }
    } else if (devices == AUDIO_DEVICE_IN_HDMI) { // TDM dump for debug
        if (*sampleRate != 44100) {
            ALOGE("%s(), AUDIO_DEVICE_IN_HDMI, wrong sampleRate %d, use %d instead.", __FUNCTION__, *sampleRate, 44100);
            *sampleRate = 44100;
            return false;
        } else {
            return true;
        }
    } else { // Normal record
        if (*sampleRate != kDefaultInputSourceSampleRate) {
            ALOGD("%s(), origin sampleRate %d, kDefaultInputSourceSampleRate %d.", __FUNCTION__, *sampleRate, kDefaultInputSourceSampleRate);

            if (mStreamAttributeTarget.mAudioInputFlags & AUDIO_INPUT_FLAG_FAST) {
                ALOGD("%s(), sampling rate no support", __FUNCTION__);
                *sampleRate = 48000;
                return false;
            }

            if (mStreamAttributeTarget.BesRecord_Info.besrecord_tuningEnable || mStreamAttributeTarget.BesRecord_Info.besrecord_dmnr_tuningEnable) {
                if (*sampleRate == 16000) {
                    ALOGE("%s(), BesRecord 16K tuning", __FUNCTION__);
                    mStreamAttributeTarget.BesRecord_Info.besrecord_tuning16K = true;
                    *sampleRate = 48000;
                    return true;
                }
            }

            return true;
        } else {
            if (mStreamAttributeTarget.BesRecord_Info.besrecord_tuningEnable || mStreamAttributeTarget.BesRecord_Info.besrecord_dmnr_tuningEnable) {
                mStreamAttributeTarget.BesRecord_Info.besrecord_tuning16K = false;
            }

            return true;
        }
    }

}

status_t AudioALSAStreamIn::set(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    audio_in_acoustics_t acoustics, uint32_t flags) {

    ALOGD("%s(), devices = 0x%x, format = 0x%x, channels = 0x%x, sampleRate = %d, acoustics = 0x%x, flags = %d",
          __FUNCTION__, devices, *format, *channels, *sampleRate, acoustics, flags);

    AL_AUTOLOCK(mLock);

    *status = NO_ERROR;

#ifdef UPLINK_LOW_LATENCY
    //fast record flags
    mStreamAttributeTarget.mAudioInputFlags = static_cast<audio_input_flags_t>(flags);
#endif

    CheckBesRecordInfo();

    // check format
    if (checkOpenStreamFormat(format) == false) {
        *status = BAD_VALUE;
    }

    // check channel mask
    if (checkOpenStreamChannels(channels) == false) {
        *status = BAD_VALUE;
    }

    // check sample rate
    if (checkOpenStreamSampleRate(devices, sampleRate) == false) {
        *status = BAD_VALUE;
    }

    // config stream attribute
    if (*status == NO_ERROR) {
        // format
        mStreamAttributeTarget.audio_format = static_cast<audio_format_t>(*format);

        // channel
        mStreamAttributeTarget.audio_channel_mask = *channels;
        mStreamAttributeTarget.num_channels = popcount(*channels);

        // sample rate
        mStreamAttributeTarget.sample_rate = *sampleRate;

        // devices
        mStreamAttributeTarget.input_device = static_cast<audio_devices_t>(devices);

        // acoustics flags
        mStreamAttributeTarget.acoustics_mask = static_cast<audio_in_acoustics_t>(acoustics);

        // set buffer size // TODO(Harvey): Check
#ifdef UPLINK_LOW_LATENCY
        size_t wordSize = 0;
        switch (mStreamAttributeTarget.audio_format) {
        case AUDIO_FORMAT_PCM_8_BIT: {
            wordSize = sizeof(int8_t);
            break;
        }
        case AUDIO_FORMAT_PCM_16_BIT: {
            wordSize = sizeof(int16_t);
            break;
        }
        case AUDIO_FORMAT_PCM_24_BIT_PACKED:
        case AUDIO_FORMAT_PCM_8_24_BIT:
        case AUDIO_FORMAT_PCM_32_BIT: {
            wordSize = sizeof(int32_t);
            break;
        }
        default: {
            wordSize = sizeof(int16_t);
            ALOGW("%s(), wrong format(0x%x), default use wordSize = %zu", __FUNCTION__, mStreamAttributeTarget.audio_format, wordSize);
            break;
        }
        }

        if (audio_is_bluetooth_sco_device(devices) &&
            mStreamAttributeTarget.mAudioInputFlags & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_MMAP_NOIRQ)) {
            // bt not support fast
            mStreamAttributeTarget.mAudioInputFlags = (audio_input_flags_t)(mStreamAttributeTarget.mAudioInputFlags & (~(AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_MMAP_NOIRQ)));
        }

        if (mStreamAttributeTarget.mAudioInputFlags & AUDIO_INPUT_FLAG_FAST) {
            mStreamAttributeTarget.buffer_size = (mStreamAttributeTarget.sample_rate / 1000) * UPLINK_LOW_LATENCY_MS * mStreamAttributeTarget.num_channels * wordSize;
        } else if (mStreamAttributeTarget.mAudioInputFlags & AUDIO_INPUT_FLAG_MMAP_NOIRQ) {
            mStreamAttributeTarget.buffer_size = MMAP_UL_PERIOD_SIZE * mStreamAttributeTarget.num_channels * wordSize;
        } else {
            if (audio_is_bluetooth_sco_device(devices) && !WCNChipController::GetInstance()->IsBTMergeInterfaceSupported()) {
                // btsco with sw cvsd latency is fixed at 60ms
                mStreamAttributeTarget.buffer_size = (mStreamAttributeTarget.sample_rate / 1000) * 60 * mStreamAttributeTarget.num_channels * wordSize;
            } else {
                mStreamAttributeTarget.buffer_size = (mStreamAttributeTarget.sample_rate / 1000) * NORMAL_BUFFER_TIME_MS * mStreamAttributeTarget.num_channels * wordSize;
            }
        }

#else
        // Buffer size: 2048(period_size) * 8(period_count) * 2(ch) * 2(byte) = 64 kb
        mStreamAttributeTarget.buffer_size = 65536; /*mStreamManager->getInputBufferSize(mStreamAttributeTarget.sample_rate,
                                                                                mStreamAttributeTarget.audio_format,
                                                                                mStreamAttributeTarget.num_channels);*/
#endif
        //debug++
        tempDebugflag = mAudioSpeechEnhanceInfoInstance->GetDebugStatus();

        mOldAudioInputFlags = mStreamAttributeTarget.mAudioInputFlags;
        mOldBuffersize = mStreamAttributeTarget.buffer_size;
        ALOGD("%s() done, sampleRate = %d, num_channels = %d, buffer_size=%d, flag = %d, tempDebugflag = %d",
              __FUNCTION__, mStreamAttributeTarget.sample_rate, mStreamAttributeTarget.num_channels,
              mStreamAttributeTarget.buffer_size, mStreamAttributeTarget.mAudioInputFlags, tempDebugflag);
    }

    return *status;
}


uint32_t AudioALSAStreamIn::sampleRate() const {
    ALOGV("%s(), return %d", __FUNCTION__, mStreamAttributeTarget.sample_rate);
    return mStreamAttributeTarget.sample_rate;
}


size_t AudioALSAStreamIn::bufferSize() const {
    ALOGV("%s(), return 0x%x", __FUNCTION__, mStreamAttributeTarget.buffer_size);
    return mStreamAttributeTarget.buffer_size;
}

uint32_t AudioALSAStreamIn::channels() const {
    ALOGV("%s(), return 0x%x", __FUNCTION__, mStreamAttributeTarget.audio_channel_mask);
    return mStreamAttributeTarget.audio_channel_mask;
}

int AudioALSAStreamIn::format() const {
    ALOGV("%s(), return 0x%x", __FUNCTION__, mStreamAttributeTarget.audio_format);
    return mStreamAttributeTarget.audio_format;
}

status_t AudioALSAStreamIn::setGain(float gain) {
    ALOGD("%s(), gain %f", __FUNCTION__, gain);
    if (gain < 0.0f || gain > 1.0f || isnan(gain)) {
        ALOGE("%s(), invalid volume, gain %f", __FUNCTION__, gain);
        return BAD_VALUE;
    }
    return NO_ERROR;
}

void AudioALSAStreamIn::SetInputMute(bool bEnable) {
    ALOGD("%s(), %d", __FUNCTION__, bEnable);
    //AL_AUTOLOCK(mLock);
    mStreamAttributeTarget.micmute = bEnable;
    ALOGD("-%s()", __FUNCTION__);
}

ssize_t AudioALSAStreamIn::read(void *buffer, ssize_t bytes) {
    ALOGV("%s(), bytes= %zu", __FUNCTION__, bytes);
    if (tempDebugflag) {
        ALOGD("%s()+, bytes= %zu", __FUNCTION__, bytes);
    }
    ssize_t ret_size = bytes;

#if defined(MTK_AUDIO_DSP_RECOVERY_SUPPORT)
    /* fast record is RT thread and keep streamin lock.
    so other thread can't get streamin lock. if necessary, read will active release CPU. */
    int tryCountDsp = 10;
    while (get_audio_dsp_recovery_lock_cnt() && tryCountDsp--) {
        usleep(300);
        if (tryCountDsp == 0) {
            ALOGD("%s, free CPU, mLockCountAdsp %d, tryCountDsp %d",
                  __FUNCTION__, get_audio_dsp_recovery_lock_cnt(), tryCountDsp);
        }
    }
    LOCK_ALOCK_MS(get_adsp_recovery_lock(), MAX_RECOVERY_LOCK_TIMEOUT_MS);
    bool isDspRecovery = get_audio_dsp_recovery_mode();
#endif


    /* fast record is RT thread and keep streamin lock.
    so other thread can't get streamin lock. if necessary, read will active release CPU. */
    int tryCount = 10;
    while ((mLockCount != 0 || mSuspendLockCount != 0) && tryCount--) {
        ALOGV("%s, free CPU, mLockCount = %d, mSuspendLockCount = %d, tryCount %d", __FUNCTION__, mLockCount, mSuspendLockCount, tryCount);
        usleep(300);
        if (tryCount == 0) {
            ALOGD("%s, free CPU, mLockCount = %d, mSuspendLockCount = %d, tryCount %d", __FUNCTION__, mLockCount, mSuspendLockCount, tryCount);
        }
    }
    {
        AL_LOCK_MS(mSuspendLock, 3000);
        // FM Record is not support concurrent with VOIP Call , so we give mute data for this scenario
        if (mSuspendCount > 0 ||
#if defined(MTK_AUDIO_DSP_RECOVERY_SUPPORT)
            isDspRecovery ||
#endif
            ((mStreamAttributeTarget.input_source == AUDIO_SOURCE_FM_TUNER
#if defined(SUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL)
             || mStreamAttributeTarget.input_source == AUDIO_SOURCE_ODM_FM_RX
#endif
            ) &&
             (mStreamManager->isEchoRefUsing() == true))) {
            AL_UNLOCK(mSuspendLock);
#if defined(MTK_AUDIO_DSP_RECOVERY_SUPPORT)
            UNLOCK_ALOCK(get_adsp_recovery_lock());
#endif
            // here to sleep a buffer size latency and return.
            memset(buffer, 0, bytes);
            uint32_t sleep_ms = get_latency_ms(bytes, &mStreamAttributeTarget);
#if defined(MTK_AUDIO_DSP_RECOVERY_SUPPORT)
            ALOGD("%s(), mSuspendCount = %u, sleep_ms = %d, recovery %d", __FUNCTION__, mSuspendCount, sleep_ms, isDspRecovery);
#else
            ALOGD("%s(), mSuspendCount = %u, sleep_ms = %d", __FUNCTION__, mSuspendCount, sleep_ms);
#endif
            usleep(sleep_ms * 1000);
            mReadBytesSum += bytes;
            mStandbyFrameCount += bytes / getSizePerFrame(mStreamAttributeTarget.audio_format, mStreamAttributeTarget.num_channels);
            ALOGD("-%s(), mReadBytesSum %" PRIu64 ", update mStandbyFrameCount %" PRIu64 "", __FUNCTION__, mReadBytesSum, mStandbyFrameCount);
            return bytes;
        }
#if defined(MTK_AUDIO_DSP_RECOVERY_SUPPORT)
        UNLOCK_ALOCK(get_adsp_recovery_lock());
#endif

        AL_AUTOLOCK(mLock);

        status_t status = NO_ERROR;

        if ((mStandby == false) &&
            ((mCaptureHandler != NULL) &&
             (mCaptureHandler->getStreamInReopen() == true))) {
            setReopenState(true);
            mCaptureHandler->setStreamInReopen(false);
        }
        {
            AL_AUTOLOCK(mStandbyLock);
            if (getReopenState()) {
                if (mStandby == false) {
                    //ASSERT(mStreamManager->isPhoneCallOpen() == false);
                    ALOGD("%s(), close handler and reopen it", __FUNCTION__);
                    setUpdateStandByFrameCount(true);
                    status = close();
                    ASSERT(status == NO_ERROR);
                }
                if (mNewInputDevice != AUDIO_DEVICE_NONE) {
                    mStreamAttributeTarget.input_device = mNewInputDevice;
                    ALOGD("%s(), mNewInputDevice = 0x%x", __FUNCTION__, mStreamAttributeTarget.input_device);
                    mNewInputDevice = AUDIO_DEVICE_NONE;
                }
                setReopenState(false);
            }
        }
        /// check open
        if (mStandby == true) {
            status = open();
        }

        /// write pcm data
        if (status != NO_ERROR || mSuspendLockCount != 0 || mCaptureHandler == NULL) {
            AL_UNLOCK(mSuspendLock);
            ALOGW("%s(), mCaptureHandler = NULL", __FUNCTION__);
            ret_size = 0;
            // phonecall record return mute data when it is not in incall mode
            if ((mStreamAttributeTarget.input_source == AUDIO_SOURCE_VOICE_UPLINK) || (mStreamAttributeTarget.input_source == AUDIO_SOURCE_VOICE_DOWNLINK)
                || (mStreamAttributeTarget.input_source == AUDIO_SOURCE_VOICE_CALL)) {
                memset(buffer, 0, bytes);
                uint32_t sleep_ms = get_latency_ms(bytes, &mStreamAttributeTarget);
                ALOGD("%s(), sleep_ms = %d", __FUNCTION__, sleep_ms);
                usleep(sleep_ms * 1000);
                ret_size = bytes;
            }
        } else {
            AL_UNLOCK(mSuspendLock);
#ifdef MTK_PHONE_CALL_RECORD_DISABLE
            // not allow phonecall record and return mute data
            if (getCaptureHandlerType() == CAPTURE_HANDLER_VOICE) {
                memset(buffer, 0, bytes);
                uint32_t sleep_ms = get_latency_ms(bytes, &mStreamAttributeTarget);
                ALOGD("%s(), phonecall record disable, sleep_ms = %d", __FUNCTION__, sleep_ms);
                usleep(sleep_ms * 1000);
                ret_size = bytes;
            } else
#endif
            {
                ret_size = mCaptureHandler->read(buffer, bytes);
                WritePcmDumpData(buffer, ret_size);
            }
        }

#ifdef MTK_LATENCY_DETECT_PULSE
        AudioDetectPulse::doDetectPulse(TAG_STREAMIN, PULSE_LEVEL, 0, (void *)buffer,
                                        ret_size, mStreamAttributeTarget.audio_format,
                                        mStreamAttributeTarget.num_channels, mStreamAttributeTarget.sample_rate);
#endif

    }
    if (tempDebugflag) {
        ALOGD("%s()-", __FUNCTION__);
    }

    mReadBytesSum += ret_size;
    return ret_size;
}


status_t AudioALSAStreamIn::dump(int fd __unused, const Vector<String16> &args __unused) {
    ALOGV("%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAStreamIn::standby(bool halRequest) {
    ALOGD("%s(), halRequest %d, mDestroy %d, flag %d", __FUNCTION__, halRequest, mDestroy,
          mStreamAttributeTarget.mAudioInputFlags);
    int oldCount;
    oldCount = android_atomic_inc(&mLockCount);
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(mStandbyLock);
    oldCount = android_atomic_dec(&mLockCount);

    status_t status = NO_ERROR;

    if (mStandby == false) {
        setUpdateStandByFrameCount(true);
        status = close();
    }
    return status;
}

bool AudioALSAStreamIn::getStandby() {
    int oldCount;
    oldCount = android_atomic_inc(&mLockCount);
    AL_AUTOLOCK(mStandbyLock);
    oldCount = android_atomic_dec(&mLockCount);
    return mStandby;
}

status_t AudioALSAStreamIn::setParameters(const String8 &keyValuePairs) {
    ALOGV("+%s(): %s", __FUNCTION__, keyValuePairs.string());
    AudioParameter param = AudioParameter(keyValuePairs);

    /// keys
    const String8 keyInputSource = String8(AudioParameter::keyInputSource);
    const String8 keyRouting     = String8(AudioParameter::keyRouting);
    const String8 keyFormat      = String8(AudioParameter::keyFormat);

    /// parse key value pairs
    status_t status = NO_ERROR;
    int value = 0;
    int oldCount;
    String8 value_str;
    /// intput source
    if (param.getInt(keyInputSource, value) == NO_ERROR) {
        param.remove(keyInputSource);
        oldCount = android_atomic_inc(&mLockCount);
        // TODO(Harvey): input source
        AL_AUTOLOCK(mLock);
        audio_source_t current_input_source = static_cast<audio_source_t>(mStreamAttributeTarget.input_source);
        ALOGV("%s() InputSource = %d", __FUNCTION__, value);
        mStreamAttributeTarget.input_source = static_cast<audio_source_t>(value);
        if (mStreamAttributeTarget.mAudioInputFlags != mOldAudioInputFlags) {
            ALOGD("%s(), reset flag: %d => %d, buffer_size: %d => %d",
                  __FUNCTION__, mStreamAttributeTarget.mAudioInputFlags, mOldAudioInputFlags, mStreamAttributeTarget.buffer_size, mOldBuffersize);
            mStreamAttributeTarget.mAudioInputFlags = mOldAudioInputFlags;
            mStreamAttributeTarget.buffer_size = mOldBuffersize;
        }

        if (mStreamAttributeTarget.BesRecord_Info.besrecord_ForceMagiASREnable == true) {
            ALOGD("%s() force input source to AUDIO_SOURCE_CUSTOMIZATION1", __FUNCTION__);
            mStreamAttributeTarget.input_source = AUDIO_SOURCE_CUSTOMIZATION1;
        }

        if (mStreamAttributeTarget.BesRecord_Info.besrecord_ForceAECRecEnable == true) {
            ALOGD("%s() force input source to AUDIO_SOURCE_CUSTOMIZATION2", __FUNCTION__);
            mStreamAttributeTarget.input_source = AUDIO_SOURCE_CUSTOMIZATION2;
        }

        if ((mStreamAttributeTarget.input_source == AUDIO_SOURCE_VOICE_PERFORMANCE) &&
            ((mStreamAttributeTarget.mAudioInputFlags & AUDIO_INPUT_FLAG_FAST) == 0)) {
            // force to use low latency record for voice_performance
            ALOGD("%s(), voice_performance, force to use Low Latency", __FUNCTION__);
            mOldAudioInputFlags = mStreamAttributeTarget.mAudioInputFlags;
            mOldBuffersize = mStreamAttributeTarget.buffer_size;
            mStreamAttributeTarget.mAudioInputFlags = (audio_input_flags_t)(mStreamAttributeTarget.mAudioInputFlags | AUDIO_INPUT_FLAG_FAST);
            mStreamAttributeTarget.buffer_size = mStreamAttributeTarget.buffer_size / UPLINK_NORMAL_LATENCY_MS * UPLINK_LOW_LATENCY_MS;
        }

        //VOIP not use fast record
        if ((mStreamAttributeTarget.input_source == AUDIO_SOURCE_VOICE_COMMUNICATION) && (mStreamAttributeTarget.mAudioInputFlags == AUDIO_INPUT_FLAG_FAST)) {
            ALOGD("%s(), VOIP not use Low Latency, force to use Normal Latency", __FUNCTION__);
            mOldAudioInputFlags = mStreamAttributeTarget.mAudioInputFlags;
            mOldBuffersize = mStreamAttributeTarget.buffer_size;
            mStreamAttributeTarget.mAudioInputFlags = AUDIO_INPUT_FLAG_NONE;
            mStreamAttributeTarget.buffer_size = mStreamAttributeTarget.buffer_size / UPLINK_LOW_LATENCY_MS * UPLINK_NORMAL_LATENCY_MS;
        }

        if (mStreamAttributeTarget.input_source != current_input_source) {
            ALOGD("%s() InputSource: %d => %d, reopen", __FUNCTION__, current_input_source, mStreamAttributeTarget.input_source);
            setUpdateStandByFrameCount(true);
            setReopenState(true);
        }
        oldCount = android_atomic_dec(&mLockCount);
    }

    /// routing
    if (param.getInt(keyRouting, value) == NO_ERROR) {
        param.remove(keyRouting);
        oldCount = android_atomic_inc(&mLockCount);

        AL_AUTOLOCK(mLock);

        audio_devices_t inputdevice = static_cast<audio_devices_t>(value);
#if 0
        //only need to modify the device while VoIP
        if (mStreamAttributeTarget.BesRecord_Info.besrecord_voip_enable == true) {
            if (mStreamAttributeTarget.output_devices == AUDIO_DEVICE_OUT_SPEAKER) {
                if (inputdevice == AUDIO_DEVICE_IN_BUILTIN_MIC) {
                    if (USE_REFMIC_IN_LOUDSPK == 1) {
                        inputdevice = AUDIO_DEVICE_IN_BACK_MIC;
                        ALOGD("%s() force change to back mic", __FUNCTION__);
                    }
                }
            }
        }
#endif
        status = mStreamManager->routingInputDevice(this, mStreamAttributeTarget.input_device, inputdevice);
        oldCount = android_atomic_dec(&mLockCount);

    }

    if (param.getInt(keyFormat, value) == NO_ERROR) {
        param.remove(keyFormat);
        oldCount = android_atomic_inc(&mLockCount);

        AL_AUTOLOCK(mLock);
        ALOGV("%s() Format = %d", __FUNCTION__, value);
        mStreamAttributeTarget.audio_format = static_cast<audio_format_t>(value);

        oldCount = android_atomic_dec(&mLockCount);
    }

#if defined(MTK_SPEAKER_MONITOR_SPEECH_SUPPORT)
    if (param.getInt(String8("MOD_DAI_INPUT"), value) == NO_ERROR) {
        param.remove(String8("MOD_DAI_INPUT"));
        ALOGD("%s() set stream to MOD_DAI", __FUNCTION__);
        // channel
        mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_MONO;
        mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);

        // sample rate
        mStreamAttributeTarget.sample_rate = 16000;
        mStreamAttributeTarget.bFixedRouting = true;
        mStreamAttributeTarget.bModemDai_Input = true;
    }
#endif

    if (param.get(String8(AUDIO_PARAMETER_DEVICE_CONNECT), value_str) == NO_ERROR) {
        if (param.getInt(String8(AUDIO_PARAMETER_DEVICE_CONNECT), value) == NO_ERROR) {
            audio_devices_t device = (audio_devices_t)value;
            updateDeviceConnectionState(device, true);
        }

        if (param.get(String8("card"), value_str) == NO_ERROR) {
            param.remove(String8("card"));
        }

        if (param.get(String8("device"), value_str) == NO_ERROR) {
            param.remove(String8("device"));
        }

        param.remove(String8(AUDIO_PARAMETER_DEVICE_CONNECT));
    }

    if (param.get(String8(AUDIO_PARAMETER_DEVICE_DISCONNECT), value_str) == NO_ERROR) {
        if (param.getInt(String8(AUDIO_PARAMETER_DEVICE_DISCONNECT), value) == NO_ERROR) {
            audio_devices_t device = (audio_devices_t)value;
            updateDeviceConnectionState(device, false);
        }

        if (param.get(String8("card"), value_str) == NO_ERROR) {
            param.remove(String8("card"));
        }

        if (param.get(String8("device"), value_str) == NO_ERROR) {
            param.remove(String8("device"));
        }

        param.remove(String8(AUDIO_PARAMETER_DEVICE_DISCONNECT));
    }

    if (param.size()) {
        ALOGW("%s(), still have param.size() = %zu, remain param = \"%s\"",
              __FUNCTION__, param.size(), param.toString().string());
        status = BAD_VALUE;
    }

    ALOGV("-%s(): %s ", __FUNCTION__, keyValuePairs.string());
    return status;
}

void AudioALSAStreamIn::updateDeviceConnectionState(audio_devices_t device, bool connect) {
    if ((device & AUDIO_DEVICE_BIT_IN) != 0) {
        mAvailableInputDevices = connect ? (mAvailableInputDevices | device) : ((mAvailableInputDevices & ~device) | AUDIO_DEVICE_BIT_IN);
    }
}

bool AudioALSAStreamIn::getDeviceConnectionState(audio_devices_t device) {
    return ((device & AUDIO_DEVICE_BIT_IN) != 0 && (mAvailableInputDevices & device) == device);
}

String8 AudioALSAStreamIn::getParameters(const String8 &keys) {
    ALOGD("%s()", __FUNCTION__);
    AudioParameter param = AudioParameter(keys);
    AudioParameter returnParam = AudioParameter();
    int value_int;
    uint32_t result_int;
    String8 value;
    String8 result;
    std::string literalFormat;
    const String8 keyInputSource = String8(AudioParameter::keyInputSource);
    const String8 keyStreamSupportedFormats = String8(AudioParameter::keyStreamSupportedFormats);

    if (param.getInt(keyInputSource, value_int) == NO_ERROR) {
        ALOGD(" getParameters(keys); = %s", keys.string());
        param.remove(keyInputSource);
        result_int = static_cast<uint32_t>(mStreamAttributeTarget.input_source);
        //return value
        returnParam.addInt(keyInputSource, result_int);
    }

    if (param.get(keyStreamSupportedFormats, value) == NO_ERROR) {
        ALOGD(" getParameters(keys); = %s", keys.string());
        param.remove(keyStreamSupportedFormats);
        FormatConverter::toString(mStreamAttributeTarget.audio_format, literalFormat);
        result.append(literalFormat.c_str());
        //return string
        returnParam.add(keyStreamSupportedFormats, result);
    }

    const String8 keyValuePairs = returnParam.toString();
    ALOGD("-%s(), return \"%s\"", __FUNCTION__, keyValuePairs.string());
    return keyValuePairs;
}

unsigned int AudioALSAStreamIn::getInputFramesLost() const {
    return 0;
}

unsigned int AudioALSAStreamIn::getInputFlags() const {
#ifdef UPLINK_LOW_LATENCY
    return mStreamAttributeTarget.mAudioInputFlags;
#else
    return 0;
#endif
}

int AudioALSAStreamIn::getCapturePosition(int64_t *frames, int64_t *time) {
    int oldCount;
    oldCount = android_atomic_inc(&mLockCount);
    if (AL_TRYLOCK(mStandbyLock) != 0) {
        oldCount = android_atomic_dec(&mLockCount);
        return -ENODATA;
    }
    oldCount = android_atomic_dec(&mLockCount);
    if (mCaptureHandler == NULL || frames == NULL || time == NULL) {
        ALOGV("%s(), mCaptureHandler == NULL", __FUNCTION__);
        AL_UNLOCK(mStandbyLock);
        return -ENODATA ;
    }

    int ret = calculateCapturePosition(frames, time);

    if ((*time == mOldTime) && (mReadBytesSum > mOldReadBytesSum) && (ret == 0)) {
        uint64_t frameSize = getSizePerFrame(mStreamAttributeTarget.audio_format, mStreamAttributeTarget.num_channels);
        *time += getBufferLatencyUs(&mStreamAttributeTarget, 2 * frameSize);
        *frames += 2;

        ALOGD("%s(), timestamp not change, update time %" PRIu64 ", frames = %" PRIu64 "", __FUNCTION__, *time, *frames);
    }

    mOldTime = *time;
    mOldFrames = *frames;
    mOldReadBytesSum = mReadBytesSum;

    AL_UNLOCK(mStandbyLock);
    ALOGV("%s(), ret = %d, frames = %" PRIu64 " (mStandbyFrameCount = %" PRIu64 ")", __FUNCTION__, ret, *frames, mStandbyFrameCount);
    return ret;
}

int AudioALSAStreamIn::getActiveMicrophones(struct audio_microphone_characteristic_t *micArray, size_t *micCount) {
    if (micCount == NULL) {
        ALOGE("%s(), micCount is NULL", __FUNCTION__);
        return -EINVAL;
    }

    if (micArray == NULL) {
        ALOGE("%s(), micArray is NULL", __FUNCTION__);
        return -EINVAL;
    }

    /* Get all mic info */
    getMicInfo(micArray, micCount);

    /* Match the device current used */
    size_t writeMicArrayIndex = 0;
    for (size_t i = 0; i < *micCount; i++) {
        if ((micArray[i].device & mStreamAttributeTarget.input_device) == mStreamAttributeTarget.input_device) {
            ALOGV("%s(), micArray[%zu].device(0x%x) is equals StreamIn's (0x%x)", __FUNCTION__, i, micArray[i].device, mStreamAttributeTarget.input_device);
            if (writeMicArrayIndex != i) {
                /* Update writeMicArrayIndex's mic info */
                micArray[writeMicArrayIndex] = micArray[i];
                ALOGV("%s(), src mic array index(%zu) is not equals dest's(%zu)", __FUNCTION__, i, writeMicArrayIndex);
            } else {
                ALOGV("%s(), src mic array index is equals dest's (%zu)", __FUNCTION__, i);
            }

            /* Update writeMicArrayIndex's channel info */
            for (size_t chIndex = 0; chIndex < channels(); chIndex++) {
                micArray[writeMicArrayIndex].channel_mapping[chIndex] = AUDIO_MICROPHONE_CHANNEL_MAPPING_DIRECT;
            }
            writeMicArrayIndex++;
        } else {
            ALOGV("%s(), micArray[%zu].device(0x%x) is not equals StreamIn's (0x%x)", __FUNCTION__, i, micArray[i].device, mStreamAttributeTarget.input_device);
        }
    }

    *micCount = writeMicArrayIndex;
    ALOGD("%s(), micArray = %p, micCount = %zu", __FUNCTION__, micArray, *micCount);

    //dumpMicInfo(micArray, *micCount);

    return 0;
}

int AudioALSAStreamIn::setMicrophoneDirection(audio_microphone_direction_t direction) {
    ALOGV("%s(), direction = %d", __FUNCTION__, direction);
    return -ENOSYS;
}

int AudioALSAStreamIn::setMicrophoneFieldDimension(float zoom) {
    ALOGV("%s(), direction = %f", __FUNCTION__, zoom);
    return -ENOSYS;
}

void AudioALSAStreamIn::updateSinkMetadata(const struct sink_metadata *sink_metadata) {
    ALOGV("%s(), sink_metadata = %p", __FUNCTION__, sink_metadata);
}

status_t AudioALSAStreamIn::addAudioEffect(effect_handle_t effect) {
    ALOGD("%s(), %p", __FUNCTION__, effect);

    int status;
    effect_descriptor_t desc;

    //record the effect which need enabled and set to capture handle later (setup it while capture handle created)
    status = (*effect)->get_descriptor(effect, &desc);
    ALOGD("%s(), effect name:%s, BackupCount=%d", __FUNCTION__, desc.name, mPreProcessEffectBackupCount);

    if (mPreProcessEffectBackupCount >= MAX_PREPROCESSORS) {
        ALOGD("%s(), exceed the uplimit", __FUNCTION__);
        return NO_ERROR;
    }

    if (status != 0) {
        ALOGD("%s(), no corresponding effect", __FUNCTION__);
        return NO_ERROR;
    } else {
        AL_AUTOLOCK(mLock);

        for (int i = 0; i < mPreProcessEffectBackupCount; i++) {
            if (mPreProcessEffectBackup[i] == effect) {
                ALOGD("%s() already found %s at index %d", __FUNCTION__, desc.name, i);
                return NO_ERROR;
            }
        }

        //echo reference
        if (memcmp(&desc.type, FX_IID_AEC, sizeof(effect_uuid_t)) == 0) {
            ALOGD("%s(), AECOn, need reopen the capture handle", __FUNCTION__);
            if (mStandby == false) {
                close();
            }
            mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_AECOn = true;
        }

        //noise suppression
        if (memcmp(&desc.type, FX_IID_NS, sizeof(effect_uuid_t)) == 0) {
            ALOGD("%s(), NSOn, need reopen the capture handle", __FUNCTION__);
            if (mStandby == false) {
                close();
            }
            mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_NSOn = true;
        }

        //auto gain control
        if (memcmp(&desc.type, FX_IID_AGC, sizeof(effect_uuid_t)) == 0) {
            ALOGD("%s(), AGCOn, need reopen the capture handle", __FUNCTION__);
            if (mStandby == false) {
                close();
            }
            mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_AGCOn = true;
        }

        mPreProcessEffectBackup[mPreProcessEffectBackupCount] = effect;
        mPreProcessEffectBackupCount++;

        mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_Record[mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_Count] = effect;
        mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_Count++;
        mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_Update = true;
    }

    ALOGD("%s()-", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAStreamIn::removeAudioEffect(effect_handle_t effect) {
    ALOGD("%s(), %p", __FUNCTION__, effect);

    int i;
    int status;
    status_t RetStatus = -EINVAL;
    effect_descriptor_t desc;

    if (mPreProcessEffectBackupCount <= 0) {
        ALOGD("%s(), mPreProcessEffectBackupCount wrong", __FUNCTION__);
        return NO_ERROR;
    }


    status = (*effect)->get_descriptor(effect, &desc);
    ALOGD("%s(), effect name:%s, BackupCount=%d", __FUNCTION__, desc.name, mPreProcessEffectBackupCount);
    if (status != 0) {
        ALOGD("%s(), no corresponding effect", __FUNCTION__);
        return NO_ERROR;
    }

    AL_AUTOLOCK(mLock);

    for (i = 0; i < mPreProcessEffectBackupCount; i++) {
        if (RetStatus == 0) { /* status == 0 means an effect was removed from a previous slot */
            mPreProcessEffectBackup[i - 1] = mPreProcessEffectBackup[i];
            mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_Record[i - 1] = mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_Record[i];
            ALOGD("%s() moving fx from %d to %d", __FUNCTION__, i, i - 1);
            continue;
        }
        if (mPreProcessEffectBackup[i] == effect) {
            ALOGD("%s() found fx at index %d, %p", __FUNCTION__, i, mPreProcessEffectBackup[i]);
            //            free(preprocessors[i].channel_configs);
            RetStatus = 0;
        }
    }

    if (RetStatus != 0) {
        ALOGD("%s() no effect found in backup queue", __FUNCTION__);
        return NO_ERROR;
    }

    //echo reference
    if (memcmp(&desc.type, FX_IID_AEC, sizeof(effect_uuid_t)) == 0) {
        if (mStandby == false) {
            close();
        }
        mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_AECOn = false;
    }

    //noise suppression
    if (memcmp(&desc.type, FX_IID_NS, sizeof(effect_uuid_t)) == 0) {
        if (mStandby == false) {
            close();
        }
        mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_NSOn = false;
    }

    //auto gain control
    if (memcmp(&desc.type, FX_IID_AGC, sizeof(effect_uuid_t)) == 0) {
        if (mStandby == false) {
            close();
        }
        mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_AGCOn = false;
    }

    mPreProcessEffectBackupCount--;
    /* if we remove one effect, at least the last preproc should be reset */
    mPreProcessEffectBackup[mPreProcessEffectBackupCount] = NULL;

    mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_Count--;
    /* if we remove one effect, at least the last preproc should be reset */
    mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_Record[mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_Count] = NULL;
    mStreamAttributeTarget.NativePreprocess_Info.PreProcessEffect_Update = true;

    ALOGD("%s()-", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAStreamIn::open() {
    // call open() only when mLock is locked.
    ASSERT(AL_TRYLOCK(mLock) != 0);

    ALOGD("%s()", __FUNCTION__);

    status_t status = NO_ERROR;

    if (mStandby == true) {
        // create capture handler
        ASSERT(mCaptureHandler == NULL);
        mCaptureHandler = mStreamManager->createCaptureHandler(&mStreamAttributeTarget);
        if (mCaptureHandler == NULL) {
            status = BAD_VALUE;
            return status;
        }
        mStandby = false;

        // open audio hardware
        status = mCaptureHandler->open();
        ASSERT(status == NO_ERROR);

        ALOGD("%s(), Set RawStartFrameCount = %" PRIu64, __FUNCTION__, mCaptureHandler->getRawStartFrameCount());
        OpenPCMDump();
    }

    return status;
}


status_t AudioALSAStreamIn::close() {
    // call close() only when mLock is locked.
    ASSERT(AL_TRYLOCK(mLock) != 0);

    ALOGD("%s()", __FUNCTION__);

    status_t status = NO_ERROR;

    if (mStandby == false) {
        mStandby = true;

        ASSERT(mCaptureHandler != NULL);

        if (getUpdateStandByFrameCount()) {
            updateStandByFrameCount();
            setUpdateStandByFrameCount(false);
        }

        // close audio hardware
        status = mCaptureHandler->close();
        if (status != NO_ERROR) {
            ALOGE("%s(), close() fail!!", __FUNCTION__);
        }

        ClosePCMDump();
        // destroy playback handler
        mStreamManager->destroyCaptureHandler(mCaptureHandler);
        mCaptureHandler = NULL;
    }

    ASSERT(mCaptureHandler == NULL);
    return status;
}


status_t AudioALSAStreamIn::routing(audio_devices_t input_device) {
    bool is_lock_in_this_function = false;
    if (AL_TRYLOCK(mLock) == 0) { // from another stream in setParameter routing
        ALOGD("%s(), is_lock_in_this_function = true", __FUNCTION__);
        is_lock_in_this_function = true;
    }

    status_t status = NO_ERROR;

    if (input_device == mStreamAttributeTarget.input_device) {
        ALOGW("%s(), input_device = 0x%x, already the same input device as current using", __FUNCTION__, input_device);

        if (is_lock_in_this_function == true) {
            AL_UNLOCK(mLock);
        }
        return status;
    }

    {
        AL_AUTOLOCK(mStandbyLock);
        if (mStandby == true && getReopenState() == false) {
            mStreamAttributeTarget.input_device = input_device;
            ALOGD("+%s(),streamin is standby, update mStreamAttributeTarget.input_device = 0x%x", __FUNCTION__, mStreamAttributeTarget.input_device);
            mNewInputDevice = AUDIO_DEVICE_NONE;
        } else {
            setReopenState(true);
            mNewInputDevice = input_device;
            ALOGD("+%s(), mNewInputDevice = 0x%x", __FUNCTION__, mNewInputDevice);
        }
    }
    if (is_lock_in_this_function == true) {
        AL_UNLOCK(mLock);
    }

    ALOGD("-%s()", __FUNCTION__);
    return status;
}


AudioALSACaptureHandlerBase *AudioALSAStreamIn::getStreamInCaptureHandler() {
    ALOGV("%s(), mCaptureHandler=%p", __FUNCTION__, mCaptureHandler);
    return mCaptureHandler;
}

capture_handler_t AudioALSAStreamIn::getCaptureHandlerType() {
    ALOGV("%s(), mCaptureHandler=%p", __FUNCTION__, mCaptureHandler);
    capture_handler_t capture_handler = CAPTURE_HANDLER_BASE;
    int oldCount;
    oldCount = android_atomic_inc(&mLockCount);
    AL_AUTOLOCK(mStandbyLock);
    oldCount = android_atomic_dec(&mLockCount);
    if (getStreamInCaptureHandler() != NULL) {
        capture_handler = getStreamInCaptureHandler()->getCaptureHandlerType();
    }
    return capture_handler;
}

status_t AudioALSAStreamIn::updateOutputDeviceInfoForInputStream(audio_devices_t output_devices) {
    status_t status = NO_ERROR;
    bool bBesRecUpdate = false;
    audio_devices_t inputdevice = mStreamAttributeTarget.input_device;

    if (isBtSpkDevice(output_devices)) {
        // use SPK setting for BTSCO + SPK
        output_devices = (audio_devices_t)(output_devices & (~AUDIO_DEVICE_OUT_ALL_SCO));
        ALOGD("%s(), Use SPK setting for BTSCO + SPK, output_devices: 0x%x => 0x%x", __FUNCTION__, mStreamAttributeTarget.output_devices, output_devices);
    }

    if (output_devices != mStreamAttributeTarget.output_devices) {
        //update VoIP parameters config, only streamin has VoIP process
        if ((mStreamAttributeTarget.BesRecord_Info.besrecord_enable == true) && (mStreamAttributeTarget.BesRecord_Info.besrecord_voip_enable == true)) {
            setReopenState(true);
        }
        ALOGD("%s(), mStreamInReopen = %d, mStreamAttributeTarget.output_devices change: 0x%x => 0x%x", __FUNCTION__, mStreamInReopen, mStreamAttributeTarget.output_devices, output_devices);
        //update output devices to input stream info
        mStreamAttributeTarget.output_devices = output_devices;

    }
    return status;
}

status_t AudioALSAStreamIn::setSuspend(const bool suspend_on) {
    ALOGD("%s(), mSuspendLockCount = %d ,mSuspendCount = %u, suspend_on = %d", __FUNCTION__, mSuspendLockCount, mSuspendCount, suspend_on);
    int oldCount;
    oldCount = android_atomic_inc(&mSuspendLockCount);
    AL_AUTOLOCK(mSuspendLock);
    oldCount = android_atomic_dec(&mSuspendLockCount);

    if (suspend_on == true) {
        mSuspendCount++;
    } else if (suspend_on == false) {
        if (mSuspendCount > 0) {
            mSuspendCount--;
        } else {
            ALOGD("%s(), mSuspendCount = 0 , no need to resume", __FUNCTION__);
        }
    }

    ALOGV("-%s(), mSuspendLockCount = %d, mSuspendCount = %u", __FUNCTION__, mSuspendLockCount, mSuspendCount);
    return NO_ERROR;
}

status_t AudioALSAStreamIn::setLowLatencyMode(bool mode) {
    ALOGD("+%s(), mode %d", __FUNCTION__, mode);
    int oldCount;
    oldCount = android_atomic_inc(&mLockCount);
    AL_AUTOLOCK(mLock);
    oldCount = android_atomic_dec(&mLockCount);

    mLowLatencyMode = mode;
    return setLowLatencyMode_l();
}


void AudioALSAStreamIn::CheckBesRecordInfo() {
    ALOGD("%s()", __FUNCTION__);

    if (mAudioSpeechEnhanceInfoInstance != NULL) {
        mStreamAttributeTarget.BesRecord_Info.besrecord_scene = mAudioSpeechEnhanceInfoInstance->GetBesRecScene();
        mAudioSpeechEnhanceInfoInstance->ResetBesRecScene();
        //for besrecord tuning
        mStreamAttributeTarget.BesRecord_Info.besrecord_tuningEnable = mAudioSpeechEnhanceInfoInstance->IsBesRecTuningEnable();

        //for DMNR tuning
        mStreamAttributeTarget.BesRecord_Info.besrecord_dmnr_tuningEnable = mAudioSpeechEnhanceInfoInstance->IsAPDMNRTuningEnable();

        mAudioSpeechEnhanceInfoInstance->GetBesRecVMFileName(mStreamAttributeTarget.BesRecord_Info.besrecord_VMFileName, VM_FILE_NAME_LEN_MAX);

        //for engineer mode
        if (mAudioSpeechEnhanceInfoInstance->GetForceMagiASRState() > 0) {
            mStreamAttributeTarget.BesRecord_Info.besrecord_ForceMagiASREnable = true;
        }
        if (mAudioSpeechEnhanceInfoInstance->GetForceAECRecState()) {
            mStreamAttributeTarget.BesRecord_Info.besrecord_ForceAECRecEnable = true;
        }

        //for dynamic mask
        mStreamAttributeTarget.BesRecord_Info.besrecord_dynamic_mask = mAudioSpeechEnhanceInfoInstance->GetDynamicVoIPSpeechEnhancementMask();
    }
}

void AudioALSAStreamIn::UpdateDynamicFunctionMask(void) {
    ALOGD("+%s()", __FUNCTION__);

    if (mAudioSpeechEnhanceInfoInstance != NULL) {
        //for dynamic mask
        mStreamAttributeTarget.BesRecord_Info.besrecord_dynamic_mask = mAudioSpeechEnhanceInfoInstance->GetDynamicVoIPSpeechEnhancementMask();
    }
    ALOGD("-%s()", __FUNCTION__);
}

bool AudioALSAStreamIn::isSupportConcurrencyInCall(void) {
    ALOGD("+%s()", __FUNCTION__);
    bool bIsSupport = false;
    int oldCount;
    oldCount = android_atomic_inc(&mLockCount);
    AL_AUTOLOCK(mStandbyLock);
    oldCount = android_atomic_dec(&mLockCount);

    if (mCaptureHandler != NULL) {
        bIsSupport = mCaptureHandler->isSupportConcurrencyInCall();
    } else {
        ALOGW("mCaptureHandler is NULL");
        bIsSupport = false;
    }

    ALOGD("-%s() bIsSupport = %d", __FUNCTION__, bIsSupport);
    return bIsSupport;
}


void AudioALSAStreamIn::OpenPCMDump() {
    ALOGV("%s()", __FUNCTION__);
    char Buf[30];
    snprintf(Buf, sizeof(Buf), "%d.%d.%s.%dch.pcm", mDumpFileNum,
            mStreamAttributeTarget.sample_rate,
            transferAudioFormatToDumpString(mStreamAttributeTarget.audio_format),
            mStreamAttributeTarget.num_channels);

    mDumpFileName = String8(streamin);
    mDumpFileName.append((const char *)Buf);

    mPCMDumpFile = NULL;
    mPCMDumpFile = AudioOpendumpPCMFile(mDumpFileName, streamin_propty);

    if (mPCMDumpFile != NULL) {
        ALOGD("%s DumpFileName = %s", __FUNCTION__, mDumpFileName.string());
    }

    mDumpFileNum++;
    mDumpFileNum %= MAX_DUMP_NUM;

}

void AudioALSAStreamIn::ClosePCMDump() {
    ALOGV("%s()", __FUNCTION__);
    if (mPCMDumpFile) {
        AudioCloseDumpPCMFile(mPCMDumpFile);
        ALOGD("%s(), close it", __FUNCTION__);
    }
}

void AudioALSAStreamIn::WritePcmDumpData(void *buffer, ssize_t bytes) {
    if (mPCMDumpFile) {
        //ALOGD("%s()", __FUNCTION__);
        AudioDumpPCMData((void *)buffer, bytes, mPCMDumpFile);
    }
}

status_t AudioALSAStreamIn::start() {
    int ret = INVALID_OPERATION;
    ALOGD("+%s()", __FUNCTION__);

    AL_AUTOLOCK(mLock);

    if (mStreamAttributeTarget.mAudioInputFlags & AUDIO_INPUT_FLAG_MMAP_NOIRQ && mCaptureHandler != NULL
        && !mStandby && !mStart) {
        ret = mCaptureHandler->start();
        if (ret == 0) {
            mStart = true;
        }
    } else {
        ALOGW("%s() fail, flags %d, !mStandby %d, !mStart %d, !mCaptureHandler %d", __FUNCTION__,
              mStreamAttributeTarget.mAudioInputFlags, !mStandby, !mStart, mCaptureHandler != NULL);
    }

    ALOGD("-%s()", __FUNCTION__);
    return ret;
}

status_t AudioALSAStreamIn::stop() {
    ALOGD("+%s()", __FUNCTION__);

    int ret = INVALID_OPERATION;

    AL_AUTOLOCK(mLock);

    if (mStreamAttributeTarget.mAudioInputFlags & AUDIO_INPUT_FLAG_MMAP_NOIRQ && mCaptureHandler != NULL
        && !mStandby && mStart) {
        ret = mCaptureHandler->stop();
        if (ret == 0) {
            mStart = false;
        }
    } else {
        ALOGW("%s() fail, flags %d, !mStandby %d, mStart %d, !mCaptureHandler %d", __FUNCTION__,
              mStreamAttributeTarget.mAudioInputFlags, !mStandby, mStart, mCaptureHandler != NULL);
    }

    ALOGD("-%s()", __FUNCTION__);
    return ret;
}

status_t AudioALSAStreamIn::createMmapBuffer(int32_t min_size_frames,
                                                struct audio_mmap_buffer_info *info) {
    ALOGD("+%s(), min_size_frames %d", __FUNCTION__, min_size_frames);

    int ret = INVALID_OPERATION;

    AL_AUTOLOCK(mLock);

    if (info == NULL || min_size_frames == 0) {
        ALOGE("%s(): info = %p, min_size_frames = %d", __FUNCTION__, info, min_size_frames);
        return BAD_VALUE;
    }

    if (mStreamAttributeTarget.mAudioInputFlags & AUDIO_INPUT_FLAG_MMAP_NOIRQ && mStandby) {
        if (mCaptureHandler == NULL) {
            mCaptureHandler = mStreamManager->createCaptureHandler(&mStreamAttributeTarget);
        }

        ret = mCaptureHandler->createMmapBuffer(min_size_frames, info);
        mStandby = false;
    } else {
         ALOGW("%s() fail, flags %d, mStandby %d", __FUNCTION__,
              mStreamAttributeTarget.mAudioInputFlags, mStandby);
     }

    ALOGD("-%s()", __FUNCTION__);
    return ret;
}

status_t AudioALSAStreamIn::getMmapPosition(struct audio_mmap_position *position) {
    int ret = INVALID_OPERATION;
    //ALOGD("+%s()", __FUNCTION__);

    AL_AUTOLOCK(mLock);

    if (position == NULL || mCaptureHandler == NULL) {
        return BAD_VALUE;
    }
    if (!(mStreamAttributeTarget.mAudioInputFlags & AUDIO_INPUT_FLAG_MMAP_NOIRQ)) {
        return BAD_VALUE;
    }

    ret = mCaptureHandler->getMmapPosition(position);

    //ALOGD("-%s()", __FUNCTION__);
    return ret;
}

}

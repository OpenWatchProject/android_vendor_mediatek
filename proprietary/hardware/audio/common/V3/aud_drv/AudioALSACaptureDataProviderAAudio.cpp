
#include "AudioALSACaptureDataProviderAAudio.h"
#include "AudioALSADriverUtility.h"

#if defined(MTK_AUDIO_KS)
#include "AudioALSADeviceConfigManager.h"
#endif

#include <audio_utils/clock.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureDataProviderAAudio"


namespace android {


/*==============================================================================
 *                     Constant
 *============================================================================*/



/*==============================================================================
 *                     Utility
 *============================================================================*/



/*==============================================================================
 *                     Implementation
 *============================================================================*/

int AudioALSACaptureDataProviderAAudio::mUsageCount = 0;
AudioLock AudioALSACaptureDataProviderAAudio::mLock;


AudioALSACaptureDataProviderAAudio *AudioALSACaptureDataProviderAAudio::mAudioALSACaptureDataProviderAAudio = NULL;
AudioALSACaptureDataProviderAAudio *AudioALSACaptureDataProviderAAudio::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderAAudio == NULL) {
        ALOGD("%s, new instance", __func__);
        mAudioALSACaptureDataProviderAAudio = new AudioALSACaptureDataProviderAAudio();
    }
    ASSERT(mAudioALSACaptureDataProviderAAudio != NULL);
    return mAudioALSACaptureDataProviderAAudio;
}


AudioALSACaptureDataProviderAAudio *AudioALSACaptureDataProviderAAudio::requestInstance() {

    ALOGD("%s, mAudioALSACaptureDataProviderAAudio %p, mUsageCount %d", __func__,
          mAudioALSACaptureDataProviderAAudio, mUsageCount);

    AL_AUTOLOCK(mLock);
    mUsageCount += 1;
    AL_UNLOCK(mLock);

    return getInstance();
}


void AudioALSACaptureDataProviderAAudio::freeInstance() {
    static AudioLock mFreeInstanceLock;
    AL_AUTOLOCK(mFreeInstanceLock);

    AL_AUTOLOCK(mLock);
    mUsageCount -= 1;
    AL_UNLOCK(mLock);

    ALOGD("%s, mAudioALSACaptureDataProviderAAudio %p, mUsageCount %d", __func__,
          mAudioALSACaptureDataProviderAAudio, mUsageCount);
    if (mAudioALSACaptureDataProviderAAudio != NULL && mUsageCount == 0) {
        ALOGD("%s, delete instance", __func__);
        delete mAudioALSACaptureDataProviderAAudio;
        mAudioALSACaptureDataProviderAAudio = NULL;
    }
}


AudioALSACaptureDataProviderAAudio::AudioALSACaptureDataProviderAAudio():
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    mTime_nanoseconds(0),
    mPosition_frames(0),
    mMin_size_frames(0),
    mPmicEnable(false) {
    ALOGD("%s()", __FUNCTION__);

    memset((void *)&mCreateMmapTime, 0, sizeof(mCreateMmapTime));

    mCaptureDataProviderType = CAPTURE_PROVIDER_NORMAL;
}


AudioALSACaptureDataProviderAAudio::~AudioALSACaptureDataProviderAAudio() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderAAudio::open() {

    ALOGD("%s(+), source format %d, rate %d, ch %d", __FUNCTION__,
          mStreamAttributeSource.audio_format,
          mStreamAttributeSource.sample_rate,
          mStreamAttributeSource.num_channels);

    ASSERT(mEnable == false);

    int pcmindex = 0;
    int cardindex = 0;

#if defined(MTK_AUDIO_KS)
    pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture7);
    cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmCapture7);
    ALOGD("%s(+), cardindex = %d, pcmindex = %d", __FUNCTION__, cardindex, pcmindex);
#else
    ALOGE("%s(), MMAP only support KS!", __FUNCTION__);
    ASSERT(0);
#endif

    mConfig.channels = mStreamAttributeSource.num_channels;
    mConfig.rate = mStreamAttributeSource.sample_rate;
    mConfig.format = (mStreamAttributeSource.audio_format == AUDIO_FORMAT_PCM_16_BIT) ?
                     PCM_FORMAT_S16_LE : PCM_FORMAT_S32_LE;

    int bytesPerSample = (mStreamAttributeSource.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4;

    // period_size & period_count
    int max_framecount = MAX_MMAP_HW_BUFFER_SIZE / mConfig.channels / bytesPerSample;
    int min_framecount = MMAP_UL_PERIOD_SIZE * MIN_MMAP_PERIOD_COUNT;

    if (mMin_size_frames < min_framecount) {
        mMin_size_frames = min_framecount;
    } else if (mMin_size_frames > MAX_MMAP_FRAME_COUNT) {
        mMin_size_frames = MAX_MMAP_FRAME_COUNT;
    }

    mConfig.period_count = 2;
    mConfig.period_size = (mMin_size_frames - 1) / mConfig.period_count + 1;
    mMin_size_frames = mConfig.period_count * mConfig.period_size;

    int scenario = (mMin_size_frames <= max_framecount) ? 1 : 0;
    ALOGD("%s(), set mmap_record_scenario %d", __FUNCTION__, scenario);
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mmap_record_scenario"), 0, scenario)) {
        ALOGW("%s(), mmap_record_scenario enable fail", __FUNCTION__);
    }


    mConfig.start_threshold = 0;
    mConfig.stop_threshold = INT32_MAX;
    mConfig.silence_threshold = 0;
    ALOGD("mConfig format: %d, channels: %d, rate: %d, period_size: %d, period_count: %d, latency: %d",
          mConfig.format, mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mlatency);

#if defined(MTK_AUDIO_KS)
    mApTurnOnSequence = AUDIO_CTL_ADDA_TO_CAPTURE7;
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(mApTurnOnSequence);

#if !defined(CONFIG_MT_ENG_BUILD)
    // need to set after query pcm_params_get, since shutdown will clear this state
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "record_xrun_assert"), 0, 1)) {
        ALOGW("%s(), record_xrun_assert enable fail", __FUNCTION__);
    }
#endif
#endif

    mStreamAttributeSource.buffer_size = mConfig.period_size *  mConfig.period_count * mConfig.channels *
                                         bytesPerSample;


    // enable pcm
    if (mPcm == NULL) {
        unsigned int flag = PCM_IN | PCM_MONOTONIC | PCM_MMAP | PCM_NOIRQ;
        openPcmDriverWithFlag(pcmindex, flag);
    }

    ASSERT(mPcm != NULL && pcm_is_ready(mPcm) == true);
    ALOGV("%s(), mPcm = %p", __FUNCTION__, mPcm);

    int prepare_error = pcm_prepare(mPcm);
    if (prepare_error != 0) {
        ASSERT(0);
        pcm_close(mPcm);
        mPcm = NULL;
        return UNKNOWN_ERROR;
    }

    mStart = false;

    ALOGD("%s(-)", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureDataProviderAAudio::close() {
    ALOGD("%s()", __FUNCTION__);

#if defined(MTK_AUDIO_KS) && !defined(CONFIG_MT_ENG_BUILD)
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "record_xrun_assert"), 0, 0)) {
        ALOGW("%s(), record_xrun_assert disable fail", __FUNCTION__);
    }
#endif

    mEnable = false;

    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    pcm_stop(mPcm);
    pcm_close(mPcm);
    mPcm = NULL;

#if defined(MTK_AUDIO_KS)
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnoffSequenceByName(mApTurnOnSequence);

    ALOGD("%s(), set mmap_record_scenario 0", __FUNCTION__);
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mmap_record_scenario"), 0, 0)) {
        ALOGW("%s(), mmap_record_scenario disable fail", __FUNCTION__);
    }
#endif

    if (mPmicEnable) {
        enablePmicInputDevice(false);
        mPmicEnable = false;
    }

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureDataProviderAAudio::start() {
    ALOGD("+%s", __FUNCTION__);

    int ret = INVALID_OPERATION;

    if (mPcm == NULL) {
        ALOGW("%s, mPcm == NULL !", __FUNCTION__);
        return ret;
    }

    if (!mPmicEnable) {
        // set pmic before pcm prepare
        enablePmicInputDevice(true);
        mPmicEnable = true;
    }


    // drop glitch
    uint64_t sleepUs = 0;
    uint64_t sleepUsMax = 30 * 1000; // 30ms
    struct timespec     curTime;

    clock_gettime(CLOCK_MONOTONIC, &curTime);
    sleepUs = get_time_diff_ns(&mCreateMmapTime, &curTime) / 1000;

    if (sleepUs < sleepUsMax) {
        sleepUs = sleepUsMax - sleepUs;
        ALOGD("%s, drop glitch %ld ms", __FUNCTION__, (long)sleepUs);
        usleep(sleepUs);
    }


    ret = pcm_start(mPcm);
    if (ret < 0) {
        ALOGE("%s: pcm_start fail %d, %s", __FUNCTION__, ret, pcm_get_error(mPcm));
    }

    ALOGD("-%s", __FUNCTION__);
    return ret;
}


status_t AudioALSACaptureDataProviderAAudio::stop() {
    int ret = INVALID_OPERATION;
    ALOGD("+%s", __FUNCTION__);

    if (mPcm == NULL) {
        ALOGW("%s, mPcm == NULL !", __FUNCTION__);
        return ret;
    }

    ret = pcm_stop(mPcm);
    if (ret < 0) {
        ALOGE("%s: pcm_stop fail %d", __FUNCTION__, ret);
    }

    if (mPmicEnable) {
        enablePmicInputDevice(false);
        mPmicEnable = false;
    }

    ALOGD("-%s", __FUNCTION__);
    return ret;
}


status_t AudioALSACaptureDataProviderAAudio::createMmapBuffer(int32_t min_size_frames,
                                                              struct audio_mmap_buffer_info *info) {
    unsigned int offset = 0;
    unsigned int frames = 0;
    uint32_t buffer_size = 0;
    int ret = INVALID_OPERATION;
    ALOGD("+%s, min_size_frames %d", __FUNCTION__, min_size_frames);

    mMin_size_frames = min_size_frames;

    // open pcm
    open();

    if (mPcm == NULL) {
        ALOGW("%s, mPcm == NULL !", __FUNCTION__);
        return ret;
    }
    // drop glitch
    clock_gettime(CLOCK_MONOTONIC, &mCreateMmapTime);


    ret = pcm_mmap_begin(mPcm, &info->shared_memory_address, &offset, &frames);
    if (ret < 0)  {
        goto exit;
    }

    info->buffer_size_frames = pcm_get_buffer_size(mPcm);
    info->burst_size_frames = MMAP_UL_PERIOD_SIZE;
    buffer_size = pcm_frames_to_bytes(mPcm, info->buffer_size_frames);

    info->shared_memory_fd = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "aaudio_ul_mmap_fd"), 0);
    if (info->shared_memory_fd == 0) {
        // share mode
        info->shared_memory_fd = pcm_get_poll_fd(mPcm);
        ALOGD("%s+, shared fd %d", __FUNCTION__, info->shared_memory_fd);
    } else {
        info->buffer_size_frames *= -1;
    }

    memset(info->shared_memory_address, 0, buffer_size);

    ALOGD("%s: fd %d, buffer address %p,  buffer_size_frames %d %d, burst_size_frames %d", __FUNCTION__,
          info->shared_memory_fd, info->shared_memory_address, info->buffer_size_frames,
          buffer_size, info->burst_size_frames);

exit:
    if (ret != 0) {
        if (mPcm != NULL) {
            pcm_close(mPcm);
            mPcm = NULL;
        }
    }

    ALOGD("-%s", __FUNCTION__);
    return ret;
}


status_t AudioALSACaptureDataProviderAAudio::getMmapPosition(struct audio_mmap_position *position) {
    int ret = INVALID_OPERATION;

    if (mPcm == NULL) {
        ALOGW("%s, mPcm == NULL !", __FUNCTION__);
        return ret;
    }

    struct timespec ts = { 0, 0 };
    ret = pcm_mmap_get_hw_ptr(mPcm, (unsigned int *)&position->position_frames, &ts);
    if (ret < 0) {
        ALOGE("%s: %s", __FUNCTION__, pcm_get_error(mPcm));
        return ret;
    }
    position->time_nanoseconds = audio_utils_ns_from_timespec(&ts);

#if 0
    // correction
    if (mTime_nanoseconds == 0) {
        mTime_nanoseconds = position->time_nanoseconds;
        mPosition_frames = position->position_frames;
        ALOGD("%s, assign time_nanoseconds %lld, mPosition_frames %d", __func__, (long long)mTime_nanoseconds, mPosition_frames);
    } else {
        position->position_frames = (position->time_nanoseconds - mTime_nanoseconds) * 48 / 1000000 + mPosition_frames;
    }
#endif

    if (position->position_frames < 0) {
        ALOGD("%s, time_nanoseconds %lld, mPosition_frames %d", __FUNCTION__,
              (long long)position->time_nanoseconds, position->position_frames);
    }

    return ret;
}


} // end of namespace android

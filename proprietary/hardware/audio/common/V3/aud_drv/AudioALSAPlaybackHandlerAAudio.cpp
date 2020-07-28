
#include "AudioALSAPlaybackHandlerAAudio.h"
#include "AudioALSAHardwareResourceManager.h"

#if defined(MTK_AUDIO_KS)
#include "AudioALSADeviceConfigManager.h"
#endif

#include "AudioSmartPaController.h"
#include <audio_utils/clock.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerAAudio"



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

AudioALSAPlaybackHandlerAAudio::AudioALSAPlaybackHandlerAAudio(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source),
    mTime_nanoseconds(0),
    mPosition_frames(0),
    mMin_size_frames(0) {
    ALOGV("%s()", __FUNCTION__);

    mPlaybackHandlerType = PLAYBACK_HANDLER_FAST;
}


AudioALSAPlaybackHandlerAAudio::~AudioALSAPlaybackHandlerAAudio() {
}


status_t AudioALSAPlaybackHandlerAAudio::open() {
    ALOGD("+%s(), flag = %d, source output_devices = 0x%x, audio_format = %x, buffer_size = %d, sample_rate = %d",
          __FUNCTION__,
          mStreamAttributeSource->mAudioOutputFlags,
          mStreamAttributeSource->output_devices,
          mStreamAttributeSource->audio_format,
          mStreamAttributeSource->buffer_size,
          mStreamAttributeSource->sample_rate);


    AL_LOCK_MS(AudioALSADriverUtility::getInstance()->getStreamSramDramLock(), 3000);

    // acquire pmic clk
    mHardwareResourceManager->EnableAudBufClk(true);

    int pcmindex = 0;
    int cardindex = 0;

#if defined(MTK_AUDIO_KS)
    pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback5);
    cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback5);

    mApTurnOnSequence = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_1, AUDIO_CTL_PLAYBACK5);
    mApTurnOnSequence2 = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_2, AUDIO_CTL_PLAYBACK5);
    mHardwareResourceManager->setCustOutputDevTurnOnSeq(mStreamAttributeSource->output_devices,
                                                        mTurnOnSeqCustDev1, mTurnOnSeqCustDev2);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence2);
    mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev1);
    mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev2);
#else
    ALOGE("%s(), MMAP only support KS!", __FUNCTION__);
    ASSERT(0);
#endif

    mStreamAttributeTarget.audio_format = (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT) ?
                                          AUDIO_FORMAT_PCM_8_24_BIT : AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeTarget.audio_channel_mask = mStreamAttributeSource->audio_channel_mask;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = mStreamAttributeSource->sample_rate;


    // HW pcm config
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;

    int bytesPerSample = (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4;


    // period_size & period_count
    int max_framecount = MAX_MMAP_HW_BUFFER_SIZE / mConfig.channels / bytesPerSample;
    int min_framecount = MMAP_DL_PERIOD_SIZE * MIN_MMAP_PERIOD_COUNT;

    if (mMin_size_frames < min_framecount) {
        mMin_size_frames = min_framecount;
    }
    else if (mMin_size_frames > MAX_MMAP_FRAME_COUNT) {
        mMin_size_frames = MAX_MMAP_FRAME_COUNT;
    }

    mConfig.period_count = 2;
    mConfig.period_size = (mMin_size_frames - 1) / mConfig.period_count + 1;
    mMin_size_frames = mConfig.period_count * mConfig.period_size;

    int scenario = (mMin_size_frames <= max_framecount) ? 1 : 0;
    ALOGD("%s(), set mmap_play_scenario %d", __FUNCTION__, scenario);
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mmap_play_scenario"), 0, scenario)) {
        ALOGW("%s(), mmap_play_scenario enable fail", __FUNCTION__);
    }


    mConfig.format = transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format);

    mConfig.start_threshold = mConfig.period_size * mConfig.period_count;;
    mConfig.stop_threshold = INT32_MAX;
    mConfig.silence_threshold = 0;
    mConfig.avail_min = 0;//mStreamAttributeSource->buffer_size / ((mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4) / mStreamAttributeSource->num_channels;
    ALOGD("%s(), mConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d, avail_min = %d, start_threshold = %d",
          __FUNCTION__,
          mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mConfig.format,
          mConfig.avail_min, mConfig.start_threshold);


    mStreamAttributeTarget.buffer_size = mConfig.period_size * mConfig.period_count * mConfig.channels *
                                         bytesPerSample;

    unsigned int flag = PCM_MMAP | PCM_NOIRQ | PCM_OUT | PCM_MONOTONIC;
    openPcmDriverWithFlag(pcmindex, flag);


    AL_UNLOCK(AudioALSADriverUtility::getInstance()->getStreamSramDramLock());


#if defined(MTK_HYBRID_NLE_SUPPORT) // must be after pcm open
    mStreamAttributeTarget.output_devices = mStreamAttributeSource->output_devices;
    initNLEProcessing();
#endif

    // open codec driver
    mHardwareResourceManager->startOutputDevice(mStreamAttributeSource->output_devices, mStreamAttributeTarget.sample_rate);




    mTimeStampValid = false;
    mBytesWriteKernel = 0;

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerAAudio::close() {
    ALOGD("+%s(), mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->output_devices);


#if defined(MTK_HYBRID_NLE_SUPPORT)
    // Must do this before close analog path
    deinitNLEProcessing();
#endif

    // close codec driver
    mHardwareResourceManager->stopOutputDevice();
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev1);
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev2);

    // close pcm driver
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());
    closePcmDriver();

#if defined(MTK_AUDIO_KS)
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence2);

    ALOGD("%s(), set mmap_play_scenario 0", __FUNCTION__);
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mmap_play_scenario"), 0, 0)) {
        ALOGW("%s(), mmap_play_scenario disable fail", __FUNCTION__);
    }
#endif


    //release pmic clk
    mHardwareResourceManager->EnableAudBufClk(false);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerAAudio::routing(const audio_devices_t output_devices) {
    mHardwareResourceManager->changeOutputDevice(output_devices);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerAAudio::setScreenState(bool mode __unused, size_t buffer_size __unused, size_t reduceInterruptSize __unused, bool bforce __unused) {
    ALOGE("MMAP call %s!!", __FUNCTION__);
    return NO_ERROR;
}


ssize_t AudioALSAPlaybackHandlerAAudio::write(const void *buffer __unused, size_t bytes) {
    ALOGE("MMAP call %s!!", __FUNCTION__);
    return bytes;
}


status_t AudioALSAPlaybackHandlerAAudio::setFilterMng(AudioMTKFilterManager *pFilterMng __unused) {
    ALOGE("MMAP call %s!!", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerAAudio::start() {
    int ret = INVALID_OPERATION;
    ALOGD("+%s", __FUNCTION__);

    if (mPcm == NULL) {
        ALOGW("%s, mPcm == NULL !", __FUNCTION__);
        return ret;
    }

    ret = pcm_start(mPcm);
    if (ret < 0) {
        ALOGE("%s: pcm_start fail %d, %s", __FUNCTION__, ret, pcm_get_error(mPcm));
    }

    ALOGD("-%s", __FUNCTION__);
    return ret;
}


status_t AudioALSAPlaybackHandlerAAudio::stop() {
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

    ALOGD("-%s", __FUNCTION__);
    return ret;
}


status_t AudioALSAPlaybackHandlerAAudio::createMmapBuffer(int32_t min_size_frames,
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

    ret = pcm_mmap_begin(mPcm, &info->shared_memory_address, &offset, &frames);
    if (ret < 0)  {
        goto exit;
    }

    info->buffer_size_frames = pcm_get_buffer_size(mPcm);
    info->burst_size_frames = MMAP_DL_PERIOD_SIZE;
    buffer_size = pcm_frames_to_bytes(mPcm, info->buffer_size_frames);

    info->shared_memory_fd = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "aaudio_dl_mmap_fd"), 0);
    if (info->shared_memory_fd == 0) {
        // share mode
        info->shared_memory_fd = pcm_get_poll_fd(mPcm);
        ALOGD("%s, shared fd %d", __FUNCTION__, info->shared_memory_fd);
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


status_t AudioALSAPlaybackHandlerAAudio::getMmapPosition(struct audio_mmap_position *position) {
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

#include "AudioALSACaptureHandlerBase.h"

#include "IAudioALSACaptureDataClient.h"
#include "AudioALSADataProcessor.h"
#include "AudioALSAHardwareResourceManager.h"

//#include "AudioALSACaptureDataProvider.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureHandlerBase"

namespace android {

AudioALSACaptureHandlerBase::AudioALSACaptureHandlerBase(stream_attribute_t *stream_attribute_target) :
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()),
    mCaptureDataClient(NULL),
    mStreamAttributeTarget(stream_attribute_target),
    mSupportConcurrencyInCall(false),
    mCaptureHandlerType(CAPTURE_HANDLER_BASE),
    mDataProcessor(AudioALSADataProcessor::getInstance()),
    mIdentity(0xFFFFFFFF) {
    ALOGV("%s()", __FUNCTION__);

    init();
}


AudioALSACaptureHandlerBase::~AudioALSACaptureHandlerBase() {
    ALOGV("%s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerBase::init() {
    return NO_ERROR;
}

int64_t AudioALSACaptureHandlerBase::getRawStartFrameCount() {
    if (mCaptureDataClient == NULL) {
        return -ENODATA;
    }
    return mCaptureDataClient->getRawStartFrameCount();
}

status_t AudioALSACaptureHandlerBase::UpdateBesRecParam() {
    ALOGD("%s()", __FUNCTION__);

    return NO_ERROR;
}

bool AudioALSACaptureHandlerBase::isSupportConcurrencyInCall() {
    ALOGD("%s()", __FUNCTION__);

    return mSupportConcurrencyInCall;
}

capture_handler_t AudioALSACaptureHandlerBase::getCaptureHandlerType() {
    ALOGD("%s(), mCaptureHandlerType = %x", __FUNCTION__, mCaptureHandlerType);
    return mCaptureHandlerType;
}

int AudioALSACaptureHandlerBase::getCapturePosition(int64_t *frames, int64_t *time) {
    if (mCaptureDataClient == NULL || frames == NULL || time == NULL) {
        return -ENODATA;
    }

    return mCaptureDataClient->getCapturePosition(frames, time);
}

status_t AudioALSACaptureHandlerBase::setLowLatencyMode(bool mode, size_t kernel_buffer_size, size_t reduce_size, bool bforce) {
    if (kernel_buffer_size == 0) {
        return NO_ERROR;
    }
    if (mCaptureDataClient == NULL) {
        return NO_ERROR;
    }
    const stream_attribute_t *attribute = mCaptureDataClient->getStreamAttributeSource();
    size_t hal_frame_count = (attribute->sample_rate) * UPLINK_NORMAL_LATENCY_MS / 1000;
    if (reduce_size == 0) {
        reduce_size = hal_frame_count;
    }
    int rate = mode ? hal_frame_count : ((attribute->buffer_size / (attribute->num_channels) / ((attribute->audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4)) - reduce_size);
    ALOGD("%s, kernel_buffer_size %zu, rate %d , mode = %d , channel %d, format %d, bforce = %d", __FUNCTION__, kernel_buffer_size, rate, mode, attribute->num_channels, attribute->audio_format, bforce );
    return mHardwareResourceManager->setULInterruptRate(rate);
}

bool AudioALSACaptureHandlerBase::getStreamInReopen() {
    if (mCaptureDataClient == NULL) {
        return false;
    }
    return mCaptureDataClient->getStreamInReopen();
}

void AudioALSACaptureHandlerBase::setStreamInReopen(bool state) {
    if (mCaptureDataClient == NULL) {
        return;
    }
    return mCaptureDataClient->setStreamInReopen(state);
}

status_t AudioALSACaptureHandlerBase::start() {
    return NO_ERROR;
}

status_t AudioALSACaptureHandlerBase::stop() {
    return NO_ERROR;
}

status_t AudioALSACaptureHandlerBase::createMmapBuffer(int32_t min_size_frames __unused,
                              struct audio_mmap_buffer_info *info __unused) {
    return NO_ERROR;
}

status_t AudioALSACaptureHandlerBase::getMmapPosition(struct audio_mmap_position *position __unused) {
    return NO_ERROR;
}

} // end of namespace android


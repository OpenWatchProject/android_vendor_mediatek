#include "AudioALSACaptureHandlerAAudio.h"
#include "AudioALSAHardwareResourceManager.h"
#include "AudioALSACaptureDataProviderAAudio.h"
#include "AudioVolumeFactory.h"


#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureHandlerAAudio"

namespace android {


AudioALSACaptureHandlerAAudio::AudioALSACaptureHandlerAAudio(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target),
    mDataProvider(NULL) {
    ALOGD("%s()", __FUNCTION__);
}


AudioALSACaptureHandlerAAudio::~AudioALSACaptureHandlerAAudio() {
    ALOGD("+%s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerAAudio::init() {
    ALOGD("%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerAAudio::open() {
    ALOGD("+%s()", __FUNCTION__);

#if defined(MTK_POWERHAL_AUDIO_LATENCY)
    if (mStreamAttributeSource.mPowerHalEnable) {
        power_hal_hint(POWERHAL_LATENCY_UL, true);
    }
#endif


    mDataProvider = AudioALSACaptureDataProviderAAudio::getInstance();

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerAAudio::close() {
    ALOGD("+%s()", __FUNCTION__);

    mDataProvider->close();
    AudioALSACaptureDataProviderAAudio::freeInstance();

#if defined(MTK_POWERHAL_AUDIO_LATENCY)
    if (mStreamAttributeSource.mPowerHalEnable) {
        power_hal_hint(POWERHAL_LATENCY_UL, false);
    }
#endif

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerAAudio::routing(const audio_devices_t input_device) {
    ALOGD("+%s()", __FUNCTION__);

    mHardwareResourceManager->changeInputDevice(input_device);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


ssize_t AudioALSACaptureHandlerAAudio::read(void *buffer __unused, ssize_t bytes __unused) {
    ALOGD("%s()", __FUNCTION__);
    return bytes;
}


status_t AudioALSACaptureHandlerAAudio::start() {
    ALOGD("+%s", __FUNCTION__);

    int ret = INVALID_OPERATION;
    if (mDataProvider) {
        ret = mDataProvider->start();
    }

    ALOGD("-%s", __FUNCTION__);
    return ret;
}


status_t AudioALSACaptureHandlerAAudio::stop() {
    ALOGD("+%s", __FUNCTION__);

    int ret = INVALID_OPERATION;
    if (mDataProvider) {
        ret = mDataProvider->stop();
    }

    ALOGD("-%s", __FUNCTION__);
    return ret;
}


status_t AudioALSACaptureHandlerAAudio::createMmapBuffer(int32_t min_size_frames,
                                                            struct audio_mmap_buffer_info *info) {
    ALOGD("+%s", __FUNCTION__);

    int ret = INVALID_OPERATION;

    // create AudioALSACaptureDataProviderAAudio
    open();

    // apply gain
    mStreamAttributeTarget->input_source = AUDIO_SOURCE_AAUDIO;
    AudioVolumeInterface *pVolume = AudioVolumeFactory::CreateAudioVolumeController();
    pVolume->SetCaptureGain(mStreamAttributeTarget->audio_mode, mStreamAttributeTarget->input_source,
                            mStreamAttributeTarget->input_device, mStreamAttributeTarget->output_devices);

    if (mDataProvider) {
        // config attribute for input device
        mDataProvider->configStreamAttribute(mStreamAttributeTarget);

        // create mmap buffer
        ret = mDataProvider->createMmapBuffer(min_size_frames, info);
    }

    ALOGD("-%s", __FUNCTION__);
    return ret;
}


status_t AudioALSACaptureHandlerAAudio::getMmapPosition(struct audio_mmap_position *position) {
    int ret = INVALID_OPERATION;
    //ALOGD("+%s", __FUNCTION__);

    if (mDataProvider) {
        ret = mDataProvider->getMmapPosition(position);
    }

    //ALOGD("-%s", __FUNCTION__);
    return ret;
}


} // end of namespace android

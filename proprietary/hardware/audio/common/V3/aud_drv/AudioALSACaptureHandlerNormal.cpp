#include "AudioALSACaptureHandlerNormal.h"

#include "AudioALSAHardwareResourceManager.h"
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include "AudioALSACaptureDataClientAurisysNormal.h"
#else
#include "AudioALSACaptureDataClient.h"
#endif
#include "AudioALSACaptureDataProviderNormal.h"

#include "AudioSmartPaController.h"
#include "AudioALSACaptureDataProviderEchoRefExt.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureHandlerNormal"

namespace android {

//static FILE *pOutFile = NULL;

AudioALSACaptureHandlerNormal::AudioALSACaptureHandlerNormal(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target) {
    ALOGD("%s()", __FUNCTION__);

    init();
}


AudioALSACaptureHandlerNormal::~AudioALSACaptureHandlerNormal() {
    ALOGV("+%s()", __FUNCTION__);
    ALOGV("%-s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerNormal::init() {
    ALOGD("%s()", __FUNCTION__);
    mCaptureHandlerType = CAPTURE_HANDLER_NORMAL;
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerNormal::open() {
    ALOGD("+%s(), input_device = 0x%x, input_source = 0x%x, sample_rate=%d, num_channels=%d",
          __FUNCTION__, mStreamAttributeTarget->input_device, mStreamAttributeTarget->input_source, mStreamAttributeTarget->sample_rate,
          mStreamAttributeTarget->num_channels);

#if defined(MTK_POWERHAL_AUDIO_LATENCY)
    if (mStreamAttributeTarget->mAudioInputFlags & AUDIO_INPUT_FLAG_FAST
        || mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION
        || mStreamAttributeTarget->audio_mode == AUDIO_MODE_IN_COMMUNICATION) {
        power_hal_hint(POWERHAL_LATENCY_UL, true);
        ALOGD("Power hal enabled for UL\n");
    }
#endif

    ASSERT(mCaptureDataClient == NULL);
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if(!AudioSmartPaController::getInstance()->isInCalibration()) {
        mCaptureDataClient = new AudioALSACaptureDataClientAurisysNormal(AudioALSACaptureDataProviderNormal::getInstance(), mStreamAttributeTarget, NULL); // NULL: w/o AEC
    } else {
        mCaptureDataClient = new AudioALSACaptureDataClientAurisysNormal(AudioALSACaptureDataProviderEchoRefExt::getInstance(), mStreamAttributeTarget, NULL); // NULL: w/o AEC
    }
#else
    if(!AudioSmartPaController::getInstance()->isInCalibration()) {
        mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderNormal::getInstance(), mStreamAttributeTarget);
    } else {
        mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderEchoRefExt::getInstance(), mStreamAttributeTarget);
    }
#endif

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerNormal::close() {
    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mCaptureDataClient != NULL);
    delete mCaptureDataClient;

#if defined(MTK_POWERHAL_AUDIO_LATENCY)
    if (mStreamAttributeTarget->mAudioInputFlags & AUDIO_INPUT_FLAG_FAST) {
        power_hal_hint(POWERHAL_LATENCY_UL, false);
    }
#endif

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerNormal::routing(const audio_devices_t input_device) {
    mHardwareResourceManager->changeInputDevice(input_device);
    return NO_ERROR;
}


ssize_t AudioALSACaptureHandlerNormal::read(void *buffer, ssize_t bytes) {
    ALOGV("%s()", __FUNCTION__);

    bytes = mCaptureDataClient->read(buffer, bytes);
#if 0   //remove dump here which might cause process too long due to SD performance
    if (pOutFile != NULL) {
        fwrite(buffer, sizeof(char), bytes, pOutFile);
    }
#endif

    return bytes;
}

} // end of namespace android

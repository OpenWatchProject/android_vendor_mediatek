
#if defined(MTK_SPEAKER_MONITOR_SPEECH_SUPPORT)

#include "AudioALSACaptureHandlerModemDai.h"

#include "AudioALSAHardwareResourceManager.h"

#include "AudioALSACaptureDataClient.h"
#include "AudioALSACaptureDataProviderModemDai.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureHandlerModemDai"

namespace android {

static FILE *pOutFile = NULL;

AudioALSACaptureHandlerModemDai::AudioALSACaptureHandlerModemDai(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target) {
    ALOGD("%s()", __FUNCTION__);
    init();
}


AudioALSACaptureHandlerModemDai::~AudioALSACaptureHandlerModemDai() {
    ALOGD("%s()+", __FUNCTION__);
}

status_t AudioALSACaptureHandlerModemDai::init() {
    ALOGD("%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSACaptureHandlerModemDai::open() {
    ALOGD("+%s(), input_device = 0x%x, input_source = 0x%x",
          __FUNCTION__, mStreamAttributeTarget->input_device, mStreamAttributeTarget->input_source);

    ASSERT(mCaptureDataClient == NULL);
    mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderModemDai::getInstance(), mStreamAttributeTarget);

    //pOutFile = fopen("/data/vendor/audiohal/SpkMonitor_MD.pcm", "wb");

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerModemDai::close() {
    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mCaptureDataClient != NULL);
    delete mCaptureDataClient;
    if (pOutFile != NULL) {
        fclose(pOutFile);
    }

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerModemDai::routing(const audio_devices_t input_device) {
    //mHardwareResourceManager->changeInputDevice(input_device);
    //Do not routing. It should be fixed.
    return NO_ERROR;
}


ssize_t AudioALSACaptureHandlerModemDai::read(void *buffer, ssize_t bytes) {
    ALOGV("%s()", __FUNCTION__);

    mCaptureDataClient->read(buffer, bytes);
    if (pOutFile != NULL) {
        fwrite(buffer, sizeof(char), bytes, pOutFile);
    }
    return bytes;
}

} // end of namespace android
#endif //end of defined(MTK_SPEAKER_MONITOR_SPEECH_SUPPORT)

#include "AudioALSACaptureHandlerFMRadio.h"

#include "AudioALSACaptureDataClient.h"
#include "AudioALSACaptureDataProviderFMRadio.h"
#include "AudioALSAStreamManager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif


#define LOG_TAG "AudioALSACaptureHandlerFMRadio"

namespace android {

AudioALSACaptureHandlerFMRadio::AudioALSACaptureHandlerFMRadio(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target) {
    ALOGD("%s()", __FUNCTION__);
    mSupportConcurrencyInCall = true;
    init();
}


AudioALSACaptureHandlerFMRadio::~AudioALSACaptureHandlerFMRadio() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerFMRadio::init() {
    ALOGD("%s()", __FUNCTION__);
    mCaptureHandlerType = CAPTURE_HANDLER_FM_RADIO;

    return NO_ERROR;
}


status_t AudioALSACaptureHandlerFMRadio::open() {
    ALOGD("+%s(), input_device = 0x%x, input_source = 0x%x",
          __FUNCTION__, mStreamAttributeTarget->input_device, mStreamAttributeTarget->input_source);

    //For Google FM, app keeps recording when entering into InCall or InCommunication Mode
    if (mSupportConcurrencyInCall == false && (AudioALSAStreamManager::getInstance()->isPhoneCallOpen() == true || AudioALSAStreamManager::getInstance()->isModeInVoipCall() == true)) {
        mCaptureDataClient = NULL;
        ALOGD("-%s() don't support FM Record at incall mode", __FUNCTION__);
        return NO_ERROR;
    } else if (AudioALSAStreamManager::getInstance()->getFmEnable() == false) { //For Google FM, app keeps recording when entering into Normal Mode from InCall
        ALOGW("StreamIn resume FM enable (App keep reading,howerver HAL disable FM for InCall)");
        AudioALSAStreamManager::getInstance()->setFmEnable(true, true, false);
    }
    // TODO(Harvey): check FM is already opened?

    ASSERT(mCaptureDataClient == NULL);
    mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderFMRadio::getInstance(), mStreamAttributeTarget);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerFMRadio::close() {
    ALOGD("+%s()", __FUNCTION__);

    if (mCaptureDataClient != NULL) {
        //ASSERT(mCaptureDataClient != NULL);
        delete mCaptureDataClient;
    }

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerFMRadio::routing(const audio_devices_t input_device __unused) {
    WARNING("Not support!!");
    return INVALID_OPERATION;
}


ssize_t AudioALSACaptureHandlerFMRadio::read(void *buffer, ssize_t bytes) {
    ALOGV("%s()", __FUNCTION__);

    if (mCaptureDataClient == NULL) {
        memset(buffer, 0, bytes);
        size_t wordSize = 0;
        switch (mStreamAttributeTarget->audio_format) {
        case AUDIO_FORMAT_PCM_8_BIT: {
            wordSize = sizeof(int8_t);
            break;
        }
        case AUDIO_FORMAT_PCM_16_BIT: {
            wordSize = sizeof(int16_t);
            break;
        }
        case AUDIO_FORMAT_PCM_8_24_BIT:
        case AUDIO_FORMAT_PCM_32_BIT: {
            wordSize = sizeof(int32_t);
            break;
        }
        default: {
            wordSize = sizeof(int16_t);
            ALOGW("%s(), wrong format(0x%x), default use wordSize = %zu", __FUNCTION__, mStreamAttributeTarget->audio_format, wordSize);
            break;
        }
        }
        size_t sleepus = ((bytes * 1000) / ((mStreamAttributeTarget->sample_rate / 1000) * mStreamAttributeTarget->num_channels * wordSize));
        ALOGD("%s(), sleepus = %zu", __FUNCTION__, sleepus);
        usleep(sleepus);
        return bytes;
    }

    return mCaptureDataClient->read(buffer, bytes);
}

} // end of namespace android

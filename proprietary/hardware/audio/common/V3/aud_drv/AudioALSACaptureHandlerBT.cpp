#include "AudioALSACaptureHandlerBT.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include "AudioALSACaptureDataClientAurisysNormal.h"
#else
#include "AudioALSACaptureDataClient.h"
#endif
#include "AudioALSACaptureDataProviderBTSCO.h"
#include "AudioALSACaptureDataProviderBTCVSD.h"

#include "WCNChipController.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureHandlerBT"

namespace android {

AudioALSACaptureHandlerBT::AudioALSACaptureHandlerBT(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target) {
    ALOGD("%s()", __FUNCTION__);
    // Should reopen to routing to use HandlerVoice when BTCVSD
    mSupportConcurrencyInCall = false;
    init();
}


AudioALSACaptureHandlerBT::~AudioALSACaptureHandlerBT() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerBT::init() {
    ALOGD("%s()", __FUNCTION__);
    mCaptureHandlerType = CAPTURE_HANDLER_BT;
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerBT::open() {
    ALOGD("+%s(), input_device = 0x%x, input_source = 0x%x",
          __FUNCTION__, mStreamAttributeTarget->input_device, mStreamAttributeTarget->input_source);

    ASSERT(mCaptureDataClient == NULL);
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (WCNChipController::GetInstance()->IsBTMergeInterfaceSupported() == true) {
        mCaptureDataClient = new AudioALSACaptureDataClientAurisysNormal(AudioALSACaptureDataProviderBTSCO::getInstance(), mStreamAttributeTarget, NULL);
    } else {
        mCaptureDataClient = new AudioALSACaptureDataClientAurisysNormal(AudioALSACaptureDataProviderBTCVSD::getInstance(), mStreamAttributeTarget, NULL);
    }
#else
    if (WCNChipController::GetInstance()->IsBTMergeInterfaceSupported() == true) {
        mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderBTSCO::getInstance(), mStreamAttributeTarget);
    } else {
        mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderBTCVSD::getInstance(), mStreamAttributeTarget);
    }
#endif

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerBT::close() {
    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mCaptureDataClient != NULL);
    delete mCaptureDataClient;

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerBT::routing(const audio_devices_t input_device __unused) {
    WARNING("Not support!!");
    return INVALID_OPERATION;
}


ssize_t AudioALSACaptureHandlerBT::read(void *buffer, ssize_t bytes) {
    ALOGV("%s()", __FUNCTION__);

    return mCaptureDataClient->read(buffer, bytes);
}

} // end of namespace android

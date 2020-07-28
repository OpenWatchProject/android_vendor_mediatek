#include "AudioALSACaptureHandlerAEC.h"

#include "AudioALSAHardwareResourceManager.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include "AudioALSACaptureDataClientAurisysNormal.h"
#endif
#include "AudioALSACaptureDataClient.h"
#include "AudioALSACaptureDataProviderNormal.h"
#include "AudioALSACaptureDataProviderBTSCO.h"
#include "AudioALSACaptureDataProviderBTCVSD.h"
#include "AudioALSACaptureDataProviderEchoRef.h"
#include "AudioALSACaptureDataProviderEchoRefBTCVSD.h"
#include "AudioALSACaptureDataProviderEchoRefBTSCO.h"
#include "AudioALSACaptureDataProviderEchoRefExt.h"

#include "AudioSmartPaController.h"

#include "WCNChipController.h"

#ifdef MTK_LATENCY_DETECT_PULSE
#include "AudioDetectPulse.h"
#endif



#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSACaptureHandlerAEC"

namespace android {

//static FILE *pOutFile = NULL;

AudioALSACaptureHandlerAEC::AudioALSACaptureHandlerAEC(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target) {
    ALOGD("%s()", __FUNCTION__);

    init();
}


AudioALSACaptureHandlerAEC::~AudioALSACaptureHandlerAEC() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerAEC::init() {
    mCaptureHandlerType = CAPTURE_HANDLER_AEC;
    memset(&mStreamAttributeTargetEchoRef, 0, sizeof(mStreamAttributeTargetEchoRef));
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerAEC::open() {

    ALOGD("+%s(), input_device = 0x%x, input_source = 0x%x, sample_rate=%d, num_channels=%d, output_devices=0x%x", __FUNCTION__,
          mStreamAttributeTarget->input_device,
          mStreamAttributeTarget->input_source,
          mStreamAttributeTarget->sample_rate,
          mStreamAttributeTarget->num_channels,
          mStreamAttributeTarget->output_devices);

    ASSERT(mCaptureDataClient == NULL);

#if 0   //enable for check EchoRef data is correct
    //mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderNormal::getInstance(), mStreamAttributeTarget);
    //mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderEchoRef::getInstance(), mStreamAttributeTarget);
    mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderEchoRefExt::getInstance(), mStreamAttributeTarget);
#else

    /* get data provider singleton for mic source */
    AudioALSACaptureDataProviderBase *pDataProvider = NULL;

    if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
        // open BT  data provider
        if (WCNChipController::GetInstance()->IsBTMergeInterfaceSupported() == true) {
            pDataProvider = AudioALSACaptureDataProviderBTSCO::getInstance();
        } else {
            pDataProvider = AudioALSACaptureDataProviderBTCVSD::getInstance();
        }
    } else {
        pDataProvider = AudioALSACaptureDataProviderNormal::getInstance();
    }
    ASSERT(pDataProvider != NULL);


    /* get data provider singleton for echo ref */
    AudioALSACaptureDataProviderBase *pDataProviderEchoRef = NULL;

    if (AudioSmartPaController::getInstance()->isHwDspSpkProtect(mStreamAttributeTarget->output_devices)) {
        pDataProviderEchoRef = AudioALSACaptureDataProviderEchoRefExt::getInstance();
    } else if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
        // open BT echoref data provider
        if (WCNChipController::GetInstance()->IsBTMergeInterfaceSupported() == true) {
            pDataProviderEchoRef = AudioALSACaptureDataProviderEchoRefBTSCO::getInstance();
        } else {
            pDataProviderEchoRef = AudioALSACaptureDataProviderEchoRefBTCVSD::getInstance();
        }
    } else {
        pDataProviderEchoRef = AudioALSACaptureDataProviderEchoRef::getInstance();
    }

    ASSERT(pDataProviderEchoRef != NULL);


    /* new data client */
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    mCaptureDataClient = new AudioALSACaptureDataClientAurisysNormal(
        pDataProvider,
        mStreamAttributeTarget,
        pDataProviderEchoRef);
#else
    mCaptureDataClient = new AudioALSACaptureDataClient(pDataProvider, mStreamAttributeTarget);
    if (mCaptureDataClient != NULL) {
        memcpy(&mStreamAttributeTargetEchoRef, mStreamAttributeTarget, sizeof(stream_attribute_t));
        mCaptureDataClient->AddEchoRefDataProvider(pDataProviderEchoRef, &mStreamAttributeTargetEchoRef);
    }
#endif
#endif //end of "enable for check EchoRef data is correct"

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerAEC::close() {
    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mCaptureDataClient != NULL);
    delete mCaptureDataClient;
    mCaptureDataClient = NULL;

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerAEC::routing(const audio_devices_t input_device) {
    if (mStreamAttributeTarget->output_devices & AUDIO_DEVICE_OUT_ALL_SCO) {
        mStreamAttributeTarget->input_device = AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET;
    }

    mHardwareResourceManager->changeInputDevice(input_device);
    return NO_ERROR;
}


ssize_t AudioALSACaptureHandlerAEC::read(void *buffer, ssize_t bytes) {
    ALOGV("%s()", __FUNCTION__);

    mCaptureDataClient->read(buffer, bytes);

#ifdef MTK_LATENCY_DETECT_PULSE
    AudioDetectPulse::doDetectPulse(TAG_CAPTURE_HANDLER, PULSE_LEVEL, 0, (void *)buffer,
                                    bytes, mStreamAttributeTarget->audio_format,
                                    mStreamAttributeTarget->num_channels,
                                    mStreamAttributeTarget->sample_rate);
#endif

#if 0
    if (pOutFile != NULL) {
        fwrite(buffer, sizeof(char), bytes, pOutFile);
    }
#endif

    return bytes;
}

status_t AudioALSACaptureHandlerAEC::UpdateBesRecParam() {
    ALOGD("+%s()", __FUNCTION__);
    if (mCaptureDataClient != NULL) {
        mCaptureDataClient->UpdateBesRecParam();
    }
    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

} // end of namespace android

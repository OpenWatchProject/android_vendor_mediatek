#include "AudioExternWrapper.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include "AudioALSACaptureDataClientAurisysNormal.h"
#else
#include "AudioALSACaptureDataClient.h"
#endif

namespace android {
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
extern "C" IAudioALSACaptureDataClient *createMTKAudioDataClient(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target, AudioALSACaptureDataProviderBase *pCaptureDataProviderEchoRef) {
    IAudioALSACaptureDataClient *pCaptureDataClient = NULL;
    pCaptureDataClient = new AudioALSACaptureDataClientAurisysNormal(pCaptureDataProvider, stream_attribute_target, pCaptureDataProviderEchoRef);
    return pCaptureDataClient;
}
#else
extern "C" IAudioALSACaptureDataClient *createMTKAudioDataClient(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target) {
    IAudioALSACaptureDataClient *pCaptureDataClient = NULL;
    pCaptureDataClient = new AudioALSACaptureDataClient(pCaptureDataProvider, stream_attribute_target);
    return pCaptureDataClient;
}
#endif

extern "C" AudioALSACaptureDataProviderUsb *createMTKAudioUSBProvider() {
    return AudioALSACaptureDataProviderUsb::getInstance();
}

extern "C" AudioALSACaptureDataProviderEchoRefUsb *createMTKAudioUSBProviderEchoRef() {
    return AudioALSACaptureDataProviderEchoRefUsb::getInstance();
}

extern "C" AudioALSAPlaybackHandlerUsb *createMTKAudioUSBPlaybackHandler(stream_attribute_t *stream_attribute_source) {
    AudioALSAPlaybackHandlerUsb *mAudioALSAPlaybackHandlerUsb = new AudioALSAPlaybackHandlerUsb(stream_attribute_source);
    return mAudioALSAPlaybackHandlerUsb;

}

}

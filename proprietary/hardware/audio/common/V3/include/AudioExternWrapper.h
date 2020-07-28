
#include "IAudioALSACaptureDataClient.h"
#include "AudioALSACaptureDataProviderUsb.h"
#include "AudioALSACaptureDataProviderEchoRefUsb.h"
#include "AudioALSAPlaybackHandlerUsb.h"
#include "AudioALSACaptureDataProviderBase.h"



#include "AudioType.h"
#include "AudioUtility.h"


namespace android {
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
extern "C" IAudioALSACaptureDataClient *createMTKAudioDataClient(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target, AudioALSACaptureDataProviderBase *pCaptureDataProviderEchoRef);
#else
extern "C" IAudioALSACaptureDataClient *createMTKAudioDataClient(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target);
#endif
extern "C" AudioALSACaptureDataProviderUsb *createMTKAudioUSBProvider();
extern "C" AudioALSACaptureDataProviderEchoRefUsb *createMTKAudioUSBProviderEchoRef();
extern "C" AudioALSAPlaybackHandlerUsb *createMTKAudioUSBPlaybackHandler(stream_attribute_t *stream_attribute_source);

} // end namespace android
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
typedef android::IAudioALSACaptureDataClient *create_AudioMTKDataClient(android::AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target, android::AudioALSACaptureDataProviderBase *pCaptureDataProviderEchoRef);
#else
typedef android::IAudioALSACaptureDataClient *create_AudioMTKDataClient(android::AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target);
#endif

typedef android::AudioALSACaptureDataProviderUsb *create_AudioMTKUSBProvider();
typedef android::AudioALSACaptureDataProviderEchoRefUsb *create_AudioMTKUSBProviderEchoRef();
typedef android::AudioALSAPlaybackHandlerUsb *create_AudioMTKUSBPlaybackHandler(stream_attribute_t *stream_attribute_source);



#include "AudioVolumeFactory.h"

AudioVolumeInterface *AudioVolumeFactory::CreateAudioVolumeController() {
    // here can create diffeerent volumecontroller base on differemt platform or policy
    AudioVolumeInterface *mInstance = NULL;
#ifdef MTK_AUDIO_GAIN_TABLE
    mInstance = android::AudioMTKGainController::getInstance();
#else
    mInstance = android::AudioALSAVolumeController::getInstance();
#endif
    return mInstance;
}

void DestroyAudioVolumeController() {
    //here to destroy
}
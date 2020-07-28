#ifndef AUDIO_VOLUME_FACTORY_H
#define AUDIO_VOLUME_FACTORY_H

#ifdef MTK_AUDIO_GAIN_TABLE

#ifdef MTK_NEW_VOL_CONTROL
#include "AudioALSAGainController.h"
#else
#include "AudioMTKGainController.h"
#endif

#else
#include "AudioALSAVolumeController.h"
#endif

#include "AudioVolumeInterface.h"

class AudioVolumeFactory {
public:
    // here to implement create and
    static AudioVolumeInterface *CreateAudioVolumeController();
    static void DestroyAudioVolumeController();
private:
};

#endif

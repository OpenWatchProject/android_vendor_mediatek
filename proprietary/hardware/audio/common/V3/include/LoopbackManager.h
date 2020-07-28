#ifndef ANDROID_LOOPBACK_MANAGER_H
#define ANDROID_LOOPBACK_MANAGER_H

#include <utils/threads.h>

#include "AudioType.h"
#include <SpeechType.h>

#include "AudioVolumeInterface.h"

namespace android {

// for loopback
enum loopback_t {
    NO_LOOPBACK                                 = 0,

    // AFE Loopback
    AP_MAIN_MIC_AFE_LOOPBACK                    = 1,
    AP_HEADSET_MIC_AFE_LOOPBACK                 = 2,
    AP_REF_MIC_AFE_LOOPBACK                     = 3,
    AP_3RD_MIC_AFE_LOOPBACK                     = 4,

    // Acoustic Loopback
    MD_MAIN_MIC_ACOUSTIC_LOOPBACK               = 21,
    MD_HEADSET_MIC_ACOUSTIC_LOOPBACK            = 22,
    MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR  = 23,
    MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR     = 24,
    MD_REF_MIC_ACOUSTIC_LOOPBACK                = 25,
    MD_3RD_MIC_ACOUSTIC_LOOPBACK                = 26,

    // BT Loopback with codec
    AP_BT_LOOPBACK                              = 30,
    MD_BT_LOOPBACK                              = 31,

    // BT Loopback without codec
    AP_BT_LOOPBACK_NO_CODEC                     = 32,
    MD_BT_LOOPBACK_NO_CODEC                     = 33,
};

enum loopback_output_device_t {
    LOOPBACK_OUTPUT_RECEIVER = 1,
    LOOPBACK_OUTPUT_EARPHONE = 2,
    LOOPBACK_OUTPUT_SPEAKER  = 3,
};

enum loopback_ipc_status_t {
    LOOPBACK_STATE_READY = 0,
    LOOPBACK_STATE_PROC_ON = 1,
    LOOPBACK_STATE_PROC_OFF  = 2,
};

//class AudioVolumeInterface;

class LoopbackManager {
public:
    virtual ~LoopbackManager();

    static LoopbackManager *GetInstance();

    loopback_t GetLoopbackType();

    status_t SetLoopbackOn(loopback_t loopback_type, loopback_output_device_t loopback_output_device);
    status_t SetLoopbackOff();

    inline bool CheckIsModemLoopback(const loopback_t loopback_type) {
        if (loopback_type == MD_MAIN_MIC_ACOUSTIC_LOOPBACK ||
            loopback_type == MD_HEADSET_MIC_ACOUSTIC_LOOPBACK ||
            loopback_type == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR ||
            loopback_type == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR ||
            loopback_type == MD_REF_MIC_ACOUSTIC_LOOPBACK ||
            loopback_type == MD_3RD_MIC_ACOUSTIC_LOOPBACK ||
            loopback_type == MD_BT_LOOPBACK ||
            loopback_type == MD_BT_LOOPBACK_NO_CODEC) {
            return true;
        } else {
            return false;
        }
    }

    loopback_t mLoopbackType;

protected:
    LoopbackManager();

    status_t CheckLoopbackTypeIsValid(loopback_t loopback_type);

    audio_devices_t GetInputDeviceByLoopbackType(loopback_t loopback_type);
    audio_devices_t GetOutputDeviceByLoopbackType(loopback_t loopback_type, loopback_output_device_t loopback_output_device);

    AudioVolumeInterface *mAudioALSAVolumeController;

    Mutex mLock;


    audio_devices_t mInputDeviceCopy;
    audio_devices_t mOutputDeviceCopy;

    float mVoiceVolumeCopy;
    float mMasterVolumeCopy;

    sph_enh_mask_struct_t mMaskCopy;

    modem_index_t mWorkingModemIndex;

    bool mBtHeadsetNrecOnCopy;
    bool mLpbkRestoreVoiceVolume;
    bool mLpbkRestoreMasterVolume;

private:
    static LoopbackManager *mLoopbackManager; // singleton

};

} // end namespace android

#endif // end of ANDROID_LOOPBACK_MANAGER_H

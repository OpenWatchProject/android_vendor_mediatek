#ifndef ANDROID_AUDIO_SCP_PHONE_CALL_CONTROLLER_H
#define ANDROID_AUDIO_SCP_PHONE_CALL_CONTROLLER_H

#include <system/audio.h>
#include <pthread.h>
#include <vector>
#include <string>

extern "C" {
#include <alsa_device_profile.h>
#include <alsa_device_proxy.h>
}
#include <AudioLock.h>
#include "AudioType.h"

struct mixer;
struct pcm_config;

namespace android {

class AudioSCPPhoneCallController {
public:
    static AudioSCPPhoneCallController *getInstance();
    ~AudioSCPPhoneCallController();

    int enable(unsigned int speechRate, const audio_devices_t inputDevice);
    int disable();
    bool isEnable();
    bool deviceSupport(const audio_devices_t output_devices);
    bool isSupportPhonecall(const audio_devices_t output_devices);

    int setUSBOutConnectionState(audio_devices_t devices, bool connect, int card, int device);
    int setUSBInConnectionState(audio_devices_t devices, bool connect, int card, int device);
    unsigned int getSpeechRate();

    int closeScpSpkHwPcm();
    int openScpSpkPcmDriverWithFlag(const unsigned int device,
                                    unsigned int flag);
    int openScpSpkPcmDriver(const unsigned int mdUlDevice,
                            const unsigned int dlDevice,
                            const unsigned int ivDevice);
    int openScpSpkHwPcm();
    int speechULPhoneMicPath(bool enable);

private:
    static AudioSCPPhoneCallController *mSCPPhoneCallController;
    AudioSCPPhoneCallController();

    unsigned int getPeriodByte(const struct pcm_config *config);
    unsigned int getPcmAvail(struct pcm *pcm);
    int  initSmartPaConfig();

    // debug
    static void setSCPDebugInfo(bool enable, int dbgType);
    int setPcmDump(bool benable);

private:
    AudioLock mLock;
    bool mEnable;
    unsigned int mSpeechRate;
    int mModemIndex;
    audio_devices_t mInputDevice;
    struct pcm_config mConfig;
    struct pcm_config mScpSpkHwConfig;
    struct pcm *mPcmMicIn;
    struct pcm *mPcmMicOut;
    struct pcm *mScpSpkPcmIn;
    struct pcm *mScpSpkDlHwPcm;
    struct pcm *mScpSpkIvHwPcm;
    struct pcm *mScpSpkMdUlHwPcm;
    static struct mixer *mMixer;
    struct timespec mLpbkNewTime, mLpbkOldTime, mLpbkStartTime;

    String8 mApTurnOnSequence;
    String8 mApTurnOnSequence2;
};

}
#endif


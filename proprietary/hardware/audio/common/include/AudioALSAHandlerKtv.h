#ifndef ANDROID_AUDIO_ALSA_HANDLER_KTV_H
#define ANDROID_AUDIO_ALSA_HANDLER_KTV_H

#include <tinyalsa/asoundlib.h>
#include "AudioType.h"
#include "AudioUtility.h"
#include "AudioALSADeviceParser.h"

#ifdef MTK_AUDIO_SCP_SUPPORT
struct ipi_msg_t;
#endif

#include "AudioDspType.h"
#include "audio_task.h"
struct aurisys_lib_manager_t;
struct aurisys_dsp_config_t;

namespace android {
class AudioALSAHardwareResourceManager;
class AudioMessengerIPI;

/// Observer pattern: Subject
class AudioALSAHandlerKtv {
public:
    virtual ~AudioALSAHandlerKtv();
    static AudioALSAHandlerKtv *getInstance();

    bool getTaskKtvStatus(void);
    void enableKtvTask(bool on);
    void recoveryKtvStatus();
    void startKtvTask(stream_attribute_t *attribute);
    void stopKtvTask(void);

protected:
    AudioALSAHandlerKtv();
    AudioMessengerIPI *mAudioMessengerIPI;

    String8 mDumpFileNameDsp;
    FILE *mPCMDumpFileDsp;
    FILE *pcmin_dump_array[DEBUG_PCMDUMP_NUM];
    static uint32_t mDumpFileNumDsp;

    /* dsp pcm dump */
    void get_task_pcmdump_info(int task_id, int param1, void **pcm_dump);
    void set_task_pcmdump_info(int task_id, int param1, void *pcm_dump);
    void processDmaMsg(struct ipi_msg_t *msg,
                       void *buf,
                       uint32_t size);
    static void processDmaMsgWrapper(struct ipi_msg_t *msg,
                                     void *buf,
                                     uint32_t size,
                                     void *arg);
    void openPCMDumpKtv(void);
    void closePCMDumpKtv(void);

    struct pcm_config mKtvUlConfig;
    struct pcm_config mKtvDlConfig;
    struct pcm_config mDspKtvConfig;
    struct pcm *mDspKtvPcm;
    struct pcm *mKtvUlPcm;
    struct pcm *mKtvDlPcm;
    int mDspKtvIndex;
    int mKtvUlindex;
    int mKtvDlindex;
    int mStreamCardIndex;
    struct aurisys_lib_manager_t *mAurisysLibManagerKtv;
    struct aurisys_dsp_config_t *mAurisysDspConfigKtv;
    bool mTaskKtvActive;
    struct mixer *mMixer;

private:
    static AudioALSAHandlerKtv *mHandlerKtv;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_HANDLER_KTV_H

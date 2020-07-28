#ifndef ANDROID_AUDIO_DSP_STREAM_MANAGER_H
#define ANDROID_AUDIO_DSP_STREAM_MANAGER_H

#include <utils/threads.h>
#include <utils/KeyedVector.h>
#ifdef MTK_BASIC_PACKAGE
#include "AudioTypeExt.h"
#endif

#include <hardware_legacy/AudioMTKHardwareInterface.h>

#include <tinyalsa/asoundlib.h>
#include "AudioType.h"
#include "AudioLock.h"
#include "AudioPolicyParameters.h"
#include "AudioDspType.h"


#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
struct aurisys_lib_manager_t;
struct aurisys_dsp_config_t;
struct aurisys_gain_config_t;
struct aurisys_lib_manager_config_t;
#endif


namespace android {

class AudioALSAPlaybackHandlerBase;
class AudioALSACaptureHandlerBase;
class AudioMessengerIPI;

class AudioDspStreamManager {
public:
    virtual ~AudioDspStreamManager();
    static AudioDspStreamManager *getInstance();

    int addPlaybackHandler(AudioALSAPlaybackHandlerBase *playbackHandler);
    int removePlaybackHandler(AudioALSAPlaybackHandlerBase *playbackHandler);

    int addCaptureHandler(AudioALSACaptureHandlerBase *captureHandler);
    int removeCaptureHandler(AudioALSACaptureHandlerBase *captureHandler);

    int dumpPlaybackHandler(void);
    int dumpCaptureHandler(void);

    unsigned int getUlLatency(void);
    unsigned int getDlLatency(void);
    unsigned int getA2dpPcmLatency(void);
    bool getDspTaskPlaybackStatus(void);
    bool getDspTaskA2DPStatus(void);
    int setAfeInDspShareMem(bool condition);
    int setAfeOutDspShareMem(unsigned int flag, bool condition);

    /* get audio dsp support stream */
    int getDspOutHandlerEnable(unsigned int flag);
    int getDspInHandlerEnable(unsigned int flag);
    int getDspPlaybackEnable(void);
    int getDspA2DPEnable(void);
    void updateMode(audio_mode_t audioMode);
    int doRecoveryState();

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    void CreateAurisysLibManager(
        struct aurisys_lib_manager_t **manager,
        struct aurisys_dsp_config_t **config,
        const uint8_t task_scene,
        const uint32_t aurisys_scenario,
        const uint8_t arsi_process_type,
        const uint32_t audio_mode,
        const struct stream_attribute_t *attribute_in,
        const struct stream_attribute_t *attribute_out,
        const struct stream_attribute_t *attribute_ref,
        const struct aurisys_gain_config_t *gain_config);
    void DestroyAurisysLibManager(
        struct aurisys_lib_manager_t **manager,
        struct aurisys_dsp_config_t **config,
        const uint8_t task_scene);
    void UpdateAurisysConfig(
        struct aurisys_lib_manager_t *pAurisysLibManager,
        struct aurisys_dsp_config_t *pAurisysDspConfig,
        const uint32_t audio_mode,
        const struct stream_attribute_t *attribute_in,
        const struct stream_attribute_t *attribute_out);
    void SetArsiTaskConfig(
        struct aurisys_lib_manager_config_t *pManagerConfig,
        const uint8_t task_scene,
        const uint32_t aurisys_scenario,
        const uint32_t audio_mode,
        const struct stream_attribute_t *attribute_in,
        const struct stream_attribute_t *attribute_out);
#endif

    int triggerDsp(unsigned int task_scene, int data_type);
    int getDspRuntimeEn(uint8_t task_scene);
    int setDspRuntimeEn(uint8_t task_scene, bool condition);
    int setAfeDspShareMem(bool condition);

private:
    /**
     * singleton pattern
     */
    AudioDspStreamManager();
    static AudioDspStreamManager *mDspStreamManager;
    AudioMessengerIPI *mAudioMessengerIPI;

    /*
     *  function to set pcm config and open pcmDumpThreadCreated
     */
    int checkPlaybackStatus(void);
    int startPlaybackTask(AudioALSAPlaybackHandlerBase *playbackHandler);
    int stopPlaybackTask(AudioALSAPlaybackHandlerBase *playbackHandler);
    bool dataPasstoDsp(AudioALSAPlaybackHandlerBase *Base);

    int setA2dpDspShareMem(bool condition);

    int checkA2DPTaskStatus(void);
    bool dataPasstoA2DPTask(AudioALSAPlaybackHandlerBase *Base);
    int startA2DPTask(AudioALSAPlaybackHandlerBase *Base);
    int stopA2DPTask(AudioALSAPlaybackHandlerBase *Base);
    void startDSPTaskUL(void);
    void stopDSPTaskUL(void);
    void openPCMDumpA2DP(AudioALSAPlaybackHandlerBase *playbackHandler);
    void closePCMDumpA2DP(AudioALSAPlaybackHandlerBase *playbackHandler);

    int mDspA2DPStreamState;
    int mDspA2DPIndex;
    int mDspDataProviderIndex;
    struct pcm *mDspA2DPPcm;
    struct pcm *mDspDataProviderPcm;
    bool mDspTaskA2DPActive;

    /**
     * stream manager lock
     */
    AudioLock mLock;

    /**
     * stream playback/capture handler vector
     */
    KeyedVector<unsigned long long, AudioALSAPlaybackHandlerBase *> mPlaybackHandlerVector;
    KeyedVector<unsigned long long, AudioALSACaptureHandlerBase *>  mCaptureHandlerVector;

    /*
     * pcm for capture pcm and playback pcmDumpThreadCreated
     */
    struct pcm_config mPlaybackUlConfig;
    struct pcm *mPlaybackUlPcm;
    int mPlaybackUlindex;
    struct pcm_config mPlaybackDlConfig;
    struct pcm *mPlaybackDlPcm;
    int mPlaybackDlindex;
    struct pcm_config mPlaybackIVConfig;
    struct pcm *mPlaybackIVPcm;
    int mPlaybackIVindex;
    struct pcm_config mDspConfig;
    struct pcm *mDspPcm;
    int mDspIndex;
    bool mDspTaskPlaybackActive;
    unsigned int multiplier;

    /* dsp stream state */
    int mDspStreamState;
    int mStreamCardIndex;
    struct stream_attribute_t mDspStreamAttribute;

    /* dsp ap interconnection to device */
    String8 mApTurnOnSequence;
    String8 mApTurnOnSequence2;
    String8 mApTurnOnSequenceIV;

    /* Mixer*/
    struct mixer *mMixer;

    // a2dp pcm dump
    FILE *mPCMDumpFileDSP;
    uint32_t mDumpFileNumDSP;

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    struct aurisys_lib_manager_t *mAurisysLibManagerPlayback;
    struct aurisys_dsp_config_t *mAurisysDspConfigPlayback;
#endif
};

} // end namespace android

#endif // end of ANDROID_AUDIO_DSP_STREAM_MANAGER_H

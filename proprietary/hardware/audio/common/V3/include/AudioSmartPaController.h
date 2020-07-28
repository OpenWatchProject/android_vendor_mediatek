#ifndef ANDROID_AUDIO_SMART_PA_CONTROLLER_H
#define ANDROID_AUDIO_SMART_PA_CONTROLLER_H

#ifdef __cplusplus
#include <AudioLock.h>
#include "AudioALSADriverUtility.h"

#if defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
#include "AudioSmartPaParam.h"
#endif

#endif

struct SmartPaRuntime {
    unsigned int sampleRate;
    int mode;
    int device;
};

struct SmartPaAttribute {
    unsigned int dspType;
    unsigned int chipDelayUs;

    char spkLibPath[128];

    unsigned int supportedRateList[32];
    unsigned int supportedRateMax;
    unsigned int supportedRateMin;

    char codecCtlName[128];
    int isAlsaCodec;
    int isApllNeeded;
    unsigned int i2sSetStage;

    int i2sOutSelect;
    int i2sInSelect;

    struct mixer *mMixer;
};

struct SmartPa;
struct SmartPaOps {
    int (*init)(struct SmartPa *smartPa);
    int (*speakerOn)(struct SmartPaRuntime *runtime);
    int (*speakerOff)();
    int (*deinit)();
    int (*speakerCalibrate)(int calibStage);
};

struct SmartPa {
    struct SmartPaOps ops;
    struct SmartPaRuntime runtime;
    struct SmartPaAttribute attribute;
};

enum spk_enhancement_type {
    SPK_AP_DSP = 0, /* AP Sw Enhancement*/
    SPK_ONBOARD_DSP = 1, /* SPK on board Enhancement*/
    SPK_APSCP_DSP = 2, /* SPK AP SCP Enhancement*/
};

enum spk_type {
    SPK_INVALID_TYPE = -1,
    SPK_NOT_SMARTPA,
    SPK_RICHTEK_RT5509,
    SPK_TI_TAS5782M,
    SPK_MTK_MT6660,
    SPK_TYPE_NUM
};

enum spk_i2s_set_stage {
    SPK_I2S_NO_NEED = 0x1 << 0,
    SPK_I2S_AUDIOSERVER_INIT = 0x1 << 1,
    SPK_I2S_BEFORE_PCM_OPEN = 0x1 << 2,
    SPK_I2S_BEFORE_SPK_ON = 0x1 << 3,
};

enum spk_calib_stage_t {
    SPK_CALIB_STAGE_UNKNOWN = -1,
    SPK_CALIB_STAGE_INIT,
    SPK_CALIB_STAGE_CALCULATE_AND_SAVE,
    SPK_CALIB_STAGE_DEINIT,
};

#ifdef __cplusplus
namespace android {
class String8;
class AudioSmartPaController {
    AudioSmartPaController();
    ~AudioSmartPaController();

    int init();
    int deinit();

    int initSpkAmpType();
    int initSmartPaAttribute();
    int initSmartPaRuntime();


    static AudioSmartPaController *mAudioSmartPaController;
    struct SmartPa mSmartPa;

    struct mixer *mMixer;

    struct pcm *mPcmEcho;
    struct pcm *mPcmEchoUL;

    void *mLibHandle;
    int (*mtk_smartpa_init)(struct SmartPa *smartPa);
    void setSmartPaRuntime(unsigned int device);
    int transformDeviceIndex(const unsigned int device);
    bool isSmartPA;
    bool isCalibrating;
    bool mPhoneCallEnable;

    struct pcm_config mSpkPcmConfig;
    struct pcm *mSpkPcmOut;

public:
    static AudioSmartPaController *getInstance();

    int speakerOn(unsigned int sampleRate, unsigned int device);
    int speakerOff();

    int dspOnBoardSpeakerOn(unsigned int sampleRate);
    int dspOnBoardSpeakerOff();

    unsigned int getSmartPaDelayUs();

    unsigned int getMaxSupportedRate();
    unsigned int getMinSupportedRate();
    bool isRateSupported(unsigned int rate);

    bool isAlsaCodec();
    bool isHwDspSpkProtect(const int device);
    bool isSwDspSpkProtect(const int device);
    bool isApSideSpkProtect();
    bool isBypassSwDspSpkProtect();
    unsigned int getSpkProtectType();
    int getI2sSetStage();
    bool isInCalibration();

    bool isSmartPAUsed();

    int getI2sOutSelect();
    int getI2sInSelect();
    String8 getI2sNumSequence(bool input = false);
    String8 getI2sSequence(const char *sequence, bool input = false);
    String8 getSphEchoRefSequence(bool enable, int md);

    int setI2sHD(bool enable, int i2sSelect);
    int setI2sOutHD(bool enable);
    int setI2sInHD(bool enable);

    bool isSmartPADynamicDetectSupport();
    int setSmartPaCalibration(int calibStage);
    bool isDualSmartPA();

    void setPhoneCallEnable(int enable);
    inline bool isPhoneCallOpen() { return mPhoneCallEnable; }
    void setSmartPaPcmEnable(int enable, int sampleRate = 48000);
    int setSmartPaAutoCalibration(void);

};

}
#endif
#endif

#ifndef _AUDIO_VIBSPK_CONTROL_H_
#define _AUDIO_VIBSPK_CONTROL_H_


#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>
#include <utils/String8.h>

#include <utils/threads.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>

#include "AudioVIBSPKCoeff.h"
#include "audio_custom_exp.h"

namespace android {
#define DELTA_FREQ  5
#define MOD_FREQ    15
#define VIB_DIGITAL_GAIN 16384
#define VIB_RAMPSTEP 16
#define VIBSPK_SPH_PARAM_SIZE 15
#define VIBSPK_AUD_PARAM_SIZE 36
#define VIBSPK_FILTER_NUM     64
#define VIBSPK_FREQ_LOWBOUND  141
#define VIBSPK_FILTER_FREQSTEP 3

#define VIBSPK_CALIBRATION_DONE    0x7777
#define VIBSPK_SETDEFAULT_VALUE    0x8888



const int16_t SPH_VIBR_FILTER_COEF_Table[VIBSPK_FILTER_NUM][VIBSPK_SPH_PARAM_SIZE] = {
    DEFAULT_SPH_VIBR_FILTER_COEF_141,
    DEFAULT_SPH_VIBR_FILTER_COEF_144,
    DEFAULT_SPH_VIBR_FILTER_COEF_147,
    DEFAULT_SPH_VIBR_FILTER_COEF_150,
    DEFAULT_SPH_VIBR_FILTER_COEF_153,
    DEFAULT_SPH_VIBR_FILTER_COEF_156,
    DEFAULT_SPH_VIBR_FILTER_COEF_159,
    DEFAULT_SPH_VIBR_FILTER_COEF_162,
    DEFAULT_SPH_VIBR_FILTER_COEF_165,
    DEFAULT_SPH_VIBR_FILTER_COEF_168,
    DEFAULT_SPH_VIBR_FILTER_COEF_171,
    DEFAULT_SPH_VIBR_FILTER_COEF_174,
    DEFAULT_SPH_VIBR_FILTER_COEF_177,
    DEFAULT_SPH_VIBR_FILTER_COEF_180,
    DEFAULT_SPH_VIBR_FILTER_COEF_183,
    DEFAULT_SPH_VIBR_FILTER_COEF_186,
    DEFAULT_SPH_VIBR_FILTER_COEF_189,
    DEFAULT_SPH_VIBR_FILTER_COEF_192,
    DEFAULT_SPH_VIBR_FILTER_COEF_195,
    DEFAULT_SPH_VIBR_FILTER_COEF_198,
    DEFAULT_SPH_VIBR_FILTER_COEF_201,
    DEFAULT_SPH_VIBR_FILTER_COEF_204,
    DEFAULT_SPH_VIBR_FILTER_COEF_207,
    DEFAULT_SPH_VIBR_FILTER_COEF_210,
    DEFAULT_SPH_VIBR_FILTER_COEF_213,
    DEFAULT_SPH_VIBR_FILTER_COEF_216,
    DEFAULT_SPH_VIBR_FILTER_COEF_219,
    DEFAULT_SPH_VIBR_FILTER_COEF_222,
    DEFAULT_SPH_VIBR_FILTER_COEF_225,
    DEFAULT_SPH_VIBR_FILTER_COEF_228,
    DEFAULT_SPH_VIBR_FILTER_COEF_231,
    DEFAULT_SPH_VIBR_FILTER_COEF_234,
    DEFAULT_SPH_VIBR_FILTER_COEF_237,
    DEFAULT_SPH_VIBR_FILTER_COEF_240,
    DEFAULT_SPH_VIBR_FILTER_COEF_243,
    DEFAULT_SPH_VIBR_FILTER_COEF_246,
    DEFAULT_SPH_VIBR_FILTER_COEF_249,
    DEFAULT_SPH_VIBR_FILTER_COEF_252,
    DEFAULT_SPH_VIBR_FILTER_COEF_255,
    DEFAULT_SPH_VIBR_FILTER_COEF_258,
    DEFAULT_SPH_VIBR_FILTER_COEF_261,
    DEFAULT_SPH_VIBR_FILTER_COEF_264,
    DEFAULT_SPH_VIBR_FILTER_COEF_267,
    DEFAULT_SPH_VIBR_FILTER_COEF_270,
    DEFAULT_SPH_VIBR_FILTER_COEF_273,
    DEFAULT_SPH_VIBR_FILTER_COEF_276,
    DEFAULT_SPH_VIBR_FILTER_COEF_279,
    DEFAULT_SPH_VIBR_FILTER_COEF_282,
    DEFAULT_SPH_VIBR_FILTER_COEF_285,
    DEFAULT_SPH_VIBR_FILTER_COEF_288,
    DEFAULT_SPH_VIBR_FILTER_COEF_291,
    DEFAULT_SPH_VIBR_FILTER_COEF_294,
    DEFAULT_SPH_VIBR_FILTER_COEF_297,
    DEFAULT_SPH_VIBR_FILTER_COEF_300,
    DEFAULT_SPH_VIBR_FILTER_COEF_303,
    DEFAULT_SPH_VIBR_FILTER_COEF_306,
    DEFAULT_SPH_VIBR_FILTER_COEF_309,
    DEFAULT_SPH_VIBR_FILTER_COEF_312,
    DEFAULT_SPH_VIBR_FILTER_COEF_315,
    DEFAULT_SPH_VIBR_FILTER_COEF_318,
    DEFAULT_SPH_VIBR_FILTER_COEF_321,
    DEFAULT_SPH_VIBR_FILTER_COEF_324,
    DEFAULT_SPH_VIBR_FILTER_COEF_327,
    DEFAULT_SPH_VIBR_FILTER_COEF_330,
};

typedef struct {
    short pParam[VIBSPK_SPH_PARAM_SIZE];
    bool  flag2in1;
} PARAM_VIBSPK;

#ifndef VIBSPK_DEFAULT_FREQ
#define VIBSPK_DEFAULT_FREQ     (156) //141~330 Hz
#endif



class AudioVIBSPKVsgGen {
public:
    static AudioVIBSPKVsgGen *getInstance();
    void freeInstance();
    uint32_t Process(uint32_t size, void *buffer, uint16_t channels, uint8_t rampcontrol, int32_t gain);
    void vsgDeInit();
    void vsgInit(int32_t samplerate, int32_t center_freq, int32_t mod_freq, int32_t delta_freq);
    uint8_t  mRampControl; //0--none, 1--rampdown, 2--rampup

private:
    AudioVIBSPKVsgGen();
    ~AudioVIBSPKVsgGen();
    int16_t  SineGen(int16_t cur_ph, int16_t ph_st);
    int16_t  mCenter_Freq;
    int16_t  mDelta_Freq;
    int16_t  mMod_Freq;
    int16_t  mCenter_Phase;
    int16_t  mCenter_PhaseInc;
    int16_t  mCenter_PhaseStat;
    int16_t  mMod_Phase;
    int16_t  mMod_PhaseInc;
    int16_t  mMod_PhaseStat;
    uint16_t mMod_Idx;
    int16_t  mGain;
    static AudioVIBSPKVsgGen *UniqueAudioVIBSPKVsgGen;

};   //AudioVIBSPKVsgGen


class AudioVIBSPKControl {
public:
    static AudioVIBSPKControl *getInstance();
    void freeInstance();
    void setVibSpkEnable(bool enable);
    bool getVibSpkEnable(void);
    void setParameters(int32_t rate, int32_t center_freq, int32_t mod_freq, int32_t delta_freq);
    void VibSpkProcess(uint32_t size, void *buffer, uint32_t channels);
    void VibSpkRampControl(uint8_t rampcontrol);
    void setVibSpkGain(int32_t MaxVolume, int32_t MinVolume, int32_t VolumeRange);
    int16_t getVibSpkGain(void);
private:
    AudioVIBSPKControl();
    ~AudioVIBSPKControl();
    Mutex   mMutex;
    int32_t mSampleRate;
    int32_t mCenterFreq;
    int32_t mModFreq;
    int32_t mDeltaFreq;
    int32_t mDigitalGain;
    AudioVIBSPKVsgGen *mVsg;
    bool mEnable;
    static AudioVIBSPKControl *UniqueAudioVIBSPKControl;
    uint8_t  mRampControl; //0--none, 1--rampdown, 2--rampup
};   //AudioVIBSPKControl

}   //namespace android

#endif   //_AUDIO_VIBSPK_CONTROL_H_

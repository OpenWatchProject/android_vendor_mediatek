/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioParamTuning.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements the method for  handling param tuning.
 *
 * Author:
 * -------
 *   Donglei Ji (mtk80823)
 *******************************************************************************/

#ifndef _AUDIO_ALSA_PARAM_TUNER_H_
#define _AUDIO_ALSA_PARAM_TUNER_H_

#include <utils/threads.h>

#include "AudioVolumeInterface.h"
//#include "AudioALSAVolumeController.h"
#include "AudioVolumeFactory.h"

#include "AudioALSAHardwareResourceManager.h"
#include "SpeechDriverFactory.h"
//#include "AudioDigitalControlInterface.h"
//#include "AudioAnalogControlInterface.h"
#include "SpeechPcm2way.h"
#include "AudioALSASpeechPhoneCallController.h"

#define MAX_VOICE_VOLUME VOICE_VOLUME_MAX
#define FILE_NAME_LEN_MAX 128

#ifdef DMNR_TUNNING_AT_MODEMSIDE
#define P2W_RECEIVER_OUT 0
#define P2W_HEADSET_OUT 1
#define P2W_NORMAL 0
#define P2W_RECONLY 1
#else
typedef enum {
    OUTPUT_DEVICE_RECEIVER = 0,
    OUTPUT_DEVICE_HEADSET,
    OUTPUT_DEVICE_SPEAKER,

    DMNR_REC_OUTPUT_DEV_NUM
} DMNR_REC_OUTPUT_DEVICE_TYPE;

typedef enum {
    RECPLAY_MODE = 0,
    RECONLY_MODE,
    RECPLAY_HF_MODE,
    RECONLY_HF_MODE,

    DMNR_REC_MODE_NUM
} DMNR_REC_MODE_TYPE;
#include "AudioALSAStreamManager.h"
#include "AudioSpeechEnhanceInfo.h"
#endif

typedef struct {
    unsigned short cmd_type;
    unsigned short slected_fir_index;
    unsigned short dlDigitalGain;
    unsigned short dlPGA;
    unsigned short phone_mode;
    unsigned short wb_mode;
    char input_file[FILE_NAME_LEN_MAX];
} AudioTasteTuningStruct;

typedef enum {
    AUD_TASTE_STOP = 0,
    AUD_TASTE_START,
    AUD_TASTE_DLDG_SETTING,
    AUD_TASTE_DLPGA_SETTING,
    AUD_TASTE_INDEX_SETTING,

    AUD_TASTE_CMD_NUM
} AUD_TASTE_CMD_TYPE;

typedef enum {
    PCM_FORMAT = 0,
    WAVE_FORMAT,

    UNSUPPORT_FORMAT
} FILE_FORMAT;

typedef enum {
    AUD_MIC_GAIN = 0,
    AUD_RECEIVER_GAIN,
    AUD_HS_GAIN,
    AUD_MIC_GAIN_HF,

    AUD_GAIN_TYPE_NUM
} DMNRGainType;

typedef enum {
    DUAL_MIC_REC_PLAY_STOP = 0,
    DUAL_MIC_REC,
    DUAL_MIC_REC_PLAY,
    DUAL_MIC_REC_PLAY_HS,
    DUAL_MIC_REC_HF,
    DUAL_MIC_REC_PLAY_HF,
    DUAL_MIC_REC_PLAY_HF_HS,
    FIR_REC,
    FIR_REC_STOP,

    DMNR_TUNING_CMD_CNT
} TuningCmdType;

typedef struct {
    unsigned int ChunkID;
    unsigned int ChunkSize;
    unsigned int Format;
    unsigned int Subchunk1ID;
    unsigned int Subchunk1IDSize;
    unsigned short AudioFormat;
    unsigned short NumChannels;
    unsigned int SampleRate;
    unsigned int ByteRate;
    unsigned short BlockAlign;
    unsigned short BitsPerSample;
    unsigned int SubChunk2ID;
    unsigned int SubChunk2Size;
} WAVEHDR;

#if defined(MTK_AUDIO_GAIN_TABLE) && !defined(MTK_NEW_VOL_CONTROL)
//for UI Disp
#define MAX_NAME_LEN 16
#define MAX_SUB_ITEM_NUM 4
typedef struct _PCDispSubItem {
    char outputDevice[MAX_NAME_LEN];
    unsigned short AnalogPoint;               // PGA and Ext Amp
} PCDispSubItem;                           // size:  18
typedef struct _PCDispItem {
    char strType[MAX_NAME_LEN];
    unsigned short level;
    unsigned int subItemNum;
    PCDispSubItem subItem[MAX_SUB_ITEM_NUM];  //3 //3 -> 4
} PCDispItem;                              // size:  22 + 18*4 = 94


#define MAX_NAME_LEN_MIC 32
#define MAX_SUB_ITEM_NUM_MIC 32
typedef struct _PCDispMic {
    char strType[MAX_NAME_LEN_MIC];
    unsigned int subItemNum;
    char subItem[MAX_SUB_ITEM_NUM_MIC][MAX_NAME_LEN_MIC];  //24  -> 32
} PCDispMic;                               //size:  1060

#define MAX_GAIN_POINT_NUM 12
#define MAX_GAIN_STEP 32
#define MAX_ITEM_NUM 20
typedef struct _PCDispTotolStru {
    unsigned int itemNum;
    PCDispItem DispItem[MAX_ITEM_NUM];        // 16 -> 20
    PCDispMic DispMic;
    unsigned short gainRangeNum;              // 9->12
    signed char gainRange[MAX_GAIN_POINT_NUM][MAX_GAIN_STEP];   // 9-> 12
} PCDispTotolStru;

static char gainRangeCopy[MAX_GAIN_POINT_NUM][MAX_GAIN_STEP] = {
    { -10,      -9,   -8,   -7,   -6,    -5,    -4,     -3,    -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    { -10,      -9,   -8,   -7,   -6,    -5,    -4,     -3,    -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0,    45,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0,     6,   12,   20,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    { -60,  -50,  -45,  -42,   -39, -36,  -33,  -30,  -27,  -24,  -21,  -20,  -19,  -18,  -17,  -16,  -15,  -14,  -13,  -12,  -11,  -10,  -9,   -8,   -7,   -6,   -5,   -4,   -3,   -2,   -1,   0  },
    {0,    6,    12,   20,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    { -60,  -50,  -45,  -42,   -39, -36,  -33,  -30,  -27,  -24,  -21,  -20,  -19,  -18,  -17,  -16,  -15,  -14,  -13,  -12,  -11,  -10,  -9,   -8,  -7,   -6,   -5,   -4,   -3,   -2,   -1,    0  },
    {0,    32,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0,    32,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {6,    7,    8,    9,    10,   11,   12,   13,   14,   15,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

static char subItemMic[MAX_SUB_ITEM_NUM_MIC][MAX_NAME_LEN_MIC] = {
    {"Idle Record Mic"},
    {"Idel Record Headset"},
    {"Voice Recognition Mic"},    //
    {"Voice Recognition Headset"},
    {"Video Record Mic"},
    {"Video Record Headset"},
    {"IncallNB Receiver"},
    {"IncallNB Headset"},
    {"IncallNB Speaker"},
    {"IncallWB Receiver"},
    {"IncallWB Headset"},
    {"IncallWB Speaker"},
    {"VOIP Receiver"},
    {"VOIP Headset"},
    {"VOIP Speaker"},
    {"TTY"},
    {"Reserved1 Mic"},
    {"Reserved1 Headset"},
    {"Reserved2 Mic"},
    {"Reserved2 Headset"},
    {""},
    {""},
    {""},
    {""},
    {""},
    {""},
    {""},
    {""}
};

#endif

namespace android {
#define MODE_NUM NUM_OF_VOL_MODE

#if defined(MTK_AUDIO_GAIN_TABLE) && !defined(MTK_NEW_VOL_CONTROL)
class AudioHWVolumeCapability;
#endif

class AudioALSAParamTuner {
public:
    AudioALSAParamTuner();
    ~AudioALSAParamTuner();

    static AudioALSAParamTuner *getInstance();

    //for taste tool
    bool isPlaying();
    status_t setMode(uint32 mode);
    uint32 getMode();
    status_t setPlaybackFileName(const char *fileName);
    status_t setDLPGA(uint32 gain);
    void updataOutputFIRCoffes(AudioTasteTuningStruct *pCustParam);
    status_t enableModemPlaybackVIASPHPROC(bool bEnable, bool bWB = false);

    FILE_FORMAT playbackFileFormat();

    // protect Play PCM With Speech Enhancement buffers
    pthread_mutex_t mPlayBufMutex;
    pthread_cond_t mPPSExit_Cond;
    pthread_mutex_t mPPSMutex;
    pthread_mutex_t mP2WMutex;

    uint32 mMode;
    bool m_bPPSThreadExit;
    bool m_bWBMode;
    FILE *m_pInputFile;

    // For DMNR Tuning
    status_t setRecordFileName(const char *fileName);
    status_t setDMNRGain(unsigned short type, unsigned short value); //for DMNR
    status_t getDMNRGain(unsigned short type, unsigned short *value); //for DMNR
#ifdef DMNR_TUNNING_AT_MODEMSIDE
    status_t enableDMNRModem2Way(bool bEnable, bool bWBMode, unsigned short outputDevice, unsigned short workMode);
#else
    AudioALSAStreamManager *getStreamManager() {return mAudioStreamManager;}
    AudioSpeechEnhanceInfo  *getSpeechEnhanceInfoInst() {return mAudioSpeechEnhanceInfoInstance;}
    int getPlaybackDb() {return mPlaybackDb_index;}
    status_t setPlaybackVolume(uint32_t mode, uint32_t gain, uint32_t device);
    status_t enableDMNRAtApSide(bool bEnable, bool bWBMode, unsigned short outputDevice, unsigned short workMode);
    status_t enableFIRRecord(bool bEnable);
#endif
    // for DMNR playback+record thread
    //rb m_sRecBuf;

    // protect DMNR Playback+record buffers
    pthread_mutex_t mRecPlayMutex;
    pthread_mutex_t mRecBufMutex;
    pthread_cond_t mRecPlayExit_Cond;

    bool m_bRecPlayThreadExit;
    FILE *m_pOutputFile;

    Play2Way *mPlay2WayInstance;
    Record2Way *mRec2WayInstance;

    uint32_t mDMNROutputDevice;

#if defined(MTK_AUDIO_GAIN_TABLE) && !defined(MTK_NEW_VOL_CONTROL)
    status_t getGainInfoForDisp(void *pParam); //gain table
#ifdef MTK_AUDIO_GAIN_TABLE_BT
    status_t getBtNrecInfoForDisp(void *pParam); //gain table
#endif
#endif

private:

    status_t setSphVolume(uint32 mode, uint32 gain);
    status_t openModemDualMicCtlFlow(bool bWB, bool bRecPly);
    status_t closeModemDualMicCtlFlow(bool bRecPly);

    // the uniqe
    static AudioALSAParamTuner *UniqueTuningInstance;

    SpeechDriverFactory *mSpeechDriverFactory;
    AudioVolumeInterface *mAudioALSAVolumeController;
    //        AudioAnalogControlInterface *mAudioAnalogInstance;
    //        AudioDigitalControlInterface *mAudioDigitalInstance;
    AudioALSAHardwareResourceManager *mAudioResourceManager;
    AudioALSASpeechPhoneCallController *mSphPhonecallCtrl;
#if defined(MTK_AUDIO_GAIN_TABLE) && !defined(MTK_NEW_VOL_CONTROL)
    status_t getPCDispItem(void *pParam);
    status_t getPCDispSubItem(void *pParam, int streamType, int speech = -1);
    status_t getPCDispMic(void *pParam);
    AudioHWVolumeCapability *mAudioHWVolumeCapabilityInstance;
#endif
    uint32 mSideTone;
    uint32 mOutputVolume[MODE_NUM];
    char m_strInputFileName[FILE_NAME_LEN_MAX];
    bool m_bPlaying;
    bool mStartRec;

    pthread_t mTasteThreadID;

    bool m_bDMNRPlaying;
    char m_strOutFileName[FILE_NAME_LEN_MAX]; // for reord
    unsigned short mDualMicTool_micGain[2]; // 0 for normal mic; 1 for handsfree mic
    unsigned short mDualMicTool_receiverGain;
    unsigned short mDualMicTool_headsetGain;
    pthread_t mRecPlayThreadID;
#ifndef DMNR_TUNNING_AT_MODEMSIDE
    AudioALSAStreamManager *mAudioStreamManager;
    AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfoInstance;
    int mPlaybackDb_index;
#endif
};
}
#endif


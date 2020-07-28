#ifndef _AUDIO_GAIN_TABLE_PARAM_H_
#define _AUDIO_GAIN_TABLE_PARAM_H_

#include <system/audio.h>
#include <vector>
#include <string>

#define PLAY_DIGI_AUDIOTYPE_NAME "PlaybackVolDigi"
#define PLAY_ANA_AUDIOTYPE_NAME "PlaybackVolAna"
#define SPEECH_VOL_AUDIOTYPE_NAME "SpeechVol"
#define REC_VOL_AUDIOTYPE_NAME "RecordVol"
#define VOIP_VOL_AUDIOTYPE_NAME "VoIPVol"
#define RINGBACK_VOL_AUDIOTYPE_NAME "RingbackVol"
#define VOLUME_AUDIOTYPE_NAME "Volume"
#define GAIN_MAP_AUDIOTYPE_NAME "VolumeGainMap"
#define GAIN_MAP_UL_AUDIOTYPE_NAME "VolumeGainMapUL"
#define HP_IMPEDANCE_AUDIOTYPE_NAME "HpImpedance"

// VOLUME INDEX
#define GAIN_MAX_VOL_INDEX (15) // index will change from 0~18 total 19 step
#define GAIN_VOL_INDEX_SIZE (GAIN_MAX_VOL_INDEX + 1)

#define GAIN_MAX_SPEECH_VOL_INDEX (7) // for voice stream, policy index range = 0~7, set 7 here.

// STREAM TYPE
#define GAIN_MIN_STREAM_TYPE (AUDIO_STREAM_VOICE_CALL)
#define GAIN_MAX_STREAM_TYPE (AUDIO_STREAM_ACCESSIBILITY)
#define GAIN_STREAM_TYPE_SIZE (GAIN_MAX_STREAM_TYPE + 1)

// SCENE INDEX
#define GAIN_SCENE_INDEX_DEFAULT (0)

// DEVICE
enum GAIN_DEVICE {
    GAIN_DEVICE_NONE = -1,
    GAIN_DEVICE_EARPIECE = 0,
    GAIN_DEVICE_HEADSET,
    GAIN_DEVICE_SPEAKER,
    GAIN_DEVICE_HEADPHONE,
    GAIN_DEVICE_HSSPK,  // headset with speaker
    GAIN_DEVICE_HEADSET_5POLE,
    GAIN_DEVICE_HEADSET_5POLE_ANC,
    GAIN_DEVICE_HAC,
    GAIN_DEVICE_BT,
    GAIN_DEVICE_TTY,
    GAIN_DEVICE_LPBK_RCV,
    GAIN_DEVICE_LPBK_SPK,
    GAIN_DEVICE_LPBK_HP,
    GAIN_DEVICE_USB,
    GAIN_DEVICE_BT_A2DP,
    GAIN_DEVICE_BT_A2DP_HP,
    GAIN_DEVICE_BT_A2DP_SPK,
    GAIN_DEVICE_RCV_EV,//super volume mode
    GAIN_DEVICE_SPK_EV,//super volume mode
    NUM_GAIN_DEVICE,
};

// GAIN_ANA_TYPE
enum GAIN_ANA_TYPE {
    GAIN_ANA_NONE = -1,
    GAIN_ANA_HANDSET = 0,   // mtk codec voice buffer
    GAIN_ANA_HEADPHONE,     // mtk codec audio buffer
    GAIN_ANA_SPEAKER,       // mtk codec speaker amp
    GAIN_ANA_LINEOUT,       // mtk codec linout buffer
    NUM_GAIN_ANA_TYPE
};

// SPEECH
enum GAIN_SPEECH_BAND {
    GAIN_SPEECH_NB,
    GAIN_SPEECH_WB,
    GAIN_SPEECH_SWB,
    NUM_GAIN_SPEECH_BAND
};

enum GAIN_SPEECH_NETWORK {
    GAIN_SPEECH_NETWORK_GSM,
    GAIN_SPEECH_NETWORK_WCDMA,
    GAIN_SPEECH_NETWORK_VOLTE,
    NUM_GAIN_SPEECH_NETWORK
};

// MIC
enum GAIN_MIC_MODE {
    GAIN_MIC_INVALID = -1,
    GAIN_MIC_NORMAL = 0,
    GAIN_MIC_VOICE_CALL,
    GAIN_MIC_CAMCORDER,
    GAIN_MIC_VOICE_RECOGNITION,
    GAIN_MIC_VOICE_COMMUNICATION,
    GAIN_MIC_VOICE_UNLOCK,
    GAIN_MIC_CUSTOMIZATION1,
    GAIN_MIC_CUSTOMIZATION2,
    GAIN_MIC_CUSTOMIZATION3,
    GAIN_MIC_UNPROCESSED,
    GAIN_MIC_AAUDIO,
    NUM_GAIN_MIC_MODE,
    /* the following is DEPRECATED, DO NOT USE!!! */
    Idle_Normal_Record,         // Record, AUDIO_SOURCE_MIC, Sound recording, rcv  (else)
    Idle_Headset_Record,        // Record, AUDIO_SOURCE_MIC, Sound recording, hs   (else)
    Voice_Rec_Mic_Handset,      // Record, AUDIO_SOURCE_VOICE_RECOGNITION, Voice recognition & verifier, rcv
    Voice_Rec_Mic_Headset,      // Record, AUDIO_SOURCE_VOICE_RECOGNITION, Voice recognition & verifier, hs
    Idle_Video_Record_Handset,  // Record, AUDIO_SOURCE_CAMCORDER, Camera recording, rcv
    Idle_Video_Record_Headset,  // Record, AUDIO_SOURCE_CAMCORDER, Camera recording, hs
    Normal_Mic,                 // Speech, NB, RCV
    Headset_Mic,                // Speech, NB, HS
    Handfree_Mic,               // Speech, NB, SPK
    Normal_WB_Mic,              // Speech, WB, RCV
    Headset_WB_Mic,             // Speech, WB, HS
    Handfree_WB_Mic,            // Speech, WB, SPK
    VOIP_Normal_Mic,            // VoIP, Handset
    VOIP_Headset_Mic,           // VoIP, Headset
    VOIP_Handfree_Mic,          // VoIP, Hands-free
    TTY_CTM_Mic,                // Record,
    Level_Shift_Buffer_Gain,    // Record,
    Analog_PLay_Gain,           // Record,
    Voice_UnLock_Mic_Handset,   // Record, AUDIO_SOURCE_VOICE_UNLOCK, , rcv
    Voice_UnLock_Mic_Headset,   // Record, AUDIO_SOURCE_VOICE_UNLOCK, ,hs
    Customization1_Mic_Handset, // Record, AUDIO_SOURCE_CUSTOMIZATION1, ASR improvement,rcv
    Customization1_Mic_Headset, // Record, AUDIO_SOURCE_CUSTOMIZATION1, ASR improvement,hs
    Customization2_Mic_Handset, // Record, AUDIO_SOURCE_CUSTOMIZATION2, , rcv
    Customization2_Mic_Headset, // Record, AUDIO_SOURCE_CUSTOMIZATION2, ,hs
    Customization3_Mic_Handset, // Record, AUDIO_SOURCE_CUSTOMIZATION3, ,rcv
    Customization3_Mic_Headset, // Record, AUDIO_SOURCE_CUSTOMIZATION3, , hs
};

struct GainTableUnit {
    unsigned char digital;
    unsigned char analog[NUM_GAIN_ANA_TYPE];
};

struct GainTableSidetoneUnit {
    unsigned char gain;
};

struct GainTableMicUnit {
    unsigned char gain;
};

struct GainTableRingbackToneUnit {
    unsigned char digital;
};

struct GainTableForScene {
    GainTableUnit streamGain[GAIN_STREAM_TYPE_SIZE][NUM_GAIN_DEVICE][GAIN_VOL_INDEX_SIZE];
    GainTableMicUnit micGain[NUM_GAIN_MIC_MODE][NUM_GAIN_DEVICE];
};

struct GainTableForNonScene{
    GainTableUnit speechGain[NUM_GAIN_SPEECH_BAND][NUM_GAIN_SPEECH_NETWORK][NUM_GAIN_DEVICE][GAIN_VOL_INDEX_SIZE];
    GainTableSidetoneUnit sidetoneGain[NUM_GAIN_SPEECH_BAND][NUM_GAIN_SPEECH_NETWORK][NUM_GAIN_DEVICE];
    GainTableMicUnit speechMicGain[NUM_GAIN_SPEECH_BAND][NUM_GAIN_SPEECH_NETWORK][NUM_GAIN_DEVICE];
    GainTableRingbackToneUnit ringbackToneGain[NUM_GAIN_DEVICE][GAIN_VOL_INDEX_SIZE];
};

struct GainTableParam {
    int sceneCount;
    GainTableForScene *sceneGain;
    GainTableForNonScene nonSceneGain;
};

struct GainTableSpec {
    int keyStepPerDb;
    float keyDbPerStep;
    float keyVolumeStep;

    int digiDbMax;
    int digiDbMin;

    int sidetoneIdxMax;
    int sidetoneIdxMin;

    int micIdxMax[NUM_GAIN_DEVICE];
    int micIdxMin[NUM_GAIN_DEVICE];
    int decRecMax;
    int decRecStepPerDb;

    unsigned int numAudioBufferGainLevel;
    std::vector<short> audioBufferGainDb;
    std::vector<short> audioBufferGainIdx;
    std::vector<std::string> audioBufferGainString;
    int audioBufferGainPreferMaxIdx;
    std::string audioBufLMixerName;
    std::string audioBufRMixerName;

    unsigned int numVoiceBufferGainLevel;
    std::vector<short> voiceBufferGainDb;
    std::vector<short> voiceBufferGainIdx;
    std::vector<std::string> voiceBufferGainString;
    int voiceBufferGainPreferMaxIdx;
    std::string voiceBufMixerName;

    unsigned int numLineoutBufferGainLevel;
    std::vector<short> lineoutBufferGainDb;
    std::vector<short> lineoutBufferGainIdx;
    std::vector<std::string> lineoutBufferGainString;
    int lineoutBufferGainPreferMaxIdx;

    unsigned int numSpkGainLevel;
    std::vector<short> spkGainDb;
    std::vector<short> spkGainIdx;
    std::vector<std::string> spkGainString;

    std::string spkLMixerName;
    std::string spkRMixerName;
    GAIN_ANA_TYPE spkAnaType;

    std::vector<short> swagcGainMap[NUM_GAIN_DEVICE];
    std::vector<short> swagcGainMapDmic[NUM_GAIN_DEVICE];
    std::vector<short> ulPgaGainMap[NUM_GAIN_DEVICE];
    std::vector<std::string> ulPgaGainString;
    int ulGainOffset;
    int ulPgaGainMapMax;
    int ulHwPgaIdxMax;
    std::string ulPgaLMixerName;
    std::string ulPgaRMixerName;

    std::vector<short> stfGainMap;

    int hpImpEnable;
    int hpImpOnBoardResistor;
    int hpImpDefaultIdx;
    std::vector<short> hpImpThresholdList;
    std::vector<short> hpImpCompensateList;
};

#endif   //_AUDIO_GAIN_TABLE_PARAM_H_


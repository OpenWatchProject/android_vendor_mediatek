
#define LOG_TAG "AudioMTKGainController"
#define MTK_LOG_ENABLE 1
#include <log/log.h>
#include <math.h>
#include <linux/fm.h>
#include "audio_custom_exp.h"
#include "SpeechDriverFactory.h"
#include "AudioMTKGainController.h"
#include "AudioALSASpeechPhoneCallController.h"
#include "SpeechEnhancementController.h"
#include "AudioAMPControlInterface.h"
#include "AudioALSADriverUtility.h"
#include "AudioALSAStreamManager.h"
#include "AudioALSAHardwareResourceManager.h"

#ifndef MTK_BASIC_PACKAGE
#include <media/IATVCtrlService.h>
#endif
#include "AudioTypeExt.h"

#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))

namespace android {

#define INDEX_REMAP //index remap for audio and voice buffer index from tuning tool

#ifndef MTK_BASIC_PACKAGE
static sp<IATVCtrlService> spATVCtrlService = NULL;
#endif

AudioMTKGainController *AudioMTKGainController::UniqueVolumeInstance = NULL;

// here can change to match audiosystem

// total 64 dB
static const float keydBPerStep = 0.25f;
static const float keyvolumeStep = 255.0f;


// shouldn't need to touch these
static const float keydBConvert = -keydBPerStep * 2.302585093f / 20.0f;
static const float keydBConvertInverse = 1.0f / keydBConvert;

static const char *PGA_Gain_String[] = {"0Db", "6Db", "12Db", "18Db", "24Db", "30Db"};

//hw spec db
const int keyAudioBufferStep       =   19;
//const int KeyAudioBufferGain[]     =  { -5, -3, -1, 1, 3, 5, 7, 9};
const int KeyAudioBufferGain[]     =  { 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -40};
static const char *DL_PGA_Headset_GAIN[] = {"8Db", "7Db", "6Db", "5Db", "4Db", "3Db", "2Db", "1Db", "0Db", "-1Db", "-2Db", "-3Db",
                                            "-4Db", "-5Db", "-6Db", "-7Db", "-8Db", "-9Db", "-10Db", "-40Db"
                                           };

const int keyVoiceBufferStep       =   19;
//const int KeyVoiceBufferGain[]     =  { -21, -19, -17, -15, -13, -11, -9, -7, -5, -3, -1, 1, 3, 5, 7, 9};
const int KeyVoiceBufferGain[]     =  { 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -40};
static const char *DL_PGA_Handset_GAIN[] = {"8Db", "7Db", "6Db", "5Db", "4Db", "3Db", "2Db", "1Db", "0Db", "-1Db", "-2Db", "-3Db",
                                            "-4Db", "-5Db", "-6Db", "-7Db", "-8Db", "-9Db", "-10Db", "-40Db"
                                           };


//const int keyULStep                =   5;
//const int KeyULGain[]              =  { -6, 0, 6, 12, 18, 24};
const int keyULGainOffset          = 2;

const int keySPKStep               =   15;
//const int KeySPKgain[]             =  { -60, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
static const char *DL_PGA_SPEAKER_GAIN[] = {"MUTE", "0Db", "4Db", "5Db", "6Db", "7Db", "8Db", "9Db", "10Db",
                                            "11Db", "12Db", "13Db", "14Db", "15Db", "16Db", "17Db"
                                           };



const int keyDLDigitalDegradeMax   = 63;

//const int keyULDigitalIncreaseMax  = 32;

static const int keySidetoneSize   = 47;

static const uint16_t SwAgc_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] = {
    25, 24, 23, 22, 21, 20, 19, 18, 17,
    16, 15, 14, 13, 12, 11,
    16, 15, 14, 13, 12, 11,
    16, 15, 14, 13, 12, 11,
    16, 15, 14, 13, 12, 11,
    16, 15, 14, 13, 12, 11,
    10, 9, 8, 7, 6, 5, 4
};

static const uint16_t PGA_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] = {
    6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6,
    12, 12, 12, 12, 12, 12,
    18, 18, 18, 18, 18, 18,
    24, 24, 24, 24, 24, 24,
    30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30
};

static const uint16_t keySideTone[] = {
    32767, 29204, 26027, 23196, 20674, 18426, 16422, 14636, 13044, 11625,  /*1dB per step*/
    10361, 9234,  8230,  7335,  6537,  5826,  5193,  4628,  4125,  3676,
    3276,  2919,  2602,  2319,  2066,  1841,  1641,  1463,  1304,  1162,
    1035,  923,   822,   733,   653,   582,   519,   462,   412,   367,
    327,   291,   260,   231,   206,   183,   163,   145
};

// HW Gain mappring
static const uint32_t kHWGainMap[] = {
    0x00000, //   0, -64.0 dB (mute)
    0x0015E, //   1, -63.5 dB
    0x00173, //   2, -63.0 dB
    0x00189, //   3, -62.5 dB
    0x001A0, //   4, -62.0 dB
    0x001B9, //   5, -61.5 dB
    0x001D3, //   6, -61.0 dB
    0x001EE, //   7, -60.5 dB
    0x0020C, //   8, -60.0 dB
    0x0022B, //   9, -59.5 dB
    0x0024C, //  10, -59.0 dB
    0x0026F, //  11, -58.5 dB
    0x00294, //  12, -58.0 dB
    0x002BB, //  13, -57.5 dB
    0x002E4, //  14, -57.0 dB
    0x00310, //  15, -56.5 dB
    0x0033E, //  16, -56.0 dB
    0x00370, //  17, -55.5 dB
    0x003A4, //  18, -55.0 dB
    0x003DB, //  19, -54.5 dB
    0x00416, //  20, -54.0 dB
    0x00454, //  21, -53.5 dB
    0x00495, //  22, -53.0 dB
    0x004DB, //  23, -52.5 dB
    0x00524, //  24, -52.0 dB
    0x00572, //  25, -51.5 dB
    0x005C5, //  26, -51.0 dB
    0x0061D, //  27, -50.5 dB
    0x00679, //  28, -50.0 dB
    0x006DC, //  29, -49.5 dB
    0x00744, //  30, -49.0 dB
    0x007B2, //  31, -48.5 dB
    0x00827, //  32, -48.0 dB
    0x008A2, //  33, -47.5 dB
    0x00925, //  34, -47.0 dB
    0x009B0, //  35, -46.5 dB
    0x00A43, //  36, -46.0 dB
    0x00ADF, //  37, -45.5 dB
    0x00B84, //  38, -45.0 dB
    0x00C32, //  39, -44.5 dB
    0x00CEC, //  40, -44.0 dB
    0x00DB0, //  41, -43.5 dB
    0x00E7F, //  42, -43.0 dB
    0x00F5B, //  43, -42.5 dB
    0x01044, //  44, -42.0 dB
    0x0113B, //  45, -41.5 dB
    0x01240, //  46, -41.0 dB
    0x01355, //  47, -40.5 dB
    0x0147A, //  48, -40.0 dB
    0x015B1, //  49, -39.5 dB
    0x016FA, //  50, -39.0 dB
    0x01857, //  51, -38.5 dB
    0x019C8, //  52, -38.0 dB
    0x01B4F, //  53, -37.5 dB
    0x01CED, //  54, -37.0 dB
    0x01EA4, //  55, -36.5 dB
    0x02075, //  56, -36.0 dB
    0x02261, //  57, -35.5 dB
    0x0246B, //  58, -35.0 dB
    0x02693, //  59, -34.5 dB
    0x028DC, //  60, -34.0 dB
    0x02B48, //  61, -33.5 dB
    0x02DD9, //  62, -33.0 dB
    0x03090, //  63, -32.5 dB
    0x03371, //  64, -32.0 dB
    0x0367D, //  65, -31.5 dB
    0x039B8, //  66, -31.0 dB
    0x03D24, //  67, -30.5 dB
    0x040C3, //  68, -30.0 dB
    0x04499, //  69, -29.5 dB
    0x048AA, //  70, -29.0 dB
    0x04CF8, //  71, -28.5 dB
    0x05188, //  72, -28.0 dB
    0x0565D, //  73, -27.5 dB
    0x05B7B, //  74, -27.0 dB
    0x060E6, //  75, -26.5 dB
    0x066A4, //  76, -26.0 dB
    0x06CB9, //  77, -25.5 dB
    0x0732A, //  78, -25.0 dB
    0x079FD, //  79, -24.5 dB
    0x08138, //  80, -24.0 dB
    0x088E0, //  81, -23.5 dB
    0x090FC, //  82, -23.0 dB
    0x09994, //  83, -22.5 dB
    0x0A2AD, //  84, -22.0 dB
    0x0AC51, //  85, -21.5 dB
    0x0B687, //  86, -21.0 dB
    0x0C157, //  87, -20.5 dB
    0x0CCCC, //  88, -20.0 dB
    0x0D8EF, //  89, -19.5 dB
    0x0E5CA, //  90, -19.0 dB
    0x0F367, //  91, -18.5 dB
    0x101D3, //  92, -18.0 dB
    0x1111A, //  93, -17.5 dB
    0x12149, //  94, -17.0 dB
    0x1326D, //  95, -16.5 dB
    0x14496, //  96, -16.0 dB
    0x157D1, //  97, -15.5 dB
    0x16C31, //  98, -15.0 dB
    0x181C5, //  99, -14.5 dB
    0x198A1, // 100, -14.0 dB
    0x1B0D7, // 101, -13.5 dB
    0x1CA7D, // 102, -13.0 dB
    0x1E5A8, // 103, -12.5 dB
    0x2026F, // 104, -12.0 dB
    0x220EA, // 105, -11.5 dB
    0x24134, // 106, -11.0 dB
    0x26368, // 107, -10.5 dB
    0x287A2, // 108, -10.0 dB
    0x2AE02, // 109,  -9.5 dB
    0x2D6A8, // 110,  -9.0 dB
    0x301B7, // 111,  -8.5 dB
    0x32F52, // 112,  -8.0 dB
    0x35FA2, // 113,  -7.5 dB
    0x392CE, // 114,  -7.0 dB
    0x3C903, // 115,  -6.5 dB
    0x4026E, // 116,  -6.0 dB
    0x43F40, // 117,  -5.5 dB
    0x47FAC, // 118,  -5.0 dB
    0x4C3EA, // 119,  -4.5 dB
    0x50C33, // 120,  -4.0 dB
    0x558C4, // 121,  -3.5 dB
    0x5A9DF, // 122,  -3.0 dB
    0x5FFC8, // 123,  -2.5 dB
    0x65AC8, // 124,  -2.0 dB
    0x6BB2D, // 125,  -1.5 dB
    0x72148, // 126,  -1.0 dB
    0x78D6F, // 127,  -0.5 dB
    0x80000, // 128,   0.0 dB
};


float AudioMTKGainController::linearToLog(int volume) {
    //ALOGD("linearToLog(%d)=%f", volume, v);
    return volume ? exp(float(keyvolumeStep - volume) * keydBConvert) : 0;
}

int AudioMTKGainController::logToLinear(float volume) {
    //ALOGD("logTolinear(%d)=%f", v, volume);
    return volume ? keyvolumeStep - int(keydBConvertInverse * log(volume) + 0.5) : 0;
}

AudioMTKGainController *AudioMTKGainController::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (UniqueVolumeInstance == 0) {
        SLOGV("+UniqueVolumeInstance\n");
        UniqueVolumeInstance = new AudioMTKGainController();
        SLOGV("-UniqueVolumeInstance\n");
    }

    return UniqueVolumeInstance;
}

AudioMTKGainController::AudioMTKGainController() {
    SLOGD("AudioMTKGainController contructor");
    mAmpControl          = AudioDeviceManger::createInstance();
    mAudioSpeechEnhanceInfoInstance = AudioSpeechEnhanceInfo::getInstance();
    //mStreamManager = AudioALSAStreamManager::getInstance();
    mHardwareResourceManager = AudioALSAHardwareResourceManager::getInstance();
    mVoiceVolume = 1.0f;
    mMasterVolume = 1.0f;
    //mFmVolume = 0xFF;
    //mFmChipVolume = 0xFFFFFFFF;
    mMatvVolume = 0xFF;
    mSpeechNB = true;
    mSupportBtVol = false;
    memset(&mHwVolume, 0xFF, sizeof(mHwVolume));
    memset(&mHwStream, 0xFF, sizeof(mHwStream));
    initVolumeController();
    mULTotalGain = 184;
    mInitDone = true;
    mMixer = NULL;
    mMixer = AudioALSADriverUtility::getInstance()->getMixer();
    ALOGD("mMixer = %p", mMixer);
    ASSERT(mMixer != NULL);
}

status_t AudioMTKGainController::initVolumeController() {
    GetAudioGainTableParamFromNV(&mCustomVolume);
    GetAudioBTGainParamFromNV(&mCustomVolume_BT);

    //-----MIC VOLUME SETTING
    SetULTotalGain(Idle_Normal_Record, mCustomVolume.mic.mic[Idle_Normal_Record].digital);

    SetULTotalGain(Idle_Headset_Record, mCustomVolume.mic.mic[Idle_Headset_Record].digital);

    // voice reconition usage
    SetULTotalGain(Voice_Rec_Mic_Handset, mCustomVolume.mic.mic[Voice_Rec_Mic_Handset].digital);

    SetULTotalGain(Voice_Rec_Mic_Headset, mCustomVolume.mic.mic[Voice_Rec_Mic_Headset].digital);

    SetULTotalGain(Idle_Video_Record_Handset, mCustomVolume.mic.mic[Idle_Video_Record_Handset].digital);

    SetULTotalGain(Idle_Video_Record_Headset, mCustomVolume.mic.mic[Idle_Video_Record_Headset].digital);

    SetULTotalGain(Normal_Mic, mCustomVolume.mic.mic[Normal_Mic].digital);

    SetULTotalGain(Headset_Mic, mCustomVolume.mic.mic[Headset_Mic].digital);

    SetULTotalGain(Handfree_Mic, mCustomVolume.mic.mic[Handfree_Mic].digital);

    SetULTotalGain(Normal_WB__Mic, mCustomVolume.mic.mic[Normal_WB__Mic].digital);

    SetULTotalGain(Headset_WB_Mic, mCustomVolume.mic.mic[Headset_WB_Mic].digital);

    SetULTotalGain(Handfree_WB_Mic, mCustomVolume.mic.mic[Handfree_WB_Mic].digital);

    // add by chiepeng for VOIP mode
    SetULTotalGain(VOIP_Normal_Mic, mCustomVolume.mic.mic[VOIP_Normal_Mic].digital);

    SetULTotalGain(VOIP_Headset_Mic, mCustomVolume.mic.mic[VOIP_Headset_Mic].digital);

    SetULTotalGain(VOIP_Handfree_Mic, mCustomVolume.mic.mic[VOIP_Handfree_Mic].digital);

    SetULTotalGain(TTY_CTM_Mic, mCustomVolume.mic.mic[TTY_CTM_Mic].digital);

    SetULTotalGain(Level_Shift_Buffer_Gain, mCustomVolume.mic.mic[Level_Shift_Buffer_Gain].digital);

    SetULTotalGain(Analog_PLay_Gain, mCustomVolume.mic.mic[Analog_PLay_Gain].digital);

    // voice unlock usage (input source AUDIO_SOURCE_VOICE_UNLOCK)
    SetULTotalGain(Voice_UnLock_Mic_Handset, mCustomVolume.mic.mic[Voice_UnLock_Mic_Handset].digital);

    SetULTotalGain(Voice_UnLock_Mic_Headset, mCustomVolume.mic.mic[Voice_UnLock_Mic_Headset].digital);

    //MIC gain for customization input source usage
    SetULTotalGain(Customization1_Mic_Handset, mCustomVolume.mic.mic[Customization1_Mic_Handset].digital);

    SetULTotalGain(Customization1_Mic_Headset, mCustomVolume.mic.mic[Customization1_Mic_Headset].digital);

    SetULTotalGain(Customization2_Mic_Handset, mCustomVolume.mic.mic[Customization2_Mic_Handset].digital);

    SetULTotalGain(Customization2_Mic_Headset, mCustomVolume.mic.mic[Customization2_Mic_Headset].digital);

    SetULTotalGain(Customization3_Mic_Handset, mCustomVolume.mic.mic[Customization3_Mic_Handset].digital);

    SetULTotalGain(Customization3_Mic_Headset, mCustomVolume.mic.mic[Customization3_Mic_Headset].digital);

    return NO_ERROR;
}


status_t AudioMTKGainController::initCheck() {
    return mInitDone;
}

status_t AudioMTKGainController::SetCaptureGain(audio_mode_t mode, audio_source_t source, audio_devices_t input_device, audio_devices_t output_devices) {
#if 1
    ALOGD("+%s(), mode=%d, source=%d, input device=0x%x, output device=0x%x", __FUNCTION__, mode, source, input_device, output_devices);
    switch (mode) {
    case AUDIO_MODE_NORMAL:
    case AUDIO_MODE_RINGTONE: {
        if (source == AUDIO_SOURCE_VOICE_RECOGNITION) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Voice_Rec_Mic_Headset, mode);
            } else {
                ApplyMicGain(Voice_Rec_Mic_Handset, mode);
            }
        } else if (source == AUDIO_SOURCE_CAMCORDER) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Idle_Video_Record_Headset, mode);
            } else {
                ApplyMicGain(Idle_Video_Record_Handset, mode);
            }
        } else if (source == AUDIO_SOURCE_VOICE_UNLOCK) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Voice_UnLock_Mic_Headset, mode);
            } else {
                ApplyMicGain(Voice_UnLock_Mic_Handset, mode);
            }
        } else if (source == AUDIO_SOURCE_CUSTOMIZATION1) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Customization1_Mic_Headset, mode);
            } else {
                ApplyMicGain(Customization1_Mic_Handset, mode);
            }
        } else if (source == AUDIO_SOURCE_CUSTOMIZATION2) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Customization2_Mic_Headset, mode);
            } else {
                ApplyMicGain(Customization2_Mic_Handset, mode);
            }
        } else if (source == AUDIO_SOURCE_CUSTOMIZATION3) {
            if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                ApplyMicGain(Customization3_Mic_Headset, mode);
            } else {
                ApplyMicGain(Customization3_Mic_Handset, mode);
            }
        } else {
            //for audio tuning tool tuning case.
            if (mAudioSpeechEnhanceInfoInstance->IsAPDMNRTuningEnable()) {  //for DMNR tuning
                if (mAudioSpeechEnhanceInfoInstance->GetAPTuningMode() == HANDSFREE_MODE_DMNR) {
                    ApplyMicGain(Handfree_Mic, mode);
                } else if (mAudioSpeechEnhanceInfoInstance->GetAPTuningMode() == NORMAL_MODE_DMNR) {
                    ApplyMicGain(Normal_Mic, mode);
                } else {
                    ApplyMicGain(Idle_Normal_Record, mode);
                }
            } else {
                if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                    ApplyMicGain(Idle_Headset_Record, mode);
                } else {
                    ApplyMicGain(Idle_Normal_Record, mode);
                }
            }
        }
        break;
    }
    case AUDIO_MODE_IN_CALL: {
        if (AudioALSASpeechPhoneCallController::getInstance()->checkTtyNeedOn() == false) {
            if (output_devices == AUDIO_DEVICE_OUT_EARPIECE) {
                ApplyMicGain(Normal_Mic, mode);
            } else if (output_devices == AUDIO_DEVICE_OUT_SPEAKER) {
                ApplyMicGain(Handfree_Mic, mode);
            } else {
                ApplyMicGain(Headset_Mic, mode);
            }
        }
        break;
    }
    case AUDIO_MODE_IN_COMMUNICATION: {
        if (output_devices == AUDIO_DEVICE_OUT_EARPIECE) {
            ApplyMicGain(VOIP_Normal_Mic, mode);
        } else if (output_devices == AUDIO_DEVICE_OUT_SPEAKER) {
            ApplyMicGain(VOIP_Handfree_Mic, mode);
        } else {
            ApplyMicGain(VOIP_Headset_Mic, mode);
        }
        break;
    }
    }
#else

#endif
    return NO_ERROR;
}


status_t AudioMTKGainController::speechBandChange(bool nb) {
    SLOGD("speechBandChange nb %d", nb);

    AutoMutex lock(mLock);
    if (mSpeechNB != nb) {
        mSpeechNB = nb;
        setAnalogVolume_l(mHwStream.stream, mHwStream.device, mHwStream.index, AUDIO_MODE_IN_CALL);
    }
    return NO_ERROR;
}

bool AudioMTKGainController::isNbSpeechBand(void) {
    AutoMutex lock(mLock);
    return mSpeechNB;
}

status_t AudioMTKGainController::setBtVolumeCapability(bool support) {
    AutoMutex lock(mLock);
    mSupportBtVol = !support; // if bt device do not support volume , we should
    return NO_ERROR;
}

status_t AudioMTKGainController::setAnalogVolume_l(int stream, int device, int index, audio_mode_t mode) {
    SLOGD("setAnalogVolume stream %d device 0x%x index %d mode %d", stream, device, index, mode);
    mHwStream.stream = stream;
    mHwStream.device = device;
    mHwStream.index  = index;
    if (isInVoiceCall(mode)) {
        setVoiceVolume(index, device, mode);
    } else {
        setNormalVolume(stream, index, device, mode);
    }

    return NO_ERROR;
}

status_t AudioMTKGainController::setAnalogVolume(int stream, int device, int index, audio_mode_t mode) {
    AutoMutex lock(mLock);
    return setAnalogVolume_l(stream, device, index, mode);
}

status_t AudioMTKGainController::setNormalVolume(int stream, int index, int device, audio_mode_t mode) {
    SLOGD("setNormalVolume stream %d, index 0x%x, device 0x%x, mode 0x%x", stream, index, device, mode);
    int arrDevices[] = {AUDIO_DEVICE_OUT_SPEAKER, AUDIO_DEVICE_OUT_WIRED_HEADSET, AUDIO_DEVICE_OUT_WIRED_HEADPHONE, AUDIO_DEVICE_OUT_EARPIECE};
    bool blnFlag = false;
    for (int i = 0; i < 4; i++) {
        if ((arrDevices[i] & device) == arrDevices[i]) {
            blnFlag = true;
            output_gain_device gainDevice = getGainDevice(arrDevices[i]);
            AUDIO_GAIN_TABLE_STRUCT *gainTable = &mCustomVolume;
            STREAM_GAIN *streamGain = &gainTable->voiceCall + stream;
            uint8_t analog = streamGain->stream[gainDevice].analog;
            //set speaker gain
            if (arrDevices[i] == AUDIO_DEVICE_OUT_SPEAKER) {
                uint8_t spkGain =  streamGain->stream[gainDevice].amp[4];
                if (spkGain == 0) { //avoid mute in k2v1 phone
                    spkGain = 1;
                }
                setSpeakerGain(spkGain);
            }
            //set PGA gain
            ApplyAudioGain(analog, mode, arrDevices[i]);
            setAMPGain(streamGain->stream[gainDevice].amp, AMP_CONTROL_POINT, arrDevices[i]);
            device &= (~arrDevices[i]);
        }
    }
    if (!blnFlag) {
        output_gain_device gainDevice = GAIN_OUTPUT_HEADSET;
        AUDIO_GAIN_TABLE_STRUCT *gainTable = &mCustomVolume;
        STREAM_GAIN *streamGain = &gainTable->voiceCall + stream;
        uint8_t analog = streamGain->stream[gainDevice].analog;
        //set PGA gain
        ApplyAudioGain(analog, mode, device);
        setAMPGain(streamGain->stream[gainDevice].amp, AMP_CONTROL_POINT, device);
    }
    //mAudioResourceManager->SetInputDeviceGain();
    //mStreamManager->SetCaptureGain();
    AudioALSAStreamManager::getInstance()->SetCaptureGain();
    return NO_ERROR;
}

status_t AudioMTKGainController::setVoiceVolume(int index, int device, audio_mode_t mode) {
    SLOGD("setVoiceVolume index %d, device 0x%x, mode %d", index, device, mode);
    output_gain_device gainDevice = getGainDevice(device);
    AUDIO_GAIN_TABLE_STRUCT *gainTable = &mCustomVolume;
    AUDIO_BT_GAIN_STRUCT *gainTableBT = &mCustomVolume_BT;
    uint8_t analog = 0;
    uint8_t digitalDegradeDb = 0;
    void *ampgain = NULL;
    STREAM_ITEM_GAIN *streamGain = NULL;
    SLOGD("mSupportBtVol is %d, mSpeechNB is %d", mSupportBtVol, mSpeechNB);
    if (audio_is_bluetooth_sco_device(device)) {
#ifdef MTK_AUDIO_GAIN_TABLE_BT
        // BT Headset NREC
        bool bt_headset_nrec_on =  SpeechEnhancementController::GetInstance()->GetBtHeadsetNrecOn();
        if (bt_headset_nrec_on) {
            streamGain =  &gainTable->blueToothSco.stream[gainDevice];
        } else {
            streamGain =  &gainTableBT->blueToothNrec.stream[gainDevice];
        }
#else
        streamGain =  &gainTable->blueToothSco.stream[gainDevice];

#endif
        if (mSupportBtVol) {
            digitalDegradeDb = streamGain->digital[index];
        } else {
            digitalDegradeDb = streamGain->digital[15];
        }
        ApplyMdDlGain(digitalDegradeDb);  // modem dl gain
        ApplyMdUlGain(0);
        return NO_ERROR;
    } else {
        SPEECH_GAIN *streamGain = mSpeechNB ? &gainTable->speechNB : &gainTable->speechWB;
        SPEECH_ITEM_GAIN *item = &streamGain->speech[gainDevice][index];
        analog  = item->analog;
        digitalDegradeDb = item->digital ;
        ampgain = item->amp;
    }
    //set spk gain
    if (device  == AUDIO_DEVICE_OUT_SPEAKER) {
        uint8_t spkGain = ((uint8_t *)ampgain)[4];
        if (spkGain == 0) { //avoid mute in k2v1 phone
            spkGain = 1;
        }
        setSpeakerGain(spkGain);
    }
    ApplyAudioGain(analog, mode, device); // audiobuffer / voice buffer
    setAMPGain(ampgain, AMP_CONTROL_POINT, device);
    ApplyMdDlGain(digitalDegradeDb);  // modem dl gain
    ApplyMicGainByDevice(device, mode);      // mic gain & modem UL gain
    ApplySideTone(gainDevice);
    return NO_ERROR;
}


void  AudioMTKGainController::ApplyMicGainByDevice(uint32_t device, audio_mode_t mode) {
    if (device & AUDIO_DEVICE_OUT_EARPIECE) {
        ApplyMicGain(mSpeechNB ? Normal_Mic : Normal_WB__Mic, mode); // set incall mic gain
    } else if (device & AUDIO_DEVICE_OUT_WIRED_HEADSET ||  device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        ApplyMicGain(mSpeechNB ? Headset_Mic : Headset_WB_Mic, mode); // set incall mic gain
    } else if (device & AUDIO_DEVICE_OUT_SPEAKER) {
        ApplyMicGain(mSpeechNB ? Handfree_Mic : Handfree_WB_Mic, mode); // set incall mic gain
    } else if (audio_is_bluetooth_sco_device(device)) {
        //when use BT_SCO , apply digital to 0db.
        ApplyMdUlGain(0);
    }
}

BUFFER_TYPE AudioMTKGainController::getBufferType(int device, audio_mode_t mode) {

#ifdef USING_EXTAMP_ALL_VOICE_BUFFER
    if (isInVoiceCall(mode)) {
        return VOICE_BUFFER;
    }
#endif
    if (device  ==  AUDIO_DEVICE_OUT_EARPIECE) {
        return VOICE_BUFFER;
    } else if ((device  == AUDIO_DEVICE_OUT_WIRED_HEADSET)
               || device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE
              ) {
        return AUDIO_BUFFER;
    } else if (device  == AUDIO_DEVICE_OUT_SPEAKER) {
        return VOICE_BUFFER;
    } else {
        return INVALID_BUFFFER;
    }
}

output_gain_device   AudioMTKGainController::getGainDevice(int device) {
    output_gain_device gainDevice = GAIN_OUTPUT_SPEAKER;

    if (device == AUDIO_DEVICE_OUT_SPEAKER) {
        gainDevice = GAIN_OUTPUT_SPEAKER;
    } else if (device == AUDIO_DEVICE_OUT_WIRED_HEADSET
               || device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE
              ) {
        gainDevice = GAIN_OUTPUT_HEADSET;
    } else if (device == AUDIO_DEVICE_OUT_EARPIECE) {
        gainDevice = GAIN_OUTPUT_EARPIECE ;
    } else {
        gainDevice = GAIN_OUTPUT_HEADSET;
    }

    return gainDevice;
}


void AudioMTKGainController::ApplyMdDlGain(int32_t degradeDb) {
    // set degarde db to mode side, DL part, here use degrade dbg
    ALOGD("ApplyMdDlGain degradeDb = %d", degradeDb);
#if 0
    if (degradeDb >= keyDLDigitalDegradeMax) {
        degradeDb = keyDLDigitalDegradeMax;
    }

    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetDownlinkGain((-1 * degradeDb) << 2); // degrade db * 4
#endif
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetDownlinkGain((-1 * degradeDb));
}

void AudioMTKGainController::ApplyMdDlEhn1Gain(int32_t Gain) {
    // set degarde db to mode side, DL part, here use degrade dbg
    ALOGD("ApplyMdDlEhn1Gain degradeDb = %d", Gain);
    //SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetEnh1DownlinkGain(-1 * (Gain) << 2); // degrade db * 4
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetEnh1DownlinkGain(-1 * (Gain));
}


void AudioMTKGainController::ApplyMdUlGain(int32_t IncreaseDb) {
    // set degarde db to mode side, UL part, here use positive gain becasue SW_agc always positive
    ALOGD("ApplyMdUlGain degradeDb = %d", IncreaseDb);

    //if (mHwVolume.swAgc != IncreaseDb)
    {
        mHwVolume.swAgc = IncreaseDb;
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetUplinkGain(IncreaseDb << 2); // degrade db * 4
    }
}


void AudioMTKGainController::ApplyAudioGain(int gain, uint32_t mode, uint32_t device) {
    ALOGD("ApplyAudioGain  gain = %d mode= %d device = %d", gain, mode, device);
    BUFFER_TYPE bufferType = getBufferType(device, (audio_mode_t)mode);
    int bufferGain;

#ifdef INDEX_REMAP
    gain = keyVoiceBufferStep - 1 - gain; //here gain means index
#endif

    bufferGain = gain;

    /*
    if (device  == AUDIO_DEVICE_OUT_SPEAKER)
    {
        if (isInVoiceCall((audio_mode_t) mode))
        {
            gain = 10; //fixed speak at 12db;
        }
        //else
        //{
        //   bufferGain = 11; //fixed hs at 1db;
        //}

        setSpeakerGain(gain);
    }
    */
    if (bufferType == VOICE_BUFFER) {
        setVoiceBufferGain(bufferGain);
    } else if (bufferType == AUDIO_BUFFER) {
        setAudioBufferGain(bufferGain);
    }
}

int AudioMTKGainController::GetReceiverGain(void) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    int index;
    ALOGD("GetReceiverGain");
    ctl = mixer_get_ctl_by_name(mMixer, "Handset_PGA_GAIN");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        index = mixer_ctl_get_value(ctl, i);
        ALOGD("GetReceiverGain i = %d index = %d ", i, index);
    }
    return index;
}

int AudioMTKGainController::GetHeadphoneRGain(void) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    int index;
    ALOGD("GetHeadphoneRGain");
    ctl = mixer_get_ctl_by_name(mMixer, "Headset_PGAR_GAIN");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        index = mixer_ctl_get_value(ctl, i);
        ALOGD("GetHeadphoneRGain i = %d index = %d ", i, index);
    }
    return index;
}


int AudioMTKGainController::GetHeadphoneLGain(void) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    int index;
    ALOGD("GetHeadphoneLGain");
    ctl = mixer_get_ctl_by_name(mMixer, "Headset_PGAL_GAIN");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        index = mixer_ctl_get_value(ctl, i);
        ALOGD("GetHeadphoneLGain i = %d index = %d ", i, index);
    }
    return index;
}

void AudioMTKGainController::SetReceiverGain(int index) {
    ALOGD("SetReceiverGain = %d", index);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    ctl = mixer_get_ctl_by_name(mMixer, "Handset_PGA_GAIN");
    type = mixer_ctl_get_type(ctl);

    if (index < 0) {
        index = 0;
    }

    if ((uint32_t)index >= _countof(DL_PGA_Handset_GAIN)) {
        index = _countof(DL_PGA_Handset_GAIN) - 1;
    }

    if (mixer_ctl_set_enum_by_string(ctl, DL_PGA_Handset_GAIN[index])) {
        ALOGE("Error: Handset_PGA_GAIN invalid value");
    }
}

void AudioMTKGainController::SetHeadPhoneLGain(int index) {
    ALOGD("SetHeadPhoneLGain = %d", index);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    ctl = mixer_get_ctl_by_name(mMixer, "Headset_PGAL_GAIN");
    type = mixer_ctl_get_type(ctl);

    if (index < 0) {
        index = 0;
    }

    if ((uint32_t)index >= _countof(DL_PGA_Headset_GAIN)) {
        index = _countof(DL_PGA_Headset_GAIN) - 1;
    }

    if (mixer_ctl_set_enum_by_string(ctl, DL_PGA_Headset_GAIN[index])) {
        ALOGE("Error: Headset_PGAL_GAIN invalid value");
    }
}

void AudioMTKGainController::SetHeadPhoneRGain(int index) {
    ALOGD("SetHeadPhoneRGain = %d", index);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    ctl = mixer_get_ctl_by_name(mMixer, "Headset_PGAR_GAIN");
    type = mixer_ctl_get_type(ctl);

    if (index < 0) {
        index = 0;
    }

    if ((uint32_t)index >= _countof(DL_PGA_Headset_GAIN)) {
        index = _countof(DL_PGA_Headset_GAIN) - 1;
    }

    if (mixer_ctl_set_enum_by_string(ctl, DL_PGA_Headset_GAIN[index])) {
        ALOGE("Error: Headset_PGAR_GAIN invalid value");
    }
}

void   AudioMTKGainController::setAudioBufferGain(int gain) {
    if (gain >= keyAudioBufferStep) {
        gain = keyAudioBufferStep;
    }

#if 1
    int regIndex;
    regIndex = GetHeadphoneLGain();
    ALOGD("setAudioBufferGain, gain %d, mHwVolume.audioBuffer %d,regIndex:%d", gain, mHwVolume.audioBuffer, regIndex);
    mHwVolume.audioBuffer = regIndex;

    if (mHwVolume.audioBuffer != gain) {
        mHwVolume.audioBuffer = gain;

        ALOGD("setAudioBufferGain = %d", gain);
        SetHeadPhoneLGain(gain);
        SetHeadPhoneRGain(gain);
    }
#else
    int regIndex =  mAudioAnalogControl->GetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, true);
    ALOGD("setAudioBufferGain, gain %d, mHwVolume.audioBuffer %d,regIndex:%d", gain, mHwVolume.audioBuffer, regIndex);
    mHwVolume.audioBuffer = regIndex;

    if (mHwVolume.audioBuffer != gain) {
        mHwVolume.audioBuffer = gain;
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, gain);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, gain);
    }
#endif
}

void  AudioMTKGainController::setVoiceBufferGain(int gain) {
    if (gain >= keyVoiceBufferStep) {
        gain = keyVoiceBufferStep;
    }
#if 1
    int regIndex = GetReceiverGain();
    ALOGD("setVoiceBufferGain, gain %d, mHwVolume.voiceBuffer %d,regIndex:%d", gain, mHwVolume.voiceBuffer, regIndex);
    mHwVolume.voiceBuffer = regIndex;

    if (mHwVolume.voiceBuffer != gain) {
        mHwVolume.voiceBuffer = gain;
        SetReceiverGain(gain);
    }
#else
    int regIndex =  mAudioAnalogControl->GetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, true);
    ALOGD("setVoiceBufferGain, gain %d, mHwVolume.voiceBuffer %d,regIndex:%d", gain, mHwVolume.voiceBuffer, regIndex);
    mHwVolume.voiceBuffer = regIndex;

    if (mHwVolume.voiceBuffer != gain) {
        mHwVolume.voiceBuffer = gain;
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, gain);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTR, gain);
    }
#endif
}

int AudioMTKGainController::GetSPKGain(void) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    int index;
    ALOGD("GetSPKGain");
    ctl = mixer_get_ctl_by_name(mMixer, "Audio_Speaker_PGA_gain");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        index = mixer_ctl_get_value(ctl, i);
        ALOGD("GetSPKGain i = %d index = %d ", i, index);
    }
    // for 0 is mute...
    return index;
}

void AudioMTKGainController::SetSpeakerGain(int index) {
    ALOGD("SetSpeakerGain,index=%d \n", index);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    ctl = mixer_get_ctl_by_name(mMixer, "Audio_Speaker_PGA_gain");
    type = mixer_ctl_get_type(ctl);

    if (index < 0) {
        index = 0;
    }

    if (mixer_ctl_set_enum_by_string(ctl, DL_PGA_SPEAKER_GAIN[index])) {
        ALOGE("Error: SetSpeakerGain invalid value");
    }
}

void   AudioMTKGainController::setSpeakerGain(int gain) {
    if (gain >= keySPKStep) {
        gain = keySPKStep;
    }
#if 1
    int regIndex = GetSPKGain();
    ALOGD("setSpeakerGain, gain %d, mHwVolume.speaker %d,regIndex:%d", gain, mHwVolume.speaker, regIndex);
    mHwVolume.speaker = regIndex;

    if (mHwVolume.speaker != gain) {
        mHwVolume.speaker = gain;
        SetSpeakerGain(gain);
    }
#else
    int regIndex =  mAudioAnalogControl->GetAnalogGain(AudioAnalogType::VOLUME_SPKL, true);
    ALOGD("setSpeakerGain, gain %d, mHwVolume.speaker %d,regIndex:%d", gain, mHwVolume.speaker, regIndex);
    mHwVolume.speaker = regIndex;

    if (mHwVolume.speaker != gain) {
        mHwVolume.speaker = gain;
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_SPKL, gain);
        mAudioAnalogControl->SetAnalogGain(AudioAnalogType::VOLUME_SPKR, gain);
    }
#endif
}

void   AudioMTKGainController::setAMPGain(void *points, int num, int device) {
    ALOGD("setAMPGain, device %d", device);
#ifdef USING_EXTAMP_TC1
    if (mAmpControl && points) {
        mAmpControl->setVolume(points, num, device);
    }
#endif
}

void AudioMTKGainController::SetAdcPga1(int gain) {
    ALOGD("SetAdcPga1 = %d", gain);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    int num_values = 0;
    ctl = mixer_get_ctl_by_name(mMixer, "Audio_PGA1_Setting");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    if (gain < 0) {
        gain = 0;
    }

    if (mixer_ctl_set_enum_by_string(ctl, PGA_Gain_String[gain])) {
        ALOGE("Error: Audio_PGA1_Setting invalid value");
    }
}
void AudioMTKGainController::SetAdcPga2(int gain) {
    ALOGD("SetAdcPga2 = %d", gain);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    int num_values = 0;
    if (gain < 0) {
        gain = 0;
    }

    ctl = mixer_get_ctl_by_name(mMixer, "Audio_PGA2_Setting");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    if (mixer_ctl_set_enum_by_string(ctl, PGA_Gain_String[gain])) {
        ALOGE("Error: Audio_PGA2_Setting invalid value");
    }
}

status_t AudioMTKGainController::ApplyMicGain(uint32_t MicType, int mode) {
    uint8_t analogidx;
    uint8_t analog  = mCustomVolume.mic.mic[MicType].digital;
    uint8_t degradedb = AUDIO_SYSTEM_UL_GAIN_MAX - analog;
    uint8_t analogdegrade = PGA_Gain_Map[degradedb];
    uint8_t swagcmap  = SwAgc_Gain_Map[degradedb];

    mULTotalGain = mULTotalGainTable[MicType];

    ALOGD("ApplyMicGain MicType=%d mode=%d micgain=%d mULTotalGain=%d", MicType, mode, analog, mULTotalGain);

    //if (mHwVolume.micGain != analog)
    //{
    mHwVolume.micGain = analog;
    analogidx = (MAX_PGA_GAIN_RANGE - analogdegrade) / AUDIO_UL_PGA_STEP;
    SetAdcPga1(analogidx);
    SetAdcPga2(analogidx);
    //}

    mHwVolume.swAgc = swagcmap;
    if (isInVoiceCall((audio_mode_t)mode)) {
        ApplyMdUlGain(swagcmap);
    }
    return NO_ERROR;
}

short  AudioMTKGainController::GetSWMICGain() {
    return mHwVolume.swAgc ;
}

status_t AudioMTKGainController::ApplySideTone(uint32_t Mode) {
    // here apply side tone gain, need base on UL and DL analog gainQuant
    AUDIO_GAIN_TABLE_STRUCT *gainTable = &mCustomVolume;
    SIDETONE_GAIN *sideTone = NULL;
    if (mSpeechNB) {
        sideTone = &gainTable->sideToneNB;
    } else {
        sideTone = &gainTable->sideToneWB;
    }
    uint8_t sidetone =  sideTone->sidetone[Mode];

    if (sidetone > keySidetoneSize) {
        sidetone = keySidetoneSize;
    }

    //if (mHwVolume.sideTone != sidetone)
    {
        mHwVolume.sideTone = sidetone;
        uint16_t value = sidetone;

#if 1
        if (Mode == EarPiece_SideTone_Gain) {
            value = updateSidetone(KeyVoiceBufferGain[GetReceiverGain()], sidetone, mHwVolume.swAgc);
        } else if (Mode == Headset_SideTone_Gain) {
            value = updateSidetone(KeyAudioBufferGain[GetHeadphoneRGain()], sidetone, mHwVolume.swAgc);
        } else if (Mode == LoudSpk_SideTone_Gain) {
            //value = updateSidetone(KeyVoiceBufferGain[GetReceiverGain()], sidetone, mHwVolume.swAgc);
            // mute sidetone gain when speaker mode.
            value = 0;
        }
#endif
        SLOGD("ApplySideTone Mode %d, sidetone %u, value %u", Mode, sidetone, value);
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetSidetoneGain(value);
    }

    return NO_ERROR;
}

uint16_t AudioMTKGainController::updateSidetone(int dlPGAGain, int  sidetone, uint8_t ulGain) {

    int vol = 0;
    uint16_t DSP_ST_GAIN = 0;
    SLOGD("updateSidetone dlPGAGain %d, sidetone %d, ulGain %u", dlPGAGain, sidetone, ulGain);

    if (sidetone == 0) {
        DSP_ST_GAIN = 0 ;
    } else {
        vol = sidetone + ulGain; //1dB/step
        vol = dlPGAGain - vol + 67 - keyULGainOffset;
        if (vol < 0) {
            vol = 0;
        }
        if (vol > keySidetoneSize) {
            vol = keySidetoneSize;
        }
        DSP_ST_GAIN = keySideTone[vol];
    }
    SLOGD("DSP_ST_GAIN=%d", DSP_ST_GAIN);
    return DSP_ST_GAIN;
}

bool AudioMTKGainController::isInVoiceCall(audio_mode_t mode) {
    return (mode == AUDIO_MODE_IN_CALL);
}

bool AudioMTKGainController::isInVoipCall(audio_mode_t mode) {
    return mode == AUDIO_MODE_IN_COMMUNICATION;
}

bool AudioMTKGainController::isInCall(audio_mode_t mode) {
    return (isInVoiceCall(mode) || isInVoipCall(mode));
}


//static functin to get FM power state
#define BUF_LEN 1
static char rbuf[BUF_LEN] = {'\0'};
static char wbuf[BUF_LEN] = {'1'};
static const char *FM_POWER_STAUTS_PATH = "/proc/fm";
static const char *FM_DEVICE_PATH = "dev/fm";

status_t AudioMTKGainController::setFmVolume(const float fm_volume) {
    ALOGV("%s(), fm_volume = %f", __FUNCTION__, fm_volume);

    // Calculate HW Gain Value
    uint32_t volume_index = logToLinear(fm_volume); // 0 ~ 256
    uint32_t hw_gain = kHWGainMap[volume_index >> 1]; // 0 ~ 0x80000

    // Set HW Gain
    return mHardwareResourceManager->setHWGain2DigitalGain(hw_gain);
}

bool AudioMTKGainController::SetFmChipVolume(int volume) {
    ALOGD("%s is deprecated and not used", __FUNCTION__);
    return false;
}
int  AudioMTKGainController::GetFmVolume(void) {
    ALOGD("%s is deprecated and not used", __FUNCTION__);
    return 0;
}

//matv adjust digital
void AudioMTKGainController::GetMatvService() {
#ifndef MTK_BASIC_PACKAGE

    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder;

    do {
        binder = sm->getService(String16("media.ATVCtrlService"));

        if (binder != 0) {
            break;
        }

        ALOGW("ATVCtrlService not published, waiting...");
        usleep(1000 * 1000); // 1 s
    } while (true);

    spATVCtrlService = interface_cast<IATVCtrlService>(binder);
#endif
}

bool AudioMTKGainController::SetMatvMute(bool b_mute) {
    ALOGD("SetMatvMute(%d), mMatvVolume(%d)", b_mute, mMatvVolume);

    return true;
}

bool AudioMTKGainController::setMatvVolume(int volume) {
    ALOGD("setMatvVolume volume=%d", volume);
    mMatvVolume = volume;
    return true;
}

int AudioMTKGainController::GetMatvVolume(void) {
    return mMatvVolume;
}

/********************************************************************************
*
*
*
*                                                            UnUsed API
*
*
*
***********************************************************************************/

uint16_t AudioMTKGainController::MappingToDigitalGain(unsigned char Gain) {
    return 0;
}

uint16_t AudioMTKGainController::MappingToPGAGain(unsigned char Gain) {
    return 0;
}

status_t AudioMTKGainController::setMasterVolume(float v, audio_mode_t mode, uint32_t devices) {
    mMasterVolume = v;
    if (!isInVoiceCall(mode)) {
        ALOGD("setMasterVolume call setNormalVolume");
        setNormalVolume(mHwStream.stream, mHwStream.index, devices, mode);
    }
    return NO_ERROR;
}

float AudioMTKGainController::getMasterVolume() {
    ALOGD("AudioMTKGainController getMasterVolume");
    return mMasterVolume;
}

status_t AudioMTKGainController::setVoiceVolume(int MapVolume, uint32_t device) {
    //API for AudioALSAParamTunner Audio Taste
    ALOGD("+%s(), MapVolume=0x%x, device=0x%x", __FUNCTION__, MapVolume, device);
    setVoiceVolume(mHwStream.index, device, AUDIO_MODE_IN_CALL);
    return NO_ERROR;
}

status_t AudioMTKGainController::setVoiceVolume(float v, audio_mode_t mode, uint32_t device) {
    mVoiceVolume = v;
    ALOGD("call setVoiceVolume");
    setVoiceVolume(mHwStream.index, device, mode);
    return NO_ERROR;
}

status_t AudioMTKGainController::ApplyVoiceGain(int degradeDb, audio_mode_t mode, uint32_t device) {
    ALOGD("%s(), Do Nothing. Return.", __FUNCTION__);
    return NO_ERROR;
}

float AudioMTKGainController::getVoiceVolume(void) {
    ALOGD("AudioMTKGainController getVoiceVolume");
    return mVoiceVolume;
}

status_t AudioMTKGainController::setStreamVolume(int stream, float v) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::setStreamMute(int stream, bool mute) {
    return INVALID_OPERATION;
}

float AudioMTKGainController::getStreamVolume(int stream) {
    return 1.0;
}

// should depend on different usage , FM ,MATV and output device to setline in gain
status_t AudioMTKGainController::SetLineInPlaybackGain(int type) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::SetLineInRecordingGain(int type) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::SetSideTone(uint32_t Mode, uint32_t Gain) {

    return INVALID_OPERATION;
}

uint32_t AudioMTKGainController::GetSideToneGain(uint32_t device) {

    return INVALID_OPERATION;
}


status_t AudioMTKGainController::SetMicGain(uint32_t Mode, uint32_t Gain) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::SetULTotalGain(uint32_t Mode, unsigned char Gain) {
    if (Gain > AUDIO_SYSTEM_UL_GAIN_MAX) {
        Gain = AUDIO_SYSTEM_UL_GAIN_MAX;
    }

    mULTotalGainTable[Mode] = UPLINK_GAIN_MAX - ((AUDIO_SYSTEM_UL_GAIN_MAX - Gain) * UPLINK_ONEDB_STEP);
    ALOGD("SetULTotalGain MicMode=%d, Volume=%d, mULTotalGain=%d", Mode, Gain, mULTotalGainTable[Mode]);
    return NO_ERROR;
}

status_t AudioMTKGainController::SetDigitalHwGain(uint32_t Mode, uint32_t Gain, uint32_t routes) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::SetMicGainTuning(uint32_t Mode, uint32_t Gain) {
    return INVALID_OPERATION;
}

bool AudioMTKGainController::GetHeadPhoneImpedance(void) {
    return true;
}

int AudioMTKGainController::ApplyAudioGainTuning(int Gain, uint32_t mode, uint32_t device) {
    return 0;
}

void AudioMTKGainController::SetLinoutRGain(int DegradedBGain) {

}

void AudioMTKGainController::SetLinoutLGain(int DegradedBGain) {

}

uint32_t AudioMTKGainController::GetOffloadGain(float vol_f) {
    return 0;
}

}

#include <math.h>
#include <SpeechDriverFactory.h>

#include "AudioALSAGainController.h"
#include <AudioALSASpeechPhoneCallController.h>
#include "SpeechEnhancementController.h"
//#include "AudioAMPControlInterface.h"
#include "AudioALSADeviceConfigManager.h"
#include "AudioALSADeviceParser.h"
#include "AudioALSADriverUtility.h"
#include "AudioALSAStreamManager.h"
#include "AudioALSAHardwareResourceManager.h"
#ifdef MTK_USB_PHONECALL
#include "AudioUSBPhoneCallController.h"
#endif
#ifdef MTK_AUDIO_SPH_LPBK_PARAM
#include "LoopbackManager.h"
#endif

#include "AudioTypeExt.h"
#if defined(MTK_HYBRID_NLE_SUPPORT)
#include "AudioALSANLEController.h"
#endif
#include <SpeechConfig.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioMTKGainController"

#ifdef ALOGG
#undef ALOGG
#endif
#ifdef CONFIG_MT_ENG_BUILD
#define ALOGG(...) ALOGD(__VA_ARGS__)
#else
#define ALOGG(...)
#endif

#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))

namespace android {
AudioMTKGainController *AudioMTKGainController::UniqueVolumeInstance = NULL;

// here can change to match audiosystem

// total 64 dB
static const float keydBPerStep = 0.25f;
static const float keyvolumeStep = 255.0f;


// shouldn't need to touch these
static const float keydBConvert = -keydBPerStep * 2.302585093f / 20.0f;
static const float keydBConvertInverse = 1.0f / keydBConvert;

//static const char *PGA_Gain_String[] = {"0Db", "6Db", "12Db" , "18Db" , "24Db" , "30Db"};

//hw spec db
//const int keyAudioBufferStep       =   19;
//const int KeyAudioBufferGain[]     =  { -5, -3, -1, 1, 3, 5, 7, 9};
//const int KeyAudioBufferGain[]     =  { 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -40}; // move to xml
/*static const char *DL_PGA_Headset_GAIN[] = {"8Db", "7Db", "6Db", "5Db", "4Db", "3Db", "2Db", "1Db", "0Db", "-1Db", "-2Db", "-3Db",
                                            "-4Db", "-5Db", "-6Db", "-7Db", "-8Db", "-9Db", "-10Db" , "-40Db"
                                           };*/

//const int keyVoiceBufferStep       =   19;
//const int KeyVoiceBufferGain[]     =  { -21, -19, -17, -15, -13, -11, -9, -7, -5, -3, -1, 1, 3, 5, 7, 9};
//const int KeyVoiceBufferGain[]     =  { 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -40}; move to xml
/*static const char *DL_PGA_Handset_GAIN[] = {"8Db", "7Db", "6Db", "5Db", "4Db", "3Db", "2Db", "1Db", "0Db", "-1Db", "-2Db", "-3Db",
                                            "-4Db", "-5Db", "-6Db", "-7Db", "-8Db", "-9Db", "-10Db" , "-40Db"
                                           };*/


//const int keyULStep                =   5;
//const int KeyULGain[]              =  { -6, 0, 6, 12, 18, 24};
//const int keyULGainOffset          = 2;

//const int keySPKStep               =   15;
//const int KeySPKgain[]             =  { -60, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
/*static const char *DL_PGA_SPEAKER_GAIN[] = {"MUTE", "0Db", "4Db", "5Db", "6Db", "7Db", "8Db", "9Db", "10Db",  // move to xml
                                            "11Db", "12Db", "13Db", "14Db", "15Db", "16Db", "17Db"
                                           };*/



//const int keyDLDigitalDegradeMax   = 63;

//const int keyULDigitalIncreaseMax  = 32;

// static const int keySidetoneSize   = 47; // move to xml

/*static const uint16_t SwAgc_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] =  // move to xml
{
    25, 24, 23, 22, 21, 20, 19, 18, 17,
    16, 15, 14, 13, 12 , 11,
    16, 15, 14, 13, 12 , 11,
    16, 15, 14, 13, 12 , 11,
    16, 15, 14, 13, 12 , 11,
    16, 15, 14, 13, 12 , 11,
    10, 9, 8, 7, 6, 5, 4
};

static const uint16_t PGA_Gain_Map[AUDIO_SYSTEM_UL_GAIN_MAX + 1] =
{
    6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6,
    12, 12, 12, 12, 12, 12,
    18, 18, 18, 18, 18, 18,
    24, 24, 24, 24, 24, 24,
    30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30
};*/

//static const uint16_t keySideTone[] =     // move to xml
//{
//    32767, 29204, 26027, 23196, 20674, 18426, 16422, 14636, 13044, 11625,  /*1dB per step*/
//    10361, 9234,  8230,  7335,  6537,  5826,  5193,  4628,  4125,  3676,
//    3276,  2919,  2602,  2319,  2066,  1841,  1641,  1463,  1304,  1162,
//    1035,  923,   822,   733,   653,   582,   519,   462,   412,   367,
//    327,   291,   260,   231,   206,   183,   163,   145
//};

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

// callback function
void xmlChangedCallback(AppHandle *_appHandle, const char *_audioTypeName) {
    // reload XML file
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
        return;
    }

    if (appOps->appHandleReloadAudioType(_appHandle, _audioTypeName) == APP_ERROR) {
        ALOGE("%s(), Reload xml fail!(audioType = %s)", __FUNCTION__, _audioTypeName);
    } else {
        AudioMTKGainController::getInstance()->updateXmlParam(_audioTypeName);
    }
}

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
        ALOGV("+UniqueVolumeInstance\n");
        UniqueVolumeInstance = new AudioMTKGainController();
        ALOGV("-UniqueVolumeInstance\n");
    }

    return UniqueVolumeInstance;
}

AudioMTKGainController::AudioMTKGainController() {
    ALOGD("AudioMTKGainController contructor");
    //    mAmpControl          = AudioDeviceManger::createInstance();
    mAudioSpeechEnhanceInfoInstance = AudioSpeechEnhanceInfo::getInstance();
    mHardwareResourceManager = AudioALSAHardwareResourceManager::getInstance();
    mVoiceVolume = 1.0f;
    mMasterVolume = 1.0f;
    //mFmVolume = 0xFF;
    //mFmChipVolume = 0xFFFFFFFF;
    mBand = GAIN_SPEECH_NB;
    mNetwork = GAIN_SPEECH_NETWORK_GSM;
    mSupportBtVol = false;
    mSceneIndex = GAIN_SCENE_INDEX_DEFAULT;
    memset(&mHwVolume, 0xFF, sizeof(mHwVolume));
    memset(&mHwStream, 0xFF, sizeof(mHwStream));
    memset(&mHwCaptureInfo, 0, sizeof(mHwCaptureInfo));
    GainTableParamParser::getInstance()->getSceneList(&mSceneList);
    allocateGainTable();
    initVolumeController();
    mULTotalGain = 184;
    mHpImpedanceIdx = mSpec->hpImpDefaultIdx;
#ifdef MTK_AUDIO_SW_DRE
    mSWDREMute = false;
    mMutedHandlerVector.clear();
    mHasMuteHandler = false;
    mNumHandler = 0;
    mFmEnable = false;
#endif
    mANCEnable = false;
    mInitDone = true;
    mMixer = NULL;
    mMixer = AudioALSADriverUtility::getInstance()->getMixer();
    ALOGV("mMixer = %p", mMixer);
    ASSERT(mMixer != NULL);

    /* XML changed callback process */
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
        return;
    }

    appOps->appHandleRegXmlChangedCb(appOps->appHandleGetInstance(), xmlChangedCallback);
}

status_t AudioMTKGainController::allocateGainTable() {
    ALOGV("%s()", __FUNCTION__);
    mGainTable.sceneGain = (GainTableForScene *)malloc(mSceneList.size() * sizeof(GainTableForScene));
    if (mGainTable.sceneGain == NULL) {
        ALOGE("%s(), Allocate scene gain table fail", __FUNCTION__);
        ASSERT(0);
        return NO_MEMORY;
    }
    return NO_ERROR;
}

status_t AudioMTKGainController::freeGainTable() {
    ALOGD("%s()", __FUNCTION__);
    free(mGainTable.sceneGain);
    return NO_ERROR;
}

AudioMTKGainController::~AudioMTKGainController() {
    freeGainTable();
}

status_t AudioMTKGainController::initVolumeController() {
    GainTableParamParser::getInstance()->getGainTableParam(&mGainTable, &mSceneList);
    GainTableParamParser::getInstance()->getGainTableSpec(&mSpec);
    return NO_ERROR;
}

status_t AudioMTKGainController::getSceneGainTableParameter(GainTableForScene *_gainTableForScene) {
    ALOGV("%s()", __FUNCTION__);
    memcpy((void *)_gainTableForScene, (void *)mGainTable.sceneGain, sizeof(GainTableForScene) * mSceneList.size());
    return NO_ERROR;
}

status_t AudioMTKGainController::getNonSceneGainTableParameter(GainTableForNonScene *_gainTableForNonScene) {
    ALOGV("%s()", __FUNCTION__);
    memcpy((void *)_gainTableForNonScene, (void *)&mGainTable.nonSceneGain, sizeof(GainTableForNonScene));
    return NO_ERROR;
}

int AudioMTKGainController::getSceneCount() {
    return (int)(mSceneList.size());
}

int AudioMTKGainController::getSceneIndex(const char *scene) {
    std::string inputScene = scene;
    for (int i = 0; i < getSceneCount(); i++) {
        if (mSceneList[i] == inputScene) {
            ALOGD("%s(), %s index found = %d", __FUNCTION__, scene, i);
            return i;
        }
    }
    ALOGD("%s(), %s index nout found, use GAIN_SCENE_INDEX_DEFAULT", __FUNCTION__, scene);
    return GAIN_SCENE_INDEX_DEFAULT;
}

int AudioMTKGainController::getCurrentSceneIndex() {
    ALOGD("%s(), mSceneIndex = %d", __FUNCTION__, mSceneIndex);
    return mSceneIndex;
}

void AudioMTKGainController::setScene(const char *scene) {
    ALOGD("%s(), scene = %s", __FUNCTION__, scene);
    mSceneIndex = getSceneIndex(scene);
}

status_t AudioMTKGainController::initCheck() {
    return mInitDone;
}

void AudioMTKGainController::updateXmlParam(const char *_audioTypeName) {
    ALOGD("%s(), audioType = %s", __FUNCTION__, _audioTypeName);

    bool needResetDlGain = false;
    bool isMicGainChanged = false;

    if (strcmp(_audioTypeName, PLAY_DIGI_AUDIOTYPE_NAME) == 0) {
        GainTableParamParser::getInstance()->updatePlaybackDigitalGain(&mGainTable, &mSceneList);
    } else if (strcmp(_audioTypeName, PLAY_ANA_AUDIOTYPE_NAME) == 0) {
        GainTableParamParser::getInstance()->updatePlaybackAnalogGain(&mGainTable, &mSceneList);
        needResetDlGain = true;
    } else if (strcmp(_audioTypeName, SPEECH_VOL_AUDIOTYPE_NAME) == 0) {
        GainTableParamParser::getInstance()->updateSpeechVol(&mGainTable);
        isMicGainChanged = true;
        needResetDlGain = true;
    } else if (strcmp(_audioTypeName, REC_VOL_AUDIOTYPE_NAME) == 0) {
        GainTableParamParser::getInstance()->updateRecordVol(&mGainTable, &mSceneList);
        isMicGainChanged = true;
    } else if (strcmp(_audioTypeName, VOIP_VOL_AUDIOTYPE_NAME) == 0) {
        GainTableParamParser::getInstance()->updateVoIPVol(&mGainTable, &mSceneList);
        isMicGainChanged = true;
        needResetDlGain = true;
    } else if (strcmp(_audioTypeName, RINGBACK_VOL_AUDIOTYPE_NAME) == 0) {
        GainTableParamParser::getInstance()->updateRingbackVol(&mGainTable);
        // No need to set DL analog gain
    } else if (strcmp(_audioTypeName, VOLUME_AUDIOTYPE_NAME) == 0) {
        GainTableParamParser::getInstance()->loadGainTableSpec();
        GainTableParamParser::getInstance()->getGainTableParam(&mGainTable, &mSceneList);
        GainTableParamParser::getInstance()->getGainTableSpec(&mSpec);
        isMicGainChanged = true;
        needResetDlGain = true;
    } else if (strcmp(_audioTypeName, GAIN_MAP_AUDIOTYPE_NAME) == 0) {
        GainTableParamParser::getInstance()->loadGainTableMapDl();
        GainTableParamParser::getInstance()->getGainTableParam(&mGainTable, &mSceneList);
        isMicGainChanged = true;
        needResetDlGain = true;
    } else if (strcmp(_audioTypeName, GAIN_MAP_UL_AUDIOTYPE_NAME) == 0) {
        GainTableParamParser::getInstance()->loadGainTableMapUl();
        GainTableParamParser::getInstance()->getGainTableParam(&mGainTable, &mSceneList);
        isMicGainChanged = true;
    } else if (strcmp(_audioTypeName, HP_IMPEDANCE_AUDIOTYPE_NAME) == 0) {
        GainTableParamParser::getInstance()->loadGainTableHpImpedance();
        GainTableParamParser::getInstance()->getGainTableSpec(&mSpec);
        needResetDlGain = true;
    }

    // reset mic gain immediately
    if (isMicGainChanged) {
        if (!isInVoiceCall(mHwStream.mode)) {
            SetCaptureGain(mHwCaptureInfo.mode, mHwCaptureInfo.source, mHwCaptureInfo.input_device, mHwCaptureInfo.output_devices);
        }
    }

    if (needResetDlGain)
        setAnalogVolume_l(mHwStream.stream,
                          mHwStream.devices,
                          mHwStream.index,
                          mHwStream.mode);
}

status_t AudioMTKGainController::SetCaptureGain(audio_mode_t mode, audio_source_t source, audio_devices_t input_device, audio_devices_t output_devices) {
    ALOGD("+%s(), mode=%d, source=%d, input device=0x%x, output device=0x%x", __FUNCTION__, mode, source, input_device, output_devices);

    if (source == AUDIO_SOURCE_HOTWORD) {
        return NO_ERROR;
    }
    if (mode == AUDIO_MODE_CURRENT) {
        mode = mHwCaptureInfo.mode;
    }
    mHwCaptureInfo.mode = mode;
    mHwCaptureInfo.source = source;
    mHwCaptureInfo.input_device = input_device;
    mHwCaptureInfo.output_devices = output_devices;

    switch (mode) {
    case AUDIO_MODE_NORMAL:
    case AUDIO_MODE_RINGTONE: {
        switch (source) {
        case AUDIO_SOURCE_CAMCORDER:
        case AUDIO_SOURCE_VOICE_RECOGNITION:
        case AUDIO_SOURCE_VOICE_UNLOCK:
        case AUDIO_SOURCE_CUSTOMIZATION1:
        case AUDIO_SOURCE_CUSTOMIZATION2:
        case AUDIO_SOURCE_CUSTOMIZATION3:
        case AUDIO_SOURCE_UNPROCESSED:
        case AUDIO_SOURCE_AAUDIO:
            ApplyMicGain(getGainMicMode(source, mode),
                         getGainDevice(input_device),
                         mode);
            break;
        default:
            //for audio tuning tool tuning case.
            if (mAudioSpeechEnhanceInfoInstance->IsAPDMNRTuningEnable()) {  //for DMNR tuning
                if (mAudioSpeechEnhanceInfoInstance->GetAPTuningMode() == HANDSFREE_MODE_DMNR) {
                    ApplyMicGain(GAIN_MIC_VOICE_CALL, GAIN_DEVICE_SPEAKER, mode);
                } else if (mAudioSpeechEnhanceInfoInstance->GetAPTuningMode() == NORMAL_MODE_DMNR) {
                    ApplyMicGain(GAIN_MIC_VOICE_CALL, GAIN_DEVICE_EARPIECE, mode);
                } else {
                    ApplyMicGain(GAIN_MIC_NORMAL, GAIN_DEVICE_EARPIECE, mode);
                }
            } else {
                ApplyMicGain(GAIN_MIC_NORMAL, getGainDevice(input_device), mode);
            }
            break;
        }
        break;
    }
    case AUDIO_MODE_IN_CALL: {
        if (AudioALSASpeechPhoneCallController::getInstance()->checkTtyNeedOn() == false) {
            if (!audio_is_bluetooth_sco_device(output_devices)) {
                ApplyMicGain(getGainMicMode(source, mode),
                             getGainDevice(output_devices),
                             mode);
            }
        }
        break;
    }
    case AUDIO_MODE_IN_COMMUNICATION: {
        ApplyMicGain(getGainMicMode(source, mode),
                     getGainDevice(output_devices),
                     mode);
        break;
    }
    default: {
        break;
    }
    }
    return NO_ERROR;
}


status_t AudioMTKGainController::speechNetworkChange(unsigned int info) {
    bool isNetworkSupport = (info & (0x1 << 15)) != 0;
    GAIN_SPEECH_BAND band;
    char *netName = NULL;
    GAIN_SPEECH_NETWORK net;

    if (isNetworkSupport) {
        band = (GAIN_SPEECH_BAND)((info >> 4) & 0x3); //info[4:5]
        netName = SpeechConfig::getInstance()->getNameForEachSpeechNetwork(info & 0xf);
        net = GainTableParamParser::getInstance()->getGainSpeechNetwork(netName);
    } else {
        band = (GAIN_SPEECH_BAND)((info >> 3) & 0x7); //info[3:5]
        net = (GAIN_SPEECH_NETWORK)0;
    }
    ALOGD("%s(), info 0x%x, band %d, net %d, netName %s", __FUNCTION__, info, band, net, netName);

    AutoMutex lock(mLock);
    if (mBand != band || mNetwork != net) {
        mBand = band;
        mNetwork = net;
        if (isInVoiceCall(mHwStream.mode)) {
            setAnalogVolume_l(mHwStream.stream, mHwStream.devices, mHwStream.index, AUDIO_MODE_IN_CALL);
        }
    }
    return NO_ERROR;
}

bool AudioMTKGainController::isNbSpeechBand(void) {
    AutoMutex lock(mLock);
    return mBand == GAIN_SPEECH_NB;
}

status_t AudioMTKGainController::setBtVolumeCapability(bool support) {
    AutoMutex lock(mLock);
    mSupportBtVol = !support; // if bt device do not support volume , we should
    return NO_ERROR;
}

status_t AudioMTKGainController::setAnalogVolume_l(int stream, int devices, int index, audio_mode_t mode) {
    ALOGG("setAnalogVolume(), stream %d, devices 0x%x, index %d, mode %d", stream, devices, index, mode);
    mHwStream.stream = stream;
    mHwStream.devices = devices;
    mHwStream.index  = index;
    mHwStream.mode = mode;

    if (isInVoiceCall(mode)) {
        setVoiceVolume(index, devices, mode);
        AudioALSASpeechPhoneCallController::getInstance()->updateVolume();
    } else {
        setNormalVolume(stream, index, devices, mode);
    }

    return NO_ERROR;
}

status_t AudioMTKGainController::setAnalogVolume(int stream, int devices, int index, audio_mode_t mode) {
    AutoMutex lock(mLock);
    return setAnalogVolume_l(stream, devices, index, mode);
}

status_t AudioMTKGainController::setNormalVolume(int stream, int index, int devices, audio_mode_t mode) {
    // Avoid pop noise when switch between headset and spk+headset mode
    if (((audio_devices_t)devices != mHardwareResourceManager->getOutputDevice()) &&
        (mHardwareResourceManager->getOutputDevice() != 0x0)) {
        ALOGW("%s(), devices(0x%x) != mOutputDevices(0x%x), skip set device gain",
              __FUNCTION__, devices, mHardwareResourceManager->getOutputDevice());
        return NO_ERROR;
    }

    if (audio_is_bluetooth_sco_device(devices)) {
        ALOGD("%s(), devices(0x%x) is BTSCO, skip set device gain", __FUNCTION__, devices);
        return NO_ERROR;
    }

    ALOGD("%s(), mSceneIndex = %d, stream %d, devices 0x%x, index %d, mode 0x%x",
          __FUNCTION__, mSceneIndex, stream, devices, index, mode);

    // get gain device
    GAIN_DEVICE gainDevice = getGainDevice(devices);

    // check stream/index range
    if (!isValidStreamType((audio_stream_type_t)stream)) {
        ALOGW("error, stream %d is invalid, use %d instead", stream, AUDIO_STREAM_MUSIC);
        stream = AUDIO_STREAM_MUSIC;
    }
    if (!isValidVolIdx(index, mode)) {
        ALOGW("error, index %d is invalid, use max %d instead", index, GAIN_MAX_VOL_INDEX);
        index = GAIN_MAX_VOL_INDEX;
    }

#ifdef MTK_HAC_SUPPORT
    // change gain device if HAC On
    if (gainDevice == GAIN_DEVICE_EARPIECE &&
        SpeechEnhancementController::GetInstance()->GetHACOn()) {
        ALOGD("%s(): HAC ON gain device change from EarPiece to HAC", __FUNCTION__);
        gainDevice = GAIN_DEVICE_HAC;
    }
#endif

    if (isSpeakerCategory(gainDevice)) {
        if (mSpec->spkAnaType >= 0 && mSpec->spkAnaType < NUM_GAIN_ANA_TYPE) {
            unsigned char gain = mGainTable.sceneGain[mSceneIndex].streamGain[stream][gainDevice][index].analog[mSpec->spkAnaType];
            setSpeakerGain(gain);
        }
    }

    if (isHeadsetCategory(gainDevice)) {
        unsigned char gain = mGainTable.sceneGain[mSceneIndex].streamGain[stream][gainDevice][index].analog[GAIN_ANA_HEADPHONE];
        ApplyAudioGain(gain, mode, gainDevice);
        //        setAMPGain(streamGain->stream[gainDevice].amp, AMP_CONTROL_POINT, devices);
    }

    if (isEarpieceCategory(gainDevice)) {
        if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER) ||
            IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER)) {
            if (mSpec->spkAnaType >= 0 && mSpec->spkAnaType < NUM_GAIN_ANA_TYPE) {
                unsigned char gain = mGainTable.sceneGain[mSceneIndex].streamGain[stream][gainDevice][index].analog[mSpec->spkAnaType];
                setSpeakerGain(gain);
            }
        } else {
            unsigned char gain = mGainTable.sceneGain[mSceneIndex].streamGain[stream][gainDevice][index].analog[GAIN_ANA_HANDSET];
            ApplyAudioGain(gain, mode, gainDevice);
        }
    }

    return NO_ERROR;
}

status_t AudioMTKGainController::setVoiceVolume(int index, int devices, audio_mode_t mode) {
    ALOGD("%s(), index = %d, devices = 0x%x, mode = %d, mBand = %d, mNetwork = %d, mVoiceVolume = %f",
          __FUNCTION__, index, devices, mode, mBand, mNetwork, mVoiceVolume);


    if (audio_is_bluetooth_sco_device(devices)) {
        // check stream/index range
        if (!isValidVolIdx(index, AUDIO_MODE_NORMAL)) {
            ALOGW("error, index %d is invalid, use max %d instead", index, GAIN_MAX_VOL_INDEX);
            index = GAIN_MAX_VOL_INDEX;
        }

        ALOGD("audio_is_bluetooth_sco_device = %d, mSupportBtVol is %d", true, mSupportBtVol);
        uint8_t digitalDegradeDb = 0;

        // help tune gain if bt device doesn't support volume ctrl, otherwise pass 0dB
        if (mSupportBtVol) {
            digitalDegradeDb = mGainTable.sceneGain[mSceneIndex].streamGain[AUDIO_STREAM_BLUETOOTH_SCO][GAIN_DEVICE_HEADSET][index].digital;
        }

        ApplyMdDlGain(index == 0 ? 255 : digitalDegradeDb);  // modem dl gain
        ApplyMdUlGain(0);

        return NO_ERROR;
    } else {
        // check stream/index range
        if (!isValidVolIdx(index, mode)) {
            ALOGW("error, index %d is invalid, use max %d instead", index, GAIN_MAX_SPEECH_VOL_INDEX);
            index = GAIN_MAX_SPEECH_VOL_INDEX;
        }

        // get gain device
        GAIN_DEVICE gainDevice = GAIN_DEVICE_NONE;
        if (AudioALSASpeechPhoneCallController::getInstance()->checkTtyNeedOn() == true) {
            gainDevice = getGainDeviceForTty();
        } else {
            gainDevice = getGainDevice(devices);
        }

        if (gainDevice == GAIN_DEVICE_HSSPK) {
            ALOGW("%s(), phone call mode can't choose HSSPK", __FUNCTION__);
            return BAD_VALUE;
        }

#ifdef MTK_HAC_SUPPORT
        // change gain device if HAC On
        if (gainDevice == GAIN_DEVICE_EARPIECE) {
            bool mHACon = SpeechEnhancementController::GetInstance()->GetHACOn();
            if (mHACon) {
                ALOGD("%s(): HAC ON = %d, gain device change from EarPiece to HAC", __FUNCTION__, mHACon);
                gainDevice = GAIN_DEVICE_HAC;
            }
        }
#endif

        // set analog gain
        if (isSpeakerCategory(gainDevice)) {
            if (mSpec->spkAnaType >= 0 && mSpec->spkAnaType < NUM_GAIN_ANA_TYPE) {
                unsigned char gain = mGainTable.nonSceneGain.speechGain[mBand][mNetwork][gainDevice][index].analog[mSpec->spkAnaType];
                setSpeakerGain(gain);
            }
        }

        if (isHeadsetCategory(gainDevice)) {
            unsigned char gain = mGainTable.nonSceneGain.speechGain[mBand][mNetwork][gainDevice][index].analog[GAIN_ANA_HEADPHONE];
            ApplyAudioGain(gain, mode, gainDevice);
        }

        if (isEarpieceCategory(gainDevice)) {
            if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER) ||
                IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER)) {
                if (mSpec->spkAnaType >= 0 && mSpec->spkAnaType < NUM_GAIN_ANA_TYPE) {
                    unsigned char gain = mGainTable.nonSceneGain.speechGain[mBand][mNetwork][gainDevice][index].analog[mSpec->spkAnaType];
                    setSpeakerGain(gain);
                }
            } else {
                unsigned char gain = mGainTable.nonSceneGain.speechGain[mBand][mNetwork][gainDevice][index].analog[GAIN_ANA_HANDSET];
                ApplyAudioGain(gain, mode, gainDevice);
            }
        }

        // set digital gain
        //setAMPGain(ampgain, AMP_CONTROL_POINT, device);
        uint8_t digitalDegradeDb = mGainTable.nonSceneGain.speechGain[mBand][mNetwork][gainDevice][index].digital;
        ApplyMdDlGain(digitalDegradeDb);  // modem dl gain

        // mic gain & modem UL gain
        if (index == 0) { // don't set ul/stf gain when force mute dl
            return 0;
        }

        if (AudioALSASpeechPhoneCallController::getInstance()->checkTtyNeedOn() == true) {
            ApplyMicGainForTty(mode);
        } else {
            ApplyMicGainByDevice(devices, mode);
        }

        ApplySideTone(gainDevice);
    }

    return NO_ERROR;
}

void  AudioMTKGainController::ApplyMicGainByDevice(uint32_t device, audio_mode_t mode) {
    if (device & AUDIO_DEVICE_OUT_EARPIECE ||
        device & AUDIO_DEVICE_OUT_WIRED_HEADSET ||
        device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
        device & AUDIO_DEVICE_OUT_SPEAKER ||
        device & AUDIO_DEVICE_OUT_BUS ||
        device & AUDIO_DEVICE_OUT_USB_DEVICE ||
        device & AUDIO_DEVICE_OUT_USB_HEADSET) {

        GAIN_MIC_MODE micMode = getGainMicMode(AUDIO_SOURCE_DEFAULT, mode);
        GAIN_DEVICE gainDevice = getGainDevice(device);

#ifdef MTK_USB_PHONECALL
        if (gainDevice == GAIN_DEVICE_USB &&
            AudioUSBPhoneCallController::getInstance()->isUsingUSBIn() == false) {
            // usb call using usb output only, input gain use HP gain
            gainDevice = GAIN_DEVICE_HEADPHONE;
        }
#endif

        ApplyMicGain(micMode, gainDevice, mode);
    } else if (audio_is_bluetooth_sco_device(device)) {
        //when use BT_SCO , apply digital to 0db.
        ApplyMdUlGain(0);
    }
}

void  AudioMTKGainController::ApplyMicGainForTty(audio_mode_t mode) {

    audio_devices_t outDevice = AudioALSASpeechPhoneCallController::getInstance()->getOutputDevice();
    TtyModeType ttyMode = AudioALSASpeechPhoneCallController::getInstance()->getTtyMode();
    GAIN_MIC_MODE micMode = getGainMicMode(AUDIO_SOURCE_DEFAULT, mode);

    if (outDevice == 0) {
        ALOGE("%s(), outDevice=0!!! do nothing", __FUNCTION__);
    } else if (outDevice & AUDIO_DEVICE_OUT_SPEAKER) {
        if (ttyMode == AUD_TTY_VCO) {
            ALOGD("%s(), speaker, TTY_VCO", __FUNCTION__);
            ApplyMicGain(micMode, GAIN_DEVICE_SPEAKER, mode);
        } else if (ttyMode == AUD_TTY_HCO) {
            ALOGD("%s(), speaker, TTY_HCO", __FUNCTION__);
            ApplyMicGain(micMode, GAIN_DEVICE_TTY, mode);
        } else if (ttyMode == AUD_TTY_FULL) {
            ALOGD("%s(), speaker, TTY_FULL", __FUNCTION__);
            ApplyMicGain(micMode, GAIN_DEVICE_TTY, mode);
        }
    } else if ((outDevice == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
               (outDevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
        if (ttyMode == AUD_TTY_VCO) {
            ALOGD("%s(), headset, TTY_VCO", __FUNCTION__);
            ApplyMicGain(micMode, GAIN_DEVICE_EARPIECE, mode);
        } else if (ttyMode == AUD_TTY_HCO) {
            ALOGD("%s(), headset, TTY_HCO", __FUNCTION__);
            ApplyMicGain(micMode, GAIN_DEVICE_TTY, mode);
        } else if (ttyMode == AUD_TTY_FULL) {
            ApplyMicGain(micMode, GAIN_DEVICE_TTY, mode);
        }
    } else if (outDevice == AUDIO_DEVICE_OUT_EARPIECE) {
        // tty device is removed. TtyCtm already off in CloseMD.
        ALOGD("%s(), receiver", __FUNCTION__);
    } else {
        ALOGD("%s(), routing = 0x%x", __FUNCTION__, outDevice);
    }
}

GAIN_DEVICE AudioMTKGainController::getGainDevice(audio_devices_t devices) {
    GAIN_DEVICE gainDevice = GAIN_DEVICE_NONE;
    bool IsSuperVolumeMode = AudioALSASpeechPhoneCallController::getInstance()->getSpeechFeatureStatus(SPEECH_FEATURE_SUPERVOLUME);

    if (devices & AUDIO_DEVICE_BIT_IN) {
        /* input device */
        if (devices == AUDIO_DEVICE_IN_WIRED_HEADSET) {
            if (mHardwareResourceManager->getNumOfHeadsetPole() == 5) {
                if (mANCEnable) {
                    gainDevice = GAIN_DEVICE_HEADSET_5POLE_ANC;
                } else {
                    gainDevice = GAIN_DEVICE_HEADSET_5POLE;
                }
            } else {
                gainDevice = GAIN_DEVICE_HEADSET;
            }
        } else if (devices == AUDIO_DEVICE_IN_BUILTIN_MIC) {
            gainDevice = GAIN_DEVICE_SPEAKER;
        } else if (devices == AUDIO_DEVICE_IN_USB_DEVICE || devices == AUDIO_DEVICE_IN_USB_HEADSET || devices == AUDIO_DEVICE_IN_BUS) {
            gainDevice = GAIN_DEVICE_USB;
        } else {
            ALOGE("%s(), error, devices (0x%x) not support, return GAIN_DEVICE_SPEAKER", __FUNCTION__, devices);
            gainDevice = GAIN_DEVICE_SPEAKER;
        }
    } else {
        /* output device */
        if (devices & AUDIO_DEVICE_OUT_SPEAKER) {
#ifdef MTK_AUDIO_SPH_LPBK_PARAM
            if (LoopbackManager::GetInstance()->CheckIsModemLoopback(LoopbackManager::GetInstance()->GetLoopbackType())) {
                gainDevice = GAIN_DEVICE_LPBK_SPK;
            } else
#endif
            {
                if (IsSuperVolumeMode == true) {
                    gainDevice = GAIN_DEVICE_SPK_EV;
                } else {
                    gainDevice = GAIN_DEVICE_SPEAKER;
                }
            }

            if ((devices & AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
                (devices & AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
                gainDevice = GAIN_DEVICE_HSSPK;
            }
        } else if (devices & AUDIO_DEVICE_OUT_WIRED_HEADSET) {
#ifdef MTK_AUDIO_SPH_LPBK_PARAM
            if (LoopbackManager::GetInstance()->CheckIsModemLoopback(LoopbackManager::GetInstance()->GetLoopbackType())) {
                gainDevice = GAIN_DEVICE_LPBK_HP;
            } else
#endif
            {
                if (mHardwareResourceManager->getNumOfHeadsetPole() == 5) {
                    if (mANCEnable) {
                        gainDevice = GAIN_DEVICE_HEADSET_5POLE_ANC;
                    } else {
                        gainDevice = GAIN_DEVICE_HEADSET_5POLE;
                    }
                } else {
                    gainDevice = GAIN_DEVICE_HEADSET;
                }
            }
        } else if (devices & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
#ifdef MTK_AUDIO_SPH_LPBK_PARAM
            if (LoopbackManager::GetInstance()->CheckIsModemLoopback(LoopbackManager::GetInstance()->GetLoopbackType())) {
                gainDevice = GAIN_DEVICE_LPBK_HP;
            } else
#endif
                gainDevice = GAIN_DEVICE_HEADPHONE;
        } else if (devices & AUDIO_DEVICE_OUT_EARPIECE) {
#ifdef MTK_AUDIO_SPH_LPBK_PARAM
            if (LoopbackManager::GetInstance()->CheckIsModemLoopback(LoopbackManager::GetInstance()->GetLoopbackType())) {
                gainDevice = GAIN_DEVICE_LPBK_RCV;
            } else
#endif
            {
                if (IsSuperVolumeMode == true) {
                    gainDevice = GAIN_DEVICE_RCV_EV;
                } else {
                    gainDevice = GAIN_DEVICE_EARPIECE;
                }
            }
        } else if (devices & AUDIO_DEVICE_OUT_USB_DEVICE || devices & AUDIO_DEVICE_OUT_USB_HEADSET || devices & AUDIO_DEVICE_OUT_BUS) {
            gainDevice = GAIN_DEVICE_USB;
        } else {
            ALOGE("%s(), error, devices (%d) not support, return GAIN_DEVICE_SPEAKER", __FUNCTION__, devices);
            gainDevice = GAIN_DEVICE_SPEAKER;
        }
    }

    ALOGG("%s(), input devices = 0x%x, return gainDevice = %d", __FUNCTION__, devices, gainDevice);
    return gainDevice;
}

GAIN_DEVICE AudioMTKGainController::getGainDeviceForTty(void) {
    GAIN_DEVICE gainDeviceForTty = GAIN_DEVICE_NONE;
    audio_devices_t outDevice = AudioALSASpeechPhoneCallController::getInstance()->getOutputDevice();
    TtyModeType ttyMode = AudioALSASpeechPhoneCallController::getInstance()->getTtyMode();

    if (outDevice & AUDIO_DEVICE_OUT_SPEAKER) {
        if (ttyMode == AUD_TTY_VCO) {
            ALOGD("%s(), speaker, TTY_VCO", __FUNCTION__);
            gainDeviceForTty = GAIN_DEVICE_HEADSET;
        } else if (ttyMode == AUD_TTY_HCO) {
            ALOGD("%s(), speaker, TTY_HCO", __FUNCTION__);
            gainDeviceForTty = GAIN_DEVICE_SPEAKER;
        } else if (ttyMode == AUD_TTY_FULL) {
            ALOGD("%s(), speaker, TTY_FULL", __FUNCTION__);
            gainDeviceForTty = GAIN_DEVICE_HEADSET;
        }
    } else if ((outDevice == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
               (outDevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
        if (ttyMode == AUD_TTY_VCO) {
            ALOGD("%s(), headset, TTY_VCO", __FUNCTION__);
            gainDeviceForTty = GAIN_DEVICE_HEADSET;
        } else if (ttyMode == AUD_TTY_HCO) {
            ALOGD("%s(), headset, TTY_HCO", __FUNCTION__);
            gainDeviceForTty = GAIN_DEVICE_EARPIECE;
        } else if (ttyMode == AUD_TTY_FULL) {
            ALOGD("%s(), headset, TTY_FULL", __FUNCTION__);
            gainDeviceForTty = GAIN_DEVICE_HEADSET;
        }
    }

    if (gainDeviceForTty < 0 || gainDeviceForTty >= NUM_GAIN_DEVICE) {
        ALOGE("%s(), error, use GAIN_DEVICE_EARPIECE", __FUNCTION__);
        gainDeviceForTty = GAIN_DEVICE_EARPIECE;
    }

    ALOGD("%s(), outDevice = 0x%x, ttyMode = %d return gainDeviceForTty = %d",
          __FUNCTION__, outDevice, ttyMode, gainDeviceForTty);
    return gainDeviceForTty;
}

GAIN_MIC_MODE AudioMTKGainController::getGainMicMode(audio_source_t _source, audio_mode_t _mode) {
    GAIN_MIC_MODE micMode = GAIN_MIC_NORMAL;

    switch (_mode) {
    case AUDIO_MODE_NORMAL:
    case AUDIO_MODE_RINGTONE:
        switch (_source) {
        case AUDIO_SOURCE_MIC:
            micMode = GAIN_MIC_NORMAL;
            break;
        case AUDIO_SOURCE_VOICE_UPLINK:
        case AUDIO_SOURCE_VOICE_DOWNLINK:
        case AUDIO_SOURCE_VOICE_CALL:
            micMode = GAIN_MIC_VOICE_CALL;
            break;
        case AUDIO_SOURCE_CAMCORDER:
            micMode = GAIN_MIC_CAMCORDER;
            break;
        case AUDIO_SOURCE_VOICE_RECOGNITION:
            micMode = GAIN_MIC_VOICE_RECOGNITION;
            break;
        case AUDIO_SOURCE_VOICE_COMMUNICATION:
            micMode = GAIN_MIC_VOICE_COMMUNICATION;
            break;
        case AUDIO_SOURCE_VOICE_UNLOCK:
            micMode = GAIN_MIC_VOICE_UNLOCK;
            break;
        case AUDIO_SOURCE_CUSTOMIZATION1:
            micMode = GAIN_MIC_CUSTOMIZATION1;
            break;
        case AUDIO_SOURCE_CUSTOMIZATION2:
            micMode = GAIN_MIC_CUSTOMIZATION2;
            break;
        case AUDIO_SOURCE_CUSTOMIZATION3:
            micMode = GAIN_MIC_CUSTOMIZATION3;
            break;
        case AUDIO_SOURCE_UNPROCESSED:
            micMode = GAIN_MIC_UNPROCESSED;
            break;
        case AUDIO_SOURCE_AAUDIO:
            micMode = GAIN_MIC_AAUDIO;
            break;
        default:
            micMode = GAIN_MIC_NORMAL;
            break;
        }
        break;
    case AUDIO_MODE_IN_CALL:
        micMode = GAIN_MIC_VOICE_CALL;
        break;
    case AUDIO_MODE_IN_COMMUNICATION:
        micMode = GAIN_MIC_VOICE_COMMUNICATION;
        break;
    default:
        ALOGE("%s(), not handled mode %d", __FUNCTION__, _mode);
        micMode = GAIN_MIC_NORMAL;
        break;
    }
    return micMode;
}

void AudioMTKGainController::ApplyMdDlGain(int32_t degradeDb) {
    // set degarde db to mode side, DL part, here use degrade dbg
    ALOGG("ApplyMdDlGain degradeDb = %d", degradeDb);
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
    ALOGG("ApplyMdUlGain degradeDb = %d", IncreaseDb);

    //if (mHwVolume.swAgc != IncreaseDb)
    {
        mHwVolume.swAgc = IncreaseDb;
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetUplinkGain(IncreaseDb << 2); // degrade db * 4
    }
}

void AudioMTKGainController::ApplyAudioGain(int gain, audio_mode_t mode, GAIN_DEVICE gainDevice) {
    ALOGG("ApplyAudioGain(), gain = %d, mode = %d, gainDevice = %d", gain, mode, gainDevice);
    int bufferGain = gain;

    unsigned int bufferGainStringSize;
    if (isHeadsetCategory(gainDevice)) {
        bufferGainStringSize = mSpec->audioBufferGainString.size();
    } else { // if (isEarpieceCategory(gainDevice))
        bufferGainStringSize = mSpec->voiceBufferGainString.size();
    }

    if (bufferGain >= (int)bufferGainStringSize) {
        bufferGain = bufferGainStringSize - 1;
    }

    // adjust gain according to master volume
    bufferGain = tuneGainForMasterVolume(bufferGain, mode, gainDevice);

    // adjust gain according to hp impedance when using HP
    bufferGain = tuneGainForHpImpedance(bufferGain, gainDevice);

    if (isEarpieceCategory(gainDevice)) {
        setVoiceBufferGain(bufferGain);
    } else if (isHeadsetCategory(gainDevice)) {
        setAudioBufferGain(bufferGain);
    }
}

int AudioMTKGainController::GetReceiverGain(void) {
    struct mixer_ctl *ctl;
    unsigned int num_values, i ;
    int index = 0;

    ctl = mixer_get_ctl_by_name(mMixer, mSpec->voiceBufMixerName.c_str());
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        index = mixer_ctl_get_value(ctl, i);
        ALOGD("GetReceiverGain i = %d index = %d ", i, index);
    }
    return index;
}

int AudioMTKGainController::GetHeadphoneRGain(void) {
#if defined(MTK_HYBRID_NLE_SUPPORT)
    size_t nle_gain_idx;
    if (AudioALSAHyBridNLEManager::getInstance()->getNleHwConfigByIndex(&nle_gain_idx) != NO_ERROR) {
        ALOGE("%s getNleHwConfigByIndex Error", __FUNCTION__);
        nle_gain_idx = 0;
    }
    return ((int)nle_gain_idx);
#else
    struct mixer_ctl *ctl;
    unsigned int num_values, i ;
    int index = 0;

    ctl = mixer_get_ctl_by_name(mMixer, mSpec->audioBufRMixerName.c_str());
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        index = mixer_ctl_get_value(ctl, i);
        ALOGD("GetHeadphoneRGain i = %d index = %d ", i, index);
    }
    return index;
#endif
}


int AudioMTKGainController::GetHeadphoneLGain(void) {
#if defined(MTK_HYBRID_NLE_SUPPORT)
    size_t nle_gain_idx;
    if (AudioALSAHyBridNLEManager::getInstance()->getNleHwConfigByIndex(&nle_gain_idx) != NO_ERROR) {
        ALOGE("%s getNleHwConfigByIndex Error", __FUNCTION__);
        nle_gain_idx = 0;
    }
    return ((int)nle_gain_idx);
#else
    struct mixer_ctl *ctl;
    unsigned int num_values, i ;
    int index = 0;
    ALOGD("GetHeadphoneLGain");
    ctl = mixer_get_ctl_by_name(mMixer, mSpec->audioBufLMixerName.c_str());
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        index = mixer_ctl_get_value(ctl, i);
        ALOGD("GetHeadphoneLGain i = %d index = %d ", i, index);
    }
    return index;
#endif
}

void AudioMTKGainController::SetReceiverGain(int index) {
    ALOGG("SetReceiverGain = %d", index);
    struct mixer_ctl *ctl;

    if (index < 0) {
        index = 0;
    }

    if ((uint32_t)index >= mSpec->voiceBufferGainString.size()) {
        index = mSpec->voiceBufferGainString.size() - 1;
    }

    ctl = mixer_get_ctl_by_name(mMixer, mSpec->voiceBufMixerName.c_str());

    if (ctl) {
        if (mixer_ctl_set_enum_by_string(ctl, mSpec->voiceBufferGainString[index].c_str())) {
            ALOGE("Error: %s invalid value", mSpec->voiceBufMixerName.c_str());
        }
    } else {
        ALOGE("Error: get mixer ctl fail, %s", mSpec->voiceBufMixerName.c_str());
    }
}

void AudioMTKGainController::SetHeadPhoneLGain(int index) {
    ALOGG("SetHeadPhoneLGain = %d", index);
    struct mixer_ctl *ctl;

    if (index < 0) {
        index = 0;
    }

    if ((uint32_t)index >= mSpec->audioBufferGainString.size()) {
        index = mSpec->audioBufferGainString.size() - 1;
    }

    ctl = mixer_get_ctl_by_name(mMixer, mSpec->audioBufLMixerName.c_str());

    if (ctl) {
        if (mixer_ctl_set_enum_by_string(ctl, mSpec->audioBufferGainString[index].c_str())) {
            ALOGE("Error: %s invalid value", mSpec->audioBufLMixerName.c_str());
        }
    } else {
        ALOGE("Error: get mixer ctl fail, %s", mSpec->audioBufLMixerName.c_str());
    }
}

void AudioMTKGainController::SetHeadPhoneRGain(int index) {
    ALOGG("SetHeadPhoneRGain = %d", index);
    struct mixer_ctl *ctl;

    if (index < 0) {
        index = 0;
    }

    if ((uint32_t)index >= mSpec->audioBufferGainString.size()) {
        index = mSpec->audioBufferGainString.size() - 1;
    }

    ctl = mixer_get_ctl_by_name(mMixer, mSpec->audioBufRMixerName.c_str());

    if (ctl) {
        if (mixer_ctl_set_enum_by_string(ctl, mSpec->audioBufferGainString[index].c_str())) {
            ALOGE("Error: %s invalid value", mSpec->audioBufRMixerName.c_str());
        }
    } else {
        ALOGE("Error: get mixer ctl fail, %s", mSpec->audioBufRMixerName.c_str());
    }
}

void AudioMTKGainController::setAudioBufferGain(int gain) {
#ifdef MTK_AUDIO_SW_DRE
    if (mSWDREMute) {
        ALOGD("%s(), bypass when SWDRE mute", __FUNCTION__);
        return;
    }
#endif

    if (gain >= (int)mSpec->audioBufferGainString.size()) {
        gain = mSpec->audioBufferGainString.size() - 1;
    }

    ALOGG("setAudioBufferGain, gain %d, mHwVolume.audioBuffer %d", gain, mHwVolume.audioBuffer);

    mHwVolume.audioBuffer = gain;
#if defined(MTK_HYBRID_NLE_SUPPORT)
    if (AudioALSAHyBridNLEManager::getInstance()->setNleHwConfigByIndex(gain) == FAILED_TRANSACTION) { // the parameter using Index of HP Gain Reg
        SetHeadPhoneLGain(gain);
        SetHeadPhoneRGain(gain);
    }
#else
    SetHeadPhoneLGain(gain);
    SetHeadPhoneRGain(gain);
#endif
}

void  AudioMTKGainController::setVoiceBufferGain(int gain) {
    if (gain >= (int)mSpec->voiceBufferGainString.size()) {
        gain = mSpec->voiceBufferGainString.size() - 1;
    }

    ALOGG("setVoiceBufferGain, gain %d, mHwVolume.voiceBuffer %d", gain, mHwVolume.voiceBuffer);

    mHwVolume.voiceBuffer = gain;
    SetReceiverGain(gain);
}

int AudioMTKGainController::GetSPKGain(void) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    int index = 0;
    ALOGD("GetSPKGain");
    ctl = mixer_get_ctl_by_name(mMixer, mSpec->spkLMixerName.c_str());
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);
    for (i = 0; i < num_values; i++) {
        index = mixer_ctl_get_value(ctl, i);
        ALOGD("GetSPKGain i = %d index = %d ", i, index);
    }

    return index;
}

void AudioMTKGainController::SetSpeakerGain(int index) {
    ALOGG("SetSpeakerGain,index=%d \n", index);

    std::vector<std::string> *enumString;
    switch (mSpec->spkAnaType) {
    case GAIN_ANA_HEADPHONE:
        enumString = &mSpec->audioBufferGainString;

        if (index < 0) {
            index = 0;
        }

        break;
    case GAIN_ANA_LINEOUT:
        enumString = &mSpec->lineoutBufferGainString;

        if (index < 0) {
            index = 0;
        }

        break;
    case GAIN_ANA_SPEAKER:
    default:
        enumString = &mSpec->spkGainString;

        if (index < 0) {
            index = 1;
        }

        break;
    }

    if ((uint32_t)index >= enumString->size()) {
        index = enumString->size() - 1;
    }

    struct mixer_ctl *ctl;
    // left chanel
    ctl = mixer_get_ctl_by_name(mMixer, mSpec->spkLMixerName.c_str());
    if (!ctl) {
        ALOGE("Error: get mixer ctl fail, %s", mSpec->spkLMixerName.c_str());
    } else {
        if (mixer_ctl_set_enum_by_string(ctl, enumString->at(index).c_str())) {
            ALOGE("Error: set %s with %s failed", mSpec->spkLMixerName.c_str(), enumString->at(index).c_str());
        }
    }

    // right channel
    ctl = mixer_get_ctl_by_name(mMixer, mSpec->spkRMixerName.c_str());
    if (!ctl) {
        ALOGE("Error: get mixer ctl fail, %s", mSpec->spkRMixerName.c_str());
    } else {
        if (mixer_ctl_set_enum_by_string(ctl, enumString->at(index).c_str())) {
            ALOGE("Error: set %s with %s failed", mSpec->spkRMixerName.c_str(), enumString->at(index).c_str());
        }
    }

}

void   AudioMTKGainController::setSpeakerGain(int gain) {
    ALOGD("%s(), gain = %d, spkAnaType = %d, spkLMixerName = %s, spkRMixerName = %s",
          __FUNCTION__,
          gain,
          mSpec->spkAnaType,
          mSpec->spkLMixerName.c_str(),
          mSpec->spkRMixerName.c_str());

    // adjust gain according to master volume
    if (mSpec->spkAnaType == GAIN_ANA_SPEAKER) {
        if (!isInVoiceCall(mHwStream.mode)) { // mMasterVolume don't affect voice call
            int degradedB = (keyvolumeStep - logToLinear(mMasterVolume)) * keydBPerStep;
            ALOGD("%s(), degraded gain of mMasterVolume = %d dB", __FUNCTION__, degradedB);
            gain -= degradedB;
            if (gain <= 0) { //avoid mute
                gain = 1;
            }
        }
    } else {
        gain = tuneGainForMasterVolume(gain, mHwStream.mode, GAIN_DEVICE_SPEAKER);
    }

    ALOGG("%s(), gain %d, mHwVolume.speaker %d", __FUNCTION__, gain, mHwVolume.speaker);

    mHwVolume.speaker = gain;
    SetSpeakerGain(gain);
}

void   AudioMTKGainController::setAMPGain(void *points __unused, int num __unused, int device __unused) {
    ALOGD("setAMPGain, device %d", device);
#ifdef USING_EXTAMP_TC1
    //    if (mAmpControl && points)
    //    {
    //        mAmpControl->setVolume(points, num, device);
    //    }
#endif
}

void AudioMTKGainController::SetAdcPga1(int gain) {
    struct mixer_ctl *ctl;
    int num_values = 0;

    ctl = mixer_get_ctl_by_name(mMixer, mSpec->ulPgaLMixerName.c_str());
    num_values = mixer_ctl_get_num_values(ctl);

    if (ctl) {
        if (mixer_ctl_set_enum_by_string(ctl, mSpec->ulPgaGainString[gain].c_str())) {
            ALOGE("Error: %s invalid value", mSpec->ulPgaLMixerName.c_str());
        }
    } else {
        ALOGE("Error: get mixer ctl fail, %s", mSpec->ulPgaLMixerName.c_str());
    }
}
void AudioMTKGainController::SetAdcPga2(int gain) {
    struct mixer_ctl *ctl;
    int num_values = 0;

    ctl = mixer_get_ctl_by_name(mMixer, mSpec->ulPgaRMixerName.c_str());
    num_values = mixer_ctl_get_num_values(ctl);

    if (ctl) {
        if (mixer_ctl_set_enum_by_string(ctl, mSpec->ulPgaGainString[gain].c_str())) {
            ALOGE("Error: %s invalid value", mSpec->ulPgaRMixerName.c_str());
        }
    } else {
        ALOGE("Error: get mixer ctl fail, %s", mSpec->ulPgaRMixerName.c_str());
    }
}

status_t AudioMTKGainController::ApplyMicGain(uint32_t MicType, int mode) {
    /* deprecated !!! DO NOT USE !!! */
    /* The following will be removed */
    GAIN_MIC_MODE micMode = GAIN_MIC_NORMAL;
    GAIN_DEVICE gainDevice = GAIN_DEVICE_EARPIECE;

    if (MicType == Normal_Mic) {
        micMode = GAIN_MIC_VOICE_CALL;
        gainDevice = GAIN_DEVICE_EARPIECE;
    } else if (MicType == Handfree_Mic) {
        micMode = GAIN_MIC_VOICE_CALL;
        gainDevice = GAIN_DEVICE_SPEAKER;
    } else if (MicType == Headset_Mic) {
        micMode = GAIN_MIC_VOICE_CALL;
        gainDevice = getGainDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
    } else if (MicType == TTY_CTM_Mic) {
        micMode = GAIN_MIC_VOICE_CALL;
        gainDevice = GAIN_DEVICE_TTY;
    } else {
        ALOGE("%s(), MicType not supported", __FUNCTION__);
        return BAD_VALUE;
    }

    return ApplyMicGain(micMode, gainDevice, (audio_mode_t)mode);
}

status_t AudioMTKGainController::ApplyMicGain(GAIN_MIC_MODE _micMode, GAIN_DEVICE _gainDevice, audio_mode_t _mode) {
    uint8_t analogidx;
    uint8_t gain = _micMode == GAIN_MIC_VOICE_CALL ?
                   mGainTable.nonSceneGain.speechMicGain[mBand][mNetwork][_gainDevice].gain :
                   mGainTable.sceneGain[mSceneIndex].micGain[_micMode][_gainDevice].gain;

    if (gain > mSpec->micIdxMax[_gainDevice]) {
        gain = mSpec->micIdxMax[_gainDevice];
    }
    if (gain < mSpec->micIdxMin[_gainDevice]) {
        gain = mSpec->micIdxMin[_gainDevice];
    }

    uint8_t degradedb = mSpec->micIdxMax[_gainDevice] - gain;
    short analogdegrade = mSpec->ulPgaGainMap[_gainDevice][degradedb];
    short swagcmap = isDmicDevice(_gainDevice) ?
                     mSpec->swagcGainMapDmic[_gainDevice][degradedb] : mSpec->swagcGainMap[_gainDevice][degradedb];

    mULTotalGain = UPLINK_GAIN_MAX - ((mSpec->micIdxMax[_gainDevice] - gain) * UPLINK_ONEDB_STEP);

    ASSERT(mSpec->ulHwPgaIdxMax != 0);

    if (mSpec->ulHwPgaIdxMax == 0) {
        ALOGE("%s(), ulHwPgaIdxMax == 0", __FUNCTION__);
        return BAD_VALUE;
    }

    if (_gainDevice != GAIN_DEVICE_USB && _gainDevice != GAIN_DEVICE_BT) {
        mHwVolume.micGain = gain;
        analogidx = (mSpec->ulPgaGainMapMax - analogdegrade) / mSpec->ulHwPgaIdxMax;
        ALOGD("%s(), analogidx = %d, mSceneIndex = %d, _mic_mode = %d, _gain_device = %d, mode = %d, micgain = %d, mULTotalGain = %d, mBand = %d, mNetwork = %d",
              __FUNCTION__, analogidx, mSceneIndex, _micMode, _gainDevice,
              _mode, gain, mULTotalGain, mBand, mNetwork);
        SetAdcPga1(analogidx);
        SetAdcPga2(analogidx);
    }

    mHwVolume.swAgc = swagcmap;
    if (isInVoiceCall(_mode)) {
        ApplyMdUlGain(swagcmap);
    }

    return NO_ERROR;
}

short AudioMTKGainController::GetSWMICGain() {
    return mHwVolume.swAgc ;
}

status_t AudioMTKGainController::ApplySideTone(uint32_t Mode) {
    if (Mode >= NUM_GAIN_DEVICE) {
        ALOGW("error, invalid gainDevice = %d, do nothing", Mode);
        return BAD_VALUE;
    }

    // here apply side tone gain, need base on UL and DL analog gainQuant
#if defined(MTK_HAC_SUPPORT)
    // change gain device if HAC On
    if (Mode == GAIN_DEVICE_EARPIECE) {
        bool mHACon = SpeechEnhancementController::GetInstance()->GetHACOn();
        if (mHACon) {
            ALOGD("%s(): HAC ON = %d, gain device change from EarPiece to HAC", __FUNCTION__, mHACon);
            Mode = GAIN_DEVICE_HAC;
        }
    }
#endif

    uint8_t sidetone = mGainTable.nonSceneGain.sidetoneGain[mBand][mNetwork][Mode].gain;
    if (sidetone > mSpec->sidetoneIdxMax) {
        sidetone = mSpec->sidetoneIdxMax;
    }

    uint16_t updated_sidetone = 0;

    if (isEarpieceCategory((enum GAIN_DEVICE)Mode)) {
        if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER) ||
            IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER)) {
            if (mSpec->spkAnaType >= 0 && mSpec->spkAnaType < NUM_GAIN_ANA_TYPE) {
                updated_sidetone = updateSidetone(mSpec->spkGainDb[GetSPKGain()], sidetone, mHwVolume.swAgc);
            } else {
                updated_sidetone = updateSidetone(0, sidetone, mHwVolume.swAgc);
            }
        } else {
            updated_sidetone = updateSidetone(mSpec->voiceBufferGainDb[GetReceiverGain()], sidetone, mHwVolume.swAgc);
        }
    } else if (isHeadsetCategory((enum GAIN_DEVICE)Mode)) {
        updated_sidetone = updateSidetone(mSpec->audioBufferGainDb[GetHeadphoneRGain()], sidetone, mHwVolume.swAgc);
    } else if (Mode == GAIN_DEVICE_SPEAKER) {
        //value = updateSidetone(KeyVoiceBufferGain[GetReceiverGain()], sidetone, mHwVolume.swAgc);
        // mute sidetone gain when speaker mode.
        updated_sidetone = 0;
    }

    ALOGD("ApplySideTone gainDevice %d, sidetone %u, updated_sidetone %u", Mode, sidetone, updated_sidetone);
#if !defined(SPH_AP_SET_SIDETONE)
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetSidetoneGain(updated_sidetone);
#endif
    return NO_ERROR;
}

uint16_t AudioMTKGainController::updateSidetone(int dlPGAGain, int  sidetone, uint8_t ulGain) {

    int vol = 0;
    uint16_t DSP_ST_GAIN = 0;
    int positive_gain_db = 0;
    int oldDlPGAGain = dlPGAGain;
    int oldSidetone = sidetone;
    int oldUlGain = ulGain;

    if (sidetone == 0) {
        DSP_ST_GAIN = 0;
    } else {
        vol = sidetone + ulGain; //1dB/step
#if !defined(SPH_AP_SET_SIDETONE)
        vol = dlPGAGain - vol + 67 - mSpec->ulGainOffset;
#else
        vol = dlPGAGain - vol + 49 - mSpec->ulGainOffset;

#if defined(SPH_POSITIVE_SIDETONE_GAIN)
        if (vol < 0) {
            vol = vol + 6;
            positive_gain_db += 6;
        }
#endif
#endif

        if (vol < 0) {
            vol = 0;
        }
        if (vol > mSpec->sidetoneIdxMax) {
            vol = mSpec->sidetoneIdxMax;
        }
        DSP_ST_GAIN = mSpec->stfGainMap[vol];
    }
    ALOGD("%s(), DSP_ST_GAIN = %d, positive_gain_db = %d, dlPGAGain %d, sidetone %d, ulGain %u",
          __FUNCTION__, DSP_ST_GAIN, positive_gain_db,
          oldDlPGAGain, oldSidetone, oldUlGain);

#if defined(SPH_AP_SET_SIDETONE)
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Sidetone_Gain"), 0, DSP_ST_GAIN)) {
        ALOGW("%s(), set Sidetone_Gain fail", __FUNCTION__);
    }
#if defined(SPH_POSITIVE_SIDETONE_GAIN)
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Sidetone_Positive_Gain_dB"), 0, positive_gain_db)) {
        ALOGW("%s(), set Sidetone_Positive_Gain_dB fail", __FUNCTION__);
    }
#endif
#endif

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

/*
status_t AudioMTKGainController::SetDigitalHwGain(int _digitalLinearGain, enum GAINTABLE_HWGAIN_TYPE _hwGainType)
{
    ALOGD("%s(), _digitalLinearGain = %d, _hwGainType = %d", __FUNCTION__, _digitalLinearGain, _hwGainType);

    uint32_t hw_gain = kHWGainMap[_digitalLinearGain >> 1]; // 0 ~ 0x80000

    switch (_hwGainType)
    {
        case GAINTABLE_HWGAIN1:
            return mHardwareResourceManager->setHWGain2DigitalGain(hw_gain);
        case GAINTABLE_HWGAIN2:
            ALOGW("%s(), error, HW Gain2 not supported", __FUNCTION__);
            return BAD_VALUE;
        default:
            ALOGW("%s(), error, undefined _HwGainType = %d", __FUNCTION__, _hwGainType);
            break;
    }
    return BAD_VALUE;
}

enum GAINTABLE_HWGAIN_TYPE AudioMTKGainController::GetHWGainTypeForFM()
{
    return GAINTABLE_HWGAIN1;
}
*/
int AudioMTKGainController::GetDigitalLinearGain(int _volIdx, audio_devices_t _device, audio_stream_type_t _streamType) {
    ALOGD("%s(), _volIdx = %d, _device = %d, _streamType = %d", __FUNCTION__, _volIdx, _device, _streamType);

    GAIN_DEVICE gainDevice = getGainDevice(_device);

    // convert _volIdx to linear(0~255)
    if (!isValidStreamType(_streamType)) {
        ALOGE("error, Invalid stream type = %d", _streamType);
        _streamType = AUDIO_STREAM_MUSIC;
    }
    int linearGain = mGainTable.sceneGain[mSceneIndex].streamGain[_streamType][gainDevice][_volIdx].digital;
    return keyvolumeStep - linearGain;
}

float AudioMTKGainController::GetDigitalLogGain(int _volIdx, audio_devices_t _device, audio_stream_type_t _streamType) {
    return linearToLog(GetDigitalLinearGain(_volIdx, _device, _streamType));
}

bool AudioMTKGainController::isValidStreamType(audio_stream_type_t _streamType) {
    return (_streamType >= GAIN_MIN_STREAM_TYPE && _streamType <= GAIN_MAX_STREAM_TYPE);
}

bool AudioMTKGainController::isValidVolIdx(int _idx, audio_mode_t _mode) {
    if (isInVoiceCall(_mode)) {
        return (_idx >= 0 && _idx <= GAIN_MAX_SPEECH_VOL_INDEX);
    } else {
        return (_idx >= 0 && _idx <= GAIN_MAX_VOL_INDEX);
    }
}

bool AudioMTKGainController::isHeadsetCategory(enum GAIN_DEVICE _gainDevice) {
    return _gainDevice == GAIN_DEVICE_HEADPHONE ||
           _gainDevice == GAIN_DEVICE_HEADSET ||
           _gainDevice == GAIN_DEVICE_HSSPK ||
           _gainDevice == GAIN_DEVICE_HEADSET_5POLE ||
           _gainDevice == GAIN_DEVICE_HEADSET_5POLE_ANC ||
           _gainDevice == GAIN_DEVICE_LPBK_HP;
}

bool AudioMTKGainController::isEarpieceCategory(enum GAIN_DEVICE _gainDevice) {
    return _gainDevice == GAIN_DEVICE_EARPIECE ||
           _gainDevice == GAIN_DEVICE_HAC ||
           _gainDevice == GAIN_DEVICE_LPBK_RCV ||
           _gainDevice == GAIN_DEVICE_RCV_EV ;
}

bool AudioMTKGainController::isSpeakerCategory(enum GAIN_DEVICE _gainDevice) {
    return _gainDevice == GAIN_DEVICE_SPEAKER ||
           (_gainDevice == GAIN_DEVICE_HSSPK && mSpec->spkAnaType != GAIN_ANA_HEADPHONE) ||
           _gainDevice == GAIN_DEVICE_LPBK_SPK ||
           _gainDevice == GAIN_DEVICE_SPK_EV;
}

bool AudioMTKGainController::isDmicDevice(enum GAIN_DEVICE _gainDevice) {
    return IsAudioSupportFeature(AUDIO_SUPPORT_DMIC) && // use DMIC & is built in mic
           (_gainDevice == GAIN_DEVICE_EARPIECE ||
            _gainDevice == GAIN_DEVICE_SPEAKER ||
            _gainDevice == GAIN_DEVICE_HEADPHONE ||
            _gainDevice == GAIN_DEVICE_LPBK_RCV ||
            _gainDevice == GAIN_DEVICE_LPBK_SPK ||
            _gainDevice == GAIN_DEVICE_RCV_EV ||
            _gainDevice == GAIN_DEVICE_SPK_EV);
}

uint32_t AudioMTKGainController::getHpImpedanceIdx(int32_t impedance) {
    for (unsigned int i = 0; i < mSpec->hpImpThresholdList.size(); i++) {
        if (impedance <= mSpec->hpImpThresholdList[i]) {
            return i;
        }
    }

    return mSpec->hpImpThresholdList.size();
}

int  AudioMTKGainController::getHpImpedanceCompesateValue(void) {
    ASSERT(mHpImpedanceIdx <= mSpec->hpImpThresholdList.size());

    ALOGG("%s(), mHpImpedanceIdx = %d, compensate value = %d",
          __FUNCTION__,
          mHpImpedanceIdx,
          mSpec->hpImpCompensateList[mHpImpedanceIdx]);

    return mSpec->hpImpCompensateList[mHpImpedanceIdx];
}

int AudioMTKGainController::tuneGainForMasterVolume(int gain, audio_mode_t mode, GAIN_DEVICE gainDevice) {
    int bufferGainPreferMaxIdx = 0;
    if (isHeadsetCategory(gainDevice)) {
        bufferGainPreferMaxIdx = mSpec->audioBufferGainPreferMaxIdx;
    } else if (isSpeakerCategory(gainDevice)) {
        switch (mSpec->spkAnaType) {
        case GAIN_ANA_HEADPHONE:
            bufferGainPreferMaxIdx = mSpec->audioBufferGainPreferMaxIdx;
            break;
        case GAIN_ANA_LINEOUT:
            bufferGainPreferMaxIdx = mSpec->lineoutBufferGainPreferMaxIdx;
            break;
        case GAIN_ANA_SPEAKER:
        default:
            ALOGE("%s(), error, this should not happen", __FUNCTION__);
            ASSERT(false);
            break;
        }
    } else { // if (isEarpieceCategory(gainDevice))
        bufferGainPreferMaxIdx = mSpec->voiceBufferGainPreferMaxIdx;
    }

    // adjust gain according to master volume
    if (!isInVoiceCall(mode) && // mMasterVolume don't affect voice call
        gain <= bufferGainPreferMaxIdx) { // don't change gain if already mute(-40dB)
        int degradedB = (keyvolumeStep - logToLinear(mMasterVolume)) * keydBPerStep;
        ALOGG("%s(), degraded gain of mMasterVolume = %d dB", __FUNCTION__, degradedB);
        if (gain + degradedB <= bufferGainPreferMaxIdx) {
            gain += degradedB;
        } else {
            gain = bufferGainPreferMaxIdx;
        }
    }

    return gain;
}

int AudioMTKGainController::tuneGainForHpImpedance(int gain, GAIN_DEVICE gainDevice) {
    if (isHeadsetCategory(gainDevice) &&
        mSpec->hpImpEnable &&
        gain <= mSpec->audioBufferGainPreferMaxIdx && // don't change gain if already mute(-40dB)
        !mANCEnable) {
        ALOGG("%s(), before compesate HP impedance, bufferGain = %d", __FUNCTION__, gain);
        gain += getHpImpedanceCompesateValue();
        // prevent set to mute(-40dB) when change gain for HP impedance
        if (gain > mSpec->audioBufferGainPreferMaxIdx) {
            gain = mSpec->audioBufferGainPreferMaxIdx;
        } else if (gain < 0) {
            gain = 0;
        }

        ALOGD("%s(), after compesate HP impedance idx (%d), bufferGain = %d",
              __FUNCTION__,
              mHpImpedanceIdx,
              gain);
    }

    return gain;
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

uint16_t AudioMTKGainController::MappingToDigitalGain(unsigned char Gain __unused) {
    return 0;
}

uint16_t AudioMTKGainController::MappingToPGAGain(unsigned char Gain __unused) {
    return 0;
}

status_t AudioMTKGainController::setMasterVolume(float v, audio_mode_t mode, uint32_t devices) {
    ALOGG("%s(), mMasterVolume = %f, mode = %d, devices = 0x%x", __FUNCTION__, v, mode, devices);
    mMasterVolume = v;

    if (mode == AUDIO_MODE_CURRENT) {
        mode = mHwStream.mode;
    }
    mHwStream.mode = mode;
    mHwStream.devices = devices;
    if (!isInVoiceCall(mode)) {
        setNormalVolume(mHwStream.stream, mHwStream.index, devices, mode);
    }
    return NO_ERROR;
}

float AudioMTKGainController::getMasterVolume() {
    ALOGV("AudioMTKGainController getMasterVolume");
    return mMasterVolume;
}


status_t AudioMTKGainController::setVoiceVolume(float v, audio_mode_t mode, uint32_t device) {
    mVoiceVolume = v;
    mHwStream.mode = mode;
    mHwStream.devices = device;
    setVoiceVolume(mHwStream.index, device, mode);
    return NO_ERROR;
}

float AudioMTKGainController::getVoiceVolume(void) {
    return mVoiceVolume;
}

void AudioMTKGainController::setVoiceVolumeIndex(int volumeIndex) {
    mHwStream.index = volumeIndex;
}

status_t AudioMTKGainController::setVoiceVolume(int MapVolume __unused, uint32_t device __unused) {
    return INVALID_OPERATION;
}
status_t AudioMTKGainController::ApplyVoiceGain(int degradeDb __unused, audio_mode_t mode __unused, uint32_t device __unused) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::setStreamVolume(int stream __unused, float v __unused) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::setStreamMute(int stream __unused, bool mute __unused) {
    return INVALID_OPERATION;
}

float AudioMTKGainController::getStreamVolume(int stream __unused) {
    return 1.0;
}

// should depend on different usage , FM ,MATV and output device to setline in gain
status_t AudioMTKGainController::SetLineInPlaybackGain(int type __unused) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::SetLineInRecordingGain(int type __unused) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::SetSideTone(uint32_t Mode __unused, uint32_t Gain __unused) {

    return INVALID_OPERATION;
}

uint32_t AudioMTKGainController::GetSideToneGain(uint32_t device __unused) {

    return INVALID_OPERATION;
}


status_t AudioMTKGainController::SetMicGain(uint32_t Mode __unused, uint32_t Gain __unused) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::SetULTotalGain(uint32_t Mode __unused, unsigned char Gain __unused) {
    /* deprecated */
    return NO_ERROR;
}

status_t AudioMTKGainController::SetDigitalHwGain(uint32_t Mode __unused, uint32_t Gain __unused, uint32_t routes __unused) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::SetMicGainTuning(uint32_t Mode __unused, uint32_t Gain __unused) {
    return INVALID_OPERATION;
}

status_t AudioMTKGainController::SetMicGainTuning(GAIN_MIC_MODE micMode, GAIN_DEVICE gainDevice, uint32_t gainDecimal) {
    unsigned char micGain;

    // handle deprecated GAIN_MIC_MODE
    if (micMode >= NUM_GAIN_MIC_MODE) {
        if (micMode == Normal_Mic) {
            micMode = GAIN_MIC_VOICE_CALL;
            gainDevice = GAIN_DEVICE_EARPIECE;
        } else if (micMode == Handfree_Mic) {
            micMode = GAIN_MIC_VOICE_CALL;
            gainDevice = GAIN_DEVICE_SPEAKER;
        } else if (micMode == Headset_Mic) {
            micMode = GAIN_MIC_VOICE_CALL;
            gainDevice = getGainDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
        } else {
            ALOGE("%s(), micMode %d not supported", __FUNCTION__, micMode);
            return BAD_VALUE;
        }
    }

    // decimal to micIdx
    micGain = mSpec->micIdxMax[gainDevice] - (mSpec->decRecMax - gainDecimal) / mSpec->decRecStepPerDb;

    if (micGain > mSpec->micIdxMax[gainDevice]) {
        micGain = mSpec->micIdxMax[gainDevice];
    }
    if (micGain < mSpec->micIdxMin[gainDevice]) {
        micGain = mSpec->micIdxMin[gainDevice];
    }

    if (micMode == GAIN_MIC_VOICE_CALL) {
        mGainTable.nonSceneGain.speechMicGain[mBand][mNetwork][gainDevice].gain = micGain;
    } else {
        mGainTable.sceneGain[mSceneIndex].micGain[micMode][gainDevice].gain = micGain;
    }

    ALOGD("%s(), micMode %d, gainDevice %d, gainDecimal %u, set micGain %hhu", __FUNCTION__, micMode, gainDevice, gainDecimal, micGain);

    return NO_ERROR;
}

unsigned short AudioMTKGainController::getMicGainDecimal(GAIN_MIC_MODE micMode, GAIN_DEVICE gainDevice) {
    uint8_t micGain;

    // handle deprecated GAIN_MIC_MODE
    if (micMode >= NUM_GAIN_MIC_MODE) {
        if (micMode == Normal_Mic) {
            micMode = GAIN_MIC_VOICE_CALL;
            gainDevice = GAIN_DEVICE_EARPIECE;
        } else if (micMode == Handfree_Mic) {
            micMode = GAIN_MIC_VOICE_CALL;
            gainDevice = GAIN_DEVICE_SPEAKER;
        } else if (micMode == Headset_Mic) {
            micMode = GAIN_MIC_VOICE_CALL;
            gainDevice = getGainDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
        } else {
            ALOGE("%s(), MicType %d not supported", __FUNCTION__, micMode);
            micMode = GAIN_MIC_VOICE_CALL;
            gainDevice = GAIN_DEVICE_EARPIECE;
        }
    }

    micGain = micMode == GAIN_MIC_VOICE_CALL ?
              mGainTable.nonSceneGain.speechMicGain[mBand][mNetwork][gainDevice].gain :
              mGainTable.sceneGain[mSceneIndex].micGain[micMode][gainDevice].gain;

    if (micGain > mSpec->micIdxMax[gainDevice]) {
        micGain = mSpec->micIdxMax[gainDevice];
    }
    if (micGain < mSpec->micIdxMin[gainDevice]) {
        micGain = mSpec->micIdxMin[gainDevice];
    }

    // calculate decimal, micIdxMax <--> mSpec->decRecMax
    unsigned short micGainDecimal = mSpec->decRecMax - (mSpec->micIdxMax[gainDevice] - micGain) * mSpec->decRecStepPerDb;
    ALOGD("%s(), micMode %d, gainDevice %d, micGainDecimal %hu",
          __FUNCTION__, micMode, gainDevice, micGainDecimal);

    return micGainDecimal;
}

bool AudioMTKGainController::GetHeadPhoneImpedance(void) {
    ALOGG("%s(), mSpec->hpImpEnable = %d OnBoardResistor = %d mHpImpedanceIdx= %d",
          __FUNCTION__, mSpec->hpImpEnable, mSpec->hpImpOnBoardResistor, mHpImpedanceIdx);

    if (mSpec->hpImpEnable) {
#if defined(MTK_AUDIO_KS)
        // enable impedance detection path
        struct pcm_config config;
        String8 apTurnOnSequence = String8(AUDIO_CTL_HP_IMPEDANCE);

        memset(&config, 0, sizeof(config));
        config.channels = 2;
        config.rate = 48000;
        config.period_size = 1024;
        config.period_count = 2;
        config.format = PCM_FORMAT_S16_LE;
        config.stop_threshold = ~(0U);

        int cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmHostlessADDADLI2SOut);
        int pcmIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessADDADLI2SOut);

        mHardwareResourceManager->enableTurnOnSequence(apTurnOnSequence);

        struct pcm *pcm = pcm_open(cardIndex, pcmIndex, PCM_OUT, &config);
        if (pcm == NULL || pcm_is_ready(pcm) == false) {
            ALOGE("%s(), Unable to open pcm device %u (%s)", __FUNCTION__, pcmIndex, pcm_get_error(pcm));
        } else {
            if (pcm_start(pcm)) {
                ALOGE("%s(), pcm_start %p fail due to %s", __FUNCTION__, pcm, pcm_get_error(pcm));
            }
        }

#endif
        uint32_t newHPImpedance = 0;
        int32_t HPImpedanceValue = 0;

        struct mixer_ctl *ctl;
        unsigned int num_values, i;
        ctl = mixer_get_ctl_by_name(mMixer, "Audio HP ImpeDance Setting");

        num_values = mixer_ctl_get_num_values(ctl);
        for (i = 0; i < num_values; i++) {
            newHPImpedance = mixer_ctl_get_value(ctl, i);
            HPImpedanceValue = newHPImpedance - mSpec->hpImpOnBoardResistor;
        }

#if defined(MTK_AUDIO_KS)
        if (pcm) {
            pcm_stop(pcm);
            pcm_close(pcm);
            pcm = NULL;
        }

        mHardwareResourceManager->disableTurnOnSequence(apTurnOnSequence);
#endif

        uint32_t newHpImpedanceIdx = getHpImpedanceIdx(HPImpedanceValue);
        ALOGD("%s(), newHpImpedanceIdx = %d, Detected value = %d, Headphone Impedance = %d, OnBoardResistor = %d",
              __FUNCTION__, newHpImpedanceIdx, newHPImpedance, HPImpedanceValue, mSpec->hpImpOnBoardResistor);

        // update volume setting
        if (mHpImpedanceIdx != newHpImpedanceIdx) {
            mHpImpedanceIdx = newHpImpedanceIdx;
            setAnalogVolume_l(mHwStream.stream, mHwStream.devices, mHwStream.index, mHwStream.mode);
        }
    }

    return true;
}

int AudioMTKGainController::ApplyAudioGainTuning(int Gain __unused, uint32_t mode __unused, uint32_t device __unused) {
    return 0;
}

void AudioMTKGainController::SetLinoutRGain(int DegradedBGain __unused) {

}

void AudioMTKGainController::SetLinoutLGain(int DegradedBGain __unused) {

}

uint32_t AudioMTKGainController::GetOffloadGain(float vol_f __unused) {
    return 0;
}

void AudioMTKGainController::setANCEnable(bool _enable) {
#ifdef MTK_AUDIO_SW_DRE
    AL_AUTOLOCK(mSWDRELock);
#endif
    mANCEnable = _enable;
#ifdef MTK_AUDIO_SW_DRE
    if (mANCEnable) {
        if (mSWDREMute) {
            SWDRERampToNormal();
            mSWDREMute = false;
        }
    } else {
        updateSWDREState(false, false);
    }
#endif
}

#ifdef MTK_AUDIO_SW_DRE
void AudioMTKGainController::registerPlaybackHandler(uint32_t _identity) {
    if (mMutedHandlerVector.indexOfKey(_identity) >= 0) {
        ALOGW("%s(), warn, playback handler already exist, _identity = %d", __FUNCTION__, _identity);
    } else {
        AL_AUTOLOCK(mSWDRELock);
        mMutedHandlerVector.add(_identity, false);
        updateSWDREState(true, false);
    }
}

void AudioMTKGainController::removePlaybackHandler(uint32_t _identity) {
    if (mMutedHandlerVector.indexOfKey(_identity) < 0) {
        ALOGW("%s(), warn, playback handler not found, _identity = %d", __FUNCTION__, _identity);
    } else {
        AL_AUTOLOCK(mSWDRELock);
        mMutedHandlerVector.removeItem(_identity);
        updateSWDREState(true, false);
    }
}

void AudioMTKGainController::requestMute(uint32_t _identity, bool _mute) {
    if (mMutedHandlerVector.indexOfKey(_identity) < 0) {
        ALOGW("%s(), warn, playback handler not found, _identity = %d", __FUNCTION__, _identity);
    } else {
        if (mMutedHandlerVector.valueFor(_identity) != _mute) {
            AL_AUTOLOCK(mSWDRELock);
            mMutedHandlerVector.replaceValueFor(_identity, _mute);
            updateSWDREState(false, true);
        }
    }
}

void AudioMTKGainController::setFmEnable(bool _enable) {
    AL_AUTOLOCK(mSWDRELock);
    if (_enable) {
        mFmEnable = true;
        if (mSWDREMute) {
            SWDRERampToNormal();
            mSWDREMute = false;
        }
    } else {
        mFmEnable = false;
        updateSWDREState(false, false);
    }
}

bool AudioMTKGainController::isOtherModuleWorking() {
    return mFmEnable || mANCEnable;
}

void AudioMTKGainController::updateSWDREState(bool _numChanged, bool _muteChanged) {
    bool curHasMuteHandler = false;
    size_t curNumHandler;

    if (_numChanged == false && _muteChanged == true) {
        for (size_t i = 0; i < mMutedHandlerVector.size(); i++) {
            if (mMutedHandlerVector.valueAt(i) == true) {
                curHasMuteHandler = true;
                break;
            }
        }

        if (mNumHandler == 1 && !isOtherModuleWorking()) {
            if (mHasMuteHandler == false && curHasMuteHandler == true) {
                ALOGD("%s(), num = 1, hasMuteHandler false -> true", __FUNCTION__);
                mSWDREMute = true;
                // call ramp to mute
                SWDRERampToMute();
            } else if (mHasMuteHandler == true && curHasMuteHandler == false) {
                ALOGD("%s(), num = 1, hasMuteHandler true -> false", __FUNCTION__);
                // call ramp to normal
                SWDRERampToNormal();
                mSWDREMute = false;
            }
        }

        mHasMuteHandler = curHasMuteHandler;
    } else if (_numChanged == true && _muteChanged == false) {
        curNumHandler = mMutedHandlerVector.size();

        if (mHasMuteHandler) {
            if (mNumHandler > 1 && curNumHandler == 1 && !isOtherModuleWorking()) {
                ALOGD("%s(), hasMuteHandler, numHandler >1 -> 1", __FUNCTION__);
                mSWDREMute = true;
                // call ramp to mute
                SWDRERampToMute();
            } else if (mNumHandler == 1 && curNumHandler > 1 && !isOtherModuleWorking()) {
                ALOGD("%s(), hasMuteHandler, numHandler 1 -> >1", __FUNCTION__);
                // call ramp to normal
                SWDRERampToNormal();
                mSWDREMute = false;
            } else if (curNumHandler == 0) {
                ALOGD("%s(), hasMuteHandler, numHandler 1 -> 0", __FUNCTION__);
                mHasMuteHandler = false;
                // call ramp to normal
                SWDRERampToNormal();
                mSWDREMute = false;
            }
        }

        mNumHandler = curNumHandler;
    } else {
        if (mNumHandler == 1 && mHasMuteHandler && !isOtherModuleWorking()) {
            ALOGD("%s(), num = 1, has mute", __FUNCTION__);
            mSWDREMute = true;
            // call ramp to mute
            SWDRERampToMute();
        }
        return;
    }
}

void AudioMTKGainController::SWDRERampToMute() {
    int cur_idx = GetHeadphoneLGain();
    int target_idx = (int)(mSpec->audioBufferGainString.size() - 1);

    ALOGD("%s(), cur_idx = %d, target_idx = %d", __FUNCTION__, cur_idx, target_idx);
#if defined(MTK_HYBRID_NLE_SUPPORT)
    if (AudioALSAHyBridNLEManager::getInstance()->setNleHwConfigByIndex(target_idx) == FAILED_TRANSACTION) { // the parameter using Index of HP Gain Reg
        SetHeadPhoneLGain(target_idx);
        SetHeadPhoneRGain(target_idx);
    }
#else
    SetHeadPhoneLGain(target_idx);
    SetHeadPhoneRGain(target_idx);
#endif
}

void AudioMTKGainController::SWDRERampToNormal() {
    int cur_idx = GetHeadphoneLGain();
    if (cur_idx != (int)mSpec->audioBufferGainString.size() - 1) {
        ALOGW("%s(), cur_idx %d != mute", __FUNCTION__, cur_idx);
    }

    GAIN_DEVICE gainDevice = getGainDevice(mHwStream.devices);

    /* routing will change the mHwStream.devices, error handle */
    if (gainDevice != GAIN_DEVICE_HEADSET && gainDevice != GAIN_DEVICE_HEADPHONE &&
        gainDevice != GAIN_DEVICE_HEADSET_5POLE && gainDevice != GAIN_DEVICE_HEADSET_5POLE_ANC) {
        gainDevice = GAIN_DEVICE_HEADSET;
    }

    unsigned char bufferGain = mGainTable.sceneGain[mSceneIndex].streamGain[mHwStream.stream][gainDevice][mHwStream.index].analog[GAIN_ANA_HEADPHONE];

    if (bufferGain >= mSpec->audioBufferGainString.size()) {
        bufferGain = mSpec->audioBufferGainString.size() - 1;
    }

    // adjust gain according to master volume
    bufferGain = tuneGainForMasterVolume(bufferGain, mHwStream.mode, gainDevice);

    // adjust gain according to hp impedance when using HP
    bufferGain = tuneGainForHpImpedance(bufferGain, gainDevice);

    int target_idx = bufferGain;

    ALOGD("%s(), cur_idx = %d, target_idx = %d, cur_devices = 0x%x", __FUNCTION__, cur_idx, target_idx, mHwStream.devices);
#if defined(MTK_HYBRID_NLE_SUPPORT)
    if (AudioALSAHyBridNLEManager::getInstance()->setNleHwConfigByIndex(target_idx) == FAILED_TRANSACTION) { // the parameter using Index of HP Gain Reg
        SetHeadPhoneLGain(target_idx);
        SetHeadPhoneRGain(target_idx);
    }
#else
    SetHeadPhoneLGain(target_idx);
    SetHeadPhoneRGain(target_idx);
#endif
}

#endif
}

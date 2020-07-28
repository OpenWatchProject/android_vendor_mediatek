#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechParserGen93"
#include <SpeechParserGen93.h>
#include <stdlib.h>     /* atoi */

#include <utils/Log.h>
#include <inttypes.h>

#include <media/AudioParameter.h>

#include <AudioLock.h>
#include <AudioUtility.h>//Mutex/assert
#include <AudioEventThreadManager.h>
#include <audio_memory_control.h>


namespace android {

#define MAX_BYTE_PARAM_SPEECH 3434
#define MAX_BYTE_PARAM_SPEECH_NETWORK 10


#define SPH_DUMP_STR_SIZE (500)
#define SPH_PARAM_UNIT_DUMP_STR_SIZE (1024)

static const uint32_t kSphParamSize =  0x3520; // AUDIO_TYPE_SPEECH, => 13K

/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT {
    uint16_t sphParserVer;
    uint16_t numLayer ;
    uint16_t numEachLayer ;
    uint16_t paramHeader[4] ;//Network, VoiceBand, Reserved, Reserved
    uint16_t sphUnitMagiNum;

    SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT() : sphParserVer(0), numLayer(0),
        numEachLayer(0), paramHeader(), sphUnitMagiNum(0) {}
};

struct AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT {
    char *audioTypeName;
    char numCategoryType;//4
    std::vector<String8> categoryType;
    std::vector<String8> categoryName;
    char numParam;//4
    std::vector<String8> paramName;
    char *logPrintParamUnit;

    AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT() : audioTypeName(NULL), numCategoryType(0),
        categoryType(), categoryName(), numParam(0), paramName(),
        logPrintParamUnit(NULL) {}
};

struct SPEECH_PARAM_INFO_STRUCT {
    speech_mode_t speechMode;
    unsigned int idxVolume;
    bool isBtNrecOn;
    bool isLPBK;
    unsigned char numHeadsetPole;
    bool isSingleBandTransfer;
    unsigned char idxVoiceBandStart;
    bool isSV;
    unsigned char idxTTY;

    SPEECH_PARAM_INFO_STRUCT() : speechMode(SPEECH_MODE_NORMAL), idxVolume(0), isBtNrecOn(0),
        isLPBK(0), numHeadsetPole(0), isSingleBandTransfer(0), idxVoiceBandStart(0),
        isSV(0), idxTTY(0) {}
};

struct SPEECH_PARAM_SUPPORT_STRUCT {
    bool isNetworkSupport;
    bool isTTYSupport;
    bool isSuperVolumeSupport;

    SPEECH_PARAM_SUPPORT_STRUCT() : isNetworkSupport(0), isTTYSupport(0),
        isSuperVolumeSupport(0) {}
};

struct SPEECH_NETWORK_STRUCT {
    char name[128];
    uint16_t supportBit;//4

    SPEECH_NETWORK_STRUCT() : name(), supportBit(0) {}
};

enum speech_profile_t {
    SPEECH_PROFILE_HANDSET = 0,
    SPEECH_PROFILE_4_POLE_HEADSET = 1,
    SPEECH_PROFILE_HANDSFREE = 2,
    SPEECH_PROFILE_BT_EARPHONE = 3,
    SPEECH_PROFILE_BT_NREC_OFF = 4,
    SPEECH_PROFILE_MAGICONFERENCE = 5,
    SPEECH_PROFILE_HAC = 6,
    SPEECH_PROFILE_LPBK_HANDSET = 7,
    SPEECH_PROFILE_LPBK_HEADSET = 8,
    SPEECH_PROFILE_LPBK_HANDSFREE = 9,
    SPEECH_PROFILE_3_POLE_HEADSET = 10,
    SPEECH_PROFILE_5_POLE_HEADSET = 11,
    SPEECH_PROFILE_5_POLE_HEADSET_ANC = 12,
    SPEECH_PROFILE_USB_HEADSET = 13,
    SPEECH_PROFILE_HANDSET_SV = 14,
    SPEECH_PROFILE_HANDSFREE_SV = 15,
    SPEECH_PROFILE_TTY_HCO_HANDSET = 16,
    SPEECH_PROFILE_TTY_HCO_HANDSFREE = 17,
    SPEECH_PROFILE_TTY_VCO_HANDSET = 18,
    SPEECH_PROFILE_TTY_VCO_HANDSFREE = 19,

    SPEECH_PROFILE_MAX_NUM = 20
};

//--------------------------------------------------------------------------------
/* XML name */
const char audioTypeNameList[8][128] = {
    "Speech",
    "SpeechDMNR",
    "SpeechGeneral",
    "SpeechMagiClarity",
    "SpeechNetwork",
    "SpeechEchoRef"
};

//audio type: Speech
#define MAX_NUM_CATEGORY_TYPE_SPEECH 4
#define MAX_NUM_PARAM_SPEECH 3
const String8 audioType_Speech_CategoryType[ ] = {
    String8("Band"),
    String8("Profile"),
    String8("VolIndex"),
    String8("Network")
};

const String8 audioType_Speech_ParamName[ ] = {
    String8("speech_mode_para"),
    String8("sph_in_fir"),
    String8("sph_out_fir"),
    String8("sph_in_iir_mic1_dsp"),
    String8("sph_in_iir_mic2_dsp"),
    String8("sph_in_iir_enh_dsp"),
    String8("sph_out_iir_enh_dsp")
};

#define NUM_VOLUME_SPEECH 7
const char audioType_Speech_CategoryName3[NUM_VOLUME_SPEECH][128] = {"0", "1", "2", "3", "4", "5", "6"};
const char audioType_Speech_CategoryName2[SPEECH_PROFILE_MAX_NUM][128] = {
    "Normal",
    "4_pole_Headset",
    "Handsfree",
    "BT_Earphone",
    "BT_NREC_Off",
    "MagiConference",
    "HAC",
    "Lpbk_Handset",
    "Lpbk_Headset",
    "Lpbk_Handsfree",
    "3_pole_Headset",
    "5_pole_Headset",
    "5_pole_Headset+ANC",
    "Usb_Headset",
    "Handset_SV",
    "Handsfree_SV",
    "Tty_HCO_Handset",
    "Tty_HCO_Handsfree",
    "Tty_VCO_Handset",
    "Tty_VCO_Handsfree"
};

//--------------------------------------------------------------------------------
//audio type: SpeechDMNR
#define MAX_NUM_CATEGORY_TYPE_SPEECH_DMNR 2
#define MAX_NUM_PARAM_SPEECH_DMNR 1
const String8 audioType_SpeechDMNR_CategoryType[ ] = {String8("Band"), String8("Profile")};
const char audioType_SpeechDMNR_CategoryName2[2][128] = {"Handset", "MagiConference"};
const String8 audioType_SpeechDMNR_ParamName[ ] = {String8("dmnr_para")};

//--------------------------------------------------------------------------------
//audio type: SpeechGeneral
#define MAX_NUM_CATEGORY_TYPE_SPEECH_GENERAL 1
#define MAX_NUM_PARAM_SPEECH_GENERAL 2
const String8 audioType_SpeechGeneral_CategoryType[ ] = {String8("CategoryLayer")};
const char audioType_SpeechGeneral_CategoryName1[1][128] = {"Common"};
const String8 audioType_SpeechGeneral_ParamName[ ] = {String8("speech_common_para"), String8("debug_info")};

//--------------------------------------------------------------------------------
//audio type: SpeechMagiClarity
#define MAX_NUM_CATEGORY_TYPE_SPEECH_MAGICLARITY 1
#define MAX_NUM_PARAM_SPEECH_MAGICLARITY 1
const String8 audioType_SpeechMagiClarity_CategoryType[ ] = {String8("CategoryLayer")};
const char audioType_SpeechMagiClarity_CategoryName1[1][128] = {"Common"};
const String8 audioType_SpeechMagiClarity_ParamName[ ] = {String8("shape_rx_fir_para")};

//--------------------------------------------------------------------------------
//audio type: SpeechNetwork
#define MAX_NUM_CATEGORY_TYPE_SPEECH_NETWORK 1
#define MAX_NUM_PARAM_SPEECH_NETWORK 1
const String8 audioType_SpeechNetwork_CategoryType[ ] = {String8("Network")};
const String8 audioType_SpeechNetwork_ParamName[ ] = {String8("speech_network_support")};


//--------------------------------------------------------------------------------
//audio type: SpeechEchoRef
#define MAX_NUM_CATEGORY_TYPE_SPEECH_ECHOREF 1
#define MAX_NUM_PARAM_SPEECH_ECHOREF 1
const String8 audioType_SpeechEchoRef_CategoryType[ ] = {String8("Device")};
const char audioType_SpeechEchoRef_CategoryName1[1][128] = {"USBAudio"};
const String8 audioType_SpeechEchoRef_ParamName[ ] = {String8("EchoRef_para")};

#define NUM_NEED_UPDATE_XML 3
#define LEN_XML_NAME 128
const char needUpdateXmlList[NUM_NEED_UPDATE_XML][LEN_XML_NAME] = {
    "Speech",
    "SpeechDMNR",
    "SpeechGeneral",
};

/*==============================================================================
 *                     Property keys
 *============================================================================*/

const char *PROPERTY_KEY_SPEECHLOG_ON = "persist.vendor.audiohal.speech_log_on";


/*==============================================================================
 *                     Callback Function
 *============================================================================*/
void callbackAudioXmlChanged(AppHandle *appHandle, const char *audioTypeName) {
    ALOGD("+%s(), audioType = %s", __FUNCTION__, audioTypeName);

    // reload XML file
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return;
    }
    bool isSpeechParamChanged = false, onlyUpdatedDuringCall = false;
    int idxXmlNeedUpdate = 0;

    if (appOps->appHandleReloadAudioType(appHandle, audioTypeName) != APP_ERROR) {
        for (idxXmlNeedUpdate = 0; idxXmlNeedUpdate < NUM_NEED_UPDATE_XML; idxXmlNeedUpdate++) {
            if (strcmp(audioTypeName, needUpdateXmlList[idxXmlNeedUpdate]) == 0) {
                isSpeechParamChanged = true;
                break;
            }
        }
        if (strcmp(audioTypeName, "Speech") == 0) {
            onlyUpdatedDuringCall = true;
        }

        if (isSpeechParamChanged) {
            if (!onlyUpdatedDuringCall) {
                SpeechParserGen93::getInstance()->mChangedXMLQueue.push_back((speech_type_dynamic_param_t)idxXmlNeedUpdate);
                AudioEventThreadManager::getInstance()->notifyCallback(AUDIO_EVENT_SPEECH_PARAM_CHANGE, SpeechParserGen93::getInstance());
            } else if (SpeechParserGen93::getInstance()->mCallOn) {
                SpeechParserGen93::getInstance()->mChangedXMLQueue.push_back((speech_type_dynamic_param_t)idxXmlNeedUpdate);
                AudioEventThreadManager::getInstance()->notifyCallback(AUDIO_EVENT_SPEECH_PARAM_CHANGE, SpeechParserGen93::getInstance());
            }
        }
    } else {
        (void) appHandle;
        ALOGE("%s(), Reload xml fail!(audioType = %s)", __FUNCTION__, audioTypeName);
    }
}

/*
 * =============================================================================
 *                     Singleton Pattern
 * =============================================================================
 */

SpeechParserGen93 *SpeechParserGen93::uniqueSpeechParser = NULL;


SpeechParserGen93 *SpeechParserGen93::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);
    if (uniqueSpeechParser == NULL) {
        uniqueSpeechParser = new SpeechParserGen93();
    }
    ASSERT(uniqueSpeechParser != NULL);
    return uniqueSpeechParser;
}

/*
 * =============================================================================
 *                     Constructor / Destructor / Init / Deinit
 * =============================================================================
 */

SpeechParserGen93::SpeechParserGen93() {
    ALOGD("%s()", __FUNCTION__);
    mSpeechParserAttribute.inputDevice = AUDIO_DEVICE_IN_BUILTIN_MIC;
    mSpeechParserAttribute.outputDevice = AUDIO_DEVICE_OUT_EARPIECE;
    mSpeechParserAttribute.idxVolume = 3;
    mSpeechParserAttribute.driverScenario = SPEECH_SCENARIO_SPEECH_ON;
    mSpeechParserAttribute.ttyMode = AUD_TTY_OFF;
    mSpeechParserAttribute.speechFeatureOn = 0;
    mCallOn = false;
    AUDIO_ALLOC_STRUCT(SPEECH_PARAM_SUPPORT_STRUCT, mSphParamSupport);
    AUDIO_ALLOC_STRUCT(SPEECH_PARAM_INFO_STRUCT, mSphParamInfo);
    AUDIO_ALLOC_STRUCT_ARRAY(SPEECH_NETWORK_STRUCT, 12, mListSpeechNetwork);
    AUDIO_ALLOC_STRUCT_ARRAY(SPEECH_NETWORK_STRUCT, 12, mNameForEachSpeechNetwork);
    mChangedXMLQueue.clear();

    mParamBufSize = getMaxBufferSize();
    if (mParamBufSize <= 0) {
        ALOGW("%s() mParamBufSize:%d, get buffer size fail!", __FUNCTION__, mParamBufSize);
    }
    init();
}

SpeechParserGen93::~SpeechParserGen93() {
    ALOGD("%s()", __FUNCTION__);
    AUDIO_FREE_POINTER(mParamBuf);
    AUDIO_FREE_POINTER(mNameForEachSpeechNetwork);
    AUDIO_FREE_POINTER(mListSpeechNetwork);
    AUDIO_FREE_POINTER(mSphParamInfo);
    AUDIO_FREE_POINTER(mSphParamSupport);
}

void SpeechParserGen93::init() {
    ALOGD("%s()", __FUNCTION__);
    initAppParser();
    initSpeechNetwork();

    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
    } else {
        const char *strSphVersion = appOps->appHandleGetFeatureOptionValue(mAppHandle, "SPH_PARAM_VERSION");
        if (strSphVersion != NULL) {
            sscanf(strSphVersion, "%" SCNd8 ".%" SCNd8, &mSpeechParamVerFirst, &mSpeechParamVerLast);
            switch (mSpeechParamVerFirst) {
            case 2:
                mSphParamSupport->isNetworkSupport = true;
                mNumSpeechParam = 7;
                break;
            case 1:
                mSphParamSupport->isNetworkSupport = true;
                mNumSpeechParam = 3;
                break;
            default:
                mSphParamSupport->isNetworkSupport = false;
                mNumSpeechParam = 3;
                break;
            }
        } else {
            mSpeechParamVerFirst = 0;
            mSpeechParamVerLast = 0;
            mSphParamSupport->isNetworkSupport = false;
            mNumSpeechParam = 3;
        }
        const char *strSphTTY = appOps->appHandleGetFeatureOptionValue(mAppHandle, "SPH_PARAM_TTY");
        if (strSphTTY != NULL) {
            if (strcmp(strSphTTY, "yes") == 0) {
                mSphParamSupport->isTTYSupport = true;
            } else {
                mSphParamSupport->isTTYSupport = false;
            }
        } else {
            mSphParamSupport->isTTYSupport = false;
        }

        const char *strSphSV = appOps->appHandleGetFeatureOptionValue(mAppHandle, "SPH_PARAM_SV");
        if (strSphSV != NULL) {
            if (strcmp(strSphSV, "yes") == 0) {
                mSphParamSupport->isSuperVolumeSupport = true;
            } else {
                mSphParamSupport->isSuperVolumeSupport = false;
            }
        } else {
            mSphParamSupport->isSuperVolumeSupport = false;
        }

        ALOGD("%s() appHandleRegXmlChangedCb", __FUNCTION__);
        /* XML changed callback process */
        appOps->appHandleRegXmlChangedCb(mAppHandle, callbackAudioXmlChanged);
    }
}

void SpeechParserGen93::deInit() {
    ALOGD("%s()", __FUNCTION__);
}


/*==============================================================================
 *                     SpeechParserGen93 Imeplementation
 *============================================================================*/

/**
 * =========================================================================
 *  @brief Parsing param file to get parameters into pOutBuf
 *
 *  @param speechParserAttribute: the attribute for parser
 *  @param pOutBuf: the output buffer
 *  @param sizeByteOutBuf: the size byte of output buffer
 *
 *  @return int
 * =========================================================================
 */

int SpeechParserGen93::getParamBuffer(SpeechParserAttribute speechParserAttribute, SpeechDataBufType *outBuf) {
    ALOGV("%s() XML scenario: 0x%x", __FUNCTION__, speechParserAttribute.driverScenario);

    mSpeechParserAttribute.inputDevice = speechParserAttribute.inputDevice;
    mSpeechParserAttribute.outputDevice = speechParserAttribute.outputDevice;
    mSpeechParserAttribute.idxVolume = speechParserAttribute.idxVolume;
    mSpeechParserAttribute.driverScenario = speechParserAttribute.driverScenario;
    mSpeechParserAttribute.speechFeatureOn = speechParserAttribute.speechFeatureOn;
    mSpeechParserAttribute.ttyMode = speechParserAttribute.ttyMode;
    ALOGD("%s() inputDevice:0x%x, outputDevice:0x%x, idxVolume:0x%x, Scenario:0x%x, FeatureOn:0x%x, ttyMode:0x%x",
          __FUNCTION__, mSpeechParserAttribute.inputDevice, mSpeechParserAttribute.outputDevice,
          mSpeechParserAttribute.idxVolume, mSpeechParserAttribute.driverScenario,
          mSpeechParserAttribute.speechFeatureOn, mSpeechParserAttribute.ttyMode);

    if (mSpeechParserAttribute.ttyMode != AUD_TTY_OFF && mSphParamSupport->isTTYSupport == false) {
        mSpeechParserAttribute.ttyMode = AUD_TTY_OFF;
        ALOGW("%s(), TTY not support! TTY mode: %d -> %d", __FUNCTION__,
              speechParserAttribute.ttyMode, mSpeechParserAttribute.ttyMode);
    }

    if (getFeatureOn(SPEECH_FEATURE_SUPERVOLUME) && mSphParamSupport->isSuperVolumeSupport == false) {
        mSpeechParserAttribute.speechFeatureOn &= ~(1 << SPEECH_FEATURE_SUPERVOLUME);
        ALOGW("%s(), SuperVolume not support! FeatureOn: %d -> %d", __FUNCTION__,
              speechParserAttribute.speechFeatureOn, mSpeechParserAttribute.speechFeatureOn);
    }

    /* dynamic allocate parser buffer */
    ASSERT(mParamBufSize > 0);
    AUDIO_FREE_POINTER(mParamBuf);
    AUDIO_ALLOC_BUFFER(mParamBuf, mParamBufSize);
    if (mParamBuf == NULL) {
        ALOGW("%s() Allocate Parser Buffer Fail!! expect:%d", __FUNCTION__, mParamBufSize);
        outBuf->memorySize = 0;
        outBuf->dataSize = 0;
        return -ENOMEM;
    }
    char *param_buf = (char *)mParamBuf;

    int concateSize = 0, sizeByte = 0;

    switch (mSpeechParserAttribute.driverScenario) {
    case SPEECH_SCENARIO_SPEECH_ON:
        sizeByte = getSpeechParamUnit(param_buf);
        if (sizeByte < 0) {return -ENOMEM;}
        concateSize += sizeByte;
        sizeByte = getDmnrParamUnit(param_buf + concateSize);
        if (sizeByte < 0) {return -ENOMEM;}
        concateSize += sizeByte;
        sizeByte = getGeneralParamUnit(param_buf + concateSize);
        if (sizeByte < 0) {return -ENOMEM;}
        concateSize += sizeByte;
        sizeByte = getMagiClarityParamUnit(param_buf + concateSize);
        if (sizeByte < 0) {return -ENOMEM;}
        concateSize += sizeByte;
        break;
    case SPEECH_SCENARIO_PARAM_CHANGE:
        if (mChangedXMLQueue.empty() != true) {
            mIdxAudioType = mChangedXMLQueue.front();
            mChangedXMLQueue.erase(mChangedXMLQueue.begin());
        } else {
            ALOGW("%s() Parameter changed XML queue empty!", __FUNCTION__);
        }
        if (mIdxAudioType == AUDIO_TYPE_SPEECH) {
            sizeByte = getSpeechParamUnit(param_buf);
        } else if (mIdxAudioType == AUDIO_TYPE_SPEECH_DMNR) {
            sizeByte = getDmnrParamUnit(param_buf);
        } else if (mIdxAudioType == AUDIO_TYPE_SPEECH_GENERAL) {
            sizeByte = getGeneralParamUnit(param_buf + concateSize);
        } else {
            ALOGW("%s(), Param Change type not support:%d", __FUNCTION__, mIdxAudioType);
        }
        if (sizeByte < 0) {return -ENOMEM;}
        concateSize += sizeByte;
        break;
    case SPEECH_SCENARIO_DEVICE_CHANGE:
    case SPEECH_SCENARIO_VOLUME_CHANGE:
    case SPEECH_SCENARIO_FEATURE_CHANGE:
        sizeByte = getSpeechParamUnit(param_buf);
        if (sizeByte < 0) {return -ENOMEM;}
        concateSize += sizeByte;
        break;
    default:
        ALOGW("%s(), not support scenario: %d", __FUNCTION__, mSpeechParserAttribute.driverScenario);
        sizeByte = 0;
        break;
    }
    outBuf->memorySize = kSphParamSize;
    outBuf->dataSize = concateSize;
    outBuf->bufferAddr = mParamBuf;

    ALOGV("%s() XML scenario: 0x%x, outBufSize:%d", __FUNCTION__, speechParserAttribute.driverScenario, concateSize);

    return 0;
}

/**
 * =========================================================================
 *  @brief set keyString string to library
 *
 *  @param keyString the "key=value" string
 *  @param sizeKeyString the size byte of string
 *
 *  @return int
 * =========================================================================
 */

int SpeechParserGen93::setKeyValuePair(const SpeechStringBufType *keyValuePair) {

    ALOGD("+%s(): %s", __FUNCTION__, keyValuePair->stringAddr);

    char *keyHeader = NULL;
    char *keyString = NULL;
    keyHeader = strtok_r(keyValuePair->stringAddr, ",", &keyString);

    if (strcmp(keyHeader, SPEECH_PARSER_SET_KEY_PREFIX) != 0) {
        ALOGE("%s(), Wrong Header: %s, expect:%s", __FUNCTION__, keyHeader, SPEECH_PARSER_SET_KEY_PREFIX);
        return -EINVAL;
    }

    AudioParameter param = AudioParameter(String8(keyString));

    ALOGV("-%s(): %s", __FUNCTION__, keyValuePair->stringAddr);
    return 0;
}

/**
 * =========================================================================
 *  @brief get keyString string from library
 *
 *  @param keyString there is only "key" when input,
           and then library need rewrite "key=value" to keyString
 *  @param sizeKeyString the size byte of string
 *
 *  @return int
 * =========================================================================
 */

int SpeechParserGen93::getKeyValuePair(SpeechStringBufType *keyValuePair) {
    ALOGV("%s(), keyString:%s", __FUNCTION__, keyValuePair->stringAddr);

    char *keyHeader = NULL;
    char *keyString = NULL;
    keyHeader = strtok_r(keyValuePair->stringAddr, ",", &keyString);

    if (keyHeader == NULL || keyString == NULL) {
        ALOGE("%s(), NULL value!! keyString:%s", __FUNCTION__, keyValuePair->stringAddr);
        return -EINVAL;
    }

    if (strcmp(keyHeader, SPEECH_PARSER_GET_KEY_PREFIX) != 0) {
        ALOGE("%s(), Wrong Header: %s, expect:%s", __FUNCTION__, keyHeader, SPEECH_PARSER_GET_KEY_PREFIX);
        return -EINVAL;
    }
    char keyValueString[MAX_SPEECH_PARSER_KEY_LEN];
    memset((void *)keyValueString, 0, MAX_SPEECH_PARSER_KEY_LEN);
    if (strcmp(keyString, SPEECH_PARSER_PARAMBUF_SIZE) == 0) {
        sprintf(keyValueString, "%d", kSphParamSize);
    }
    keyValuePair->stringAddr = keyValueString;
    ALOGD("%s(),key:%s  , return keyValue:%s", __FUNCTION__, keyString, keyValuePair->stringAddr);
    return 0;
}
/**
 * =========================================================================
 *  @brief update phone call status from driver
 *
 *  @param callOn: the phone call status: true(On), false(Off)
 *
 *  @return int
 * =========================================================================
 */
int SpeechParserGen93::updatePhoneCallStatus(bool callOn) {
    ALOGV("%s(), callOn:%d", __FUNCTION__, callOn);
    if (callOn == false) {
        AUDIO_FREE_POINTER(mParamBuf);
    }
    if (mCallOn == callOn) {
        ALOGW("%s(), callOn(%d) == mCallOn(%d), return",
              __FUNCTION__, callOn, mCallOn);
        return 0;
    }
    mCallOn = callOn;
    return 0;
}

uint32_t SpeechParserGen93::getMaxBufferSize() {
    uint32_t paramBufSize = 0;
    char keyString[MAX_SPEECH_PARSER_KEY_LEN];
    memset((void *)keyString, 0, MAX_SPEECH_PARSER_KEY_LEN);

    SpeechStringBufType keyValuePair;
    memset(&keyValuePair, 0, sizeof(SpeechStringBufType));
    keyValuePair.memorySize = strlen(keyString) + 1;
    keyValuePair.stringSize = strlen(keyString);
    keyValuePair.stringAddr = keyString;

    sprintf(keyString, "%s,%s", SPEECH_PARSER_GET_KEY_PREFIX, SPEECH_PARSER_PARAMBUF_SIZE);//"SPEECH_PARSER_GET_PARAM,PARAMBUF_SIZE"
    //get from default parser
    getKeyValuePair(&keyValuePair);
    paramBufSize += (uint32_t) atoi(keyValuePair.stringAddr);

    //get from customized parser
#if defined(CUSTOMIZED_PARSER)
    SpOps *spOps = spOpsGetInstance();
    SpHandle *spHandle = spOps->spHandleGetInstance();
    spOps->getKeyValuePair(spHandle, &keyValuePair);
    paramBufSize += (uint32_t) atoi(keyValuePair.stringAddr);
#endif

    ALOGV("%s() paramBufSize:%d", __FUNCTION__, paramBufSize);
    return paramBufSize;

}

int SpeechParserGen93::getSpeechProfile(const SpeechParserAttribute speechParserAttribute) {
    speech_profile_t idxSphProfile;

    if (getFeatureOn(SPEECH_FEATURE_LOOPBACK)) {
        switch (speechParserAttribute.outputDevice) {
        case AUDIO_DEVICE_OUT_SPEAKER:
            idxSphProfile = SPEECH_PROFILE_LPBK_HANDSFREE;
            break;
        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
        case AUDIO_DEVICE_OUT_WIRED_HEADSET:
            idxSphProfile = SPEECH_PROFILE_LPBK_HEADSET;
            break;
        default:
            idxSphProfile = SPEECH_PROFILE_LPBK_HANDSET;
            break;
        }
    } else if (audio_is_bluetooth_sco_device(speechParserAttribute.outputDevice)) {
        if (getFeatureOn(SPEECH_FEATURE_BTNREC)) {

            idxSphProfile = SPEECH_PROFILE_BT_EARPHONE;
        } else {

            idxSphProfile = SPEECH_PROFILE_BT_NREC_OFF;
        }
    } else {

        switch (speechParserAttribute.outputDevice) {
        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
            idxSphProfile = SPEECH_PROFILE_3_POLE_HEADSET;
            break;
        case AUDIO_DEVICE_OUT_WIRED_HEADSET:
            idxSphProfile = SPEECH_PROFILE_4_POLE_HEADSET;
            break;

        case AUDIO_DEVICE_OUT_SPEAKER:
#if defined(MTK_INCALL_HANDSFREE_DMNR)
            idxSphProfile = SPEECH_PROFILE_MAGICONFERENCE;
#else
            if (speechParserAttribute.ttyMode == AUD_TTY_OFF) {
                if (getFeatureOn(SPEECH_FEATURE_SUPERVOLUME)) {
                    idxSphProfile = SPEECH_PROFILE_HANDSFREE_SV;
                } else {
                    idxSphProfile = SPEECH_PROFILE_HANDSFREE;
                }
            } else {
                switch (speechParserAttribute.ttyMode) {
                case AUD_TTY_HCO:
                    idxSphProfile = SPEECH_PROFILE_TTY_HCO_HANDSFREE;
                    break;
                case AUD_TTY_VCO:
                    idxSphProfile = SPEECH_PROFILE_TTY_VCO_HANDSFREE;
                    break;
                default:
                    idxSphProfile = SPEECH_PROFILE_TTY_HCO_HANDSFREE;
                    break;
                }
            }
#endif
            break;

        case AUDIO_DEVICE_OUT_USB_DEVICE:
            idxSphProfile = SPEECH_PROFILE_USB_HEADSET;
            break;

        default:
            if (getFeatureOn(SPEECH_FEATURE_HAC)) {
                idxSphProfile = SPEECH_PROFILE_HAC;

            } else {
                if (speechParserAttribute.ttyMode == AUD_TTY_OFF) {
                    if (getFeatureOn(SPEECH_FEATURE_SUPERVOLUME)) {
                        idxSphProfile = SPEECH_PROFILE_HANDSET_SV;
                    } else {
                        idxSphProfile = SPEECH_PROFILE_HANDSET;
                    }
                } else {
                    switch (speechParserAttribute.ttyMode) {
                    case AUD_TTY_HCO:
                        idxSphProfile = SPEECH_PROFILE_TTY_HCO_HANDSET;
                        break;
                    case AUD_TTY_VCO:
                        idxSphProfile = SPEECH_PROFILE_TTY_VCO_HANDSET;
                        break;
                    default:
                        idxSphProfile = SPEECH_PROFILE_TTY_HCO_HANDSET;
                        break;
                    }
                }
                break;
            }
        }
    }
    ALOGV("%s(), idxSphProfile = %d", __FUNCTION__, idxSphProfile);
    return idxSphProfile;
}

/*==============================================================================
 *                     Original SpeechParserGen93 Imeplementation
 *============================================================================*/

void SpeechParserGen93::initAppParser() {
    ALOGV("+%s()", __FUNCTION__);
    /* Init AppHandle */
    ALOGV("%s() appHandleGetInstance", __FUNCTION__);
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
        return;
    }
    mAppHandle = appOps->appHandleGetInstance();
    ALOGD("%s() appHandleRegXmlChangedCb", __FUNCTION__);

}

status_t SpeechParserGen93::speechDataDump(char *bufDump,
                                           uint16_t idxSphType,
                                           const char *nameParam,
                                           const char *speechParamData) {
    if (nameParam == NULL) {
        return NO_ERROR;
    }
    // Speech Log system property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_SPEECHLOG_ON, property_value, "0"); //"0": default off
    if (property_value[0] == '0') {
#if !defined(CONFIG_MT_ENG_BUILD) // user or user debug load
        return NO_ERROR;
#endif
    }

    ALOGV("+%s(), idxSphType=%d", __FUNCTION__, idxSphType);
    char sphDumpStr[SPH_DUMP_STR_SIZE] = {0};
    int idxDump = 0, sizeDump = 0, DataTypePrint = 0;
    //speech parameter dump

    switch (idxSphType) {
    case AUDIO_TYPE_SPEECH: {
        if (strcmp(nameParam, "speech_mode_para") == 0) {
            sizeDump = 16;
        } else if (strcmp(nameParam, "sph_in_fir") == 0) {
            sizeDump = 5;
        } else if (strcmp(nameParam, "sph_out_fir") == 0) {
            sizeDump = 5;
        } else if (strcmp(nameParam, "sph_in_iir_mic1_dsp") == 0) {
            sizeDump = 5;
        } else if (strcmp(nameParam, "sph_in_iir_mic2_dsp") == 0) {
            sizeDump = 5;
        } else if (strcmp(nameParam, "sph_in_iir_enh_dsp") == 0) {
            sizeDump = 5;
        } else if (strcmp(nameParam, "sph_out_iir_enh_dsp") == 0) {
            sizeDump = 5;
        }
        break;
    }
    case AUDIO_TYPE_SPEECH_GENERAL: {
        if (strcmp(nameParam, "speech_common_para") == 0) {
            sizeDump = 12;
        } else if (strcmp(nameParam, "debug_info") == 0) {
            sizeDump = 8;
        }
        break;
    }
    case AUDIO_TYPE_SPEECH_NETWORK: {
        if (strcmp(nameParam, "speech_network_support") == 0) {
            DataTypePrint = 1;
            sizeDump = 1;
        }
        break;
    }
    case AUDIO_TYPE_SPEECH_ECHOREF: {
        if (strcmp(nameParam, "USBAudio") == 0) {
            sizeDump = 3;
        }
        break;
    }

    }
    snprintf(sphDumpStr, SPH_DUMP_STR_SIZE, "%s[%d]=", nameParam, sizeDump);

    for (idxDump = 0; idxDump < sizeDump; idxDump++) {
        char sphDumpTemp[100] = {0};
        if (DataTypePrint == 1) {
            snprintf(sphDumpTemp, 100, "[%d]0x%x,", idxDump, *((uint16_t *)speechParamData + idxDump));
        } else {
            snprintf(sphDumpTemp, 100, "[%d]%d,", idxDump, *((uint16_t *)speechParamData + idxDump));
        }
        audio_strncat(sphDumpStr, sphDumpTemp, SPH_DUMP_STR_SIZE);
    }

    if (idxDump != 0 && bufDump != NULL) {
        audio_strncat(bufDump, sphDumpStr, SPH_DUMP_STR_SIZE);
    }
    return NO_ERROR;
}

status_t SpeechParserGen93::getSpeechParamFromAppParser(uint16_t idxSphType,
                                                        AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT *paramLayerInfo,
                                                        char *bufParamUnit,
                                                        uint16_t *sizeByteTotal) {
    ALOGV("+%s(), paramLayerInfo->numCategoryType=0x%x", __FUNCTION__, paramLayerInfo->numCategoryType);

    if (mAppHandle == NULL) {
        ALOGE("%s() mAppHandle == NULL, Assert!!!", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }

    char *categoryPath = NULL;
    ParamUnit *paramUnit = NULL;
    uint16_t  sizeByteParam = 0, idxCount;
    uint16_t bufSize = (idxSphType == AUDIO_TYPE_SPEECH_NETWORK) ? MAX_BYTE_PARAM_SPEECH_NETWORK : MAX_BYTE_PARAM_SPEECH;
    Param  *SpeechParam;
    UT_string *uts_categoryPath = NULL;

    /* If user select a category path, just like "NarrowBand / Normal of Handset / Level0" */
    utstring_new(uts_categoryPath);

    ALOGV("%s(), categoryType.size=%zu, paramName.size=%zu", __FUNCTION__, paramLayerInfo->categoryType.size(), paramLayerInfo->paramName.size());
    for (idxCount = 0; idxCount < paramLayerInfo->categoryType.size() ; idxCount++) {
        ALOGV("%s(), categoryType[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo->categoryType.at(idxCount).string());
    }
    for (idxCount = 0; idxCount < paramLayerInfo->categoryName.size() ; idxCount++) {
        ALOGV("%s(), categoryName[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo->categoryName.at(idxCount).string());
    }


    for (idxCount = 0; idxCount < paramLayerInfo->numCategoryType ; idxCount++) {
        if (idxCount == paramLayerInfo->numCategoryType - 1) {
            //last time concat
            utstring_printf(uts_categoryPath, "%s,%s", (char *)(paramLayerInfo->categoryType.at(idxCount).string()), (char *)(paramLayerInfo->categoryName.at(idxCount).string()));
        } else {
            utstring_printf(uts_categoryPath, "%s,%s,", (char *)(paramLayerInfo->categoryType.at(idxCount).string()), (char *)(paramLayerInfo->categoryName.at(idxCount).string()));
        }
    }
    categoryPath = strdup(utstring_body(uts_categoryPath));
    utstring_free(uts_categoryPath);

    ALOGV("%s() audioTypeName=%s", __FUNCTION__, paramLayerInfo->audioTypeName);
    /* Query AudioType */
    AppOps *appOps = appOpsGetInstance();
    AudioType *audioType = NULL;
    if (appOps == NULL) {
        free(categoryPath);
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    } else {
        audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, paramLayerInfo->audioTypeName);
    }
    if (!audioType) {
        free(categoryPath);
        ALOGE("%s() can't find audioTypeName=%s, Assert!!!", __FUNCTION__, paramLayerInfo->audioTypeName);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }

    /* Query the ParamUnit */
    appOps->audioTypeReadLock(audioType, __FUNCTION__);
    paramUnit = appOps->audioTypeGetParamUnit(audioType, categoryPath);
    if (!paramUnit) {
        appOps->audioTypeUnlock(audioType);
        ALOGE("%s() can't find paramUnit, Assert!!! audioType=%s, categoryPath=%s", __FUNCTION__, audioType->name, categoryPath);
        free(categoryPath);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }

    char sphLogTemp[SPH_DUMP_STR_SIZE] = {0};
    snprintf(sphLogTemp, SPH_DUMP_STR_SIZE, "(path=%s,id=%d),", categoryPath, paramUnit->paramId);
    audio_strncat(paramLayerInfo->logPrintParamUnit, sphLogTemp, SPH_DUMP_STR_SIZE);

    //for speech param dump
    char *bufParamDump = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
    memset(bufParamDump, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

    for (idxCount = 0; idxCount < (*paramLayerInfo).numParam ; idxCount++) {

        SpeechParam = appOps->paramUnitGetParamByName(paramUnit, (const char *)paramLayerInfo->paramName.at(idxCount).string());
        if (SpeechParam) {
            sizeByteParam = appOps->paramGetNumOfBytes(SpeechParam);
            if (*sizeByteTotal + sizeByteParam > bufSize) {
                ALOGE("%s(), bufParamUnit overflow!! max:%d, total use:%d", __FUNCTION__, bufSize, (int)(*sizeByteTotal + sizeByteParam));
                if (bufParamDump != NULL) {
                    if (bufParamDump[0] != 0) {
                        ALOGD("%s(),dump: %s", __FUNCTION__, bufParamDump);
                    }
                    delete[] bufParamDump;
                }
                appOps->audioTypeUnlock(audioType);
                free(categoryPath);
                return -ENOMEM;
            } else {
                memcpy(bufParamUnit + *sizeByteTotal, SpeechParam->data, sizeByteParam);
                *sizeByteTotal += sizeByteParam;
            }
            ALOGV("%s() paramName=%s, sizeByteParam=%d",
                  __FUNCTION__, paramLayerInfo->paramName.at(idxCount).string(), sizeByteParam);
            //speech parameter dump
            speechDataDump(bufParamDump, idxSphType, (const char *)paramLayerInfo->paramName.at(idxCount).string(), (const char *)SpeechParam->data);
        }
    }

    if (bufParamDump != NULL) {
        if (bufParamDump[0] != 0) {
            ALOGD("%s(),dump: %s", __FUNCTION__, bufParamDump);
        }
        delete[] bufParamDump;
    }

    appOps->audioTypeUnlock(audioType);
    free(categoryPath);

    return NO_ERROR;
}

status_t SpeechParserGen93::setMDParamUnitHdr(speech_type_dynamic_param_t idxAudioType,
                                              SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT *paramUnitHdr,
                                              uint16_t configValue) {
    switch (idxAudioType) {
    case AUDIO_TYPE_SPEECH:
        paramUnitHdr->sphUnitMagiNum = 0xAA01;
        paramUnitHdr->sphParserVer = 1;
        paramUnitHdr->numLayer = 0x2;
        paramUnitHdr->paramHeader[0] = 0x1F;//all network use, while modem not check it
        //Network: bit0: GSM, bit1: WCDMA,.bit2: CDMA, bit3: VoLTE, bit4:C2K
        if (mSphParamInfo->isSingleBandTransfer) {
            switch (configValue) {
            case 0:
                paramUnitHdr->paramHeader[1] = 0x1;//voice band:NB
                break;
            case 1:
                paramUnitHdr->paramHeader[1] = 0x2;//voice band:WB
                break;

            default:
                paramUnitHdr->paramHeader[1] = 0x1;//voice band:NB
                break;
            }
        } else {
            switch (configValue) {
            case 1:
                paramUnitHdr->paramHeader[1] = 0x1;//voice band:NB
                break;
            case 2:
                paramUnitHdr->paramHeader[1] = 0x3;//voice band:NB,WB
                break;
            case 3:
                paramUnitHdr->paramHeader[1] = 0x7;//voice band:NB,WB,SWB
                break;
            case 4:
                paramUnitHdr->paramHeader[1] = 0xF;//voice band:NB,WB,SWB,FB
                break;
            default:
                paramUnitHdr->paramHeader[1] = 0x3;//voice band:NB,WB
                break;
            }
        }
        paramUnitHdr->paramHeader[2] = (mSpeechParamVerFirst << 4) + mSpeechParamVerLast;
        ALOGV("%s(), sphUnitMagiNum = 0x%x, SPH_PARAM_VERSION(0x%x)",
              __FUNCTION__, paramUnitHdr->sphUnitMagiNum, paramUnitHdr->paramHeader[2]);
        break;
    case AUDIO_TYPE_SPEECH_DMNR:
        paramUnitHdr->sphUnitMagiNum = 0xAA03;
        paramUnitHdr->sphParserVer = 1;
        paramUnitHdr->numLayer = 0x2;
        paramUnitHdr->paramHeader[0] = 0x3;//OutputDeviceType
        switch (configValue) {
        case 1:
            paramUnitHdr->paramHeader[1] = 0x1;//voice band:NB
            break;
        case 2:
            paramUnitHdr->paramHeader[1] = 0x3;//voice band:NB,WB
            break;
        case 3:
            paramUnitHdr->paramHeader[1] = 0x7;//voice band:NB,WB,SWB
            break;
        case 4:
            paramUnitHdr->paramHeader[1] = 0xF;//voice band:NB,WB,SWB,FB
            break;
        default:
            paramUnitHdr->paramHeader[1] = 0x3;//voice band:NB,WB
            break;
        }
        paramUnitHdr->paramHeader[2] = (mSpeechParamVerFirst << 4) + mSpeechParamVerLast;
        ALOGV("%s(), sphUnitMagiNum = 0x%x, Version = 0x%x",
              __FUNCTION__, paramUnitHdr->sphUnitMagiNum, paramUnitHdr->paramHeader[2]);
        break;

    default:
        break;
    }

    // Speech Log system property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_SPEECHLOG_ON, property_value, "0"); //"0": default off
    if (property_value[0] == '0') {
        return NO_ERROR;
    } else {
        char sphDumpStr[SPH_DUMP_STR_SIZE] = "MDParamUnitHdr ";
        int idxDump = 0;
        for (idxDump = 0; idxDump < (int)(sizeof(paramUnitHdr) >> 1); idxDump++) { //uint16_t
            char sphDumpTemp[100] = {0};
            snprintf(sphDumpTemp, 100, "[%d]0x%x, ", idxDump, *((uint16_t *)&paramUnitHdr + idxDump));
            audio_strncat(sphDumpStr, sphDumpTemp, SPH_DUMP_STR_SIZE);
        }
        if (idxDump != 0) {
            ALOGD("%s(), %s", __FUNCTION__,  sphDumpStr);
        }
    }
    return NO_ERROR;
}

uint16_t SpeechParserGen93::setMDParamDataHdr(SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT paramUnitHdr,
                                              const char *cateBandName, const char *cateNetworkName) {
    uint16_t idxCount = 0;
    uint16_t dataHeader = 0, MaskNetwork = 0;
    bool bNetworkMatch = false;

    if (cateBandName != NULL) {
        if (strcmp(cateBandName, "NB") == 0) { //All netwrok use
            dataHeader = 0x1000;
        } else if (strcmp(cateBandName, "WB") == 0) {
            dataHeader = 0x2000;
        } else if (strcmp(cateBandName, "SWB") == 0) {
            dataHeader = 0x3000;
        } else if (strcmp(cateBandName, "FB") == 0) {
            dataHeader = 0x4000;
        }
    } else {
        dataHeader = 0x1000;
    }
    //search matched network
    if (cateNetworkName != NULL) {
        for (idxCount = 0; idxCount < mNumSpeechNetwork ; idxCount++) {
            ALOGV("%s(), cateNetwork= %s, mListSpeechNetwork[%d]=%s",
                  __FUNCTION__, cateNetworkName, idxCount, mListSpeechNetwork[idxCount].name);
            if (strcmp(cateNetworkName, mListSpeechNetwork[idxCount].name) == 0) {
                MaskNetwork = mListSpeechNetwork[idxCount].supportBit;
                ALOGV("%s(), cateNetwork= %s, mListSpeechNetwork[%d]=%s, MaskNetwork=0x%x",
                      __FUNCTION__, cateNetworkName, idxCount, mListSpeechNetwork[idxCount].name, MaskNetwork);
                bNetworkMatch = true;
                break;
            }
        }
        if (!bNetworkMatch) {
            ALOGE("%s(), cateNetwork= %s, mListSpeechNetwork[%d]=%s, bNetworkMatch=%d, NO match!!!",
                  __FUNCTION__, cateNetworkName, idxCount, mListSpeechNetwork[idxCount].name, bNetworkMatch);
        }
    }
    if (!mSphParamSupport->isNetworkSupport) {
        dataHeader = dataHeader >> 8;
        MaskNetwork = 0xF;
    }
    dataHeader |= MaskNetwork;
    ALOGV("-%s(), sphUnitMagiNum=0x%x, dataHeader=0x%x, MaskNetwork=0x%x, cateBand=%s",
          __FUNCTION__, paramUnitHdr.sphUnitMagiNum, dataHeader, MaskNetwork, cateBandName);

    return dataHeader;
}

int SpeechParserGen93::initSpeechNetwork(void) {
    uint16_t size = 0, idxCount, sizeByteFromApp = 0;
    char *packedParamUnitFromApp = new char [MAX_BYTE_PARAM_SPEECH_NETWORK];
    memset(packedParamUnitFromApp, 0, MAX_BYTE_PARAM_SPEECH_NETWORK);

    AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT paramLayerInfo;

    //-------------
    paramLayerInfo.audioTypeName = (char *) audioTypeNameList[AUDIO_TYPE_SPEECH_NETWORK];

    if (mAppHandle == NULL) {
        ALOGE("%s() mAppHandle == NULL, Assert!!!", __FUNCTION__);
        ASSERT(0);
    }

    /* Query AudioType */
    AppOps *appOps = appOpsGetInstance();
    AudioType *audioType = NULL;
    if (appOps != NULL) {
        audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, paramLayerInfo.audioTypeName);
        paramLayerInfo.numCategoryType = appOps->audioTypeGetNumOfCategoryType(audioType);//1

        paramLayerInfo.numParam = MAX_NUM_PARAM_SPEECH_NETWORK;//4

        paramLayerInfo.categoryType.assign(audioType_SpeechNetwork_CategoryType, audioType_SpeechNetwork_CategoryType + paramLayerInfo.numCategoryType);
        paramLayerInfo.paramName.assign(audioType_SpeechNetwork_ParamName, audioType_SpeechNetwork_ParamName + paramLayerInfo.numParam);
        paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
        memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

        ALOGV("%s(), categoryType.size=%zu, paramName.size=%zu", __FUNCTION__, paramLayerInfo.categoryType.size(), paramLayerInfo.paramName.size());
        for (idxCount = 0; idxCount < paramLayerInfo.categoryType.size() ; idxCount++) {
            ALOGV("%s(), categoryType[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.categoryType.at(idxCount).string());
        }
        for (idxCount = 0; idxCount < paramLayerInfo.paramName.size() ; idxCount++) {
            ALOGV("%s(), paramName[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.paramName.at(idxCount).string());
        }
        //-----------
        //parse layer
        CategoryType *categoryNetwork = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_SpeechNetwork_CategoryType[0].string());
        mNumSpeechNetwork = appOps->categoryTypeGetNumOfCategory(categoryNetwork);

        //parse network
        for (int i = 0; i < mNumSpeechNetwork; i++) {
            Category *CateNetwork = appOps->categoryTypeGetCategoryByIndex(categoryNetwork, i);
            sizeByteFromApp = 0;
            //clear
            while (!paramLayerInfo.categoryName.empty()) {
                paramLayerInfo.categoryName.pop_back();
            }
            audio_strncpy(mListSpeechNetwork[i].name, CateNetwork->name, 128);

            paramLayerInfo.categoryName.push_back(String8(CateNetwork->name));//Network

            getSpeechParamFromAppParser(AUDIO_TYPE_SPEECH_NETWORK, &paramLayerInfo, packedParamUnitFromApp, &sizeByteFromApp);
            mListSpeechNetwork[i].supportBit = *((uint16_t *)packedParamUnitFromApp);
            size += sizeByteFromApp;

            ALOGV("%s(), i=%d, sizeByteFromApp=%d, supportBit=0x%x",
                  __FUNCTION__, i, sizeByteFromApp, mListSpeechNetwork[i].supportBit);
        }
        ALOGV("-%s(), total size byte=%d", __FUNCTION__, size);
    } else {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
    }
    //init the Name mapping table  for each SpeechNetwork
    bool IsNetworkFound = false;
    for (int bitIndex = 0; bitIndex < 12; bitIndex++) {
        IsNetworkFound = false;
        for (int NetworkIndex = 0; NetworkIndex < mNumSpeechNetwork; NetworkIndex++) {
            if (((mListSpeechNetwork[NetworkIndex].supportBit >> bitIndex) & 1) == 1) {
                audio_strncpy(mNameForEachSpeechNetwork[bitIndex].name, mListSpeechNetwork[NetworkIndex].name, 128);
                IsNetworkFound = true;
                break;
            }
        }
        if (!IsNetworkFound) {
            audio_strncpy(mNameForEachSpeechNetwork[bitIndex].name, mListSpeechNetwork[0].name, 128);
        }
        ALOGV("%s(), mNameForEachSpeechNetwork[%d].name = %s",
              __FUNCTION__, bitIndex, mNameForEachSpeechNetwork[bitIndex].name);
    }
    if (packedParamUnitFromApp != NULL) {
        delete[] packedParamUnitFromApp;
    }
    if (paramLayerInfo.logPrintParamUnit != NULL) {
        delete[] paramLayerInfo.logPrintParamUnit;
    }
    return size;
}


int SpeechParserGen93::getSpeechParamUnit(char *bufParamUnit) {
    uint16_t size = 0, idxCount, sizeByteFromApp = 0;
    uint16_t dataHeader, idxInfo = 0, idxTmp = 0, numBand = 0, numNetwork = 0, numVolume = 0;
    SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT headerParamUnit;
    int idxProfile = 0, parserOption = 0;

    idxProfile = getSpeechProfile(mSpeechParserAttribute);
    int idxVolume = mSpeechParserAttribute.idxVolume;
    bool btHeadsetNrecOn = getFeatureOn(SPEECH_FEATURE_BTNREC);
    mSphParamInfo->isBtNrecOn = btHeadsetNrecOn;
    mSphParamInfo->idxVolume = idxVolume;

    speech_mode_t sphMode = getSpeechModeByOutputDevice(mSpeechParserAttribute.outputDevice);

    /* 93MD unused */
    parserOption = 0;
    //bit 0: dv profile, bit 1: single band, bit 4~7: band number
    mSphParamInfo->isSingleBandTransfer = (bool)(parserOption & 0x2);
    mSphParamInfo->idxVoiceBandStart = (unsigned char)((parserOption & 0xf0) >> 4);
    //-----------------

    ALOGD("+%s(), Volume=0x%x, BtNrecOn=0x%x, Profile=%d, parserOption=0x%x",
          __FUNCTION__, idxVolume, btHeadsetNrecOn, idxProfile, parserOption);

    AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT paramLayerInfo;
    //-------------
    paramLayerInfo.audioTypeName = (char *) audioTypeNameList[AUDIO_TYPE_SPEECH];

    if (mAppHandle == NULL) {
        ALOGE("%s() mAppHandle == NULL, Assert!!!", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }

    /* Query AudioType */
    AppOps *appOps = appOpsGetInstance();
    AudioType *audioType = NULL;
    if (appOps == NULL) {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    } else {
        audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, paramLayerInfo.audioTypeName);
    }

    char *packedParamUnitFromApp = new char [MAX_BYTE_PARAM_SPEECH];
    memset(packedParamUnitFromApp, 0, MAX_BYTE_PARAM_SPEECH);

    paramLayerInfo.numCategoryType = appOps->audioTypeGetNumOfCategoryType(audioType);
    paramLayerInfo.numParam = mNumSpeechParam;//4

    paramLayerInfo.categoryType.assign(audioType_Speech_CategoryType, audioType_Speech_CategoryType + paramLayerInfo.numCategoryType);
    paramLayerInfo.paramName.assign(audioType_Speech_ParamName, audioType_Speech_ParamName + paramLayerInfo.numParam);

    ALOGV("%s(), categoryType.size=%zu, paramName.size=%zu",
          __FUNCTION__, paramLayerInfo.categoryType.size(), paramLayerInfo.paramName.size());
    for (idxCount = 0; idxCount < paramLayerInfo.categoryType.size() ; idxCount++) {
        ALOGV("%s(), categoryType[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.categoryType.at(idxCount).string());
    }
    for (idxCount = 0; idxCount < paramLayerInfo.paramName.size() ; idxCount++) {
        ALOGV("%s(), paramName[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.paramName.at(idxCount).string());
    }
    //-----------

    //parse layer
    CategoryType *categoryNetwork = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_Speech_CategoryType[3].string());
    CategoryType *categoryBand = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_Speech_CategoryType[0].string());
    numNetwork = appOps->categoryTypeGetNumOfCategory(categoryNetwork);
    numBand = appOps->categoryTypeGetNumOfCategory(categoryBand);

    CategoryType *categoryVolume = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_Speech_CategoryType[2].string());
    CategoryGroup *categoryGroupVolume = appOps->categoryTypeGetCategoryGroupByIndex(categoryVolume, 0);
    numVolume = appOps->categoryGroupGetNumOfCategory(categoryGroupVolume);
    idxTmp = (numBand & 0xF) << 4;
    headerParamUnit.numEachLayer = idxTmp + (numNetwork & 0xF);
    ALOGV("%s(), sphUnitMagiNum= 0x%x, numEachLayer=0x%x", __FUNCTION__, headerParamUnit.sphUnitMagiNum, headerParamUnit.numEachLayer);
    if (mSphParamInfo->isSingleBandTransfer) {
        setMDParamUnitHdr(AUDIO_TYPE_SPEECH, &headerParamUnit, mSphParamInfo->idxVoiceBandStart);
    } else {
        setMDParamUnitHdr(AUDIO_TYPE_SPEECH, &headerParamUnit, numBand);
    }
    ALOGV("%s(), sphUnitMagiNum= 0x%x, numEachLayer=0x%x", __FUNCTION__, headerParamUnit.sphUnitMagiNum, headerParamUnit.numEachLayer);
    ALOGV("%s(), categoryNetwork= %s, categoryBand = %s, categoryVolume = %s",
          __FUNCTION__, categoryNetwork->name, categoryBand->name, categoryVolume->name);
    ALOGV("%s(), numNetwork= %d, numBand = %d, numVolume = %d", __FUNCTION__, numNetwork, numBand, numVolume);

    if (size + sizeof(headerParamUnit) > kSphParamSize) {
        ALOGE("%s(), headerParamUnit overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, (int)(size + sizeof(headerParamUnit)));
        if (packedParamUnitFromApp != NULL) {
            delete[] packedParamUnitFromApp;
        }
        return -ENOMEM;
    } else {
        memcpy(bufParamUnit + size, &headerParamUnit, sizeof(headerParamUnit));
        size += sizeof(headerParamUnit);
    }

    idxInfo = sphMode & 0xF;
    ALOGV("%s(), add mode idxInfo=0x%x", __FUNCTION__, idxInfo);
    idxTmp = idxVolume << 4;
    idxInfo += idxTmp;
    ALOGV("%s(), add volume<<4 idxInfo=0x%x, idxTmp=0x%x", __FUNCTION__, idxInfo, idxTmp);
    paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
    memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

    if (size + sizeof(idxInfo) > kSphParamSize) {
        ALOGE("%s(), idxInfo overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, (int)(size + sizeof(idxInfo)));
        if (packedParamUnitFromApp != NULL) {
            delete[] packedParamUnitFromApp;
        }
        if (paramLayerInfo.logPrintParamUnit != NULL) {
            delete[] paramLayerInfo.logPrintParamUnit;
        }
        return -ENOMEM;
    } else {
        memcpy(bufParamUnit + size, &idxInfo, sizeof(idxInfo));
        size += sizeof(idxInfo);
    }
    //parse network
    for (int idxNetwork = 0; idxNetwork < numNetwork; idxNetwork++) {
        Category *CateNetwork =  appOps->categoryTypeGetCategoryByIndex(categoryNetwork, idxNetwork);
        //parse band
        for (int idxBand = mSphParamInfo->idxVoiceBandStart; idxBand < mSphParamInfo->idxVoiceBandStart + numBand; idxBand++) {
            sizeByteFromApp = 0;
            Category *CateBand =  appOps->categoryTypeGetCategoryByIndex(categoryBand, idxBand);

            dataHeader = setMDParamDataHdr(headerParamUnit, CateBand->name, CateNetwork->name);
            if (size + sizeof(dataHeader) > kSphParamSize) {
                ALOGE("%s(), dataHeader overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, (int)(size + sizeof(dataHeader)));
                if (packedParamUnitFromApp != NULL) {
                    delete[] packedParamUnitFromApp;
                }
                if (paramLayerInfo.logPrintParamUnit != NULL) {
                    delete[] paramLayerInfo.logPrintParamUnit;
                }
                return -ENOMEM;
            } else {
                memcpy(bufParamUnit + size, &dataHeader, sizeof(dataHeader));
                size += sizeof(dataHeader);
            }
            while (!paramLayerInfo.categoryName.empty()) {
                paramLayerInfo.categoryName.pop_back();
            }
            //Band
            paramLayerInfo.categoryName.push_back(String8(CateBand->name));//Band
            //Profile
            paramLayerInfo.categoryName.push_back(String8(audioType_Speech_CategoryName2[idxProfile]));
            //Volume
            if (idxVolume > 6 || idxVolume < 0) {
                paramLayerInfo.categoryName.push_back(String8(audioType_Speech_CategoryName3[3]));//volume
                ALOGE("%s(), Invalid IdxVolume=0x%x, use 3 !!!", __FUNCTION__, idxVolume);
            } else {
                paramLayerInfo.categoryName.push_back(String8(audioType_Speech_CategoryName3[idxVolume]));
            }
            paramLayerInfo.categoryName.push_back(String8(CateNetwork->name));//Network

            for (idxCount = 0; idxCount < paramLayerInfo.categoryName.size() ; idxCount++) {
                ALOGV("%s(), categoryName[%d]= %s",
                      __FUNCTION__, idxCount, paramLayerInfo.categoryName.at(idxCount).string());
            }

            getSpeechParamFromAppParser(AUDIO_TYPE_SPEECH, &paramLayerInfo, packedParamUnitFromApp, &sizeByteFromApp);
            if (size + sizeByteFromApp > kSphParamSize) {
                ALOGE("%s(), packedParamUnitFromApp overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, size + sizeByteFromApp);
                if (packedParamUnitFromApp != NULL) {
                    delete[] packedParamUnitFromApp;
                }
                if (paramLayerInfo.logPrintParamUnit != NULL) {
                    delete[] paramLayerInfo.logPrintParamUnit;
                }
                return -ENOMEM;
            } else {
                memcpy(bufParamUnit + size, packedParamUnitFromApp, sizeByteFromApp);
                size += sizeByteFromApp;
            }

            char sphLogTemp[SPH_DUMP_STR_SIZE] = {0};
            snprintf(sphLogTemp, SPH_DUMP_STR_SIZE, "size(b)=%d; total size(b)=%d", sizeByteFromApp, size);
            audio_strncat(paramLayerInfo.logPrintParamUnit, sphLogTemp, SPH_DUMP_STR_SIZE);
        }

        ALOGD("-%s(), MagiNum(0x%x),xml(%s),version(0x%x),%s", __FUNCTION__, headerParamUnit.sphUnitMagiNum,
              paramLayerInfo.audioTypeName, headerParamUnit.paramHeader[2], paramLayerInfo.logPrintParamUnit);
        //reset buffer pointer
        memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);
        paramLayerInfo.logPrintParamUnit[0] = '\0';
    }

    if (packedParamUnitFromApp != NULL) {
        delete[] packedParamUnitFromApp;
    }

    if (paramLayerInfo.logPrintParamUnit != NULL) {
        delete[] paramLayerInfo.logPrintParamUnit;
    }

    return size;
}

int SpeechParserGen93::getGeneralParamUnit(char *bufParamUnit) {
    ALOGV("+%s()", __FUNCTION__);
    uint16_t size = 0, idxCount = 0, idxCount2 = 0, sizeByteFromApp = 0;
    uint16_t dataHeader;
    SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT headerParamUnit;

    headerParamUnit.sphParserVer = 1;
    headerParamUnit.numLayer = 0x1;
    headerParamUnit.numEachLayer = 0x1;
    headerParamUnit.paramHeader[0] = 0x1;//Common
    headerParamUnit.sphUnitMagiNum = 0xAA02;

    if (size + sizeof(headerParamUnit) > kSphParamSize) {
        ALOGE("%s(), headerParamUnit overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, (int)(size + sizeof(headerParamUnit)));
        return -ENOMEM;
    } else {
        memcpy(bufParamUnit + size, &headerParamUnit, sizeof(headerParamUnit));
        size += sizeof(headerParamUnit);
    }

    char *packedParamUnitFromApp = new char [MAX_BYTE_PARAM_SPEECH];
    memset(packedParamUnitFromApp, 0, MAX_BYTE_PARAM_SPEECH);

    AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT paramLayerInfo;

    paramLayerInfo.audioTypeName = (char *) audioTypeNameList[AUDIO_TYPE_SPEECH_GENERAL];
    paramLayerInfo.numCategoryType = MAX_NUM_CATEGORY_TYPE_SPEECH_GENERAL;//4
    paramLayerInfo.numParam = MAX_NUM_PARAM_SPEECH_GENERAL;//4

    paramLayerInfo.categoryType.assign(audioType_SpeechGeneral_CategoryType, audioType_SpeechGeneral_CategoryType + paramLayerInfo.numCategoryType);
    paramLayerInfo.paramName.assign(audioType_SpeechGeneral_ParamName, audioType_SpeechGeneral_ParamName + paramLayerInfo.numParam);
    paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
    memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

    ALOGD("%s(), eParamUnitHdr.sphUnitMagiNum= 0x%x, categoryType.size=%zu, paramName.size=%zu",
          __FUNCTION__, headerParamUnit.sphUnitMagiNum, paramLayerInfo.categoryType.size(), paramLayerInfo.paramName.size());
    for (idxCount = 0; idxCount < paramLayerInfo.paramName.size() ; idxCount++) {
        ALOGV("%s(), paramName[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.paramName.at(idxCount).string());
    }

    dataHeader = 0x000F;

    if (size + sizeof(dataHeader) > kSphParamSize) {
        ALOGE("%s(), dataHeader overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, (int)(size + sizeof(dataHeader)));
        if (packedParamUnitFromApp != NULL) {
            delete[] packedParamUnitFromApp;
        }
        if (paramLayerInfo.logPrintParamUnit != NULL) {
            delete[] paramLayerInfo.logPrintParamUnit;
        }
        return -ENOMEM;
    } else {
        memcpy(bufParamUnit + size, &dataHeader, sizeof(dataHeader));
        size += sizeof(dataHeader);
    }

    paramLayerInfo.categoryName.push_back(String8(audioType_SpeechGeneral_CategoryName1[0]));

    getSpeechParamFromAppParser(AUDIO_TYPE_SPEECH_GENERAL, &paramLayerInfo, packedParamUnitFromApp, &sizeByteFromApp);

    if (size + sizeByteFromApp > kSphParamSize) {
        ALOGE("%s(), packedParamUnitFromApp overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, size + sizeByteFromApp);
        if (packedParamUnitFromApp != NULL) {
            delete[] packedParamUnitFromApp;
        }
        if (paramLayerInfo.logPrintParamUnit != NULL) {
            delete[] paramLayerInfo.logPrintParamUnit;
        }
        return -ENOMEM;
    } else {
        memcpy(bufParamUnit + size, packedParamUnitFromApp, sizeByteFromApp);
        size += sizeByteFromApp;
    }

    if (packedParamUnitFromApp != NULL) {
        delete[] packedParamUnitFromApp;
    }

    char sphLogTemp[SPH_DUMP_STR_SIZE] = {0};
    snprintf(sphLogTemp, SPH_DUMP_STR_SIZE, "total size(b)=%d", size);
    audio_strncat(paramLayerInfo.logPrintParamUnit, sphLogTemp, SPH_DUMP_STR_SIZE);
    ALOGD("%s(),MagiNum(0x%x),xml(%s), %s",
          __FUNCTION__, headerParamUnit.sphUnitMagiNum, paramLayerInfo.audioTypeName, paramLayerInfo.logPrintParamUnit);
    if (paramLayerInfo.logPrintParamUnit != NULL) {
        delete[] paramLayerInfo.logPrintParamUnit;
    }

    return size;
}

int SpeechParserGen93::getMagiClarityParamUnit(char *bufParamUnit) {
    ALOGV("+%s()", __FUNCTION__);
    uint16_t size = 0, idxCount = 0, idxCount2 = 0, sizeByteFromApp = 0;
    uint16_t dataHeader;
    SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT headerParamUnit;

    headerParamUnit.sphParserVer = 1;
    headerParamUnit.numLayer = 0x1;
    headerParamUnit.numEachLayer = 0x1;
    headerParamUnit.paramHeader[0] = 0x1;//Common
    headerParamUnit.sphUnitMagiNum = 0xAA04;

    if (size + sizeof(headerParamUnit) > kSphParamSize) {
        ALOGE("%s(), headerParamUnit overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, (int)(size + sizeof(headerParamUnit)));
        return -ENOMEM;
    } else {
        memcpy(bufParamUnit + size, &headerParamUnit, sizeof(headerParamUnit));
        size += sizeof(headerParamUnit);
    }

    char *packedParamUnitFromApp = new char [MAX_BYTE_PARAM_SPEECH];
    memset(packedParamUnitFromApp, 0, MAX_BYTE_PARAM_SPEECH);
    AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT paramLayerInfo;

    paramLayerInfo.audioTypeName = (char *) audioTypeNameList[AUDIO_TYPE_SPEECH_MAGICLARITY];
    paramLayerInfo.numCategoryType = MAX_NUM_CATEGORY_TYPE_SPEECH_MAGICLARITY;//4
    paramLayerInfo.numParam = MAX_NUM_PARAM_SPEECH_MAGICLARITY;//4

    paramLayerInfo.categoryType.assign(audioType_SpeechMagiClarity_CategoryType, audioType_SpeechMagiClarity_CategoryType + paramLayerInfo.numCategoryType);
    paramLayerInfo.paramName.assign(audioType_SpeechMagiClarity_ParamName, audioType_SpeechMagiClarity_ParamName + paramLayerInfo.numParam);
    paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
    memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

    for (idxCount = 0; idxCount < paramLayerInfo.paramName.size() ; idxCount++) {
        ALOGD("%s(), paramName[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.paramName.at(idxCount).string());
    }
    dataHeader = 0x000F;
    if (size + sizeof(dataHeader) > kSphParamSize) {
        ALOGE("%s(), dataHeader overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, (int)(size + sizeof(dataHeader)));
        if (packedParamUnitFromApp != NULL) {
            delete[] packedParamUnitFromApp;
        }
        if (paramLayerInfo.logPrintParamUnit != NULL) {
            delete[] paramLayerInfo.logPrintParamUnit;
        }
        return -ENOMEM;
    } else {
        memcpy(bufParamUnit + size, &dataHeader, sizeof(dataHeader));
        size += sizeof(dataHeader);
    }

    paramLayerInfo.categoryName.push_back(String8(audioType_SpeechMagiClarity_CategoryName1[0]));

    getSpeechParamFromAppParser(AUDIO_TYPE_SPEECH_MAGICLARITY, &paramLayerInfo, packedParamUnitFromApp, &sizeByteFromApp);

    if (size + sizeByteFromApp > kSphParamSize) {
        ALOGE("%s(), packedParamUnitFromApp overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, size + sizeByteFromApp);
        if (packedParamUnitFromApp != NULL) {
            delete[] packedParamUnitFromApp;
        }
        if (paramLayerInfo.logPrintParamUnit != NULL) {
            delete[] paramLayerInfo.logPrintParamUnit;
        }
        return -ENOMEM;
    } else {
        memcpy(bufParamUnit + size, packedParamUnitFromApp, sizeByteFromApp);
        size += sizeByteFromApp;
    }

    if (packedParamUnitFromApp != NULL) {
        delete[] packedParamUnitFromApp;
    }
    char sphLogTemp[SPH_DUMP_STR_SIZE] = {0};
    snprintf(sphLogTemp, SPH_DUMP_STR_SIZE, "total size(b)=%d", size);
    audio_strncat(paramLayerInfo.logPrintParamUnit, sphLogTemp, SPH_DUMP_STR_SIZE);
    ALOGD("%s(),MagiNum(0x%x),xml(%s), %s", __FUNCTION__,
          headerParamUnit.sphUnitMagiNum, paramLayerInfo.audioTypeName, paramLayerInfo.logPrintParamUnit);
    if (paramLayerInfo.logPrintParamUnit != NULL) {
        delete[] paramLayerInfo.logPrintParamUnit;
    }

    return size;
}

int SpeechParserGen93::getDmnrParamUnit(char *bufParamUnit) {
    ALOGV("+%s()", __FUNCTION__);
    uint16_t size = 0, idxBand = 0, idxProfile = 0, sizeByteFromApp = 0;
    uint16_t dataHeader, idxTmp = 0, numBand = 0, numProfile = 0;
    SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT headerParamUnit;
    AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT paramLayerInfo;

    paramLayerInfo.audioTypeName = (char *) audioTypeNameList[AUDIO_TYPE_SPEECH_DMNR];
    paramLayerInfo.numCategoryType = MAX_NUM_CATEGORY_TYPE_SPEECH_DMNR;//4
    paramLayerInfo.numParam = MAX_NUM_PARAM_SPEECH_DMNR;//4
    paramLayerInfo.categoryType.assign(audioType_SpeechDMNR_CategoryType, audioType_SpeechDMNR_CategoryType + paramLayerInfo.numCategoryType);
    paramLayerInfo.paramName.assign(audioType_SpeechDMNR_ParamName, audioType_SpeechDMNR_ParamName + paramLayerInfo.numParam);
    paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
    memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

    ALOGD("%s(), categoryType.size=%zu, paramName.size=%zu",
          __FUNCTION__, paramLayerInfo.categoryType.size(), paramLayerInfo.paramName.size());
    /* Query AudioType */
    AppOps *appOps = appOpsGetInstance();
    AudioType *audioType = NULL;
    if (appOps == NULL) {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    } else {
        audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, paramLayerInfo.audioTypeName);
    }
    char *packedParamUnitFromApp = new char [MAX_BYTE_PARAM_SPEECH];
    memset(packedParamUnitFromApp, 0, MAX_BYTE_PARAM_SPEECH);

    CategoryType *categoryBand = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_SpeechDMNR_CategoryType[0].string());
    numBand = appOps->categoryTypeGetNumOfCategory(categoryBand);
    CategoryType *categoryProfile = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_SpeechDMNR_CategoryType[1].string());
    numProfile = appOps->categoryTypeGetNumOfCategory(categoryProfile);
    idxTmp = (numBand & 0xF) << 4;
    headerParamUnit.numEachLayer = idxTmp + (numProfile & 0xF);
    setMDParamUnitHdr(AUDIO_TYPE_SPEECH_DMNR, &headerParamUnit, numBand);

    if (size + sizeof(headerParamUnit) > kSphParamSize) {
        ALOGE("%s(), headerParamUnit overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, (int)(size + sizeof(headerParamUnit)));
        if (packedParamUnitFromApp != NULL) {
            delete[] packedParamUnitFromApp;
        }
        if (paramLayerInfo.logPrintParamUnit != NULL) {
            delete[] paramLayerInfo.logPrintParamUnit;
        }
        return -ENOMEM;
    } else {
        memcpy(bufParamUnit + size, &headerParamUnit, sizeof(headerParamUnit));
        size += sizeof(headerParamUnit);
    }

    for (idxBand = 0; idxBand < numBand; idxBand++) { //NB, WB, SWB
        for (idxProfile = 0; idxProfile < numProfile; idxProfile++) {
            sizeByteFromApp = 0;
            dataHeader = ((idxBand + 1) << 4) + (idxProfile + 1);
            if (size + sizeof(dataHeader) > kSphParamSize) {
                ALOGE("%s(), dataHeader overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, (int)(size + sizeof(dataHeader)));
                if (packedParamUnitFromApp != NULL) {
                    delete[] packedParamUnitFromApp;
                }
                if (paramLayerInfo.logPrintParamUnit != NULL) {
                    delete[] paramLayerInfo.logPrintParamUnit;
                }
                return -ENOMEM;
            } else {
                memcpy(bufParamUnit + size, &dataHeader, sizeof(dataHeader));
                size += sizeof(dataHeader);
            }
            Category *CateBand =  appOps->categoryTypeGetCategoryByIndex(categoryBand, idxBand);
            paramLayerInfo.categoryName.push_back(String8(CateBand->name));//Band
            paramLayerInfo.categoryName.push_back(String8(audioType_SpeechDMNR_CategoryName2[idxProfile]));//Profile

            getSpeechParamFromAppParser(AUDIO_TYPE_SPEECH_DMNR, &paramLayerInfo, packedParamUnitFromApp, &sizeByteFromApp);

            if (size + sizeByteFromApp > kSphParamSize) {
                ALOGE("%s(), packedParamUnitFromApp overflow!! max:%d, total use:%d", __FUNCTION__, kSphParamSize, size + sizeByteFromApp);
                if (packedParamUnitFromApp != NULL) {
                    delete[] packedParamUnitFromApp;
                }
                if (paramLayerInfo.logPrintParamUnit != NULL) {
                    delete[] paramLayerInfo.logPrintParamUnit;
                }
                return -ENOMEM;
            } else {
                memcpy(bufParamUnit + size, packedParamUnitFromApp, sizeByteFromApp);
                size += sizeByteFromApp;
            }
            char sphLogTemp[SPH_DUMP_STR_SIZE] = {0};
            snprintf(sphLogTemp, SPH_DUMP_STR_SIZE, "header=0x%x[%d,%d], size(b)=%d;", dataHeader, idxBand, idxProfile, sizeByteFromApp);
            audio_strncat(paramLayerInfo.logPrintParamUnit, sphLogTemp, SPH_DUMP_STR_SIZE);
            paramLayerInfo.categoryName.pop_back();
            paramLayerInfo.categoryName.pop_back();

        }
    }

    if (packedParamUnitFromApp != NULL) {
        delete[] packedParamUnitFromApp;
    }

    char sphLogTemp[SPH_DUMP_STR_SIZE] = {0};
    snprintf(sphLogTemp, SPH_DUMP_STR_SIZE, "total size(b)=%d", size);
    audio_strncat(paramLayerInfo.logPrintParamUnit, sphLogTemp, SPH_DUMP_STR_SIZE);
    ALOGD("%s(),MagiNum(0x%x),xml(%s), %s",
          __FUNCTION__, headerParamUnit.sphUnitMagiNum, paramLayerInfo.audioTypeName, paramLayerInfo.logPrintParamUnit);
    if (paramLayerInfo.logPrintParamUnit != NULL) {
        delete[] paramLayerInfo.logPrintParamUnit;
    }

    return size;
}

speech_mode_t SpeechParserGen93::getSpeechModeByOutputDevice(const audio_devices_t output_device) {
    speech_mode_t speech_mode = SPEECH_MODE_NORMAL;

    if (audio_is_bluetooth_sco_device(output_device)) {
        speech_mode = SPEECH_MODE_BT_EARPHONE;
    } else if (output_device == AUDIO_DEVICE_OUT_SPEAKER) {
#if defined(MTK_INCALL_HANDSFREE_DMNR)
        speech_mode = SPEECH_MODE_MAGIC_CON_CALL;
#else
        speech_mode = SPEECH_MODE_LOUD_SPEAKER;
#endif
    } else if (output_device == AUDIO_DEVICE_OUT_WIRED_HEADSET) {
        speech_mode = SPEECH_MODE_EARPHONE;
    } else if (output_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        speech_mode = SPEECH_MODE_EARPHONE;
    }
#ifdef MTK_USB_PHONECALL
    else if (output_device == AUDIO_DEVICE_OUT_USB_DEVICE) {
        speech_mode = SPEECH_MODE_USB_AUDIO;
    }
#endif
    else if (output_device == AUDIO_DEVICE_OUT_EARPIECE) {
        if (getFeatureOn(SPEECH_FEATURE_HAC)) {
            speech_mode = SPEECH_MODE_HAC;
        } else    {
            speech_mode = SPEECH_MODE_NORMAL;
        }
    }
    return speech_mode;
}

bool SpeechParserGen93::getFeatureOn(const SpeechFeatureType featureType) {
    uint16_t featureMaskType = 1 << featureType;
    const bool featureOn = mSpeechParserAttribute.speechFeatureOn & featureMaskType;
    ALOGV("%s() featureMaskType: 0x%x, featureOn=%d", __FUNCTION__, featureMaskType, featureOn);
    return featureOn;
}


} // end namespace android


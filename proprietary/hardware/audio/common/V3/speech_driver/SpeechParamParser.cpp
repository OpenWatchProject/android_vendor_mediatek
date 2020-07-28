#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechParamParser"
#include "SpeechParamParser.h"
#include "utstring.h"
#include <utils/Log.h>
#include <inttypes.h>
#include <media/AudioParameter.h>

#include "AudioUtility.h"//Mutex/assert
#include "AudioALSAStreamManager.h"
#include <audio_memory_control.h>


namespace android {

#define MAX_BYTE_PARAM_SPEECH 3434

#define SPH_DUMP_STR_SIZE (500)
#define SPH_PARAM_UNIT_DUMP_STR_SIZE (1024)

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */
struct SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT {
    uint16_t sphParserVer;
    uint16_t numLayer;
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


/*==============================================================================
 *                     Property keys
 *============================================================================*/
const char *PROPERTY_KEY_SPEECHLOG_ON = "persist.vendor.audiohal.speech_log_on";

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

SpeechParamParser *SpeechParamParser::UniqueSpeechParamParser = NULL;


SpeechParamParser *SpeechParamParser::getInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);
    ALOGV("%s()", __FUNCTION__);

    if (UniqueSpeechParamParser == NULL) {
        UniqueSpeechParamParser = new SpeechParamParser();
    }
    ASSERT(UniqueSpeechParamParser != NULL);
    return UniqueSpeechParamParser;
}
/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

SpeechParamParser::SpeechParamParser() {
    ALOGD("%s()", __FUNCTION__);
    mAppHandle = NULL;
    mSpeechParamVerFirst = 0;
    mSpeechParamVerLast = 0;
    mNumSpeechParam = 3;

    mSphParamInfo = NULL;
    mSphParamSupport = NULL;
    mListSpeechNetwork = NULL;
    mNameForEachSpeechNetwork = NULL;
    AUDIO_ALLOC_STRUCT(SPEECH_PARAM_INFO_STRUCT, mSphParamInfo);
    AUDIO_ALLOC_STRUCT(SPEECH_PARAM_SUPPORT_STRUCT, mSphParamSupport);
    AUDIO_ALLOC_STRUCT_ARRAY(SPEECH_NETWORK_STRUCT, 12, mListSpeechNetwork);
    AUDIO_ALLOC_STRUCT_ARRAY(SPEECH_NETWORK_STRUCT, 12, mNameForEachSpeechNetwork);

    init();
}

SpeechParamParser::~SpeechParamParser() {
    ALOGD("%s()", __FUNCTION__);

    AUDIO_FREE_POINTER(mSphParamInfo);
    AUDIO_FREE_POINTER(mSphParamSupport);
    AUDIO_FREE_POINTER(mListSpeechNetwork);
    AUDIO_FREE_POINTER(mNameForEachSpeechNetwork);
}

void SpeechParamParser::init() {
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
    }
}

/*==============================================================================
 *                     SpeechParamParser Imeplementation
 *============================================================================*/
bool SpeechParamParser::GetSpeechParamSupport(const char *paramName) {
    bool mSupport = false;
    if (paramName != NULL) {
        if (strcmp(paramName, "SPH_PARAM_TTY") == 0) {
            mSupport = mSphParamSupport->isTTYSupport;
        } else if (strcmp(paramName, "SPH_PARAM_SV") == 0) {
            mSupport = mSphParamSupport->isSuperVolumeSupport;
        } else {
            mSupport = false;
        }
        ALOGV("%s(), %s = %d", __FUNCTION__, paramName, mSupport);
    } else {
        mSupport = false;
    }
    return mSupport;
}

void SpeechParamParser::initAppParser() {
    ALOGD("+%s()", __FUNCTION__);
    /* Init AppHandle */
    ALOGD("%s() appHandleGetInstance", __FUNCTION__);
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
        return;
    }
    mAppHandle = appOps->appHandleGetInstance();
    ALOGD("%s() appHandleRegXmlChangedCb", __FUNCTION__);

}

status_t SpeechParamParser::speechDataDump(char *bufDump,
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


status_t SpeechParamParser::getSpeechParamFromAppParser(uint16_t idxSphType,
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
    Param  *SpeechParam;
    UT_string *uts_categoryPath = NULL;

    /* If user select a category path, just like "NarrowBand / Normal of Handset / Level0" */
    utstring_new(uts_categoryPath);

    ALOGV("%s(), categoryType.size=%zu, paramName.size=%zu",
          __FUNCTION__, paramLayerInfo->categoryType.size(), paramLayerInfo->paramName.size());
    for (idxCount = 0; idxCount < paramLayerInfo->categoryType.size() ; idxCount++) {
        ALOGV("%s(), categoryType[%d]= %s",
              __FUNCTION__, idxCount, paramLayerInfo->categoryType.at(idxCount).string());
    }
    for (idxCount = 0; idxCount < paramLayerInfo->categoryName.size() ; idxCount++) {
        ALOGV("%s(), categoryName[%d]= %s",
              __FUNCTION__, idxCount, paramLayerInfo->categoryName.at(idxCount).string());
    }


    for (idxCount = 0; idxCount < paramLayerInfo->numCategoryType ; idxCount++) {
        if (idxCount == paramLayerInfo->numCategoryType - 1) {
            //last time concat
            utstring_printf(uts_categoryPath, "%s,%s", (char *)(paramLayerInfo->categoryType.at(idxCount).string()),
                            (char *)(paramLayerInfo->categoryName.at(idxCount).string()));
        } else {
            utstring_printf(uts_categoryPath, "%s,%s,", (char *)(paramLayerInfo->categoryType.at(idxCount).string()),
                            (char *)(paramLayerInfo->categoryName.at(idxCount).string()));
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
        ALOGE("%s() can't find paramUnit, Assert!!! audioType=%s, categoryPath=%s",
              __FUNCTION__, audioType->name, categoryPath);
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

        SpeechParam = appOps->paramUnitGetParamByName(paramUnit,
                                                      (const char *)paramLayerInfo->paramName.at(idxCount).string());
        if (SpeechParam) {
            sizeByteParam = appOps->paramGetNumOfBytes(SpeechParam);
            memcpy(bufParamUnit + *sizeByteTotal, SpeechParam->data, sizeByteParam);
            *sizeByteTotal += sizeByteParam;
            ALOGV("%s() paramName=%s, sizeByteParam=%d",
                  __FUNCTION__, paramLayerInfo->paramName.at(idxCount).string(), sizeByteParam);
            //speech parameter dump
            speechDataDump(bufParamDump, idxSphType, (const char *)paramLayerInfo->paramName.at(idxCount).string(),
                           (const char *)SpeechParam->data);
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


int SpeechParamParser::GetDmnrParamUnit(char *bufParamUnit) {
    ALOGV("+%s()", __FUNCTION__);
    uint16_t size = 0, idxBand = 0, idxProfile = 0, sizeByteFromApp = 0;
    uint16_t dataHeader, idxTmp = 0, numBand = 0, numProfile = 0;
    SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT headerParamUnit;
    memset(&headerParamUnit, 0, sizeof(headerParamUnit));

    AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT paramLayerInfo;

    paramLayerInfo.audioTypeName = (char *) audioTypeNameList[AUDIO_TYPE_SPEECH_DMNR];
    paramLayerInfo.numCategoryType = MAX_NUM_CATEGORY_TYPE_SPEECH_DMNR;//4
    paramLayerInfo.numParam = MAX_NUM_PARAM_SPEECH_DMNR;//4
    paramLayerInfo.categoryType.assign(audioType_SpeechDMNR_CategoryType,
                                       audioType_SpeechDMNR_CategoryType + paramLayerInfo.numCategoryType);
    paramLayerInfo.paramName.assign(audioType_SpeechDMNR_ParamName,
                                    audioType_SpeechDMNR_ParamName + paramLayerInfo.numParam);
    paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
    memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

    ALOGV("%s(), categoryType.size=%zu, paramName.size=%zu",
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

    CategoryType *categoryBand = appOps->audioTypeGetCategoryTypeByName(audioType,
                                                                        audioType_SpeechDMNR_CategoryType[0].string());
    numBand = appOps->categoryTypeGetNumOfCategory(categoryBand);
    CategoryType *categoryProfile = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_SpeechDMNR_CategoryType[1].string());
    numProfile = appOps->categoryTypeGetNumOfCategory(categoryProfile);
    idxTmp = (numBand & 0xF) << 4;
    headerParamUnit.numEachLayer = idxTmp + (numProfile & 0xF);
    setMDParamUnitHdr(AUDIO_TYPE_SPEECH_DMNR, &headerParamUnit, numBand);

    memcpy(bufParamUnit + size, &headerParamUnit, sizeof(headerParamUnit));
    size += sizeof(headerParamUnit);

    for (idxBand = 0; idxBand < numBand; idxBand++) { //NB, WB, SWB
        for (idxProfile = 0; idxProfile < numProfile; idxProfile++) {
            sizeByteFromApp = 0;
            dataHeader = ((idxBand + 1) << 4) + (idxProfile + 1);
            memcpy(bufParamUnit + size, &dataHeader, sizeof(dataHeader));
            size += sizeof(dataHeader);
            Category *CateBand =  appOps->categoryTypeGetCategoryByIndex(categoryBand, idxBand);
            paramLayerInfo.categoryName.push_back(String8(CateBand->name));//Band
            paramLayerInfo.categoryName.push_back(String8(audioType_SpeechDMNR_CategoryName2[idxProfile]));//Profile

            getSpeechParamFromAppParser(AUDIO_TYPE_SPEECH_DMNR, &paramLayerInfo,
                                        packedParamUnitFromApp, &sizeByteFromApp);

            memcpy(bufParamUnit + size, packedParamUnitFromApp, sizeByteFromApp);
            size += sizeByteFromApp;
            char sphLogTemp[SPH_DUMP_STR_SIZE] = {0};
            snprintf(sphLogTemp, SPH_DUMP_STR_SIZE, "header=0x%x[%d,%d], size(b)=%d;",
                     dataHeader, idxBand, idxProfile, sizeByteFromApp);
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
    ALOGD("%s(),MagiNum(0x%x),xml(%s), %s", __FUNCTION__,
          headerParamUnit.sphUnitMagiNum, paramLayerInfo.audioTypeName, paramLayerInfo.logPrintParamUnit);
    if (paramLayerInfo.logPrintParamUnit != NULL) {
        delete[] paramLayerInfo.logPrintParamUnit;
    }

    return size;
}

int SpeechParamParser::GetGeneralParamUnit(char *bufParamUnit) {
    ALOGV("+%s()", __FUNCTION__);
    uint16_t size = 0, idxCount = 0, idxCount2 = 0, sizeByteFromApp = 0;
    uint16_t dataHeader;
    SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT headerParamUnit;
    memset(&headerParamUnit, 0, sizeof(headerParamUnit));

    headerParamUnit.sphParserVer = 1;
    headerParamUnit.numLayer = 0x1;
    headerParamUnit.numEachLayer = 0x1;
    headerParamUnit.paramHeader[0] = 0x1;//Common
    headerParamUnit.sphUnitMagiNum = 0xAA02;

    memcpy(bufParamUnit + size, &headerParamUnit, sizeof(headerParamUnit));
    size += sizeof(headerParamUnit);

    char *packedParamUnitFromApp = new char [MAX_BYTE_PARAM_SPEECH];
    memset(packedParamUnitFromApp, 0, MAX_BYTE_PARAM_SPEECH);

    AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT paramLayerInfo;

    paramLayerInfo.audioTypeName = (char *) audioTypeNameList[AUDIO_TYPE_SPEECH_GENERAL];
    paramLayerInfo.numCategoryType = MAX_NUM_CATEGORY_TYPE_SPEECH_GENERAL;//4
    paramLayerInfo.numParam = MAX_NUM_PARAM_SPEECH_GENERAL;//4

    paramLayerInfo.categoryType.assign(audioType_SpeechGeneral_CategoryType,
                                       audioType_SpeechGeneral_CategoryType + paramLayerInfo.numCategoryType);
    paramLayerInfo.paramName.assign(audioType_SpeechGeneral_ParamName,
                                    audioType_SpeechGeneral_ParamName + paramLayerInfo.numParam);
    paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
    memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

    ALOGV("%s(), eParamUnitHdr.sphUnitMagiNum= 0x%x, categoryType.size=%zu, paramName.size=%zu", __FUNCTION__,
          headerParamUnit.sphUnitMagiNum, paramLayerInfo.categoryType.size(), paramLayerInfo.paramName.size());
    for (idxCount = 0; idxCount < paramLayerInfo.paramName.size() ; idxCount++) {
        ALOGV("%s(), paramName[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.paramName.at(idxCount).string());
    }

    dataHeader = 0x000F;
    memcpy(bufParamUnit + size, &dataHeader, sizeof(dataHeader));
    size += sizeof(dataHeader);

    paramLayerInfo.categoryName.push_back(String8(audioType_SpeechGeneral_CategoryName1[0]));

    getSpeechParamFromAppParser(AUDIO_TYPE_SPEECH_GENERAL, &paramLayerInfo, packedParamUnitFromApp, &sizeByteFromApp);

    memcpy(bufParamUnit + size, packedParamUnitFromApp, sizeByteFromApp);
    size += sizeByteFromApp;

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

int SpeechParamParser::GetEchoRefParamUnit(char *bufParamUnit) {
    ALOGV("+%s()", __FUNCTION__);
    uint16_t size = 0, idxCount = 0, idxCount2 = 0, sizeByteFromApp = 0;
    uint16_t dataHeader;
    SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT headerParamUnit;
    memset(&headerParamUnit, 0, sizeof(headerParamUnit));

    headerParamUnit.sphParserVer = 1;
    headerParamUnit.numLayer = 0x1;
    headerParamUnit.numEachLayer = 0x1;
    headerParamUnit.paramHeader[0] = 0x1;
    headerParamUnit.sphUnitMagiNum = 0xAA06;

    memcpy(bufParamUnit + size, &headerParamUnit, sizeof(headerParamUnit));
    size += sizeof(headerParamUnit);

    char *packedParamUnitFromApp = new char [MAX_BYTE_PARAM_SPEECH];
    memset(packedParamUnitFromApp, 0, MAX_BYTE_PARAM_SPEECH);
    AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT paramLayerInfo;

    paramLayerInfo.audioTypeName = (char *) audioTypeNameList[AUDIO_TYPE_SPEECH_ECHOREF];
    paramLayerInfo.numCategoryType = MAX_NUM_CATEGORY_TYPE_SPEECH_ECHOREF;//4
    paramLayerInfo.numParam = MAX_NUM_PARAM_SPEECH_ECHOREF;//4

    paramLayerInfo.categoryType.assign(audioType_SpeechEchoRef_CategoryType,
                                       audioType_SpeechEchoRef_CategoryType + paramLayerInfo.numCategoryType);
    paramLayerInfo.paramName.assign(audioType_SpeechEchoRef_ParamName,
                                    audioType_SpeechEchoRef_ParamName + paramLayerInfo.numParam);
    paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
    memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

    ALOGV("%s(), eParamUnitHdr.sphUnitMagiNum= 0x%x, categoryType.size=%zu, paramName.size=%zu", __FUNCTION__,
          headerParamUnit.sphUnitMagiNum, paramLayerInfo.categoryType.size(), paramLayerInfo.paramName.size());
    for (idxCount = 0; idxCount < paramLayerInfo.paramName.size() ; idxCount++) {
        ALOGV("%s(), paramName[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.paramName.at(idxCount).string());
    }

    dataHeader = 0x000F;
    memcpy(bufParamUnit + size, &dataHeader, sizeof(dataHeader));
    size += sizeof(dataHeader);

    paramLayerInfo.categoryName.push_back(String8(audioType_SpeechEchoRef_CategoryName1[0]));

    getSpeechParamFromAppParser(AUDIO_TYPE_SPEECH_ECHOREF, &paramLayerInfo, packedParamUnitFromApp, &sizeByteFromApp);

    memcpy(bufParamUnit + size, packedParamUnitFromApp, sizeByteFromApp);
    size += sizeByteFromApp;

    if (packedParamUnitFromApp != NULL) {
        delete[] packedParamUnitFromApp;
    }
    char sphLogTemp[SPH_DUMP_STR_SIZE] = {0};
    snprintf(sphLogTemp, SPH_DUMP_STR_SIZE, "total size(b)=%d", size);
    audio_strncat(paramLayerInfo.logPrintParamUnit, sphLogTemp, SPH_DUMP_STR_SIZE);
    ALOGD("%s(),MagiNum(0x%x),xml(%s), %s", __FUNCTION__, headerParamUnit.sphUnitMagiNum,
          paramLayerInfo.audioTypeName, paramLayerInfo.logPrintParamUnit);
    if (paramLayerInfo.logPrintParamUnit != NULL) {
        delete[] paramLayerInfo.logPrintParamUnit;
    }

    return size;
}
int SpeechParamParser::getSpeechProfile(const speech_mode_t sphMode, bool btHeadsetNrecOn) {
    speech_profile_t idxSphProfile;

    if (mSphParamInfo->isLPBK) {
        switch (sphMode) {
        case SPEECH_MODE_NORMAL:
            idxSphProfile = SPEECH_PROFILE_LPBK_HANDSET;
            break;
        case SPEECH_MODE_EARPHONE:
            idxSphProfile = SPEECH_PROFILE_LPBK_HEADSET;
            break;
        case SPEECH_MODE_LOUD_SPEAKER:
            idxSphProfile = SPEECH_PROFILE_LPBK_HANDSFREE;
            break;
        default:
            idxSphProfile = SPEECH_PROFILE_LPBK_HANDSET;

            break;
        }
    } else {
        switch (sphMode) {
        case SPEECH_MODE_NORMAL:
            if (mSphParamInfo->idxTTY == TTY_PARAM_OFF) {
                if (mSphParamInfo->isSV) {
                    idxSphProfile = SPEECH_PROFILE_HANDSET_SV;
                } else {
                    idxSphProfile = SPEECH_PROFILE_HANDSET;
                }
            } else {
                switch (mSphParamInfo->idxTTY) {
                case TTY_PARAM_HCO:
                    idxSphProfile = SPEECH_PROFILE_TTY_HCO_HANDSET;
                    break;
                case TTY_PARAM_VCO:
                    idxSphProfile = SPEECH_PROFILE_TTY_VCO_HANDSET;
                    break;
                default:
                    idxSphProfile = SPEECH_PROFILE_TTY_HCO_HANDSET;
                    break;
                }
            }
            break;

        case SPEECH_MODE_EARPHONE:
            switch (mSphParamInfo->numHeadsetPole) {
            case 3:
                idxSphProfile = SPEECH_PROFILE_3_POLE_HEADSET;
                break;
            case 4:
                idxSphProfile = SPEECH_PROFILE_4_POLE_HEADSET;
                break;
            case 5:
                idxSphProfile = SPEECH_PROFILE_5_POLE_HEADSET;
                break;
            default:
                idxSphProfile = SPEECH_PROFILE_4_POLE_HEADSET;
                break;
            }
            break;
        case SPEECH_MODE_LOUD_SPEAKER:
            if (mSphParamInfo->idxTTY == TTY_PARAM_OFF) {
                if (mSphParamInfo->isSV) {
                    idxSphProfile = SPEECH_PROFILE_HANDSFREE_SV;
                } else {
                    idxSphProfile = SPEECH_PROFILE_HANDSFREE;
                }
            } else {
                switch (mSphParamInfo->idxTTY) {
                case TTY_PARAM_HCO:
                    idxSphProfile = SPEECH_PROFILE_TTY_HCO_HANDSFREE;
                    break;
                case TTY_PARAM_VCO:
                    idxSphProfile = SPEECH_PROFILE_TTY_VCO_HANDSFREE;
                    break;
                default:
                    idxSphProfile = SPEECH_PROFILE_TTY_HCO_HANDSFREE;
                    break;
                }
            }
            break;
        case SPEECH_MODE_BT_EARPHONE:
        case SPEECH_MODE_BT_CORDLESS:
        case SPEECH_MODE_BT_CARKIT:
            if (btHeadsetNrecOn == true) {
                idxSphProfile = SPEECH_PROFILE_BT_EARPHONE;
            } else {
                idxSphProfile = SPEECH_PROFILE_BT_NREC_OFF;
            }
            break;
        case SPEECH_MODE_MAGIC_CON_CALL:
            idxSphProfile = SPEECH_PROFILE_MAGICONFERENCE;
            break;
        case SPEECH_MODE_HAC:
            idxSphProfile = SPEECH_PROFILE_HAC;
            break;
        case SPEECH_MODE_USB_AUDIO:
            idxSphProfile = SPEECH_PROFILE_USB_HEADSET;
            break;
        default:
            idxSphProfile = SPEECH_PROFILE_HANDSET;

            break;
        }
    }
    ALOGV("%s(), sphMode = %d, idxSphProfile = %d", __FUNCTION__, sphMode, idxSphProfile);

    return idxSphProfile;
}

status_t SpeechParamParser::setMDParamUnitHdr(speech_type_dynamic_param_t idxAudioType,
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

uint16_t SpeechParamParser::setMDParamDataHdr(SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT paramUnitHdr,
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


int SpeechParamParser::initSpeechNetwork(void) {
    uint16_t size = 0, idxCount, sizeByteFromApp = 0;
    char *packedParamUnitFromApp = new char [10];
    memset(packedParamUnitFromApp, 0, 10);

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

        paramLayerInfo.categoryType.assign(audioType_SpeechNetwork_CategoryType,
                                           audioType_SpeechNetwork_CategoryType + paramLayerInfo.numCategoryType);
        paramLayerInfo.paramName.assign(audioType_SpeechNetwork_ParamName,
                                        audioType_SpeechNetwork_ParamName + paramLayerInfo.numParam);
        paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
        memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

        ALOGV("%s(), categoryType.size=%zu, paramName.size=%zu",
              __FUNCTION__, paramLayerInfo.categoryType.size(), paramLayerInfo.paramName.size());
        for (idxCount = 0; idxCount < paramLayerInfo.categoryType.size() ; idxCount++) {
            ALOGV("%s(), categoryType[%d]= %s",
                  __FUNCTION__, idxCount, paramLayerInfo.categoryType.at(idxCount).string());
        }
        for (idxCount = 0; idxCount < paramLayerInfo.paramName.size() ; idxCount++) {
            ALOGV("%s(), paramName[%d]= %s",
                  __FUNCTION__, idxCount, paramLayerInfo.paramName.at(idxCount).string());
        }
        //-----------
        //parse layer
        CategoryType *categoryNetwork = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_SpeechNetwork_CategoryType[0].string());
        mNumSpeechNetwork = appOps->categoryTypeGetNumOfCategory(categoryNetwork);

        //parse network
        for (int idxNetwork = 0; idxNetwork < mNumSpeechNetwork; idxNetwork++) {
            Category *CateNetwork = appOps->categoryTypeGetCategoryByIndex(categoryNetwork, idxNetwork);
            sizeByteFromApp = 0;
            //clear
            while (!paramLayerInfo.categoryName.empty()) {
                paramLayerInfo.categoryName.pop_back();
            }
            audio_strncpy(mListSpeechNetwork[idxNetwork].name, CateNetwork->name, 128);

            paramLayerInfo.categoryName.push_back(String8(CateNetwork->name));//Network

            getSpeechParamFromAppParser(AUDIO_TYPE_SPEECH_NETWORK, &paramLayerInfo, packedParamUnitFromApp, &sizeByteFromApp);
            mListSpeechNetwork[idxNetwork].supportBit = *((uint16_t *)packedParamUnitFromApp);
            size += sizeByteFromApp;

            ALOGD("%s(), idxNetwork=%d, sizeByteFromApp=%d, supportBit=0x%x",
                  __FUNCTION__, idxNetwork, sizeByteFromApp, mListSpeechNetwork[idxNetwork].supportBit);
        }
        ALOGD("-%s(), total size byte=%d", __FUNCTION__, size);
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
        ALOGD("%s(), mNameForEachSpeechNetwork[%d].name = %s",
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


int SpeechParamParser::GetSpeechParamUnit(char *bufParamUnit, int *paramArg) {
    uint16_t size = 0, idxCount, sizeByteFromApp = 0;
    uint16_t dataHeader, idxInfo = 0, idxTmp = 0, numBand = 0, numNetwork = 0, numVolume = 0;
    SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT headerParamUnit;
    int idxProfile = 0, parserOption = 0;

    speech_mode_t sphMode = (speech_mode_t) * ((int *)paramArg);
    int idxVolume = *((int *)paramArg + 1);
    bool btHeadsetNrecOn = (bool) * ((int *)paramArg + 2);
    mSphParamInfo->isBtNrecOn = btHeadsetNrecOn;
    mSphParamInfo->idxVolume = idxVolume;
    mSphParamInfo->speechMode = sphMode;

    parserOption = * ((int *)paramArg + 3);
    //bit 0: dv profile, bit 1: single band, bit 4~7: band number
    mSphParamInfo->isSingleBandTransfer = (bool)(parserOption & 0x2);
    mSphParamInfo->idxVoiceBandStart = (unsigned char)((parserOption & 0xf0) >> 4);

    idxProfile = getSpeechProfile(sphMode, btHeadsetNrecOn);
    ALOGD("+%s(), sphMode=0x%x, Volume=0x%x, BtNrecOn=0x%x, Profile=%d, parserOption=0x%x",
          __FUNCTION__, sphMode, idxVolume, btHeadsetNrecOn, idxProfile, parserOption);

    memset(&headerParamUnit, 0, sizeof(headerParamUnit));

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

    paramLayerInfo.categoryType.assign(audioType_Speech_CategoryType,
                                       audioType_Speech_CategoryType + paramLayerInfo.numCategoryType);
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
    CategoryType *categoryNetwork = appOps->audioTypeGetCategoryTypeByName(audioType,
                                                                           audioType_Speech_CategoryType[3].string());
    CategoryType *categoryBand = appOps->audioTypeGetCategoryTypeByName(audioType,
                                                                        audioType_Speech_CategoryType[0].string());
    numNetwork = appOps->categoryTypeGetNumOfCategory(categoryNetwork);
    numBand = appOps->categoryTypeGetNumOfCategory(categoryBand);

    CategoryType *categoryVolume = appOps->audioTypeGetCategoryTypeByName(audioType,
                                                                          audioType_Speech_CategoryType[2].string());
    CategoryGroup *categoryGroupVolume = appOps->categoryTypeGetCategoryGroupByIndex(categoryVolume, 0);
    numVolume = appOps->categoryGroupGetNumOfCategory(categoryGroupVolume);
    idxTmp = (numBand & 0xF) << 4;
    headerParamUnit.numEachLayer = idxTmp + (numNetwork & 0xF);
    ALOGV("%s(), sphUnitMagiNum= 0x%x, numEachLayer=0x%x",
          __FUNCTION__, headerParamUnit.sphUnitMagiNum, headerParamUnit.numEachLayer);
    if (mSphParamInfo->isSingleBandTransfer) {
        setMDParamUnitHdr(AUDIO_TYPE_SPEECH, &headerParamUnit, mSphParamInfo->idxVoiceBandStart);
    } else {
        setMDParamUnitHdr(AUDIO_TYPE_SPEECH, &headerParamUnit, numBand);
    }
    ALOGV("%s(), sphUnitMagiNum= 0x%x, numEachLayer=0x%x",
           __FUNCTION__, headerParamUnit.sphUnitMagiNum, headerParamUnit.numEachLayer);
    ALOGV("%s(), categoryNetwork= %s, categoryBand = %s, categoryVolume = %s",
          __FUNCTION__, categoryNetwork->name, categoryBand->name, categoryVolume->name);
    ALOGV("%s(), numNetwork= %d, numBand = %d, numVolume = %d", __FUNCTION__, numNetwork, numBand, numVolume);

    memcpy(bufParamUnit + size, &headerParamUnit, sizeof(headerParamUnit));
    size += sizeof(headerParamUnit);

    idxInfo = sphMode & 0xF;
    ALOGV("%s(), add mode idxInfo=0x%x", __FUNCTION__, idxInfo);
    idxTmp = idxVolume << 4;
    idxInfo += idxTmp;
    ALOGV("%s(), add volume<<4 idxInfo=0x%x, idxTmp=0x%x", __FUNCTION__, idxInfo, idxTmp);
    paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
    memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

    memcpy(bufParamUnit + size, &idxInfo, sizeof(idxInfo));
    size += sizeof(idxInfo);
    //parse network
    for (int idxNetwork = 0; idxNetwork < numNetwork; idxNetwork++) {
        Category *CateNetwork =  appOps->categoryTypeGetCategoryByIndex(categoryNetwork, idxNetwork);
        //parse band
        for (int idxBand = mSphParamInfo->idxVoiceBandStart; idxBand < mSphParamInfo->idxVoiceBandStart + numBand; idxBand++) {
            sizeByteFromApp = 0;
            Category *CateBand =  appOps->categoryTypeGetCategoryByIndex(categoryBand, idxBand);

            dataHeader = setMDParamDataHdr(headerParamUnit, CateBand->name, CateNetwork->name);
            memcpy(bufParamUnit + size, &dataHeader, sizeof(dataHeader));
            size += sizeof(dataHeader);
            while (!paramLayerInfo.categoryName.empty()) {
                paramLayerInfo.categoryName.pop_back();
            }
            //Band
            paramLayerInfo.categoryName.push_back(String8(CateBand->name));//Band
            //Profile
            paramLayerInfo.categoryName.push_back(String8(audioType_Speech_CategoryName2[idxProfile]));
            //Volume
            if (idxVolume >= NUM_VOLUME_SPEECH || idxVolume < 0) {
                paramLayerInfo.categoryName.push_back(String8(audioType_Speech_CategoryName3[3]));//volume
                ALOGE("%s(), Invalid idxVolume=0x%x, use %s !!!",
                      __FUNCTION__, idxVolume, audioType_Speech_CategoryName3[3]);
            } else {
                paramLayerInfo.categoryName.push_back(String8(audioType_Speech_CategoryName3[idxVolume]));
            }
            paramLayerInfo.categoryName.push_back(String8(CateNetwork->name));//Network

            for (idxCount = 0; idxCount < paramLayerInfo.categoryName.size() ; idxCount++) {
                ALOGV("%s(), categoryName[%d]= %s",
                      __FUNCTION__, idxCount, paramLayerInfo.categoryName.at(idxCount).string());
            }

            getSpeechParamFromAppParser(AUDIO_TYPE_SPEECH, &paramLayerInfo, packedParamUnitFromApp, &sizeByteFromApp);

            memcpy(bufParamUnit + size, packedParamUnitFromApp, sizeByteFromApp);
            size += sizeByteFromApp;

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


status_t SpeechParamParser::SetParamInfo(const String8 &keyParamPairs) {
    ALOGV("+%s(): %s", __FUNCTION__, keyParamPairs.string());
    AudioParameter param = AudioParameter(keyParamPairs);
    int value;
    if (param.getInt(String8("ParamSphLpbk"), value) == NO_ERROR) {
        param.remove(String8("ParamSphLpbk"));
#if defined(MTK_AUDIO_SPH_LPBK_PARAM)
        mSphParamInfo->isLPBK = (value == 1) ? true : false;
#else
        mSphParamInfo->isLPBK = false;

#endif
        ALOGD("%s(): mSphParamInfo->isLPBK = %d", __FUNCTION__, mSphParamInfo->isLPBK);
    }
    if (param.getInt(String8("ParamHeadsetPole"), value) == NO_ERROR) {
        param.remove(String8("ParamHeadsetPole"));
        if (value == 3 || value == 4 || value == 5) {
            mSphParamInfo->numHeadsetPole = value;
        } else {
            mSphParamInfo->numHeadsetPole = 4;
            ALOGE("%s(): Invalid HeadsetPole(%d), set default 4_pole!!!", __FUNCTION__, value);
        }
        ALOGD("%s(): mSphParamInfo->numHeadsetPole = %d", __FUNCTION__, mSphParamInfo->numHeadsetPole);
    }
    if (param.getInt(String8("ParamSphTty"), value) == NO_ERROR) {
        param.remove(String8("ParamSphTty"));
        if (mSphParamSupport->isTTYSupport) {
            mSphParamInfo->idxTTY = value;
        }
        ALOGD("%s(): mSphParamInfo->idxTTY = %d", __FUNCTION__, mSphParamInfo->idxTTY);
    }
    if (param.getInt(String8("ParamSphSV"), value) == NO_ERROR) {
        param.remove(String8("ParamSphSV"));
        if (mSphParamSupport->isSuperVolumeSupport) {
            mSphParamInfo->isSV = (value == 1) ? true : false;
        } else {
            mSphParamInfo->isSV = false;
        }
        ALOGD("%s(): mSphParamInfo->isSV = %d", __FUNCTION__, mSphParamInfo->isSV);
    }

    ALOGD("-%s(): %s", __FUNCTION__, keyParamPairs.string());
    return NO_ERROR;
}

bool SpeechParamParser::GetParamStatus(const char *paramName) {
    bool status = false;
    if (paramName != NULL) {
        if (strcmp(paramName, "ParamSphTty") == 0) {
            status = (mSphParamInfo->idxTTY == 0) ? false : true;
        } else if (strcmp(paramName, "ParamSphSV") == 0) {
            status = mSphParamInfo->isSV;
        } else {
            status = false;
        }
        ALOGD("%s(), %s = %d", __FUNCTION__, paramName, status);
    } else {
        status = false;
    }
    return status;
}

int SpeechParamParser::GetMagiClarityParamUnit(char *bufParamUnit) {
    ALOGV("+%s()", __FUNCTION__);
    uint16_t size = 0, idxCount = 0, idxCount2 = 0, sizeByteFromApp = 0;
    uint16_t dataHeader;
    SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT headerParamUnit;
    memset(&headerParamUnit, 0, sizeof(headerParamUnit));

    headerParamUnit.sphParserVer = 1;
    headerParamUnit.numLayer = 0x1;
    headerParamUnit.numEachLayer = 0x1;
    headerParamUnit.paramHeader[0] = 0x1;//Common
    headerParamUnit.sphUnitMagiNum = 0xAA04;

    memcpy(bufParamUnit + size, &headerParamUnit, sizeof(headerParamUnit));
    size += sizeof(headerParamUnit);

    char *packedParamUnitFromApp = new char [MAX_BYTE_PARAM_SPEECH];
    memset(packedParamUnitFromApp, 0, MAX_BYTE_PARAM_SPEECH);
    AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT paramLayerInfo;

    paramLayerInfo.audioTypeName = (char *) audioTypeNameList[AUDIO_TYPE_SPEECH_MAGICLARITY];
    paramLayerInfo.numCategoryType = MAX_NUM_CATEGORY_TYPE_SPEECH_MAGICLARITY;//4
    paramLayerInfo.numParam = MAX_NUM_PARAM_SPEECH_MAGICLARITY;//4

    paramLayerInfo.categoryType.assign(audioType_SpeechMagiClarity_CategoryType,
                                       audioType_SpeechMagiClarity_CategoryType + paramLayerInfo.numCategoryType);
    paramLayerInfo.paramName.assign(audioType_SpeechMagiClarity_ParamName,
                                    audioType_SpeechMagiClarity_ParamName + paramLayerInfo.numParam);
    paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
    memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

    for (idxCount = 0; idxCount < paramLayerInfo.paramName.size() ; idxCount++) {
        ALOGV("%s(), paramName[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.paramName.at(idxCount).string());
    }
    dataHeader = 0x000F;
    memcpy(bufParamUnit + size, &dataHeader, sizeof(dataHeader));
    size += sizeof(dataHeader);

    paramLayerInfo.categoryName.push_back(String8(audioType_SpeechMagiClarity_CategoryName1[0]));

    getSpeechParamFromAppParser(AUDIO_TYPE_SPEECH_MAGICLARITY, &paramLayerInfo,
                                packedParamUnitFromApp, &sizeByteFromApp);

    memcpy(bufParamUnit + size, packedParamUnitFromApp, sizeByteFromApp);
    size += sizeByteFromApp;

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

int SpeechParamParser::GetBtDelayTime(const char *btDeviceName) {
    int btDelayMs = 0;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL || btDeviceName == NULL) {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
        return 0;
    } else {
        /* Get the BT device delay parameter */
        AudioType *audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, "BtInfo");
        if (audioType) {
            String8 categoryPath("BT headset,");
            categoryPath += btDeviceName;

            ParamUnit *paramUnit = appOps->audioTypeGetParamUnit(audioType, categoryPath.string());
            ASSERT(paramUnit);

            Param *param = appOps->paramUnitGetParamByName(paramUnit, "voice_cp_delay_ms");
            ASSERT(param);

            btDelayMs = *(int *)param->data;
        }
        ALOGD("%s(), btDeviceName=%s, btDelayMs=%d", __FUNCTION__, btDeviceName, btDelayMs);
        return btDelayMs;
    }
}
}

//namespace android

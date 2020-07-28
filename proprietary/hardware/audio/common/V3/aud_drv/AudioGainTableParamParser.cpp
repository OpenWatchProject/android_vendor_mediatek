#include "AudioGainTableParamParser.h"

#include <utils/Log.h>
#include "AudioUtility.h"//Mutex/assert
#include <system/audio.h>

#include <string>
#include "AudioSmartPaController.h"
#include "AudioALSAHardwareResourceManager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "GainTableParamParser"

#ifdef ALOGG
#undef ALOGG
#endif
#ifdef CONFIG_MT_ENG_BUILD
#define ALOGG(...) ALOGD(__VA_ARGS__)
#else
#define ALOGG(...)
#endif

namespace android {

// Stream Type
const std::string gppStreamTypeXmlName[GAIN_STREAM_TYPE_SIZE] = {"Voice",
                                                                 "System",
                                                                 "Ring",
                                                                 "Music",
                                                                 "Alarm",
                                                                 "Notification",
                                                                 "Bluetooth_sco",
                                                                 "Enforced_Audible",
                                                                 "DTMF",
                                                                 "TTS",
                                                                 "Accessibility"
                                                                };
// Device
const std::string gppDeviceXmlName[NUM_GAIN_DEVICE] = {"RCV",
                                                       "HS",
                                                       "SPK",
                                                       "HP",
                                                       "HSSPK",
                                                       "HS5POLE",
                                                       "HS5POLE_ANC",
                                                       "HAC",
                                                       "BT",
                                                       "TTY",
                                                       "LPBK_RCV",
                                                       "LPBK_SPK",
                                                       "LPBK_HP",
                                                       "USB",
                                                       "BT_A2DP",
                                                       "BT_A2DP_HP",
                                                       "BT_A2DP_SPK",
                                                       "RCV_SV",
                                                       "SPK_SV"
                                                      };
// Speech
const std::string gppBandXmlName[NUM_GAIN_SPEECH_BAND] = {"NB",
                                                          "WB",
                                                          "SWB"
                                                         };

const std::string gppNetXmlName[NUM_GAIN_SPEECH_NETWORK] = {"GSM",
                                                            "WCDMA",
                                                            "VoLTE"
                                                           };

// MIC
const std::string gppMicModeXmlName[NUM_GAIN_MIC_MODE] = {"Sound recording",
                                                          "Voice Call",
                                                          "Camera recording",
                                                          "VR",
                                                          "VoIP",
                                                          "VOICE_UNLOCK",
                                                          "CUSTOMIZATION1",
                                                          "CUSTOMIZATION2",
                                                          "CUSTOMIZATION3",
                                                          "Unprocessed",
                                                          "AAudio"
                                                         };
/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

GainTableParamParser *GainTableParamParser::mGainTableParamParser = NULL;

GainTableParamParser *GainTableParamParser::getInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);
    ALOGG("%s()", __FUNCTION__);

    if (mGainTableParamParser == NULL) {
        ALOGD("%s()", __FUNCTION__);
        mGainTableParamParser = new GainTableParamParser();
    }
    ASSERT(mGainTableParamParser != NULL);
    return mGainTableParamParser;
}
/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

GainTableParamParser::GainTableParamParser() {
    ALOGD("%s()", __FUNCTION__);
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(appOps);
        mAppHandle = NULL;
    } else {
        mAppHandle = appOps->appHandleGetInstance();
    }
    loadGainTableParam();
}

GainTableParamParser::~GainTableParamParser() {
    for (unsigned int i = 0; i < NUM_GAIN_DEVICE; i++) {
        mMapDlDigital[i].clear();
        mMapDlAnalog[i].clear();
        mSpec.swagcGainMap[i].clear();
        mSpec.swagcGainMapDmic[i].clear();
        mSpec.ulPgaGainMap[i].clear();
    }
}

status_t GainTableParamParser::getGainTableParam(GainTableParam *_gainTable, std::vector<std::string> *sceneList) {
    ALOGD("%s()", __FUNCTION__);
    clearTableParam(_gainTable, sceneList->size());

    status_t status = NO_ERROR;
    _gainTable->sceneCount = (int)(sceneList->size());
    status |= updatePlaybackDigitalGain(_gainTable, sceneList);
    status |= updatePlaybackAnalogGain(_gainTable, sceneList);
    status |= updateSpeechVol(_gainTable);
    status |= updateRecordVol(_gainTable, sceneList);
    status |= updateVoIPVol(_gainTable, sceneList);
    // Ringback tone need to be updated after VOIP for initializing with voice stream gain
    status |= updateRingbackVol(_gainTable);

    if (status != NO_ERROR) {
        ALOGE("error, %s() failed, status = %d", __FUNCTION__, status);
        return status;
    }

    return NO_ERROR;
}

status_t GainTableParamParser::getCategoryList(AudioType *audioType, std::vector<std::string> *sceneList) {
    int i, j;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }

    std::string sceneStr = "Scene";

    appOps->audioTypeReadLock(audioType, __FUNCTION__);
    int numOfCategoryType = appOps->audioTypeGetNumOfCategoryType(audioType);

    ALOGV("\n====%s AudioType's Category, numOfCategoryType = %d====\n\n", audioType->name, numOfCategoryType);

    for (i = 0; i < numOfCategoryType; i++) {
        CategoryType *categoryType = appOps->audioTypeGetCategoryTypeByIndex(audioType, i);
        ALOGV("CategoryGroup[%d] name = %s wording = %s %s\n", i, categoryType->name, categoryType->wording, categoryType->visible ? "" : "visible = 0");
        std::string xmlStr = categoryType->name;
        if (sceneStr == xmlStr) {
            /* Scene CategoryType's category */
            int numOfCategory = appOps->categoryTypeGetNumOfCategory(categoryType);
            for (j = 0; j < numOfCategory; j++) {
                Category *category = appOps->categoryTypeGetCategoryByIndex(categoryType, j);
                ALOGV("\tCategory[%d] name = %s wording = %s %s\n", j , category->name, category->wording, category->visible ? "" : "visible = 0");
                if (!isInSceneList(sceneList, category->name)) {
                    sceneList->push_back(category->name);
                }
            }
            break;
        }
    }
    appOps->audioTypeUnlock(audioType);
    return NO_ERROR;
}

bool GainTableParamParser::isInSceneList(std::vector<std::string> *sceneList, std::string scene) {
    bool inSceneList = false;
    for (size_t i = 0; i < sceneList->size(); i++) {
        if ((*sceneList)[i] == scene) {
            inSceneList = true;
            break;
        }
    }
    ALOGV("%s(): inSceneList = %d", __FUNCTION__, inSceneList);
    return inSceneList;
}

status_t GainTableParamParser::getSceneList(std::vector<std::string> *sceneList) {
    ALOGV("%s()", __FUNCTION__);

    AudioType *audioType;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }

    // Initialize scene list and add Default scene
    sceneList->clear();
    sceneList->push_back("Default");

    // Get scene from XML
    // PlaybackVolDigi
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, PLAY_DIGI_AUDIOTYPE_NAME);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", PLAY_DIGI_AUDIOTYPE_NAME);
        return BAD_VALUE;
    }
    getCategoryList(audioType, sceneList);

    // PlaybackVolAna
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, PLAY_ANA_AUDIOTYPE_NAME);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", PLAY_ANA_AUDIOTYPE_NAME);
        return BAD_VALUE;
    }
    getCategoryList(audioType, sceneList);

    // RecordVol
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, REC_VOL_AUDIOTYPE_NAME);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", REC_VOL_AUDIOTYPE_NAME);
        return BAD_VALUE;
    }
    getCategoryList(audioType, sceneList);

    // VoIPVol
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, VOIP_VOL_AUDIOTYPE_NAME);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", VOIP_VOL_AUDIOTYPE_NAME);
        return BAD_VALUE;
    }
    getCategoryList(audioType, sceneList);

    for (int i = 0; i < (int)sceneList->size(); i++) {
        ALOGG("%s(): sceneList[%d] = %s", __FUNCTION__, i, (*sceneList)[i].c_str());
    }

    return NO_ERROR;
}

status_t GainTableParamParser::getGainTableSpec(GainTableSpec **_gainTableSpec) {
    *_gainTableSpec = &mSpec;
    return NO_ERROR;
}

status_t GainTableParamParser::clearTableParam(GainTableParam *_gainTable, int sceneCount) {
    memset((void *)_gainTable->sceneGain, 0, sceneCount * sizeof(GainTableForScene));
    memset((void *)&_gainTable->nonSceneGain, 0, sizeof(GainTableForNonScene));
    return NO_ERROR;
}

status_t GainTableParamParser::updatePlaybackDigitalGain(GainTableParam *_gainTable, std::vector<std::string> *sceneList) {
    ALOGD("%s()", __FUNCTION__);

    // define xml names
    const char audioTypeName[] = PLAY_DIGI_AUDIOTYPE_NAME;
    const char paramName[] = "digital_gain";
    const std::string *profileName = gppDeviceXmlName;
    const std::string *volumeTypeName = gppStreamTypeXmlName;

    // extract parameters from xml
    AudioType *audioType;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }

    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, audioTypeName);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", audioTypeName);
        return BAD_VALUE;
    }

    // Read lock
    appOps->audioTypeReadLock(audioType, __FUNCTION__);

    for (int scene = 0; scene < (int)(sceneList->size()); scene++) {
        for (int stream = GAIN_MIN_STREAM_TYPE; stream <= GAIN_MAX_STREAM_TYPE; stream++) {
            for (int device = 0; device < NUM_GAIN_DEVICE; device++) {
                // get param unit using param path
                std::string paramPath = "Scene," +
                                        (*sceneList)[scene] +
                                        ",Volume type," +
                                        volumeTypeName[stream] +
                                        ",Profile," +
                                        profileName[device];
                ALOGV("paramPath = %s", paramPath.c_str());

                ParamUnit *paramUnit;
                paramUnit = appOps->audioTypeGetParamUnit(audioType, paramPath.c_str());
                if (!paramUnit) {
                    ALOGV("warn: get paramUnit fail, paramPath = %s", paramPath.c_str());

                    if (device == GAIN_DEVICE_BT_A2DP ||
                        device == GAIN_DEVICE_BT_A2DP_HP ||
                        device == GAIN_DEVICE_BT_A2DP_SPK) {
                        // handle xml without bt a2dp gain table, use headphone
                        memcpy(_gainTable->sceneGain[scene].streamGain[stream][device],
                               _gainTable->sceneGain[scene].streamGain[stream][GAIN_DEVICE_HEADPHONE],
                               sizeof(struct GainTableUnit) * GAIN_VOL_INDEX_SIZE);
                    }

                    continue;
                }

                Param *param;
                param = appOps->paramUnitGetParamByName(paramUnit, paramName);
                if (!param) {
                    ALOGW("error: get param fail");
                    continue;
                }

                // convert xml param to gain table
                short *shortArray = (short *)param->data;
                int arraySize = param->arraySize;
                if (arraySize > GAIN_VOL_INDEX_SIZE) {
                    ALOGW("error, param->arraySize %d exceed digital array size %d", arraySize, GAIN_VOL_INDEX_SIZE);
                    arraySize = GAIN_VOL_INDEX_SIZE;
                }

                for (int i = 0; i < arraySize; i++) {
                    unsigned char *digital = &_gainTable->sceneGain[scene].streamGain[stream][device][i].digital;
                    // convert 0~-64 dB to 0~255
                    if (shortArray[i] > mSpec.digiDbMax) {
                        ALOGW("error, param out of range, val %d > %d", shortArray[i], mSpec.digiDbMax);
                        *digital = 0;
                    } else if (shortArray[i] <= mSpec.digiDbMin) {
                        ALOGV("error, param out of range, val %d <= %d", shortArray[i], mSpec.digiDbMin);
                        *digital = mSpec.keyVolumeStep;
                    } else {
                        *digital = (shortArray[i] * -1 * mSpec.keyStepPerDb);
                    }

                    ALOGV("\tscene = %d, stream = %d, device = %d, array[%d] = %d, convert result = %d\n",
                            scene, stream, device, i, shortArray[i], *digital);
                }
            }
        }
    }

    // Unlock
    appOps->audioTypeUnlock(audioType);

    return NO_ERROR;
}

status_t GainTableParamParser::updatePlaybackAnalogGain(GainTableParam *_gainTable, std::vector<std::string> *sceneList) {
    ALOGD("%s()", __FUNCTION__);

    // define xml names
    const char audioTypeName[] = PLAY_ANA_AUDIOTYPE_NAME;
    const char paramHsName[] = "headset_pga";
    const char paramSpkName[] = "speaker_pga";
    const char paramRcvName[] = "receiver_pga";
    const std::string *profileName = gppDeviceXmlName;
    const std::string *volumeTypeName = gppStreamTypeXmlName;

    // extract parameters from xml
    AudioType *audioType;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, audioTypeName);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", audioTypeName);
        return BAD_VALUE;
    }

    // Read lock
    appOps->audioTypeReadLock(audioType, __FUNCTION__);

    for (int scene = 0; scene < (int)(sceneList->size()); scene++) {
        for (int stream = GAIN_MIN_STREAM_TYPE; stream <= GAIN_MAX_STREAM_TYPE; stream++) {
            for (int device = 0; device < NUM_GAIN_DEVICE; device++) {
                // get param unit using param path
                std::string paramPath = "Scene," +
                                        (*sceneList)[scene] +
                                        ",Volume type," +
                                        volumeTypeName[stream] +
                                        ",Profile," +
                                        profileName[device];
                ALOGV("paramPath = %s", paramPath.c_str());

                ParamUnit *paramUnit;
                paramUnit = appOps->audioTypeGetParamUnit(audioType, paramPath.c_str());
                if (!paramUnit) {
                    ALOGV("warn: get paramUnit fail, paramPath = %s", paramPath.c_str());
                    continue;
                }
                Param *param_hs;
                param_hs = appOps->paramUnitGetParamByName(paramUnit, paramHsName);
                if (!param_hs) {
                    ALOGW("warn: get param_hs fail");
                    continue;
                }
                Param *param_spk;
                param_spk = appOps->paramUnitGetParamByName(paramUnit, paramSpkName);
                if (!param_spk) {
                    ALOGW("warn: get param_spk fail");
                    continue;
                }
                Param *param_rcv;
                param_rcv = appOps->paramUnitGetParamByName(paramUnit, paramRcvName);
                if (!param_rcv) {
                    ALOGW("warn: get param_rcv fail");
                    continue;
                }

                // TODO: check if param is in range using checlist field
                if (param_hs->arraySize != 1 || param_spk->arraySize != 1 || param_rcv->arraySize != 1) {
                    ALOGW("warn: %s arraySize(%zu) != 1 || %s arraySize(%zu) != 1|| %s arraySize(%zu) != 1",
                          paramHsName,
                          param_hs->arraySize,
                          paramSpkName,
                          param_spk->arraySize,
                          paramRcvName,
                          param_rcv->arraySize);
                }

                // analog is the same for different volume level for normal playbck
                for (int i = 0; i < GAIN_VOL_INDEX_SIZE; i++) {
                    unsigned char *analog = _gainTable->sceneGain[scene].streamGain[stream][device][i].analog;
                    if (*(short *)param_spk->data >= 0 && mSpec.spkAnaType >= 0 && mSpec.spkAnaType < NUM_GAIN_ANA_TYPE) {
                        analog[mSpec.spkAnaType] = *(short *)param_spk->data;
                        ALOGV("\tscene = %d, stream = %d, device = %d, i = %d, analog[mSpec.spkAnaType] = %d\n",
                                scene, stream, device, i, analog[mSpec.spkAnaType]);
                    }
                    if (*(short *)param_rcv->data >= 0) {
                        analog[GAIN_ANA_HANDSET] = *(short *)param_rcv->data;
                        ALOGV("\tscene = %d, stream = %d, device = %d, i = %d, analog[GAIN_ANA_HANDSET] = %d\n",
                                scene, stream, device, i, analog[GAIN_ANA_HANDSET]);
                    }
                    if (*(short *)param_hs->data >= 0) {
                        analog[GAIN_ANA_HEADPHONE] = *(short *)param_hs->data;
                        ALOGV("\tscene = %d, stream = %d, device = %d, i = %d, analog[GAIN_ANA_HEADPHONE] = %d\n",
                                scene, stream, device, i, analog[GAIN_ANA_HEADPHONE]);
                    }
                }
            }
        }
    }
    // Unlock
    appOps->audioTypeUnlock(audioType);

    return NO_ERROR;
}

status_t GainTableParamParser::updateSpeechVol(GainTableParam *_gainTable) {
    ALOGD("%s()", __FUNCTION__);

    // define xml names
    char audioTypeName[] = SPEECH_VOL_AUDIOTYPE_NAME;
    char paramStfName[] = "stf_gain";
    char paramUlName[] = "ul_gain";
    char paramDlName[] = "dl_gain";

    const std::string *profileName = gppDeviceXmlName;
    const std::string *bandName = gppBandXmlName;
    const std::string *netName = gppNetXmlName;

    // extract parameters from xml
    AudioType *audioType;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, audioTypeName);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", audioTypeName);
        return BAD_VALUE;
    }

    // Read lock
    appOps->audioTypeReadLock(audioType, __FUNCTION__);

    for (int net = 0; net < NUM_GAIN_SPEECH_NETWORK; net++) {
        for (int band = 0; band < NUM_GAIN_SPEECH_BAND; band++) {
            for (int device = 0; device < NUM_GAIN_DEVICE; device++) {
                // get param unit using param path
                std::string paramPath = "Band," +
                                        bandName[band] +
                                        ",Profile," +
                                        profileName[device] +
                                        ",Network," +
                                        netName[net];
                ALOGG("paramPath = %s", paramPath.c_str());

                ParamUnit *paramUnit;
                paramUnit = appOps->audioTypeGetParamUnit(audioType, paramPath.c_str());
                if (!paramUnit) {
                    ALOGV("warn: get paramUnit fail, paramPath = %s", paramPath.c_str());
                    continue;
                }

                // Sidetone gain
                Param *param_stf_gain;
                param_stf_gain = appOps->paramUnitGetParamByName(paramUnit, paramStfName);
                if (!param_stf_gain) {
                    ALOGW("error: get param_stf_gain fail");
                    continue;
                }

                if (*(short *)param_stf_gain->data > mSpec.sidetoneIdxMax ||
                    *(short *)param_stf_gain->data < mSpec.sidetoneIdxMin) {
                    ALOGW("error, band %d, device %d, stf_gain = %d out of bound", band, device, *(short *)param_stf_gain->data);
                }
                _gainTable->nonSceneGain.sidetoneGain[band][net][device].gain = *(short *)param_stf_gain->data;

                // Uplink gain
                Param *param_ul_gain;
                param_ul_gain = appOps->paramUnitGetParamByName(paramUnit, paramUlName);
                if (!param_ul_gain) {
                    ALOGW("error: get param_ul_gain fail");
                    continue;
                }

                if (*(int *)param_ul_gain->data > mSpec.micIdxMax[device] ||
                    *(int *)param_ul_gain->data < mSpec.micIdxMin[device]) {
                    ALOGW("error, ul_gain = %d out of bound, band %d, device %d", *(int *)param_ul_gain->data, band, device);
                }
                _gainTable->nonSceneGain.speechMicGain[band][net][device].gain = *(int *)param_ul_gain->data;

                // Downlink gain
                Param *param_dl_gain;
                param_dl_gain = appOps->paramUnitGetParamByName(paramUnit, paramDlName);
                if (!param_dl_gain) {
                    ALOGW("error: get param_dl_gain fail");
                    continue;
                }

                short *shortArray = (short *)param_dl_gain->data;
                int arraySize = param_dl_gain->arraySize;
                if (arraySize + 1 > GAIN_VOL_INDEX_SIZE) {
                    ALOGW("error, param->arraySize + 1 %d exceed digital array size %d", arraySize, GAIN_VOL_INDEX_SIZE);
                    arraySize = GAIN_VOL_INDEX_SIZE - 1;
                }

                if (mMapDlDigital[device].size() == 0 ||
                    mMapDlAnalog[device].size() == 0 ||
                    mMapDlDigital[device].size() != mMapDlAnalog[device].size()) {
                    ALOGE("error, digi & analog map size = %zu & %zu", mMapDlDigital[device].size(),
                          mMapDlAnalog[device].size());
                    continue;
                }

                // xml 0~6 map to 1~7 here, index 0 is hard code mute
                for (int i = 0; i < arraySize + 1; i++) {
                    short dl_idx, digital, analog;

                    if (i == 0) {
                        dl_idx = shortArray[i];
                        digital = -64;
                        analog = mMapDlAnalog[device][dl_idx];
                    } else {
                        dl_idx = shortArray[i - 1];
                        digital = mMapDlDigital[device][dl_idx];
                        analog = mMapDlAnalog[device][dl_idx];
                    }
                    // set digital gain

                    // convert 0~-64 dB to 0~255
                    if (digital > mSpec.digiDbMax) {
                        ALOGW("error, param out of range, val %d > %d", digital, mSpec.digiDbMax);
                        _gainTable->nonSceneGain.speechGain[band][net][device][i].digital = 0;
                    } else if (digital <= mSpec.digiDbMin) {
                        ALOGV("error, param out of range, val %d <= %d", digital, mSpec.digiDbMin);
                        _gainTable->nonSceneGain.speechGain[band][net][device][i].digital = mSpec.keyVolumeStep;
                    } else {
                        _gainTable->nonSceneGain.speechGain[band][net][device][i].digital = (digital * -1 * mSpec.keyStepPerDb);
                    }

                    // set analog gain
                    if (mMapDlAnalogType[device] < 0 || mMapDlAnalogType[device] >= NUM_GAIN_ANA_TYPE) {
                        if (i == 0) {
                            ALOGG("\tcontinue, paramPath = %s, mMapDlAnalogType[%d] = %d",
                                  paramPath.c_str(), device, mMapDlAnalogType[device]);
                        }
                        continue;
                    }

                    if (mMapDlAnalogType[device] == GAIN_ANA_SPEAKER) {
                        _gainTable->nonSceneGain.speechGain[band][net][device][i].analog[mMapDlAnalogType[device]] = spkGainDb2Idx(analog);
                    } else if (mMapDlAnalogType[device] == GAIN_ANA_LINEOUT) {
                        _gainTable->nonSceneGain.speechGain[band][net][device][i].analog[mMapDlAnalogType[device]] = lineoutBufferGainDb2Idx(analog);
                    } else if (mMapDlAnalogType[device] == GAIN_ANA_HEADPHONE) {
                        _gainTable->nonSceneGain.speechGain[band][net][device][i].analog[mMapDlAnalogType[device]] = audioBufferGainDb2Idx(analog);
                    } else { // if (mMapDlAnalogType[device] == GAIN_ANA_HANDSET)
                        _gainTable->nonSceneGain.speechGain[band][net][device][i].analog[mMapDlAnalogType[device]] = voiceBufferGainDb2Idx(analog);
                    }

                    ALOGV("\tvol_idx = %d, dl_gain_idx = %d, digital = %d, analog = %d, digitaldB = %d, analogdB = %d\n",
                          i,
                          dl_idx,
                          _gainTable->nonSceneGain.speechGain[band][net][device][i].digital,
                          _gainTable->nonSceneGain.speechGain[band][net][device][i].analog[mMapDlAnalogType[device]],
                          digital,
                          analog);
                }
            }
        }
    }
    // Unlock
    appOps->audioTypeUnlock(audioType);

    return NO_ERROR;
}

status_t GainTableParamParser::updateRecordVol(GainTableParam *_gainTable, std::vector<std::string> *sceneList) {
    ALOGD("%s()", __FUNCTION__);

    // define xml names
    char audioTypeName[] = REC_VOL_AUDIOTYPE_NAME;
    const std::string *micModeName = gppMicModeXmlName;
    const std::string *profileName = gppDeviceXmlName;

    // extract parameters from xml
    AudioType *audioType;
    ParamUnit *paramUnit;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, audioTypeName);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", audioTypeName);
        return BAD_VALUE;
    }

    // Read lock
    appOps->audioTypeReadLock(audioType, __FUNCTION__);

    for (int scene = 0; scene < (int)(sceneList->size()); scene++) {
        for (int mode = 0; mode < NUM_GAIN_MIC_MODE; mode++) {
            for (int device = 0; device < NUM_GAIN_DEVICE; device++) {
                // get param unit using param path
                std::string paramPath = "Scene," +
                                        (*sceneList)[scene] +
                                        ",Application," +
                                        micModeName[mode] +
                                        ",Profile," +
                                        profileName[device];
                ALOGV("paramPath = %s", paramPath.c_str());

                paramUnit = appOps->audioTypeGetParamUnit(audioType, paramPath.c_str());
                if (!paramUnit) {
                    ALOGV("warn: get paramUnit fail, paramPath = %s", paramPath.c_str());
                    continue;
                }

                // Uplink gain
                Param *param_ul_gain;
                param_ul_gain = appOps->paramUnitGetParamByName(paramUnit, "ul_gain");
                if (!param_ul_gain) {
                    ALOGW("error: get param_ul_gain fail");
                    continue;
                }

                if (*(int *)param_ul_gain->data > mSpec.micIdxMax[device] ||
                    *(int *)param_ul_gain->data < mSpec.micIdxMin[device]) {
                    ALOGG("error, ul_gain = %d out of bound, paramPath = %s", *(int *)param_ul_gain->data, paramPath.c_str());
                }
                _gainTable->sceneGain[scene].micGain[mode][device].gain = *(int *)param_ul_gain->data;
                ALOGV("\tscene = %d, mode = %d, device = %d, gain = %d\n",
                        scene, mode, device, _gainTable->sceneGain[scene].micGain[mode][device].gain);
            }
        }
    }
    // Unlock
    appOps->audioTypeUnlock(audioType);

    return NO_ERROR;
}

status_t GainTableParamParser::updateVoIPVol(GainTableParam *_gainTable, std::vector<std::string> *sceneList) {
    ALOGD("%s()", __FUNCTION__);

    // define xml names
    char audioTypeName[] = VOIP_VOL_AUDIOTYPE_NAME;
    char paramUlName[] = "ul_gain";
    char paramDlName[] = "dl_gain";
    const std::string *profileName = gppDeviceXmlName;

    // extract parameters from xml
    AudioType *audioType;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, audioTypeName);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", audioTypeName);
        return BAD_VALUE;
    }

    // Read lock
    appOps->audioTypeReadLock(audioType, __FUNCTION__);

    for (int scene = 0; scene < (int)(sceneList->size()); scene++) {
        for (int device = 0; device < NUM_GAIN_DEVICE; device++) {
            // get param unit using param path
            std::string paramPath = "Scene," +
                                    (*sceneList)[scene] +
                                    ",Profile," +
                                    profileName[device];
            ALOGV("paramPath = %s", paramPath.c_str());

            ParamUnit *paramUnit;
            paramUnit = appOps->audioTypeGetParamUnit(audioType, paramPath.c_str());
            if (!paramUnit) {
                ALOGG("error: get paramUnit fail, paramPath = %s", paramPath.c_str());
                continue;
            }

            // Uplink gain
            const GAIN_MIC_MODE micType = GAIN_MIC_VOICE_COMMUNICATION;

            if (micType >= 0 && micType < NUM_GAIN_MIC_MODE) {
                Param *param_ul_gain;
                param_ul_gain = appOps->paramUnitGetParamByName(paramUnit, paramUlName);
                if (!param_ul_gain) {
                    ALOGW("error: get param_ul_gain fail, param name = %s", paramUlName);
                    continue;
                }

                if (*(int *)param_ul_gain->data > mSpec.micIdxMax[device] ||
                    *(int *)param_ul_gain->data < mSpec.micIdxMin[device]) {
                    ALOGW("error, ul_gain = %d out of bound, device %d", *(int *)param_ul_gain->data, device);
                }
                _gainTable->sceneGain[scene].micGain[micType][device].gain = *(int *)param_ul_gain->data;
                ALOGV("\tscene = %d, micType = %d, device = %d, gain = %d\n",
                        scene, micType, device, _gainTable->sceneGain[scene].micGain[micType][device].gain);
            }

            // Downlink gain
            // convert xml param to gain table
            Param *param_dl_gain;
            param_dl_gain = appOps->paramUnitGetParamByName(paramUnit, paramDlName);
            if (!param_dl_gain) {
                ALOGW("error: get param_dl_gain fail, param name = %s", paramDlName);
                continue;
            }

            short *shortArray = (short *)param_dl_gain->data;
            int arraySize = param_dl_gain->arraySize;
            if (arraySize + 1 > GAIN_VOL_INDEX_SIZE) {
                ALOGW("error, param->arraySize + 1 %d exceed digital array size %d", arraySize, GAIN_VOL_INDEX_SIZE);
                arraySize = GAIN_VOL_INDEX_SIZE - 1;
            }

            if (mMapDlDigital[device].size() == 0 ||
                mMapDlAnalog[device].size() == 0 ||
                mMapDlDigital[device].size() != mMapDlAnalog[device].size()) {
                ALOGE("error, digi & analog map size = %zu & %zu", mMapDlDigital[device].size(),
                      mMapDlAnalog[device].size());
                continue;
            }

            // xml 0~6 map to 1~7 here, 0 is hard code mute
            for (int i = 0; i < arraySize + 1; i++) {
                short dl_idx, digital, analog;

                if (i == 0) {
                    dl_idx = shortArray[i];
                    digital = -64;
                    analog = mMapDlAnalog[device][dl_idx];
                } else {
                    dl_idx = shortArray[i - 1];
                    digital = mMapDlDigital[device][dl_idx];
                    analog = mMapDlAnalog[device][dl_idx];
                }

                // set digital gain
                // convert 0~-64 dB to 0~255
                if (digital > mSpec.digiDbMax) {
                    ALOGW("error, param out of range, val %d > %d", digital, mSpec.digiDbMax);
                    _gainTable->sceneGain[scene].streamGain[AUDIO_STREAM_VOICE_CALL][device][i].digital = 0;
                } else if (digital <= mSpec.digiDbMin) {
                    ALOGV("error, param out of range, val %d <= %d", digital, mSpec.digiDbMin);
                    _gainTable->sceneGain[scene].streamGain[AUDIO_STREAM_VOICE_CALL][device][i].digital = mSpec.keyVolumeStep;
                } else {
                    _gainTable->sceneGain[scene].streamGain[AUDIO_STREAM_VOICE_CALL][device][i].digital = (digital * -1 * mSpec.keyStepPerDb);
                }

                // set analog gain
                if (mMapDlAnalogType[device] < 0 || mMapDlAnalogType[device] >= NUM_GAIN_ANA_TYPE) {
                    if (i == 0) {
                        ALOGG("\tcontinue, paramPath = %s, mMapDlAnalogType[%d] = %d",
                              paramPath.c_str(), device, mMapDlAnalogType[device]);
                    }
                    continue;
                }

                if (mMapDlAnalogType[device] == GAIN_ANA_SPEAKER) {
                    _gainTable->sceneGain[scene].streamGain[AUDIO_STREAM_VOICE_CALL][device][i].analog[mMapDlAnalogType[device]] = spkGainDb2Idx(analog);
                } else if (mMapDlAnalogType[device] == GAIN_ANA_LINEOUT) {
                    _gainTable->sceneGain[scene].streamGain[AUDIO_STREAM_VOICE_CALL][device][i].analog[mMapDlAnalogType[device]] = lineoutBufferGainDb2Idx(analog);
                } else if (mMapDlAnalogType[device] == GAIN_ANA_HEADPHONE) {
                    _gainTable->sceneGain[scene].streamGain[AUDIO_STREAM_VOICE_CALL][device][i].analog[mMapDlAnalogType[device]] = audioBufferGainDb2Idx(analog);
                } else { // if (mMapDlAnalogType[device] == GAIN_ANA_HANDSET)
                    _gainTable->sceneGain[scene].streamGain[AUDIO_STREAM_VOICE_CALL][device][i].analog[mMapDlAnalogType[device]] = voiceBufferGainDb2Idx(analog);
                }

                ALOGV("\tvol_idx = %d, dl_gain_idx = %d, digital = %d, analog = %d, digitaldB = %d, analogdB = %d\n",
                      i,
                      dl_idx,
                      _gainTable->sceneGain[scene].streamGain[AUDIO_STREAM_VOICE_CALL][device][i].digital,
                      _gainTable->sceneGain[scene].streamGain[AUDIO_STREAM_VOICE_CALL][device][i].analog[mMapDlAnalogType[device]],
                      digital,
                      analog);

            }
        }
    }
    // Unlock
    appOps->audioTypeUnlock(audioType);

    return NO_ERROR;
}

status_t GainTableParamParser::updateRingbackVol(GainTableParam *_gainTable) {
    ALOGD("%s()", __FUNCTION__);

    // define xml names
    char audioTypeName[] = RINGBACK_VOL_AUDIOTYPE_NAME;
    char paramRingbackName[] = "ringback_gain";
    const std::string *profileName = gppDeviceXmlName;

    // Initialize ringback tone with voice stream gain
    int streamDevice = 0;
    for (int device = 0; device < NUM_GAIN_DEVICE; device++) {
        // Using speaker stream gain to initialize hs + spk ringback tone gain
        if (device != GAIN_DEVICE_HSSPK) {
            streamDevice = device;
        } else {
            streamDevice = GAIN_DEVICE_SPEAKER;
        }
        for (int i = 0; i < GAIN_VOL_INDEX_SIZE; i++) {
            _gainTable->nonSceneGain.ringbackToneGain[device][i].digital =
                    _gainTable->sceneGain[GAIN_SCENE_INDEX_DEFAULT].streamGain[AUDIO_STREAM_VOICE_CALL][streamDevice][i].digital;
        }
    }

    // extract parameters from xml
    AudioType *audioType;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, audioTypeName);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", audioTypeName);
        return BAD_VALUE;
    }

    // Read lock
    appOps->audioTypeReadLock(audioType, __FUNCTION__);

    for (int device = 0; device < NUM_GAIN_DEVICE; device++) {
        // get param unit using param path
        std::string paramPath = "Profile," +
                                profileName[device];
        ALOGV("paramPath = %s", paramPath.c_str());

        ParamUnit *paramUnit;
        paramUnit = appOps->audioTypeGetParamUnit(audioType, paramPath.c_str());
        if (!paramUnit) {
            ALOGG("warn: get paramUnit fail, paramPath = %s", paramPath.c_str());
            continue;
        }

        // Ringback tone gain
        // convert xml param to gain table
        Param *param_ringback_gain;
        param_ringback_gain = appOps->paramUnitGetParamByName(paramUnit, paramRingbackName);
        if (!param_ringback_gain) {
            ALOGW("error: get param_ringback_gain fail, param name = %s", paramRingbackName);
            continue;
        }

        short *shortArray = (short *)param_ringback_gain->data;
        int arraySize = param_ringback_gain->arraySize;
        if (arraySize + 1 > GAIN_VOL_INDEX_SIZE) {
            ALOGW("error, param->arraySize + 1 %d exceed digital array size %d", arraySize, GAIN_VOL_INDEX_SIZE);
            arraySize = GAIN_VOL_INDEX_SIZE - 1;
        }

        if (mMapDlDigital[device].size() == 0) {
            ALOGE("error, digi = %zu", mMapDlDigital[device].size());
            continue;
        }

        // xml 0~6 map to 1~7 here, 0 is hard code mute
        for (int i = 0; i < arraySize + 1; i++) {
            short dl_idx, digital;

            if (i == 0) {
                dl_idx = shortArray[i];
                digital = -64;
            } else {
                dl_idx = shortArray[i - 1];
                digital = mMapDlDigital[device][dl_idx];
            }

            // set digital gain
            // convert 0~-64 dB to 0~255
            if (digital > mSpec.digiDbMax) {
                ALOGW("error, param out of range, val %d > %d", digital, mSpec.digiDbMax);
                _gainTable->nonSceneGain.ringbackToneGain[device][i].digital = 0;
            } else if (digital <= mSpec.digiDbMin) {
                ALOGV("error, param out of range, val %d <= %d", digital, mSpec.digiDbMin);
                _gainTable->nonSceneGain.ringbackToneGain[device][i].digital = mSpec.keyVolumeStep;
            } else {
                _gainTable->nonSceneGain.ringbackToneGain[device][i].digital = (digital * -1 * mSpec.keyStepPerDb);
            }

            ALOGV("\tvol_idx = %d, dl_gain_idx = %d, digital = %d, digitaldB = %d\n",
                  i,
                  dl_idx,
                  _gainTable->nonSceneGain.ringbackToneGain[device][i].digital,
                  digital);

            // Fill HSSPK ringback gain with SPK
            if (device == GAIN_DEVICE_SPEAKER) {
                _gainTable->nonSceneGain.ringbackToneGain[GAIN_DEVICE_HSSPK][i].digital =
                        _gainTable->nonSceneGain.ringbackToneGain[device][i].digital;
            }
        }
    }

    // Unlock
    appOps->audioTypeUnlock(audioType);

    return NO_ERROR;
}

status_t GainTableParamParser::loadGainTableParam() {
    loadGainTableSpec();
    loadGainTableMapDl();
    loadGainTableMapUl();
    loadGainTableHpImpedance();
    return NO_ERROR;
}

status_t GainTableParamParser::loadGainTableSpec() {
    ALOGG("%s()", __FUNCTION__);

    // define xml names
    char audioTypeName[] = VOLUME_AUDIOTYPE_NAME;

    // extract parameters from xml
    AudioType *audioType;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, audioTypeName);
    if (!audioType) {
        ALOGE("error: get audioType fail, audioTypeName = %s", audioTypeName);
        return BAD_VALUE;
    }

    std::string paramPath = "VolumeParam,Common";

    ParamUnit *paramUnit;
    paramUnit = appOps->audioTypeGetParamUnit(audioType, paramPath.c_str());
    if (!paramUnit) {
        ALOGE("error: get paramUnit fail, paramPath = %s", paramPath.c_str());
        return BAD_VALUE;
    }

    // Read lock
    appOps->audioTypeReadLock(audioType, __FUNCTION__);

    // spec
    getParam<int>(paramUnit, &mSpec.keyStepPerDb, "step_per_db");
    ALOGV("mSpec.keyStepPerDb = %d", mSpec.keyStepPerDb);
    getParam<float>(paramUnit, &mSpec.keyDbPerStep, "db_per_step");
    ALOGV("mSpec.keyDbPerStep = %f", mSpec.keyDbPerStep);
    getParam<float>(paramUnit, &mSpec.keyVolumeStep, "volume_step");
    ALOGV("mSpec.keyVolumeStep = %f", mSpec.keyVolumeStep);

    getParam<int>(paramUnit, &mSpec.digiDbMax, "play_digi_range_max");
    ALOGV("mSpec.digiDbMax = %d", mSpec.digiDbMax);
    getParam<int>(paramUnit, &mSpec.digiDbMin, "play_digi_range_min");
    ALOGV("mSpec.digiDbMin = %d", mSpec.digiDbMin);
    getParam<int>(paramUnit, &mSpec.sidetoneIdxMax, "stf_idx_range_max");
    ALOGV("mSpec.sidetoneIdxMax = %d", mSpec.sidetoneIdxMax);
    getParam<int>(paramUnit, &mSpec.sidetoneIdxMin, "stf_idx_range_min");
    ALOGV("mSpec.sidetoneIdxMin = %d", mSpec.sidetoneIdxMin);

    getParam<int>(paramUnit, &mSpec.decRecMax, "dec_rec_max");
    ALOGV("mSpec.decRecMax = %d", mSpec.decRecMax);
    getParam<int>(paramUnit, &mSpec.decRecStepPerDb, "dec_rec_step_per_db");
    ALOGV("mSpec.decRecStepPerDb = %d", mSpec.decRecStepPerDb);

    getParam<int>(paramUnit, &mSpec.ulGainOffset, "ul_gain_offset");
    ALOGV("mSpec.ulGainOffset = %d", mSpec.ulGainOffset);
    getParam<int>(paramUnit, &mSpec.ulPgaGainMapMax, "ul_pga_gain_map_max");
    ALOGV("mSpec.ulPgaGainMapMax = %d", mSpec.ulPgaGainMapMax);
    getParam<int>(paramUnit, &mSpec.ulHwPgaIdxMax, "ul_hw_pga_max_idx");
    ALOGV("mSpec.ulHwPgaIdxMax = %d", mSpec.ulHwPgaIdxMax);

    // audio buffer gain spec
    getParamVector<short>(paramUnit, &mSpec.audioBufferGainDb, "audio_buffer_gain_db");
    getParamVector<short>(paramUnit, &mSpec.audioBufferGainIdx, "audio_buffer_gain_idx");
    getParamVector(paramUnit, &mSpec.audioBufferGainString, "audio_buffer_gain_string");

    getParam<int>(paramUnit, &mSpec.audioBufferGainPreferMaxIdx, "audio_buffer_gain_prefer_max_idx");
    getParam(paramUnit, &mSpec.audioBufLMixerName, "audio_buffer_l_mixer_name");
    getParam(paramUnit, &mSpec.audioBufRMixerName, "audio_buffer_r_mixer_name");
    ALOGD("%s(), mSpec.audioBufferGainPreferMaxIdx = %d, audioBufLMixerName = %s, audioBufRMixerName = %s",
          __FUNCTION__,
          mSpec.audioBufferGainPreferMaxIdx,
          mSpec.audioBufLMixerName.c_str(),
          mSpec.audioBufRMixerName.c_str());

    size_t db_size = mSpec.audioBufferGainDb.size();
    size_t idx_size = mSpec.audioBufferGainIdx.size();
    size_t str_size = mSpec.audioBufferGainString.size();

    if (db_size != idx_size || db_size != str_size) {
        ALOGW("warn: db & idx & str_size mapping array size is not the same, db.size()=%zu, idx.size()=%zu, str_size()=%zu",
              db_size,
              idx_size,
              str_size);
    }

    mSpec.numAudioBufferGainLevel = (db_size <= idx_size) ? db_size : idx_size;

    for (unsigned int i = 0; i < mSpec.numAudioBufferGainLevel; i++) {
        ALOGG("audio buffer, db = %d, idx = %d", mSpec.audioBufferGainDb[i], mSpec.audioBufferGainIdx[i]);
    }

    // voice buffer gain spec
    getParamVector<short>(paramUnit, &mSpec.voiceBufferGainDb, "voice_buffer_gain_db");
    getParamVector<short>(paramUnit, &mSpec.voiceBufferGainIdx, "voice_buffer_gain_idx");
    getParamVector(paramUnit, &mSpec.voiceBufferGainString, "voice_buffer_gain_string");
    getParam<int>(paramUnit, &mSpec.voiceBufferGainPreferMaxIdx, "voice_buffer_gain_prefer_max_idx");
    getParam(paramUnit, &mSpec.voiceBufMixerName, "voice_buffer_mixer_name");
    ALOGD("%s(), mSpec.voiceBufferGainPreferMaxIdx = %d, voiceBufMixerName = %s",
          __FUNCTION__,
          mSpec.voiceBufferGainPreferMaxIdx,
          mSpec.voiceBufMixerName.c_str());

    db_size = mSpec.voiceBufferGainDb.size();
    idx_size = mSpec.voiceBufferGainIdx.size();
    str_size = mSpec.voiceBufferGainString.size();

    if (db_size != idx_size || db_size != str_size) {
        ALOGW("warn: db & idx & str_size mapping array size is not the same, db.size()=%zu, idx.size()=%zu, str_size()=%zu",
              db_size,
              idx_size,
              str_size);
    }

    mSpec.numVoiceBufferGainLevel = (db_size <= idx_size) ? db_size : idx_size;

    for (unsigned int i = 0; i < mSpec.numVoiceBufferGainLevel; i++) {
        ALOGG("voice buffer, db = %d, idx = %d", mSpec.voiceBufferGainDb[i], mSpec.voiceBufferGainIdx[i]);
    }

    // lineout buffer gain spec
    getParamVector<short>(paramUnit, &mSpec.lineoutBufferGainDb, "lineout_buffer_gain_db");
    getParamVector<short>(paramUnit, &mSpec.lineoutBufferGainIdx, "lineout_buffer_gain_idx");
    getParamVector(paramUnit, &mSpec.lineoutBufferGainString, "lineout_buffer_gain_string");
    getParam<int>(paramUnit, &mSpec.lineoutBufferGainPreferMaxIdx, "lineout_buffer_gain_prefer_max_idx");
    ALOGD("%s(), mSpec.lineoutBufferGainPreferMaxIdx = %d", __FUNCTION__, mSpec.lineoutBufferGainPreferMaxIdx);

    db_size = mSpec.lineoutBufferGainDb.size();
    idx_size = mSpec.lineoutBufferGainIdx.size();
    str_size = mSpec.lineoutBufferGainString.size();

    if (db_size != idx_size || db_size != str_size) {
        ALOGW("warn: db & idx & str_size mapping array size is not the same, db.size()=%zu, idx.size()=%zu, str_size()=%zu",
              db_size,
              idx_size,
              str_size);
    }

    mSpec.numLineoutBufferGainLevel = (db_size <= idx_size) ? db_size : idx_size;

    for (unsigned int i = 0; i < mSpec.numLineoutBufferGainLevel; i++) {
        ALOGG("lineout buffer, db = %d, idx = %d", mSpec.lineoutBufferGainDb[i], mSpec.lineoutBufferGainIdx[i]);
    }

    // spk gain spec
    getParamVector<short>(paramUnit, &mSpec.spkGainDb, "spk_gain_db");
    getParamVector<short>(paramUnit, &mSpec.spkGainIdx, "spk_gain_idx");
    getParamVector(paramUnit, &mSpec.spkGainString, "spk_gain_string");

    if (AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        mSpec.spkAnaType = GAIN_ANA_NONE;
    } else {
        if (AudioSmartPaController::getInstance()->isSmartPADynamicDetectSupport()) {
            int spkType = AudioALSAHardwareResourceManager::getInstance()->getNonSmartPAType();
            switch (spkType) {
            case AUDIO_SPK_INTAMP:
                mSpec.spkAnaType = GAIN_ANA_SPEAKER;
                break;
            case AUDIO_SPK_EXTAMP_LO:
                mSpec.spkAnaType = GAIN_ANA_LINEOUT;
                break;
            case AUDIO_SPK_EXTAMP_HP:
                mSpec.spkAnaType = GAIN_ANA_HEADPHONE;
                break;
            default:
                ALOGW("error! default set ANA_LINEOUT\n");
                mSpec.spkAnaType = GAIN_ANA_LINEOUT;
                break;
            }
        } else {
            getParam<GAIN_ANA_TYPE>(paramUnit, &mSpec.spkAnaType, "spk_analog_type");
        }
    }

    getParam(paramUnit, &mSpec.spkLMixerName, "spk_l_mixer_name");
    getParam(paramUnit, &mSpec.spkRMixerName, "spk_r_mixer_name");
    ALOGD("%s(), mSpec.spkAnaType = %d, spkLMixerName = %s, spkRMixerName = %s",
          __FUNCTION__,
          mSpec.spkAnaType,
          mSpec.spkLMixerName.c_str(),
          mSpec.spkRMixerName.c_str());


    db_size = mSpec.spkGainDb.size();
    idx_size = mSpec.spkGainIdx.size();
    str_size = mSpec.spkGainString.size();

    if (db_size != idx_size || db_size != str_size) {
        ALOGW("warn: db & idx & str_size mapping array size is not the same, db.size()=%zu, idx.size()=%zu, str_size()=%zu",
              db_size,
              idx_size,
              str_size);
    }

    mSpec.numSpkGainLevel = (db_size <= idx_size) ? db_size : idx_size;

    for (unsigned int i = 0; i < mSpec.numSpkGainLevel; i++) {
        ALOGG("spk, db = %d, idx = %d", mSpec.spkGainDb[i], mSpec.spkGainIdx[i]);
    }

    // ul gain map
    getParamVector(paramUnit, &mSpec.ulPgaGainString, "ul_pga_gain_string");
    getParam(paramUnit, &mSpec.ulPgaLMixerName, "ul_pga_l_mixer_name");
    getParam(paramUnit, &mSpec.ulPgaRMixerName, "ul_pga_r_mixer_name");
    ALOGD("%s(), mSpec.ulPgaLMixerName = %s, ulPgaRMixerName = %s",
          __FUNCTION__,
          mSpec.ulPgaLMixerName.c_str(), mSpec.ulPgaRMixerName.c_str());


    // stf gain map
    getParamVector<short>(paramUnit, &mSpec.stfGainMap, "stf_gain_map");
    if ((int)mSpec.stfGainMap.size() != mSpec.sidetoneIdxMax + 1) {
        ALOGW("warn: stfGainMap.size %zu != sidetoneIdxMax %d + 1",
              mSpec.stfGainMap.size(),
              mSpec.sidetoneIdxMax);
    }

    // Unlock
    appOps->audioTypeUnlock(audioType);

    return NO_ERROR;
}


status_t GainTableParamParser::loadGainTableMapDl() {
    ALOGD("%s()", __FUNCTION__);

    // define xml names
    char audioTypeName[] = GAIN_MAP_AUDIOTYPE_NAME;
    char paramTotalName[] = "dl_total_gain";
    char paramDigitalName[] = "dl_digital_gain";
    char paramAnalogName[] = "dl_analog_gain";
    char paramAnaTypeName[] = "dl_analog_type";

    const std::string *profileName = gppDeviceXmlName;

    // extract parameters from xml
    AudioType *audioType;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, audioTypeName);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", audioTypeName);
        return BAD_VALUE;
    }

    // Read lock
    appOps->audioTypeReadLock(audioType, __FUNCTION__);

    for (int device = 0; device < NUM_GAIN_DEVICE; device++) {
        // get param unit using param path
        std::string paramPath = "Profile," +
                                profileName[device];
        ALOGV("paramPath = %s", paramPath.c_str());

        ParamUnit *paramUnit;
        paramUnit = appOps->audioTypeGetParamUnit(audioType, paramPath.c_str());
        if (!paramUnit) {
            ALOGV("error: get paramUnit fail, paramPath = %s", paramPath.c_str());
            continue;
        }
        Param *param_total;
        param_total = appOps->paramUnitGetParamByName(paramUnit, paramTotalName);
        if (!param_total) {
            ALOGW("error: get param_total fail, param_name = %s", paramTotalName);
            continue;
        }

        Param *param_digital;
        param_digital = appOps->paramUnitGetParamByName(paramUnit, paramDigitalName);
        if (!param_digital) {
            ALOGW("error: get param_digital fail, param_name = %s", paramDigitalName);
            continue;
        }
        Param *param_analog;
        param_analog = appOps->paramUnitGetParamByName(paramUnit, paramAnalogName);
        if (!param_analog) {
            ALOGW("error: get param_analog fail, param_name = %s", paramAnalogName);
            continue;
        }
        Param *param_ana_type;
        param_ana_type = appOps->paramUnitGetParamByName(paramUnit, paramAnaTypeName);
        if (!param_ana_type) {
            ALOGW("error: get param_ana_type fail, param_name = %s", paramAnaTypeName);
            continue;
        }
        mMapDlAnalogType[device] = (GAIN_ANA_TYPE) * (int *)param_ana_type->data;

        if (param_digital->arraySize != param_analog->arraySize) {
            ALOGE("error: digi & ana mapping array size is not the same, digi.size()=%zu, ana.size()=%zu", param_digital->arraySize, param_analog->arraySize);
            continue;
        }

        if (param_total->arraySize != param_digital->arraySize) {
            ALOGW("error, total gain && digi & ana array size does not match, total.size()=%zu, digi.size()=%zu", param_total->arraySize, param_digital->arraySize);
        }

        short *digital_raw = (short *)param_digital->data;
        mMapDlDigital[device].assign(digital_raw, digital_raw + param_digital->arraySize);

        short *analog_raw = (short *)param_analog->data;
        mMapDlAnalog[device].assign(analog_raw, analog_raw + param_analog->arraySize);

        for (unsigned int i = 0; i < mMapDlDigital[device].size(); i++) {
            ALOGV("digi = %d, ana = %d", mMapDlDigital[device][i], mMapDlAnalog[device][i]);
        }
    }

    // Unlock
    appOps->audioTypeUnlock(audioType);

    return NO_ERROR;

}

status_t GainTableParamParser::loadGainTableMapUl() {
    ALOGD("%s()", __FUNCTION__);

    // define xml names
    char audioTypeName[] = GAIN_MAP_UL_AUDIOTYPE_NAME;
    char paramSwagcMapName[] = "swagc_gain_map";
    char paramSwagcMapDmicName[] = "swagc_gain_map_dmic";
    char paramUlPgaName[] = "ul_pga_gain_map";

    const std::string *profileName = gppDeviceXmlName;

    // extract parameters from xml
    AudioType *audioType;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, audioTypeName);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", audioTypeName);
        return BAD_VALUE;
    }

    // Read lock
    appOps->audioTypeReadLock(audioType, __FUNCTION__);

    for (int device = 0; device < NUM_GAIN_DEVICE; device++) {
        // get param unit using param path
        std::string paramPath = "Profile," +
                                profileName[device];
        ALOGG("paramPath = %s", paramPath.c_str());

        ParamUnit *paramUnit;
        paramUnit = appOps->audioTypeGetParamUnit(audioType, paramPath.c_str());
        if (!paramUnit) {
            ALOGV("error: get paramUnit fail, paramPath = %s", paramPath.c_str());
            continue;
        }

        Param *param_swagc;
        param_swagc = appOps->paramUnitGetParamByName(paramUnit, paramSwagcMapName);
        if (!param_swagc) {
            ALOGW("error: get param_swagc fail, param_name = %s", paramSwagcMapName);
            continue;
        }

        Param *param_swagc_dmic;
        param_swagc_dmic = appOps->paramUnitGetParamByName(paramUnit, paramSwagcMapDmicName);
        if (!param_swagc_dmic) {
            ALOGW("error: get param_swagc_dmic fail, param_name = %s", paramSwagcMapDmicName);
            continue;
        }

        Param *param_ul_pga;
        param_ul_pga = appOps->paramUnitGetParamByName(paramUnit, paramUlPgaName);
        if (!param_ul_pga) {
            ALOGW("error: get param_ul_pga fail, param_name = %s", paramUlPgaName);
            continue;
        }

        getParam<int>(paramUnit, &mSpec.micIdxMax[device], "mic_idx_range_max");
        ALOGG("mSpec.micIdxMax[%d] = %d", device, mSpec.micIdxMax[device]);

        getParam<int>(paramUnit, &mSpec.micIdxMin[device], "mic_idx_range_min");
        ALOGG("mSpec.micIdxMin[%d] = %d", device, mSpec.micIdxMin[device]);

        if (param_swagc->arraySize != param_ul_pga->arraySize ||
            param_swagc->arraySize != param_swagc_dmic->arraySize) {
            ALOGW("error, swagc gain && ul_pga array size does not match, swagc.size()=%zu, pga.size()=%zu, swagc_dmic.size()=%zu", param_swagc->arraySize, param_ul_pga->arraySize, param_swagc_dmic->arraySize);
        }

        short *swagc_raw = (short *)param_swagc->data;
        mSpec.swagcGainMap[device].assign(swagc_raw, swagc_raw + param_swagc->arraySize);

        short *swagc_dmic_raw = (short *)param_swagc_dmic->data;
        mSpec.swagcGainMapDmic[device].assign(swagc_dmic_raw, swagc_dmic_raw + param_swagc_dmic->arraySize);

        short *pga_raw = (short *)param_ul_pga->data;
        mSpec.ulPgaGainMap[device].assign(pga_raw, pga_raw + param_ul_pga->arraySize);

        for (unsigned int i = 0; i < mSpec.swagcGainMap[device].size(); i++) {
            ALOGV("swagc = %d, swagc_dmic = %d, pga = %d", mSpec.swagcGainMap[device][i], mSpec.swagcGainMapDmic[device][i], mSpec.ulPgaGainMap[device][i]);
        }
    }

    // Unlock
    appOps->audioTypeUnlock(audioType);

    return NO_ERROR;
}

status_t GainTableParamParser::loadGainTableHpImpedance() {
    ALOGG("%s()", __FUNCTION__);

    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(false);
        return UNKNOWN_ERROR;
    }

    // define xml names
    char audioTypeName[] = HP_IMPEDANCE_AUDIOTYPE_NAME;

    // extract parameters from xml
    AudioType *audioType;
    audioType = appOps->appHandleGetAudioTypeByName(appOps->appHandleGetInstance(), audioTypeName);
    if (!audioType) {
        ALOGE("%s(), get audioType fail, audioTypeName = %s", __FUNCTION__, audioTypeName);
        return BAD_VALUE;
    }

    std::string paramCommonPath = "HpImpedance,Common";
    std::string paramPath = "HpImpedance,";
    const char *platformChar = appOps->appHandleGetFeatureOptionValue(appOps->appHandleGetInstance(), "MTK_PLATFORM");

    if (platformChar) {
        paramPath += std::string(platformChar);
    }

    ParamUnit *paramUnit;
    paramUnit = appOps->audioTypeGetParamUnit(audioType, paramPath.c_str());
    if (!paramUnit) {
        ALOGW("%s(), get paramUnit fail, paramPath = %s, use common", __FUNCTION__, paramPath.c_str());

        paramUnit = appOps->audioTypeGetParamUnit(audioType, paramCommonPath.c_str());
        if (!paramUnit) {
            ALOGE("%s(), get paramUnit fail, paramCommonPath = %s", __FUNCTION__, paramCommonPath.c_str());
            return BAD_VALUE;
        }
    }

    // Read lock
    appOps->audioTypeReadLock(audioType, __FUNCTION__);

    // headphone impedance related
    getParam<int>(paramUnit, &mSpec.hpImpEnable, "hp_impedance_enable");
    if (mSpec.hpImpEnable) {
        mSpec.hpImpOnBoardResistor = 0;
        getParam<int>(paramUnit, &mSpec.hpImpOnBoardResistor, "hp_impedance_onboard_resistor");
        getParam<int>(paramUnit, &mSpec.hpImpDefaultIdx, "hp_impedance_default_idx");
        ALOGD("%s(), mSpec.hpImpEnable = %d, mSpec.hpImpOnBoardResistor = %d, mSpec.hpImpDefaultIdx = %d",
              __FUNCTION__,
              mSpec.hpImpEnable,
              mSpec.hpImpOnBoardResistor,
              mSpec.hpImpDefaultIdx);
        getParamVector<short>(paramUnit, &mSpec.hpImpThresholdList, "hp_impedance_threshold_list");
        getParamVector<short>(paramUnit, &mSpec.hpImpCompensateList, "hp_impedance_gain_degrade_list");
        ASSERT(mSpec.hpImpThresholdList.size() == (mSpec.hpImpCompensateList.size() - 1));
        ASSERT(mSpec.hpImpThresholdList.size() != 0);
        ASSERT(mSpec.hpImpDefaultIdx >= 0 && mSpec.hpImpDefaultIdx < (int)mSpec.hpImpThresholdList.size());
    }

    // Unlock
    appOps->audioTypeUnlock(audioType);

    return NO_ERROR;
}

/*
 * Utility functions
 */
unsigned int GainTableParamParser::audioBufferGainDb2Idx(int dB) {
    for (unsigned int i = 0; i < mSpec.numAudioBufferGainLevel; i++) {
        if (dB == mSpec.audioBufferGainDb[i]) {
            return mSpec.audioBufferGainIdx[i];
        }
    }

    ALOGW("error, %s(), cannot find corresponding BufferGainIdx, return idx 0, %ddB", __FUNCTION__, mSpec.audioBufferGainDb[0]);
    return 0;
}

unsigned int GainTableParamParser::voiceBufferGainDb2Idx(int dB) {
    for (unsigned int i = 0; i < mSpec.numVoiceBufferGainLevel; i++) {
        if (dB == mSpec.voiceBufferGainDb[i]) {
            return mSpec.voiceBufferGainIdx[i];
        }
    }

    ALOGW("error, %s(), cannot find corresponding BufferGainIdx, return idx 0, %ddB", __FUNCTION__, mSpec.voiceBufferGainDb[0]);
    return 0;
}

unsigned int GainTableParamParser::lineoutBufferGainDb2Idx(int dB) {
    for (unsigned int i = 0; i < mSpec.numLineoutBufferGainLevel; i++) {
        if (dB == mSpec.lineoutBufferGainDb[i]) {
            return mSpec.lineoutBufferGainIdx[i];
        }
    }

    ALOGW("error, %s(), cannot find corresponding BufferGainIdx, return idx 0, %ddB", __FUNCTION__, mSpec.lineoutBufferGainDb[0]);
    return 0;
}

unsigned int GainTableParamParser::spkGainDb2Idx(int dB) {
    for (size_t i = 0; i < mSpec.numSpkGainLevel; i++) {
        if (dB == mSpec.spkGainDb[i]) {
            return mSpec.spkGainIdx[i];
        }
    }

    ALOGW("error, %s(), cannot find corresponding BufferGainIdx, return idx 1, %ddB", __FUNCTION__, mSpec.spkGainDb[1]);
    return 1;
}

GAIN_SPEECH_NETWORK GainTableParamParser::getGainSpeechNetwork(const char *name) {
    for (int i = 0; i < NUM_GAIN_SPEECH_NETWORK; i++) {
        if (strcmp(name, gppNetXmlName[i].c_str()) == 0) {
            return (GAIN_SPEECH_NETWORK)i;
        }
    }

    ALOGW("%s(), speech network not found, name %s, return 0", __FUNCTION__, name);

    return (GAIN_SPEECH_NETWORK)0;
}

template<class T>
status_t GainTableParamParser::getParam(ParamUnit *_paramUnit, T *_param, const char *_paramName) {
    Param *param;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    param = appOps->paramUnitGetParamByName(_paramUnit, _paramName);
    if (!param) {
        ALOGE("error: get param fail, param_name = %s", _paramName);
        return BAD_VALUE;
    } else {
        *_param = *(T *)param->data;
    }

    return NO_ERROR;
}

status_t GainTableParamParser::getParam(ParamUnit *_paramUnit, std::string *_param, const char *_paramName) {
    Param *param;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    param = appOps->paramUnitGetParamByName(_paramUnit, _paramName);
    if (!param) {
        ALOGE("error: get param fail, param_name = %s", _paramName);
        return BAD_VALUE;
    } else {
        if (param->paramInfo->dataType == TYPE_STR) {
            *_param = (char *)param->data;
        } else {
            ALOGW("warn, param->paramInfo->dataType %d != TYPE_STR %d", param->paramInfo->dataType, TYPE_STR);
            return BAD_VALUE;
        }
    }

    return NO_ERROR;
}


template<class T>
status_t GainTableParamParser::getParamVector(ParamUnit *_paramUnit, std::vector<T> *_param, const char *_paramName) {
    Param *param;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    param = appOps->paramUnitGetParamByName(_paramUnit, _paramName);
    if (!param) {
        ALOGE("error: get param fail, param_name = %s", _paramName);
        return BAD_VALUE;
    } else {
        T *raw = (T *)param->data;
        _param->assign(raw, raw + param->arraySize);
    }

    return NO_ERROR;
}

status_t GainTableParamParser::getParamVector(ParamUnit *_paramUnit, std::vector<std::string> *_param, const char *_paramName) {
    Param *param;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }
    param = appOps->paramUnitGetParamByName(_paramUnit, _paramName);
    if (!param) {
        ALOGE("error: get param fail, param_name = %s", _paramName);
        return BAD_VALUE;
    } else {
        if (param->paramInfo->dataType == TYPE_STR) {
            _param->clear();
            std::string raw((char *)param->data);
            ALOGV("%s = %s", _paramName, raw.c_str());

            ASSERT(!raw.empty());

            int pre_pos = -1;
            size_t find_pos = raw.find(',', pre_pos + 1);

            std::string sub_str = raw.substr(pre_pos + 1, find_pos - pre_pos - 1);
            do {
                _param->push_back(sub_str);
                ALOGV("\t%s", _param->back().c_str());
                if (find_pos == std::string::npos) {
                    break;
                }
                pre_pos = find_pos;
                find_pos = raw.find(',', pre_pos + 1);
                sub_str = raw.substr(pre_pos + 1, find_pos - pre_pos - 1);
            } while (!sub_str.empty());
        } else {
            ALOGW("warn, param->paramInfo->dataType %d != %d", param->paramInfo->dataType, TYPE_STR);
            return BAD_VALUE;
        }
    }

    return NO_ERROR;
}


}

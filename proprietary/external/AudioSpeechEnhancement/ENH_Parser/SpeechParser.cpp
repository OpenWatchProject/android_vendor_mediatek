#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechParser"
#include "SpeechParser.h"
#include "utstring.h"
#include <utils/Log.h>
#include <inttypes.h>
#include <media/AudioParameter.h>
#include <assert.h>



namespace android {
extern "C" {
#define XML_FOLDER "/system/etc/audio_param/"


#define MAX_BYTE_PARAM_SPEECH 4200
#define SPH_DUMP_STR_SIZE (500)

#define SPH_PARAM_UNIT_DUMP_STR_SIZE (1024)
    //--------------------------------------------------------------------------------
    //audio type: Speech
#define MAX_NUM_CATEGORY_TYPE_SPEECH 4
#define MAX_NUM_PARAM_SPEECH 3
    const String8 audioType_Speech_CategoryType[] = { String8("Band"), String8("Profile"), String8("VolIndex"), String8("Network") };
    const String8 audioType_Speech_ParamName[] = { String8("speech_mode_para"), String8("sph_in_fir"), String8("sph_out_fir"), String8("sph_in_iir_mic1_dsp"), String8("sph_in_iir_mic2_dsp"), String8("sph_in_iir_enh_dsp"), String8("sph_out_iir_enh_dsp") };

#define NUM_VOLUME_SPEECH 7
    const char audioType_Speech_CategoryName3[NUM_VOLUME_SPEECH][128] = { "0", "1", "2", "3", "4", "5", "6" };
    const char audioType_Speech_CategoryName2[PARSER_SPEECH_PROFILE_MAX_NUM][128] = {
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
    const String8 audioType_SpeechDMNR_CategoryType[] = { String8("Band"), String8("Profile") };
    const char audioType_SpeechDMNR_CategoryName2[2][128] = { "Handset", "MagiConference" };
    const String8 audioType_SpeechDMNR_ParamName[] = { String8("dmnr_para") };

    //--------------------------------------------------------------------------------
    //audio type: SpeechGeneral
#define MAX_NUM_CATEGORY_TYPE_SPEECH_GENERAL 1
#define MAX_NUM_PARAM_SPEECH_GENERAL 2
    const String8 audioType_SpeechGeneral_CategoryType[] = { String8("CategoryLayer") };
    const char audioType_SpeechGeneral_CategoryName1[1][128] = { "Common" };
    const String8 audioType_SpeechGeneral_ParamName[] = { String8("speech_common_para"), String8("debug_info") };

    //--------------------------------------------------------------------------------
    //audio type: SpeechMagiClarity
#define MAX_NUM_CATEGORY_TYPE_SPEECH_MAGICLARITY 1
#define MAX_NUM_PARAM_SPEECH_MAGICLARITY 1
    const String8 audioType_SpeechMagiClarity_CategoryType[] = { String8("CategoryLayer") };
    const char audioType_SpeechMagiClarity_CategoryName1[1][128] = { "Common" };
    const String8 audioType_SpeechMagiClarity_ParamName[] = { String8("shape_rx_fir_para") };

    //--------------------------------------------------------------------------------
    //audio type: SpeechNetwork
#define MAX_NUM_CATEGORY_TYPE_SPEECH_NETWORK 1
#define MAX_NUM_PARAM_SPEECH_NETWORK 1
    const String8 audioType_SpeechNetwork_CategoryType[] = { String8("Network") };
    const String8 audioType_SpeechNetwork_ParamName[] = { String8("speech_network_support") };


    //--------------------------------------------------------------------------------
    //audio type: SpeechEchoRef
#define MAX_NUM_CATEGORY_TYPE_SPEECH_ECHOREF 1
#define MAX_NUM_PARAM_SPEECH_ECHOREF 1
    const String8 audioType_SpeechEchoRef_CategoryType[] = { String8("Device") };
//    const char audioType_SpeechEchoRef_CategoryName1[1][128] = { "USBAudio" };
    const String8 audioType_SpeechEchoRef_ParamName[] = { String8("EchoRef_para") };

#define ENH_SPH_PARSER_VER 0x0001

    AppHandle *mAppHandle; // = appHandleGetInstance();
    uint8_t numSpeechNetwork, mSpeechParamVerFirst, mSpeechParamVerLast, numSpeechParam;
    PARSER_SPEECH_NETWORK_STRUCT mNameForEachSpeechNetwork[12];
    PARSER_SPEECH_PARAM_SUPPORT_STRUCT mSphParamSupport;

    PARSER_SPEECH_NETWORK_STRUCT mListSpeechNetwork[12];
    static short SpeechParserInited = 0;
    static short Parser_time_count = 0;

#define TESTMODE_MAX_LENGTH 20

    char TestMode[TESTMODE_MAX_LENGTH] = "Default";

    //      const uint32_t CommonInfo_offset = 9;
    //      const uint32_t DebugInfo_offset = 22;
    uint32_t MD_version = 0;

    static const uint32_t SD_SphParamSize = 0xC000;  // 48K
    /*==============================================================================
    *                     Lock
    *============================================================================*/
    static pthread_rwlock_t spHandleInstLock = PTHREAD_RWLOCK_INITIALIZER;

    EXPORT int spHandleInstWriteLock(const char *callerFun) {
        int res = 0;
        while (1) {
            if (pthread_rwlock_trywrlock(&spHandleInstLock) == 0) {
                //ALOGD("%s acquired the spHandleInstLock\n", callerFun);
                break;
            } else {
                ALOGE("Cannot lock the spHandleInstLock, delay some time. (the locker is %s)\n", callerFun);
                usleep(1);
            }
        }
        return res;
    }

    EXPORT int spHandleInstUnlock() {
        int res = 0;
        ALOGV("Unlock spHandleInst lock\n");
        res = pthread_rwlock_unlock(&spHandleInstLock);
        return res;
    }

    /*==============================================================================
    *                     Constructor / Destructor / Init / Deinit
    *============================================================================*/
    static SpHandle spHandleInst;
    static int spHandleInited = 0;

    EXPORT SP_STATUS spHandleInit(SpHandle * spHandle) {
        ALOGV("%s(), spHandle = 0x%p\n", __FUNCTION__, spHandle);
        return SP_NO_ERROR;
    }

    EXPORT SP_STATUS spHandleUninit(SpHandle * spHandle) {
        //ALOGD("%s(), spHandle = 0x%p\n", __FUNCTION__, spHandle);
        if (!spHandle) {
            ALOGW("SpHandle is NULL!\n");
            return SP_ERROR;
        } else {
            /* If spHandle is singleton instance, reset the init info */
            if (spHandle == &spHandleInst) {
                spHandleInited = 0;
            }

            return SP_NO_ERROR;
        }
    }
    /*==============================================================================
    *                     Singleton Pattern
    *============================================================================*/
    EXPORT SpHandle * spHandleGetInstance() {
        spHandleInstWriteLock(__FUNCTION__);

        if (!spHandleInited) {
            spHandleInit(&spHandleInst);
            spHandleInited = 1;
        }

        spHandleInstUnlock();

        return &spHandleInst;
    }
    /*==============================================================================
    *                     Method
    *============================================================================*/



    uint16_t sizeByteParaData(DATA_TYPE dataType, uint16_t arraySize) {

        uint16_t sizeUnit = 4;
        switch (dataType) {
        case TYPE_INT:
            sizeUnit = 4;
            break;
        case TYPE_UINT:
            sizeUnit = 4;
            break;
        case TYPE_FLOAT:
            sizeUnit = 4;
            break;
        case TYPE_BYTE_ARRAY:
            sizeUnit = arraySize;
            break;
        case TYPE_UBYTE_ARRAY:
            sizeUnit = arraySize;
            break;
        case TYPE_SHORT_ARRAY:
            sizeUnit = arraySize << 1;
            break;
        case TYPE_USHORT_ARRAY:
            sizeUnit = arraySize << 1;
            break;
        case TYPE_INT_ARRAY:
            sizeUnit = arraySize << 2;
            break;
        case TYPE_UINT_ARRAY:
            sizeUnit = arraySize << 2;
            break;
        default:
            ALOGE("%s(), Not an available dataType(%d)", __FUNCTION__, dataType);

            break;

        }

        ALOGV("-%s(), arraySize=%d, sizeUnit=%d", __FUNCTION__, arraySize, sizeUnit);
        return sizeUnit;
    }

    uint16_t SetMDParamDataHdr_SP(Category * cateBand, Category * cateNetwork) {
        uint16_t dataHeader = 0, MaskNetwork = 0;;

        if (cateBand->name != NULL) {
            if (strcmp(cateBand->name, "NB") == 0) { //All netwrok use
                dataHeader = 0x1000;
            } else if (strcmp(cateBand->name, "WB") == 0) {
                dataHeader = 0x2000;
            } else if (strcmp(cateBand->name, "SWB") == 0) {
                dataHeader = 0x3000;
            } else if (strcmp(cateBand->name, "FB") == 0) {
                dataHeader = 0x4000;
            }
        } else {
            dataHeader = 0x1000;
        }

        if (cateNetwork->name != NULL) {
            if (strcmp(cateNetwork->name, "GSM") == 0) {    //Mapping network to dataHdr
                MaskNetwork = 0x1;
            } else if (strcmp(cateNetwork->name, "WCDMA") == 0) {
                MaskNetwork = 0x2;
            } else if (strcmp(cateNetwork->name, "WCDMA_FDD") == 0) {
                MaskNetwork = 0x2;
            } else if (strcmp(cateNetwork->name, "VoLTE") == 0) {
                MaskNetwork = 0x4;
            } else if (strcmp(cateNetwork->name, "ViLTE") == 0) {
                MaskNetwork = 0x8;
            } else if (strcmp(cateNetwork->name, "C2K") == 0) {
                MaskNetwork = 0x10;
            } else if (strcmp(cateNetwork->name, "VoWifi") == 0) {
                MaskNetwork = 0x20;
            } else if (strcmp(cateNetwork->name, "Viwifi") == 0) {
                MaskNetwork = 0x40;
            } else if (strcmp(cateNetwork->name, "5G") == 0) {
                MaskNetwork = 0x80;
            }
        }

        dataHeader |= MaskNetwork;
        return dataHeader;
    }

    status_t SpeechDataDump_new(char *bufDump,  uint16_t idxSphType, const char *nameParam, const char *speechParamData) {

        if (nameParam == NULL) {
            return NO_ERROR;
        }

        ALOGV("+%s(), idxSphType=%d", __FUNCTION__, idxSphType);
        char sphDumpStr[SPH_DUMP_STR_SIZE] = { 0 };
        int idxDump = 0, sizeDump = 0, DataTypePrint = 0;
        //speech parameter dump

        switch (idxSphType) {
        case PARSER_AUDIO_TYPE_SPEECH: {
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
        case PARSER_AUDIO_TYPE_SPEECH_GENERAL: {
            if (strcmp(nameParam, "speech_common_para") == 0) {
                sizeDump = 12;
            } else if (strcmp(nameParam, "debug_info") == 0) {
                sizeDump = 8;
            }
            break;
        }
        case PARSER_AUDIO_TYPE_SPEECH_NETWORK: {
            if (strcmp(nameParam, "speech_network_support") == 0) {
                DataTypePrint = 1;
                sizeDump = 1;
            }
            break;
        }
        case PARSER_AUDIO_TYPE_SPEECH_ECHOREF: {
            if (strcmp(nameParam, "USBAudio") == 0) {
                sizeDump = 3;
            }
            break;
        }
        case PARSER_AUDIO_TYPE_SPEECH_MAGICLARITY: {
            if (strcmp(nameParam, "shape_rx_fir_para") == 0) {
                sizeDump = 32;
            }
            break;
        }
        }
        snprintf(sphDumpStr, SPH_DUMP_STR_SIZE, "%s[%d]=", nameParam, sizeDump);

        for (idxDump = 0; idxDump < sizeDump; idxDump++) {
            char sphDumpTemp[100] = { 0 };
            if (DataTypePrint == 1) {
                snprintf(sphDumpTemp, 100, "[%d]0x%x,", idxDump, *((uint16_t *)speechParamData + idxDump));
            } else {
                snprintf(sphDumpTemp, 100, "[%d]%d,", idxDump, *((uint16_t *)speechParamData + idxDump));
            }
            strncat(sphDumpStr, sphDumpTemp, SPH_DUMP_STR_SIZE - 1);
        }

        if (idxDump != 0 && bufDump != NULL) {
            strncat(bufDump, sphDumpStr, SPH_DUMP_STR_SIZE - 1);
        }
        return NO_ERROR;
    }

    status_t GetSpeechParamFromAppParser_new(uint16_t idxSphType,
                                             PARSER_AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT * paramLayerInfo,
                                             char *bufParamUnit,
                                             uint16_t *sizeByteTotal,
                                             uint16_t Dump_Flag) {
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

        ALOGV("%s(), categoryType.size=%zu, paramName.size=%zu", __FUNCTION__, paramLayerInfo->categoryType.size(), paramLayerInfo->paramName.size());
        for (idxCount = 0; idxCount < paramLayerInfo->categoryType.size(); idxCount++) {
            ALOGV("%s(), categoryType[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo->categoryType.at(idxCount).string());
        }
        for (idxCount = 0; idxCount < paramLayerInfo->categoryName.size(); idxCount++) {
            ALOGV("%s(), categoryName[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo->categoryName.at(idxCount).string());
        }

        for (idxCount = 0; idxCount < paramLayerInfo->numCategoryType; idxCount++) {
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

        if (Dump_Flag != 0) {
            ALOGD("%s() audioType=%s, categoryPath=%s, paramId=%d", __FUNCTION__, audioType->name, categoryPath, paramUnit->paramId);
        }
        ALOGV(" (*paramLayerInfo).numParam  = %d", (*paramLayerInfo).numParam);


        //for speech param dump
        char *bufParamDump = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
        memset(bufParamDump, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

        uint16_t SpeechParam_size = 0;

        for (idxCount = 0; idxCount < (*paramLayerInfo).numParam; idxCount++) {

            SpeechParam = appOps->paramUnitGetParamByName(paramUnit, (const char *)paramLayerInfo->paramName.at(idxCount).string());
            if (SpeechParam) {
                sizeByteParam = sizeByteParaData((DATA_TYPE)SpeechParam->paramInfo->dataType, SpeechParam->arraySize);

                SpeechParam_size = (uint16_t)(SpeechParam->arraySize);

                //For mode param , if (SpeechParam_size == 48)

                if ((SpeechParam_size == 48) && (idxSphType == PARSER_AUDIO_TYPE_SPEECH)) {
                    SpeechParam_size = 64;
                }

                if ((idxSphType == PARSER_AUDIO_TYPE_SPEECH) || (idxSphType == PARSER_AUDIO_TYPE_SPEECH_DMNR) || (idxSphType == PARSER_AUDIO_TYPE_SPEECH_GENERAL) || (idxSphType == PARSER_AUDIO_TYPE_SPEECH_MAGICLARITY)) {
                    //Add parameter total size at first
                    memcpy(bufParamUnit + *sizeByteTotal, &(SpeechParam_size), sizeof(uint16_t));
                    *sizeByteTotal += sizeof(uint16_t);
                }

                memcpy(bufParamUnit + *sizeByteTotal, SpeechParam->data, sizeByteParam);

                *sizeByteTotal += sizeByteParam;

                //For mode param 48-> 64
                if ((SpeechParam_size == 64) && (idxSphType == PARSER_AUDIO_TYPE_SPEECH)) {
                    *sizeByteTotal += (16 * 2);
                    //    ALOGD("Test for speech param sizeByteTotal  = %d", *sizeByteTotal);
                }

                // add Reserved Words behind the data
                if (idxSphType == PARSER_AUDIO_TYPE_SPEECH_DMNR) {
                    *sizeByteTotal += (SpeechParam_size * 2);
                }

                ALOGV("%s() paramName=%s, sizeByteParam=%d",
                      __FUNCTION__, paramLayerInfo->paramName.at(idxCount).string(), sizeByteParam);
                //speech parameter dump
                SpeechDataDump_new(bufParamDump, idxSphType, (const char *)paramLayerInfo->paramName.at(idxCount).string(), (const char *)SpeechParam->data);
            }
        }

        if (bufParamDump != NULL) {
            if (bufParamDump[0] != 0) {
                if (Dump_Flag != 0) {
                    ALOGD("%s(),dump: %s", __FUNCTION__, bufParamDump);
                }
            }
            delete[] bufParamDump;
        }

        appOps->audioTypeUnlock(audioType);
        free(categoryPath);

        return NO_ERROR;
    }

    bool getFeatureOn(const SpeechFeatureType featureType, const uint16_t speechFeatureOn) {
        uint16_t featureMaskType = 1 << featureType;
        const bool featureOn = speechFeatureOn & featureMaskType;
        ALOGV("%s() featureMaskType: 0x%x, featureOn=%d", __FUNCTION__, featureMaskType, featureOn);
        return featureOn;
    }

    speech_profile_SP SP_GetSpeechProfile(const SpeechParserAttribute speechParserAttribute) {
        speech_profile_SP idxSphProfile;

        if (getFeatureOn(SPEECH_FEATURE_LOOPBACK, speechParserAttribute.speechFeatureOn)) {
            switch (speechParserAttribute.outputDevice) {
            case AUDIO_DEVICE_OUT_SPEAKER:
                idxSphProfile = PARSER_SPEECH_PROFILE_LPBK_HANDSFREE;
                break;
            case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
            case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                idxSphProfile = PARSER_SPEECH_PROFILE_LPBK_HEADSET;
                break;
            default:
                idxSphProfile = PARSER_SPEECH_PROFILE_LPBK_HANDSET;
                break;
            }
        } else if (audio_is_bluetooth_sco_device(speechParserAttribute.outputDevice)) {
            if (getFeatureOn(SPEECH_FEATURE_BTNREC, speechParserAttribute.speechFeatureOn)) {

                idxSphProfile = PARSER_SPEECH_PROFILE_BT_EARPHONE;
            } else {

                idxSphProfile = PARSER_SPEECH_PROFILE_BT_NREC_OFF;
            }

        } else {

            switch (speechParserAttribute.outputDevice) {
            case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                idxSphProfile = PARSER_SPEECH_PROFILE_3_POLE_HEADSET;
                break;
            case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                idxSphProfile = PARSER_SPEECH_PROFILE_4_POLE_HEADSET;
                break;

            case AUDIO_DEVICE_OUT_SPEAKER:
#if defined(MTK_INCALL_HANDSFREE_DMNR)
                idxSphProfile = PARSER_SPEECH_PROFILE_MAGICONFERENCE;
#else
                if (speechParserAttribute.ttyMode == AUD_TTY_OFF) {
                    //              if (mSphParamInfo.isSV) {
                    if (getFeatureOn(SPEECH_FEATURE_SUPERVOLUME, speechParserAttribute.speechFeatureOn)) {
                        idxSphProfile = PARSER_SPEECH_PROFILE_HANDSFREE_SV;
                    } else {
                        idxSphProfile = PARSER_SPEECH_PROFILE_HANDSFREE;
                    }
                } else {
                    switch (speechParserAttribute.ttyMode) {
                    case AUD_TTY_HCO:
                        idxSphProfile = PARSER_SPEECH_PROFILE_TTY_HCO_HANDSFREE;
                        break;
                    case AUD_TTY_VCO:
                        idxSphProfile = PARSER_SPEECH_PROFILE_TTY_VCO_HANDSFREE;
                        break;
                    default:
                        idxSphProfile = PARSER_SPEECH_PROFILE_TTY_HCO_HANDSFREE;
                        break;
                    }
                }
#endif
                break;

            case AUDIO_DEVICE_OUT_USB_DEVICE:
                idxSphProfile = PARSER_SPEECH_PROFILE_USB_HEADSET;
                break;

            default:
                if (getFeatureOn(SPEECH_FEATURE_HAC, speechParserAttribute.speechFeatureOn)) {
                    idxSphProfile = PARSER_SPEECH_PROFILE_HAC;

                } else {
                    if (speechParserAttribute.ttyMode == AUD_TTY_OFF) {
                        //                  if (mSphParamInfo.isSV) {
                        if (getFeatureOn(SPEECH_FEATURE_SUPERVOLUME, speechParserAttribute.speechFeatureOn)) {
                            idxSphProfile = PARSER_SPEECH_PROFILE_HANDSET_SV;
                        } else {
                            idxSphProfile = PARSER_SPEECH_PROFILE_HANDSET;
                        }
                    } else {
                        switch (speechParserAttribute.ttyMode) {
                        case AUD_TTY_HCO:
                            idxSphProfile = PARSER_SPEECH_PROFILE_TTY_HCO_HANDSET;
                            break;
                        case AUD_TTY_VCO:
                            idxSphProfile = PARSER_SPEECH_PROFILE_TTY_VCO_HANDSET;
                            break;
                        default:
                            idxSphProfile = PARSER_SPEECH_PROFILE_TTY_HCO_HANDSET;
                            break;
                        }
                    }
                    break;
                }
            }
        }
        ALOGD("%s(), idxSphProfile = %d", __FUNCTION__, idxSphProfile);

        return idxSphProfile;
    }

    void Init() {
        ALOGD("%s()", __FUNCTION__);
        InitAppParser();
        InitSpeechNetwork();

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
                    mSphParamSupport.isNetworkSupport = true;
                    numSpeechParam = 7;
                    break;
                case 1:
                    mSphParamSupport.isNetworkSupport = true;
                    numSpeechParam = 3;
                    break;
                default:
                    mSphParamSupport.isNetworkSupport = false;
                    numSpeechParam = 3;
                    break;
                }
            } else {
                mSpeechParamVerFirst = 0;
                mSpeechParamVerLast = 0;
                mSphParamSupport.isNetworkSupport = false;
                numSpeechParam = 3;
            }

            const char *strSphTTY = appOps->appHandleGetFeatureOptionValue(mAppHandle, "SPH_PARAM_TTY");
            if (strSphTTY != NULL) {
                if (strcmp(strSphTTY, "yes") == 0) {
                    mSphParamSupport.isTTYSupport = true;
                } else {
                    mSphParamSupport.isTTYSupport = false;
                }
            } else {
                mSphParamSupport.isTTYSupport = false;
            }

            const char *strSphSV = appOps->appHandleGetFeatureOptionValue(mAppHandle, "SPH_PARAM_SV");
            if (strSphSV != NULL) {
                if (strcmp(strSphSV, "yes") == 0) {
                    mSphParamSupport.isSuperVolumeSupport = true;
                } else {
                    mSphParamSupport.isSuperVolumeSupport = false;
                }
            } else {
                mSphParamSupport.isSuperVolumeSupport = false;
            }
        }
    }

    void InitAppParser() {
        //ALOGD("+%s()", __FUNCTION__);
        /* Init AppHandle */
        //ALOGD("%s() appHandleGetInstance", __FUNCTION__);
        AppOps *appOps = appOpsGetInstance();
        if (appOps == NULL) {
            ALOGE("Error %s %d", __FUNCTION__, __LINE__);
            ASSERT(0);
            return;
        }
        mAppHandle = appOps->appHandleGetInstance();
        //ALOGD("%s() appHandleRegXmlChangedCb", __FUNCTION__);

    }

    int InitSpeechNetwork() {
        uint16_t size = 0, sizeByteFromApp = 0;
        char *packedParamUnitFromApp = new char[10];
        memset(packedParamUnitFromApp, 0, 10);

        PARSER_AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT paramLayerInfo;
        memset(&paramLayerInfo, 0, sizeof(PARSER_AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT));

        paramLayerInfo.audioTypeName = (char *)Parser_audioTypeNameList[PARSER_AUDIO_TYPE_SPEECH_NETWORK];

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

            //          ALOGV("%s(), categoryType.size=%zu, paramName.size=%zu", __FUNCTION__, paramLayerInfo.categoryType.size(), paramLayerInfo.paramName.size());
            //          for (idxCount = 0; idxCount < paramLayerInfo.categoryType.size(); idxCount++) {
            //              ALOGV("%s(), categoryType[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.categoryType.at(idxCount).string());
            //          }
            //          for (idxCount = 0; idxCount < paramLayerInfo.paramName.size(); idxCount++) {
            //              ALOGV("%s(), paramName[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.paramName.at(idxCount).string());
            //          }
            //-----------
            //parse layer
            CategoryType *categoryNetwork = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_SpeechNetwork_CategoryType[0].string());
            numSpeechNetwork = appOps->categoryTypeGetNumOfCategory(categoryNetwork);

            //parse network
            for (int i = 0; i < numSpeechNetwork; i++) {
                Category *CateNetwork = appOps->categoryTypeGetCategoryByIndex(categoryNetwork, i);
                sizeByteFromApp = 0;
                //clear
                while (!paramLayerInfo.categoryName.empty()) {
                    paramLayerInfo.categoryName.pop_back();
                }
                strncpy(mListSpeechNetwork[i].name, CateNetwork->name, 128);
                mListSpeechNetwork[i].name[127] = '\0';
                //strncpy(mListSpeechNetwork[i].name, CateNetwork->name, 128);
                paramLayerInfo.categoryName.push_back(String8(CateNetwork->name));//Network

                GetSpeechParamFromAppParser_new(PARSER_AUDIO_TYPE_SPEECH_NETWORK, &paramLayerInfo, packedParamUnitFromApp, &sizeByteFromApp, 1);
                mListSpeechNetwork[i].supportBit = *((uint16_t *)packedParamUnitFromApp);
                size += sizeByteFromApp;

                ALOGD("%s(), i=%d, sizeByteFromApp=%d, supportBit=0x%x",
                      __FUNCTION__, i, sizeByteFromApp, mListSpeechNetwork[i].supportBit);
            }
            //ALOGD("-%s(), total size byte=%d", __FUNCTION__, size);
        } else {
            ALOGE("Error %s %d", __FUNCTION__, __LINE__);
            ASSERT(0);
        }
        //init the Name mapping table  for each SpeechNetwork
        bool IsNetworkFound = false;
        for (int bitIndex = 0; bitIndex < 12; bitIndex++) {
            IsNetworkFound = false;
            for (int NetworkIndex = 0; NetworkIndex < numSpeechNetwork; NetworkIndex++) {
                if (((mListSpeechNetwork[NetworkIndex].supportBit >> bitIndex) & 1) == 1) {
                    strncpy(mNameForEachSpeechNetwork[bitIndex].name, mListSpeechNetwork[NetworkIndex].name, 128);
                    mNameForEachSpeechNetwork[bitIndex].name[127] = '\0';
                    IsNetworkFound = true;
                    break;
                }
            }
            if (!IsNetworkFound) {
                strncpy(mNameForEachSpeechNetwork[bitIndex].name, mListSpeechNetwork[0].name, 128);
                mNameForEachSpeechNetwork[bitIndex].name[127] = '\0';
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

    char *GetNameForEachSpeechNetwork(unsigned char bitIndex) {
        ALOGD("%s(), mNameForEachSpeechNetwork[%d].name = %s", __FUNCTION__, bitIndex, mNameForEachSpeechNetwork[bitIndex].name);
        return mNameForEachSpeechNetwork[bitIndex].name;
    }

    void SetDBGinfoindex10toTestMode(uint16_t dbgindex10) {
        //uint16_t dbgindex10 = *((uint16_t *)(outBuf->bufferAddr) + count_print + 32);

        switch (dbgindex10) {
        case 0:
            strncpy(TestMode, "Default", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 1:
            strncpy(TestMode, "TMO_FT_F4L", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 2:
            strncpy(TestMode, "TMO_Lab_AE", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 3:
            strncpy(TestMode, "ATT_FT_F4L", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 4:
            strncpy(TestMode, "ATT_Lab_AE", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 5:
            strncpy(TestMode, "CMCC_FT", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 6:
            strncpy(TestMode, "CMCC_Lab_AE", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 7:
            strncpy(TestMode, "CMCC_Benchmark", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 8:
            strncpy(TestMode, "CT_FT", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 9:
            strncpy(TestMode, "CT_Lab_AE", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 10:
            strncpy(TestMode, "CT_Benchmark", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 11:
            strncpy(TestMode, "CU_FT", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 12:
            strncpy(TestMode, "CU_Lab_AE", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 13:
            strncpy(TestMode, "VZW_FT", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 14:
            strncpy(TestMode, "VZW_Lab", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 15:
            strncpy(TestMode, "Sprint_FT", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 16:
            strncpy(TestMode, "Sprint_Lab", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 17:
            strncpy(TestMode, "TWN_FT", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 18:
            strncpy(TestMode, "AUX_Mode1", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 19:
            strncpy(TestMode, "AUX_Mode2", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        case 20:
            strncpy(TestMode, "AUX_Mode3", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        default:
            strncpy(TestMode, "Default", TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            break;
        }

    }

    EXPORT int getParamBuffer(SpHandle * spHandle, SpeechParserAttribute speechParserAttribute, SpeechDataBufType * outBuf) {

        ALOGD("%s, speechParserAttribute inputDevice=%u, outputDevice=%u, idxVolume=%d, driverScenario=%d, speechFeatureOn=%d, ttyMode=%d, outBuf = %p, outBuf->bufferAddr = %p, memorySize=%u, dataSize=%u, TestMode = %s", __FUNCTION__, speechParserAttribute.inputDevice, speechParserAttribute.outputDevice, speechParserAttribute.idxVolume, speechParserAttribute.driverScenario, speechParserAttribute.speechFeatureOn, speechParserAttribute.ttyMode, outBuf, outBuf->bufferAddr, outBuf->memorySize, outBuf->dataSize, TestMode);
        //ALOGD("%s, outBuf = %p, outBuf->bufferAddr = %p, memorySize=%p, dataSize=%p ", __FUNCTION__, outBuf, outBuf->bufferAddr, outBuf->memorySize, outBuf->dataSize);
		ALOGV("%s(), spHandle = 0x%p\n", __FUNCTION__, spHandle);

        uint32_t MD_version_First, MD_version_Last, MD_version_Last2, MD_version_Last3;

        if (SpeechParserInited == 0) {
            Init();

            SpeechParserInited = 1;
            ALOGV("SpeechParser new Init !");
        }

        MD_version_First = (MD_version >> 12);
        MD_version_Last = (MD_version >> 8) & (0xF);
        MD_version_Last2 = (MD_version >> 4) & (0xF);
        MD_version_Last3 = MD_version & (0xF);

        ALOGD("AP version (XML version) = %d.%d ,  MD_version = 0x%x  = %d.%d.%d.%d", mSpeechParamVerFirst, mSpeechParamVerLast, MD_version, MD_version_First, MD_version_Last, MD_version_Last2, MD_version_Last3);

        //if ((mSpeechParamVerFirst == MD_version_First) && (mSpeechParamVerLast == MD_version_Last)) {
        //    ALOGD("AP(xml) version == MD_version !");
        //} else {
        //    ALOGD("AP(xml) version != MD_version , version mismatch => use default parameter !");
        //    return -1;      //error handle
        //}

        if ((mSpeechParamVerFirst != MD_version_First) || (mSpeechParamVerLast != MD_version_Last)) {
            ALOGE("AP(xml) version != MD_version , version mismatch => use default parameter !");
            return -1;      //error handle
        }


        int EMI_48K_MAX_PAYLOAD_DATA_BYTE = 48 * 1024;  // 0xC000 ?
        memset(outBuf->bufferAddr, 0, EMI_48K_MAX_PAYLOAD_DATA_BYTE);

        int idxProfile_Speech = 0;
        uint16_t size = 0, idxCount, sizeByteFromApp = 0;
        uint16_t numBand = 0, numNetwork = 0, numVolume = 0;
        int idxVolume = speechParserAttribute.idxVolume;
        uint16_t EMI_3K_zero = 3 * 1024;
        uint16_t ParserCheckNum_General_End = 0;
        uint16_t count_print = 0;
        //int counter;

        //-------------------------------Start General parameter-----------------------------------------------

        SP_SPEECH_PARAM_UNIT_HDR_STRUCT headerParamUnit;
        memset(&headerParamUnit, 0, sizeof(headerParamUnit));

        headerParamUnit.sphParserVer = ENH_SPH_PARSER_VER;
        headerParamUnit.ParserCheckingNum = 0x000B;
        headerParamUnit.SpeechDebugNum = 0x1010;
        headerParamUnit.Reserved[2] = Parser_time_count;
        headerParamUnit.Reserved[3] = speechParserAttribute.outputDevice;
        headerParamUnit.Reserved[4] = (uint16_t)speechParserAttribute.idxVolume;
        // 5 reserved elements = 0  , Reserved[2] = Parser_time_count; Reserved[3] = speechParserAttribute.outputDevice; Reserved[4] = speechParserAttribute.idxVolume;

        ALOGD("Parser_time_count = %d", Parser_time_count);
        Parser_time_count++;

        if (Parser_time_count >= 0x100) {
            Parser_time_count = 0;
        }

        size += EMI_3K_zero;

        memcpy((char *)outBuf->bufferAddr + size, &headerParamUnit, sizeof(headerParamUnit));
        size += sizeof(headerParamUnit);

        char *packedParamUnitFromApp = new char[MAX_BYTE_PARAM_SPEECH];
        memset(packedParamUnitFromApp, 0, MAX_BYTE_PARAM_SPEECH);

        PARSER_AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT paramLayerInfo;
        memset(&paramLayerInfo, 0, sizeof(PARSER_AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT));

        paramLayerInfo.audioTypeName = (char *)Parser_audioTypeNameList[PARSER_AUDIO_TYPE_SPEECH_GENERAL];
        paramLayerInfo.numCategoryType = MAX_NUM_CATEGORY_TYPE_SPEECH_GENERAL;//4
        paramLayerInfo.numParam = MAX_NUM_PARAM_SPEECH_GENERAL;//4

        paramLayerInfo.categoryType.assign(audioType_SpeechGeneral_CategoryType, audioType_SpeechGeneral_CategoryType + paramLayerInfo.numCategoryType);
        paramLayerInfo.paramName.assign(audioType_SpeechGeneral_ParamName, audioType_SpeechGeneral_ParamName + paramLayerInfo.numParam);

        paramLayerInfo.categoryName.push_back(String8(audioType_SpeechGeneral_CategoryName1[0]));

        GetSpeechParamFromAppParser_new(PARSER_AUDIO_TYPE_SPEECH_GENERAL, &paramLayerInfo, packedParamUnitFromApp, &sizeByteFromApp, 1);

        memcpy((char *)outBuf->bufferAddr + size, packedParamUnitFromApp, sizeByteFromApp);
        size += sizeByteFromApp;

        //Set Reserved parameters
        size += 60;

        if (packedParamUnitFromApp != NULL) {
            delete[] packedParamUnitFromApp;
        }

        ParserCheckNum_General_End = 0x0B0B;
        memcpy((char *)outBuf->bufferAddr + size, &ParserCheckNum_General_End, sizeof(ParserCheckNum_General_End));
        size += sizeof(ParserCheckNum_General_End);
        //For reserved
        size += 6;

        //print the Common parameters ,   skip EMI and print 0~37
        //for (count_print = EMI_3K_zero / 2 + 0; count_print < EMI_3K_zero / 2 + 8; count_print++) {
        //   ALOGD("Common, SpeechParam->data [%d] = %p", count_print, *((uint16_t *)(outBuf->bufferAddr) + count_print));
        //}

        count_print = EMI_3K_zero / 2 + 0;

        SetDBGinfoindex10toTestMode(*((uint16_t *)(outBuf->bufferAddr) + count_print + 32));    //Use DBGinfo index10 value to set TestMode string

        ALOGD("Common, SpeechParam->data [%d] = %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu  Common ParserCheckingNumEnd [%d] = %hu , DebugInfo index10 = %d , TestMode = %s", count_print, *((uint16_t *)(outBuf->bufferAddr) + count_print), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 1)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 2)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 3)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 4)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 5)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 6)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 7)), EMI_3K_zero / 2 + 68, *((uint16_t *)outBuf->bufferAddr + (EMI_3K_zero / 2 + 68)), *((uint16_t *)(outBuf->bufferAddr) + count_print + 32), TestMode);

        //ALOGD(" Common ParserCheckingNumEnd [%d] = %p", EMI_3K_zero / 2 + 68, *((uint16_t *)outBuf->bufferAddr + (EMI_3K_zero / 2 + 68)));

        //End of setting Common Param ****************************************************************************

        //Start setting DMNR format-----------------------------------------------------------------------

        //Start setting DMNR format
        uint16_t ParserCheckNum_DMNR_End = 0, OutDevice_VoiceBand = 0;
        uint16_t idxBand = 0, idxProfile = 0;
        uint16_t  numProfile = 0;
        numBand = 0;

        memset(&headerParamUnit, 0, sizeof(headerParamUnit));

        headerParamUnit.sphParserVer = ENH_SPH_PARSER_VER;
        headerParamUnit.ParserCheckingNum = 0x000C;
        headerParamUnit.SpeechDebugNum = 0x1010;
        // 5 reserved elements = 0

        memcpy((char *)outBuf->bufferAddr + size, &headerParamUnit, sizeof(headerParamUnit));
        size += sizeof(headerParamUnit);

        memset(&paramLayerInfo, 0, sizeof(PARSER_AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT));

        paramLayerInfo.audioTypeName = (char *)Parser_audioTypeNameList[PARSER_AUDIO_TYPE_SPEECH_DMNR];
        paramLayerInfo.numCategoryType = MAX_NUM_CATEGORY_TYPE_SPEECH_DMNR;//4
        paramLayerInfo.numParam = MAX_NUM_PARAM_SPEECH_DMNR;//4
        paramLayerInfo.categoryType.assign(audioType_SpeechDMNR_CategoryType, audioType_SpeechDMNR_CategoryType + paramLayerInfo.numCategoryType);
        paramLayerInfo.paramName.assign(audioType_SpeechDMNR_ParamName, audioType_SpeechDMNR_ParamName + paramLayerInfo.numParam);

        AppOps *appOps = appOpsGetInstance();
        AudioType *audioType = NULL;
        if (appOps == NULL) {
            ALOGE("Error %s %d", __FUNCTION__, __LINE__);
            ASSERT(0);
            return UNKNOWN_ERROR;
        } else {
            audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, paramLayerInfo.audioTypeName);
        }

        char *packedParamUnitFromApp_DMNR = new char[MAX_BYTE_PARAM_SPEECH];
        memset(packedParamUnitFromApp_DMNR, 0, MAX_BYTE_PARAM_SPEECH);

        CategoryType *categoryBand = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_SpeechDMNR_CategoryType[0].string());
        numBand = appOps->categoryTypeGetNumOfCategory(categoryBand);
        CategoryType *categoryProfile = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_SpeechDMNR_CategoryType[1].string());
        numProfile = appOps->categoryTypeGetNumOfCategory(categoryProfile);

        for (idxBand = 0; idxBand < numBand; idxBand++) { //NB, WB, SWB
            for (idxProfile = 0; idxProfile < numProfile; idxProfile++) {
                sizeByteFromApp = 0;

                Category *CateBand = appOps->categoryTypeGetCategoryByIndex(categoryBand, idxBand);
                paramLayerInfo.categoryName.push_back(String8(CateBand->name));//Band
                paramLayerInfo.categoryName.push_back(String8(audioType_SpeechDMNR_CategoryName2[idxProfile]));//Profile

                OutDevice_VoiceBand = ((1 << idxBand) << 12) + (idxProfile + 1);
                memcpy((char *)outBuf->bufferAddr + size, &OutDevice_VoiceBand, sizeof(OutDevice_VoiceBand));
                size += sizeof(OutDevice_VoiceBand);

                GetSpeechParamFromAppParser_new(PARSER_AUDIO_TYPE_SPEECH_DMNR, &paramLayerInfo, packedParamUnitFromApp_DMNR, &sizeByteFromApp, 1);

                memcpy((char *)outBuf->bufferAddr + size, packedParamUnitFromApp_DMNR, sizeByteFromApp);
                size += sizeByteFromApp;

                paramLayerInfo.categoryName.pop_back();
                paramLayerInfo.categoryName.pop_back();

            }
        }
        if (numBand == 2) {         // Doesn't add SWB,FB OutputDevice Type.....0602, if we have SWB XML, modify here
            size += (242 * 4) * 2;    //Bytes reserve for SWB ,FB
        } else if (numBand == 3) {
            size += (242 * 2) * 2;    //Bytes reserve for FB
        }

        if (packedParamUnitFromApp_DMNR != NULL) {
            delete[] packedParamUnitFromApp_DMNR;
        }

        ParserCheckNum_DMNR_End = 0x0C0C;
        memcpy((char *)outBuf->bufferAddr + size, &ParserCheckNum_DMNR_End, sizeof(ParserCheckNum_DMNR_End));
        size += sizeof(ParserCheckNum_DMNR_End);
        //For reserved
        size += 6;

        //1~8 DMNR mode param
        //for (count_print = (EMI_3K_zero / 2 + 72); count_print < (EMI_3K_zero / 2 + 72) + 8; count_print++) {
        //    ALOGD("DMNR, SpeechParam->data [%d] = %p", count_print, *((uint16_t *)(outBuf->bufferAddr) + count_print));
        //}
        count_print = (EMI_3K_zero / 2 + 72);
        ALOGD("DMNR, SpeechParam->data [%d] = %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu", count_print, *((uint16_t *)(outBuf->bufferAddr) + count_print), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 1)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 2)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 3)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 4)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 5)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 6)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 7)));

        //End of setting DMNR Param******************************************************************************

        // START MagiClarity-------------------------------------------------------------------
        uint16_t ParserCheckNum_MagiClarity_End = 0;

        memset(&headerParamUnit, 0, sizeof(headerParamUnit));

        headerParamUnit.sphParserVer = ENH_SPH_PARSER_VER;
        headerParamUnit.ParserCheckingNum = 0x000D;
        headerParamUnit.SpeechDebugNum = 0x1010;
        // 5 reserved elements = 0

        memcpy((char *)outBuf->bufferAddr + size, &headerParamUnit, sizeof(headerParamUnit));
        size += sizeof(headerParamUnit);

        char *packedParamUnitFromApp_MagiClarity = new char[MAX_BYTE_PARAM_SPEECH];
        memset(packedParamUnitFromApp_MagiClarity, 0, MAX_BYTE_PARAM_SPEECH);

        memset(&paramLayerInfo, 0, sizeof(PARSER_AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT));

        paramLayerInfo.audioTypeName = (char *)Parser_audioTypeNameList[PARSER_AUDIO_TYPE_SPEECH_MAGICLARITY];
        paramLayerInfo.numCategoryType = MAX_NUM_CATEGORY_TYPE_SPEECH_MAGICLARITY;//4
        paramLayerInfo.numParam = MAX_NUM_PARAM_SPEECH_MAGICLARITY;//4

        paramLayerInfo.categoryType.assign(audioType_SpeechMagiClarity_CategoryType, audioType_SpeechMagiClarity_CategoryType + paramLayerInfo.numCategoryType);
        paramLayerInfo.paramName.assign(audioType_SpeechMagiClarity_ParamName, audioType_SpeechMagiClarity_ParamName + paramLayerInfo.numParam);

        for (idxCount = 0; idxCount < paramLayerInfo.paramName.size(); idxCount++) {
            ALOGV("%s(), paramName[%d]= %s", __FUNCTION__, idxCount, paramLayerInfo.paramName.at(idxCount).string());
        }

        paramLayerInfo.categoryName.push_back(String8(audioType_SpeechMagiClarity_CategoryName1[0]));

        sizeByteFromApp = 0;
        GetSpeechParamFromAppParser_new(PARSER_AUDIO_TYPE_SPEECH_MAGICLARITY, &paramLayerInfo, packedParamUnitFromApp_MagiClarity, &sizeByteFromApp, 1);

        memcpy((char *)outBuf->bufferAddr + size, packedParamUnitFromApp_MagiClarity, sizeByteFromApp);
        size += sizeByteFromApp;

        //Set Reserved parameters
        size += 60;

        if (packedParamUnitFromApp_MagiClarity != NULL) {
            delete[] packedParamUnitFromApp_MagiClarity;
        }

        ParserCheckNum_MagiClarity_End = 0x0D0D;
        memcpy((char *)outBuf->bufferAddr + size, &ParserCheckNum_MagiClarity_End, sizeof(ParserCheckNum_MagiClarity_End));
        size += sizeof(ParserCheckNum_MagiClarity_End);
        //For reserved
        size += 6;

        //for (count_print = EMI_3K_zero / 2 + 72 + 1468; count_print < EMI_3K_zero / 2 + 72 + 1468 + 8; count_print++) {
        //    ALOGD("Magiclarity, SpeechParam->data [%d] = %p", count_print, *((uint16_t *)(outBuf->bufferAddr) + count_print));
        //}

        count_print = EMI_3K_zero / 2 + 72 + 1468;
        ALOGD("Magiclarity, SpeechParam->data [%d] = %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu", count_print, *((uint16_t *)(outBuf->bufferAddr) + count_print), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 1)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 2)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 3)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 4)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 5)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 6)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 7)));
        // END of MagiClarity-------------------------------------------------------------------

        //Start setting Speech Param---------------------------------------------------------------------------
        uint16_t  ParserCheckNum_Speech_End = 0, VoiceBand_Network = 0;

        memset(&headerParamUnit, 0, sizeof(headerParamUnit));

        headerParamUnit.sphParserVer = ENH_SPH_PARSER_VER;
        headerParamUnit.ParserCheckingNum = 0x000A;
        headerParamUnit.SpeechDebugNum = 0x1010;
        // 5 reserved elements = 0

        memcpy((char *)outBuf->bufferAddr + size, &headerParamUnit, sizeof(headerParamUnit));
        size += sizeof(headerParamUnit);

        idxProfile_Speech = SP_GetSpeechProfile(speechParserAttribute);

        //2. xml working structure
        memset(&paramLayerInfo, 0, sizeof(PARSER_AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT));
        //-------------
        paramLayerInfo.audioTypeName = (char *)Parser_audioTypeNameList[PARSER_AUDIO_TYPE_SPEECH];

        //3. get app handle
        if (mAppHandle == NULL) {
            ALOGE("%s() mAppHandle == NULL, Assert!!!", __FUNCTION__);
            ASSERT(0);
            return UNKNOWN_ERROR;
        }
        //4. get xml
        // Query AudioType
        appOps = appOpsGetInstance();
        audioType = NULL;
        if (appOps == NULL) {
            ALOGE("Error %s %d", __FUNCTION__, __LINE__);
            ASSERT(0);
            return UNKNOWN_ERROR;
        } else {
            audioType = appOps->appHandleGetAudioTypeByName(mAppHandle, paramLayerInfo.audioTypeName);
        }

        //5 internal working buffer
        char *packedParamUnitFromApp_Speech = new char[MAX_BYTE_PARAM_SPEECH];
        memset(packedParamUnitFromApp_Speech, 0, MAX_BYTE_PARAM_SPEECH);

        //4 layers
        paramLayerInfo.numCategoryType = appOps->audioTypeGetNumOfCategoryType(audioType);//4
        //paramLayerInfo.numParam = numSpeechParam;// 7 kinds of parameters
        paramLayerInfo.numParam = 7;

        //push category name   "Band","Profile","VolIndex","Network"
        paramLayerInfo.categoryType.assign(audioType_Speech_CategoryType, audioType_Speech_CategoryType + paramLayerInfo.numCategoryType);
        //store  "speech_mode_para","sph_in_fir","sph_out_fir","sph_in_iir_mic1_dsp","sph_in_iir_mic2_dsp","sph_in_iir_enh_dsp","sph_out_iir_enh_dsp"
        paramLayerInfo.paramName.assign(audioType_Speech_ParamName, audioType_Speech_ParamName + paramLayerInfo.numParam);

        //parse layer
        //network ¦WNetwork
        CategoryType *categoryNetwork = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_Speech_CategoryType[3].string());
        //band name
        categoryBand = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_Speech_CategoryType[0].string());
        //band number
        numBand = appOps->categoryTypeGetNumOfCategory(categoryBand);

        //network number
        numNetwork = appOps->categoryTypeGetNumOfCategory(categoryNetwork);
        //volume name
        CategoryType *categoryVolume = appOps->audioTypeGetCategoryTypeByName(audioType, audioType_Speech_CategoryType[2].string());
        CategoryGroup *categoryGroupVolume = appOps->categoryTypeGetCategoryGroupByIndex(categoryVolume, 0);
        numVolume = appOps->categoryGroupGetNumOfCategory(categoryGroupVolume);

        //log buffer
        paramLayerInfo.logPrintParamUnit = new char[SPH_PARAM_UNIT_DUMP_STR_SIZE];
        memset(paramLayerInfo.logPrintParamUnit, 0, SPH_PARAM_UNIT_DUMP_STR_SIZE);

        for (int idxNetwork = 0; idxNetwork < 8; idxNetwork++) {//ºô¸ôbit0~7
            char *nameNetwork = GetNameForEachSpeechNetwork(idxNetwork); //bit 0  "GSM",... put each network name to this

            Category *CateNetwork = appOps->categoryTypeGetCategoryByName(categoryNetwork, nameNetwork);
            //Category *CateNetwork = appOps->categoryTypeGetCategoryByIndex(categoryNetwork, idxNetwork);

            ALOGV("Ver 1, Network bit %d , name = %s, nameNetwork = %s", idxNetwork, CateNetwork->name, nameNetwork);

            if (idxNetwork == 2) {          //idxNetwork 2~4  in 2nd EMI 13K
                size = (16 + 3) * 1024;
            } else if (idxNetwork == 5) {   //idxNetwork 5~7  in 3nd EMI 13K
                size = (16 + 16 + 3) * 1024;
            }

            //parse band
            for (int idxBand = 0; idxBand < numBand; idxBand++) {
                sizeByteFromApp = 0;
                Category *CateBand = appOps->categoryTypeGetCategoryByIndex(categoryBand, idxBand);

                VoiceBand_Network = SetMDParamDataHdr_SP(CateBand, CateNetwork);    //set DataHdr Band, Network
                memcpy((char *)outBuf->bufferAddr + size, &VoiceBand_Network, sizeof(VoiceBand_Network));
                size += sizeof(VoiceBand_Network);

                while (!paramLayerInfo.categoryName.empty()) {
                    paramLayerInfo.categoryName.pop_back();
                }
                //Band
                paramLayerInfo.categoryName.push_back(String8(CateBand->name));//Band
                //Profile
                if (strcmp(TestMode, "Default") != 0) {
                    //ALOGD("TestMode = %s != Default", TestMode);
                    paramLayerInfo.categoryName.push_back(String8(TestMode));
                } else {
                    paramLayerInfo.categoryName.push_back(String8(audioType_Speech_CategoryName2[idxProfile_Speech]));

                }

                //Volume
                if (idxVolume > 6 || idxVolume < 0) {
                    paramLayerInfo.categoryName.push_back(String8(audioType_Speech_CategoryName3[3]));//volume
                    ALOGE("%s(), Invalid IdxVolume=0x%x, use 3 !!!", __FUNCTION__, idxVolume);
                } else {
                    paramLayerInfo.categoryName.push_back(String8(audioType_Speech_CategoryName3[idxVolume]));
                }
                paramLayerInfo.categoryName.push_back(String8(nameNetwork));//Network

                for (idxCount = 0; idxCount < paramLayerInfo.categoryName.size(); idxCount++) {
                    ALOGV("%s(), categoryName[%d]= %s",
                          __FUNCTION__, idxCount, paramLayerInfo.categoryName.at(idxCount).string());
                }

                //For fix log too much issue, only print idxNetwork = 0 => "GSM" network parameter
                //If Customer request different network for different param, open the dump
                if (idxNetwork == 0) {
                    GetSpeechParamFromAppParser_new(PARSER_AUDIO_TYPE_SPEECH, &paramLayerInfo, packedParamUnitFromApp_Speech, &sizeByteFromApp, 1);
                } else {
                    GetSpeechParamFromAppParser_new(PARSER_AUDIO_TYPE_SPEECH, &paramLayerInfo, packedParamUnitFromApp_Speech, &sizeByteFromApp, 0);
                }

                memcpy((char *)outBuf->bufferAddr + size, packedParamUnitFromApp_Speech, sizeByteFromApp);
                size += sizeByteFromApp;

                size = size + (91 + 43) * 2;  //reserve  for FIR UL2, Enh IIR UL2

                size += 60; //reserve 30 words

                char sphLogTemp[SPH_DUMP_STR_SIZE] = { 0 };
                snprintf(sphLogTemp, SPH_DUMP_STR_SIZE, "size(b)=%d; total size(b)=%d", sizeByteFromApp, size);
                strncat(paramLayerInfo.logPrintParamUnit, sphLogTemp, SPH_DUMP_STR_SIZE - 1);
            }
            size += 540 * 2;   //Bytes reserve for FB
        }

        ParserCheckNum_Speech_End = 0x0A0A;
        memcpy((char *)outBuf->bufferAddr + size, &ParserCheckNum_Speech_End, sizeof(ParserCheckNum_Speech_End));
        size += sizeof(ParserCheckNum_Speech_End);
        //For reserved
        size += 6;

        //End of setting Speech Param*******************************************************************************

        if (packedParamUnitFromApp_Speech != NULL) {
            delete[] packedParamUnitFromApp_Speech;
        }
        if (paramLayerInfo.logPrintParamUnit != NULL) {
            delete[] paramLayerInfo.logPrintParamUnit;
        }

        outBuf->memorySize = SD_SphParamSize;   // 48K,  0xC000
        outBuf->dataSize = SD_SphParamSize;  // 48K,  0xC000

        //for (count_print = (EMI_3K_zero / 2 + 72 + 1468 + 75); count_print < (EMI_3K_zero / 2 + 72 + 1468 + 75) + 8; count_print++) {
        //    ALOGD("Speech, SpeechParam->data [%d] = %p", count_print, *((uint16_t *)(outBuf->bufferAddr) + count_print));
        //}
        count_print = (EMI_3K_zero / 2 + 72 + 1468 + 75);
        ALOGD("Speech, SpeechParam->data [%d] = %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, End of getParamBuffer, outBuf->dataSize = %d ", count_print, *((uint16_t *)(outBuf->bufferAddr) + count_print), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 1)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 2)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 3)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 4)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 5)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 6)), *((uint16_t *)(outBuf->bufferAddr) + (count_print + 7)), outBuf->dataSize);

        //ALOGD("End of getParamBuffer, outBuf->dataSize = %d", outBuf->dataSize);

        return 0;   //general case, return 0    when AP&MD version mismatch, use default param, return -1
    }

    EXPORT int setKeyValuePair(SpHandle * spHandle, const SpeechStringBufType * keyValuePair) {
        ALOGD("+%s(): %s", __FUNCTION__, keyValuePair->stringAddr);
		ALOGV("%s(), spHandle = 0x%p\n", __FUNCTION__, spHandle);

        if (keyValuePair->stringAddr == NULL) {
            ALOGD("%s() keyValuePair->stringAddr == NULL, Assert!!!", __FUNCTION__);
        }

        char *keyHeader = NULL;
        char *keyString = NULL;
        keyHeader = strtok_r(keyValuePair->stringAddr, ",", &keyString);

        //ex:TestMode,TMO_FT_F4L
        if (strcmp(keyHeader, "TestMode") == 0) {
            strncpy(TestMode, keyString, TESTMODE_MAX_LENGTH);
            TestMode[TESTMODE_MAX_LENGTH - 1] = '\0';
            ALOGD("+%s(): TestMode = %s", __FUNCTION__, TestMode);
            return 0;
        } else if (strcmp(keyHeader, SPEECH_PARSER_SET_KEY_PREFIX) != 0) {
            ALOGE("%s(), Wrong Header: %s, expect:%s", __FUNCTION__, keyHeader, SPEECH_PARSER_SET_KEY_PREFIX);
            return -EINVAL;
        }

        AudioParameter param = AudioParameter(String8(keyString));
        int value;

        if (param.getInt(String8("MDVERSION"), value) == NO_ERROR) {
            param.remove(String8("MDVERSION"));
            MD_version = value;
            ALOGD("%s(): MD_version = 0x%x", __FUNCTION__, MD_version);
        }

        return 0;
    }

    EXPORT int getKeyValuePair(SpHandle * spHandle, SpeechStringBufType * keyValuePair) {
        ALOGD("%s(), keyString:%s", __FUNCTION__, keyValuePair->stringAddr);
		ALOGV("%s(), spHandle = 0x%p\n", __FUNCTION__, spHandle);

        if (keyValuePair->stringAddr == NULL) {
            ALOGD("%s() keyValuePair->stringAddr == NULL, Assert!!!", __FUNCTION__);
        }

        char *keyHeader = NULL;
        char *keyString = NULL;
        keyHeader = strtok_r(keyValuePair->stringAddr, ",", &keyString);

        if (strcmp(keyHeader, "TestMode") == 0) {
            keyValuePair->stringAddr = TestMode;
            ALOGD("%s(), return keyString:%s", __FUNCTION__, keyValuePair->stringAddr);

            return 0;
        } else if (strcmp(keyHeader, SPEECH_PARSER_GET_KEY_PREFIX) != 0) {
            ALOGE("%s(), Wrong Header: %s, expect:%s", __FUNCTION__, keyHeader, SPEECH_PARSER_GET_KEY_PREFIX);
            return -EINVAL;
        }

        char keyValueString[MAX_SPEECH_PARSER_KEY_LEN];
        memset((void *)keyValueString, 0, MAX_SPEECH_PARSER_KEY_LEN);

        if (strcmp(keyString, String8("PARAMBUF_SIZE")) == 0) {
            sprintf(keyValueString, "%d", SD_SphParamSize);
        }

        keyValuePair->stringAddr = keyValueString;
        ALOGD("%s(), return keyString:%s", __FUNCTION__, keyValuePair->stringAddr);

        return 0;
    }

}
};

//namespace android

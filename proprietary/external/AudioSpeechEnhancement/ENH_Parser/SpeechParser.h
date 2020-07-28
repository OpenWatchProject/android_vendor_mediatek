#ifndef _SPEECH_PARSER_H_
#define _SPEECH_PARSER_H_

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#include "AudioType.h"
#include <unistd.h>
#include <vector>
#include "AudioParamParser.h"
#include "SpeechParserType.h"
#include "SpeechFeatureDef.h"

#include "utstring.h"
#include "uthash.h"
#include "utlist.h"

#include <dlfcn.h>
#include <pthread.h>
#include <utils/Log.h>

#include <utils/String8.h>
#include <media/AudioParameter.h>
#include "AudioParamParser.h"



namespace android {

/*****************************************************************************
*                         I O C T R L  M E S S A G E S
******************************************************************************
*/

#define EXPORT
extern "C" {

    /*****************************************************************************
    *                         I O C T R L  M E S S A G E S
    ******************************************************************************
    */


    //Define Struct AudioType
#ifndef int8_t
    typedef signed char         int8_t;
#endif

#ifndef uint8_t
    typedef unsigned char       uint8_t;
#endif

#ifndef int16_t
    typedef signed short        int16_t;
#endif

#ifndef uint16_t
    typedef unsigned short      uint16_t;
#endif

#ifndef int32_t
    typedef signed int          int32_t;
#endif

#ifndef uint32_t
    typedef unsigned int        uint32_t;
#endif

#ifndef status_t
    typedef signed int          status_t;
#endif

    //Define Struct SpeechType

    enum speech_mode_SP {
        PARSER_SPEECH_MODE_NORMAL = 0,
        PARSER_SPEECH_MODE_EARPHONE = 1,
        PARSER_SPEECH_MODE_LOUD_SPEAKER = 2,
        PARSER_SPEECH_MODE_BT_EARPHONE = 3,
        PARSER_SPEECH_MODE_BT_CORDLESS = 4,
        PARSER_SPEECH_MODE_BT_CARKIT = 5,
        PARSER_SPEECH_MODE_MAGIC_CON_CALL = 6,
        PARSER_SPEECH_MODE_PRESERVED_2 = 7,
        PARSER_SPEECH_MODE_HAC = 8,
        PARSER_SPEECH_MODE_USB_AUDIO = 9,
        PARSER_SPEECH_MODE_NO_CONNECT = 10
    };


    enum speech_type_dynamic_param_SP {
        PARSER_AUDIO_TYPE_SPEECH = 0,
        PARSER_AUDIO_TYPE_SPEECH_DMNR = 1,
        PARSER_AUDIO_TYPE_SPEECH_GENERAL = 2,
        PARSER_AUDIO_TYPE_SPEECH_MAGICLARITY = 3,
        PARSER_AUDIO_TYPE_SPEECH_NETWORK = 4,
        PARSER_AUDIO_TYPE_SPEECH_ECHOREF = 5,


        PARSER_NUM_AUDIO_TYPE_SPEECH_TYPE  // the #types of speech_type_dynamic_param_t
    };




    const char Parser_audioTypeNameList[8][128] = {
        "Speech",
        "SpeechDMNR",
        "SpeechGeneral",
        "SpeechMagiClarity",
        "SpeechNetwork",
        "SpeechEchoRef"
    };

    /*****************************************************************************
    *                         I O C T R L  M E S S A G E S
    ******************************************************************************
    */



    typedef struct _PARSER_AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT {
        char *audioTypeName;
        char numCategoryType;//4
        std::vector<String8> categoryType;
        std::vector<String8> categoryName;
        char numParam;//4
        std::vector<String8> paramName;
        char *logPrintParamUnit;
    } PARSER_AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT;

    enum speech_profile_SP {
        PARSER_SPEECH_PROFILE_HANDSET = 0,
        PARSER_SPEECH_PROFILE_4_POLE_HEADSET = 1,
        PARSER_SPEECH_PROFILE_HANDSFREE = 2,
        PARSER_SPEECH_PROFILE_BT_EARPHONE = 3,
        PARSER_SPEECH_PROFILE_BT_NREC_OFF = 4,
        PARSER_SPEECH_PROFILE_MAGICONFERENCE = 5,
        PARSER_SPEECH_PROFILE_HAC = 6,
        PARSER_SPEECH_PROFILE_LPBK_HANDSET = 7,
        PARSER_SPEECH_PROFILE_LPBK_HEADSET = 8,
        PARSER_SPEECH_PROFILE_LPBK_HANDSFREE = 9,
        PARSER_SPEECH_PROFILE_3_POLE_HEADSET = 10,
        PARSER_SPEECH_PROFILE_5_POLE_HEADSET = 11,
        PARSER_SPEECH_PROFILE_5_POLE_HEADSET_ANC = 12,
        PARSER_SPEECH_PROFILE_USB_HEADSET = 13,
        PARSER_SPEECH_PROFILE_HANDSET_SV = 14,
        PARSER_SPEECH_PROFILE_HANDSFREE_SV = 15,
        PARSER_SPEECH_PROFILE_TTY_HCO_HANDSET = 16,
        PARSER_SPEECH_PROFILE_TTY_HCO_HANDSFREE = 17,
        PARSER_SPEECH_PROFILE_TTY_VCO_HANDSET = 18,
        PARSER_SPEECH_PROFILE_TTY_VCO_HANDSFREE = 19,

        PARSER_SPEECH_PROFILE_MAX_NUM = 20
    };

    typedef struct _PARSER_SPEECH_PARAM_SUPPORT_STRUCT {
        bool isNetworkSupport;
        bool isTTYSupport;
        bool isSuperVolumeSupport;

    } PARSER_SPEECH_PARAM_SUPPORT_STRUCT;

    typedef struct _PARSER_SPEECH_NETWORK_STRUCT {
        char name[128];
        uint16_t supportBit;//4

    } PARSER_SPEECH_NETWORK_STRUCT;


    enum tty_param_t_SP {
        PARSER_TTY_PARAM_OFF = 0,
        PARSER_TTY_PARAM_HCO = 1,
        PARSER_TTY_PARAM_VCO = 2
    };


    // Self define struct ( Parser_Info_Struct )


    typedef struct _SPEECH_PARAM_INFO {
        speech_mode_SP SpeechMode;
        unsigned int u4VolumeIndex;
        bool bBtHeadsetNrecOn;
        bool bLPBK;
        unsigned char uHeadsetPole;
        bool bSV;
        unsigned char idxTTY;

        bool isSingleBandTransfer;
        unsigned char IndexVoiceBandStart;

    } SPEECH_PARAM_INFO;

    typedef struct _Parser_Info_Struct {
        char *Buffer;
        SPEECH_PARAM_INFO SphParamInfo_SP;
        uint16_t MD_version;
        SpeechScenario Scenario;

        uint32_t CommonInfo_offset;
        uint32_t DebugInfo_offset;

        speech_type_dynamic_param_SP AP_version;
        uint16_t Network;
        uint32_t Buffer_length;
    } Parser_Info_Struct;


    typedef struct _SP_SPEECH_PARAM_UNIT_HDR {
        uint16_t sphParserVer;
        uint16_t ParserCheckingNum;
        uint16_t SpeechDebugNum;
        uint16_t Reserved[5];  //2 +3

    } SP_SPEECH_PARAM_UNIT_HDR_STRUCT;



    // Global variable/function public


    void Init();
    void InitAppParser();
    int InitSpeechNetwork();


    uint16_t sizeByteParaData(DATA_TYPE dataType, uint16_t arraySize);

    uint16_t SetMDParamDataHdr_SP(Category *cateBand, Category *cateNetwork);

    bool getFeatureOn(const SpeechFeatureType featureType, const uint16_t speechFeatureOn);

    speech_profile_SP SP_GetSpeechProfile(const SpeechParserAttribute speechParserAttribute);

    char *GetNameForEachSpeechNetwork(unsigned char bitIndex);

    status_t SpeechDataDump_new(char *bufDump, uint16_t idxSphType, const char *nameParam, const char *speechParamData);
    status_t GetSpeechParamFromAppParser_new(uint16_t idxSphType, PARSER_AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT *paramLayerInfo, char *bufParamUnit, uint16_t *sizeByteTotal, uint16_t Dump_Flag);

    void SetDBGinfoindex10toTestMode(uint16_t dbgindex10);
    /*****************************************************************************
    *                         F U N C T I O N S
    ******************************************************************************
    */

#define SP_LIB_NAME "libspeechparser_vendor.so"
    typedef enum {
        SP_ERROR = 0,
        SP_NO_ERROR = 1,
    } SP_STATUS;

    struct _SpHandle {
        pthread_rwlock_t lock;
        pthread_rwlock_t notifyLock;
    };

    typedef struct _SpHandle               SpHandle;

    typedef struct SpOps {
        void            *handle;
        SpHandle       *(*spHandleGetInstance)(void);

        SP_STATUS(*spHandleInit)(SpHandle *spHandle);
        SP_STATUS(*spHandleUninit)(SpHandle *spHandle);

        int(*getParamBuffer)(SpHandle *spHandle, SpeechParserAttribute speechParserAttribute, SpeechDataBufType *outBuf);
        int(*setKeyValuePair)(SpHandle *spHandle, const SpeechStringBufType *keyValuePair);
        int(*getKeyValuePair)(SpHandle *spHandle, SpeechStringBufType *keyValuePair);

    } SpOps;


    static SpOps       spOps;
    static short        spOpsInited = 0;
    EXPORT static SpOps *spOpsGetInstance(void) {
        const char *error;
        const char *funName = NULL;

        if (spOpsInited == 0) {
            ALOGD("%s(), init SpOps struct, lib is %s", __FUNCTION__, SP_LIB_NAME);

            /* dlopen */
            ALOGD("Go to dlopen");
            spOps.handle = dlopen(SP_LIB_NAME, RTLD_LAZY);
            if (!spOps.handle) {
                ALOGE("%s(), dlopen fail! (%s)\n", __FUNCTION__, dlerror());
                return NULL;
            }
            dlerror();    /* Clear any existing error */


            ALOGD("Go to spHandleGetInstance");
            funName = "spHandleGetInstance";
            spOps.spHandleGetInstance = (SpHandle * (*)(void)) dlsym(spOps.handle, funName);
            error = dlerror();
            if (error != NULL) {
                ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
                ALOGD("Error to spHandleGetInstance");
                return NULL;
            }

            /* spHandle API */
            ALOGD("Go to spHandleInit");
            funName = "spHandleInit";
            spOps.spHandleInit = (SP_STATUS(*)(SpHandle * spHandle)) dlsym(spOps.handle, funName);
            error = dlerror();
            if (error != NULL) {
                ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
                ALOGD("Error to spHandleInit");
                return NULL;
            }

            ALOGD("Go to spHandleUninit");
            funName = "spHandleUninit";
            spOps.spHandleUninit = (SP_STATUS(*)(SpHandle * spHandle)) dlsym(spOps.handle, funName);
            error = dlerror();
            if (error != NULL) {
                ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
                ALOGD("Error to spHandleUninit");
                return NULL;
            }

            ALOGD("Go to getParamBuffer");
            funName = "getParamBuffer";
            spOps.getParamBuffer = (int(*)(SpHandle * spHandle, SpeechParserAttribute speechParserAttribute, SpeechDataBufType * outBuf)) dlsym(spOps.handle, funName);
            error = dlerror();
            if (error != NULL) {
                ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
                ALOGD("Error to getParamBuffer");
                return NULL;
            }

            ALOGD("Go to setKeyValuePair");
            funName = "setKeyValuePair";
            spOps.setKeyValuePair = (int(*)(SpHandle * spHandle, const SpeechStringBufType * keyValuePair)) dlsym(spOps.handle, funName);
            error = dlerror();
            if (error != NULL) {
                ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
                ALOGD("Error to setKeyValuePair");
                return NULL;
            }

            ALOGD("Go to getKeyValuePair");
            funName = "getKeyValuePair";
            spOps.getKeyValuePair = (int(*)(SpHandle * spHandle, SpeechStringBufType * keyValuePair)) dlsym(spOps.handle, funName);
            error = dlerror();
            if (error != NULL) {
                ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
                ALOGD("Error to getKeyValuePair");
                return NULL;
            }

            spOpsInited = 1;
        }

        return &spOps;
    }

    static void spOpsDelInstance() {
        if (spOpsInited == 1) {
            dlclose(spOps.handle);
            spOps.handle = NULL;
            spOpsInited = 0;
        }
    }
};

}

#endif

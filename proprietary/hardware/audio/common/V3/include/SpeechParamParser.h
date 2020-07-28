#ifndef _SPEECH_PARAM_PARSER_H_
#define _SPEECH_PARAM_PARSER_H_

/*
 * =============================================================================
 *                     external references
 * =============================================================================
 */
#include "AudioType.h"
#include "SpeechType.h"
#include <vector>
#include "AudioParamParser.h"

namespace android {

/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT;
struct AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT;
struct SPEECH_PARAM_INFO_STRUCT;
struct SPEECH_PARAM_SUPPORT_STRUCT;
struct SPEECH_NETWORK_STRUCT;

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */
enum speech_type_dynamic_param_t {
    AUDIO_TYPE_SPEECH               = 0,
    AUDIO_TYPE_SPEECH_DMNR          = 1,
    AUDIO_TYPE_SPEECH_GENERAL       = 2,
    AUDIO_TYPE_SPEECH_MAGICLARITY   = 3,
    AUDIO_TYPE_SPEECH_NETWORK       = 4,
    AUDIO_TYPE_SPEECH_ECHOREF       = 5,


    NUM_AUDIO_TYPE_SPEECH_TYPE  /* the #types of speech_type_dynamic_param_t */
};

/* XML name */
const char audioTypeNameList[8][128] = {
    "Speech",
    "SpeechDMNR",
    "SpeechGeneral",
    "SpeechMagiClarity",
    "SpeechNetwork",
    "SpeechEchoRef"
};

enum tty_param_t {
    TTY_PARAM_OFF        = 0,
    TTY_PARAM_HCO        = 1,
    TTY_PARAM_VCO        = 2
};

/*
 * =============================================================================
 *                     class
 * =============================================================================
 */

class SpeechParamParser {
public:
    virtual ~SpeechParamParser();
    static SpeechParamParser *getInstance();
    bool GetSpeechParamSupport(const char *paramName);
    int GetSpeechParamUnit(char *bufParamUnit, int *paramArg);
    int GetGeneralParamUnit(char *bufParamUnit);
    int GetDmnrParamUnit(char *bufParamUnit);
    int GetMagiClarityParamUnit(char *bufParamUnit);
    int GetEchoRefParamUnit(char *bufParamUnit);
    status_t SetParamInfo(const String8 &keyParamPairs);
    int GetBtDelayTime(const char *btDeviceName);
    bool GetParamStatus(const char *paramName);

protected:


private:
    SpeechParamParser();
    static SpeechParamParser *UniqueSpeechParamParser;
    AppHandle *mAppHandle;
    int getSpeechProfile(const speech_mode_t sphMode, bool btHeadsetNrecOn);

    void init();
    void initAppParser();
    status_t getSpeechParamFromAppParser(uint16_t idxSphType,
                                         AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT *paramLayerInfo,
                                         char *bufParamUnit,
                                         uint16_t *sizeByteTotal);
    status_t speechDataDump(char *bufDump,
                            uint16_t idxSphType,
                            const char *nameParam,
                            const char *speechParamData);
    status_t setMDParamUnitHdr(speech_type_dynamic_param_t idxAudioType,
                               SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT *paramUnitHdr,
                               uint16_t configValue);
    uint16_t setMDParamDataHdr(SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT paramUnitHdr,
                               const char *cateBandName,
                               const char *cateNetworkName);
    int initSpeechNetwork(void);

    SPEECH_PARAM_INFO_STRUCT *mSphParamInfo;
    SPEECH_NETWORK_STRUCT *mListSpeechNetwork;
    SPEECH_NETWORK_STRUCT *mNameForEachSpeechNetwork;
    SPEECH_PARAM_SUPPORT_STRUCT *mSphParamSupport;
    uint8_t mNumSpeechNetwork, mSpeechParamVerFirst, mSpeechParamVerLast, mNumSpeechParam;

};   //SpeechParamParser

}   //namespace android

#endif

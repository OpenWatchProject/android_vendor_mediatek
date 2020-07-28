#ifndef _SPEECH_PARSER_GEN93_H_
#define _SPEECH_PARSER_GEN93_H_

#include "AudioType.h"
#include "SpeechType.h"
#include <vector>
#include <AudioParamParser.h>
#include "SpeechParserBase.h"

namespace android {

/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT;
struct SPEECH_ECHOREF_PARAM_STRUCT;
struct AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT;
struct SPEECH_PARAM_INFO_STRUCT;
struct SPEECH_PARAM_SUPPORT_STRUCT;
struct SPEECH_NETWORK_STRUCT;

enum speech_type_dynamic_param_t {
    AUDIO_TYPE_SPEECH               = 0,
    AUDIO_TYPE_SPEECH_DMNR          = 1,
    AUDIO_TYPE_SPEECH_GENERAL       = 2,
    AUDIO_TYPE_SPEECH_MAGICLARITY   = 3,
    AUDIO_TYPE_SPEECH_NETWORK       = 4,
    AUDIO_TYPE_SPEECH_ECHOREF       = 5,


    NUM_AUDIO_TYPE_SPEECH_TYPE  /* the #types of speech_type_dynamic_param_t */
};

/*
 * =============================================================================
 *                     class
 * =============================================================================
 */

class SpeechParserGen93 : public SpeechParserBase {

public:
    virtual ~SpeechParserGen93();

    /**
     * get instance's pointer
     */
    static SpeechParserGen93 *getInstance();

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
    virtual int getParamBuffer(SpeechParserAttribute speechParserAttribute, SpeechDataBufType *outBuf);

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
    virtual int setKeyValuePair(const SpeechStringBufType *keyValuePair);

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
    virtual int getKeyValuePair(SpeechStringBufType *keyValuePair);

    /**
     * =========================================================================
     *  @brief update phone call status from driver
     *
     *  @param callOn: the phone call status: true(On), false(Off)
     *
     *  @return int
     * =========================================================================
     */
    virtual int updatePhoneCallStatus(bool callOn);

    speech_type_dynamic_param_t mIdxAudioType;
    std::vector<speech_type_dynamic_param_t> mChangedXMLQueue;

protected:
    SpeechParserGen93();
    SpeechParserAttribute mSpeechParserAttribute;

private:
    /**
     * singleton pattern
     */
    static SpeechParserGen93 *uniqueSpeechParser;
    AppHandle *mAppHandle;
    int getSpeechProfile(const SpeechParserAttribute speechParserAttribute);

    void init();
    void initAppParser();
    void deInit();

    /*original api end*/
    int getSpeechParamUnit(char *packedParamUnit);
    int getGeneralParamUnit(char *bufParamUnit);
    int getMagiClarityParamUnit(char *bufParamUnit);
    int getDmnrParamUnit(char *bufParamUnit);
    /*original api end*/

    status_t getSpeechParamFromAppParser(uint16_t speechTypeIndex, AUDIO_TYPE_SPEECH_LAYERINFO_STRUCT *paramLayerInfo, char *packedParamUnit, uint16_t *sizeByteTotal);
    status_t speechDataDump(char *bufDump, uint16_t speechTypeIndex, const char *nameParam, const char *speechParamData);
    status_t setMDParamUnitHdr(speech_type_dynamic_param_t idxAudioType, SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT *paramUnitHdr, uint16_t configValue);
    uint16_t setMDParamDataHdr(SPEECH_DYNAMIC_PARAM_UNIT_HDR_STRUCT paramUnitHdr, const char *cateBandName, const char *cateNetworkName);
    int initSpeechNetwork(void);

    SPEECH_PARAM_INFO_STRUCT *mSphParamInfo;
    SPEECH_NETWORK_STRUCT *mListSpeechNetwork;
    uint8_t mNumSpeechNetwork, mSpeechParamVerFirst, mSpeechParamVerLast, mNumSpeechParam;
    SPEECH_NETWORK_STRUCT *mNameForEachSpeechNetwork;
    SPEECH_PARAM_SUPPORT_STRUCT *mSphParamSupport;
    speech_mode_t getSpeechModeByOutputDevice(const audio_devices_t output_device);
    uint32_t getMaxBufferSize();
    bool getFeatureOn(const SpeechFeatureType featureType);

};   //SpeechParserLegacy

}   //namespace android

#endif   //_SPEECH_PARSER_GEN93_H_

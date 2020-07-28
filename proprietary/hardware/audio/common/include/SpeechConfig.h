#ifndef _SPEECH_CONFIG_H_
#define _SPEECH_CONFIG_H_

/*
 * =============================================================================
 *                     external references
 * =============================================================================
 */
#include <vector>
#include <AudioParamParser.h>
#include "SpeechFeatureDef.h"

namespace android {

/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */
struct SPEECH_PARAM_SUPPORT_STRUCT;
struct SPEECH_NETWORK_STRUCT;

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

enum DRIVER_PARAM_TYPE {
    DRIVER_PARAM_COMMON_PAR,
    DRIVER_PARAM_DEBUG_INFO,
    NUM_DRIVER_PARAM
};

/*
 * =============================================================================
 *                     class
 * =============================================================================
 */

class SpeechConfig {
public:
    virtual ~SpeechConfig();
    static SpeechConfig *getInstance();
    bool getSpeechParamSupport(const SpeechFeatureType featureType);
    int getBtDelayTime(const char *btDeviceName);
    char *getNameForEachSpeechNetwork(unsigned char bitIndex);
    int getEchoRefParam(uint8_t *usbDelayMs);
    int getDriverParam(uint8_t paramType, void *paramBuf);

protected:

private:
    SpeechConfig();
    static SpeechConfig *uniqueSpeechConfig;
    AppHandle *mAppHandle;

    void init();
    void initAppParser();
    void initFeatureSupport();
    int initSpeechNetwork();
    int speechDataDump(char *dumpBuf,
                       const char *nameXml,
                       const Param *param);

    uint8_t mNumSpeechNetwork, mSpeechParamVerFirst, mSpeechParamVerLast;
    SPEECH_NETWORK_STRUCT *mListSpeechNetwork;
    SPEECH_NETWORK_STRUCT *mNameForEachSpeechNetwork;
    SPEECH_PARAM_SUPPORT_STRUCT *mSphParamSupport;


};   //SpeechConfig

}   //namespace android

#endif

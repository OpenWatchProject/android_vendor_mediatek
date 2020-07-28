#ifndef ANDROID_SPEECH_PARSER_BASE_H
#define ANDROID_SPEECH_PARSER_BASE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "SpeechParserType.h"


namespace android {

/*
 * =============================================================================
 *                     class
 * =============================================================================
 */

class SpeechParserBase {
public:
    virtual ~SpeechParserBase() {}

    /**
     * get instance's pointer
     */
    static SpeechParserBase *getInstance();

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
    virtual int getParamBuffer(SpeechParserAttribute speechParserAttribute, SpeechDataBufType *outBuf) = 0;

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
    virtual int setKeyValuePair(const SpeechStringBufType *keyValuePair) = 0;

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
    virtual int getKeyValuePair(SpeechStringBufType *keyValuePair) = 0;

    /**
     * =========================================================================
     *  @brief update phone call status from driver
     *
     *  @param callOn: the phone call status: true(On), false(Off)
     *
     *  @return int
     * =========================================================================
     */
    virtual int updatePhoneCallStatus(bool callOn) = 0;

    bool mCallOn;

protected:
    SpeechParserAttribute mSpeechParserAttribute;
    SpeechParserBase() {
        mCallOn = false;
        mSpeechParserAttribute.inputDevice = AUDIO_DEVICE_IN_BUILTIN_MIC;
        mSpeechParserAttribute.outputDevice = AUDIO_DEVICE_OUT_EARPIECE;
        mSpeechParserAttribute.idxVolume = 3;
        mSpeechParserAttribute.driverScenario = SPEECH_SCENARIO_SPEECH_ON;
        mSpeechParserAttribute.ttyMode = AUD_TTY_OFF;
        mSpeechParserAttribute.speechFeatureOn = 0;
        mParamBuf = NULL;
        mParamBufSize = 0;
    }
    void *mParamBuf;
    uint16_t mParamBufSize;

private:
    /**
     * singleton pattern
     */
    static SpeechParserBase *uniqueSpeechParser;

};



} // end namespace android

#endif // end of ANDROID_SPEECH_PARSER_BASE_H

#ifndef ANDROID_SPEECH_PARSER_GEN95_H
#define ANDROID_SPEECH_PARSER_GEN95_H

#include "SpeechParserBase.h"
#include <vector>

namespace android {

/*
 * =============================================================================
 *                     class
 * =============================================================================
 */

class SpeechParserGen95 : public SpeechParserBase {


public:
    virtual ~SpeechParserGen95();

    /**
     * get instance's pointer
     */
    static SpeechParserGen95 *getInstance();

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

    char* mMonitoredXmlName;
    std::vector<char*> mChangedXMLQueue;

protected:
    SpeechParserGen95();
    SpeechParserAttribute mSpeechParserAttribute;

private:
    /**
     * singleton pattern
     */
    static SpeechParserGen95 *uniqueSpeechParser;

    uint32_t getMaxBufferSize();
};



} // end namespace android

#endif // end of ANDROID_SPEECH_PARSER_GEN95_H

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechParserBase"
#include <SpeechParserBase.h>
#include <utils/Log.h>
#if defined(MTK_SPEECH_USIP_EMI_SUPPORT)
#include <SpeechParserGen95.h>
#else
#include <SpeechParserGen93.h>
#endif

#include <AudioLock.h>
#include <AudioUtility.h>//Mutex/assert


namespace android {

/*
 * =============================================================================
 *                     Singleton Pattern
 * =============================================================================
 */
SpeechParserBase *SpeechParserBase::uniqueSpeechParser = NULL;


SpeechParserBase *SpeechParserBase::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    ALOGV("%s()", __FUNCTION__);

    if (uniqueSpeechParser == NULL) {
#if defined(MTK_SPEECH_USIP_EMI_SUPPORT)
        uniqueSpeechParser = SpeechParserGen95::getInstance();
#else
        uniqueSpeechParser = SpeechParserGen93::getInstance();
#endif
    }
    ASSERT(uniqueSpeechParser != NULL);
    return uniqueSpeechParser;
}

} // end namespace android


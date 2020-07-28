#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechShareMemBase"
#include <SpeechShareMemBase.h>
#include <utils/Log.h>
#include <AudioLock.h>
#include <AudioAssert.h>//Mutex/assert
#if defined(MTK_SPEECH_USIP_EMI_SUPPORT)
#include <SpeechShareMemGen95.h>
#else
#include <SpeechShareMemGen93.h>
#endif


namespace android {

/*
 * =============================================================================
 *                     Singleton Pattern
 * =============================================================================
 */

SpeechShareMemBase *SpeechShareMemBase::createInstance() {
#if defined(MTK_SPEECH_USIP_EMI_SUPPORT)
    SpeechShareMemBase *speechShareMem = new SpeechShareMemGen95();
#else
    SpeechShareMemBase *speechShareMem = new SpeechShareMemGen93();
#endif
    ASSERT(speechShareMem != NULL);
    return speechShareMem;
}

SpeechShareMemBase *SpeechShareMemBase::destroyInstance(SpeechShareMemBase *speechShareMem) {
    if (speechShareMem != NULL) {
        delete speechShareMem;
    }
    return 0;
}

SpeechShareMemBase::SpeechShareMemBase() {
    mModemIndex = MODEM_1;
    mCcciShareMemoryHandler = 0;
    mShareMemoryBase = NULL;
    mShareMemoryLength = 0;
}


} // end namespace android


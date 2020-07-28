#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechParserGen95"
#include <SpeechParserGen95.h>
#include <stdlib.h>     /* atoi */
#include <utils/Log.h>
#include <AudioLock.h>
#include <AudioAssert.h>//Mutex/assert
#include <AudioParamParser.h>
#include <SpeechParser.h>//external/AudioSpeechEnhancement
#include <AudioEventThreadManager.h>


namespace android {

/*
* =============================================================================
*                     define/enum
* =============================================================================
*/
#define NUM_NEED_UPDATE_XML 3
#define LEN_XML_NAME 128
const char needUpdateXmlList[NUM_NEED_UPDATE_XML][LEN_XML_NAME] = {
    "Speech",
    "SpeechDMNR",
    "SpeechGeneral",
};
/*==============================================================================
*                     Callback Function
*============================================================================*/
void callbackAudioXmlChanged(AppHandle *appHandle, const char *audioTypeName) {
    ALOGD("%s(), audioType = %s", __FUNCTION__, audioTypeName);

    // reload XML file
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return;
    }
    bool isSpeechParamChanged = false, onlyUpdatedDuringCall = false;
    int idxXmlNeedUpdate = 0;

    if (appOps->appHandleReloadAudioType(appHandle, audioTypeName) != APP_ERROR) {
        for (idxXmlNeedUpdate = 0; idxXmlNeedUpdate < NUM_NEED_UPDATE_XML; idxXmlNeedUpdate++) {
            if (strcmp(audioTypeName, needUpdateXmlList[idxXmlNeedUpdate]) == 0) {
                isSpeechParamChanged = true;
                break;
            }
        }
        if (strcmp(audioTypeName, "Speech") == 0) {
            onlyUpdatedDuringCall = true;
        }
        if (isSpeechParamChanged) {
            if (!onlyUpdatedDuringCall) {
                SpeechParserGen95::getInstance()->mChangedXMLQueue.push_back((char *)needUpdateXmlList[idxXmlNeedUpdate]);
                AudioEventThreadManager::getInstance()->notifyCallback(AUDIO_EVENT_SPEECH_PARAM_CHANGE,
                                                                       SpeechParserGen95::getInstance());
            } else if (SpeechParserGen95::getInstance()->mCallOn) {
                SpeechParserGen95::getInstance()->mChangedXMLQueue.push_back((char *)needUpdateXmlList[idxXmlNeedUpdate]);
                AudioEventThreadManager::getInstance()->notifyCallback(AUDIO_EVENT_SPEECH_PARAM_CHANGE,
                                                                       SpeechParserGen95::getInstance());
            }
        }
    } else {
        (void) appHandle;
        ALOGE("%s(), reload xml fail!(audioType = %s)", __FUNCTION__, audioTypeName);
    }
}

/*
* =============================================================================
*                     Singleton Pattern
* =============================================================================
*/
SpeechParserGen95 *SpeechParserGen95::uniqueSpeechParser = NULL;

SpeechParserGen95 *SpeechParserGen95::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    ALOGV("%s()", __FUNCTION__);
    if (uniqueSpeechParser == NULL) {
        uniqueSpeechParser = new SpeechParserGen95();
    }
    ASSERT(uniqueSpeechParser != NULL);
    return uniqueSpeechParser;
}

/*
* =============================================================================
*                     class implementation
* =============================================================================
*/
SpeechParserGen95::SpeechParserGen95() {
    ALOGD("%s()", __FUNCTION__);
    mSpeechParserAttribute.inputDevice = AUDIO_DEVICE_IN_BUILTIN_MIC;
    mSpeechParserAttribute.outputDevice = AUDIO_DEVICE_OUT_EARPIECE;
    mSpeechParserAttribute.idxVolume = 3;
    mSpeechParserAttribute.driverScenario = SPEECH_SCENARIO_SPEECH_ON;
    mSpeechParserAttribute.ttyMode = AUD_TTY_OFF;
    mSpeechParserAttribute.speechFeatureOn = 0;
    mCallOn = false;
    mMonitoredXmlName = new char[128];
    mChangedXMLQueue.clear();
    mParamBufSize = getMaxBufferSize();
    if (mParamBufSize <= 0) {
        ALOGW("%s() mParamBufSize:%d, get buffer size fail!", __FUNCTION__, mParamBufSize);
    }
    AppHandle *appHandle = NULL;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("Error %s %d", __FUNCTION__, __LINE__);
        ASSERT(0);
    } else {
        appHandle = appOps->appHandleGetInstance();
        ALOGD("%s() appHandleRegXmlChangedCb", __FUNCTION__);
        /* XML changed callback process */
        appOps->appHandleRegXmlChangedCb(appHandle, callbackAudioXmlChanged);
    }
}

SpeechParserGen95::~SpeechParserGen95() {
    ALOGD("%s()", __FUNCTION__);
    if (mMonitoredXmlName != NULL) {
        delete(mMonitoredXmlName);
        mMonitoredXmlName = NULL;
    }
    AUDIO_FREE_POINTER(mParamBuf);
    mChangedXMLQueue.clear();

}
/**
* =========================================================================
*  @brief Parsing param file to get parameters into pOutBuf
*
*  @param speechParserAttribute: the attribute for parser
*  @param pOutBuf: the output buffer
*
*  @return int
* =========================================================================
*/
int SpeechParserGen95::getParamBuffer(SpeechParserAttribute speechParserAttribute, SpeechDataBufType *outBuf) {
    int retval = 0;
    mSpeechParserAttribute.inputDevice = speechParserAttribute.inputDevice;
    mSpeechParserAttribute.outputDevice = speechParserAttribute.outputDevice;
    mSpeechParserAttribute.idxVolume = speechParserAttribute.idxVolume;
    mSpeechParserAttribute.driverScenario = speechParserAttribute.driverScenario;
    mSpeechParserAttribute.speechFeatureOn = speechParserAttribute.speechFeatureOn;
    mSpeechParserAttribute.ttyMode = speechParserAttribute.ttyMode;
    ALOGD("%s() inputDevice: 0x%x, outputDevice: 0x%x, Volume: 0x%x, Scenario: 0x%x, FeatureOn: 0x%x, ttyMode: 0x%x",
          __FUNCTION__, mSpeechParserAttribute.inputDevice, mSpeechParserAttribute.outputDevice,
          mSpeechParserAttribute.idxVolume, mSpeechParserAttribute.driverScenario,
          mSpeechParserAttribute.speechFeatureOn, mSpeechParserAttribute.ttyMode);

    /* dynamic allocate parser buffer */
    AUDIO_FREE_POINTER(mParamBuf);
    AUDIO_ALLOC_BUFFER(mParamBuf, mParamBufSize);
    if (mParamBuf == NULL) {
        ALOGW("%s() Allocate Parser Buffer Fail!! expect:%d", __FUNCTION__, mParamBufSize);
        outBuf->memorySize = 0;
        outBuf->dataSize = 0;
        return -ENOMEM;
    }
    outBuf->bufferAddr = (char *)mParamBuf;
    outBuf->memorySize = mParamBufSize;
    outBuf->dataSize = 0;

    if (mSpeechParserAttribute.driverScenario == SPEECH_SCENARIO_PARAM_CHANGE) {
        if (mChangedXMLQueue.empty() != true) {
            mMonitoredXmlName = mChangedXMLQueue.front();
            mChangedXMLQueue.erase(mChangedXMLQueue.begin());
            ALOGD("%s() parameter changed (%s)!", __FUNCTION__, mMonitoredXmlName);
        } else {
            ALOGW("%s() parameter changed XML queue empty!", __FUNCTION__);
        }
    }
    SpOps *spOps = spOpsGetInstance();
    SpHandle *spHandle = spOps->spHandleGetInstance();
    retval = spOps->getParamBuffer(spHandle, mSpeechParserAttribute, outBuf);
    ALOGD("%s(), scenario: 0x%x, dataSize:%d, retval:%d",
          __FUNCTION__, speechParserAttribute.driverScenario, outBuf->dataSize, retval);

    return retval;
}

/**
* =========================================================================
*  @brief set keyString string to library
*
*  @param keyString the "key = value" string
*  @param sizeKeyString the size byte of string
*
*  @return int
* =========================================================================
*/
int SpeechParserGen95::setKeyValuePair(const SpeechStringBufType *keyValuePair) {
    ALOGD("+%s(), %s stringAddr =%p, memorySize = 0x%x, dataSize = 0x%x",
          __FUNCTION__,
          keyValuePair->stringAddr,
          keyValuePair->stringAddr,
          keyValuePair->memorySize,
          keyValuePair->stringSize);

    SpOps *spOps = spOpsGetInstance();
    SpHandle *spHandle = spOps->spHandleGetInstance();
    spOps->setKeyValuePair(spHandle, keyValuePair);
    ALOGD("-%s(), %s", __FUNCTION__, keyValuePair->stringAddr);
    return 0;
}

/**
* =========================================================================
*  @brief get keyString string from library
*
*  @param keyString there is only "key" when input,
and then library need rewrite "key = value" to keyString
*  @param sizeKeyString the size byte of string
*
*  @return int
* =========================================================================
*/
int SpeechParserGen95::getKeyValuePair(SpeechStringBufType *keyValuePair) {
    ALOGD("+%s(), %s stringAddr = 0x%p, memorySize = 0x%x, dataSize = 0x%x",
          __FUNCTION__,
          keyValuePair->stringAddr,
          keyValuePair->stringAddr,
          keyValuePair->memorySize,
          keyValuePair->stringSize);
    SpOps *spOps = spOpsGetInstance();
    SpHandle *spHandle = spOps->spHandleGetInstance();
    spOps->getKeyValuePair(spHandle, keyValuePair);
    ALOGD("-%s(),%s", __FUNCTION__, keyValuePair->stringAddr);
    return 0;
}

/**
* =========================================================================
*  @brief update phone call status from driver
*
*  @param callOn: the phone call status: true(On), false(Off)
*
*  @return int
* =========================================================================
*/
int SpeechParserGen95::updatePhoneCallStatus(bool callOn) {
    ALOGD("%s(), callOn:%d", __FUNCTION__, callOn);
    if (callOn == false) {
        AUDIO_FREE_POINTER(mParamBuf);
    }
    if (mCallOn == callOn) {
        ALOGW("%s(), callOn(%d) == mCallOn(%d), return",
              __FUNCTION__, callOn, mCallOn);
        return 0;
    }
    mCallOn = callOn;
    return 0;
}

uint32_t SpeechParserGen95::getMaxBufferSize() {
    uint32_t paramBufSize = 0;
    char keyString[MAX_SPEECH_PARSER_KEY_LEN];
    memset((void *)keyString, 0, MAX_SPEECH_PARSER_KEY_LEN);

    SpeechStringBufType keyValuePair;
    memset(&keyValuePair, 0, sizeof(SpeechStringBufType));
    keyValuePair.memorySize = strlen(keyString) + 1;
    keyValuePair.stringSize = strlen(keyString);
    keyValuePair.stringAddr = keyString;

    snprintf(keyString, MAX_SPEECH_PARSER_KEY_LEN, "%s,%s", SPEECH_PARSER_GET_KEY_PREFIX, SPEECH_PARSER_PARAMBUF_SIZE);
    //get from default parser
    SpOps *spOps = spOpsGetInstance();
    SpHandle *spHandle = spOps->spHandleGetInstance();
    spOps->getKeyValuePair(spHandle, &keyValuePair);
    paramBufSize = (uint32_t) atoi(keyValuePair.stringAddr);
    return paramBufSize;

}
} // end namespace android


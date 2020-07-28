#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechShareMemGen95"
#include <SpeechShareMemGen95.h>
#include <SpeechExtMemCCCI.h>
#include <SpeechExtMemUSIP.h>
#include <utils/Log.h>
#include <AudioLock.h>
#include <AudioAssert.h>//Mutex/assert


namespace android {

/*
 * =============================================================================
 *                     class implementation
 * =============================================================================
 */
SpeechShareMemGen95::SpeechShareMemGen95() {
    ALOGD("%s()", __FUNCTION__);
    mSpeechExtMemCCCI = new SpeechExtMemCCCI();
    if (mSpeechExtMemCCCI == NULL) {
        ALOGE("%s(), mSpeechExtMemCCCI == NULL!!", __FUNCTION__);
    }
    mSpeechExtMemUSIP = new SpeechExtMemUSIP();
    if (mSpeechExtMemUSIP == NULL) {
        ALOGE("%s(), mSpeechExtMemUSIP == NULL!!", __FUNCTION__);
    }
}

SpeechShareMemGen95::~SpeechShareMemGen95() {
    if (mSpeechExtMemCCCI != NULL) {
        delete mSpeechExtMemCCCI;
        mSpeechExtMemCCCI = NULL;
    }
    if (mSpeechExtMemUSIP != NULL) {
        delete mSpeechExtMemUSIP;
        mSpeechExtMemUSIP = NULL;
    }
}

int SpeechShareMemGen95::resetShareMemoryIndex() {
    int retval = 0;
    retval = mSpeechExtMemCCCI->resetShareMemoryIndex();
    if (retval < 0) {
        ALOGE("%s(), open CCCI Fail.", __FUNCTION__);
        return retval;
    }
    retval = mSpeechExtMemUSIP->resetShareMemoryIndex();
    if (retval < 0) {
        ALOGE("%s(), open USIP Fail.", __FUNCTION__);
    }
    return retval;
}

int SpeechShareMemGen95::writeSphParamToShareMemory(const void *p_sph_param,
                                                    uint32_t sph_param_length,
                                                    uint32_t *p_write_idx) {
    int retval = 0;
    ALOGV("%s()", __FUNCTION__);
    retval = mSpeechExtMemUSIP->writeSphParamToShareMemory(p_sph_param,
                                                           sph_param_length,
                                                           p_write_idx);
    return retval;
}

int SpeechShareMemGen95::writeApDataToShareMemory(const void *data_buf,
                                                  uint16_t data_type,
                                                  uint16_t data_size,
                                                  uint16_t *p_payload_length,
                                                  uint32_t *p_write_idx) {
    int retval = 0;
    retval = mSpeechExtMemCCCI->writeApDataToShareMemory(data_buf,
                                                         data_type,
                                                         data_size,
                                                         p_payload_length,
                                                         p_write_idx);
    return retval;
}

int SpeechShareMemGen95::readMdDataFromShareMemory(void *p_data_buf,
                                                   uint16_t *p_data_type,
                                                   uint16_t *p_data_size,
                                                   uint16_t payload_length,
                                                   uint32_t read_idx) {
    int retval = 0;
    retval = mSpeechExtMemCCCI->readMdDataFromShareMemory(p_data_buf,
                                                          p_data_type,
                                                          p_data_size,
                                                          payload_length,
                                                          read_idx);
    return retval;
}

int SpeechShareMemGen95::openShareMemory(const modem_index_t modem_index) {
    int retval = 0, retUsip = 0;
    mModemIndex = modem_index;
    retval = mSpeechExtMemCCCI->openShareMemory(mModemIndex);
    if (retval < 0) {
        ALOGE("%s(), open CCCI Fail.", __FUNCTION__);
        return retval;
    }
    retUsip = mSpeechExtMemUSIP->openShareMemory(mModemIndex);
    if (retUsip < 0) {
        ALOGE("%s(), open USIP Fail.", __FUNCTION__);
    }
    return retval;
}

int SpeechShareMemGen95::closeShareMemory() {
    int retval = 0, retUsip = 0;
    retval = mSpeechExtMemCCCI->closeShareMemory();
    if (retval < 0) {
        ALOGE("%s(), open CCCI Fail.", __FUNCTION__);
        return retval;
    }
    retUsip = mSpeechExtMemUSIP->closeShareMemory();
    if (retUsip < 0) {
        ALOGE("%s(), open USIP Fail.", __FUNCTION__);
    }
    return retval;

}

int SpeechShareMemGen95::formatShareMemory() {
    int retval = 0, retUsip = 0;
    retval = mSpeechExtMemCCCI->formatShareMemory();
    if (retval < 0) {
        ALOGE("%s(), open CCCI Fail.", __FUNCTION__);
        return retval;
    }
    retUsip = mSpeechExtMemUSIP->formatShareMemory();
    if (retUsip < 0) {
        ALOGE("%s(), open USIP Fail.", __FUNCTION__);
    }
    return retval;
}

bool SpeechShareMemGen95::checkModemAlive() {
    bool isAlive = 0;
    isAlive = mSpeechExtMemCCCI->checkModemAlive();
    return isAlive;
}

int SpeechShareMemGen95::getShareMemoryType() {
    return SPH_PARAM_VIA_SHM_USIP;
}


} // end namespace android


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechShareMemGen93"
#include <SpeechShareMemGen93.h>
#include <SpeechExtMemCCCI.h>
#include <utils/Log.h>
#include <AudioLock.h>
#include <AudioAssert.h>//Mutex/assert


namespace android {

/*
 * =============================================================================
 *                     class implementation
 * =============================================================================
 */
SpeechShareMemGen93::SpeechShareMemGen93() {
    mSpeechExtMemCCCI = new SpeechExtMemCCCI();
    if (mSpeechExtMemCCCI == NULL) {
        ALOGE("%s(), mSpeechExtMemCCCI == NULL!!", __FUNCTION__);
    }
}

SpeechShareMemGen93::~SpeechShareMemGen93() {
    if (mSpeechExtMemCCCI != NULL) {
        delete mSpeechExtMemCCCI;
    }
}

int SpeechShareMemGen93::resetShareMemoryIndex() {
    return mSpeechExtMemCCCI->resetShareMemoryIndex();
}

int SpeechShareMemGen93::writeSphParamToShareMemory(const void *p_sph_param,
                                                    uint32_t sph_param_length,
                                                    uint32_t *p_write_idx) {
    return mSpeechExtMemCCCI->writeSphParamToShareMemory(p_sph_param,
                                                         sph_param_length,
                                                         p_write_idx);
}

int SpeechShareMemGen93::writeApDataToShareMemory(const void *data_buf,
                                                  uint16_t data_type,
                                                  uint16_t data_size,
                                                  uint16_t *p_payload_length,
                                                  uint32_t *p_write_idx) {
    return mSpeechExtMemCCCI->writeApDataToShareMemory(data_buf,
                                                       data_type,
                                                       data_size,
                                                       p_payload_length,
                                                       p_write_idx);
}

int SpeechShareMemGen93::readMdDataFromShareMemory(void *p_data_buf,
                                                   uint16_t *p_data_type,
                                                   uint16_t *p_data_size,
                                                   uint16_t payload_length,
                                                   uint32_t read_idx) {
    return mSpeechExtMemCCCI->readMdDataFromShareMemory(p_data_buf,
                                                        p_data_type,
                                                        p_data_size,
                                                        payload_length,
                                                        read_idx);
}

int SpeechShareMemGen93::openShareMemory(const modem_index_t modem_index) {
    mModemIndex = modem_index;
    return mSpeechExtMemCCCI->openShareMemory(mModemIndex);
}

int SpeechShareMemGen93::closeShareMemory() {
    return mSpeechExtMemCCCI->closeShareMemory();
}

int SpeechShareMemGen93::formatShareMemory() {
    return mSpeechExtMemCCCI->formatShareMemory();
}

int SpeechShareMemGen93::getShareMemoryType() {
    return SPH_PARAM_VIA_SHM_CCCI;
}

bool SpeechShareMemGen93::checkModemAlive() {
    return mSpeechExtMemCCCI->checkModemAlive();
}

} // end namespace android


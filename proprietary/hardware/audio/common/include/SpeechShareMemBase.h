#ifndef ANDROID_SPEECH_SHAREMEM_BASE_H
#define ANDROID_SPEECH_SHAREMEM_BASE_H

#include "SpeechType.h"


namespace android {

/*
 * =============================================================================
 *                     class
 * =============================================================================
 */

class SpeechShareMemBase {
public:
    static SpeechShareMemBase *createInstance();
    static SpeechShareMemBase *destroyInstance(SpeechShareMemBase *speechShareMem);
    /** virtual dtor */
    virtual ~SpeechShareMemBase() {}

    /** implementation according to different platform */

    virtual int resetShareMemoryIndex() = 0;

    virtual int writeSphParamToShareMemory(const void *p_sph_param,
                                           uint32_t sph_param_length,
                                           uint32_t *p_write_idx) = 0;

    virtual int writeApDataToShareMemory(const void *data_buf,
                                         uint16_t data_type,
                                         uint16_t data_size,
                                         uint16_t *p_payload_length,
                                         uint32_t *p_write_idx) = 0;

    virtual int readMdDataFromShareMemory(void *p_data_buf,
                                          uint16_t *p_data_type,
                                          uint16_t *p_data_size,
                                          uint16_t payload_length,
                                          uint32_t read_idx) = 0;

    virtual int openShareMemory(const modem_index_t modem_index) = 0;
    virtual int closeShareMemory() = 0;
    virtual int formatShareMemory() = 0;
    virtual bool checkModemAlive() = 0;
    virtual int getShareMemoryType() = 0;

protected:
    SpeechShareMemBase();
    modem_index_t mModemIndex;
    int mCcciShareMemoryHandler;

    unsigned char *mShareMemoryBase;
    unsigned int mShareMemoryLength;

private:
    static SpeechShareMemBase *uniqueSpeechShareMem;

};

} /* end of namespace android */

#endif /* end of ANDROID_SPEECH_SHAREMEM_BASE_H */


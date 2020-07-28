#ifndef ANDROID_SPEECH_SHAREMEM_GEN93_H
#define ANDROID_SPEECH_SHAREMEM_GEN93_H

#include "SpeechShareMemBase.h"

namespace android {

/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */
class SpeechExtMemCCCI;

class SpeechShareMemGen93 : public SpeechShareMemBase {

public:
    SpeechShareMemGen93();
    /** virtual dtor */
    virtual ~SpeechShareMemGen93();

    /** implementation according to different platform */

    virtual int resetShareMemoryIndex();

    virtual int writeSphParamToShareMemory(const void *p_sph_param,
                                           uint32_t sph_param_length,
                                           uint32_t *p_write_idx);

    virtual int writeApDataToShareMemory(const void *data_buf,
                                         uint16_t data_type,
                                         uint16_t data_size,
                                         uint16_t *p_payload_length,
                                         uint32_t *p_write_idx);

    virtual int readMdDataFromShareMemory(void *p_data_buf,
                                          uint16_t *p_data_type,
                                          uint16_t *p_data_size,
                                          uint16_t payload_length,
                                          uint32_t read_idx);

    virtual int openShareMemory(const modem_index_t modem_index);
    virtual int closeShareMemory();
    virtual int formatShareMemory();
    virtual bool checkModemAlive();
    virtual int getShareMemoryType();

private:
    SpeechExtMemCCCI *mSpeechExtMemCCCI;

};

} /* end of namespace android */

#endif /* end of ANDROID_SPEECH_SHAREMEM_GEN93_H */


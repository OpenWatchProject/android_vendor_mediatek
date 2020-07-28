#ifndef ANDROID_SPEECH_EXTMEM_USIP_H
#define ANDROID_SPEECH_EXTMEM_USIP_H

#include "SpeechType.h"
#include <AudioLock.h>

namespace android {
/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */
struct region_info_t;
struct sph_usip_shm_t;

/*
 * =============================================================================
 *                     class
 * =============================================================================
 */
class SpeechExtMemUSIP {
public:
    SpeechExtMemUSIP();
    /** virtual dtor */
    virtual ~SpeechExtMemUSIP();

    /** implementation according to different platform */

    virtual int resetShareMemoryIndex();
    virtual int writeSphParamToShareMemory(const void *p_sph_param,
                                           uint32_t sph_param_length,
                                           uint32_t *p_write_idx);
    virtual int openShareMemory(const modem_index_t modem_index);
    virtual int closeShareMemory();
    virtual int formatShareMemory();

private:
    modem_index_t mModemIndex;
    int mCcciShareMemoryHandler;
    AudioLock    mShareMemoryHandlerLock;

    unsigned char *mShareMemoryBase;
    unsigned int mShareMemoryLength;

    sph_usip_shm_t *mShareMemory;
    AudioLock    mShareMemoryLock;

    region_info_t* mSpeechParamRegion;

    uint32_t shm_region_data_count(region_info_t *p_region);
    uint32_t shm_region_free_space(region_info_t *p_region);
    void shm_region_write_from_linear(region_info_t *p_region,
                                      const void *linear_buf,
                                      uint32_t count);
    void shm_region_read_to_linear(void *linear_buf,
                                   region_info_t *p_region,
                                   uint32_t count);
    int updateWriteIndex(region_info_t *p_region, uint32_t count);

};

} /* end of namespace android */

#endif /* end of ANDROID_SPEECH_EXTMEM_USIP_H */


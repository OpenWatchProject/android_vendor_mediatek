#ifndef ANDROID_SPEECH_EXTMEM_CCCI_H
#define ANDROID_SPEECH_EXTMEM_CCCI_H

#include "SpeechType.h"

#include <AudioLock.h>
namespace android {
/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */
struct region_info_t;
struct sph_shm_region_t;
struct sph_shm_t;

class SpeechExtMemCCCI {
public:
    SpeechExtMemCCCI();
    /** virtual dtor */
    virtual ~SpeechExtMemCCCI();

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

private:
    modem_index_t mModemIndex;
    int mCcciShareMemoryHandler;
    AudioLock    mShareMemoryHandlerLock;

    unsigned char *mShareMemoryBase;
    unsigned int mShareMemoryLength;

    sph_shm_t *mShareMemory;//shm struc
    AudioLock    mShareMemoryLock;//lock for shm struc

    AudioLock    mShareMemorySpeechParamLock;
    AudioLock    mShareMemoryApDataLock;
    AudioLock    mShareMemoryMdDataLock;

    uint32_t shm_region_data_count(region_info_t *p_region);
    uint32_t shm_region_free_space(region_info_t *p_region);
    void shm_region_write_from_linear(region_info_t *p_region,
                                      const void *linear_buf,
                                      uint32_t count);
    void shm_region_read_to_linear(void *linear_buf,
                                   region_info_t *p_region,
                                   uint32_t count);

};

} /* end of namespace android */

#endif /* end of ANDROID_SPEECH_EXTMEM_CCCI_H */


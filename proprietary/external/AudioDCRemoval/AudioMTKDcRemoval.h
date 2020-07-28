#ifndef AUDIO_MTK_DCREMOVE_PROCESS_H
#define AUDIO_MTK_DCREMOVE_PROCESS_H
#include <stdint.h>
#include <sys/types.h>
#include <cutils/log.h>
#include <utils/threads.h>

#ifdef __cplusplus
            extern "C" {
#endif    

#include <dc_removal_flt.h>
#ifdef __cplusplus
}
#endif
namespace android
{

class DcRemove
{
    public:
        enum {
            DCR_MODE_1 = 0,
            DCR_MODE_2,
            DCR_MODE_3
        };
        enum {
            DCREMOVE_BIT16 = 0,
            DCREMOVE_BIT24,
        };
        DcRemove();
        ~DcRemove();
        status_t init(uint32_t channel, uint32_t samplerate, uint32_t drcMode, uint32_t dBit = DCREMOVE_BIT16);
        size_t process(const void *inbuffer, size_t bytes, void *outbuffer);
        status_t close();
    private:
        DCRemove_Handle *mHandle;
        signed char *p_internal_buf;
        DcRemove(const DcRemove &);
        DcRemove &operator=(const DcRemove &);
        mutable Mutex  mLock;
        uint32_t mBitProcess;
        size_t mSamplerate;
};
};
#endif

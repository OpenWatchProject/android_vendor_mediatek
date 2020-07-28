#ifndef AUDIO_MTK_DCREMOVE_PROCESS_H
#define AUDIO_MTK_DCREMOVE_PROCESS_H
#include <stdint.h>
#include <sys/types.h>
#include <cutils/log.h>
#include <utils/threads.h>
#include "MtkAudioComponent.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <dc_removal_flt.h>
#ifdef __cplusplus
}
#endif
namespace android
{

class MtkAudioDcRemove : public MtkAudioDcRemoveBase
{
    public:
        MtkAudioDcRemove();
        virtual ACE_ERRID init(uint32_t channel, uint32_t samplerate, uint32_t drcMode,
                               uint32_t dBit = DCREMOVE_BIT16);
        virtual ACE_ERRID setParameter(uint32_t paramID, void *param);
        virtual ACE_ERRID getParameter(uint32_t paramID, void *param);
        virtual ACE_ERRID open(void);
        virtual ACE_ERRID close(void);
        virtual ACE_ERRID resetBuffer(void);
        virtual ACE_ERRID process(void *pInputBuffer,   /* Input, pointer to input buffer */
                                  uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                                  /* Output, length(byte) left in the input buffer after conversion */
                                  void *pOutputBuffer,           /* Input, pointer to output buffer */
                                  uint32_t *OutputSampleCount);  /* Input, length(byte) of output buffer */
        virtual ~MtkAudioDcRemove();
    private:
        DCRemove_Handle *mHandle;
        signed char *p_internal_buf;
        MtkAudioDcRemove(const MtkAudioDcRemove &);
        MtkAudioDcRemove &operator=(const MtkAudioDcRemove &);
        mutable Mutex  mLock;
        size_t mSamplerate;
        uint32_t mBitProcess;
};

};
#endif

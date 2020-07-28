#ifndef ANDROID_SPEECH_PCM_MIXER_BASE_H
#define ANDROID_SPEECH_PCM_MIXER_BASE_H

#include <pthread.h>
#include <AudioType.h>
#include "AudioUtility.h"
#include <MtkAudioComponent.h>
#include <AudioLock.h>
#include "SpeechDriverInterface.h"

namespace android {

#if defined(SPH_SR48K) || defined(SPH_SR32K)
#define PCM_MIXER_TARGET_SAMPLE_RATE  (32000)
#else
#define PCM_MIXER_TARGET_SAMPLE_RATE  (16000)
#endif

// PCM_MIXER ring buffer
#define MODEM_FRAME_MS (20)
#define PCM_MIXER_KEEP_NUM_FRAME (4)
#define PCM_MIXER_EXTRA_NUM_FRAME (3)

#if PCM_MIXER_KEEP_NUM_FRAME < PCM_MIXER_EXTRA_NUM_FRAME
#error PCM_MIXER_KEEP_NUM_FRAME >= PCM_MIXER_EXTRA_NUM_FRAME
#endif

// pcm_16_bit * 1ch * sample rate * ms
#define PCM_MIXER_PERIOD_SIZE \
    ((2 * 1 * PCM_MIXER_TARGET_SAMPLE_RATE * MODEM_FRAME_MS) / 1000)

#define PCM_MIXER_PLAY_BUFFER_LEN \
    ((PCM_MIXER_PERIOD_SIZE * PCM_MIXER_KEEP_NUM_FRAME) + RING_BUF_SIZE_OFFSET)


enum { /* uint32_t: pcm_mixer_log_level_t */ /* PROPERTY_KEY_PCM_MIXER_LOG_LEVEL */
    PCM_MIXER_LOG_LEVEL_PLAYBACK_HANDLER = 1,
    PCM_MIXER_LOG_LEVEL_MODEM = 2,
    PCM_MIXER_LOG_LEVEL_PCM_MIXER = 4
};

enum {
    PCM_MIXER_TYPE_BGS = 0,
    PCM_MIXER_TYPE_VOIPRX = 1,
    PCM_MIXER_TYPE_TELEPHONYTX = 2
};


bool getPcmMixerLogEnableByLevel(const uint32_t pcm_mixer_log_level); /* pcm_mixer_log_level_t */


/*=============================================================================
 *                              Class definition
 *===========================================================================*/

class SpeechPcmMixerBase;
class SpeechDriverInterface;

class SpeechPcmMixerBaseBuffer {
public:
    bool        isBufferEnough(void);

private:
    SpeechPcmMixerBaseBuffer();
    virtual ~SpeechPcmMixerBaseBuffer(); // only destroied by friend class SpeechPcmMixerBase
    friend          class SpeechPcmMixerBase;
    virtual status_t        InitPcmMixerBuffer(SpeechPcmMixerBase *playPointer, uint32_t sampleRate, uint32_t chNum, int32_t mFormat, uint32_t pcmMixerType);
    virtual uint32_t        Write(char *buf, uint32_t num);
    virtual bool        IsPcmMixerBlisrcDumpEnable();

    int32_t         mFormat;
    RingBuf         mRingBuf;
    MtkAudioSrcBase     *mBliSrc;
    char           *mBliOutputLinearBuffer;

    AudioLock       mPcmMixerBufferRuningMutex;
    AudioLock       mPcmMixerBufferMutex;

    bool            mExitRequest;

    bool        mIsPcmMixerBlisrcDumpEnable;
    FILE        *pDumpFile;
};

class SpeechPcmMixerBase {
public:
    virtual ~SpeechPcmMixerBase();

    virtual SpeechPcmMixerBaseBuffer      *CreatePcmMixerBuffer(uint32_t sampleRate, uint32_t chNum, int32_t format, uint32_t pcmMixerType);
    virtual void                DestroyPcmMixerBuffer(SpeechPcmMixerBaseBuffer *pPcmMixerBuffer);
    virtual bool                Open(SpeechDriverInterface *pSpeechDriver);
    virtual bool                Close();
    virtual bool                IsPcmMixerDumpEnable();
    virtual uint32_t            Write(SpeechPcmMixerBaseBuffer *pPcmMixerBuffer, void *buf, uint32_t num);
    virtual uint32_t            PutData(SpeechPcmMixerBaseBuffer *pPcmMixerBuffer, char *target_ptr, uint16_t num_data_request);
    virtual uint16_t            PutDataToSpeaker(char *target_ptr, uint16_t num_data_request);
    virtual status_t            pcmMixerOn(SpeechDriverInterface *pSpeechDriver) = 0;
    virtual status_t            pcmMixerOff(SpeechDriverInterface *pSpeechDriver) = 0;
    virtual uint32_t            getPcmMixerType() = 0;

protected:
    SpeechPcmMixerBase();

private:
    SpeechDriverInterface        *mSpeechDriver;
    SortedVector<SpeechPcmMixerBaseBuffer *> mPcmMixerBufferVector;

    char        *mBufBaseTemp;

    AudioLock    mPcmMixerBufferVectorLock;
    AudioLock    mCountLock;
    uint16_t     mCount;

    bool        mIsPcmMixerDumpEnable;
    FILE        *pDumpFile;

    uint16_t    mPcmMixerPeriodSize;
    uint16_t    mUnderflowCount;
};


} // end namespace android

#endif //ANDROID_SPEECH_PCM_MIXER_BASE_H
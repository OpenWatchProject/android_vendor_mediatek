#ifndef ANDROID_AUDIO_MIXER_OUT_H
#define ANDROID_AUDIO_MIXER_OUT_H

#include <system/audio.h>
#include <utils/KeyedVector.h>
#include <pthread.h>

#include "AudioType.h"
#include "AudioUtility.h"
#include "AudioLock.h"

namespace android {

enum MIXER_USAGE {
    MIXER_USAGE_UNKNOWN,    // just for error handle
    MIXER_USAGE_BT,
    MIXER_USAGE_DEEP_FAST,  // temp, pre-design
    MIXER_USAGE_SMARTPA,
    MIXER_USAGE_NUM,
};

class MtkAudioSrcBase;
class AudioALSAPlaybackHandlerBase;
class AudioALSAStreamManager;

struct MixerOutClient {
    const void *id;
    stream_attribute_t attribute;
    RingBuf dataBuffer;

    bool suspend;

    AudioLock *dataBufferLock;

    // blisrc
    MtkAudioSrcBase *blisrc;
    char *blisrcOutBuffer;

    // bit convert
    audio_format_t dstFmt;
    audio_format_t srcFmt;
    char *bitConvertBuffer;

    // screen state
    bool screenMode;
    size_t screenBufferSize;
    size_t screenReduceInterruptSize;
    bool screenForce;
    uint64_t writtenBytes;
    int64_t startTime;
    uint32_t unitSleepUs;
    uint32_t errorThdSleepUs;
    int isDumpLatency;
};

typedef KeyedVector<const void *, struct MixerOutClient *> MixerOutClientVector;

struct MixerOutInfo {
    const void *id;
    enum MIXER_USAGE usage;

    stream_attribute_t attribute;
    stream_attribute_t attribute2;

    AudioLock *threadLock;
    AudioLock *waitSuspendLock;
    AudioLock *waitOutThreadLock;

    MixerOutClientVector *clients;
    bool clientAllSuspend;

    // bit convert
    audio_format_t dstFmt;
    audio_format_t srcFmt;
    char *bitConvertBuffer;

    unsigned int readBufferTimeUs;
    unsigned int minWriteFrameCnt;

    AudioALSAPlaybackHandlerBase *playHandler;
    AudioALSAPlaybackHandlerBase *playHandler2;

    // screen state
    bool screenMode;
    size_t screenBufferSize;
    size_t screenReduceInterruptSize;
    bool screenForce;
};

class AudioMixerOut {
public:
    static AudioMixerOut *getInstance(enum MIXER_USAGE usage);
    ~AudioMixerOut();

    status_t attach(const void *id, const stream_attribute_t *attribute);
    void detach(const void *id);

    size_t write(const void *id, const void *buffer, size_t bytes);
    status_t setSuspend(const void *id, bool suspend);

    status_t getHardwareBufferInfo(const void *id, time_info_struct_t *HWBuffer_Time_Info);
    status_t setScreenState(const void *id, bool mode, size_t bufferSize, size_t reduceInterruptSize, bool force = false);

    int getLatency();

private:
    static AudioMixerOut *mAudioMixerOut;
    AudioMixerOut(enum MIXER_USAGE usage);

    status_t createOutThread();
    void destroyOutThread();

    void deleteClient(struct MixerOutClient *client);

    static status_t setScreenState_l(struct MixerOutInfo *info);

    static void *outThread(void *arg);

    static int destroyPlaybackHandler(AudioALSAPlaybackHandlerBase *playbackHandler,
                                      AudioALSAStreamManager *streamManager);
    static bool clientAllSuspend(const MixerOutClientVector *clients);
    static int waitSignal(AudioLock *mWaitLock, unsigned int waitTimeUs, const char *dbgString);

    // dump
    static FILE *mixerOutDumpOpen(const char *name, const char *property, const struct MixerOutInfo *info);
    static void mixerOutDumpClose(FILE *file);
    static void mixerOutDumpWriteData(FILE *file, const void *buffer, size_t bytes);

    // blisrc
    static status_t initBliSrc(struct MixerOutClient *client, const struct MixerOutInfo *outInfo);
    static status_t deinitBliSrc(struct MixerOutClient *client);
    static status_t doBliSrc(struct MixerOutClient *client, void *pInBuffer, uint32_t inBytes,
                             void **ppOutBuffer, uint32_t *pOutBytes);

    // bit convert
    static unsigned int getBitConvertDstBufferSize(audio_format_t dstFmt,
                                                   audio_format_t srcFmt,
                                                   unsigned int srcBufSizeByte);
    static status_t initBitConverter(struct MixerOutClient *client, audio_format_t dstFmt);
    static status_t initBitConverter(struct MixerOutInfo *info, audio_format_t srcFmt);
    template<class T>
    static status_t initBitConverter(T *client, audio_format_t srcFmt, audio_format_t dstFmt);
    template<class T>
    static status_t deinitBitConverter(T *client);
    template<class T>
    static status_t doBitConversion(T *client,
                                    void *pInBuffer, uint32_t inBytes,
                                    void **ppOutBuffer, uint32_t *pOutBytes);
private:
    AudioLock mLock;
    AudioLock mWaitSuspendLock;
    AudioLock mWaitOutThreadLock;
    AudioLock mThreadLock;

    enum MIXER_USAGE mUsage;
    struct MixerOutInfo mOutInfo;

    pthread_t mOutThread;

    /**
     * client vector
     */
    MixerOutClientVector mClients;
    KeyedVector<const void *, AudioLock *> mClientsLock;

    int mDebugType;
};

}
#endif


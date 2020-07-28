#ifndef AUDIO_UTILITY_H
#define AUDIO_UTILITY_H


#include <stdlib.h>
#include <stdio.h>
#include "AudioDef.h"
#include "AudioType.h"
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <stdint.h>
#include <sys/stat.h>
#include <utils/Vector.h>
#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <time.h>
#include <cutils/atomic.h>

#if defined(PC_EMULATION)
#include "Windows.h"
#include "WinBase.h"
#else
#include <cutils/atomic.h>

#include <log/log.h>
#include <cutils/properties.h>
#endif

#include <tinyalsa/asoundlib.h>
#include "MtkAudioComponent.h"
#include "audio_custom_exp.h"
#include "AudioCompFltCustParam.h"

/*
    this function should implement the basic function for debug information
    or basic function proviede to audio hardware modules
*/

struct audio_ringbuf_t;
#define UNUSED(x) ((void)(x))

namespace android {
#define MAX_DUMP_NUM (1024)

enum {
    AUDIO_LOG_HARDWARE = 1,
    AUDIO_LOG_A2DPHARDWARE,
    AUDIO_LOG_STREAMOUT,
    AUDIO_LOG_STREAMIN,
    AUDIO_LOG_I2SSTREAMIN,
    AUDIO_LOG_VOLUMECONTROLLER,
    AUDIO_LOG_RESOURCEMANAGER,
    AUDIO_LOG_AUDIODEVCE,
    AUDIO_LOG_ANALOG,
    AUDIO_LOG_DIGITAL,
    AUDIO_LOG_AUDIOFLINGER,
    AUDIO_LOG_SPEECHCONTROL,
    AUDIO_LOG_AUDIOPOLICYSERVICE,
    AUDIO_LOG_AUDIOPOLICYANAGER,
    AUDIO_LOG_AUDIOMIXER
};

enum {
    AUDIO_SUPPORT_DMIC = 1,
    AUDIO_SUPPORT_2IN1_SPEAKER,
    AUDIO_SUPPORT_VIBRATION_SPEAKER,
    AUDIO_SUPPORT_EXTERNAL_BUILTIN_MIC,
    AUDIO_SUPPORT_EXTERNAL_ECHO_REFERENCE,
    AUDIO_SUPPORT_VOW_DUAL_MIC,
};

enum {
    AUDIO_SPK_INVALID = -1,
    AUDIO_SPK_INTAMP,
    AUDIO_SPK_EXTAMP_LO,
    AUDIO_SPK_EXTAMP_HP,
    AUDIO_SPK_2_IN_1,
    AUDIO_SPK_3_IN_1,
};

enum {
    AUDIO_I2S_INVALID = -1,
    AUDIO_I2S0,
    AUDIO_I2S1,
    AUDIO_I2S2,
    AUDIO_I2S3,
    AUDIO_I2S5,
    AUDIO_I2S6,
    AUDIO_I2S7,
    AUDIO_I2S8,
    AUDIO_I2S9,
    AUDIO_TINYCONN_I2S0,
    AUDIO_TINYCONN_I2S1,
    AUDIO_TINYCONN_I2S2,
    AUDIO_TINYCONN_I2S3,
    AUDIO_TINYCONN_I2S5,
    AUDIO_TINYCONN_I2S6,
    AUDIO_TINYCONN_I2S7,
    AUDIO_TINYCONN_I2S8,
    AUDIO_TINYCONN_I2S9,
};

// TurnOnSequence type
enum {
    TURN_ON_SEQUENCE_1,
    TURN_ON_SEQUENCE_2,
    TURN_ON_SEQUENCE_3,
    TURN_ON_SEQUENCE_DSP,
};

static enum pcm_format pcm_format_from_audio_format(audio_format_t format) {
    switch (format) {
#ifdef HAVE_BIG_ENDIAN
    case AUDIO_FORMAT_PCM_16_BIT:
        return PCM_FORMAT_S16_BE;
    case AUDIO_FORMAT_PCM_24_BIT_PACKED:
        return PCM_FORMAT_S24_3BE;
    case AUDIO_FORMAT_PCM_32_BIT:
        return PCM_FORMAT_S32_BE;
    case AUDIO_FORMAT_PCM_8_24_BIT:
        return PCM_FORMAT_S24_BE;
#else
    case AUDIO_FORMAT_PCM_16_BIT:
        return PCM_FORMAT_S16_LE;
    case AUDIO_FORMAT_PCM_24_BIT_PACKED:
        return PCM_FORMAT_S24_3LE;
    case AUDIO_FORMAT_PCM_32_BIT:
        return PCM_FORMAT_S32_LE;
    case AUDIO_FORMAT_PCM_8_24_BIT:
        return PCM_FORMAT_S24_LE;
#endif
    case AUDIO_FORMAT_PCM_FLOAT:  /* there is no equivalent for float */
    default:
        LOG_ALWAYS_FATAL("pcm_format_from_audio_format: invalid audio format %#x", format);
        return PCM_FORMAT_S16_LE;//0; will build error in hardware/audio_alsaops.h
    }
}

/* Converts pcm_format to audio_format.
 * Parameters:
 *  format  the pcm_format to convert
 *
 * Logs a fatal error if format is not a valid convertible pcm_format.
 */
static audio_format_t audio_format_from_pcm_format(enum pcm_format format) {
    switch (format) {
#ifdef HAVE_BIG_ENDIAN
    case PCM_FORMAT_S16_BE:
        return AUDIO_FORMAT_PCM_16_BIT;
    case PCM_FORMAT_S24_3BE:
        return AUDIO_FORMAT_PCM_24_BIT_PACKED;
    case PCM_FORMAT_S24_BE:
        return AUDIO_FORMAT_PCM_8_24_BIT;
    case PCM_FORMAT_S32_BE:
        return AUDIO_FORMAT_PCM_32_BIT;
#else
    case PCM_FORMAT_S16_LE:
        return AUDIO_FORMAT_PCM_16_BIT;
    case PCM_FORMAT_S24_3LE:
        return AUDIO_FORMAT_PCM_24_BIT_PACKED;
    case PCM_FORMAT_S24_LE:
        return AUDIO_FORMAT_PCM_8_24_BIT;
    case PCM_FORMAT_S32_LE:
        return AUDIO_FORMAT_PCM_32_BIT;
#endif
    default:
        LOG_ALWAYS_FATAL("audio_format_from_pcm_format: invalid pcm format %#x", format);
        return AUDIO_FORMAT_DEFAULT;//0; will build error in hardware/audio_alsaops.h
    }
}

class AudioTimeStamp {
public:
    AudioTimeStamp();
    ~AudioTimeStamp();
    void SetSystemTimeValid(const char str);
private:
    struct timespec systemtime[30];
    String8 TimeString[30];
    const int MaxRecord = 30;
    int index;
};

#define MAX_RING_BUF_SIZE (512 * 1024)
#define RING_BUF_SIZE_OFFSET 8
struct RingBuf {
    char *pBufBase;
    char *pRead;
    char *pWrite;
    char *pBufEnd;
    int   bufLen;

    RingBuf() : pBufBase(NULL), pRead(NULL), pWrite(NULL), pBufEnd(NULL), bufLen(0) {}
};


struct WriteSmoother;

extern const char *audio_dump_path;
extern const char *streamout_ori;
extern const char *streamout_ori_propty;
extern const char *streamout_dcr;
extern const char *streamout_dcr_propty;

extern const char *streamout_s2m;
extern const char *streamout_s2m_propty;
extern const char *streamout_acf;
extern const char *streamout_acf_propty;
extern const char *streamout_hcf;
extern const char *streamout_hcf_propty;

extern const char *streamout;
extern const char *streamoutfinal;
extern const char *streamout_propty;
extern const char *aud_dumpftrace_dbg_propty;
extern const char *streaminIVCPMIn;
extern const char *streaminIVIn;
extern const char *streamout_vibsignal;
extern const char *streamout_notch;
extern const char *streamoutdsp_propty;
extern const char *a2dpdsp_propty;

extern const char *streaminmanager;
extern const char *streamin;;
extern const char *streaminOri;
extern const char *streaminI2S;
extern const char *streaminDAIBT;
extern const char *streaminSpk;
extern const char *streaminSpk_propty;
extern const char *capture_data_provider;

extern const char *streamin_propty;
extern const char *streamin_epl_propty;
extern const char *streamindsp_propty;

extern const char *allow_low_latency_propty;
extern const char *streamhfp_propty;
extern const char *allow_offload_propty;
extern const char *streaminlog_propty;

extern bool bDumpStreamOutFlg;
extern bool bDumpStreamInFlg;

const char *transferAudioFormatToDumpString(const audio_format_t format);
int AudiocheckAndCreateDirectory(const char *pC);
FILE *AudioOpendumpPCMFile(const char *filepath, const char *propty);
void AudioCloseDumpPCMFile(FILE  *file);
void AudioDumpPCMData(void *buffer, uint32_t bytes, FILE  *file);
timespec GetSystemTime(bool print = 0);
unsigned long long TimeDifference(struct timespec time1, struct timespec time2);
bool generateVmDumpByEpl(const char *eplPath, const char *vmPath);

//-------ring buffer operation
int RingBuf_getDataCount(const RingBuf *RingBuf1);
int RingBuf_getFreeSpace(const RingBuf *RingBuf1);
void RingBuf_copyToLinear(char *buf, RingBuf *RingBuf1, int count);
void RingBuf_copyFromLinear(RingBuf *RingBuf1, const char *buf, int count);
void RingBuf_fillZero(RingBuf *RingBuf1, int count);
void RingBuf_copyEmpty(RingBuf *RingBuft, RingBuf *RingBufs);
int RingBuf_copyFromRingBuf(RingBuf *RingBuft, RingBuf *RingBufs, int count);
void RingBuf_writeDataValue(RingBuf *RingBuf1, const int value, const int count);
int RingBuf_discardData(RingBuf *ringBuf, int count);
int RingBuf_checkDataCrossBoundary(const RingBuf *ringBuf, int count);

void RingBuf_copyFromLinearSRC(void *pSrcHdl, RingBuf *RingBuft, char *buf, int num, int srt, int srs);
void RingBuf_copyEmptySRC(void *pSrcHdl, RingBuf *RingBuft, const RingBuf *RingBufs, int srt, int srs);

void RingBuf_dynamicChangeBufSize(struct RingBuf *ringBuf, uint32_t count);


/**
 * write smoother
 */
struct WriteSmoother *createWriteSmoother(void);

void doWriteSmoother(
    struct WriteSmoother *smoother,
    const uint64_t bufferTimeUs,
    const uint64_t safeFrameCount); // at least keep # frames in buf

void updateWriteSmootherTime( // for suspend, update time only
    struct WriteSmoother *smoother,
    const uint64_t bufferTimeUs);

void destroyWriteSmoother(struct WriteSmoother **smoother);



short clamp16(int sample);

BCV_PCM_FORMAT get_bcv_pcm_format(audio_format_t source, audio_format_t target);

size_t getSizePerFrame(audio_format_t fmt, unsigned int numChannel);
uint32_t getPeriodBufSize(const stream_attribute_t *attribute, uint32_t period_time_ms);
uint64_t getBufferLatencyMs(const stream_attribute_t *attribute, uint64_t bytes);
uint64_t getBufferLatencyUs(const stream_attribute_t *attribute, uint64_t bytes);


void CVSDLoopbackGetWriteBuffer(uint8_t **buffer, uint32_t *buf_len);
void CVSDLoopbackGetReadBuffer(uint8_t **buffer, uint32_t *buf_len);
void CVSDLoopbackReadDataDone(uint32_t len);
void CVSDLoopbackWriteDataDone(uint32_t len);
void CVSDLoopbackResetBuffer(void);
int32_t CVSDLoopbackGetFreeSpace(void);
int32_t CVSDLoopbackGetDataCount(void);

bool IsAudioSupportFeature(int dFeatureOption);

bool isBtSpkDevice(audio_devices_t devices);

uint32_t GetMicDeviceMode(uint32_t mic_category);
unsigned int FormatTransfer(int SourceFormat, int TargetFormat, void *Buffer, unsigned int mReadBufferSize);
int InFactoryMode();
int In64bitsProcess();

void calculateTimeStampByFrames(struct timespec startTime, uint32_t mTotalCaptureFrameSize, stream_attribute_t streamAttribute, struct timespec *newTimeStamp);
void calculateTimeStampByBytes(struct timespec startTime, uint32_t totalBufferSize, stream_attribute_t streamAttribute, struct timespec *newTimeStamp);
void adjustTimeStamp(struct timespec *startTime, int delayMs);
uint32_t convertMsToBytes(const uint32_t ms, const stream_attribute_t *streamAttribute);

void collectPlatformOutputFlags(audio_output_flags_t flag);
bool platformIsolatedDeepBuffer();
bool isIsolatedDeepBuffer(const audio_output_flags_t flag);

unsigned int wordSizeAlign(unsigned int inSize);

/**
 * newMtkAudioBitConverter(uint32_t sampling_rate, uint32_t channel_num, BCV_PCM_FORMAT format)
 *    Using dlopen/dlsym to new MtkAudioBitConverterBase object
 */
MtkAudioBitConverterBase *newMtkAudioBitConverter(uint32_t sampling_rate, uint32_t channel_num, BCV_PCM_FORMAT format);

/**
 * newMtkAudioSrc(uint32_t input_SR, uint32_t input_channel_num, uint32_t output_SR, uint32_t output_channel_num, SRC_PCM_FORMAT format)
 *    Using dlopen/dlsym to new MtkAudioSrcBase object
 */
MtkAudioSrcBase *newMtkAudioSrc(uint32_t input_SR, uint32_t input_channel_num, uint32_t output_SR, uint32_t output_channel_num, SRC_PCM_FORMAT format);

/**
 * newMtkAudioLoud(uint32_t eFLTtype, bool bFastTrack = false)
 *    Using dlopen/dlsym to new MtkAudioLoudBase object
 */
MtkAudioLoudBase *newMtkAudioLoud(uint32_t eFLTtype, bool bFastTrack = false);

/**
 * newMtkDcRemove(void)
 *    Using dlopen/dlsym to new MtkAudioDcRemoveBase object
 */
MtkAudioDcRemoveBase *newMtkDcRemove(void);

/**
 * deleteMtkAudioBitConverter(MtkAudioBitConverterBase* pObject)
 *    Using dlopen/dlsym to delete MtkAudioBitConverterBase object
 */
void deleteMtkAudioBitConverter(MtkAudioBitConverterBase *pObject);

/**
 * deleteMtkAudioSrc(MtkAudioSrcBase* pObject)
 *    Using dlopen/dlsym to delete MtkAudioSrcBase object
 */
void deleteMtkAudioSrc(MtkAudioSrcBase *pObject);

/**
 * deleteMtkAudioLoud(MtkAudioLoudBase *pObject)
 *    Using dlopen/dlsym to delete MtkAudioLoudBase object
 */
void deleteMtkAudioLoud(MtkAudioLoudBase *pObject);

/**
 * deleteMtkDcRemove(MtkAudioDcRemoveBase *pObject)
 *    Using dlopen/dlsym to delete MtkAudioDcRemoveBase object
 */
void deleteMtkDcRemove(MtkAudioDcRemoveBase *pObject);

/**
 * setAudioCompFltCustParam(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam)
 *    Using dlopen/dlsym to set Audio Parameters
 */
int setAudioCompFltCustParam(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam);

/**
 * getAudioCompFltCustParam(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam)
 *    Using dlopen/dlsym to get Audio Parameters
 */
int getAudioCompFltCustParam(AudioCompFltType_t eFLTtype, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioParam, const char *custScene = NULL);

/**
 *  SpeechMemCpy(void *dest, void *src, size_t n)
 *    use to replace memcpy on CCCI raw memory
 */
void SpeechMemCpy(void *dest, void *src, size_t n);


/**
 * safe version of strncpy
 */
char *audio_strncpy(char *target, const char *source, size_t target_size);


/**
 * safe version of strncat
 */
char *audio_strncat(char *target, const char *source, size_t target_size);


/**
 * PowerHAL hint control
 */
enum PowerHalHint {
    POWERHAL_LATENCY_DL        = 0,
    POWERHAL_LATENCY_UL        = 1,
    POWERHAL_POWER_DL          = 2,
    POWERHAL_DISABLE_WIFI_POWER_SAVE = 3
};

void initPowerHal();
bool getPowerHal();
void power_hal_hint(PowerHalHint hint, bool enable);

void setNeedAEETimeoutFlg(bool state);
bool getNeedAEETimeoutFlg();

/**
 * audio sched_setscheduler
 */

int audio_sched_setschedule(pid_t pid, int policy, int sched_priority);

/**
 * Get MicInfo
 */
bool getMicInfo(audio_microphone_characteristic_t *micArray, size_t *micCount);
void dumpMicInfo(struct audio_microphone_characteristic_t *micArray, size_t micCount);
bool findEnumByString(const struct enum_to_str_table *table, const char *str, uint32_t *enumVal);


/**
 * adaptive sleep
 */
int adaptiveSleepUs(uint32_t sleepUs, uint32_t unitSleepUs, uint32_t errorThdUs);


}

#endif

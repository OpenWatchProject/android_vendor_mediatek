#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechPcmMixerBaseBuffer"
#include <SpeechPcmMixerBase.h>
#include <sys/time.h>
#include <utils/threads.h>
#include <audio_utils/primitives.h>
#include <SpeechUtility.h>
#include <AudioUtility.h>


#ifndef pcm_mixer_msleep
#define pcm_mixer_msleep(ms) usleep((ms)*1000)
#endif
#define PCM_MIXER_RETRY_TIMES 5
//Maximum Latency between two modem data request: 200ms
//AP sould fill data to buffer in 60ms while receiving request

namespace android {
/*==============================================================================
 *                     Property keys
 *============================================================================*/
// property name exceed original max length PROPERTY_KEY_MAX in system/core/include/cutils/Properties.h
// use pointer instead
const char *PROPERTY_KEY_PCM_MIXER_DUMP_ON = "persist.vendor.audiohal.pcm_mixer_dump_on";
const char *PROPERTY_KEY_PCM_MIXER_BLISRC_DUMP_ON = "persist.vendor.audiohal.pcm_mixer_blisrc_dump_on";
const char *PROPERTY_KEY_PCM_MIXER_LOG_LEVEL = "persist.vendor.audiohal.speech.pcm_mixer.log";
const char *PROPERTY_KEY_PCM_MIXER_INDEX = "persist.vendor.audiohal.speech.pcm_mixer.index";
const char *PROPERTY_KEY_PCM_MIXER_BLISRC_INDEX = "persist.vendor.audiohal.speech.pcm_mixer_blisrc.index";


/*==============================================================================
 *                     Constant
 *============================================================================*/
#define PCM_MIXER_CHANNEL_NUM         (1)

// PROPERTY_KEY_PCM_MIXER_DUMP_ON
//   SpeechPcmMixerBase::PutDataToSpeaker()
static const char kFileNamePcmMixer[] = "/data/vendor/audiohal/audio_dump/PcmMixer_type_";

// PROPERTY_KEY_PCM_MIXER_BLISRC_DUMP_ON
//   SpeechPcmMixerBaseBuffer::Write()
//   before SRC, same as AudioALSAPlaybackHandlerVoice dump
static const char kFileNamePcmMixerBlisrc[] = "/data/vendor/audiohal/audio_dump/PcmMixer_before_Blisrc_type_";

static const uint32_t kMaxSizeOfFileName = 128;
static const uint32_t kSizeOfPrefixFileNamePcmMixer = sizeof(kFileNamePcmMixer);
static const uint32_t kSizeOfPrefixFileNamePcmMixerBlisrc = sizeof(kFileNamePcmMixerBlisrc) - 1;

#ifdef PCM_MIXER_USE_SINE_WAVE
static const uint16_t table_1k_tone_16000_hz[] = {
    0x0000, 0x30FC, 0x5A82, 0x7641,
    0x7FFF, 0x7641, 0x5A82, 0x30FB,
    0x0001, 0xCF05, 0xA57E, 0x89C0,
    0x8001, 0x89BF, 0xA57E, 0xCF05
};
static const uint32_t kSizeSinewaveTable = sizeof(table_1k_tone_16000_hz);
#endif

static uint32_t gPcmMixerLogLevel;


/*==============================================================================
 *                     Implementation
 *============================================================================*/

bool getPcmMixerLogEnableByLevel(const uint32_t pcm_mixer_log_level) {
    return ((gPcmMixerLogLevel & pcm_mixer_log_level) != 0);
}


SpeechPcmMixerBaseBuffer::SpeechPcmMixerBaseBuffer() :
    mExitRequest(false) {

    mFormat = AUDIO_FORMAT_DEFAULT;
    mRingBuf.pBufBase = NULL;
    mRingBuf.bufLen   = 0;
    mRingBuf.pRead = NULL;
    mRingBuf.pWrite   = NULL;
    mRingBuf.pBufEnd = NULL;
    mBliSrc = NULL;
    mIsPcmMixerBlisrcDumpEnable = false;
    mBliOutputLinearBuffer = NULL;
    pDumpFile = NULL;
}

bool SpeechPcmMixerBaseBuffer::IsPcmMixerBlisrcDumpEnable() {
    // PcmMixer Dump before Blisrc system property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_PCM_MIXER_BLISRC_DUMP_ON, property_value, "0"); //"0": default off
    if (property_value[0] == '1') {
        return true;
    } else {
        return false;
    }
}

status_t SpeechPcmMixerBaseBuffer::InitPcmMixerBuffer(SpeechPcmMixerBase *playPointer,
                                                      uint32_t sampleRate,
                                                      uint32_t chNum,
                                                      int32_t format,
                                                      uint32_t pcmMixerType) {
    ALOGV("InitPcmMixerBuffer sampleRate=%d ,ch=%d, format=%d", sampleRate, chNum, format);
    (void)playPointer;
    // keep the format
    ASSERT(format == AUDIO_FORMAT_PCM_16_BIT);
    mFormat = format;

    // set internal ring buffer
    mRingBuf.pBufBase = new char[PCM_MIXER_PLAY_BUFFER_LEN];
    mRingBuf.bufLen   = PCM_MIXER_PLAY_BUFFER_LEN;
    mRingBuf.pRead    = mRingBuf.pBufBase;
    mRingBuf.pWrite   = mRingBuf.pBufBase + (PCM_MIXER_EXTRA_NUM_FRAME * PCM_MIXER_PERIOD_SIZE);
    memset((void *)mRingBuf.pBufBase, 0, mRingBuf.bufLen);


    ALOGV("%s(), pBufBase: %p, pRead: 0x%x, pWrite: 0x%x, bufLen:%u", __FUNCTION__,
          mRingBuf.pBufBase, (int)(mRingBuf.pRead - mRingBuf.pBufBase), (int)(mRingBuf.pWrite - mRingBuf.pBufBase), mRingBuf.bufLen);

    mIsPcmMixerBlisrcDumpEnable = IsPcmMixerBlisrcDumpEnable();
    if (mIsPcmMixerBlisrcDumpEnable) {
        char fileNamePcmMixerBlisrc[kMaxSizeOfFileName];
        char pcmMixerTypeString[10];
        memset((void *)fileNamePcmMixerBlisrc, 0, kMaxSizeOfFileName);
        memset((void *)pcmMixerTypeString, 0, 10);

        audio_strncpy(fileNamePcmMixerBlisrc, kFileNamePcmMixerBlisrc, kMaxSizeOfFileName);
        snprintf(pcmMixerTypeString, sizeof(pcmMixerTypeString), "%u", pcmMixerType);
        audio_strncat(fileNamePcmMixerBlisrc, pcmMixerTypeString, sizeof(fileNamePcmMixerBlisrc));

        time_t rawtime;
        time(&rawtime);
        struct tm *timeinfo = localtime(&rawtime);
        if (timeinfo != NULL) {
            strftime(fileNamePcmMixerBlisrc + kSizeOfPrefixFileNamePcmMixerBlisrc, kMaxSizeOfFileName - kSizeOfPrefixFileNamePcmMixerBlisrc - 1, "_%Y_%m_%d_%H%M%S.pcm", timeinfo);
        } else {
            ALOGW("%s() timeinfo == NULL, use index for dump filename!", __FUNCTION__);
            char property_value[PROPERTY_VALUE_MAX];
            property_get(PROPERTY_KEY_PCM_MIXER_BLISRC_INDEX, property_value, "0");

            uint8_t pcm_mixer_file_number = atoi(property_value);
            char fileIndex[kMaxSizeOfFileName];
            memset((void *)fileIndex, 0, kMaxSizeOfFileName);
            sprintf(fileIndex, "_%u.pcm", pcm_mixer_file_number++);
            audio_strncat(fileNamePcmMixerBlisrc, fileIndex, sizeof(fileNamePcmMixerBlisrc));

            sprintf(property_value, "%u", pcm_mixer_file_number & 0x7);
            property_set(PROPERTY_KEY_PCM_MIXER_BLISRC_INDEX, property_value);
        }
        if (pDumpFile == NULL) {
            AudiocheckAndCreateDirectory(fileNamePcmMixerBlisrc);
            pDumpFile = fopen(fileNamePcmMixerBlisrc, "wb");
        }
        if (pDumpFile == NULL) {
            ALOGW("%s(), Fail to open %s", __FUNCTION__, fileNamePcmMixerBlisrc);
        } else {
            ALOGD("%s(), open %s", __FUNCTION__, fileNamePcmMixerBlisrc);
        }
    }
    // set blisrc
    mBliSrc = newMtkAudioSrc(sampleRate, chNum, PCM_MIXER_TARGET_SAMPLE_RATE, PCM_MIXER_CHANNEL_NUM, SRC_IN_Q1P15_OUT_Q1P15);
    mBliSrc->open();

    ASSERT(mBliSrc != NULL);

    // set blisrc converted buffer
    mBliOutputLinearBuffer = new char[PCM_MIXER_PLAY_BUFFER_LEN];
    ALOGV("%s(), mBliOutputLinearBuffer = %p, size = %u", __FUNCTION__, mBliOutputLinearBuffer, PCM_MIXER_PLAY_BUFFER_LEN);

    return NO_ERROR;
}

SpeechPcmMixerBaseBuffer::~SpeechPcmMixerBaseBuffer() {
    mExitRequest = true;

    AL_LOCK(mPcmMixerBufferRuningMutex);
    AL_LOCK(mPcmMixerBufferMutex);

    // delete blisrc handler buffer
    if (mBliSrc) {
        mBliSrc->close();
        deleteMtkAudioSrc(mBliSrc);
        mBliSrc = NULL;
    }

    // delete blisrc converted buffer
    delete[] mBliOutputLinearBuffer;

    // delete internal ring buffer
    delete[] mRingBuf.pBufBase;

    if (pDumpFile != NULL) {
        fclose(pDumpFile);
        pDumpFile = NULL;
    }

    AL_SIGNAL(mPcmMixerBufferMutex);
    AL_UNLOCK(mPcmMixerBufferMutex);
    AL_UNLOCK(mPcmMixerBufferRuningMutex);
}

uint32_t SpeechPcmMixerBaseBuffer::Write(char *buf, uint32_t num) {
    // lock
    AL_LOCK(mPcmMixerBufferRuningMutex);
    AL_LOCK(mPcmMixerBufferMutex);

    ALOGV("%s(), num = %u", __FUNCTION__, num);

    if (mIsPcmMixerBlisrcDumpEnable) {
        if (pDumpFile != NULL) {
            fwrite(buf, sizeof(char), num, pDumpFile);
        }
    }
    uint32_t leftCount = num;
    uint16_t dataCountInBuf = 0;
    uint32_t tryCount = 0;
    while (tryCount < PCM_MIXER_RETRY_TIMES && !mExitRequest) { // max mLatency = 200, max sleep (20 * 10) ms here
        // BLISRC: output buffer: buf => local buffer: mRingBuf
        if (leftCount > 0) {
            // get free space in ring buffer
            uint32_t outCount = RingBuf_getFreeSpace(&mRingBuf);

            // do conversion
            ASSERT(mBliSrc != NULL);
            uint32_t consumed = leftCount;
            mBliSrc->process((int16_t *)buf, &leftCount, (int16_t *)mBliOutputLinearBuffer, &outCount);
            consumed -= leftCount;

            buf += consumed;
            ALOGV("%s(), buf consumed = %u, leftCount = %u, outCount = %u",
                  __FUNCTION__, consumed, leftCount, outCount);

            // copy converted data to ring buffer //TODO(Harvey): try to reduce additional one memcpy here
            RingBuf_copyFromLinear(&mRingBuf, mBliOutputLinearBuffer, outCount);
            if (getPcmMixerLogEnableByLevel(PCM_MIXER_LOG_LEVEL_PCM_MIXER)) {
                ALOGD("%s(), pRead: 0x%x, pWrite: 0x%x, leftCount: %u, dataCount: %u",
                      __FUNCTION__,
                      (int)(mRingBuf.pRead - mRingBuf.pBufBase),
                      (int)(mRingBuf.pWrite - mRingBuf.pBufBase),
                      leftCount,
                      RingBuf_getDataCount(&mRingBuf));
            }
        }

        // leave while loop
        if (leftCount <= 0) {
            break;
        }

        // wait modem side to retrieve data
        int retval = AL_WAIT_MS(mPcmMixerBufferMutex, 40);
        if (!mExitRequest) {
            dataCountInBuf = RingBuf_getDataCount(&mRingBuf);
        }
        if (retval != 0) {
            ALOGV("%s(), tryCount = %u, leftCount = %u, dataCountInBuf = %u",
                  __FUNCTION__, tryCount, leftCount, dataCountInBuf);
            tryCount++;
        }

    }

    // leave warning message if need
    if (leftCount != 0) {
        dataCountInBuf = RingBuf_getDataCount(&mRingBuf);
        ALOGW("%s(), still leftCount = %u, dataCountInBuf = %u", __FUNCTION__, leftCount, dataCountInBuf);
    }

    // unlock
    AL_UNLOCK(mPcmMixerBufferMutex);
    AL_UNLOCK(mPcmMixerBufferRuningMutex);

    return num - leftCount;
}


bool SpeechPcmMixerBaseBuffer::isBufferEnough(void) {
    uint16_t dataCountInBuf = 0;

    dataCountInBuf = RingBuf_getDataCount(&mRingBuf);
    if (dataCountInBuf < (PCM_MIXER_PERIOD_SIZE * PCM_MIXER_EXTRA_NUM_FRAME)) {
        return false;
    }

    return true;
}



//*****************************************************************************************
//--------------------------for LAD Player------------------------------------------
//*****************************************************************************************
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechPcmMixerBase"

SpeechPcmMixerBase::SpeechPcmMixerBase() {
    // initial all table entry to zero, means non of them are occupied
    mCount = 0;
    mBufBaseTemp = new char[PCM_MIXER_PLAY_BUFFER_LEN];
    mSpeechDriver = NULL;
    mIsPcmMixerDumpEnable = false;
    pDumpFile = NULL;
    mPcmMixerPeriodSize = 0;
    mUnderflowCount = 0;
}

SpeechPcmMixerBase::~SpeechPcmMixerBase() {
    AL_AUTOLOCK(mPcmMixerBufferVectorLock);

    size_t count = mPcmMixerBufferVector.size();
    for (size_t i = 0 ; i < count ; i++) {
        SpeechPcmMixerBaseBuffer *pPcmMixerBuffer = mPcmMixerBufferVector.itemAt(i);
        delete pPcmMixerBuffer;
    }
    mPcmMixerBufferVector.clear();

    delete[] mBufBaseTemp;

    if (pDumpFile != NULL) {
        fclose(pDumpFile);
        pDumpFile = NULL;
    }
}

SpeechPcmMixerBaseBuffer *SpeechPcmMixerBase::CreatePcmMixerBuffer(uint32_t sampleRate, uint32_t chNum, int32_t format, uint32_t pcmMixerType) {
    ALOGV("CreatePcmMixerBuffer sampleRate=%u ,ch=%u, format=%d", sampleRate, chNum, format);

    // protection
    ASSERT(format == AUDIO_FORMAT_PCM_16_BIT);

    // create PcmMixerBuffer
    SpeechPcmMixerBaseBuffer *pPcmMixerBuffer = new SpeechPcmMixerBaseBuffer();
    pPcmMixerBuffer->InitPcmMixerBuffer(this, sampleRate, chNum, format, pcmMixerType);

    AL_LOCK(mPcmMixerBufferVectorLock);
    mPcmMixerBufferVector.add(pPcmMixerBuffer);
    AL_UNLOCK(mPcmMixerBufferVectorLock);

    return pPcmMixerBuffer;
}

bool SpeechPcmMixerBase::IsPcmMixerDumpEnable() {
    // PcmMixer Dump system property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_PCM_MIXER_DUMP_ON, property_value, "0"); //"0": default off
    if (property_value[0] == '1') {
        return true;
    } else {
        return false;
    }
}

uint32_t SpeechPcmMixerBase::Write(SpeechPcmMixerBaseBuffer *pPcmMixerBuffer, void *buf, uint32_t num) {
    ASSERT(pPcmMixerBuffer != NULL);
    return pPcmMixerBuffer->Write((char *)buf, num);
}

void SpeechPcmMixerBase::DestroyPcmMixerBuffer(SpeechPcmMixerBaseBuffer *pPcmMixerBuffer) {
    ASSERT(pPcmMixerBuffer != NULL);

    AL_LOCK(mPcmMixerBufferVectorLock);
    mPcmMixerBufferVector.remove(pPcmMixerBuffer);
    AL_UNLOCK(mPcmMixerBufferVectorLock);

    delete pPcmMixerBuffer;
}

bool SpeechPcmMixerBase::Open(SpeechDriverInterface *pSpeechDriver) {
    AL_AUTOLOCK(mCountLock);

    if (NULL != mSpeechDriver && mSpeechDriver != pSpeechDriver) {
        ALOGE("PcmMixer can't support differ SpeechDriver");
        return false;
    }

    mCount++;
    if (1 != mCount) {
        SLOG_ENG("%s(), already open. mCount %d", __FUNCTION__, mCount);
        return true;
    }

    mPcmMixerPeriodSize = PCM_MIXER_PERIOD_SIZE;

    mUnderflowCount = 0;

    SLOG_ENG("%s(), first open, mCount %d, mPcmMixerPeriodSize: %u",
             __FUNCTION__, mCount, mPcmMixerPeriodSize);

    // get Speech Driver (to open/close PcmMixer)
    mSpeechDriver = pSpeechDriver;
    mIsPcmMixerDumpEnable = IsPcmMixerDumpEnable();

    gPcmMixerLogLevel = get_uint32_from_property(PROPERTY_KEY_PCM_MIXER_LOG_LEVEL);

    if (mIsPcmMixerDumpEnable) {
        char fileNamePcmMixer[kMaxSizeOfFileName];
        char pcmMixerTypeString[10];
        memset((void *)fileNamePcmMixer, 0, kMaxSizeOfFileName);
        memset((void *)pcmMixerTypeString, 0, 10);


        audio_strncpy(fileNamePcmMixer, kFileNamePcmMixer, kMaxSizeOfFileName);

        snprintf(pcmMixerTypeString, sizeof(pcmMixerTypeString), "%u", getPcmMixerType());
        audio_strncat(fileNamePcmMixer, pcmMixerTypeString, sizeof(fileNamePcmMixer));

        time_t rawtime;
        time(&rawtime);
        struct tm *timeinfo = localtime(&rawtime);
        if (timeinfo != NULL) {
            strftime(fileNamePcmMixer + kSizeOfPrefixFileNamePcmMixer, kMaxSizeOfFileName - kSizeOfPrefixFileNamePcmMixer - 1, "_%Y_%m_%d_%H%M%S.pcm", timeinfo);
        } else {
            ALOGW("%s() timeinfo == NULL, use index for dump filename!", __FUNCTION__);
            char property_value[PROPERTY_VALUE_MAX];
            property_get(PROPERTY_KEY_PCM_MIXER_INDEX, property_value, "0");

            uint8_t pcm_mixer_file_number = atoi(property_value);
            char fileIndex[kMaxSizeOfFileName];
            memset((void *)fileIndex, 0, kMaxSizeOfFileName);
            sprintf(fileIndex, "_%u.pcm", pcm_mixer_file_number++);
            audio_strncat(fileNamePcmMixer, fileIndex, sizeof(fileNamePcmMixer));

            sprintf(property_value, "%u", pcm_mixer_file_number & 0x7);
            property_set(PROPERTY_KEY_PCM_MIXER_INDEX, property_value);
        }

        if (pDumpFile == NULL) {
            AudiocheckAndCreateDirectory(fileNamePcmMixer);
            pDumpFile = fopen(fileNamePcmMixer, "wb");
        }
        if (pDumpFile == NULL) {
            ALOGW("%s(), Fail to open %s", __FUNCTION__, fileNamePcmMixer);
        } else {
            ALOGD("%s(), open %s", __FUNCTION__, fileNamePcmMixer);
        }
    }
    //turn on pcm mixer
    pcmMixerOn(pSpeechDriver);

    return true;
}

uint32_t SpeechPcmMixerBase::PutData(SpeechPcmMixerBaseBuffer *pPcmMixerBuffer, char *target_ptr, uint16_t num_data_request) {
    uint16_t write_count = 0;

    if (pPcmMixerBuffer == NULL) {
        ALOGW("%s(), pPcmMixerBuffer == NULL, return 0.", __FUNCTION__);
        return 0;
    }

    AL_LOCK(pPcmMixerBuffer->mPcmMixerBufferMutex);

    // check data count in pPcmMixerBuffer
    uint16_t dataCountInBuf = RingBuf_getDataCount(&pPcmMixerBuffer->mRingBuf);
    if (dataCountInBuf == 0) { // no data in buffer, just return 0
        ALOGV("%s(), dataCountInBuf == 0, return 0.", __FUNCTION__);
        AL_UNLOCK(pPcmMixerBuffer->mPcmMixerBufferMutex);
        return 0;
    }

    write_count = (dataCountInBuf >= num_data_request) ? num_data_request : dataCountInBuf;

    // copy to share buffer
    RingBuf_copyToLinear(target_ptr, &pPcmMixerBuffer->mRingBuf, write_count);
    if (getPcmMixerLogEnableByLevel(PCM_MIXER_LOG_LEVEL_PCM_MIXER)) {
        ALOGD("%s(), pRead: 0x%x, pWrite: 0x%x, write_count:%u, remain dataCount:%u",
              __FUNCTION__,
              (int)(pPcmMixerBuffer->mRingBuf.pRead - pPcmMixerBuffer->mRingBuf.pBufBase),
              (int)(pPcmMixerBuffer->mRingBuf.pWrite - pPcmMixerBuffer->mRingBuf.pBufBase),
              write_count,
              RingBuf_getDataCount(&pPcmMixerBuffer->mRingBuf));
    }

    AL_SIGNAL(pPcmMixerBuffer->mPcmMixerBufferMutex);
    AL_UNLOCK(pPcmMixerBuffer->mPcmMixerBufferMutex);

    return write_count;
}

uint16_t SpeechPcmMixerBase::PutDataToSpeaker(char *target_ptr, uint16_t num_data_request) {
    uint16_t write_count = 0;
    uint16_t numFrames = 0;
    uint16_t needFrames = 0;


#ifndef PCM_MIXER_USE_SINE_WAVE
    AL_AUTOLOCK(mPcmMixerBufferVectorLock);

    size_t count = mPcmMixerBufferVector.size();

    if (count == 0) {
        ALOGW("%s(), mPcmMixerBufferVector == NULL, return 0.", __FUNCTION__);
        return 0;
    }

    uint16_t dataCountInBuf, dataCountInBufMin = 65535;
    for (size_t i = 0 ; i < count ; i++) {
        SpeechPcmMixerBaseBuffer *pPcmMixerBuffer = mPcmMixerBufferVector.itemAt(i);

        AL_LOCK(pPcmMixerBuffer->mPcmMixerBufferMutex);
        dataCountInBuf = RingBuf_getDataCount(&pPcmMixerBuffer->mRingBuf);
        AL_UNLOCK(pPcmMixerBuffer->mPcmMixerBufferMutex);

        if (dataCountInBuf < dataCountInBufMin) {
            dataCountInBufMin = dataCountInBuf;
        }
    }

    // check data count in pPcmMixerBuffer
    if (dataCountInBufMin < mPcmMixerPeriodSize) { // data not enough in buffer, just return 0
        ALOGE("%s(), dataCountInBufMin: %d!! num_data_request %d, underflow!!",
              __FUNCTION__, dataCountInBufMin, num_data_request);
        mUnderflowCount++;
        return 0;
    }
    if (num_data_request < mPcmMixerPeriodSize) { // modem still have enough data...
        ALOGW("%s(), dataCountInBufMin: %d, num_data_request %d, modem have enough data",
              __FUNCTION__, dataCountInBufMin, num_data_request);
        return 0;
    }


    write_count = (dataCountInBufMin >= num_data_request) ? num_data_request : dataCountInBufMin;

    // align frame size
    if (mUnderflowCount == 0) { // offer 1 frame to modem per time
        write_count = mPcmMixerPeriodSize;
    } else { // underflow before!! offer underflow cnt + 1 frame to modem
        numFrames = write_count / mPcmMixerPeriodSize;
        needFrames = mUnderflowCount + 1;
        if (numFrames >= needFrames) {
            write_count = needFrames * mPcmMixerPeriodSize;
            mUnderflowCount = 0;
        } else {
            write_count = numFrames * mPcmMixerPeriodSize;
            mUnderflowCount -= (numFrames - 1);
        }
    }

    memset(target_ptr, 0, num_data_request);
    for (size_t i = 0 ; i < count ; i++) {
        SpeechPcmMixerBaseBuffer *pPcmMixerBuffer = mPcmMixerBufferVector.itemAt(i);
        if (count == 1) {
            PutData(pPcmMixerBuffer, target_ptr, write_count);
            continue;
        }
        PutData(pPcmMixerBuffer, mBufBaseTemp, write_count);

        // mixer
        int16_t *in = (int16_t *)mBufBaseTemp;
        int16_t *out = (int16_t *)target_ptr;
        int16_t frameCnt = write_count / PCM_MIXER_CHANNEL_NUM / audio_bytes_per_sample(AUDIO_FORMAT_PCM_16_BIT);
        for (int16_t j = 0; j < frameCnt; j++) {
            out[j] = clamp16((int32_t)out[j] + (int32_t)in[j]);
        }
    }
#else
    static uint32_t i4Count = 0;
    uint32_t current_count = 0, remain_count = 0;
    char *tmp_ptr = NULL;

    remain_count = write_count = num_data_request;
    tmp_ptr = target_ptr;

    if (remain_count > (kSizeSinewaveTable - i4Count)) {
        memcpy(tmp_ptr, table_1k_tone_16000_hz + (i4Count >> 1), kSizeSinewaveTable - i4Count);
        tmp_ptr += (kSizeSinewaveTable - i4Count);
        remain_count -= (kSizeSinewaveTable - i4Count);
        i4Count = 0;
    }
    while (remain_count > kSizeSinewaveTable) {
        memcpy(tmp_ptr, table_1k_tone_16000_hz, kSizeSinewaveTable);
        tmp_ptr += kSizeSinewaveTable;
        remain_count -= kSizeSinewaveTable;
    }
    if (remain_count > 0) {
        memcpy(tmp_ptr, table_1k_tone_16000_hz, remain_count);
        i4Count = remain_count;
    }
#endif

    if (mIsPcmMixerDumpEnable) {
        if (pDumpFile != NULL) {
            fwrite(target_ptr, sizeof(char), write_count, pDumpFile);
        }
    }
    return write_count;
}


bool SpeechPcmMixerBase::Close() {
    AL_AUTOLOCK(mCountLock);

    mCount--;
    if (0 != mCount) {
        SLOG_ENG("%s, has other user, return. mCount %d", __FUNCTION__, mCount);
        return true;
    }

    SLOG_ENG("%s(), mCount %d, stop", __FUNCTION__, mCount);

    // tell modem side to close Pcm Mixer
    pcmMixerOff(mSpeechDriver);
    mSpeechDriver = NULL;

    if (pDumpFile != NULL) {
        fclose(pDumpFile);
        pDumpFile = NULL;
    }
    return true;
}

}; // namespace android



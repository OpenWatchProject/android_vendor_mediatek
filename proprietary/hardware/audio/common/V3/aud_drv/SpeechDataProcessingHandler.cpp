#include "SpeechDataProcessingHandler.h"

#include <sys/prctl.h>
#include "AudioALSACaptureDataProviderVoiceDL.h"
#include "AudioALSACaptureDataProviderVoiceUL.h"
#include "AudioALSACaptureDataProviderVoiceMix.h"
#include "AudioType.h"
#include "SpeechDriverFactory.h"
#include "SpeechMessengerInterface.h"
#include "SpeechType.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "SpeechDataProcessingHandler"

#define SINE_WAVE_ENABLED 0

namespace android {


/*==============================================================================
 *                     Constant
 *============================================================================*/

static const uint32_t kPcmRingBufferSize = 0x1000; // 4k, used for DL/UL

static const uint32_t kTargetSampleRate = 16000;

static const audio_format_t kTargetBitFormat = AUDIO_FORMAT_PCM_16_BIT;

static const uint32_t kTargetULChannelNumber = 1;

static const uint32_t kTargetDLChannelNumber = 1;


/*==============================================================================
 *                     Utils
 *============================================================================*/

// 166Hz for 8kHz
#define TBL_SZ_48KHz_1K 48

const uint16_t tone1k_48kHz[TBL_SZ_48KHz_1K] = {
    0xFFFA, 0x075B, 0x0E9C, 0x159C, 0x1C3D, 0x2263, 0x27F3, 0x2CD4,
    0x30F0, 0x3435, 0x3697, 0x3808, 0x3885, 0x380A, 0x3699, 0x3439,
    0x30F5, 0x2CDB, 0x27FB, 0x226C, 0x1C47, 0x15A7, 0x0EA7, 0x0766,
    0x0006, 0xF8A5, 0xF165, 0xEA63, 0xE3C2, 0xDD9C, 0xD80C, 0xD32C,
    0xCF11, 0xCBCB, 0xC969, 0xC7F8, 0xC77A, 0xC7F7, 0xC967, 0xCBC6,
    0xCF0B, 0xD326, 0xD804, 0xDD94, 0xE3BA, 0xEA5A, 0xF15A, 0xF89A
};

void dumpRingBuf(const char *str, RingBuf *buf) {
    int count = RingBuf_getDataCount(buf);
    ALOGD("%s(), Dump %s ,base: 0x%p, len: %d, read: 0x%p, write: 0x%p, data_count: %d\n", __FUNCTION__, str, buf->pBufBase, buf->bufLen, buf->pRead, buf->pWrite, count);
    ALOGD("  [0] 0x%c [1] 0x%c [2] 0x%c [3] 0x%c... [%d] 0x%c [%d] 0x%c\n", buf->pBufBase[0], buf->pBufBase[1], buf->pBufBase[2], buf->pBufBase[3], count - 2, buf->pBufBase[count - 2], count - 1, buf->pBufBase[count - 1]);
}

void dumpLinearBuf(const char *str, const char *buf, int from, int to) {
    ALOGD("%s(),Dump %s buffer(%p)\n", __FUNCTION__, str, buf);
    for (int i = from; i <= to; i++) {
        ALOGD("%s(), [%d] %c\n", __FUNCTION__, i, buf[i]);
    }
}

void setupLinearBufWithSineWave(char *buf, uint32_t size) {
    ALOGD("%s(), buf = %p, size = %d\n", __FUNCTION__, buf, size);
    char *p = buf;
    static uint32_t index_of_table = 0;
    uint32_t table_bytes = TBL_SZ_48KHz_1K * sizeof(uint16_t);

    if (index_of_table != 0) {
        memcpy(p, &((char *)tone1k_48kHz)[index_of_table], table_bytes - index_of_table);
        size -= table_bytes - index_of_table;
        p += table_bytes - index_of_table;
        index_of_table = 0;
    }

    while (size >= table_bytes) {
        memcpy(p, tone1k_48kHz, table_bytes);
        size -= table_bytes;
        p += table_bytes;
    }

    if (size > 0) {
        memcpy(p, tone1k_48kHz, size);
        index_of_table += size;
    }
}

/*==============================================================================
 *                     Implementation
 *============================================================================*/
SpeechDataProcessingHandler *SpeechDataProcessingHandler::mSpeechDataProcessingHandler = NULL;
AudioLock speechDataProcessingHandlerLock;

SpeechDataProcessingHandler *SpeechDataProcessingHandler::getInstance() {
    AL_AUTOLOCK(speechDataProcessingHandlerLock);

    if (mSpeechDataProcessingHandler == NULL) {
        mSpeechDataProcessingHandler = new SpeechDataProcessingHandler();
    }
    ASSERT(mSpeechDataProcessingHandler != NULL);
    return mSpeechDataProcessingHandler;
}

void SpeechDataProcessingHandler::destoryInstanceSafely() {
    ALOGD("+%s()\n", __FUNCTION__);

    AL_AUTOLOCK(speechDataProcessingHandlerLock);
    if (!AudioALSACaptureDataProviderVoiceDL::hasInstance()
        && !AudioALSACaptureDataProviderVoiceUL::hasInstance()
        && !AudioALSACaptureDataProviderVoiceMix::hasInstance()) {
        delete mSpeechDataProcessingHandler;
        mSpeechDataProcessingHandler = NULL;
    }
    ALOGD("-%s()\n", __FUNCTION__);
}

SpeechDataProcessingHandler::SpeechDataProcessingHandler() {
    ALOGD("+%s()", __FUNCTION__);

    // Init thread resources
    mStopThreadFlag = false;
    mBliSrcUL = NULL;
    mBliSrcDL = NULL;
    mSrcSampleRateUL = 0;
    mSrcSampleRateDL = 0;
    mSpeechRecordOn = false;

    int ret;

    ret = pthread_cond_init(&mSpeechDataNotifyEvent, NULL);
    if (ret != 0) {
        ALOGE("mSpeechDataNotifyEvent create fail!!!");
    }

    ret = pthread_mutex_init(&mSpeechDataNotifyMutex, NULL);
    if (ret != 0) {
        ALOGE("nSpeechDataNotifyMutex create fail!!!");
    }

    ret = pthread_create(&mSpeechDataProcessingThread, NULL, SpeechDataProcessingHandler::threadLoop, (void *)this);
    if (ret != 0) {
        ALOGE("mSpeechDataProcessingThread create fail!!!");
    } else {
        ALOGD("mSpeechDataProcessingThread = %lu created", mSpeechDataProcessingThread);
    }

    ALOGD("-%s()", __FUNCTION__);
}

SpeechDataProcessingHandler::~SpeechDataProcessingHandler() {
    ALOGD("+%s()", __FUNCTION__);

    if (mBliSrcDL != NULL) {
        mBliSrcDL->close();
        deleteMtkAudioSrc(mBliSrcDL);
        mBliSrcDL = NULL;
        mSrcSampleRateDL = 0;
    }

    if (mBliSrcUL != NULL) {
        mBliSrcUL->close();
        deleteMtkAudioSrc(mBliSrcUL);
        mBliSrcUL = NULL;
        mSrcSampleRateUL = 0;
    }

    mStopThreadFlag = true;
    pthread_cond_signal(&mSpeechDataNotifyEvent);
    pthread_join(mSpeechDataProcessingThread, NULL);
    mSpeechDataProcessingThread = (pthread_t)NULL;

    pthread_cond_destroy(&mSpeechDataNotifyEvent);
    pthread_mutex_destroy(&mSpeechDataNotifyMutex);
    ALOGD("-%s()", __FUNCTION__);
}

void SpeechDataProcessingHandler::getStreamAttributeSource(Caller caller, stream_attribute_t *streamAttributeSource) {
    if (streamAttributeSource != NULL) {
        SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
        streamAttributeSource->audio_format = kTargetBitFormat;
        streamAttributeSource->sample_rate = kTargetSampleRate;

        switch (caller) {
        case VOICE_UL_CALLER:
            streamAttributeSource->audio_channel_mask = AUDIO_CHANNEL_IN_VOICE_UPLINK;
            streamAttributeSource->num_channels = 1;
            break;
        case VOICE_DL_CALLER:
            streamAttributeSource->audio_channel_mask = AUDIO_CHANNEL_IN_VOICE_DNLINK;
            streamAttributeSource->num_channels = 1;
            break;
        case VOICE_MIX_CALLER:
            streamAttributeSource->audio_channel_mask = AUDIO_CHANNEL_IN_VOICE_UPLINK | AUDIO_CHANNEL_IN_VOICE_DNLINK;
            streamAttributeSource->num_channels = 2;
            break;
        }

        // Reset frames readed counter
        streamAttributeSource->Time_Info.total_frames_readed = 0;
    }
}
static int mUserCounter = 0;
status_t SpeechDataProcessingHandler::recordOn(RecordType type __unused) {
    ALOGD("+%s()\n", __FUNCTION__);
    AL_AUTOLOCK(speechDataProcessingHandlerLock);
#if 0
    if (mSpeechRecordOn == false) {
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->recordOn(type);
        mSpeechRecordOn = true;
        ALOGD("%s(), recordOn(%d)\n", __FUNCTION__, type);
    } else {
        bool hasMix = AudioALSACaptureDataProviderVoiceMix::hasInstance();
        bool hasDL = AudioALSACaptureDataProviderVoiceDL::hasInstance();
        bool hasUL = AudioALSACaptureDataProviderVoiceUL::hasInstance();

        RecordType recordType = type;
        if (hasMix || (hasDL && hasUL)) {
            recordType = RECORD_TYPE_MIX;
        }
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->setPcmRecordType(recordType);
        ALOGD("%s(), recordOn(%d) -> setPcmRecordType(%d)\n", __FUNCTION__, type, recordType);
    }
#else
    mUserCounter++;
    SpcRecordTypeStruct typeRecord;
    typeRecord.direction = RECORD_TYPE_MIX;
#ifdef MTK_PHONE_CALL_RECORD_VOICE_ONLY
    typeRecord.dlPosition = RECORD_POS_DL_HEAD;
#else
    typeRecord.dlPosition = RECORD_POS_DL_END;
#endif
    if (mUserCounter == 1) {
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->recordOn(typeRecord);
        ALOGD("%s(), First user, record on. dlPosition: %d\n", __FUNCTION__, typeRecord.dlPosition);
    } else {
        ALOGD("%s(), Record already on. user = %d\n", __FUNCTION__, mUserCounter);
    }
#endif
    ALOGD("-%s()\n", __FUNCTION__);
    return NO_ERROR;
}

status_t SpeechDataProcessingHandler::recordOff(RecordType type __unused) {
    ALOGD("+%s()\n", __FUNCTION__);
    AL_AUTOLOCK(speechDataProcessingHandlerLock);
#if 0
    bool hasMix = AudioALSACaptureDataProviderVoiceMix::hasInstance();
    bool hasDL = AudioALSACaptureDataProviderVoiceDL::hasInstance();
    bool hasUL = AudioALSACaptureDataProviderVoiceUL::hasInstance();
    bool onlyOneProvider = (hasMix && !hasDL && !hasUL) || (!hasMix && hasDL && !hasUL) || (!hasMix && !hasDL && hasUL);

    if (onlyOneProvider) {
        ALOGD("%s(), Last record type, recordOff(%d)\n", __FUNCTION__, type);
        mSpeechRecordOn = false;
        return SpeechDriverFactory::GetInstance()->GetSpeechDriver()->recordOff(type);
    } else {
        switch (type) {
        case RECORD_TYPE_DL:
            if (!hasMix && hasDL && hasUL) {
                ALOGD("%s(), recordOff(%d) -> setPcmRecordType(RECORD_TYPE_UL)\n", __FUNCTION__, type);
                return SpeechDriverFactory::GetInstance()->GetSpeechDriver()->setPcmRecordType(RECORD_TYPE_UL);
            }
            break;
        case RECORD_TYPE_UL:
            if (!hasMix && hasDL && hasUL) {
                ALOGD("%s(), recordOff(%d) -> setPcmRecordType(RECORD_TYPE_DL)\n", __FUNCTION__, type);
                return SpeechDriverFactory::GetInstance()->GetSpeechDriver()->setPcmRecordType(RECORD_TYPE_DL);
            }
            break;
        case RECORD_TYPE_MIX:
            if (hasDL) {
                ALOGD("%s(), recordOff(%d) -> setPcmRecordType(RECORD_TYPE_DL)\n", __FUNCTION__, type);
                return SpeechDriverFactory::GetInstance()->GetSpeechDriver()->setPcmRecordType(RECORD_TYPE_DL);
            } else if (hasUL) {
                ALOGD("%s(), recordOff(%d) -> setPcmRecordType(RECORD_TYPE_UL)\n", __FUNCTION__, type);
                return SpeechDriverFactory::GetInstance()->GetSpeechDriver()->setPcmRecordType(RECORD_TYPE_UL);
            }
            break;
        }
    }
    ALOGD("-%s(), recordOff(%d), do nothing. (UL:%d DL:%d MIX:%d)\n", __FUNCTION__, type, hasUL, hasDL, hasMix);
#else
    mUserCounter--;
    SpcRecordTypeStruct typeRecord;
    typeRecord.direction = RECORD_TYPE_MIX;
#ifdef MTK_PHONE_CALL_RECORD_VOICE_ONLY
    typeRecord.dlPosition = RECORD_POS_DL_HEAD;
#else
    typeRecord.dlPosition = RECORD_POS_DL_END;
#endif
    if (mUserCounter == 0) {
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->recordOff(typeRecord);
        ALOGD("%s(), No user, record off!\n", __FUNCTION__);
    } else {
        ALOGD("%s(), Record is still using. user = %d\n", __FUNCTION__, mUserCounter);
    }
    ALOGD("-%s()\n", __FUNCTION__);
#endif
    return NO_ERROR;
}

status_t SpeechDataProcessingHandler::provideModemRecordDataToProvider(RingBuf pcm_read_buf) {
    ALOGV("%s(), Got speech packet, addr = %p, mSpeechDataList = %p, mSpeechDataNotifyEvent = %p, mSpeechDataNotifyMutex = %p, mStopThreadFlag = %d, this(Handler) = %p\n", __FUNCTION__, pcm_read_buf.pRead, &mSpeechDataList, &mSpeechDataNotifyEvent, &mSpeechDataNotifyMutex, mStopThreadFlag, (void *)this);

    if (mStopThreadFlag == true) {
        ALOGW("%s(), SpeechDataprocessingHandler is stoping, ignore packet!\n", __FUNCTION__);
    }

    int speechDataSize = RingBuf_getDataCount(&pcm_read_buf);
    if (speechDataSize < (int)sizeof(android::spcApRAWPCMBufHdr)) {
        ALOGW("%s(), no valid struct in pcm buf\n", __FUNCTION__);
        return NO_ERROR;
    }

    // Dup ring buf and push it to list
    RingBuf *ringBuf = new RingBuf;
    ringBuf->pBufBase = new char[speechDataSize];
    ringBuf->bufLen = speechDataSize;
    ringBuf->pRead = ringBuf->pBufBase;
    ringBuf->pWrite = ringBuf->pBufBase + speechDataSize;
    RingBuf_copyToLinear(ringBuf->pBufBase, &pcm_read_buf, speechDataSize);

    // Check sync word
    spcApRAWPCMBufHdrStruct *speechPacketHeader = (spcApRAWPCMBufHdrStruct *)ringBuf->pBufBase;
    uint16_t syncWord = speechPacketHeader->u16SyncWord;
    uint16_t pcmLength = speechPacketHeader->u16Length;
    char *pcmData = ringBuf->pBufBase + sizeof(android::spcApRAWPCMBufHdrStruct);

    // fill pcm data with sine wave
    if (syncWord != EEMCS_M2A_SHARE_BUFF_HEADER_SYNC) {
        ALOGE("%s(), Invalid packet found!! (SyncWord: 0x%x, addr = %p)\n", __FUNCTION__, syncWord, &speechPacketHeader->u16SyncWord);
        delete[] ringBuf->pBufBase;
        delete ringBuf;
        return NO_ERROR;
    }

    pthread_mutex_lock(&mSpeechDataNotifyMutex);
    mSpeechDataList.push_back(ringBuf);
    pthread_cond_signal(&mSpeechDataNotifyEvent);
    pthread_mutex_unlock(&mSpeechDataNotifyMutex);

    return NO_ERROR;
}

void *SpeechDataProcessingHandler::threadLoop(void *arg) {
    ALOGD("%s()\n", __FUNCTION__);
    char *pInputPacketBuf = NULL;
    int speechDataSize = 0;
    RingBuf *ringBuf = NULL;
    SpeechDataProcessingHandler *pHandler = (SpeechDataProcessingHandler *)arg;

    while (!pHandler->mStopThreadFlag) {
        pInputPacketBuf = NULL;
        speechDataSize = 0;
        ringBuf = NULL;

        // Pop speech data from list
        pthread_mutex_lock(&pHandler->mSpeechDataNotifyMutex);
        if (pHandler->mSpeechDataList.begin() != pHandler->mSpeechDataList.end()) {
            ringBuf = *(pHandler->mSpeechDataList.begin());
            pHandler->mSpeechDataList.erase(pHandler->mSpeechDataList.begin());
        } else {
            ALOGV("%s(), wait for new speech data, pHandler = %p, pHandler->mStopThreadFlag = %d\n", __FUNCTION__, pHandler, pHandler->mStopThreadFlag);
            pthread_cond_wait(&pHandler->mSpeechDataNotifyEvent, &pHandler->mSpeechDataNotifyMutex);
        }
        pthread_mutex_unlock(&pHandler->mSpeechDataNotifyMutex);

        // Process speech data
        if (ringBuf != NULL) {
            pInputPacketBuf = ringBuf->pBufBase;
            speechDataSize = ringBuf->bufLen;
            delete ringBuf;

            if (pInputPacketBuf != NULL) {
                pHandler->processSpeechPacket(pInputPacketBuf, speechDataSize);
                delete[] pInputPacketBuf;
            } else {
                ALOGW("%s(), pInputPacketBuf is NULL\n", __FUNCTION__);
            }
        } else {
            ALOGV("%s(), ringBuf is NULL\n", __FUNCTION__);
        }
    }

    ALOGD("threadLoop exit mSpeechDataProcessingThread = %lu\n", pHandler->mSpeechDataProcessingThread);
    pthread_exit(NULL);
    return 0;
}

status_t SpeechDataProcessingHandler::processSpeechPacket(char *pInputPacketBuf, uint32_t speechDataSize) {
    ALOGV("+%s(), pInputPacketBuf = %p, speechDataSize = %d\n", __FUNCTION__, pInputPacketBuf, speechDataSize);

    char *pULPcmInputBuf = NULL;
    uint32_t uULPcmInputBufSize = 0;
    uint16_t uULFreq = 0;
    char *pDLPcmInputBuf = NULL;
    uint32_t uDLPcmInputBufSize = 0;
    uint16_t uDLFreq = 0;
    char *ptr = pInputPacketBuf;

    while (ptr < pInputPacketBuf + speechDataSize) {
        spcApRAWPCMBufHdrStruct *speechPacketHeader = (spcApRAWPCMBufHdrStruct *)ptr;
        uint16_t syncWord = speechPacketHeader->u16SyncWord;
        uint16_t rawPcmDir = speechPacketHeader->u16RawPcmDir;
        uint16_t freq = speechPacketHeader->u16Freq;
        uint16_t pcmLength = speechPacketHeader->u16Length;
        uint16_t channel = speechPacketHeader->u16Channel;
        uint16_t bitFormat = speechPacketHeader->u16BitFormat;
        char *pcmData = ptr + sizeof(android::spcApRAWPCMBufHdrStruct);
        ALOGV("%s(), Process speech packet, syncWord = 0x%x, dir = %s, freq = %d, channel = %d, BitFormat = %d, length = %d\n", __FUNCTION__, syncWord, rawPcmDir == 0 ? "UL" : "DL", freq, channel, bitFormat, pcmLength);

        if (syncWord != EEMCS_M2A_SHARE_BUFF_HEADER_SYNC) {
            ALOGW("%s(), Invalid packet. (syncWord: 0x%x)\n", __FUNCTION__, syncWord);
            return NO_ERROR;
        }

        if (bitFormat != kTargetBitFormat) {
            ALOGW("%s(), Invalid packet. (bit format: %d)\n", __FUNCTION__, bitFormat);
            return NO_ERROR;
        }

        if (rawPcmDir == RECORD_TYPE_UL) {
            ASSERT(pULPcmInputBuf == NULL);

#if SINE_WAVE_ENABLED
            setupLinearBufWithSineWave(pcmData, pcmLength);
#endif

            pULPcmInputBuf = pcmData;
            uULPcmInputBufSize = pcmLength;
            uULFreq = freq;

            if (uULFreq != mSrcSampleRateUL && uULFreq != kTargetSampleRate) {
                if (mBliSrcUL != NULL) {
                    deleteMtkAudioSrc(mBliSrcUL);
                    ALOGD("%s(), delete old BliSrcUL (SR:%d -> %d)\n", __FUNCTION__, mSrcSampleRateUL, kTargetSampleRate);
                }

                mSrcSampleRateUL = uULFreq;
                mBliSrcUL = newMtkAudioSrc(
                                mSrcSampleRateUL, channel,
                                kTargetSampleRate, kTargetULChannelNumber,
                                SRC_IN_Q1P15_OUT_Q1P15);
                mBliSrcUL->open();
                ALOGD("%s(), create BliSrcUL (SR:%d -> %d)\n", __FUNCTION__, mSrcSampleRateUL, kTargetSampleRate);
            }
        } else {
            ASSERT(pDLPcmInputBuf == NULL);

#if SINE_WAVE_ENABLED
            setupLinearBufWithSineWave(pcmData, pcmLength);
#endif

            pDLPcmInputBuf = pcmData;
            uDLPcmInputBufSize = pcmLength;
            uDLFreq = freq;

            if (uDLFreq != mSrcSampleRateDL && uDLFreq != kTargetSampleRate) {
                if (mBliSrcDL != NULL) {
                    deleteMtkAudioSrc(mBliSrcDL);
                    ALOGD("%s(), delete old BliSrcDL (SR:%d -> %d)\n", __FUNCTION__, mSrcSampleRateDL, kTargetSampleRate);
                }

                mSrcSampleRateDL = uDLFreq;
                mBliSrcDL = newMtkAudioSrc(
                                mSrcSampleRateDL, channel,
                                kTargetSampleRate, kTargetDLChannelNumber,
                                SRC_IN_Q1P15_OUT_Q1P15);
                mBliSrcDL->open();
                ALOGD("%s(), create BliSrcDL (SR:%d -> %d)\n", __FUNCTION__, mSrcSampleRateDL, kTargetSampleRate);
            }
        }

        // Update ptr
        ptr += sizeof(android::spcApRAWPCMBufHdrStruct) + pcmLength;
        // ALOGD("%s(), One packet parsed, header size = %d, pcmLength = %d, ptr = 0x%x\n", __FUNCTION__, sizeof(android::spcApRAWPCMBufHdrStruct), pcmLength, ptr);
    }

    if (uULPcmInputBufSize != 0) {
        ALOGV("%s(), UL pcm buf = %p, size = %d\n", __FUNCTION__, pULPcmInputBuf, uULPcmInputBufSize);
    }

    if (uDLPcmInputBufSize != 0) {
        ALOGV("%s(), DL pcm buf = %p, size = %d\n", __FUNCTION__, pDLPcmInputBuf, uDLPcmInputBufSize);
    }

    // 3. ul/dl pcm SRC
    char *pULSrcOutputBuf = NULL;
    if (uULFreq != 0 && uULFreq != kTargetSampleRate) {
        uint32_t uSrcOutputBufSize = uULPcmInputBufSize * kTargetSampleRate / uULFreq;
        ASSERT(uSrcOutputBufSize > 0);
        pULSrcOutputBuf = new char[uSrcOutputBufSize];

        char *p_read = pULPcmInputBuf;
        uint32_t num_raw_data_left = uULPcmInputBufSize;
        uint32_t num_converted_data = uSrcOutputBufSize;

        ALOGV("%s(), before SRC, num_raw_data_left = %u, num_converted_data = %u",
              __FUNCTION__, num_raw_data_left, num_converted_data);
        uint32_t consumed = num_raw_data_left;
        mBliSrcUL->process((int16_t *)p_read, &num_raw_data_left,
                           (int16_t *)pULSrcOutputBuf, &num_converted_data);
        consumed -= num_raw_data_left;
        ALOGV("%s(), after num_raw_data_left = %u, num_converted_data = %u",
              __FUNCTION__, num_raw_data_left, num_converted_data);

        ASSERT(num_raw_data_left == 0);

        // Update size & pointer
        uULPcmInputBufSize = num_converted_data;
        pULPcmInputBuf = pULSrcOutputBuf;
    }

    char *pDLSrcOutputBuf = NULL;
    if (uDLFreq != 0 && uDLFreq != kTargetSampleRate) {
        uint32_t uSrcOutputBufSize = uDLPcmInputBufSize * kTargetSampleRate / uDLFreq;
        ASSERT(uSrcOutputBufSize > 0);
        pDLSrcOutputBuf = new char[uSrcOutputBufSize];

        char *p_read = pDLPcmInputBuf;
        uint32_t num_raw_data_left = uDLPcmInputBufSize;
        uint32_t num_converted_data = uSrcOutputBufSize;

        ALOGV("%s(), before SRC, num_raw_data_left = %u, num_converted_data = %u",
              __FUNCTION__, num_raw_data_left, num_converted_data);
        uint32_t consumed = num_raw_data_left;
        mBliSrcDL->process((int16_t *)p_read, &num_raw_data_left,
                           (int16_t *)pDLSrcOutputBuf, &num_converted_data);
        consumed -= num_raw_data_left;
        ALOGV("%s(), num_raw_data_left = %u, num_converted_data = %u",
              __FUNCTION__, num_raw_data_left, num_converted_data);

        ASSERT(num_raw_data_left == 0);

        // Update size & pointer
        uDLPcmInputBufSize = num_converted_data;
        pDLPcmInputBuf = pDLSrcOutputBuf;
    }

    if (pULSrcOutputBuf != NULL) {
        ALOGV("%s(), After SRC, UL buf = %p, size = %d\n", __FUNCTION__, pULPcmInputBuf, uULPcmInputBufSize);
    }

    if (pDLSrcOutputBuf != NULL) {
        ALOGV("%s(), After SRC, DL buf = %p, size = %d\n", __FUNCTION__, pDLPcmInputBuf, uDLPcmInputBufSize);
    }

    // 4. Dispatch UL/DL data to UL/DL/MIX data provider
    for (int recordType = RECORD_TYPE_UL; recordType <= RECORD_TYPE_MIX; recordType++) {
        switch (recordType) {
        case RECORD_TYPE_UL:
            if (pULPcmInputBuf != NULL && AudioALSACaptureDataProviderVoiceUL::hasInstance()) {
                RingBuf ringBuf;
                ringBuf.pBufBase = pULPcmInputBuf;
                ringBuf.bufLen   = uULPcmInputBufSize + 1;
                ringBuf.pRead    = ringBuf.pBufBase;
                ringBuf.pWrite   = ringBuf.pRead + uULPcmInputBufSize;
                ringBuf.pBufEnd  = ringBuf.pBufBase + ringBuf.bufLen;

                AudioALSACaptureDataProviderVoiceUL::getInstance()->provideModemRecordDataToProvider(ringBuf);
            }
            break;
        case RECORD_TYPE_DL:
            if (pDLPcmInputBuf != NULL && AudioALSACaptureDataProviderVoiceDL::hasInstance()) {
                RingBuf ringBuf;
                ringBuf.pBufBase = pDLPcmInputBuf;
                ringBuf.bufLen   = uDLPcmInputBufSize + 1;
                ringBuf.pRead    = ringBuf.pBufBase;
                ringBuf.pWrite   = ringBuf.pBufBase + uDLPcmInputBufSize;
                ringBuf.pBufEnd  = ringBuf.pBufBase + ringBuf.bufLen;

                AudioALSACaptureDataProviderVoiceDL::getInstance()->provideModemRecordDataToProvider(ringBuf);
            }
            break;
        case RECORD_TYPE_MIX:
            if (pULPcmInputBuf != NULL && pDLPcmInputBuf != NULL && AudioALSACaptureDataProviderVoiceMix::hasInstance()) {
                // [TODO] only support 16bit now (JH)
                ASSERT(kTargetBitFormat == AUDIO_FORMAT_PCM_16_BIT);
                uint32_t bufferSize = (uDLPcmInputBufSize > uULPcmInputBufSize ? uULPcmInputBufSize : uDLPcmInputBufSize) * 2;

                char *pcmMixBuf = new char[bufferSize];
                uint32_t samples = bufferSize / 2 / 2;  /* 2ch / 16bit */
                for (uint32_t index = 0; index < samples; index++) {
                    ((uint16_t *)pcmMixBuf)[index * 2] = ((uint16_t *)pDLPcmInputBuf)[index];
                    ((uint16_t *)pcmMixBuf)[index * 2 + 1] = ((uint16_t *)pULPcmInputBuf)[index];
                }

                RingBuf ringBuf;
                ringBuf.pBufBase = pcmMixBuf;
                ringBuf.bufLen   = bufferSize + 1;
                ringBuf.pRead    = ringBuf.pBufBase;
                ringBuf.pWrite   = ringBuf.pBufBase + bufferSize;
                ringBuf.pBufEnd  = ringBuf.pBufBase + ringBuf.bufLen;

                AudioALSACaptureDataProviderVoiceMix::getInstance()->provideModemRecordDataToProvider(ringBuf);
                delete[] pcmMixBuf;
            }
            break;
        }
    }

    if (pULSrcOutputBuf != NULL) {
        delete[] pULSrcOutputBuf;
    }

    if (pDLSrcOutputBuf != NULL) {
        delete[] pDLSrcOutputBuf;
    }

    return NO_ERROR;
}

} // end of namespace android


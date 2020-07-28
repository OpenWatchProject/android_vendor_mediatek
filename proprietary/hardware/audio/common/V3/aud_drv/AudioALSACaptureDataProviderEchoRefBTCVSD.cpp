#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureDataProviderEchoRefBTCVSD"

#include "AudioALSACaptureDataProviderEchoRefBTCVSD.h"

#include <math.h>
#include <pthread.h>

#include <sys/prctl.h>

#include "AudioType.h"

#include "AudioALSADriverUtility.h"
#include "AudioALSAPlaybackHandlerBTCVSD.h"
#include "AudioALSASampleRateController.h"
#include "WCNChipController.h"

//#define DEBUG_TIMESTAMP

#ifdef DEBUG_TIMESTAMP
#define SHOW_TIMESTAMP(format, args...) ALOGD(format, ##args)
#else
#define SHOW_TIMESTAMP(format, args...)
#endif

#define MAX_LOCK_TIME_OUT_MS (500)
#define MAX_READ_DATA_TIME_OUT_MS (60)

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)

namespace android {


/*==============================================================================
 *                     Constant
 *============================================================================*/

static uint32_t kReadBufferSize = 0;


/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderEchoRefBTCVSD *AudioALSACaptureDataProviderEchoRefBTCVSD::mAudioALSACaptureDataProviderEchoRefBTCVSD = NULL;
AudioALSACaptureDataProviderEchoRefBTCVSD *AudioALSACaptureDataProviderEchoRefBTCVSD::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderEchoRefBTCVSD == NULL) {
        mAudioALSACaptureDataProviderEchoRefBTCVSD = new AudioALSACaptureDataProviderEchoRefBTCVSD();
    }
    ASSERT(mAudioALSACaptureDataProviderEchoRefBTCVSD != NULL);
    return mAudioALSACaptureDataProviderEchoRefBTCVSD;
}

AudioALSACaptureDataProviderEchoRefBTCVSD::AudioALSACaptureDataProviderEchoRefBTCVSD() :
    mTotalCaptureBufSize(0),
    mWCNChipController(WCNChipController::GetInstance()),
    hReadThread(0) {
    ALOGD("%s()", __FUNCTION__);

    mCaptureDataProviderType = CAPTURE_PROVIDER_ECHOREF_BTCVSD;

    memset((void *)&mDataRingBuf, 0, sizeof(mDataRingBuf));

    memset((void *)&mCaptureStartTime, 0, sizeof(mCaptureStartTime));

    memset((void *)&mEstimateTimeStamp, 0, sizeof(mEstimateTimeStamp));

    memset((void *)&mOldEstimateTimeStamp, 0, sizeof(mOldEstimateTimeStamp));

    memset((void *)&mNewtime, 0, sizeof(mNewtime));

    memset((void *)&mOldtime, 0, sizeof(mOldtime));
}

AudioALSACaptureDataProviderEchoRefBTCVSD::~AudioALSACaptureDataProviderEchoRefBTCVSD() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderEchoRefBTCVSD::open() {
    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mEnable == false);

    AudioALSASampleRateController *pAudioALSASampleRateController = AudioALSASampleRateController::getInstance();
    pAudioALSASampleRateController->setScenarioStatus(PLAYBACK_SCENARIO_ECHO_REF);

    // config attribute (will used in client SRC/Enh/... later) // TODO(Sam): query the mConfig?
    mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeSource.num_channels = popcount(mStreamAttributeSource.audio_channel_mask);
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    mStreamAttributeSource.sample_rate = mWCNChipController->GetBTCurrentSamplingRateNumber();
#else
    mStreamAttributeSource.sample_rate = pAudioALSASampleRateController->getPrimaryStreamOutSampleRate();
#endif
    mStreamAttributeSource.latency = UPLINK_NORMAL_LATENCY_MS;

    uint32_t frameBytes = mStreamAttributeSource.num_channels * audio_bytes_per_sample(mStreamAttributeSource.audio_format);
    kReadBufferSize = (mStreamAttributeSource.sample_rate * mStreamAttributeSource.latency / 1000) * frameBytes;

    /* Reset buffer & timestamp info */
    initDataRingBuf(kReadBufferSize * 10);
    resetTimeStampInfo();

    ALOGD("%s(), audio_format = %d, audio_channel_mask=%x, num_channels=%d, sample_rate=%d, latency=%dms", __FUNCTION__,
          mStreamAttributeSource.audio_format, mStreamAttributeSource.audio_channel_mask, mStreamAttributeSource.num_channels, mStreamAttributeSource.sample_rate, mStreamAttributeSource.latency);

    OpenPCMDump(LOG_TAG);

    // create reading thread
    mEnable = true;
    int ret = pthread_create(&hReadThread, NULL, AudioALSACaptureDataProviderEchoRefBTCVSD::readThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    ALOGD("-%s()", __FUNCTION__);

    return NO_ERROR;
}

status_t AudioALSACaptureDataProviderEchoRefBTCVSD::close() {
    ALOGD("+%s()", __FUNCTION__);

    if (mEnable == true) {
        mEnable = false;

        /* Signal the echo ref data waiting to avoid deadlock */
        signalDataWaiting();

        ClosePCMDump();


        AudioALSASampleRateController *pAudioALSASampleRateController = AudioALSASampleRateController::getInstance();
        pAudioALSASampleRateController->resetScenarioStatus(PLAYBACK_SCENARIO_ECHO_REF);

        deinitDataRingBuf();
    }

    ALOGD("-%s()", __FUNCTION__);

    return NO_ERROR;
}

void *AudioALSACaptureDataProviderEchoRefBTCVSD::readThread(void *arg) {
    status_t retval = NO_ERROR;
    AudioALSACaptureDataProviderEchoRefBTCVSD *pDataProvider = static_cast<AudioALSACaptureDataProviderEchoRefBTCVSD *>(arg);

    uint32_t open_index = pDataProvider->mOpenIndex;

    char nameset[32];
    sprintf(nameset, "%s%d", __FUNCTION__, pDataProvider->mCaptureDataProviderType);
    prctl(PR_SET_NAME, (unsigned long)nameset, 0, 0, 0);
    pDataProvider->setThreadPriority();

    ALOGD("+%s(), pid: %d, tid: %d, kReadBufferSize=%x", __FUNCTION__, getpid(), gettid(), kReadBufferSize);

    // read raw data from alsa driver
    char linear_buffer[kReadBufferSize];
    while (pDataProvider->mEnable == true) {
        if (open_index != pDataProvider->mOpenIndex) {
            ALOGD("%s(), open_index(%d) != mOpenIndex(%d), return", __FUNCTION__, open_index, pDataProvider->mOpenIndex);
            break;
        }

        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[0] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;

        /* Get capture timestamp by start tiem */
        pDataProvider->GetCaptureTimeStampByStartTime(&pDataProvider->mStreamAttributeSource.Time_Info);

        ALOGV("%s(), EchoRef_mTotalEchoRefBufSize = unknown, read size = %d, newTimeStamp = %ld.%09ld",
              __FUNCTION__, kReadBufferSize, pDataProvider->mStreamAttributeSource.Time_Info.timestamp_get.tv_sec, pDataProvider->mStreamAttributeSource.Time_Info.timestamp_get.tv_nsec);

        /* Read data from internal ring buffer */
        int retval = pDataProvider->readData(linear_buffer, kReadBufferSize);
        if (retval != NO_ERROR) {
            ALOGE("%s(), readData() error, retval = %d", __FUNCTION__, retval);
            continue;
        }

        pDataProvider->mPcmReadBuf.pBufBase = linear_buffer;
        pDataProvider->mPcmReadBuf.bufLen   = kReadBufferSize + 1; // +1: avoid pRead == pWrite
        pDataProvider->mPcmReadBuf.pRead    = linear_buffer;
        pDataProvider->mPcmReadBuf.pWrite   = linear_buffer + kReadBufferSize;


        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[1] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;

        if (pDataProvider->mStreamAttributeSource.input_source == AUDIO_SOURCE_ECHO_REFERENCE) {
            pDataProvider->provideCaptureDataToAllClients(open_index);
        } else {
            pDataProvider->provideEchoRefCaptureDataToAllClients(open_index);
        }
        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[2] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;
        ALOGV("%s, latency_in_us,%1.6lf,%1.6lf,%1.6lf", __FUNCTION__, pDataProvider->timerec[0], pDataProvider->timerec[1], pDataProvider->timerec[2]);
    }

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_exit(NULL);
    return NULL;
}


void AudioALSACaptureDataProviderEchoRefBTCVSD::initDataRingBuf(uint32_t size) {
    ALOGD("+%s()", __FUNCTION__);

    AL_LOCK_MS(mDataBufLock, MAX_LOCK_TIME_OUT_MS);

    if (mDataRingBuf.pBufBase) {
        delete[] mDataRingBuf.pBufBase;
    }

    mDataRingBuf.pBufBase = new char[size];
    mDataRingBuf.bufLen   = size;
    mDataRingBuf.pRead    = mDataRingBuf.pBufBase;
    mDataRingBuf.pWrite   = mDataRingBuf.pBufBase;

    AL_UNLOCK(mDataBufLock);

    ALOGD("-%s()", __FUNCTION__);
}

void AudioALSACaptureDataProviderEchoRefBTCVSD::deinitDataRingBuf() {
    ALOGD("+%s()", __FUNCTION__);

    AL_LOCK_MS(mDataBufLock, MAX_LOCK_TIME_OUT_MS);

    if (mDataRingBuf.pBufBase) {
        delete[] mDataRingBuf.pBufBase;
        memset(&mDataRingBuf, 0, sizeof(mDataRingBuf));
    }

    AL_UNLOCK(mDataBufLock);

    ALOGD("-%s()", __FUNCTION__);
}

void AudioALSACaptureDataProviderEchoRefBTCVSD::resetTimeStampInfo() {
    // Reset timestamp struct
    mCaptureStartTime.tv_nsec = 0;
    mCaptureStartTime.tv_sec = 0;

    mEstimateTimeStamp.tv_nsec = 0;
    mEstimateTimeStamp.tv_sec = 0;

    mOldEstimateTimeStamp.tv_nsec = 0;
    mOldEstimateTimeStamp.tv_sec = 0;

    // Reset total data read counter
    mTotalCaptureBufSize = 0;
}

uint32_t AudioALSACaptureDataProviderEchoRefBTCVSD::compensateSilenceData(uint32_t msec, RingBuf *ringBuf) {
    // DL data is not enough, compensate silence data
    uint32_t compensateBytes = msec * mStreamAttributeSource.num_channels * audio_bytes_per_sample(mStreamAttributeSource.audio_format) * mStreamAttributeSource.sample_rate / 1000;
    compensateBytes = compensateBytes - compensateBytes % (mStreamAttributeSource.num_channels * audio_bytes_per_sample(mStreamAttributeSource.audio_format));

    int freeSpace = RingBuf_getFreeSpace(ringBuf);
    if ((uint32_t)freeSpace >= compensateBytes) {
        ALOGD("%s() msec = %d, compensateBytes  = %d",
              __FUNCTION__,
              msec,
              compensateBytes);

        RingBuf_fillZero(ringBuf, compensateBytes);
    } else {
        ALOGE("%s(), buffer overflow! (msec = %d, %d < %d)", __FUNCTION__, msec, freeSpace, compensateBytes);
        freeSpace = freeSpace - freeSpace % (mStreamAttributeSource.num_channels * audio_bytes_per_sample(mStreamAttributeSource.audio_format));
        RingBuf_fillZero(ringBuf, freeSpace);
        compensateBytes = freeSpace;
    }

    return compensateBytes;
}

// TODO(JH): change name to writeEchoRefData?
status_t AudioALSACaptureDataProviderEchoRefBTCVSD::writeData(const char *data, uint32_t dataSize, struct timespec *timestamp) {
    status_t ret = NO_ERROR;
    ALOGV("+%s()", __FUNCTION__);

    /* Push pcm data to echoref buffer */
    AL_LOCK_MS(mDataBufLock, MAX_LOCK_TIME_OUT_MS);

    if ((uint32_t)RingBuf_getFreeSpace(&mDataRingBuf) < dataSize) {
        ASSERT(1);
        ALOGE("%s(), data buffer overflow! (%d > %d)", __FUNCTION__, dataSize, RingBuf_getFreeSpace(&mDataRingBuf));
        // TODO(JH): buffer overflow, resync?
        ret = -ENOSYS;
    } else {
        /* Using new time stamp to update mCaptureStartTime if mCaptureStartTime not set */
        updateStartTimeStamp(timestamp);

        bool compensateData = false;
        bool writeData = false;
        int timeDiff = (int)round(calc_time_diff((*timestamp), mEstimateTimeStamp) * 1000);
        uint32_t compensateBytes = 0;
        uint32_t totalDataSize = 0;

        if (timeDiff != 0 && (mEstimateTimeStamp.tv_sec != 0 || mEstimateTimeStamp.tv_nsec != 0)) {
            if (timeDiff > 0) {
                // Data is not enough, compensate it.
                compensateData = true;
                writeData = true;
            } else {
                // DL data is out of date, ignore this data
                compensateData = false;
                writeData = false;
            }
        } else {
            // Normal case, just wrie data, no compensation
            compensateData = false;
            writeData = true;
        }

        SHOW_TIMESTAMP("%s() timeDiff = %d (%ld.%09ld->%ld.%09ld), compensateData = %d, writeData = %d",
                       __FUNCTION__,
                       timeDiff,
                       mEstimateTimeStamp.tv_sec, mEstimateTimeStamp.tv_nsec,
                       timestamp->tv_sec, timestamp->tv_nsec,
                       compensateData,
                       writeData);

        if (compensateData) {
            /* If this buffer's time stamp is not expected, compensate it. (my caused by DL open/close) */
            compensateBytes = compensateSilenceData((uint32_t)timeDiff, &mDataRingBuf);
        }

        if (writeData) {
            if (((uint32_t)RingBuf_getFreeSpace(&mDataRingBuf) >= dataSize)) {
                /* Write new data */
                RingBuf_copyFromLinear(&mDataRingBuf, (const char *)data, dataSize);

                totalDataSize = dataSize + compensateBytes;

                /* Calculate mEstimateTimeStamp */
                calculateTimeStampByBytes(*timestamp, dataSize, mStreamAttributeSource, &mEstimateTimeStamp);

                SHOW_TIMESTAMP("%s() update mEstimateTimeStamp (%ld.%09ld->%ld.%09ld)",
                               __FUNCTION__,
                               timestamp->tv_sec, timestamp->tv_nsec,
                               mEstimateTimeStamp.tv_sec, mEstimateTimeStamp.tv_nsec);
            } else {
                totalDataSize = compensateBytes;

                /* Calculate mEstimateTimeStamp */
                calculateTimeStampByBytes(mOldEstimateTimeStamp, totalDataSize, mStreamAttributeSource, &mEstimateTimeStamp);

                SHOW_TIMESTAMP("%s() update mEstimateTimeStamp (%ld.%09ld->%ld.%09ld)",
                               __FUNCTION__,
                               mOldEstimateTimeStamp.tv_sec, mOldEstimateTimeStamp.tv_nsec,
                               mEstimateTimeStamp.tv_sec, mEstimateTimeStamp.tv_nsec);
            }

            mOldEstimateTimeStamp = mEstimateTimeStamp;
        }
        SHOW_TIMESTAMP("%s(), write data size = %d (free = %d), signal...", __FUNCTION__, totalDataSize, RingBuf_getFreeSpace(&mDataRingBuf));

        /* Notify provider thread */
        AL_SIGNAL(mDataBufLock);
    }

    AL_UNLOCK(mDataBufLock);

    ALOGV("-%s()", __FUNCTION__);

    return ret;
}

status_t AudioALSACaptureDataProviderEchoRefBTCVSD::readData(char *buffer, uint32_t size) {
    ALOGV("+%s()", __FUNCTION__);

    AL_LOCK_MS(mDataBufLock, MAX_LOCK_TIME_OUT_MS);

    while ((uint32_t)RingBuf_getDataCount(&mDataRingBuf) < size) {
        SHOW_TIMESTAMP("%s(), echoref data is not enough, waiting... (data size = %d, read size = %d)", __FUNCTION__, RingBuf_getDataCount(&mDataRingBuf), size);
        if (AL_WAIT_MS(mDataBufLock, MAX_READ_DATA_TIME_OUT_MS) != 0) {
            SHOW_TIMESTAMP("-%s(), wait timeout! (data size = %d, read size = %d)", __FUNCTION__, RingBuf_getDataCount(&mDataRingBuf), size);
            AL_UNLOCK(mDataBufLock);
            return -ETIMEDOUT;
        }

        SHOW_TIMESTAMP("-%s(), echoref data is comming, wake up... (data size = %d, read size = %d)", __FUNCTION__, RingBuf_getDataCount(&mDataRingBuf), size);

        if (mEnable == false) {
            SHOW_TIMESTAMP("-%s(), closed, exit readData()", __FUNCTION__);
            AL_UNLOCK(mDataBufLock);
            return -ENOSYS;
        }
    }

    /* Read data from DataRingBuf */
    uint32_t srcDataSize = RingBuf_getDataCount(&mDataRingBuf);
    uint32_t copyBufSize = size > srcDataSize ? srcDataSize : size;
    RingBuf_copyToLinear(buffer, &mDataRingBuf, copyBufSize);
    SHOW_TIMESTAMP("%s(), read data size = %d (dstFreeSize = %d, srcDataSize = %d)", __FUNCTION__, copyBufSize, size, srcDataSize);

    updateTotalCaptureBufSize(copyBufSize);

    AL_UNLOCK(mDataBufLock);

    ALOGV("-%s()", __FUNCTION__);

    return NO_ERROR;
}

void AudioALSACaptureDataProviderEchoRefBTCVSD::signalDataWaiting() {
    ALOGV("+%s()", __FUNCTION__);
    AL_LOCK_MS(mDataBufLock, MAX_LOCK_TIME_OUT_MS);
    AL_SIGNAL(mDataBufLock);
    AL_UNLOCK(mDataBufLock);
    ALOGV("-%s()", __FUNCTION__);
}

void AudioALSACaptureDataProviderEchoRefBTCVSD::updateTotalCaptureBufSize(uint32_t captureSize) {
    mTotalCaptureBufSize += captureSize;
}

void AudioALSACaptureDataProviderEchoRefBTCVSD::updateStartTimeStamp(struct timespec *timeStamp) {
    if (mCaptureStartTime.tv_sec == 0 && mCaptureStartTime.tv_nsec == 0) {
        if (timeStamp) {
            mCaptureStartTime = *timeStamp;
        } else {
            clock_gettime(CLOCK_REALTIME, &mCaptureStartTime);  // Use CLOCK_REALTIME to align kernel timestamp format
        }
        ALOGD("%s(), update DataProvider(%d) start timestamp (%ld.%09ld), mTotalCaptureBufSize = %d", __FUNCTION__, mCaptureDataProviderType, mCaptureStartTime.tv_sec, mCaptureStartTime.tv_nsec, mTotalCaptureBufSize);
    } else {
        ALOGV("%s(), DataProvider(%d) start timestamp (%ld.%09ld), mTotalCaptureBufSize = %d", __FUNCTION__, mCaptureDataProviderType, mCaptureStartTime.tv_sec, mCaptureStartTime.tv_nsec, mTotalCaptureBufSize);
    }
}

status_t AudioALSACaptureDataProviderEchoRefBTCVSD::GetCaptureTimeStampByStartTime(time_info_struct_t *timeInfo) {
    timeInfo->buffer_per_time = 0;
    timeInfo->kernelbuffer_ns = 0;
    timeInfo->frameInfo_get = 0;
    calculateTimeStampByBytes(mCaptureStartTime, mTotalCaptureBufSize, mStreamAttributeSource, &timeInfo->timestamp_get);

    return NO_ERROR;
}

} // end of namespace android

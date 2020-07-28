#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSACaptureDataProviderBTSCO"

#include "AudioALSACaptureDataProviderBTSCO.h"

#include <pthread.h>

#include <sys/prctl.h>

#include "AudioALSADriverUtility.h"
#include "AudioType.h"

#include "WCNChipController.h"
#include "AudioALSAStreamManager.h"

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)

namespace android {


/*==============================================================================
 *                     Constant
 *============================================================================*/

static uint32_t kReadBufferSize = 0;


/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderBTSCO *AudioALSACaptureDataProviderBTSCO::mAudioALSACaptureDataProviderBTSCO = NULL;
AudioALSACaptureDataProviderBTSCO *AudioALSACaptureDataProviderBTSCO::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderBTSCO == NULL) {
        mAudioALSACaptureDataProviderBTSCO = new AudioALSACaptureDataProviderBTSCO();
    }
    ASSERT(mAudioALSACaptureDataProviderBTSCO != NULL);
    return mAudioALSACaptureDataProviderBTSCO;
}

AudioALSACaptureDataProviderBTSCO::AudioALSACaptureDataProviderBTSCO() :
    mWCNChipController(WCNChipController::GetInstance()),
    hReadThread(0) {
    ALOGD("%s()", __FUNCTION__);

    mCaptureDataProviderType = CAPTURE_PROVIDER_BT_SCO;
    memset(&mNewtime, 0, sizeof(mNewtime));
    memset(&mOldtime, 0, sizeof(mOldtime));
    memset(timerec, 0, sizeof(timerec));
}

AudioALSACaptureDataProviderBTSCO::~AudioALSACaptureDataProviderBTSCO() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderBTSCO::open() {
    ALOGD("%s()", __FUNCTION__);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    ASSERT(mEnable == false);

    // config attribute (will used in client SRC/Enh/... later) // TODO(Harvey): query this
    mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_MONO;
    mStreamAttributeSource.num_channels = popcount(mStreamAttributeSource.audio_channel_mask);
    mStreamAttributeSource.sample_rate = mWCNChipController->GetBTCurrentSamplingRateNumber();

    // Reset frames readed counter
    mStreamAttributeSource.Time_Info.total_frames_readed = 0;


    // pcm config
    mConfig.channels = mStreamAttributeSource.num_channels;
    mConfig.rate = mStreamAttributeSource.sample_rate;

    mlatency = UPLINK_NORMAL_LATENCY_MS ; //20ms
#ifdef UPLINK_LOW_LATENCY
    if (HasLowLatencyCapture()) { mlatency = UPLINK_LOW_LATENCY_MS; }
#endif

    mConfig.period_size = (mlatency * mConfig.rate) / 1000;
    mConfig.period_count = 4;
    mConfig.format = PCM_FORMAT_S16_LE;

    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;

    kReadBufferSize = mConfig.period_size * mConfig.channels * (pcm_format_to_bits(mConfig.format) / 8);

    ALOGD("%s(), audio_format = %d, audio_channel_mask=%x, num_channels=%d, sample_rate=%d", __FUNCTION__,
          mStreamAttributeSource.audio_format, mStreamAttributeSource.audio_channel_mask, mStreamAttributeSource.num_channels, mStreamAttributeSource.sample_rate);

    ALOGD("%s(), format = %d, channels=%d, rate=%d, kReadBufferSize %u", __FUNCTION__,
          mConfig.format, mConfig.channels, mConfig.rate, kReadBufferSize);

    OpenPCMDump(LOG_TAG);

    // enable pcm
    ASSERT(mPcm == NULL);
    int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVOIPCallBTCapture);
    int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVOIPCallBTCapture);
    mPcm = pcm_open(cardIdx, pcmIdx, PCM_IN | PCM_MONOTONIC, &mConfig);
    ASSERT(mPcm != NULL && pcm_is_ready(mPcm) == true);
    ALOGV("%s(), mPcm = %p", __FUNCTION__, mPcm);

    if (isNeedSyncPcmStart() == false) {
        pcm_start(mPcm);
    } else {
        mStart = false;
        mReadThreadReady = false;
    }

    // create reading thread
    mEnable = true;
    int ret = pthread_create(&hReadThread, NULL, AudioALSACaptureDataProviderBTSCO::readThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}

status_t AudioALSACaptureDataProviderBTSCO::close() {
    ALOGD("%s()", __FUNCTION__);

    mEnable = false;
    pthread_join(hReadThread, NULL);
    ALOGD("pthread_join hReadThread done");

    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    ClosePCMDump();

    pcm_stop(mPcm);
    pcm_close(mPcm);
    mPcm = NULL;

    return NO_ERROR;
}

void *AudioALSACaptureDataProviderBTSCO::readThread(void *arg) {
    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);

    ALOGD("+%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    status_t retval = NO_ERROR;
    AudioALSACaptureDataProviderBTSCO *pDataProvider = static_cast<AudioALSACaptureDataProviderBTSCO *>(arg);
    pDataProvider->setThreadPriority();

    uint32_t open_index = pDataProvider->mOpenIndex;

    pDataProvider->waitPcmStart();

    // read raw data from alsa driver
    char linear_buffer[kReadBufferSize];
    while (pDataProvider->mEnable == true) {
        if (open_index != pDataProvider->mOpenIndex) {
            ALOGD("%s(), open_index(%d) != mOpenIndex(%d), return", __FUNCTION__, open_index, pDataProvider->mOpenIndex);
            break;
        }

        ASSERT(pDataProvider->mPcm != NULL);

        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[0] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;

        int retval = pcm_read(pDataProvider->mPcm, linear_buffer, kReadBufferSize);
        if (retval != 0) {
            ALOGE("%s(), pcm_read() error, retval = %d", __FUNCTION__, retval);
        }

        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[1] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;


        //struct timespec tempTimeStamp;
        pDataProvider->GetCaptureTimeStamp(&pDataProvider->mStreamAttributeSource.Time_Info, kReadBufferSize);

        // use ringbuf format to save buffer info
        pDataProvider->mPcmReadBuf.pBufBase = linear_buffer;
        pDataProvider->mPcmReadBuf.bufLen   = kReadBufferSize + 1; // +1: avoid pRead == pWrite
        pDataProvider->mPcmReadBuf.pRead    = linear_buffer;
        pDataProvider->mPcmReadBuf.pWrite   = linear_buffer + kReadBufferSize;

        pDataProvider->provideCaptureDataToAllClients(open_index);

        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[2] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;
        ALOGD("%s, latency_in_us,%1.6lf,%1.6lf,%1.6lf", __FUNCTION__, pDataProvider->timerec[0], pDataProvider->timerec[1], pDataProvider->timerec[2]);
    }

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_exit(NULL);
    return NULL;
}

} // end of namespace android

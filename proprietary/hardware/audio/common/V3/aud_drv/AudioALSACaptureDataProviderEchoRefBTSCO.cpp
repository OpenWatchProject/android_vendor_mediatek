#include "AudioALSACaptureDataProviderEchoRefBTSCO.h"

#include <pthread.h>

#include <sys/prctl.h>

#include "AudioALSADriverUtility.h"
#include "AudioType.h"
#include "WCNChipController.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureDataProviderEchoRefBTSCO"

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)

namespace android {


/*==============================================================================
 *                     Constant
 *============================================================================*/

//static const uint32_t kReadBufferSize = 0x1B90; // 44.1K\stereo\40ms data (framebase: get 40ms data/per time), downlink sample rate(DL1)
static const uint32_t kReadBufferSize = 0x500; // 16K\stereo\20 ms data



/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderEchoRefBTSCO *AudioALSACaptureDataProviderEchoRefBTSCO::mAudioALSACaptureDataProviderEchoRefBTSCO = NULL;
AudioALSACaptureDataProviderEchoRefBTSCO *AudioALSACaptureDataProviderEchoRefBTSCO::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderEchoRefBTSCO == NULL) {
        mAudioALSACaptureDataProviderEchoRefBTSCO = new AudioALSACaptureDataProviderEchoRefBTSCO();
    }
    ASSERT(mAudioALSACaptureDataProviderEchoRefBTSCO != NULL);
    return mAudioALSACaptureDataProviderEchoRefBTSCO;
}

AudioALSACaptureDataProviderEchoRefBTSCO::AudioALSACaptureDataProviderEchoRefBTSCO() :
    mWCNChipController(WCNChipController::GetInstance()),
    hReadThread(0) {
    ALOGD("%s()", __FUNCTION__);

    mCaptureDataProviderType = CAPTURE_PROVIDER_ECHOREF_BTSCO;
    memset(&mNewtime, 0, sizeof(mNewtime));
    memset(&mOldtime, 0, sizeof(mOldtime));
}

AudioALSACaptureDataProviderEchoRefBTSCO::~AudioALSACaptureDataProviderEchoRefBTSCO() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderEchoRefBTSCO::open() {
    ALOGD("%s()", __FUNCTION__);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    ASSERT(mEnable == false);

    // config attribute (will used in client SRC/Enh/... later) // TODO(Sam): query the mConfig?
    mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeSource.num_channels = popcount(mStreamAttributeSource.audio_channel_mask);
    mStreamAttributeSource.sample_rate = mWCNChipController->GetBTCurrentSamplingRateNumber();

    // Reset frames readed counter
    mStreamAttributeSource.Time_Info.total_frames_readed = 0;


    mConfig.channels = mStreamAttributeSource.num_channels;
    mConfig.rate = mStreamAttributeSource.sample_rate;

    // Buffer size: 2048(period_size) * 2(ch) * 2(byte) * 8(period_count) = 64 kb
    mConfig.period_size = 2048;
    mConfig.period_count = 8;
    mConfig.format = PCM_FORMAT_S16_LE;

    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;


#if 0
    //latency time, set as DataProvider buffer size
    mStreamAttributeSource.latency = (kReadBufferSize * 1000) / (mStreamAttributeSource.num_channels * mStreamAttributeSource.sample_rate *
                                                                 (mStreamAttributeSource.audio_format == AUDIO_FORMAT_PCM_8_BIT ? 1 :    //8  1byte/frame
                                                                  (mStreamAttributeSource.audio_format == AUDIO_FORMAT_PCM_32_BIT ? 4 :   //24bit 3bytes/frame
                                                                   2)));   //default 2bytes/sample
#else
    //latency time, set as hardware buffer size
    mStreamAttributeSource.latency = (mConfig.period_size * mConfig.period_count * 1000) / mConfig.rate;
#endif

    ALOGD("%s(), audio_format = %d, audio_channel_mask=%x, num_channels=%d, sample_rate=%d, latency=%dms", __FUNCTION__,
          mStreamAttributeSource.audio_format, mStreamAttributeSource.audio_channel_mask, mStreamAttributeSource.num_channels, mStreamAttributeSource.sample_rate, mStreamAttributeSource.latency);

    ALOGD("%s(), format = %d, channels=%d, rate=%d", __FUNCTION__,
          mConfig.format, mConfig.channels, mConfig.rate);

    OpenPCMDump(LOG_TAG);

    // enable pcm
    ASSERT(mPcm == NULL);
    int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmDl1AwbCapture);
    int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmDl1AwbCapture);
    mPcm = pcm_open(cardIdx, pcmIdx, PCM_IN | PCM_MONOTONIC, &mConfig);
    ASSERT(mPcm != NULL && pcm_is_ready(mPcm) == true);

    mStart = false;
    mReadThreadReady = false;

    // create reading thread
    mEnable = true;
    int ret = pthread_create(&hReadThread, NULL, AudioALSACaptureDataProviderEchoRefBTSCO::readThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}

status_t AudioALSACaptureDataProviderEchoRefBTSCO::close() {
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

void *AudioALSACaptureDataProviderEchoRefBTSCO::readThread(void *arg) {
    status_t retval = NO_ERROR;
    AudioALSACaptureDataProviderEchoRefBTSCO *pDataProvider = static_cast<AudioALSACaptureDataProviderEchoRefBTSCO *>(arg);

    uint32_t open_index = pDataProvider->mOpenIndex;

    char nameset[32];
    sprintf(nameset, "%s%d", __FUNCTION__, pDataProvider->mCaptureDataProviderType);
    prctl(PR_SET_NAME, (unsigned long)nameset, 0, 0, 0);
    pDataProvider->setThreadPriority();


    ALOGD("+%s(), pid: %d, tid: %d, kReadBufferSize=%x", __FUNCTION__, getpid(), gettid(), kReadBufferSize);

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

        pDataProvider->GetCaptureTimeStamp(&pDataProvider->mStreamAttributeSource.Time_Info, kReadBufferSize);

        // use ringbuf format to save buffer info
        pDataProvider->mPcmReadBuf.pBufBase = linear_buffer;
        pDataProvider->mPcmReadBuf.bufLen   = kReadBufferSize + 1; // +1: avoid pRead == pWrite
        pDataProvider->mPcmReadBuf.pRead    = linear_buffer;
        pDataProvider->mPcmReadBuf.pWrite   = linear_buffer + kReadBufferSize;

        //Provide EchoRef data
#if 0
        pDataProvider->provideCaptureDataToAllClients(open_index);
#else
        if (pDataProvider->mStreamAttributeSource.input_source == AUDIO_SOURCE_ECHO_REFERENCE) {
            pDataProvider->provideCaptureDataToAllClients(open_index);
        } else {
            pDataProvider->provideEchoRefCaptureDataToAllClients(open_index);
        }
#endif
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

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureDataProviderEchoRef"

#include "AudioALSACaptureDataProviderEchoRef.h"

#include <pthread.h>

#include <sys/prctl.h>

#include "AudioType.h"

#include "AudioALSADriverUtility.h"
#include "AudioALSASampleRateController.h"
#include "AudioALSAStreamManager.h"
#if defined(MTK_AUDIO_KS)
#include "AudioALSADeviceConfigManager.h"
#endif

#if defined(MTK_AUDIODSP_SUPPORT)
#include "AudioDspStreamManager.h"
#endif

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)

namespace android {


/*==============================================================================
 *                     Constant
 *============================================================================*/

static uint32_t kReadBufferSize = 0 ;

static bool btempDebug = false;

/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderEchoRef *AudioALSACaptureDataProviderEchoRef::mAudioALSACaptureDataProviderEchoRef = NULL;
AudioALSACaptureDataProviderEchoRef *AudioALSACaptureDataProviderEchoRef::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderEchoRef == NULL) {
        mAudioALSACaptureDataProviderEchoRef = new AudioALSACaptureDataProviderEchoRef();
    }
    ASSERT(mAudioALSACaptureDataProviderEchoRef != NULL);
    return mAudioALSACaptureDataProviderEchoRef;
}

AudioALSACaptureDataProviderEchoRef::AudioALSACaptureDataProviderEchoRef():
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    hReadThread(0) {
    ALOGD("%s()", __FUNCTION__);
    memset(&mNewtime, 0, sizeof(mNewtime));
    memset(&mOldtime, 0, sizeof(mOldtime));
    mCaptureDataProviderType = CAPTURE_PROVIDER_ECHOREF;
}

AudioALSACaptureDataProviderEchoRef::~AudioALSACaptureDataProviderEchoRef() {
    ALOGV("%s()", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderEchoRef::open() {
    ALOGV("%s()", __FUNCTION__);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());
    ASSERT(mEnable == false);

    AudioALSASampleRateController *pAudioALSASampleRateController = AudioALSASampleRateController::getInstance();
    pAudioALSASampleRateController->setScenarioStatus(PLAYBACK_SCENARIO_ECHO_REF);


    // config attribute (will used in client SRC/Enh/... later) // TODO(Sam): query the mConfig?
    mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeSource.num_channels = popcount(mStreamAttributeSource.audio_channel_mask);
    mStreamAttributeSource.sample_rate = pAudioALSASampleRateController->getPrimaryStreamOutSampleRate();

    // Reset frames readed counter
    mStreamAttributeSource.Time_Info.total_frames_readed = 0;

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    mlatency = UPLINK_NORMAL_LATENCY_MS;

#ifdef UPLINK_LOW_LATENCY
    if (HasLowLatencyCapture()) { mlatency = UPLINK_LOW_LATENCY_MS; }
#endif
#else
    mlatency = UPLINK_LOW_LATENCY_MS;

#ifdef UPLINK_LOW_LATENCY
    if (HasLowLatencyCapture()) { mlatency /= 2; }
#endif
#endif

    mConfig.rate = mStreamAttributeSource.sample_rate;
    mConfig.channels = mStreamAttributeSource.num_channels;
    mConfig.format = PCM_FORMAT_S16_LE;
    kReadBufferSize = getPeriodBufSize(&mStreamAttributeSource, mlatency);
#if 0 // cause latency issue...... = =a
    kReadBufferSize &= 0xFFFFFFC0; // (DL1)44.1K\20ms data\stereo\2byte\(Align64byte)
#endif
    //Buffer size: 2048(period_size) * 2(ch) * 2(byte) * 8(period_count) = 64 kb
    mConfig.period_size = kReadBufferSize / mConfig.channels / (pcm_format_to_bits(mConfig.format) / 8);
    mConfig.period_count = 160 / mlatency;

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

    ALOGD("%s(), audio_format = %d, output_device=0x%x, audio_channel_mask=%x, num_channels=%d, sample_rate=%d, buf_total_latency=%dms", __FUNCTION__,
          mStreamAttributeSource.audio_format, mStreamAttributeSource.output_devices, mStreamAttributeSource.audio_channel_mask, mStreamAttributeSource.num_channels, mStreamAttributeSource.sample_rate, mStreamAttributeSource.latency);

    ALOGD("%s(), format = %d, channels=%d, rate=%d, period_size=%d, period_count=%d,latency=%d,kReadBufferSize=%d", __FUNCTION__,
          mConfig.format, mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mlatency, kReadBufferSize);


    OpenPCMDump(LOG_TAG);

    btempDebug = AudioSpeechEnhanceInfo::getInstance()->GetDebugStatus();

#if defined(MTK_AUDIO_KS)
    int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture2);
    int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmCapture2);
#ifdef MTK_AUDIODSP_SUPPORT
    if (mStreamAttributeSource.output_devices & AUDIO_DEVICE_OUT_SPEAKER) {
        if (AudioDspStreamManager::getInstance()->getDspPlaybackEnable() == true) {
            mApTurnOnSequence = AUDIO_CTL_DL_MEMIF_TO_CAPTURE2_SPEAKER_HIFI3;
        } else {
            mApTurnOnSequence = AUDIO_CTL_DL_MEMIF_TO_CAPTURE2;
        }
    } else {
        if (AudioDspStreamManager::getInstance()->getDspPlaybackEnable() == true) {
            mApTurnOnSequence = AUDIO_CTL_DL_PLAYBACK_TO_CAPTURE2_NON_SPEAKER_HIFI3;
        } else {
            mApTurnOnSequence = AUDIO_CTL_DL_MEMIF_TO_CAPTURE2; //todo: remove this
        }
    }
#else
    mApTurnOnSequence = AUDIO_CTL_DL_MEMIF_TO_CAPTURE2;
#endif
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(mApTurnOnSequence);

    // need to set after query pcm_params_get, since shutdown will clear this state
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "echo_ref_xrun_assert"), 0, 1)) {
        ALOGW("%s(), echo_ref_xrun_assert enable fail", __FUNCTION__);
    }
#else
    int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmDl1AwbCapture);
    int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmDl1AwbCapture);
#endif

    // enable pcm
    ASSERT(mPcm == NULL);
    mPcm = pcm_open(cardIdx, pcmIdx, PCM_IN | PCM_MONOTONIC, &mConfig);
    ASSERT(mPcm != NULL && pcm_is_ready(mPcm) == true);

    mStart = false;
    mReadThreadReady = false;


    // create reading thread
    mEnable = true;
    int ret = pthread_create(&hReadThread, NULL, AudioALSACaptureDataProviderEchoRef::readThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}


status_t AudioALSACaptureDataProviderEchoRef::close() {
    ALOGD("%s()", __FUNCTION__);

#if defined(MTK_AUDIO_KS)
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "echo_ref_xrun_assert"), 0, 0)) {
        ALOGW("%s(), echo_ref_xrun_assert disable fail", __FUNCTION__);
    }
#endif

    mEnable = false;
    pthread_join(hReadThread, NULL);
    ALOGV("pthread_join hReadThread done");

    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    ClosePCMDump();

    pcm_stop(mPcm);
    pcm_close(mPcm);
    mPcm = NULL;

#if defined(MTK_AUDIO_KS)
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnoffSequenceByName(mApTurnOnSequence);
#endif

    AudioALSASampleRateController *pAudioALSASampleRateController = AudioALSASampleRateController::getInstance();
    pAudioALSASampleRateController->resetScenarioStatus(PLAYBACK_SCENARIO_ECHO_REF);

    return NO_ERROR;
}

void *AudioALSACaptureDataProviderEchoRef::readThread(void *arg) {
    status_t ret = NO_ERROR;
    int retval = 0;

    AudioALSACaptureDataProviderEchoRef *pDataProvider = static_cast<AudioALSACaptureDataProviderEchoRef *>(arg);

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

        if (btempDebug) {
            clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
            pDataProvider->timerec[0] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
            pDataProvider->mOldtime = pDataProvider->mNewtime;
        }

        int retval = pcm_read(pDataProvider->mPcm, linear_buffer, kReadBufferSize);
        if (retval != 0) {
            ALOGE("%s(), pcm_read() error, retval = %d", __FUNCTION__, retval);
            clock_gettime(CLOCK_REALTIME, &pDataProvider->mOldtime);
            continue;
        }

        //struct timespec tempTimeStamp;
        ret = pDataProvider->GetCaptureTimeStamp(&pDataProvider->mStreamAttributeSource.Time_Info, kReadBufferSize);
#if 0
        if (ret != NO_ERROR) {
            clock_gettime(CLOCK_REALTIME, &pDataProvider->mOldtime);
            continue;
        }
#endif

        if (btempDebug) {
            clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
            pDataProvider->timerec[1] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
            pDataProvider->mOldtime = pDataProvider->mNewtime;
        }

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

        if (btempDebug) {
            clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
            pDataProvider->timerec[2] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
            pDataProvider->mOldtime = pDataProvider->mNewtime;

            ALOGD("%s, latency_in_us,%1.6lf,%1.6lf,%1.6lf", __FUNCTION__, pDataProvider->timerec[0], pDataProvider->timerec[1], pDataProvider->timerec[2]);
        }
    }

    ALOGV("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_exit(NULL);
    return NULL;
}

} // end of namespace android

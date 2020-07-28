#include "AudioALSACaptureDataProviderEchoRefExt.h"

#include <pthread.h>

#include <sys/prctl.h>

#include "AudioType.h"
#include "AudioALSADriverUtility.h"
#include "AudioALSASampleRateController.h"
#include "AudioALSAStreamManager.h"
#if defined(MTK_AUDIO_KS)
#include "AudioALSADeviceConfigManager.h"
#endif

#include "AudioSmartPaController.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureDataProviderEchoRefExt"

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)

namespace android {

/*==============================================================================
 *                     Constant
 *============================================================================*/

static uint32_t kReadBufferSize = 0;

static bool btempDebug = false;


/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderEchoRefExt *AudioALSACaptureDataProviderEchoRefExt::mAudioALSACaptureDataProviderEchoRefExt = NULL;
AudioALSACaptureDataProviderEchoRefExt *AudioALSACaptureDataProviderEchoRefExt::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderEchoRefExt == NULL) {
        mAudioALSACaptureDataProviderEchoRefExt = new AudioALSACaptureDataProviderEchoRefExt();
    }
    ASSERT(mAudioALSACaptureDataProviderEchoRefExt != NULL);
    return mAudioALSACaptureDataProviderEchoRefExt;
}

AudioALSACaptureDataProviderEchoRefExt::AudioALSACaptureDataProviderEchoRefExt():
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    mSmartPaController(AudioSmartPaController::getInstance()),
    hReadThread(0) {
    ALOGD("%s()", __FUNCTION__);

    mCaptureDataProviderType = CAPTURE_PROVIDER_ECHOREF_EXT;
    memset(&mNewtime, 0, sizeof(mNewtime));
    memset(&mOldtime, 0, sizeof(mOldtime));
}

AudioALSACaptureDataProviderEchoRefExt::~AudioALSACaptureDataProviderEchoRefExt() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderEchoRefExt::open() {
    ALOGD("%s()", __FUNCTION__);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    ASSERT(mEnable == false);

    AudioALSASampleRateController *pAudioALSASampleRateController = AudioALSASampleRateController::getInstance();
    pAudioALSASampleRateController->setScenarioStatus(PLAYBACK_SCENARIO_ECHO_REF_EXT);

    // config attribute (will used in client SRC/Enh/... later) // TODO(Sam): query the mConfig?
    if (!AudioSmartPaController::getInstance()->isInCalibration()) {
        mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    } else {
        mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
    }
    mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeSource.num_channels = popcount(mStreamAttributeSource.audio_channel_mask);
    mStreamAttributeSource.sample_rate = AudioALSASampleRateController::getInstance()->getPrimaryStreamOutSampleRate();

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
    mConfig.format = pcm_format_from_audio_format(mStreamAttributeSource.audio_format);

    kReadBufferSize = getPeriodBufSize(&mStreamAttributeSource, mlatency);
#if 0 // cause latency issue...... = =a
    kReadBufferSize &= 0xFFFFFFC0; // (DL1)44.1K\20ms data\stereo\2byte\(Align64byte)
#endif
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

    ALOGD("%s(), audio_format = %d, audio_channel_mask=%x, num_channels=%d, sample_rate=%d, buf_total_latency=%dms", __FUNCTION__,
          mStreamAttributeSource.audio_format, mStreamAttributeSource.audio_channel_mask, mStreamAttributeSource.num_channels, mStreamAttributeSource.sample_rate, mStreamAttributeSource.latency);

    ALOGD("%s(), format = %d, channels=%d, rate=%d, period_size=%d, period_count=%d,latency=%d,kReadBufferSize=%d", __FUNCTION__,
          mConfig.format, mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mlatency, kReadBufferSize);

    if(!AudioSmartPaController::getInstance()->isInCalibration()) {
        OpenPCMDump(LOG_TAG);
    } else {
        OpenPCMDump("AudioALSACaptureDataProvider_ivdump");
    }

    btempDebug = AudioSpeechEnhanceInfo::getInstance()->GetDebugStatus();

#if defined(MTK_AUDIO_KS)
    int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture2);
    int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmCapture2);
    if (AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        int i2sIn = mSmartPaController->getI2sInSelect();
        mSmartPaController->setI2sHD(true, i2sIn);

        mApTurnOnSequence = i2sIn == AUDIO_I2S0 ?
                            AUDIO_CTL_I2S0_TO_CAPTURE2 : AUDIO_CTL_I2S2_TO_CAPTURE2;
        AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(mApTurnOnSequence);
    }

    // need to set after query pcm_params_get, since shutdown will clear this state
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "echo_ref_xrun_assert"), 0, 1)) {
        ALOGW("%s(), echo_ref_xrun_assert enable fail", __FUNCTION__);
    }
#else
    int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmI2SAwbCapture);
    int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmI2SAwbCapture);
#endif

    // enable pcm
    ASSERT(mPcm == NULL);
    mPcm = pcm_open(cardIdx, pcmIdx, PCM_IN | PCM_MONOTONIC, &mConfig);
    ASSERT(mPcm != NULL && pcm_is_ready(mPcm) == true);

    mStart = false;
    mReadThreadReady = false;

    // create reading thread
    mEnable = true;
    int ret = pthread_create(&hReadThread, NULL, AudioALSACaptureDataProviderEchoRefExt::readThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}

status_t AudioALSACaptureDataProviderEchoRefExt::close() {
    ALOGD("%s()", __FUNCTION__);

#if defined(MTK_AUDIO_KS)
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "echo_ref_xrun_assert"), 0, 0)) {
        ALOGW("%s(), echo_ref_xrun_assert disable fail", __FUNCTION__);
    }
#endif

    mEnable = false;
    pthread_join(hReadThread, NULL);
    ALOGD("pthread_join hReadThread done");

    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    ClosePCMDump();

    pcm_stop(mPcm);
    pcm_close(mPcm);
    mPcm = NULL;

#if defined(MTK_AUDIO_KS)
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnoffSequenceByName(mApTurnOnSequence);
    if (AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        mSmartPaController->setI2sHD(false, mSmartPaController->getI2sInSelect());
    }
#endif

    AudioALSASampleRateController *pAudioALSASampleRateController = AudioALSASampleRateController::getInstance();
    pAudioALSASampleRateController->resetScenarioStatus(PLAYBACK_SCENARIO_ECHO_REF_EXT);

    return NO_ERROR;
}

void *AudioALSACaptureDataProviderEchoRefExt::readThread(void *arg) {
    status_t ret = NO_ERROR;
    int retval = 0;

    AudioALSACaptureDataProviderEchoRefExt *pDataProvider = static_cast<AudioALSACaptureDataProviderEchoRefExt *>(arg);

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

        retval = pcm_read(pDataProvider->mPcm, linear_buffer, kReadBufferSize);
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
#if 0   //for check the echoref data got
        pDataProvider->provideCaptureDataToAllClients(open_index);
#else
        if(AudioSmartPaController::getInstance()->isInCalibration() ||
           pDataProvider->mStreamAttributeSource.input_source == AUDIO_SOURCE_ECHO_REFERENCE) {
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

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_exit(NULL);
    return NULL;
}

} // end of namespace android

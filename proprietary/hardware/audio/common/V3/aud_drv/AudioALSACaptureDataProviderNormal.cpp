#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureDataProviderNormal"

#include "AudioALSACaptureDataProviderNormal.h"

#include <pthread.h>
#include <sys/prctl.h>

#include "AudioALSADriverUtility.h"
#include "AudioType.h"
#include "AudioSpeechEnhanceInfo.h"
#include "AudioALSASpeechPhoneCallController.h"
#include "AudioALSAStreamManager.h"
#include "AudioALSAHardwareResourceManager.h"

#if defined(MTK_AUDIO_KS)
#include "AudioALSADeviceConfigManager.h"
#endif

#ifdef MTK_LATENCY_DETECT_PULSE
#include "AudioDetectPulse.h"
#endif

#ifdef MTK_AUDIODSP_SUPPORT
#include "AudioDspStreamManager.h"
#include <audio_task.h>
#include <AudioALSAHandlerKtv.h>
#endif

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)


#define AUDIO_CHANNEL_IN_3MIC (AUDIO_CHANNEL_IN_LEFT | AUDIO_CHANNEL_IN_RIGHT | AUDIO_CHANNEL_IN_BACK)
#define AUDIO_CHANNEL_IN_4MIC (AUDIO_CHANNEL_IN_LEFT | AUDIO_CHANNEL_IN_RIGHT | AUDIO_CHANNEL_IN_BACK) // ch4 == ch3

#define UPLINK_SET_AMICDCC_BUFFER_TIME_MS 80

namespace android {


/*==============================================================================
 *                     Constant
 *============================================================================*/


static uint32_t kReadBufferSize = 0;
static const uint32_t kDCRReadBufferSize = 0x2EE00; //48K\stereo\1s data , calculate 1time/sec

//static FILE *pDCCalFile = NULL;
static bool btempDebug = false;



/*==============================================================================
 *                     Utility
 *============================================================================*/

#define LINEAR_4CH_TO_3CH(linear_buf, data_size, type) \
    ({ \
        uint32_t __channel_size = (data_size >> 2); \
        uint32_t __num_sample = __channel_size / sizeof(type); \
        uint32_t __data_size_3ch = __channel_size * 3; \
        type    *__linear_4ch = (type *)(linear_buf); \
        type    *__linear_3ch = (type *)(linear_buf); \
        uint32_t __idx_sample = 0; \
        for (__idx_sample = 0; __idx_sample < __num_sample; __idx_sample++) { \
            memmove(__linear_3ch, __linear_4ch, 3 * sizeof(type)); \
            __linear_3ch += 3; \
            __linear_4ch += 4; \
        } \
        __data_size_3ch; \
    })


static uint32_t doDownMixFrom4chTo3ch(void *linear_buf, uint32_t data_size, uint32_t audio_format) {
    if (audio_format == AUDIO_FORMAT_PCM_16_BIT) {
        return LINEAR_4CH_TO_3CH(linear_buf, data_size, int16_t);
    } else if (audio_format == AUDIO_FORMAT_PCM_32_BIT ||
               audio_format == AUDIO_FORMAT_PCM_8_24_BIT) {
        return LINEAR_4CH_TO_3CH(linear_buf, data_size, int32_t);
    } else {
        return 0;
    }
}


/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderNormal *AudioALSACaptureDataProviderNormal::mAudioALSACaptureDataProviderNormal = NULL;
AudioALSACaptureDataProviderNormal *AudioALSACaptureDataProviderNormal::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderNormal == NULL) {
        mAudioALSACaptureDataProviderNormal = new AudioALSACaptureDataProviderNormal();
    }
    ASSERT(mAudioALSACaptureDataProviderNormal != NULL);
    return mAudioALSACaptureDataProviderNormal;
}

AudioALSACaptureDataProviderNormal::AudioALSACaptureDataProviderNormal():
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    hReadThread(0),
    mCaptureDropSize(0) {
    ALOGD("%s()", __FUNCTION__);

    mCaptureDataProviderType = CAPTURE_PROVIDER_NORMAL;
    memset(&mNewtime, 0, sizeof(mNewtime));
    memset(&mOldtime, 0, sizeof(mOldtime));
    memset(timerec, 0, sizeof(timerec));
}

AudioALSACaptureDataProviderNormal::~AudioALSACaptureDataProviderNormal() {
    ALOGV("%s()", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderNormal::open() {

    ALOGV("%s(+)", __FUNCTION__);

    ASSERT(mEnable == false);

#if defined(MTK_AUDIO_KS)
    int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture1);
    int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmCapture1);
#else
    int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmUl1Capture);
    int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmUl1Capture);
#endif
    bool audiomode = AudioALSAStreamManager::getInstance()->isPhoneCallOpen(); // TODO: should not use AudioALSAStreamManager.......
    ALOGD("%s(+), audiomode=%d, cardindex = %d, pcmindex = %d", __FUNCTION__, audiomode, cardindex, pcmindex);

    struct pcm_params *params;
    params = pcm_params_get(cardindex, pcmindex,  PCM_IN);
    if (params == NULL) {
        ALOGD("Device does not exist.\n");
    }

    {
        AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());
        unsigned int buffersizemax = pcm_params_get_max(params, PCM_PARAM_BUFFER_BYTES);
        pcm_params_free(params);
        bool bHifiRecord = AudioSpeechEnhanceInfo::getInstance()->GetHifiRecord();
        //debug++
        btempDebug = AudioSpeechEnhanceInfo::getInstance()->GetDebugStatus();
        ALOGD("buffersizemax: %d, bHifiRecord: %d, btempDebug: %d", buffersizemax, bHifiRecord, btempDebug);
         //debug--


#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
        mConfig.channels = 2; // non aurisys... use 2ch
#else
        if (mStreamAttributeSource.input_source == AUDIO_SOURCE_UNPROCESSED) {
            mConfig.channels = 1;
        } else if (mStreamAttributeSource.input_device == AUDIO_DEVICE_IN_BUILTIN_MIC) {
            mConfig.channels = AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport();

            switch (mConfig.channels) {
            case 1:
            case 2:
                break;
            case 3:
                ALOGV("alsa not support 3ch... use 4ch"); // ch4 is same as ch3
                mConfig.channels = 4;
                break;
            case 4:
                break;
            default:
                ALOGW("mConfig.channels = %d!! use 2ch", mConfig.channels);
                mConfig.channels = 2;
            }
        } else if (mStreamAttributeSource.input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
            switch (AudioALSAHardwareResourceManager::getInstance()->getNumOfHeadsetPole()) {
                case 4:
                    mConfig.channels = 1;
                    break;
                case 5:
                    mConfig.channels = 2;
                    break;
                default:
                    mConfig.channels = 1;
                    ALOGE("%s(), can't found matched pole number, use 1 ch", __FUNCTION__);
                    break;
            }
        } else {
            ALOGW("device 0x%x not support... use 2ch", mStreamAttributeSource.input_device);
            mConfig.channels = 2;
        }
#endif // end of MTK_AURISYS_FRAMEWORK_SUPPORT

        mConfig.rate = 48000;

        mlatency = UPLINK_NORMAL_LATENCY_MS ; //20ms
#ifdef UPLINK_LOW_LATENCY
        if (HasLowLatencyCapture()) { mlatency = UPLINK_LOW_LATENCY_MS; }
#endif

        if (bHifiRecord == true) {
            mConfig.rate = 96000;
        }

#ifdef MTK_DMIC_SR_LIMIT
        if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC)) {
            if (mStreamAttributeSource.input_device != AUDIO_DEVICE_IN_WIRED_HEADSET) {
                mConfig.rate = 32000;
            }
        }
#endif

        if (audiomode == true) {
        //if not BT phonecall
            if (!(audio_is_bluetooth_sco_device(mStreamAttributeSource.output_devices))) {
                uint32_t sampleRate = AudioALSASpeechPhoneCallController::getInstance()->getSampleRate();
                if (sampleRate != 0) {
                    mConfig.rate = sampleRate;
                    ALOGD("Phone call mode active, change smaple rate: audiomode=%d, mConfig.rate=%d", audiomode, mConfig.rate);
                }
            }
        }

#ifdef RECORD_INPUT_24BITS
#if defined(MTK_AUDIO_KS)
        mConfig.format = PCM_FORMAT_S24_LE;
#else
        mConfig.format = PCM_FORMAT_S32_LE;
#endif
        mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
#else
        mConfig.format = PCM_FORMAT_S16_LE;
        mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_16_BIT;
#endif

    // config attribute (will used in client SRC/Enh/... later)
        switch (mConfig.channels) {
        case 1:
            mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_MONO;
            break;
        case 2:
            mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
            break;
        case 3:
            mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_3MIC;
            break;
        case 4:
            mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_4MIC;
            break;
        default:
            mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
        }

        mStreamAttributeSource.num_channels = mConfig.channels;
        mStreamAttributeSource.sample_rate = mConfig.rate;  //48000;
#ifdef MTK_DMIC_SR_LIMIT
        mStreamAttributeSource.sample_rate = mConfig.rate;  //32000;
#endif

    // Reset frames readed counter
        mStreamAttributeSource.Time_Info.total_frames_readed = 0;

        kReadBufferSize = getPeriodBufSize(&mStreamAttributeSource, mlatency);
#if 0 // cause latency issue...... = =a
        kReadBufferSize &= 0xFFFFFFC0; // (UL)48K\5ms data\stereo\4byte\(Align64byte)
#endif

        // should set mStreamAttributeSource.num_channels after getPeriodBufSize
        if (mConfig.channels == 4) {
            ALOGV("src not support 4ch -> 3ch... use 3ch");
            mStreamAttributeSource.num_channels = 3;
        }

#if defined(MTK_AUDIO_KS)
        if (isNeedSyncPcmStart() == true) {
           mConfig.period_count = 8;
        } else {
           //determine period count by querying SRAM size in KS
           uint32_t checkSramSize;
           checkSramSize = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "sram_size"), 0);
           ALOGW("check sram size = %d", checkSramSize);
           mConfig.period_count = checkSramSize / kReadBufferSize;
        }
#else
        mConfig.period_count = 4;
        if (isNeedSyncPcmStart() == true) {
           mConfig.period_count = 6;
        }
        if (mlatency == UPLINK_LOW_LATENCY_MS) {
           mConfig.period_count = 40 / UPLINK_LOW_LATENCY_MS; // latency/count : 1/40, 2/20, 3/13, 5/8;
        }
#endif

#ifdef UPLINK_LOW_LATENCY
        mConfig.period_size = kReadBufferSize / mConfig.channels / (pcm_format_to_bits(mConfig.format) / 8);  //period size will impact the interrupt interval
#else
        mConfig.period_size = (buffersizemax / mConfig.channels / (pcm_format_to_bits(mConfig.format) / 8) / mConfig.period_count);
#endif

#if !defined(MTK_AUDIO_KS)
        // If the period count * period size is out of bound, update period count.
        if (buffersizemax < mConfig.period_size * mConfig.period_count * mConfig.channels * (pcm_format_to_bits(mConfig.format) / 8)) {
            mConfig.period_count = buffersizemax / (mConfig.period_size * mConfig.channels * (pcm_format_to_bits(mConfig.format) / 8));
        }
#endif

        mStreamAttributeSource.buffer_size = mConfig.period_size * mConfig.period_count * mConfig.channels *
                                             (pcm_format_to_bits(mConfig.format) / 8);

        mConfig.start_threshold = 0;
#if defined(MTK_AUDIO_KS)
        mConfig.stop_threshold = mConfig.period_size * mConfig.period_count;
#else
        mConfig.stop_threshold = 0;
#endif
        mConfig.silence_threshold = 0;

        OpenPCMDump(LOG_TAG);

        ALOGD("mConfig format: %d, channels: %d, rate: %d, period_size: %d, period_count: %d, latency: %d, kReadBufferSize: %u, mCaptureDropSize: %u",
              mConfig.format, mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mlatency, kReadBufferSize, mCaptureDropSize);

#if defined(MTK_AUDIO_KS)
        mApTurnOnSequence = AudioALSAHardwareResourceManager::getInstance()->getMicRecordPath(mConfig.channels);
        AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(mApTurnOnSequence);

        // set pmic before pcm prepare
        enablePmicInputDevice(true);

#if defined(MTK_AUDIO_KS)
        // need to set after query pcm_params_get, since shutdown will clear this state
        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "record_xrun_assert"), 0, 0)) {
            ALOGW("%s(), record_xrun_assert enable fail", __FUNCTION__);
        }
#endif
#endif

        // enable pcm
        ASSERT(mPcm == NULL);
#if defined(CAPTURE_MMAP) // must be after pcm open
        unsigned int flag = PCM_IN | PCM_MONOTONIC | PCM_MMAP;
        openPcmDriverWithFlag(pcmindex, flag);
#else
        mPcm = pcm_open(cardindex, pcmindex, PCM_IN | PCM_MONOTONIC, &mConfig);
#endif
        ASSERT(mPcm != NULL && pcm_is_ready(mPcm) == true);
        ALOGV("%s(), mPcm = %p", __FUNCTION__, mPcm);

        int prepare_error = pcm_prepare(mPcm);
        if (prepare_error != 0) {
            ASSERT(0);
            pcm_close(mPcm);
            mPcm = NULL;
            return UNKNOWN_ERROR;
        }
#if !defined(MTK_AUDIO_KS)
        // Need to enable PMIC device before AFE to reduce mute data in the beginning
        enablePmicInputDevice(true);
#endif
        mStart = false;
        mReadThreadReady = false;
    }
    // create reading thread
    mEnable = true;
    int ret = pthread_create(&hReadThread, NULL, AudioALSACaptureDataProviderNormal::readThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create hReadThread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

#ifdef MTK_AUDIODSP_SUPPORT
    if (mStreamAttributeSource.input_device == AUDIO_DEVICE_IN_WIRED_HEADSET &&
        AudioALSAHandlerKtv::getInstance()->getTaskKtvStatus() &&
        (mStreamAttributeSource.mVoIPEnable != true)) {
        AudioALSAHandlerKtv::getInstance()->startKtvTask(&mStreamAttributeSource);
    }
#endif

    ALOGD("%s(-)", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureDataProviderNormal::close() {
    ALOGD("%s()", __FUNCTION__);

#ifdef MTK_AUDIODSP_SUPPORT
    if (mStreamAttributeSource.input_device == AUDIO_DEVICE_IN_WIRED_HEADSET &&
        AudioDspStreamManager::getInstance()->getDspRuntimeEn(TASK_SCENE_KTV)) {
        AudioALSAHandlerKtv::getInstance()->stopKtvTask();
    }
#endif

#if defined(MTK_AUDIO_KS)
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "record_xrun_assert"), 0, 0)) {
        ALOGW("%s(), record_xrun_assert disable fail", __FUNCTION__);
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
#endif

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

void *AudioALSACaptureDataProviderNormal::readThread(void *arg) {
    status_t retval = NO_ERROR;
    int ret = 0;
    uint32_t counter = 1;

    AudioALSACaptureDataProviderNormal *pDataProvider = static_cast<AudioALSACaptureDataProviderNormal *>(arg);

    uint32_t open_index = pDataProvider->mOpenIndex;

    char nameset[32];
    sprintf(nameset, "%s%d", __FUNCTION__, pDataProvider->mCaptureDataProviderType);
    prctl(PR_SET_NAME, (unsigned long)nameset, 0, 0, 0);
    pDataProvider->setThreadPriority();

    ALOGD("+%s(), pid: %d, tid: %d, kReadBufferSize=0x%x, open_index=%d, UPLINK_SET_AMICDCC_BUFFER_TIME_MS = %d, counter=%d ", __FUNCTION__, getpid(), gettid(), kReadBufferSize, open_index, UPLINK_SET_AMICDCC_BUFFER_TIME_MS, counter);

    pDataProvider->waitPcmStart();

    // read raw data from alsa driver
    char linear_buffer[kReadBufferSize];
    uint32_t Read_Size = kReadBufferSize;
    uint32_t kReadBufferSize_new;
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

        ret = pDataProvider->pcmRead(pDataProvider->mPcm, linear_buffer, kReadBufferSize);
        if (ret != 0) {
            ALOGE("%s(), pcm_read() error, retval = %d", __FUNCTION__, retval);
            clock_gettime(CLOCK_REALTIME, &pDataProvider->mOldtime);
            continue;
        }

        //struct timespec tempTimeStamp;
        retval = pDataProvider->GetCaptureTimeStamp(&pDataProvider->mStreamAttributeSource.Time_Info, kReadBufferSize);
#if 0
        if (retval != NO_ERROR) {
            clock_gettime(CLOCK_REALTIME, &pDataProvider->mOldtime);
            continue;
        }
#endif

        if (btempDebug) {
            clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
            pDataProvider->timerec[1] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
            pDataProvider->mOldtime = pDataProvider->mNewtime;
        }

#ifdef MTK_LATENCY_DETECT_PULSE
        stream_attribute_t *attribute = &(pDataProvider->mStreamAttributeSource);
        AudioDetectPulse::doDetectPulse(TAG_CAPTURE_DATA_PROVIDER, PULSE_LEVEL, 0, (void *)linear_buffer,
                                        kReadBufferSize, attribute->audio_format, attribute->num_channels,
                                        attribute->sample_rate);
#endif

        if (pDataProvider->mConfig.channels == 4 && pDataProvider->mPCMDumpFile4ch) {
            AudioDumpPCMData(linear_buffer, kReadBufferSize, pDataProvider->mPCMDumpFile4ch);
        }

        // 4ch to 3ch
        if (pDataProvider->mConfig.channels == 4 &&
            pDataProvider->mStreamAttributeSource.num_channels == 3) {
            Read_Size = doDownMixFrom4chTo3ch(
                            linear_buffer,
                            kReadBufferSize,
                            pDataProvider->mStreamAttributeSource.audio_format);
        } else {
            Read_Size = kReadBufferSize;
        }

        // Adjust AMIC 3DB Corner clock setting
        if (counter <= (UPLINK_SET_AMICDCC_BUFFER_TIME_MS / pDataProvider->mlatency)) {
            if (counter == (UPLINK_SET_AMICDCC_BUFFER_TIME_MS / pDataProvider->mlatency)) {
                pDataProvider->adjustSpike();
            }
            counter++;
        }

        // use ringbuf format to save buffer info
        pDataProvider->mPcmReadBuf.pBufBase = linear_buffer;
        pDataProvider->mPcmReadBuf.bufLen   = Read_Size + 1; // +1: avoid pRead == pWrite
        pDataProvider->mPcmReadBuf.pRead    = linear_buffer;
        pDataProvider->mPcmReadBuf.pWrite   = linear_buffer + Read_Size;
        pDataProvider->provideCaptureDataToAllClients(open_index);

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

void  AudioALSACaptureDataProviderNormal::adjustSpike()
{
    status_t retval = mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mAudioALSACaptureDataProviderNormal->mMixer, "Audio_AMIC_DCC_Setting"), "On");
    if (retval != 0) {
        ALOGV("%s(), Can not find Audio_AMIC_DCC_Setting!", __FUNCTION__);
    }
}

} // end of namespace android

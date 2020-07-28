#include "AudioALSACaptureDataClientAurisysNormal.h"

#include <inttypes.h>
#include <pthread.h>
#include <sys/prctl.h>

#include <utils/threads.h> /*  for ANDROID_PRIORITY_AUDIO */


#include "AudioUtility.h"
#include "SpeechUtility.h"


#include "AudioType.h"
#include <AudioLock.h>

#include "AudioALSACaptureDataProviderBase.h"
#include "AudioALSAHardwareResourceManager.h"

#include <SpeechEnhancementController.h>

#include "AudioVolumeInterface.h"
#include "AudioVolumeFactory.h"


#include <audio_memory_control.h>
#include <audio_lock.h>
#include <audio_ringbuf.h>
#include <audio_time.h>


#include <audio_task.h>
#include <aurisys_scenario.h>

#include <arsi_type.h>
#include <aurisys_config.h>

#include <audio_pool_buf_handler.h>

#include <aurisys_utility.h>
#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>

#ifdef MTK_LATENCY_DETECT_PULSE
#include "AudioDetectPulse.h"
#endif

#include "AudioSmartPaController.h"

#include "AudioParamParser.h"

//Android Native Preprocess effect +++
#include "AudioPreProcess.h"
//Android Native Preprocess effect ---

//#undef ALOGV
//#define ALOGV ALOGD


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSACaptureDataClientAurisysNormal"


#ifdef AURISYS_ENABLE_LATENCY_DEBUG
#ifdef LATENCY_LOG
#undef LATENCY_LOG
#endif
#define LATENCY_LOG(x...) ALOGW(x)
#else
#define LATENCY_LOG(x...)
#endif

#define UL_LOCK_MS(al, ms) \
    ({ \
        int __RET__ = 0; \
        if (getNeedAEETimeoutFlg()) { \
            __RET__ = AL_LOCK_MS(al, ms); \
        } else { \
            __RET__ = AL_LOCK_MS_NO_ASSERT(al, ms); \
        } \
        __RET__; \
    })


#ifdef AURISYS_DUMP_LOG_V
#undef  AUD_LOG_V
#define AUD_LOG_V(x...) AUD_LOG_D(x)
#endif



namespace android {

/*==============================================================================
 *                     Constant
 *============================================================================*/

static AudioLock mAurisysLibManagerLock;


#define MTK_STREAMIN_VOLUEM_MAX (0x1000)
#define MTK_STREAMIN_VOLUME_VALID_BIT (12)

#ifndef UPLINK_DROP_POP_MS
#define DROP_POP_MS (60)
#else
#define DROP_POP_MS (UPLINK_DROP_POP_MS)
#endif

#ifndef UPLINK_DROP_POP_MS_FOR_UNPROCESSED
#define DROP_POP_MS_FOR_UNPROCESSED (80)
#else
#define DROP_POP_MS_FOR_UNPROCESSED (UPLINK_DROP_POP_MS_FOR_UNPROCESSED)
#endif

#define MAX_LOCK_TIME_OUT_MS (500)
#define MAX_PROCESS_DATA_WAIT_TIME_OUT_MS (40)

#ifndef MAX_RAW_DATA_LOCK_TIME_OUT_MS
#ifdef CONFIG_MT_ENG_BUILD
#define MAX_RAW_DATA_LOCK_TIME_OUT_MS (500 * 3)
#else
#define MAX_RAW_DATA_LOCK_TIME_OUT_MS (500)
#endif
#endif

#ifndef MAX_PROCESS_DATA_LOCK_TIME_OUT_MS
#ifdef CONFIG_MT_ENG_BUILD
#define MAX_PROCESS_DATA_LOCK_TIME_OUT_MS (500 * 3)
#else
#define MAX_PROCESS_DATA_LOCK_TIME_OUT_MS (500)
#endif
#endif


/*==============================================================================
 *                     Utility
 *============================================================================*/

inline bool isTimeStampStarted(const struct timespec *time_stamp) {
    return (time_stamp->tv_sec != 0 || time_stamp->tv_nsec != 0);
}


static int calculateTotalTimeSpec(const time_info_struct_t *timeInfo,
                                  struct timespec *time_stamp) {
    long ret_ns = 0;

    if (timeInfo == NULL || time_stamp == NULL) {
        ASSERT(timeInfo != NULL);
        ASSERT(time_stamp != NULL);
        return -1;
    }

    if (isTimeStampStarted(&timeInfo->timestamp_get) == false) {
        ALOGE("%s() timestamp_get not start!!", __FUNCTION__);
        return -1;
    }

    time_stamp->tv_sec = timeInfo->timestamp_get.tv_sec;
    time_stamp->tv_nsec = timeInfo->timestamp_get.tv_nsec;

    ret_ns = timeInfo->kernelbuffer_ns;
    if (time_stamp->tv_nsec >= ret_ns) {
        time_stamp->tv_nsec -= ret_ns;
    } else {
        time_stamp->tv_sec -= 1;
        time_stamp->tv_nsec = 1000000000 + time_stamp->tv_nsec - ret_ns;
    }
    ALOGV("%s(), sec: %ld, nsec: %ld, ret_ns: %ld", __FUNCTION__,
          time_stamp->tv_sec, time_stamp->tv_nsec, ret_ns);

    return 0;
}


static uint32_t getDiffBufSize(const stream_attribute_t *attribute, uint64_t time_diff_ns) {
    uint64_t diff_buf_size = 0;

    uint64_t size_per_sample = 0;
    uint64_t size_per_frame = 0;


    size_per_sample = audio_bytes_per_sample(attribute->audio_format);
    size_per_frame  = attribute->num_channels * size_per_sample;

    diff_buf_size = (size_per_frame *
                     attribute->sample_rate *
                     time_diff_ns) / 1000000000;
    if ((diff_buf_size % size_per_frame) != 0) { // alignment
        diff_buf_size = ((diff_buf_size / size_per_frame) + 1) * size_per_frame;
    }

    return (uint32_t)diff_buf_size;
}

static uint32_t getDropMs(stream_attribute_t *attribute_target) {
    if (attribute_target->input_source == AUDIO_SOURCE_UNPROCESSED) {
        return DROP_POP_MS_FOR_UNPROCESSED;
    } else {
        return DROP_POP_MS;
    }
}


#ifdef MTK_LATENCY_DETECT_PULSE
inline void detectPulseForRingbuf(
    const PULSE_TAG tag,
    const audio_ringbuf_t *ringbuf,
    const stream_attribute_t *attribute) {

    uint32_t data_count = 0;
    char *linear_buf = NULL;
    bool malloc_flag = false;
    uint32_t r2e = 0;


    if (ringbuf == NULL || attribute == NULL) {
        ALOGW("%s(), NULL!!", __FUNCTION__);
        return;
    }

    data_count = audio_ringbuf_count(ringbuf);
    if (data_count == 0) {
        ALOGW("%s(), data_count = 0!!", __FUNCTION__);
        return;
    }

    if (ringbuf->write >= ringbuf->read) {
        linear_buf = ringbuf->read;
    } else {
        malloc_flag = true;
        linear_buf = (char *)malloc(data_count);
        if(!linear_buf) {
            ALOGE("%s(): malloc fail!", __FUNCTION__);
            return;
        }

        r2e = (ringbuf->base + ringbuf->size) - ringbuf->read;
        memcpy(linear_buf, ringbuf->read, r2e);
        if (data_count > r2e) {
            memcpy(linear_buf + r2e, ringbuf->base, data_count - r2e);
        }
    }

    AudioDetectPulse::doDetectPulse(
        tag,
        PULSE_LEVEL,
        0,
        (void *)linear_buf,
        data_count,
        attribute->audio_format,
        attribute->num_channels,
        attribute->sample_rate);

    if (malloc_flag) {
        free(linear_buf);
    }
}
#endif


/*==============================================================================
 *                     Implementation
 *============================================================================*/
AudioALSACaptureDataClientAurisysNormal::AudioALSACaptureDataClientAurisysNormal(
    AudioALSACaptureDataProviderBase *pCaptureDataProvider,
    stream_attribute_t *stream_attribute_target,
    AudioALSACaptureDataProviderBase *pCaptureDataProviderEchoRef) :
    mStreamAttributeSource(NULL),
    mStreamAttributeTarget(stream_attribute_target),
    mCaptureDataProvider(pCaptureDataProvider),
    mRawStartFrameCount(0),
    mMicMute(false),
    mMuteTransition(false),
    mAudioALSAVolumeController(AudioVolumeFactory::CreateAudioVolumeController()),
    mEnable(false),
    mStreamInReopen(false),
    mProcessThreadLaunched(false),
    mLatency(0),
    mRawDataPeriodBufSize(0),
    mProcessedDataPeriodBufSize(0),
    mCaptureDataProviderEchoRef(pCaptureDataProviderEchoRef),
    mStreamAttributeSourceEchoRef(NULL),
    mEchoRefDataPeriodBufSize(0),
    mIsEchoRefDataSync(false),
    mAurisysLibManager(NULL),
    mManagerConfig(NULL),
    mAurisysScenario(AURISYS_SCENARIO_INVALID),
    mAudioPoolBufUlIn(NULL),
    mAudioPoolBufUlOut(NULL),
    mAudioPoolBufUlAec(NULL),
    mAudioPoolBufDlIn(NULL),
    mAudioPoolBufDlOut(NULL),
    mLinearOut(NULL),
    mDropPopSize(0),
    mAudioPreProcessEffect(NULL) {

    // init var
    uint32_t drop_ms = 0;
    uint32_t size_per_sample = 0;
    uint32_t size_per_frame = 0;
    int ret = 0;


    ALOGD("%s(+)", __FUNCTION__);


    // init raw data buf
    memset((void *)&mRawDataBufTimeStamp, 0, sizeof(mRawDataBufTimeStamp));
    memset((void *)&mRawDataBuf, 0, sizeof(mRawDataBuf));

    // init processed data buf
    memset((void *)&mProcessedDataBuf, 0, sizeof(mProcessedDataBuf));

    // init echo ref data buf
    memset((void *)&mEchoRefDataBufTimeStamp, 0, sizeof(mEchoRefDataBufTimeStamp));
    memset((void *)&mEchoRefDataBuf, 0, sizeof(mEchoRefDataBuf));

    // config attribute for input device
    mCaptureDataProvider->configStreamAttribute(mStreamAttributeTarget);

    // get attribute before enable
    mStreamAttributeSource = mCaptureDataProvider->getStreamAttributeSource();
    if (IsAECEnable()) {
        mStreamAttributeSourceEchoRef = mCaptureDataProviderEchoRef->getStreamAttributeSource();
    }

    // enable before attach
    mEnable = true;

    // attach client to capture data provider (after data buf ready)
    mCaptureDataProvider->attach(this);
    if ((mStreamAttributeSource->input_device == AUDIO_DEVICE_IN_USB_DEVICE) && (getPcmStatus() != true)) {
        hProcessThread = 0;
        ALOGD("%s, PCM Open/Read Fail...USB Device is unplugged ?", __FUNCTION__);
        return;
    }
    if (IsAECEnable()) {
        mCaptureDataProviderEchoRef->configStreamAttribute(mStreamAttributeTarget);
        mCaptureDataProviderEchoRef->attach(this);
    }

    // get latency (for library, but not data provider)
    mLatency = (IsLowLatencyCapture()) ? UPLINK_LOW_LATENCY_MS : UPLINK_NORMAL_LATENCY_MS;

    // get period size
    mRawDataPeriodBufSize = getPeriodBufSize(mStreamAttributeSource, mLatency);
    mProcessedDataPeriodBufSize = getPeriodBufSize(mStreamAttributeTarget, mLatency);
    if (IsAECEnable()) {
        mEchoRefDataPeriodBufSize = getPeriodBufSize(mStreamAttributeSourceEchoRef, mLatency);
    }
    ALOGD("mLatency %u, mRawDataPeriodBufSize %u, mProcessedDataPeriodBufSize %u, mEchoRefDataPeriodBufSize %u, stream_attribute_target->audio_format = %d mStreamAttributeSource->audio_format = %d",
          mLatency, mRawDataPeriodBufSize, mProcessedDataPeriodBufSize, mEchoRefDataPeriodBufSize, stream_attribute_target->audio_format, mStreamAttributeSource->audio_format);

    // pcm start if need
    if (isNeedSyncPcmStart() == true) {
        if (IsAECEnable() == false) {
            while (mCaptureDataProvider->getReadThreadReady() == false) {
                usleep(2 * 1000);
            }
            ALOGD("isNeedSyncPcmStart, read thread ready! pcm start");
            mCaptureDataProvider->signalPcmStart();
        } else {
            while (mCaptureDataProvider->getReadThreadReady() == false ||
                   mCaptureDataProviderEchoRef->getReadThreadReady() == false) {
                usleep(2 * 1000);
            }
            ALOGD("isNeedSyncPcmStart, read thread ready! pcm start");
            mCaptureDataProvider->signalPcmStart();
            mCaptureDataProviderEchoRef->signalPcmStart();
        }
    }


    // Gain control // TODO: AudioMTKGainController::SetCaptureGain, multi data client !?
    AUD_ASSERT(mAudioALSAVolumeController != NULL);
    if (mAudioALSAVolumeController != NULL) {
        mAudioALSAVolumeController->SetCaptureGain(mStreamAttributeTarget->audio_mode,
                                                   mStreamAttributeTarget->input_source,
                                                   mStreamAttributeTarget->input_device,
                                                   mStreamAttributeTarget->output_devices);
    }

    AUDIO_ALLOC_STRUCT(struct data_buf_t, mLinearOut);

    // create lib manager
    CreateAurisysLibManager();

    //Android Native Preprocess effect +++
    mAudioPreProcessEffect = new AudioPreProcess(mStreamAttributeTarget); // use the same attribute as stream in
    ASSERT(mAudioPreProcessEffect != NULL);
    CheckNativeEffect();
    //Android Native Preprocess effect ---


    // depop
    drop_ms = getDropMs(mStreamAttributeTarget);
    if ((drop_ms % mLatency) != 0) { // drop data size need to align interrupt rate
        drop_ms = ((drop_ms / mLatency) + 1) * mLatency; // cell()
    }
    mDropPopSize = (audio_bytes_per_sample(mStreamAttributeTarget->audio_format) *
                    mStreamAttributeTarget->num_channels *
                    mStreamAttributeTarget->sample_rate *
                    drop_ms) / 1000;

    size_per_sample = audio_bytes_per_sample(mStreamAttributeTarget->audio_format);
    size_per_frame  = mStreamAttributeTarget->num_channels * size_per_sample;
    if ((mDropPopSize % size_per_frame) != 0) { // alignment
        mDropPopSize = ((mDropPopSize / size_per_frame) + 1) * size_per_frame;
    }
    ASSERT(mDropPopSize >= mProcessedDataPeriodBufSize);


    // processThread
    hProcessThread = 0;
    ret = pthread_create(&hProcessThread, NULL,
                         AudioALSACaptureDataClientAurisysNormal::processThread,
                         (void *)this);
    ASSERT(ret == 0);
    ALOGD("%s(-), drop_ms = %d", __FUNCTION__, drop_ms);
}


AudioALSACaptureDataClientAurisysNormal::~AudioALSACaptureDataClientAurisysNormal() {
    // start to close
    mEnable = false;

    // terminate processThread thread
    pthread_join(hProcessThread, NULL);
    ALOGD("pthread_join hProcessThread done");

    // detach client to capture data provider
    mCaptureDataProvider->detach(this);
    ALOGV("mCaptureDataProvider detach done");

    if (IsAECEnable()) {
        mCaptureDataProviderEchoRef->detach(this);
        ALOGD("mCaptureDataProviderEchoRef detach done");
    }



    // close
    AL_LOCK_MS(mRawDataBufLock, MAX_LOCK_TIME_OUT_MS);
    AL_LOCK_MS(mProcessedDataBufLock, MAX_LOCK_TIME_OUT_MS);


    //Android Native Preprocess effect +++
    if (mAudioPreProcessEffect != NULL) { delete mAudioPreProcessEffect; }
    //Android Native Preprocess effect ---

    DestroyAurisysLibManager();

    AUDIO_FREE_POINTER(mLinearOut);

    AUDIO_FREE_POINTER(mRawDataBuf.base);
    AUDIO_FREE_POINTER(mProcessedDataBuf.base);
    AUDIO_FREE_POINTER(mEchoRefDataBuf.base);


    AL_UNLOCK(mProcessedDataBufLock);
    AL_UNLOCK(mRawDataBufLock);
}


uint32_t AudioALSACaptureDataClientAurisysNormal::copyCaptureDataToClient(RingBuf pcm_read_buf) {
    audio_ringbuf_t pcm_read_buf_wrap;
    int ret = 0;

    if (mProcessThreadLaunched == false) {
        ALOGD("%s(), mProcessThreadLaunched == false. return", __FUNCTION__);
        return 0;
    }

    if ((mStreamAttributeSource->input_device == AUDIO_DEVICE_IN_USB_DEVICE) && (getPcmStatus() != true)) {
        ALOGD("%s, PCM Open/Read Fail...USB Device is unplugged ?", __FUNCTION__);
        AL_SIGNAL(mRawDataBufLock);
        return 0;
    }
    pcm_read_buf_wrap.base = pcm_read_buf.pBufBase;
    pcm_read_buf_wrap.read = pcm_read_buf.pRead;
    pcm_read_buf_wrap.write = pcm_read_buf.pWrite;
    pcm_read_buf_wrap.size = pcm_read_buf.bufLen;


#ifdef MTK_LATENCY_DETECT_PULSE
    if (AudioDetectPulse::getDetectPulse()) {
        detectPulseForRingbuf(
            TAG_CAPTURE_DATA_CLIENT5,
            &pcm_read_buf_wrap,
            mStreamAttributeSource);
    }
#endif

    ret = UL_LOCK_MS(mRawDataBufLock, MAX_RAW_DATA_LOCK_TIME_OUT_MS);
    if (ret != 0) {
        handleLockTimeout();
    }
    LATENCY_LOG("R+");

    if (mEnable == false) {
        ALOGV("%s(), mEnable == false. return", __FUNCTION__);
        AL_SIGNAL(mRawDataBufLock);
        AL_UNLOCK(mRawDataBufLock);
        return 0;
    }

    if (IsAECEnable() == false) {
        audio_ringbuf_copy_from_ringbuf_all(&mRawDataBuf, &pcm_read_buf_wrap);
        AL_SIGNAL(mRawDataBufLock);
    } else {
        if (mIsEchoRefDataSync == false) {
            syncEchoRefData(DATA_BUF_UPLINK_IN, &pcm_read_buf_wrap);
        } else {
            audio_ringbuf_copy_from_ringbuf_all(&mRawDataBuf, &pcm_read_buf_wrap);
        }

        if (mIsEchoRefDataSync == true &&
            audio_ringbuf_count(&mRawDataBuf)     >= mRawDataPeriodBufSize &&
            audio_ringbuf_count(&mEchoRefDataBuf) >= mEchoRefDataPeriodBufSize) {
            AL_SIGNAL(mRawDataBufLock);
        }
    }

    LATENCY_LOG("R-");
    AL_UNLOCK(mRawDataBufLock);
    return 0;
}
bool AudioALSACaptureDataClientAurisysNormal::getPcmStatus(void) {
    return (mCaptureDataProvider->getPcmStatus() == NO_ERROR);
}

void *AudioALSACaptureDataClientAurisysNormal::processThread(void *arg) {
    char thread_name[128] = {0};

    AudioALSACaptureDataClientAurisysNormal *client = NULL;

    audio_ringbuf_t *raw_ul = NULL;
    audio_ringbuf_t *raw_aec = NULL;
    audio_ringbuf_t *processed = NULL;

    aurisys_lib_manager_t *manager = NULL;
    audio_pool_buf_t *ul_in = NULL;
    audio_pool_buf_t *ul_out = NULL;
    audio_pool_buf_t *ul_aec = NULL;

    char *effect_buf = NULL;

    uint32_t frame_idx = 0;

    uint32_t data_count_raw_ul = 0;
    uint32_t data_count_raw_aec = 0;

    uint32_t data_count = 0;
    uint32_t data_count_src = 0;

    int wait_result = 0;
    int ret = 0;

    client = static_cast<AudioALSACaptureDataClientAurisysNormal *>(arg);

    CONFIG_THREAD(thread_name, ANDROID_PRIORITY_AUDIO);
#ifdef UPLINK_LOW_LATENCY
    if (client->IsLowLatencyCapture()) {
        audio_sched_setschedule(0, SCHED_RR, sched_get_priority_min(SCHED_RR));
    }
#endif

    /* process thread created */
    client->mProcessThreadLaunched = true;

    /* get buffer address */
    raw_ul    = &client->mRawDataBuf;
    raw_aec   = &client->mEchoRefDataBuf;
    processed = &client->mProcessedDataBuf;

    manager = client->mAurisysLibManager;
    ul_in = client->mAudioPoolBufUlIn;
    ul_out = client->mAudioPoolBufUlOut;
    ul_aec = client->mAudioPoolBufUlAec;



    while (client->mEnable == true) {
        ALOGV("%s(+)", __FUNCTION__);

        LATENCY_LOG("1.1");
        if ((client->mStreamAttributeSource->input_device == AUDIO_DEVICE_IN_USB_DEVICE) && (client->getPcmStatus() != true)) {
            ALOGD("%s, PCM Open/Read Fail...USB Device is unplugged ?", __FUNCTION__);
            AL_SIGNAL(client->mProcessedDataBufLock);
            usleep(100);
            continue;
        }

        // get data from raw buffer
        ret = UL_LOCK_MS(client->mRawDataBufLock, MAX_RAW_DATA_LOCK_TIME_OUT_MS);
        if (ret != 0) {
            client->handleLockTimeout();
        }

        LATENCY_LOG("S+");
        data_count_raw_ul = audio_ringbuf_count(raw_ul);
        if (client->IsAECEnable()) { data_count_raw_aec = audio_ringbuf_count(raw_aec); }

        // data not reary, wait data
        if ((data_count_raw_ul < client->mRawDataPeriodBufSize) ||
            (client->IsAECEnable() == true &&
             (client->mIsEchoRefDataSync == false ||
              data_count_raw_aec < client->mEchoRefDataPeriodBufSize))) {
            LATENCY_LOG("1.2");
            wait_result = AL_WAIT_MS(client->mRawDataBufLock, MAX_PROCESS_DATA_WAIT_TIME_OUT_MS);
            LATENCY_LOG("S");
            if (wait_result != 0) {
                data_count_raw_ul = audio_ringbuf_count(raw_ul);
                if (client->IsAECEnable()) {
                    data_count_raw_aec = audio_ringbuf_count(raw_aec);
                }

                if (client->IsAECEnable() && ((data_count_raw_ul < client->mRawDataPeriodBufSize) ||
                    (data_count_raw_aec < client->mEchoRefDataPeriodBufSize))) {
                    ALOGW("data_count_raw_ul %u, mRawDataPeriodBufSize %u, data_count_raw_aec %u, mEchoRefDataPeriodBufSize %u",
                           data_count_raw_ul, client->mRawDataPeriodBufSize, data_count_raw_aec, client->mEchoRefDataPeriodBufSize);
                } else if (data_count_raw_ul < client->mRawDataPeriodBufSize) {
                    ALOGW("data_count_raw_ul %u, mRawDataPeriodBufSize %u", data_count_raw_ul, client->mRawDataPeriodBufSize);
                }

                LATENCY_LOG("S-");
                AL_UNLOCK(client->mRawDataBufLock);
                usleep(100);
                continue;
            }
            LATENCY_LOG("1.3");

            if (client->mEnable == false) {
                ALOGV("%s(), record stopped. return", __FUNCTION__);
                LATENCY_LOG("S-");
                AL_UNLOCK(client->mRawDataBufLock);
                break;
            }

            data_count_raw_ul = audio_ringbuf_count(raw_ul);
            ALOGV("data_count_raw_ul %u, mRawDataPeriodBufSize %u",
                  data_count_raw_ul, client->mRawDataPeriodBufSize);

            if (client->IsAECEnable()) {
                data_count_raw_aec = audio_ringbuf_count(raw_aec);
                ALOGV("data_count_raw_aec %u, mEchoRefDataPeriodBufSize %u",
                      data_count_raw_aec, client->mEchoRefDataPeriodBufSize);
            }

            if ((data_count_raw_ul < client->mRawDataPeriodBufSize) ||
                (client->IsAECEnable() == true &&
                 (client->mIsEchoRefDataSync == false ||
                  data_count_raw_aec < client->mEchoRefDataPeriodBufSize))) {
                AL_UNLOCK(client->mRawDataBufLock);
                usleep(100);
                continue;
            }
        }

        LATENCY_LOG("2");

        // make sure data ready
        if (data_count_raw_ul < client->mRawDataPeriodBufSize) {
            ALOGE("data_count_raw_ul %u != mRawDataPeriodBufSize %u",
                  data_count_raw_ul, client->mRawDataPeriodBufSize);
            ASSERT(data_count_raw_ul >= client->mRawDataPeriodBufSize);
        }
        if (client->IsAECEnable()) {
            if (data_count_raw_aec < client->mEchoRefDataPeriodBufSize) {
                ALOGE("data_count_raw_aec %u != mEchoRefDataPeriodBufSize %u",
                      data_count_raw_aec, client->mEchoRefDataPeriodBufSize);
                ASSERT(data_count_raw_aec >= client->mEchoRefDataPeriodBufSize);
            }
        }

        // copy data
        audio_pool_buf_copy_from_ringbuf(ul_in, raw_ul, client->mRawDataPeriodBufSize);
        if (client->IsAECEnable()) {
            audio_pool_buf_copy_from_ringbuf(ul_aec, raw_aec, client->mEchoRefDataPeriodBufSize);
        }
        frame_idx++;
        LATENCY_LOG("S-");
        AL_UNLOCK(client->mRawDataBufLock);
        ALOGV("data_count %u, %dL", audio_ringbuf_count(&ul_in->ringbuf), __LINE__);


        // aurisys processThread
        // TODO: support channel remix in audio_pool_buf_handler.c
#ifdef MTK_LATENCY_DETECT_PULSE
        if (AudioDetectPulse::getDetectPulse()) {
            detectPulseForRingbuf(
                TAG_CAPTURE_DATA_CLIENT4,
                &ul_in->ringbuf,
                client->mStreamAttributeSource);
        }
#endif
        aurisys_process_ul_only(manager,
                                ul_in,
                                ul_out,
                                ul_aec);
        data_count = audio_ringbuf_count(&ul_out->ringbuf);
        ALOGV("data_count %u, %dL", data_count, __LINE__);
        if (data_count == 0) {
            ALOGD("%s(), frame #%d data_count == 0, %dL", __FUNCTION__, frame_idx, __LINE__);
            continue;
        }
        LATENCY_LOG("3");

#ifdef MTK_LATENCY_DETECT_PULSE
        if (AudioDetectPulse::getDetectPulse()) {
            detectPulseForRingbuf(
                TAG_CAPTURE_DATA_CLIENT3,
                &ul_out->ringbuf,
                client->mStreamAttributeTarget);
        }
#endif

        // depop
        if (client->mDropPopSize > 0) {
            ALOGV("data_count %u, mDropPopSize %u, %dL", data_count, client->mDropPopSize, __LINE__);

            if (data_count >= client->mDropPopSize) {
                audio_ringbuf_drop_data(&ul_out->ringbuf, client->mDropPopSize);
                data_count -= client->mDropPopSize;
                client->mDropPopSize = 0;
            } else {
                audio_ringbuf_drop_data(&ul_out->ringbuf, data_count);
                client->mDropPopSize -= data_count;
                data_count = 0;
            }
        }
        if (data_count == 0) {
            ALOGV("%s(), frame #%d data_count == 0, %dL", __FUNCTION__, frame_idx, __LINE__);
            continue;
        }


        //Native Preprocess effect+++
        ALOGV("data_count %u, %dL", data_count, __LINE__);
        audio_pool_buf_copy_to_linear(
            &client->mLinearOut->p_buffer,
            &client->mLinearOut->memory_size,
            ul_out,
            data_count);
        effect_buf = (char *)client->mLinearOut->p_buffer;
        data_count = client->NativePreprocess(effect_buf, data_count);
        ALOGV("data_count %u, %dL", data_count, __LINE__);
        if (data_count == 0) {
            ALOGD("%s(), frame #%d data_count == 0, %dL", __FUNCTION__, frame_idx, __LINE__);
            continue;
        }
        //Native Preprocess effect---


        // copy to processed buf and signal read()
        ret = UL_LOCK_MS(client->mProcessedDataBufLock, MAX_PROCESS_DATA_LOCK_TIME_OUT_MS);
        if (ret != 0) {
            client->handleLockTimeout();
        }

        if (client->mCaptureDataProvider->getCaptureDataProviderType() == CAPTURE_PROVIDER_USB){
            data_count = client->mCaptureDataProvider->doBcvProcess(effect_buf,data_count);
        }
        audio_ringbuf_copy_from_linear(processed, effect_buf, data_count);

#ifdef MTK_LATENCY_DETECT_PULSE
        if (AudioDetectPulse::getDetectPulse()) {
            detectPulseForRingbuf(
                TAG_CAPTURE_DATA_CLIENT2,
                processed,
                client->mStreamAttributeTarget);
        }
#endif

        AL_SIGNAL(client->mProcessedDataBufLock);
        AL_UNLOCK(client->mProcessedDataBufLock);
        LATENCY_LOG("4");

        ALOGV("%s(-)", __FUNCTION__);
    }


    ALOGV("%s terminated", thread_name);
    pthread_exit(NULL);
    return NULL;
}

ssize_t AudioALSACaptureDataClientAurisysNormal::read(void *buffer, ssize_t bytes) {
    ALOGV("%s(+), bytes: %u", __FUNCTION__, (uint32_t)bytes);

    char *write = (char *)buffer;
    uint32_t data_count = 0;
    uint32_t left_count_to_read = bytes;

    int try_count = 8;
    int wait_result = 0;
    int ret = 0;

    uint32_t wait_ms = 0;

    // clean buffer
    memset(buffer, 0, bytes);

    // wait time
    wait_ms = (IsLowLatencyCapture()) ? mLatency * 5 : MAX_PROCESS_DATA_WAIT_TIME_OUT_MS;

    // push clean up handlder for read thread termination
    CLEANUP_PUSH_ALOCK(mProcessedDataBufLock.getAlock());

    // copy processed data
    do {
        ret = UL_LOCK_MS(mProcessedDataBufLock, MAX_PROCESS_DATA_LOCK_TIME_OUT_MS);
        if (ret != 0) {
            handleLockTimeout();
        }
        data_count = audio_ringbuf_count(&mProcessedDataBuf);
        if (data_count == 0) {
            // wait for new data
            wait_result = AL_WAIT_MS(mProcessedDataBufLock, wait_ms);
            if ((mStreamAttributeSource->input_device == AUDIO_DEVICE_IN_USB_DEVICE) && (getPcmStatus() != true)) {
                ALOGD("%s, PCM Open/Read Fail...USB Device is unplugged ?", __FUNCTION__);
                AL_UNLOCK(mProcessedDataBufLock);
                left_count_to_read = bytes;
                break;
            }
            if (wait_result != 0) { // something error, exit
                AL_UNLOCK(mProcessedDataBufLock);
                try_count--;
                usleep(100);
                continue;
            }

            if (mEnable == false) {
                ALOGD("%s(), record stopped. return", __FUNCTION__);
                AL_UNLOCK(mProcessedDataBufLock);
                break;
            }

            data_count = audio_ringbuf_count(&mProcessedDataBuf);
        }

#ifdef MTK_LATENCY_DETECT_PULSE
        if (AudioDetectPulse::getDetectPulse()) {
            detectPulseForRingbuf(
                TAG_CAPTURE_DATA_CLIENT1,
                &mProcessedDataBuf,
                mStreamAttributeTarget);
        }
#endif

        if (data_count >= left_count_to_read) { // ring buffer is enough, copy & exit
            audio_ringbuf_copy_to_linear(write, &mProcessedDataBuf, left_count_to_read);
            AL_UNLOCK(mProcessedDataBufLock);
            left_count_to_read = 0;
            break;
        }

        audio_ringbuf_copy_to_linear((char *)write, &mProcessedDataBuf, data_count);
        AL_UNLOCK(mProcessedDataBufLock);
        left_count_to_read -= data_count;
        write += data_count;

        try_count--;
    } while (left_count_to_read > 0 && try_count > 0 && mEnable == true);

    // pop clean up handlder for read thread termination
    CLEANUP_POP_ALOCK(mProcessedDataBufLock.getAlock());


    if (left_count_to_read > 0) {
        ALOGW("left_count_to_read %d!!", left_count_to_read);
    }

    // apply volume if need
    bytes -= left_count_to_read;
    ApplyVolume(buffer, bytes);

    ALOGV("%s(-), bytes: %u", __FUNCTION__, (uint32_t)bytes);
    return bytes;
}


bool AudioALSACaptureDataClientAurisysNormal::IsLowLatencyCapture(void) {
    bool low_latency_on = false;
    bool voip_on = IsVoIPEnable();
    bool aec_on = IsAECEnable();

#ifdef UPLINK_LOW_LATENCY
    if (voip_on == false && aec_on == false &&
        (mStreamAttributeTarget->mAudioInputFlags & AUDIO_INPUT_FLAG_FAST)) {
        low_latency_on = true;
    }
#endif

    ALOGV("%s(), low_latency_on: %d, voip_on: %d", __FUNCTION__, low_latency_on, voip_on);
    return low_latency_on;
}


int AudioALSACaptureDataClientAurisysNormal::getCapturePosition(int64_t *frames, int64_t *time) {
    if (mCaptureDataProvider == NULL || frames == NULL || time == NULL) {
        return -EINVAL;
    }

    /* Convert provider sample rate to streamin sample rate*/
    int ret = mCaptureDataProvider->getCapturePosition(frames, time);
    *frames = (*frames) * mStreamAttributeTarget->sample_rate / mStreamAttributeSource->sample_rate;

    /* Compensate drop ms to frames */
    uint32 dropMs = getDropMs(mStreamAttributeTarget);
    *frames = (*frames) - ((int64_t)dropMs * (int64_t)mStreamAttributeTarget->sample_rate / 1000);

    ALOGV("%s(), frames = %" PRIu64 ", tar sample = %d, src sample = %d", __FUNCTION__, *frames, mStreamAttributeTarget->sample_rate, mStreamAttributeSource->sample_rate);
    return ret;
}


void AudioALSACaptureDataClientAurisysNormal::AddEchoRefDataProvider(
    AudioALSACaptureDataProviderBase *pCaptureDataProvider,
    stream_attribute_t *stream_attribute_target) {

    if (!pCaptureDataProvider || !stream_attribute_target) {
        ALOGE("%s(), NULL! return", __FUNCTION__);
        return;
    }

    WARNING("INVALID_OPERATION"); /* already added in ctor */
    return;
}


uint32_t AudioALSACaptureDataClientAurisysNormal::copyEchoRefCaptureDataToClient(RingBuf pcm_read_buf) {
    audio_ringbuf_t pcm_read_buf_wrap;

    if (mProcessThreadLaunched == false) {
        ALOGD("%s(), mProcessThreadLaunched == false. return", __FUNCTION__);
        return 0;
    }

    pcm_read_buf_wrap.base = pcm_read_buf.pBufBase;
    pcm_read_buf_wrap.read = pcm_read_buf.pRead;
    pcm_read_buf_wrap.write = pcm_read_buf.pWrite;
    pcm_read_buf_wrap.size = pcm_read_buf.bufLen;

    ASSERT(IsAECEnable());

    AL_LOCK_MS(mRawDataBufLock, MAX_RAW_DATA_LOCK_TIME_OUT_MS);
    LATENCY_LOG("E+");

    if (mEnable == false) {
        ALOGV("%s(), mEnable == false. return", __FUNCTION__);
        AL_SIGNAL(mRawDataBufLock);
        AL_UNLOCK(mRawDataBufLock);
        return 0;
    }

    if (mIsEchoRefDataSync == false) {
        syncEchoRefData(DATA_BUF_ECHO_REF, &pcm_read_buf_wrap);
    } else {
        audio_ringbuf_copy_from_ringbuf_all(&mEchoRefDataBuf, &pcm_read_buf_wrap);
    }

    if (mIsEchoRefDataSync == true &&
        audio_ringbuf_count(&mRawDataBuf)     >= mRawDataPeriodBufSize &&
        audio_ringbuf_count(&mEchoRefDataBuf) >= mEchoRefDataPeriodBufSize) {
        AL_SIGNAL(mRawDataBufLock);
    }

    LATENCY_LOG("E-");
    AL_UNLOCK(mRawDataBufLock);
    return 0;
}


void AudioALSACaptureDataClientAurisysNormal::syncEchoRefData(
    const uint8_t data_buf_type, /* data_buf_type_t */
    audio_ringbuf_t *rb) {

    uint64_t time_diff_ns = 0;
    uint32_t diff_buf_size = 0;

    uint32_t data_count = 0;

    int ret = 0;


    ASSERT(IsAECEnable() == true);
    ASSERT(mIsEchoRefDataSync == false);
    if (IsAECEnable() == false || mIsEchoRefDataSync == true) {
        return;
    }


    if (data_buf_type == DATA_BUF_UPLINK_IN) {
        if (isTimeStampStarted(&mRawDataBufTimeStamp) == false) { // only keep the first timestamp
            ret = calculateTotalTimeSpec(&mStreamAttributeSource->Time_Info, &mRawDataBufTimeStamp);
            if (ret != 0) {
                //ASSERT(ret == 0);
                return;
            }
            ALOGD("%s(UL*),  UL in: %lld.%.9ld, EchoRef: %lld.%.9ld", __FUNCTION__,
                  (long long)mRawDataBufTimeStamp.tv_sec,
                  mRawDataBufTimeStamp.tv_nsec,
                  (long long)mEchoRefDataBufTimeStamp.tv_sec,
                  mEchoRefDataBufTimeStamp.tv_nsec);
        } else {
            ALOGD("%s(UL),   UL in: %lld.%.9ld, EchoRef: %lld.%.9ld", __FUNCTION__,
                  (long long)mStreamAttributeSource->Time_Info.timestamp_get.tv_sec,
                  mStreamAttributeSource->Time_Info.timestamp_get.tv_nsec,
                  (long long)mEchoRefDataBufTimeStamp.tv_sec,
                  mEchoRefDataBufTimeStamp.tv_nsec);
        }

        if (isTimeStampStarted(&mEchoRefDataBufTimeStamp) == false) { // echo ref data not ready
            audio_ringbuf_copy_from_ringbuf_all(&mRawDataBuf, rb);
        } else { // already got echo ref data
            audio_ringbuf_copy_from_ringbuf_all(&mRawDataBuf, rb);

            if (t1_older_then_t2(&mEchoRefDataBufTimeStamp, &mRawDataBufTimeStamp)) { // ul new
                time_diff_ns = get_time_diff_ns(&mEchoRefDataBufTimeStamp, &mRawDataBufTimeStamp);
                diff_buf_size = getDiffBufSize(mStreamAttributeSource, time_diff_ns);
                ALOGD("%s(), I. time_diff_ms %lf, compensate %u ul data", __FUNCTION__, ((double)time_diff_ns / 1000000), diff_buf_size);

                audio_ringbuf_compensate_value(&mRawDataBuf, 0, diff_buf_size);
                mIsEchoRefDataSync = true;
            } else { // aec new
                time_diff_ns = get_time_diff_ns(&mRawDataBufTimeStamp, &mEchoRefDataBufTimeStamp);
                diff_buf_size = getDiffBufSize(mStreamAttributeSourceEchoRef, time_diff_ns);
                ALOGD("%s(), II. time_diff_ms %lf, compensate %u echo ref data", __FUNCTION__, ((double)time_diff_ns / 1000000), diff_buf_size);

                audio_ringbuf_compensate_value(&mEchoRefDataBuf, 0, diff_buf_size);
                mIsEchoRefDataSync = true;
            }
        }
    } else if (data_buf_type == DATA_BUF_ECHO_REF) {
        if (isTimeStampStarted(&mEchoRefDataBufTimeStamp) == false) { // only keep the first timestamp
            ret = calculateTotalTimeSpec(&mStreamAttributeSourceEchoRef->Time_Info, &mEchoRefDataBufTimeStamp);
            if (ret != 0) {
                //ASSERT(ret == 0);
                return;
            }
            ALOGD("%s(AEC*), UL in: %lld.%.9ld, EchoRef: %lld.%.9ld", __FUNCTION__,
                  (long long)mRawDataBufTimeStamp.tv_sec,
                  mRawDataBufTimeStamp.tv_nsec,
                  (long long)mEchoRefDataBufTimeStamp.tv_sec,
                  mEchoRefDataBufTimeStamp.tv_nsec);
        } else {
            ALOGD("%s(AEC),  UL in: %lld.%.9ld, EchoRef: %lld.%.9ld", __FUNCTION__,
                  (long long)mRawDataBufTimeStamp.tv_sec,
                  mRawDataBufTimeStamp.tv_nsec,
                  (long long)mStreamAttributeSourceEchoRef->Time_Info.timestamp_get.tv_sec,
                  mStreamAttributeSourceEchoRef->Time_Info.timestamp_get.tv_nsec);
        }

        if (isTimeStampStarted(&mRawDataBufTimeStamp) == false) { // ul data not ready
            audio_ringbuf_copy_from_ringbuf_all(&mEchoRefDataBuf, rb);
        } else { // already got ul data
            audio_ringbuf_copy_from_ringbuf_all(&mEchoRefDataBuf, rb);

            if (t1_older_then_t2(&mRawDataBufTimeStamp, &mEchoRefDataBufTimeStamp)) { // aec new
                time_diff_ns = get_time_diff_ns(&mRawDataBufTimeStamp, &mEchoRefDataBufTimeStamp);
                diff_buf_size = getDiffBufSize(mStreamAttributeSourceEchoRef, time_diff_ns);
                ALOGD("%s(), III. time_diff_ms %lf, compensate %u echo ref data", __FUNCTION__, ((double)time_diff_ns / 1000000), diff_buf_size);

                audio_ringbuf_compensate_value(&mEchoRefDataBuf, 0, diff_buf_size);
                mIsEchoRefDataSync = true;
            } else { // ul new
                time_diff_ns = get_time_diff_ns(&mEchoRefDataBufTimeStamp, &mRawDataBufTimeStamp);
                diff_buf_size = getDiffBufSize(mStreamAttributeSource, time_diff_ns);
                ALOGD("%s(), IV. time_diff_ms %lf, compensate %u ul data", __FUNCTION__, ((double)time_diff_ns / 1000000), diff_buf_size);

                audio_ringbuf_compensate_value(&mRawDataBuf, 0, diff_buf_size);
                mIsEchoRefDataSync = true;
            }
        }
    } else {
        ASSERT(0);
    }

    if (mIsEchoRefDataSync) { // compensate data will add latency, drop it
        AUD_LOG_D("+mRawDataBuf data_count %u, mEchoRefDataBuf data_count %u, mRawDataPeriodBufSize = %u, mEchoRefDataPeriodBufSize = %u",
                   audio_ringbuf_count(&mRawDataBuf), audio_ringbuf_count(&mEchoRefDataBuf), mRawDataPeriodBufSize, mEchoRefDataPeriodBufSize);
        while (audio_ringbuf_count(&mRawDataBuf)     >= mRawDataPeriodBufSize &&
               audio_ringbuf_count(&mEchoRefDataBuf) >= mEchoRefDataPeriodBufSize) {
            audio_ringbuf_drop_data(&mRawDataBuf, mRawDataPeriodBufSize);
            audio_ringbuf_drop_data(&mEchoRefDataBuf, mEchoRefDataPeriodBufSize);
        }
        AUD_LOG_D("-mRawDataBuf data_count %u, -mEchoRefDataBuf data_count %u", audio_ringbuf_count(&mRawDataBuf), audio_ringbuf_count(&mEchoRefDataBuf));
    }
}


bool AudioALSACaptureDataClientAurisysNormal::IsNeedApplyVolume() {
    bool ret = false;

    /* Only real input CaptureDataprovider need to apply volume for mic mute */
    switch (mCaptureDataProvider->getCaptureDataProviderType()) {
    case CAPTURE_PROVIDER_NORMAL:
    case CAPTURE_PROVIDER_BT_SCO:
    case CAPTURE_PROVIDER_BT_CVSD:
    case CAPTURE_PROVIDER_TDM_RECORD:
    case CAPTURE_PROVIDER_EXTERNAL:
        ret = true;
        break;
    default:
        ret = false;
        break;
    }

    return ret;
}


status_t AudioALSACaptureDataClientAurisysNormal::ApplyVolume(void *Buffer, uint32_t BufferSize) {
    if (IsNeedApplyVolume() == false) {
        return NO_ERROR;
    }

    // check if need apply mute
    if (mMicMute != mStreamAttributeTarget->micmute) {
        mMicMute =  mStreamAttributeTarget->micmute ;
        mMuteTransition = false;
    }

    if (mMicMute == true) {
        // do ramp down
        if (mMuteTransition == false) {
            uint32_t count = BufferSize >> 1;
            float Volume_inverse = ((float)MTK_STREAMIN_VOLUEM_MAX / (float)count) * -1;
            short *pPcm = (short *)Buffer;
            int ConsumeSample = 0;
            int value = 0;
            while (count) {
                value = *pPcm * (MTK_STREAMIN_VOLUEM_MAX + (Volume_inverse * ConsumeSample));
                *pPcm = clamp16(value >> MTK_STREAMIN_VOLUME_VALID_BIT);
                pPcm++;
                count--;
                ConsumeSample ++;
                //ALOGD("ApplyVolume Volume_inverse = %f ConsumeSample = %d",Volume_inverse,ConsumeSample);
            }
            mMuteTransition = true;
        } else {
            memset(Buffer, 0, BufferSize);
        }
    } else if (mMicMute == false) {
        // do ramp up
        if (mMuteTransition == false) {
            uint32_t count = BufferSize >> 1;
            float Volume_inverse = ((float)MTK_STREAMIN_VOLUEM_MAX / (float)count);
            short *pPcm = (short *)Buffer;
            int ConsumeSample = 0;
            int value = 0;
            while (count) {
                value = *pPcm * (Volume_inverse * ConsumeSample);
                *pPcm = clamp16(value >> MTK_STREAMIN_VOLUME_VALID_BIT);
                pPcm++;
                count--;
                ConsumeSample ++;
                //ALOGD("ApplyVolume Volume_inverse = %f ConsumeSample = %d",Volume_inverse,ConsumeSample);
            }
            mMuteTransition = true;
        }
    }

    return NO_ERROR;
}


/*
 * =============================================================================
 *                     Aurisys Framework 2.0
 * =============================================================================
 */

void AudioALSACaptureDataClientAurisysNormal::CreateAurisysLibManager() {
    uint32_t sample_rate = 0 ;

    /* scenario & sample rate */
    if (IsVoIPEnable() ||
        mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1 ||
        mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2) {
        mAurisysScenario = (IsAECEnable()) ? AURISYS_SCENARIO_VOIP : AURISYS_SCENARIO_VOIP_WITHOUT_AEC;
        sample_rate = 16000;
    } else if (IsLowLatencyCapture() || mStreamAttributeTarget->input_source == AUDIO_SOURCE_UNPROCESSED) {
        mAurisysScenario = AURISYS_SCENARIO_RECORD_LOW_LATENCY;
        if (mStreamAttributeSource->sample_rate == 16000 ||
            mStreamAttributeSource->sample_rate == 48000) {
            sample_rate = mStreamAttributeSource->sample_rate;
        } else {
            sample_rate = 48000;
        }
    } else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_RECOGNITION) { /* no echo ref */
        mAurisysScenario = AURISYS_SCENARIO_RECORD_WITHOUT_AEC;
        sample_rate = 48000;
    } else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION3 ||
               mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_CALL ||
               mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_UPLINK ||
               mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_DOWNLINK) {
        mAurisysScenario = AURISYS_SCENARIO_RECORD_WITHOUT_AEC;
        sample_rate = 16000;
    } else {
        mAurisysScenario = AURISYS_SCENARIO_RECORD_WITHOUT_AEC;
        sample_rate = 48000;
    }

    /* manager config */
    AUDIO_ALLOC_STRUCT(struct aurisys_lib_manager_config_t, mManagerConfig);

    mManagerConfig->aurisys_scenario = mAurisysScenario;
    mManagerConfig->arsi_process_type = ARSI_PROCESS_TYPE_UL_ONLY;
    mManagerConfig->audio_format = mStreamAttributeSource->audio_format;
    mManagerConfig->sample_rate = sample_rate;
    mManagerConfig->frame_size_ms = (uint8_t)mLatency;
    mManagerConfig->num_channels_ul = mStreamAttributeSource->num_channels;
    mManagerConfig->num_channels_dl = 2;
    mManagerConfig->core_type = AURISYS_CORE_HAL;

    /* set custom info to all UL lib*/
#if (defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT) && (MTK_AUDIO_TUNING_TOOL_V2_PHASE >= 2))
    AppOps *appOps = appOpsGetInstance();
    if (appOps && appOps->appHandleIsFeatureOptionEnabled(appOps->appHandleGetInstance(), "VIR_SCENE_CUSTOMIZATION_SUPPORT")) {
        snprintf(mManagerConfig->custom_info, MAX_CUSTOM_INFO_LEN, "SetAudioCustomScene=%s",
                 mStreamAttributeTarget->mCustScene);
    }
#endif

    ALOGD("%s(), voip: %d, low_latency: %d, aec: %d, input_source: %d, flag: 0x%x => mAurisysScenario: %u",
          __FUNCTION__,
          IsVoIPEnable(),
          IsLowLatencyCapture(),
          IsAECEnable(),
          mStreamAttributeTarget->input_source,
          mStreamAttributeTarget->mAudioInputFlags,
          mAurisysScenario);

    /* task config */
    InitArsiTaskConfig(mManagerConfig);

    /* create manager */
    AL_AUTOLOCK(mAurisysLibManagerLock);

    mAurisysLibManager = create_aurisys_lib_manager(mManagerConfig);
    InitBufferConfig(mAurisysLibManager);

    aurisys_parsing_param_file(mAurisysLibManager);
    aurisys_create_arsi_handlers(mAurisysLibManager); /* should init task/buf configs first */
    aurisys_pool_buf_formatter_init(mAurisysLibManager); /* should init task/buf configs first */

    int16_t ulDigitalGain = mAudioALSAVolumeController->GetSWMICGain() << 2; // (unit: 0.25 db)
    int16_t ulAnalogGain = ((mAudioALSAVolumeController->GetULTotalGain() - 192) / 4 + 34 - mAudioALSAVolumeController->GetSWMICGain()) << 2; // (unit: 0.25 db)
    if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
        ulDigitalGain = 0;
        ulAnalogGain = 0;
        ALOGD("BT path set Digital MIC gain = 0");
    }
    aurisys_set_ul_digital_gain(mAurisysLibManager, ulAnalogGain, ulDigitalGain);
    //aurisys_set_dl_digital_gain(mAurisysLibManager, 0, 0);
}


/* TODO: move to aurisys framework?? add a new struct to keep hal arributes */
void AudioALSACaptureDataClientAurisysNormal::InitArsiTaskConfig(
    struct aurisys_lib_manager_config_t *pManagerConfig) {
    struct arsi_task_config_t *pTaskConfig = &pManagerConfig->task_config;

    /* input device */
    pTaskConfig->input_device_info.devices = mStreamAttributeTarget->input_device; /* TODO: HAL capture */
    pTaskConfig->input_device_info.audio_format = mStreamAttributeSource->audio_format;
    pTaskConfig->input_device_info.sample_rate = mStreamAttributeSource->sample_rate;
    pTaskConfig->input_device_info.channel_mask = mStreamAttributeSource->audio_channel_mask; /* TODO */
    pTaskConfig->input_device_info.num_channels = mStreamAttributeSource->num_channels;
    pTaskConfig->input_device_info.hw_info_mask = 0; /* TODO */

    /* output device (no DL path for record) */
    pTaskConfig->output_device_info.devices = mStreamAttributeTarget->output_devices;
    pTaskConfig->output_device_info.audio_format = AUDIO_FORMAT_DEFAULT;
    pTaskConfig->output_device_info.sample_rate = 0;
    pTaskConfig->output_device_info.channel_mask = AUDIO_CHANNEL_NONE;
    pTaskConfig->output_device_info.num_channels = 0;
    pTaskConfig->output_device_info.hw_info_mask = 0;

    /* task scene */
    pTaskConfig->task_scene = map_aurisys_scenario_to_task_scene(
        pManagerConfig->core_type,
        pManagerConfig->aurisys_scenario);

    /* audio mode */
    pTaskConfig->audio_mode = mStreamAttributeTarget->audio_mode;

    /* max device capability for allocating memory */
    pTaskConfig->max_input_device_sample_rate  = 48000; /* TODO */
    pTaskConfig->max_output_device_sample_rate = 48000; /* TODO */

    pTaskConfig->max_input_device_num_channels =
        AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport();
    pTaskConfig->max_output_device_num_channels = 2; /* TODO */

    /* flag & source */
    pTaskConfig->output_flags = 0;
    pTaskConfig->input_source = mStreamAttributeTarget->input_source;
    pTaskConfig->input_flags  = mStreamAttributeTarget->mAudioInputFlags;

    if (pTaskConfig->output_device_info.devices & AUDIO_DEVICE_OUT_SPEAKER &&
        AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        pTaskConfig->output_device_info.hw_info_mask = OUTPUT_DEVICE_HW_INFO_SMARTPA_SPEAKER;
        ALOGV("%s(), SmartPA suppport, pTaskConfig->output_device_info.hw_info_mask = %d",
              __FUNCTION__, pTaskConfig->output_device_info.hw_info_mask);
    }

    /* Enhancement feature */
    if (pTaskConfig->output_device_info.devices == AUDIO_DEVICE_OUT_EARPIECE &&
        SpeechEnhancementController::GetInstance()->GetHACOn()) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_EARPIECE_HAC;
    }

    if ((pTaskConfig->input_device_info.devices & AUDIO_DEVICE_IN_ALL_SCO)
        && (pTaskConfig->output_device_info.devices & AUDIO_DEVICE_OUT_ALL_SCO)
        && SpeechEnhancementController::GetInstance()->GetBtHeadsetNrecOn()) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_BT_NREC;
    }

    if ((pTaskConfig->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION)
        && (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_AECOn == true)
        && (pTaskConfig->input_device_info.devices != AUDIO_DEVICE_IN_ALL_SCO)) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_EC;
    }

    if ((pTaskConfig->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION)
        && (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_NSOn == true)
        && (pTaskConfig->input_device_info.devices != AUDIO_DEVICE_IN_ALL_SCO)) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_NS;
    }

    if ((pTaskConfig->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION)
        && (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_AGCOn == true)
        && (pTaskConfig->input_device_info.devices != AUDIO_DEVICE_IN_ALL_SCO)) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_AGC;
    }

    dump_task_config(pTaskConfig);
}


void AudioALSACaptureDataClientAurisysNormal::InitBufferConfig(struct aurisys_lib_manager_t *manager) {
    /* UL in */
    mAudioPoolBufUlIn = create_audio_pool_buf(manager, DATA_BUF_UPLINK_IN, 0);

    mAudioPoolBufUlIn->buf->b_interleave = 1; /* LRLRLRLR*/
    mAudioPoolBufUlIn->buf->frame_size_ms = 0;
    mAudioPoolBufUlIn->buf->num_channels = mStreamAttributeSource->num_channels;
    mAudioPoolBufUlIn->buf->sample_rate_buffer = mStreamAttributeSource->sample_rate;
    mAudioPoolBufUlIn->buf->sample_rate_content = mStreamAttributeSource->sample_rate;
    mAudioPoolBufUlIn->buf->audio_format = mStreamAttributeSource->audio_format;


    /* UL out */
    mAudioPoolBufUlOut = create_audio_pool_buf(manager, DATA_BUF_UPLINK_OUT, 0);

    mAudioPoolBufUlOut->buf->b_interleave = 1; /* LRLRLRLR*/
    mAudioPoolBufUlOut->buf->frame_size_ms = 0;
    mAudioPoolBufUlOut->buf->num_channels = mStreamAttributeTarget->num_channels;
    mAudioPoolBufUlOut->buf->sample_rate_buffer = mStreamAttributeTarget->sample_rate;
    mAudioPoolBufUlOut->buf->sample_rate_content = mStreamAttributeTarget->sample_rate;
    mAudioPoolBufUlOut->buf->audio_format = mStreamAttributeTarget->audio_format;


    /* Echo Ref */
    if (IsAECEnable()) {
        ASSERT(mStreamAttributeSourceEchoRef != NULL);
        mAudioPoolBufUlAec = create_audio_pool_buf(manager, DATA_BUF_ECHO_REF, 0);

        mAudioPoolBufUlAec->buf->b_interleave = 1; /* LRLRLRLR*/
        mAudioPoolBufUlAec->buf->frame_size_ms = 0;
        mAudioPoolBufUlAec->buf->num_channels = mStreamAttributeSourceEchoRef->num_channels;
        mAudioPoolBufUlAec->buf->sample_rate_buffer = mStreamAttributeSourceEchoRef->sample_rate;
        mAudioPoolBufUlAec->buf->sample_rate_content = mStreamAttributeSourceEchoRef->sample_rate;
        mAudioPoolBufUlAec->buf->audio_format = mStreamAttributeSourceEchoRef->audio_format;
    }

}


void AudioALSACaptureDataClientAurisysNormal::DestroyAurisysLibManager() {
    ALOGD("%s()", __FUNCTION__);

    AL_AUTOLOCK(mAurisysLibManagerLock);

    aurisys_destroy_arsi_handlers(mAurisysLibManager);
    aurisys_pool_buf_formatter_deinit(mAurisysLibManager);
    destroy_aurisys_lib_manager(mAurisysLibManager);
    mAurisysLibManager = NULL;

    /* NOTE: auto destroy audio_pool_buf when destroy_aurisys_lib_manager() */
    mAudioPoolBufUlIn = NULL;
    mAudioPoolBufUlOut = NULL;
    mAudioPoolBufUlAec = NULL;
    mAudioPoolBufDlIn = NULL;
    mAudioPoolBufDlOut = NULL;

    AUDIO_FREE_POINTER(mLinearOut->p_buffer);
    memset(mLinearOut, 0, sizeof(data_buf_t));

    AUDIO_FREE_POINTER(mManagerConfig);
}


//Android Native Preprocess effect +++
void AudioALSACaptureDataClientAurisysNormal::CheckNativeEffect(void) {
    if (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Update == false) {
        return; // nothing to update
    }

    ASSERT(mAudioPreProcessEffect != NULL);
    mAudioPreProcessEffect->CheckNativeEffect();
    mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Update = false;
}


uint32_t AudioALSACaptureDataClientAurisysNormal::NativePreprocess(void *buffer, uint32_t bytes) {
    CheckNativeEffect(); // stream in might addAudioEffect/removeAudioEffect
    if (mAudioPreProcessEffect->num_preprocessors == 0 || IsVoIPEnable() == true) {
        return bytes;
    }

    return mAudioPreProcessEffect->NativePreprocess(buffer, bytes, &mStreamAttributeSource->Time_Info);
}
//Android Native Preprocess effect ---


void AudioALSACaptureDataClientAurisysNormal::handleLockTimeout() {
    ALOGE("%s(), Lock timeout. Reopen StreamIn", __FUNCTION__);
    setStreamInReopen(true);
}


} // end of namespace android


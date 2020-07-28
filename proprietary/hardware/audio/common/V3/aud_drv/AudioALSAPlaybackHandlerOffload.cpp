#include "AudioALSAPlaybackHandlerOffload.h"
#include "AudioALSAHardwareResourceManager.h"
#include "AudioVolumeFactory.h"
#include "AudioALSASampleRateController.h"
#include "AudioALSADeviceParser.h"
#include "AudioALSADriverUtility.h"
#include "AudioMTKFilter.h"
#include "AudioALSADeviceConfigManager.h"
#include "AudioSmartPaController.h"
#ifdef MTK_AUDIODSP_SUPPORT
#include "AudioDspStreamManager.h"
#include "AudioALSAStreamManager.h"
#endif

#include <tinycompress/tinycompress.h>

#include <pthread.h>
#include <sys/prctl.h>

#ifdef MTK_AUDIO_SCP_SUPPORT
#include <audio_task.h>
#include <AudioMessengerIPI.h>
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerOffload"

namespace android {
static void *writeThreadOffload(void *arg);
static bool threadExit = true;
static const char* PROPERTY_KEY_EXTDAC = "vendor.audiohal.resource.extdac.support";
static bool offloadRouting = false;

struct offload_stream_property offload_stream;
struct offload_thread_property offload_thread;
struct offload_write_info offload_write;

static void *offload_threadloop(void *arg) {
    // force to set priority
    int command, ret = 0;
    int write_ret = 0;
    struct offload_cmd *cmd;
    bool callback, exit, drain;
    stream_callback_event_t event = STREAM_CBK_EVENT_ERROR;
    struct listnode *item;
    struct sched_param sched_p;

    pthread_mutex_lock(&offload_thread.offload_mutex);
    AudioALSAPlaybackHandlerOffload *pOffloadHandler = (AudioALSAPlaybackHandlerOffload *)arg;
#ifdef MTK_AUDIO_ADJUST_PRIORITY
#define RTPM_PRIO_AUDIO_PLAYBACK (95)
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = RTPM_PRIO_AUDIO_PLAYBACK;
    if (0 != sched_setscheduler(0, SCHED_RR, &sched_p)) {
        ALOGE("%s() failed, errno: %d", __FUNCTION__, errno);
    } else {
        sched_getparam(0, &sched_p);
        ALOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
    }
#endif
    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    offload_stream.offload_state = OFFLOAD_STATE_IDLE;
    offload_stream.remain_write = 0;
    exit = false;
    drain = false;

    for (;;) {
        cmd     = NULL;
        command = -1;
        callback = false;
        if (list_empty(&offload_thread.offload_cmd_list)) {
            ALOGV("%s(),list_empty, state:%x, remain:%x", __FUNCTION__, offload_stream.offload_state, offload_stream.remain_write);
            if (drain && offload_stream.offload_state == OFFLOAD_STATE_EARLY_DRAIN) {
                command = OFFLOAD_CMD_DRAIN;
            } else if (offload_stream.remain_write && offload_stream.offload_state == OFFLOAD_STATE_PLAYING) {
                command = OFFLOAD_CMD_WRITE;
            }
        } else {
            ALOGV("%s(),list not empty", __FUNCTION__);
            item = list_head(&offload_thread.offload_cmd_list);
            cmd = node_to_item(item, struct offload_cmd, node);
            command = cmd->cmd;
            list_remove(item);
            free(cmd);
        }

        if (command == -1) {
            pthread_cond_wait(&offload_thread.offload_cond, &offload_thread.offload_mutex);
            ALOGV("%s(),waitevent-", __FUNCTION__);
            continue;
        }

        pthread_mutex_unlock(&offload_thread.offload_mutex);
        ALOGV("%s()command:%x", __FUNCTION__, command);

        switch (command) {
        case OFFLOAD_CMD_WRITE:
            write_ret = pOffloadHandler->process_write();
            if (pOffloadHandler->isformatnotsupport() == 1) {
                ALOGD("%s() format not support, write ERROR callback notify", __FUNCTION__);
                write_ret = OFFLOAD_WRITE_ERROR;
            }
            if (write_ret == OFFLOAD_WRITE_REMAIN) {
                offload_stream.remain_write = 1;
                pOffloadHandler->process_writewait();
            } else if (write_ret == OFFLOAD_WRITE_EMPTY) {
                offload_stream.remain_write = 0;
                event = STREAM_CBK_EVENT_WRITE_READY;
                callback = true;
                drain = false;
                ALOGV("%s() write callback notify", __FUNCTION__);
            } else if (write_ret == OFFLOAD_WRITE_ERROR) {
                event = STREAM_CBK_EVENT_ERROR;
                callback = true;
                drain = false;
                ALOGV("%s() write ERROR callback notify", __FUNCTION__);
            }
            break;
        case OFFLOAD_CMD_DRAIN:

            ret = pOffloadHandler->process_drain();

            if (offload_stream.offload_state == OFFLOAD_STATE_EARLY_DRAIN && (list_empty(&offload_thread.offload_cmd_list))) {
                drain = true;
                pOffloadHandler->process_writewait();
            }

            if (list_empty(&offload_thread.offload_cmd_list)) {
                if (pOffloadHandler->isformatnotsupport() != 1){
                    event = STREAM_CBK_EVENT_DRAIN_READY;
                    callback = true;
                    drain = false;
                    ALOGD("%s() drain callback notify", __FUNCTION__);
                    pOffloadHandler->offload_initialize();
                    offload_stream.remain_write = 0;
                    offload_stream.offload_state = OFFLOAD_STATE_DRAINED;
                } else {
                    event = STREAM_CBK_EVENT_ERROR;
                    callback = true;
                    ALOGD("%s() OFFLOAD_CMD_DRAIN: format error", __FUNCTION__);
                }
            }
            break;
        case OFFLOAD_CMD_PAUSE:
            if (offload_stream.offload_state == OFFLOAD_STATE_PLAYING || offload_stream.offload_state == OFFLOAD_STATE_EARLY_DRAIN) {
                offload_stream.offload_state = OFFLOAD_STATE_PAUSED;
            }
            break;
        case OFFLOAD_CMD_RESUME:
            if (offload_stream.offload_state == OFFLOAD_STATE_PAUSED) {
                offload_stream.offload_state = OFFLOAD_STATE_PLAYING;
            }
            break;
        case OFFLOAD_CMD_FLUSH:
            if (offload_stream.offload_state == OFFLOAD_STATE_PLAYING || offload_stream.offload_state == OFFLOAD_STATE_EARLY_DRAIN
                || offload_stream.offload_state == OFFLOAD_STATE_PAUSED) {
                offload_stream.offload_state = OFFLOAD_STATE_IDLE;
                pOffloadHandler->offload_initialize();
                offload_stream.remain_write = 0;
                ret = 0;
            }
            break;
        case OFFLOAD_CMD_CLOSE:
            exit = true;
            break;
        default:
            ALOGE("%s(),Invalid Command", __FUNCTION__);
            break;
        }

        if (callback) {
            pOffloadHandler->offload_callback(event);
        }

        pthread_mutex_lock(&offload_thread.offload_mutex);

        if (exit && !offloadRouting) {
            pOffloadHandler->offload_callback(STREAM_CBK_EVENT_WRITE_READY);
            pOffloadHandler->offload_callback(STREAM_CBK_EVENT_DRAIN_READY);
            ALOGD("%s(),decode done, exit threadloop", __FUNCTION__);
            break;
        } else if (exit && offloadRouting){
            offloadRouting = false;
            break;
        } else if (pOffloadHandler->isformatnotsupport() == 1) {
            if (event != STREAM_CBK_EVENT_ERROR) {
                ALOGD("%s(),Invalid format, STREAM_CBK_EVENT_ERROR", __FUNCTION__);
                pOffloadHandler->offload_callback(STREAM_CBK_EVENT_ERROR);
           }
            ALOGD("%s(),Invalid format, exit threadloop", __FUNCTION__);
            break;
        }

    }
    pthread_mutex_unlock(&offload_thread.offload_mutex);
    ALOGD("%s()-", __FUNCTION__);
    threadExit = true;

    return NULL;
}

static int send_offload_cmd(int command) {
    struct offload_cmd *cmd = (struct offload_cmd *)calloc(1, sizeof(struct offload_cmd));

    cmd->cmd = command;

    if (!threadExit) {
        list_add_tail(&offload_thread.offload_cmd_list, &cmd->node);
        pthread_mutex_lock(&offload_thread.offload_mutex);
        pthread_cond_signal(&offload_thread.offload_cond);
        pthread_mutex_unlock(&offload_thread.offload_mutex);
    }

    ALOGD("%s %d", __FUNCTION__, command);
    return 0;
}

AudioALSAPlaybackHandlerOffload::AudioALSAPlaybackHandlerOffload(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source),
    mWriteBsbufSize(0),
    mReady(false) {
    ALOGD("%s()", __FUNCTION__);
    mPlaybackHandlerType = PLAYBACK_HANDLER_OFFLOAD;
    mFormat = stream_attribute_source->audio_format;
    memset(&mComprConfig, 0, sizeof(mComprConfig));
    mStreamAttributeTarget.audio_offload_format = stream_attribute_source->audio_offload_format;

}


AudioALSAPlaybackHandlerOffload::~AudioALSAPlaybackHandlerOffload() {
    ALOGD("%s()", __FUNCTION__);
}

void AudioALSAPlaybackHandlerOffload::offload_initialize() {
    mReady = false;
    mWriteBsbufSize  = 0;
}

void AudioALSAPlaybackHandlerOffload::set_pcmdump(int enable) {
    int ret = NO_ERROR;
    ret = mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "offload set dump"), 0, enable);
}

status_t AudioALSAPlaybackHandlerOffload::setFilterMng(AudioMTKFilterManager *pFilterMng) {
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    ALOGD("+%s() mAudioFilterManagerHandler [%p]", __FUNCTION__, pFilterMng);
    mAudioFilterManagerHandler = pFilterMng;
#else
    (void *)pFilterMng;
#endif
    return NO_ERROR;
}

void AudioALSAPlaybackHandlerOffload::offload_callback(stream_callback_event_t event) {
    if (mCbkCookie != NULL) {
        mStreamCbk(event, 0, mCbkCookie);
    }
}


uint32_t AudioALSAPlaybackHandlerOffload::GetLowJitterModeSampleRate() {
    return 48000;
}


bool AudioALSAPlaybackHandlerOffload::SetLowJitterMode(bool bEnable, uint32_t SampleRate) {
    ALOGD("%s() bEanble = %d SampleRate = %u", __FUNCTION__, bEnable, SampleRate);

#if !defined(MTK_AUDIO_KS)
    enum mixer_ctl_type type;
    struct mixer_ctl *ctl;
    int retval = 0;

    // check need open low jitter mode
    if (SampleRate <= GetLowJitterModeSampleRate() && (AudioALSADriverUtility::getInstance()->GetPropertyValue(PROPERTY_KEY_EXTDAC)) == false) {
        if (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_SPEAKER) {
            ALOGD("%s(), force enable low jitter mode, bEnable = %d, device = 0x%x", __FUNCTION__, bEnable, mStreamAttributeSource->output_devices);
        } else {
            ALOGD("%s(), bypass low jitter mode, bEnable = %d, device = 0x%x", __FUNCTION__, bEnable, mStreamAttributeSource->output_devices);
            return false;
        }
    }

    ctl = mixer_get_ctl_by_name(mMixer, "Audio_I2S0dl1_hd_Switch");

    if (ctl == NULL) {
        ALOGE("Audio_I2S0dl1_hd_Switch not support");
        return false;
    }

    if (bEnable == true) {
        retval = mixer_ctl_set_enum_by_string(ctl, "On");
        ASSERT(retval == 0);
    } else {
        retval = mixer_ctl_set_enum_by_string(ctl, "Off");
        ASSERT(retval == 0);
    }
#endif
    return true;
}

int AudioALSAPlaybackHandlerOffload::setAfeDspSharemem(bool condition) {
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_offload_sharemem_scenario"), 0, condition))
        ALOGW("%s(), enable fail", __FUNCTION__);
    return 0;
}

int AudioALSAPlaybackHandlerOffload::setDspRuntimeEn(bool condition) {
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "dsp_offload_runtime_en"), 0, condition)) {
        ALOGW("%s(), enable fail", __FUNCTION__);
        return -1;
    }
    return 0;
}


status_t AudioALSAPlaybackHandlerOffload::open() {
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    int ret = 0;
    int pcmindex;
    int cardindex;

#ifdef MTK_AUDIO_SCP_SUPPORT
    /* Load task scene when opening */
    mAudioMessengerIPI->loadTaskScene(TASK_SCENE_PLAYBACK_MP3);

    if (mStreamAttributeTarget.audio_offload_format == AUDIO_FORMAT_MP3) {
        ALOGD("%s(),AUDIO_FORMAT_MP3 loadTaskScene", __FUNCTION__);
        ret = mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "offload set format"), 0, TASK_SCENE_OFFLOAD_MP3);
    } else if (mStreamAttributeTarget.audio_offload_format == AUDIO_FORMAT_AAC_LC) {
        ALOGD("%s(),AUDIO_FORMAT_AAC_LC loadTaskScene", __FUNCTION__);
        ret = mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "offload set format"), 0, TASK_SCENE_OFFLOAD_AAC);
    }
    if (ret != 0) {
        ALOGE("%s, Failed to initialize Scene!", __FUNCTION__);
        return INVALID_OPERATION;
    }
    int formaterror = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "offload set format"), 1);
    ALOGD("%s(),offload formaterror = %d", __FUNCTION__, formaterror);

    setAfeDspSharemem(true);
    setDspRuntimeEn(true);
#endif

    mHardwareResourceManager->EnableAudBufClk(true);

    mStreamAttributeTarget.buffer_size = 32768; //32K
#ifdef PLAYBACK_USE_24BITS_ONLY
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
#else
    mStreamAttributeTarget.audio_format = (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT) ? AUDIO_FORMAT_PCM_8_24_BIT : AUDIO_FORMAT_PCM_16_BIT;
#endif
    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = ChooseTargetSampleRate(AudioALSASampleRateController::getInstance()->getPrimaryStreamOutSampleRate());
    mStreamAttributeTarget.output_devices = mStreamAttributeSource->output_devices;
    mStreamAttributeTarget.mAudioOutputFlags = mStreamAttributeSource->mAudioOutputFlags;

    // debug pcm dump
    OpenPCMDump(LOG_TAG);

    int compress_dev_index = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmOffloadPlayback);
    int compress_cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmOffloadPlayback);

// using DL6 for offload , if not exixst using DL3
#if defined(MTK_AUDIO_KS)
    String8 playbackSeq = String8();

    pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback6);
    if (pcmindex < 0) {
        pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback3);
        cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback3);
        playbackSeq = String8(AUDIO_CTL_PLAYBACK3);
    } else {
        cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback6);
        playbackSeq = String8(AUDIO_CTL_PLAYBACK6);
    }

#if defined(MTK_AUDIODSP_SUPPORT)
    mApTurnOnSequence3 = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_3, playbackSeq);
    mApTurnOnSequenceDsp = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_DSP, playbackSeq);

    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequenceDsp);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence3);
#endif
    mApTurnOnSequence = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_1, playbackSeq);
    mApTurnOnSequence2 = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_2, playbackSeq);
    mHardwareResourceManager->setCustOutputDevTurnOnSeq(mStreamAttributeSource->output_devices,
                                                        mTurnOnSeqCustDev1, mTurnOnSeqCustDev2);

    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence2);
    mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev1);
    mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev2);

#else
    /* not MTK_AUDIO_KS */
    pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmDl3Meida);
    cardindex =  AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmDl3Meida);
    ALOGD("AudioALSAPlaybackHandlerOffload::open() compress dev index = %d cardindex = %d", compress_dev_index, cardindex);
#endif //  defined(MTK_AUDIO_KS)

    mComprConfig.codec = (struct snd_codec *)malloc(sizeof(struct snd_codec));

    if (mComprConfig.codec == NULL) {
        ALOGE("%s() mComprConfig.codec malloc failed", __FUNCTION__);
        goto STATUS_ERROR;
    }
    //kernel buffersize = 4M -32K
    mComprConfig.fragments = 2;
    mComprConfig.fragment_size = OFFLOAD_BUFFER_SIZE_PER_ACCESSS;
    mComprConfig.codec->sample_rate = mStreamAttributeSource->offload_codec_info.codec_samplerate;
    mComprConfig.codec->bit_rate = mStreamAttributeSource->offload_codec_info.codec_bitrate;
    mComprConfig.codec->reserved[0] = (mStreamAttributeTarget.buffer_size / (mStreamAttributeTarget.num_channels * 2)) / ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);
    mComprConfig.codec->reserved[1] = mComprConfig.fragments * mComprConfig.fragment_size;
    mComprConfig.codec->reserved[2] = mStreamAttributeTarget.sample_rate;

    if (transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format) == PCM_FORMAT_S16_LE) {
        mComprConfig.codec->format = SNDRV_PCM_FORMAT_S16_LE;
    } else {
        mComprConfig.codec->format = SNDRV_PCM_FORMAT_S32_LE;
    }

    mComprConfig.codec->id = SND_AUDIOCODEC_MP3;
    mComprConfig.codec->ch_in = mStreamAttributeTarget.num_channels;
    mComprConfig.codec->ch_out = mStreamAttributeTarget.num_channels;
    // set HW pcm config
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;
    // Buffer size: 1536(period_size) * 2(ch) * 4(byte) * 2(period_count) = 24 kb
    mConfig.period_count = 2;
    mConfig.period_size = (mStreamAttributeTarget.buffer_size / (mConfig.channels * mConfig.period_count)) /
                          ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);
    mConfig.format = transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format);
    mConfig.start_threshold = 0;
    mConfig.stop_threshold = ~(0U);
    mConfig.silence_threshold = 0;
    mConfig.avail_min = mStreamAttributeSource->buffer_size / ((mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4) / mStreamAttributeSource->num_channels;
    //lowjitter mode
    SetLowJitterMode(true, mStreamAttributeTarget.sample_rate);

     //if( compress_set_gapless_metadata(mComprStream, &offload_stream.offload_mdata) != 0)

    if (openComprDriver(compress_dev_index) != NO_ERROR) {
        goto STATUS_ERROR;
    }

    if (openPcmDriver(pcmindex) != NO_ERROR) {
        goto STATUS_ERROR;
    }
    if (pcm_start(mPcm) != 0) {
        ALOGE("%s(), pcm_start(%p) == false due to %s", __FUNCTION__, mPcm, pcm_get_error(mPcm));
    }
    ALOGD("-%s(),", __FUNCTION__);

#if defined(MTK_AUDIODSP_SUPPORT)
    if (AudioDspStreamManager::getInstance()->getDspPlaybackEnable() == true ||
        AudioDspStreamManager::getInstance()->getDspA2DPEnable() == true) {
        AudioDspStreamManager::getInstance()->addPlaybackHandler(this);
    }
    audio_ipi_dma_cbk_register(TASK_SCENE_PLAYBACK_MP3,
                               0,
                               0x48000,
                               processDmaMsgWrapper,
                               this);
    OpenPCMDumpDSP(LOG_TAG, TASK_SCENE_PLAYBACK_MP3);
#endif

    mHardwareResourceManager->startOutputDevice(mStreamAttributeSource->output_devices, mStreamAttributeTarget.sample_rate);
#if defined(MTK_HYBRID_NLE_SUPPORT) // must be after pcm open
    mStreamAttributeTarget.output_devices = mStreamAttributeSource->output_devices;
    initNLEProcessing();
#endif

    offload_stream.fragment_size = OFFLOAD_BUFFER_SIZE_PER_ACCESSS;
    offload_stream.tmpbsBuffer = (void *)malloc(offload_stream.fragment_size << 2);
    offload_write.tmpBuffer = (void *)malloc(offload_stream.fragment_size);
    offload_write.bytes = offload_stream.fragment_size;

    //mWritebytes = 0;
    offload_stream.num_channels = mStreamAttributeTarget.num_channels;
    offload_stream.sample_rate = mStreamAttributeTarget.sample_rate;
    offload_stream.remain_write = 0;
    ALOGD("%s open offload num_channels = %d, sample_rate = %d , remain_write = %d, offload_write.tmpBuffer = %p",
          __FUNCTION__, offload_stream.num_channels, offload_stream.sample_rate, offload_stream.remain_write, offload_write.tmpBuffer);

    list_init(&offload_thread.offload_cmd_list);
    ret = pthread_mutex_init(&offload_thread.offload_mutex, NULL);

    if (ret != 0) {
        ALOGE("%s, Failed to initialize Mutex!", __FUNCTION__);
        goto STATUS_ERROR;
    }

    ret = pthread_cond_init(&offload_thread.offload_cond, NULL);

    if (ret != 0) {
        ALOGE("%s, Failed to initialize Cond!", __FUNCTION__);
        goto STATUS_ERROR;
    }

    ret = pthread_create(&offload_thread.offload_pthread, NULL, &offload_threadloop, this);

    if (ret != 0) {
        ALOGE("%s() create thread OffloadWrite fail!!", __FUNCTION__);
        goto STATUS_ERROR;
    }
    threadExit = false;
    usleep(1 * 1000);
    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
STATUS_ERROR:
    ALOGD("-%s() STATUS_ERROR ret = %d", __FUNCTION__, ret);
    return INVALID_OPERATION;
}

status_t AudioALSAPlaybackHandlerOffload::close() {
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    if (offload_stream.offload_state == OFFLOAD_STATE_PLAYING
        || offload_stream.offload_state == OFFLOAD_STATE_PAUSED
        || offload_stream.offload_state == OFFLOAD_STATE_DRAINED) {
        offload_stream.offload_state = OFFLOAD_STATE_IDLE;
    }
    closeComprDriver();
    //close compress device driver
    send_offload_cmd(OFFLOAD_CMD_CLOSE);

#if defined(MTK_AUDIODSP_SUPPORT)
    if (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_ALL_A2DP) {
        AudioALSAStreamManager::getInstance()->setA2dpPlaybackPaused(false);
    }
#endif

    ALOGD("%s() start wait", __FUNCTION__);
    while (!threadExit) {
        usleep(1 * 1000);
    }
    pthread_join(offload_thread.offload_pthread, (void **) NULL);
    pthread_cond_destroy(&offload_thread.offload_cond);
    pthread_mutex_destroy(&offload_thread.offload_mutex);

#if defined(MTK_HYBRID_NLE_SUPPORT)
    // Must do this before close analog path
    deinitNLEProcessing();
#endif
    // close codec driver
    mHardwareResourceManager->stopOutputDevice();
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev1);
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev2);

    // close pcm driver
    closePcmDriver();

#if defined(MTK_AUDIODSP_SUPPORT)
    if (AudioDspStreamManager::getInstance()->getDspPlaybackEnable() == true ||
        AudioDspStreamManager::getInstance()->getDspA2DPEnable() == true) {
        AudioDspStreamManager::getInstance()->removePlaybackHandler(this);
    }
#endif

#if defined(MTK_AUDIO_KS)
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence2);
#ifdef MTK_AUDIODSP_SUPPORT
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequenceDsp);
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence3);
#endif
#endif

    // disable lowjitter mode  //doug to check
    SetLowJitterMode(false, mStreamAttributeTarget.sample_rate);

    setDspRuntimeEn(false);
    setAfeDspSharemem(false);

    // debug pcm dump
    ClosePCMDump();
#ifdef MTK_AUDIODSP_SUPPORT
    audio_ipi_dma_cbk_deregister(TASK_SCENE_PLAYBACK_MP3);
    ClosePCMDumpDSP(TASK_SCENE_PLAYBACK_MP3);
#endif

    //release pmic clk
    mHardwareResourceManager->EnableAudBufClk(false);

    if (mComprConfig.codec != NULL) {
        free(mComprConfig.codec);
        mComprConfig.codec = NULL;
    }
    if (offload_stream.tmpbsBuffer != NULL) {
        free(offload_stream.tmpbsBuffer);
        offload_stream.tmpbsBuffer = NULL;
    }
    if (offload_write.tmpBuffer != NULL) {
        free(offload_write.tmpBuffer);
        offload_write.tmpBuffer = NULL;
    }

    ALOGD("%s() closed", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerOffload::routing(const audio_devices_t output_devices) {
    mHardwareResourceManager->changeOutputDevice(output_devices);
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (mAudioFilterManagerHandler) { mAudioFilterManagerHandler->setDevice(output_devices); }
#endif
    return NO_ERROR;
}

int AudioALSAPlaybackHandlerOffload::pause() {
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());
    if (threadExit) {
        return INVALID_OPERATION;
    }

    int ret = NO_ERROR;

#if defined(MTK_AUDIODSP_SUPPORT)
    if (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_ALL_A2DP) {
        AudioALSAStreamManager::getInstance()->setA2dpPlaybackPaused(true);
        int status = AudioALSAStreamManager::getInstance()->getA2dpSuspendStatus();
        if (status != 4) { // Send SUSPEND request when status is not SUSPEND SUCCESS
            ALOGD("%s() status=%d, setA2dpSuspend true", __FUNCTION__, status);
            AudioALSAStreamManager::getInstance()->setA2dpSuspend(true);
        }
    }
#endif

    ALOGD("%s() state:%x", __FUNCTION__, offload_stream.offload_state);
    send_offload_cmd(OFFLOAD_CMD_PAUSE);
    ret = compress_pause(mComprStream);
    if (ret != 0) {
        ALOGE("%s() error %d", __FUNCTION__, ret);
        return -ENODATA;
    }
    return 0;
}

int AudioALSAPlaybackHandlerOffload::resume() {
    if (threadExit) {
        return INVALID_OPERATION;
    }

    int ret = NO_ERROR;
  
#if defined(MTK_AUDIODSP_SUPPORT)
    if (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_ALL_A2DP) {
        AudioALSAStreamManager::getInstance()->setA2dpPlaybackPaused(false);
        int status = AudioALSAStreamManager::getInstance()->getA2dpSuspendStatus();
        if (status != 0) { // Send START request when status is not START SUCCESS
            ALOGD("%s() status=%d, setA2dpSuspend false", __FUNCTION__, status);
            AudioALSAStreamManager::getInstance()->setA2dpSuspend(false);
        }
    }
#endif
    ALOGD("%s() state:%x", __FUNCTION__, offload_stream.offload_state);
    send_offload_cmd(OFFLOAD_CMD_RESUME);
    ret = compress_resume(mComprStream);
    if (ret != 0) {
        ALOGE("%s() error %d", __FUNCTION__, ret);
        return -ENODATA;
    }
    return 0;
}

status_t AudioALSAPlaybackHandlerOffload::setVolume(uint32_t vl) {
    int ret = NO_ERROR;

    offload_stream.offload_gain[0] = vl;
    offload_stream.offload_gain[1] = 1;
    ret = mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "offload digital volume"), 0, vl);
    ALOGD("%s() VOL:0x%x ", __FUNCTION__, vl);
    return ret;
}

int AudioALSAPlaybackHandlerOffload::drain(audio_drain_type_t type) {
    if (threadExit) {
        return INVALID_OPERATION;
    }
    /* make sure to let kernel blocking in drain state*/
    int ret = NO_ERROR;
    offload_stream.offload_state = OFFLOAD_STATE_EARLY_DRAIN;
    process_write();
    //ret = compress_drain(mComprStream);
    send_offload_cmd(OFFLOAD_CMD_DRAIN);
    ALOGD("%s() drain type = %d", __FUNCTION__, type);

    return 0;
}

int AudioALSAPlaybackHandlerOffload::flush() {
    if (threadExit) {
        return INVALID_OPERATION;
    }

    int ret = NO_ERROR;

    ALOGD("%s() state:%x", __FUNCTION__, offload_stream.offload_state);
    ret = compress_stop(mComprStream);
    if (ret != 0) {
        ALOGE("%s() error %d", __FUNCTION__, ret);
    }
    send_offload_cmd(OFFLOAD_CMD_FLUSH);
    return 0;
}


ssize_t AudioALSAPlaybackHandlerOffload::write(const void *buffer, size_t bytes) {
    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);

    if (offload_stream.offload_state == OFFLOAD_STATE_DRAINED) { //gapless:no close & flush
        if (compress_stop(mComprStream) != 0) {
            ALOGE("%s() error", __FUNCTION__);
        }
        offload_stream.offload_state = OFFLOAD_STATE_IDLE;
    }

    memcpy((char *)offload_stream.tmpbsBuffer + mWriteBsbufSize, buffer, bytes);
    mWriteBsbufSize += bytes;
    ALOGV("%s(), offload_write buffer = %p, bytes = %zu, mWriteBsbufSize =%d", __FUNCTION__, offload_write.tmpBuffer, bytes, mWriteBsbufSize);
    send_offload_cmd(OFFLOAD_CMD_WRITE);
    return bytes;
}




uint32_t AudioALSAPlaybackHandlerOffload::ChooseTargetSampleRate(uint32_t SampleRate) {
    uint32_t TargetSampleRate = 44100;

    if (SampleRate <=  192000 && SampleRate > 96000) {
        TargetSampleRate = 192000;
    } else if (SampleRate <= 96000 && SampleRate > 48000) {
        TargetSampleRate = 96000;
    } else {
        TargetSampleRate = SampleRate;
    }
    return TargetSampleRate;
}

int AudioALSAPlaybackHandlerOffload::process_writewait() {
    ALOGD("%s()", __FUNCTION__);
    compress_wait(mComprStream, -1);
    return 0;
}

int AudioALSAPlaybackHandlerOffload::isformatnotsupport() {
    int formaterror = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "offload set format"), 0);
    if (mStreamAttributeSource->offload_codec_info.disable_codec == 1 || formaterror == 1) {
        ALOGD("%s(), formaterror:%d, mStreamAttributeSource.offload_codec_info.disable_codec:%d", __FUNCTION__, formaterror, mStreamAttributeSource->offload_codec_info.disable_codec);
        return true;
    }
    return false;
}

int AudioALSAPlaybackHandlerOffload::process_drain() {
    ALOGD("%s()", __FUNCTION__);
    compress_drain(mComprStream);
    return 0;
}
int AudioALSAPlaybackHandlerOffload::process_write() {
    int ret = NO_ERROR;
    int remained_bitstream = 0;

    // check  setwriteblock flag
    if (mWriteBsbufSize >= offload_stream.fragment_size) {
        remained_bitstream = mWriteBsbufSize - offload_stream.fragment_size;
        memcpy(offload_write.tmpBuffer, offload_stream.tmpbsBuffer, offload_stream.fragment_size);
        ret = compress_write(mComprStream, offload_write.tmpBuffer, offload_stream.fragment_size);
        if (offload_stream.offload_state == OFFLOAD_STATE_EARLY_DRAIN) {
            ALOGD("%s(), OFFLOAD_STATE_EARLY_DRAIN, process_write 1", __FUNCTION__);
        }
        if (ret < 0) {
            ALOGE("%s(), write() error, ret = %d", __FUNCTION__, ret);
            return OFFLOAD_WRITE_ERROR;
        }
    } else {
        if (offload_stream.offload_state == OFFLOAD_STATE_EARLY_DRAIN && mWriteBsbufSize != 0) {
            mWriteBsbufSize = mWriteBsbufSize & 0Xffffff80;
            memcpy(offload_write.tmpBuffer, offload_stream.tmpbsBuffer, mWriteBsbufSize);
            ret = compress_write(mComprStream, offload_write.tmpBuffer, mWriteBsbufSize);
            ALOGD("%s(), OFFLOAD_STATE_EARLY_DRAIN, compress_write 2 , ret = %d, mWriteBsbufSize = %d", __FUNCTION__, ret, mWriteBsbufSize);
            if (ret < 0) {
                ALOGE("%s(), write() error, ret = %d", __FUNCTION__, ret);
                return OFFLOAD_WRITE_ERROR;
            }
            mWriteBsbufSize = 0;
        }
        return OFFLOAD_WRITE_EMPTY;
    }
    if (!mReady) {
        mReady = true;
        if (offload_stream.offload_state == OFFLOAD_STATE_IDLE) {
            offload_stream.offload_state = OFFLOAD_STATE_PLAYING;
            compress_start(mComprStream);
            compress_nonblock(mComprStream, 1);
            ALOGD("%s(), OFFLOADSERVICE_START", __FUNCTION__);
#if defined(MTK_AUDIODSP_SUPPORT)
            if (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_ALL_A2DP) {
                int status = AudioALSAStreamManager::getInstance()->getA2dpSuspendStatus();
                if (status != 0) { // Send start request when status is not START SUCCESS
                    ALOGD("%s() status=%d, setA2dpSuspend false", __FUNCTION__, status);
                    AudioALSAStreamManager::getInstance()->setA2dpSuspend(false); // for seek+next or pause+seek+resume operation
                }
            }
#endif
        }
    }
    if (ret == (int)offload_stream.fragment_size) {
        memmove(offload_stream.tmpbsBuffer, (char *)offload_stream.tmpbsBuffer + offload_stream.fragment_size, remained_bitstream);
        mWriteBsbufSize = remained_bitstream;
        ALOGV("%s(), ret = %d mWriteBsbufSize = Remained BS = %d", __FUNCTION__, ret, remained_bitstream);
        return OFFLOAD_WRITE_EMPTY;
    } else {
        remained_bitstream = mWriteBsbufSize - ret;

        if (ret != 0) {
            memmove(offload_stream.tmpbsBuffer, (char *)offload_stream.tmpbsBuffer + ret, remained_bitstream);
            mWriteBsbufSize = remained_bitstream;
        }
        ALOGV("%s(), REMAIN ret = %d Remained BS = %d", __FUNCTION__, ret, remained_bitstream);
        return OFFLOAD_WRITE_REMAIN;
    }

    return ret;
}

bool AudioALSAPlaybackHandlerOffload::setOffloadRoutingFlag(bool enable) {

    ALOGD("%s(), set offloadRouting = %d", __FUNCTION__, enable);
    offloadRouting = enable;
    return offloadRouting;
}

} // end of namespace android



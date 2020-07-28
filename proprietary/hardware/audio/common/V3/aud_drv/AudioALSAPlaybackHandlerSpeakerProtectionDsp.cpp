#include "AudioALSAPlaybackHandlerSpeakerProtectionDsp.h"

#include "AudioALSAHardwareResourceManager.h"
#include "AudioVolumeFactory.h"
#include "AudioALSASampleRateController.h"

#include "AudioMTKFilter.h"
#include "AudioALSADeviceParser.h"
#include "AudioALSADriverUtility.h"
#include <AudioMessengerIPI.h>
#include "AudioSmartPaParam.h"

#undef MTK_HDMI_SUPPORT

#if defined(MTK_HDMI_SUPPORT)
#include "AudioExtDisp.h"
#endif

#include "AudioSmartPaController.h"
#include "AudioALSAStreamManager.h"
#include "AudioALSADeviceConfigManager.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <audio_ringbuf.h>
#include <audio_pool_buf_handler.h>
#include <audio_task.h>
#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>
#endif


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerSpeakerProtectionDsp"

// Latency Detect
//#define DEBUG_LATENCY
#ifdef DEBUG_LATENCY
#define THRESHOLD_FRAMEWORK   0.010
#define THRESHOLD_HAL         0.010
#define THRESHOLD_KERNEL      0.010
#endif
#define SCP_SMARTPA_KERNEL_BUF_SIZE (32 * 1024)

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)

namespace android {

AudioALSAPlaybackHandlerSpeakerProtectionDsp::AudioALSAPlaybackHandlerSpeakerProtectionDsp(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source) {
    ALOGD("%s()", __FUNCTION__);
    memset((void *)&mNewtime, 0, sizeof(mNewtime));
    memset((void *)&mOldtime, 0, sizeof(mOldtime));
    memset((void *)&mScpSpkHwOutConfig, 0, sizeof(mScpSpkHwOutConfig));
    memset((void *)&mScpSpkHwInConfig, 0, sizeof(mScpSpkHwInConfig));
    mPlaybackHandlerType = PLAYBACK_HANDLER_SPEAKERPROTECTION;
    mScpSpkDlHwPcm = NULL;
    mScpSpkIvHwPcm = NULL;
}

AudioALSAPlaybackHandlerSpeakerProtectionDsp::~AudioALSAPlaybackHandlerSpeakerProtectionDsp() {
    ALOGD("%s()", __FUNCTION__);
}

int AudioALSAPlaybackHandlerSpeakerProtectionDsp::setPcmDump(bool enable) {
    ALOGD("%s() enable = %d", __FUNCTION__, enable);

    char value[PROPERTY_VALUE_MAX];
    char ctrlValue[32];
    int ret;
    enum mixer_ctl_type type;
    struct mixer_ctl *ctl;
    int retval = 0;

    memset(ctrlValue, '\0', 32);

    property_get(streamout_propty, value, "0");
    int flag = atoi(value);

    if (flag == 0) {
        ALOGD("%s() %s property not set no dump", __FUNCTION__, streamout_propty);
        return 0;
    }

    ret = AudiocheckAndCreateDirectory(audio_dump_path);
    if (ret < 0) {
        ALOGE("AudiocheckAndCreateDirectory(%s) fail!", audio_dump_path);
        flag = 0;
    }

    ctl = mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_pcm_dump");
    if (ctl == NULL) {
        ALOGE("mtk_scp_spk_pcm_dump not support");
        return -1;
    }

    if (flag == 1) {
        strcpy(ctrlValue, "normal_dump");
    } else if (flag == 2) {
        strcpy(ctrlValue, "split_dump");
    } else {
        ALOGE("property set not support!\n");
        ASSERT(0);
        return -1;
    }

    if (enable == true) {
        retval = mixer_ctl_set_enum_by_string(ctl, ctrlValue);
        ASSERT(retval == 0);
    } else {
        retval = mixer_ctl_set_enum_by_string(ctl, "off");
        ASSERT(retval == 0);
    }

    return 0;
}

uint32_t AudioALSAPlaybackHandlerSpeakerProtectionDsp::ChooseTargetSampleRate(uint32_t SampleRate, audio_devices_t outputdevice) {
    ALOGD("ChooseTargetSampleRate SampleRate in = %d outputdevice = %d", SampleRate, outputdevice);
    uint32_t TargetSampleRate = 48000;

    if (AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        ALOGD("%s(): find out speaker support rate", __FUNCTION__);
        bool isRateSupport = AudioSmartPaController::getInstance()->isRateSupported(SampleRate);
        if (isRateSupport == true) {
            TargetSampleRate =  SampleRate;
        } else {
            TargetSampleRate = AudioSmartPaController::getInstance()->getMaxSupportedRate();
        }
    } else {
        if (SampleRate >= 48000) {
            TargetSampleRate = 48000;
        } else if (SampleRate < 48000 && SampleRate >= 44100) {
            TargetSampleRate = 44100;
        } else if (SampleRate == 16000) {
            TargetSampleRate = 16000;
        }
    }

    return TargetSampleRate;
}

status_t AudioALSAPlaybackHandlerSpeakerProtectionDsp::openScpSpkPcmDriverWithFlag
                                                       (const unsigned int device,
                                                        unsigned int flag) {
    struct pcm *mScpSpkPcm = NULL;

    ALOGD("+%s(), pcm device = %d, flag = 0x%x", __FUNCTION__, device, flag);

    if (flag & PCM_IN) {
        ASSERT(mScpSpkIvHwPcm == NULL);
        mScpSpkIvHwPcm = pcm_open(AudioALSADeviceParser::getInstance()->GetCardIndex(),
                                  device, flag, &mScpSpkHwInConfig);
        mScpSpkPcm = mScpSpkIvHwPcm;
    } else {
        ASSERT(mScpSpkDlHwPcm == NULL);
        mScpSpkDlHwPcm = pcm_open(AudioALSADeviceParser::getInstance()->GetCardIndex(),
                                  device, flag, &mScpSpkHwOutConfig);
        mScpSpkPcm = mScpSpkDlHwPcm;
    }

    if (mScpSpkPcm == NULL) {
        ALOGE("%s(), mScpSpkPcm == NULL!!", __FUNCTION__);
    } else if (pcm_is_ready(mScpSpkPcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.",
              __FUNCTION__, mScpSpkPcm, pcm_get_error(mScpSpkPcm));
        pcm_close(mScpSpkPcm);
        mScpSpkPcm = NULL;
    } else if (pcm_prepare(mScpSpkPcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.",
              __FUNCTION__, mScpSpkPcm, pcm_get_error(mScpSpkPcm));
        pcm_close(mScpSpkPcm);
        mScpSpkPcm = NULL;
    }

    ASSERT(mScpSpkPcm != NULL);

    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtectionDsp::openScpSpkPcmDriver(const unsigned int dlDevice,
                                                                           const unsigned int ivDevice) {
    int ret;
    ret = openScpSpkPcmDriverWithFlag(dlDevice, PCM_OUT | PCM_MONOTONIC);
    ret = openScpSpkPcmDriverWithFlag(ivDevice, PCM_IN | PCM_MONOTONIC);
    return ret;
}

status_t AudioALSAPlaybackHandlerSpeakerProtectionDsp::openScpSpkHwPcm() {
#if defined(MTK_AUDIO_KS)
    int pcmDlIndex = -1, cardDlIndex = 0;
    int pcmIvIndex = -1, cardIvIndex = 0;
    struct pcm_params *params = NULL;

    ALOGV("+%s(),", __FUNCTION__);

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_dl_scenario"), 0, true)) {
        ALOGE("Error: mtk_scp_spk_dl_scenario invalid value");
    }

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_iv_scenario"), 0, true)) {
        ALOGE("Error: mtk_scp_spk_iv_scenario invalid value");
    }

    // scp spk DL data control path
    pcmDlIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback1);
    cardDlIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback1);

    mApTurnOnSequence = AudioSmartPaController::getInstance()->getI2sSequence(AUDIO_CTL_PLAYBACK1);
    AudioSmartPaController::getInstance()->setI2sOutHD(true);
    if (popcount(mStreamAttributeSource->output_devices) > 1) {
        mApTurnOnSequence2 = mHardwareResourceManager->getOutputTurnOnSeq(mStreamAttributeSource->output_devices,
                                                                          true, AUDIO_CTL_PLAYBACK1);
    }

    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence2);

    // scp spk IV data control path
    pcmIvIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture4);
    cardIvIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmCapture4);

    mApTurnOnSequence = AudioSmartPaController::getInstance()->getI2sSequence(
                                                               AUDIO_CTL_I2S_TO_CAPTURE4, true);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence);
    AudioSmartPaController::getInstance()->setI2sInHD(true);

    ALOGD("%s(), pcmDlIndex = %d cardDlIndex = %d pcmIvIndex = %d cardIvIndex = %d",
          __FUNCTION__, pcmDlIndex, cardDlIndex, pcmIvIndex, cardIvIndex);

    /* allocate the same with dsp platform drver */
    memcpy(&mScpSpkHwOutConfig, &mConfig, sizeof(struct pcm_config));
    memcpy(&mScpSpkHwInConfig, &mConfig, sizeof(struct pcm_config));
    mScpSpkHwInConfig.period_count = mScpSpkHwOutConfig.period_count / 2;
    mScpSpkHwInConfig.period_size = mScpSpkHwOutConfig.period_size;

    ALOGD("%s(), mScpSpkHwOutConfig:period_size = %d period_count = %d, mScpSpkHwInConfig:period_size = %d period_count = %d",
          __FUNCTION__,
          mScpSpkHwOutConfig.period_size, mScpSpkHwOutConfig.period_count,
          mScpSpkHwInConfig.period_size, mScpSpkHwInConfig.period_count);

    openScpSpkPcmDriver(pcmDlIndex, pcmIvIndex);

    ALOGV("-%s(),", __FUNCTION__);
#else
    ALOGE("%s() not support", __FUNCTION__);
#endif
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtectionDsp::closeScpSpkHwPcm() {

    if (mScpSpkDlHwPcm != NULL) {
        pcm_close(mScpSpkDlHwPcm);
        mScpSpkDlHwPcm = NULL;
    }

    if (mScpSpkIvHwPcm != NULL) {
        pcm_close(mScpSpkIvHwPcm);
        mScpSpkIvHwPcm = NULL;
    }

    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence2);

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_dl_scenario"), 0, false)) {
        ALOGE("Error: mtk_scp_spk_dl_scenario invalid value");
    }

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_iv_scenario"), 0, false)) {
        ALOGE("Error: mtk_scp_spk_iv_scenario invalid value");
    }

    ALOGV("-%s(), mScpSpkDlHwPcm = %p, mScpSpkIvHwPcm = %p",
          __FUNCTION__, mScpSpkDlHwPcm, mScpSpkIvHwPcm);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtectionDsp::open() {
    ALOGD("+%s(), flag %d, mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags,
          mStreamAttributeSource->output_devices);
    int pcmIndex, cardIndex;

    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

#if defined(MTK_AUDIO_KS)
    pcmIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmScpSpkPlayback);
    cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmScpSpkPlayback);
#else
    // acquire pmic clk
    mHardwareResourceManager->EnableAudBufClk(true);
    pcmIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmDl1SpkPlayback);
    cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmDl1SpkPlayback);
#endif

    //ListPcmDriver(cardindex, pcmindex);

#if defined(SCP_SMARTPA_KERNEL_BUF_SIZE)
    mStreamAttributeTarget.buffer_size = SCP_SMARTPA_KERNEL_BUF_SIZE;
#else
    mStreamAttributeTarget.buffer_size = AudioALSADeviceParser::getInstance()->GetPcmBufferSize(pcmIndex, PCM_OUT);
#endif

#ifdef PLAYBACK_USE_24BITS_ONLY
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
#else
    mStreamAttributeTarget.audio_format = (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT) ? AUDIO_FORMAT_PCM_8_24_BIT : AUDIO_FORMAT_PCM_16_BIT;
#endif
    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);

    mStreamAttributeTarget.sample_rate = ChooseTargetSampleRate(AudioALSASampleRateController::getInstance()->getPrimaryStreamOutSampleRate(),
                                                                mStreamAttributeSource->output_devices);

    ALOGD("%s(): pcmindex = %d, mStreamAttributeTarget.sample_rate = %d",
          __FUNCTION__, pcmIndex, mStreamAttributeTarget.sample_rate);

    // HW pcm config
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;
    mConfig.period_count = 4;
    mConfig.period_size = (mStreamAttributeTarget.buffer_size / (mConfig.channels * mConfig.period_count)) /
                          ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);
    mConfig.format = transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format);
    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;
    ALOGD("%s(), mConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mConfig.format);

    // debug pcm dump
    OpenPCMDump(LOG_TAG);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (get_aurisys_on() && !mStreamAttributeSource->isMixerOut) {
        CreateAurisysLibManager();
    } else
#endif
    {
        // post processing
        initPostProcessing();

        // SRCS_ESTSAMPLERATE
        initBliSrc();

        // bit conversion
        initBitConverter();

        initDataPending();
    }
    /* smartpa param*/
    initSmartPaConfig();

    setPcmDump(true);
    // init DC Removal
    initDcRemoval();

#if defined(MTK_AUDIO_KS)
    openScpSpkHwPcm();
#endif

    // open pcm driver
    openPcmDriver(pcmIndex);

    // open codec driver
    mHardwareResourceManager->startOutputDevice(mStreamAttributeSource->output_devices, mStreamAttributeTarget.sample_rate);

    mTimeStampValid = false;
    mBytesWriteKernel = 0;


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

int AudioALSAPlaybackHandlerSpeakerProtectionDsp::initSmartPaConfig() {
    ALOGD("%s", __FUNCTION__);
    AudioSmartPaParam *mAudioSmartPainstance = AudioSmartPaParam::getInstance();

    if (mAudioSmartPainstance == NULL) {
        return -1;
    }

    arsi_lib_config_t arsiLibConfig;
    arsiLibConfig.p_ul_buf_in = NULL;
    arsiLibConfig.p_ul_buf_out = NULL;
    arsiLibConfig.p_ul_ref_bufs = NULL;

    arsiLibConfig.p_dl_buf_in = NULL;
    arsiLibConfig.p_dl_buf_out = NULL;
    arsiLibConfig.p_dl_ref_bufs = NULL;

    /* lib */
    arsiLibConfig.sample_rate = mStreamAttributeTarget.sample_rate;
    arsiLibConfig.audio_format = AUDIO_FORMAT_PCM_32_BIT;
    arsiLibConfig.frame_size_ms = 0;

    /* output device */
    arsi_task_config_t ArsiTaskConfig;
    ArsiTaskConfig.output_device_info.devices = AUDIO_DEVICE_OUT_SPEAKER;
    ArsiTaskConfig.output_device_info.audio_format = AUDIO_FORMAT_PCM_32_BIT;
    ArsiTaskConfig.output_device_info.sample_rate = mStreamAttributeTarget.sample_rate;
    ArsiTaskConfig.output_device_info.channel_mask = AUDIO_CHANNEL_IN_STEREO;
    ArsiTaskConfig.output_device_info.num_channels = 2;
    ArsiTaskConfig.output_device_info.hw_info_mask = 0;

    /* task scene */
    ArsiTaskConfig.task_scene = TASK_SCENE_SPEAKER_PROTECTION;

    /* audio mode */
    ArsiTaskConfig.audio_mode = AudioALSAStreamManager::getInstance()->isModeInRingtone() ?
                                AUDIO_MODE_RINGTONE : AUDIO_MODE_NORMAL;

    /* max device capability for allocating memory */
    ArsiTaskConfig.max_output_device_sample_rate = mStreamAttributeTarget.sample_rate;
    ArsiTaskConfig.max_output_device_num_channels = 2;

    ALOGD("+%s(), output_device %d, sample_rate %d, audio_mode %d, max sample_rate %d",
        __FUNCTION__, ArsiTaskConfig.output_device_info.devices,
        ArsiTaskConfig.output_device_info.sample_rate,
        ArsiTaskConfig.audio_mode,
        ArsiTaskConfig.max_output_device_sample_rate);

    mAudioSmartPainstance->setArsiTaskConfig(&ArsiTaskConfig);
    mAudioSmartPainstance->setSmartpaParam();

    return 0;
}

status_t AudioALSAPlaybackHandlerSpeakerProtectionDsp::close() {
    ALOGD("+%s()", __FUNCTION__);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    mHardwareResourceManager->stopOutputDevice();

    // close pcm driver
    closePcmDriver();

#if defined(MTK_AUDIO_KS)
    closeScpSpkHwPcm();
#endif

    //DC removal
    deinitDcRemoval();

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (get_aurisys_on() && !mStreamAttributeSource->isMixerOut) {
        DestroyAurisysLibManager();
    } else
#endif
    {
        DeinitDataPending();

        // bit conversion
        deinitBitConverter();

        // SRC
        deinitBliSrc();

        // post processing
        deinitPostProcessing();
    }

    // debug pcm dump
    setPcmDump(false);
    ClosePCMDump();

    //release pmic clk
    mHardwareResourceManager->EnableAudBufClk(false);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtectionDsp::routing(const audio_devices_t output_devices) {
    mHardwareResourceManager->changeOutputDevice(output_devices);
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (mAudioFilterManagerHandler) { mAudioFilterManagerHandler->setDevice(output_devices); }
#endif
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtectionDsp::setScreenState(bool mode __unused, size_t buffer_size __unused,
                                                                      size_t reduceInterruptSize __unused,
                                                                      bool bforce __unused) {
    // don't increase irq period due to no enough data for SCP processing
    return NO_ERROR;
}

// here using android define format
unsigned int  AudioALSAPlaybackHandlerSpeakerProtectionDsp::GetSampleSize(unsigned int Format) {
    unsigned returnsize = 2;
    if (Format == AUDIO_FORMAT_PCM_16_BIT) {
        returnsize = 2;
    } else if (Format == AUDIO_FORMAT_PCM_32_BIT || Format == AUDIO_FORMAT_PCM_8_24_BIT) {
        returnsize = 4;
    } else if (Format == AUDIO_FORMAT_PCM_8_BIT) {
        returnsize = 1;
    } else {
        ALOGD("%s Format == %d", __FUNCTION__, Format);
    }
    return returnsize;
}

// here using android define format
unsigned int  AudioALSAPlaybackHandlerSpeakerProtectionDsp::GetFrameSize(unsigned int channels, unsigned int Format) {
    unsigned returnsize = 2;
    if (Format == AUDIO_FORMAT_PCM_16_BIT) {
        returnsize = 2;
    } else if (Format == AUDIO_FORMAT_PCM_32_BIT || Format == AUDIO_FORMAT_PCM_8_24_BIT) {
        returnsize = 4;
    } else if (Format == AUDIO_FORMAT_PCM_8_BIT) {
        returnsize = 1;
    } else {
        ALOGD("%s Format = %d", __FUNCTION__, Format);
    }
    returnsize *= channels;
    return returnsize;;
}

ssize_t AudioALSAPlaybackHandlerSpeakerProtectionDsp::write(const void *buffer, size_t bytes) {
    //ALOGD("%s(), buffer = %p, bytes = %zu", __FUNCTION__, buffer, bytes);

    if (mPcm == NULL) {
        ALOGE("%s(), mPcm == NULL, return", __FUNCTION__);
        return bytes;
    }

    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);
    ASSERT(pBuffer != NULL);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[0] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

    void *pBufferAfterDcRemoval = NULL;
    uint32_t bytesAfterDcRemoval = 0;
    // DC removal before DRC
    doDcRemoval(pBuffer, bytes, &pBufferAfterDcRemoval, &bytesAfterDcRemoval);

    doStereoToMonoConversionIfNeed(pBufferAfterDcRemoval, bytesAfterDcRemoval);

    void *pBufferAfterPending = NULL;
    uint32_t bytesAfterpending = 0;

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (get_aurisys_on() && !mStreamAttributeSource->isMixerOut) {
        // expect library output amount smoothly
        mTransferredBufferSize = GetTransferredBufferSize(
            bytesAfterDcRemoval,
            mStreamAttributeSource,
            &mStreamAttributeTarget);

        audio_pool_buf_copy_from_linear(
            mAudioPoolBufDlIn,
            pBufferAfterDcRemoval,
            bytesAfterDcRemoval);

        // post processing + SRC + Bit conversion
        aurisys_process_dl_only(mAurisysLibManager, mAudioPoolBufDlIn, mAudioPoolBufDlOut);

        // data pending: sram is device memory, need word size align 64 byte for 64 bit platform
        uint32_t data_size = audio_ringbuf_count(&mAudioPoolBufDlOut->ringbuf);
        if (data_size > mTransferredBufferSize) {
            data_size = mTransferredBufferSize;
        }
        data_size &= 0xFFFFFFC0;
        audio_pool_buf_copy_to_linear(
            &mLinearOut->p_buffer,
            &mLinearOut->memory_size,
            mAudioPoolBufDlOut,
            data_size);
        //ALOGD("aurisys process data_size: %u", data_size);

        // wrap to original playback handler
        pBufferAfterPending = (void *)mLinearOut->p_buffer;
        bytesAfterpending = data_size;
    } else
#endif
    {
        // post processing (can handle both Q1P16 and Q1P31 by audio_format_t)
        void *pBufferAfterPostProcessing = NULL;
        uint32_t bytesAfterPostProcessing = 0;
        status_t ret = doPostProcessing(pBufferAfterDcRemoval, bytesAfterDcRemoval, &pBufferAfterPostProcessing, &bytesAfterPostProcessing);
        if (ret != NO_ERROR) {
            ALOGW("%s(), No processed data output, don't write data to PCM(input bytes = %zu)", __FUNCTION__, bytes);
            return bytes;
        }

        // SRC
        void *pBufferAfterBliSrc = NULL;
        uint32_t bytesAfterBliSrc = 0;
        doBliSrc(pBufferAfterPostProcessing, bytesAfterPostProcessing, &pBufferAfterBliSrc, &bytesAfterBliSrc);

        // bit conversion
        void *pBufferAfterBitConvertion = NULL;
        uint32_t bytesAfterBitConvertion = 0;
        doBitConversion(pBufferAfterBliSrc, bytesAfterBliSrc, &pBufferAfterBitConvertion, &bytesAfterBitConvertion);

        // data pending
        pBufferAfterPending = NULL;
        bytesAfterpending = 0;
        dodataPending(pBufferAfterBitConvertion, bytesAfterBitConvertion, &pBufferAfterPending, &bytesAfterpending);
    }

    // pcm dump
    WritePcmDumpData(pBufferAfterPending, bytesAfterpending);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[1] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

    // write data to pcm driver
    int retval = pcm_write(mPcm, pBufferAfterPending, bytesAfterpending);
    mBytesWriteKernel = mBytesWriteKernel + bytesAfterpending;
    if (mTimeStampValid == false) {
        if (mBytesWriteKernel >= (mStreamAttributeTarget.buffer_size >> 1)) {
            mTimeStampValid = true;
        }
    }

    updateHardwareBufferInfo(bytes, bytesAfterpending);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[2] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

    if (retval != 0) {
        ALOGE("%s(), pcm_write() error, retval = %d", __FUNCTION__, retval);
    }

#ifdef DEBUG_LATENCY
    ALOGD("%s ::write (-) latency_in_us,%1.6lf,%1.6lf,%1.6lf", __FUNCTION__, latencyTime[0], latencyTime[1], latencyTime[2]);
#endif

    return bytes;
}

status_t AudioALSAPlaybackHandlerSpeakerProtectionDsp::setFilterMng(AudioMTKFilterManager *pFilterMng) {
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    ALOGD("+%s() mAudioFilterManagerHandler [%p]", __FUNCTION__, pFilterMng);
    mAudioFilterManagerHandler = pFilterMng;
#else
    (void *)pFilterMng;
#endif
    return NO_ERROR;
}

} // end of namespace android

#include "AudioALSAPlaybackHandlerDsp.h"
#include "AudioALSAHardwareResourceManager.h"
#include "AudioALSASampleRateController.h"
#if defined(MTK_AUDIO_KS)
#include "AudioALSADeviceConfigManager.h"
#endif

#include "AudioMTKFilter.h"
#include "AudioALSADeviceParser.h"
#include "AudioALSADriverUtility.h"
#include "AudioALSAStreamManager.h"

#include "AudioSmartPaController.h"


#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <aurisys_scenario_dsp.h>
#include <arsi_type.h>
#include <aurisys_config.h>
#endif

#ifdef MTK_LATENCY_DETECT_PULSE
#include "AudioDetectPulse.h"
#endif

#ifdef MTK_AUDIODSP_SUPPORT
#include "AudioDspStreamManager.h"
#include <audio_task.h>
#endif

#ifdef MTK_AUDIO_SCP_SUPPORT
#include <AudioMessengerIPI.h>
#endif


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerDsp"

// Latency Detect
//#define DEBUG_LATENCY
#ifdef DEBUG_LATENCY
#define THRESHOLD_FRAMEWORK   0.010
#define THRESHOLD_HAL         0.010
#define THRESHOLD_KERNEL      0.010
#endif

#ifndef KERNEL_BUFFER_SIZE_DL1_DATA2_NORMAL
#define KERNEL_BUFFER_SIZE_DL1_DATA2_NORMAL         KERNEL_BUFFER_SIZE_DL1_NORMAL
#endif

#ifndef KERNEL_BUFFER_SIZE_DL1_DATA2_HIFI_96K
#define KERNEL_BUFFER_SIZE_DL1_DATA2_HIFI_96K       KERNEL_BUFFER_SIZE_DL1_HIFI_96K
#endif

#ifndef KERNEL_BUFFER_SIZE_DL1_DATA2_HIFI_192K
#define KERNEL_BUFFER_SIZE_DL1_DATA2_HIFI_192K      KERNEL_BUFFER_SIZE_DL1_HIFI_192K
#endif

#define AUDIO_DSP_HW_MS_PERIOD (20)
#define AUDIO_DSP_HW_PERIOD_COUNT (2)


#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)
static const char* PROPERTY_KEY_EXTDAC = "vendor.audiohal.resource.extdac.support";
static const uint32_t kPcmDriverBufferSize = 0x20000; // 128k

namespace android {

AudioALSAPlaybackHandlerDsp::AudioALSAPlaybackHandlerDsp(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source),
    mDspHwPcm(NULL),
    mForceMute(false),
    mCurMuteBytes(0),
    mStartMuteBytes(0),
    mSupportNLE(false),
    mTaskScene(TASK_SCENE_INVALID),
    mAurisysDspConfig(NULL) {
    mPlaybackHandlerType = isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags) ?
                           PLAYBACK_HANDLER_DEEP_BUFFER : (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) ?
                           PLAYBACK_HANDLER_VOIP : PLAYBACK_HANDLER_NORMAL;

    ALOGD("%s() mPlaybackHandlerType = %d", __FUNCTION__, mPlaybackHandlerType);

    if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_FAST) {
        mPlaybackHandlerType = PLAYBACK_HANDLER_FAST;
    }

    memset((void *)&mNewtime, 0, sizeof(mNewtime));
    memset((void *)&mOldtime, 0, sizeof(mOldtime));
    memset(&mDsphwConfig, 0, sizeof(mDsphwConfig));

    if (!(platformIsolatedDeepBuffer()) ||
        (platformIsolatedDeepBuffer() &&
         mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_DEEP_BUFFER)) {
        mSupportNLE = true;
    } else {
        mSupportNLE = false;
    }

    mPCMDumpFileDSP = NULL;
    mDspStreamManager = AudioDspStreamManager::getInstance();
    ASSERT(mDspStreamManager != NULL);
}

AudioALSAPlaybackHandlerDsp::~AudioALSAPlaybackHandlerDsp() {
    ALOGD("%s()", __FUNCTION__);
}

uint32_t AudioALSAPlaybackHandlerDsp::getLowJitterModeSampleRate() {
    return 48000;
}

int AudioALSAPlaybackHandlerDsp::setAfeDspShareMem(unsigned int flag, bool condition) {
    mDspStreamManager->setAfeOutDspShareMem(flag, condition);
    return 0;
}

int AudioALSAPlaybackHandlerDsp::setDspRuntimeEn(bool condition) {
    if (isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags)) {
        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "dsp_deepbuf_runtime_en"), 0, condition)) {
            ALOGW("%s(), enable fail", __FUNCTION__);
            return -1;
        }
    } else if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "dsp_voipdl_runtime_en"), 0, condition)) {
            ALOGW("%s(), enable fail", __FUNCTION__);
            return -1;
        }
    } else {
        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "dsp_primary_runtime_en"), 0, condition)) {
            ALOGW("%s(), enable fail", __FUNCTION__);
            return -1;
        }
    }
    return 0;
}

bool AudioALSAPlaybackHandlerDsp::deviceSupportHifi(audio_devices_t outputdevice) {
    // modify this to let output device support hifi audio
    if (outputdevice == AUDIO_DEVICE_OUT_WIRED_HEADSET || outputdevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        return true;
    } else {
        return false;
    }
}

uint32_t AudioALSAPlaybackHandlerDsp::chooseTargetSampleRate(uint32_t SampleRate, audio_devices_t outputdevice) {
    ALOGV("ChooseTargetSampleRate SampleRate = %d outputdevice = %d", SampleRate, outputdevice);
    uint32_t TargetSampleRate = 48000;
#if defined(MTK_HIFIAUDIO_SUPPORT)
    bool hifi_enable = mHardwareResourceManager->getHiFiStatus();
    bool device_support_hifi = deviceSupportHifi(outputdevice);
    ALOGD("%s() hifi_enable = %d device_support_hifi = %d, PrimarySampleRate = %d",
          __FUNCTION__, hifi_enable, device_support_hifi, SampleRate);

    if (hifi_enable && device_support_hifi) {
        TargetSampleRate = SampleRate;
    }
#else
    if (SampleRate <=  192000 && SampleRate > 96000 && deviceSupportHifi(outputdevice)) {
        TargetSampleRate = 192000;
    } else if (SampleRate <= 96000 && SampleRate > 48000 && deviceSupportHifi(outputdevice)) {
        TargetSampleRate = 96000;
    } else if (SampleRate <= 48000 && SampleRate >= 32000) {
        TargetSampleRate = SampleRate;
    }
#endif
    return TargetSampleRate;
}

String8 AudioALSAPlaybackHandlerDsp::getPlaybackTurnOnSequence(unsigned int turnOnSeqType,
                                                               const char *playbackSeq) {
    String8 sequence = String8();
    bool isSpk = (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_SPEAKER);
    bool isSmartPA = AudioSmartPaController::getInstance()->isSmartPAUsed();
    bool isADSPPlayback = AudioDspStreamManager::getInstance()->getDspPlaybackEnable();
    bool isADSPA2dpUsed = (AudioDspStreamManager::getInstance()->getDspA2DPEnable() == true) &&
                          (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_ALL_A2DP);
    ASSERT(playbackSeq != NULL);

    switch (turnOnSeqType) {
    case TURN_ON_SEQUENCE_1:
        if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
            if (isSpk && isADSPPlayback) {
                sequence = String8(playbackSeq) + AUDIO_CTL_ADSP_UL;
            } else {
                sequence = mHardwareResourceManager->getOutputTurnOnSeq(mStreamAttributeSource->output_devices,
                                                                        false, playbackSeq);
            }
        } else {
            if (isADSPPlayback) {
                sequence = String8(playbackSeq) + AUDIO_CTL_ADSP_UL;
            } else {
                sequence = mHardwareResourceManager->getOutputTurnOnSeq(mStreamAttributeSource->output_devices,
                                                                        false, playbackSeq);
            }
        }
        break;
    case TURN_ON_SEQUENCE_2:
        if (popcount(mStreamAttributeSource->output_devices) > 1) {
            if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
                if (isSpk && isSmartPA && !isADSPPlayback) {
                    sequence = mHardwareResourceManager->getOutputTurnOnSeq(mStreamAttributeSource->output_devices,
                                                                           true, playbackSeq);
                }
            } else {
                if ((isSpk && isSmartPA) || isADSPPlayback) {
                    sequence = mHardwareResourceManager->getOutputTurnOnSeq(mStreamAttributeSource->output_devices,
                                                                            true, playbackSeq);
                }
            }
        }
        break;
    case TURN_ON_SEQUENCE_3:
        if (((mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) == 0) &&
            !isADSPPlayback && isADSPA2dpUsed) {
            sequence = String8(playbackSeq) + AUDIO_CTL_ADSP_UL;
        }
        break;
    case TURN_ON_SEQUENCE_DSP:
        if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
            if (isSpk && isADSPPlayback) {
                sequence = String8(playbackSeq) + AUDIO_CTL_DSPDL;
            }
        } else {
            if (isADSPPlayback) {
                sequence = String8(playbackSeq) + AUDIO_CTL_DSPDL;
            } else {
                if (!isSpk && !isSmartPA && strcmp(playbackSeq, AUDIO_CTL_PLAYBACK3) == 0) {
                    sequence = String8(playbackSeq) + AUDIO_CTL_DSPDL;
                }

                if (isADSPA2dpUsed) {
                    sequence = String8(playbackSeq) + AUDIO_CTL_DSPDL;
                }
            }
        }
        break;
    default:
        ASSERT(0);
        break;
    }

    return sequence;
}

status_t AudioALSAPlaybackHandlerDsp::openDspHwPcm() {
    int pcmindex = -1, cardindex = 0, ret = 0;
    unsigned int pcmmaxsize, pcmconfigsize;
    struct pcm_params *params = NULL;

    ALOGV("+%s(),", __FUNCTION__);

#if defined(MTK_AUDIO_KS)
    String8 playbackSeq = String8();
    if (isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags)) {
        pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback3);
        if (pcmindex < 0) {
            // use playback 2 if this platform does not have playback 3
            pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback2);
            cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback2);
            playbackSeq = String8(AUDIO_CTL_PLAYBACK2);
        } else {
            cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback3);
            playbackSeq = String8(AUDIO_CTL_PLAYBACK3);
        }

        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "deep_buffer_scenario"), 0, 1)) {
            ALOGW("%s(), deep_buffer_scenario enable fail", __FUNCTION__);
        }
    } else if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
        // voip dl using DL12 , if not support using DL3
        pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback12);
        if(pcmindex < 0) {
            pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback3);
            cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback3);
            playbackSeq = String8(AUDIO_CTL_PLAYBACK3);
        } else {
            cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback12);
            playbackSeq = String8(AUDIO_CTL_PLAYBACK12);
        }
    } else {
        pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback1);
        cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback1);
        playbackSeq = String8(AUDIO_CTL_PLAYBACK1);
    }

    mApTurnOnSequence = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_1, playbackSeq);
    mApTurnOnSequence2 = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_2, playbackSeq);
    if ((mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) == 0) {
        mApTurnOnSequence3 = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_3, playbackSeq);
    }
    mApTurnOnSequenceDsp = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_DSP, playbackSeq);
    mHardwareResourceManager->setCustOutputDevTurnOnSeq(mStreamAttributeSource->output_devices,
                                                        mTurnOnSeqCustDev1, mTurnOnSeqCustDev2);

    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence2);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequenceDsp);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence3);
    mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev1);
    mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev2);
#else
    String8 pcmPath = isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags) ?
                      keypcmDL1DATA2PLayback : keypcmI2S0Dl1Playback;

    pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(pcmPath);
    cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(pcmPath);
#endif

    ALOGD("%s(), pcmindex = %d cardindex = %d ", __FUNCTION__, pcmindex, cardindex);

    /* allocate the same with dsp platform drver */
    mDsphwConfig.period_size = mConfig.period_size;
    mDsphwConfig.period_count = mConfig.period_count;
    mDsphwConfig.channels = mStreamAttributeTarget.num_channels;
    mDsphwConfig.rate = mStreamAttributeTarget.sample_rate;
    mDsphwConfig.format = transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format);

    pcmconfigsize = mDsphwConfig.period_count * mDsphwConfig.period_size * mDsphwConfig.channels * (pcm_format_to_bits(mDsphwConfig.format) / 8);

    ALOGD("%s(), flag %d, mDevice = 0x%x, mDsphwConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags, mStreamAttributeSource->output_devices,
          mDsphwConfig.channels, mDsphwConfig.rate, mDsphwConfig.period_size, mDsphwConfig.period_count, mDsphwConfig.format);

    mDsphwConfig.start_threshold = (mDsphwConfig.period_count * mDsphwConfig.period_size);
    if (mStreamAttributeSource->mAudioInputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
        mDsphwConfig.start_threshold = ((mDsphwConfig.period_count - 1) * mDsphwConfig.period_size);
    }
    mDsphwConfig.stop_threshold = ~(0U);
    mDsphwConfig.silence_threshold = 0;

    opeDspPcmDriver(pcmindex);
    if (pcm_start(mDspHwPcm) != 0) {
        ALOGE("%s(), pcm_start(%p) == false due to %s", __FUNCTION__, mDspHwPcm, pcm_get_error(mDspHwPcm));
    }
    ALOGV("-%s(),", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerDsp::open() {
    ALOGV("+%s(), flag %d, mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags,
          mStreamAttributeSource->output_devices);

    struct pcm_params *params = NULL;
    int dspPcmIndex, dspCardIndex = 0;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    uint32_t aurisys_scenario = 0xFFFFFFFF;
    uint8_t arsi_process_type = ARSI_PROCESS_TYPE_DL_ONLY;
#endif
    unsigned int feature_id = getDspFeatureID(mStreamAttributeSource->mAudioOutputFlags);

    setAfeDspShareMem(mStreamAttributeSource->mAudioOutputFlags, true);
    setDspRuntimeEn(true);
    mAudioMessengerIPI->registerAdspFeature(feature_id);

    if (isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags)) {
        mTaskScene = TASK_SCENE_DEEPBUFFER;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
        aurisys_scenario = AURISYS_SCENARIO_DSP_DEEP_BUF;
#endif
    } else if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
        mTaskScene = TASK_SCENE_VOIP;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
        aurisys_scenario = AURISYS_SCENARIO_DSP_VOIP;
        arsi_process_type = ARSI_PROCESS_TYPE_DL_ONLY; //ARSI_PROCESS_TYPE_UL_AND_DL;
#endif
    } else {
        mTaskScene = TASK_SCENE_PRIMARY;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
        aurisys_scenario = AURISYS_SCENARIO_DSP_PRIMARY;
#endif
    }
    mAudioMessengerIPI->registerDmaCbk(
        mTaskScene,
        0x10000,
        0x48000,
        processDmaMsgWrapper,
        this);


    // debug pcm dump
    OpenPCMDump(LOG_TAG);

    if (isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags)) {
        dspPcmIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlaybackDspDeepbuf);
        dspCardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlaybackDspDeepbuf);
        ALOGD("%s(), dspPcmIndex = %d dspCardIndex = %d deep buffer",
              __FUNCTION__, dspPcmIndex, dspCardIndex);
    } else if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
        dspPcmIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlaybackDspVoip);
        dspCardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlaybackDspVoip);
        ALOGD("%s(), dspPcmIndex = %d dspCardIndex = %d voip dl",
              __FUNCTION__, dspPcmIndex, dspCardIndex);

    } else {
        dspPcmIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlaybackDspprimary);
        dspCardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlaybackDspprimary);
        ALOGD("%s(), dspPcmIndex = %d dspCardIndex = %d",
              __FUNCTION__, dspPcmIndex, dspCardIndex);
    }

    // HW attribute config
#ifdef PLAYBACK_USE_24BITS_ONLY
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
#else
    mStreamAttributeTarget.audio_format = (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT) ? AUDIO_FORMAT_PCM_8_24_BIT : AUDIO_FORMAT_PCM_16_BIT;
#endif

    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);

    mStreamAttributeTarget.sample_rate = chooseTargetSampleRate(AudioALSASampleRateController::getInstance()->getPrimaryStreamOutSampleRate(),
                                                                mStreamAttributeSource->output_devices);

    mStreamAttributeTarget.output_devices = mStreamAttributeSource->output_devices;
    mStreamAttributeTarget.mAudioOutputFlags = mStreamAttributeSource->mAudioOutputFlags;

#ifdef HIFI_DEEP_BUFFER
    if (mStreamAttributeTarget.sample_rate <= 48000) {
        mStreamAttributeTarget.buffer_size = isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags) ?
                                             KERNEL_BUFFER_SIZE_DL1_DATA2_NORMAL :
                                             KERNEL_BUFFER_SIZE_DL1_NORMAL;
#if defined(MTK_HYBRID_NLE_SUPPORT)
#ifdef PLAYBACK_USE_24BITS_ONLY
#define KERNEL_BUFFER_SIZE_WITH_DRE  (40 * 1024) /* 40KB for 32bit hal */
#else
#define KERNEL_BUFFER_SIZE_WITH_DRE  (20 * 1024) /* 20KB for 16bit hal */
#endif
        if (mSupportNLE && (mStreamAttributeTarget.buffer_size < KERNEL_BUFFER_SIZE_WITH_DRE)) {
            mStreamAttributeTarget.buffer_size = KERNEL_BUFFER_SIZE_WITH_DRE;
        }
#endif
    } else if (mStreamAttributeTarget.sample_rate > 48000 && mStreamAttributeTarget.sample_rate <= 96000) {
        uint32_t hifi_buffer_size = isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags) ?
                                    KERNEL_BUFFER_SIZE_DL1_DATA2_HIFI_96K :
                                    KERNEL_BUFFER_SIZE_DL1_HIFI_96K;

        if (mStreamAttributeTarget.buffer_size >= hifi_buffer_size) {
            mStreamAttributeTarget.buffer_size = hifi_buffer_size;
        }
    }  else {
        uint32_t hifi_buffer_size = isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags) ?
                                    KERNEL_BUFFER_SIZE_DL1_DATA2_HIFI_192K :
                                    KERNEL_BUFFER_SIZE_DL1_HIFI_192K;

        if (mStreamAttributeTarget.buffer_size >= hifi_buffer_size) {
            mStreamAttributeTarget.buffer_size = hifi_buffer_size;
        }
    }
#endif  /* end of #ifdef HIFI_DEEP_BUFFER */

    //Change hwbuffer size in Comminuication
    if (!(platformIsolatedDeepBuffer()) &&
        mStreamAttributeSource->audio_mode == AUDIO_MODE_IN_COMMUNICATION) {
        mStreamAttributeTarget.buffer_size = 2 * mStreamAttributeSource->buffer_size /
                                             ((mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4) *
                                             ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);
    }

    // HW pcm config
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = mStreamAttributeSource->num_channels;
    mConfig.rate = mStreamAttributeSource->sample_rate;

    if (isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags)) {
        mConfig.period_count = 2;
        mConfig.period_size = (mStreamAttributeTarget.buffer_size / (mConfig.channels * mConfig.period_count)) / ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);
    } else {
        mConfig.period_count = 4;
        mConfig.period_size = (mStreamAttributeTarget.buffer_size / (mConfig.channels * mConfig.period_count)) / ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);
    }
    mConfig.format = transferAudioFormatToPcmFormat(mStreamAttributeSource->audio_format);

    mConfig.start_threshold = (mConfig.period_count * mConfig.period_size);
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;
    ALOGD("%s(), flag %d, mDevice = 0x%x, mConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d avail_min = %d",
          __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags, mStreamAttributeSource->output_devices,
          mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mConfig.format,  mConfig.avail_min);

    mStreamAttributeTarget.stream_type = mStreamAttributeSource->stream_type;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    mDspStreamManager->CreateAurisysLibManager(
        &mAurisysLibManager,
        &mAurisysDspConfig,
        mTaskScene,
        aurisys_scenario,
        arsi_process_type,
        mStreamAttributeSource->audio_mode,
        mStreamAttributeSource,
        &mStreamAttributeTarget,
        NULL,
        NULL);
#endif

    // open pcm driver
    openPcmDriver(dspPcmIndex);

    openDspHwPcm();

#ifdef MTK_AUDIODSP_SUPPORT
    mDspStreamManager->addPlaybackHandler(this);
    OpenPCMDumpDSP(LOG_TAG, mTaskScene);
#endif

#if defined(MTK_HYBRID_NLE_SUPPORT) // must be after pcm open
    mStreamAttributeTarget.output_devices = mStreamAttributeSource->output_devices;
    initNLEProcessing();
#endif

    // open codec driver
    mHardwareResourceManager->startOutputDevice(mStreamAttributeSource->output_devices, mStreamAttributeTarget.sample_rate);

    //===========================================

    mTimeStampValid = false;
    mBytesWriteKernel = 0;
    ALOGV("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerDsp::close() {
    ALOGD("+%s(), flag %d, mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags,
          mStreamAttributeSource->output_devices);
    unsigned int feature_id = getDspFeatureID(mStreamAttributeSource->mAudioOutputFlags);

#if defined(MTK_HYBRID_NLE_SUPPORT)
    // Must do this before close analog path
    deinitNLEProcessing();
#endif

    // close codec driver
    mHardwareResourceManager->stopOutputDevice();
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev1);
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev2);

#ifdef MTK_AUDIODSP_SUPPORT
    mDspStreamManager->removePlaybackHandler(this);
#endif

    // close pcm driver
    closeDspPcmDriver();
    closePcmDriver();

    setAfeDspShareMem(mStreamAttributeSource->mAudioOutputFlags, false);
    setDspRuntimeEn(false);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    mDspStreamManager->DestroyAurisysLibManager(
        &mAurisysLibManager, &mAurisysDspConfig, mTaskScene);
#endif
    mAudioMessengerIPI->deregisterDmaCbk(mTaskScene);
    mAudioMessengerIPI->deregisterAdspFeature(feature_id);

#if defined(MTK_AUDIO_KS)
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequenceDsp);
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence2);
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence3);

    if (isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags)) {
        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "deep_buffer_scenario"), 0, 0)) {
            ALOGW("%s(), deep_buffer_scenario disable fail", __FUNCTION__);
        }
    }
#endif

    // debug pcm dump
    ClosePCMDump();
#ifdef MTK_AUDIODSP_SUPPORT
    ClosePCMDumpDSP(mTaskScene);
#endif

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerDsp::openDspPcmDriverWithFlag(const unsigned int device, unsigned int flag) {
    ALOGD("+%s(), mDspHwPcm device = %d, flag = 0x%x", __FUNCTION__, device, flag);

    ASSERT(mDspHwPcm == NULL);
    mDspHwPcm = pcm_open(AudioALSADeviceParser::getInstance()->GetCardIndex(),
                         device, flag, &mDsphwConfig);
    if (mDspHwPcm == NULL) {
        ALOGE("%s(), mDspHwPcm == NULL!!", __FUNCTION__);
    } else if (pcm_is_ready(mDspHwPcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mDspHwPcm, pcm_get_error(mDspHwPcm));
        pcm_close(mDspHwPcm);
        mDspHwPcm = NULL;
    } else if (pcm_prepare(mDspHwPcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mDspHwPcm, pcm_get_error(mDspHwPcm));
        pcm_close(mDspHwPcm);
        mDspHwPcm = NULL;
    }

    ASSERT(mDspHwPcm != NULL);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerDsp::opeDspPcmDriver(const unsigned int device) {
    return openDspPcmDriverWithFlag(device, PCM_OUT | PCM_MONOTONIC);
}

status_t AudioALSAPlaybackHandlerDsp::closeDspPcmDriver() {

    if (mDspHwPcm != NULL) {
        pcm_stop(mDspHwPcm);
        pcm_close(mDspHwPcm);
        mDspHwPcm = NULL;
    }

    ALOGV("-%s(), mDspHwPcm = %p", __FUNCTION__, mDspHwPcm);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerDsp::routing(const audio_devices_t output_devices) {
    mHardwareResourceManager->changeOutputDevice(output_devices);
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (mAudioFilterManagerHandler) { mAudioFilterManagerHandler->setDevice(output_devices); }
#endif
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerDsp::setScreenState(bool mode, size_t buffer_size, size_t reduceInterruptSize, bool bforce __unused) {
    // don't increase irq period when play hifi
    if (mode == 0 && mStreamAttributeSource->sample_rate > 48000) {
        return NO_ERROR;
    }

    ALOGD("%s, flag %d, %f, mode = %d , buffer_size = %zu, channel %d, format%d reduceInterruptSize = %zu",
          __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags,
          mStreamAttributeTarget.mInterrupt, mode, buffer_size, mConfig.channels,
          mStreamAttributeTarget.audio_format, reduceInterruptSize);

    return NO_ERROR;
}

ssize_t AudioALSAPlaybackHandlerDsp::write(const void *buffer, size_t bytes) {
    ALOGV("%s(), buffer = %p, bytes = %zu", __FUNCTION__, buffer, bytes);

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
    // stereo to mono for speaker
    doStereoToMonoConversionIfNeed(pBuffer, bytes);

    // pcm dump
    WritePcmDumpData(pBuffer, bytes);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[1] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

#ifdef MTK_LATENCY_DETECT_PULSE
    AudioDetectPulse::doDetectPulse(TAG_PLAYERBACK_HANDLER, PULSE_LEVEL, 0, (void *)pBuffer,
                                    bytes, mStreamAttributeTarget.audio_format,
                                    mStreamAttributeTarget.num_channels, mStreamAttributeTarget.sample_rate);
#endif

    // write data to pcm driver
    int retval = pcm_write(mPcm, pBuffer, bytes);

    mBytesWriteKernel = mBytesWriteKernel + bytes;
    if (mTimeStampValid == false) {
        if (mBytesWriteKernel >= (mStreamAttributeTarget.buffer_size >> 1)) {
            mTimeStampValid = true;
        }
    }
#if defined(MTK_HYBRID_NLE_SUPPORT)
    if (mSupportNLE) {
        doNLEProcessing(pBuffer, bytes);
    }
#endif

    updateHardwareBufferInfo(bytes, bytes);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[2] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

    if (retval != 0) {
        ALOGE("%s(), pcm_write() error, retval = %d", __FUNCTION__, retval);
    }

#ifdef DEBUG_LATENCY
    if (latencyTime[0] > THRESHOLD_FRAMEWORK || latencyTime[1] > THRESHOLD_HAL || latencyTime[2] > (mStreamAttributeTarget.mInterrupt - latencyTime[0] - latencyTime[1] + THRESHOLD_KERNEL)) {
        ALOGD("latency_in_s,%1.3lf,%1.3lf,%1.3lf, interrupt,%1.3lf,byte:%u", latencyTime[0], latencyTime[1], latencyTime[2], mStreamAttributeTarget.mInterrupt, bytesAfterpending);
    }
#endif

    return bytes;
}

status_t AudioALSAPlaybackHandlerDsp::setFilterMng(AudioMTKFilterManager *pFilterMng) {
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    ALOGD("+%s() mAudioFilterManagerHandler [%p]", __FUNCTION__, pFilterMng);
    mAudioFilterManagerHandler = pFilterMng;
#else
    (void *)pFilterMng;
#endif
    return NO_ERROR;
}

} // end of namespace android

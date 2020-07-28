#include <inttypes.h>

#include "AudioALSAPlaybackHandlerNormal.h"

#include "AudioALSAHardwareResourceManager.h"
//#include "AudioALSAVolumeController.h"
//#include "AudioVolumeInterface.h"
#include "AudioVolumeFactory.h"
#include "AudioALSASampleRateController.h"
#if defined(MTK_AUDIO_KS)
#include "AudioALSADeviceConfigManager.h"
#endif

#include "AudioMTKFilter.h"
#include "AudioALSADeviceParser.h"
#include "AudioALSADriverUtility.h"
#include "AudioSmartPaController.h"

#undef MTK_HDMI_SUPPORT

#if defined(MTK_HDMI_SUPPORT)
#include "AudioExtDisp.h"
#endif

#include "AudioSmartPaController.h"

#include "AudioALSAStreamManager.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <audio_ringbuf.h>
#include <audio_pool_buf_handler.h>

#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>
#endif

#ifdef MTK_LATENCY_DETECT_PULSE
#include "AudioDetectPulse.h"
#endif

#if defined(MTK_AUDIODSP_SUPPORT)
#include "AudioDspStreamManager.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerNormal"

// define this to enable mmap playback.
//#define PLAYBACK_MMAP

#ifdef DEBUG_LATENCY
// Latency Detect
//#define DEBUG_LATENCY
#define THRESHOLD_FRAMEWORK   0.010
#define THRESHOLD_HAL         0.010
#define THRESHOLD_KERNEL      0.010
#endif

#ifndef KERNEL_BUFFER_SIZE_DL1_DATA2_NORMAL
#define KERNEL_BUFFER_SIZE_DL1_DATA2_NORMAL         KERNEL_BUFFER_SIZE_DL1_NORMAL
#endif

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)
static const char *PROPERTY_KEY_EXTDAC = "vendor.audiohal.resource.extdac.support";

static const uint32_t kPcmDriverBufferSize = 0x20000; // 128k

namespace android {

AudioALSAPlaybackHandlerNormal::AudioALSAPlaybackHandlerNormal(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source),
    mHpImpeDancePcm(NULL),
    mForceMute(false),
    mCurMuteBytes(0),
    mStartMuteBytes(0),
    mAllZeroBlock(NULL) {
    ALOGV("%s()", __FUNCTION__);

    mPlaybackHandlerType = isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags) ?
                           PLAYBACK_HANDLER_DEEP_BUFFER : PLAYBACK_HANDLER_NORMAL;

    if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_FAST) {
        mPlaybackHandlerType = PLAYBACK_HANDLER_FAST;
    } else if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
        mPlaybackHandlerType = PLAYBACK_HANDLER_VOIP;
    }

    memset((void *)&mNewtime, 0, sizeof(mNewtime));
    memset((void *)&mOldtime, 0, sizeof(mOldtime));
    memset((void *)&mHpImpedanceConfig, 0, sizeof(mHpImpedanceConfig));

    if (!(platformIsolatedDeepBuffer()) ||
        (platformIsolatedDeepBuffer() &&
         mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_DEEP_BUFFER)) {
        mSupportNLE = true;
    } else {
        mSupportNLE = false;
    }

}


AudioALSAPlaybackHandlerNormal::~AudioALSAPlaybackHandlerNormal() {
}

uint32_t AudioALSAPlaybackHandlerNormal::GetLowJitterModeSampleRate() {
    return 48000;
}

bool AudioALSAPlaybackHandlerNormal::SetLowJitterMode(bool bEnable, uint32_t SampleRate) {
    ALOGV("%s() bEanble = %d SampleRate = %u", __FUNCTION__, bEnable, SampleRate);
#if !defined(MTK_AUDIO_KS)
    enum mixer_ctl_type type;
    struct mixer_ctl *ctl;
    int retval = 0;

    // check need open low jitter mode
    if (SampleRate <= GetLowJitterModeSampleRate() && (AudioALSADriverUtility::getInstance()->GetPropertyValue(PROPERTY_KEY_EXTDAC)) == false) {
        ALOGD("%s(), bypass low jitter mode, bEnable = %d, device = 0x%x, SampleRate = %u",
              __FUNCTION__, bEnable, mStreamAttributeSource->output_devices, SampleRate);
        return false;
    } else {
        ALOGD("%s() bEanble = %d, SampleRate = %u, use %s",
              __FUNCTION__, bEnable, SampleRate,
              isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags) ?
              "deep_buffer_dl_hd_Switch" : "Audio_I2S0dl1_hd_Switch");
    }

    if (isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags)) {
        ctl = mixer_get_ctl_by_name(mMixer, "deep_buffer_dl_hd_Switch");
    } else {
        ctl = mixer_get_ctl_by_name(mMixer, "Audio_I2S0dl1_hd_Switch");
    }

    if (ctl == NULL) {
        ALOGE("mixer control not support");
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

bool AudioALSAPlaybackHandlerNormal::DeviceSupportHifi(audio_devices_t outputdevice) {
    // modify this to let output device support hifi audio
    if (outputdevice == AUDIO_DEVICE_OUT_WIRED_HEADSET || outputdevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        return true;
    }
    /*
          else if (outputdevice & AUDIO_DEVICE_OUT_SPEAKER) {
            if (AudioSmartPaController::getInstance()->isSmartPAUsed()) {
                return AudioSmartPaController::getInstance()->getMaxSupportedRate() > 48000;
            } else {
                return false;
            }
        }
    */
    return false;
}


uint32_t AudioALSAPlaybackHandlerNormal::ChooseTargetSampleRate(uint32_t SampleRate, audio_devices_t outputdevice) {
    ALOGV("ChooseTargetSampleRate SampleRate = %d outputdevice = %d", SampleRate, outputdevice);
    uint32_t TargetSampleRate = 48000;
#if defined(MTK_HIFIAUDIO_SUPPORT)
    bool hifi_enable = mHardwareResourceManager->getHiFiStatus();
    bool device_support_hifi = DeviceSupportHifi(outputdevice);
    ALOGD("%s() hifi_enable = %d device_support_hifi = %d, PrimarySampleRate = %d",
          __FUNCTION__, hifi_enable, device_support_hifi, SampleRate);

    if (hifi_enable && device_support_hifi) {
        TargetSampleRate = SampleRate;
    }
#else
    if (SampleRate <=  192000 && SampleRate > 96000 && DeviceSupportHifi(outputdevice)) {
        TargetSampleRate = 192000;
    } else if (SampleRate <= 96000 && SampleRate > 48000 && DeviceSupportHifi(outputdevice)) {
        TargetSampleRate = 96000;
    } else if (SampleRate <= 48000 && SampleRate >= 32000) {
        TargetSampleRate = SampleRate;
    }
#endif
    return TargetSampleRate;
}
#if defined(MTK_HIFIAUDIO_SUPPORT)

#ifndef KERNEL_BUFFER_SIZE_HIFI
#define KERNEL_BUFFER_SIZE_HIFI      KERNEL_BUFFER_SIZE_IN_HIFI_MODE
#endif

uint32_t AudioALSAPlaybackHandlerNormal::UpdateKernelBufferSize(audio_devices_t outputdevice) {
    uint32_t target_size = KERNEL_BUFFER_SIZE_DL1_NORMAL;
    bool hifi_enable = mHardwareResourceManager->getHiFiStatus();
    bool device_support_hifi = DeviceSupportHifi(outputdevice);
    uint32_t flag_support_hifi = isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags) ||
                                 (mStreamAttributeSource->mAudioOutputFlags == AUDIO_OUTPUT_FLAG_PRIMARY);

    ALOGD("%s(), hifi_enable = %d device_support_hifi(device:%d) = %d, flag_support_hifi(flag:%d) = %d",
          __FUNCTION__, hifi_enable, outputdevice, device_support_hifi,
          mStreamAttributeSource->mAudioOutputFlags, flag_support_hifi);

    if (hifi_enable && device_support_hifi && flag_support_hifi) {
        target_size = KERNEL_BUFFER_SIZE_HIFI;
        ALOGD("%s() update kernel BufferSize = %d", __FUNCTION__, target_size);
    }

    return target_size;
}
#endif

status_t AudioALSAPlaybackHandlerNormal::open() {
    ALOGD("+%s(), flag = %d, source output_devices = 0x%x, audio_format = %x, buffer_size = %d, sample_rate = %d",
          __FUNCTION__,
          mStreamAttributeSource->mAudioOutputFlags,
          mStreamAttributeSource->output_devices,
          mStreamAttributeSource->audio_format,
          mStreamAttributeSource->buffer_size,
          mStreamAttributeSource->sample_rate);

    AL_LOCK_MS(AudioALSADriverUtility::getInstance()->getStreamSramDramLock(), 3000);
    int pcmindex = 0;
    int cardindex = 0;

    // acquire pmic clk
    mHardwareResourceManager->EnableAudBufClk(true);

#if defined(MTK_AUDIO_KS)
    String8 playbackSeq = String8();
    if (isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags)) {

        ALOGD("%s(), isolated deep buffer keypcmDeepBuffer = %s", __FUNCTION__, keypcmDeepBuffer.string());

        pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmDeepBuffer);
        cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmDeepBuffer);

        // use playback 2
        if (keypcmDeepBuffer.compare(keypcmPlayback2) == 0) {
            playbackSeq = String8(AUDIO_CTL_PLAYBACK2);
        } else {
            playbackSeq = String8(AUDIO_CTL_PLAYBACK3);
        }

        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "deep_buffer_scenario"), 0, 1)) {
            ALOGW("%s(), deep_buffer_scenario enable fail", __FUNCTION__);
        }
    } else if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
        pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback12);
        cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback12);
        playbackSeq = String8(AUDIO_CTL_PLAYBACK12);
    } else {
        pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback1);
        cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback1);
        playbackSeq = String8(AUDIO_CTL_PLAYBACK1);
    }

    mApTurnOnSequence = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_1, playbackSeq);
    mApTurnOnSequence2 = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_2, playbackSeq);
#if defined(MTK_AUDIODSP_SUPPORT)
    mApTurnOnSequence3 = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_3, playbackSeq);
    mApTurnOnSequenceDsp = getPlaybackTurnOnSequence(TURN_ON_SEQUENCE_DSP, playbackSeq);
#endif
    mHardwareResourceManager->setCustOutputDevTurnOnSeq(mStreamAttributeSource->output_devices,
                                                        mTurnOnSeqCustDev1, mTurnOnSeqCustDev2);

    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence2);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequenceDsp);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence3);
    mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev1);
    mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev2);
#else
    /* not MTK_AUDIO_KS */
    String8 pcmPath = isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags) ?
                      keypcmDL1DATA2PLayback : keypcmI2S0Dl1Playback;

    pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(pcmPath);
    cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(pcmPath);
#endif

    //ListPcmDriver(cardindex, pcmindex);

    struct pcm_params *params;
    params = pcm_params_get(cardindex, pcmindex,  PCM_OUT);
    if (params == NULL) {
        ALOGD("Device does not exist.\n");
    }
    mStreamAttributeTarget.buffer_size = pcm_params_get_max(params, PCM_PARAM_BUFFER_BYTES);
    pcm_params_free(params);

    // HW attribute config // TODO(Harvey): query this
#ifdef PLAYBACK_USE_24BITS_ONLY
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
#else
    mStreamAttributeTarget.audio_format = (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT) ? AUDIO_FORMAT_PCM_8_24_BIT : AUDIO_FORMAT_PCM_16_BIT;
#endif

    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);

    mStreamAttributeTarget.sample_rate = ChooseTargetSampleRate(AudioALSASampleRateController::getInstance()->getPrimaryStreamOutSampleRate(),
                                                                mStreamAttributeSource->output_devices);

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
    }

    //Change hwbuffer size in Comminuication
    if (!(platformIsolatedDeepBuffer()) &&
        mStreamAttributeSource->audio_mode == AUDIO_MODE_IN_COMMUNICATION) {
        mStreamAttributeTarget.buffer_size = 2 * mStreamAttributeSource->buffer_size /
                                             ((mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4) *
                                             ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);
    }

    // HW pcm config
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;


    if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_FAST) {
        // audio low latency param - playback - interrupt rate
        if (AudioALSAStreamManager::getInstance()->isModeInVoipCall()) {
            mConfig.period_count = 4;
        } else {
            mConfig.period_count = 2;
        }
        mConfig.period_size = (mStreamAttributeSource->buffer_size / mConfig.channels /
                               ((mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4));
        mStreamAttributeTarget.buffer_size = mConfig.period_size * mConfig.period_count * mConfig.channels *
                                             ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);

#if defined(MTK_AUDIO_KS)
        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "fast_play_scenario"), 0, 1)) {
            ALOGW("%s(), fast_play_scenario enable fail", __FUNCTION__);
        }
#else
        // Soc_Aud_AFE_IO_Block_MEM_DL1 assign to use DRAM.
        AudioALSAHardwareResourceManager::getInstance()->AssignDRAM(0);
#endif
    } else {

#if defined(MTK_HIFIAUDIO_SUPPORT)
        if (mStreamAttributeTarget.sample_rate > 48000) {
            mStreamAttributeTarget.buffer_size = UpdateKernelBufferSize(mStreamAttributeSource->output_devices);
        }
#endif

        if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
            if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "voip_rx_scenario"), 0, 1)) {
                ALOGW("%s(), voip_rx_scenario enable fail", __FUNCTION__);
            }
        } else if (!isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags)) {
            if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "primary_play_scenario"), 0, 1)) {
                ALOGW("%s(), primary_play_scenario enable fail", __FUNCTION__);
            }
        }

        // Buffer size: 1536(period_size) * 2(ch) * 4(byte) * 2(period_count) = 24 kb
        mConfig.period_count = 2;
        mConfig.period_size = (mStreamAttributeTarget.buffer_size / (mConfig.channels * mConfig.period_count)) /
                              ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);
    }

    mConfig.format = transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format);

    mConfig.start_threshold = (mPlaybackHandlerType == PLAYBACK_HANDLER_DEEP_BUFFER) ? 0 : mConfig.period_size * mConfig.period_count;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;
    mConfig.avail_min = mStreamAttributeSource->buffer_size / ((mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4) / mStreamAttributeSource->num_channels;
    ALOGD("%s(), flag = %d, mConfig: channels = %d, sample_rate(target) = %d, period_size = %d, period_count = %d, format = %d, avail_min = %d, start_threshold = %d",
          __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags,
          mConfig.channels, mConfig.rate,
          mConfig.period_size, mConfig.period_count, mConfig.format,
          mConfig.avail_min, mConfig.start_threshold);

    // debug pcm dump
    OpenPCMDump(LOG_TAG);

    // disable lowjitter mode
    SetLowJitterMode(true, mStreamAttributeTarget.sample_rate);

#if defined(PLAYBACK_MMAP) // must be after pcm open
    unsigned int flag = PCM_MMAP | PCM_OUT | PCM_MONOTONIC;
    openPcmDriverWithFlag(pcmindex, flag);
#else
    openPcmDriver(pcmindex);
#endif

#if defined(MTK_AUDIODSP_SUPPORT)
    if ((mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_SPEAKER) ||
        (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_ALL_A2DP)) {
        mStreamAttributeTarget.output_devices = mStreamAttributeSource->output_devices;
        AudioDspStreamManager::getInstance()->addPlaybackHandler(this);
    }

#endif

    AL_UNLOCK(AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

#if defined(MTK_AUDIO_SW_DRE) && defined(MTK_NEW_VOL_CONTROL)
    mStartMuteBytes = mConfig.period_size *
                      mConfig.period_count *
                      mConfig.channels *
                      (pcm_format_to_bits(mConfig.format) / 8);

    mAllZeroBlock = new char[kPcmDriverBufferSize];
    memset(mAllZeroBlock, 0, kPcmDriverBufferSize);
#endif

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (get_aurisys_on()) {
        CreateAurisysLibManager();
    } else
#endif
    {
        // post processing
        initPostProcessing();

        // SRC
        initBliSrc();

        // bit conversion
        initBitConverter();

        initDataPending();
    }

    // init DC Removal
    initDcRemoval();

#if defined(MTK_HYBRID_NLE_SUPPORT) // must be after pcm open
    mStreamAttributeTarget.output_devices = mStreamAttributeSource->output_devices;
    initNLEProcessing();
#endif

    // open codec driver
    mHardwareResourceManager->startOutputDevice(mStreamAttributeSource->output_devices, mStreamAttributeTarget.sample_rate);

    mTimeStampValid = false;
    mBytesWriteKernel = 0;

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerNormal::close() {
    ALOGD("+%s(), flag = %d, mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags,
          mStreamAttributeSource->output_devices);

#if defined(MTK_AUDIO_SW_DRE) && defined(MTK_NEW_VOL_CONTROL)
    delete [] mAllZeroBlock;
    if (mForceMute) {
        mForceMute = false;
        ALOGD("%s(), SWDRE swdre unmute", __FUNCTION__);
        AudioMTKGainController::getInstance()->requestMute(getIdentity(), false);
    }
#endif

#if defined(MTK_HYBRID_NLE_SUPPORT)
    // Must do this before close analog path
    deinitNLEProcessing();
#endif

    // close codec driver
    mHardwareResourceManager->stopOutputDevice();
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev1);
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev2);

    // close pcm driver
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());
    closePcmDriver();

#if defined(MTK_AUDIODSP_SUPPORT)
    if (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_SPEAKER ||
       (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_ALL_A2DP)) {
        AudioDspStreamManager::getInstance()->removePlaybackHandler(this);
    }
#endif

#if defined(MTK_AUDIO_KS)
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence2);

#ifdef MTK_AUDIODSP_SUPPORT
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence3);
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequenceDsp);
#endif

    if (isIsolatedDeepBuffer(mStreamAttributeSource->mAudioOutputFlags)) {
        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "deep_buffer_scenario"), 0, 0)) {
            ALOGW("%s(), deep_buffer_scenario disable fail", __FUNCTION__);
        }
    } else {
        if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_FAST) {
            if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "fast_play_scenario"), 0, 0)) {
                ALOGW("%s(), fast_play_scenario disable fail", __FUNCTION__);
            }
        } else if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
            if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "voip_rx_scenario"), 0, 0)) {
                ALOGW("%s(), voip_rx_scenario disable fail", __FUNCTION__);
            }
        } else {
            if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "primary_play_scenario"), 0, 0)) {
                ALOGW("%s(), primary_play_scenario disable fail", __FUNCTION__);
            }
        }
    }
#endif

    // disable lowjitter mode
    SetLowJitterMode(false, mStreamAttributeTarget.sample_rate);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (get_aurisys_on()) {
        DestroyAurisysLibManager();
    } else
#endif
    {
        // bit conversion
        deinitBitConverter();

        // SRC
        deinitBliSrc();

        // post processing
        deinitPostProcessing();

        DeinitDataPending();
    }

    //DC removal
    deinitDcRemoval();

    // debug pcm dump
    ClosePCMDump();

    //release pmic clk
    mHardwareResourceManager->EnableAudBufClk(false);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerNormal::routing(const audio_devices_t output_devices) {
    mHardwareResourceManager->changeOutputDevice(output_devices);
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (mAudioFilterManagerHandler) { mAudioFilterManagerHandler->setDevice(output_devices); }
#endif
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerNormal::setScreenState(bool mode, size_t buffer_size, size_t reduceInterruptSize, bool bforce __unused) {
#if defined(MTK_HIFIAUDIO_SUPPORT)    /* Tune Power */
    bool hifi_enable = mHardwareResourceManager->getHiFiStatus();
    bool device_support_hifi = DeviceSupportHifi(mStreamAttributeSource->output_devices);
    uint32_t source_format = (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4;
    uint32_t target_format = (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4;
    ALOGD("+%s(), flag = %d, mode = %d, sample_rate(source/target) = %d/%d, buffer_size(source/target) = %d/%d, hifi_enable = %d device_support_hifi = %d",
          __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags, mode,
          mStreamAttributeSource->sample_rate, mStreamAttributeTarget.sample_rate,
          mStreamAttributeSource->buffer_size, mStreamAttributeTarget.buffer_size,
          hifi_enable, device_support_hifi);
#endif
    uint32_t sample_rate = mStreamAttributeTarget.sample_rate;

    if (0 == buffer_size) {
        buffer_size = mStreamAttributeSource->buffer_size;
    }

#if defined(MTK_POWERHAL_AUDIO_POWER)
    if (!(platformIsolatedDeepBuffer()) ||
        (platformIsolatedDeepBuffer() &&
         mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_DEEP_BUFFER)) {
        if (mStreamAttributeSource->mPowerHalEnable) {
            if (mode) {
                power_hal_hint(POWERHAL_POWER_DL, false);
            } else {
                power_hal_hint(POWERHAL_POWER_DL, true);
            }
        }
    }
#endif

    int rate;
#if defined(MTK_HIFIAUDIO_SUPPORT)
    /* Tune Power */
    if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_FAST) {
        return NO_ERROR;
    }

    if (hifi_enable) {
        if ((mode == 0) && (sample_rate == 48000) && (sample_rate == mStreamAttributeSource->sample_rate)) {
            reduceInterruptSize *= 1;
            rate = ((mStreamAttributeTarget.buffer_size / mConfig.channels) / target_format) - reduceInterruptSize;
        } else if ((mode == 0) && (sample_rate > 48000) && (device_support_hifi)) {
            reduceInterruptSize *= 6;
            rate = ((mStreamAttributeTarget.buffer_size / mConfig.channels) / target_format) - reduceInterruptSize;
        } else {
            rate = (buffer_size / mStreamAttributeSource->num_channels) / source_format;
        }
    } else {
        rate = mode ? ((buffer_size / mStreamAttributeSource->num_channels) / source_format) :
               ((mStreamAttributeTarget.buffer_size / mConfig.channels) / target_format) - reduceInterruptSize;
    }
#else
    if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_FAST) {
        return NO_ERROR;
    } else {
        rate = mode ? (buffer_size / mStreamAttributeSource->num_channels) / ((mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4) :
               ((mStreamAttributeTarget.buffer_size / mConfig.channels) / ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4)
                 - reduceInterruptSize);
    }
#endif

    mStreamAttributeTarget.mInterrupt = (rate + 0.0) / sample_rate;

    ALOGD("%s(), flag = %d, rate = %d, mInterrupt = %f, mode = %d, sample_rate(target) = %d, buffer_size = %zu, channel = %d, format = %d, reduceInterruptSize = %zu",
          __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags, rate,
          mStreamAttributeTarget.mInterrupt, mode, sample_rate, buffer_size, mConfig.channels,
          mStreamAttributeTarget.audio_format, reduceInterruptSize);

    mHardwareResourceManager->setInterruptRate(mStreamAttributeSource->mAudioOutputFlags, rate);

    return NO_ERROR;
}

ssize_t AudioALSAPlaybackHandlerNormal::write(const void *buffer, size_t bytes) {
    ALOGV("%s(), flag %d, buffer = %p, bytes = %zu",
          __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags, buffer, bytes);


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

#if defined(MTK_AUDIO_SW_DRE) && defined(MTK_NEW_VOL_CONTROL)
    if (mStreamAttributeSource->output_devices == AUDIO_DEVICE_OUT_WIRED_HEADSET ||
        mStreamAttributeSource->output_devices == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        bool isAllMute = false;

        /* check if contents is mute */
        if (!memcmp(mAllZeroBlock, buffer, bytes)) {
            isAllMute = true;
        } else {
            isAllMute = true;
            size_t tmp_bytes = bytes;
            int32_t *sample = (int32_t *)buffer;
            while (tmp_bytes > 0) {
                if ((*sample) >> 8 != 0 && ((*sample) & 0xffffff00) != 0xffffff00) {
                    isAllMute = false;
                    break;
                }
                tmp_bytes -= 4;
                sample++;
            }
        }

        /* calculate delay and apply mute */
        ALOGV("%s(), isAllMute = %d, mForceMute = %d, mCurMuteBytes = %d, mStartMuteBytes = %d",
              __FUNCTION__,
              isAllMute,
              mForceMute,
              mCurMuteBytes,
              mStartMuteBytes);

        if (isAllMute) {
            if (!mForceMute) { /* not mute yet */
                mCurMuteBytes += bytes;
                if (mCurMuteBytes >= mStartMuteBytes) {
                    mForceMute = true;
                    ALOGD("%s(), SWDRE swdre mute", __FUNCTION__);
                    AudioMTKGainController::getInstance()->requestMute(getIdentity(), true);
                }
            }
        } else {
            mCurMuteBytes = 0;

            if (mForceMute) {
                mForceMute = false;
                ALOGD("%s(), SWDRE swdre unmute", __FUNCTION__);
                AudioMTKGainController::getInstance()->requestMute(getIdentity(), false);
            }
        }
    }
#endif
    void *pBufferAfterDcRemoval = NULL;
    uint32_t bytesAfterDcRemoval = 0;
    // DC removal before DRC
    doDcRemoval(pBuffer, bytes, &pBufferAfterDcRemoval, &bytesAfterDcRemoval);


    // stereo to mono for speaker
    doStereoToMonoConversionIfNeed(pBufferAfterDcRemoval, bytesAfterDcRemoval);


    void *pBufferAfterPending = NULL;
    uint32_t bytesAfterpending = 0;

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (get_aurisys_on()) {
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
        doPostProcessing(pBufferAfterDcRemoval, bytesAfterDcRemoval, &pBufferAfterPostProcessing, &bytesAfterPostProcessing);

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

#ifdef MTK_LATENCY_DETECT_PULSE
    AudioDetectPulse::doDetectPulse(TAG_PLAYERBACK_HANDLER, PULSE_LEVEL, 0, (void *)pBufferAfterPending,
                                    bytesAfterpending, mStreamAttributeTarget.audio_format,
                                    mStreamAttributeTarget.num_channels, mStreamAttributeTarget.sample_rate);
#endif

    // write data to pcm driver
    int retval = pcmWrite(mPcm, pBufferAfterPending, bytesAfterpending);

    mBytesWriteKernel = mBytesWriteKernel + bytesAfterpending;

    if (mTimeStampValid == false) {
        if (mBytesWriteKernel >= (mStreamAttributeTarget.buffer_size >> 1)) {
            mTimeStampValid = true;
        }
    }
#if defined(MTK_HYBRID_NLE_SUPPORT)
    if (mSupportNLE) {
        doNLEProcessing(pBufferAfterPending, bytesAfterpending);
    }
#endif

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
    if (latencyTime[0] > THRESHOLD_FRAMEWORK || latencyTime[1] > THRESHOLD_HAL || latencyTime[2] > (mStreamAttributeTarget.mInterrupt - latencyTime[0] - latencyTime[1] + THRESHOLD_KERNEL)) {
        ALOGD("latency_in_s,%1.3lf,%1.3lf,%1.3lf, interrupt,%1.3lf,byte:%u", latencyTime[0], latencyTime[1], latencyTime[2], mStreamAttributeTarget.mInterrupt, bytesAfterpending);
    }
#endif

    return bytes;
}

status_t AudioALSAPlaybackHandlerNormal::setFilterMng(AudioMTKFilterManager *pFilterMng) {
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    ALOGD("+%s() mAudioFilterManagerHandler [%p]", __FUNCTION__, pFilterMng);
    mAudioFilterManagerHandler = pFilterMng;
#else
    (void *)pFilterMng;
#endif
    return NO_ERROR;
}

} // end of namespace android

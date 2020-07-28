#include "AudioALSAPlaybackHandlerHDMI.h"

#include "AudioALSAHardwareResourceManager.h"
#include "AudioVolumeFactory.h"


#include "AudioMTKFilter.h"
#include "AudioALSADeviceParser.h"
#include "AudioALSADriverUtility.h"


#include <linux/hdmitx.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerI2SHDMI"

#if defined(CONFIG_MT_ENG_BUILD)
#define DEBUG_LATENCY
#endif

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)

namespace android {

AudioALSAPlaybackHandlerHDMI::AudioALSAPlaybackHandlerHDMI(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source) {
    ALOGD("%s()", __FUNCTION__);
    mPlaybackHandlerType = PLAYBACK_HANDLER_HDMI;
    memset((void *)&mNewtime, 0, sizeof(mNewtime));
    memset((void *)&mOldtime, 0, sizeof(mOldtime));
}


AudioALSAPlaybackHandlerHDMI::~AudioALSAPlaybackHandlerHDMI() {
    ALOGD("%s()", __FUNCTION__);
}

status_t AudioALSAPlaybackHandlerHDMI::SetMHLChipParameter(int channels, int bits, int samplerate) {
    ALOGD("+%s()", __FUNCTION__);

    int ret = 0;
    int setsamplerate = 0;
    int setbits = 0;

    // File descriptor
    int fd_audio = ::open(HDMI_DEV_DRV, O_RDWR);
    ALOGD("%s(), open(%s), fd_audio = %d", __FUNCTION__, HDMI_DEV_DRV, fd_audio);
    ASSERT(fd_audio >= 0);

    switch (samplerate) {
    case 32000:
        setsamplerate = HDMI_MAX_SAMPLERATE_32;
        break;
    case 44100:
        setsamplerate = HDMI_MAX_SAMPLERATE_44;
        break;
    case 48000:
        setsamplerate = HDMI_MAX_SAMPLERATE_48;
        break;
    case 192000:
        setsamplerate = HDMI_MAX_SAMPLERATE_192;
        break;
    default:
        setsamplerate = HDMI_MAX_SAMPLERATE_44;
        break;
    }

    switch (bits) {
    case AUDIO_FORMAT_PCM_32_BIT:
        setbits = HDMI_MAX_BITWIDTH_24;
        break;
    case AUDIO_FORMAT_PCM_8_24_BIT:
        setbits = HDMI_MAX_BITWIDTH_24;
        break;
    case AUDIO_FORMAT_PCM_16_BIT:
        setbits = HDMI_MAX_BITWIDTH_16;
        break;
    default:
        setbits = HDMI_MAX_BITWIDTH_16;
        break;
    }

    if (fd_audio >= 0) {
        ::ioctl(fd_audio, MTK_HDMI_AUDIO_CONFIG, (channels | (setsamplerate << 4) | (setbits << 7)));

        ALOGD("%s(), ioctl:MTK_HDMI_AUDIO_CONFIG =0x%x \n", __FUNCTION__, (channels | (setsamplerate << 4) | (setbits << 6)));

        ::close(fd_audio);
    }


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerHDMI::SetMHLChipEnable(int enable) {
    ALOGD("+%s(), enable %d", __FUNCTION__, enable);

#if defined(MTK_HDMI_SUPPORT)
    // File descriptor
    int fd_audio = ::open(HDMI_DEV_DRV, O_RDWR);
    ALOGD("%s(), open(%s), fd_audio = %d", __FUNCTION__, HDMI_DEV_DRV, fd_audio);

    if (fd_audio >= 0) {
        ::ioctl(fd_audio, MTK_HDMI_AUDIO_ENABLE, enable);

        ALOGD("%s(), ioctl:MTK_HDMI_AUDIO_ENABLE =0x%x \n", __FUNCTION__, enable);

        ::close(fd_audio);
    }
    ALOGD("-%s(), fd_audio=%d", __FUNCTION__, fd_audio);
#endif

    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerHDMI::open() {
    ALOGD("+%s(), mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->output_devices);

    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    // acquire pmic clk
    mHardwareResourceManager->EnableAudBufClk(true);
#if defined(USING_I2S5_8183HW)
	int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmI2S5Dl1Playback);
	int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmI2S5Dl1Playback);
#else
	int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmI2S0Dl1Playback);
	int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmI2S0Dl1Playback);
#endif
    ALOGD("AudioALSAPlaybackHandlerHDMI::open() pcmindex = %d", pcmindex);
    ListPcmDriver(cardindex, pcmindex);

    struct pcm_params *params;
    params = pcm_params_get(cardindex, pcmindex,  PCM_OUT);
    if (params == NULL) {
        ALOGD("Device does not exist.\n");
    }
    mStreamAttributeTarget.buffer_size = pcm_params_get_max(params, PCM_PARAM_BUFFER_BYTES);
    ALOGD("buffersizemax = %d", mStreamAttributeTarget.buffer_size);
    pcm_params_free(params);

    // HW attribute config // TODO(Harvey): query this
#ifdef PLAYBACK_USE_24BITS_ONLY
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
#else
    mStreamAttributeTarget.audio_format = (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT) ? AUDIO_FORMAT_PCM_8_24_BIT : AUDIO_FORMAT_PCM_16_BIT;
#endif
    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = mStreamAttributeSource->sample_rate; // same as source stream


    // HW pcm config
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;

    // Buffer size: 1536(period_size) * 2(ch) * 4(byte) * 2(period_count) = 24 kb

    mConfig.period_count = 2;
    mConfig.period_size = (mStreamAttributeTarget.buffer_size / (mConfig.channels * mConfig.period_count)) / ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);

    mConfig.period_size = (mConfig.period_size / 2);

    mConfig.format = transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format);

    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;
    ALOGD("%s(), mConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mConfig.format);

    if (mStreamAttributeSource->output_devices != 0) {
        if ((AUDIO_DEVICE_OUT_AUX_DIGITAL & mStreamAttributeSource->output_devices) == 0) {
            SetMHLChipEnable(false);
        } else {
            SetMHLChipEnable(true);
        }
    }
    SetMHLChipParameter(mConfig.channels, mStreamAttributeTarget.audio_format, mConfig.rate);

    // debug pcm dump
    OpenPCMDump(LOG_TAG);

    // post processing
    initPostProcessing();

    // SRC
    initBliSrc();


    // bit conversion
    initBitConverter();

    // init DC Removal
    initDcRemoval();


    // open pcm driver
    openPcmDriver(pcmindex);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerHDMI::close() {
    ALOGD("+%s()", __FUNCTION__);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    SetMHLChipEnable(false);

    // close pcm driver
    closePcmDriver();

    //DC removal
    deinitDcRemoval();


    // bit conversion
    deinitBitConverter();


    // SRC
    deinitBliSrc();


    // post processing
    deinitPostProcessing();


    // debug pcm dump
    ClosePCMDump();

    //release pmic clk
    mHardwareResourceManager->EnableAudBufClk(false);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerHDMI::routing(const audio_devices_t output_devices) {
    mHardwareResourceManager->changeOutputDevice(output_devices);
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (mAudioFilterManagerHandler) { mAudioFilterManagerHandler->setDevice(output_devices); }
#endif
    return NO_ERROR;
}

#if 0
status_t AudioALSAPlaybackHandlerHDMI::setScreenState(bool mode, size_t buffer_size, size_t reduceInterruptSize, bool bforce) {
    // don't increase irq period when play hifi
    if (mode == 0 && mStreamAttributeSource->sample_rate > 48000) {
        return NO_ERROR;
    }

    if (0 == buffer_size) {
        buffer_size = mStreamAttributeSource->buffer_size;
    }

    int rate = mode ? (buffer_size / mConfig.channels) / ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4) :
               ((mStreamAttributeTarget.buffer_size - reduceInterruptSize) / mConfig.channels) / ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);

    ALOGD("%s, rate %d, mode = %d , buffer_size = %d, channel %d, format%d", __FUNCTION__, rate, mode, buffer_size, mConfig.channels, mStreamAttributeTarget.audio_format);

    mHardwareResourceManager->setInterruptRate(rate);
    return NO_ERROR;
}
#endif

ssize_t AudioALSAPlaybackHandlerHDMI::write(const void *buffer, size_t bytes) {
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
    void *pBufferAfterDcRemoval = NULL;
    uint32_t bytesAfterDcRemoval = 0;
    // DC removal before DRC
    doDcRemoval(pBuffer, bytes, &pBufferAfterDcRemoval, &bytesAfterDcRemoval);
    pBuffer = pBufferAfterDcRemoval;
    bytes = bytesAfterDcRemoval;

    // stereo to mono for speaker
    if (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) { // AudioMixer will perform stereo to mono when 32-bit
        doStereoToMonoConversionIfNeed(pBuffer, bytes);
    }


    // post processing (can handle both Q1P16 and Q1P31 by audio_format_t)
    /*void *pBufferAfterPostProcessing = NULL;
    uint32_t bytesAfterPostProcessing = 0;
    doPostProcessing(pBuffer, bytes, &pBufferAfterPostProcessing, &bytesAfterPostProcessing);*/


    // SRC
    void *pBufferAfterBliSrc = NULL;
    uint32_t bytesAfterBliSrc = 0;
    /*doBliSrc(pBufferAfterPostProcessing, bytesAfterPostProcessing, &pBufferAfterBliSrc, &bytesAfterBliSrc);*/
    doBliSrc(pBuffer, bytes, &pBufferAfterBliSrc, &bytesAfterBliSrc);


    // bit conversion
    void *pBufferAfterBitConvertion = NULL;
    uint32_t bytesAfterBitConvertion = 0;
    doBitConversion(pBufferAfterBliSrc, bytesAfterBliSrc, &pBufferAfterBitConvertion, &bytesAfterBitConvertion);


    // pcm dump
    WritePcmDumpData(pBufferAfterBitConvertion, bytesAfterBitConvertion);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[1] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

    // write data to pcm driver
    int retval = pcm_write(mPcm, pBufferAfterBitConvertion, bytesAfterBitConvertion);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[2] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif


    if (retval != 0) {
        ALOGE("%s(), pcm_write() error, retval = %d", __FUNCTION__, retval);
    }

#ifdef DEBUG_LATENCY
    ALOGD("AudioALSAPlaybackHandlerHDMI::write (-) latency_in_us,%1.6lf,%1.6lf,%1.6lf", latencyTime[0], latencyTime[1], latencyTime[2]);
#endif

    return bytes;
}


#if 0
status_t AudioALSAPlaybackHandlerHDMI::setFilterMng(AudioMTKFilterManager *pFilterMng) {
    ALOGD("+%s() mAudioFilterManagerHandler [0x%x]", __FUNCTION__, pFilterMng);
    mAudioFilterManagerHandler = pFilterMng;
    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}
#endif

} // end of namespace android

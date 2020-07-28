#include "AudioALSAPlaybackHandlerHDMI.h"
#include <linux/hdmitx.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/mtkfb_info.h>
#include "AudioALSADriverUtility.h"

#define LOG_TAG "AudioALSAPlaybackHandlerHDMI"
//#define __2CH_TO_8CH
//#define _TDM_DEBUG
#include "AudioTypeExt.h"

namespace android {

#ifdef _TDM_DEBUG
static FILE *pOutFile = NULL;
static FILE *pOutFileorg = NULL;
#endif


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

    if (fd_audio < 0) {
        ALOGE("%s(), open HDMI_DEV_DRV fail", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

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

    ALOGD("%s(), channels=%d, setsamplerate=%d, setbits=%d\n", __FUNCTION__, channels, setsamplerate, setbits);

    ::ioctl(fd_audio, MTK_HDMI_AUDIO_CONFIG, (channels | (setsamplerate << 4) | (setbits << 7)));

    ALOGD("%s(), ioctl:MTK_HDMI_AUDIO_CONFIG =0x%x \n", __FUNCTION__, (channels | (setsamplerate << 4) | (setbits << 6)));

    ::close(fd_audio);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

int AudioALSAPlaybackHandlerHDMI::QueryDisplayType(void) {
    ALOGD("+%s()", __FUNCTION__);

    int ret = 0;
    int display_type = 0;
#if 1
    mtk_dispif_info_t hdmi_info;
    memset((void *)&hdmi_info, 0, sizeof(mtk_dispif_info_t));

    // File descriptor
    int fd_audio = ::open(HDMI_DEV_DRV, O_RDWR);
    ALOGD("%s(), open(%s), fd_audio = %d", __FUNCTION__, HDMI_DEV_DRV, fd_audio);

    if (fd_audio < 0) {
        ALOGE("%s(), open HDMI_DEV_DRV fail", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    ret = ::ioctl(fd_audio, MTK_HDMI_GET_DEV_INFO, (long)&hdmi_info);
    if (ret >= 0) {
        if (hdmi_info.displayType == HDMI_SMARTBOOK) {
            display_type = 0;
        } else if (hdmi_info.displayType == MHL) {
            display_type = 0;
        } else if (hdmi_info.displayType == SLIMPORT) {
            display_type = 1;
        } else {
            display_type = 0;
        }
    }
    ALOGD("%s(), ioctl:MTK_HDMI_GET_DEV_INFO =0x%x display_type =%d \n", __FUNCTION__, ret, display_type);

    ::close(fd_audio);
#endif
    ALOGD("-%s()", __FUNCTION__);
    return display_type;
}

status_t AudioALSAPlaybackHandlerHDMI::SetMHLChipEnable(int enable) {
    ALOGD("+%s()", __FUNCTION__);

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
    int display_type = 0;

    ALOGD("%s(), mStreamAttributeSource: format = %d", __FUNCTION__, mStreamAttributeSource->audio_format);


    //    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeTarget.audio_format = (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT || mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_8_24_BIT) ? AUDIO_FORMAT_PCM_8_24_BIT : AUDIO_FORMAT_PCM_16_BIT;

    mStreamAttributeTarget.audio_channel_mask = mStreamAttributeSource->audio_channel_mask;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = mStreamAttributeSource->sample_rate; // same as source stream

    ALOGD("%s(), mStreamAttributeTarget: channels = %d, rate = %d, format = %d",
          __FUNCTION__, mStreamAttributeTarget.num_channels, mStreamAttributeTarget.sample_rate, mStreamAttributeTarget.audio_format);

    // HW pcm config
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;

#ifdef __2CH_TO_8CH
    mConfig.period_size = 1536 * 4;
    mConfig.period_count = 8;
#else
    if (mStreamAttributeTarget.num_channels == 2) {
        mConfig.period_size = 3072 * 2;
        mConfig.period_count = 2;
        mConfig.channels = 2;
    } else if (mStreamAttributeTarget.num_channels == 4) {
        mConfig.period_size = 3072 * 2;
        mConfig.period_count = 2;
        mConfig.channels = 4;
    } else {
        mConfig.period_size = 3072 * 2;
        mConfig.period_count = 2;
        mConfig.channels = 8;
    }
#endif

    mConfig.format = transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format);
    display_type = QueryDisplayType();
    ALOGD("%s(), display_type =%d, ",
          __FUNCTION__, display_type);

    if (display_type) { // SLIMPORT
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Hdmi_Display_Switch"), "SLIMPORT")) {
            ALOGE("Error: Audio_Hdmi_Display_Switch invalid value");
        }
    } else { // MHL
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Hdmi_Display_Switch"), "MHL")) {
            ALOGE("Error: Audio_Hdmi_Display_Switch invalid value");
        }
    }

    SetMHLChipParameter(mConfig.channels, mStreamAttributeTarget.audio_format, mConfig.rate);
    mConfig.start_threshold = mConfig.period_size;
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

#ifdef _TDM_DEBUG

#ifdef __2CH_TO_8CH
    pOutFile = fopen("/data/vendor/audiohal/hdmi8ch.pcm", "wb");
    if (pOutFile == NULL) {
        ALOGW("%s(), open file fail ", __FUNCTION__);
    }
#endif

    pOutFileorg = fopen("/data/vendor/audiohal/hdmi.pcm", "wb");
    if (pOutFileorg == NULL) {
        ALOGW("%s(), open file fail ", __FUNCTION__);
    }
#else
    OpenPCMDump(LOG_TAG);
#endif

    // SRC
    initBliSrc();

    // bit conversion
    initBitConverter();

    // init DC Removal
    initDcRemoval();

    // open pcm driver
    openPcmDriver(AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHDMI));

#if 0
    ASSERT(mPcm == NULL);
    int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHDMI);
    int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmHDMI);

    mPcm = pcm_open(cardIdx, pcmIdx, PCM_OUT, &mConfig);

    if (mPcm == NULL) {
        ALOGE("%s(), mPcm == NULL!!", __FUNCTION__);
    } else if (pcm_is_ready(mPcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mPcm, pcm_get_error(mPcm));
        pcm_close(mPcm);
        mPcm = NULL;
    } else {
        pcm_start(mPcm);
    }

    ALOGD("-%s(), mPcm = %p", __FUNCTION__, mPcm);
    ASSERT(mPcm != NULL);

#endif

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


#ifdef _TDM_DEBUG

#ifdef __2CH_TO_8CH
    if (pOutFile != NULL) {
        fclose(pOutFile);
    }
#endif
    if (pOutFileorg != NULL) {
        fclose(pOutFileorg);
    }
#else
    ClosePCMDump();
#endif


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerHDMI::routing(const audio_devices_t output_devices) {
    return INVALID_OPERATION;
}

ssize_t AudioALSAPlaybackHandlerHDMI::write(const void *buffer, size_t bytes) {
    ALOGV("%s(), buffer = %p, bytes = %d", __FUNCTION__, buffer, bytes);

    if (mPcm == NULL) {
        ALOGE("%s(), mPcm == NULL, return", __FUNCTION__);
        return bytes;
    }

    // work around for incall mode HDMI is playing
    if (mStreamAttributeSource->audio_mode == AUDIO_MODE_IN_CALL) {
        usleep(30 * 1000);
        return bytes;
    }
    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);
    ASSERT(pBuffer != NULL);

    void *pBufferAfterDcRemoval = NULL;
    uint32_t bytesAfterDcRemoval = 0;
    // DC removal before DRC
    doDcRemoval(pBuffer, bytes, &pBufferAfterDcRemoval, &bytesAfterDcRemoval);
    pBuffer = pBufferAfterDcRemoval;
    bytes = bytesAfterDcRemoval;

    // SRC
    void *pBufferAfterBliSrc = NULL;
    uint32_t bytesAfterBliSrc = 0;
    doBliSrc(pBuffer, bytes, &pBufferAfterBliSrc, &bytesAfterBliSrc);


    // bit conversion
    void *pBufferAfterBitConvertion = NULL;
    uint32_t bytesAfterBitConvertion = 0;
    doBitConversion(pBufferAfterBliSrc, bytesAfterBliSrc, &pBufferAfterBitConvertion, &bytesAfterBitConvertion);

    void *newbuffer[96 * 1024] = {0};
    unsigned char *aaa;
    unsigned char *bbb;
    int i = 0;
    int j = 0;
    int retval = 0;

    aaa = (unsigned char *)newbuffer;
    bbb = (unsigned char *)pBufferAfterBitConvertion;


#ifdef __2CH_TO_8CH


    for (i = 0 ; j < bytesAfterBitConvertion; i += 16) {
        memcpy(aaa + i,      bbb + j, 4);
        memcpy(aaa + i + 4,  bbb + j, 4);
        memcpy(aaa + i + 8,  bbb + j, 4);
        memcpy(aaa + i + 12, bbb + j, 4);
        j += 4;
    }

    // write data to pcm driver
    retval = pcm_write(mPcm, newbuffer, bytesAfterBitConvertion << 2);
#else
    ALOGD("%s(), channels = %d, format = %d ,bytes = %d", __FUNCTION__, mStreamAttributeTarget.num_channels, mStreamAttributeTarget.audio_format, bytes);

    if (mStreamAttributeTarget.num_channels == 6) {
        if (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) {
            aaa = (unsigned char *)newbuffer;
            bbb = (unsigned char *)pBuffer;

            for (i = 0 ; j < bytesAfterBitConvertion; i += 16) {
                memcpy(aaa + i,      bbb + j, 12);
                //              memcpy(aaa + i,      bbb + j + 4, 2); //test copy center to left
                //              memcpy(aaa + i + 4,      bbb + j + 2, 2); //test copy right to center
                memset(aaa + i + 12, 0, 4);
                j += 12;
            }
            retval = pcm_write(mPcm, newbuffer, ((bytesAfterBitConvertion * 4) / 3));
        } else {
            for (i = 0 ; j < bytesAfterBitConvertion; i += 32) {
                memcpy(aaa + i,      bbb + j, 24);
                memset(aaa + i + 24, 0, 8);
                j += 24;
            }
            retval = pcm_write(mPcm, newbuffer, ((bytesAfterBitConvertion * 4) / 3));

        }
    } else {
        // write data to pcm driver
        retval = pcm_write(mPcm, pBufferAfterBitConvertion, bytesAfterBitConvertion);
    }
    // write data to pcm driver
#endif


    if (retval != 0) {
        ALOGE("%s(), pcm_write() error, retval = %d", __FUNCTION__, retval);
    }


#ifdef _TDM_DEBUG

#ifdef __2CH_TO_8CH

    if (pOutFile != NULL) {
        ALOGD("%s(), newbuffer = %p", __FUNCTION__, newbuffer);
        fwrite(newbuffer, sizeof(char), bytesAfterBitConvertion * 4, pOutFile);
    }
#endif
    if (pOutFileorg != NULL) {
        ALOGD("%s(), pBufferAfterBitConvertion = %p", __FUNCTION__, pBufferAfterBitConvertion);
        fwrite(pBufferAfterBitConvertion, sizeof(char), bytesAfterBitConvertion, pOutFileorg);
    }
#else
    if (mStreamAttributeTarget.num_channels == 6) {
        ALOGD("%s(), dumpbytes = %d", __FUNCTION__, ((bytesAfterBitConvertion * 16) / 12));

        WritePcmDumpData(newbuffer, ((bytesAfterBitConvertion * 16) / 12));
    } else {
        WritePcmDumpData(pBufferAfterBitConvertion, bytesAfterBitConvertion);
    }
#endif


    return bytes;
}


} // end of namespace android

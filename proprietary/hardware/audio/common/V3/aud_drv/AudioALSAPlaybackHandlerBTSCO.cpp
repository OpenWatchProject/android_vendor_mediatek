#include "AudioALSAPlaybackHandlerBTSCO.h"

#include "AudioALSADriverUtility.h"
#include "AudioALSAHardwareResourceManager.h"
#include <AudioLock.h>

#include "WCNChipController.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <audio_ringbuf.h>
#include <audio_pool_buf_handler.h>

#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>
#endif



#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerBTSCO"


#if defined(CONFIG_MT_ENG_BUILD)
#define DEBUG_LATENCY
#endif

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)

namespace android {

AudioALSAPlaybackHandlerBTSCO::AudioALSAPlaybackHandlerBTSCO(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source),
    mWCNChipController(WCNChipController::GetInstance()) {
    ALOGD("%s()", __FUNCTION__);
    mPlaybackHandlerType = PLAYBACK_HANDLER_BT_SCO;
    memset(&mNewtime, 0, sizeof(mNewtime));
    memset(&mOldtime, 0, sizeof(mOldtime));
}


AudioALSAPlaybackHandlerBTSCO::~AudioALSAPlaybackHandlerBTSCO() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSAPlaybackHandlerBTSCO::open() {
    ALOGD("+%s(), mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->output_devices);

    if (mStreamAttributeSource->isMixerOut) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "dl1bt_memif_select"), "dl2")) {
            ALOGE("Error: dl1bt_memif_select invalid value");
        }
    } else {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "dl1bt_memif_select"), "dl1")) {
            ALOGE("Error: dl1bt_memif_select invalid value");
        }
    }

    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVOIPCallBTPlayback);
    int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVOIPCallBTPlayback);

    struct pcm_params *params;
    params = pcm_params_get(cardindex, pcmindex,  PCM_OUT);
    if (params == NULL) {
        ALOGD("Device does not exist.\n");
    }
#ifdef PLAYBACK_USE_24BITS_ONLY
    const uint32_t bt_max_buffer_size = 8192; // 8k
#else
    const uint32_t bt_max_buffer_size = 4096; // 4k
#endif
    const uint32_t kernel_max_buffer_size = pcm_params_get_max(params, PCM_PARAM_BUFFER_BYTES);
    mStreamAttributeTarget.buffer_size = (kernel_max_buffer_size < bt_max_buffer_size) ? kernel_max_buffer_size : bt_max_buffer_size;
    ALOGD("buffersizemax = %d", mStreamAttributeTarget.buffer_size);
    pcm_params_free(params);

    // HW attribute config // TODO(Harvey): query this
#ifdef PLAYBACK_USE_24BITS_ONLY
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
#else
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_16_BIT;
#endif
    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = mWCNChipController->GetBTCurrentSamplingRateNumber();


    // HW pcm config
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;

    mConfig.period_count = 2;
    mConfig.period_size = (mStreamAttributeTarget.buffer_size / (mConfig.channels * mConfig.period_count)) / ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);

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
        // SRC
        initBliSrc();

        // bit conversion
        initBitConverter();

        initDataPending();
    }

    // init DC Removal
    initDcRemoval();

    // open pcm driver
    openPcmDriver(pcmindex);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBTSCO::close() {
    ALOGD("+%s()", __FUNCTION__);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    const uint32_t mute_buf_len = 8192;
    char mute_buf[mute_buf_len];
    memset(mute_buf, 0, mute_buf_len);

    this->write(mute_buf, mute_buf_len);
    this->write(mute_buf, mute_buf_len);
    this->write(mute_buf, mute_buf_len);
    this->write(mute_buf, mute_buf_len);
    this->write(mute_buf, mute_buf_len);
    this->write(mute_buf, mute_buf_len);
    this->write(mute_buf, mute_buf_len);
    this->write(mute_buf, mute_buf_len);

    // close pcm driver
    closePcmDriver();

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
    }

    // debug pcm dump
    ClosePCMDump();


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBTSCO::routing(const audio_devices_t output_devices __unused) {
    return INVALID_OPERATION;
}

ssize_t AudioALSAPlaybackHandlerBTSCO::write(const void *buffer, size_t bytes) {
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

    void *pBufferAfterPending = NULL;
    uint32_t bytesAfterpending = 0;

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (get_aurisys_on() && !mStreamAttributeSource->isMixerOut) {
        // expect library output amount smoothly
        mTransferredBufferSize = GetTransferredBufferSize(
            bytes,
            mStreamAttributeSource,
            &mStreamAttributeTarget);

        audio_pool_buf_copy_from_linear(
            mAudioPoolBufDlIn,
            pBuffer,
            bytes);

        // post processing + SRC + Bit conversion
        aurisys_process_dl_only(mAurisysLibManager, mAudioPoolBufDlIn, mAudioPoolBufDlOut);

        // data pending: sram is device memory, need word size align 64 byte for 64 bit platform
        uint32_t data_size = audio_ringbuf_count(&mAudioPoolBufDlOut->ringbuf);
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
        // SRC
        void *pBufferAfterBliSrc = NULL;
        uint32_t bytesAfterBliSrc = 0;
        doBliSrc(pBuffer, bytes, &pBufferAfterBliSrc, &bytesAfterBliSrc);


        // bit conversion
        void *pBufferAfterBitConvertion = NULL;
        uint32_t bytesAfterBitConvertion = 0;
        doBitConversion(pBufferAfterBliSrc, bytesAfterBliSrc, &pBufferAfterBitConvertion, &bytesAfterBitConvertion);


        // data pending
        pBufferAfterPending = NULL;
        bytesAfterpending = 0;
        dodataPending(pBufferAfterBitConvertion, bytesAfterBitConvertion, &pBufferAfterPending, &bytesAfterpending);
    }

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[1] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

    // write data to pcm driver
    WritePcmDumpData(pBufferAfterPending, bytesAfterpending);
    int retval = pcm_write(mPcm, pBufferAfterPending, bytesAfterpending);


#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[2] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif


    if (retval != 0) {
        ALOGE("%s(), pcm_write() error, retval = %d", __FUNCTION__, retval);
    }

#ifdef DEBUG_LATENCY
    ALOGD("AudioALSAPlaybackHandlerNormal::write (-) latency_in_us,%1.6lf,%1.6lf,%1.6lf", latencyTime[0], latencyTime[1], latencyTime[2]);
#endif


    return bytes;
}


} // end of namespace android

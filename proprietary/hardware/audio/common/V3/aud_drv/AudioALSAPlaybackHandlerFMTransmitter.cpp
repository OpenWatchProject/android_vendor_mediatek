#include "AudioALSAPlaybackHandlerFMTransmitter.h"

#include "WCNChipController.h"

#include "AudioALSADeviceParser.h"
#include "AudioALSADriverUtility.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerFMTransmitter"

namespace android {

AudioALSAPlaybackHandlerFMTransmitter::AudioALSAPlaybackHandlerFMTransmitter(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source),
    mWCNChipController(WCNChipController::GetInstance()) {
    ALOGD("%s()", __FUNCTION__);
    mPlaybackHandlerType = PLAYBACK_HANDLER_FM_TX;
}


AudioALSAPlaybackHandlerFMTransmitter::~AudioALSAPlaybackHandlerFMTransmitter() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSAPlaybackHandlerFMTransmitter::open() {
    ALOGD("+%s(), mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->output_devices);

    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmMRGTxPlayback);
    int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmMRGTxPlayback);
    ALOGD("AudioALSAPlaybackHandlerFMTransmitter::open() pcmindex = %d", pcmindex);
    ListPcmDriver(cardindex, pcmindex);

    struct pcm_params *params;
    params = pcm_params_get(cardindex, pcmindex,  PCM_OUT);
    if (params == NULL) {
        ALOGD("AudioALSAPlaybackHandlerFMTransmitter Device does not exist.\n");
    }
    mStreamAttributeTarget.buffer_size = pcm_params_get_max(params, PCM_PARAM_BUFFER_BYTES);
    ALOGD("AudioALSAPlaybackHandlerFMTransmitter buffersizemax = %d", mStreamAttributeTarget.buffer_size);
    pcm_params_free(params);

    // HW attribute config // TODO(Harvey): query this
#ifdef PLAYBACK_USE_24BITS_ONLY
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
#else
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_16_BIT;
#endif
    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = 44100; // TODO(Harvey, Chipeng): query?


    // HW pcm config
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;

    // Buffer size
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

    // SRC
    initBliSrc();

    // bit conversion
    initBitConverter();

    // Get pcm open Info

    // open pcm driver
    openPcmDriver(pcmindex);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerFMTransmitter::close() {
    ALOGD("+%s()", __FUNCTION__);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    // close pcm driver
    closePcmDriver();


    // bit conversion
    deinitBitConverter();


    // SRC
    deinitBliSrc();


    // debug pcm dump
    ClosePCMDump();


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerFMTransmitter::routing(const audio_devices_t output_devices __unused) {
    return INVALID_OPERATION;
}

ssize_t AudioALSAPlaybackHandlerFMTransmitter::write(const void *buffer, size_t bytes) {
    ALOGV("%s(), buffer = %p, bytes = %zu", __FUNCTION__, buffer, bytes);

    if (mPcm == NULL) {
        ALOGE("%s(), mPcm == NULL, return", __FUNCTION__);
        return bytes;
    }

    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);
    ASSERT(pBuffer != NULL);


    // SRC
    void *pBufferAfterBliSrc = NULL;
    uint32_t bytesAfterBliSrc = 0;
    doBliSrc(pBuffer, bytes, &pBufferAfterBliSrc, &bytesAfterBliSrc);


    // bit conversion
    void *pBufferAfterBitConvertion = NULL;
    uint32_t bytesAfterBitConvertion = 0;
    doBitConversion(pBufferAfterBliSrc, bytesAfterBliSrc, &pBufferAfterBitConvertion, &bytesAfterBitConvertion);


    // write data to pcm driver
    WritePcmDumpData(pBufferAfterBitConvertion, bytesAfterBitConvertion);
    int retval = pcm_write(mPcm, pBufferAfterBitConvertion, bytesAfterBitConvertion);


    if (retval != 0) {
        ALOGE("%s(), pcm_write() error, retval = %d", __FUNCTION__, retval);
    }

    return bytes;
}


} // end of namespace android

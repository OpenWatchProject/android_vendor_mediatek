#include "AudioALSAPlaybackHandlerMixer.h"

#include "AudioMixerOut.h"
#include "AudioSmartPaController.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <audio_ringbuf.h>
#include <audio_pool_buf_handler.h>

#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>
#endif
#include <SpeechUtility.h>
#define DEBUG_LATENCY

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerMixer"

#ifdef DEBUG_LATENCY
// Latency Detect
//#define DEBUG_LATENCY
#define THRESHOLD_FRAMEWORK   0.010
#define THRESHOLD_HAL         0.010
#define THRESHOLD_KERNEL      0.010
#endif

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)

namespace android {

AudioALSAPlaybackHandlerMixer::AudioALSAPlaybackHandlerMixer(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source),
    mMixerOut(NULL) {
    ALOGD("%s()", __FUNCTION__);
    mPlaybackHandlerType = PLAYBACK_HANDLER_MIXER;

    memset((void *)&mNewtime, 0, sizeof(mNewtime));
    memset((void *)&mOldtime, 0, sizeof(mOldtime));
}

AudioALSAPlaybackHandlerMixer::~AudioALSAPlaybackHandlerMixer() {
    ALOGD("%s()", __FUNCTION__);
}

status_t AudioALSAPlaybackHandlerMixer::open() {
    ALOGD("+%s(), flag %d, mDevice = 0x%x, buffer_size %d", __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags,
          mStreamAttributeSource->output_devices, mStreamAttributeSource->buffer_size);

    mStreamAttributeTarget = *mStreamAttributeSource;

    // debug pcm dump
    OpenPCMDump(LOG_TAG);

    // get mixer out
    enum MIXER_USAGE usage;
    if (audio_is_bluetooth_sco_device(mStreamAttributeSource->output_devices) ||
        isBtSpkDevice(mStreamAttributeSource->output_devices)) {
        usage = MIXER_USAGE_BT;
    } else if (AudioSmartPaController::getInstance()->isSwDspSpkProtect(mStreamAttributeSource->output_devices)) {
        usage = MIXER_USAGE_SMARTPA;
    } else if ((mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_FAST) ||
               (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_DEEP_BUFFER)) {
        // temp, rough condition
        ASSERT(0);
        usage = MIXER_USAGE_DEEP_FAST;
    } else {
        ASSERT(0);
        usage = MIXER_USAGE_UNKNOWN;
    }
    mMixerOut = AudioMixerOut::getInstance(usage);

    mMixerOut->attach(this, &mStreamAttributeTarget);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (get_aurisys_on()) {
        CreateAurisysLibManager();
    }
#endif

    mSmoother = createWriteSmoother();
    mIsForceDumpLatency = get_uint32_from_property("vendor.audiohal.dump_latency");

    ALOGD("-%s(), mIsForceDumpLatency = %d", __FUNCTION__, mIsForceDumpLatency);

    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerMixer::close() {
    ALOGD("+%s()", __FUNCTION__);

    destroyWriteSmoother(&mSmoother);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (mAurisysLibManager && get_aurisys_on()) {
        DestroyAurisysLibManager();
    }
#endif

    mMixerOut->detach(this);

    // debug pcm dump
    ClosePCMDump();

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

ssize_t AudioALSAPlaybackHandlerMixer::write(const void *buffer, size_t bytes) {
    ALOGV("%s(), buffer = %p, bytes = %zu", __FUNCTION__, buffer, bytes);

    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);
    ASSERT(pBuffer != NULL);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[0] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

    void *finalBuffer = pBuffer;
    uint32_t finalBufferBytes = bytes;

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (mAurisysLibManager && get_aurisys_on()) {
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
        if (data_size > mTransferredBufferSize) {
            data_size = mTransferredBufferSize;
        }
        data_size &= 0xFFFFFFC0;
        audio_pool_buf_copy_to_linear(
            &mLinearOut->p_buffer,
            &mLinearOut->memory_size,
            mAudioPoolBufDlOut,
            data_size);

        // wrap to original playback handler
        finalBuffer = (void *)mLinearOut->p_buffer;
        finalBufferBytes = data_size;
    }
#endif

    // pcm dump
    WritePcmDumpData(finalBuffer, finalBufferBytes);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[1] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

    // write data to Mixer
    mMixerOut->write(this, finalBuffer, finalBufferBytes);

    // sleep buffer latency
    doWriteSmoother(
        mSmoother,
        getBufferLatencyUs(mStreamAttributeSource, bytes),
        2); // only sleep when left buffer time > 2 frames

    updateHardwareBufferInfo(bytes, finalBufferBytes);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[2] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
    if ((mIsForceDumpLatency != 0) ||
        (latencyTime[0] > THRESHOLD_FRAMEWORK || latencyTime[1] > THRESHOLD_HAL ||
         latencyTime[2] > (mStreamAttributeTarget.mInterrupt - latencyTime[0] - latencyTime[1] + THRESHOLD_KERNEL))) {
        ALOGD("%s(), latency_in_s,%1.3lf,%1.3lf,%1.3lf, interrupt,%1.3lf, bytes = %zu",
              __FUNCTION__, latencyTime[0], latencyTime[1], latencyTime[2], mStreamAttributeTarget.mInterrupt, bytes);
    }
#endif

    return bytes;
}

status_t AudioALSAPlaybackHandlerMixer::getHardwareBufferInfo(time_info_struct_t *HWBuffer_Time_Info) {
    if (mMixerOut->getHardwareBufferInfo(this, HWBuffer_Time_Info) != NO_ERROR) {
        return UNKNOWN_ERROR;
    } else {
        HWBuffer_Time_Info->halQueuedFrame += mHalQueuedFrame;
    }

    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerMixer::setScreenState(bool mode,
                                                       size_t buffer_size,
                                                       size_t reduceInterruptSize,
                                                       bool bforce) {
    return mMixerOut->setScreenState(this, mode, buffer_size, reduceInterruptSize, bforce);
}

int AudioALSAPlaybackHandlerMixer::setSuspend(bool suspend) {
    mMixerOut->setSuspend(this, suspend);
    return 0;
}

status_t AudioALSAPlaybackHandlerMixer::routing(const audio_devices_t output_devices __unused) {
    return NO_ERROR;
}

int AudioALSAPlaybackHandlerMixer::getLatency() {
    return mMixerOut->getLatency();
}

} // end of namespace android

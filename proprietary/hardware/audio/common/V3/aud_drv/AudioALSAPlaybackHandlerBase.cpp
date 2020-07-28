#include "AudioALSAPlaybackHandlerBase.h"
#include "AudioALSADataProcessor.h"
#include "AudioALSADriverUtility.h"
#include "AudioALSAHardwareResourceManager.h"
#include "AudioUtility.h"
#include "AudioMTKFilter.h"
#include "AudioALSADeviceConfigManager.h"

#if (defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT) && (MTK_AUDIO_TUNING_TOOL_V2_PHASE >= 2))
#include "AudioParamParser.h"
#endif

#include <SpeechEnhancementController.h>

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <utils/String8.h>

#include <audio_memory_control.h>
#include <audio_lock.h>
#include <audio_ringbuf.h>


#include <audio_task.h>
#include <aurisys_scenario.h>

#include <arsi_type.h>
#include <aurisys_config.h>

#include <audio_pool_buf_handler.h>

#include <aurisys_utility.h>
#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>
#endif

#ifdef MTK_AUDIO_SCP_SUPPORT
#include <AudioMessengerIPI.h>
#endif

#ifdef MTK_AUDIODSP_SUPPORT
#include "AudioDspStreamManager.h"
#include <audio_task.h>
#include <AudioDspType.h>
#endif

#include "AudioSmartPaController.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerBase"

namespace android {

static const uint32_t kMaxPcmDriverBufferSize = 0x20000;  // 128k
static const uint32_t kBliSrcOutputBufferSize = 0x10000;  // 64k
static const uint32_t kPcmDriverBufferSize    = 0x20000;  // 128k


uint32_t AudioALSAPlaybackHandlerBase::mDumpFileNum = 0;

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
static AudioLock mAurisysLibManagerLock;
#endif
#ifdef MTK_AUDIODSP_SUPPORT
static FILE *pcmdump_array[TASK_SCENE_SIZE][DEBUG_PCMDUMP_NUM];
uint32_t AudioALSAPlaybackHandlerBase::mDumpFileNumDSP = 0;
#endif

AudioALSAPlaybackHandlerBase::AudioALSAPlaybackHandlerBase(const stream_attribute_t *stream_attribute_source) :
#ifdef MTK_AUDIO_SCP_SUPPORT
    mAudioMessengerIPI(AudioMessengerIPI::getInstance()),
#else
    mAudioMessengerIPI(NULL),
#endif
    mPlaybackHandlerType(PLAYBACK_HANDLER_BASE),
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()),
    mStreamAttributeSource(stream_attribute_source),
    mPcm(NULL),
    mComprStream(NULL),
    mStreamCbk(NULL),
    mCbkCookie(NULL),
    mAudioFilterManagerHandler(NULL),
    mPostProcessingOutputBuffer(NULL),
    mPostProcessingOutputBufferSize(0),
    mFirstDataWrite(true),
    mDcRemove(NULL),
    mDcRemoveWorkBuffer(NULL),
    mDcRemoveBufferSize(0),
    mBliSrc(NULL),
    mBliSrcOutputBuffer(NULL),
    mBitConverter(NULL),
    mBitConverterOutputBuffer(NULL),
    mdataPendingOutputBuffer(NULL),
    mdataPendingTempBuffer(NULL),
    mdataPendingOutputBufferSize(0),
    mdataPendingRemindBufferSize(0),
    mDataAlignedSize(64),
    mDataPendingForceUse(false),
    mNLEMnger(NULL),
    mPCMDumpFile(NULL),
#ifdef MTK_AUDIODSP_SUPPORT
    mPCMDumpFileDSP(NULL),
#endif
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    mBytesWriteKernel(0),
    mTimeStampValid(true),
    mHalQueuedFrame(0),
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    mAurisysLibManager(NULL),
    mManagerConfig(NULL),
    mAurisysScenario(AURISYS_SCENARIO_INVALID),
    mAudioPoolBufUlIn(NULL),
    mAudioPoolBufUlOut(NULL),
    mAudioPoolBufDlIn(NULL),
    mAudioPoolBufDlOut(NULL),
    mTransferredBufferSize(0xFFFFFFFF),
    mLinearOut(NULL),
#endif
    mIsNeedUpdateLib(false),
    mUpdateLibCount(0),
    mSmoother(NULL),
    mDataProcessor(AudioALSADataProcessor::getInstance()),
    mIdentity(0xFFFFFFFF),
    mPcmflag(0),
    audio_pcm_write_wrapper_fp(NULL) {
    ALOGV("%s()", __FUNCTION__);

    memset(&mConfig, 0, sizeof(mConfig));
    memset(&mStreamAttributeTarget, 0, sizeof(mStreamAttributeTarget));
    memset(&mComprConfig, 0, sizeof(mComprConfig));

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    AUDIO_ALLOC_STRUCT(struct data_buf_t, mLinearOut);
#endif
}


AudioALSAPlaybackHandlerBase::~AudioALSAPlaybackHandlerBase() {
    ALOGV("%s()", __FUNCTION__);
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    AUDIO_FREE_POINTER(mLinearOut);
#endif
}

String8 AudioALSAPlaybackHandlerBase::getPlaybackTurnOnSequence(unsigned int turnOnSeqType,
                                                                const char *playbackSeq) {
    String8 sequence = String8();
#if !defined(MTK_AUDIO_KS)
    UNUSED(turnOnSeqType);
    UNUSED(playbackSeq);
#else
    bool isSpk = (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_SPEAKER);
    bool isSmartPA = AudioSmartPaController::getInstance()->isSmartPAUsed();
    bool isSpkInCalibration = AudioSmartPaController::getInstance()->isInCalibration();
#if defined(MTK_AUDIODSP_SUPPORT)
    bool isADSPPlayback = AudioDspStreamManager::getInstance()->getDspPlaybackEnable();
    bool isADSPA2dpUsed = (AudioDspStreamManager::getInstance()->getDspA2DPEnable() == true) &&
                          (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_ALL_A2DP);
#else
    bool isADSPPlayback = false;
    bool isADSPA2dpUsed = false;
#endif /* end of defined(MTK_AUDIODSP_SUPPORT) */

    ASSERT(playbackSeq != NULL);

    switch (turnOnSeqType) {
    case TURN_ON_SEQUENCE_1:
        if (isSpk && isADSPPlayback && !isSpkInCalibration) {
            sequence = String8(playbackSeq) + AUDIO_CTL_ADSP_UL;
        } else {
            sequence = mHardwareResourceManager->getOutputTurnOnSeq(mStreamAttributeSource->output_devices,
                                                                    false, playbackSeq);
        }
        break;
    case TURN_ON_SEQUENCE_2:
        if (!isADSPPlayback) {
            if (isSpk && isSmartPA && (popcount(mStreamAttributeSource->output_devices) > 1)) {
                sequence = mHardwareResourceManager->getOutputTurnOnSeq(mStreamAttributeSource->output_devices,
                                                                        true, playbackSeq);
            }
        }
        break;
#if defined(MTK_AUDIODSP_SUPPORT)
    case TURN_ON_SEQUENCE_3:
        if (!(isSpk && isADSPPlayback && !isSpkInCalibration)) {
            if (isADSPA2dpUsed) {
                sequence = String8(playbackSeq) + AUDIO_CTL_ADSP_UL;
            }
        }
        break;
    case TURN_ON_SEQUENCE_DSP:
        if (isSpk && isADSPPlayback && !isSpkInCalibration) {
            sequence = String8(playbackSeq) + AUDIO_CTL_DSPDL;
        } else {
            if (isADSPA2dpUsed) {
                sequence = String8(playbackSeq) + AUDIO_CTL_DSPDL;
            }
        }
        break;
#endif
    default:
        ASSERT(0);
        break;
    }
#endif /* end of !defined(MTK_AUDIO_KS) */
    return sequence;
}

status_t AudioALSAPlaybackHandlerBase::ListPcmDriver(const unsigned int card, const unsigned int device) {
    struct pcm_params *params;
    unsigned int min, max ;
    params = pcm_params_get(card, device, PCM_OUT);
    if (params == NULL) {
        ALOGD("Device does not exist.\n");
    }
    min = pcm_params_get_min(params, PCM_PARAM_RATE);
    max = pcm_params_get_max(params, PCM_PARAM_RATE);
    ALOGD("        Rate:\tmin=%uHz\tmax=%uHz\n", min, max);
    min = pcm_params_get_min(params, PCM_PARAM_CHANNELS);
    max = pcm_params_get_max(params, PCM_PARAM_CHANNELS);
    ALOGD("    Channels:\tmin=%u\t\tmax=%u\n", min, max);
    min = pcm_params_get_min(params, PCM_PARAM_SAMPLE_BITS);
    max = pcm_params_get_max(params, PCM_PARAM_SAMPLE_BITS);
    ALOGD(" Sample bits:\tmin=%u\t\tmax=%u\n", min, max);
    min = pcm_params_get_min(params, PCM_PARAM_PERIOD_SIZE);
    max = pcm_params_get_max(params, PCM_PARAM_PERIOD_SIZE);
    ALOGD(" Period size:\tmin=%u\t\tmax=%u\n", min, max);
    min = pcm_params_get_min(params, PCM_PARAM_PERIODS);
    max = pcm_params_get_max(params, PCM_PARAM_PERIODS);
    ALOGD("Period count:\tmin=%u\t\tmax=%u\n", min, max);
    max = pcm_params_get_max(params, PCM_PARAM_BUFFER_SIZE);
    ALOGD("PCM_PARAM_BUFFER_SIZE :\t max=%u\t\n", max);
    max = pcm_params_get_max(params, PCM_PARAM_BUFFER_BYTES);
    ALOGD("PCM_PARAM_BUFFER_BYTES :\t max=%u\t\n", max);
    pcm_params_free(params);

    return NO_ERROR;
}
status_t AudioALSAPlaybackHandlerBase::openPcmDriverWithFlag(const unsigned int device, unsigned int flag) {
    ALOGV("+%s(), pcm device = %d flag = 0x%x", __FUNCTION__, device, flag);

    ASSERT(mPcm == NULL);

    mPcmflag = flag;
    mPcm = pcm_open(AudioALSADeviceParser::getInstance()->GetCardIndex(),
                    device, flag, &mConfig);
    if (mPcm == NULL) {
        ALOGE("%s(), mPcm == NULL!!", __FUNCTION__);
    } else if (pcm_is_ready(mPcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mPcm, pcm_get_error(mPcm));
        pcm_close(mPcm);
        mPcm = NULL;
    } else if (pcm_prepare(mPcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mPcm, pcm_get_error(mPcm));
        pcm_close(mPcm);
        mPcm = NULL;
    }

    if (mPcmflag & PCM_MMAP) {
        audio_pcm_write_wrapper_fp = pcm_mmap_write;
    } else {
        audio_pcm_write_wrapper_fp = pcm_write;
    }

    ALOGD("%s(), pcm device = %d flag = 0x%x mPcm = %p", __FUNCTION__, device, flag, mPcm);
    ASSERT(mPcm != NULL);
    return NO_ERROR;

}

status_t AudioALSAPlaybackHandlerBase::pcmWrite(struct pcm *pcm, const void *data, unsigned int count) {
    return audio_pcm_write_wrapper_fp(pcm, data, count);
}

status_t AudioALSAPlaybackHandlerBase::openPcmDriver(const unsigned int device) {
    return openPcmDriverWithFlag(device, PCM_OUT | PCM_MONOTONIC);
}

status_t AudioALSAPlaybackHandlerBase::closePcmDriver() {
    ALOGV("+%s(), mPcm = %p", __FUNCTION__, mPcm);

    if (mPcm != NULL) {
        pcm_stop(mPcm);
        pcm_close(mPcm);
        mPcm = NULL;
    }

    ALOGD("-%s(), mPcm = %p", __FUNCTION__, mPcm);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerBase::openComprDriver(const unsigned int device) {
    ALOGD("+%s(), compr device = %d", __FUNCTION__, device);
    ASSERT(mComprStream == NULL);
    mComprStream = compress_open(AudioALSADeviceParser::getInstance()->GetCardIndex(),
                                 device, COMPRESS_IN, &mComprConfig);
    if (mComprStream == NULL) {
        ALOGE("%s(), mComprStream == NULL!!", __FUNCTION__);
        return INVALID_OPERATION;
    } else if (is_compress_ready(mComprStream) == false) {
        ALOGE("%s(), compress device open fail:%s", __FUNCTION__, compress_get_error(mComprStream));
        compress_close(mComprStream);
        mComprStream = NULL;
        return INVALID_OPERATION;
    }
    ALOGD("-%s(), mComprStream = %p", __FUNCTION__, mComprStream);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerBase::closeComprDriver() {
    ALOGD("+%s(), mComprStream = %p", __FUNCTION__, mComprStream);

    if (mComprStream != NULL) {
        //close compress driver
        compress_stop(mComprStream);
        compress_close(mComprStream);
        mComprStream = NULL;
    }

    ALOGD("-%s(), mComprStream = %p", __FUNCTION__, mComprStream);
    return NO_ERROR;

}

status_t AudioALSAPlaybackHandlerBase::setComprCallback(stream_callback_t StreamCbk, void *CbkCookie) {
    mStreamCbk = StreamCbk;
    mCbkCookie = CbkCookie;
    return NO_ERROR;
}

int AudioALSAPlaybackHandlerBase::updateAudioMode(audio_mode_t mode) {
    ALOGV("%s(), mode %d", __FUNCTION__, mode);
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    uint32_t newAurisysScenario = GetAurisysScenario();
    ALOGV("%s(), mAurisysScenario = %d -> %d", __FUNCTION__, mAurisysScenario, newAurisysScenario);
    if (mAurisysLibManager && get_aurisys_on()) {
        if (mAurisysScenario != newAurisysScenario) {
            if ((mAurisysScenario == AURISYS_SCENARIO_PLAYBACK_NORMAL ||
                mAurisysScenario == AURISYS_SCENARIO_PLAYBACK_LOW_LATENCY) &&
                newAurisysScenario == AURISYS_SCENARIO_VOIP) {
                // change from normal to voip need delay
                mIsNeedUpdateLib = true;
            } else if ((mAurisysScenario == AURISYS_SCENARIO_VOIP) &&
                       (newAurisysScenario == AURISYS_SCENARIO_PLAYBACK_LOW_LATENCY)) {
                // change form voip to fast need delay for potential underflow
                mIsNeedUpdateLib = true;
            } else if (mAurisysScenario != AURISYS_SCENARIO_PLAYBACK_LOW_LATENCY) {
                mIsNeedUpdateLib = false;
                DestroyAurisysLibManager();
                CreateAurisysLibManager();
            }
        } else {
            mIsNeedUpdateLib = false;
        }
   }
#endif
    return 0;
}

int AudioALSAPlaybackHandlerBase::preWriteOperation(const void *buffer, size_t bytes) {
    ALOGV("%s(), buffer %p, bytes %zu, mIsNeedUpdateLib %d", __FUNCTION__, buffer, bytes, mIsNeedUpdateLib);
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#define NEED_UPDATE_COUNT_FAST 5
#define NEED_UPDATE_COUNT 1
    // reopen aurisys when one hal buffer is in low level, below -60dB
    if (!mIsNeedUpdateLib) {
        return 0;
    }

    bool lowVolume = true;
    uint32_t need_update_count = 0;
    size_t tmp_bytes = bytes;

    if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_FAST) {
        need_update_count = NEED_UPDATE_COUNT_FAST;
    } else {
        need_update_count = NEED_UPDATE_COUNT;
    }

    if (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) {
        int16_t *sample = (int16_t *)buffer;
        int16_t threshold = 32;  // below -60dB, ((2^16) >> 10) / 2 = 32

        while (tmp_bytes > 0) {
            if (((*sample) > threshold) || ((*sample) < (-1 * threshold))) {
                lowVolume = false;
                break;
            }
            tmp_bytes -= audio_bytes_per_sample(mStreamAttributeSource->audio_format);
            sample++;
        }
    } else {    // assume AUDIO_FORMAT_PCM_32_BIT
        int32_t *sample = (int32_t *)buffer;
        int32_t threshold = 2097152;   // below -60dB, ((2^32) >> 10) / 2 = 2097152

        while (tmp_bytes > 0) {
            if (((*sample) > threshold) || ((*sample) < (-1 * threshold))) {
                lowVolume = false;
                break;
            }
            tmp_bytes -= audio_bytes_per_sample(mStreamAttributeSource->audio_format);
            sample++;
        }
    }

    if (lowVolume == false) {
        mUpdateLibCount = 0; // reset update lib count
    } else {
        mUpdateLibCount++;
    }

    if (mUpdateLibCount > need_update_count) {
        mIsNeedUpdateLib = false;
        mUpdateLibCount = 0;
        DestroyAurisysLibManager();
        CreateAurisysLibManager();
    }

#endif
    return 0;
}

status_t AudioALSAPlaybackHandlerBase::doStereoToMonoConversionIfNeed(void *buffer __unused, size_t bytes __unused) {
#ifndef ENABLE_STEREO_SPEAKER
    if (mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_SPEAKER) {
        if (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT) {
            int32_t *Sample = (int32_t *)buffer;
            while (bytes > 0) {
                int32_t averageValue = ((*Sample) >> 1) + ((*(Sample + 1)) >> 1);
                *Sample++ = averageValue;
                *Sample++ = averageValue;
                bytes -= 8;
            }
        } else if (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) {
            int16_t *Sample = (int16_t *)buffer;
            while (bytes > 0) {
                int16_t averageValue = ((*Sample) >> 1) + ((*(Sample + 1)) >> 1);
                *Sample++ = averageValue;
                *Sample++ = averageValue;
                bytes -= 4;
            }
        }
    }
#endif
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerBase::setScreenState(bool mode __unused,
                                                      size_t buffer_size __unused,
                                                      size_t reduceInterruptSize __unused,
                                                      bool bforce __unused) {
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerBase::getHardwareBufferInfo(time_info_struct_t *HWBuffer_Time_Info) {
#if defined(CONFIG_MT_ENG_BUILD)
    ALOGV("+%s()", __FUNCTION__);
#endif

    if (mComprStream == NULL) {
        ASSERT(mPcm != NULL);
    } else {
        ALOGD("%s(), no pcm handler, return directly", __FUNCTION__);
        return NO_ERROR;
    }

    if (mTimeStampValid == false) {
        ALOGV("getHardwareBufferInfo: It doesn't start to fetch data on PCM buffer");
        return UNKNOWN_ERROR;
    }

    ASSERT(mPcm != NULL);
    int ret = pcm_get_htimestamp(mPcm, &HWBuffer_Time_Info->frameInfo_get, &HWBuffer_Time_Info->timestamp_get);
    if (ret != 0) {
        ALOGE("-%s(), pcm_get_htimestamp fail, ret = %d, pcm_get_error = %s", __FUNCTION__, ret, pcm_get_error(mPcm));
        return UNKNOWN_ERROR;
    } else {
        // kernel total buffer size to frame
        HWBuffer_Time_Info->buffer_per_time = pcm_bytes_to_frames(mPcm, mStreamAttributeTarget.buffer_size);

        HWBuffer_Time_Info->halQueuedFrame = mHalQueuedFrame;
    }
    ALOGV("-%s, frameInfo_get = %u, mStreamAttributeTarget.buffer_size = %d, buffer_per_time = %u, halQueuedFrame = %d",
          __FUNCTION__, HWBuffer_Time_Info->frameInfo_get, mStreamAttributeTarget.buffer_size, HWBuffer_Time_Info->buffer_per_time, HWBuffer_Time_Info->halQueuedFrame);
    return NO_ERROR;
}
status_t AudioALSAPlaybackHandlerBase::get_timeStamp(unsigned long *frames, unsigned int *samplerate) {
    if (mComprStream == NULL) {
        ALOGE("%s(), mComprStream NULL", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    if (compress_get_tstamp(mComprStream, frames, samplerate) == 0) {
        ALOGV("%s(), frames:%lu, samplerate:%u", __FUNCTION__, *frames, *samplerate);
        return NO_ERROR;
    } else {
        ALOGE("%s get_tstamp fail %s\n", __FUNCTION__, compress_get_error(mComprStream));
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerBase::updateHardwareBufferInfo(size_t sourceWrittenBytes, uint32_t targetWrittenBytes) {
    // calculated hal queued buffer
    size_t sourceSizePerFrame = getSizePerFrame(mStreamAttributeSource->audio_format,
                                                mStreamAttributeSource->num_channels);
    size_t targetSizePerFrame = getSizePerFrame(mStreamAttributeTarget.audio_format,
                                                mStreamAttributeTarget.num_channels);

    size_t inBytesInHandler = ((uint64_t)sourceWrittenBytes * mStreamAttributeTarget.sample_rate * targetSizePerFrame) /
                              (sourceSizePerFrame * mStreamAttributeSource->sample_rate);

    if (inBytesInHandler >= targetWrittenBytes) {
        if (mPcm) {
            mHalQueuedFrame += pcm_bytes_to_frames(mPcm, inBytesInHandler - targetWrittenBytes);
        } else {
            mHalQueuedFrame += (inBytesInHandler - targetWrittenBytes) / targetSizePerFrame;
        }
    } else {
        if (mPcm) {
            mHalQueuedFrame -= pcm_bytes_to_frames(mPcm, targetWrittenBytes - inBytesInHandler);
        } else {
            mHalQueuedFrame -= (targetWrittenBytes - inBytesInHandler) / targetSizePerFrame;
        }
    }

    return NO_ERROR;
}

playback_handler_t AudioALSAPlaybackHandlerBase::getPlaybackHandlerType() {
    return mPlaybackHandlerType;
}

status_t AudioALSAPlaybackHandlerBase::setFilterMng(AudioMTKFilterManager *pFilterMng __unused) {
    ALOGW("%s(), do nothing", __FUNCTION__);
    return INVALID_OPERATION;
}

status_t AudioALSAPlaybackHandlerBase::initPostProcessing() {
    // init post processing
    mPostProcessingOutputBufferSize = kMaxPcmDriverBufferSize;
    mPostProcessingOutputBuffer = new char[mPostProcessingOutputBufferSize];
    ASSERT(mPostProcessingOutputBuffer != NULL);

    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBase::deinitPostProcessing() {
    // deinit post processing
    if (mPostProcessingOutputBuffer) {
        delete []mPostProcessingOutputBuffer;
        mPostProcessingOutputBuffer = NULL;
        mPostProcessingOutputBufferSize = 0;
    }
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (mAudioFilterManagerHandler) {
        mAudioFilterManagerHandler->stop();
        mAudioFilterManagerHandler = NULL;
    }
#endif
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBase::doPostProcessing(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes) {
    // bypass downlink filter while DMNR tuning // TO DO Verification, HoChi
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (mAudioFilterManagerHandler && mStreamAttributeSource->BesRecord_Info.besrecord_dmnr_tuningEnable == false && mStreamAttributeSource->bBypassPostProcessDL == false)
    {
        if (inBytes > mPostProcessingOutputBufferSize) {
            ALOGW("%s(), inBytes %d > mPostProcessingOutputBufferSize %d", __FUNCTION__, inBytes, mPostProcessingOutputBufferSize);
            ASSERT(0);
            *ppOutBuffer = pInBuffer;
            *pOutBytes = inBytes;
        } else {
            mAudioFilterManagerHandler->start(mFirstDataWrite); // TODO(Harvey, Hochi), why start everytime in write() ??
            uint32_t outputSize = mAudioFilterManagerHandler->process(pInBuffer, inBytes, mPostProcessingOutputBuffer, mPostProcessingOutputBufferSize);
            if (outputSize == 0) {
                *ppOutBuffer = pInBuffer;
                *pOutBytes = inBytes;
            } else {
                *ppOutBuffer = mPostProcessingOutputBuffer;
                *pOutBytes = outputSize;
            }
        }
    } else
#endif
    { // bypass
        *ppOutBuffer = pInBuffer;
        *pOutBytes = inBytes;
    }

    ASSERT(*ppOutBuffer != NULL && *pOutBytes != 0);
    return NO_ERROR;
}

int32 AudioALSAPlaybackHandlerBase::initDcRemoval() {
    DCR_BITDEPTH depth = DCREMOVE_BIT24;
    mDcRemove = newMtkDcRemove();
    ASSERT(mDcRemove != NULL);
    if (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) {
        depth = DCREMOVE_BIT16;
    }
    mDcRemove->init(mStreamAttributeSource->num_channels, mStreamAttributeSource->sample_rate,
                    DCR_MODE_3, depth);
    mDcRemoveBufferSize = kMaxPcmDriverBufferSize;
    mDcRemoveWorkBuffer = new char[mDcRemoveBufferSize];
    memset(mDcRemoveWorkBuffer, 0, mDcRemoveBufferSize);
    ASSERT(mDcRemoveWorkBuffer != NULL);
    return NO_ERROR;

}

int32 AudioALSAPlaybackHandlerBase::deinitDcRemoval() {
    if (mDcRemove) {
        mDcRemove->close();
        deleteMtkDcRemove(mDcRemove);
        mDcRemove = NULL;
    }
    if (mDcRemoveWorkBuffer) {
        delete [] mDcRemoveWorkBuffer;
        mDcRemoveWorkBuffer = NULL;
    }
    return NO_ERROR;
}

int32 AudioALSAPlaybackHandlerBase::doDcRemoval(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes) {
    uint32_t num_process_data = mDcRemoveBufferSize;

    if (mDcRemove == NULL) {
        *ppOutBuffer = pInBuffer;
        *pOutBytes = inBytes;
    } else if (inBytes > mDcRemoveBufferSize) {
        ALOGW("%s(), inBytes %d > mDcRemoveBufferSize %d", __FUNCTION__, inBytes, mDcRemoveBufferSize);
        ASSERT(0);
        *ppOutBuffer = pInBuffer;
        *pOutBytes = inBytes;
    } else {
        mDcRemove->process(pInBuffer, &inBytes, mDcRemoveWorkBuffer, &num_process_data);
        *ppOutBuffer = mDcRemoveWorkBuffer;
        *pOutBytes = num_process_data;
    }
    ALOGV("%s(), inBytes: %d, pOutBytes: %d ppOutBuffer = %p", __FUNCTION__, inBytes, *pOutBytes, *ppOutBuffer);
    ASSERT(*ppOutBuffer != NULL && *pOutBytes != 0);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBase::initBliSrc() {
    // init BLI SRC if need
    if (mStreamAttributeSource->sample_rate  != mStreamAttributeTarget.sample_rate  ||
        mStreamAttributeSource->num_channels != mStreamAttributeTarget.num_channels) {
        ALOGD("%s(), sample_rate: %d => %d, num_channels: %d => %d, mStreamAttributeSource->audio_format: 0x%x", __FUNCTION__,
              mStreamAttributeSource->sample_rate,  mStreamAttributeTarget.sample_rate,
              mStreamAttributeSource->num_channels, mStreamAttributeTarget.num_channels,
              mStreamAttributeSource->audio_format);

        SRC_PCM_FORMAT src_pcm_format = SRC_IN_Q1P15_OUT_Q1P15;
        if (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT) {
            src_pcm_format = SRC_IN_Q1P31_OUT_Q1P31;
        } else if (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) {
            src_pcm_format = SRC_IN_Q1P15_OUT_Q1P15;
        } else {
            ALOGE("%s(), not support mStreamAttributeSource->audio_format(0x%x) SRC!!", __FUNCTION__, mStreamAttributeSource->audio_format);
        }

        mBliSrc = newMtkAudioSrc(
                      mStreamAttributeSource->sample_rate, mStreamAttributeSource->num_channels,
                      mStreamAttributeTarget.sample_rate,  mStreamAttributeTarget.num_channels,
                      src_pcm_format);
        ASSERT(mBliSrc != NULL);
        mBliSrc->open();

        mBliSrcOutputBuffer = new char[kBliSrcOutputBufferSize];
        ASSERT(mBliSrcOutputBuffer != NULL);
    }

    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBase::deinitBliSrc() {
    // deinit BLI SRC if need
    if (mBliSrc != NULL) {
        mBliSrc->close();
        deleteMtkAudioSrc(mBliSrc);
        mBliSrc = NULL;
    }

    if (mBliSrcOutputBuffer != NULL) {
        delete[] mBliSrcOutputBuffer;
        mBliSrcOutputBuffer = NULL;
    }

    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBase::doBliSrc(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes) {
    if (mBliSrc == NULL) { // No need SRC
        *ppOutBuffer = pInBuffer;
        *pOutBytes = inBytes;
    } else {
        char *p_read = (char *)pInBuffer;
        uint32_t num_raw_data_left = inBytes;
        uint32_t num_converted_data = kBliSrcOutputBufferSize; // max convert num_free_space

        uint32_t consumed = num_raw_data_left;
        mBliSrc->process((int16_t *)p_read, &num_raw_data_left,
                         (int16_t *)mBliSrcOutputBuffer, &num_converted_data);
        consumed -= num_raw_data_left;
        p_read += consumed;

        ALOGV("%s(), num_raw_data_left = %u, num_converted_data = %u",
              __FUNCTION__, num_raw_data_left, num_converted_data);

        if (num_raw_data_left > 0) {
            ALOGW("%s(), num_raw_data_left(%u) > 0", __FUNCTION__, num_raw_data_left);
            ASSERT(num_raw_data_left == 0);
        }

        *ppOutBuffer = mBliSrcOutputBuffer;
        *pOutBytes = num_converted_data;
    }

    ASSERT(*ppOutBuffer != NULL && *pOutBytes != 0);
    return NO_ERROR;
}


pcm_format AudioALSAPlaybackHandlerBase::transferAudioFormatToPcmFormat(const audio_format_t audio_format) const {
    pcm_format retval = PCM_FORMAT_S16_LE;

    switch (audio_format) {
    case AUDIO_FORMAT_PCM_8_BIT: {
        retval = PCM_FORMAT_S8;
        break;
    }
    case AUDIO_FORMAT_PCM_16_BIT: {
        retval = PCM_FORMAT_S16_LE;
        break;
    }
    case AUDIO_FORMAT_PCM_8_24_BIT: {
#if defined(MTK_AUDIO_KS)
        retval = PCM_FORMAT_S24_LE;
#else
        retval = PCM_FORMAT_S32_LE;
#endif
        break;
    }
    case AUDIO_FORMAT_PCM_32_BIT: {
        retval = PCM_FORMAT_S32_LE;
        break;
    }
    default: {
        ALOGE("No such audio format(0x%x)!! Use AUDIO_FORMAT_PCM_16_BIT(0x%x) instead", audio_format, PCM_FORMAT_S16_LE);
        retval = PCM_FORMAT_S16_LE;
        break;
    }
    }

    ALOGD("%s(), audio_format(0x%x) => pcm_format(0x%x)", __FUNCTION__, audio_format, retval);
    return retval;
}


status_t AudioALSAPlaybackHandlerBase::initBitConverter() {
    // init bit converter if need
    if (mStreamAttributeSource->audio_format != mStreamAttributeTarget.audio_format) {
        BCV_PCM_FORMAT bcv_pcm_format;
        bool isInputValid = true;
        if ((mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT) || (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_8_24_BIT)) {
            if (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) {
                bcv_pcm_format = BCV_IN_Q1P31_OUT_Q1P15;
            } else if (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_8_24_BIT) {
                bcv_pcm_format = BCV_IN_Q1P31_OUT_Q9P23;
            } else {
                isInputValid = false;
            }
        } else if (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) {
            if (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_32_BIT) {
                bcv_pcm_format = BCV_IN_Q1P15_OUT_Q1P31;
            } else if (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_8_24_BIT) {
                bcv_pcm_format = BCV_IN_Q1P15_OUT_Q9P23;
            } else {
                isInputValid = false;
            }
        } else if (mStreamAttributeSource->audio_format == AUDIO_FORMAT_MP3) { //doug for tunneling
            if (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) {
                return NO_ERROR;
            } else if (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_8_24_BIT) {
                bcv_pcm_format = BCV_IN_Q1P15_OUT_Q9P23;
            } else {
                isInputValid = false;
            }
        }

        if (!isInputValid) {
            ASSERT(0);
            ALOGD("%s(), invalid, audio_format: 0x%x => 0x%x",
                  __FUNCTION__, mStreamAttributeSource->audio_format, mStreamAttributeTarget.audio_format);
            return INVALID_OPERATION;
        }

        ALOGD("%s(), audio_format: 0x%x => 0x%x, bcv_pcm_format = 0x%x",
              __FUNCTION__, mStreamAttributeSource->audio_format, mStreamAttributeTarget.audio_format, bcv_pcm_format);

        if (mStreamAttributeSource->num_channels > 2) {
            mBitConverter = newMtkAudioBitConverter(
                                mStreamAttributeSource->sample_rate,
                                2,
                                bcv_pcm_format);
        } else {
            mBitConverter = newMtkAudioBitConverter(
                                mStreamAttributeSource->sample_rate,
                                mStreamAttributeSource->num_channels,
                                bcv_pcm_format);
        }

        ASSERT(mBitConverter != NULL);
        mBitConverter->open();
        mBitConverter->resetBuffer();

        mBitConverterOutputBuffer = new char[kMaxPcmDriverBufferSize];
        ASSERT(mBitConverterOutputBuffer != NULL);
        ASSERT(mBitConverterOutputBuffer != NULL);
    }

    ALOGV("%s(), mBitConverter = %p, mBitConverterOutputBuffer = %p", __FUNCTION__, mBitConverter, mBitConverterOutputBuffer);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBase::deinitBitConverter() {
    // deinit bit converter if need
    if (mBitConverter != NULL) {
        mBitConverter->close();
        deleteMtkAudioBitConverter(mBitConverter);
        mBitConverter = NULL;
    }

    if (mBitConverterOutputBuffer != NULL) {
        delete[] mBitConverterOutputBuffer;
        mBitConverterOutputBuffer = NULL;
    }

    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBase::doBitConversion(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes) {
    if (mBitConverter != NULL) {
        *pOutBytes = kPcmDriverBufferSize;
        mBitConverter->process(pInBuffer, &inBytes, (void *)mBitConverterOutputBuffer, pOutBytes);
        *ppOutBuffer = mBitConverterOutputBuffer;
    } else {
        *ppOutBuffer = pInBuffer;
        *pOutBytes = inBytes;
    }

    ASSERT(*ppOutBuffer != NULL && *pOutBytes != 0);
    return NO_ERROR;
}

// we assue that buufer should write as 64 bytes align , so only src handler is create,
// will cause output buffer is not 64 bytes align
status_t AudioALSAPlaybackHandlerBase::initDataPending() {
    ALOGV("mBliSrc = %p", mBliSrc);
    if (mBliSrc != NULL || mDataPendingForceUse) {
        mdataPendingOutputBufferSize = (1024 * 128) + mDataAlignedSize; // here nned to cover max write buffer size
        mdataPendingOutputBuffer = new char[mdataPendingOutputBufferSize];
        mdataPendingTempBuffer  = new char[mDataAlignedSize];
        ASSERT(mdataPendingOutputBuffer != NULL);
    }
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerBase::DeinitDataPending() {
    ALOGD("DeinitDataPending");
    if (mdataPendingOutputBuffer != NULL) {
        delete[] mdataPendingOutputBuffer;
        mdataPendingOutputBuffer = NULL;
    }
    if (mdataPendingTempBuffer != NULL) {
        delete[] mdataPendingTempBuffer ;
        mdataPendingTempBuffer = NULL;
    }
    mdataPendingOutputBufferSize = 0;
    mdataPendingRemindBufferSize = 0;
    return NO_ERROR;
}

// we assue that buufer should write as 64 bytes align , so only src handler is create,
// will cause output buffer is not 64 bytes align
status_t AudioALSAPlaybackHandlerBase::dodataPending(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes) {
    char *DataPointer = (char *)mdataPendingOutputBuffer;
    char *DatainputPointer = (char *)pInBuffer;
    uint32 TotalBufferSize  = inBytes + mdataPendingRemindBufferSize;
    uint32 tempRemind = TotalBufferSize % mDataAlignedSize;
    uint32 TotalOutputSize = TotalBufferSize - tempRemind;
    uint32 TotalOutputCount = TotalOutputSize;
    if (mBliSrc != NULL || mDataPendingForceUse) { // do data pending
        //ALOGD("inBytes = %d mdataPendingRemindBufferSize = %d TotalOutputSize = %d",inBytes,mdataPendingRemindBufferSize,TotalOutputSize);

        if (TotalOutputSize != 0) {
            if (mdataPendingRemindBufferSize != 0) { // deal previous remaind buffer
                memcpy((void *)DataPointer, (void *)mdataPendingTempBuffer, mdataPendingRemindBufferSize);
                DataPointer += mdataPendingRemindBufferSize;
                TotalOutputCount -= mdataPendingRemindBufferSize;
            }

            //deal with input buffer
            memcpy((void *)DataPointer, pInBuffer, TotalOutputCount);
            DataPointer += TotalOutputCount;
            DatainputPointer += TotalOutputCount;
            TotalOutputCount = 0;

            //ALOGD("tempRemind = %d pOutBytes = %d",tempRemind,*pOutBytes);

            // deal with remind buffer
            memcpy((void *)mdataPendingTempBuffer, (void *)DatainputPointer, tempRemind);
            mdataPendingRemindBufferSize = tempRemind;
        } else {
            // deal with remind buffer
            memcpy((void *)(mdataPendingTempBuffer + mdataPendingRemindBufferSize), (void *)DatainputPointer, inBytes);
            mdataPendingRemindBufferSize += inBytes;
        }

        // update pointer and data count
        *ppOutBuffer = mdataPendingOutputBuffer;
        *pOutBytes = TotalOutputSize;
    } else {
        *ppOutBuffer = pInBuffer;
        *pOutBytes = inBytes;
    }

    ASSERT(*ppOutBuffer != NULL);
    if (!mDataPendingForceUse) {
        ASSERT(*pOutBytes != 0);
    }
    return NO_ERROR;
}

/* mStreamAttributeTarget needs to be set first for printing format in file name */
void AudioALSAPlaybackHandlerBase::OpenPCMDump(const char *className) {
    ALOGV("%s()", __FUNCTION__);
    char mDumpFileName[128];
    sprintf(mDumpFileName, "%s.%d.%s.pid%d.tid%d.%d.%s.%dch.pcm", streamout, mDumpFileNum,
            className, getpid(), gettid(),
            mStreamAttributeTarget.sample_rate,
            transferAudioFormatToDumpString(mStreamAttributeTarget.audio_format),
            mStreamAttributeTarget.num_channels);

    mPCMDumpFile = NULL;
    mPCMDumpFile = AudioOpendumpPCMFile(mDumpFileName, streamout_propty);

    if (mPCMDumpFile != NULL) {
        ALOGD("%s DumpFileName = %s", __FUNCTION__, mDumpFileName);

        mDumpFileNum++;
        mDumpFileNum %= MAX_DUMP_NUM;
    }
}

void AudioALSAPlaybackHandlerBase::ClosePCMDump() {
    ALOGV("%s()", __FUNCTION__);
    if (mPCMDumpFile) {
        AudioCloseDumpPCMFile(mPCMDumpFile);
        ALOGD("%s(), close it", __FUNCTION__);
    }
}

void  AudioALSAPlaybackHandlerBase::WritePcmDumpData(const void *buffer, ssize_t bytes) {
    if (mPCMDumpFile) {
        //ALOGD("%s()", __FUNCTION__);
        AudioDumpPCMData((void *)buffer, bytes, mPCMDumpFile);
    }
}

#ifdef MTK_AUDIODSP_SUPPORT
int AudioALSAPlaybackHandlerBase::setDspDumpWakelock(bool condition) {
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "audio_dsp_wakelock"), 0, condition)) {
        ALOGW("%s(), enable fail", __FUNCTION__);
        return -1;
    }
    return 0;
}

void AudioALSAPlaybackHandlerBase::OpenPCMDumpDSP(const char *className, uint8_t task_scene) {
#define MAX_TASKNAME_LEN (128)

    const char *audio_dump = "/data/vendor/audiohal/audio_dump";

    char mDumpFileName[128];
    char task_name[MAX_TASKNAME_LEN];
    char value[PROPERTY_VALUE_MAX];
    int i, dsp_taskdump_property = 0;
    struct ipi_msg_t ipi_msg;
    FILE *pcm_dump = NULL;

    switch (task_scene) {
        case TASK_SCENE_PRIMARY:
            strncpy(task_name, "TaskPrimary", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_DEEPBUFFER:
            strncpy(task_name, "TaskDeepBuf", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_VOIP:
            strncpy(task_name, "TaskVoip", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_PLAYBACK_MP3:
            strncpy(task_name, "TaskOffload", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_AUDPLAYBACK:
            strncpy(task_name, "TaskPlayback", MAX_TASKNAME_LEN);
            break;
        default:
            strncpy(task_name, "TaskPrimary", MAX_TASKNAME_LEN);
            break;
    }

    property_get(streamoutdsp_propty, value, "0");
    dsp_taskdump_property = atoi(value);
    ALOGD("%s() flag %d, dsp_taskdump_property = %d",
          __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags, dsp_taskdump_property);

    if (dsp_taskdump_property) {
        setDspDumpWakelock(true);
        for (i = 0; i < DEBUG_PCMDUMP_NUM; i++) {
            get_task_pcmdump_info(task_scene, i, (void **)&pcm_dump);

            switch (i) {
                case DEBUG_PCMDUMP_IN:
                    sprintf(mDumpFileName, "%s/%s.%d.%d.%d.%s_datain.pcm",
                            audio_dump, className, mDumpFileNumDSP, getpid(), gettid(), task_name);
                    break;
                case DEBUG_PCMDUMP_OUT:
                    sprintf(mDumpFileName, "%s/%s.%d.%d.%d.%s_dataout.pcm",
                            audio_dump, className, mDumpFileNumDSP, getpid(), gettid(), task_name);
                    break;
                case DEBUG_PCMDUMP_RESERVED:
                    sprintf(mDumpFileName, "%s/%s.%d.%d.%d.%s_echoref.pcm",
                            audio_dump, className, mDumpFileNumDSP, getpid(), gettid(), task_name);
                    break;
                case DEBUG_PCMDUMP_IV:
                    sprintf(mDumpFileName, "%s/%s.%d.%d.%d.%s_ivdump.pcm",
                            audio_dump, className, mDumpFileNumDSP, getpid(), gettid(), task_name);
                    break;
                default:
                    sprintf(mDumpFileName, "%s/%s.%d.%d.%d.%s_datain.pcm",
                            audio_dump, className, mDumpFileNumDSP, getpid(), gettid(), task_name);
                    break;
            }

            mPCMDumpFileDSP = AudioOpendumpPCMFile(mDumpFileName, streamoutdsp_propty);
            if (mPCMDumpFileDSP != NULL) {
                ALOGD("%s() flag %d, DumpFileName = %s",
                      __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags, mDumpFileName);
            }
            set_task_pcmdump_info(task_scene, i, (void *)mPCMDumpFileDSP);
        }

        // send PCM_DUMP_ENABLE ipi to DSP
        mAudioMessengerIPI->sendIpiMsg(
            &ipi_msg,
            task_scene, AUDIO_IPI_LAYER_TO_DSP,
            AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
            AUDIO_DSP_TASK_PCMDUMP_ON, dsp_taskdump_property, 0,
            NULL);

        mDumpFileNumDSP++;
        mDumpFileNumDSP %= MAX_DUMP_NUM;
    } else {
        mAudioMessengerIPI->sendIpiMsg(
            &ipi_msg,
            task_scene, AUDIO_IPI_LAYER_TO_DSP,
            AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
            AUDIO_DSP_TASK_PCMDUMP_ON, dsp_taskdump_property, 0,
            NULL);
    }
}

void AudioALSAPlaybackHandlerBase::ClosePCMDumpDSP(uint8_t task_scene) {

    char value[PROPERTY_VALUE_MAX];
    int dsp_taskdump_property;
    FILE *pcm_dump = NULL;
    int i;

    ALOGD("%s() flag %d, task_scene = %d",
          __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags, task_scene);

    property_get(streamoutdsp_propty, value, "0");
    dsp_taskdump_property = atoi(value);

    for (i = 0; i < DEBUG_PCMDUMP_NUM; i++) {
        get_task_pcmdump_info(task_scene, i, (void **)&pcm_dump);
        if (pcm_dump != NULL) {
            AudioCloseDumpPCMFile(pcm_dump);
            set_task_pcmdump_info(task_scene, i, NULL);
        }
    }

    if (dsp_taskdump_property) {
        setDspDumpWakelock(false);
    }
}
#endif
#if defined(MTK_HYBRID_NLE_SUPPORT)
status_t AudioALSAPlaybackHandlerBase::initNLEProcessing() {
    status_t dRet;

    if (mNLEMnger != NULL) {
        return ALREADY_EXISTS;
    }

    mNLEMnger = AudioALSAHyBridNLEManager::getInstance();

    if (mNLEMnger == NULL) {
        ALOGE("[Err] NLE %s New Fail Line#%d", __FUNCTION__, __LINE__);
        return NO_MEMORY;
    }

    dRet = mNLEMnger->initPlayBackHandler(mPlaybackHandlerType, &mStreamAttributeTarget, this);

    if (dRet != NO_ERROR) {
        mNLEMnger = NULL;
        ALOGV("Unsupport the Handler NLE %s init Fail Line#%d", __FUNCTION__, __LINE__);
    }
    return dRet;
}


status_t AudioALSAPlaybackHandlerBase::deinitNLEProcessing() {
    status_t dRet;

    if (mNLEMnger != NULL) {
        dRet = mNLEMnger->deinitPlayBackHandler(mPlaybackHandlerType);
        if (dRet != NO_ERROR) {
            ALOGW("[Warn] NLE %s deinit Fail Line#%d", __FUNCTION__, __LINE__);
        }
        mNLEMnger = NULL;
        return dRet;
    } else {
        ALOGV("Unsupport the Handler NLE %s ObjNull Fail Line#%d", __FUNCTION__, __LINE__);
        return NO_INIT;
    }
}


status_t AudioALSAPlaybackHandlerBase::doNLEProcessing(void *pInBuffer, size_t inBytes) {
    size_t dWriteByte = 0;
    status_t dRet;

    if (mNLEMnger != NULL) {
        dRet = mNLEMnger->process(mPlaybackHandlerType, pInBuffer, inBytes);
        if (dRet != NO_ERROR) {
            ALOGV("[Warn] NLE %s Line#%d dRet %d", __FUNCTION__, __LINE__, dRet);
        }
        return dRet;
    } else {
        ALOGV("[Warn] NLE %s ObjNull Fail Line#%d", __FUNCTION__, __LINE__);
        return NO_INIT;
    }
}
#else
status_t AudioALSAPlaybackHandlerBase::initNLEProcessing() {
    return INVALID_OPERATION;
}

status_t AudioALSAPlaybackHandlerBase::deinitNLEProcessing() {
    return INVALID_OPERATION;
}

status_t AudioALSAPlaybackHandlerBase::doNLEProcessing(void *pInBuffer __unused, size_t inBytes __unused) {
    return INVALID_OPERATION;
}
#endif



/*
 * =============================================================================
 *                     Aurisys Framework 2.0
 * =============================================================================
 */

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
void AudioALSAPlaybackHandlerBase::CreateAurisysLibManager() {
    /* get scenario */
    mAurisysScenario = GetAurisysScenario();

    ALOGD("%s, voip: %d, mAurisysScenario: %u", __FUNCTION__, IsVoIPEnable(), mAurisysScenario);

    /* manager config */
    AUDIO_ALLOC_STRUCT(struct aurisys_lib_manager_config_t, mManagerConfig);

    mManagerConfig->aurisys_scenario = mAurisysScenario;
    mManagerConfig->arsi_process_type = ARSI_PROCESS_TYPE_DL_ONLY;
    mManagerConfig->audio_format = mStreamAttributeSource->audio_format;
    mManagerConfig->sample_rate = mStreamAttributeSource->sample_rate;
    mManagerConfig->frame_size_ms = 20; /* TODO */
    mManagerConfig->num_channels_ul = 2;
    mManagerConfig->num_channels_dl = mStreamAttributeSource->num_channels;
    mManagerConfig->core_type = AURISYS_CORE_HAL;

    /* custom info */
#if (defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT) && (MTK_AUDIO_TUNING_TOOL_V2_PHASE >= 2))
    AppOps *appOps = appOpsGetInstance();
    if (appOps && appOps->appHandleIsFeatureOptionEnabled(appOps->appHandleGetInstance(), "VIR_SCENE_CUSTOMIZATION_SUPPORT")) {
        snprintf(mManagerConfig->custom_info, MAX_CUSTOM_INFO_LEN, "SetAudioCustomScene=%s",
                 mStreamAttributeSource->mCustScene);
    }
#endif

    /* task config */
    InitArsiTaskConfig(mManagerConfig);

    /* create manager */
    AL_AUTOLOCK(mAurisysLibManagerLock);

    mAurisysLibManager = create_aurisys_lib_manager(mManagerConfig);
    InitBufferConfig(mAurisysLibManager);

    aurisys_parsing_param_file(mAurisysLibManager);
    aurisys_create_arsi_handlers(mAurisysLibManager); /* should init task/buf configs first */
    aurisys_pool_buf_formatter_init(mAurisysLibManager); /* should init task/buf configs first */
    //aurisys_set_dl_digital_gain(mAurisysLibManager, 0, 0);
}

uint32_t AudioALSAPlaybackHandlerBase::GetAurisysScenario() {
    uint32_t ret = 0;
    /* scenario & sample rate */
    if (IsVoIPEnable()) {
        ret = AURISYS_SCENARIO_VOIP; /* VoIP DL */
    } else if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_FAST) {
        ret = AURISYS_SCENARIO_PLAYBACK_LOW_LATENCY;
    } else {
        ret = AURISYS_SCENARIO_PLAYBACK_NORMAL;
    }
    return ret;
}


/* TODO: move to aurisys framework?? add a new struct to keep hal arributes */
void AudioALSAPlaybackHandlerBase::InitArsiTaskConfig(
    struct aurisys_lib_manager_config_t *pManagerConfig) {
    struct arsi_task_config_t *pTaskConfig = &pManagerConfig->task_config;

    /* input device */ /* TODO: get voip ul attribute */
    pTaskConfig->input_device_info.devices = mStreamAttributeSource->input_device; /* TODO */
    pTaskConfig->input_device_info.audio_format = mStreamAttributeSource->audio_format;
    pTaskConfig->input_device_info.sample_rate = mStreamAttributeSource->sample_rate; /* TODO */
    pTaskConfig->input_device_info.channel_mask = mStreamAttributeSource->audio_channel_mask;
    pTaskConfig->input_device_info.num_channels = mStreamAttributeSource->num_channels;
    if (mStreamAttributeSource->output_devices == AUDIO_DEVICE_OUT_WIRED_HEADSET) {
        switch (AudioALSAHardwareResourceManager::getInstance()->getNumOfHeadsetPole()) {
            case 4:
                pTaskConfig->input_device_info.num_channels = 1;
                break;
            case 5:
                pTaskConfig->input_device_info.num_channels = 2;
                break;
            default:
                pTaskConfig->input_device_info.num_channels = 1;
                ALOGE("%s(), can't found matched pole number, use 1 ch", __FUNCTION__);
                break;
        }
    }
    pTaskConfig->input_device_info.hw_info_mask = 0; /* TODO */

    /* output device */
    audio_devices_t output_devices = mStreamAttributeSource->output_devices;
    if (isBtSpkDevice(output_devices)) {
        // use SPK setting for BTSCO + SPK
        output_devices = (audio_devices_t)(output_devices & (~AUDIO_DEVICE_OUT_ALL_SCO));
    }

    pTaskConfig->output_device_info.devices = mStreamAttributeSource->output_devices;
    pTaskConfig->output_device_info.audio_format = mStreamAttributeSource->audio_format;
    pTaskConfig->output_device_info.sample_rate = mStreamAttributeSource->sample_rate;
    pTaskConfig->output_device_info.channel_mask = mStreamAttributeSource->audio_channel_mask;
    pTaskConfig->output_device_info.num_channels = mStreamAttributeSource->num_channels;
    if (AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        pTaskConfig->output_device_info.hw_info_mask = OUTPUT_DEVICE_HW_INFO_SMARTPA_SPEAKER; /* SMARTPA */
    } else {
        pTaskConfig->output_device_info.hw_info_mask = 0;
    }

    /* task scene */
    pTaskConfig->task_scene = map_aurisys_scenario_to_task_scene(
        pManagerConfig->core_type,
        pManagerConfig->aurisys_scenario);

    /* audio mode */
    pTaskConfig->audio_mode = mStreamAttributeSource->audio_mode;

    /* stream type */
    pTaskConfig->stream_type = mStreamAttributeSource->stream_type;

    /* max device capability for allocating memory */
    pTaskConfig->max_input_device_sample_rate  = 48000; /* TODO */
    pTaskConfig->max_output_device_sample_rate = 48000; /* TODO */

    pTaskConfig->max_input_device_num_channels  =
        AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport();
    pTaskConfig->max_output_device_num_channels = 2; /* TODO */

    /* flag & source */
    pTaskConfig->output_flags = mStreamAttributeSource->mAudioOutputFlags;
    pTaskConfig->input_source = mStreamAttributeSource->input_source;
    pTaskConfig->input_flags  = 0;

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


    dump_task_config(pTaskConfig);
}


void AudioALSAPlaybackHandlerBase::InitBufferConfig(struct aurisys_lib_manager_t *manager) {
    /* DL in */
    mAudioPoolBufDlIn = create_audio_pool_buf(manager, DATA_BUF_DOWNLINK_IN, 0);

    mAudioPoolBufDlIn->buf->b_interleave = 1; /* LRLRLRLR*/
    mAudioPoolBufDlIn->buf->frame_size_ms = 0;
    mAudioPoolBufDlIn->buf->num_channels = mStreamAttributeSource->num_channels;
    mAudioPoolBufDlIn->buf->sample_rate_buffer = mStreamAttributeSource->sample_rate;
    mAudioPoolBufDlIn->buf->sample_rate_content = mStreamAttributeSource->sample_rate;
    mAudioPoolBufDlIn->buf->audio_format = mStreamAttributeSource->audio_format;


    /* DL out */
    mAudioPoolBufDlOut = create_audio_pool_buf(manager, DATA_BUF_DOWNLINK_OUT, 0);

    mAudioPoolBufDlOut->buf->b_interleave = 1; /* LRLRLRLR*/
    mAudioPoolBufDlOut->buf->frame_size_ms = 0;
    mAudioPoolBufDlOut->buf->num_channels = mStreamAttributeTarget.num_channels;
    mAudioPoolBufDlOut->buf->sample_rate_buffer = mStreamAttributeTarget.sample_rate;
    mAudioPoolBufDlOut->buf->sample_rate_content = mStreamAttributeTarget.sample_rate;
    mAudioPoolBufDlOut->buf->audio_format = mStreamAttributeTarget.audio_format;
}


void AudioALSAPlaybackHandlerBase::DestroyAurisysLibManager() {
    ALOGD("%s()", __FUNCTION__);

    AL_AUTOLOCK(mAurisysLibManagerLock);

    aurisys_destroy_arsi_handlers(mAurisysLibManager);
    aurisys_pool_buf_formatter_deinit(mAurisysLibManager);
    destroy_aurisys_lib_manager(mAurisysLibManager);
    mAurisysLibManager = NULL;

    /* NOTE: auto destroy audio_pool_buf when destroy_aurisys_lib_manager() */
    mAudioPoolBufUlIn = NULL;
    mAudioPoolBufUlOut = NULL;
    mAudioPoolBufDlIn = NULL;
    mAudioPoolBufDlOut = NULL;

    mIsNeedUpdateLib = false;

    AUDIO_FREE_POINTER(mLinearOut->p_buffer);
    memset(mLinearOut, 0, sizeof(data_buf_t));

    AUDIO_FREE_POINTER(mManagerConfig);
}


uint32_t AudioALSAPlaybackHandlerBase::GetTransferredBufferSize(uint32_t sourceBytes,
                                                                const stream_attribute_t *source,
                                                                const stream_attribute_t *target) {

    uint32_t bytesPerSampleSource = (uint32_t)audio_bytes_per_sample(source->audio_format);
    uint32_t bytesPerSampleTarget = (uint32_t)audio_bytes_per_sample(target->audio_format);

    uint32_t bytesPerSecondSource = source->sample_rate * source->num_channels * bytesPerSampleSource;
    uint32_t bytesPerSecondTarget = target->sample_rate * target->num_channels * bytesPerSampleTarget;

    uint32_t unitTargetBytes = bytesPerSampleTarget * target->num_channels;
    uint32_t targetBytes = 0;

    if (bytesPerSecondSource == 0 || bytesPerSecondTarget == 0 || unitTargetBytes == 0) {
        ALOGW("%s(), audio_format: 0x%x, 0x%x error!!", __FUNCTION__,
              source->audio_format, target->audio_format);
        return sourceBytes;
    }

    targetBytes = (uint32_t)((double)sourceBytes *
                              ((double)bytesPerSecondTarget /
                               (double)bytesPerSecondSource));
    if ((targetBytes % unitTargetBytes) != 0) {
        targetBytes = ((targetBytes / unitTargetBytes) + 1) * unitTargetBytes; // cell()
    }

    return targetBytes;
}

#endif /* end if MTK_AURISYS_FRAMEWORK_SUPPORT */

int AudioALSAPlaybackHandlerBase::getLatency() {

#ifdef MTK_AUDIODSP_SUPPORT
    int latency = 0;
    const uint8_t size_per_channel = (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_8_BIT ? 1 :
                                      (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT ? 2 :
                                       (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_32_BIT ? 4 :
                                        (mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_8_24_BIT ? 4 :
                                         2))));
    const uint8_t size_per_frame = mStreamAttributeTarget.num_channels * size_per_channel;
    latency = (mStreamAttributeTarget.buffer_size * 1000) / (mStreamAttributeTarget.sample_rate * size_per_frame);

    if(mStreamAttributeSource->output_devices & AUDIO_DEVICE_OUT_ALL_A2DP) {
        mStreamAttributeTarget.dspLatency = AudioDspStreamManager::getInstance()->getA2dpPcmLatency() +
                                            AudioALSAHardwareResourceManager::getInstance()->getA2dpLatency();
    } else {
        mStreamAttributeTarget.dspLatency = AudioDspStreamManager::getInstance()->getDlLatency();
    }
    latency += mStreamAttributeTarget.dspLatency;
    return latency;
#else
    return -ENOSYS;
#endif
}

#ifdef MTK_AUDIODSP_SUPPORT
void AudioALSAPlaybackHandlerBase::get_task_pcmdump_info(int task_id, int param, void **pcm_dump) {
    *pcm_dump = pcmdump_array[task_id][param];

    ALOGV("%s() %p %d %d\n", __FUNCTION__, *((FILE **)pcm_dump), task_id, param);
}

void AudioALSAPlaybackHandlerBase::set_task_pcmdump_info(int task_id, int param, void *pcm_dump) {
    pcmdump_array[task_id][param] = (FILE *)pcm_dump;
    ALOGV("%s() %p %d %d\n", __FUNCTION__, pcmdump_array[task_id][param], task_id, param);
}

void AudioALSAPlaybackHandlerBase::processDmaMsg(
    struct ipi_msg_t *msg,
    void *buf,
    uint32_t size) {
    FILE *pcm_dump = NULL;

    ALOGV("%s() msg_id=0x%x, task_scene=%d, param2=0x%x, size=%d\n",
          __FUNCTION__, msg->msg_id, msg->task_scene, msg->param2, size);

    switch (msg->msg_id) {
    case AUDIO_DSP_TASK_PCMDUMP_DATA:
        get_task_pcmdump_info(msg->task_scene, msg->param2, (void **)&pcm_dump);
        if (pcm_dump != NULL) {
            AudioDumpPCMData(buf, size, pcm_dump);
        }
        break;

    default:
        break;
    }
}

void AudioALSAPlaybackHandlerBase::processDmaMsgWrapper(
    struct ipi_msg_t *msg,
    void *buf,
    uint32_t size,
    void *arg) {

    AudioALSAPlaybackHandlerBase *pAudioALSAPlaybackHandlerBase =
        static_cast<AudioALSAPlaybackHandlerBase *>(arg);

    if (pAudioALSAPlaybackHandlerBase != NULL) {
        pAudioALSAPlaybackHandlerBase->processDmaMsg(msg, buf, size);
    }
}
#endif

void AudioALSAPlaybackHandlerBase::updateSmootherTime(const uint64_t bufferTimeUs) {
    if (mSmoother != NULL) {
        updateWriteSmootherTime(mSmoother, bufferTimeUs);
    }
}

status_t AudioALSAPlaybackHandlerBase::start() {
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerBase::stop() {
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerBase::createMmapBuffer(int32_t min_size_frames __unused,
                              struct audio_mmap_buffer_info *info __unused) {
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerBase::getMmapPosition(struct audio_mmap_position *position __unused) {
    return NO_ERROR;
}

} // end of namespace android

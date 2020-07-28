#include "AudioALSAPlaybackHandlerBTCVSD.h"

#include <inttypes.h>
#include <time.h>

#include "AudioUtility.h"

#include "WCNChipController.h"
#include "AudioALSACaptureDataProviderEchoRefBTCVSD.h"
#include "AudioALSACaptureDataProviderBTCVSD.h"
#include "AudioALSAStreamManager.h"
#include "AudioBTCVSDControl.h"
#include "AudioALSADriverUtility.h"

#if (defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT) && (MTK_AUDIO_TUNING_TOOL_V2_PHASE >= 2))
#include "AudioParamParser.h"
#endif


#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <audio_ringbuf.h>
#include <audio_pool_buf_handler.h>

#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>
#endif



#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerBTCVSD"

//#define DEBUG_TIMESTAMP

#ifdef DEBUG_TIMESTAMP
#define SHOW_TIMESTAMP(format, args...) ALOGD(format, ##args)
#else
#define SHOW_TIMESTAMP(format, args...)
#endif

#define BUFFER_SIZE_PER_ACCESSS_32BIT     (8192)
#define BUFFER_SIZE_PER_ACCESSS_16BIT     (4096)

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)

#define BT_IRQ_PERIOD_US 22500

namespace android {
/*==============================================================================
 *                     Implementation
 *============================================================================*/

static bool mBTMode_Open;

AudioALSAPlaybackHandlerBTCVSD::AudioALSAPlaybackHandlerBTCVSD(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source),
    mTotalEchoRefBufSize(0),
    mDataProviderEchoRefBTCVSD(AudioALSACaptureDataProviderEchoRefBTCVSD::getInstance()),
    mWCNChipController(WCNChipController::GetInstance()),
    mAudioBTCVSDControl(AudioBTCVSDControl::getInstance()),
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    mInitWrite(true),
    mWrittenFrame(0)
#ifndef MTK_SUPPORT_BTCVSD_ALSA
    , mFd2(mAudioBTCVSDControl->getFd())
#else
    , mFd2(-1)
#endif
{
    ALOGD("%s()", __FUNCTION__);
    mPlaybackHandlerType = PLAYBACK_HANDLER_BT_CVSD;

    /* Init EchoRef Resource */
    memset(&mEchoRefStartTime, 0, sizeof(mEchoRefStartTime));

    /* Init EchoRef Resource */
    memset(&mStreamAttributeTargetEchoRef, 0, sizeof(mStreamAttributeTargetEchoRef));

    /* Init timestamp*/
    memset(&mNewtime, 0, sizeof(mNewtime));
    memset(&mOldtime, 0, sizeof(mOldtime));
}


AudioALSAPlaybackHandlerBTCVSD::~AudioALSAPlaybackHandlerBTCVSD() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSAPlaybackHandlerBTCVSD::open() {
    ALOGD("+%s(), mDevice = 0x%x, sample_rate = %d, num_channels = %d, buffer_size = %d, audio_format = %d",
          __FUNCTION__, mStreamAttributeSource->output_devices,
          mStreamAttributeSource->sample_rate, mStreamAttributeSource->num_channels,
          mStreamAttributeSource->buffer_size, mStreamAttributeSource->audio_format);

    // HW attribute config // TODO(Harvey): query this
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = mWCNChipController->GetBTCurrentSamplingRateNumber();

    // Setup echoref stream attribute
    mStreamAttributeTargetEchoRef.audio_format = mStreamAttributeTarget.audio_format;
    mStreamAttributeTargetEchoRef.audio_channel_mask = mStreamAttributeTarget.audio_channel_mask;
    mStreamAttributeTargetEchoRef.num_channels = mStreamAttributeTarget.num_channels;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    mStreamAttributeTargetEchoRef.sample_rate = mStreamAttributeTarget.sample_rate;
#else
    mStreamAttributeTargetEchoRef.sample_rate = mStreamAttributeSource->sample_rate;    // No src applied, using source's sample rate
#endif

#ifdef MTK_SUPPORT_BTCVSD_ALSA
    //////copy from Capture
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;
    // total buffer size must be multiple of SCO_TX_ENCODE_SIZE
    mConfig.period_size = 45;   // 1 bt interrupt = 22.5ms, = 180 bytes, 180 / 2ch / 2 byte (16bit) = 45
    mConfig.period_count = 6;   // 6 bt interrupt
    mConfig.format = PCM_FORMAT_S16_LE;
    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;

    ASSERT(mPcm == NULL);
    int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmBTCVSDPlayback);
    int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmBTCVSDPlayback);
    mPcm = pcm_open(cardIdx, pcmIdx, PCM_OUT, &mConfig);
    ASSERT(mPcm != NULL && pcm_is_ready(mPcm) == true);

    // debug pcm dump
    OpenPCMDump(LOG_TAG);

    if (pcm_prepare(mPcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mPcm, pcm_get_error(mPcm));
        pcm_close(mPcm);
        mPcm = NULL;
    }
#endif
    mBTMode_Open = mAudioBTCVSDControl->BT_SCO_isWideBand();
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "BTCVSD Band"), mBTMode_Open ? "WB" : "NB")) {
        ALOGE("Error: BTCVSD Band invalid value");
    }

    uint32_t bufferSizePerAccess = mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT ?
                                   BUFFER_SIZE_PER_ACCESSS_16BIT : BUFFER_SIZE_PER_ACCESSS_32BIT;

    ALOGD("%s(), data align to %d", __FUNCTION__, bufferSizePerAccess);
    mDataAlignedSize = bufferSizePerAccess;
    mDataPendingForceUse = true;

    initDataPending();

    // init DC Removal
    initDcRemoval();

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (get_aurisys_on() && !mStreamAttributeSource->isMixerOut) {
        // open mAudioBTCVSDControl
        mAudioBTCVSDControl->BTCVSD_Init(mFd2, mStreamAttributeTarget.sample_rate, mStreamAttributeTarget.num_channels);

        CreateAurisysLibManager();
    } else
#endif
    {
        // open mAudioBTCVSDControl
        mAudioBTCVSDControl->BTCVSD_Init(mFd2, mStreamAttributeTarget.sample_rate, mStreamAttributeTarget.num_channels);

        // SRC
        initBliSrc();

        // bit conversion
        initBitConverter();
    }

    /* Reset software timestamp information */
    mTotalEchoRefBufSize = 0;
    memset((void *)&mEchoRefStartTime, 0, sizeof(mEchoRefStartTime));

    ALOGD("-%s(), mStreamAttributeTarget, ch=%d, sr=%d, mStreamAttributeTargetEchoRef, ch=%d, sr=%d",
        __FUNCTION__,
        mStreamAttributeTarget.num_channels,
        mStreamAttributeTarget.sample_rate,
        mStreamAttributeTargetEchoRef.num_channels,
        mStreamAttributeTargetEchoRef.sample_rate);

    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBTCVSD::close() {
    ALOGD("+%s()", __FUNCTION__);

#ifndef MTK_SUPPORT_BTCVSD_ALSA // clean bt buffer before closing, alsa version will clean in kernel close
    const uint32_t mute_buf_len = mDataAlignedSize;
    char mute_buf[mute_buf_len];
    memset(mute_buf, 0, mute_buf_len);

    this->write(mute_buf, mute_buf_len);
    this->write(mute_buf, mute_buf_len);
    this->write(mute_buf, mute_buf_len);
    this->write(mute_buf, mute_buf_len);
#endif

#ifdef MTK_SUPPORT_BTCVSD_ALSA
    // close pcm driver
    closePcmDriver();
#endif

    // close mAudioBTCVSDControl
    mAudioBTCVSDControl->BTCVSD_StandbyProcess(mFd2);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (get_aurisys_on() && !mStreamAttributeSource->isMixerOut) {
        DestroyAurisysLibManager();
    } else
#endif
    {
        // bit conversion
        deinitBitConverter();

        // SRC
        deinitBliSrc();
    }
    //DC removal
    deinitDcRemoval();

    DeinitDataPending();

    // debug pcm dump
    ClosePCMDump();


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBTCVSD::routing(const audio_devices_t output_devices __unused) {
    return INVALID_OPERATION;
}

int AudioALSAPlaybackHandlerBTCVSD::getLatency() {
    int latency;

    // buffer size bytes in kernel
    latency = mConfig.period_size *
              mConfig.period_count *
              getSizePerFrame(audio_format_from_pcm_format(mConfig.format), mConfig.channels);

    // buffer size (encoded data) to latency ms
    latency = ((latency / SCO_TX_ENCODE_SIZE / 3) * BT_IRQ_PERIOD_US) / 1000;

    return latency;
}

ssize_t AudioALSAPlaybackHandlerBTCVSD::write(const void *buffer, size_t bytes) {
#ifdef MTK_SUPPORT_BTCVSD_ALSA
    if (mPcm == NULL) {
        ALOGE("%s(), mPcm == NULL, return", __FUNCTION__);
        return bytes;
    }
#endif
    ALOGV("%s(), buffer = %p, bytes = %zu", __FUNCTION__, buffer, bytes);

    if (mPCMDumpFile) {
        clock_gettime(CLOCK_REALTIME, &mNewtime);
        latencyTime[0] = calc_time_diff(mNewtime, mOldtime);
        mOldtime = mNewtime;
    }

    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);
    ASSERT(pBuffer != NULL);

    void *pBufferAfterDataPending = NULL;
    uint32_t bytesAfterDataPending = 0;
    if (mDataPendingForceUse) {
        dodataPending(pBuffer, bytes, &pBufferAfterDataPending, &bytesAfterDataPending);

        if (bytesAfterDataPending < mDataAlignedSize) {
            ALOGV("%s(), bytesAfterDataPending %u, return", __FUNCTION__, bytesAfterDataPending);
            return bytes;
        }
    } else {
        pBufferAfterDataPending = pBuffer;
        bytesAfterDataPending = bytes;
    }

    void *pBufferAfterDcRemoval = NULL;
    uint32_t bytesAfterDcRemoval = 0;
    // DC removal before DRC
    doDcRemoval(pBufferAfterDataPending, bytesAfterDataPending, &pBufferAfterDcRemoval, &bytesAfterDcRemoval);

    // bit conversion
    void *pBufferAfterBitConvertion = NULL;
    uint32_t bytesAfterBitConvertion = 0;

    uint32_t buf_sample_rate = 0;
    uint32_t buf_num_channels = 0;

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (get_aurisys_on() && !mStreamAttributeSource->isMixerOut) {
        buf_sample_rate = mStreamAttributeTarget.sample_rate;
        buf_num_channels = mStreamAttributeTarget.num_channels;

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

        uint32_t data_size = audio_ringbuf_count(&mAudioPoolBufDlOut->ringbuf);
        if (data_size > mTransferredBufferSize) {
            data_size = mTransferredBufferSize;
        }
        audio_pool_buf_copy_to_linear(
            &mLinearOut->p_buffer,
            &mLinearOut->memory_size,
            mAudioPoolBufDlOut,
            data_size);
        //ALOGD("aurisys process data_size: %u", data_size);

        // wrap to original playback handler
        pBufferAfterBitConvertion = (void *)mLinearOut->p_buffer;
        bytesAfterBitConvertion = data_size;
    } else
#endif
    {
        // SRC
        void *pBufferAfterBliSrc = NULL;
        uint32_t bytesAfterBliSrc = 0;

        buf_sample_rate = mStreamAttributeTarget.sample_rate;
        buf_num_channels = mStreamAttributeTarget.num_channels;

        doBliSrc(pBufferAfterDcRemoval, bytesAfterDcRemoval, &pBufferAfterBliSrc, &bytesAfterBliSrc);

        doBitConversion(pBufferAfterBliSrc, bytesAfterBliSrc, &pBufferAfterBitConvertion, &bytesAfterBitConvertion);
    }

    // EchoRef
    uint32_t echoRefDataSize = bytesAfterBitConvertion;
    const char *pEchoRefBuffer = (const char *)pBufferAfterBitConvertion;

    // write data to bt cvsd driver
    uint8_t *outbuffer, *inbuf, *workbuf;
    uint32_t insize, outsize, workbufsize, total_outsize, src_fs_s;

    inbuf = (uint8_t *)pBufferAfterBitConvertion;

    if (mPCMDumpFile) {
        clock_gettime(CLOCK_REALTIME, &mNewtime);
        latencyTime[1] = calc_time_diff(mNewtime, mOldtime);
        mOldtime = mNewtime;
    }

    do {
        outbuffer = mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf();
        outsize = SCO_TX_ENCODE_SIZE;
        insize = bytesAfterBitConvertion;
        workbuf = mAudioBTCVSDControl->BT_SCO_TX_GetCVSDWorkBuf();
        workbufsize = SCO_TX_PCM64K_BUF_SIZE;
        total_outsize = 0;
        do {
            if (mBTMode_Open != mAudioBTCVSDControl->BT_SCO_isWideBand()) {
                ALOGD("BTSCO change mode after TX_Begin!!!");
                mAudioBTCVSDControl->BT_SCO_TX_End(mFd2);
                mAudioBTCVSDControl->BT_SCO_TX_Begin(mFd2, buf_sample_rate, buf_num_channels);
                mBTMode_Open = mAudioBTCVSDControl->BT_SCO_isWideBand();
                if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "BTCVSD Band"), mBTMode_Open ? "WB" : "NB")) {
                    ALOGE("Error: BTCVSD Band invalid value");
                }
                return bytes;
            }

            if (mAudioBTCVSDControl->BT_SCO_isWideBand()) {
                mAudioBTCVSDControl->btsco_process_TX_MSBC(inbuf, &insize, outbuffer, &outsize, workbuf); // return insize is consumed size
                ALOGV("WriteDataToBTSCOHW, do mSBC encode outsize=%d, consumed size=%d, bytesAfterBitConvertion=%d", outsize, insize, bytesAfterBitConvertion);
            } else {
                mAudioBTCVSDControl->btsco_process_TX_CVSD(inbuf, &insize, outbuffer, &outsize, workbuf, workbufsize); // return insize is consumed size
                ALOGV("WriteDataToBTSCOHW, do CVSD encode outsize=%d, consumed size=%d, bytesAfterBitConvertion=%d", outsize, insize, bytesAfterBitConvertion);
            }
            outbuffer += outsize;
            inbuf += insize;

            ASSERT(bytesAfterBitConvertion >= insize);  // bytesAfterBitConvertion - insize >= 0
            bytesAfterBitConvertion -= insize;

            insize = bytesAfterBitConvertion;
            total_outsize += outsize;
        } while (total_outsize < BTSCO_CVSD_TX_OUTBUF_SIZE && outsize != 0);

        ALOGV("WriteDataToBTSCOHW write to kernel(+) total_outsize = %d", total_outsize);
        if (total_outsize > 0) {
            WritePcmDumpData((void *)(mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf()), total_outsize);
#ifdef MTK_SUPPORT_BTCVSD_ALSA
            // prevent stuck in alsa due to bt stop before audio start
            bool skipWrite = false;
            int maxTimeoutTrial = 2;
            unsigned int avail = 0;
            unsigned int totalOutFrame = (total_outsize / (mConfig.channels * pcm_format_to_bits(mConfig.format) / 8));

            while (maxTimeoutTrial >= 0) {
                bool needSkipWrite = false;
                struct timespec timeStamp;

                if (pcm_get_htimestamp(mPcm, &avail, &timeStamp) != 0) {
                    ALOGW("%s(), pcm_get_htimestamp fail %s, mWrittenFrame %u, totalOutFrame %u\n", __FUNCTION__, pcm_get_error(mPcm), mWrittenFrame, totalOutFrame);
                    if (mInitWrite && mWrittenFrame + totalOutFrame > mConfig.period_size * mConfig.period_count) {
                        needSkipWrite = true;
                    }
                } else {
                    mInitWrite = false;
                    if (totalOutFrame > avail) {
                        needSkipWrite = true;
                    }
                }

                if (needSkipWrite) {
                    skipWrite = true;
                    usleep(BT_IRQ_PERIOD_US);
                    maxTimeoutTrial--;
                } else {
                    skipWrite = false;
                    break;
                }
            }

            if (!skipWrite) {
                int retval = pcm_write(mPcm, mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf(), total_outsize);
                if (retval != 0) {
                    ALOGE("%s(), pcm_write() error, retval = %d", __FUNCTION__, retval);
                } else {
                    mWrittenFrame += totalOutFrame;
                }
            } else {
                ALOGW("%s(), skip write, total_out_size %u, avail %u, mWrittenFrame %u", __FUNCTION__, total_outsize, avail, mWrittenFrame);
            }
#else
            ssize_t WrittenBytes = ::write(mFd2, mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf(), total_outsize);
#endif
            updateStartTimeStamp();

        }

        ALOGV("WriteDataToBTSCOHW write to kernel(-) remaining bytes = %d", bytesAfterBitConvertion);
    } while (bytesAfterBitConvertion > 0);

    if (mPCMDumpFile) {
        clock_gettime(CLOCK_REALTIME, &mNewtime);
        latencyTime[2] = calc_time_diff(mNewtime, mOldtime);
        mOldtime = mNewtime;
    }

    /* Write echo ref data to data provider if needed */
    writeEchoRefDataToDataProvider(mDataProviderEchoRefBTCVSD, pEchoRefBuffer, echoRefDataSize);

    if (mPCMDumpFile) {
        clock_gettime(CLOCK_REALTIME, &mNewtime);
        latencyTime[3] = calc_time_diff(mNewtime, mOldtime);
        mOldtime = mNewtime;

        if (latencyTime[3] > 0.022) {
            ALOGD("latency_in_s,%1.3lf,%1.3lf,%1.3lf,%1.3lf, interrupt,%1.3lf", latencyTime[0], latencyTime[1], latencyTime[2], latencyTime[3], mStreamAttributeTarget.mInterrupt);
        }
    }

    return bytes;
}


status_t AudioALSAPlaybackHandlerBTCVSD::updateStartTimeStamp() {
    if (mEchoRefStartTime.tv_sec == 0 && mEchoRefStartTime.tv_nsec == 0) {
        TimeBufferInfo *pTimeInfoBuffer = NULL;

        /* Get tx timestamp */
#ifdef MTK_SUPPORT_BTCVSD_ALSA
        struct mixer_ctl *ctl;
        unsigned int num_values, i;
        int index = 0;
        TimeBufferInfo timeBufferInfo;

        /* get timestamp from driver*/
        ctl = mixer_get_ctl_by_name(mMixer, "BTCVSD Tx Timestamp");
        int ret_val = mixer_ctl_get_array(ctl, &timeBufferInfo, sizeof(timeBufferInfo));
        if (ret_val < 0) {
            ALOGE("%s() mixer_ctl_get_array() failed (error %d)", __FUNCTION__, ret_val);
            pTimeInfoBuffer = NULL;
            return INVALID_OPERATION;
        } else {
            pTimeInfoBuffer = &timeBufferInfo;
        }
#else
        pTimeInfoBuffer = (TimeBufferInfo *)(mAudioBTCVSDControl->BT_SCO_TX_GetTimeBufferInfo());
#endif
        ASSERT(pTimeInfoBuffer != NULL);

        /* Convert TimeBufferInfo to timespec */
        unsigned long long timeStamp = pTimeInfoBuffer->timestampUS + pTimeInfoBuffer->dataCountEquiTime;
        mEchoRefStartTime.tv_sec = timeStamp / 1000000000;
        mEchoRefStartTime.tv_nsec = timeStamp % 1000000000;

        int delayMs = 0;
        const char *btDeviceName = AudioALSAStreamManager::getInstance()->GetBtHeadsetName();

#if (defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT) && (MTK_AUDIO_TUNING_TOOL_V2_PHASE >= 2))
        /* Get the BT device delay parameter */
        AppOps *appOps = appOpsGetInstance();
        if (appOps == NULL) {
            ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
            ASSERT(0);
            return INVALID_OPERATION;
        }

        AppHandle *pAppHandle = appOps->appHandleGetInstance();
        AudioType *audioType = appOps->appHandleGetAudioTypeByName(pAppHandle, "BtInfo");
        if (audioType) {
            String8 categoryPath("BT headset,");
            categoryPath += (btDeviceName ? btDeviceName : "");

            ParamUnit *paramUnit = appOps->audioTypeGetParamUnit(audioType, categoryPath.string());
            ASSERT(paramUnit);

            // isVoIP : true(VoIP) or false(3GVT)
            Param *param = appOps->paramUnitGetParamByName(paramUnit, "voip_ap_delay_ms");
            ASSERT(param);

            delayMs = *(int *)param->data;
        } else {
            ALOGW("%s(), No BtInfo audio type found!", __FUNCTION__);
        }
#endif

        struct timespec origStartTime = mEchoRefStartTime;
        adjustTimeStamp(&mEchoRefStartTime, delayMs);

        ALOGD("%s(), Set start timestamp (%ld.%09ld->%ld.%09ld), mTotalEchoRefBufSize = %d, BT headset = %s, delayMs = %d (audio_mode = %d), dataCountEquiTime=%" PRIu64 ", timestampUS=%" PRIu64 "",
              __FUNCTION__,
              origStartTime.tv_sec,
              origStartTime.tv_nsec,
              mEchoRefStartTime.tv_sec,
              mEchoRefStartTime.tv_nsec,
              mTotalEchoRefBufSize,
              btDeviceName,
              delayMs,
              mStreamAttributeSource->audio_mode,
              pTimeInfoBuffer->dataCountEquiTime,
              pTimeInfoBuffer->timestampUS);
    } else {
        ALOGV("%s(), start timestamp (%ld.%09ld), mTotalEchoRefBufSize = %d", __FUNCTION__, mEchoRefStartTime.tv_sec, mEchoRefStartTime.tv_nsec, mTotalEchoRefBufSize);
    }

    return NO_ERROR;
}

bool AudioALSAPlaybackHandlerBTCVSD::writeEchoRefDataToDataProvider(AudioALSACaptureDataProviderEchoRefBTCVSD *dataProvider, const char *echoRefData, uint32_t dataSize) {
    if (dataProvider->isEnable()) {
        /* Calculate buffer's time stamp */
        struct timespec newTimeStamp;
        calculateTimeStampByBytes(mEchoRefStartTime, mTotalEchoRefBufSize, mStreamAttributeTargetEchoRef, &newTimeStamp);

        SHOW_TIMESTAMP("%s(), mTotalEchoRefBufSize = %d, write size = %d, newTimeStamp = %ld.%09ld -> %ld.%09ld",
                       __FUNCTION__, mTotalEchoRefBufSize, dataSize, mEchoRefStartTime.tv_sec, mEchoRefStartTime.tv_nsec,
                       newTimeStamp.tv_sec, newTimeStamp.tv_nsec);

        // TODO(JH): Consider the close case, need to free EchoRef data from provider
        dataProvider->writeData(echoRefData, dataSize, &newTimeStamp);

        //WritePcmDumpData(echoRefData, dataSize);
    } else {
        SHOW_TIMESTAMP("%s(), data provider is not enabled, Do not write echo ref data to provider", __FUNCTION__);
    }
    mTotalEchoRefBufSize += dataSize;

    return true;
}

} // end of namespace android

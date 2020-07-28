#include "AudioALSACaptureDataProviderBTCVSD.h"

#include <math.h>
#include <pthread.h>

#include <sys/prctl.h>

#include "AudioALSAHardware.h"

#include "AudioType.h"

#include "WCNChipController.h"

#include "AudioBTCVSDDef.h"
#include "AudioBTCVSDControl.h"
#include "AudioALSADriverUtility.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureDataProviderBTCVSD"

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)
#define PCM_READ_MS 60
#define BT_IRQ_PERIOD_US 22500
#define BT_IRQ_PERIOD_MS (BT_IRQ_PERIOD_US / 1000.f)
#define DEBUG_TIMESTAMP

#ifdef DEBUG_TIMESTAMP
#define SHOW_TIMESTAMP(format, args...) ALOGD(format, ##args)
#else
#define SHOW_TIMESTAMP(format, args...)
#endif

namespace android {

static bool mBTMode_Open;
/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderBTCVSD *AudioALSACaptureDataProviderBTCVSD::mAudioALSACaptureDataProviderBTCVSD = NULL;
AudioALSACaptureDataProviderBTCVSD *AudioALSACaptureDataProviderBTCVSD::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderBTCVSD == NULL) {
        mAudioALSACaptureDataProviderBTCVSD = new AudioALSACaptureDataProviderBTCVSD();
    }
    ASSERT(mAudioALSACaptureDataProviderBTCVSD != NULL);
    return mAudioALSACaptureDataProviderBTCVSD;
}

AudioALSACaptureDataProviderBTCVSD::AudioALSACaptureDataProviderBTCVSD() :
    mWCNChipController(WCNChipController::GetInstance()),
    mAudioBTCVSDControl(AudioBTCVSDControl::getInstance()),
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    mBTIrqReceived(false),
    hReadThread(0),
    mReadBufferSize(0),
    mBliSrc(NULL),
    mBliSrcOutputBuffer(NULL),
#ifndef MTK_SUPPORT_BTCVSD_ALSA
    mFd2(mAudioBTCVSDControl->getFd())
#else
    mFd2(0)
#endif
{
    ALOGD("%s()", __FUNCTION__);

    mCaptureDataProviderType = CAPTURE_PROVIDER_BT_CVSD;

    /* Init EchoRef Resource */
    memset(&mCaptureStartTime, 0, sizeof(mCaptureStartTime));

    /* Init EchoRef Resource */
    memset(&mEstimatedBufferTimeStamp, 0, sizeof(mEstimatedBufferTimeStamp));

    /* Init timestamp*/
    memset(&mNewtime, 0, sizeof(mNewtime));
    memset(&mOldtime, 0, sizeof(mOldtime));
    memset(&mOldCVSDReadTime, 0, sizeof(mOldCVSDReadTime));
    memset(mTimerec, 0, sizeof(mTimerec));
}

AudioALSACaptureDataProviderBTCVSD::~AudioALSACaptureDataProviderBTCVSD() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderBTCVSD::open() {
    ASSERT(mEnable == false);

    // config attribute (will used in client SRC/Enh/... later)
    mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_MONO;
    mStreamAttributeSource.num_channels = popcount(mStreamAttributeSource.audio_channel_mask);
    mStreamAttributeSource.sample_rate = mWCNChipController->GetBTCurrentSamplingRateNumber();

    /* Reset frames readed counter & mCaptureStartTime */
    mStreamAttributeSource.Time_Info.total_frames_readed = 0;
    memset((void *)&mCaptureStartTime, 0, sizeof(mCaptureStartTime));
    memset((void *)&mEstimatedBufferTimeStamp, 0, sizeof(mEstimatedBufferTimeStamp));

    if (mAudioBTCVSDControl->BT_SCO_isWideBand() == true) {
        mReadBufferSize = MSBC_PCM_FRAME_BYTE * 6 * 2; // 16k mono->48k stereo
    } else {
        mReadBufferSize = SCO_RX_PCM8K_BUF_SIZE * 12 * 2; // 8k mono->48k stereo
    }

    mBTMode_Open = mAudioBTCVSDControl->BT_SCO_isWideBand();
    initBliSrc();
    ALOGD("%s(), audio_format = %d, audio_channel_mask=%x, num_channels=%d, sample_rate=%d", __FUNCTION__,
          mStreamAttributeSource.audio_format, mStreamAttributeSource.audio_channel_mask, mStreamAttributeSource.num_channels, mStreamAttributeSource.sample_rate);

    OpenPCMDump(LOG_TAG);

    // enable bt cvsd driver
#ifdef MTK_SUPPORT_BTCVSD_ALSA
#if 0
    int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmBTCVSDCapture);
    int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmBTCVSDCapture);

    struct pcm_params *params;
    params = pcm_params_get(cardindex, pcmindex,  PCM_OUT);
    if (params == NULL) {
        ALOGD("Device does not exist.\n");
    }

    // HW pcm config
    mConfig.channels = mStreamAttributeSource.num_channels;
    mConfig.rate = mStreamAttributeSource.sample_rate;
    mConfig.period_count = 2;
    mConfig.period_size = 1024;//(mStreamAttributeTarget.buffer_size / (mConfig.channels * mConfig.period_count)) / ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);
    mConfig.format = PCM_FORMAT_S16_LE;//transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format);
    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;
    ALOGD("%s(), mConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d, pcmindex=%d",
          __FUNCTION__, mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mConfig.format, pcmindex);

    // open pcm driver
    openPcmDriver(pcmindex);
    //openPcmDriver(25);
#else
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = mStreamAttributeSource.num_channels;
    mConfig.rate = mStreamAttributeSource.sample_rate;
    mConfig.period_size = 1024;
    mConfig.period_count = 2;
    mConfig.format = PCM_FORMAT_S16_LE;
    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;

    ASSERT(mPcm == NULL);
    int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmBTCVSDCapture);
    int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmBTCVSDCapture);
    mPcm = pcm_open(cardIdx, pcmIdx, PCM_IN, &mConfig);
    ASSERT(mPcm != NULL && pcm_is_ready(mPcm) == true);
    mAudioBTCVSDControl->BT_SCO_RX_Begin(mFd2);
    if (isNeedSyncPcmStart() == false) {
        pcm_start(mPcm);
    } else {
        mStart = false;
        mReadThreadReady = false;
    }
#endif
#else
    mAudioBTCVSDControl->BT_SCO_RX_Begin(mFd2);
#endif

    // create reading thread
    mEnable = true;
    int ret = pthread_create(&hReadThread, NULL, AudioALSACaptureDataProviderBTCVSD::readThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}

status_t AudioALSACaptureDataProviderBTCVSD::close() {
    ALOGD("%s()", __FUNCTION__);

    mEnable = false;
    pthread_join(hReadThread, NULL);
    ALOGD("pthread_join hReadThread done");


    ClosePCMDump();

    mAudioBTCVSDControl->BT_SCO_RX_End(mFd2);

#ifdef MTK_SUPPORT_BTCVSD_ALSA
    pcm_stop(mPcm);
    pcm_close(mPcm);
    mPcm = NULL;
#endif
    return NO_ERROR;
}


void *AudioALSACaptureDataProviderBTCVSD::readThread(void *arg) {
    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);

    ALOGD("+%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    status_t retval = NO_ERROR;
    AudioALSACaptureDataProviderBTCVSD *pDataProvider = static_cast<AudioALSACaptureDataProviderBTCVSD *>(arg);

    pDataProvider->setThreadPriority();

    uint32_t open_index = pDataProvider->mOpenIndex;

    pDataProvider->waitPcmStart();

    // read raw data from alsa driver
    uint32_t read_size = 0;
    char linear_buffer[MSBC_PCM_FRAME_BYTE * 6 * 2]; // fixed at size for WB
    while (pDataProvider->mEnable == true) {
        if (open_index != pDataProvider->mOpenIndex) {
            ALOGD("%s(), open_index(%d) != mOpenIndex(%d), return", __FUNCTION__, open_index, pDataProvider->mOpenIndex);
            break;
        }


        if (pDataProvider->mPCMDumpFile) {
            clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
            pDataProvider->mTimerec[0] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
            pDataProvider->mOldtime = pDataProvider->mNewtime;
        }

        bool isMuteData = false;
        struct timespec bufferTimeStamp;
        read_size = pDataProvider->readDataFromBTCVSD(linear_buffer, &isMuteData, &bufferTimeStamp);
        if (read_size == 0) {
            ALOGV("%s(), read_size == 0", __FUNCTION__);
            continue;
        }

        if (pDataProvider->mPCMDumpFile) {
            clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
            pDataProvider->mTimerec[2] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
            pDataProvider->mOldtime = pDataProvider->mNewtime;
        }

        // SRC
        void *pBufferAfterBliSrc = NULL;
        uint32_t bytesAfterBliSrc = 0;
        pDataProvider->doBliSrc(linear_buffer, read_size, &pBufferAfterBliSrc, &bytesAfterBliSrc);

        /* Update capture start time if needed */
        pDataProvider->updateStartTimeStamp(bufferTimeStamp);

        /* check the time stamp is match the estimatedTimeStamp, if not, compensate it! */
        if (!isMuteData && !pDataProvider->isBufferTimeStampMatchEstimated(bufferTimeStamp)) {
            double timeDiff = calc_time_diff(bufferTimeStamp, pDataProvider->mEstimatedBufferTimeStamp);
            if (timeDiff > 0) {
                /* bufferTimeStamp > mEstimatedBufferTimeStamp ==> provide extra timeDiff mute data to provider */
                uint32_t compensateBytes = convertMsToBytes((int)(timeDiff * 1000), &pDataProvider->mStreamAttributeSource);
                compensateBytes = compensateBytes - compensateBytes % (pDataProvider->mStreamAttributeSource.num_channels * audio_bytes_per_sample(pDataProvider->mStreamAttributeSource.audio_format));

                char *compensateBuffer = new char[compensateBytes];
                memset(compensateBuffer, 0, compensateBytes);

                pDataProvider->mPcmReadBuf.pBufBase = (char *)compensateBuffer;
                pDataProvider->mPcmReadBuf.bufLen   = compensateBytes + 1; // +1: avoid pRead == pWrite
                pDataProvider->mPcmReadBuf.pRead    = (char *)pDataProvider->mPcmReadBuf.pBufBase;
                pDataProvider->mPcmReadBuf.pWrite   = (char *)pDataProvider->mPcmReadBuf.pBufBase + compensateBytes;

                /* Update capture timestamp by start time */
                pDataProvider->updateCaptureTimeStampByStartTime(compensateBytes);

                SHOW_TIMESTAMP("%s(), compensateBytes = %d, newTimeStamp = %ld.%09ld",
                               __FUNCTION__, compensateBytes, pDataProvider->mStreamAttributeSource.Time_Info.timestamp_get.tv_sec, pDataProvider->mStreamAttributeSource.Time_Info.timestamp_get.tv_nsec);

                pDataProvider->provideCaptureDataToAllClients(open_index);

                delete[] compensateBuffer;
            } else {
                /* mEstimatedBufferTimeStamp > bufferTimeStamp ==> drop extra UL data */
                uint32_t dropBytes = convertMsToBytes((int)(fabs(timeDiff) * 1000), &pDataProvider->mStreamAttributeSource);
                dropBytes = dropBytes - dropBytes % (pDataProvider->mStreamAttributeSource.num_channels * audio_bytes_per_sample(pDataProvider->mStreamAttributeSource.audio_format));
                if (dropBytes > bytesAfterBliSrc) {
                    SHOW_TIMESTAMP("%s() all data timeout, drop all data! (dropBytes = %d, available bytes = %d)", __FUNCTION__, dropBytes, bytesAfterBliSrc);
                    continue;
                }

                bytesAfterBliSrc -= dropBytes;
                pBufferAfterBliSrc = ((char *)pBufferAfterBliSrc) + dropBytes;

                SHOW_TIMESTAMP("%s(), dropBytes = %d, timeDiff = %lf, bytesAfterBliSrc = %d", __FUNCTION__, dropBytes, timeDiff, bytesAfterBliSrc);
            }
        }

        /*
         * Change the data provider reported data size from 60ms -> 20ms,
         * Because changing the read buffer size is not easy, so we divide the report data size here
         */
        ssize_t frameBytes = pDataProvider->mStreamAttributeSource.num_channels * audio_bytes_per_sample(pDataProvider->mStreamAttributeSource.audio_format);
        ssize_t totalFrameSize = bytesAfterBliSrc / frameBytes;
        ssize_t maxProvideFrameSize = pDataProvider->mStreamAttributeSource.sample_rate * pDataProvider->getLatencyTime() / 1000;
        ssize_t providedFrameSize = 0;
        ALOGV("%s(), FrameBytes = %zu, bytesAfterBliSrc = %d, totalFrameSize = %zu, maxProvideFrameSize = %zu", __FUNCTION__, frameBytes, bytesAfterBliSrc, totalFrameSize, maxProvideFrameSize);

        while (providedFrameSize < totalFrameSize) {
            ssize_t remainFrameSize = totalFrameSize - providedFrameSize;
            ssize_t frameSize = 0;
            ssize_t bufferSize = 0;
            if (remainFrameSize > maxProvideFrameSize) {
                frameSize = maxProvideFrameSize;
            } else {
                frameSize = remainFrameSize;
            }
            bufferSize = frameSize * frameBytes;
            ALOGV("%s(), frameSize = %zu, bufferSize = %zu, providedFrameSize = %zu/%zu", __FUNCTION__, frameSize, bufferSize, providedFrameSize, totalFrameSize);

            // use ringbuf format to save buffer info
            pDataProvider->mPcmReadBuf.pBufBase = (char *)pBufferAfterBliSrc + providedFrameSize * frameBytes;
            pDataProvider->mPcmReadBuf.bufLen   = bufferSize + 1; // +1: avoid pRead == pWrite
            pDataProvider->mPcmReadBuf.pRead    = (char *)pDataProvider->mPcmReadBuf.pBufBase;
            pDataProvider->mPcmReadBuf.pWrite   = (char *)pDataProvider->mPcmReadBuf.pBufBase + bufferSize;

            /* update capture timestamp by start time */
            pDataProvider->updateCaptureTimeStampByStartTime(bufferSize);

            ALOGV("%s(), read size = %zu, newTimeStamp = %ld.%09ld",
                  __FUNCTION__, bufferSize, pDataProvider->mStreamAttributeSource.Time_Info.timestamp_get.tv_sec, pDataProvider->mStreamAttributeSource.Time_Info.timestamp_get.tv_nsec);

            pDataProvider->provideCaptureDataToAllClients(open_index);
            providedFrameSize += frameSize;
        }

        if (pDataProvider->mPCMDumpFile) {
            clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
            pDataProvider->mTimerec[3] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
            pDataProvider->mOldtime = pDataProvider->mNewtime;

            ALOGD("%s, latency_in_us,%1.6lf,%1.6lf,%1.6lf,%1.6lf,%1.6lf", __FUNCTION__, pDataProvider->mTimerec[0], pDataProvider->mTimerec[1], pDataProvider->mTimerec[2], pDataProvider->mTimerec[3], pDataProvider->mTimerec[4]);
        }
    }

    pDataProvider->deinitBliSrc();

    pDataProvider->mBTIrqReceived = false;

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_exit(NULL);
    return NULL;
}

uint32_t AudioALSACaptureDataProviderBTCVSD::readDataFromBTCVSD(void *linear_buffer, bool *isMuteData, struct timespec *bufferTimeStamp) {
    ALOGV("+%s()", __FUNCTION__);

    *isMuteData = false;

    uint8_t *cvsd_raw_data = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDTempInBuf();

#ifdef MTK_SUPPORT_BTCVSD_ALSA
    ASSERT(mPcm != NULL);

    // make sure receiving bt irq before start reading
    if (!mBTIrqReceived) {
        if (pcm_prepare(mPcm)) {
            ALOGE("%s(), pcm_prepare(%p) == false due to %s", __FUNCTION__, mPcm, pcm_get_error(mPcm));
        }

        struct mixer_ctl *ctl;
        unsigned int num_values, i;
        int index = 0;
        ctl = mixer_get_ctl_by_name(mMixer, "BTCVSD Rx Irq Received Switch");
        num_values = mixer_ctl_get_num_values(ctl);
        for (i = 0; i < num_values; i++) {
            index = mixer_ctl_get_value(ctl, i);
            ALOGV("%s(), BTCVSD Rx Irq Received Switch, i = %d, index = %d", __FUNCTION__, i, index);
        }
        mBTIrqReceived = index != 0;
    }

    float returnMuteDataMs = 0.0f;  /* If we cannot read data from kernel, return mute data first */

    if (mBTIrqReceived) {
        // check if enough data in kernel
        struct timespec timeStamp;
        unsigned int avail;
        if (pcm_get_htimestamp(mPcm, &avail, &timeStamp) != 0) {
            ALOGV("%s(), pcm_get_htimestamp fail %s\n", __FUNCTION__, pcm_get_error(mPcm));
        } else {
            if ((BTSCO_CVSD_RX_TEMPINPUTBUF_SIZE / (mConfig.channels * pcm_format_to_bits(mConfig.format) / 8)) > avail) {
                bool btscoOn = AudioALSAHardware::GetInstance()->getBtscoOn();
                ALOGW("%s(), read size %u, avail %u, skip read, btscoOn %d",
                      __FUNCTION__, BTSCO_CVSD_RX_TEMPINPUTBUF_SIZE, avail, btscoOn);
                if (btscoOn) {
                    usleep(BT_IRQ_PERIOD_US);
                    return 0;
                } else {
                    returnMuteDataMs = BT_IRQ_PERIOD_MS;
                }
            }
        }

        if (returnMuteDataMs == 0) {
            int retval = pcm_read(mPcm, cvsd_raw_data, BTSCO_CVSD_RX_TEMPINPUTBUF_SIZE);
            if (retval != 0) {
                // read fail, return mute data
                ALOGE("%s(), pcm_read() error, retval = %d, fail due to %s", __FUNCTION__, retval, pcm_get_error(mPcm));
                returnMuteDataMs = 60.0f;
            } else {
                // check if timeout during read bt data
                struct mixer_ctl *ctl = mixer_get_ctl_by_name(mMixer, "BTCVSD Rx Timeout Switch");
                int index = mixer_ctl_get_value(ctl, 0);
                bool rx_timeout = index != 0;
                ALOGV("%s(), BTCVSD Rx Timeout Switch, rx_timeout %d, index %d", __FUNCTION__, rx_timeout, index);
                if (rx_timeout) {
                    // read timeout, return mute data
                    ALOGE("%s(), rx_timeout %d, index %d, return mute data", __FUNCTION__, rx_timeout, index);
                    returnMuteDataMs = 60.0f;
                }
            }
        }
    } else {
        // IRQ not received, return mute data
        ALOGW("%s(), mBTIrqReceived = %d", __FUNCTION__, mBTIrqReceived);
        returnMuteDataMs = BT_IRQ_PERIOD_MS;
    }

    if (returnMuteDataMs != 0.0f) {
        *isMuteData = true;
        *bufferTimeStamp = getCurrentTimeStamp();
        uint32_t muteBytes = mAudioBTCVSDControl->BT_SCO_isWideBand() ? returnMuteDataMs * 16 * 2 : returnMuteDataMs * 8 * 2;
        usleep(returnMuteDataMs * 1000);    // bt irq interval is 22.5ms
        memset(linear_buffer, 0, MSBC_PCM_FRAME_BYTE * 6 * 2);

        return muteBytes;
    }

#else
    uint32_t raw_data_size = ::read(mFd2, cvsd_raw_data, BTSCO_CVSD_RX_TEMPINPUTBUF_SIZE);
    ALOGV("%s(), cvsd_raw_data = %p, raw_data_size = %d", __FUNCTION__, cvsd_raw_data, raw_data_size);

    if (raw_data_size == 0) {
        ALOGE("%s(), raw_data_size == 0", __FUNCTION__);
        return 0;
    }
#endif

    if (getKernelTimeStamp(bufferTimeStamp) != NO_ERROR) {
        ALOGE("%s(), Cannot get kernel timestamp correctly!", __FUNCTION__);
    }

    SHOW_TIMESTAMP("%s() isMuteData = %d, bufferTimeStamp = %ld.%09ld", __FUNCTION__, *isMuteData, bufferTimeStamp->tv_sec, bufferTimeStamp->tv_nsec);


    uint8_t *inbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDInBuf();
    uint32_t insize = SCO_RX_PLC_SIZE;

    uint8_t *outbuf = NULL;
    uint32_t outsize = 0;
    if (mAudioBTCVSDControl->BT_SCO_isWideBand() == true) {
        outbuf = mAudioBTCVSDControl->BT_SCO_RX_GetMSBCOutBuf();
        outsize = MSBC_PCM_FRAME_BYTE;
    } else {
        outbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDOutBuf();
        outsize = SCO_RX_PCM8K_BUF_SIZE;
    }

    uint8_t *workbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDWorkBuf();
    uint32_t workbufsize = SCO_RX_PCM64K_BUF_SIZE;


    uint8_t packetvalid = 0;
    uint32_t total_read_size = 0;
    uint32_t bytes = BTSCO_CVSD_RX_INBUF_SIZE;
    do {
        memcpy(inbuf, cvsd_raw_data, SCO_RX_PLC_SIZE);
        cvsd_raw_data += SCO_RX_PLC_SIZE;

        packetvalid = *cvsd_raw_data; // parser packet valid info for each 30-byte packet
        //packetvalid = 1; // force packvalid to 1 for test
        cvsd_raw_data += BTSCO_CVSD_PACKET_VALID_SIZE;

        insize = SCO_RX_PLC_SIZE;

        if (mBTMode_Open != mAudioBTCVSDControl->BT_SCO_isWideBand()) {
            ALOGW("%s(), BTSCO change mode(WB %d) after RX_Begin!!!skip write", __FUNCTION__, mAudioBTCVSDControl->BT_SCO_isWideBand());
            mAudioBTCVSDControl->BT_SCO_RX_End(mFd2);
            mAudioBTCVSDControl->BT_SCO_RX_Begin(mFd2);
            mBTMode_Open = mAudioBTCVSDControl->BT_SCO_isWideBand();

            if (mAudioBTCVSDControl->BT_SCO_isWideBand() == true) {
                mReadBufferSize = MSBC_PCM_FRAME_BYTE * 6 * 2; // 16k mono->48k stereo
            } else {
                mReadBufferSize = SCO_RX_PCM8K_BUF_SIZE * 12 * 2; // 8k mono->48k stereo
            }

            // enable bli src when needed
            initBliSrc();
            return 0;
        }

        outsize = (mAudioBTCVSDControl->BT_SCO_isWideBand() == true) ? MSBC_PCM_FRAME_BYTE : SCO_RX_PCM8K_BUF_SIZE;
        ALOGV("btsco_process_RX_CVSD/MSBC(+), insize = %d, outsize = %d, packetvalid = %u", insize, outsize, packetvalid);

        if (mAudioBTCVSDControl->BT_SCO_isWideBand() == true) {
            mAudioBTCVSDControl->btsco_process_RX_MSBC(inbuf, &insize, outbuf, &outsize, workbuf, packetvalid);
        } else {
            mAudioBTCVSDControl->btsco_process_RX_CVSD(inbuf, &insize, outbuf, &outsize, workbuf, workbufsize, packetvalid);
        }
        inbuf += SCO_RX_PLC_SIZE;
        bytes -= insize;
        ALOGV("btsco_process_RX_CVSD/MSBC(-), insize = %d, outsize = %d, bytes = %d", insize, outsize, bytes);


        if (outsize > 0) {
            if (total_read_size + outsize <= mReadBufferSize) {
                memcpy(linear_buffer, outbuf, outsize);
            } else {
                ALOGE("%s(), total_read_size %u + outsize %u > mReadBufferSize %u, bytes %u", __FUNCTION__, total_read_size, outsize, mReadBufferSize, bytes);
                ASSERT(total_read_size + outsize <= mReadBufferSize);
                if (total_read_size < mReadBufferSize) {
                    outsize = mReadBufferSize - total_read_size;
                    memcpy(linear_buffer, outbuf, outsize);
                } else {
                    break;
                }
            }
            linear_buffer = ((char *)linear_buffer) + outsize;
            total_read_size += outsize;
        }
    } while (bytes > 0 && total_read_size < mReadBufferSize);


    ALOGV("+%s(), total_read_size = %u", __FUNCTION__, total_read_size);
    return total_read_size;
}

static const uint32_t kBliSrcOutputBufferSize = 0x10000;  // 64k
status_t AudioALSACaptureDataProviderBTCVSD::initBliSrc() {
    ALOGD("%s(), bt band = %d, mStreamAttributeSource.sample_rate = %u mBliSrc = %p", __FUNCTION__,
          mAudioBTCVSDControl->BT_SCO_isWideBand(), mStreamAttributeSource.sample_rate, mBliSrc);

    bool needSrc = false;
    // bt band and rate is not match, need src
    if ((mAudioBTCVSDControl->BT_SCO_isWideBand() && mStreamAttributeSource.sample_rate != 16000) ||
        (!mAudioBTCVSDControl->BT_SCO_isWideBand() && mStreamAttributeSource.sample_rate != 8000)) {
        needSrc = true;

    }

    // always recreate blisrc
    if (mBliSrc) {
        deinitBliSrc();
    }

    // init BLI SRC if need
    if (!mBliSrc && needSrc) {
        SRC_PCM_FORMAT src_pcm_format = SRC_IN_Q1P15_OUT_Q1P15;

        mBliSrc = newMtkAudioSrc(
                      mAudioBTCVSDControl->BT_SCO_isWideBand() ? 16000 : 8000, 1,
                      mStreamAttributeSource.sample_rate,  mStreamAttributeSource.num_channels,
                      src_pcm_format);

        ASSERT(mBliSrc != NULL);
        mBliSrc->open();

        mBliSrcOutputBuffer = new char[kBliSrcOutputBufferSize];
        ASSERT(mBliSrcOutputBuffer != NULL);
    }

    return NO_ERROR;
}


status_t AudioALSACaptureDataProviderBTCVSD::deinitBliSrc() {
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


status_t AudioALSACaptureDataProviderBTCVSD::doBliSrc(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes) {
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

uint32_t AudioALSACaptureDataProviderBTCVSD::getLatencyTime() {
    mlatency = UPLINK_NORMAL_LATENCY_MS ; //20ms
    return mlatency;
}

bool AudioALSACaptureDataProviderBTCVSD::isBufferTimeStampMatchEstimated(struct timespec timeStamp) {
    if (mEstimatedBufferTimeStamp.tv_sec == 0 && mEstimatedBufferTimeStamp.tv_nsec == 0) {
        return true;
    }

    double timeDiff = calc_time_diff(timeStamp, mEstimatedBufferTimeStamp);
    bool isMatch = false;

    if (fabs(timeDiff) <= (PCM_READ_MS / 1000.0f)) {
        isMatch = true;

        SHOW_TIMESTAMP("%s(), %s, timeDiff = %1.6lf (Estimated = %ld.%09ld, buffer = %ld.%09ld)",
                       __FUNCTION__, isMatch ? "Match" : "NOT match", timeDiff,
                       mEstimatedBufferTimeStamp.tv_sec, mEstimatedBufferTimeStamp.tv_nsec,
                       timeStamp.tv_sec, timeStamp.tv_nsec);
    } else {
        ALOGW("%s(), %s, timeDiff = %1.6lf (Estimated = %ld.%09ld, buffer = %ld.%09ld)",
              __FUNCTION__, isMatch ? "Match" : "NOT match", timeDiff,
              mEstimatedBufferTimeStamp.tv_sec, mEstimatedBufferTimeStamp.tv_nsec,
              timeStamp.tv_sec, timeStamp.tv_nsec);
    }

    return isMatch;
}

status_t AudioALSACaptureDataProviderBTCVSD::getKernelTimeStamp(struct timespec *captureStartTime) {
    /* get timestamp from driver*/
    TimeBufferInfo *pTimeBufferInfo = NULL;

#ifdef MTK_SUPPORT_BTCVSD_ALSA
    struct mixer_ctl *ctl;
    unsigned int num_values, i;
    int index = 0;
    TimeBufferInfo timeBufferInfo;

    /* get timestamp from driver*/
    ctl = mixer_get_ctl_by_name(mMixer, "BTCVSD Rx Timestamp");
    int ret_val = mixer_ctl_get_array(ctl, &timeBufferInfo, sizeof(timeBufferInfo));
    if (ret_val < 0) {
        ALOGE("%s() mixer_ctl_get_array() failed (error %d)", __FUNCTION__, ret_val);
        return BAD_VALUE;
    }

    pTimeBufferInfo = &timeBufferInfo;
#else
    pTimeBufferInfo = (TimeBufferInfo *)(mAudioBTCVSDControl->BT_SCO_RX_GetTimeBufferInfo());
#endif
    uint64_t timeStamp = pTimeBufferInfo->timestampUS - pTimeBufferInfo->dataCountEquiTime;
    captureStartTime->tv_sec = timeStamp / 1000000000;
    captureStartTime->tv_nsec = timeStamp % 1000000000;

    ALOGV("%s(), kernel captureStartTime = %ld.%09ld",
          __FUNCTION__, captureStartTime->tv_sec, captureStartTime->tv_nsec);

    return NO_ERROR;
}

struct timespec AudioALSACaptureDataProviderBTCVSD::getCurrentTimeStamp() {
    struct timespec currentTime;

    clock_gettime(CLOCK_MONOTONIC, &currentTime);

    ALOGV("%s(), timestamp = %ld.%09ld", __FUNCTION__, currentTime.tv_sec, currentTime.tv_nsec);

    return currentTime;
}

status_t AudioALSACaptureDataProviderBTCVSD::updateStartTimeStamp(struct timespec timeStamp) {
    if (mCaptureStartTime.tv_sec == 0 && mCaptureStartTime.tv_nsec == 0) {
        mCaptureStartTime = timeStamp;

        SHOW_TIMESTAMP("%s(), set start timestamp = %ld.%09ld",
                       __FUNCTION__, mCaptureStartTime.tv_sec, mCaptureStartTime.tv_nsec);

        return NO_ERROR;
    }

    return INVALID_OPERATION;
}

status_t AudioALSACaptureDataProviderBTCVSD::updateCaptureTimeStampByStartTime(uint32_t readBytes) {
    ALOGV("%s()", __FUNCTION__);

    if (mCaptureStartTime.tv_sec == 0 && mCaptureStartTime.tv_nsec == 0) {
        ALOGW("No valid mCaptureStartTime! Don't update timestamp info.");
        return BAD_VALUE;
    }

    /* Update timeInfo structure */
    uint32_t bytesPerSample = audio_bytes_per_sample(mStreamAttributeSource.audio_format);
    if (bytesPerSample == 0) {
        ALOGW("audio_format is invalid! (%d)", mStreamAttributeSource.audio_format);
        return BAD_VALUE;
    }

    uint32_t readFrames = readBytes / bytesPerSample / mStreamAttributeSource.num_channels;
    time_info_struct_t *timeInfo = &mStreamAttributeSource.Time_Info;

    timeInfo->frameInfo_get = 0;    // Already counted in mCaptureStartTime
    timeInfo->buffer_per_time = 0;  // Already counted in mCaptureStartTime
    timeInfo->kernelbuffer_ns = 0;
    calculateTimeStampByFrames(mCaptureStartTime, timeInfo->total_frames_readed, mStreamAttributeSource, &timeInfo->timestamp_get);

    /* Update total_frames_readed after timestamp calculation */
    timeInfo->total_frames_readed += readFrames;

    SHOW_TIMESTAMP("%s(), read size = %d, readFrames = %d (bytesPerSample = %d, ch = %d, new total_frames_readed = %d), timestamp = %ld.%09ld -> %ld.%09ld",
                   __FUNCTION__,
                   readBytes, readFrames, bytesPerSample, mStreamAttributeSource.num_channels, timeInfo->total_frames_readed,
                   mCaptureStartTime.tv_sec, mCaptureStartTime.tv_nsec,
                   timeInfo->timestamp_get.tv_sec, timeInfo->timestamp_get.tv_nsec);

    /* Write time stamp to cache to avoid getCapturePosition performance issue */
    AL_LOCK(mTimeStampLock);
    mCaptureFramesReaded = timeInfo->total_frames_readed;
    mCaptureTimeStamp = timeInfo->timestamp_get;
    AL_UNLOCK(mTimeStampLock);

    /* Update next estimated timestamp */
    calculateTimeStampByFrames(mCaptureStartTime, timeInfo->total_frames_readed, mStreamAttributeSource, &mEstimatedBufferTimeStamp);
    SHOW_TIMESTAMP("%s(), estimated next buffer timestamp = %ld.%09ld",
                   __FUNCTION__, mEstimatedBufferTimeStamp.tv_sec, mEstimatedBufferTimeStamp.tv_nsec);

    return NO_ERROR;
}

} // end of namespace android

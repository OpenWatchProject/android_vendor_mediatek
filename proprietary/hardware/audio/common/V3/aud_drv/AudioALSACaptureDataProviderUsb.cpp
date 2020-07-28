#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureDataProviderUsb"

#include "AudioALSACaptureDataProviderUsb.h"
//#include "AudioType.h"
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <audio_fmt_conv.h>
#endif
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <sys/prctl.h>

#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <log/log.h>
#include <cutils/str_parms.h>
#include <cutils/properties.h>
#include "AudioALSADriverUtility.h"

#include <hardware/audio.h>
//#include <hardware/audio_alsaops.h>
#include <hardware/hardware.h>

#include <system/audio.h>

#include "IAudioALSACaptureDataClient.h"
#include "AudioALSAStreamManager.h"

extern "C" {
//#include <tinyalsa/asoundlib.h>
#include "alsa_device_profile.h"
#include "alsa_device_proxy.h"
#include "alsa_logging.h"
#include <audio_utils/channels.h>

/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#if defined(__LP64__)
#define AUDIO_FMT_CONV_LIB_PATH "/vendor/lib64/libaudiofmtconv.so"
#else
#define AUDIO_FMT_CONV_LIB_PATH "/vendor/lib/libaudiofmtconv.so"
#endif

#define LINK_AUD_FMT_CONV_API_NAME "link_aud_fmt_conv_api"


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

typedef int (*link_aud_fmt_conv_api_fp_t)(struct aud_fmt_conv_api_t *api);

/*
 * =============================================================================
 *                     private global members
 * =============================================================================
 */

static void *dlopen_handle; /* for dlopen libaudiofmtconv.so */
static link_aud_fmt_conv_api_fp_t link_aud_fmt_conv_api_fp;
static struct aud_fmt_conv_api_t g_fmt_conv_api;
static audio_format_t gAurisysSupportFormat = AUDIO_FORMAT_PCM_32_BIT;
static audio_format_t gAurisysNotSupportFormat = AUDIO_FORMAT_PCM_24_BIT_PACKED;


void fmt_conv_file_init_ul(void) {
   // char *dlopen_lib_path = NULL;

    /* get dlopen_lib_path */
    if (access(AUDIO_FMT_CONV_LIB_PATH, R_OK) == 0) {
        //dlopen_lib_path = AUDIO_FMT_CONV_LIB_PATH;

    } else {
        AUD_LOG_E("%s(), dlopen_lib_path not found!!", __FUNCTION__);
        //AUD_ASSERT(dlopen_lib_path != NULL);
        return;
    }

    /* dlopen for libaudiocomponentenginec.so */
    dlopen_handle = dlopen(AUDIO_FMT_CONV_LIB_PATH, RTLD_NOW);
    if (dlopen_handle == NULL) {
        AUD_LOG_E("dlopen(%s) fail!!", AUDIO_FMT_CONV_LIB_PATH);
        AUD_ASSERT(dlopen_handle != NULL);
        return;
    }

    link_aud_fmt_conv_api_fp = (link_aud_fmt_conv_api_fp_t)dlsym(dlopen_handle,
                                                                 LINK_AUD_FMT_CONV_API_NAME);
    if (link_aud_fmt_conv_api_fp == NULL) {
        AUD_LOG_E("dlsym(%s) for %s fail!!", AUDIO_FMT_CONV_LIB_PATH, LINK_AUD_FMT_CONV_API_NAME);
        AUD_ASSERT(link_aud_fmt_conv_api_fp != NULL);
        return;
    }

    link_aud_fmt_conv_api_fp(&g_fmt_conv_api);
}


void fmt_conv_file_deinit_ul(void) {
    memset(&g_fmt_conv_api, 0, sizeof(struct aud_fmt_conv_api_t));
    if (dlopen_handle != NULL) {
        dlclose(dlopen_handle);
        dlopen_handle = NULL;
        link_aud_fmt_conv_api_fp = NULL;
    }
}
#endif

}


#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)
#ifdef DEBUG_TIMESTAMP
#define SHOW_TIMESTAMP(format, args...) ALOGD(format, ##args)
#else
#define SHOW_TIMESTAMP(format, args...)
#endif

namespace android {


/*==============================================================================
 *                     Constant
 *============================================================================*/


static uint32_t kReadBufferSize = 0;
static alsa_device_proxy *usbProxy = NULL;
static bool usbVoipMode = false;
//static FILE *pDCCalFile = NULL;
static bool btempDebug = false;
static bool bBcv_in = false;
static bool bBcv_out = false;


/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderUsb *AudioALSACaptureDataProviderUsb::mAudioALSACaptureDataProviderUsb = NULL;

AudioALSACaptureDataProviderUsb *AudioALSACaptureDataProviderUsb::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderUsb == NULL) {
        mAudioALSACaptureDataProviderUsb = new AudioALSACaptureDataProviderUsb();
    }
    ASSERT(mAudioALSACaptureDataProviderUsb != NULL);
    return mAudioALSACaptureDataProviderUsb;
}

AudioALSACaptureDataProviderUsb::AudioALSACaptureDataProviderUsb():
    hReadThread(0) {
    ALOGD("%s()", __FUNCTION__);
    memset(&mNewtime, 0, sizeof(mNewtime));
    memset(&mOldtime, 0, sizeof(mOldtime));
    memset(timerec, 0, sizeof(timerec));
    memset((void *)&mCaptureStartTime, 0, sizeof(mCaptureStartTime));
    memset((void *)&mEstimatedBufferTimeStamp, 0, sizeof(mEstimatedBufferTimeStamp));
    mlatency = 16;
}

AudioALSACaptureDataProviderUsb::~AudioALSACaptureDataProviderUsb() {
    ALOGD("%s()", __FUNCTION__);
}

void AudioALSACaptureDataProviderUsb::initUsbInfo(stream_attribute_t stream_attribute_source_usb, alsa_device_proxy *proxy, size_t buffer_size, bool enable, policy_support_format UL_Format) {
    usbProxy = proxy;
    kReadBufferSize = (uint32_t)buffer_size;
    usbVoipMode = false;
    bBcv_in= false;
    bBcv_out= false;

    mStreamAttributeSource = stream_attribute_source_usb;
    mStreamAttributeSource.BesRecord_Info.besrecord_voip_enable = false;
    mStreamAttributeSource.mVoIPEnable = false;
    mStreamAttributeSource.audio_mode = AUDIO_MODE_NORMAL;
    mPcmStatus = NO_ERROR;
    bool audiomode = AudioALSAStreamManager::getInstance()->isModeInVoipCall();
    ALOGD("%s(), kReadBufferSize = %d, enable = %d, mStreamAttributeSource.input_source = %d ,audiomode = %d, UL_Format.gPolicyFormat = %d", __FUNCTION__, kReadBufferSize, enable, mStreamAttributeSource.input_source, audiomode, UL_Format.gPolicyFormat);

    if ((mStreamAttributeSource.input_source == AUDIO_SOURCE_VOICE_COMMUNICATION) || (audiomode == true)) {
        usbVoipMode = true;
        if (enable == true) {
            mStreamAttributeSource.BesRecord_Info.besrecord_voip_enable = true;
            mStreamAttributeSource.mVoIPEnable = true;
            mStreamAttributeSource.audio_mode = AUDIO_MODE_IN_COMMUNICATION;
        } else {
            // LIB Parser error handling
            mStreamAttributeSource.input_source = AUDIO_SOURCE_MIC;
        }
    }
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    struct aud_fmt_cfg_t source_in;
    struct aud_fmt_cfg_t target_in;
    int ret = 0;

    if (mStreamAttributeSource.audio_format == gAurisysNotSupportFormat || UL_Format.gPolicyFormat == gAurisysNotSupportFormat){
        fmt_conv_file_init_ul();
        source_in.num_channels = (UL_Format.gPolicyChannel != 0) ? UL_Format.gPolicyChannel: mStreamAttributeSource.num_channels;
        source_in.audio_format = mStreamAttributeSource.audio_format;
        source_in.sample_rate = (UL_Format.gPolicyRate!= 0) ? UL_Format.gPolicyRate: mStreamAttributeSource.sample_rate;
        memcpy(&target_in, &source_in, sizeof(struct aud_fmt_cfg_t));
        ALOGD("%s(), mStreamAttributeSource.audio_format = %d, UL_Format.gPolicyFormat = %d,gAurisysNotSupportFormat = %d, source_in.num_channels %d, source_in.sample_rate %d",
            __FUNCTION__, mStreamAttributeSource.audio_format, UL_Format.gPolicyFormat, gAurisysNotSupportFormat, source_in.num_channels, source_in.sample_rate);
    }

    if (UL_Format.gPolicyFormat == gAurisysNotSupportFormat) {//32->24
        source_in.audio_format = gAurisysSupportFormat;
        target_in.audio_format = UL_Format.gPolicyFormat;
        ret = g_fmt_conv_api.create(
            &source_in,
            &target_in,
            &aud_fmt_conv_hdl_out);
        ALOGD("%s(), Set bBcv_out: ret = %d, source_in.audio_format = %d, target_in.audio_format %d",
            __FUNCTION__, ret, source_in.audio_format, target_in.audio_format);
        AUD_ASSERT(ret == 0);
        bBcv_out = true;
    }

    if (mStreamAttributeSource.audio_format == gAurisysNotSupportFormat) {//24->32
        ALOGD("%s(),USB Format:  AUDIO_FORMAT_PCM_24_BIT_PACKED", __FUNCTION__);
        target_in.audio_format = gAurisysSupportFormat;
        mStreamAttributeSource.audio_format = gAurisysSupportFormat;
        ret = g_fmt_conv_api.create(
                &source_in,
                &target_in,
                &aud_fmt_conv_hdl_in);
        ALOGD("%s(), Set bBcv_in: ret = %d, source_in.audio_format = %d, target_in.audio_format %d",
            __FUNCTION__, ret, source_in.audio_format, target_in.audio_format);
        AUD_ASSERT(ret == 0);
        bBcv_in = true;
    }
#endif

    return;
}

ssize_t AudioALSACaptureDataProviderUsb::doBcvProcess(void *buffer, ssize_t bytes) {
    void *buf_out = NULL;
    uint32_t Bcvd_Size = 0;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (bBcv_out == true) {
        g_fmt_conv_api.process(
                buffer, bytes,
                &buf_out, &Bcvd_Size,
                aud_fmt_conv_hdl_out);
        memcpy(buffer,buf_out,Bcvd_Size);
    } else
#endif
    {
        buf_out = buffer;
        Bcvd_Size = bytes;
    }
return Bcvd_Size;
}

bool AudioALSACaptureDataProviderUsb::isNeedEchoRefData() {
    ALOGD("%s(), usbVoipMode = %d, mStreamAttributeSource.input_source = %d", __FUNCTION__, usbVoipMode, mStreamAttributeSource.input_source);
    if (usbVoipMode == true) {
        return true;
    }
    return false;
}

status_t AudioALSACaptureDataProviderUsb::open() {
    ALOGD("%s()", __FUNCTION__);

    ASSERT(mEnable == false);
    mCaptureDataProviderType = CAPTURE_PROVIDER_USB;
    /* Reset frames readed counter & mCaptureStartTime */
    mStreamAttributeSource.Time_Info.total_frames_readed = 0;
    memset((void *)&mCaptureStartTime, 0, sizeof(mCaptureStartTime));
    memset((void *)&mEstimatedBufferTimeStamp, 0, sizeof(mEstimatedBufferTimeStamp));
    mEnable = true;

    int ret_thread = pthread_create(&hReadThread, NULL, AudioALSACaptureDataProviderUsb::readThread, (void *)this);
    if (ret_thread != 0) {
        ALOGD("%s(),pthread_create fail", __FUNCTION__);
        mEnable = false;
        pthread_join(hReadThread, NULL);
        proxy_close(usbProxy);
        //ASSERT((ret == 0)&&(ret_thread == 0));
        mPcmStatus = BAD_VALUE;
        return mPcmStatus;
    } else {
        mPcmStatus = NO_ERROR;
    }
    mPcm = usbProxy->pcm;
    OpenPCMDump(LOG_TAG);
    return NO_ERROR;
}


status_t AudioALSACaptureDataProviderUsb::close() {
    ALOGD("%s(), kReadBufferSize = %d", __FUNCTION__, kReadBufferSize);
    if (mEnable == true) {
        mEnable = false;
        pthread_join(hReadThread, NULL);
        ALOGD("pthread_join hReadThread done");
        ClosePCMDump();
        mPcm = NULL;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
        if (bBcv_in == true) {
            g_fmt_conv_api.destroy(aud_fmt_conv_hdl_in);
            aud_fmt_conv_hdl_in = NULL;
        }
        if (bBcv_out == true) {
            g_fmt_conv_api.destroy(aud_fmt_conv_hdl_out);
            aud_fmt_conv_hdl_out = NULL;
        }
        if (bBcv_in == true || bBcv_out == true) {
            fmt_conv_file_deinit_ul();
            bBcv_in = false;
            bBcv_out = false;
        }
#endif
    }
    return NO_ERROR;
}

void *AudioALSACaptureDataProviderUsb::readThread(void *arg) {
    ALOGD("+%s1(), kReadBufferSize = %d", __FUNCTION__, kReadBufferSize);
    int ret = 0;

    status_t retval = NO_ERROR;
    AudioALSACaptureDataProviderUsb *pDataProvider = static_cast<AudioALSACaptureDataProviderUsb *>(arg);

    uint32_t open_index = pDataProvider->mOpenIndex;

    char nameset[32];
    sprintf(nameset, "%s%d", __FUNCTION__, pDataProvider->mCaptureDataProviderType);
    prctl(PR_SET_NAME, (unsigned long)nameset, 0, 0, 0);
    pDataProvider->setThreadPriority();
    ALOGD("+%s2(), pid: %d, tid: %d, kReadBufferSize = %d, open_index=%d", __FUNCTION__, getpid(), gettid(), kReadBufferSize, open_index);


    // read raw data from alsa driver
    char linear_buffer[kReadBufferSize];
    //char linear_buffer_bcv[kReadBufferSize];
    uint32_t Read_Size = kReadBufferSize;
	uint32_t Bcvd_Size = 0;
    while (pDataProvider->mEnable == true) {
        if (open_index != pDataProvider->mOpenIndex) {
            ALOGD("%s(), open_index(%d) != mOpenIndex(%d), return", __FUNCTION__, open_index, pDataProvider->mOpenIndex);
            break;
        }

        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[0] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;

        int retval = proxy_read(usbProxy, linear_buffer, kReadBufferSize);
        if (retval != 0) {
            ALOGD("%s(), proxy_read fail", __FUNCTION__);
            usleep(15000);
            mPcmStatus = BAD_VALUE;
            continue;
        }
        mPcmStatus = NO_ERROR;
        retval = pDataProvider->GetCaptureTimeStamp(&pDataProvider->mStreamAttributeSource.Time_Info, kReadBufferSize);
        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[1] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;
        /* Update capture start time if needed */
        pDataProvider->updateStartTimeStamp(pDataProvider->mStreamAttributeSource.Time_Info.timestamp_get);
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
        void *buf_out = NULL;
        if (bBcv_in == true) {
            g_fmt_conv_api.process(
                    linear_buffer, kReadBufferSize,
                    &buf_out, &Bcvd_Size,
                    pDataProvider->aud_fmt_conv_hdl_in);
            pDataProvider->mPcmReadBuf.pBufBase = (char*)buf_out;
            pDataProvider->mPcmReadBuf.bufLen	= Bcvd_Size + 1; // +1: avoid pRead == pWrite
            pDataProvider->mPcmReadBuf.pRead	= (char*)buf_out;
            pDataProvider->mPcmReadBuf.pWrite	= (char*)buf_out + Bcvd_Size;
            pDataProvider->updateCaptureTimeStampByStartTime(kReadBufferSize);
        } else
#endif
        {

            //use ringbuf format to save buffer info
            pDataProvider->mPcmReadBuf.pBufBase = linear_buffer;
            pDataProvider->mPcmReadBuf.bufLen   = kReadBufferSize + 1; // +1: avoid pRead == pWrite
            pDataProvider->mPcmReadBuf.pRead    = linear_buffer;
            pDataProvider->mPcmReadBuf.pWrite   = linear_buffer + kReadBufferSize;
            pDataProvider->updateCaptureTimeStampByStartTime(kReadBufferSize);
        }
        /* update capture timestamp by start time */

        pDataProvider->provideCaptureDataToAllClients(open_index);

        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[2] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;
        if (pDataProvider->mPCMDumpFile || btempDebug) {
            ALOGD("%s, latency_in_us,%1.6lf,%1.6lf,%1.6lf", __FUNCTION__, pDataProvider->timerec[0], pDataProvider->timerec[1], pDataProvider->timerec[2]);
        }
    }

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_exit(NULL);
    return NULL;


}

status_t AudioALSACaptureDataProviderUsb::updateStartTimeStamp(struct timespec timeStamp) {
    if (mCaptureStartTime.tv_sec == 0 && mCaptureStartTime.tv_nsec == 0) {
        mCaptureStartTime = timeStamp;

        ALOGD("%s(), set start timestamp = %ld.%09ld",
              __FUNCTION__, mCaptureStartTime.tv_sec, mCaptureStartTime.tv_nsec);

        return NO_ERROR;
    }

    return INVALID_OPERATION;
}

status_t AudioALSACaptureDataProviderUsb::updateCaptureTimeStampByStartTime(uint32_t readBytes) {
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

    ALOGV("%s(), read size = %d, readFrames = %d (bytesPerSample = %d, ch = %d, new total_frames_readed = %d), timestamp = %ld.%09ld -> %ld.%09ld",
          __FUNCTION__,
          readBytes, readFrames, bytesPerSample, mStreamAttributeSource.num_channels, timeInfo->total_frames_readed,
          mCaptureStartTime.tv_sec, mCaptureStartTime.tv_nsec,
          timeInfo->timestamp_get.tv_sec, timeInfo->timestamp_get.tv_nsec);

    /* Write time stamp to cache to avoid getCapturePosition performance issue */
    AL_LOCK(mTimeStampLock);
    mCaptureFramesReaded = timeInfo->total_frames_readed;
    mCaptureTimeStamp = timeInfo->timestamp_get;
    AL_UNLOCK(mTimeStampLock);

    return NO_ERROR;
}

} // end of namespace android

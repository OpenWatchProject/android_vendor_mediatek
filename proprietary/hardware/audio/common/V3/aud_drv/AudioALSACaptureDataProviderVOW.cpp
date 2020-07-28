#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureDataProviderVOW"

#include "AudioALSACaptureDataProviderVOW.h"

#include <pthread.h>

#include <sys/prctl.h>

#include "AudioType.h"

#include <hardware_legacy/power.h>

static const char VOW_DEBUG_WAKELOCK_NAME[] = "VOW_DEBUG_WAKELOCK_NAME";

namespace android {


/*==============================================================================
 *                     Constant
 *============================================================================*/

//extend buffer for seamless record and normal debug dump together case


#ifdef MTK_VOW_DUAL_MIC_SUPPORT
static const uint32_t kReadBufferSize = 0x12E80 * 2; //(0x12480 + 0x0A00) * 2;
#else
static const uint32_t kReadBufferSize = 0x12E80; //0x12480 + 0x0A00;
#endif


/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderVOW *AudioALSACaptureDataProviderVOW::mAudioALSACaptureDataProviderVOW = NULL;
AudioALSACaptureDataProviderVOW *AudioALSACaptureDataProviderVOW::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderVOW == NULL) {
        mAudioALSACaptureDataProviderVOW = new AudioALSACaptureDataProviderVOW();
    }
    ASSERT(mAudioALSACaptureDataProviderVOW != NULL);
    ALOGD("%s(), mAudioALSACaptureDataProviderVOW=%p", __FUNCTION__, mAudioALSACaptureDataProviderVOW);
    return mAudioALSACaptureDataProviderVOW;
}

AudioALSACaptureDataProviderVOW::AudioALSACaptureDataProviderVOW() {
    //use for VOW phase 2 debug tempory. Read from VOW kernel driver, not from PCM
    ALOGD("%s()", __FUNCTION__);

    mCaptureDataProviderType = CAPTURE_PROVIDER_VOW;
    hReadThread = 0;
    memset(&vow_info_buf, 0, sizeof(vow_info_buf));
    mFd = 0;
    mFd = ::open("/dev/vow", O_RDONLY);

    if (mFd == 0) {
        ALOGE("%s(), open mFd fail", __FUNCTION__);
        ASSERT(mFd != 0);
    }
    ALOGD("mFd: %d", mFd);
    ALOGD("%s()-", __FUNCTION__);
}

AudioALSACaptureDataProviderVOW::~AudioALSACaptureDataProviderVOW() {
    ALOGD("%s()", __FUNCTION__);
    if (mFd > 0) {
        ::close(mFd);
        mFd = 0;
    }
    ALOGD("%s()-", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderVOW::open() {
    ALOGD("%s()+", __FUNCTION__);
    ASSERT(mEnable == false);

    // config attribute (will used in client SRC/Enh/... later) // TODO(Harvey): query this
    mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_16_BIT;
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
    mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
#else
    mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_MONO;
#endif
    mStreamAttributeSource.num_channels = popcount(mStreamAttributeSource.audio_channel_mask);
    mStreamAttributeSource.sample_rate = 16000;

    // Reset frames readed counter
    mStreamAttributeSource.Time_Info.total_frames_readed = 0;


    OpenPCMDump(LOG_TAG);

    //open VOW driver
    // enable pcm
    // here open audio hardware for register setting

    if (mFd == 0) {
        ALOGE("%s(), open mFd fail", __FUNCTION__);
        ASSERT(mFd != 0);
    }

    memset(&vow_info_buf, 0, sizeof(vow_info_buf));

    // create reading thread
    //mOpenIndex++;
    mEnable = true;
    int ret = pthread_create(&hReadThread, NULL, AudioALSACaptureDataProviderVOW::readThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    acquire_wake_lock(PARTIAL_WAKE_LOCK, VOW_DEBUG_WAKELOCK_NAME);
    ALOGD("%s()-", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSACaptureDataProviderVOW::close() {
    ALOGD("%s()", __FUNCTION__);
    int ret;

    mEnable = false;
    /* first, let vow kernel stop send data */
    ret = ::ioctl(mFd, VOW_SET_CONTROL, (unsigned int)VOWControlCmd_DisableDebug);
    ALOGD("%s(), VOWControlCmd_DisableDebug set, ret = %d", __FUNCTION__, ret);
    if (ret != 0) {
        ALOGE("%s(), VOWControlCmd_DisableDebug error, ret = %d", __FUNCTION__, ret);
    }
    /* second, stop the pthread */
    pthread_join(hReadThread, NULL);
    ALOGD("pthread_join hReadThread done");

    ClosePCMDump();

    //close VOW kernel driver
    release_wake_lock(VOW_DEBUG_WAKELOCK_NAME);
    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

void  AudioALSACaptureDataProviderVOW::WriteVOWPcmData() {
    ALOGV("+%s()", __FUNCTION__);
    if (mPCMDumpFile) {
        //ALOGD("%s()", __FUNCTION__);
        AudioDumpPCMData((void *)vow_info_buf.addr, vow_info_buf.size, mPCMDumpFile);
    }
    ALOGV("-%s()", __FUNCTION__);
}

void *AudioALSACaptureDataProviderVOW::readThread(void *arg) {
    AudioALSACaptureDataProviderVOW *pDataProvider = static_cast<AudioALSACaptureDataProviderVOW *>(arg);
    uint32_t open_index = pDataProvider->mOpenIndex;

    char nameset[32];
    sprintf(nameset, "%s%d", __FUNCTION__, pDataProvider->mCaptureDataProviderType);
    prctl(PR_SET_NAME, (unsigned long)nameset, 0, 0, 0);
    pDataProvider->setThreadPriority();

    ALOGD("+%s(), pid: %d, tid: %d, kReadBufferSize=%x", __FUNCTION__, getpid(), gettid(), kReadBufferSize);

    // read raw data from alsa driver
    char linear_buffer[kReadBufferSize];
    uint32_t Read_Size = kReadBufferSize;
    uint32_t return_size;
    //memset(linear_buffer, 0xCCCC, Read_Size);
    bool enable_debug = false;
    pDataProvider->vow_info_buf.addr = (long)linear_buffer;
    pDataProvider->vow_info_buf.size = (long)Read_Size;
    pDataProvider->vow_info_buf.return_size_addr = (long)&return_size;
    unsigned long vow_info = (unsigned long)&pDataProvider->vow_info_buf;

    ALOGD("%s(), vow_info = %lu, addr=%lu, size=%ld", __FUNCTION__, vow_info, pDataProvider->vow_info_buf.addr, pDataProvider->vow_info_buf.size);

    int ret = ::ioctl(pDataProvider->mFd, VOW_SET_APREG_INFO, vow_info);
    ALOGD("%s(), VOW_SET_APREG_INFO set, ret = %d", __FUNCTION__, ret);
    if (ret != 0) {
        ALOGE("%s(), VOW_SET_APREG_INFO error, ret = %d", __FUNCTION__, ret);
    }

    /*ret = ::ioctl(pDataProvider->mFd, VOW_SET_CONTROL, (unsigned int)VOWControlCmd_EnableDebug);
    ALOGD("%s(), VOWControlCmd_EnableDebug set, ret = %d", __FUNCTION__, ret);
    if (ret != 0)
    {
        ALOGE("%s(), VOWControlCmd_EnableDebug error, ret = %d", __FUNCTION__, ret);
    }*/

    while (pDataProvider->mEnable == true) {
#if 0
        if (open_index != pDataProvider->mOpenIndex) {
            ALOGD("%s(), open_index(%d) != mOpenIndex(%d), return", __FUNCTION__, open_index, pDataProvider->mOpenIndex);
            break;
        }
#endif

        //ASSERT(pDataProvider->mPcm != NULL);
        if (!enable_debug) {
            ALOGI("%s(), VOWControlCmd_EnableDebug set, ret = %d", __FUNCTION__, ret);
            ret = ::ioctl(pDataProvider->mFd, VOW_SET_CONTROL, (unsigned int)VOWControlCmd_EnableDebug);
            if (ret != 0) {
                ALOGE("%s(), VOWControlCmd_EnableDebug error, ret = %d", __FUNCTION__, ret);
            }
            enable_debug = true;
        }
        int retval = ::ioctl(pDataProvider->mFd, VOW_SET_CONTROL, (unsigned int)VOWControlCmd_ReadVoiceData);
        if (retval != 0) {
            ALOGE("%s(), pcm_read() error, retval = %d", __FUNCTION__, retval);
        } else {
            ALOGV("%s(), pcm_read() retval = %d", __FUNCTION__, retval);
        }
        if (return_size > kReadBufferSize) {
            return_size = kReadBufferSize;
        }
        pDataProvider->mPcmReadBuf.pBufBase = linear_buffer;
        //pDataProvider->mPcmReadBuf.bufLen   = kReadBufferSize + 1; // +1: avoid pRead == pWrite
        pDataProvider->mPcmReadBuf.bufLen   = return_size + 1;
        pDataProvider->mPcmReadBuf.pRead    = linear_buffer;
        pDataProvider->mPcmReadBuf.pWrite   = linear_buffer + return_size;
        pDataProvider->provideCaptureDataToAllClients(open_index);
        //pDataProvider->WriteVOWPcmData();

        usleep(8000);
    }

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_exit(NULL);
    return NULL;
}

} // end of namespace android

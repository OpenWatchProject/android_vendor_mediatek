#include "AudioALSALoopbackController.h"
#include "AudioBTCVSDControl.h"
#include "AudioALSADriverUtility.h"
#include "AudioALSADeviceParser.h"
#include "AudioALSAStreamManager.h"
#include "AudioALSAHardwareResourceManager.h"
#include "WCNChipController.h"
#include "AudioALSAPlaybackHandlerBTCVSD.h"
#include "AudioALSAStreamOut.h"

#if defined(MTK_AUDIO_KS)
#include "AudioALSADeviceConfigManager.h"
#include "AudioSmartPaController.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSALoopbackController"

static char const *const kBTCVSDDeviceName = "/dev/ebc";
#define AUD_DRV_IOC_MAGIC 'C'
#define ALLOCATE_FREE_BTCVSD_BUF _IOWR(AUD_DRV_IOC_MAGIC, 0xE0, unsigned int)
#define SET_BTCVSD_STATE         _IOWR(AUD_DRV_IOC_MAGIC, 0xE1, unsigned int)
#define GET_BTCVSD_STATE         _IOWR(AUD_DRV_IOC_MAGIC, 0xE2, unsigned int)

namespace android {
//static AudioMTKStreamOutInterface *streamOutput;
AudioMTKStreamOutInterface *streamOutput = NULL;
static struct mixer *mMixerForBtLoopback;

AudioALSALoopbackController *AudioALSALoopbackController::mAudioALSALoopbackController = NULL;
AudioALSALoopbackController *AudioALSALoopbackController::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSALoopbackController == NULL) {
        mAudioALSALoopbackController = new AudioALSALoopbackController();
    }
    ASSERT(mAudioALSALoopbackController != NULL);
    return mAudioALSALoopbackController;
}


AudioALSALoopbackController::AudioALSALoopbackController() :
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()),
    mPcmDL(NULL),
    mPcmUL(NULL),
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    mFd2(0),
    mBtLoopbackWithCodec(false),
    mBtLoopbackWithoutCodec(false),
    mUseBtCodec(false) {
    mMixerForBtLoopback = AudioALSADriverUtility::getInstance()->getMixer();

    memset((void *)&mConfig, 0, sizeof(mConfig));
}


AudioALSALoopbackController::~AudioALSALoopbackController() {

}


status_t AudioALSALoopbackController::open(const audio_devices_t output_devices,
                                           const audio_devices_t input_device,
                                           loopback_t loopbackType) {
#if !defined(MTK_AUDIO_KS)
    UNUSED(loopbackType);
#endif
    ALOGD("+%s(), output_devices = 0x%x, input_device = 0x%x", __FUNCTION__, output_devices, input_device);
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    // DL loopback setting
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = 2;
    mConfig.rate = 48000;
    mConfig.period_size = 1024;
    mConfig.period_count = 2;
    mConfig.format = PCM_FORMAT_S16_LE;
    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;
    ALOGD("+%s(), mConfig.rate=%d", __FUNCTION__, mConfig.rate);

#if defined(MTK_AUDIO_KS)
    mConfig.stop_threshold = ~(0U);
    if ((output_devices & AUDIO_DEVICE_OUT_SPEAKER) &&
        AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        mApTurnOnSequence = mHardwareResourceManager->getSingleMicLoopbackPath(output_devices, false, loopbackType);

        if (popcount(output_devices) > 1) {
            mApTurnOnSequence2 = mHardwareResourceManager->getSingleMicLoopbackPath(output_devices, true, loopbackType);
        }
    } else {
        mApTurnOnSequence = mHardwareResourceManager->getSingleMicLoopbackPath(output_devices, false, loopbackType);
    }
    mHardwareResourceManager->setCustOutputDevTurnOnSeq(output_devices, mTurnOnSeqCustDev1, mTurnOnSeqCustDev2);

    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence2);

    int cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmHostlessLpbk);
    int pcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessLpbk);
    int pcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessLpbk);

    mHardwareResourceManager->startInputDevice(input_device);
#else
    int pcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmUlDlLoopback);
    int pcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmUlDlLoopback);
    int cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmUlDlLoopback);
#endif

    ASSERT(mPcmUL == NULL && mPcmDL == NULL);
    mPcmUL = pcm_open(cardIndex, pcmInIdx, PCM_IN, &mConfig);
    mPcmDL = pcm_open(cardIndex, pcmOutIdx, PCM_OUT, &mConfig);
    ASSERT(mPcmUL != NULL && mPcmDL != NULL);
    ALOGV("%s(), mPcmUL = %p, mPcmDL = %p", __FUNCTION__, mPcmUL, mPcmDL);

    if (input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
        if (mHardwareResourceManager->getNumOfHeadsetPole() == 5) {
            setLoopbackUseLCh(false);
        } else {
            setLoopbackUseLCh(true);
        }
    }

    pcm_start(mPcmUL);
    pcm_start(mPcmDL);

#if !defined(MTK_AUDIO_KS)
    mHardwareResourceManager->startInputDevice(input_device);
#endif
    mHardwareResourceManager->startOutputDevice(output_devices, mConfig.rate);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSALoopbackController::close() {
    ALOGD("+%s()", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    mHardwareResourceManager->stopOutputDevice();
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev1);
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev2);

    pcm_stop(mPcmDL);
    pcm_stop(mPcmUL);
    pcm_close(mPcmDL);
    pcm_close(mPcmUL);

    mPcmDL = NULL;
    mPcmUL = NULL;

#if defined(MTK_AUDIO_KS)
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence2);
#endif

    if (mHardwareResourceManager->getInputDevice() == AUDIO_DEVICE_IN_WIRED_HEADSET) {
        setLoopbackUseLCh(false);
    }

    mHardwareResourceManager->stopInputDevice(mHardwareResourceManager->getInputDevice());

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSALoopbackController::SetApBTCodec(bool enable_codec) {
    ALOGD("+%s(), enable_codec = %d", __FUNCTION__, enable_codec);
    mUseBtCodec = enable_codec;
    return NO_ERROR;
}

bool AudioALSALoopbackController::IsAPBTLoopbackWithCodec(void) {
    //ALOGD("+%s(), mBtLoopbackWithCodec = %d", __FUNCTION__, mBtLoopbackWithCodec);
    return mBtLoopbackWithCodec;
}

status_t AudioALSALoopbackController::OpenAudioLoopbackControlFlow(const audio_devices_t input_device,
                                                                   const audio_devices_t output_device) {
    // check BT device
    const bool bt_device_on = audio_is_bluetooth_sco_device(output_device);

    ALOGD("+%s(), input_device = 0x%x, output_device = 0x%x, bt_device_on = %d, mUseBtCodec = %d,"
          "mBtLoopbackWithoutCodec: %d, mBtLoopbackWithCodec: %d", __FUNCTION__,
          input_device, output_device, bt_device_on, mUseBtCodec, mBtLoopbackWithoutCodec, mBtLoopbackWithCodec);
    if (bt_device_on == true) {
        // DAIBT
        if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true) {
            if (!mUseBtCodec) {
                mBtLoopbackWithoutCodec = 1;
                mFd2 = 0;
#ifndef MTK_SUPPORT_BTCVSD_ALSA
                mFd2 = ::open(kBTCVSDDeviceName, O_RDWR);
                ALOGD("+%s(), CVSD AP loopback without codec, mFd2: %d, AP errno: %d", __FUNCTION__, mFd2, errno);
                ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 0); //allocate TX working buffers in kernel
                ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 2); //allocate TX working buffers in kernel
                ::ioctl(mFd2, SET_BTCVSD_STATE, BT_SCO_TXSTATE_DIRECT_LOOPBACK); //set state to kernel
#else
                if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixerForBtLoopback, "BTCVSD Loopback Switch"), 0, 1)) {
                    ALOGE("Error: BTCVSD Loopback Switch invalid value");
                } else {
                    ALOGD("+%s(), CVSD AP loopback without codec, BT_DIRECT_LOOPBACK==On", __FUNCTION__);
                }
#endif
            } else {
                int format = AUDIO_FORMAT_PCM_16_BIT;
                uint32_t channels = AUDIO_CHANNEL_OUT_MONO;
                uint32_t sampleRate = 8000;
                status_t status = NO_ERROR;
                mBtLoopbackWithCodec = 1;
                streamOutput = AudioALSAStreamManager::getInstance()->openOutputStream(output_device, &format,
                                                                                       &channels, &sampleRate,
                                                                                       &status);
                ALOGD("+%s(), CVSD AP loopback with codec, streamOutput: %p", __FUNCTION__, streamOutput);
                mBTCVSDLoopbackThread = new AudioMTKLoopbackThread();
                if (mBTCVSDLoopbackThread.get()) {
                    mBTCVSDLoopbackThread->run("mBTCVSDLoopbackThread");
                }
            }
        }
    }

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSALoopbackController::CloseAudioLoopbackControlFlow(void) {
    AL_AUTOLOCK(mLock);
    ALOGD("+%s()", __FUNCTION__);

    //const bool bt_device_on = audio_is_bluetooth_sco_device(output_device);
    bool bt_device_on = true;

    ALOGD("%s(), bt_device_on = %d, mBtLoopbackWithoutCodec: %d, mBtLoopbackWithCodec: %d",
          __FUNCTION__, bt_device_on, mBtLoopbackWithoutCodec, mBtLoopbackWithCodec);

    if (bt_device_on) {
        if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true) {
            if (mBtLoopbackWithoutCodec) {
#ifndef MTK_SUPPORT_BTCVSD_ALSA
                ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 1); //allocate TX working buffers in kernel
                ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 3); //allocate TX working buffers in kernel
                ::ioctl(mFd2, SET_BTCVSD_STATE, BT_SCO_TXSTATE_IDLE); //set state to kernel
#else
                if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixerForBtLoopback, "BTCVSD Loopback Switch"), 0, 0)) {
                    ALOGE("Error: BTCVSD Loopback Switch invalid value");
                } else {
                    ALOGD("%s(), CVSD AP loopback without codec, BT_DIRECT_LOOPBACK==Off", __FUNCTION__);
                }
#endif
                mBtLoopbackWithoutCodec = 0;
            } else if (mBtLoopbackWithCodec) {
                ((AudioALSAStreamOut *)streamOutput)->setSuspend(true);
                streamOutput->standbyStreamOut();
                if (mBTCVSDLoopbackThread.get()) {
                    int ret = 0;
                    ret = mBTCVSDLoopbackThread->requestExitAndWait();
                    if (ret == WOULD_BLOCK) {
                        mBTCVSDLoopbackThread->requestExit();
                    }
                    mBTCVSDLoopbackThread.clear();
                }
                AudioALSAStreamManager::getInstance()->closeOutputStream(streamOutput);
                mBtLoopbackWithCodec = 0;
            }
        }
    }

    ALOGD("-%s()", __FUNCTION__);

    return NO_ERROR;
}


void AudioALSALoopbackController::setLoopbackUseLCh(bool enable) {
    enum mixer_ctl_type type;
    struct mixer_ctl *ctl;
    int retval = 0;

    ctl = mixer_get_ctl_by_name(mMixer, "LPBK_IN_USE_LCH");

    if (ctl == NULL) {
        ALOGE("LPBK_IN_USE_LCH not support");
        return;
    }

    if (enable == true) {
        retval = mixer_ctl_set_enum_by_string(ctl, "On");
        ASSERT(retval == 0);
    } else {
        retval = mixer_ctl_set_enum_by_string(ctl, "Off");
        ASSERT(retval == 0);
    }
}


#if 1 //0902

extern void CVSDLoopbackResetBuffer();
extern void CVSDLoopbackReadDataDone(uint32_t len);
extern void CVSDLoopbackGetReadBuffer(uint8_t **buffer, uint32_t *buf_len);
extern int32_t CVSDLoopbackGetDataCount();

AudioALSALoopbackController::AudioMTKLoopbackThread::AudioMTKLoopbackThread() {
    ALOGD("BT_SW_CVSD AP loopback AudioMTKLoopbackThread constructor");
}

AudioALSALoopbackController::AudioMTKLoopbackThread::~AudioMTKLoopbackThread() {
    ALOGD("BT_SW_CVSD AP loopback ~AudioMTKLoopbackThread");
}

void AudioALSALoopbackController::AudioMTKLoopbackThread::onFirstRef() {
    ALOGD("BT_SW_CVSD AP loopback AudioMTKLoopbackThread::onFirstRef");
    run(mName, ANDROID_PRIORITY_URGENT_AUDIO);
}

status_t  AudioALSALoopbackController::AudioMTKLoopbackThread::readyToRun() {
    ALOGD("BT_SW_CVSD AP loopback AudioMTKLoopbackThread::readyToRun()");
    return NO_ERROR;
}

bool AudioALSALoopbackController::AudioMTKLoopbackThread::threadLoop() {
    uint8_t *pReadBuffer;
    uint32_t uReadByte, uWriteDataToBT;
    CVSDLoopbackResetBuffer();
    while (!(exitPending() == true)) {
        ALOGD("BT_SW_CVSD AP loopback threadLoop(+)");
        uWriteDataToBT = 0;
        CVSDLoopbackGetReadBuffer(&pReadBuffer, &uReadByte);
        uReadByte &= 0xFFFFFFFE;

        if (uReadByte > 480) {
            uReadByte = 480;
        }

        if (uReadByte) {
            uWriteDataToBT = streamOutput->write(pReadBuffer, uReadByte);
            CVSDLoopbackReadDataDone(uWriteDataToBT);
        } else {
            usleep(5 * 1000); //5ms
        }
        ALOGD("BT_SW_CVSD AP loopback threadLoop(-), uReadByte: %d, uWriteDataToBT: %d", uReadByte, uWriteDataToBT);
    }
    ALOGD("BT_SW_CVSD AP loopback threadLoop exit");
    return false;
}

#endif



} // end of namespace android

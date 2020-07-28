#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSASpeechStreamController"

#include <unistd.h>
#include <sched.h>
#include <sys/prctl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <cutils/properties.h>
#include "AudioALSAHardwareResourceManager.h"
#include "SpeechDriverInterface.h"
#include "SpeechDriverFactory.h"
#include "AudioALSAStreamOut.h"
#include "AudioALSAStreamIn.h"
#include "AudioALSASpeechStreamController.h"
#include "AudioALSASpeechStreamController.h"
#include "AudioALSACaptureHandlerModemDai.h"
#include "MtkAudioComponent.h"
#include "AudioDeviceInt.h"

#define FRAME_BLOCK_SIZE 320

namespace android {

/*==============================================================================
 *                     Property keys
 *============================================================================*/
AudioALSASpeechStreamController *AudioALSASpeechStreamController::UniqueInstance = NULL;
AudioALSASpeechStreamController *AudioALSASpeechStreamController::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (UniqueInstance == NULL) {
        UniqueInstance = new AudioALSASpeechStreamController();
    }
    ASSERT(UniqueInstance != NULL);
    return UniqueInstance;
}

AudioALSASpeechStreamController::AudioALSASpeechStreamController() {
    int ret;
    ALOGD("%s()", __FUNCTION__);

    m_bEnabled = false;
    m_bThreadExit = false;
    mOutputDevices = AUDIO_DEVICE_NONE;
    mHardwareResourceManager = NULL;
    mAudioMode  = AUDIO_MODE_NORMAL;
    mRoutingForTty = 0;
    mPcmIn = NULL;
    mPcmOut = NULL;
    mSpeechStreamThreadID = 0;
    mAudioMtkStreamManager = AudioALSAStreamManager::getInstance();

    ret = pthread_mutex_init(&mSpeechStreamMutex, NULL);
    if (ret != 0) {
        ALOGE("Failed to initialize mSpeechStreamMutex!");
    }

    ret = pthread_cond_init(&mSphStream_Cond, NULL);
    if (ret != 0) {
        ALOGE("Failed to initialize mSphStream_Cond!");
    }
}

AudioALSASpeechStreamController::~AudioALSASpeechStreamController() {
    ALOGD("%s()", __FUNCTION__);
    pthread_cond_destroy(&mSphStream_Cond);
}

void *AudioALSASpeechStreamController::SpeechStreamThread(void *arg) {
    ALOGD("%s() +", __FUNCTION__);

    /*
      *  deivce setting
    */
    int Inputformat = AUDIO_FORMAT_PCM_16_BIT;
    uint32_t Inputdevice = AUDIO_DEVICE_IN_SPK_FEED;
    uint32_t Inputchannel = AUDIO_CHANNEL_IN_STEREO;
    uint32_t InputsampleRate = 16000;
    status_t status = 0;

    /**
    * Bit Converter
    */
    MtkAudioBitConverterBase *mBitConverter = NULL;
    int   *mBitConverterOutputBuffer = NULL;
    uint32_t bytesAfterpostprocessings = 0;
    uint32_t bytesBeforepostprocessings = 0;

    /*
    *  StreamIn Read Parameters
    */
    const int ReadStreamInLength = (FRAME_BLOCK_SIZE * sizeof(short));
    int mStreaimInReadBytes = 0;
    void *pWriteBuffer = NULL;
    short *ReadStreamInBuffer = NULL;

    AudioMTKStreamInInterface *streamInput = NULL;
    AudioMTKStreamOutInterface *streamOutput = NULL;
    AudioALSASpeechStreamController *pSphStrmCtrl = static_cast<AudioALSASpeechStreamController *>(arg);

    if (pSphStrmCtrl == NULL) {
        ALOGE("SpeechStreamThread pSphStrmCtrl = NULL");
        return 0;
    }

    if (mBitConverter == NULL) {
        mBitConverter = newMtkAudioBitConverter(InputsampleRate, 2, BCV_IN_Q1P15_OUT_Q1P31);
        ASSERT(mBitConverter != NULL);
        if (mBitConverter != NULL) {
            mBitConverter->open();
            mBitConverter->resetBuffer();
        }
        if (mBitConverterOutputBuffer == NULL) {
            mBitConverterOutputBuffer = new int[FRAME_BLOCK_SIZE];
            ASSERT(mBitConverterOutputBuffer != NULL);
        }
    }

    if (ReadStreamInBuffer == NULL) {
        ReadStreamInBuffer = new short[FRAME_BLOCK_SIZE];
        memset(ReadStreamInBuffer, 0, sizeof(short)*FRAME_BLOCK_SIZE);
    }

    pthread_mutex_lock(&pSphStrmCtrl->mSpeechStreamMutex);
    pSphStrmCtrl->m_bThreadExit = false;

    /*
    *   Adjust thread priority
    */
    prctl(PR_SET_NAME, (unsigned long)"SpeechStreamPlayback", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_HIGHEST);
    ALOGD("+%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    /*
      *  ----start the loop --------
    */
    streamInput = pSphStrmCtrl->getStreamManager()->openInputStream(Inputdevice, &Inputformat, &Inputchannel, &InputsampleRate, &status, (audio_in_acoustics_t)0);
    ASSERT(streamInput != NULL);

    /*
    *  output device setting
    */

    uint32_t OutputDevice = AUDIO_DEVICE_OUT_SPEAKER_SAFE; // need to different fomr normal playback
    uint32_t Outputchannel = AUDIO_CHANNEL_OUT_STEREO;
    int  Outputformat = AUDIO_FORMAT_PCM_32_BIT;
    uint32_t sampleRate = 16000;
    streamOutput = pSphStrmCtrl->getStreamManager()->openOutputStream(OutputDevice, &Outputformat, &Outputchannel, &sampleRate, &status);
    ASSERT(streamOutput != NULL);


    ALOGD("pthread_cond_signal(&pSpkMonitor->mSphStream_Cond)");
    pthread_cond_signal(&pSphStrmCtrl->mSphStream_Cond);
    pthread_mutex_unlock(&pSphStrmCtrl->mSpeechStreamMutex);
    ALOGD("%s() loop start", __FUNCTION__);

    //AudioALSAHardwareResourceManager::getInstance ()->setSgenMode(SGEN_MODE_O12);

    while (pSphStrmCtrl->m_bEnabled && pSphStrmCtrl->m_bThreadExit == false) {
        if (streamInput != NULL) {
            mStreaimInReadBytes = streamInput->read(ReadStreamInBuffer, ReadStreamInLength);
            if (mBitConverter != NULL) {
                bytesBeforepostprocessings = ReadStreamInLength;
                bytesAfterpostprocessings = ReadStreamInLength * 2;
                mBitConverter->process(ReadStreamInBuffer, &bytesBeforepostprocessings, (void *)mBitConverterOutputBuffer, &bytesAfterpostprocessings);
                pWriteBuffer = (void *)mBitConverterOutputBuffer;
            }

            if (streamOutput != NULL && pSphStrmCtrl->m_bEnabled == true && pSphStrmCtrl->m_bThreadExit == false) {
                streamOutput->write(pWriteBuffer, bytesAfterpostprocessings);
            }
        }
    }

    ALOGD("%s() loop end", __FUNCTION__);
    if (streamOutput != NULL) {
        streamOutput->standbyStreamOut();
        pSphStrmCtrl->getStreamManager()->closeOutputStream(streamOutput);
        streamOutput = NULL;
    }

    if (streamInput != NULL) {
        streamInput->standby();
        pSphStrmCtrl->getStreamManager()->closeInputStream(streamInput);
        streamInput = NULL;
    }

    if (ReadStreamInBuffer != NULL) {
        delete[] ReadStreamInBuffer;
        ReadStreamInBuffer = NULL;
    }

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

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_mutex_lock(&pSphStrmCtrl->mSpeechStreamMutex);
    ALOGD("pthread_cond_signal(&pSphStrmCtrl->mSphStream_Cond)");
    pthread_cond_signal(&pSphStrmCtrl->mSphStream_Cond);
    pthread_mutex_unlock(&pSphStrmCtrl->mSpeechStreamMutex);
    pthread_exit(NULL);
    return 0;
}

audio_devices_t AudioALSASpeechStreamController::GetStreamOutputDevice(void) {
    ALOGD("%s mOutputDevices = %d", __FUNCTION__, mOutputDevices);
    return mOutputDevices;
}

status_t AudioALSASpeechStreamController::SetStreamOutputDevice(audio_devices_t OutputDevices) {
    ALOGD("%s(), %d", __FUNCTION__, OutputDevices);
    mOutputDevices = OutputDevices;
    return NO_ERROR;
}

bool AudioALSASpeechStreamController::IsSpeechStreamThreadEnable(void) {
    ALOGD("%s(), %d", __FUNCTION__, m_bEnabled);
    return m_bEnabled;
}

status_t AudioALSASpeechStreamController::EnableSpeechStreamThread(bool enable) {
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;
    int ret;
    void *status;
    ALOGD("%s() %d", __FUNCTION__, enable);

    if (enable == true && m_bEnabled == false) {
        ALOGD("open SpeechStreamThread");
        pthread_mutex_lock(&mSpeechStreamMutex);
        ret = pthread_create(&mSpeechStreamThreadID, NULL, AudioALSASpeechStreamController::SpeechStreamThread, (void *)this);
        if (ret != 0) {
            ALOGE("EnableSpeechStreamThread pthread_create error!!");
        }

        ALOGD("+mSphStream_Cond wait");
        m_bEnabled = true;
        ret = pthread_cond_timedwait(&mSphStream_Cond, &mSpeechStreamMutex, &timeout);
        ALOGD("-mSphStream_Cond receive ret=%d", ret);
        pthread_mutex_unlock(&mSpeechStreamMutex);

    } else if (enable == false && m_bEnabled == true) {
        ALOGD("close SpeechStreamThread");
        pthread_mutex_lock(&mSpeechStreamMutex);
        if (!m_bThreadExit) {
            m_bThreadExit = true;
            ALOGD("+mSphStream_Cond wait");
            ret = pthread_cond_timedwait(&mSphStream_Cond, &mSpeechStreamMutex, &timeout);
            ALOGD("-mSphStream_Cond receive ret=%d", ret);
        }
        m_bEnabled = false;
        pthread_mutex_unlock(&mSpeechStreamMutex);
        ALOGD("%s +pthread_join", __FUNCTION__);
        pthread_join(mSpeechStreamThreadID, &status);
        ALOGD("%s -pthread_join", __FUNCTION__);
    }
    return NO_ERROR;
}

} // end of namespace android

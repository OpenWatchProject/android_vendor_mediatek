#include "AudioALSAPcmDataCaptureIn.h"
#include "AudioUtility.h"
#include "AudioType.h"
#include <AudioLock.h>
#include "AudioALSADeviceParser.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include "AudioALSADriverUtility.h"
#include <cutils/sched_policy.h>

#define LOG_TAG "AudioALSAPcmDataCaptureIn"
#define IV_RETRY_TIMES 10

namespace android {


AudioALSAPcmDataCaptureIn *AudioALSAPcmDataCaptureIn::mAudioALSAPcmDataCaptureIn = NULL;

AudioALSAPcmDataCaptureIn *AudioALSAPcmDataCaptureIn::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSAPcmDataCaptureIn == NULL) {
        mAudioALSAPcmDataCaptureIn = new AudioALSAPcmDataCaptureIn();
    }
    ASSERT(mAudioALSAPcmDataCaptureIn != NULL);
    return mAudioALSAPcmDataCaptureIn;
}


AudioALSAPcmDataCaptureIn::AudioALSAPcmDataCaptureIn() {
    ALOGD("%s()", __FUNCTION__);
    mAudioPcmInputThread = new AudioPCMInputThread(this);
    ALOGD("%s new mAudioPcmInputThread", __func__);

    if (mAudioPcmInputThread.get() != NULL) {
        ALOGD("mAudioPcmInputThread->run();");
        mAudioPcmInputThread->run("AudioALSAPcmDataCaptureIn");
    } else {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
    }
    memset((void *)&mStreamAttributeSource, 0, sizeof(stream_attribute_t));

    ALOGD("-%s()", __FUNCTION__);
}

AudioALSAPcmDataCaptureIn::~AudioALSAPcmDataCaptureIn() {
    ALOGD("%s()", __FUNCTION__);

    int ret = 0;
    ret = mAudioPcmInputThread->requestExitAndWait();
    if (ret == WOULD_BLOCK) {
        mAudioPcmInputThread->requestExit();
    }
    mAudioPcmInputThread.clear();
    mAudioPcmInputThread = NULL;

    ALOGD("-%s()", __FUNCTION__);
}

ssize_t AudioALSAPcmDataCaptureIn::read(void *buffer, ssize_t bytes) {
    int retcode = 0;
    retcode = mAudioPcmInputThread->readIVData(buffer, bytes);
    return retcode;
}

status_t AudioALSAPcmDataCaptureIn::SetThreadEnable() {
    mAudioPcmInputThread->SetThreadEnable();
    return NO_ERROR;
}

status_t AudioALSAPcmDataCaptureIn::SetThreadDisable() {
    mAudioPcmInputThread->SetThreadDisable();
    return NO_ERROR;
}


status_t AudioALSAPcmDataCaptureIn::Standby() {
    ALOGD("%s", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAPcmDataCaptureIn::GetPcmConfig(pcm_config *mPcmInputConfig) {
    mAudioPcmInputThread->GetPcmConfig(mPcmInputConfig);
    return NO_ERROR;
}

status_t AudioALSAPcmDataCaptureIn::SetPcmConfig(pcm_config mPcmInputConfig) {
    mAudioPcmInputThread->SetPcmConfig(mPcmInputConfig);
    return NO_ERROR;
}

status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread::SetPcmConfig(pcm_config I2SInputConfig) {
    ALOGD("%s", __FUNCTION__);
    memcpy((void *)&mPcmInputConfig, (void *)&I2SInputConfig, sizeof(pcm_config));
    return NO_ERROR;
}

status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread::GetPcmConfig(pcm_config *pI2SInputConfig) {
    ALOGD("%s", __FUNCTION__);
    memcpy((void *)pI2SInputConfig, (void *)&mPcmInputConfig, sizeof(pcm_config));
    return NO_ERROR;
}

void AudioALSAPcmDataCaptureIn::AudioPCMInputThread::onFirstRef() {
    ALOGD("%s", __func__);
    mReadBufferSize = 0;
    mReadBuffer =  NULL;
    mbRunning = false;
    mbStatusChange = false;
    mReadBuffer = NULL;
    mReadBufferSize = 0;
    memset((void *)&mPcmInputConfig, 0, sizeof(pcm_config));
    AllocateResource();
}

void AudioALSAPcmDataCaptureIn::AudioPCMInputThread::OpenPCMDump(const char *class_name) {
    char mDumpFileName[128];
    sprintf(mDumpFileName, "%s.%d.%s.pcm", streaminIVIn, DumpIVFileNum, class_name);

    mPCMIVDumpFile = NULL;
    mPCMIVDumpFile = AudioOpendumpPCMFile(mDumpFileName, streamout_propty);

    if (mPCMIVDumpFile != NULL) {
        ALOGD("%s DumpFileName = %s", __FUNCTION__, mDumpFileName);
        DumpIVFileNum++;
        DumpIVFileNum %= MAX_DUMP_NUM;
    }
}

void AudioALSAPcmDataCaptureIn::AudioPCMInputThread::ClosePCMDump() {
    if (mPCMIVDumpFile) {
        AudioCloseDumpPCMFile(mPCMIVDumpFile);
        ALOGD("%s(), close it", __FUNCTION__);
    }
}

status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread:: SetThreadEnable() {
    uint32_t tryCount = 0;

    ALOGD("%s", __FUNCTION__);
    Mutex::Autolock _l(mPcmLock);
    OpenPCMDump(LOG_TAG);
    mbStatusChange = mbRunning ^ 1;
    ALOGD("%s mbStatusChange = %d mbRunning = %d", __FUNCTION__, mbStatusChange, mbRunning);
    mbRunning = true;
    if (mbStatusChange == true) {
        mWaitWorkCV.signal();
    }
    ALOGD("+%s", __FUNCTION__);
    status_t retval = mPcmWaitWorkCV.waitRelative(mPcmLock, milliseconds(50));
    while (retval !=  NO_ERROR && tryCount < IV_RETRY_TIMES) {
        tryCount++;
        ALOGD("%s try mPcmWaitWorkCV tryCount = %d retval = %d", __FUNCTION__, tryCount, retval);
        mWaitWorkCV.signal();
        retval = mPcmWaitWorkCV.waitRelative(mPcmLock, milliseconds(50));

    }
    ALOGD("-%s retval = %d", __FUNCTION__, retval);
    return NO_ERROR;
}


status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread:: SetThreadDisable() {
    ALOGD("+%s", __FUNCTION__);
    Mutex::Autolock _l(mPcmLock);
    ClosePCMDump();
    mbStatusChange = mbRunning ^ 0;
    mbRunning = false;
    ALOGD("++%s", __FUNCTION__);
    mPcmWaitWorkCV.wait(mPcmLock);
    ALOGD("-%s", __FUNCTION__);
    return NO_ERROR;
}

/*
* Read from Ringbuffer
* @ param buffer : buffer to be filled
* @ param bytes : how many bytes to be read
*/
int AudioALSAPcmDataCaptureIn::AudioPCMInputThread::readIVData(void *buffer, ssize_t bytes) {
    Mutex::Autolock _l(mRingLock);
    int datacount =  RingBuf_getDataCount(&mRawDataBuf);
    //ALOGD("+%s bytes = %d datacount = %d", __FUNCTION__, bytes,datacount);
    if (datacount < bytes) {
        ALOGD("%s datacount < bytes datacount = %d bytes = %d", __FUNCTION__, datacount, bytes);
        memset(buffer,0,bytes);
        return bytes;
    }
    RingBuf_copyToLinear((char *)buffer, &mRawDataBuf, bytes);
    //ALOGD("-%s bytes = %d", __func__, bytes);
    return bytes;
}


status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread::OpenPcm() {
    // open pcm driver
    int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmI2S2ADCCapture);
    int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmI2S2ADCCapture);
    ALOGD("%s channels = %d format = %d period_count = %d period_size= %d", __FUNCTION__,
          mPcmInputConfig.channels, mPcmInputConfig.format, mPcmInputConfig.period_count, mPcmInputConfig.period_size);
    mInputPcm = pcm_open(cardindex, pcmindex, PCM_IN, &mPcmInputConfig);
    if (mInputPcm == NULL) {
        ALOGE("%s(), mPcm == NULL!!", __FUNCTION__);
    }
    ALOGD("-%s(), mPcm = %p", __FUNCTION__, mInputPcm);
    ASSERT(mInputPcm != NULL);
    StartPcm();

    return NO_ERROR;
}

status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread::ClosePcm() {
    ALOGD("%s", __FUNCTION__);
    if (mInputPcm != NULL) {
        pcm_stop(mInputPcm);
        pcm_close(mInputPcm);
        mInputPcm = NULL;
    }
    return NO_ERROR;
}

status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread::StartPcm() {
    ALOGD("%s(), mPcm = %p", __FUNCTION__, mInputPcm);
    int ret = pcm_start(mInputPcm);
    if (ret) {
        ALOGE("%s(), pcm_start(%p) fail due to %s", __FUNCTION__, mInputPcm, pcm_get_error(mInputPcm));
        ASSERT(0);
    }
    return ret;
}

unsigned int AudioALSAPcmDataCaptureIn::AudioPCMInputThread::FormatTransfer(
    int SourceFormat, int TargetFormat, void *Buffer, unsigned int mReadBufferSize) {
    unsigned mReformatSize = 0;
    int *srcbuffer = (int *)Buffer;
    short *dstbuffer = (short *)Buffer;
    if (SourceFormat == PCM_FORMAT_S32_LE && TargetFormat == PCM_FORMAT_S16_LE) {
        short temp = 0;
        while (mReadBufferSize) {
            temp = (short)((*srcbuffer) >> 8);
            *dstbuffer = temp;
            srcbuffer++;
            dstbuffer++;
            mReadBufferSize -= sizeof(int);
            mReformatSize += sizeof(short);
        }
    } else {
        mReformatSize = mReadBufferSize;
    }
    return mReformatSize;
}


int AudioALSAPcmDataCaptureIn::AudioPCMInputThread::GetPcmData() {
    //ALOGD("+%s", __FUNCTION__);
    int ReadSize = 0;
    unsigned int RingFreeSpace = 0;
    unsigned int ReformatSize = mReadBufferSize;
    if (mbRunning == true) {
        if (mInputPcm != NULL) {
            memset((void *)mReadBuffer, 0, mReadBufferSize);
            ReadSize = pcm_read(mInputPcm, (void *)mReadBuffer, mReadBufferSize);
            if (ReadSize != 0) {
                ALOGD("+%s mReadBufferSize = %d ReadSize = %d", __FUNCTION__, mReadBuffer, mReadBufferSize);
                return -EPIPE;
            }

            ReformatSize = FormatTransfer(mPcmInputConfig.format, PCM_FORMAT_S16_LE, (void *)mReadBuffer, mReadBufferSize);
            if (mPCMIVDumpFile != NULL) { // dump data if needed
                AudioDumpPCMData((void *)mReadBuffer, ReformatSize, mPCMIVDumpFile);
            }
        }

        /*
        * copy read data to ring buffer
        */
        mRingLock.lock();
        RingFreeSpace = RingBuf_getFreeSpace(&mRawDataBuf);
        if (RingFreeSpace >= mReadBufferSize) {
            RingBuf_copyFromLinear(&mRawDataBuf, mReadBuffer, ReformatSize);
        } else {
            ALOGE("RingFreeSpace = %d mReadBufferSize = %d", RingFreeSpace, ReformatSize);
        }
        mRingWaitWorkCV.signal();
        mRingLock.unlock();

    } else {
        ALOGD("mbRunning = %d no need read", mbRunning);
        return 0;
    }
    //ALOGD("+%s", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread::AssignReadSize() {
    mReadBufferSize = mPcmInputConfig.period_size;
    mReadBufferSize *= mPcmInputConfig.channels;
    if (mPcmInputConfig.format == PCM_FORMAT_S16_LE) {
        mReadBufferSize *= 2;
    } else if (mPcmInputConfig.format != PCM_FORMAT_S8) {
        mReadBufferSize *= 4;
    }
    ALOGD("%s mReadBufferSize = %u", __FUNCTION__, mReadBufferSize);
    return NO_ERROR;
}


status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread::AllocateResource() {
    AssignReadSize();

    ALOGD("%s mReadBufferSize = %u mReadBufferAllocateSize = %d", __FUNCTION__, mReadBufferSize, mReadBufferAllocateSize);

    if (mReadBuffer == NULL) {
        mReadBuffer = new char[mReadBufferAllocateSize];
        memset((void *)mReadBuffer, 0, mReadBufferAllocateSize);
        ALOGD("%s mReadBufferAllocateSize = %d", __FUNCTION__, mReadBufferAllocateSize);
    }

    if (mRawDataBuf.pBufBase == NULL) {
        mRawDataBuf.bufLen = mReadBufferAllocateSize * 4;
        mRawDataBuf.pBufBase =  new char[mRawDataBuf.bufLen];
        memset((void *)mRawDataBuf.pBufBase, 0, mRawDataBuf.bufLen);
        mRawDataBuf.pWrite =  mRawDataBuf.pBufBase;
        mRawDataBuf.pRead =  mRawDataBuf.pBufBase;
    }

    ASSERT(mRawDataBuf.pBufBase != NULL);

    return NO_ERROR;
}

status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread::ResetResource() {
    ALOGD("+ %s", __FUNCTION__);

    if (mRawDataBuf.pBufBase != NULL) {
        mRawDataBuf.pWrite =  mRawDataBuf.pBufBase;
        mRawDataBuf.pRead =  mRawDataBuf.pBufBase;
    }

    ALOGD("- %s", __FUNCTION__);

    return NO_ERROR;
}


status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread::ReleaseResource() {
    ALOGD("+ %s", __FUNCTION__);
    if (mReadBuffer != NULL) {
        delete []mReadBuffer ;
        mReadBuffer = NULL;
    }

    if (mRawDataBuf.pBufBase != NULL) {
        mRawDataBuf.bufLen = 0;
        delete[] mRawDataBuf.pBufBase;
        mRawDataBuf.pBufBase = NULL;
        mRawDataBuf.pWrite = NULL;
        mRawDataBuf.pRead =  NULL;
    }

    ALOGD("- %s", __FUNCTION__);

    return NO_ERROR;
}


status_t AudioALSAPcmDataCaptureIn::AudioPCMInputThread::ProcessStateChange() {
    if (mbStatusChange == false) {
        return NO_ERROR;
    } else { // state has change
        Mutex::Autolock _l(mPcmLock);
        if (mbRunning == true) { // thread start
            OpenPcm();
            AssignReadSize();
            mPcmWaitWorkCV.signal();
        } else { // threadstop
            ClosePcm();
            ResetResource();
            mPcmWaitWorkCV.signal();
            mRingWaitWorkCV.signal();
        }
        mbStatusChange = false;
    }
    return NO_ERROR;
}

bool AudioALSAPcmDataCaptureIn::AudioPCMInputThread::threadLoop() {
    // do threadloop thing
    unsigned long long diffns, diffus = 0;
    int ReadSize = 0;
    struct timespec tstemp1, tstemp2;
    while (!(exitPending() == true)) {
        mLock.lock();
        if (mbRunning == false && mbStatusChange == false) {
            ALOGD("%s going to sleep", __FUNCTION__);
            mWaitWorkCV.wait(mLock);
            ALOGD("%s waking up", __FUNCTION__);
        }
        /*
        *  threadloop
        */
        tstemp1 = GetSystemTime(false);
        ProcessStateChange();
        ReadSize = GetPcmData();
        mLock.unlock();

        if(ReadSize < 0)
            usleep(20 * 1000);

        tstemp2 = GetSystemTime(false);
        diffns = TimeDifference(tstemp1, tstemp2);
        diffus = diffns / 1000;
        //ALOGD("%s TimeDifference = %llu in ns %llu in us mbStatusChange = %d mbRunning = %d", __FUNCTION__, diffns, diffus,mbStatusChange,mbRunning);
        return true;
    }

    ALOGD("threadLoop exit");
    return false;
}

AudioALSAPcmDataCaptureIn::AudioPCMInputThread::AudioPCMInputThread(AudioALSAPcmDataCaptureIn *PcmDataCaptureIn) {
    ALOGD("%s constructor", __func__);
    mAudioALSAPcmDataCaptureIn = PcmDataCaptureIn;
    mReadBuffer = NULL;
    mReadBufferSize = 0;
    memset((void *)&mRawDataBuf, 0, sizeof(RingBuf));
    mbRunning = false;
    mPCMIVDumpFile = NULL;
    DumpIVFileNum = 0;
    memset((void *)&mPcmInputConfig, 0, sizeof(pcm_config));
    mInputPcm = NULL;
    mbStatusChange = false;
}

AudioALSAPcmDataCaptureIn::AudioPCMInputThread::~AudioPCMInputThread() {
    ALOGD("%s destructor", __func__);
    ReleaseResource();
}

// Good place to do one-time initializations
status_t  AudioALSAPcmDataCaptureIn::AudioPCMInputThread::readyToRun() {
    ALOGD("%s readyToRun()", __func__);

#ifdef MTK_AUDIO_ADJUST_PRIORITY
    // force to set priority
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = RTPM_PRIO_AUDIO_PLAYBACK;
    if (0 != sched_setscheduler(0, SCHED_RR, &sched_p)) {
        ALOGE("[%s] failed, errno: %d", __FUNCTION__, errno);
    } else {
        sched_p.sched_priority = RTPM_PRIO_AUDIO_CCCI_THREAD;
        sched_getparam(0, &sched_p);
        ALOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
    }
#endif
    prctl(PR_SET_NAME, (unsigned long)"AudioIVDataCaptureIn", 0, 0, 0);

    return NO_ERROR;
}


} // end of namespace android



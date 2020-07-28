#include "AudioALSACaptureDataProviderVoice.h"

#include <pthread.h>

#include <sys/prctl.h>

#include "AudioType.h"

#include "SpeechDriverFactory.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureDataProviderVoice"

namespace android {


/*==============================================================================
 *                     Constant
 *============================================================================*/

static const uint32_t kReadBufferSize = 0x2000; // 8k


/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderVoice *AudioALSACaptureDataProviderVoice::mAudioALSACaptureDataProviderVoice = NULL;
AudioALSACaptureDataProviderVoice *AudioALSACaptureDataProviderVoice::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderVoice == NULL) {
        mAudioALSACaptureDataProviderVoice = new AudioALSACaptureDataProviderVoice();
    }
    ASSERT(mAudioALSACaptureDataProviderVoice != NULL);
    return mAudioALSACaptureDataProviderVoice;
}

AudioALSACaptureDataProviderVoice::AudioALSACaptureDataProviderVoice() {
    ALOGD("%s()", __FUNCTION__);
    mCaptureDataProviderType = CAPTURE_PROVIDER_FM_RADIO;
}

AudioALSACaptureDataProviderVoice::~AudioALSACaptureDataProviderVoice() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderVoice::open() {
    ALOGD("%s()", __FUNCTION__);

    ASSERT(mEnable == false);

    // config attribute (will used in client SRC/Enh/... later)
    SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();

    mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeSource.num_channels = pSpeechDriver->GetRecordChannelNumber();
    mStreamAttributeSource.audio_channel_mask = (mStreamAttributeSource.num_channels == 1) ? AUDIO_CHANNEL_IN_MONO : AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeSource.sample_rate = pSpeechDriver->GetRecordSampleRate();

    // Reset frames readed counter
    mStreamAttributeSource.Time_Info.total_frames_readed = 0;


    mEnable = true;

    OpenPCMDump(LOG_TAG);

    return SpeechDriverFactory::GetInstance()->GetSpeechDriver()->recordOn();
}

status_t AudioALSACaptureDataProviderVoice::close() {
    ALOGD("%s()", __FUNCTION__);

    mEnable = false;

    ClosePCMDump();
    return SpeechDriverFactory::GetInstance()->GetSpeechDriver()->recordOff();
}

status_t AudioALSACaptureDataProviderVoice::provideModemRecordDataToProvider(RingBuf modem_record_buf) {
    if (mEnable == false) {
        ALOGW("%s(), mEnable == false, return", __FUNCTION__);
        return NO_INIT;
    }

    mPcmReadBuf = modem_record_buf;
    ALOGV("%s(), pBufBase(%p), bufLen(%d), pRead(%p), pWrite(%p)",
          __FUNCTION__,
          mPcmReadBuf.pBufBase,
          mPcmReadBuf.bufLen,
          mPcmReadBuf.pRead,
          mPcmReadBuf.pWrite);


    provideCaptureDataToAllClients(mOpenIndex);

    return NO_ERROR;
}


} // end of namespace android

#include "AudioALSASpeechLoopbackController.h"

#include "AudioALSADriverUtility.h"
#include "AudioALSADeviceParser.h"
#include "AudioALSAHardwareResourceManager.h"
#if defined(MTK_AUDIO_KS)
#include "AudioALSADeviceConfigManager.h"
#include "AudioSmartPaController.h"
#endif

#include "SpeechDriverInterface.h"
#include "SpeechDriverFactory.h"
#include "SpeechVMRecorder.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSASpeechLoopbackController"

namespace android {

AudioALSASpeechLoopbackController *AudioALSASpeechLoopbackController::mSpeechLoopbackController = NULL;
AudioALSASpeechLoopbackController *AudioALSASpeechLoopbackController::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mSpeechLoopbackController == NULL) {
        mSpeechLoopbackController = new AudioALSASpeechLoopbackController();
    }
    ASSERT(mSpeechLoopbackController != NULL);
    return mSpeechLoopbackController;
}


AudioALSASpeechLoopbackController::AudioALSASpeechLoopbackController() :
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()),
    mSpeechDriverFactory(SpeechDriverFactory::GetInstance()),
    mPcmUL(NULL),
    mPcmDL(NULL),
    mUseBtCodec(false) {
    memset((void *)&mConfig, 0, sizeof(mConfig));
}


AudioALSASpeechLoopbackController::~AudioALSASpeechLoopbackController() {

}

status_t AudioALSASpeechLoopbackController::open(const audio_devices_t output_devices,
                                                 const audio_devices_t input_device,
                                                 loopback_t loopbackType) {
#if !defined(MTK_AUDIO_KS)
    UNUSED(loopbackType);
#endif
    ALOGD("+%s(), output_devices = 0x%x, input_device = 0x%x", __FUNCTION__, output_devices, input_device);
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    // get speech driver instance
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    // check BT device // TODO(Harvey): BT Loopback?
#if defined(SPH_SR32K)
    const uint32_t sample_rate = 32000;
#elif defined(SPH_SR48K)
    const uint32_t sample_rate = 48000;
#else
    const uint32_t sample_rate = 16000;
#endif
    ALOGD("%s(), sample_rate = %d", __FUNCTION__, sample_rate);


    //--- here to test pcm interface platform driver_attach
#ifdef MTK_VOICE_ULTRA
    if (output_devices == AUDIO_DEVICE_OUT_EARPIECE) {
        int PcmInIdx, PcmOutIdx, CardIndex;
        PcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceUltra);
        PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceUltra);
        CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceUltra);
        String8 mixer_ctl_name;

        if (mSpeechDriverFactory->GetActiveModemIndex() == MODEM_1) {
            mixer_ctl_name = "md1";
        } else {
            mixer_ctl_name = "md2";
        }

        ALOGD("%s(), select ultra %s, PcmOutIdx = %d PcmInIdx = %d", __FUNCTION__,
              mixer_ctl_name.string(), PcmOutIdx, PcmInIdx);
        if (mixer_ctl_set_enum_by_string(
                mixer_get_ctl_by_name(AudioALSADriverUtility::getInstance()->getMixer(), "Ultra_Modem_Select")
                , mixer_ctl_name.string())) {
            ALOGE("Error: Ultra_Modem_Select invalid value");
        }

        // ASSERT(mPcmIn == NULL && mPcmOut == NULL); // TODO:

        // use pcm out to set memif, ultrasound, downlink
        enum pcm_format memifFormat = PCM_FORMAT_S16_LE;    // PCM_FORMAT_S32_LE or PCM_FORMAT_S16_LE
        unsigned int ultraRate = 96000;  // 192000 or 96000
        unsigned int msPerPeriod = 10;  // note: max sram is 48k for mt6797

        memset(&mConfig, 0, sizeof(mConfig));
        mConfig.channels = 1;
        mConfig.rate = ultraRate;
        mConfig.period_size = (ultraRate * msPerPeriod) / 1000;
        mConfig.period_count = 2;
        mConfig.format = memifFormat;

        mPcmDL = pcm_open(CardIndex, PcmOutIdx, PCM_OUT, &mConfig);

        // use pcm in to set modem, uplink
        memset(&mConfig, 0, sizeof(mConfig));
        mConfig.channels = 2;
        mConfig.rate = sample_rate;  // modem rate
        mConfig.period_size = 1024;
        mConfig.period_count = 2;
        mConfig.format = PCM_FORMAT_S16_LE;

        mPcmUL = pcm_open(CardIndex, PcmInIdx, PCM_IN, &mConfig);
    } else
#endif
    {
        int pcmInIdx, pcmOutIdx, cardIndex;

        memset(&mConfig, 0, sizeof(mConfig));
        mConfig.channels = 2;
        mConfig.rate = sample_rate;
        mConfig.period_size = 1024;
        mConfig.period_count = 2;
        mConfig.format = PCM_FORMAT_S16_LE;
        mConfig.start_threshold = 0;
        mConfig.stop_threshold = 0;
        mConfig.silence_threshold = 0;

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
        mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev1);
        mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev2);

        pcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpeech);
        pcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpeech);
        cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmHostlessSpeech);

        mHardwareResourceManager->startInputDevice(input_device);
#else
        if (mSpeechDriverFactory->GetActiveModemIndex() == MODEM_1) {
            pcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD1);
            pcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD1);
            cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceMD1);
        } else {
            pcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD2);
            pcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD2);
            cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceMD2);
        }
#endif

        ASSERT(mPcmUL == NULL && mPcmDL == NULL);
        mPcmUL = pcm_open(cardIndex, pcmInIdx, PCM_IN, &mConfig);
        mPcmDL = pcm_open(cardIndex, pcmOutIdx, PCM_OUT, &mConfig);
        ASSERT(mPcmUL != NULL && mPcmDL != NULL);
        ALOGV("%s(), mPcmUL = %p, mPcmDL = %p", __FUNCTION__, mPcmUL, mPcmDL);
    }
    if (pcm_start(mPcmUL)) {
        ALOGE("%s(), pcm_start UL %p fail due to %s", __FUNCTION__, mPcmUL, pcm_get_error(mPcmUL));
    }

    if (pcm_start(mPcmDL)) {
        ALOGE("%s(), pcm_start DL(%p) fail due to %s", __FUNCTION__, mPcmDL, pcm_get_error(mPcmDL));
    }

#if !defined(MTK_AUDIO_KS)
    // Set PMIC digital/analog part - uplink has pop, open first
    mHardwareResourceManager->startInputDevice(input_device);
#endif

    // Clean Side Tone Filter gain
    pSpeechDriver->SetSidetoneGain(0);

    // Set MD side sampling rate
    pSpeechDriver->SetModemSideSamplingRate(sample_rate);
    pSpeechDriver->setLpbkFlag(true);

    // Set speech mode
    pSpeechDriver->SetSpeechMode(input_device, output_devices);

    // Loopback on
    pSpeechDriver->SetAcousticLoopback(true);

    mHardwareResourceManager->startOutputDevice(output_devices, sample_rate);

    // check VM need open
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();
    if (pSpeechVMRecorder->getVmConfig() == SPEECH_VM_SPEECH) {
        ALOGD("%s(), Open VM/EPL record", __FUNCTION__);
        pSpeechVMRecorder->open();
    }

    ALOGD("-%s(), output_devices = 0x%x, input_device = 0x%x", __FUNCTION__, output_devices, input_device);
    return NO_ERROR;
}


status_t AudioALSASpeechLoopbackController::close() {
    ALOGD("+%s()", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    // check VM need close
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();
    if (pSpeechVMRecorder->getVMRecordStatus() == true) {
        ALOGD("%s(), Close VM/EPL record", __FUNCTION__);
        pSpeechVMRecorder->close();
    }

    mHardwareResourceManager->stopOutputDevice();
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev1);
    mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev2);

    // Stop MODEM_PCM
    pcm_stop(mPcmDL);
    pcm_stop(mPcmUL);
    pcm_close(mPcmDL);
    pcm_close(mPcmUL);

    mPcmDL = NULL;
    mPcmUL = NULL;

    mHardwareResourceManager->stopInputDevice(mHardwareResourceManager->getInputDevice());

#if defined(MTK_AUDIO_KS)
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence);
    mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence2);
#endif

    // Get current active speech driver
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    // Loopback off
    pSpeechDriver->SetAcousticLoopback(false);
    pSpeechDriver->setLpbkFlag(false);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSASpeechLoopbackController::SetModemBTCodec(bool enable_codec) {
    ALOGD("+%s(), enable_codec = %d", __FUNCTION__, enable_codec);
    mUseBtCodec = enable_codec;
    return NO_ERROR;
}

status_t AudioALSASpeechLoopbackController::OpenModemLoopbackControlFlow(const audio_devices_t input_device, const audio_devices_t output_device) {
    ALOGD("+%s(), output_device = 0x%x, input_device = 0x%x", __FUNCTION__, output_device, input_device);
    AL_AUTOLOCK(mLock);

    // get speech driver instance
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    // check BT device // TODO(Harvey): BT Loopback?
    const bool bt_device_on = audio_is_bluetooth_sco_device(output_device);
#if defined(SPH_SR32K)
    const int  sample_rate  = (bt_device_on == true) ? 8000 : 32000;
#elif defined(SPH_SR48K)
    const int  sample_rate  = (bt_device_on == true) ? 8000 : 48000;
#else
    const int  sample_rate  = (bt_device_on == true) ? 8000 : 16000;
#endif
    ALOGD("%s(), sample_rate = %d", __FUNCTION__, sample_rate);

    // Clean Side Tone Filter gain
    pSpeechDriver->SetSidetoneGain(0);

    // Set MD side sampling rate
    pSpeechDriver->SetModemSideSamplingRate(sample_rate);

    // Set speech mode
    pSpeechDriver->SetSpeechMode(input_device, output_device);

    // BT Loopback on
    pSpeechDriver->SetAcousticLoopbackBtCodec(mUseBtCodec);
    pSpeechDriver->SetAcousticLoopback(true);

    ALOGD("-%s(), output_devices = 0x%x, input_device = 0x%x", __FUNCTION__, output_device, input_device);
    return NO_ERROR;
}


status_t AudioALSASpeechLoopbackController::CloseModemLoopbackControlFlow(void) {
    AL_AUTOLOCK(mLock);
    ALOGD("+%s()", __FUNCTION__);

    // Get current active speech driver
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    // Loopback off
    pSpeechDriver->SetAcousticLoopback(false);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


} // end of namespace android

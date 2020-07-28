#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSASpeechPhoneCallController"
#include <AudioALSASpeechPhoneCallController.h>

#include <cutils/properties.h>

#include <pthread.h>
#include <utils/threads.h>

#include <hardware_legacy/power.h>

#include <SpeechUtility.h>

#include <AudioALSAHardwareResourceManager.h>
#include <AudioALSAStreamManager.h>

#include <AudioVolumeFactory.h>
#include <SpeechDriverFactory.h>

#include <SpeechEnhancementController.h>
#include <SpeechPcm2way.h>
#include <SpeechPcmMixerBGSPlayer.h>
#include <SpeechVMRecorder.h>
#include <WCNChipController.h>
#include <AudioALSADriverUtility.h>
#include <AudioALSADeviceParser.h>

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
#include <SpeechConfig.h>
#include <AudioALSAParamTuner.h>
#if !defined(MTK_COMBO_MODEM_SUPPORT)
#include <SpeechParamParser.h>
#endif
#endif

#if defined(MTK_AUDIO_KS)
#include <AudioALSADeviceConfigManager.h>
#include <AudioSmartPaController.h>
#endif

#if defined(MTK_AURISYS_PHONE_CALL_SUPPORT)
#include <audio_task.h>
#endif
#if defined(SPH_VCL_SUPPORT)
#include <SpeechVoiceCustomLogger.h>
#endif
#if defined(MTK_SPEECH_ENCRYPTION_SUPPORT)
#include <SpeechDataEncrypter.h>
#endif

#if defined(MTK_USB_PHONECALL)
#include <AudioUSBPhoneCallController.h>
#endif

#if defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
#include <AudioSCPPhoneCallController.h>
#endif

#if defined(MTK_AUDIODSP_SUPPORT)
#include <audio_dsp_service.h>
#include <AudioMessengerIPI.h>
#endif
// refer to /alps/vendor/mediatek/proprietary/hardware/ril/libratconfig/ratconfig.c
#define CDMA "C"


namespace android {

/*==============================================================================
 *                     Property keys
 *============================================================================*/
static const char *PROPERTY_KEY_MIC_MUTE_ON = "vendor.audiohal.recovery.mic_mute_on";
static const char *PROPERTY_KEY_DL_MUTE_ON = "vendor.audiohal.recovery.dl_mute_on";
static const char *PROPERTY_KEY_UL_MUTE_ON = "vendor.audiohal.recovery.ul_mute_on";
static const char *PROPERTY_KEY_PHONE1_MD = "vendor.audiohal.recovery.phone1.md";
static const char *PROPERTY_KEY_PHONE2_MD = "vendor.audiohal.recovery.phone2.md";
static const char *PROPERTY_KEY_FOREGROUND_PHONE_ID = "vendor.audiohal.recovery.phone_id";

static const char WAKELOCK_NAME[] = "EXT_MD_AUDIO_WAKELOCK";


#define DEFAULT_WAIT_SHUTTER_SOUND_UNMUTE_MS (1000) /* 1 sec */
#define DEFAULT_WAIT_ROUTING_UNMUTE_MS (150) /* 150ms */
#define DEFAULT_WAIT_PMIC_RESET_MS (1000) /* 1 sec */

enum {
    SPH_MUTE_THREAD_STATE_IDLE,
    SPH_MUTE_THREAD_STATE_WAIT
};

enum {
    SPH_MUTE_CTRL_IDLE,
    SPH_MUTE_CTRL_ROUTING_START,
    SPH_MUTE_CTRL_ROUTING_END,
    SPH_MUTE_CTRL_VOLUME_UPDATE
};

enum {
    RTT_CALL_TYPE_CS = 0,
    RTT_CALL_TYPE_RTT = 1,
    RTT_CALL_TYPE_PS = 2,
    RTT_CALL_TYPE_CS_NO_TTY = 3
};

enum {
    TTY_SWITCH_SKIP = 0,
    TTY_SWITCH_BY_MODE = 1,
    TTY_SWITCH_FORCE_OFF = 2
};

enum {
    AUD_RTT_OFF = 0,
    AUD_RTT_ON = 1
};

static struct mixer *mMixer;

#if !defined (MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
static void setTtyEnhancementMask(TtyModeType ttMode, const audio_devices_t outputDevice);
#endif
/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

AudioALSASpeechPhoneCallController *AudioALSASpeechPhoneCallController::mSpeechPhoneCallController = NULL;
AudioALSASpeechPhoneCallController *AudioALSASpeechPhoneCallController::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mSpeechPhoneCallController == NULL) {
        mSpeechPhoneCallController = new AudioALSASpeechPhoneCallController();
    }
    ASSERT(mSpeechPhoneCallController != NULL);
    return mSpeechPhoneCallController;
}



/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

AudioALSASpeechPhoneCallController::AudioALSASpeechPhoneCallController() :
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()),
    mStreamManager(NULL),
    mAudioALSAVolumeController(AudioVolumeFactory::CreateAudioVolumeController()),
    mSpeechDriverFactory(SpeechDriverFactory::GetInstance()),
    mAudioBTCVSDControl(NULL),
    mAudioMode(AUDIO_MODE_NORMAL),
    mMicMute(false),
    mVtNeedOn(false),
    bAudioTaste(false),
    mTtyMode(AUD_TTY_OFF),
    mInputDevice(AUDIO_DEVICE_NONE),
    mOutputDevice(AUDIO_DEVICE_NONE),
    mAdjustedInDev(AUDIO_DEVICE_NONE),
    mAdjustedOutDev(AUDIO_DEVICE_NONE),
    mIsBtSpkDevice(false),
    mBTMode(0),
    mIdxMD(MODEM_1),
    mPcmIn(NULL),
    mPcmOut(NULL),
#if defined(MTK_AUDIODSP_SUPPORT)
    mPcmDlIn(NULL),
    mPcmDlOut(NULL),
    mPcmIv(NULL),
    mPcmDsp(NULL),
#endif
    mRfInfo(0),
    mRfMode(0),
    mASRCNeedOn(0),
    mSpeechDVT_SampleRate(0),
    mSpeechDVT_MD_IDX(0),
    mIsSidetoneEnable(false),
    mCallStatus(false),
    hPmicResetThread(0),
    mIsPmicResetThreadEnable(false),
    hMuteDlCodecForShutterSoundThread(0),
    mMuteDlCodecForShutterSoundThreadEnable(false),
    mMuteDlCodecForShutterSoundCount(0),
    mIsMuteDlCodec(false),
    mMuteDlUlForRoutingThread(0),
    mMuteDlUlForRoutingThreadEnable(false),
    mMuteDlUlForRoutingState(SPH_MUTE_THREAD_STATE_IDLE),
    mMuteDlUlForRoutingCtrl(SPH_MUTE_CTRL_IDLE),
    mRttCallType(RTT_CALL_TYPE_CS),
    mRttMode(AUD_RTT_OFF),
    mSuperVolumeEnable(false) {

    mLogEnable = __android_log_is_loggable(ANDROID_LOG_DEBUG, LOG_TAG, ANDROID_LOG_INFO);
    // check need mute mic or not after kill mediaserver
    mMicMute = get_uint32_from_mixctrl(PROPERTY_KEY_MIC_MUTE_ON);

    // Need Mute DL Voice
    mDlMute = get_uint32_from_mixctrl(PROPERTY_KEY_DL_MUTE_ON);

    // Need Mute UL Voice
    mUlMute = get_uint32_from_mixctrl(PROPERTY_KEY_UL_MUTE_ON);

    //"0": default 0
    mPhoneId = (phone_id_t)get_uint32_from_mixctrl(PROPERTY_KEY_FOREGROUND_PHONE_ID);

    //"0": default MD1
    mIdxMDByPhoneId[0] = (get_uint32_from_mixctrl(PROPERTY_KEY_PHONE1_MD) == 0) ? MODEM_1 : MODEM_EXTERNAL;

    //"0": default MD1
    mIdxMDByPhoneId[1] = (get_uint32_from_mixctrl(PROPERTY_KEY_PHONE2_MD) == 0) ? MODEM_1 : MODEM_EXTERNAL;

    mMixer = AudioALSADriverUtility::getInstance()->getMixer();
    ASSERT(mMixer != NULL);
    // initialize mConfig
    memset((void *)&mConfig, 0, sizeof(pcm_config));

}

AudioALSASpeechPhoneCallController::~AudioALSASpeechPhoneCallController() {

}

bool AudioALSASpeechPhoneCallController::isTtyDevice(audio_devices_t outputDevice) const {
    return !(audio_is_bluetooth_sco_device(outputDevice) ||
             outputDevice == AUDIO_DEVICE_OUT_USB_DEVICE ||
             outputDevice == AUDIO_DEVICE_OUT_USB_HEADSET ||
             outputDevice == AUDIO_DEVICE_OUT_BUS);
}

bool AudioALSASpeechPhoneCallController::checkTtyNeedOn() const {
    return (mTtyMode != AUD_TTY_OFF && mVtNeedOn == false && mTtyMode != AUD_TTY_ERR &&
            mRttCallType == RTT_CALL_TYPE_CS &&
            isTtyDevice(mOutputDevice));
}

bool AudioALSASpeechPhoneCallController::checkSideToneFilterNeedOn(const audio_devices_t output_device) const {
    // TTY do not use STMF. Open only for earphone & receiver when side tone != 0.
    return ((checkTtyNeedOn() == false) &&
            //disable the condition, turn on sidetone without check the gain value
            //            (mAudioALSAVolumeController->GetSideToneGain(output_device) != 0) &&
            (output_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
             output_device == AUDIO_DEVICE_OUT_WIRED_HEADSET ||
             output_device == AUDIO_DEVICE_OUT_EARPIECE));
}

status_t AudioALSASpeechPhoneCallController::init() {
    return NO_ERROR;
}


audio_devices_t AudioALSASpeechPhoneCallController::getInputDeviceForPhoneCall(const audio_devices_t output_devices) {
    audio_devices_t input_device;

    switch (output_devices) {
    case AUDIO_DEVICE_OUT_WIRED_HEADSET: {
        if (mTtyMode == AUD_TTY_VCO) {
            input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
            ALOGD("%s(), headset, TTY_VCO, input_device(0x%x)", __FUNCTION__, input_device);
        } else {
            input_device = AUDIO_DEVICE_IN_WIRED_HEADSET;
        }
        break;
    }
    case AUDIO_DEVICE_OUT_EARPIECE:
    case AUDIO_DEVICE_OUT_WIRED_HEADPHONE: {
        input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        break;
    }
    case AUDIO_DEVICE_OUT_SPEAKER: {
        if (mTtyMode == AUD_TTY_HCO || mTtyMode == AUD_TTY_FULL) {
            input_device = AUDIO_DEVICE_IN_WIRED_HEADSET;
            ALOGD("%s(), speaker, mTtyMode(%d), input_device(0x%x)", __FUNCTION__, mTtyMode,  input_device);
        } else {
            if (USE_REFMIC_IN_LOUDSPK == 1) {
                input_device = AUDIO_DEVICE_IN_BACK_MIC;
            } else {
                input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
            }
        }
        break;
    }
    case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
    case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
    case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT: {
        input_device = AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET;
        break;
    }
#if defined(MTK_USB_PHONECALL)
    case AUDIO_DEVICE_OUT_USB_DEVICE: {
        input_device = AudioUSBPhoneCallController::getInstance()->getUSBCallInDevice();
        break;
    }
#endif
    default: {
        ALOGW("%s(), no support such output_devices(0x%x), "
              "default use AUDIO_DEVICE_IN_BUILTIN_MIC(0x%x) as input_device",
              __FUNCTION__, output_devices, AUDIO_DEVICE_IN_BUILTIN_MIC);
        input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        break;
    }
    }

    return input_device;
}
bool AudioALSASpeechPhoneCallController::getCallStatus() {
    AL_AUTOLOCK(mLock);
    return mCallStatus;
}

status_t AudioALSASpeechPhoneCallController::open(const audio_mode_t audio_mode,
                                                  const audio_devices_t output_devices,
                                                  const audio_devices_t input_device) {
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());
    AL_AUTOLOCK(mCheckOpenLock);

    ALOGD("%s(+), mAudioMode: %d => %d, mCallStatus: %d, output_devices: 0x%x, input_device: 0x%x",
          __FUNCTION__, mAudioMode, audio_mode, mCallStatus, output_devices, input_device);

    int PcmInIdx = 0;
    int PcmOutIdx = 0;
#if defined(MTK_AUDIODSP_SUPPORT)
    int PcmDlInIdx = 0;
    int PcmDlOutIdx = 0;
    int PcmIvIdx = 0;
    int PcmDspIdx = 0;
#endif
    int CardIndex = 0;
    uint32_t sample_rate = 0;

    mLogEnable = __android_log_is_loggable(ANDROID_LOG_DEBUG, LOG_TAG, ANDROID_LOG_INFO);

    mAudioMode = audio_mode;
    // set speech driver instance
    if (bAudioTaste) {
        mIdxMD = MODEM_1;
    } else {
        modem_index_t modem_index_property = updatePhysicalModemIdx(audio_mode);
        mIdxMD = modem_index_property;
    }

    hMuteDlCodecForShutterSoundThread = 0;
    mMuteDlCodecForShutterSoundCount = 0;
    mMuteDlCodecForShutterSoundThreadEnable = true;
    mIsMuteDlCodec = false;
    int ret = pthread_create(&hMuteDlCodecForShutterSoundThread, NULL,
                             AudioALSASpeechPhoneCallController::muteDlCodecForShutterSoundThread,
                             (void *)this);
    ASSERT(ret == 0);

    mMuteDlUlForRoutingThread = 0;
    mMuteDlUlForRoutingThreadEnable = true;
    mMuteDlUlForRoutingState = SPH_MUTE_THREAD_STATE_IDLE;
    ret = pthread_create(&mMuteDlUlForRoutingThread, NULL,
                         AudioALSASpeechPhoneCallController::muteDlUlForRoutingThread,
                         (void *)this);
    ASSERT(ret == 0);

    mSpeechDriverFactory->SetActiveModemIndex(mIdxMD);
    char isC2kSupported[PROPERTY_VALUE_MAX];
    property_get("ro.vendor.mtk_ps1_rat", isC2kSupported, "0"); //"0": default not support

    if (strstr(isC2kSupported, CDMA)  ==  NULL) {
        // wake lock for external modem
        if (mIdxMD == MODEM_EXTERNAL) {
            int ret = acquire_wake_lock(PARTIAL_WAKE_LOCK, WAKELOCK_NAME);
        }
    }
#if defined(MTK_AURISYS_PHONE_CALL_SUPPORT)
    /* Load task scene when opening */
    AudioMessengerIPI::getInstance()->loadTaskScene(TASK_SCENE_PHONE_CALL);
#endif
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
    mOutputDevice = output_devices;
    mInputDevice = input_device;
    if (checkTtyNeedOn() == true) {
        adjustTtyInOutDevice();
    } else {
        mAdjustedOutDev = mOutputDevice;
        mAdjustedInDev = mInputDevice;
    }

    // check BT device
    const bool bt_device_on = audio_is_bluetooth_sco_device(mAdjustedOutDev);
    if (mSpeechDVT_SampleRate != 0) {
        sample_rate = mSpeechDVT_SampleRate;
        ALOGD("%s(), SpeechDVT sample_rate = %d", __FUNCTION__, sample_rate);
    } else {
        sample_rate = calculateSampleRate(bt_device_on);
    }


    //--- here to test pcm interface platform driver_attach
    if (bt_device_on) {
        if (WCNChipController::GetInstance()->IsBTMergeInterfaceSupported() == true) {
            memset(&mConfig, 0, sizeof(mConfig));
            mConfig.channels = 1;
            mConfig.rate = sample_rate;
            mConfig.period_size = 4096;
            mConfig.period_count = 2;
            mConfig.format = PCM_FORMAT_S16_LE;
            mConfig.start_threshold = 0;
            mConfig.stop_threshold = 0;
            mConfig.silence_threshold = 0;
            ALOGE_IF(mPcmOut != NULL, "%s(), mPcmOut = %p", __FUNCTION__, mPcmOut);
            ASSERT(mPcmOut == NULL);

            if (mIdxMD == MODEM_EXTERNAL || mIdxMD == MODEM_2) {
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD2BT);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceMD2BT);
                mPcmOut = pcm_open(CardIndex, PcmOutIdx, PCM_OUT, &mConfig);
            } else {
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD1BT);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceMD1BT);
                mPcmOut = pcm_open(CardIndex, PcmOutIdx, PCM_OUT, &mConfig);
            }

            ALOGD_IF(mPcmOut == NULL, "%s(), mPcmOut = %p, PcmOutIdx = %d, CardIndex = %d",
                     __FUNCTION__, mPcmOut, PcmOutIdx, CardIndex);
            ASSERT(mPcmOut != NULL);
            pcm_start(mPcmOut);
        }
#if defined(MTK_USB_PHONECALL)
    } else if (AudioUSBPhoneCallController::getInstance()->isForceUSBCall() ||
               mAdjustedOutDev == AUDIO_DEVICE_OUT_USB_DEVICE) {
        AudioUSBPhoneCallController::getInstance()->enable(sample_rate);
#endif
#if defined(MTK_AUDIO_KS) && defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
    } else if (AudioSCPPhoneCallController::getInstance()->isSupportPhonecall(mAdjustedOutDev)) {
        AudioSCPPhoneCallController::getInstance()->enable(sample_rate, mAdjustedInDev);
#endif
    } else {
        ALOGE_IF(mPcmIn != NULL, "%s(), mPcmIn = %p", __FUNCTION__, mPcmIn);
        ALOGE_IF(mPcmOut != NULL, "%s(), mPcmOut = %p", __FUNCTION__, mPcmOut);
        ASSERT(mPcmIn == NULL && mPcmOut == NULL);

#if defined(MTK_VOICE_ULTRA)
        if (mAdjustedOutDev == AUDIO_DEVICE_OUT_EARPIECE) {
            PcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceUltra);
            PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceUltra);
            CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceUltra);
            String8 mixer_ctl_name;
            if (mIdxMD == MODEM_EXTERNAL || mIdxMD == MODEM_2) {
                mixer_ctl_name = "md2";
            } else {
                mixer_ctl_name = "md1";
            }

            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Ultra_Modem_Select"),
                                             mixer_ctl_name.string())) {
                ALOGE("Error: Ultra_Modem_Select invalid value");
            }
            // use pcm out to set memif, ultrasound, downlink
            enum pcm_format memifFormat = PCM_FORMAT_S16_LE;    // or PCM_FORMAT_S32_LE
            unsigned int ultraRate = 96000;
            unsigned int msPerPeriod = 10;  // note: max sram is 48k

            memset(&mConfig, 0, sizeof(mConfig));
            mConfig.channels = 1;
            mConfig.rate = ultraRate;
            mConfig.period_size = (ultraRate * msPerPeriod) / 1000;
            mConfig.period_count = 2;
            mConfig.format = memifFormat;

            mPcmOut = pcm_open(CardIndex, PcmOutIdx, PCM_OUT, &mConfig);

            // use pcm in to set modem, uplink
            memset(&mConfig, 0, sizeof(mConfig));
            mConfig.channels = 2;
            mConfig.rate = sample_rate;
            mConfig.period_size = 1024;
            mConfig.period_count = 2;
            mConfig.format = PCM_FORMAT_S16_LE;
            mConfig.start_threshold = 0;
            mConfig.stop_threshold = 0;
            mConfig.silence_threshold = 0;
            mPcmIn = pcm_open(CardIndex, PcmInIdx, PCM_IN, &mConfig);
        } else
#endif
        {
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
#if defined(MTK_AUDIODSP_SUPPORT)
            if ((mAdjustedOutDev & AUDIO_DEVICE_OUT_SPEAKER) &&
                AudioSmartPaController::getInstance()->isSwDspSpkProtect(mAdjustedOutDev) &&
                !AudioSmartPaController::getInstance()->isBypassSwDspSpkProtect()) {
                if (mIdxMD == MODEM_EXTERNAL || mIdxMD == MODEM_2) {
                    mApTurnOnSequence = AUDIO_CTL_MD2_TO_CAPTURE6;
                } else {
                    mApTurnOnSequence = AUDIO_CTL_MD1_TO_CAPTURE6;
                }
                mApTurnOnSequenceIv = AudioSmartPaController::getInstance()->getI2sSequence(AUDIO_CTL_I2S_TO_CAPTURE4, true);
                AudioSmartPaController::getInstance()->setI2sInHD(true);
                AudioSmartPaController::getInstance()->setI2sOutHD(true);
                mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequenceIv);
            } else
#endif
            {
                mApTurnOnSequence = mHardwareResourceManager->getOutputTurnOnSeq(mAdjustedOutDev, false,
                                                                                 mIdxMD == MODEM_1 ? AUDIO_CTL_MD1 : AUDIO_CTL_MD2);
            }

            if ((mAdjustedOutDev & AUDIO_DEVICE_OUT_SPEAKER) &&
                AudioSmartPaController::getInstance()->isSmartPAUsed() &&
                popcount(mAdjustedOutDev) > 1) {
                mApTurnOnSequence2 = mHardwareResourceManager->getOutputTurnOnSeq(mAdjustedOutDev, true,
                                                                                  mIdxMD == MODEM_1 ? AUDIO_CTL_MD1 : AUDIO_CTL_MD2);
            }
            mHardwareResourceManager->setCustOutputDevTurnOnSeq(mAdjustedOutDev, mTurnOnSeqCustDev1, mTurnOnSeqCustDev2);

            mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence);
            mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence2);
            mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev1);
            mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev2);
#if defined(MTK_AUDIODSP_SUPPORT)
            if (AudioSmartPaController::getInstance()->isSwDspSpkProtect(mAdjustedOutDev) &&
                !AudioSmartPaController::getInstance()->isBypassSwDspSpkProtect()) {
                PcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpeech);
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpeech);
                PcmDlOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback4);
                PcmDlInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture6);
                PcmIvIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture4);
                PcmDspIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCallfinalDsp);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback4);
                mConfig.period_size = 256;
                mConfig.period_count = 4;
                mConfig.start_threshold = mConfig.period_size * mConfig.period_count;
                mConfig.format = PCM_FORMAT_S24_LE;
                if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "dsp_call_final_runtime_en"), 0, 1)) {
                    ALOGE("%s(), Error: enable dsp_call_final_runtime_en fail \n", __FUNCTION__);
                }
                if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_call_final_sharemem_scenario"), 0, 1)) {
                    ALOGW("%s(), set sharemem fail", __FUNCTION__);
                }

                AudioMessengerIPI::getInstance()->registerAdspFeature(CALL_FINAL_FEATURE_ID);
            } else
#endif
            {
                PcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpeech);
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpeech);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmHostlessSpeech);
            }
#else
            if (mIdxMD == MODEM_EXTERNAL || mIdxMD == MODEM_2) {
                PcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD2);
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD2);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceMD2);
            } else {
                PcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD1);
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD1);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceMD1);
            }
#endif
            mPcmIn = pcm_open(CardIndex, PcmInIdx, PCM_IN, &mConfig);
            mConfig.channels = 2;
            mPcmOut = pcm_open(CardIndex, PcmOutIdx, PCM_OUT, &mConfig);
#if defined(MTK_AUDIODSP_SUPPORT)
            if (PcmDlOutIdx && PcmDlInIdx && PcmIvIdx && PcmDspIdx) {
                mPcmDlOut = pcm_open(CardIndex, PcmDlOutIdx, PCM_OUT, &mConfig);
                if (pcm_prepare(mPcmDlOut) != 0) {
                    ALOGE("mPcmDlOut prepare fail, err:%s", pcm_get_error(mPcmDlOut));
                }
                mPcmDlIn = pcm_open(CardIndex, PcmDlInIdx, PCM_IN, &mConfig);
                if (pcm_prepare(mPcmDlIn) != 0) {
                    ALOGE("mPcmDlIn prepare fail, err:%s", pcm_get_error(mPcmDlIn));
                }
                mPcmIv = pcm_open(CardIndex, PcmIvIdx, PCM_IN, &mConfig);
                if (pcm_prepare(mPcmIv) != 0) {
                    ALOGE("mPcmIv prepare fail, err:%s", pcm_get_error(mPcmIv));
                }
                mPcmDsp = pcm_open(CardIndex, PcmDspIdx, PCM_OUT, &mConfig);
                if (pcm_prepare(mPcmDsp) != 0) {
                    ALOGE("mPcmDsp prepare fail, err:%s", pcm_get_error(mPcmDsp));
                }
            }
#endif
        }
        ALOGD_IF(mPcmIn == NULL, "%s(), mPcmIn = %p, PcmInIdx = %d, CardIndex = %d, err: %s",
                 __FUNCTION__, mPcmIn, PcmInIdx, CardIndex, pcm_get_error(mPcmIn));
        ALOGD_IF(mPcmOut == NULL, "%s(), mPcmOut = %p, PcmOutIdx = %d, CardIndex = %d",
                 __FUNCTION__, mPcmOut, PcmOutIdx, CardIndex);
        ASSERT(mPcmIn != NULL && mPcmOut != NULL);

#if !defined(MTK_AUDIO_KS)
        pcm_start(mPcmIn);
#endif
        pcm_start(mPcmOut);

#if !defined(MTK_AUDIO_KS) && defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
        if (AudioSCPPhoneCallController::getInstance()->isSupportPhonecall(mAdjustedOutDev)) {
            AudioSCPPhoneCallController::getInstance()->enable(sample_rate, mAdjustedInDev);
        }
#endif
    }

#if !defined(MTK_AUDIO_KS)
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Speech_MD_USAGE"), "On")) {
        ALOGE("Error: Speech_MD_USAGE invalid value");
    }
#endif
    if (checkTtyNeedOn() == true) {
        setTtyInOutDevice();
    } else {

        // Set PMIC digital/analog part - uplink has pop, open first
#if defined(MTK_USB_PHONECALL)
        if (!AudioUSBPhoneCallController::getInstance()->isEnable()) {
            mHardwareResourceManager->startInputDevice(mAdjustedInDev);
        }
#else
        mHardwareResourceManager->startInputDevice(mAdjustedInDev);
#endif
    }

#if defined(MTK_AUDIO_KS)
    // after start input device
    if (mPcmIn) {
        pcm_start(mPcmIn);
    }
#endif

    // start Side Tone Filter
    if (checkSideToneFilterNeedOn(mAdjustedOutDev) == true) {
        mHardwareResourceManager->EnableSideToneFilter(true);
        mIsSidetoneEnable = true;
    }
#if defined(MTK_SPEECH_ENCRYPTION_SUPPORT)
    if (SpeechDataEncrypter::GetInstance()->GetEnableStatus()) {
        SpeechDataEncrypter::GetInstance()->Start();
    }
#endif

    // Set MD side sampling rate
    pSpeechDriver->SetModemSideSamplingRate(sample_rate);

    // Set speech mode
    if (checkTtyNeedOn() == false) {
#if defined(MTK_USB_PHONECALL)
        if (AudioUSBPhoneCallController::getInstance()->isForceUSBCall()) {
            pSpeechDriver->SetSpeechMode(mAdjustedInDev, AUDIO_DEVICE_OUT_USB_DEVICE);
        } else {
            pSpeechDriver->SetSpeechMode(mAdjustedInDev, mAdjustedOutDev);
        }

        if (!AudioUSBPhoneCallController::getInstance()->isEnable()) {
            mHardwareResourceManager->startOutputDevice(mAdjustedOutDev, sample_rate);
        }
#else
        pSpeechDriver->SetSpeechMode(mAdjustedInDev, mAdjustedOutDev);
        mHardwareResourceManager->startOutputDevice(mAdjustedOutDev, sample_rate);
#endif
    }

    // set mute info to speech driver
    setMuteInfo();

    // Speech/VT on
    if (mVtNeedOn == true) {
        pSpeechDriver->VideoTelephonyOn();

        // trun on P2W for Video Telephony
        bool wideband_on = false; // VT default use Narrow Band (8k), modem side will SRC to 16K
        pSpeechDriver->PCM2WayOn(wideband_on);
    } else {
        pSpeechDriver->SpeechOn();

        // turn on TTY
        if (checkTtyNeedOn() == true) {
            pSpeechDriver->TtyCtmOn();
#if !defined (MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
            setTtyEnhancementMask(mTtyMode, mAdjustedOutDev);
#endif
        }
    }
#if !defined(MTK_COMBO_MODEM_SUPPORT)
    // for the case that customized app set mute before speech on
    pSpeechDriver->SetUplinkMute(mMicMute);
    pSpeechDriver->SetDownlinkMute(mDlMute);
#endif

#if defined(MTK_AUDIODSP_SUPPORT)
    if (mPcmDlOut && mPcmDlIn && mPcmIv && mPcmDsp) {
        pcm_start(mPcmIv);
        pcm_start(mPcmDlIn);
        pcm_start(mPcmDlOut);
        pcm_start(mPcmDsp);
    }
#endif
#if defined(MTK_RTT_SUPPORT)
    pSpeechDriver->RttConfig(mRttMode);
#endif

#if defined(SPH_VCL_SUPPORT)
    // check VCL need open
    SpeechVoiceCustomLogger *pSpeechVoiceCustomLogger = SpeechVoiceCustomLogger::GetInstance();
    if (pSpeechVoiceCustomLogger->UpdateVCLSwitch() == true) {
        pSpeechVoiceCustomLogger->Open();
    }
#endif

#if defined(SPEECH_PMIC_RESET_ACC) || defined(SPEECH_PMIC_RESET_DCC)
    hPmicResetThread = 0;
    AL_LOCK(mPmicResetLock);
    mIsPmicResetThreadEnable = true;
    AL_UNLOCK(mPmicResetLock);
    ret = pthread_create(&hPmicResetThread, NULL,
                         AudioALSASpeechPhoneCallController::pmicResetThread,
                         (void *)this);
    ASSERT(ret == 0);
#endif

    // check VM need open
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();
    if (pSpeechVMRecorder->getVmConfig() == SPEECH_VM_SPEECH) {
        pSpeechVMRecorder->open();
    }

    mCallStatus = true;

    ALOGD("%s(-), mAudioMode: %d, mIdxMD: %d, bt_device_on: %d, sample_rate: %u"
          ", isC2kSupported: %s, mCallStatus: %d"
          ", CardIndex: %d, PcmInIdx: %d, PcmOutIdx: %d, mPcmIn: %p, mPcmOut: %p",
          __FUNCTION__, mAudioMode, mIdxMD, bt_device_on, sample_rate,
          isC2kSupported, mCallStatus,
          CardIndex, PcmInIdx, PcmOutIdx, mPcmIn, mPcmOut);

    return NO_ERROR;
}


status_t AudioALSASpeechPhoneCallController::close() {
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());
    ALOGD("%s(), mAudioMode: %d => 0, mCallStatus: %d", __FUNCTION__, mAudioMode, mCallStatus);

    const modem_index_t modem_index = mSpeechDriverFactory->GetActiveModemIndex();

    // Get current active speech driver
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
    pSpeechDriver->SetUplinkMute(true);
#if defined(MTK_SPEECH_ENCRYPTION_SUPPORT)
    if (SpeechDataEncrypter::GetInstance()->GetStartStatus()) {
        SpeechDataEncrypter::GetInstance()->Stop();
    }
#endif

    // check VM need close
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();
    if (pSpeechVMRecorder->getVMRecordStatus() == true) {
        pSpeechVMRecorder->close();
    }

#if defined(SPH_VCL_SUPPORT)
    // check VCL need open
    SpeechVoiceCustomLogger *pSpeechVoiceCustomLogger = SpeechVoiceCustomLogger::GetInstance();
    if (pSpeechVoiceCustomLogger->GetVCLRecordStatus() == true) {
        pSpeechVoiceCustomLogger->Close();
    }
#endif

    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values;

#if defined(MTK_USB_PHONECALL)
    if (!AudioUSBPhoneCallController::getInstance()->isEnable())
#endif
    {
        mHardwareResourceManager->stopOutputDevice();

        // Stop Side Tone Filter
        if (mIsSidetoneEnable) {
            mHardwareResourceManager->EnableSideToneFilter(false);
            mIsSidetoneEnable = false;
        }
    }
    // Stop MODEM_PCM
#if defined(MTK_AUDIODSP_SUPPORT)
    if (mPcmDsp != NULL) {
        pcm_stop(mPcmDsp);
        pcm_close(mPcmDsp);
        mPcmDsp = NULL;
    }
    if (mPcmDlOut != NULL) {
        pcm_stop(mPcmDlOut);
        pcm_close(mPcmDlOut);
        mPcmDlOut = NULL;
    }
    if (mPcmDlIn != NULL) {
        pcm_stop(mPcmDlIn);
        pcm_close(mPcmDlIn);
        mPcmDlIn = NULL;
    }
    if (mPcmIv != NULL) {
        pcm_stop(mPcmIv);
        pcm_close(mPcmIv);
        mPcmIv = NULL;
        mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequenceIv);
    }

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "dsp_call_final_runtime_en"), 0, 0)) {
        ALOGE("%s(), Error: enable dsp_call_final_runtime_en fail \n", __FUNCTION__);
    }
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_call_final_sharemem_scenario"), 0, 0)) {
        ALOGW("%s(), set sharemem fail", __FUNCTION__);
    }

    AudioMessengerIPI::getInstance()->deregisterAdspFeature(CALL_FINAL_FEATURE_ID);

#endif
    if (mPcmIn != NULL) {
        pcm_stop(mPcmIn);
        pcm_close(mPcmIn);
        mPcmIn = NULL;
    }

    if (mPcmOut != NULL) {
        mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev1);
        mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev2);

        pcm_stop(mPcmOut);
        pcm_close(mPcmOut);
        mPcmOut = NULL;

#if defined(MTK_AUDIO_KS)
        mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence);
        mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence2);
#endif
    }

#if defined(MTK_USB_PHONECALL)
    if (!AudioUSBPhoneCallController::getInstance()->isEnable()) {
        mHardwareResourceManager->stopInputDevice(mAdjustedInDev);
        mAdjustedInDev = AUDIO_DEVICE_NONE;
    } else {
        AudioUSBPhoneCallController::getInstance()->disable();
    }
#else
    mHardwareResourceManager->stopInputDevice(mAdjustedInDev);
    mAdjustedInDev = AUDIO_DEVICE_NONE;
#endif
    mAdjustedOutDev = AUDIO_DEVICE_NONE;

#if defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
    if (AudioSCPPhoneCallController::getInstance()->isEnable()) {
        AudioSCPPhoneCallController::getInstance()->disable();
    }
#endif
    // terminated pmic reset thread
    if (mIsPmicResetThreadEnable == true) {
        AL_LOCK(mPmicResetLock);
        mIsPmicResetThreadEnable = false;
        AL_SIGNAL(mPmicResetLock);
        AL_UNLOCK(mPmicResetLock);

        pthread_join(hPmicResetThread, NULL);
    }

    // terminated shutter sound thread
    if (mMuteDlCodecForShutterSoundThreadEnable == true) {
        AL_LOCK(mMuteDlCodecForShutterSoundLock);
        mMuteDlCodecForShutterSoundThreadEnable = false;
        AL_SIGNAL(mMuteDlCodecForShutterSoundLock);
        AL_UNLOCK(mMuteDlCodecForShutterSoundLock);

        pthread_join(hMuteDlCodecForShutterSoundThread, NULL);
    }

    // terminated mute for routing thread
    if (mMuteDlUlForRoutingThreadEnable == true) {
        AL_LOCK(mMuteDlUlForRoutingLock);
        mMuteDlUlForRoutingThreadEnable = false;
        AL_SIGNAL(mMuteDlUlForRoutingLock);
        AL_UNLOCK(mMuteDlUlForRoutingLock);

        pthread_join(mMuteDlUlForRoutingThread, NULL);
    }

#if defined(MTK_RTT_SUPPORT)
    pSpeechDriver->RttConfig(AUD_RTT_OFF);
#endif

    // Speech/VT off
    if (pSpeechDriver->GetApSideModemStatus(VT_STATUS_MASK) == true) {
        pSpeechDriver->PCM2WayOff();
        pSpeechDriver->VideoTelephonyOff();
    } else if (pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == true) {
        if (pSpeechDriver->GetApSideModemStatus(TTY_STATUS_MASK) == true) {
            pSpeechDriver->TtyCtmOff();

#if !defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
            pSpeechDriver->SetSpeechEnhancement(true);
            pSpeechDriver->SetSpeechEnhancementMask(SpeechEnhancementController::GetInstance()->GetSpeechEnhancementMask());
#endif
        }
        pSpeechDriver->SpeechOff();
    } else {
        ALOGE("%s(), mAudioMode = %d, Speech & VT are already closed!!", __FUNCTION__, mAudioMode);
        ASSERT(pSpeechDriver->GetApSideModemStatus(VT_STATUS_MASK)     == true ||
               pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == true);
    }

    // clean VT status
    if (mVtNeedOn == true) {
        ALOGD("%s(), Set mVtNeedOn = false", __FUNCTION__);
        mVtNeedOn = false;
    }
    pSpeechDriver->SetUplinkMute(mMicMute);

    char isC2kSupported[PROPERTY_VALUE_MAX];
    property_get("ro.vendor.mtk_ps1_rat", isC2kSupported, "0"); //"0": default not support

    if (strstr(isC2kSupported, CDMA)  ==  NULL) {
        // wake lock for external modem
        if (mIdxMD == MODEM_EXTERNAL) {
            int ret = release_wake_lock(WAKELOCK_NAME);
        }
    }

#if !defined(MTK_AUDIO_KS)
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Speech_MD_USAGE"), "Off")) {
        ALOGE("Error: Speech_MD_USAGE invalid value");
    }
#endif
    mAudioMode = AUDIO_MODE_NORMAL; // TODO(Harvey): default value? VoIP?
    mOutputDevice = mAdjustedOutDev;
    mInputDevice = mAdjustedInDev;
    mCallStatus = false;

    return NO_ERROR;
}


status_t AudioALSASpeechPhoneCallController::routing(const audio_devices_t new_output_devices,
                                                     const audio_devices_t new_input_device) {
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    ALOGD("%s(+), mAudioMode: %d, mCallStatus: %d, mIdxMD: %d, new_output_devices: 0x%x, new_input_device: 0x%x",
          __FUNCTION__, mAudioMode, mCallStatus, mIdxMD, new_output_devices, new_input_device);

    const modem_index_t modem_index = mSpeechDriverFactory->GetActiveModemIndex();

    // Get current active speech driver
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    int PcmInIdx = 0;
    int PcmOutIdx = 0;
#if defined(MTK_AUDIODSP_SUPPORT)
    int PcmDlInIdx = 0;
    int PcmDlOutIdx = 0;
    int PcmIvIdx = 0;
    int PcmDspIdx = 0;
#endif
    int CardIndex = 0;

    // Mute during device change.
    muteDlUlForRouting(SPH_MUTE_CTRL_ROUTING_START);

#if defined(MTK_USB_PHONECALL)
    if (!AudioUSBPhoneCallController::getInstance()->isEnable())
#endif
    {
        // Stop PMIC digital/analog part - downlink
        mHardwareResourceManager->stopOutputDevice();

        // Stop Side Tone Filter
        if (mIsSidetoneEnable) {
            mHardwareResourceManager->EnableSideToneFilter(false);
            mIsSidetoneEnable = false;
        }

        // Stop MODEM_PCM
        //mAudioDigitalInstance->SetModemPcmEnable(modem_index, false);

        // Stop PMIC digital/analog part - uplink
        mHardwareResourceManager->stopInputDevice(mAdjustedInDev);
        mAdjustedInDev = AUDIO_DEVICE_NONE;

    }

    // Stop AP side digital part
    if (pSpeechDriver->GetApSideModemStatus(TTY_STATUS_MASK) == true) {
        pSpeechDriver->TtyCtmOff();
    }

    // Get new device
    mOutputDevice = new_output_devices;
    mInputDevice = new_input_device;
    if (checkTtyNeedOn() == true) {
        adjustTtyInOutDevice();
    } else {
        mAdjustedOutDev = mOutputDevice;
        mAdjustedInDev = mInputDevice;
    }


    // Check BT device
    const bool bt_device_on = audio_is_bluetooth_sco_device(mAdjustedOutDev);
    uint32_t sample_rate = 0;
    if (mSpeechDVT_SampleRate != 0) {
        sample_rate = mSpeechDVT_SampleRate;
        ALOGD("%s(), SpeechDVT sample_rate = %d", __FUNCTION__, sample_rate);
    } else {
        sample_rate = calculateSampleRate(bt_device_on);
    }

    //close previous device
#if defined(MTK_AUDIODSP_SUPPORT)
    if (mPcmDsp != NULL) {
        pcm_stop(mPcmDsp);
        pcm_close(mPcmDsp);
        mPcmDsp = NULL;
    }
    if (mPcmDlOut != NULL) {
        pcm_stop(mPcmDlOut);
        pcm_close(mPcmDlOut);
        mPcmDlOut = NULL;
    }
    if (mPcmDlIn != NULL) {
        pcm_stop(mPcmDlIn);
        pcm_close(mPcmDlIn);
        mPcmDlIn = NULL;
    }
    if (mPcmIv != NULL) {
        pcm_stop(mPcmIv);
        pcm_close(mPcmIv);
        mPcmIv = NULL;
        mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequenceIv);
    }
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "dsp_call_final_runtime_en"), 0, 0)) {
        ALOGE("%s(), Error: enable dsp_call_final_runtime_en fail \n", __FUNCTION__);
    }
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_call_final_sharemem_scenario"), 0, 0)) {
        ALOGW("%s(), set sharemem fail", __FUNCTION__);
    }

    AudioMessengerIPI::getInstance()->deregisterAdspFeature(CALL_FINAL_FEATURE_ID);

#endif
    if (mPcmIn != NULL) {
        pcm_stop(mPcmIn);
        pcm_close(mPcmIn);
        mPcmIn = NULL;
    }

    if (mPcmOut != NULL) {
        mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev1);
        mHardwareResourceManager->disableTurnOnSequence(mTurnOnSeqCustDev2);

        pcm_stop(mPcmOut);
        pcm_close(mPcmOut);
        mPcmOut = NULL;

#if defined(MTK_AUDIO_KS)
        mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence);
        mHardwareResourceManager->disableTurnOnSequence(mApTurnOnSequence2);
#endif
    }

#if defined(MTK_USB_PHONECALL)
    if (AudioUSBPhoneCallController::getInstance()->isEnable()) {
        AudioUSBPhoneCallController::getInstance()->disable();
    }
#endif
#if defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
    if (AudioSCPPhoneCallController::getInstance()->isEnable()) {
        AudioSCPPhoneCallController::getInstance()->disable();
    }
#endif
    if (bt_device_on) {
        if (WCNChipController::GetInstance()->IsBTMergeInterfaceSupported() == true) {
            //open bt sco device
            memset(&mConfig, 0, sizeof(mConfig));

            mConfig.channels = 1;
            mConfig.rate = sample_rate;
            mConfig.period_size = 4096;
            mConfig.period_count = 2;
            mConfig.format = PCM_FORMAT_S16_LE;
            mConfig.start_threshold = 0;
            mConfig.stop_threshold = 0;
            mConfig.silence_threshold = 0;

            ALOGE_IF(mPcmOut != NULL, "%s(), mPcmOut = %p", __FUNCTION__, mPcmOut);
            ASSERT(mPcmOut == NULL);

            if (mIdxMD == MODEM_EXTERNAL || mIdxMD == MODEM_2) {
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD2BT);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceMD2BT);
                mPcmOut = pcm_open(CardIndex, PcmOutIdx, PCM_OUT, &mConfig);
            } else {
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD1BT);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceMD1BT);
                mPcmOut = pcm_open(CardIndex, PcmOutIdx, PCM_OUT, &mConfig);
            }
            ALOGD_IF(mPcmOut == NULL, "%s(), mPcmOut = %p, PcmOutIdx = %d, CardIndex = %d",
                     __FUNCTION__, mPcmOut, PcmOutIdx, CardIndex);
            ASSERT(mPcmOut != NULL);

            pcm_start(mPcmOut);
        }
#if defined(MTK_USB_PHONECALL)
    } else if (AudioUSBPhoneCallController::getInstance()->isForceUSBCall() ||
               mAdjustedOutDev == AUDIO_DEVICE_OUT_USB_DEVICE) {
        AudioUSBPhoneCallController::getInstance()->enable(sample_rate);
#endif
#if defined(MTK_AUDIO_KS) && defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
    } else if (AudioSCPPhoneCallController::getInstance()->isSupportPhonecall(new_output_devices)) {
        AudioSCPPhoneCallController::getInstance()->enable(sample_rate, new_input_device);
#endif
    } else {
#if defined(MTK_VOICE_ULTRA)
        if (mAdjustedOutDev == AUDIO_DEVICE_OUT_EARPIECE) {
            PcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceUltra);
            PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceUltra);
            CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceUltra);
            String8 mixer_ctl_name;
            if (mIdxMD == MODEM_EXTERNAL || mIdxMD == MODEM_2) {
                mixer_ctl_name = "md2";
            } else {
                mixer_ctl_name = "md1";
            }

            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Ultra_Modem_Select"),
                                             mixer_ctl_name.string())) {
                ALOGE("Error: Ultra_Modem_Select invalid value");
            }

            ALOGE_IF(mPcmIn != NULL, "%s(), mPcmIn = %p", __FUNCTION__, mPcmIn);
            ALOGE_IF(mPcmOut != NULL, "%s(), mPcmOut = %p", __FUNCTION__, mPcmOut);
            ASSERT(mPcmIn == NULL && mPcmOut == NULL);

            // use pcm out to set memif, ultrasound, downlink
            enum pcm_format memifFormat = PCM_FORMAT_S16_LE;    // or PCM_FORMAT_S32_LE
            unsigned int ultraRate = 96000;
            unsigned int msPerPeriod = 10;  // note: max sram is 48k

            memset(&mConfig, 0, sizeof(mConfig));
            mConfig.channels = 1;
            mConfig.rate = ultraRate;
            mConfig.period_size = (ultraRate * msPerPeriod) / 1000;
            mConfig.period_count = 2;
            mConfig.format = memifFormat;

            mPcmOut = pcm_open(CardIndex, PcmOutIdx, PCM_OUT, &mConfig);

            // use pcm in to set modem, uplink
            memset(&mConfig, 0, sizeof(mConfig));
            mConfig.channels = 2;
            mConfig.rate = sample_rate; // modem rate
            mConfig.period_size = 1024;
            mConfig.period_count = 2;
            mConfig.format = PCM_FORMAT_S16_LE;

            mPcmIn = pcm_open(CardIndex, PcmInIdx, PCM_IN, &mConfig);
        } else
#endif
        {
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
#if defined(MTK_AUDIODSP_SUPPORT)
            if ((mAdjustedOutDev & AUDIO_DEVICE_OUT_SPEAKER) &&
                AudioSmartPaController::getInstance()->isSwDspSpkProtect(mAdjustedOutDev) &&
                !AudioSmartPaController::getInstance()->isBypassSwDspSpkProtect()) {
                if (mIdxMD == MODEM_EXTERNAL || mIdxMD == MODEM_2) {
                    mApTurnOnSequence = AUDIO_CTL_MD2_TO_CAPTURE6;
                } else {
                    mApTurnOnSequence = AUDIO_CTL_MD1_TO_CAPTURE6;
                }
                mApTurnOnSequenceIv = AudioSmartPaController::getInstance()->getI2sSequence(AUDIO_CTL_I2S_TO_CAPTURE4, true);
                AudioSmartPaController::getInstance()->setI2sInHD(true);
                AudioSmartPaController::getInstance()->setI2sOutHD(true);

                mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequenceIv);
            } else
#endif
            {
                mApTurnOnSequence = mHardwareResourceManager->getOutputTurnOnSeq(mAdjustedOutDev, false,
                                                                                 mIdxMD == MODEM_1 ? AUDIO_CTL_MD1 : AUDIO_CTL_MD2);
            }

            if ((mAdjustedOutDev & AUDIO_DEVICE_OUT_SPEAKER) &&
                AudioSmartPaController::getInstance()->isSmartPAUsed() &&
                popcount(mAdjustedOutDev) > 1) {
                mApTurnOnSequence2 = mHardwareResourceManager->getOutputTurnOnSeq(mAdjustedOutDev, true,
                                                                                  mIdxMD == MODEM_1 ? AUDIO_CTL_MD1 : AUDIO_CTL_MD2);
            }
            mHardwareResourceManager->setCustOutputDevTurnOnSeq(mAdjustedOutDev, mTurnOnSeqCustDev1, mTurnOnSeqCustDev2);

            mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence);
            mHardwareResourceManager->enableTurnOnSequence(mApTurnOnSequence2);
            mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev1);
            mHardwareResourceManager->enableTurnOnSequence(mTurnOnSeqCustDev2);
#if defined(MTK_AUDIODSP_SUPPORT)
            if (AudioSmartPaController::getInstance()->isSwDspSpkProtect(mAdjustedOutDev) &&
                !AudioSmartPaController::getInstance()->isBypassSwDspSpkProtect()) {
                PcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpeech);
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpeech);
                PcmDlOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback4);
                PcmDlInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture6);
                PcmIvIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture4);
                PcmDspIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCallfinalDsp);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback4);
                mConfig.period_size = 256;
                mConfig.period_count = 4;
                mConfig.start_threshold = mConfig.period_size * mConfig.period_count;
                mConfig.format = PCM_FORMAT_S24_LE;
                if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "dsp_call_final_runtime_en"), 0, 1)) {
                    ALOGE("%s(), Error: enable dsp_call_final_runtime_en fail \n", __FUNCTION__);
                }
                if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_call_final_sharemem_scenario"), 0, 1)) {
                    ALOGW("%s(), set sharemem fail", __FUNCTION__);
                }

                AudioMessengerIPI::getInstance()->registerAdspFeature(CALL_FINAL_FEATURE_ID);
            } else
#endif
            {
                PcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpeech);
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpeech);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmHostlessSpeech);
            }
#else
            if (mIdxMD == MODEM_EXTERNAL || mIdxMD == MODEM_2) {
                PcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD2);
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD2);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceMD2);
            } else {
                PcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD1);
                PcmOutIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmVoiceMD1);
                CardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmVoiceMD1);
            }
#endif
            ALOGE_IF(mPcmIn != NULL, "%s(), mPcmIn = %p", __FUNCTION__, mPcmIn);
            ALOGE_IF(mPcmOut != NULL, "%s(), mPcmOut = %p", __FUNCTION__, mPcmOut);
            ASSERT(mPcmIn == NULL && mPcmOut == NULL);
            mPcmIn = pcm_open(CardIndex, PcmInIdx, PCM_IN, &mConfig);

            mConfig.channels = 2;
            mPcmOut = pcm_open(CardIndex, PcmOutIdx, PCM_OUT, &mConfig);
#if defined(MTK_AUDIODSP_SUPPORT)
            if (PcmDlOutIdx && PcmDlInIdx && PcmIvIdx && PcmDspIdx) {
                mPcmDlOut = pcm_open(CardIndex, PcmDlOutIdx, PCM_OUT, &mConfig);
                if (pcm_prepare(mPcmDlOut) != 0) {
                    ALOGE("mPcmDlOut prepare fail, err:%s", pcm_get_error(mPcmDlOut));
                }
                mPcmDlIn = pcm_open(CardIndex, PcmDlInIdx, PCM_IN, &mConfig);
                if (pcm_prepare(mPcmDlIn) != 0) {
                    ALOGE("mPcmDlIn prepare fail, err:%s", pcm_get_error(mPcmDlIn));
                }
                mPcmIv = pcm_open(CardIndex, PcmIvIdx, PCM_IN, &mConfig);
                if (pcm_prepare(mPcmIv) != 0) {
                    ALOGE("mPcmIv prepare fail, err:%s", pcm_get_error(mPcmIv));
                }
                mPcmDsp = pcm_open(CardIndex, PcmDspIdx, PCM_OUT, &mConfig);
                if (pcm_prepare(mPcmDsp) != 0) {
                    ALOGE("mPcmDsp prepare fail, err:%s", pcm_get_error(mPcmDsp));
                }
            }
#endif
        }
        ALOGD_IF(mPcmIn == NULL, "%s(), mPcmIn = %p, PcmInIdx = %d, CardIndex = %d",
                 __FUNCTION__, mPcmIn, PcmInIdx, CardIndex);
        ALOGD_IF(mPcmOut == NULL, "%s(), mPcmOut = %p, PcmOutIdx = %d, CardIndex = %d",
                 __FUNCTION__, mPcmOut, PcmOutIdx, CardIndex);
        ASSERT(mPcmIn != NULL && mPcmOut != NULL);

#if !defined(MTK_AUDIO_KS)
        pcm_start(mPcmIn);
#endif
        pcm_start(mPcmOut);

#if defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
        if (AudioSCPPhoneCallController::getInstance()->isSupportPhonecall(new_output_devices)) {
            AudioSCPPhoneCallController::getInstance()->enable(sample_rate, new_input_device);
        }
#endif
    }

    // Set new device
    if (checkTtyNeedOn() == true) {
        setTtyInOutDevice();
#if !defined (MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
        setTtyEnhancementMask(mTtyMode, mAdjustedOutDev);
#endif
    } else {
#if defined(MTK_USB_PHONECALL)
        if (!AudioUSBPhoneCallController::getInstance()->isEnable())
#endif
        {
            // Set PMIC digital/analog part - uplink has pop, open first
            mHardwareResourceManager->startInputDevice(mAdjustedInDev);

            // Set PMIC digital/analog part - DL need trim code.
            mHardwareResourceManager->startOutputDevice(mAdjustedOutDev, sample_rate);
        }
    }

#if defined(MTK_AUDIO_KS)
    // after start input device
    if (mPcmIn) {
        pcm_start(mPcmIn);
    }
#endif

    // start Side Tone Filter
    if (checkSideToneFilterNeedOn(mAdjustedOutDev) == true) {
        mHardwareResourceManager->EnableSideToneFilter(true);
        mIsSidetoneEnable = true;
    }

    // Set MD side sampling rate
    pSpeechDriver->SetModemSideSamplingRate(sample_rate);

    // Set speech mode
    if (checkTtyNeedOn() == false) {
        pSpeechDriver->SetSpeechMode(mAdjustedInDev, mAdjustedOutDev);
    } else {
        pSpeechDriver->TtyCtmOn();
    }

#if defined(MTK_AUDIODSP_SUPPORT)
    if (mPcmDlOut && mPcmDlIn && mPcmIv && mPcmDsp) {
        pcm_start(mPcmDlIn);
        pcm_start(mPcmIv);
        pcm_start(mPcmDlOut);
        pcm_start(mPcmDsp);
    }
#endif
    // Need recover mute state, trigger to wait for timeout unmute
    muteDlUlForRouting(SPH_MUTE_CTRL_ROUTING_END);

    ALOGD("%s(-), Adjusted output_devices: 0x%x, input_device: 0x%x"
          ", bt_device_on: %d, sample_rate: %u"
          ", CardIndex: %d, PcmInIdx: %d, PcmOutIdx: %d, mPcmIn: %p, mPcmOut: %p",
          __FUNCTION__,
          mAdjustedOutDev,
          mAdjustedInDev,
          bt_device_on, sample_rate,
          CardIndex, PcmInIdx, PcmOutIdx, mPcmIn, mPcmOut);

    return NO_ERROR;
}

audio_devices_t AudioALSASpeechPhoneCallController::getAdjustedInputDevice() {
    // mAdjustedInDev could be different from mInputDevice in TTY case
    AL_AUTOLOCK(mLock);
    return mAdjustedInDev;
}

audio_devices_t AudioALSASpeechPhoneCallController::getAdjustedOutputDevice() {
    // mAdjustedOutDev could be different from mOutputDevice in TTY case
    AL_AUTOLOCK(mLock);
    return mAdjustedOutDev;
}

audio_devices_t AudioALSASpeechPhoneCallController::getOutputDevice() {
    AL_AUTOLOCK(mLock);
    return mOutputDevice;
}

status_t AudioALSASpeechPhoneCallController::setTtyMode(const TtyModeType ttyMode) {
    return setTtyMode(ttyMode, mRttCallType);
}

status_t AudioALSASpeechPhoneCallController::setTtyMode(const TtyModeType ttyMode, const int rttCallType) {
    ALOGD("+%s(), mTtyMode(%d -> %d), mRttCallType(%d -> %d)",
          __FUNCTION__, mTtyMode, ttyMode, mRttCallType, rttCallType);

#if defined(MTK_TTY_SUPPORT)
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
    int ttySwitchType = TTY_SWITCH_SKIP;
    if (mTtyMode != ttyMode) {
        switch (ttyMode) {
        case AUD_TTY_VCO:
        case AUD_TTY_HCO:
        case AUD_TTY_FULL:
            ttySwitchType = TTY_SWITCH_BY_MODE;
            break;
        case AUD_TTY_OFF:
            ttySwitchType = TTY_SWITCH_FORCE_OFF;
            break;
        default:
            break;
        }
        mTtyMode = ttyMode;
    }
    if (mRttCallType != rttCallType) {
        if (rttCallType != RTT_CALL_TYPE_CS) {
            ttySwitchType = TTY_SWITCH_FORCE_OFF;
        } else {
            ttySwitchType = TTY_SWITCH_BY_MODE;
        }
        mRttCallType = rttCallType;
    }
    switch (ttySwitchType) {
    case TTY_SWITCH_FORCE_OFF:
        if ((pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == true) &&
            (pSpeechDriver->GetApSideModemStatus(TTY_STATUS_MASK) == true)) {
            //force disable TTY during call
            routing(mOutputDevice, mInputDevice);
            mAudioALSAVolumeController->setVoiceVolume(mAudioALSAVolumeController->getVoiceVolume(),
                                                       AudioALSAStreamManager::getInstance()->getModeForGain(),
                                                       mAdjustedOutDev);
        }
        break;

    case TTY_SWITCH_BY_MODE:
        if ((pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == true) &&
            (mTtyMode != AUD_TTY_OFF) && (mTtyMode != AUD_TTY_ERR) &&
            isTtyDevice(mOutputDevice)) {
            //recover TTY
            routing(mOutputDevice, mInputDevice);
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
            mAudioALSAVolumeController->setVoiceVolume(mAudioALSAVolumeController->getVoiceVolume(),
                                                       AudioALSAStreamManager::getInstance()->getModeForGain(),
                                                       mAdjustedOutDev);
#else
            setTtyEnhancementMask(mTtyMode, mAdjustedOutDev);
#endif
        }
        break;

    default:
        break;
    }
    ALOGD("-%s(), mTtyMode = %d, mRttCallType = %d, ttySwitchType = %d",
          __FUNCTION__, mTtyMode, mRttCallType, ttySwitchType);
#endif
    return NO_ERROR;
}

int AudioALSASpeechPhoneCallController::setTtyInOutDevice() {
    int gainMicTty = Normal_Mic;
    audio_devices_t sphModeOutDev = mAdjustedOutDev;
    ALOGV("+%s(), mTtyMode = %d", __FUNCTION__, mTtyMode);

    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
#if defined(MTK_TTY_SUPPORT)
    pSpeechDriver->setTtyMode(mTtyMode);
    mHardwareResourceManager->startOutputDevice(mAdjustedOutDev, mConfig.rate);
    mHardwareResourceManager->startInputDevice(mAdjustedInDev);

    switch (mTtyMode) {
    case AUD_TTY_VCO:
        if (mOutputDevice & AUDIO_DEVICE_OUT_SPEAKER) {
            gainMicTty = Handfree_Mic;
            sphModeOutDev = AUDIO_DEVICE_OUT_SPEAKER;
        } else if ((mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
                   (mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
            gainMicTty = Normal_Mic;
            sphModeOutDev = AUDIO_DEVICE_OUT_EARPIECE;
        } else {
            gainMicTty = Normal_Mic;
        }
        break;

    case AUD_TTY_HCO:
        if (mOutputDevice & AUDIO_DEVICE_OUT_SPEAKER) {
            gainMicTty = TTY_CTM_Mic;
            sphModeOutDev = AUDIO_DEVICE_OUT_SPEAKER;
        } else if ((mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
                   (mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
            gainMicTty = TTY_CTM_Mic;
        } else {
            gainMicTty = Normal_Mic;
        }
        break;

    case AUD_TTY_FULL:
        if (mOutputDevice & AUDIO_DEVICE_OUT_SPEAKER) {
            gainMicTty = TTY_CTM_Mic;
            sphModeOutDev = AUDIO_DEVICE_OUT_WIRED_HEADSET;
        } else if ((mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
                   (mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
            gainMicTty = TTY_CTM_Mic;
            sphModeOutDev = AUDIO_DEVICE_OUT_WIRED_HEADSET;
        } else {
            gainMicTty = Normal_Mic;
        }
        break;

    default:
        break;
    }
    pSpeechDriver->SetSpeechMode(mAdjustedInDev, sphModeOutDev);
    mAudioALSAVolumeController->ApplyMicGain(gainMicTty, mAudioMode);

#if !defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    mAudioALSAVolumeController->setVoiceVolume(mAudioALSAVolumeController->getVoiceVolume(),
                                               AudioALSAStreamManager::getInstance()->getModeForGain(),
                                               mAdjustedOutDev);
#endif

#endif
    ALOGD("%s(), mTtyMode(%d), gainMicTty(%d), mAdjustedOutDev(0x%x), sphModeOutDev(0x%x)",
          __FUNCTION__, mTtyMode, gainMicTty, mAdjustedOutDev, sphModeOutDev);
    return NO_ERROR;
}


int AudioALSASpeechPhoneCallController::adjustTtyInOutDevice() {
    bool isOutDevSpk = false;
    ALOGV("+%s(), mOutputDevice = 0x%x, mTtyMode = %d", __FUNCTION__, mOutputDevice, mTtyMode);
    mAdjustedOutDev = mOutputDevice;
    mAdjustedInDev = getInputDeviceForPhoneCall(mAdjustedOutDev);
#if defined(MTK_TTY_SUPPORT)
    if (mOutputDevice == AUDIO_DEVICE_NONE) {
        mAdjustedOutDev = AUDIO_DEVICE_OUT_DEFAULT;
        mAdjustedInDev = getInputDeviceForPhoneCall(mAdjustedOutDev);
        return NO_ERROR;
    }
    switch (mTtyMode) {
    case AUD_TTY_VCO:
        if ((isOutDevSpk = mOutputDevice & AUDIO_DEVICE_OUT_SPEAKER) ||
            (mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
            (mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
            if (isOutDevSpk) {
                ALOGD("%s(), speaker, TTY_VCO", __FUNCTION__);
            } else {
                ALOGD("%s(), headset, TTY_VCO", __FUNCTION__);
            }
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
            mAdjustedOutDev = AUDIO_DEVICE_OUT_EARPIECE;
#else
            mAdjustedOutDev = AUDIO_DEVICE_OUT_WIRED_HEADSET;
#endif
            mAdjustedInDev = AUDIO_DEVICE_IN_BUILTIN_MIC;
        }
        break;

    case AUD_TTY_HCO:
        if (mOutputDevice & AUDIO_DEVICE_OUT_SPEAKER) {
            ALOGD("%s(), speaker, TTY_HCO", __FUNCTION__);
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
            mAdjustedOutDev = AUDIO_DEVICE_OUT_EARPIECE;
#else
            mAdjustedOutDev = AUDIO_DEVICE_OUT_SPEAKER;
#endif
            mAdjustedInDev = AUDIO_DEVICE_IN_WIRED_HEADSET;
        } else if ((mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
                   (mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
            ALOGD("%s(), headset, TTY_HCO", __FUNCTION__);
            mAdjustedOutDev = AUDIO_DEVICE_OUT_EARPIECE;
            mAdjustedInDev = AUDIO_DEVICE_IN_WIRED_HEADSET;
        }
        break;
    case AUD_TTY_FULL:
        if ((isOutDevSpk = mOutputDevice & AUDIO_DEVICE_OUT_SPEAKER) ||
            (mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
            (mOutputDevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
            if (isOutDevSpk) {
                ALOGD("%s(), speaker, TTY_FULL", __FUNCTION__);
            } else {
                ALOGD("%s(), headset, TTY_FULL", __FUNCTION__);
            }
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
            mAdjustedOutDev = AUDIO_DEVICE_OUT_EARPIECE;
#else
            mAdjustedOutDev = AUDIO_DEVICE_OUT_WIRED_HEADSET;
#endif
            mAdjustedInDev = AUDIO_DEVICE_IN_WIRED_HEADSET;
        }
        break;
    default:
        break;
    }
#endif
    ALOGD("%s(), mTtyMode(%d), mOutputDevice(0x%x), mAdjustedInDev(0x%x), mAdjustedOutDev(0x%x)",
          __FUNCTION__, mTtyMode, mOutputDevice, mAdjustedInDev, mAdjustedOutDev);
    return NO_ERROR;
}

int AudioALSASpeechPhoneCallController::setRttCallType(const int rttCallType) {
#if defined(MTK_RTT_SUPPORT)
    ALOGD("+%s(), mRttCallType(%d -> %d)", __FUNCTION__, mRttCallType, rttCallType);
    if (mRttCallType != rttCallType) {
        SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
        switch (rttCallType) {
        case RTT_CALL_TYPE_RTT:
            mRttMode = AUD_RTT_ON;
            setTtyMode((const TtyModeType) mTtyMode, rttCallType);
            if (pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK)) {
                pSpeechDriver->RttConfig(mRttMode);
            }
            break;

        case RTT_CALL_TYPE_CS:
        case RTT_CALL_TYPE_PS:
        case RTT_CALL_TYPE_CS_NO_TTY:
            mRttMode = AUD_RTT_OFF;
            setTtyMode((const TtyModeType) mTtyMode, rttCallType);
            if (pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK)) {
                pSpeechDriver->RttConfig(mRttMode);
            }
            break;

        default:
            ALOGE("%s(): Invalid rttCallType(%d)", __FUNCTION__, rttCallType);
            break;
        }
    }
    ALOGD("-%s(): mRttCallType = %d, mRttMode = %d", __FUNCTION__, mRttCallType, mRttMode);
    return NO_ERROR;
#else
    ALOGW("%s(), rttCallType = %d, NOT Supported!", __FUNCTION__, rttCallType);
    return INVALID_OPERATION;
#endif

}

void AudioALSASpeechPhoneCallController::setVtNeedOn(const bool vt_on) {
    ALOGD("%s(), new vt_on = %d, old mVtNeedOn = %d", __FUNCTION__, vt_on, mVtNeedOn);
    AL_AUTOLOCK(mLock);

    mVtNeedOn = vt_on;
}

void AudioALSASpeechPhoneCallController::setMicMute(const bool mute_on) {
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(mMuteDlUlForRoutingLock);

    mMicMute = mute_on;
    mSpeechDriverFactory->GetSpeechDriver()->SetUplinkMute(mute_on);

    set_uint32_to_mixctrl(PROPERTY_KEY_MIC_MUTE_ON, mute_on);
}

void AudioALSASpeechPhoneCallController::setDlMute(const bool mute_on) {
    ALOGD("%s(), mDlMute: %d => %d", __FUNCTION__, mDlMute, mute_on);
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(mMuteDlUlForRoutingLock);

    mDlMute = mute_on;
    mSpeechDriverFactory->GetSpeechDriver()->SetDownlinkMute(mute_on);

    set_uint32_to_mixctrl(PROPERTY_KEY_DL_MUTE_ON, mute_on);
}

void AudioALSASpeechPhoneCallController::setUlMute(const bool mute_on) {
    ALOGD("%s(), mUlMute: %d => %d", __FUNCTION__, mUlMute, mute_on);
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(mMuteDlUlForRoutingLock);

    mUlMute = mute_on;
    mSpeechDriverFactory->GetSpeechDriver()->SetUplinkSourceMute(mute_on);

    set_uint32_to_mixctrl(PROPERTY_KEY_UL_MUTE_ON, mute_on);
}

void AudioALSASpeechPhoneCallController::setMuteInfo() {
    AL_AUTOLOCK(mMuteDlUlForRoutingLock);

    // check Mute bitwise info
    uint32_t mMuteInfo = ((mDlMute) ? SPH_ON_MUTE_BIT_DL : 0) |
                         ((mMicMute) ? SPH_ON_MUTE_BIT_UL : 0) |
                         ((mUlMute) ? SPH_ON_MUTE_BIT_UL_SOURCE : 0);

    ALOGD("%s(), mMuteInfo: 0x%x", __FUNCTION__, mMuteInfo);

    mSpeechDriverFactory->GetSpeechDriver()->setMuteInfo(mMuteInfo);
}

void AudioALSASpeechPhoneCallController::setBTMode(const int mode) {
    ALOGD("%s(), mBTMode: %d => %d", __FUNCTION__, mBTMode, mode);
    AL_AUTOLOCK(mLock);

    bool isSpeechOpen = mSpeechDriverFactory->GetSpeechDriver()->GetApSideModemStatus(SPEECH_STATUS_MASK);
    mSpeechDriverFactory->GetSpeechDriver()->setBTMode(mode);

    if (mBTMode != mode) {
        mBTMode = mode;
        if (isSpeechOpen &&
            (audio_is_bluetooth_sco_device(mOutputDevice) || mIsBtSpkDevice == true)) {
            uint32_t sample_rate = calculateSampleRate(true);
            mSpeechDriverFactory->GetSpeechDriver()->switchBTMode(sample_rate);
        }
    }
}


void AudioALSASpeechPhoneCallController::setBtSpkDevice(const bool flag) {
    if (mIsBtSpkDevice != flag) {
        ALOGD("%s(), mIsBtSpkDevice: %d => %d", __FUNCTION__, mIsBtSpkDevice, flag);
        mIsBtSpkDevice = flag;
        mSpeechDriverFactory->GetSpeechDriver()->setBtSpkDevice(flag);
    }
}


void AudioALSASpeechPhoneCallController::getRFInfo() {
    WARNING("Not implement yet!!");
}


modem_index_t AudioALSASpeechPhoneCallController::updatePhysicalModemIdx(const audio_mode_t audio_mode) {
#if defined(MTK_COMBO_MODEM_SUPPORT)
    (void)audio_mode;
    return MODEM_1;
#else
    modem_index_t modem_index = MODEM_1;

    if (mSpeechDVT_MD_IDX == 0) {
        modem_index = mIdxMDByPhoneId[mPhoneId];
        ALOGD("%s(), audio_mode(%d), mPhoneId(%d), modem_index=%d",
              __FUNCTION__, audio_mode, mPhoneId, modem_index);
    } else {
        switch (mSpeechDVT_MD_IDX) {
        case 1: {
            modem_index = MODEM_1;
            break;
        }
        case 2: {
            modem_index = MODEM_EXTERNAL;
            break;
        }
        default: {
            modem_index = MODEM_1;
            break;
        }
        }
        ALOGD("%s(), SpeechDVT_MD_IDX = %d, modem_index=%d",
              __FUNCTION__, mSpeechDVT_MD_IDX, modem_index);
    }
    return modem_index;
#endif
}

int AudioALSASpeechPhoneCallController::setPhoneId(const phone_id_t phoneId) {
#if !defined(MTK_COMBO_MODEM_SUPPORT)
    ALOGD("+%s(), mPhoneId = %d, new phoneId = %d", __FUNCTION__, mPhoneId, phoneId);
#endif
    if (phoneId != mPhoneId) {
        if (phoneId == PHONE_ID_0 || phoneId == PHONE_ID_1) {
            mPhoneId = phoneId;
            set_uint32_to_mixctrl(PROPERTY_KEY_FOREGROUND_PHONE_ID, mPhoneId);
#if !defined(MTK_COMBO_MODEM_SUPPORT)
            ALOGD("-%s(), mPhoneId = %d", __FUNCTION__, mPhoneId);
#endif
        } else {
            ALOGW("-%s(), Invalid %d. return. mPhoneId = %d", __FUNCTION__, phoneId, mPhoneId);
        }
    }
    return NO_ERROR;
}

/**
 * check if Phone Call need reopen according to RIL mapped modem
 */
bool AudioALSASpeechPhoneCallController::checkReopen(const modem_index_t rilMappedMDIdx) {
    AL_AUTOLOCK(mCheckOpenLock);
    bool needReopen = false;
    bool isSpeechOpen = mSpeechDriverFactory->GetSpeechDriver()->GetApSideModemStatus(SPEECH_STATUS_MASK);
    modem_index_t activeMDIdx = mSpeechDriverFactory->GetActiveModemIndex();
    if (isSpeechOpen) {
        //check modem index
        if (activeMDIdx != rilMappedMDIdx) {
            needReopen = true;
        }
    }
    ALOGD("%s(), needReopen(%d), MDIdx(%d->%d), isSpeechOpen(%d)",
          __FUNCTION__, needReopen, activeMDIdx, rilMappedMDIdx, isSpeechOpen);
    return needReopen;
}

status_t AudioALSASpeechPhoneCallController::setParam(const String8 &keyParamPairs) {
    ALOGD("+%s(): %s", __FUNCTION__, keyParamPairs.string());
    AudioParameter param = AudioParameter(keyParamPairs);
    int value;
    String8 ValueParam;

    if (param.getInt(String8("AudioTaste"), value) == NO_ERROR) {
        param.remove(String8("AudioTaste"));
        bAudioTaste = (value == 1) ? true : false;

        ALOGD("%s(): bAudioTaste = %d", __FUNCTION__, bAudioTaste);
    } else if (param.getInt(String8("SpeechDVT_SampleRate"), value) == NO_ERROR) {
        param.remove(String8("SpeechDVT_SampleRate"));
        mSpeechDVT_SampleRate = value;

        ALOGD("%s(): mSpeechDVT_SampleRate = %d", __FUNCTION__, mSpeechDVT_SampleRate);
    } else if (param.getInt(String8("SpeechDVT_MD_IDX"), value) == NO_ERROR) {
        param.remove(String8("SpeechDVT_MD_IDX"));
        mSpeechDVT_MD_IDX = value;

        ALOGD("%s(): mSpeechDVT_MD_IDX = %d", __FUNCTION__, mSpeechDVT_MD_IDX);
    } else if (param.get(String8("Phone1Modem"), ValueParam) == NO_ERROR) {
        param.remove(String8("Phone1Modem"));
        if (ValueParam.string() != NULL) {
            if (strcmp(ValueParam.string(), "MD1") == 0) {
                mIdxMDByPhoneId[0] = MODEM_1;
            } else if (strcmp(ValueParam.string(), "MD3") == 0) {
                mIdxMDByPhoneId[0] = MODEM_EXTERNAL;
            } else {
                ALOGW("%s(), %s, Invalid MD Index. return", __FUNCTION__, ValueParam.string());
            }
            set_uint32_to_mixctrl(PROPERTY_KEY_PHONE1_MD, mIdxMDByPhoneId[0]);
        }
    } else if (param.get(String8("Phone2Modem"), ValueParam) == NO_ERROR) {
        param.remove(String8("Phone2Modem"));
        if (ValueParam.string() != NULL) {
            if (strcmp(ValueParam.string(), "MD1") == 0) {
                mIdxMDByPhoneId[1] = MODEM_1;
            } else if (strcmp(ValueParam.string(), "MD3") == 0) {
                mIdxMDByPhoneId[1] = MODEM_EXTERNAL;
            } else {
                ALOGW("%s(), %s, Invalid MD Index. return", __FUNCTION__, ValueParam.string());
            }
            set_uint32_to_mixctrl(PROPERTY_KEY_PHONE2_MD, mIdxMDByPhoneId[1]);
        }
    }
    ALOGD("-%s(): %s", __FUNCTION__, keyParamPairs.string());
    return NO_ERROR;
}

status_t AudioALSASpeechPhoneCallController::updateSpeechFeature(const SpeechFeatureType speechFeatureType,
                                                                 bool enable) {

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    switch (speechFeatureType) {
    case SPEECH_FEATURE_SUPERVOLUME:
        if (SpeechConfig::getInstance()->getSpeechParamSupport(speechFeatureType)) {//PARSE BY DRIVER
            ALOGD("%s(), Super Volume(%d -> %d)", __FUNCTION__, mSuperVolumeEnable, enable);
            if (mSuperVolumeEnable != enable) {

#if defined(MTK_COMBO_MODEM_SUPPORT)
                SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
                pSpeechDriver->updateSpeechFeature(SPEECH_FEATURE_SUPERVOLUME, enable);
#else
                if (enable) {
                    SpeechParamParser::getInstance()->SetParamInfo(String8("ParamSphSV=1;"));
                } else {
                    SpeechParamParser::getInstance()->SetParamInfo(String8("ParamSphSV=0;"));
                }
#endif
                mSuperVolumeEnable = enable;
            }
        }
        break;
    default:
        ALOGD("%s() speechFeatureType(%d) NOT Supported!", __FUNCTION__, speechFeatureType);
        return INVALID_OPERATION;
    }
    return NO_ERROR;

#else
    (void)enable;
    (void)speechFeatureType;
    ALOGD("%s() NOT Supported!", __FUNCTION__);
    return INVALID_OPERATION;
#endif
}

bool AudioALSASpeechPhoneCallController::getSpeechFeatureStatus(const SpeechFeatureType speechFeatureType) {
    bool status = false;
    switch (speechFeatureType) {
    case SPEECH_FEATURE_SUPERVOLUME:
        ALOGV("%s(), mSuperVolumeEnable=%d", __FUNCTION__, mSuperVolumeEnable);
        status =  mSuperVolumeEnable;
        break;
    default:
        ALOGW("%s() speechFeatureType(%d) NOT Supported!", __FUNCTION__, speechFeatureType);
        break;
    }
    return status;
}


#if !defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
void setTtyEnhancementMask(TtyModeType ttMode, const audio_devices_t outputDevice) {
    ALOGD("+%s, ttMode=%d, outputDevice=%d", __FUNCTION__, ttMode, outputDevice);
    SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
    sph_enh_mask_struct_t sphMask;
    if (ttMode == AUD_TTY_VCO) {
        if (outputDevice == AUDIO_DEVICE_OUT_SPEAKER) {
            //handfree mic
            sphMask.main_func = SPH_ENH_MAIN_MASK_AEC |
                                SPH_ENH_MAIN_MASK_EES |
                                SPH_ENH_MAIN_MASK_ULNR |
                                SPH_ENH_MAIN_MASK_TDNC |
                                SPH_ENH_MAIN_MASK_DMNR |
                                SPH_ENH_MAIN_MASK_AGC;
            sphMask.dynamic_func = (SPH_ENH_DYNAMIC_MASK_ALL  & (~SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN));
            pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_DEFAULT, AUDIO_DEVICE_OUT_SPEAKER);
        } else if ((outputDevice == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
                   (outputDevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
            //handset mic
            sphMask.main_func = SPH_ENH_MAIN_MASK_AEC |
                                SPH_ENH_MAIN_MASK_EES |
                                SPH_ENH_MAIN_MASK_ULNR |
                                SPH_ENH_MAIN_MASK_TDNC |
                                SPH_ENH_MAIN_MASK_DMNR |
                                SPH_ENH_MAIN_MASK_AGC;
            sphMask.dynamic_func = (SPH_ENH_DYNAMIC_MASK_ALL  & (~SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN));
            pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_DEFAULT, AUDIO_DEVICE_OUT_EARPIECE);
        }
    } else if (ttMode == AUD_TTY_HCO) {
        if (outputDevice == AUDIO_DEVICE_OUT_SPEAKER) {
            // handfree speaker
            sphMask.main_func = SPH_ENH_MAIN_MASK_DLNR;
            sphMask.dynamic_func = (SPH_ENH_DYNAMIC_MASK_ALL  & (~SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN));
            pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_DEFAULT, AUDIO_DEVICE_OUT_SPEAKER);
        } else if ((outputDevice == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
                   (outputDevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
            // handset receiver
            sphMask.main_func = SPH_ENH_MAIN_MASK_DLNR;
            sphMask.dynamic_func = (SPH_ENH_DYNAMIC_MASK_ALL  & (~SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN));
            pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_DEFAULT, AUDIO_DEVICE_OUT_EARPIECE);
        }
    } else if (ttMode == AUD_TTY_FULL) {
        pSpeechDriver->SetSpeechEnhancement(false);
        ALOGD("-%s, disable speech enhancement", __FUNCTION__);
        return;
    } else if (ttMode == AUD_TTY_OFF) {
        pSpeechDriver->SetSpeechEnhancement(true);
        pSpeechDriver->SetSpeechEnhancementMask(SpeechEnhancementController::GetInstance()->GetSpeechEnhancementMask());
        ALOGD("-%s, recover all speech enhancement", __FUNCTION__);
        return;
    }

    pSpeechDriver->SetSpeechEnhancement(true);
    pSpeechDriver->SetSpeechEnhancementMask(sphMask);
    ALOGD("-%s, main_func=0x%x, dynamic_func=0x%x", __FUNCTION__, sphMask.main_func, sphMask.dynamic_func);

}
#endif

void *AudioALSASpeechPhoneCallController::pmicResetThread(void *arg) {
    AudioALSASpeechPhoneCallController *call_controller = NULL;
    AudioLock *lock = NULL;
    struct mixer_ctl *mixerCtrl = NULL;

    char thread_name[128] = {0};
    int retvalWait = 0;
    int retMixerCtrl = 0;

    CONFIG_THREAD(thread_name, ANDROID_PRIORITY_AUDIO);

    call_controller = static_cast<AudioALSASpeechPhoneCallController *>(arg);
    if (call_controller == NULL) {
        ALOGE("%s(), call_controller is NULL!!", __FUNCTION__);
        goto PMIC_RESET_THREAD_DONE;
    }
    mixerCtrl = mixer_get_ctl_by_name(mMixer, "PMIC_REG_CLEAR");
    if (mixerCtrl == NULL) {
        ALOGE("%s(), mixerCtrl is NULL!!", __FUNCTION__);
        goto PMIC_RESET_THREAD_DONE;
    }

    lock = &call_controller->mPmicResetLock;
    AL_LOCK(lock);
    ALOGD("%s() started, mIsPmicResetThreadEnable(%d)",
          thread_name, call_controller->mIsPmicResetThreadEnable);

    while (call_controller->mIsPmicResetThreadEnable == true) {
        retvalWait = AL_WAIT_MS(lock, DEFAULT_WAIT_PMIC_RESET_MS);
        if (retvalWait == -ETIMEDOUT) { //1s time out, do pmic reset
#if defined(SPEECH_PMIC_RESET_ACC)
            retMixerCtrl = mixer_ctl_set_enum_by_string(mixerCtrl, "ACC");
#elif defined(SPEECH_PMIC_RESET_DCC)
            retMixerCtrl = mixer_ctl_set_enum_by_string(mixerCtrl, "DCC");
#endif
            if (retMixerCtrl) {
                ALOGE("%s(), Error: PMIC_REG_CLEAR invalid value", thread_name);
            } else {
                ALOGD("%s(), do PMIC_REG_CLEAR every 1sec, mIsPmicResetThreadEnable(%d)",
                      thread_name, call_controller->mIsPmicResetThreadEnable);
            }
        }
    }
    AL_UNLOCK(lock);

PMIC_RESET_THREAD_DONE:
    if (call_controller == NULL) {
        ALOGE("%s() terminated", thread_name);
    } else {
        ALOGD("%s() terminated, mIsPmicResetThreadEnable(%d)",
              thread_name, call_controller->mIsPmicResetThreadEnable);
    }
    pthread_exit(NULL);
    return NULL;
}


void AudioALSASpeechPhoneCallController::muteDlCodecForShutterSound(const bool mute_on) {
    ALOGD("%s(), mMuteDlCodecForShutterSoundCount: %u, do mute_on: %d",
          __FUNCTION__, mMuteDlCodecForShutterSoundCount, mute_on);

    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    if (pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == false &&
        pSpeechDriver->GetApSideModemStatus(VT_STATUS_MASK) == false) {
        ALOGW("%s(), speech off!! do nothing!!", __FUNCTION__);
        return;
    }

    AL_LOCK(mMuteDlCodecForShutterSoundLock);

    if (mute_on == true) {
        if (mMuteDlCodecForShutterSoundCount == 0) {
            if (mIsMuteDlCodec == false) {
                pSpeechDriver->SetDownlinkMuteCodec(true);
                mIsMuteDlCodec = true;
            } else {
                AL_SIGNAL(mMuteDlCodecForShutterSoundLock); // cancel wait & mute
            }
        }
        mMuteDlCodecForShutterSoundCount++;
    } else { // unmute
        if (mMuteDlCodecForShutterSoundCount == 0) {
            WARNING("BGS unmute DL Codec not in pair!!");
        } else {
            mMuteDlCodecForShutterSoundCount--;
            if (mMuteDlCodecForShutterSoundCount == 0) {
                AL_SIGNAL(mMuteDlCodecForShutterSoundLock); // notify to wait & mute
            }
        }
    }

    AL_UNLOCK(mMuteDlCodecForShutterSoundLock);

}


void *AudioALSASpeechPhoneCallController::muteDlCodecForShutterSoundThread(void *arg) {
    AudioALSASpeechPhoneCallController *call_controller = NULL;
    SpeechDriverInterface *pSpeechDriver = NULL;
    AudioLock *lock = NULL;

    char thread_name[128] = {0};
    int retval = 0;

    CONFIG_THREAD(thread_name, ANDROID_PRIORITY_AUDIO);

    call_controller = static_cast<AudioALSASpeechPhoneCallController *>(arg);
    if (call_controller == NULL) {
        ALOGE("%s(), call_controller is NULL!!", __FUNCTION__);
        goto MUTE_DL_CODEC_FOR_SHUTTER_SOUND_THREAD_DONE;
    }

    lock = &call_controller->mMuteDlCodecForShutterSoundLock;

    AL_LOCK(lock);

    while (call_controller->mMuteDlCodecForShutterSoundThreadEnable == true) {
        // sleep until signal comes
        AL_WAIT_NO_TIMEOUT(lock);

        // debug
        ALOGD("%s(), count: %u, mute: %d, start to wait & mute", __FUNCTION__,
              call_controller->mMuteDlCodecForShutterSoundCount,
              call_controller->mIsMuteDlCodec);

        // wait and then unmute
        if (call_controller->mMuteDlCodecForShutterSoundCount == 0 &&
            call_controller->mIsMuteDlCodec == true) {
            retval = AL_WAIT_MS(lock, DEFAULT_WAIT_SHUTTER_SOUND_UNMUTE_MS);
            if (call_controller->mMuteDlCodecForShutterSoundCount == 0 &&
                call_controller->mIsMuteDlCodec == true) { // double check
                ALOGD("%s(), count: %u, mute: %d, do mute DL codec", __FUNCTION__,
                      call_controller->mMuteDlCodecForShutterSoundCount,
                      call_controller->mIsMuteDlCodec);
                pSpeechDriver = call_controller->mSpeechDriverFactory->GetSpeechDriver();
                pSpeechDriver->SetDownlinkMuteCodec(false);
                call_controller->mIsMuteDlCodec = false;
            } else {
                ALOGD("%s(), count: %u, mute: %d, mute canceled, retval: %d", __FUNCTION__,
                      call_controller->mMuteDlCodecForShutterSoundCount,
                      call_controller->mIsMuteDlCodec, retval);
            }
        }

    }

    AL_UNLOCK(lock);

MUTE_DL_CODEC_FOR_SHUTTER_SOUND_THREAD_DONE:
    ALOGV("%s terminated", thread_name);
    pthread_exit(NULL);
    return NULL;
}

void AudioALSASpeechPhoneCallController::updateVolume() {
    muteDlUlForRouting(SPH_MUTE_CTRL_VOLUME_UPDATE);
}

void AudioALSASpeechPhoneCallController::muteDlUlForRouting(const int muteCtrl) {
    ALOGD_IF(mLogEnable, "%s(), do mute_ctrl: %d, mMuteDlUlForRoutingState: %d, routing output device = 0x%x",
             __FUNCTION__, muteCtrl, mMuteDlUlForRoutingState, mAdjustedOutDev);

    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
    if (pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == false) {
        ALOGW("%s(), speech off!! do nothing!!", __FUNCTION__);
        return;
    }

    AL_LOCK(mMuteDlUlForRoutingLock);
    mMuteDlUlForRoutingCtrl = muteCtrl;

    switch (mMuteDlUlForRoutingCtrl) {
    case SPH_MUTE_CTRL_ROUTING_START:
        if (mMuteDlUlForRoutingState != SPH_MUTE_THREAD_STATE_WAIT) {
            pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
            pSpeechDriver->SetDownlinkMute(true);
            pSpeechDriver->SetUplinkMute(true);
            pSpeechDriver->SetUplinkSourceMute(true); // avoid hw pop
            ALOGD_IF(mLogEnable, "%s(), mMuteDlUlForRoutingCtrl = %d, mMuteDlUlForRoutingState = %d, "
                     "do mute only",
                     __FUNCTION__, mMuteDlUlForRoutingCtrl, mMuteDlUlForRoutingState);
        } else {
            ALOGD_IF(mLogEnable, "%s(), mMuteDlUlForRoutingCtrl = %d, mMuteDlUlForRoutingState = %d, "
                     "do mute and stop waiting",
                     __FUNCTION__, mMuteDlUlForRoutingCtrl, mMuteDlUlForRoutingState);
            AL_SIGNAL(mMuteDlUlForRoutingLock);
        }
        break;

    case SPH_MUTE_CTRL_ROUTING_END:
        ALOGD_IF(mLogEnable, "%s(), mMuteDlUlForRoutingCtrl = %d, trigger thread, routing output device = 0x%x",
                 __FUNCTION__, mMuteDlUlForRoutingCtrl, mAdjustedOutDev);
        AL_SIGNAL(mMuteDlUlForRoutingLock); // notify to wait & mute
        break;

    case SPH_MUTE_CTRL_VOLUME_UPDATE:
        if (mMuteDlUlForRoutingState == SPH_MUTE_THREAD_STATE_WAIT) {
            ALOGD_IF(mLogEnable, "%s(), mMuteDlUlForRoutingCtrl = %d, mMuteDlUlForRoutingState = %d, "
                     "do unmute directly",
                     __FUNCTION__, mMuteDlUlForRoutingCtrl, mMuteDlUlForRoutingState);
            AL_SIGNAL(mMuteDlUlForRoutingLock); // notify to mute directly
        }
        break;

    case SPH_MUTE_CTRL_IDLE:
    default:
        ALOGD_IF(mLogEnable, "%s(), mMuteDlUlForRoutingCtrl = %d, mMuteDlUlForRoutingState = %d",
                 __FUNCTION__, mMuteDlUlForRoutingCtrl, mMuteDlUlForRoutingState);
        break;
    }

    AL_UNLOCK(mMuteDlUlForRoutingLock);
}

void *AudioALSASpeechPhoneCallController::muteDlUlForRoutingThread(void *arg) {
    AudioALSASpeechPhoneCallController *call_controller = NULL;
    SpeechDriverInterface *pSpeechDriver = NULL;
    AudioLock *lock = NULL;

    char thread_name[128] = {0};
    int retvalWait = 0;

    CONFIG_THREAD(thread_name, ANDROID_PRIORITY_AUDIO);

    call_controller = static_cast<AudioALSASpeechPhoneCallController *>(arg);
    if (call_controller == NULL) {
        ALOGE("%s(), call_controller is NULL!!", __FUNCTION__);
        goto MUTE_DL_UL_FOR_ROUTING_THREAD_DONE;
    }

    lock = &call_controller->mMuteDlUlForRoutingLock;
    call_controller->mMuteDlUlForRoutingState = SPH_MUTE_THREAD_STATE_IDLE;

    AL_LOCK(lock);

    while (call_controller->mMuteDlUlForRoutingThreadEnable == true) {
        // sleep until signal comes
        AL_WAIT_NO_TIMEOUT(lock);

        // debug
        ALOGD_IF(call_controller->mLogEnable, "%s(), Ctrl: %d, State: %d, start to wait & mute", __FUNCTION__,
                 call_controller->mMuteDlUlForRoutingCtrl,
                 call_controller->mMuteDlUlForRoutingState);

        // wait and then recover to current mute status
        if (call_controller->mMuteDlUlForRoutingCtrl == SPH_MUTE_CTRL_ROUTING_END) {
            call_controller->mMuteDlUlForRoutingState = SPH_MUTE_THREAD_STATE_WAIT;
            retvalWait = AL_WAIT_MS(lock, DEFAULT_WAIT_ROUTING_UNMUTE_MS);
            call_controller->mMuteDlUlForRoutingState = SPH_MUTE_THREAD_STATE_IDLE;

            if (retvalWait == -ETIMEDOUT) { //time out, do unmute
                pSpeechDriver = call_controller->mSpeechDriverFactory->GetSpeechDriver();
                pSpeechDriver->SetUplinkSourceMute(call_controller->mUlMute);
                pSpeechDriver->SetUplinkMute(call_controller->mMicMute);
                pSpeechDriver->SetDownlinkMute(call_controller->mDlMute);
                ALOGD("%s(), Ctrl: %d, State: %d, wait retval(%d), wait %dms and unmute", __FUNCTION__,
                      call_controller->mMuteDlUlForRoutingCtrl,
                      call_controller->mMuteDlUlForRoutingState,
                      retvalWait, DEFAULT_WAIT_ROUTING_UNMUTE_MS);

            } else {//disturb wait
                if (call_controller->mMuteDlUlForRoutingCtrl == SPH_MUTE_CTRL_ROUTING_START) {
                    //break wait
                    ALOGD_IF(call_controller->mLogEnable, "%s(), Ctrl: %d, State: %d, wait retval(%d), break waiting, "
                             "keep routing mute", __FUNCTION__,
                             call_controller->mMuteDlUlForRoutingCtrl,
                             call_controller->mMuteDlUlForRoutingState, retvalWait);
                } else if (call_controller->mMuteDlUlForRoutingCtrl == SPH_MUTE_CTRL_VOLUME_UPDATE) {
                    pSpeechDriver = call_controller->mSpeechDriverFactory->GetSpeechDriver();
                    pSpeechDriver->SetUplinkSourceMute(call_controller->mUlMute);
                    pSpeechDriver->SetUplinkMute(call_controller->mMicMute);
                    pSpeechDriver->SetDownlinkMute(call_controller->mDlMute);
                    ALOGD("%s(), Ctrl: %d, State: %d, wait retval(%d), unmute directly", __FUNCTION__,
                          call_controller->mMuteDlUlForRoutingCtrl,
                          call_controller->mMuteDlUlForRoutingState, retvalWait);
                }
            }
        }
    }

    AL_UNLOCK(lock);

MUTE_DL_UL_FOR_ROUTING_THREAD_DONE:
    ALOGV("%s terminated", thread_name);
    pthread_exit(NULL);
    return NULL;
}

} // end of namespace android

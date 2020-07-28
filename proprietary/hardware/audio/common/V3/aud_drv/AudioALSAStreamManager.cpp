#include "AudioALSAStreamManager.h"

#include <cutils/properties.h>

#include <tinyalsa/asoundlib.h> // TODO(Harvey): move it

#include "WCNChipController.h"

#include "AudioALSAStreamOut.h"
#include "AudioALSAStreamIn.h"
#include "AudioALSAPlaybackHandlerBase.h"
#include "AudioALSAPlaybackHandlerMixer.h"
#include "AudioALSAPlaybackHandlerNormal.h"
#include "AudioALSAPlaybackHandlerFast.h"
#include "AudioALSAPlaybackHandlerVoice.h"
#include "AudioALSAPlaybackHandlerBTSCO.h"
#include "AudioALSAPlaybackHandlerBTCVSD.h"
#include "AudioALSAPlaybackHandlerFMTransmitter.h"
#include "AudioALSAPlaybackHandlerHDMI.h"
#ifdef MTK_MAXIM_SPEAKER_SUPPORT
#include "AudioALSAPlaybackHandlerSpeakerProtection.h"
#endif

#if defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
#include "AudioALSAPlaybackHandlerSpeakerProtectionDsp.h"
#endif

#ifdef MTK_AUDIODSP_SUPPORT
#include "AudioALSAPlaybackHandlerDsp.h"
#include "AudioALSACaptureHandlerDsp.h"
#include "AudioDspStreamManager.h"
#endif

#if !defined(MTK_BASIC_PACKAGE)
#include "AudioALSAPlaybackHandlerOffload.h"
#endif

#include "AudioALSACaptureHandlerBase.h"
#include "AudioALSACaptureHandlerNormal.h"
#include "AudioALSACaptureHandlerSyncIO.h"
#include "AudioALSACaptureHandlerVoice.h"
#include "AudioALSACaptureHandlerFMRadio.h"
#include "AudioALSACaptureHandlerBT.h"
#ifdef MTK_VOW_SUPPORT
#include "AudioALSACaptureHandlerVOW.h"
#include "AudioALSAVoiceWakeUpController.h"
#endif
#include "AudioALSACaptureHandlerAEC.h"
#include "AudioALSACaptureHandlerTDM.h"

#include "AudioALSACaptureHandlerModemDai.h"

#include "AudioALSASpeechPhoneCallController.h"
#include "AudioALSAFMController.h"

//#include "AudioALSAVolumeController.h"
//#include "AudioVolumeInterface.h"
#include "AudioVolumeFactory.h"
#include "AudioDeviceInt.h"



#include "AudioALSAHardwareResourceManager.h" // TODO(Harvey): move it
#include "AudioALSASpeechStreamController.h"
#include "AudioALSASampleRateController.h"

#include "AudioCompFltCustParam.h"
#include "SpeechDriverInterface.h"
#include "SpeechDriverFactory.h"
#include "AudioALSADriverUtility.h"
#include "SpeechEnhancementController.h"
#include "SpeechVMRecorder.h"
#include "AudioSmartPaController.h"

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
#include <AudioParamParser.h>
#include <AudioALSAParamTuner.h>
#include <SpeechConfig.h>
#if !defined(MTK_COMBO_MODEM_SUPPORT)
#include <SpeechParamParser.h>
#endif
#endif

#if defined(MTK_HYBRID_NLE_SUPPORT)
#include "AudioALSANLEController.h"
#endif

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <aurisys_controller.h>
#endif

#ifdef MTK_AUDIO_SCP_SUPPORT
#include <AudioMessengerIPI.h>
#endif

#if defined(MTK_AUDIODSP_SUPPORT)
#include <audio_task.h>
#include <AudioDspType.h>
#include "audio_a2dp_msg_id.h"
#endif

#if defined(MTK_AUDIO_DSP_RECOVERY_SUPPORT)
#include <audio_dsp_controller.h>
#endif

#include <AudioEventThreadManager.h>

#if defined(MTK_AUDIO_AAUDIO_SUPPORT)
#include "AudioALSAPlaybackHandlerAAudio.h"
#include "AudioALSACaptureHandlerAAudio.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAStreamManager"
//#define FM_HIFI_NOT_CONCURRENT
#define AUDIO_HIFI_RATE_MIN (48000)

/* For UplinkCustomization information query */
#define UPLINK_CUSTOMIZATION_AUDIO_TYPE "UplinkConfiguration"
#define OFFLOAD_PARAM_NAME              "offload"
#define NUM_OF_MIC_PARAM_NAME           "num of mic"
#define VOIP_PATH                       "InputSource,VoIP"
#define FAST_RECORD_PATH                "InputSource,FastRecord"
#define CAMCORDER_PATH                  "InputSource,Camcorder"
#define VOICE_RECOGNITION_PATH          "InputSource,VoiceRecognition"
#define UNPROCESSED_PATH                "InputSource,Unprocessed"
#define VOICE_PERFORMANCE_PATH          "InputSource,VoicePerformance"
#define MIC_PATH                        "InputSource,MIC"

static struct pcm_config mLoopbackConfig; // TODO(Harvey): move it to AudioALSAHardwareResourceManager later
static struct pcm *mLoopbackPcm = NULL; // TODO(Harvey): move it to AudioALSAHardwareResourceManager & AudioALSAPlaybackDataDispatcher later
static struct pcm_config mLoopbackUlConfig; // TODO(Harvey): move it to AudioALSAHardwareResourceManager later
static struct pcm *mLoopbackUlPcm = NULL; // TODO(Harvey): move it to AudioALSAHardwareResourceManager & AudioALSAPlaybackDataDispatcher later
static struct pcm *mHdmiPcm = NULL; // TODO(Harvey): move it to AudioALSAHardwareResourceManager & AudioALSAPlaybackDataDispatcher later

namespace android {

/*==============================================================================
 *                     Property keys
 *============================================================================*/
const char *PROPERTY_KEY_SET_BT_NREC = "persist.vendor.debug.set_bt_aec";
const char *PROPERTY_KEY_BESLOUDNESS_SWITCH_STATE = "persist.vendor.audiohal.besloudness_state";
const char *PROPERTY_KEY_FIRSTBOOT_STATE = "persist.vendor.audiohal.firstboot";
#define BESLOUDNESS_SWITCH_DEFAULT_STATE (1)


/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

AudioALSAStreamManager *AudioALSAStreamManager::mStreamManager = NULL;
AudioALSAStreamManager *AudioALSAStreamManager::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mStreamManager == NULL) {
        mStreamManager = new AudioALSAStreamManager();
    }
    ASSERT(mStreamManager != NULL);
    return mStreamManager;
}


/*==============================================================================
 *                     Callback Function
 *============================================================================*/
void callbackPhoneCallReopen(int audioEventType, void *caller, void *arg) {
    ALOGD("%s(), audioEventType = %d, caller(%p), arg(%p)",
          __FUNCTION__, audioEventType, caller, arg);
    AudioALSAStreamManager *streamManager = NULL;
    streamManager = static_cast<AudioALSAStreamManager *>(caller);
    if (streamManager == NULL) {
        ALOGE("%s(), streamManager is NULL!!", __FUNCTION__);
    } else {
        streamManager->phoneCallReopen();
    }
}

bool speakerStatusChangeCb(const DeviceStatus status) {
    ALOGV("%s(), status = %d", __FUNCTION__, status);
    bool ret = true;
#ifdef MTK_VOW_BARGE_IN_SUPPORT
    bool isSpeakerPlaying = (status == DEVICE_STATUS_ON) ? true : false;
    ret = AudioALSAVoiceWakeUpController::getInstance()->updateSpeakerPlaybackStatus(isSpeakerPlaying);
#endif
    return ret;
}


/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

AudioALSAStreamManager::AudioALSAStreamManager() :
    mStreamOutIndex(0),
    mStreamInIndex(0),
    mPlaybackHandlerIndex(0),
    mCaptureHandlerIndex(0),
    mSpeechPhoneCallController(AudioALSASpeechPhoneCallController::getInstance()),
    mSmartPaController(AudioSmartPaController::getInstance()),
    mFMController(AudioALSAFMController::getInstance()),
    mAudioALSAVolumeController(AudioVolumeFactory::CreateAudioVolumeController()),
    mSpeechDriverFactory(SpeechDriverFactory::GetInstance()),
    mMicMute(false),
    mAudioMode(AUDIO_MODE_NORMAL),
    mAudioModePolicy(AUDIO_MODE_NORMAL),
    mEnterPhoneCallMode(false),
    mPhoneCallControllerStatusPolicy(false),
    mResumeAllStreamsAtRouting(false),
    mIsNeedResumeStreamOut(false),
    mLoopbackEnable(false),
    mHdmiEnable(false),
    mBesLoudnessStatus(false),
    mBesLoudnessControlCallback(NULL),
    mAudioSpeechEnhanceInfoInstance(AudioSpeechEnhanceInfo::getInstance()),
    mHeadsetChange(false),
#ifdef MTK_VOW_SUPPORT
    mAudioALSAVoiceWakeUpController(AudioALSAVoiceWakeUpController::getInstance()),
#else
    mAudioALSAVoiceWakeUpController(0),
#endif
    mVoiceWakeUpNeedOn(false),
    mForceDisableVoiceWakeUpForSetMode(false),
    mBypassPostProcessDL(false),
    mBypassDualMICProcessUL(false),
    mBtHeadsetName(NULL),
    mAvailableOutputDevices(AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_EARPIECE),
    mCustScene(""),
    mStreamManagerDestructing(false),
    mVolumeIndex(0),
    mAvailableOutputFlags(0) {
    ALOGD("%s()", __FUNCTION__);

    mOutputStreamForCall = NULL;
    mCurrentOutputDevicesForCall = AUDIO_DEVICE_NONE;
    mOutputDevicesForCall = AUDIO_DEVICE_NONE;

#ifdef MTK_AUDIO_DSP_RECOVERY_SUPPORT
    audio_dsp_cbk_register(
        audioDspReadyWrap,
        audioDspStopWrap,
        this);
#endif

#ifdef CONFIG_MT_ENG_BUILD
    mLogEnable = 1;
#else
    mLogEnable = __android_log_is_loggable(ANDROID_LOG_DEBUG, LOG_TAG, ANDROID_LOG_INFO);
#endif

    mStreamOutVector.clear();
    mStreamInVector.clear();

    mPlaybackHandlerVector.clear();
    mCaptureHandlerVector.clear();
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    mFilterManagerVector.clear();
#endif

    char property_value[PROPERTY_VALUE_MAX];

    mAudioCustParamClient = NULL;
    mAudioCustParamClient = AudioCustParamClient::GetInstance();

#ifdef MTK_BESLOUDNESS_SUPPORT
    property_get(PROPERTY_KEY_FIRSTBOOT_STATE, property_value, "1"); //"1": first boot, "0": not first boot
    int firstboot = atoi(property_value);
    if ((firstboot == 1) && AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        mBesLoudnessStatus = false;
        property_set(PROPERTY_KEY_FIRSTBOOT_STATE, "0");
        property_set(PROPERTY_KEY_BESLOUDNESS_SWITCH_STATE, "0");
    } else {
        if (firstboot == 1) {
            property_set(PROPERTY_KEY_FIRSTBOOT_STATE, "0");
            property_set(PROPERTY_KEY_BESLOUDNESS_SWITCH_STATE, BESLOUDNESS_SWITCH_DEFAULT_STATE ? "1" : "0");
        }
        property_get(PROPERTY_KEY_BESLOUDNESS_SWITCH_STATE, property_value, BESLOUDNESS_SWITCH_DEFAULT_STATE ? "1" : "0");
        int besloudness_enable = atoi(property_value);
        if (besloudness_enable) {
            mBesLoudnessStatus = true;
        } else {
            mBesLoudnessStatus = false;
        }
    }
    ALOGD("firstboot %d mBesLoudnessStatus %d", firstboot, mBesLoudnessStatus);
#else
    mBesLoudnessStatus = false;
    ALOGD("%s(), mBesLoudnessStatus [%d] (Always) \n", __FUNCTION__, mBesLoudnessStatus);
#endif
    AudioEventThreadManager::getInstance()->registerCallback(AUDIO_EVENT_PHONECALL_REOPEN,
                                                             callbackPhoneCallReopen, this);

    // get power hal service first to reduce time for later usage of power hal
    initPowerHal();

    mBluetoothAudioOffloadHostIf = NULL;
    mBluetoothAudioOffloadSession = 0;
    mA2dpSuspend = -1;
    mA2dpPlaybackPaused = false;
    // Set speaker change callback
    AudioALSAHardwareResourceManager::getInstance()->setSpeakerStatusChangeCb(speakerStatusChangeCb);

#ifdef MTK_AUDIODSP_SUPPORT
    AudioDspStreamManager::getInstance()->doRecoveryState();
#endif
}


AudioALSAStreamManager::~AudioALSAStreamManager() {
    ALOGD("%s()", __FUNCTION__);

    mStreamManagerDestructing = true;

    if (mBtHeadsetName) {
        free((void *)mBtHeadsetName);
        mBtHeadsetName = NULL;
    }

    mStreamManager = NULL;

    AudioEventThreadManager::getInstance()->unregisterCallback(AUDIO_EVENT_PHONECALL_REOPEN);
}


/*==============================================================================
 *                     Implementations
 *============================================================================*/

AudioMTKStreamOutInterface *AudioALSAStreamManager::openOutputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    uint32_t output_flag) {
    AL_AUTOLOCK(mStreamVectorLock);
    AL_AUTOLOCK(mLock);

    if (format == NULL || channels == NULL || sampleRate == NULL || status == NULL) {
        ALOGE("%s(), NULL pointer!! format = %p, channels = %p, sampleRate = %p, status = %p",
              __FUNCTION__, format, channels, sampleRate, status);
        if (status != NULL) { *status = INVALID_OPERATION; }
        return NULL;
    }

    ALOGD("+%s(), devices = 0x%x, format = 0x%x, channels = 0x%x, sampleRate = %d, status = 0x%x",
          __FUNCTION__, devices, *format, *channels, *sampleRate, *status);

    // stream out flags
#if 1 // TODO(Harvey): why.........
    mStreamOutIndex = (uint32_t)(*status);
#endif
    //const uint32_t flags = 0; //(uint32_t)(*status);

    // create stream out
    AudioALSAStreamOut *pAudioALSAStreamOut = new AudioALSAStreamOut();
    pAudioALSAStreamOut->set(devices, format, channels, sampleRate, status, output_flag);
    if (*status != NO_ERROR) {
        ALOGE("-%s(), set fail, return NULL", __FUNCTION__);
        delete pAudioALSAStreamOut;
        pAudioALSAStreamOut = NULL;
        return NULL;
    }

    // save stream out object in vector
#if 0 // TODO(Harvey): why.........
    pAudioALSAStreamOut->setIdentity(mStreamOutIndex);
    mStreamOutVector.add(mStreamOutIndex, pAudioALSAStreamOut);
    mStreamOutIndex++;
#else
    pAudioALSAStreamOut->setIdentity(mStreamOutIndex);
    mStreamOutVector.add(mStreamOutIndex, pAudioALSAStreamOut);
#endif

#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    // setup Filter for ACF/HCF/AudEnh/VibSPK // TODO Check return status of pAudioALSAStreamOut->set
    AudioMTKFilterManager *pAudioFilterManagerHandler = new AudioMTKFilterManager(*sampleRate, popcount(*channels), *format, pAudioALSAStreamOut->bufferSize());
    if (pAudioFilterManagerHandler != NULL) {
        if (pAudioFilterManagerHandler->init(output_flag) == NO_ERROR) {
            mFilterManagerVector.add(mStreamOutIndex, pAudioFilterManagerHandler);
        } else {
            delete pAudioFilterManagerHandler;
        }
    }
#endif

    mAvailableOutputFlags |= output_flag;

    ALOGD("-%s(), out = %p, status = 0x%x, mStreamOutVector.size() = %zu",
          __FUNCTION__, pAudioALSAStreamOut, *status, mStreamOutVector.size());


    return pAudioALSAStreamOut;
}

void AudioALSAStreamManager::closeOutputStream(AudioMTKStreamOutInterface *out) {
    ALOGD("+%s(), out = %p, mStreamOutVector.size() = %zu", __FUNCTION__, out, mStreamOutVector.size());
    AL_AUTOLOCK(mStreamVectorLock);
    AL_AUTOLOCK(mLock);

    if (out == NULL) {
        ALOGE("%s(), Cannot close null output stream!! return", __FUNCTION__);
        return;
    }

    AudioALSAStreamOut *pAudioALSAStreamOut = static_cast<AudioALSAStreamOut *>(out);
    ASSERT(pAudioALSAStreamOut != 0);

    uint32_t streamOutId = pAudioALSAStreamOut->getIdentity();

    mStreamOutVector.removeItem(streamOutId);
    delete pAudioALSAStreamOut;
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    uint32_t dFltMngindex = mFilterManagerVector.indexOfKey(streamOutId);

    if (dFltMngindex < mFilterManagerVector.size()) {
        AudioMTKFilterManager *pAudioFilterManagerHandler = static_cast<AudioMTKFilterManager *>(mFilterManagerVector[dFltMngindex]);
        ALOGD("%s, remove mFilterManagerVector Success [%d]/[%zu] [%d], pAudioFilterManagerHandler=%p",
              __FUNCTION__, dFltMngindex, mFilterManagerVector.size(), streamOutId, pAudioFilterManagerHandler);
        ASSERT(pAudioFilterManagerHandler != 0);
        mFilterManagerVector.removeItem(streamOutId);
        delete pAudioFilterManagerHandler;
    } else {
        ALOGD("%s, Remove mFilterManagerVector Error [%d]/[%zu]", __FUNCTION__, dFltMngindex, mFilterManagerVector.size());
    }
#endif
    ALOGD("-%s(), mStreamOutVector.size() = %zu", __FUNCTION__, mStreamOutVector.size());
}


AudioMTKStreamInInterface *AudioALSAStreamManager::openInputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    audio_in_acoustics_t acoustics,
    uint32_t input_flag) {
    AL_AUTOLOCK(mStreamVectorLock);
    AL_AUTOLOCK(mLock);

    if (format == NULL || channels == NULL || sampleRate == NULL || status == NULL) {
        ALOGE("%s(), NULL pointer!! format = %p, channels = %p, sampleRate = %p, status = %p",
              __FUNCTION__, format, channels, sampleRate, status);
        if (status != NULL) { *status = INVALID_OPERATION; }
        return NULL;
    }

    ALOGD("%s(), devices = 0x%x, format = 0x%x, channels = 0x%x, sampleRate = %d, status = %d, acoustics = 0x%x",
          __FUNCTION__, devices, *format, *channels, *sampleRate, *status, acoustics);

#if 1 // TODO(Harvey): why.........
    mStreamInIndex = (uint32_t)(*status);
#endif

    // create stream in
    AudioALSAStreamIn *pAudioALSAStreamIn = new AudioALSAStreamIn();
    audio_devices_t input_device = static_cast<audio_devices_t>(devices);
    int mNumPhoneMicSupport = AudioCustParamClient::GetInstance()->getNumMicSupport();

    if ((input_device == AUDIO_DEVICE_IN_BACK_MIC) && (mNumPhoneMicSupport < 2)) {
        input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        devices = static_cast<uint32_t>(input_device);
        ALOGW("%s(), not support back_mic if mic < 2, force to set input_device = 0x%x", __FUNCTION__, input_device);
    }

    bool sharedDevice = (input_device & ~AUDIO_DEVICE_BIT_IN) & (AUDIO_DEVICE_IN_BUILTIN_MIC | AUDIO_DEVICE_IN_BACK_MIC | AUDIO_DEVICE_IN_WIRED_HEADSET);
    //In PhonecallMode and the new input device is sharedDevice,we may do some check
    if ((isPhoneCallOpen() == true) && (sharedDevice == true)) {
        input_device = mSpeechPhoneCallController->getAdjustedInputDevice();
        sharedDevice = (input_device & ~AUDIO_DEVICE_BIT_IN) & (AUDIO_DEVICE_IN_BUILTIN_MIC | AUDIO_DEVICE_IN_BACK_MIC | AUDIO_DEVICE_IN_WIRED_HEADSET);
        if (sharedDevice == true) { //if phonecall_device also use sharedDevice, set the input_device = phonecall_device
            devices = static_cast<uint32_t>(input_device);
            ALOGD("+%s(), isPhoneCallOpen, force to set input_device = 0x%x", __FUNCTION__, input_device);
        }
    } else {
        if ((sharedDevice == true) && (mStreamInVector.size() > 0)) {
#ifdef MTK_CHECK_INPUT_DEVICE_PRIORITY
            input_device = CheckInputDevicePriority(input_device);
#endif
            devices = static_cast<uint32_t>(input_device);
            for (size_t i = 0; i < mStreamInVector.size(); i++) {
                sharedDevice = ((mStreamInVector[i]->getStreamAttribute()->input_device) & ~AUDIO_DEVICE_BIT_IN) & (AUDIO_DEVICE_IN_BUILTIN_MIC | AUDIO_DEVICE_IN_BACK_MIC | AUDIO_DEVICE_IN_WIRED_HEADSET);
                if ((sharedDevice == true) && ((mStreamInVector[i]->getStreamAttribute()->input_device) != input_device)) {
                    mStreamInVector[i]->routing(input_device);
                }
            }
        }
    }

#ifdef UPLINK_LOW_LATENCY
    pAudioALSAStreamIn->set(devices, format, channels, sampleRate, status, acoustics, input_flag);
#else
    pAudioALSAStreamIn->set(devices, format, channels, sampleRate, status, acoustics);
#endif
    if (*status != NO_ERROR) {
        ALOGE("-%s(), set fail, return NULL", __FUNCTION__);
        delete pAudioALSAStreamIn;
        pAudioALSAStreamIn = NULL;
        return NULL;
    }

    // save stream in object in vector
#if 0 // TODO(Harvey): why.........
    pAudioALSAStreamIn->setIdentity(mStreamInIndex);
    mStreamInVector.add(mStreamInIndex, pAudioALSAStreamIn);
    mStreamInIndex++;
#else
    pAudioALSAStreamIn->setIdentity(mStreamInIndex);
    mStreamInVector.add(mStreamInIndex, pAudioALSAStreamIn);
#endif

    ALOGD("-%s(), in = %p, status = 0x%x, mStreamInVector.size() = %zu",
          __FUNCTION__, pAudioALSAStreamIn, *status, mStreamInVector.size());
    return pAudioALSAStreamIn;
}


void AudioALSAStreamManager::closeInputStream(AudioMTKStreamInInterface *in) {
    ALOGD("+%s(), in = %p, size() = %zu", __FUNCTION__, in, mStreamInVector.size());
    AL_AUTOLOCK(mStreamVectorLock);
    AL_AUTOLOCK(mLock);

    if (in == NULL) {
        ALOGE("%s(), Cannot close null input stream!! return", __FUNCTION__);
        return;
    }

    AudioALSAStreamIn *pAudioALSAStreamIn = static_cast<AudioALSAStreamIn *>(in);
    ASSERT(pAudioALSAStreamIn != 0);

#ifdef MTK_VOW_SUPPORT
    // close HOTWORD training
    if ((mStreamInVector.size() == 1) &&
        (mStreamInVector[0]->getStreamAttribute()->input_source == AUDIO_SOURCE_HOTWORD) &&
        (mVoiceWakeUpNeedOn == false)) {
        if (mAudioALSAVoiceWakeUpController->getVoiceWakeUpEnable() == true) {
            ALOGD("%s(), close HOTWORD training", __FUNCTION__);
            mAudioALSAVoiceWakeUpController->setVoiceWakeUpEnable(false);
        }
    }
#endif

    mStreamInVector.removeItem(pAudioALSAStreamIn->getIdentity());
    delete pAudioALSAStreamIn;

#ifdef MTK_VOW_SUPPORT
    // make sure voice wake up is resume when all capture stream stop if need
    if (mVoiceWakeUpNeedOn == true &&
        mForceDisableVoiceWakeUpForSetMode == false &&
        mStreamInVector.size() == 0) {
        ALOGD("%s(), resume voice wake up", __FUNCTION__);
        //ASSERT(mAudioALSAVoiceWakeUpController->getVoiceWakeUpEnable() == false); // TODO(Harvey): double check, remove it later
        if (mAudioALSAVoiceWakeUpController->getVoiceWakeUpEnable() == false) {
            mAudioALSAVoiceWakeUpController->setVoiceWakeUpEnable(true);
        }
    }
#endif

    if (mStreamInVector.size() == 0) {
        mAudioSpeechEnhanceInfoInstance->SetHifiRecord(false);
    } else {
        bool bClear = true;
        for (size_t i = 0; i < mStreamInVector.size(); i++) {
            pAudioALSAStreamIn = mStreamInVector[i];

            if (pAudioALSAStreamIn->getStreamInCaptureHandler() == NULL) {
                ALOGD("%s(), mStreamInVector[%zu] capture handler close already", __FUNCTION__, i);
                continue;
            }

            if (pAudioALSAStreamIn->getStreamInCaptureHandler()->getCaptureHandlerType() == CAPTURE_HANDLER_NORMAL) {
                bClear = false;
                break;
            }
        }
        if (bClear) { //if still has Normal capture handler, not to reset hifi record status.
            mAudioSpeechEnhanceInfoInstance->SetHifiRecord(false);
        }
    }

    ALOGD("-%s(), mStreamInVector.size() = %zu", __FUNCTION__, mStreamInVector.size());
}

void dlStreamAttributeSourceCustomization(stream_attribute_t *streamAttribute) {
    if (!strcmp(streamAttribute->mCustScene, "App1")) {
        /* App1 Scene customization */
    } if (!strcmp(streamAttribute->mCustScene, "App2")) {
        /* App2 Scene customization: Music playback will using VoIP DL processing */
        streamAttribute->mVoIPEnable = true;
        ALOGD("%s(), Scene is App2, enable VoIP DL processing", __FUNCTION__);
    }
}

AudioALSAPlaybackHandlerBase *AudioALSAStreamManager::createPlaybackHandler(
    stream_attribute_t *stream_attribute_source) {
    ALOGD("+%s(), mAudioMode = %d, output_devices = 0x%x, isMixerOut = 0x%x, flag = %d",
          __FUNCTION__, mAudioMode, stream_attribute_source->output_devices,
          stream_attribute_source->isMixerOut, stream_attribute_source->mAudioOutputFlags);

    AL_AUTOLOCK(mAudioModeLock);

    // Init input stream attribute here
    stream_attribute_source->audio_mode = mAudioMode; // set mode to stream attribute for mic gain setting
    stream_attribute_source->mVoIPEnable = needEnableVoip(stream_attribute_source);

    // just use what stream out is ask to use
    //stream_attribute_source->sample_rate = AudioALSASampleRateController::getInstance()->getPrimaryStreamOutSampleRate();

    //for DMNR tuning
    stream_attribute_source->BesRecord_Info.besrecord_dmnr_tuningEnable = mAudioSpeechEnhanceInfoInstance->IsAPDMNRTuningEnable();
    stream_attribute_source->bBypassPostProcessDL = mBypassPostProcessDL;
    strncpy(stream_attribute_source->mCustScene, mCustScene.string(), SCENE_NAME_LEN_MAX - 1);

    //todo:: enable ACF if support
    if (stream_attribute_source->sample_rate > 48000) {
        stream_attribute_source->bBypassPostProcessDL = true;
    }

    dlStreamAttributeSourceCustomization(stream_attribute_source);

    // create
    AudioALSAPlaybackHandlerBase *pPlaybackHandler = NULL;
    if (isPhoneCallOpen() == true) {
        switch (stream_attribute_source->output_devices) {
#ifdef MTK_MAXIM_SPEAKER_SUPPORT
        case AUDIO_DEVICE_OUT_SPEAKER_SAFE: {
            stream_attribute_source->bBypassPostProcessDL = true;
            stream_attribute_source->sample_rate = 16000;
            pPlaybackHandler = new AudioALSAPlaybackHandlerSpeakerProtection(stream_attribute_source);
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
            uint32_t dFltMngindex = mFilterManagerVector.indexOfKey(stream_attribute_source->mStreamOutIndex);
            ALOGD("%s(), ApplyFilter [%u]/[%zu] Device [0x%x]", __FUNCTION__, dFltMngindex, mFilterManagerVector.size(), stream_attribute_source->output_devices);
            if (dFltMngindex < mFilterManagerVector.size()) {
                pPlaybackHandler->setFilterMng(static_cast<AudioMTKFilterManager *>(mFilterManagerVector[dFltMngindex]));
                mFilterManagerVector[dFltMngindex]->setDevice(stream_attribute_source->output_devices);
            }
#endif
            break;
        }
#endif
        case AUDIO_DEVICE_OUT_AUX_DIGITAL:
            pPlaybackHandler = new AudioALSAPlaybackHandlerHDMI(stream_attribute_source);
            break;
        default:
            pPlaybackHandler = new AudioALSAPlaybackHandlerVoice(stream_attribute_source);
            break;
        }
    } else {
        switch (stream_attribute_source->output_devices) {
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT: {
            if (!stream_attribute_source->isMixerOut) {
                pPlaybackHandler = new AudioALSAPlaybackHandlerMixer(stream_attribute_source);
            } else {
                if (WCNChipController::GetInstance()->IsBTMergeInterfaceSupported() == true) {
                    pPlaybackHandler = new AudioALSAPlaybackHandlerBTSCO(stream_attribute_source);
                } else {
                    pPlaybackHandler = new AudioALSAPlaybackHandlerBTCVSD(stream_attribute_source);
                }
            }
            break;
        }
        case AUDIO_DEVICE_OUT_AUX_DIGITAL: {
            pPlaybackHandler = new AudioALSAPlaybackHandlerHDMI(stream_attribute_source);
            break;
        }
        case AUDIO_DEVICE_OUT_FM: {
            pPlaybackHandler = new AudioALSAPlaybackHandlerFMTransmitter(stream_attribute_source);
            break;
        }
        case AUDIO_DEVICE_OUT_EARPIECE:
        case AUDIO_DEVICE_OUT_WIRED_HEADSET:
        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
        case AUDIO_DEVICE_OUT_SPEAKER:
        default: {
            if (isBtSpkDevice(stream_attribute_source->output_devices)) {
                if (!stream_attribute_source->isMixerOut) {
                    pPlaybackHandler = new AudioALSAPlaybackHandlerMixer(stream_attribute_source);
                    break;
                }
            }

#if !defined(MTK_BASIC_PACKAGE)
            if (AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD & stream_attribute_source->mAudioOutputFlags) {
                pPlaybackHandler = new AudioALSAPlaybackHandlerOffload(stream_attribute_source);
                break;
            } else
#endif
            {
#if defined(MTK_MAXIM_SPEAKER_SUPPORT) || (MTK_AUDIO_SMARTPASCP_SUPPORT)
                if (AudioSmartPaController::getInstance()->isSwDspSpkProtect(stream_attribute_source->output_devices)) {
                    if (!stream_attribute_source->isMixerOut) {
                        pPlaybackHandler = new AudioALSAPlaybackHandlerMixer(stream_attribute_source);
                        break;
                    }
#if defined(MTK_MAXIM_SPEAKER_SUPPORT)
                    if (AudioSmartPaController::getInstance()->getSpkProtectType() == SPK_AP_DSP) {
                        pPlaybackHandler = new AudioALSAPlaybackHandlerSpeakerProtection(stream_attribute_source);
                        break;
                    }
#elif defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
                    if (AudioSmartPaController::getInstance()->getSpkProtectType() == SPK_APSCP_DSP) {
                        pPlaybackHandler = new AudioALSAPlaybackHandlerSpeakerProtectionDsp(stream_attribute_source);
                        break;
                    }
#endif
                    ALOGE("%s(), open SmartPA playbackHandler error! use PlaybackHandlerNormal", __FUNCTION__);
                    pPlaybackHandler = new AudioALSAPlaybackHandlerNormal(stream_attribute_source);
                    ASSERT(0);
                    break;
                } else
#endif // end of #if defined(MTK_MAXIM_SPEAKER_SUPPORT) || (MTK_AUDIO_SMARTPASCP_SUPPORT)
                {
#ifdef DOWNLINK_LOW_LATENCY
                    if (AUDIO_OUTPUT_FLAG_FAST & stream_attribute_source->mAudioOutputFlags &&
                        !(AUDIO_OUTPUT_FLAG_PRIMARY & stream_attribute_source->mAudioOutputFlags)) {
                        pPlaybackHandler = new AudioALSAPlaybackHandlerFast(stream_attribute_source);
                        break;
                    }
#if defined(MTK_AUDIO_AAUDIO_SUPPORT)
                    else if (AUDIO_OUTPUT_FLAG_MMAP_NOIRQ & stream_attribute_source->mAudioOutputFlags) {
                        pPlaybackHandler = new AudioALSAPlaybackHandlerAAudio(stream_attribute_source);
                        break;
                    }
#endif
                    else
#endif
                    {
                        if (AudioSmartPaController::getInstance()->isInCalibration()) {
                            pPlaybackHandler = new AudioALSAPlaybackHandlerNormal(stream_attribute_source);
                            break;
                        }
#if defined(MTK_AUDIODSP_SUPPORT)
                        if (AudioDspStreamManager::getInstance()->getDspOutHandlerEnable(stream_attribute_source->mAudioOutputFlags)) {
                            pPlaybackHandler = new AudioALSAPlaybackHandlerDsp(stream_attribute_source);
                        } else {
                            pPlaybackHandler = new AudioALSAPlaybackHandlerNormal(stream_attribute_source);
                        }
                        break;
#else
                        pPlaybackHandler = new AudioALSAPlaybackHandlerNormal(stream_attribute_source);
                        break;
#endif
                    }
                }
            }
        }
        }
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
        uint32_t dFltMngindex = mFilterManagerVector.indexOfKey(stream_attribute_source->mStreamOutIndex);
        ALOGV("%s(), ApplyFilter [%u]/[%zu] Device [0x%x]", __FUNCTION__, dFltMngindex, mFilterManagerVector.size(), stream_attribute_source->output_devices);

        if (dFltMngindex < mFilterManagerVector.size()) {
            pPlaybackHandler->setFilterMng(static_cast<AudioMTKFilterManager *>(mFilterManagerVector[dFltMngindex]));
            mFilterManagerVector[dFltMngindex]->setDevice(stream_attribute_source->output_devices);
        }
#endif
    }

    // save playback handler object in vector
    if (pPlaybackHandler) {
        pPlaybackHandler->setIdentity(mPlaybackHandlerIndex);

        AL_LOCK(mPlaybackHandlerVectorLock);
        mPlaybackHandlerVector.add(mPlaybackHandlerIndex, pPlaybackHandler);
        AL_UNLOCK(mPlaybackHandlerVectorLock);

        mPlaybackHandlerIndex++;

#if defined(MTK_AUDIO_SW_DRE) && defined(MTK_NEW_VOL_CONTROL)
        AudioMTKGainController::getInstance()->registerPlaybackHandler(pPlaybackHandler->getIdentity());
#endif
    } else {
        ASSERT(pPlaybackHandler != NULL);
    }

    ALOGD_IF(mLogEnable, "-%s(), mPlaybackHandlerVector.size() = %zu", __FUNCTION__, mPlaybackHandlerVector.size());
    return pPlaybackHandler;
}


status_t AudioALSAStreamManager::destroyPlaybackHandler(AudioALSAPlaybackHandlerBase *pPlaybackHandler) {
    ALOGV("+%s(), mode = %d, pPlaybackHandler = %p", __FUNCTION__, mAudioMode, pPlaybackHandler);
    //AL_AUTOLOCK(mLock); // TODO(Harvey): setparam -> routing -> close -> destroy deadlock

    status_t status = NO_ERROR;

#if defined(MTK_AUDIO_SW_DRE) && defined(MTK_NEW_VOL_CONTROL)
    AudioMTKGainController::getInstance()->removePlaybackHandler(pPlaybackHandler->getIdentity());
#endif

    AL_LOCK(mPlaybackHandlerVectorLock);
    mPlaybackHandlerVector.removeItem(pPlaybackHandler->getIdentity());
    AL_UNLOCK(mPlaybackHandlerVectorLock);

    ALOGD_IF(mLogEnable, "-%s(), mode = %d, pPlaybackHandler = %p, mPlaybackHandlerVector.size() = %zu",
             __FUNCTION__, mAudioMode, pPlaybackHandler, mPlaybackHandlerVector.size());

    delete pPlaybackHandler;

    return status;
}

void ulStreamAttributeTargetCustomization(stream_attribute_t *streamAttribute) {
    if (!strcmp((char *)streamAttribute->mCustScene, "App1")) {
        /* App1 Scene customization */
    } else if (!strcmp((char *)streamAttribute->mCustScene, "App2")) {
        /* App2 Scene customization: normal record will using VoIP processing */
        if (streamAttribute->input_source == AUDIO_SOURCE_MIC) {
            streamAttribute->input_source = AUDIO_SOURCE_VOICE_COMMUNICATION;
            ALOGD("%s(), Scene is App2, replace MIC input source with communication", __FUNCTION__);
        }
    } else if (!strcmp(streamAttribute->mCustScene, "ASR")) {
        /* ASR Scene customization: Voice recognition + AEC processing */
        if (streamAttribute->input_source == AUDIO_SOURCE_VOICE_RECOGNITION) {
            // Change VR input source to MagiASR(81), ASR parameter is already built-in in XML
            streamAttribute->input_source = AUDIO_SOURCE_CUSTOMIZATION1;
        }
        ALOGD("%s(), Scene is ASR, input source is %d", __FUNCTION__, streamAttribute->input_source);
    } else if (!strcmp(streamAttribute->mCustScene, "AEC_REC")) {
        /* AEC_REC Scene customization: Normal record + AEC processing */
        if (streamAttribute->input_source == AUDIO_SOURCE_MIC) {
            // Change normal record input source with CUSTOMIZATION2(82), CUSTOMIZATION2 parameter is already built-in in XML
            streamAttribute->input_source = AUDIO_SOURCE_CUSTOMIZATION2;
        }
        ALOGD("%s(), Scene is AEC_REC, input source is %d", __FUNCTION__, streamAttribute->input_source);
    }
}

bool AudioALSAStreamManager::isCaptureOffload(stream_attribute_t *stream_attribute_target __unused)
{
    bool offload = false;

#ifdef MTK_AUDIODSP_SUPPORT
    if (AudioDspStreamManager::getInstance()->getDspInHandlerEnable(stream_attribute_target->mAudioInputFlags)) {
#if (defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT) && (MTK_AUDIO_TUNING_TOOL_V2_PHASE >= 2))
        const char* paramPath = "";
        if (stream_attribute_target->mVoIPEnable) {
            paramPath = VOIP_PATH;
        } else if (stream_attribute_target->mAudioInputFlags & AUDIO_INPUT_FLAG_FAST) {
            switch(stream_attribute_target->input_source) {
            case AUDIO_SOURCE_VOICE_PERFORMANCE:
                paramPath = VOICE_PERFORMANCE_PATH;
                break;
            default:
                paramPath = FAST_RECORD_PATH;
                break;
            }
        } else {
            switch(stream_attribute_target->input_source) {
            case AUDIO_SOURCE_CAMCORDER:
                paramPath = CAMCORDER_PATH;
                break;
            case AUDIO_SOURCE_VOICE_RECOGNITION:
                paramPath = VOICE_RECOGNITION_PATH;
                break;
            case AUDIO_SOURCE_VOICE_COMMUNICATION:
                paramPath = VOIP_PATH;
                break;
            case AUDIO_SOURCE_UNPROCESSED:
                paramPath = UNPROCESSED_PATH;
                break;
            case AUDIO_SOURCE_VOICE_PERFORMANCE:
                paramPath = VOICE_PERFORMANCE_PATH;
                break;
            case AUDIO_SOURCE_DEFAULT:
            case AUDIO_SOURCE_MIC:
            default:
                paramPath = MIC_PATH;
                break;
            }
        }

        AppOps *appOps = appOpsGetInstance();
        if (appOps == NULL) {
            ASSERT(appOps);
            return true;
        }

        AudioType *audioType = appOps->appHandleGetAudioTypeByName(appOps->appHandleGetInstance(), UPLINK_CUSTOMIZATION_AUDIO_TYPE);
        if (!audioType) {
            return true;
        }

        ParamUnit *paramUnit = appOps->audioTypeGetParamUnit(audioType, paramPath);
        if (!paramUnit) {
            return true;
        }

        Param *param = appOps->paramUnitGetParamByName(paramUnit, OFFLOAD_PARAM_NAME);
        if (!param) {
            offload = true;
        } else {
            offload = *((int*)param->data);
            ALOGD("%s(), offload customization is %d (%s)\n", __FUNCTION__, offload, paramPath);
        }
#else
        /* No XML support, default offload enabled */
        offload = true;
#endif
    }
#endif

    return offload;
}

AudioALSACaptureHandlerBase *AudioALSAStreamManager::createCaptureHandler(
    stream_attribute_t *stream_attribute_target) {
    ALOGD("+%s(), mAudioMode = %d, input_source = %d, input_device = 0x%x, mBypassDualMICProcessUL=%d, sample_rate=%d",
          __FUNCTION__, mAudioMode, stream_attribute_target->input_source, stream_attribute_target->input_device, mBypassDualMICProcessUL, stream_attribute_target->sample_rate);
    //AL_AUTOLOCK(mLock);
    status_t retval = AL_LOCK_MS_NO_ASSERT(mLock, 1000);
    if (retval != NO_ERROR) {
        ALOGD("mLock timeout : 1s , return NULL");
        return NULL;
    }

    // use primary stream out device as default
    audio_devices_t current_output_devices = (mStreamOutVector.size() > 0) ?
                                             mStreamOutVector[0]->getStreamAttribute()->output_devices :
                                             AUDIO_DEVICE_NONE;

    // use active stream out device
    for (size_t i = 0; i < mStreamOutVector.size(); i++) {
        if (mStreamOutVector[i]->isOutPutStreamActive()) {
            current_output_devices = mStreamOutVector[i]->getStreamAttribute()->output_devices;
            break;
        }
    }

    if (isBtSpkDevice(current_output_devices)) {
        // use SPK setting for BTSCO + SPK
        current_output_devices = (audio_devices_t)(current_output_devices & (~AUDIO_DEVICE_OUT_ALL_SCO));
    }

    // Init input stream attribute here
    stream_attribute_target->audio_mode = mAudioMode; // set mode to stream attribute for mic gain setting
    stream_attribute_target->output_devices = current_output_devices; // set output devices to stream attribute for mic gain setting and BesRecord parameter
    stream_attribute_target->micmute = mMicMute;
    strncpy(stream_attribute_target->mCustScene, mCustScene.string(), SCENE_NAME_LEN_MAX - 1);

    // BesRecordInfo
    stream_attribute_target->BesRecord_Info.besrecord_enable = false; // default set besrecord off
    stream_attribute_target->BesRecord_Info.besrecord_bypass_dualmicprocess = mBypassDualMICProcessUL;  // bypass dual MIC preprocess
    stream_attribute_target->BesRecord_Info.besrecord_voip_enable = false;
    stream_attribute_target->mVoIPEnable = false;
    ALOGD_IF(mLogEnable, "%s(), ulStreamAttributeTargetCustomization", __FUNCTION__);

    /* StreamAttribute customization for scene */
    ulStreamAttributeTargetCustomization(stream_attribute_target);

    ALOGD_IF(mLogEnable, "%s(), ulStreamAttributeTargetCustomization done", __FUNCTION__);
    // create
    AudioALSACaptureHandlerBase *pCaptureHandler = NULL;
#if 0
#if defined(MTK_SPEAKER_MONITOR_SPEECH_SUPPORT)
    if (stream_attribute_target->input_device == AUDIO_DEVICE_IN_SPK_FEED) {
        pCaptureHandler = new AudioALSACaptureHandlerModemDai(stream_attribute_target);
    } else
#endif
#endif
    {
        if (stream_attribute_target->input_source == AUDIO_SOURCE_FM_TUNER
#if defined(SUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL)
            || stream_attribute_target->input_source == AUDIO_SOURCE_ODM_FM_RX
#endif
           ) {
            if (isEchoRefUsing() == true) {
                ALOGD("%s(), not support FM record in VoIP mode, return NULL", __FUNCTION__);
                AL_UNLOCK(mLock);
                return NULL;
            }

            pCaptureHandler = new AudioALSACaptureHandlerFMRadio(stream_attribute_target);
        } else if (stream_attribute_target->input_device == AUDIO_DEVICE_IN_BUS) {
            pCaptureHandler = new AudioALSACaptureHandlerVoice(stream_attribute_target);
#if defined(MTK_VOW_SUPPORT)
        } else if (stream_attribute_target->input_source == AUDIO_SOURCE_HOTWORD) {
            if (mForceDisableVoiceWakeUpForSetMode == true) {
                ALOGD("%s(), ForceDisableVoiceWakeUpForSetMode, return", __FUNCTION__);
                AL_UNLOCK(mLock);
                return NULL;
            } else {
                if (mAudioALSAVoiceWakeUpController->getVoiceWakeUpEnable() == false) {
                    mAudioALSAVoiceWakeUpController->setVoiceWakeUpEnable(true);
                }
                if (mVoiceWakeUpNeedOn == true) {
                    mAudioALSAVoiceWakeUpController->SeamlessRecordEnable();
                }
                pCaptureHandler = new AudioALSACaptureHandlerVOW(stream_attribute_target);
            }
#endif
        } else if (stream_attribute_target->input_source == AUDIO_SOURCE_VOICE_UNLOCK ||
                   stream_attribute_target->input_source == AUDIO_SOURCE_ECHO_REFERENCE) {
            if (stream_attribute_target->input_source == AUDIO_SOURCE_ECHO_REFERENCE && isEchoRefUsing() == true) {
                ALOGD("%s(), not support EchoRef  record in VoIP mode, return NULL", __FUNCTION__);
                AL_UNLOCK(mLock);
                return NULL;
            }
            pCaptureHandler = new AudioALSACaptureHandlerSyncIO(stream_attribute_target);
        } else if (isPhoneCallOpen() == true) {
            pCaptureHandler = new AudioALSACaptureHandlerVoice(stream_attribute_target);
        } else if ((isPhoneCallOpen() == false) && ((isModeInVoipCall() == true) || (stream_attribute_target->NativePreprocess_Info.PreProcessEffect_AECOn == true)
                                                    || (stream_attribute_target->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION)
                                                    || (stream_attribute_target->input_source == AUDIO_SOURCE_CUSTOMIZATION1) //MagiASR enable AEC
                                                    || (stream_attribute_target->input_source == AUDIO_SOURCE_CUSTOMIZATION2))) { //Normal REC with AEC
            stream_attribute_target->BesRecord_Info.besrecord_enable = EnableBesRecord();
            if (mStreamInVector.size() > 1) {
                for (size_t i = 0; i < mStreamInVector.size(); i++) {
                    if (mStreamInVector[i]->getStreamAttribute()->input_source == AUDIO_SOURCE_FM_TUNER
#if defined(SUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL)
                        || mStreamInVector[i]->getStreamAttribute()->input_source == AUDIO_SOURCE_ODM_FM_RX
#endif
                       ) {
                        mStreamInVector[i]->standby();
                    }
                }
            }
            if (isModeInVoipCall() == true || (stream_attribute_target->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION)) {
                stream_attribute_target->BesRecord_Info.besrecord_voip_enable = true;
                stream_attribute_target->mVoIPEnable = true;
#if 0
                if (current_output_devices == AUDIO_DEVICE_OUT_SPEAKER) {
                    if (stream_attribute_target->input_device == AUDIO_DEVICE_IN_BUILTIN_MIC) {
                        if (USE_REFMIC_IN_LOUDSPK == 1) {
                            ALOGD("%s(), routing changed!! input_device: 0x%x => 0x%x",
                                  __FUNCTION__, stream_attribute_target->input_device, AUDIO_DEVICE_IN_BACK_MIC);
                            stream_attribute_target->input_device = AUDIO_DEVICE_IN_BACK_MIC;
                        }
                    }
                }
#endif
            }
#if defined(MTK_AURISYS_FRAMEWORK_SUPPORT) && defined(MTK_CUS2_AUDIO_SOURCE_REPLACE_AEC_EFFECT)
            else if (stream_attribute_target->NativePreprocess_Info.PreProcessEffect_AECOn == true
                     && stream_attribute_target->input_source == AUDIO_SOURCE_MIC) {
                // Not VoIP/AEC input source but AEC effect enabled, using customization2 to do AEC processing
                stream_attribute_target->input_source = AUDIO_SOURCE_CUSTOMIZATION2;
                ALOGD("Normal record && AEC enabled, set the input source with %d", stream_attribute_target->input_source);
            }
#endif
            switch (stream_attribute_target->input_device) {
            case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET: {
#if defined(MTK_AURISYS_FRAMEWORK_SUPPORT) || defined(MTK_SUPPORT_BTCVSD_ALSA)
                /* Only BT ALSA+Aurisys arch support AEC processing */
                if (stream_attribute_target->output_devices & AUDIO_DEVICE_OUT_ALL_SCO) {
                    pCaptureHandler = new AudioALSACaptureHandlerAEC(stream_attribute_target);
                } else {
                    pCaptureHandler = new AudioALSACaptureHandlerBT(stream_attribute_target);
                }
#else
                pCaptureHandler = new AudioALSACaptureHandlerBT(stream_attribute_target);
#endif
                break;
            }
            default: {
#ifdef MTK_AUDIODSP_SUPPORT
                if (isCaptureOffload(stream_attribute_target)) {
                    pCaptureHandler = new AudioALSACaptureHandlerDsp(stream_attribute_target);
                } else
#endif
                {
                    pCaptureHandler = new AudioALSACaptureHandlerAEC(stream_attribute_target);
                }
                break;
            }
            }
        } else {
            //enable BesRecord if not these input sources
            if ((stream_attribute_target->input_source != AUDIO_SOURCE_VOICE_UNLOCK) &&
                (stream_attribute_target->input_source != AUDIO_SOURCE_FM_TUNER
#if defined(SUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL)
                 && stream_attribute_target->input_source != AUDIO_SOURCE_ODM_FM_RX
#endif
                )) { // TODO(Harvey, Yu-Hung): never go through here?
#if 0   //def UPLINK_LOW_LATENCY
                if ((stream_attribute_target->mAudioInputFlags & AUDIO_INPUT_FLAG_FAST) || (stream_attribute_target->sample_rate > 48000)) {
                    stream_attribute_target->BesRecord_Info.besrecord_enable = false;
                } else {
                    stream_attribute_target->BesRecord_Info.besrecord_enable = EnableBesRecord();
                }
#else
                if (stream_attribute_target->sample_rate > 48000) { //no uplink preprocess for sample rate higher than 48k
                    stream_attribute_target->BesRecord_Info.besrecord_enable = false;
                } else {
                    stream_attribute_target->BesRecord_Info.besrecord_enable = EnableBesRecord();
                }
#endif
            }

            bool bReCreate = false;
            if ((stream_attribute_target->sample_rate > 48000) && !mAudioSpeechEnhanceInfoInstance->GetHifiRecord()) { //no HifiRecord ongoing, and need to create HiFiRecord
                mAudioSpeechEnhanceInfoInstance->SetHifiRecord(true);
                if (mCaptureHandlerVector.size() > 0) { //already has another streamin ongoing with CAPTURE_HANDLER_NORMAL
                    for (size_t i = 0; i < mCaptureHandlerVector.size(); i++) {
                        if (mCaptureHandlerVector[i]->getCaptureHandlerType() == CAPTURE_HANDLER_NORMAL) {
                            bReCreate = true;
                            break;
                        }
                    }
                }
                if (bReCreate) { //need to re-create related capture handler for dataprovider reopen and dataclient SRC set.
                    ALOGD("%s(), reCreate streamin for hifi record +", __FUNCTION__);
                    //only suspend and standby CAPTURE_HANDLER_NORMAL streamin
                    setAllInputStreamsSuspend(true, false, CAPTURE_HANDLER_NORMAL);
                    standbyAllInputStreams(false, CAPTURE_HANDLER_NORMAL);
                    setAllInputStreamsSuspend(false, false, CAPTURE_HANDLER_NORMAL);
                    ALOGD("%s(), reCreate streamin for hifi record -", __FUNCTION__);
                }
            }

            switch (stream_attribute_target->input_device) {
            case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET: {
                pCaptureHandler = new AudioALSACaptureHandlerBT(stream_attribute_target);
                break;
            }
#if defined(MTK_TDM_SUPPORT)
            case AUDIO_DEVICE_IN_HDMI: {
                pCaptureHandler = new AudioALSACaptureHandlerTDM(stream_attribute_target);
                break;
            }
#endif
            case AUDIO_DEVICE_IN_BUILTIN_MIC:
            case AUDIO_DEVICE_IN_BACK_MIC:
            case AUDIO_DEVICE_IN_WIRED_HEADSET:
            default: {
                if (AudioSmartPaController::getInstance()->isInCalibration()) {
                    pCaptureHandler = new AudioALSACaptureHandlerNormal(stream_attribute_target);
                    break;
                }
#ifdef MTK_AUDIODSP_SUPPORT
                if (isCaptureOffload(stream_attribute_target) &&
                    !(AUDIO_INPUT_FLAG_MMAP_NOIRQ & stream_attribute_target->mAudioInputFlags)) {
                    if (isPhoneCallOpen() == true) {
                        pCaptureHandler = new AudioALSACaptureHandlerVoice(stream_attribute_target);
                    } else {
                        pCaptureHandler = new AudioALSACaptureHandlerDsp(stream_attribute_target);
                    }
                } else
#endif
                {
#if defined(MTK_AUDIO_AAUDIO_SUPPORT)
                    if (AUDIO_INPUT_FLAG_MMAP_NOIRQ & stream_attribute_target->mAudioInputFlags) {
                        pCaptureHandler = new AudioALSACaptureHandlerAAudio(stream_attribute_target);
                    } else
#endif
                    {
                        pCaptureHandler = new AudioALSACaptureHandlerNormal(stream_attribute_target);
                    }
                }
                break;
            }
            }
        }
    }
#if defined(MTK_VOW_SUPPORT)
    if (stream_attribute_target->input_source != AUDIO_SOURCE_HOTWORD) {
        if (mAudioALSAVoiceWakeUpController->getVoiceWakeUpEnable() == true) {
            ALOGI("Not Hotword Record,Actually Force Close VOW");
            mAudioALSAVoiceWakeUpController->setVoiceWakeUpEnable(false);
        }
    }
#endif
    // save capture handler object in vector
    ASSERT(pCaptureHandler != NULL);
    pCaptureHandler->setIdentity(mCaptureHandlerIndex);
    mCaptureHandlerVector.add(mCaptureHandlerIndex, pCaptureHandler);
    mCaptureHandlerIndex++;
    AL_UNLOCK(mLock);
    ALOGD_IF(mLogEnable, "-%s(), mCaptureHandlerVector.size() = %zu", __FUNCTION__, mCaptureHandlerVector.size());
    return pCaptureHandler;
}


status_t AudioALSAStreamManager::destroyCaptureHandler(AudioALSACaptureHandlerBase *pCaptureHandler) {
    ALOGD_IF(mLogEnable, "%s(), mode = %d, pCaptureHandler = %p", __FUNCTION__, mAudioMode, pCaptureHandler);
    //AL_AUTOLOCK(mLock); // TODO(Harvey): setparam -> routing -> close -> destroy deadlock

    status_t status = NO_ERROR;

    mCaptureHandlerVector.removeItem(pCaptureHandler->getIdentity());
    delete pCaptureHandler;

    ALOGD_IF(mLogEnable, "-%s(), mCaptureHandlerVector.size() = %zu", __FUNCTION__, mCaptureHandlerVector.size());
    return status;
}


status_t AudioALSAStreamManager::setVoiceVolume(float volume) {
    ALOGD("%s(), volume = %f", __FUNCTION__, volume);

    if (volume < 0.0 || volume > 1.0) {
        ALOGE("-%s(), strange volume level %f, something wrong!!", __FUNCTION__, volume);
        return BAD_VALUE;
    }

    AL_AUTOLOCK(mLock);

    if (mAudioALSAVolumeController) {
        // match volume to volume index
        int volumeIndex = (int)(volume * 5) + 1;
        // use primary stream out device
        const audio_devices_t current_output_devices = (mStreamOutVector.size() > 0)
                                                       ? mStreamOutVector[0]->getStreamAttribute()->output_devices
                                                       : AUDIO_DEVICE_NONE;
        mAudioALSAVolumeController->setVoiceVolumeIndex(volumeIndex);
        mAudioALSAVolumeController->setVoiceVolume(volume, getModeForGain(), current_output_devices);
        AudioALSASpeechPhoneCallController::getInstance()->updateVolume();
    }

    return NO_ERROR;
}

#ifdef MTK_AUDIO_GAIN_TABLE
status_t AudioALSAStreamManager::setAnalogVolume(int stream, int device, int index, bool force_incall) {
    ALOGV("%s(),stream=%d, device=%d, index=%d", __FUNCTION__, stream, device, index);

    AL_AUTOLOCK(mLock);

    if (mAudioALSAVolumeController) {
        if (force_incall == 0) {
            mAudioALSAVolumeController->setAnalogVolume(stream, device, index, getModeForGain());
        } else {
            mAudioALSAVolumeController->setAnalogVolume(stream, device, index, AUDIO_MODE_IN_CALL);
        }
    }

    return NO_ERROR;
}

int AudioALSAStreamManager::SetCaptureGain(void) {
    const stream_attribute_t *mStreamAttributeTarget;
    uint32_t i;
    ALOGD("%s(), mStreamInVector.size() = %zu", __FUNCTION__, mStreamInVector.size());

    for (i = 0; i < mStreamInVector.size(); i++) {
        //if(mStreamInVector[i]->getStreamAttribute()->output_devices == output_devices)
        {
            mStreamAttributeTarget = mStreamInVector[i]->getStreamAttribute();
            if (mAudioALSAVolumeController != NULL) {
                mAudioALSAVolumeController->SetCaptureGain(getModeForGain(), mStreamAttributeTarget->input_source, mStreamAttributeTarget->input_device, mStreamAttributeTarget->output_devices);
            }
        }
    }
    return 0;
}

#endif
float AudioALSAStreamManager::getMasterVolume(void) {
    return mAudioALSAVolumeController->getMasterVolume();
}

uint32_t AudioALSAStreamManager::GetOffloadGain(float vol_f) {
    if (mAudioALSAVolumeController != NULL) {
        return mAudioALSAVolumeController->GetOffloadGain(vol_f);
    }
    ALOGE("%s(), VolumeController Null", __FUNCTION__);
    return -1;
}

status_t AudioALSAStreamManager::setMasterVolume(float volume, uint32_t iohandle) {
    ALOGD_IF(mLogEnable, "%s(), volume = %f", __FUNCTION__, volume);

    if (volume < 0.0 || volume > 1.0) {
        ALOGE("-%s(), strange volume level %f, something wrong!!", __FUNCTION__, volume);
        return BAD_VALUE;
    }

    AL_AUTOLOCK(mLock);
    if (mAudioALSAVolumeController) {
        audio_devices_t current_output_devices;
        uint32_t index = mStreamOutVector.indexOfKey(iohandle);
        if (index < mStreamOutVector.size()) {
            current_output_devices = mStreamOutVector[index]->getStreamAttribute()->output_devices;
        } else {
            current_output_devices  = (mStreamOutVector.size() > 0)
                                      ? mStreamOutVector[0]->getStreamAttribute()->output_devices
                                      : AUDIO_DEVICE_NONE;
        }
        mAudioALSAVolumeController->setMasterVolume(volume, getModeForGain(), current_output_devices);
    }

    return NO_ERROR;
}

status_t AudioALSAStreamManager::setHeadsetVolumeMax() {
    ALOGD("%s()", __FUNCTION__);
    mAudioALSAVolumeController->setAudioBufferGain(0);
    return NO_ERROR;
}

status_t AudioALSAStreamManager::setFmVolume(float volume) {
    ALOGV("+%s(), volume = %f", __FUNCTION__, volume);

    if (volume < 0.0 || volume > 1.0) {
        ALOGE("-%s(), strange volume level %f, something wrong!!", __FUNCTION__, volume);
        return BAD_VALUE;
    }

    AL_AUTOLOCK(mLock);
    mFMController->setFmVolume(volume);

    return NO_ERROR;
}

status_t AudioALSAStreamManager::setMicMute(bool state) {
    ALOGD("%s(), mMicMute: %d => %d", __FUNCTION__, mMicMute, state);
    AL_AUTOLOCK(mLock);
    AudioALSASpeechPhoneCallController::getInstance()->setMicMute(state);
    if (isPhoneCallOpen() == false) {
        SetInputMute(state);
    }
    mMicMute = state;
    return NO_ERROR;
}


bool AudioALSAStreamManager::getMicMute() {
    ALOGD_IF(mLogEnable, "%s(), mMicMute = %d", __FUNCTION__, mMicMute);
    AL_AUTOLOCK(mLock);

    return mMicMute;
}

void AudioALSAStreamManager::SetInputMute(bool bEnable) {
    ALOGD("+%s(), %d", __FUNCTION__, bEnable);
    if (mStreamInVector.size() > 0) {
        for (size_t i = 0; i < mStreamInVector.size(); i++) { // TODO(Harvey): Mic+FM !?
            mStreamInVector[i]->SetInputMute(bEnable);
        }
    }
    ALOGD_IF(mLogEnable, "-%s(), %d", __FUNCTION__, bEnable);
}

status_t AudioALSAStreamManager::setVtNeedOn(const bool vt_on) {
    ALOGD("%s(), setVtNeedOn: %d", __FUNCTION__, vt_on);
    AudioALSASpeechPhoneCallController::getInstance()->setVtNeedOn(vt_on);

    return NO_ERROR;
}

status_t AudioALSAStreamManager::setMode(audio_mode_t new_mode) {
    bool resumeAllStreamsAtSetMode = false;
    bool updateModeToStreamOut = false;
    int ret = 0;

    AL_AUTOLOCK(mStreamVectorLock);

    // check value
    if ((new_mode < AUDIO_MODE_NORMAL) || (new_mode > AUDIO_MODE_MAX)) {
        ALOGW("%s(), new_mode: %d is BAD_VALUE, return", __FUNCTION__, new_mode);
        return BAD_VALUE;
    }

    // TODO(Harvey): modem 1 / modem 2 check

    if (new_mode == mAudioMode) {
        ALOGW("%s(), mAudioMode: %d == %d, return", __FUNCTION__, mAudioMode, new_mode);
        return NO_ERROR;
    }

    if (mStreamManagerDestructing == true) {
        ALOGW("%s(), setMode %d => %d during StreamManager destructing, return", __FUNCTION__, mAudioMode, new_mode);
        return NO_ERROR;
    }
#ifdef MTK_VOW_SUPPORT
    // make sure voice wake up is closed before enter INCALL and COMMUNICATION
    //if (new_mode != AUDIO_MODE_NORMAL) {
    if ((new_mode == AUDIO_MODE_IN_CALL) || (new_mode == AUDIO_MODE_IN_COMMUNICATION)) {
        mForceDisableVoiceWakeUpForSetMode = true;
        if (mAudioALSAVoiceWakeUpController->getVoiceWakeUpEnable() == true) {
            ALOGD("%s(), force close voice wake up", __FUNCTION__);
            mAudioALSAVoiceWakeUpController->setVoiceWakeUpEnable(false);
        }
    }
#endif
#ifndef MTK_COMBO_MODEM_SUPPORT
    else if (isModeInPhoneCall(mAudioMode) == true) {
        //check if any stream out is playing during leaving IN_CALL mode
        int DelayCount = 0;
        bool IsStreamActive;
        do {
            IsStreamActive = false;
            for (size_t i = 0; i < mStreamOutVector.size(); i++) {
                if (mStreamOutVector[i]->isOutPutStreamActive() == true) {
                    IsStreamActive = true;
                }
            }
            if (IsStreamActive) {
                usleep(20 * 1000);
                ALOGD_IF(mLogEnable, "%s(), delay 20ms x(%d) for active stream out playback", __FUNCTION__, DelayCount);
            }
            DelayCount++;
        } while ((DelayCount <= 10) && (IsStreamActive));
    }
#endif

    mEnterPhoneCallMode = isModeInPhoneCall(new_mode);

    if (isModeInPhoneCall(new_mode) == true || isPhoneCallOpen() == true ||
        isModeInVoipCall(new_mode)  == true || isModeInVoipCall(mAudioMode) == true) {
        setAllInputStreamsSuspend(true, true);
        standbyAllInputStreams(true);
        //Need to reset MicInverse when phone/VOIP call
        AudioALSAHardwareResourceManager::getInstance()->setMicInverse(0);

        if ((isModeInPhoneCall(mAudioMode) == false && // non-phone call --> voip mode
             isModeInVoipCall(new_mode) == true) ||
            (isModeInVoipCall(mAudioMode) == true && // leave voip, not enter phone call, and not 2->3->0
             isModeInPhoneCall(new_mode) == false &&
             isPhoneCallOpen() == false)) {
            mIsNeedResumeStreamOut = false;
            updateModeToStreamOut = true;
        } else {
            setAllOutputStreamsSuspend(true, true);
            standbyAllOutputStreams(true);
            mIsNeedResumeStreamOut = true;
        }

        /* Only change mode to non-call need resume streams at the end of setMode().
           Otherwise, resume streams when get the routing command. */
        /* Not use isModeInPhoneCall() because 2->3 case need to resume in routing command.*/
        if (isModeInPhoneCall(new_mode) == true) {
            mResumeAllStreamsAtRouting = true;
        } else {
            resumeAllStreamsAtSetMode = true;
        }
    }

    ALOGD("+%s(), mAudioMode: %d => %d, mEnterPhoneCallMode = %d, mResumeAllStreamsAtRouting = %d, resumeAllStreamsAtSetMode = %d",
          __FUNCTION__, mAudioMode, new_mode, mEnterPhoneCallMode, mResumeAllStreamsAtRouting, resumeAllStreamsAtSetMode);

    // TODO(Harvey): // close mATV when mode swiching

    {
        AL_AUTOLOCK(mLock);
        AL_AUTOLOCK(mAudioModeLock);

        // use primary stream out device // TODO(Harvey): add a function? get from hardware?
#ifdef FORCE_ROUTING_RECEIVER
        const audio_devices_t current_output_devices = AUDIO_DEVICE_OUT_EARPIECE;
#else
        const audio_devices_t current_output_devices = (mStreamOutVector.size() > 0)
                                                       ? mStreamOutVector[0]->getStreamAttribute()->output_devices
                                                       : AUDIO_DEVICE_NONE;
#endif

        // close previous call if needed
        /*if ((isModeInPhoneCall(mAudioMode) == true)
            && (isModeInPhoneCall(new_mode) == false))*/
        if ((isModeInNormal(new_mode) == true) && (isPhoneCallOpen() == true)) {
            mSpeechPhoneCallController->close();
            ALOGD("%s(), force unmute mic after phone call closed", __FUNCTION__);
            if (mSmartPaController->isSmartPAUsed()) {
                mSmartPaController->setPhoneCallEnable(false);
            }
            mSpeechPhoneCallController->setMicMute(false);
            mSpeechPhoneCallController->setDlMute(false);
        }
        // open next call if needed
        if ((isModeInPhoneCall(new_mode) == true) && (isPhoneCallOpen() == false)) {
            ALOGD("%s(), open next call", __FUNCTION__);
        }

        mAudioMode = new_mode;

        if (isModeInPhoneCall() == false && isPhoneCallOpen() == false) {
            mAudioALSAVolumeController->setMasterVolume(mAudioALSAVolumeController->getMasterVolume(),
                                                        getModeForGain(), current_output_devices);
        }

#ifdef MTK_VOW_SUPPORT
        // make sure voice wake up is resume when not in INCALL or COMMUNICATION
        //if (mAudioMode == AUDIO_MODE_NORMAL) {
        if ((mAudioMode != AUDIO_MODE_IN_CALL) && (mAudioMode != AUDIO_MODE_IN_COMMUNICATION)) {
            mForceDisableVoiceWakeUpForSetMode = false;
            if (mVoiceWakeUpNeedOn == true &&
                mStreamInVector.size() == 0 &&
                mAudioALSAVoiceWakeUpController->getVoiceWakeUpEnable() == false) {
                ALOGD("%s(), resume voice wake up", __FUNCTION__);
                //ASSERT(mAudioALSAVoiceWakeUpController->getVoiceWakeUpEnable() == false); // TODO(Harvey): double check, remove it later
                mAudioALSAVoiceWakeUpController->setVoiceWakeUpEnable(true);
            }
        }
#endif

#ifdef MTK_POWERHAL_WIFI_POWRER_SAVE
        if (isModeInVoipCall()) {
            power_hal_hint(POWERHAL_DISABLE_WIFI_POWER_SAVE, true);
        } else {
            power_hal_hint(POWERHAL_DISABLE_WIFI_POWER_SAVE, false);
        }
#endif
    }

    // update audio mode to stream out if not suspend/standby streamout
    if (updateModeToStreamOut) {
        for (size_t i = 0; i < mStreamOutVector.size(); i++) {
            ret = mStreamOutVector[i]->updateAudioMode(mAudioMode);
            ASSERT(ret == 0);
        }
#ifdef MTK_AUDIODSP_SUPPORT
        AudioDspStreamManager::getInstance()->updateMode(mAudioMode);
#endif
    }

    if (resumeAllStreamsAtSetMode == true) {
        if (mIsNeedResumeStreamOut) {
            mIsNeedResumeStreamOut = false;
            setAllOutputStreamsSuspend(false, true);
        }
        setAllInputStreamsSuspend(false, true);
    }

#if defined(MTK_HYBRID_NLE_SUPPORT)
    AudioALSAHyBridNLEManager::getInstance()->setAudioMode(mAudioMode);
#endif

    ALOGD("-%s(), mAudioMode = %d, mResumeAllStreamsAtRouting = %d, resumeAllStreamsAtSetMode = %d",
          __FUNCTION__, mAudioMode, mResumeAllStreamsAtRouting, resumeAllStreamsAtSetMode);

    return NO_ERROR;
}

audio_mode_t AudioALSAStreamManager::getMode() {
    AL_AUTOLOCK(mAudioModeLock);
    ALOGD("%s(), mAudioMode = %d", __FUNCTION__, mAudioMode);

    return mAudioMode;
}

bool AudioALSAStreamManager::isPhoneCallOpen() {
    return mSpeechPhoneCallController->getCallStatus();
}

status_t AudioALSAStreamManager::syncSharedOutDevice(audio_devices_t routingSharedOutDevice,
                                                     AudioALSAStreamOut *currentStreamOut) {
    ALOGD("+%s(), routingSharedOutDevice: %d",
          __FUNCTION__, routingSharedOutDevice);
    AL_AUTOLOCK(mLock);

    status_t status = NO_ERROR;
    Vector<AudioALSAStreamOut *> streamOutToRoute;

    // Check if shared device
    AudioALSAHardwareResourceManager *hwResMng = AudioALSAHardwareResourceManager::getInstance();
    if (!hwResMng->isSharedOutDevice(routingSharedOutDevice)) {
        ALOGD("-%s(), this stream out is not shared out device, return.", __FUNCTION__);
        return NO_ERROR;
    }

    // suspend before routing & check which streamout need routing
    for (size_t i = 0; i < mStreamOutVector.size(); i++) {
        if (isOutputNeedRouting(mStreamOutVector[i], currentStreamOut, routingSharedOutDevice)) {
            mStreamOutVector[i]->setSuspend(true);
            streamOutToRoute.add(mStreamOutVector[i]);
        }
    }

    // routing
    for (size_t i = 0; i < streamOutToRoute.size(); i++) {
        status = streamOutToRoute[i]->routing(routingSharedOutDevice);
        ASSERT(status == NO_ERROR);
        if (streamOutToRoute[i] != currentStreamOut) {
            streamOutToRoute[i]->setMuteForRouting(true);
        }
    }

    // resume suspend
    for (size_t i = 0; i < streamOutToRoute.size(); i++) {
        streamOutToRoute[i]->setSuspend(false);
    }

    if (streamOutToRoute.size() > 0) {
        updateOutputDeviceForAllStreamIn_l(routingSharedOutDevice);

        // volume control
        if (!isPhoneCallOpen()) {
            mAudioALSAVolumeController->setMasterVolume(mAudioALSAVolumeController->getMasterVolume(),
                                                        getModeForGain(), routingSharedOutDevice);
        }
    }

    ALOGD("-%s()", __FUNCTION__);
    return status;
}

bool AudioALSAStreamManager::isOutputNeedRouting(AudioALSAStreamOut *eachStreamOut,
                                                 AudioALSAStreamOut *currentStreamOut,
                                                 audio_devices_t routingSharedOutDevice) {
    audio_devices_t streamOutDevice = eachStreamOut->getStreamAttribute()->output_devices;
    bool isSharedStreamOutDevice = AudioALSAHardwareResourceManager::getInstance()->isSharedOutDevice(streamOutDevice);
    bool isSharedRoutingDevice = AudioALSAHardwareResourceManager::getInstance()->isSharedOutDevice(routingSharedOutDevice);

    if (streamOutDevice == routingSharedOutDevice) {
        return false;
    }

    if (eachStreamOut->isOutPutStreamActive()) {

        // active currentStreamOut always need routing
        if (currentStreamOut == eachStreamOut) {
            return true;
        }

        if (isSharedStreamOutDevice && isSharedRoutingDevice) {
            return true;
        }
    }

    return false;
}

status_t AudioALSAStreamManager::DeviceNoneUpdate() {
    ALOGD("+%s()", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    status_t status = NO_ERROR;
#ifdef MTK_VOW_SUPPORT
    // update the output device info for voice wakeup (even when "routing=0")
    mAudioALSAVoiceWakeUpController->updateDeviceInfoForVoiceWakeUp();
#endif
    ALOGD("-%s()", __FUNCTION__);

    return status;
}

status_t AudioALSAStreamManager::routingOutputDevice(AudioALSAStreamOut *pAudioALSAStreamOut, const audio_devices_t current_output_devices, audio_devices_t output_devices) {
    AL_AUTOLOCK(mLock);

    status_t status = NO_ERROR;
    audio_devices_t streamOutDevice = pAudioALSAStreamOut->getStreamAttribute()->output_devices;

    Vector<AudioALSAStreamOut *> streamOutSuspendInCall;

#if defined(MTK_BT_SPK_DUAL_PATH_SUPPORT)
    // for BTSCO + SPK dual path device
    mSpeechPhoneCallController->setBtSpkDevice(false);
    if (isBtSpkDevice(output_devices) && isPhoneCallOpen()) {
        audio_devices_t source_output_devices = output_devices;
        // use SPK setting for BTSCO + SPK
        output_devices = (audio_devices_t)(output_devices & (~AUDIO_DEVICE_OUT_ALL_SCO));
        mSpeechPhoneCallController->setBtSpkDevice(true);
        ALOGD("%s(), Use SPK setting for BTSCO + SPK, output_devices: 0x%x => 0x%x", __FUNCTION__, source_output_devices, output_devices);
    }
#endif

    if (pAudioALSAStreamOut->getStreamAttribute()->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_PRIMARY) {
        mOutputStreamForCall = pAudioALSAStreamOut;
        mCurrentOutputDevicesForCall = current_output_devices;
        mOutputDevicesForCall = output_devices;
    }

#ifdef MTK_VOW_SUPPORT
    // update the output device info for voice wakeup (even when "routing=0")
    mAudioALSAVoiceWakeUpController->updateDeviceInfoForVoiceWakeUp();
#endif
    if (output_devices == AUDIO_DEVICE_NONE) {
        ALOGW("%s(), flag: 0x%x, output_devices == AUDIO_DEVICE_NONE(0x%x), return",
              __FUNCTION__, pAudioALSAStreamOut->getStreamAttribute()->mAudioOutputFlags, AUDIO_DEVICE_NONE);
        return NO_ERROR;
    } else if (output_devices == streamOutDevice) {
        if ((mPhoneCallControllerStatusPolicy == true) || (mResumeAllStreamsAtRouting == true)) {
            ALOGD("+%s(), flag: 0x%x, output_devices = current_devices(0x%x), mResumeAllStreamsAtRouting = %d",
                  __FUNCTION__, pAudioALSAStreamOut->getStreamAttribute()->mAudioOutputFlags,
                  current_output_devices, mResumeAllStreamsAtRouting);
        }
#ifdef MTK_AUDIO_TTY_SPH_ENH_SUPPORT
        else if ((isPhoneCallOpen() == true) && (mSpeechPhoneCallController->checkTtyNeedOn() == true)) {
            ALOGW("+%s(), flag: 0x%x, output_devices == current_output_devices(0x%x), but TTY call is enabled",
                  __FUNCTION__, pAudioALSAStreamOut->getStreamAttribute()->mAudioOutputFlags, streamOutDevice);
        }
#endif
        else {
            ALOGW("%s(), flag: 0x%x, output_devices == current_output_devices(0x%x), return",
                  __FUNCTION__, pAudioALSAStreamOut->getStreamAttribute()->mAudioOutputFlags, streamOutDevice);
            return NO_ERROR;
        }
    } else {
        ALOGD("+%s(), flag: 0x%x, output_devices: 0x%x => 0x%x",
              __FUNCTION__, pAudioALSAStreamOut->getStreamAttribute()->mAudioOutputFlags,
              streamOutDevice, output_devices);
    }

    // close FM when mode swiching
    if (mFMController->getFmEnable() && (mPhoneCallControllerStatusPolicy == true)) {
        //setFmEnable(false);
        mFMController->setFmEnable(false, current_output_devices, false, false, true);
    }


    // do routing
    if ((mPhoneCallControllerStatusPolicy == true) &&
        pAudioALSAStreamOut->getStreamAttribute()->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_PRIMARY) {
        bool checkrouting = CheckStreaminPhonecallRouting(mSpeechPhoneCallController->getInputDeviceForPhoneCall(output_devices), false);
        bool isFirstRoutingInCall = false;

        if (isPhoneCallOpen() == false) {
#ifdef FORCE_ROUTING_RECEIVER
            output_devices = AUDIO_DEVICE_OUT_EARPIECE;
#endif
            if (mSmartPaController->isSmartPAUsed()) {
                mSmartPaController->setPhoneCallEnable(true);
            }

            mSpeechPhoneCallController->open(
                mAudioMode,
                output_devices,
                mSpeechPhoneCallController->getInputDeviceForPhoneCall(output_devices));

            isFirstRoutingInCall = true;
        }

        if (output_devices == streamOutDevice) {
            if (mSpeechPhoneCallController->checkTtyNeedOn() == true) {
                ALOGW("-%s(), output_devices == current_output_devices(0x%x), but TTY call is enabled",
                      __FUNCTION__, current_output_devices);
                for (size_t i = 0; i < mStreamOutVector.size(); i++) {
                    mStreamOutVector[i]->setSuspend(true);
                    if (mStreamOutVector[i]->isOutPutStreamActive()) {
                        mStreamOutVector[i]->standbyStreamOut();
                    }
                    streamOutSuspendInCall.add(mStreamOutVector[i]);
                }
                mSpeechPhoneCallController->routing(
                    output_devices,
                    mSpeechPhoneCallController->getInputDeviceForPhoneCall(output_devices));
            }
        } else if (!isFirstRoutingInCall) {
            for (size_t i = 0; i < mStreamOutVector.size(); i++) {
                mStreamOutVector[i]->setSuspend(true);
                if (mStreamOutVector[i]->isOutPutStreamActive()) {
                    mStreamOutVector[i]->standbyStreamOut();
                }
                streamOutSuspendInCall.add(mStreamOutVector[i]);
            }
            mSpeechPhoneCallController->routing(
                output_devices,
                mSpeechPhoneCallController->getInputDeviceForPhoneCall(output_devices));
        }

        //Need to resume the streamin
        if (checkrouting == true) {
            CheckStreaminPhonecallRouting(mSpeechPhoneCallController->getInputDeviceForPhoneCall(output_devices), true);
        }

        // volume control
        mAudioALSAVolumeController->setVoiceVolume(mAudioALSAVolumeController->getVoiceVolume(),
                                                   getModeForGain(), output_devices);

        for (size_t i = 0; i < mStreamOutVector.size(); i++) {
            mStreamOutVector[i]->syncPolicyDevice();
        }
    }

    if (mResumeAllStreamsAtRouting == true) {
        setAllStreamsSuspend(false, true);
        mResumeAllStreamsAtRouting = false;
    }


    Vector<AudioALSAStreamOut *> streamOutToRoute;

    // Check if non active streamout device
    if (!pAudioALSAStreamOut->isOutPutStreamActive()) {
        ALOGD("-%s(), stream out not active, route itself and return", __FUNCTION__);
        pAudioALSAStreamOut->routing(output_devices);
        status = NO_ERROR;
#if defined(SUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL)
        // FM follow Primary routing
        if (pAudioALSAStreamOut->getStreamAttribute()->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_PRIMARY
            && mFMController->getFmEnable()) {
            mFMController->routing(current_output_devices, output_devices);
        }
#endif
        goto ROUTE_OUTPUT_DONE;
    }

    // suspend before routing & check if other streamouts need routing
    for (size_t i = 0; i < mStreamOutVector.size(); i++) {
        if (isOutputNeedRouting(mStreamOutVector[i], pAudioALSAStreamOut, output_devices)) {
            mStreamOutVector[i]->setSuspend(true);
            streamOutToRoute.add(mStreamOutVector[i]);
        }
    }

    // routing
    for (size_t i = 0; i < streamOutToRoute.size(); i++) {
        status = streamOutToRoute[i]->routing(output_devices);
        ASSERT(status == NO_ERROR);
        if (streamOutToRoute[i] != pAudioALSAStreamOut) {
            streamOutToRoute[i]->setMuteForRouting(true);
        }
    }

#if defined(SUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL)
    if (pAudioALSAStreamOut->getStreamAttribute()->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_PRIMARY
        && mFMController->getFmEnable()) {
        mFMController->routing(current_output_devices, output_devices);
    }
#endif

    // resume suspend
    for (size_t i = 0; i < streamOutToRoute.size(); i++) {
        streamOutToRoute[i]->setSuspend(false);
    }

    if (streamOutToRoute.size() > 0) {
        updateOutputDeviceForAllStreamIn_l(output_devices);

        // volume control
        if (!isPhoneCallOpen()) {
            mAudioALSAVolumeController->setMasterVolume(mAudioALSAVolumeController->getMasterVolume(),
                                                        getModeForGain(), output_devices);
        }
    }

    ALOGD("-%s(), flag: 0x%x, output_devices = 0x%x", __FUNCTION__,
          pAudioALSAStreamOut->getStreamAttribute()->mAudioOutputFlags,
          output_devices);

ROUTE_OUTPUT_DONE:
    for (size_t i = 0; i < streamOutSuspendInCall.size(); i++) {
        streamOutSuspendInCall[i]->setSuspend(false);
    }

    return status;
}


status_t AudioALSAStreamManager::routingInputDevice(AudioALSAStreamIn *pAudioALSAStreamIn, const audio_devices_t current_input_device, audio_devices_t input_device) {
    ALOGD("+%s(), input_device: 0x%x => 0x%x", __FUNCTION__, current_input_device, input_device);
    AL_AUTOLOCK(mLock);

    status_t status = NO_ERROR;

    int mNumPhoneMicSupport = AudioCustParamClient::GetInstance()->getNumMicSupport();
    if ((input_device == AUDIO_DEVICE_IN_BACK_MIC) && (mNumPhoneMicSupport < 2)) {
        input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        ALOGW("%s(), not support back_mic if mic < 2, force to set input_device = 0x%x", __FUNCTION__, input_device);
    }

    bool sharedDevice = (input_device & ~AUDIO_DEVICE_BIT_IN) & (AUDIO_DEVICE_IN_BUILTIN_MIC | AUDIO_DEVICE_IN_BACK_MIC | AUDIO_DEVICE_IN_WIRED_HEADSET);
    //In PhonecallMode and the new input_device / phonecall_device are both sharedDevice,we may change the input_device = phonecall_device
    if ((isPhoneCallOpen() == true) && (sharedDevice == true)) {
        audio_devices_t phonecall_device = mSpeechPhoneCallController->getAdjustedInputDevice();
        sharedDevice = (phonecall_device & ~AUDIO_DEVICE_BIT_IN) & (AUDIO_DEVICE_IN_BUILTIN_MIC | AUDIO_DEVICE_IN_BACK_MIC | AUDIO_DEVICE_IN_WIRED_HEADSET);
        if (sharedDevice == true) {
            input_device = phonecall_device;
        }
        ALOGD("+%s(), isPhoneCallOpen, input_device = 0x%x", __FUNCTION__, input_device);
#ifdef MTK_CHECK_INPUT_DEVICE_PRIORITY
    } else if ((sharedDevice == true) && (mStreamInVector.size() > 1)) {
        input_device = CheckInputDevicePriority(input_device);
#endif
    }

    if (input_device == AUDIO_DEVICE_NONE) {
        ALOGW("-%s(), input_device == AUDIO_DEVICE_NONE(0x%x), return", __FUNCTION__, AUDIO_DEVICE_NONE);
        return NO_ERROR;
    } else if (input_device == current_input_device) {
        ALOGW("-%s(), input_device == current_input_device(0x%x), return", __FUNCTION__, current_input_device);
        return NO_ERROR;
    }

    if (mStreamInVector.size() > 0) {
        for (size_t i = 0; i < mStreamInVector.size(); i++) {
            if ((input_device == AUDIO_DEVICE_IN_FM_TUNER) || (current_input_device == AUDIO_DEVICE_IN_FM_TUNER)) {
                if (pAudioALSAStreamIn == mStreamInVector[i]) {
                    status = mStreamInVector[i]->routing(input_device);
                    ASSERT(status == NO_ERROR);
                }
            } else {
                status = mStreamInVector[i]->routing(input_device);
                ASSERT(status == NO_ERROR);
            }
        }
    }
    return status;
}

// check if headset has changed
bool AudioALSAStreamManager::CheckHeadsetChange(const audio_devices_t current_output_devices, audio_devices_t output_device) {
    ALOGD("+%s(), current_output_devices = %d output_device = %d ", __FUNCTION__, current_output_devices, output_device);
    if (current_output_devices == output_device) {
        return false;
    }
    if (current_output_devices == AUDIO_DEVICE_NONE || output_device == AUDIO_DEVICE_NONE) {
        return true;
    }
    if (current_output_devices == AUDIO_DEVICE_OUT_WIRED_HEADSET || current_output_devices == AUDIO_DEVICE_OUT_WIRED_HEADPHONE
        || output_device == AUDIO_DEVICE_OUT_WIRED_HEADSET || output_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        return true;
    }
    return false;
}

status_t AudioALSAStreamManager::setFmEnable(const bool enable, bool bForceControl, bool bForce2DirectConn, audio_devices_t output_device) { // TODO(Harvey)
    //AL_AUTOLOCK(mLock);

    // Reject set fm enable during phone call mode
    if (mPhoneCallControllerStatusPolicy || isPhoneCallOpen()) {
        ALOGW("-%s(), mAudioMode(%d), phonecall is opened, return.", __FUNCTION__, mAudioMode);
        return INVALID_OPERATION;
    }

    // use primary stream out device // TODO(Harvey): add a function? get from hardware?
    audio_devices_t current_output_devices;
    if (output_device == AUDIO_DEVICE_NONE) {
        audio_devices_t primary_streamout_device = ((mStreamOutVector.size() > 0)
                                                    ? mStreamOutVector[0]->getStreamAttribute()->output_devices : AUDIO_DEVICE_NONE);
        for (size_t i = 0; i < mStreamOutVector.size(); i++) {
            if (mStreamOutVector[i]->getStreamAttribute()->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_PRIMARY) {
                primary_streamout_device = mStreamOutVector[i]->getStreamAttribute()->output_devices;
            }
        }
        current_output_devices = primary_streamout_device;
    } else {
        current_output_devices = output_device;
    }
    mFMController->setFmEnable(enable, current_output_devices, bForceControl, bForce2DirectConn);
    return NO_ERROR;
}

status_t AudioALSAStreamManager::setHdmiEnable(const bool enable) { // TODO(George): tmp, add a class to do it
    ALOGD("+%s(), enable = %d", __FUNCTION__, enable);
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    if (enable == mHdmiEnable) {
        return ALREADY_EXISTS;
    }
    mHdmiEnable = enable;

    if (enable == true) {
        int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmI2S0Dl1Playback);
        int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmI2S0Dl1Playback);

        // DL loopback setting
        mLoopbackConfig.channels = 2;
        mLoopbackConfig.rate = 44100;
        mLoopbackConfig.period_size = 512;
        mLoopbackConfig.period_count = 4;
        mLoopbackConfig.format = PCM_FORMAT_S32_LE;
        mLoopbackConfig.start_threshold = 0;
        mLoopbackConfig.stop_threshold = 0;
        mLoopbackConfig.silence_threshold = 0;
        if (mHdmiPcm == NULL) {
            mHdmiPcm = pcm_open(cardIdx, pcmIdx, PCM_OUT, &mLoopbackConfig);
            ALOGD("pcm_open mHdmiPcm = %p", mHdmiPcm);
        }
        if (!mHdmiPcm || !pcm_is_ready(mHdmiPcm)) {
            ALOGD("Unable to open mHdmiPcm device %u (%s)", pcmIdx, pcm_get_error(mHdmiPcm));
        }

        ALOGD("pcm_start(mHdmiPcm)");
        pcm_start(mHdmiPcm);
    } else {
        ALOGD("pcm_close");
        if (mHdmiPcm != NULL) {
            pcm_close(mHdmiPcm);
            mHdmiPcm = NULL;
        }
        ALOGD("pcm_close done");
    }


    ALOGD("-%s(), enable = %d", __FUNCTION__, enable);
    return NO_ERROR;
}

status_t AudioALSAStreamManager::setLoopbackEnable(const bool enable) { // TODO(Harvey): tmp, add a class to do it
    ALOGD("+%s(), enable = %d", __FUNCTION__, enable);
    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(*AudioALSADriverUtility::getInstance()->getStreamSramDramLock());

    if (enable == mLoopbackEnable) {
        return ALREADY_EXISTS;
    }
    mLoopbackEnable = enable;

    if (enable == true) {
        int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmUlDlLoopback);
        int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmUlDlLoopback);

        // DL loopback setting
        mLoopbackConfig.channels = 2;
        mLoopbackConfig.rate = 48000;
        mLoopbackConfig.period_size = 512;
        mLoopbackConfig.period_count = 4;
        mLoopbackConfig.format = PCM_FORMAT_S16_LE;
        mLoopbackConfig.start_threshold = 0;
        mLoopbackConfig.stop_threshold = 0;
        mLoopbackConfig.silence_threshold = 0;
        if (mLoopbackPcm == NULL) {
            mLoopbackPcm = pcm_open(cardIdx, pcmIdx, PCM_OUT, &mLoopbackConfig);
            ALOGD("pcm_open mLoopbackPcm = %p", mLoopbackPcm);
        }
        if (!mLoopbackPcm || !pcm_is_ready(mLoopbackPcm)) {
            ALOGD("Unable to open mLoopbackPcm device %u (%s)", pcmIdx, pcm_get_error(mLoopbackPcm));
        }

        ALOGD("pcm_start(mLoopbackPcm)");
        pcm_start(mLoopbackPcm);

        //UL loopback setting
        mLoopbackUlConfig.channels = 2;
        mLoopbackUlConfig.rate = 48000;
        mLoopbackUlConfig.period_size = 512;
        mLoopbackUlConfig.period_count = 4;
        mLoopbackUlConfig.format = PCM_FORMAT_S16_LE;
        mLoopbackUlConfig.start_threshold = 0;
        mLoopbackUlConfig.stop_threshold = 0;
        mLoopbackUlConfig.silence_threshold = 0;
        if (mLoopbackUlPcm == NULL) {
            mLoopbackUlPcm = pcm_open(cardIdx, pcmIdx, PCM_IN, &mLoopbackUlConfig);
            ALOGD("pcm_open mLoopbackPcm = %p", mLoopbackUlPcm);
        }
        if (!mLoopbackUlPcm || !pcm_is_ready(mLoopbackUlPcm)) {
            ALOGD("Unable to open mLoopbackUlPcm device %u (%s)", pcmIdx, pcm_get_error(mLoopbackUlPcm));
        }
        ALOGD("pcm_start(mLoopbackUlPcm)");
        pcm_start(mLoopbackUlPcm);
    } else {
        ALOGD("pcm_close");
        if (mLoopbackPcm != NULL) {
            pcm_close(mLoopbackPcm);
            mLoopbackPcm = NULL;
        }
        if (mLoopbackUlPcm != NULL) {
            pcm_close(mLoopbackUlPcm);
            mLoopbackUlPcm = NULL;
        }
        ALOGD("pcm_close done");
    }


    ALOGD("-%s(), enable = %d", __FUNCTION__, enable);
    return NO_ERROR;
}

bool AudioALSAStreamManager::getFmEnable() {
    AL_AUTOLOCK(mLock);
    return mFMController->getFmEnable();
}

status_t AudioALSAStreamManager::setAllOutputStreamsSuspend(const bool suspend_on, const bool setModeRequest __unused) {
    for (size_t i = 0; i < mStreamOutVector.size(); i++) {
        ASSERT(mStreamOutVector[i]->setSuspend(suspend_on) == NO_ERROR);
    }

    return NO_ERROR;
}

status_t AudioALSAStreamManager::setAllInputStreamsSuspend(const bool suspend_on, const bool setModeRequest, const capture_handler_t caphandler) {
    ALOGV("%s()", __FUNCTION__);

    status_t status = NO_ERROR;

    AudioALSAStreamIn *pAudioALSAStreamIn = NULL;

    for (size_t i = 0; i < mStreamInVector.size(); i++) {
        pAudioALSAStreamIn = mStreamInVector[i];

        if ((setModeRequest == true) && (mEnterPhoneCallMode == true) && (mStreamInVector[i]->getStreamInCaptureHandler() != NULL)) {
            //No need to do reopen when mode change
            if ((pAudioALSAStreamIn->isSupportConcurrencyInCall()) == true) {
                ALOGD("%s(), Enter phone call mode, mStreamInVector[%zu] support concurrency!!", __FUNCTION__, i);
                continue;
            }
        }

        if (pAudioALSAStreamIn->getStreamInCaptureHandler() == NULL) {
            ALOGD("%s(), this streamin does not have capture handler, just set suspend", __FUNCTION__);
            status = pAudioALSAStreamIn->setSuspend(suspend_on);
            continue;
        }
        if (pAudioALSAStreamIn->getCaptureHandlerType() & caphandler) {
            ALOGD("%s(), find corresponding streamin, suspend it", __FUNCTION__);
            status = pAudioALSAStreamIn->setSuspend(suspend_on);
        }

        if (status != NO_ERROR) {
            ALOGE("%s(), mStreamInVector[%zu] setSuspend() fail!!", __FUNCTION__, i);
        }
    }

    ALOGV("%s()-", __FUNCTION__);
    return status;
}

status_t AudioALSAStreamManager::setAllStreamsSuspend(const bool suspend_on, const bool setModeRequest) {
    ALOGD("%s(), suspend_on = %d", __FUNCTION__, suspend_on);

    status_t status = NO_ERROR;

    status = setAllOutputStreamsSuspend(suspend_on, setModeRequest);
    status = setAllInputStreamsSuspend(suspend_on, setModeRequest);

    return status;
}


status_t AudioALSAStreamManager::standbyAllOutputStreams(const bool setModeRequest __unused) {
    ALOGD_IF(mLogEnable, "%s()", __FUNCTION__);
    status_t status = NO_ERROR;

    AudioALSAStreamOut *pAudioALSAStreamOut = NULL;

    for (size_t i = 0; i < mStreamOutVector.size(); i++) {
        pAudioALSAStreamOut = mStreamOutVector[i];
        status = pAudioALSAStreamOut->standbyStreamOut();
        if (status != NO_ERROR) {
            ALOGE("%s(), mStreamOutVector[%zu] standbyStreamOut() fail!!", __FUNCTION__, i);
        }
    }

    return status;
}

status_t AudioALSAStreamManager::standbyAllInputStreams(const bool setModeRequest, capture_handler_t caphandler) {
    ALOGD_IF(mLogEnable, "%s()", __FUNCTION__);
    status_t status = NO_ERROR;

    AudioALSAStreamIn *pAudioALSAStreamIn = NULL;

    for (size_t i = 0; i < mStreamInVector.size(); i++) {
        pAudioALSAStreamIn = mStreamInVector[i];

        if ((setModeRequest == true) && (mEnterPhoneCallMode == true) && (mStreamInVector[i]->getStreamInCaptureHandler() != NULL)) {
            //No need to do reopen when mode change
            if ((pAudioALSAStreamIn->isSupportConcurrencyInCall()) == true) {
                ALOGD("%s(), Enter phone call mode, mStreamInVector[%zu] support concurrency!!", __FUNCTION__, i);
                continue;
            }
        }

        if (pAudioALSAStreamIn->getStreamInCaptureHandler() == NULL) {
            ALOGD("%s(), mStreamInVector[%zu] capture handler not created yet, pAudioALSAStreamIn=%p, this=%p", __FUNCTION__, i, pAudioALSAStreamIn, this);
            continue;
        }

        if ((pAudioALSAStreamIn->getStreamInCaptureHandler() != NULL) && (pAudioALSAStreamIn->getStreamInCaptureHandler()->getCaptureHandlerType() & caphandler)) {
            ALOGD("%s(), find corresponding streamin, standby it", __FUNCTION__);
            status = pAudioALSAStreamIn->standby();
        }

        if (status != NO_ERROR) {
            ALOGE("%s(), mStreamInVector[%zu] standby() fail!!", __FUNCTION__, i);
        }
    }

    ALOGV("%s()-", __FUNCTION__);
    return status;
}

status_t AudioALSAStreamManager::standbyAllStreams(const bool setModeRequest) {
    ALOGD_IF("%s()", __FUNCTION__);

    status_t status = NO_ERROR;

    status = standbyAllOutputStreams(setModeRequest);
    status = standbyAllInputStreams(setModeRequest);

    return status;
}

#if defined(MTK_HIFIAUDIO_SUPPORT)
int AudioALSAStreamManager::setAllStreamHiFi(AudioALSAStreamOut *pAudioALSAStreamOut, uint32_t sampleRate) {
    int status = 0;
    const audio_devices_t streamOutDevice = pAudioALSAStreamOut->getStreamAttribute()->output_devices;
    audio_output_flags_t streamOutFlag = pAudioALSAStreamOut->getStreamAttribute()->mAudioOutputFlags;

    bool bFMState = mFMController->getFmEnable();
    bool bHiFiState = AudioALSAHardwareResourceManager::getInstance()->getHiFiStatus();
    ALOGD("%s(), previous mHiFiState = %s, sampleRate = %u, FM state: %d, streamOutFlag = %d",
          __FUNCTION__, bHiFiState ? "true" : "false", sampleRate, bFMState, streamOutFlag);

    AL_AUTOLOCK(mStreamVectorLock);
    bool needEnableFm = false;

    if (bFMState) {
        ALOGV("%s(), getFmEnable() = true, setFmEnable(false), sampleRate = %d", __FUNCTION__, sampleRate);
        mFMController->setFmEnable(false, streamOutDevice, false, false, true);
        needEnableFm = true;
    }

    setAllStreamsSuspend(true, true);
    standbyAllStreams(true);
    AL_AUTOLOCK(mLock);
    for (size_t i = 0; i < mStreamOutVector.size(); i++) {
        if (mStreamOutVector[i]->getStreamAttribute()->mAudioOutputFlags == streamOutFlag) {
            ALOGD("%s(), flag: %d, update streamout source sampling rate(%d)",
                  __FUNCTION__, mStreamOutVector[i]->getStreamAttribute()->mAudioOutputFlags, sampleRate);
            mStreamOutVector[i]->setStreamOutSampleRate(sampleRate);
        } else {
            ALOGV("%s(), flag: %d, dont update streamout source sampling rate",
                  __FUNCTION__, mStreamOutVector[i]->getStreamAttribute()->mAudioOutputFlags);
        }
    }

    // Update sample rate
    ALOGD("%s(), Update target sample rate = %d", __FUNCTION__, sampleRate);
    AudioALSASampleRateController::getInstance()->setPrimaryStreamOutSampleRate(sampleRate);

    setAllStreamsSuspend(false, true);

    if (needEnableFm) {
        ALOGV("%s(), re-Enable FM", __FUNCTION__);
        mFMController->setFmEnable(true, streamOutDevice, true, true, true);
    }

    ALOGD("%s(), return status = %d", __FUNCTION__, status);
    return status;
}
#endif

audio_devices_t AudioALSAStreamManager::CheckInputDevicePriority(audio_devices_t input_device) {
    for (size_t i = 0; i < mStreamInVector.size(); i++) {
        if (setUsedDevice(input_device) == 0) {
            break;
        }
        audio_devices_t old_device = mStreamInVector[i]->getStreamAttribute()->input_device;
        bool sharedDevice = (old_device & ~AUDIO_DEVICE_BIT_IN) & (AUDIO_DEVICE_IN_BUILTIN_MIC | AUDIO_DEVICE_IN_BACK_MIC | AUDIO_DEVICE_IN_WIRED_HEADSET);
        if (sharedDevice == false) {
            continue;
        }
        if ((old_device != input_device) && (setUsedDevice(old_device) < setUsedDevice(input_device))) {
            input_device = old_device;
        }
    }
    ALOGD("%s(),input_device = 0x%x", __FUNCTION__, input_device);
    return input_device;
}

uint32_t AudioALSAStreamManager::setUsedDevice(const audio_devices_t used_device) {
    uint32_t usedInputDeviceIndex = 0;
    switch (used_device) {
    case AUDIO_DEVICE_IN_BUILTIN_MIC: {
        usedInputDeviceIndex = 0;
        break;
    }
    case AUDIO_DEVICE_IN_WIRED_HEADSET: {
        usedInputDeviceIndex = 1;
        break;
    }
    case AUDIO_DEVICE_IN_BACK_MIC: {
        usedInputDeviceIndex = 2;
        break;
    }
    }
    return usedInputDeviceIndex;
}

bool AudioALSAStreamManager::CheckStreaminPhonecallRouting(audio_devices_t new_phonecall_device, bool checkrouting) {
    if (checkrouting == true) { //Already Routing, Need to resume streamin
        setAllInputStreamsSuspend(false, false);
    } else { //Need to check the streamin to do routing
        bool newsharedDevice = ((new_phonecall_device & ~AUDIO_DEVICE_BIT_IN) & (AUDIO_DEVICE_IN_BUILTIN_MIC | AUDIO_DEVICE_IN_BACK_MIC | AUDIO_DEVICE_IN_WIRED_HEADSET));
        if ((mStreamInVector.size() > 0) && (newsharedDevice == true)) {
            status_t status = NO_ERROR;
            bool oldsharedDevice = 0;
            audio_devices_t old_device;
            for (size_t i = 0; i < mStreamInVector.size(); i++) {
                old_device = mStreamInVector[i]->getStreamAttribute()->input_device;
                oldsharedDevice = (old_device & ~AUDIO_DEVICE_BIT_IN) & (AUDIO_DEVICE_IN_BUILTIN_MIC | AUDIO_DEVICE_IN_BACK_MIC | AUDIO_DEVICE_IN_WIRED_HEADSET);
                if ((oldsharedDevice == true) && (old_device != new_phonecall_device)) {
                    if (checkrouting == false) {
                        setAllInputStreamsSuspend(true, false);
                        standbyAllInputStreams(false);
                        checkrouting = true;
                    }
                    ALOGD("+%s(),old_device = 0x%x -> new_phonecall_device = 0x%x", __FUNCTION__, oldsharedDevice, new_phonecall_device);
                    status = mStreamInVector[i]->routing(new_phonecall_device);
                    ASSERT(status == NO_ERROR);
                }
            }
        }
    }
    return checkrouting;
}

bool AudioALSAStreamManager::getPhoncallOutputDevice() {
#ifdef FORCE_ROUTING_RECEIVER
    const audio_devices_t current_output_devices = AUDIO_DEVICE_OUT_EARPIECE;
#else
    const audio_devices_t current_output_devices = (mStreamOutVector.size() > 0)
                                                   ? mStreamOutVector[0]->getStreamAttribute()->output_devices
                                                   : AUDIO_DEVICE_NONE;
#endif
    ALOGD("%s(),current_output_devices = %d ", __FUNCTION__, current_output_devices);
    bool bt_device_on = audio_is_bluetooth_sco_device(current_output_devices);
    ALOGD("%s(),bt_device_on = %d ", __FUNCTION__, bt_device_on);
    return bt_device_on;
}

size_t AudioALSAStreamManager::getInputBufferSize(uint32_t sampleRate, audio_format_t format, uint32_t channelCount) {
    size_t wordSize = 0;
    switch (format) {
    case AUDIO_FORMAT_PCM_8_BIT: {
        wordSize = sizeof(int8_t);
        break;
    }
    case AUDIO_FORMAT_PCM_16_BIT: {
        wordSize = sizeof(int16_t);
        break;
    }
    case AUDIO_FORMAT_PCM_8_24_BIT:
    case AUDIO_FORMAT_PCM_32_BIT: {
        wordSize = sizeof(int32_t);
        break;
    }
    default: {
        ALOGW("%s(), wrong format(0x%x), default use wordSize = %zu", __FUNCTION__, format, sizeof(int16_t));
        wordSize = sizeof(int16_t);
        break;
    }
    }

    size_t bufferSize = ((sampleRate * channelCount * wordSize) * 20) / 1000; // TODO (Harvey): why 20 ms here?

    ALOGD("%s(), sampleRate = %u, format = 0x%x, channelCount = %d, bufferSize = %zu",
          __FUNCTION__, sampleRate, format, channelCount, bufferSize);
    return bufferSize;
}

status_t AudioALSAStreamManager::updateOutputDeviceForAllStreamIn_l(audio_devices_t output_devices) {
    status_t status = NO_ERROR;

    if (mStreamInVector.size() > 0) {
        // update the output device info for input stream
        // (ex:for BesRecord parameters update or mic device change)
        ALOGD_IF(mLogEnable, "%s(), mStreamInVector.size() = %zu", __FUNCTION__, mStreamInVector.size());
        for (size_t i = 0; i < mStreamInVector.size(); i++) {
            status = mStreamInVector[i]->updateOutputDeviceInfoForInputStream(output_devices);
            ASSERT(status == NO_ERROR);
        }
    }

    return status;
}

status_t AudioALSAStreamManager::updateOutputDeviceForAllStreamIn(audio_devices_t output_devices) {
    AL_AUTOLOCK(mLock);

    return updateOutputDeviceForAllStreamIn_l(output_devices);
}

// set musicplus to streamout
status_t AudioALSAStreamManager::SetMusicPlusStatus(bool bEnable) {
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    for (size_t i = 0; i < mFilterManagerVector.size() ; i++) {
        AudioMTKFilterManager  *pTempFilter = mFilterManagerVector[i];
        pTempFilter->setParamFixed(bEnable ? true : false);
    }
#else
    (void) bEnable;
#endif
    return NO_ERROR;
}

bool AudioALSAStreamManager::GetMusicPlusStatus() {
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    for (size_t i = 0; i < mFilterManagerVector.size() ; i++) {
        AudioMTKFilterManager  *pTempFilter = mFilterManagerVector[i];
        bool musicplus_status = pTempFilter->isParamFixed();
        if (musicplus_status) {
            return true;
        }
    }
    return false;
#else
    return false;
#endif
}

status_t AudioALSAStreamManager::UpdateACFHCF(int value) {
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    ALOGD("%s()", __FUNCTION__);

    AUDIO_ACF_CUSTOM_PARAM_STRUCT sACFHCFParam;

    for (size_t i = 0; i < mFilterManagerVector.size() ; i++) {
        AudioMTKFilterManager  *pTempFilter = mFilterManagerVector[i];
        if (value == 0) {
            ALOGD("setParameters Update ACF Parames");
            getAudioCompFltCustParam(AUDIO_COMP_FLT_AUDIO, &sACFHCFParam);
            pTempFilter->setParameter(AUDIO_COMP_FLT_AUDIO, &sACFHCFParam);

        } else if (value == 1) {
            ALOGD("setParameters Update HCF Parames");
            getAudioCompFltCustParam(AUDIO_COMP_FLT_HEADPHONE, &sACFHCFParam);
            pTempFilter->setParameter(AUDIO_COMP_FLT_HEADPHONE, &sACFHCFParam);

        } else if (value == 2) {
            ALOGD("setParameters Update ACFSub Parames");
            getAudioCompFltCustParam(AUDIO_COMP_FLT_AUDIO_SUB, &sACFHCFParam);
            pTempFilter->setParameter(AUDIO_COMP_FLT_AUDIO_SUB, &sACFHCFParam);

        }
    }
#else
    (void) value;
#endif
    return NO_ERROR;
}

// ACF Preview parameter
status_t AudioALSAStreamManager::SetACFPreviewParameter(void *ptr, int len __unused) {
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    ALOGD("%s()", __FUNCTION__);

    for (size_t i = 0; i < mFilterManagerVector.size() ; i++) {
        AudioMTKFilterManager  *pTempFilter = mFilterManagerVector[i];
        pTempFilter->setParameter(AUDIO_COMP_FLT_AUDIO, (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)ptr);
    }
#else
    (void *) ptr;
#endif
    return NO_ERROR;
}

status_t AudioALSAStreamManager::SetHCFPreviewParameter(void *ptr, int len __unused) {
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    ALOGD("%s()", __FUNCTION__);

    for (size_t i = 0; i < mFilterManagerVector.size() ; i++) {
        AudioMTKFilterManager  *pTempFilter = mFilterManagerVector[i];
        pTempFilter->setParameter(AUDIO_COMP_FLT_HEADPHONE, (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)ptr);
    }
#else
    (void *) ptr;
#endif
    return NO_ERROR;
}


status_t AudioALSAStreamManager::SetBesLoudnessStatus(bool bEnable) {
    ALOGD("mBesLoudnessStatus() flag %d", bEnable);

#ifdef MTK_BESLOUDNESS_SUPPORT
    mBesLoudnessStatus = bEnable;
    property_set(PROPERTY_KEY_BESLOUDNESS_SWITCH_STATE, (mBesLoudnessStatus) ? "1" : "0");
#if 0
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_BESLOUDNESS_SWITCH_STATE, property_value, "0");
    int besloudness_enable = atoi(property_value);
    ALOGD("SetBesLoudnessStatus After Setting %d, input %d", besloudness_enable, bEnable);
#endif
    if (mBesLoudnessControlCallback != NULL) {
        mBesLoudnessControlCallback((void *)mBesLoudnessStatus);
    }
#else
    ALOGD("Unsupport set mBesLoudnessStatus()");
#endif
    return NO_ERROR;
}

bool AudioALSAStreamManager::GetBesLoudnessStatus() {
    return mBesLoudnessStatus;
}

status_t AudioALSAStreamManager::SetBesLoudnessControlCallback(const BESLOUDNESS_CONTROL_CALLBACK_STRUCT *callback_data) {
    if (callback_data == NULL) {
        mBesLoudnessControlCallback = NULL;
    } else {
        mBesLoudnessControlCallback = callback_data->callback;
        ASSERT(mBesLoudnessControlCallback != NULL);
        mBesLoudnessControlCallback((void *)mBesLoudnessStatus);
    }

    return NO_ERROR;
}

status_t AudioALSAStreamManager::setSpkOutputGain(int32_t gain, uint32_t ramp_sample_cnt) {
    ALOGD("%s(), gain = %d, ramp_sample_cnt = %u", __FUNCTION__, gain, ramp_sample_cnt);

#if 0 //K2 mark temp
    for (size_t i = 0; i < mFilterManagerVector.size() ; i++) {
        AudioMTKFilterManager  *pTempFilter = mFilterManagerVector[i];
        pTempFilter->setSpkOutputGain(gain, ramp_sample_cnt);
    }
#endif

    return NO_ERROR;
}

status_t AudioALSAStreamManager::setSpkFilterParam(uint32_t fc, uint32_t bw, int32_t th) {
    ALOGD("%s(), fc %d, bw %d, th %d", __FUNCTION__, fc, bw, th);

#if 0 //K2 mark temp
    for (size_t i = 0; i < mFilterManagerVector.size() ; i++) {
        AudioMTKFilterManager  *pTempFilter = mFilterManagerVector[i];
        pTempFilter->setSpkFilterParam(fc, bw, th);
    }
#endif
    return NO_ERROR;
}

status_t AudioALSAStreamManager::SetEMParameter(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB) {
    ALOGD("%s()", __FUNCTION__);

    mAudioCustParamClient->SetNBSpeechParamToNVRam(pSphParamNB);
    SpeechEnhancementController::GetInstance()->SetNBSpeechParametersToAllModem(pSphParamNB);
    // Speech Enhancement, VM, Speech Driver
    // update VM/EPL/TTY record capability & enable if needed
    SpeechVMRecorder::getInstance()->configVm(pSphParamNB);
    return NO_ERROR;
}

status_t AudioALSAStreamManager::updateSpeechNVRAMParam(const int speech_band) {
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    (void) speech_band;
#else
    ALOGD("%s(), speech_band=%d", __FUNCTION__, speech_band);

    //speech_band: 0:Narrow Band, 1: Wide Band, 2: Super Wideband, ..., 8: All
    if (speech_band == 0) { //Narrow Band
        AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
        mAudioCustParamClient->GetNBSpeechParamFromNVRam(&eSphParamNB);
        SpeechEnhancementController::GetInstance()->SetNBSpeechParametersToAllModem(&eSphParamNB);
        ALOGD("JT:================================");
        for (int i = 0; i < SPEECH_COMMON_NUM ; i++) {
            ALOGD("JT:speech_common_para[%d] = %d", i, eSphParamNB.speech_common_para[i]);
        }
        for (int i = 0; i < SPEECH_PARA_MODE_NUM; i++) {
            for (int j = 0; j < SPEECH_PARA_NUM; j++) {
                ALOGD("JT:speech_mode_para[%d][%d] = %d", i, j, eSphParamNB.speech_mode_para[i][j]);
            }
        }
        for (int i = 0; i < 4; i++) {
            ALOGD("JT:speech_volume_para[%d] = %d", i, eSphParamNB.speech_volume_para[i]);
        }
    }
#if defined(MTK_WB_SPEECH_SUPPORT)
    else if (speech_band == 1) { //Wide Band
        AUDIO_CUSTOM_WB_PARAM_STRUCT eSphParamWB;
        mAudioCustParamClient->GetWBSpeechParamFromNVRam(&eSphParamWB);
        SpeechEnhancementController::GetInstance()->SetWBSpeechParametersToAllModem(&eSphParamWB);
    }
#endif
    else if (speech_band == 8) { //set all mode parameters
        AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
        AUDIO_CUSTOM_WB_PARAM_STRUCT eSphParamWB;
        mAudioCustParamClient->GetNBSpeechParamFromNVRam(&eSphParamNB);
        SpeechEnhancementController::GetInstance()->SetNBSpeechParametersToAllModem(&eSphParamNB);
#if defined(MTK_WB_SPEECH_SUPPORT)
        mAudioCustParamClient->GetWBSpeechParamFromNVRam(&eSphParamWB);
        SpeechEnhancementController::GetInstance()->SetWBSpeechParametersToAllModem(&eSphParamWB);
#endif
    }

    if (isPhoneCallOpen() == true) { // get output device for in_call, and set speech mode
        UpdateSpeechMode();
    }
#endif
    return NO_ERROR;
}

status_t AudioALSAStreamManager::UpdateSpeechLpbkParams() {
#if !defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    ALOGD("%s()", __FUNCTION__);
    AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
    AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT  eSphParamNBLpbk;
    mAudioCustParamClient->GetNBSpeechParamFromNVRam(&eSphParamNB);
    mAudioCustParamClient->GetNBSpeechLpbkParamFromNVRam(&eSphParamNBLpbk);
    SpeechEnhancementController::GetInstance()->SetNBSpeechLpbkParametersToAllModem(&eSphParamNB, &eSphParamNBLpbk);
    //no need to set speech mode, only for loopback parameters update
#endif
    return NO_ERROR;
}

status_t AudioALSAStreamManager::UpdateMagiConParams() {
    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() < 2) {
        ALOGW("-%s(), MagiConference Not Support", __FUNCTION__);
        return INVALID_OPERATION;
    }

    ALOGD("%s()", __FUNCTION__);
#if defined(MTK_MAGICONFERENCE_SUPPORT)
#ifndef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
    AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT eSphParamMagiCon;
    mAudioCustParamClient->GetMagiConSpeechParamFromNVRam(&eSphParamMagiCon);
    SpeechEnhancementController::GetInstance()->SetMagiConSpeechParametersToAllModem(&eSphParamMagiCon);

    if (isPhoneCallOpen() == true) { // get output device for in_call, and set speech mode
        UpdateSpeechMode();
    }

#endif
    return NO_ERROR;
#else
    ALOGW("-%s(), MagiConference Not Support", __FUNCTION__);
    return INVALID_OPERATION;
#endif
}

status_t AudioALSAStreamManager::UpdateHACParams() {
    ALOGD("%s()", __FUNCTION__);
#if defined(MTK_HAC_SUPPORT)
#ifndef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
    AUDIO_CUSTOM_HAC_PARAM_STRUCT eSphParamHAC;
    mAudioCustParamClient->GetHACSpeechParamFromNVRam(&eSphParamHAC);
    SpeechEnhancementController::GetInstance()->SetHACSpeechParametersToAllModem(&eSphParamHAC);

    if (isPhoneCallOpen() == true) { // get output device for in_call, and set speech mode
        UpdateSpeechMode();
    }

#endif
    return NO_ERROR;
#else
    ALOGW("-%s(), HAC Not Support", __FUNCTION__);
    return INVALID_OPERATION;

#endif
}

status_t AudioALSAStreamManager::UpdateDualMicParams() {
    ALOGD("%s()", __FUNCTION__);
#ifndef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
    AUDIO_CUSTOM_EXTRA_PARAM_STRUCT eSphParamDualMic;
    mAudioCustParamClient->GetDualMicSpeechParamFromNVRam(&eSphParamDualMic);
    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        SpeechEnhancementController::GetInstance()->SetDualMicSpeechParametersToAllModem(&eSphParamDualMic);
    }

    if (isPhoneCallOpen() == true) { // get output device for in_call, and set speech mode
        UpdateSpeechMode();
    }

#endif
    return NO_ERROR;
}

int AudioALSAStreamManager::setVolumeIndex(int stream, int device, int index) {
    ALOGD("%s() stream= %x, device= %x, index= %x", __FUNCTION__, stream, device, index);
    if (stream == 0) { //stream voice call/voip call
        if (isPhoneCallOpen() == true) {
            if (index == 0) {
                mSpeechPhoneCallController->setDlMute(true);
            } else if (mSpeechPhoneCallController->getDlMute() == true) {
                mSpeechPhoneCallController->setDlMute(false);
            }
            mSpeechDriverFactory->GetSpeechDriver()->setMDVolumeIndex(stream, device, index);
        } else {
            SpeechDriverInterface *pSpeechDriver = NULL;
            for (int mdIndex = MODEM_1; mdIndex < NUM_MODEM; mdIndex++) {
                pSpeechDriver = mSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)mdIndex);
                if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
                    pSpeechDriver->setMDVolumeIndex(stream, device, index);
                }
            }
        }
    }
    mVolumeIndex = index;
    return NO_ERROR;
}

status_t AudioALSAStreamManager::UpdateSpeechMode() {
    ALOGD("%s()", __FUNCTION__);
    //tina todo
    const audio_devices_t output_device = (audio_devices_t)AudioALSASpeechPhoneCallController::getInstance()->getAdjustedOutputDevice();
    const audio_devices_t input_device  = (audio_devices_t)AudioALSASpeechPhoneCallController::getInstance()->getAdjustedInputDevice();
    mSpeechDriverFactory->GetSpeechDriver()->SetSpeechMode(input_device, output_device);

    return NO_ERROR;
}

status_t AudioALSAStreamManager::UpdateSpeechVolume() {
    ALOGD("%s()", __FUNCTION__);
    mAudioALSAVolumeController->initVolumeController();

    if (isPhoneCallOpen() == true) {
        //TINA TODO GET DEVICE
        int32_t outputDevice = (audio_devices_t)AudioALSASpeechPhoneCallController::getInstance()->getOutputDevice();
        AudioALSASpeechPhoneCallController *pSpeechPhoneCallController = AudioALSASpeechPhoneCallController::getInstance();
#ifndef MTK_AUDIO_GAIN_TABLE
        mAudioALSAVolumeController->setVoiceVolume(mAudioALSAVolumeController->getVoiceVolume(),
                                                   getModeForGain(), (uint32)outputDevice);
#endif
        switch (outputDevice) {
        case AUDIO_DEVICE_OUT_WIRED_HEADSET : {
#ifdef  MTK_TTY_SUPPORT
            if (pSpeechPhoneCallController->getTtyMode() == AUD_TTY_VCO) {
                mAudioALSAVolumeController->ApplyMicGain(Normal_Mic, getModeForGain());
            } else if (pSpeechPhoneCallController->getTtyMode() == AUD_TTY_HCO || pSpeechPhoneCallController->getTtyMode() == AUD_TTY_FULL) {
                mAudioALSAVolumeController->ApplyMicGain(TTY_CTM_Mic, getModeForGain());
            } else {
                mAudioALSAVolumeController->ApplyMicGain(Headset_Mic, getModeForGain());
            }
#else
            mAudioALSAVolumeController->ApplyMicGain(Headset_Mic, getModeForGain());
#endif
            break;
        }
        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE : {
#ifdef  MTK_TTY_SUPPORT
            if (pSpeechPhoneCallController->getTtyMode() == AUD_TTY_VCO) {
                mAudioALSAVolumeController->ApplyMicGain(Normal_Mic, getModeForGain());
            } else if (pSpeechPhoneCallController->getTtyMode() == AUD_TTY_HCO || pSpeechPhoneCallController->getTtyMode() == AUD_TTY_FULL) {
                mAudioALSAVolumeController->ApplyMicGain(TTY_CTM_Mic, getModeForGain());
            } else {
                mAudioALSAVolumeController->ApplyMicGain(Headset_Mic, getModeForGain());
            }
#else
            mAudioALSAVolumeController->ApplyMicGain(Headset_Mic, getModeForGain());
#endif
            break;
        }
        case AUDIO_DEVICE_OUT_SPEAKER: {
#ifdef  MTK_TTY_SUPPORT
            if (pSpeechPhoneCallController->getTtyMode() == AUD_TTY_VCO) {
                mAudioALSAVolumeController->ApplyMicGain(Normal_Mic, getModeForGain());
            } else if (pSpeechPhoneCallController->getTtyMode() == AUD_TTY_HCO || pSpeechPhoneCallController->getTtyMode() == AUD_TTY_FULL) {
                mAudioALSAVolumeController->ApplyMicGain(TTY_CTM_Mic, getModeForGain());
            } else {
                mAudioALSAVolumeController->ApplyMicGain(Handfree_Mic, getModeForGain());
            }
#else
            mAudioALSAVolumeController->ApplyMicGain(Handfree_Mic, getModeForGain());
#endif
            break;
        }
        case AUDIO_DEVICE_OUT_EARPIECE: {
            mAudioALSAVolumeController->ApplyMicGain(Normal_Mic, getModeForGain());
            break;
        }
        default: {
            break;
        }
        }
    } else {
        setMasterVolume(mAudioALSAVolumeController->getMasterVolume());
    }
    return NO_ERROR;

}

status_t AudioALSAStreamManager::SetVCEEnable(bool bEnable) {
    ALOGD("%s()", __FUNCTION__);
    SpeechEnhancementController::GetInstance()->SetDynamicMaskOnToAllModem(SPH_ENH_DYNAMIC_MASK_VCE, bEnable);
    return NO_ERROR;

}

status_t AudioALSAStreamManager::SetMagiConCallEnable(bool bEnable) {
    ALOGD("%s(), bEnable=%d", __FUNCTION__, bEnable);

    // enable/disable flag
    SpeechEnhancementController::GetInstance()->SetMagicConferenceCallOn(bEnable);
    if (isPhoneCallOpen() == true) { // get output device for in_call, and set speech mode
        UpdateSpeechMode();
    }

    return NO_ERROR;

}

bool AudioALSAStreamManager::GetMagiConCallEnable(void) {
    bool bEnable = SpeechEnhancementController::GetInstance()->GetMagicConferenceCallOn();
    ALOGD("-%s(), bEnable=%d", __FUNCTION__, bEnable);

    return bEnable;
}

status_t AudioALSAStreamManager::SetHACEnable(bool bEnable) {
    ALOGD("%s(), bEnable=%d", __FUNCTION__, bEnable);

    // enable/disable flag
    SpeechEnhancementController::GetInstance()->SetHACOn(bEnable);
    if (isPhoneCallOpen() == true) { // get output device for in_call, and set speech mode
        UpdateSpeechMode();
    }

    return NO_ERROR;

}

bool AudioALSAStreamManager::GetHACEnable(void) {
    bool bEnable = SpeechEnhancementController::GetInstance()->GetHACOn();
    ALOGD("-%s(), bEnable=%d", __FUNCTION__, bEnable);

    return bEnable;
}

/**
 * reopen Phone Call audio path according to RIL mapped modem notify
 */
int AudioALSAStreamManager::phoneCallRefreshModem(const char *rilMappedMDName) {
    bool isPhoneCallNeedReopen = false;
    modem_index_t rilMappedMDIdx = MODEM_1;
    audio_mode_t currentAudioMode = getMode();

    if (rilMappedMDName != NULL) {
        if (isPhoneCallOpen()) {
            if (strcmp("MD1", rilMappedMDName) == 0) {
                rilMappedMDIdx = MODEM_1;
            } else if (strcmp("MD3", rilMappedMDName) == 0) {
                rilMappedMDIdx = MODEM_EXTERNAL;
            } else {
                ALOGW("%s(), Invalid rilMappedMDName=%s, currentAudioMode(%d), isPhoneCallOpen(%d)",
                      __FUNCTION__, rilMappedMDName, currentAudioMode, isPhoneCallOpen());
                return -EINVAL;
            }
            isPhoneCallNeedReopen = mSpeechPhoneCallController->checkReopen(rilMappedMDIdx);
        }
        ALOGV("%s(), rilMappedMDName=%s, currentAudioMode(%d), isPhoneCallNeedReopen(%d)",
              __FUNCTION__, rilMappedMDName, currentAudioMode, isPhoneCallNeedReopen);
    } else {
        ALOGW("%s(), Invalid rilMappedMDName=NULL, currentAudioMode(%d)", __FUNCTION__, currentAudioMode);
        return -EINVAL;
    }
    if (isPhoneCallNeedReopen) {
        ALOGD("%s(), rilMappedMDIdx(%d), currentAudioMode(%d), start to reopen",
              __FUNCTION__, rilMappedMDIdx, currentAudioMode);
        phoneCallReopen();
    } else {
        ALOGD("-%s(), rilMappedMDName=%s, currentAudioMode(%d), no need to reopen",
              __FUNCTION__, rilMappedMDName, currentAudioMode);
    }
    return 0;
}

/**
 * reopen Phone Call audio path
 */
int AudioALSAStreamManager::phoneCallReopen() {
    AL_AUTOLOCK(mStreamVectorLock);
    audio_mode_t currentAudioMode = getMode();
    ALOGD("+%s(), currentAudioMode(%d)", __FUNCTION__, currentAudioMode);

    if (isPhoneCallOpen()) {
        setAllStreamsSuspend(true, true);
        standbyAllStreams(true);
        AL_AUTOLOCK(mLock);
        mSpeechPhoneCallController->setMicMute(true);
        const audio_devices_t phonecallOutputDevice = mSpeechPhoneCallController->getOutputDevice();
        const audio_devices_t phonecallInputputDevice = mSpeechPhoneCallController->getAdjustedInputDevice();

        mSpeechPhoneCallController->close();
        mSpeechPhoneCallController->open(currentAudioMode, phonecallOutputDevice, phonecallInputputDevice);

        mAudioALSAVolumeController->setVoiceVolume(mAudioALSAVolumeController->getVoiceVolume(),
                                                   getModeForGain(), phonecallOutputDevice);
        mSpeechPhoneCallController->setMicMute(mMicMute);
        setAllStreamsSuspend(false, true);
        ALOGD("-%s(), currentAudioMode(%d), phonecallOutputDevice(0x%x), reopen end",
              __FUNCTION__, currentAudioMode, phonecallOutputDevice);
        return 0;
    } else {
        ALOGW("-%s(), isPhoneCallOpen(%d) skip reopen.", __FUNCTION__, isPhoneCallOpen());
        return -EINVAL;
    }
}

/**
 * update Phone Call phone id
 */
int AudioALSAStreamManager::phoneCallUpdatePhoneId(const phone_id_t phoneId) {
    if (phoneId != PHONE_ID_0 && phoneId != PHONE_ID_1) {
        return -EINVAL;
    }
    if (isPhoneCallOpen()) {
        phone_id_t currentPhoneId = mSpeechPhoneCallController->getPhoneId();

        if (phoneId != currentPhoneId) {
            ALOGD("%s(), phoneId(%d->%d), mAudioMode(%d), isModeInPhoneCall(%d)",
                  __FUNCTION__, currentPhoneId, phoneId, mAudioMode, isPhoneCallOpen());
            mSpeechPhoneCallController->setPhoneId(phoneId);
            modem_index_t newMDIdx = mSpeechPhoneCallController->getIdxMDByPhoneId(phoneId);
            if (newMDIdx == MODEM_EXTERNAL) {
                phoneCallRefreshModem("MD3");
            } else {
                phoneCallRefreshModem("MD1");
            }
        }
    } else {
        mSpeechPhoneCallController->setPhoneId(phoneId);
    }
    return 0;
}

status_t AudioALSAStreamManager::SetBtHeadsetName(const char *btHeadsetName) {
    if (mBtHeadsetName) {
        free((void *)mBtHeadsetName);
        mBtHeadsetName = NULL;
    }
    if (btHeadsetName) {
        mBtHeadsetName = strdup(btHeadsetName);
        ALOGV("%s(), mBtHeadsetName = %s", __FUNCTION__, mBtHeadsetName);
    }
    return NO_ERROR;
}

const char *AudioALSAStreamManager::GetBtHeadsetName() {
    return mBtHeadsetName;
}

status_t AudioALSAStreamManager::SetBtHeadsetNrec(bool bEnable) {
#if defined(CONFIG_MT_ENG_BUILD)
    // Used for testing the BT_NREC_OFF case
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_SET_BT_NREC, property_value, "-1");
    int btNrecProp = atoi(property_value);
    if (btNrecProp != -1) {
        bEnable = btNrecProp;
        ALOGD("%s(), force set the BT headset NREC = %d", __FUNCTION__, bEnable);
    }
#endif

    ALOGV("%s(), bEnable=%d", __FUNCTION__, bEnable);

    // enable/disable flag
    if (SpeechEnhancementController::GetInstance()->GetBtHeadsetNrecOn() != bEnable) {
        SpeechEnhancementController::GetInstance()->SetBtHeadsetNrecOnToAllModem(bEnable);
    }

    return NO_ERROR;

}

bool AudioALSAStreamManager::GetBtHeadsetNrecStatus(void) {
    bool bEnable = SpeechEnhancementController::GetInstance()->GetBtHeadsetNrecOn();
    ALOGD("-%s(), bEnable=%d", __FUNCTION__, bEnable);

    return bEnable;
}

status_t AudioALSAStreamManager::Enable_DualMicSettng(sph_enh_dynamic_mask_t sphMask, bool bEnable) {
    ALOGD("%s(), bEnable=%d", __FUNCTION__, bEnable);

    SpeechEnhancementController::GetInstance()->SetDynamicMaskOnToAllModem(sphMask, bEnable);
    return NO_ERROR;

}

status_t AudioALSAStreamManager::Set_LSPK_DlMNR_Enable(sph_enh_dynamic_mask_t sphMask, bool bEnable) {
    ALOGD("%s(), bEnable=%d", __FUNCTION__, bEnable);

    Enable_DualMicSettng(sphMask, bEnable);

    if (SpeechEnhancementController::GetInstance()->GetMagicConferenceCallOn() == true &&
        SpeechEnhancementController::GetInstance()->GetDynamicMask(sphMask) == true) {
        ALOGE("Cannot open MagicConCall & LoudSpeaker DMNR at the same time!!");
    }
    return NO_ERROR;

}

#ifdef MTK_VOW_SUPPORT
bool AudioALSAStreamManager::getVoiceWakeUpNeedOn() {
    return mVoiceWakeUpNeedOn;
}

status_t AudioALSAStreamManager::setVoiceWakeUpNeedOn(const bool enable) {
    ALOGD("+%s(), mVoiceWakeUpNeedOn: %d => %d ", __FUNCTION__, mVoiceWakeUpNeedOn, enable);
    AL_AUTOLOCK(mStreamVectorLock);

    if (enable == mVoiceWakeUpNeedOn) {
        ALOGW("-%s(), enable(%d) == mVoiceWakeUpNeedOn(%d), return", __FUNCTION__, enable, mVoiceWakeUpNeedOn);
        return INVALID_OPERATION;
    }

    if (enable == true) {
        if (mStreamInVector.size() != 0 || mForceDisableVoiceWakeUpForSetMode == true) {
            ALOGD("-%s(), mStreamInVector.size() = %zu, mForceDisableVoiceWakeUpForSetMode = %d, return", __FUNCTION__, mStreamInVector.size(), mForceDisableVoiceWakeUpForSetMode);
        } else {
            if (mAudioALSAVoiceWakeUpController->getVoiceWakeUpEnable() == false) {
                mAudioALSAVoiceWakeUpController->setVoiceWakeUpEnable(true);
            }
        }
    } else {
        if (mAudioALSAVoiceWakeUpController->getVoiceWakeUpEnable() == true) {
            mAudioALSAVoiceWakeUpController->setVoiceWakeUpEnable(false);
        }
    }

    mVoiceWakeUpNeedOn = enable;

    ALOGD("-%s(), mVoiceWakeUpNeedOn: %d", __FUNCTION__, mVoiceWakeUpNeedOn);
    return NO_ERROR;
}
#endif

void AudioALSAStreamManager::UpdateDynamicFunctionMask(void) {
    ALOGD("+%s()", __FUNCTION__);
    if (mStreamInVector.size() > 0) {
        for (size_t i = 0; i < mStreamInVector.size(); i++) {
            mStreamInVector[i]->UpdateDynamicFunctionMask();
        }
    }
    ALOGD("-%s()", __FUNCTION__);
}

bool AudioALSAStreamManager::EnableBesRecord(void) {
    bool bRet = false;
    if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_HD_RECORD) > 0) {
        bRet = true;
        ALOGD_IF(mLogEnable, "%s(), %x", __FUNCTION__, bRet);
    }

    return bRet;
}

status_t AudioALSAStreamManager::setScreenState(bool mode) {
    AL_AUTOLOCK(mStreamVectorLock);
    AudioALSAStreamOut *pAudioALSAStreamOut = NULL;
    AudioALSAStreamIn *pAudioALSAStreamIn = NULL;

    for (size_t i = 0; i < mStreamOutVector.size(); i++) {
        pAudioALSAStreamOut = mStreamOutVector[i];
        pAudioALSAStreamOut->setScreenState(mode);
    }

    for (size_t i = 0; i < mStreamInVector.size(); i++) {
        pAudioALSAStreamIn = mStreamInVector[i];
        // Update IRQ period when all streamin are Normal Record
        if ((pAudioALSAStreamIn->getStreamInCaptureHandler() != NULL) &&
            (((pAudioALSAStreamIn->getInputFlags() & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_MMAP_NOIRQ)) != 0) ||
             (pAudioALSAStreamIn->getStreamInCaptureHandler()->getCaptureHandlerType() != CAPTURE_HANDLER_NORMAL))) {
            //break here because sharing the same dataprovider
            break;
        }
        if (i == (mStreamInVector.size() - 1)) {
            ALOGE("%s, mStreamInVector[%zu]->getInputFlags() = 0x%x\n", __FUNCTION__, i, mStreamInVector[i]->getInputFlags());
            pAudioALSAStreamIn->setLowLatencyMode(mode);
        }

    }
    return NO_ERROR;
}

using android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioHost;
using android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration;

typedef struct {
    uint32_t codec_type; // codec types ex: SBC/AAC/LDAC/APTx
    uint32_t sample_rate;    // sample rate, ex: 44.1/48.88.2/96 KHz
    uint32_t encoded_audio_bitrate;  // encoder audio bitrates
    uint16_t max_latency;    // maximum latency
    uint16_t scms_t_enable;  // content protection enable
    uint16_t acl_hdl;    // connection handle
    uint16_t l2c_rcid;   // l2cap channel id
    uint16_t mtu;        // mtu size
    unsigned char bits_per_sample; // bits per sample, ex: 16/24/32
    unsigned char ch_mode;         // None:0, Left:1, Right:2
    unsigned char codec_info[32];  //codec specific information
} A2DP_CODEC_INFO_T;

static A2DP_CODEC_INFO_T a2dp_codecinfo;

void AudioALSAStreamManager::setBluetoothAudioOffloadParam(const sp<IBluetoothAudioHost> &hostIf,
                                                           const CodecConfiguration &codecConfig,
                                                           bool on) {
    ALOGD("+%s()", __FUNCTION__);
    if (on == true) {
        mBluetoothAudioOffloadHostIf = hostIf;
        a2dp_codecinfo.codec_type = (uint32_t)codecConfig.codecType;
        a2dp_codecinfo.sample_rate = (uint32_t)codecConfig.sampleRate;
        a2dp_codecinfo.encoded_audio_bitrate = codecConfig.encodedAudioBitrate;
        a2dp_codecinfo.mtu = codecConfig.peerMtu;
        a2dp_codecinfo.bits_per_sample = (uint32_t)codecConfig.bitsPerSample;
        a2dp_codecinfo.ch_mode = (uint32_t)codecConfig.channelMode;

        if (a2dp_codecinfo.codec_type == 0x1) { // sbc
            a2dp_codecinfo.codec_info[0] = codecConfig.codecSpecific.sbcData.codecParameters;
            a2dp_codecinfo.codec_info[1] = codecConfig.codecSpecific.sbcData.minBitpool;
            a2dp_codecinfo.codec_info[2] = codecConfig.codecSpecific.sbcData.maxBitpool;
        }

        mBluetoothAudioOffloadSession++;
        ALOGD("codecType=%u, sampleRate=%u, bitsPerSample=%u, channelMode=%u",
              (uint32_t)a2dp_codecinfo.codec_type,
              (uint32_t)a2dp_codecinfo.sample_rate,
              (unsigned char)a2dp_codecinfo.bits_per_sample,
              (unsigned char)a2dp_codecinfo.ch_mode);
        ALOGD("encodedAudioBitrate=%u, peerMtu=%u",
              (uint32_t)a2dp_codecinfo.encoded_audio_bitrate,
              (uint16_t)a2dp_codecinfo.mtu);
        ALOGD("codecParameters=%u, minBitpool=%u, maxBitpool=%u",
              (unsigned char)a2dp_codecinfo.codec_info[0],
              (unsigned char)a2dp_codecinfo.codec_info[1],
              (unsigned char)a2dp_codecinfo.codec_info[2]);
    } else {
        mBluetoothAudioOffloadSession--;
        if (mBluetoothAudioOffloadSession < 0) {
            ALOGW("%s() Error: mBluetoothAudioOffloadSession < 0, reset to 0!", __FUNCTION__);
            mBluetoothAudioOffloadSession = 0;
        }
        if (mBluetoothAudioOffloadSession == 0) {
            mBluetoothAudioOffloadHostIf = NULL;
        }
    }
    ALOGD("-%s() mBluetoothAudioOffloadSession=%d", __FUNCTION__, mBluetoothAudioOffloadSession);
}

void *AudioALSAStreamManager::getBluetoothAudioCodecInfo(void) {
    return (void *)&a2dp_codecinfo;
}

uint32_t AudioALSAStreamManager::getBluetoothAudioCodecType(void) {
    if (mBluetoothAudioOffloadSession > 0) {
        ALOGD("%s(), codec_type = %d", __FUNCTION__, a2dp_codecinfo.codec_type);
        return a2dp_codecinfo.codec_type;
    } else {
        ALOGD("%s(), mBluetoothAudioOffloadSession = %d, set codec_type unknown", __FUNCTION__,
              mBluetoothAudioOffloadSession);
        return 0; // unknown
    }
}

void AudioALSAStreamManager::setA2dpSuspend(bool on) {
#ifdef MTK_AUDIODSP_SUPPORT
    static AudioLock A2dpSuspendLock;
    AL_AUTOLOCK(A2dpSuspendLock);

    int32_t outputDevice = (audio_devices_t)AudioALSAHardwareResourceManager::getInstance()->getOutputDevice();
    ALOGD("%s(), suspend = %d, outputDevice = 0x%x", __FUNCTION__, on, outputDevice);

    if ((mBluetoothAudioOffloadHostIf != NULL) &&
        (mBluetoothAudioOffloadSession > 0)) {
        if (on) {
            if (isModeInPhoneCall() || isPhoneCallOpen() || isModeInVoipCall() || (outputDevice & AUDIO_DEVICE_OUT_ALL_A2DP)) {
                mBluetoothAudioOffloadHostIf->suspendStream();
            }
        } else {
            if (outputDevice & AUDIO_DEVICE_OUT_ALL_A2DP) {
                mBluetoothAudioOffloadHostIf->startStream();
            }
        }
    }
#else
    ALOGV("%s(), suspend = %d", __FUNCTION__, on);
#endif
}

void AudioALSAStreamManager::setA2dpSuspendStatus(int status) {
    AL_AUTOLOCK(mA2dpSuspendStatusLock);
    ALOGD("%s() status=%d", __FUNCTION__, status);
#ifdef MTK_AUDIODSP_SUPPORT
    // 0: Start SUCCESS, 1: Start FAILURE, 3: Start PENDING
    // 4: Suspend SUCCESS, 5: Suspend FAILURE, 7: Suspend PENDING
    mA2dpSuspend = status;

    // send a2dp PAUSE control to DSP
    struct ipi_msg_t ipi_msg;
    if (mA2dpSuspend == 4) { // 4: Suspend SUCCESS
        audio_send_ipi_msg(&ipi_msg,
                           TASK_SCENE_DATAPROVIDER, AUDIO_IPI_LAYER_TO_DSP,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_DATAPROVIDER_SUSPEND, 1, 0,
                           NULL);
    } else if (mA2dpSuspend == 0) { // 0: Start SUCCESS
        audio_send_ipi_msg(&ipi_msg,
                           TASK_SCENE_DATAPROVIDER, AUDIO_IPI_LAYER_TO_DSP,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_DATAPROVIDER_SUSPEND, 0, 0,
                           NULL);
    }
#endif
}

int AudioALSAStreamManager::getA2dpSuspendStatus(void) {
    AL_AUTOLOCK(mA2dpSuspendStatusLock);

#ifdef MTK_AUDIODSP_SUPPORT
    return mA2dpSuspend;
#else
    return false;
#endif
}

bool AudioALSAStreamManager::getA2dpPlaybackPaused(void) {
    AL_AUTOLOCK(mA2dpPlaybackPausedLock);

#ifdef MTK_AUDIODSP_SUPPORT
    return mA2dpPlaybackPaused;
#else
    return false;
#endif
}

void AudioALSAStreamManager::setA2dpPlaybackPaused(bool status) {
    AL_AUTOLOCK(mA2dpPlaybackPausedLock);
    ALOGD("%s() status=%d", __FUNCTION__, status);
#ifdef MTK_AUDIODSP_SUPPORT
    mA2dpPlaybackPaused = status;
#endif
}

status_t AudioALSAStreamManager::setBypassDLProcess(bool flag) {
    AL_AUTOLOCK(mLock);
    AudioALSAStreamOut *pAudioALSAStreamOut = NULL;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    set_aurisys_on(!(bool)flag);
#endif

    mBypassPostProcessDL = flag;

    return NO_ERROR;
}

status_t AudioALSAStreamManager::EnableSphStrmByDevice(audio_devices_t output_devices) {
    AudioALSASpeechStreamController::getInstance()->SetStreamOutputDevice(output_devices);
    if (isPhoneCallOpen()) {
        if ((output_devices & AUDIO_DEVICE_OUT_SPEAKER) != 0) {
            AudioALSASpeechStreamController::getInstance()->EnableSpeechStreamThread(true);
        }
    }
    return NO_ERROR;
}

status_t AudioALSAStreamManager::DisableSphStrmByDevice(audio_devices_t output_devices) {
    AudioALSASpeechStreamController::getInstance()->SetStreamOutputDevice(output_devices);
    if (isModeInPhoneCallSupportEchoRef(mAudioMode) == true) {
        if (AudioALSASpeechStreamController::getInstance()->IsSpeechStreamThreadEnable() == true) {
            AudioALSASpeechStreamController::getInstance()->EnableSpeechStreamThread(false);
        }
    }
    return NO_ERROR;
}

status_t AudioALSAStreamManager::EnableSphStrm(audio_mode_t new_mode) {
    ALOGD("%s new_mode = %d", __FUNCTION__, new_mode);
    if ((new_mode < AUDIO_MODE_NORMAL) || (new_mode > AUDIO_MODE_MAX)) {
        return BAD_VALUE;
    }

    if (isPhoneCallOpen() == true) {
        if ((AudioALSASpeechStreamController::getInstance()->GetStreamOutputDevice() & AUDIO_DEVICE_OUT_SPEAKER) != 0 &&
            (AudioALSASpeechStreamController::getInstance()->IsSpeechStreamThreadEnable() == false)) {
            AudioALSASpeechStreamController::getInstance()->EnableSpeechStreamThread(true);
        }
    }
    return NO_ERROR;
}

status_t AudioALSAStreamManager::DisableSphStrm(audio_mode_t new_mode) {
    ALOGD("%s new_mode = %d", __FUNCTION__, new_mode);
    if ((new_mode < AUDIO_MODE_NORMAL) || (new_mode > AUDIO_MODE_MAX)) {
        return BAD_VALUE;
    }
    if (new_mode == mAudioMode) {
        ALOGW("-%s(), mAudioMode: %d == %d, return", __FUNCTION__, mAudioMode, new_mode);
        return BAD_VALUE;
    }

    if (isModeInPhoneCallSupportEchoRef(mAudioMode) == true) {
        if (AudioALSASpeechStreamController::getInstance()->IsSpeechStreamThreadEnable() == true) {
            AudioALSASpeechStreamController::getInstance()->EnableSpeechStreamThread(false);
        }
    }
    return NO_ERROR;
}

bool AudioALSAStreamManager::IsSphStrmSupport(void) {
    char property_value[PROPERTY_VALUE_MAX];
    bool Currentsupport = false;
    property_get("vendor.streamout.speech_stream.enable", property_value, "1");
    int speech_stream = atoi(property_value);
#if defined(MTK_MAXIM_SPEAKER_SUPPORT)&&defined(MTK_SPEAKER_MONITOR_SPEECH_SUPPORT)
    Currentsupport = true;
#endif
    ALOGD("%s = %d Currentsupport = %d", __FUNCTION__, speech_stream, Currentsupport);
    return (speech_stream & Currentsupport);
}

bool AudioALSAStreamManager::isModeInPhoneCallSupportEchoRef(const audio_mode_t audio_mode) {
    if (audio_mode == AUDIO_MODE_IN_CALL) {
        return true;
    } else {
        return false;
    }
}

status_t AudioALSAStreamManager::setParametersToStreamOut(const String8 &keyValuePairs) { // TODO(Harvey
    if (mStreamOutVector.size() == 0) {
        return INVALID_OPERATION;
    }

    AudioALSAStreamOut *pAudioALSAStreamOut = NULL;
    for (size_t i = 0; i < mStreamOutVector.size() ; i++) {
        pAudioALSAStreamOut = mStreamOutVector[i];
        pAudioALSAStreamOut->setParameters(keyValuePairs);
    }

    return NO_ERROR;
}

status_t AudioALSAStreamManager::setParameters(const String8 &keyValuePairs, int IOport) { // TODO(Harvey)
    status_t status = PERMISSION_DENIED;
    ssize_t index = -1;

    ALOGD("+%s(), IOport = %d, keyValuePairs = %s", __FUNCTION__, IOport, keyValuePairs.string());

    index = mStreamOutVector.indexOfKey(IOport);
    if (index >= 0) {
        ALOGV("Send to mStreamOutVector[%zu]", index);
        AudioALSAStreamOut *pAudioALSAStreamOut = mStreamOutVector.valueAt(index);
        status = pAudioALSAStreamOut->setParameters(keyValuePairs);
        ALOGV("-%s()", __FUNCTION__);
        return status;
    }

    index = mStreamInVector.indexOfKey(IOport);
    if (index >= 0) {
        ALOGV("Send to mStreamInVector [%zu]", index);
        AudioALSAStreamIn *pAudioALSAStreamIn = mStreamInVector.valueAt(index);
        status = pAudioALSAStreamIn->setParameters(keyValuePairs);
        ALOGV("-%s()", __FUNCTION__);
        return status;
    }

    ALOGE("-%s(), do nothing, return", __FUNCTION__);
    return status;
}

void AudioALSAStreamManager::updateDeviceConnectionState(audio_devices_t device, bool connect) {
    if ((device & AUDIO_DEVICE_BIT_IN) == false) {
        mAvailableOutputDevices = connect ? mAvailableOutputDevices | device : mAvailableOutputDevices & !device;
    }
}

bool AudioALSAStreamManager::getDeviceConnectionState(audio_devices_t device) {
    if ((device & AUDIO_DEVICE_BIT_IN) == false) {
        return !!(mAvailableOutputDevices & device);
    }
    return false;
}

void AudioALSAStreamManager::setCustScene(const String8 scene) {
    mCustScene = scene;
#if defined(MTK_NEW_VOL_CONTROL)
    AudioMTKGainController::getInstance()->setScene(scene.string());
#endif
}

bool AudioALSAStreamManager::isEchoRefUsing() {
    if (isModeInVoipCall() == true) {
        return true;
    }
    if (mStreamInVector.size() > 1) {
        for (size_t i = 0; i < mStreamInVector.size(); i++) {
            if ((mStreamInVector[i]->getStreamAttribute()->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION)
                || (mStreamInVector[i]->getStreamAttribute()->NativePreprocess_Info.PreProcessEffect_AECOn == true)
                || (mStreamInVector[i]->getStreamAttribute()->input_source == AUDIO_SOURCE_CUSTOMIZATION1) //MagiASR enable AEC
                || (mStreamInVector[i]->getStreamAttribute()->input_source == AUDIO_SOURCE_CUSTOMIZATION2)) {
                return true;
            }
        }
    }
    return false;
}

/*==============================================================================
 *                     ADSP Recovery
 *============================================================================*/

void AudioALSAStreamManager::audioDspStopWrap(void *arg) {
#ifndef MTK_AUDIO_DSP_RECOVERY_SUPPORT
    ALOGD("%s() not support!! arg %p", __FUNCTION__, arg);
#else
    AudioALSAStreamManager *mgr = static_cast<AudioALSAStreamManager *>(arg);

    if (mgr != NULL) {
        mgr->audioDspStatusUpdate(false);
    }
#endif
}


void AudioALSAStreamManager::audioDspReadyWrap(void *arg) {
#ifndef MTK_AUDIO_DSP_RECOVERY_SUPPORT
    ALOGD("%s() not support!! arg %p", __FUNCTION__, arg);
#else
    AudioALSAStreamManager *mgr = static_cast<AudioALSAStreamManager *>(arg);

    if (mgr != NULL) {
        mgr->audioDspStatusUpdate(true);
    }
#endif
}


void AudioALSAStreamManager::audioDspStatusUpdate(const bool adspReady) {
#ifndef MTK_AUDIO_DSP_RECOVERY_SUPPORT
    ALOGD("%s() not support!! adspReady %d", __FUNCTION__, adspReady);
#else
    static AudioLock updateLock;
    AL_AUTOLOCK(updateLock);

    // competetor: stream out write() / stream in read() / alsa hw setMode()
    add_audio_dsp_recovery_lock_cnt();
    LOCK_ALOCK_MS(get_adsp_recovery_lock(), MAX_RECOVERY_LOCK_TIMEOUT_MS);
    dec_audio_dsp_recovery_lock_cnt();

    // (in recovery && not ready) || (not in recovery && ready)
    if (get_audio_dsp_recovery_mode() != adspReady) {
        ALOGE("%s(), recovery %d ready %d. bypass", __FUNCTION__,
              get_audio_dsp_recovery_mode(), adspReady);
        UNLOCK_ALOCK(get_adsp_recovery_lock());
        return;
    }


    if (adspReady == false) { // adsp reboot
        set_audio_dsp_recovery_mode(true);
        standbyAllStreams(false); // stop all in/out streams
        setMode(AUDIO_MODE_NORMAL);
    } else { // adsp ready
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
        wrap_config_to_dsp();
#endif
        // if setMode 2->3
        if (mPhoneCallControllerStatusPolicy == true && isModeInPhoneCall(mAudioModePolicy) == false) {
            setMode(AUDIO_MODE_IN_CALL);
        }
        setMode(mAudioModePolicy);

        if (mPhoneCallControllerStatusPolicy == true) {
            if (mOutputStreamForCall) {
                routingOutputDevice(
                    mOutputStreamForCall,
                    mCurrentOutputDevicesForCall,
                    mOutputDevicesForCall);
            } else {
                routingOutputDevice(
                    mStreamOutVector[0],
                    AUDIO_DEVICE_OUT_EARPIECE,
                    AUDIO_DEVICE_OUT_EARPIECE);
            }
            mSpeechPhoneCallController->setMicMute(mMicMute);
        }
        set_audio_dsp_recovery_mode(false);
    }

    UNLOCK_ALOCK(get_adsp_recovery_lock());
#endif
}


void AudioALSAStreamManager::updateAudioModePolicy(const audio_mode_t new_mode) {
    if (new_mode == AUDIO_MODE_IN_CALL) {
        mPhoneCallControllerStatusPolicy = true;
    } else if (new_mode == AUDIO_MODE_NORMAL) {
        mPhoneCallControllerStatusPolicy = false;
    }

#if defined(MTK_AUDIO_DSP_RECOVERY_SUPPORT)
    ALOGD("%s(), mAudioModePolicy: %d => %d, mPhoneCallControllerStatusPolicy: %d",
          __FUNCTION__,
          mAudioModePolicy,
          new_mode,
          mPhoneCallControllerStatusPolicy);
#endif

    mAudioModePolicy = new_mode;
}


bool AudioALSAStreamManager::needEnableVoip(const stream_attribute_t *streamAttribute) {
    ALOGD("%s(), output_devices = 0x%x, flags: 0x%x, mAvailableOutputFlags: 0x%x",
          __FUNCTION__, streamAttribute->output_devices, streamAttribute->mAudioOutputFlags, mAvailableOutputFlags);
    uint32_t voipIndDevice = (AUDIO_DEVICE_OUT_EARPIECE |
                              AUDIO_DEVICE_OUT_SPEAKER |
                              AUDIO_DEVICE_OUT_WIRED_HEADSET |
                              AUDIO_DEVICE_OUT_WIRED_HEADPHONE |
                              AUDIO_DEVICE_OUT_BLUETOOTH_SCO |
                              AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET |
                              AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT);

    if ((streamAttribute->output_devices & voipIndDevice) &&
        (mAvailableOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX)) {
        return ((streamAttribute->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) != 0);
    }

    if (!(streamAttribute->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_DEEP_BUFFER)) {
        return isModeInVoipCall();
    } else {
        return false;
    }
}



} // end of namespace android

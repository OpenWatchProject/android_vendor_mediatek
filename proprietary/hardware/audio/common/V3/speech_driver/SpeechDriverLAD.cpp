#include <SpeechDriverLAD.h>

#include <SpeechMessengerECCCI.h>
#include <SpeechMessengerCCCI.h>

#if defined (AUDIO_C2K_SUPPORT)
#include <SpeechMessengerEVDO.h>
#endif

#include <CFG_AUDIO_File.h> // alps/vendor/mediatek/proprietary/custom/common/cgen/cfgfileinc/CFG_AUDIO_File.h

#include <SpeechEnhancementController.h>
#include <SpeechVMRecorder.h>

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
#include <AudioParamParser.h>
#include <SpeechParamParser.h>
#endif

//#if defined(MTK_VIBSPK_SUPPORT)
#include <AudioCompFltCustParam.h>
#include <AudioVIBSPKControl.h>
//#endif
#include <AudioALSAStreamManager.h>

#include <AudioLock.h>
#include <AudioUtility.h>
#include <AudioSmartPaController.h>
#include <AudioALSAHardwareResourceManager.h>
#include <SpeechDriverFactory.h>

//#define SPH_SKIP_A2M_BUFF_MSG
//Phone Call Test without buffer message transfer
#define WAIT_VM_RECORD_ON_CNT (20)

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
#define MODEM_DYNAMIC_PARAM
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechDriverLAD"

// refer to /alps/vendor/mediatek/proprietary/hardware/ril/libratconfig/ratconfig.c
#define CDMA "C"


namespace android {

/*==============================================================================
 *                     Const Value
 *============================================================================*/
static const uint16_t kSpeechOnWaitModemAckMaxTimeMs = 0; // 0: dont't wait
static const uint16_t kSpeechOffWaitModemAckMaxTimeMs = 500;

static const uint32_t kDefaultAcousticLoopbackDelayFrames = 12;
static const uint32_t kMaxAcousticLoopbackDelayFrames     = 64;

/*==============================================================================
 *                     Callback Function
 *============================================================================*/
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
static void callbackAudioXmlChanged(AppHandle *appHandle, const char *audioTypeName) {
    ALOGD("+%s(), audioType = %s", __FUNCTION__, audioTypeName);
    bool isSpeechParamChanged = false, onlyUpdatedDuringCall = false;
    int audioTypeUpdate = 0;
    // reload XML file
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return;
    }

    if (appOps->appHandleReloadAudioType(appHandle, audioTypeName) == APP_ERROR) {
        ALOGE("%s(), Reload xml fail!(audioType = %s)", __FUNCTION__, audioTypeName);
    } else {
        if (strcmp(audioTypeName, audioTypeNameList[AUDIO_TYPE_SPEECH]) == 0) {
            //"Speech"
            isSpeechParamChanged = true;
            onlyUpdatedDuringCall = true;
            audioTypeUpdate = (int) AUDIO_TYPE_SPEECH;
        } else if (strcmp(audioTypeName, audioTypeNameList[AUDIO_TYPE_SPEECH_DMNR]) == 0) {
            //"SpeechDMNR"
            isSpeechParamChanged = true;
            audioTypeUpdate = (int) AUDIO_TYPE_SPEECH_DMNR;
        } else if (strcmp(audioTypeName, audioTypeNameList[AUDIO_TYPE_SPEECH_GENERAL]) == 0) {
            //"SpeechGeneral"
            isSpeechParamChanged = true;
            audioTypeUpdate = (int) AUDIO_TYPE_SPEECH_GENERAL;
        }
        if (isSpeechParamChanged) {
            modem_index_t currentMD = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
            SpeechDriverLAD::GetInstance(currentMD)->updateParam(audioTypeUpdate);
        }

    }
}
#endif


/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

SpeechDriverLAD *SpeechDriverLAD::mLad1 = NULL;
SpeechDriverLAD *SpeechDriverLAD::mLad2 = NULL;
SpeechDriverLAD *SpeechDriverLAD::mLad3 = NULL;

SpeechDriverLAD *SpeechDriverLAD::GetInstance(modem_index_t modem_index) {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    SpeechDriverLAD *pLad = NULL;
    ALOGD("%s(), modem_index=%d", __FUNCTION__, modem_index);

    switch (modem_index) {
    case MODEM_1:
        if (mLad1 == NULL) {
            mLad1 = new SpeechDriverLAD(modem_index);
        }
        pLad = mLad1;
        break;
    case MODEM_2:
        if (mLad2 == NULL) {
            mLad2 = new SpeechDriverLAD(modem_index);
        }
        pLad = mLad2;
        break;
    case MODEM_EXTERNAL:
        if (mLad3 == NULL) {
            mLad3 = new SpeechDriverLAD(modem_index);
        }
        pLad = mLad3;
        break;
    default:
        ALOGE("%s: no such modem_index %d", __FUNCTION__, modem_index);
        break;
    }

    ASSERT(pLad != NULL);
    return pLad;
}

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

SpeechDriverLAD::SpeechDriverLAD(modem_index_t modem_index) {
    ALOGD("%s(), modem_index = %d", __FUNCTION__, modem_index);
    mModemIndex = modem_index;
    pCCCI = NULL;

    if (mModemIndex == MODEM_1 || mModemIndex == MODEM_2) {
        ALOGD("%s(), SpeechMessengerCCCI modem_index = %d", __FUNCTION__, modem_index);
#if defined(CCCI_FORCE_USE)
        pCCCI = new SpeechMessengerCCCI(mModemIndex, this);
#else
        pCCCI = new SpeechMessengerECCCI(mModemIndex, this);
#endif
    } else if (mModemIndex == MODEM_EXTERNAL) {
#if defined(AUDIO_C2K_SUPPORT)
        char isC2kSupported[PROPERTY_VALUE_MAX];
        property_get("ro.vendor.mtk_ps1_rat", isC2kSupported, "0"); //"0": default not support

        if (strstr(isC2kSupported, CDMA)  !=  NULL) {
            ALOGD("%s(), isC2kSupported = %s", __FUNCTION__, isC2kSupported);

            pCCCI = new SpeechMessengerEVDO(mModemIndex, this);
        } else {
            pCCCI = new SpeechMessengerECCCI(mModemIndex, this);

        }
#elif !defined(CCCI_FORCE_USE)
        ALOGE("%s(),  AUDIO_C2K_SUPPORT/CCCI_FORCE_USE is not defined!!!", __FUNCTION__);
        pCCCI = new SpeechMessengerECCCI(mModemIndex, this);

#endif
    }

    if (pCCCI == NULL) {
        ALOGE("%s(),  pCCCI == NULL!!!", __FUNCTION__);
        ASSERT(pCCCI != NULL);
    } else {
        status_t ret = pCCCI->Initial();

        if (ret == NO_ERROR) {
            RecoverModemSideStatusToInitState();
        }
    }
    // Speech mode
    mSpeechMode = SPEECH_MODE_NORMAL;

    // Record capability
    mRecordSampleRateType = RECORD_SAMPLE_RATE_08K;
    mRecordChannelType    = RECORD_CHANNEL_MONO;
    mRecordType.direction = RECORD_TYPE_UL;
    mRecordType.dlPosition= RECORD_POS_DL_END;
    mVolumeIndex = 0x3;

    mUseBtCodec = 1;

    // BT Headset NREC
    mBtHeadsetNrecOn = SpeechEnhancementController::GetInstance()->GetBtHeadsetNrecOn();

    // loopback delay frames (1 frame = 20 ms)
    mAcousticLoopbackDelayFrames = kDefaultAcousticLoopbackDelayFrames;
    mIsLpbk = false;

    // RTT
    mRttMode = 0;

#if defined(MODEM_DYNAMIC_PARAM)
    SpeechParamParser::getInstance();
#endif
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    AppHandle *mAppHandle;
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return;
    }

    /* Init AppHandle */
    ALOGD("%s() appHandleGetInstance", __FUNCTION__);
    mAppHandle = appOps->appHandleGetInstance();
    ALOGD("%s() appHandleRegXmlChangedCb", __FUNCTION__);

    /* XML changed callback process */
    appOps->appHandleRegXmlChangedCb(mAppHandle, callbackAudioXmlChanged);
#endif

}


SpeechDriverLAD::~SpeechDriverLAD() {
    pCCCI->Deinitial();
    delete pCCCI;
}

/*==============================================================================
 *                     Speech Control
 *============================================================================*/
speech_mode_t SpeechDriverLAD::GetSpeechModeByOutputDevice(const audio_devices_t output_device) {
    speech_mode_t speech_mode = SPEECH_MODE_NORMAL;
    if (audio_is_bluetooth_sco_device(output_device)) {
#if !defined(MODEM_DYNAMIC_PARAM)
        if (mBtHeadsetNrecOn == true) {
            speech_mode = SPEECH_MODE_BT_EARPHONE;
        } else {
            speech_mode = SPEECH_MODE_BT_CARKIT;
        }
#else
        speech_mode = SPEECH_MODE_BT_EARPHONE;
#endif
    } else if (output_device == AUDIO_DEVICE_OUT_SPEAKER) {
        speech_mode = SPEECH_MODE_LOUD_SPEAKER;
#if defined(MODEM_DYNAMIC_PARAM) && (defined(MTK_INCALL_HANDSFREE_DMNR) || defined(MTK_MAGICONFERENCE_SUPPORT))
        if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2 &&
            SpeechEnhancementController::GetInstance()->GetMagicConferenceCallOn() == true) {
            speech_mode = SPEECH_MODE_MAGIC_CON_CALL;
        }
#endif
    } else if (output_device == AUDIO_DEVICE_OUT_WIRED_HEADSET) {
        speech_mode = SPEECH_MODE_EARPHONE;
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
        SpeechParamParser::getInstance()->SetParamInfo(String8("ParamHeadsetPole=4;"));
#endif
    } else if (output_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        speech_mode = SPEECH_MODE_EARPHONE;
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
        SpeechParamParser::getInstance()->SetParamInfo(String8("ParamHeadsetPole=3;"));
#endif
    }
#if defined(MTK_USB_PHONECALL)
    else if (output_device == AUDIO_DEVICE_OUT_USB_DEVICE) {
        speech_mode = SPEECH_MODE_USB_AUDIO;
    }
#endif
    else if (output_device == AUDIO_DEVICE_OUT_EARPIECE) {
#if defined(MTK_HAC_SUPPORT)
        if (SpeechEnhancementController::GetInstance()->GetHACOn() == true) {
            speech_mode = SPEECH_MODE_HAC;
        } else
#endif
        {
            speech_mode = SPEECH_MODE_NORMAL;
        }
    }

    return speech_mode;
}

status_t SpeechDriverLAD::SetSpeechMode(const audio_devices_t input_device, const audio_devices_t output_device) {
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    speech_mode_t speech_mode = GetSpeechModeByOutputDevice(output_device);
    ALOGD("%s(), input_device = 0x%x, output_device = 0x%x, speech_mode = %d", __FUNCTION__, input_device, output_device, speech_mode);

    // AP side have to set speech mode before speech/record/loopback on,
    // hence we check whether modem side get all necessary speech enhancement parameters here
    // if not, re-send it !!
    if (pCCCI->CheckSpeechParamAckAllArrival() == false) {
        ALOGW("%s(), Do SetAllSpeechEnhancementInfoToModem() done. Start set speech_mode = %d", __FUNCTION__, speech_mode);
    }

    mSpeechMode = speech_mode;

    // set a unreasonable gain value s.t. the reasonable gain can be set to modem next time
    mDownlinkGain   = kUnreasonableGainValue;
    mUplinkGain     = kUnreasonableGainValue;
    mSideToneGain   = kUnreasonableGainValue;

#if defined(SPH_BT_DELAYTIME_SUPPORT)
    if (speech_mode == SPEECH_MODE_BT_EARPHONE) {
        const char *btDeviceName = AudioALSAStreamManager::getInstance()->GetBtHeadsetName();
        int btDelayMs =  SpeechParamParser::getInstance()->GetBtDelayTime(btDeviceName);
        ALOGD("%s(), btDeviceName = %s, btDelayMs=%d", __FUNCTION__, btDeviceName, btDelayMs);
        pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SET_BT_DELAY_TIME, (uint16_t)btDelayMs, 0));
    }
#endif

#if defined(MODEM_DYNAMIC_PARAM)

#if defined(MTK_AUDIO_SPH_LPBK_PARAM)
    if (mIsLpbk) {
        SpeechParamParser::getInstance()->SetParamInfo(String8("ParamSphLpbk=1;"));
    }
#endif

    int param_arg[4];
    param_arg[0] = (int) mSpeechMode;
    param_arg[1] = mVolumeIndex;
    param_arg[2] = mBtHeadsetNrecOn;
    param_arg[3] = 0;//bit 0: customized profile, bit 1: single band, bit 4~7: band number
    SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH, param_arg);

    return NO_ERROR;
#else

    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SET_SPH_MODE, speech_mode, 0));
#endif
}

status_t SpeechDriverLAD::setMDVolumeIndex(int stream, int device, int index) {
    bool bSpeechStatus = GetApSideModemStatus(SPEECH_STATUS_MASK);
    ALOGD("+%s() stream= %d, device = 0x%x, index =%d, bSpeechStatus=%d", __FUNCTION__, stream, device, index, bSpeechStatus);
    //Android M Voice volume index: available index 1~7, 0 for mute
    //Android L Voice volume index: available index 0~6
    if (index <= 0) {
        return NO_ERROR;
    } else {
        mVolumeIndex = index - 1;
    }
    // set a unreasonable gain value s.t. the reasonable gain can be set to modem next time
#if defined(MODEM_DYNAMIC_PARAM)
    if (bSpeechStatus) {
        int param_arg[4];
        param_arg[0] = (int) mSpeechMode;
        param_arg[1] = mVolumeIndex;
        param_arg[2] = mBtHeadsetNrecOn;
        param_arg[3] = 0;//bit 0: customized profile, bit 1: single band, bit 4~7: band number
        SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH, param_arg);
    } else {
        ALOGD("%s() bSpeechStatus=%d, only update index(%d)", __FUNCTION__, bSpeechStatus, index);

    }
#endif
    return NO_ERROR;

}

status_t SpeechDriverLAD::SpeechOn() {
    ALOGD("%s()", __FUNCTION__);
    CheckApSideModemStatusAllOffOrDie();
    SetApSideModemStatus(SPEECH_STATUS_MASK);

    // Check if Modem reset  re-send parameter here
    if (pCCCI->WasModemReset() == true) {
        ALOGD("%s modem was reset, send speech param now", __FUNCTION__);
        SetAllSpeechEnhancementInfoToModem();
        pCCCI->ResetWasModemReset(false);
    } else {
#if defined(MODEM_DYNAMIC_PARAM)
        int param_arg[2];
        SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_GENERAL, param_arg);
#if defined(MTK_SPH_MAGICLARITY_SHAPEFIR_SUPPORT)
        SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_MAGICLARITY, param_arg);
#endif

        if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
            SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_DMNR, param_arg);
        }

#if defined(MTK_USB_PHONECALL)
        SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_ECHOREF, param_arg);
#endif
#endif

    }
    status_t retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_ON, RAT_2G_MODE, 0));

    if (retval == NO_ERROR) { // In queue or had sent to modem side => wait ack
        WaitUntilSignaledOrTimeout(kSpeechOnWaitModemAckMaxTimeMs);
    }
    // Clean gain value and mute status
    CleanGainValueAndMuteStatus();

    return retval;
}

status_t SpeechDriverLAD::SpeechOff() {
    ALOGD("%s()", __FUNCTION__);

    ResetApSideModemStatus(SPEECH_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();

    status_t retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_OFF, 0, 0));

    if (retval == NO_ERROR) { // In queue or had sent to modem side => wait ack
        WaitUntilSignaledOrTimeout(kSpeechOffWaitModemAckMaxTimeMs);
    }
    // mVolumeIndex = 0x3;

    return retval;
}

status_t SpeechDriverLAD::VideoTelephonyOn() {
    ALOGD("%s()", __FUNCTION__);

    CheckApSideModemStatusAllOffOrDie();
    SetApSideModemStatus(VT_STATUS_MASK);
    // Check if Modem reset  re-send parameter here
    if (pCCCI->WasModemReset() == true) {
        ALOGD("%s modem was reset, send speech param now", __FUNCTION__);
        SetAllSpeechEnhancementInfoToModem();
        pCCCI->ResetWasModemReset(false);
    } else {
#if defined(MODEM_DYNAMIC_PARAM)
        int size_byte;
        int param_arg[2];
        SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_GENERAL, param_arg);
#if defined(MTK_SPH_MAGICLARITY_SHAPEFIR_SUPPORT)
        SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_MAGICLARITY, param_arg);
#endif

        if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
            SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_DMNR, param_arg);
        }

#if defined(MTK_USB_PHONECALL)
        SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_ECHOREF, param_arg);
#endif
#endif

    }
    status_t retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_ON, RAT_3G324M_MODE, 0));

    if (retval == NO_ERROR) { // In queue or had sent to modem side => wait ack
        WaitUntilSignaledOrTimeout(kSpeechOnWaitModemAckMaxTimeMs);
    }
    // Clean gain value and mute status
    CleanGainValueAndMuteStatus();

    return retval;
}

status_t SpeechDriverLAD::VideoTelephonyOff() {
    ALOGD("%s()", __FUNCTION__);

    ResetApSideModemStatus(VT_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();

    status_t retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_OFF, 0, 0));

    if (retval == NO_ERROR) { // In queue or had sent to modem side => wait ack
        WaitUntilSignaledOrTimeout(kSpeechOffWaitModemAckMaxTimeMs);
    }

    return retval;
}

status_t SpeechDriverLAD::SpeechRouterOn() {
    ALOGD("%s()", __FUNCTION__);

    CheckApSideModemStatusAllOffOrDie();
    SetApSideModemStatus(SPEECH_ROUTER_STATUS_MASK);

    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_ROUTER_ON, true, 0));
}

status_t SpeechDriverLAD::SpeechRouterOff() {
    ALOGD("%s()", __FUNCTION__);

    ResetApSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();

    // Clean gain value and mute status
    CleanGainValueAndMuteStatus();

    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_ROUTER_ON, false, 0));
}


/*==============================================================================
 *                     Recording Control
 *============================================================================*/

status_t SpeechDriverLAD::recordOn() {
    ALOGD("%s(), sample_rate = %d, channel = %d, MSG_A2M_PCM_REC_ON", __FUNCTION__, mRecordSampleRateType, mRecordChannelType);

    SetApSideModemStatus(RECORD_STATUS_MASK);

    // Note: the record capability is fixed in constructor
    uint16_t param_16bit = mRecordSampleRateType  | (mRecordChannelType << 4);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PCM_REC_ON, param_16bit, 0));
}

status_t SpeechDriverLAD::recordOn(SpcRecordTypeStruct typeRecord) {
    ALOGD("%s(), sample_rate = %d, channel = %d, typeRecord = %d, MSG_A2M_RECORD_RAW_PCM_ON", __FUNCTION__, mRecordSampleRateType, mRecordChannelType, typeRecord.direction);
    uint16_t param_16bit;

    SetApSideModemStatus(RAW_RECORD_STATUS_MASK);
    mRecordType = typeRecord;
    pCCCI->setPcmRecordType(typeRecord);
    param_16bit = mRecordSampleRateType  | (mRecordChannelType << 4);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_RECORD_RAW_PCM_ON, param_16bit, 0));
}

status_t SpeechDriverLAD::recordOff() {
    ALOGD("%s()", __FUNCTION__);

    ResetApSideModemStatus(RECORD_STATUS_MASK);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PCM_REC_OFF, 0, 0));
}

status_t SpeechDriverLAD::recordOff(SpcRecordTypeStruct typeRecord) {
    ALOGD("%s(), typeRecord = %d, MSG_A2M_RECORD_RAW_PCM_OFF", __FUNCTION__, typeRecord.direction);

    ResetApSideModemStatus(RAW_RECORD_STATUS_MASK);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_RECORD_RAW_PCM_OFF, 0, 0));
}

status_t SpeechDriverLAD::setPcmRecordType(SpcRecordTypeStruct typeRecord) {
    ALOGD("%s(), typeRecord = %d", __FUNCTION__, typeRecord.direction);
    pCCCI->setPcmRecordType(typeRecord);
    return NO_ERROR;
}

status_t SpeechDriverLAD::VoiceMemoRecordOn() {
    ALOGD("%s(), MSG_A2M_VM_REC_ON", __FUNCTION__);

    SetApSideModemStatus(VM_RECORD_STATUS_MASK);

    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_VM_REC_ON, RECORD_FORMAT_VM, 0));
}

status_t SpeechDriverLAD::VoiceMemoRecordOff() {
    ALOGD("%s()", __FUNCTION__);
    uint8_t i = 0;
    while ((GetApSideModemStatus(VM_RECORD_STATUS_MASK) != true) && (i < WAIT_VM_RECORD_ON_CNT)) {
        i++;
        usleep(5 * 1000);
        ALOGD("%s() wait %d ms...", __FUNCTION__, (i + 1) * 5);
    }

    ResetApSideModemStatus(VM_RECORD_STATUS_MASK);

    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_VM_REC_OFF, 0, 0));
}

uint16_t SpeechDriverLAD::GetRecordSampleRate() const {
    // Note: the record capability is fixed in constructor

    uint16_t num_sample_rate;

    switch (mRecordSampleRateType) {
    case RECORD_SAMPLE_RATE_08K:
        num_sample_rate = 8000;
        break;
    case RECORD_SAMPLE_RATE_16K:
        num_sample_rate = 16000;
        break;
    case RECORD_SAMPLE_RATE_32K:
        num_sample_rate = 32000;
        break;
    case RECORD_SAMPLE_RATE_48K:
        num_sample_rate = 48000;
        break;
    default:
        num_sample_rate = 8000;
        break;
    }

    ALOGD("%s(), num_sample_rate = %u", __FUNCTION__, num_sample_rate);
    return num_sample_rate;
}

uint16_t SpeechDriverLAD::GetRecordChannelNumber() const {
    // Note: the record capability is fixed in constructor

    uint16_t num_channel;

    switch (mRecordChannelType) {
    case RECORD_CHANNEL_MONO:
        num_channel = 1;
        break;
    case RECORD_CHANNEL_STEREO:
        num_channel = 2;
        break;
    default:
        num_channel = 1;
        break;
    }

    ALOGD("%s(), num_channel = %u", __FUNCTION__, num_channel);
    return num_channel;
}


/*==============================================================================
 *                     Background Sound
 *============================================================================*/

status_t SpeechDriverLAD::BGSoundOn() {
    ALOGD("%s()", __FUNCTION__);
    SetApSideModemStatus(BGS_STATUS_MASK);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_BGSND_ON, 0, 0));
}

status_t SpeechDriverLAD::BGSoundConfig(uint8_t ul_gain, uint8_t dl_gain) {
    ALOGD("%s(), ul_gain = 0x%x, dl_gain = 0x%x", __FUNCTION__, ul_gain, dl_gain);
    uint16_t param_16bit = (ul_gain << 8) | dl_gain;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_BGSND_CONFIG, param_16bit, 0));
}

status_t SpeechDriverLAD::BGSoundOff() {
    ALOGD("%s()", __FUNCTION__);
    ResetApSideModemStatus(BGS_STATUS_MASK);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_BGSND_OFF, 0, 0));
}

/*==============================================================================
 *                     PCM 2 Way
 *============================================================================*/

status_t SpeechDriverLAD::PCM2WayOn(const bool wideband_on) {
    mPCM2WayState = (SPC_PNW_MSG_BUFFER_SPK | SPC_PNW_MSG_BUFFER_MIC | (wideband_on << 4));
    ALOGD("%s(), mPCM2WayState = 0x%x", __FUNCTION__, mPCM2WayState);
    SetApSideModemStatus(P2W_STATUS_MASK);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_ON, mPCM2WayState, 0));
}

status_t SpeechDriverLAD::PCM2WayOff() {
    mPCM2WayState = 0;
    ALOGD("%s(), mPCM2WayState = 0x%x", __FUNCTION__, mPCM2WayState);
    ResetApSideModemStatus(P2W_STATUS_MASK);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_PNW_OFF, 0, 0));
}


/*==============================================================================
 *                     TTY-CTM Control
 *============================================================================*/
status_t SpeechDriverLAD::TtyCtmOn() {
    ALOGD("%s(), mTtyMode = %d", __FUNCTION__, mTtyMode);
    status_t retval;
    const bool uplink_mute_on_copy = mUplinkMuteOn;
    SetUplinkMute(true);
    SetApSideModemStatus(TTY_STATUS_MASK);
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();
    TtyCtmDebugOn(pSpeechVMRecorder->getVmConfig() == SPEECH_VM_CTM4WAY);
    retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_CTM_ON, mTtyMode, 0));
    SetUplinkMute(uplink_mute_on_copy);
    return retval;
}

status_t SpeechDriverLAD::TtyCtmOff() {
    ALOGD("%s()", __FUNCTION__);
    char paramTtyInfo[50] = {0};
    mTtyMode = AUD_TTY_OFF;
    ResetApSideModemStatus(TTY_STATUS_MASK);
    TtyCtmDebugOn(false);
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    snprintf(paramTtyInfo, sizeof(paramTtyInfo), "ParamSphTty=%d;", mTtyMode);
    SpeechParamParser::getInstance()->SetParamInfo(String8(paramTtyInfo));
#endif
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_CTM_OFF, 0, 0));
}

status_t SpeechDriverLAD::TtyCtmDebugOn(bool tty_debug_flag) {
    ALOGD("%s(), tty_debug_flag = %d", __FUNCTION__, tty_debug_flag);
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();
    if (tty_debug_flag) {
        pSpeechVMRecorder->startCtmDebug();
    } else {
        pSpeechVMRecorder->stopCtmDebug();
    }
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_CTM_DUMP_DEBUG_FILE, tty_debug_flag, 0));
}

int SpeechDriverLAD::setTtyMode(const TtyModeType ttyMode) {
    ALOGD("%s(), ttyMode = %d, old mTtyMode = %d", __FUNCTION__, ttyMode, mTtyMode);

    if (ttyMode == mTtyMode) { return 0; }
    mTtyMode = ttyMode;
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    char paramTtyInfo[50] = {0};
    snprintf(paramTtyInfo, sizeof(paramTtyInfo), "ParamSphTty=%d;", mTtyMode);
    SpeechParamParser::getInstance()->SetParamInfo(String8(paramTtyInfo));
#endif
    return 0;
}

/*==============================================================================
 *                     RTT
 *============================================================================*/
int SpeechDriverLAD::RttConfig(int rttMode) {
    ALOGD("%s(), rttMode = %d, old mRttMode = %d", __FUNCTION__, rttMode, mRttMode);
    if (rttMode == mRttMode) { return NO_ERROR; }
    mRttMode = rttMode;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_RTT_CONFIG, (uint16_t)mRttMode, 0));
}

/*==============================================================================
 *                     Modem Audio DVT and Debug
 *============================================================================*/

status_t SpeechDriverLAD::SetModemLoopbackPoint(uint16_t loopback_point) {
    ALOGD("%s(), loopback_point = %d", __FUNCTION__, loopback_point);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SET_LPBK_POINT_DVT, loopback_point, 0));
}

/*==============================================================================
 *                     Speech Encryption
 *============================================================================*/

//Speech Encryption Type: 0: disable, 1: Hard bit Encryption, 2: Pcm Data Encryption
status_t SpeechDriverLAD::SetEncryption(bool encryption_on) {
#if defined (SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    uint16_t encryptionType = 0;

#if defined(MTK_SPEECH_ENCRYPTION_PCM)
    encryptionType = (encryption_on == true) ? 2 : 0;
#else
    encryptionType = (encryption_on == true) ? 1 : 0;
#endif

    ALOGD("%s(), encryption_on = %d", __FUNCTION__, encryption_on);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_ENCRYPTION, encryptionType, 0));
}

/*==============================================================================
 *                     Acoustic Loopback
 *============================================================================*/

status_t SpeechDriverLAD::SetAcousticLoopback(bool loopback_on) {
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    ALOGD("%s(), loopback_on = %d, mAcousticLoopbackDelayFrames=%d", __FUNCTION__, loopback_on, mAcousticLoopbackDelayFrames);

    if (loopback_on == true) {
        CheckApSideModemStatusAllOffOrDie();
        SetApSideModemStatus(LOOPBACK_STATUS_MASK);
        // Check if Modem reset  re-send parameter here
        if (pCCCI->WasModemReset() == true) {
            ALOGD("%s modem was reset, send speech param now", __FUNCTION__);
            SetAllSpeechEnhancementInfoToModem();
            pCCCI->ResetWasModemReset(false);
        } else {
#if defined(MODEM_DYNAMIC_PARAM)
            int param_arg[2];
            SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_GENERAL, param_arg);
#if defined(MTK_SPH_MAGICLARITY_SHAPEFIR_SUPPORT)
            SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_MAGICLARITY, param_arg);
#endif

            if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
                SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_DMNR, param_arg);
            }

#if defined(MTK_USB_PHONECALL)
            SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_ECHOREF, param_arg);
#endif
#endif
        }
        // Clean gain value and mute status
        CleanGainValueAndMuteStatus();

    } else {
        ResetApSideModemStatus(LOOPBACK_STATUS_MASK);
        CheckApSideModemStatusAllOffOrDie();

        mUseBtCodec = 1;
#if defined(MODEM_DYNAMIC_PARAM)&&defined(MTK_AUDIO_SPH_LPBK_PARAM)
        SpeechParamParser::getInstance()->SetParamInfo(String8("ParamSphLpbk=0;"));
#endif
    }

    const bool use_loopback_delay_control = true;
    bool disable_btcodec = !mUseBtCodec;
    int16_t param16 = (use_loopback_delay_control << 2) | (disable_btcodec << 1) | loopback_on;
    status_t retval = pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SET_ACOUSTIC_LOOPBACK, param16, mAcousticLoopbackDelayFrames));

    if (retval == NO_ERROR) { // In queue or had sent to modem side => wait ack
        WaitUntilSignaledOrTimeout(loopback_on == true ?
                                   kSpeechOnWaitModemAckMaxTimeMs :
                                   kSpeechOffWaitModemAckMaxTimeMs);
    }

    return retval;
}

status_t SpeechDriverLAD::SetAcousticLoopbackBtCodec(bool enable_codec) {
    mUseBtCodec = enable_codec;
    return NO_ERROR;
}

status_t SpeechDriverLAD::SetAcousticLoopbackDelayFrames(int32_t delay_frames) {
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    ALOGD("%s(), delay_frames = %d", __FUNCTION__, delay_frames);

    if (delay_frames < 0) {
        ALOGE("%s(), delay_frames(%d) must >= 0 !! Set 0 instead.", __FUNCTION__, delay_frames);
        delay_frames = 0;
    } else if ((uint32_t)delay_frames > kMaxAcousticLoopbackDelayFrames) {
        ALOGE("%s(), delay_frames(%d) must <= %d !! Set %d instead.", __FUNCTION__, delay_frames, kMaxAcousticLoopbackDelayFrames, kMaxAcousticLoopbackDelayFrames);
        delay_frames = kMaxAcousticLoopbackDelayFrames;
    }

    mAcousticLoopbackDelayFrames = delay_frames;

    if (GetApSideModemStatus(LOOPBACK_STATUS_MASK) == true) {
        ALOGW("Loopback is enabled now! The new delay_frames will be applied next time.");
    }

    return NO_ERROR;
}

status_t SpeechDriverLAD::setLpbkFlag(bool enableLpbk) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s(), enableLpbk = %d, old mIsLpbk = %d", __FUNCTION__, enableLpbk, mIsLpbk);
    if (enableLpbk == mIsLpbk) { return NO_ERROR; }
    mIsLpbk = enableLpbk;
    return NO_ERROR;
}

/*==============================================================================
 *                     Volume Control
 *============================================================================*/

//param gain: data range is 0~0xFF00, which is mapping to 0dB to -64dB. The effective interval is 0.25dB by data increasing/decreasing 1.
status_t SpeechDriverLAD::SetDownlinkGain(int16_t gain) {
    ALOGD("%s(), gain = 0x%x, old mDownlinkGain = 0x%x", __FUNCTION__, gain, mDownlinkGain);
    if (gain == mDownlinkGain) { return NO_ERROR; }

    mDownlinkGain = gain;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_DL_DIGIT_VOLUME, gain, 0));
}

status_t SpeechDriverLAD::SetEnh1DownlinkGain(int16_t gain) {
    ALOGD("%s(), gain = 0x%x, old SetEnh1DownlinkGain = 0x%x", __FUNCTION__, gain, mDownlinkenh1Gain);
    if (gain == mDownlinkenh1Gain) { return NO_ERROR; }

    mDownlinkenh1Gain = gain;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_DL_ENH_REF_DIGIT_VOLUME, gain, 0));
}

//param gain: data range is 0~120, which is mapping to 0dB to 30dB. The effective interval is 1dB by data increasing/decreasing 4.
status_t SpeechDriverLAD::SetUplinkGain(int16_t gain) {
    ALOGD("%s(), gain = 0x%x, old mUplinkGain = 0x%x", __FUNCTION__, gain, mUplinkGain);
    if (gain == mUplinkGain) { return NO_ERROR; }

    mUplinkGain = gain;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_UL_DIGIT_VOLUME, gain, 0));
}

status_t SpeechDriverLAD::SetDownlinkMute(bool mute_on) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s(), mute_on = %d, old mDownlinkMuteOn = %d", __FUNCTION__, mute_on, mDownlinkMuteOn);
    if (mute_on == mDownlinkMuteOn) { return NO_ERROR; }

    mDownlinkMuteOn = mute_on;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_MUTE_SPH_DL, mute_on, 0));
}

status_t SpeechDriverLAD::SetDownlinkMuteCodec(bool mute_on) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s(), mute_on = %d, old mDownlinkMuteOn = %d", __FUNCTION__, mute_on, mDownlinkMuteOn);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_MUTE_SPH_DL_CODEC, mute_on, 0));
}

status_t SpeechDriverLAD::SetUplinkMute(bool mute_on) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s(), mute_on = %d, old mUplinkMuteOn = %d", __FUNCTION__, mute_on, mUplinkMuteOn);

    mUplinkMuteOn = mute_on;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_MUTE_SPH_UL, mute_on, 0));
}

status_t SpeechDriverLAD::SetUplinkSourceMute(bool mute_on) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s(), mute_on = %d, old mUplinkSourceMuteOn = %d", __FUNCTION__, mute_on, mUplinkSourceMuteOn);
    if (mute_on == mUplinkSourceMuteOn) { return NO_ERROR; }

    mUplinkSourceMuteOn = mute_on;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_MUTE_SPH_UL_SOURCE, mute_on, 0));
}

status_t SpeechDriverLAD::SetSidetoneGain(int16_t gain) {
    ALOGD("%s(), gain = 0x%x, old mSideToneGain = 0x%x", __FUNCTION__, gain, mSideToneGain);
    if (gain == mSideToneGain) { return NO_ERROR; }

    mSideToneGain = gain;
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SIDETONE_VOLUME, gain, 0));
}


status_t SpeechDriverLAD::SetDSPSidetoneFilter(const bool dsp_stf_on) {
    ALOGD("%s(), dsp_stf_on = %d", __FUNCTION__, dsp_stf_on);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SIDETONE_CONFIG, dsp_stf_on, 0));
}


/*==============================================================================
 *                     Device related Config
 *============================================================================*/

status_t SpeechDriverLAD::SetModemSideSamplingRate(uint16_t sample_rate) {
    ALOGD("%s(), sample_rate = %d", __FUNCTION__, sample_rate);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SET_SAMPLE_RATE, sample_rate, 0));
}

/*==============================================================================
 *                     Speech Enhancement Control
 *============================================================================*/
status_t SpeechDriverLAD::SetSpeechEnhancement(bool enhance_on) {
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    ALOGD("%s(), enhance_on = %d, mForceDisableSpeechEnhancement = %d", __FUNCTION__, enhance_on, mForceDisableSpeechEnhancement);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_CTRL_SPH_ENH, enhance_on & !mForceDisableSpeechEnhancement, 0));
}

status_t SpeechDriverLAD::SetSpeechEnhancementMask(const sph_enh_mask_struct_t &mask) {
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    ALOGD("%s(), main_func = 0x%x, dynamic_func = 0x%x", __FUNCTION__, mask.main_func, mask.dynamic_func);
    return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_CONFIG_SPH_ENH, mask.main_func, mask.dynamic_func));
}

status_t SpeechDriverLAD::SetBtHeadsetNrecOn(const bool bt_headset_nrec_on) {
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    ALOGD("%s(), bt_headset_nrec_on = %d, mSpeechMode = %d", __FUNCTION__, bt_headset_nrec_on, mSpeechMode);
    mBtHeadsetNrecOn = bt_headset_nrec_on;

    return NO_ERROR;
}


/*==============================================================================
 *                     Speech Enhancement Parameters
 *============================================================================*/
status_t SpeechDriverLAD::SetDynamicSpeechParameters(const int type, const void *param_arg) {
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif

    int u4I, size_byte;
    char *pPackedSphParamUnit = new char [CCCI_MAX_PAYLOAD_DATA_BYTE];
    ALOGD("+%s(), type[%d]", __FUNCTION__, type);
    int *u4ParamArg = (int *)param_arg;
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    switch (type) {
    case AUDIO_TYPE_SPEECH:
        memset(pPackedSphParamUnit, 0, CCCI_MAX_PAYLOAD_DATA_BYTE);
#if defined(SINGLE_VOICE_BAND_SPEECH_TRANSFER)

        u4ParamArg[3] = u4ParamArg[3] | 0x2; //bit 1: single band, bit 4~7: band number  NB
        size_byte = SpeechParamParser::getInstance()->GetSpeechParamUnit(pPackedSphParamUnit, u4ParamArg);
        SetVariousKindsOfSpeechParameters(pPackedSphParamUnit, size_byte, MSG_A2M_EM_DYNAMIC_SPH);

        u4ParamArg[3] = u4ParamArg[3] | 0x10; //bit 1: single band, bit 4~7: band number  WB
        memset(pPackedSphParamUnit, 0, CCCI_MAX_PAYLOAD_DATA_BYTE);
        size_byte = SpeechParamParser::getInstance()->GetSpeechParamUnit(pPackedSphParamUnit, u4ParamArg);
        SetVariousKindsOfSpeechParameters(pPackedSphParamUnit, size_byte, MSG_A2M_EM_DYNAMIC_SPH);
#else
        size_byte = SpeechParamParser::getInstance()->GetSpeechParamUnit(pPackedSphParamUnit, u4ParamArg);
        SetVariousKindsOfSpeechParameters(pPackedSphParamUnit, size_byte, MSG_A2M_EM_DYNAMIC_SPH);

#endif
        break;
    case AUDIO_TYPE_SPEECH_DMNR:
        memset(pPackedSphParamUnit, 0, CCCI_MAX_PAYLOAD_DATA_BYTE);
        //return array with size
        size_byte = SpeechParamParser::getInstance()->GetDmnrParamUnit(pPackedSphParamUnit);

#if 0
        for (u4I = 0; u4I<size_byte >> 1; u4I++) {
            ALOGV("%s(), DMNR pPackedSphParamUnit[%d] = 0x%x", __FUNCTION__, u4I, *((uint16_t *)pPackedSphParamUnit + u4I));
        }
#endif
        SetVariousKindsOfSpeechParameters(pPackedSphParamUnit, size_byte, MSG_A2M_EM_DYNAMIC_SPH);
        break;
    case AUDIO_TYPE_SPEECH_GENERAL:
        memset(pPackedSphParamUnit, 0, CCCI_MAX_PAYLOAD_DATA_BYTE);
        //return array with size
        size_byte = SpeechParamParser::getInstance()->GetGeneralParamUnit(pPackedSphParamUnit);

#if 0
        for (u4I = 0; u4I<size_byte >> 1; u4I++) {
            ALOGV("%s(), General pPackedSphParamUnit[%d] = 0x%x", __FUNCTION__, u4I, *((uint16_t *)pPackedSphParamUnit + u4I));
        }
#endif
        SetVariousKindsOfSpeechParameters(pPackedSphParamUnit, size_byte, MSG_A2M_EM_DYNAMIC_SPH);
        break;
    case AUDIO_TYPE_SPEECH_MAGICLARITY:
        memset(pPackedSphParamUnit, 0, CCCI_MAX_PAYLOAD_DATA_BYTE);
        //return array with size
        size_byte = SpeechParamParser::getInstance()->GetMagiClarityParamUnit(pPackedSphParamUnit);
#if 0
        for (u4I = 0; u4I<size_byte >> 1; u4I++) {
            ALOGV("%s(), MagiClairity pPackedSphParamUnit[%d] = 0x%x", __FUNCTION__, u4I, *((uint16_t *)pPackedSphParamUnit + u4I));
        }
#endif
        SetVariousKindsOfSpeechParameters(pPackedSphParamUnit, size_byte, MSG_A2M_EM_DYNAMIC_SPH);
        break;
    case AUDIO_TYPE_SPEECH_ECHOREF:
        memset(pPackedSphParamUnit, 0, 2048);
        //return array with size
        size_byte = SpeechParamParser::getInstance()->GetEchoRefParamUnit(pPackedSphParamUnit);
        SetVariousKindsOfSpeechParameters(pPackedSphParamUnit, size_byte, MSG_A2M_EM_DYNAMIC_SPH);
        break;
    default:
        break;

    }
#endif
    if (pPackedSphParamUnit != NULL) {
        delete[] pPackedSphParamUnit;
    }
    return NO_ERROR;

}

#if defined(MODEM_DYNAMIC_PARAM) && (USE_CCCI_SHARE_BUFFER)
status_t SpeechDriverLAD::SetVariousKindsOfSpeechParameters(const void *param, const uint16_t data_length, const uint16_t ccci_message_id) {
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    if (ccci_message_id != MSG_A2M_EM_DYNAMIC_SPH) {
        ALOGE("%s(), only support Speech Dynamic Parameters", __FUNCTION__);
        return NO_ERROR;
    }
    if (pCCCI->GetMDResetFlag() == false) { // check MD Reset Flag
        {
            // get share buffer address
            uint16_t offset;
            uint16_t avail;
            char *addr;
            uint32_t trycnt = 0;
            const uint32_t kMaxTryCnt = 30; // total 3 sec
            do {
                pCCCI->GetA2MRawParaRingBuffer(&offset, &avail);
                if (avail >= (data_length + CCCI_SHARE_BUFF_HEADER_LEN)) {
                    break;
                } else {
                    usleep(100 * 1000);
                    trycnt ++;
                    if (trycnt >= kMaxTryCnt) {break;}
                }
            } while (1);

            if (avail < (data_length + CCCI_SHARE_BUFF_HEADER_LEN)) {
                ALOGE("%s() fail due to unable get buffer space ccci_message_id = 0x%x, avail %d, need %d",
                      __FUNCTION__, ccci_message_id, avail, data_length + (uint16_t)CCCI_SHARE_BUFF_HEADER_LEN);
                return UNKNOWN_ERROR;
            }

            share_buff_data_type_t type = SHARE_BUFF_DATA_TYPE_CCCI_DYNAMIC_PARAM_TYPE;

            ALOGD("%s() type = %d, ccci_message_id = 0x%x", __FUNCTION__, type, ccci_message_id);

            // fill header info
            uint16_t temp[3];
            pCCCI->SetShareBufHeader((uint16_t *) temp,
                                     CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                     type,
                                     data_length);

            // write header to Ring buffer
            pCCCI->WriteA2MRawParaRingBuffer((char *) temp, CCCI_SHARE_BUFF_HEADER_LEN);

            // fill speech enhancement parameter
            pCCCI->WriteA2MRawParaRingBuffer((char *) param, data_length);

            // send data notify to modem side
            const uint16_t payload_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
            return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(ccci_message_id, payload_length, (uint32_t)offset));
        }
    } else {

        ALOGD("%s(), SKIP Speech Parameters setting because MD Reset", __FUNCTION__);
        return NO_ERROR;

    }
}
#else
status_t SpeechDriverLAD::SetVariousKindsOfSpeechParameters(const void *param, const uint16_t data_length, const uint16_t ccci_message_id) {
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    if (pCCCI->GetMDResetFlag() == false) { // check MD Reset Flag
        if (pCCCI->A2MBufLock() == false) { // get buffer lock to prevent overwrite other's data
            ALOGE("%s() fail due to unalbe get A2MBufLock, ccci_message_id = 0x%x", __FUNCTION__, ccci_message_id);
            return TIMED_OUT;
        } else {
            // get share buffer address
            uint16_t offset = A2M_SHARED_BUFFER_SPH_PARAM_BASE;
            char *p_header_address = pCCCI->GetA2MShareBufBase() + offset;
            char *p_data_address = p_header_address + CCCI_SHARE_BUFF_HEADER_LEN;
            share_buff_data_type_t type = SHARE_BUFF_DATA_TYPE_CCCI_EM_PARAM;

            switch (ccci_message_id) {
#if defined(MODEM_DYNAMIC_PARAM)
            case MSG_A2M_EM_DYNAMIC_SPH:
                type = SHARE_BUFF_DATA_TYPE_CCCI_DYNAMIC_PARAM_TYPE;
                break;

            default:
                type = SHARE_BUFF_DATA_TYPE_CCCI_DYNAMIC_PARAM_TYPE;
                break;
#else
            case MSG_A2M_EM_MAGICON:
                type = SHARE_BUFF_DATA_TYPE_CCCI_MAGICON_PARAM;
                break;
            case MSG_A2M_EM_HAC:
                type = SHARE_BUFF_DATA_TYPE_CCCI_HAC_PARAM;
                break;

            default:
                type = SHARE_BUFF_DATA_TYPE_CCCI_EM_PARAM;
                break;
#endif
            }

            ALOGD("%s() type = %d, ccci_message_id = 0x%x", __FUNCTION__, type, ccci_message_id);

            // fill header info
            pCCCI->SetShareBufHeader((uint16_t *)p_header_address,
                                     CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                     type,
                                     data_length);

            // fill speech enhancement parameter
            memcpy((void *)p_data_address, (void *)param, data_length);

            // send data notify to modem side
            const uint16_t payload_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
#if defined(MODEM_DYNAMIC_PARAM)
            if (ccci_message_id == MSG_A2M_EM_DYNAMIC_SPH) {
                return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(ccci_message_id, payload_length, offset));
            } else {
                ALOGE("%s(), only support Speech Dynamic Parameters", __FUNCTION__);
                pCCCI->A2MBufUnLock();
                return NO_ERROR;
            }
#else
            return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(ccci_message_id, payload_length, offset));

#endif


        }
    } else {

        ALOGD("%s(), SKIP Speech Parameters setting because MD Reset", __FUNCTION__);
        return NO_ERROR;

    }
}
#endif

status_t SpeechDriverLAD::SetNBSpeechParameters(const void *pSphParamNB) {
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    ALOGD("%s()", __FUNCTION__);
    return SetVariousKindsOfSpeechParameters(pSphParamNB, sizeof(AUDIO_CUSTOM_PARAM_STRUCT), MSG_A2M_EM_NB);
}

status_t SpeechDriverLAD::SetDualMicSpeechParameters(const void *pSphParamDualMic) {
    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() < 2) {
        ALOGE("%s()", __FUNCTION__);
        return INVALID_OPERATION;
    }

#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    ALOGD("%s()", __FUNCTION__);

#if defined(MTK_WB_SPEECH_SUPPORT)
    // NVRAM always contain(44+76), for WB we send full (44+76)
    uint16_t data_length = sizeof(unsigned short) * (NUM_ABF_PARAM + NUM_ABFWB_PARAM); // NB + WB

    // Check if support Loud Speaker Mode DMNR
    if (sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT) >= (data_length * 2)) {
        data_length *= 2; // 1 for receiver mode DMNR, 1 for loud speaker mode DMNR
    }
#else
    // for NB we send (44) only
    uint16_t data_length = sizeof(unsigned short) * (NUM_ABF_PARAM); // NB Only
#endif

    return SetVariousKindsOfSpeechParameters(pSphParamDualMic, data_length, MSG_A2M_EM_DMNR);
}

status_t SpeechDriverLAD::SetMagiConSpeechParameters(const void *pSphParamMagiCon) {
    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() < 2) {
        ALOGE("%s()", __FUNCTION__);
        return INVALID_OPERATION;
    }

#if defined(MTK_MAGICONFERENCE_SUPPORT)
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    ALOGD("%s()", __FUNCTION__);

    uint16_t data_length = sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT); // NB + WB

    return SetVariousKindsOfSpeechParameters(pSphParamMagiCon, data_length, MSG_A2M_EM_MAGICON);
#else
    (void)pSphParamMagiCon;
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
#endif
}

status_t SpeechDriverLAD::SetHACSpeechParameters(const void *pSphParamHAC) {
#if defined(MTK_HAC_SUPPORT)
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    AUDIO_CUSTOM_HAC_SPEECH_PARAM_STRUCT mSphParamHAC;
    memcpy(&mSphParamHAC, pSphParamHAC, sizeof(AUDIO_CUSTOM_HAC_SPEECH_PARAM_STRUCT));

    uint16_t data_length = sizeof(AUDIO_CUSTOM_HAC_SPEECH_PARAM_STRUCT); // NB + WB
    ALOGD("%s(), data_length=%d", __FUNCTION__, data_length);
    return SetVariousKindsOfSpeechParameters(&mSphParamHAC, data_length, MSG_A2M_EM_HAC);

#else
    ALOGE("%s()", __FUNCTION__);
    (void)pSphParamHAC;
    return INVALID_OPERATION;
#endif
}

#if defined(MTK_WB_SPEECH_SUPPORT)
status_t SpeechDriverLAD::SetWBSpeechParameters(const void *pSphParamWB) {
#if defined(SPH_SKIP_A2M_BUFF_MSG)
    return NO_ERROR;
#endif
    ALOGD("%s()", __FUNCTION__);
    return SetVariousKindsOfSpeechParameters(pSphParamWB, sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT), MSG_A2M_EM_WB);
}
#else
status_t SpeechDriverLAD::SetWBSpeechParameters(const void *pSphParamWB) {
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}
#endif


status_t SpeechDriverLAD::GetVibSpkParam(void *eVibSpkParam) {
    int32_t frequency;
    AUDIO_ACF_CUSTOM_PARAM_STRUCT audioParam;
    memset(&audioParam, 0, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    getAudioCompFltCustParam(AUDIO_COMP_FLT_VIBSPK, &audioParam);
    PARAM_VIBSPK *pParamVibSpk = (PARAM_VIBSPK *)eVibSpkParam;
    int dTableIndex;

    if (audioParam.bes_loudness_WS_Gain_Max != VIBSPK_CALIBRATION_DONE && audioParam.bes_loudness_WS_Gain_Max != VIBSPK_SETDEFAULT_VALUE) {
        frequency = VIBSPK_DEFAULT_FREQ;
    } else {
        frequency = audioParam.bes_loudness_WS_Gain_Min;
    }

    if (frequency < VIBSPK_FREQ_LOWBOUND) {
        dTableIndex = 0;
    } else {
        dTableIndex = (frequency - VIBSPK_FREQ_LOWBOUND + 1) / VIBSPK_FILTER_FREQSTEP;
    }

    if (dTableIndex < VIBSPK_FILTER_NUM && dTableIndex >= 0) {
        memcpy(pParamVibSpk->pParam, &SPH_VIBR_FILTER_COEF_Table[dTableIndex], sizeof(uint16_t)*VIBSPK_SPH_PARAM_SIZE);
    }

    if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER)) {
        pParamVibSpk->flag2in1 = false;
    } else {
        pParamVibSpk->flag2in1 = true;
    }

    return NO_ERROR;
}

status_t SpeechDriverLAD::SetVibSpkParam(void *eVibSpkParam) {
    if (pCCCI->A2MBufLock() == false) {
        ALOGE("%s() fail due to unalbe get A2MBufLock, ccci_message_id = 0x%x", __FUNCTION__, MSG_A2M_VIBSPK_PARAMETER);
        ALOGD("VibSpkSetSphParam Fail!");
        return TIMED_OUT;
    } else {
        uint16_t offset = A2M_SHARED_BUFFER_SPH_PARAM_BASE;
        char *p_header_address = pCCCI->GetA2MShareBufBase() + offset;
        char *p_data_address = p_header_address + CCCI_SHARE_BUFF_HEADER_LEN;
        uint16_t data_length = sizeof(PARAM_VIBSPK);
        ALOGD("VibSpkSetSphParam Success!");
        // fill header info
        pCCCI->SetShareBufHeader((uint16_t *)p_header_address,
                                 CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                 SHARE_BUFF_DATA_TYPE_CCCI_VIBSPK_PARAM,
                                 data_length);

        // fill speech enhancement parameter
        memcpy((void *)p_data_address, (void *)eVibSpkParam, data_length);

        // send data notify to modem side
        const uint16_t message_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
        return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_VIBSPK_PARAMETER, message_length, offset));
    }

}
//#endif //defined(MTK_VIBSPK_SUPPORT)

typedef struct {
    bool isFeatureOptOn;
    short switch_and_delay;   // bit8: switch; bit7~0: delay
    short mic_index;   // bit wise definition ordered from main mic to reference mic. Only one bit is set!! bit 0: o17, bit 1: o18, bit 2: o23, bit 3: o24, bit 4: o25
} PARAM_SMARTPA;

#define SMARTPA_SUPPORT_BIT    8
#define MIC_INDEX_O17              1<<0
#define MIC_INDEX_O18              1<<1
#define MIC_INDEX_O23              1<<2
#define MIC_INDEX_O24              1<<3
#define MIC_INDEX_O25              1<<4
#define MIC_INDEX_O26              1<<3
#define MIC_INDEX_O27              1<<2 // due to DSP has no mic3

status_t SpeechDriverLAD::GetSmartpaParam(void *eParamSmartpa __unused) {
    PARAM_SMARTPA *pParamSmartpa = (PARAM_SMARTPA *)eParamSmartpa;;

    if (AudioSmartPaController::getInstance()->isSmartPAUsed()) {
#if defined(FORCE_DISABLE_EXTCODEC_ECHO_REFERENCE)
        pParamSmartpa->isFeatureOptOn = 0;
#else
        pParamSmartpa->isFeatureOptOn = 1;
#endif

        if (mModemIndex == MODEM_1) {
            pParamSmartpa->mic_index = MIC_INDEX_O24;
        } else {
            pParamSmartpa->mic_index = MIC_INDEX_O27;
        }

        pParamSmartpa->switch_and_delay = ((pParamSmartpa->isFeatureOptOn << SMARTPA_SUPPORT_BIT) |
                                           (AudioSmartPaController::getInstance()->getSmartPaDelayUs() / 1000));

        ALOGD("%s, isNxpFeatureOptOn=%d, switch_and_delay=%d, mic_index=%d", __FUNCTION__,
              pParamSmartpa->isFeatureOptOn, pParamSmartpa->switch_and_delay, pParamSmartpa->mic_index);
    }
    return NO_ERROR;
}

status_t SpeechDriverLAD::SetSmartpaParam(void *eParamSmartpa) {
    if (pCCCI->A2MBufLock() == false) {
        ALOGE("%s() fail due to unalbe get A2MBufLock, ccci_message_id = 0x%x", __FUNCTION__, MSG_A2M_SMARTPA_PARAMETER);
        ALOGD("SmartpaSetSphParam Fail!");
        return TIMED_OUT;
    } else {
        //PARAM_SMARTPA paramSmartpa;
        uint16_t offset = A2M_SHARED_BUFFER_SPH_PARAM_BASE;
        char *p_header_address = pCCCI->GetA2MShareBufBase() + offset;
        char *p_data_address = p_header_address + CCCI_SHARE_BUFF_HEADER_LEN;
        uint16_t data_length = sizeof(PARAM_SMARTPA);
        ALOGD("SmartpaSetSphParam Success!");
        // fill header info
        pCCCI->SetShareBufHeader((uint16_t *)p_header_address,
                                 CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                 SHARE_BUFF_DATA_TYPE_CCCI_SMARTPA_PARAM,
                                 data_length);

        // fill speech enhancement parameter
        memcpy((void *)p_data_address, (void *)eParamSmartpa, data_length);

        // send data notify to modem side
        const uint16_t message_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
        return pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SMARTPA_PARAMETER, message_length, offset));
    }

}

status_t SpeechDriverLAD::SetAllSpeechEnhancementInfoToModem() {
    // Wait until modem ready
    //if (pCCCI->WaitUntilModemReady() == TIMED_OUT) //entering flight mode, waste too much time waiting...
    if (pCCCI->GetMDResetFlag() == true) {
        ALOGD("%s() MD not ready", __FUNCTION__);
        return NO_ERROR;
    }

    // Lock
    static AudioLock _mutex;
    AL_LOCK_MS(_mutex, 10000); // wait 10 sec

    uint16_t speechEnhancementCore = 1;
    pCCCI->SendMessageInQueue(pCCCI->InitCcciMailbox(MSG_A2M_SPH_ENH_CORE, speechEnhancementCore, 0));

#if defined(MODEM_DYNAMIC_PARAM)

    int param_arg[4];
    SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_GENERAL, param_arg);

#if defined(MTK_SPH_MAGICLARITY_SHAPEFIR_SUPPORT)
    SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_MAGICLARITY, param_arg);
#endif
    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_DMNR, param_arg);
    }
#if defined(MTK_USB_PHONECALL)
    SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH_ECHOREF, param_arg);
#endif

    if (AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        PARAM_SMARTPA eSmartpaParam;
        memset(&eSmartpaParam, 0, sizeof(PARAM_SMARTPA));
        GetSmartpaParam((void *)&eSmartpaParam);
        SetSmartpaParam((void *)&eSmartpaParam);
    }


#else
    AudioCustParamClient *pAudioCustParamClient = AudioCustParamClient::GetInstance();

    // NB Speech Enhancement Parameters
    AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
    memset(&eSphParamNB, 0, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
    pAudioCustParamClient->GetNBSpeechParamFromNVRam(&eSphParamNB);
    if (mIsLpbk == true) {
        AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT eSphParamNBLpbk;
        memset(&eSphParamNBLpbk, 0, sizeof(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT));
        pAudioCustParamClient->GetNBSpeechLpbkParamFromNVRam(&eSphParamNBLpbk);
        //replace receiver/headset/loudspk mode parameters
        memcpy(&eSphParamNB.speech_mode_para[0][0], &eSphParamNBLpbk, sizeof(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT));
    }
    SetNBSpeechParameters(&eSphParamNB);
    ALOGD("NVRAM debug: speech_mode_para[0][0]=%d should not be zero", eSphParamNB.speech_mode_para[0][0]);

    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        // Dual Mic Speech Enhancement Parameters
        AUDIO_CUSTOM_EXTRA_PARAM_STRUCT eSphParamDualMic;
        memset(&eSphParamDualMic, 0, sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
        pAudioCustParamClient->GetDualMicSpeechParamFromNVRam(&eSphParamDualMic);
        SetDualMicSpeechParameters(&eSphParamDualMic);

#if defined(MTK_INCALL_HANDSFREE_DMNR) ||defined(MTK_MAGICONFERENCE_SUPPORT)
        AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT eSphParamMagiCon;
        memset(&eSphParamMagiCon, 0, sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT));
        pAudioCustParamClient->GetMagiConSpeechParamFromNVRam(&eSphParamMagiCon);
        SetMagiConSpeechParameters(&eSphParamMagiCon);
#endif
    }

#if defined(MTK_HAC_SUPPORT)
    AUDIO_CUSTOM_HAC_PARAM_STRUCT eSphParamHAC;
    memset(&eSphParamHAC, 0, sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT));
    pAudioCustParamClient->GetHACSpeechParamFromNVRam(&eSphParamHAC);
    SetHACSpeechParameters(&eSphParamHAC);
#endif

#if defined(MTK_WB_SPEECH_SUPPORT)
    // WB Speech Enhancement Parameters
    AUDIO_CUSTOM_WB_PARAM_STRUCT eSphParamWB;
    memset(&eSphParamWB, 0, sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
    pAudioCustParamClient->GetWBSpeechParamFromNVRam(&eSphParamWB);
    SetWBSpeechParameters(&eSphParamWB);
#endif
    //#if defined(MTK_VIBSPK_SUPPORT)
    if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER)) {
        PARAM_VIBSPK eVibSpkParam;
        memset(&eVibSpkParam, 0, sizeof(PARAM_VIBSPK));
        GetVibSpkParam((void *)&eVibSpkParam);
        SetVibSpkParam((void *)&eVibSpkParam);
    }
    //#endif

    if (AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        PARAM_SMARTPA eSmartpaParam;
        memset(&eSmartpaParam, 0, sizeof(PARAM_SMARTPA));
        GetSmartpaParam((void *)&eSmartpaParam);
        SetSmartpaParam((void *)&eSmartpaParam);
    }

#endif

    // Set speech enhancement parameters' mask to modem side
    SetSpeechEnhancementMask(SpeechEnhancementController::GetInstance()->GetSpeechEnhancementMask());

    //    // Use lock to ensure the previous command with share buffer control is completed
    //    if (pCCCI->A2MBufLock() == true)
    //    {
    //        pCCCI->A2MBufUnLock();
    //    }
    //    else
    //    {
    //        ALOGE("%s() fail to get A2M Buffer Lock!!", __FUNCTION__);
    //    }

    // Unock
    AL_UNLOCK(_mutex);
    return NO_ERROR;
}


/*==============================================================================
 *                     Recover State
 *============================================================================*/

void SpeechDriverLAD::RecoverModemSideStatusToInitState() {
    // Record
    if (pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, record_on = true",  __FUNCTION__, mModemIndex);
        SetApSideModemStatus(RECORD_STATUS_MASK);
        recordOff();
    }

    // Raw Record
    if (pCCCI->GetModemSideModemStatus(RAW_RECORD_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, raw_record_on = true",  __FUNCTION__, mModemIndex);
        SetApSideModemStatus(RAW_RECORD_STATUS_MASK);
        recordOff(mRecordType);
    }

    // VM Record
    if (pCCCI->GetModemSideModemStatus(VM_RECORD_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, vm_on = true",  __FUNCTION__, mModemIndex);
        SetApSideModemStatus(VM_RECORD_STATUS_MASK);
        VoiceMemoRecordOff();
    }

    // BGS
    if (pCCCI->GetModemSideModemStatus(BGS_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, bgs_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(BGS_STATUS_MASK);
        BGSoundOff();
    }

    // TTY
    if (pCCCI->GetModemSideModemStatus(TTY_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, tty_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(TTY_STATUS_MASK);
        TtyCtmOff();
    }

    // P2W
    if (pCCCI->GetModemSideModemStatus(P2W_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, p2w_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(P2W_STATUS_MASK);
        PCM2WayOff();
    }

    // Phone Call / Loopback
    if (pCCCI->GetModemSideModemStatus(VT_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, vt_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(VT_STATUS_MASK);
        VideoTelephonyOff();
    } else if (pCCCI->GetModemSideModemStatus(SPEECH_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, speech_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(SPEECH_STATUS_MASK);
        SpeechOff();
    } else if (pCCCI->GetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, speech_router_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
        SpeechRouterOff();
    } else if (pCCCI->GetModemSideModemStatus(LOOPBACK_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, loopback_on = true", __FUNCTION__, mModemIndex);
        SetApSideModemStatus(LOOPBACK_STATUS_MASK);
        SetAcousticLoopback(false);
    }
}

/*==============================================================================
 *                     Check Modem Status
 *============================================================================*/
bool SpeechDriverLAD::CheckModemIsReady() {
    return pCCCI->CheckModemIsReady();
};

/*==============================================================================
 *                     Run-Time parameter tuning
 *============================================================================*/
int SpeechDriverLAD::updateParam(int audioTypeUpdate) {
#if (!defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT))||defined(SPH_SKIP_A2M_BUFF_MSG)
    (void) audioTypeUpdate;
    return NO_ERROR;
#else

    switch (audioTypeUpdate) {
    case AUDIO_TYPE_SPEECH_DMNR:
    case AUDIO_TYPE_SPEECH_GENERAL:
        SetDynamicSpeechParameters(audioTypeUpdate, &audioTypeUpdate);
        break;
    case AUDIO_TYPE_SPEECH: {
        bool isSpeechOn = GetApSideModemStatus(SPEECH_STATUS_MASK);
        if (isSpeechOn) {
#if defined(MTK_AUDIO_SPH_LPBK_PARAM)
            if (mIsLpbk) {
                SpeechParamParser::getInstance()->SetParamInfo(String8("ParamSphLpbk=1;"));
            }
#endif
            int param_arg[4];
            param_arg[0] = (int) mSpeechMode;
            param_arg[1] = mVolumeIndex;
            param_arg[2] = mBtHeadsetNrecOn;
            param_arg[3] = 0;//bit 0: customized profile, bit 1: single band, bit 4~7: band number
            SetDynamicSpeechParameters((int)AUDIO_TYPE_SPEECH, param_arg);
        }
        break;
    }
    default: // do nothing
        break;
    }
#endif
    return NO_ERROR;
}

} // end of namespace android

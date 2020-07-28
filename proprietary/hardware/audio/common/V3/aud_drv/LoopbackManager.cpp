#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "LoopbackManager"
#include "LoopbackManager.h"

#include <hardware_legacy/power.h>

//#include <DfoDefines.h>

#include "AudioALSAStreamManager.h"

#include "AudioALSALoopbackController.h"
#include "AudioALSASpeechLoopbackController.h"

#include <SpeechEnhancementController.h>
#include <SpeechDriverFactory.h>

#include "AudioALSAHardwareResourceManager.h"

#include "AudioVolumeFactory.h"

#if defined(MTK_NEW_VOL_CONTROL)
#include "AudioGainTableParamParser.h"
#endif

namespace android {
static const char LOOPBACK_WAKELOCK_NAME[] = "LOOPBACK_WAKELOCK_NAME";

static const float kVoiceVolumeForLoopback  = 1.0f; // max
static const float kMasterVolumeForLoopback = 1.0f; // max


LoopbackManager *LoopbackManager::mLoopbackManager = NULL;
LoopbackManager *LoopbackManager::GetInstance() {
    if (mLoopbackManager == NULL) {
        mLoopbackManager = new LoopbackManager();
    }
    ASSERT(mLoopbackManager != NULL);
    return mLoopbackManager;
}

LoopbackManager::LoopbackManager() :
    mLoopbackType(NO_LOOPBACK),
    mAudioALSAVolumeController(AudioVolumeFactory::CreateAudioVolumeController()),
    mInputDeviceCopy(AUDIO_DEVICE_IN_BUILTIN_MIC),
    mOutputDeviceCopy(AUDIO_DEVICE_OUT_SPEAKER),
    mVoiceVolumeCopy(1.0f),
    mMasterVolumeCopy(1.0f),
    mWorkingModemIndex(MODEM_1),
    mBtHeadsetNrecOnCopy(true),
    mLpbkRestoreVoiceVolume(false),
    mLpbkRestoreMasterVolume(false) {

    char isMD1Supported[PROPERTY_VALUE_MAX];
    property_get("ro.vendor.mtk_md1_support", isMD1Supported, "0"); //"0": default not support

    int i4MD1Supported = atoi(isMD1Supported);
    mWorkingModemIndex = MODEM_1;
    memset(&mMaskCopy, 0, sizeof(mMaskCopy));
    ALOGD("%s(), isMD1Supported = %s, mWorkingModemIndex=%d", __FUNCTION__, isMD1Supported, mWorkingModemIndex);

}

LoopbackManager::~LoopbackManager() {

}

loopback_t LoopbackManager::GetLoopbackType() {
    ALOGV("%s(), mLoopbackType = %d", __FUNCTION__, mLoopbackType);
    return mLoopbackType;
}

status_t LoopbackManager::SetLoopbackOn(loopback_t loopback_type, loopback_output_device_t loopback_output_device) {
    ALOGD("+%s(), loopback_type = %d, loopback_output_device = %d", __FUNCTION__,  loopback_type, loopback_output_device);

    Mutex::Autolock _l(mLock);

    if (mLoopbackType != NO_LOOPBACK) { // check no loobpack function on
        ALOGD("-%s() : Please Turn off Loopback Type %d First!!", __FUNCTION__, mLoopbackType);
        return ALREADY_EXISTS;
    } else if (CheckLoopbackTypeIsValid(loopback_type) != NO_ERROR) { // to avoid using undefined loopback type & ref/dual mic in single mic project
        ALOGW("-%s(): No such Loopback type %d", __FUNCTION__, loopback_type);
        return BAD_TYPE;
    }


    // suspend & standby all input/output streams
    // if ((loopback_type != AP_BT_LOOPBACK_NO_CODEC) && (loopback_type != AP_BT_LOOPBACK))
    // AP_BT_LOOPBACK_NO_CODEC no need to standby
    if (loopback_type != AP_BT_LOOPBACK_NO_CODEC) {
        AudioALSAStreamManager::getInstance()->setAllStreamsSuspend(true);
        AudioALSAStreamManager::getInstance()->standbyAllStreams();
    }

    // copy current device // TODO(Harvey): recover device
    //mInputDeviceCopy  = (audio_devices_t)pAudioResourceManager->getUlInputDevice();
    //mOutputDeviceCopy = (audio_devices_t)pAudioResourceManager->getDlOutputDevice();

    // get loopback device
    audio_devices_t input_device  = GetInputDeviceByLoopbackType(loopback_type);
    audio_devices_t output_device = GetOutputDeviceByLoopbackType(loopback_type, loopback_output_device);

    // set specific mic type
    if (loopback_type == AP_MAIN_MIC_AFE_LOOPBACK || loopback_type == MD_MAIN_MIC_ACOUSTIC_LOOPBACK) {
        AudioALSAHardwareResourceManager::getInstance()->setBuiltInMicSpecificType(BUILTIN_MIC_MIC1_ONLY);
    } else if (loopback_type == AP_REF_MIC_AFE_LOOPBACK || loopback_type == MD_REF_MIC_ACOUSTIC_LOOPBACK) {
        AudioALSAHardwareResourceManager::getInstance()->setBuiltInMicSpecificType(BUILTIN_MIC_MIC2_ONLY);
    } else if (loopback_type == AP_3RD_MIC_AFE_LOOPBACK || loopback_type == MD_3RD_MIC_ACOUSTIC_LOOPBACK) {
        AudioALSAHardwareResourceManager::getInstance()->setBuiltInMicSpecificType(BUILTIN_MIC_MIC3_ONLY);
    }


    // check modem status
    if (CheckIsModemLoopback(loopback_type) == true) {
        SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex(mWorkingModemIndex);
        if (pSpeechDriver->CheckModemIsReady() == false) { // modem is sleep...
            for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) { // get working modem index
                pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex((modem_index_t)modem_index);
                if (pSpeechDriver != NULL && pSpeechDriver->CheckModemIsReady() == true) {
                    mWorkingModemIndex = (modem_index_t)modem_index;
                    SpeechDriverFactory::GetInstance()->SetActiveModemIndex(mWorkingModemIndex);
                    break;
                }
            }
        }
    }

    // to avoid BT test being interferenced by modem side speech enhancement
    mBtHeadsetNrecOnCopy = SpeechEnhancementController::GetInstance()->GetBtHeadsetNrecOn();
    if (loopback_type == MD_BT_LOOPBACK || loopback_type == MD_BT_LOOPBACK_NO_CODEC) {
        SpeechEnhancementController::GetInstance()->SetBtHeadsetNrecOnToAllModem(false);
    }

    // to turn on/off DMNR
    if (loopback_type == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR ||
        loopback_type == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR) {
        mMaskCopy = SpeechEnhancementController::GetInstance()->GetSpeechEnhancementMask(); // copy DMNR mask
        sph_enh_mask_struct_t mask = mMaskCopy;
        if (loopback_type == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR) {
            mask.dynamic_func &= (~SPH_ENH_DYNAMIC_MASK_DMNR);
        } else if (loopback_type == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR) {
            mask.dynamic_func |= SPH_ENH_DYNAMIC_MASK_DMNR;
        }
        SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex(mWorkingModemIndex)->SetSpeechEnhancementMask(mask);
    }

    // save opened loobpack type
    mLoopbackType = loopback_type;

    // BT CVSD
    if (loopback_type == AP_BT_LOOPBACK) {
        AudioALSALoopbackController::getInstance()->SetApBTCodec(true);
        AudioALSALoopbackController::getInstance()->OpenAudioLoopbackControlFlow(input_device, output_device);
    } else if (loopback_type == AP_BT_LOOPBACK_NO_CODEC) {
        AudioALSALoopbackController::getInstance()->SetApBTCodec(false);
        AudioALSALoopbackController::getInstance()->OpenAudioLoopbackControlFlow(input_device, output_device);
    } else if (loopback_type == MD_BT_LOOPBACK) {
        AudioALSASpeechLoopbackController::getInstance()->SetModemBTCodec(true);
        AudioALSASpeechLoopbackController::getInstance()->OpenModemLoopbackControlFlow(input_device, output_device);
    } else if (loopback_type == MD_BT_LOOPBACK_NO_CODEC) {
        AudioALSASpeechLoopbackController::getInstance()->SetModemBTCodec(false);
        AudioALSASpeechLoopbackController::getInstance()->OpenModemLoopbackControlFlow(input_device, output_device);
    } else {
        // Enable loopback function
        switch (loopback_type) {
        case AP_MAIN_MIC_AFE_LOOPBACK:
        case AP_HEADSET_MIC_AFE_LOOPBACK:
        case AP_REF_MIC_AFE_LOOPBACK:
        case AP_3RD_MIC_AFE_LOOPBACK:
        //case AP_BT_LOOPBACK:
        //case AP_BT_LOOPBACK_NO_CODEC:
        {
            AudioALSALoopbackController::getInstance()->open(output_device, input_device, loopback_type);
            break;
        }
        case MD_MAIN_MIC_ACOUSTIC_LOOPBACK:
        case MD_HEADSET_MIC_ACOUSTIC_LOOPBACK:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR:
        case MD_REF_MIC_ACOUSTIC_LOOPBACK:
        case MD_3RD_MIC_ACOUSTIC_LOOPBACK:
        //case MD_BT_LOOPBACK:
        //case MD_BT_LOOPBACK_NO_CODEC:
        {
#if (!defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)) && defined(MTK_AUDIO_SPH_LPBK_PARAM)
            AudioALSAStreamManager::getInstance()->UpdateSpeechLpbkParams();
#endif
            AudioALSASpeechLoopbackController::getInstance()->open(output_device, input_device, loopback_type);
            break;
        }
        default: {
            ALOGW("%s(): Loopback type %d not implemented!!", __FUNCTION__, loopback_type);
            ASSERT(0);
        }
        }
    }
    /*
        // only use L ch data, so mute R ch. (Disconnect ADC_I2S_IN_R -> MODEM_PCM_TX_R)
        if (loopback_type == MD_MAIN_MIC_ACOUSTIC_LOOPBACK ||
            loopback_type == MD_REF_MIC_ACOUSTIC_LOOPBACK)
        {
            AudioDigitalControlFactory::CreateAudioDigitalControl()->SetinputConnection(
                AudioDigitalType::DisConnect,
                AudioDigitalType::I04,
                (mWorkingModemIndex == MODEM_1) ? AudioDigitalType::O18 : AudioDigitalType::O08);
        }
    */

    // acquire wake lock
    int ret = acquire_wake_lock(PARTIAL_WAKE_LOCK, LOOPBACK_WAKELOCK_NAME);
    ALOGD("%s(), acquire_wake_lock:%s, return %d.", __FUNCTION__, LOOPBACK_WAKELOCK_NAME, ret);
    // Volume
    if ((loopback_type != AP_BT_LOOPBACK) && (loopback_type != AP_BT_LOOPBACK_NO_CODEC) && (loopback_type != MD_BT_LOOPBACK) && (loopback_type != MD_BT_LOOPBACK_NO_CODEC)) {
        if (CheckIsModemLoopback(loopback_type) == true) {
            mVoiceVolumeCopy = mAudioALSAVolumeController->getVoiceVolume();
#if defined(MTK_NEW_VOL_CONTROL)
            mAudioALSAVolumeController->setAnalogVolume(AUDIO_STREAM_VOICE_CALL, output_device, GAIN_MAX_SPEECH_VOL_INDEX, AUDIO_MODE_IN_CALL);
#else
            mAudioALSAVolumeController->setVoiceVolume(kVoiceVolumeForLoopback, AUDIO_MODE_IN_CALL, output_device);
#endif
        } else {
            mMasterVolumeCopy = mAudioALSAVolumeController->getMasterVolume();
#if defined(MTK_NEW_VOL_CONTROL)
            mAudioALSAVolumeController->setAnalogVolume(AUDIO_STREAM_MUSIC, output_device, GAIN_MAX_VOL_INDEX, AUDIO_MODE_NORMAL);
#else
            mAudioALSAVolumeController->setMasterVolume(kMasterVolumeForLoopback, AUDIO_MODE_NORMAL, output_device);
#endif
        }
    }

    ALOGD("-%s(), loopback_type = %d, loopback_output_device = %d", __FUNCTION__,  loopback_type, loopback_output_device);
    return NO_ERROR;
}

status_t LoopbackManager::SetLoopbackOff() {
    Mutex::Autolock _l(mLock);

    ALOGD("+%s(), mLoopbackType = %d", __FUNCTION__, mLoopbackType);
    if (mLoopbackType == NO_LOOPBACK) { // check loobpack do exist to be turned off
        ALOGD("-%s() : No looback to be closed", __FUNCTION__);
        return INVALID_OPERATION;
    }

    // Disable Loopback function

    // BT CVSD
    if ((mLoopbackType == AP_BT_LOOPBACK) || (mLoopbackType == AP_BT_LOOPBACK_NO_CODEC)) {
        AudioALSALoopbackController::getInstance()->CloseAudioLoopbackControlFlow();
    } else if ((mLoopbackType == MD_BT_LOOPBACK) || (mLoopbackType == MD_BT_LOOPBACK_NO_CODEC)) {
        AudioALSASpeechLoopbackController::getInstance()->CloseModemLoopbackControlFlow();
    } else {
        switch (mLoopbackType) {
        case AP_MAIN_MIC_AFE_LOOPBACK:
        case AP_HEADSET_MIC_AFE_LOOPBACK:
        case AP_REF_MIC_AFE_LOOPBACK:
        case AP_3RD_MIC_AFE_LOOPBACK:
            //case AP_BT_LOOPBACK:
            //case AP_BT_LOOPBACK_NO_CODEC:
        {
            AudioALSALoopbackController::getInstance()->close();
            break;
        }
        case MD_MAIN_MIC_ACOUSTIC_LOOPBACK:
        case MD_HEADSET_MIC_ACOUSTIC_LOOPBACK:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR:
        case MD_REF_MIC_ACOUSTIC_LOOPBACK:
        case MD_3RD_MIC_ACOUSTIC_LOOPBACK:
            //case MD_BT_LOOPBACK:
            //case MD_BT_LOOPBACK_NO_CODEC:
        {
            AudioALSASpeechLoopbackController::getInstance()->close();
#if !defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)&&defined(MTK_AUDIO_SPH_LPBK_PARAM)
            AudioALSAStreamManager::getInstance()->updateSpeechNVRAMParam(0);
#endif
            break;
        }
        default: {
            ALOGW("%s(): Loopback type %d not implemented!!", __FUNCTION__, mLoopbackType);
            ASSERT(0);
        }
        }
    }

    // recover DMNR
    if (mLoopbackType == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR ||
        mLoopbackType == MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR) {
        SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex(mWorkingModemIndex)->SetSpeechEnhancementMask(mMaskCopy);
    }

    // recover modem side speech enhancement
    if (mLoopbackType == MD_BT_LOOPBACK || mLoopbackType == MD_BT_LOOPBACK_NO_CODEC) {
        SpeechEnhancementController::GetInstance()->SetBtHeadsetNrecOnToAllModem(mBtHeadsetNrecOnCopy);
    }

    // recover device
    //pAudioResourceManager->setDlOutputDevice(mOutputDeviceCopy);
    //pAudioResourceManager->setUlInputDevice(mInputDeviceCopy);

    // recover specific mic type
    if (mLoopbackType == AP_MAIN_MIC_AFE_LOOPBACK || mLoopbackType == MD_MAIN_MIC_ACOUSTIC_LOOPBACK ||
        mLoopbackType == AP_REF_MIC_AFE_LOOPBACK || mLoopbackType == MD_REF_MIC_ACOUSTIC_LOOPBACK ||
        mLoopbackType == AP_3RD_MIC_AFE_LOOPBACK || mLoopbackType == MD_3RD_MIC_ACOUSTIC_LOOPBACK) {
        AudioALSAHardwareResourceManager::getInstance()->setBuiltInMicSpecificType(BUILTIN_MIC_DEFAULT);
    }


    // recover volume
    if ((mLoopbackType != AP_BT_LOOPBACK) && (mLoopbackType != AP_BT_LOOPBACK_NO_CODEC) && (mLoopbackType != MD_BT_LOOPBACK) && (mLoopbackType != MD_BT_LOOPBACK_NO_CODEC)) {
        if (CheckIsModemLoopback(mLoopbackType) == true) {
            mAudioALSAVolumeController->setVoiceVolume(mVoiceVolumeCopy, AUDIO_MODE_IN_CALL, mOutputDeviceCopy);
        } else {
            mAudioALSAVolumeController->setMasterVolume(mMasterVolumeCopy, AUDIO_MODE_NORMAL, mOutputDeviceCopy);
        }
    }

    // release wake lock
    int ret = release_wake_lock(LOOPBACK_WAKELOCK_NAME);
    ALOGD("%s(), release_wake_lock:%s return %d.", __FUNCTION__, LOOPBACK_WAKELOCK_NAME, ret);

    // unsuspend all input/output streams
    if (mLoopbackType != AP_BT_LOOPBACK_NO_CODEC) {
        AudioALSAStreamManager::getInstance()->setAllStreamsSuspend(false);
    }

    // clean
    mLoopbackType = NO_LOOPBACK;

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t LoopbackManager::CheckLoopbackTypeIsValid(loopback_t loopback_type) {
    status_t retval;

    switch (loopback_type) {
    case AP_MAIN_MIC_AFE_LOOPBACK:
    case AP_HEADSET_MIC_AFE_LOOPBACK:
    case AP_3RD_MIC_AFE_LOOPBACK:
    case MD_MAIN_MIC_ACOUSTIC_LOOPBACK:
    case MD_HEADSET_MIC_ACOUSTIC_LOOPBACK:
    case MD_3RD_MIC_ACOUSTIC_LOOPBACK:
    case AP_BT_LOOPBACK:
    case MD_BT_LOOPBACK:
    case AP_BT_LOOPBACK_NO_CODEC:
    case MD_BT_LOOPBACK_NO_CODEC:
        retval = NO_ERROR;
        break;
    case AP_REF_MIC_AFE_LOOPBACK:
    case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR:
    case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR:
    case MD_REF_MIC_ACOUSTIC_LOOPBACK:
        if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
            retval = NO_ERROR;
        } else {
            retval = BAD_TYPE;
        }
        break;
    default:
        retval = BAD_TYPE;
        break;
    }

    return retval;
}


audio_devices_t LoopbackManager::GetInputDeviceByLoopbackType(loopback_t loopback_type) {
    audio_devices_t input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;

    switch (loopback_type) {
    case AP_MAIN_MIC_AFE_LOOPBACK:
    case MD_MAIN_MIC_ACOUSTIC_LOOPBACK:
    case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR:
    case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR:
    case AP_REF_MIC_AFE_LOOPBACK:
    case MD_REF_MIC_ACOUSTIC_LOOPBACK:
    case AP_3RD_MIC_AFE_LOOPBACK:
    case MD_3RD_MIC_ACOUSTIC_LOOPBACK: {
        input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        break;
    }
    case AP_HEADSET_MIC_AFE_LOOPBACK:
    case MD_HEADSET_MIC_ACOUSTIC_LOOPBACK: {
        input_device = AUDIO_DEVICE_IN_WIRED_HEADSET;
        break;
    }
    case AP_BT_LOOPBACK:
    case MD_BT_LOOPBACK:
    case AP_BT_LOOPBACK_NO_CODEC:
    case MD_BT_LOOPBACK_NO_CODEC: {
        input_device = AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET;
        break;
    }
    default: {
        ALOGW("%s(): Loopback type %d not implemented!!", __FUNCTION__, loopback_type);
        ASSERT(0);
    }
    }

    return input_device;
}

audio_devices_t LoopbackManager::GetOutputDeviceByLoopbackType(loopback_t loopback_type, loopback_output_device_t loopback_output_device) {
    // BT Loopback only use BT headset
    if (loopback_type == AP_BT_LOOPBACK ||
        loopback_type == MD_BT_LOOPBACK ||
        loopback_type == AP_BT_LOOPBACK_NO_CODEC ||
        loopback_type == MD_BT_LOOPBACK_NO_CODEC) { // BT
        return AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET;
    }

    // Get Output Devices By LoopbackType
    audio_devices_t output_device;

    switch (loopback_output_device) {
    case LOOPBACK_OUTPUT_RECEIVER: {
        output_device = AUDIO_DEVICE_OUT_EARPIECE;
        break;
    }
    case LOOPBACK_OUTPUT_EARPHONE: {
        if (loopback_type == AP_HEADSET_MIC_AFE_LOOPBACK ||
            loopback_type == MD_HEADSET_MIC_ACOUSTIC_LOOPBACK) {
            output_device = AUDIO_DEVICE_OUT_WIRED_HEADSET;
        } else {
            output_device = AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
        }
        break;
    }
    case LOOPBACK_OUTPUT_SPEAKER: {
        output_device = AUDIO_DEVICE_OUT_SPEAKER;
        break;
    }
    default: {
        output_device = AUDIO_DEVICE_OUT_EARPIECE;
        break;
    }
    }

    return output_device;
}


} // end of namespace android

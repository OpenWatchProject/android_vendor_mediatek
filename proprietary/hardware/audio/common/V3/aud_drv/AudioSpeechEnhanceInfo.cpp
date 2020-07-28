#include "AudioSpeechEnhanceInfo.h"
#include <utils/Log.h>
#include <utils/String16.h>
#include "AudioUtility.h"
#include <cutils/properties.h>
#include "AudioALSAStreamManager.h"
//#include <AudioLock.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioSpeechEnhanceInfo"
static const char* PROPERTY_KEY_VOIP_SPH_ENH_MASKS = "persist.vendor.audiohal.voip.sph_enh_mask";


namespace android {
AudioSpeechEnhanceInfo *AudioSpeechEnhanceInfo::mAudioSpeechEnhanceInfo = NULL;

AudioSpeechEnhanceInfo *AudioSpeechEnhanceInfo::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioSpeechEnhanceInfo == NULL) {
        mAudioSpeechEnhanceInfo = new AudioSpeechEnhanceInfo();
    }
    ASSERT(mAudioSpeechEnhanceInfo != NULL);
    return mAudioSpeechEnhanceInfo;
}

AudioSpeechEnhanceInfo::AudioSpeechEnhanceInfo() {
    ALOGV("%s()", __FUNCTION__);
    mBesRecScene = -1;

    //for tuning purpose
    mBesRecTuningEnable = false;
    mAPDMNRTuningEnable = false;
    mAPTuningMode = TUNING_MODE_NONE;

    mForceMagiASR = false;
    mForceAECRec = false;

    mHiFiRecordEnable = false;

    // default value (all enhancement on)
    char property_default_value[PROPERTY_VALUE_MAX];
    sprintf(property_default_value, "0x%x", VOIP_SPH_ENH_DYNAMIC_MASK_ALL);

    // get voip sph_enh_mask_struct from property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_VOIP_SPH_ENH_MASKS, property_value, property_default_value);

    // parse mask info from property_value
    sscanf(property_value, "0x%x", &mVoIPSpeechEnhancementMask.dynamic_func);

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
    memset((void *)&mPreLoadBesRecordSceneTable, 0, sizeof(mPreLoadBesRecordSceneTable));
    memset((void *)&mPreLoadBesRecordParam, 0, sizeof(mPreLoadBesRecordParam));
    memset((void *)&mPreLoadVOIPParam, 0, sizeof(mPreLoadVOIPParam));
    memset((void *)&mPreLoadDMNRParam, 0, sizeof(mPreLoadDMNRParam));
#endif

    memset(mVMFileName, 0, VM_FILE_NAME_LEN_MAX);

    mAudioCustParamClient = NULL;
    mAudioCustParamClient = AudioCustParamClient::GetInstance();
    ALOGD("%s(), mAudioCustParamClient(%p)", __FUNCTION__, mAudioCustParamClient);

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
    PreLoadBesRecordParams();
#endif

    //debug purpose++
    mDebugflag = false;
    char debug_value[PROPERTY_VALUE_MAX];
    property_get(streaminlog_propty, debug_value, "0");
    int bflag = atoi(debug_value);
    if (bflag) {
        mDebugflag = true;
    }
    //debug purpose--
}

AudioSpeechEnhanceInfo::~AudioSpeechEnhanceInfo() {
    ALOGD("%s()", __FUNCTION__);
}

//----------------for HD Record Preprocess +++ -------------------------
void AudioSpeechEnhanceInfo::SetBesRecScene(int32_t BesRecScene) {
    AL_AUTOLOCK(mLock);
    ALOGV("%s() %d", __FUNCTION__, BesRecScene);
    mBesRecScene = BesRecScene;
}

int32_t AudioSpeechEnhanceInfo::GetBesRecScene() {
    AL_AUTOLOCK(mLock);
    ALOGV("%s() %d", __FUNCTION__, mBesRecScene);
    return mBesRecScene;
}

void AudioSpeechEnhanceInfo::ResetBesRecScene() {
    AL_AUTOLOCK(mLock);
    ALOGV("%s()", __FUNCTION__);
    mBesRecScene = -1;
}

//----------------for HD Record Preprocess --- -----------------------------

//----------------Get MMI info for AP Speech Enhancement --------------------------------
void AudioSpeechEnhanceInfo::UpdateDynamicSpeechEnhancementMask(const voip_sph_enh_mask_struct_t &mask) {
    uint32_t feature_support = mAudioCustParamClient->QueryFeatureSupportInfo();

    ALOGD("%s(), mask = %x, feature_support=%x, %x", __FUNCTION__, mask.dynamic_func, feature_support, (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE)));

    if (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE)) {

        char property_value[PROPERTY_VALUE_MAX];
        sprintf(property_value, "0x%x", mask.dynamic_func);
        property_set(PROPERTY_KEY_VOIP_SPH_ENH_MASKS, property_value);

        mVoIPSpeechEnhancementMask = mask;
        AudioALSAStreamManager::getInstance()->UpdateDynamicFunctionMask();
    } else {
        ALOGD("%s(), not support", __FUNCTION__);
    }

}

status_t AudioSpeechEnhanceInfo::SetDynamicVoIPSpeechEnhancementMask(const voip_sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on) {
    //Mutex::Autolock lock(mHDRInfoLock);
    uint32_t feature_support = mAudioCustParamClient->QueryFeatureSupportInfo();

    ALOGD("%s(), feature_support=%x, %x", __FUNCTION__, feature_support, (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE)));

    if (feature_support & (SUPPORT_DMNR_3_0 | SUPPORT_VOIP_ENHANCE)) {
        voip_sph_enh_mask_struct_t mask = GetDynamicVoIPSpeechEnhancementMask();

        ALOGW("%s(), dynamic_mask_type(%x), %x",
              __FUNCTION__, dynamic_mask_type, mask.dynamic_func);
        const bool current_flag_on = ((mask.dynamic_func & dynamic_mask_type) > 0);
        if (new_flag_on == current_flag_on) {
            ALOGW("%s(), dynamic_mask_type(%x), new_flag_on(%d) == current_flag_on(%d), return",
                  __FUNCTION__, dynamic_mask_type, new_flag_on, current_flag_on);
            return NO_ERROR;
        }

        if (new_flag_on == false) {
            mask.dynamic_func &= (~dynamic_mask_type);
        } else {
            mask.dynamic_func |= dynamic_mask_type;
        }

        UpdateDynamicSpeechEnhancementMask(mask);
    } else {
        ALOGW("%s(), not support", __FUNCTION__);
    }

    return NO_ERROR;
}

//----------------Audio tunning +++ --------------------------------
//----------------for BesRec tunning --------------------------------
void AudioSpeechEnhanceInfo::SetBesRecTuningEnable(bool bEnable) {
    ALOGV("%s()+", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    mBesRecTuningEnable = bEnable;
    ALOGV("%s()- %d", __FUNCTION__, bEnable);
}

bool AudioSpeechEnhanceInfo::IsBesRecTuningEnable(void) {
    ALOGV("%s()+", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    ALOGD("%s()- %d", __FUNCTION__, mBesRecTuningEnable);
    return mBesRecTuningEnable;
}

status_t AudioSpeechEnhanceInfo::SetBesRecVMFileName(const char *fileName) {
    ALOGD("%s()+", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    if (fileName != NULL && strlen(fileName) < 128 - 1) {
        ALOGV("%s(), file name:%s", __FUNCTION__, fileName);
        audio_strncpy(mVMFileName, fileName, VM_FILE_NAME_LEN_MAX);
    } else {
        ALOGV("%s(), input file name NULL or too long!", __FUNCTION__);
        return BAD_VALUE;
    }
    return NO_ERROR;
}
void AudioSpeechEnhanceInfo::GetBesRecVMFileName(char *VMFileName, size_t string_size) {
    ALOGV("%s()+", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    audio_strncpy(VMFileName, mVMFileName, string_size);
    ALOGV("%s(), mVMFileName=%s, VMFileName=%s", __FUNCTION__, mVMFileName, VMFileName);
}

//----------------for AP DMNR tunning +++ --------------------------------
void AudioSpeechEnhanceInfo::SetAPDMNRTuningEnable(bool bEnable) {
    if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) {
        AL_AUTOLOCK(mLock);
        ALOGV("%s(), %d", __FUNCTION__, bEnable);
        mAPDMNRTuningEnable = bEnable;
    } else {
        ALOGV("%s(), no Dual MIC, not set", __FUNCTION__);
    }
}

bool AudioSpeechEnhanceInfo::IsAPDMNRTuningEnable(void) {
    if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) {
        ALOGV("%s()+", __FUNCTION__);
        AL_AUTOLOCK(mLock);
        ALOGD("%s(), %d", __FUNCTION__, mAPDMNRTuningEnable);
        return mAPDMNRTuningEnable;
    } else {
        return false;
    }
}

bool AudioSpeechEnhanceInfo::SetAPTuningMode(const TOOL_TUNING_MODE mode) {
    bool bRet = false;
    //for different MIC gain
    ALOGV("%s(), SetAPTuningMode mAPDMNRTuningEnable=%d, mode=%d", __FUNCTION__, mAPDMNRTuningEnable, mode);
    if (mAPDMNRTuningEnable) {
        mAPTuningMode = mode;
        bRet = true;
    }
    return bRet;
}

int AudioSpeechEnhanceInfo::GetAPTuningMode() {
    ALOGV("%s(), mAPTuningMode=%d", __FUNCTION__, mAPTuningMode);

    return mAPTuningMode;
}

//----------------Audio tunning --- --------------------------------

//Engineer mode enable MagiASR+++
bool AudioSpeechEnhanceInfo::SetForceMagiASR(bool enable) {
    ALOGV("%s, %d", __FUNCTION__, enable);
    mForceMagiASR = enable;
    return true;
}

status_t AudioSpeechEnhanceInfo::GetForceMagiASRState() {
    status_t ret = 0;
    uint32_t feature_support = mAudioCustParamClient->QueryFeatureSupportInfo();

    ALOGV("%s(), feature_support=%x, %x, mForceMagiASR=%d", __FUNCTION__, feature_support, (feature_support & SUPPORT_ASR), mForceMagiASR);

    if (feature_support & SUPPORT_ASR) {
        if (mForceMagiASR) {
            ret = 1;
        } else {
            ret = -1;
        }
    } else {
        ret = 0;
    }

    return ret;
}

//Engineer mode MagiASR---

//Engineer mode enable AECRecord+
bool AudioSpeechEnhanceInfo::SetForceAECRec(bool enable) {
    ALOGV("%s, %d", __FUNCTION__, enable);
    mForceAECRec = enable;
    return true;
}

bool AudioSpeechEnhanceInfo::GetForceAECRecState() {
    status_t ret = false;

    ALOGV("%s(), mForceAECRec=%d", __FUNCTION__, mForceAECRec);

    if (mForceAECRec) {
        ret = true;
    }
    return ret;
}
//Engineer mode enable AECRecord-

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
//BesRecord+++
//preload BesRecord parames to avoid record loading fail when storage full
void AudioSpeechEnhanceInfo::PreLoadBesRecordParams(void) {
    ALOGD("%s+", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    //for NVRAM create file first to reserve the memory
    // get scene table
    if (mAudioCustParamClient->GetHdRecordSceneTableFromNV(&mPreLoadBesRecordSceneTable) == 0) {
        ALOGD("GetHdRecordSceneTableFromNV fail");
    }

    // get hd rec param
    if (mAudioCustParamClient->GetHdRecordParamFromNV(&mPreLoadBesRecordParam) == 0) {
        ALOGD("GetHdRecordParamFromNV fail");
    }

    //get VoIP param
    if (mAudioCustParamClient->GetAudioVoIPParamFromNV(&mPreLoadVOIPParam) == 0) {
        ALOGD("GetAudioVoIPParamFromNV fail");
    }

    //get DMNR param
    if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) {
        if (mAudioCustParamClient->GetDualMicSpeechParamFromNVRam(&mPreLoadDMNRParam) == 0) {
            ALOGD("GetDualMicSpeechParamFromNVRam fail");
        }
    }
    ALOGD("%s-", __FUNCTION__);
}

void AudioSpeechEnhanceInfo::UpdateBesRecordParams(void) {
    ALOGD("%s+", __FUNCTION__);
    PreLoadBesRecordParams();
    ALOGD("%s-", __FUNCTION__);
}

void AudioSpeechEnhanceInfo::GetPreLoadBesRecordSceneTable(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara) {
    ALOGD("%s+", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    memcpy((void *)pPara, &mPreLoadBesRecordSceneTable, sizeof(mPreLoadBesRecordSceneTable));
    ALOGD("%s-", __FUNCTION__);
}
void AudioSpeechEnhanceInfo::GetPreLoadBesRecordParam(AUDIO_HD_RECORD_PARAM_STRUCT *pPara) {
    ALOGD("%s+", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    memcpy((void *)pPara, &mPreLoadBesRecordParam, sizeof(mPreLoadBesRecordParam));
    ALOGD("%s-", __FUNCTION__);
}
void AudioSpeechEnhanceInfo::GetPreLoadAudioVoIPParam(AUDIO_VOIP_PARAM_STRUCT *pPara) {
    ALOGD("%s+", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    memcpy((void *)pPara, &mPreLoadVOIPParam, sizeof(mPreLoadVOIPParam));
    ALOGD("%s-", __FUNCTION__);
}
void AudioSpeechEnhanceInfo::GetPreLoadDualMicSpeechParam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic) {
    ALOGD("%s+", __FUNCTION__);
    AL_AUTOLOCK(mLock);
    memcpy((void *)pSphParamDualMic, &mPreLoadDMNRParam, sizeof(mPreLoadDMNRParam));
    ALOGD("%s-", __FUNCTION__);
}

//BesRecord---
#endif

//Hifi Record +++
void AudioSpeechEnhanceInfo::SetHifiRecord(bool bEnable) {
    ALOGV("%s+, bEnable=%d", __FUNCTION__, bEnable);
    AL_AUTOLOCK(mLock);
    mHiFiRecordEnable = bEnable;
    ALOGV("%s, mHiFiRecordEnable=%d", __FUNCTION__, mHiFiRecordEnable);
}

bool AudioSpeechEnhanceInfo::GetHifiRecord(void) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s, mHiFiRecordEnable=%d", __FUNCTION__, mHiFiRecordEnable);
    return mHiFiRecordEnable;
}
//Hifi Record ---

//debug purpose
bool AudioSpeechEnhanceInfo::GetDebugStatus(void) {
    ALOGV("%s, mDebugflag=%d", __FUNCTION__, mDebugflag);
    return mDebugflag;
}

}


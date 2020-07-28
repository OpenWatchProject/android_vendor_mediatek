#include <inttypes.h>
#include <SpeechEnhancementController.h>
#include <SpeechDriverFactory.h>
#include <SpeechDriverInterface.h>
#include <SpeechType.h>
#include <SpeechParserType.h>

#include <AudioALSAHardwareResourceManager.h>

#include <cutils/properties.h>

#include <utils/threads.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechEnhancementController"

static const char *PROPERTY_KEY_SPH_ENH_MASKS = "persist.vendor.audiohal.modem.sph_enh_mask";
static const char *PROPERTY_KEY_BT_HEADSET_NREC_ON = "persist.vendor.audiohal.bt_headset_nrec_on";
static const char *PROPERTY_KEY_HAC_ON = "persist.vendor.audiohal.hac_on";


namespace android {

SpeechEnhancementController *SpeechEnhancementController::mSpeechEnhancementController = NULL;
SpeechEnhancementController *SpeechEnhancementController::GetInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mSpeechEnhancementController == NULL) {
        mSpeechEnhancementController = new SpeechEnhancementController();
    }
    ASSERT(mSpeechEnhancementController != NULL);
    return mSpeechEnhancementController;
}


SpeechEnhancementController::SpeechEnhancementController() {
    // default value (all enhancement on)
    char property_default_value[PROPERTY_VALUE_MAX];
    sph_enh_mask_struct_t default_mask;

    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        default_mask.main_func = SPH_ENH_MAIN_MASK_ALL;
        default_mask.dynamic_func = (SPH_ENH_DYNAMIC_MASK_ALL &
                                     (~SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) &
                                     (~SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN));
#if defined(MTK_INCALL_HANDSFREE_DMNR)
        default_mask.dynamic_func |= SPH_ENH_DYNAMIC_MASK_LSPK_DMNR;
#endif

    } else {
        default_mask.main_func = (SPH_ENH_MAIN_MASK_ALL & (~SPH_ENH_MAIN_MASK_DMNR));
        default_mask.dynamic_func = (SPH_ENH_DYNAMIC_MASK_ALL &
                                     (~SPH_ENH_DYNAMIC_MASK_DMNR) &
                                     (~SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) &
                                     (~SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN));
    }

    ALOGV("default_mask: main_func = 0x%x, sub_func = 0x%x",
          default_mask.main_func,
          default_mask.dynamic_func);
    snprintf(property_default_value, sizeof(property_default_value),
             "%d,%d", default_mask.main_func, default_mask.dynamic_func);

    // get sph_enh_mask_struct from property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_SPH_ENH_MASKS, property_value, property_default_value);

    // parse mask info from property_value
    sscanf(property_value, "%" SCNd16 ",%" SCNd32,
           &mSpeechEnhancementMask.main_func,
           &mSpeechEnhancementMask.dynamic_func);


    ALOGD("mSpeechEnhancementMask: main_func = 0x%x, sub_func = 0x%x",
          mSpeechEnhancementMask.main_func,
          mSpeechEnhancementMask.dynamic_func);

    // Magic conference call
    mMagicConferenceCallOn = (mSpeechEnhancementMask.dynamic_func & SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) ? true : false;

    // HAC
    char hac_on[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_HAC_ON, hac_on, "0"); //"0": default off
    mHACOn = (hac_on[0] == '0') ? false : true;

    // BT Headset NREC
    char bt_headset_nrec_on[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_BT_HEADSET_NREC_ON, bt_headset_nrec_on, "1"); //"1": default on
    mBtHeadsetNrecOn = (bt_headset_nrec_on[0] == '0') ? false : true;
    mSMNROn = false;
}

SpeechEnhancementController::~SpeechEnhancementController() {

}

status_t SpeechEnhancementController::SetNBSpeechParametersToAllModem(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB) {
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;
    AUDIO_CUSTOM_PARAM_STRUCT mSphParamNB;

    if (mSMNROn == true) {
        //forcely set single mic setting
        ALOGD("%s(), mSMNROn = %d, set single mic setting", __FUNCTION__, mSMNROn);
        memcpy(&mSphParamNB, pSphParamNB, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        for (int speech_mode_index = 0; speech_mode_index < 8; speech_mode_index++) {
            (mSphParamNB.speech_mode_para[speech_mode_index][13]) = 0;
            (mSphParamNB.speech_mode_para[speech_mode_index][14]) = 0;

        }
    }

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
            if (mSMNROn != true) {
                pSpeechDriver->SetNBSpeechParameters(pSphParamNB);
            } else {
                pSpeechDriver->SetNBSpeechParameters(&mSphParamNB);
            }
        }
    }

    return NO_ERROR;
}

status_t SpeechEnhancementController::SetDualMicSpeechParametersToAllModem(const AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic) {
    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() < 2) {
        ALOGE("%s()", __FUNCTION__);
        return INVALID_OPERATION;
    }

    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
            pSpeechDriver->SetDualMicSpeechParameters(pSphParamDualMic);
        }
    }

    return NO_ERROR;
}

status_t SpeechEnhancementController::SetMagiConSpeechParametersToAllModem(const AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon) {
    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() < 2) {
        ALOGE("%s()", __FUNCTION__);
        return INVALID_OPERATION;
    }

#if defined(MTK_MAGICONFERENCE_SUPPORT)
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
            pSpeechDriver->SetMagiConSpeechParameters(pSphParamMagiCon);
        }
    }

    return NO_ERROR;
#else
    (void)pSphParamMagiCon;
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
#endif
}

status_t SpeechEnhancementController::SetHACSpeechParametersToAllModem(const AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC) {
#if defined(MTK_HAC_SUPPORT)
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
            pSpeechDriver->SetHACSpeechParameters(pSphParamHAC);
        }
    }
    return NO_ERROR;
#else
    ALOGE("%s()", __FUNCTION__);
    (void)pSphParamHAC;
    return INVALID_OPERATION;
#endif
}


#if defined(MTK_WB_SPEECH_SUPPORT)
status_t SpeechEnhancementController::SetWBSpeechParametersToAllModem(const AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB) {
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;
    AUDIO_CUSTOM_WB_PARAM_STRUCT mSphParamWB;

    if (mSMNROn == true) {
        //forcely set single mic setting
        ALOGD("%s(), mSMNROn = %d, set single mic setting", __FUNCTION__, mSMNROn);
        memcpy(&mSphParamWB, pSphParamWB, sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));

        for (int speech_mode_index = 0; speech_mode_index < 8; speech_mode_index++) {
            (mSphParamWB.speech_mode_wb_para[speech_mode_index][13]) = 0;
            (mSphParamWB.speech_mode_wb_para[speech_mode_index][14]) = 0;
        }
    }

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
            if (mSMNROn != true) {
                pSpeechDriver->SetWBSpeechParameters(pSphParamWB);
            } else {
                pSpeechDriver->SetWBSpeechParameters(&mSphParamWB);
            }
        }
    }

    return NO_ERROR;
}
#else
status_t SpeechEnhancementController::SetWBSpeechParametersToAllModem(const AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB) {
    ALOGE("%s()", __FUNCTION__);
    return INVALID_OPERATION;
}
#endif

status_t SpeechEnhancementController::SetNBSpeechLpbkParametersToAllModem(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB, AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT *pSphParamNBLpbk) {
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;
    AUDIO_CUSTOM_PARAM_STRUCT mSphParamNB;

    memcpy(&mSphParamNB, pSphParamNB, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
    //replace receiver/headset/loudspk mode parameters
    memcpy(&mSphParamNB.speech_mode_para[0][0], pSphParamNBLpbk, sizeof(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT));

    //    ALOGD("%s(), speech [0][0] (%d) ori253,lpbk224", __FUNCTION__, mSphParamNB.speech_mode_para[0][0]);
    //    ALOGD("%s(), speech [0][1] (%d) ori253,lpbk224", __FUNCTION__, mSphParamNB.speech_mode_para[0][1]);
    //    ALOGD("%s(), speech [0][2] (%d) ori253,lpbk224", __FUNCTION__, mSphParamNB.speech_mode_para[0][2]);
    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
            pSpeechDriver->SetNBSpeechParameters(&mSphParamNB);
        }
    }

    return NO_ERROR;
}

status_t SpeechEnhancementController::SetSpeechEnhancementMaskToAllModem(const sph_enh_mask_struct_t &mask) {
    char property_value[PROPERTY_VALUE_MAX];
    snprintf(property_value, sizeof(property_value), "%d,%d",
             mask.main_func, mask.dynamic_func);
    property_set(PROPERTY_KEY_SPH_ENH_MASKS, property_value);

    mSpeechEnhancementMask = mask;

    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
            pSpeechDriver->SetSpeechEnhancementMask(mSpeechEnhancementMask);
        }
    }

    return NO_ERROR;
}


status_t SpeechEnhancementController::SetDynamicMaskOnToAllModem(const sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on) {
    sph_enh_mask_struct_t mask = GetSpeechEnhancementMask();

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

#if defined(MTK_COMBO_MODEM_SUPPORT)
    SpeechFeatureType speechFeature = SPEECH_FEATURE_DMNR;
    bool needUpdate = false;
    if (dynamic_mask_type == SPH_ENH_DYNAMIC_MASK_DMNR) {
        speechFeature = SPEECH_FEATURE_DMNR;
        needUpdate = true;
    } else if (dynamic_mask_type == SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) {
        speechFeature = SPEECH_FEATURE_LSPK_DMNR;
        needUpdate = true;
    }
    if (needUpdate) {
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->updateSpeechFeature(speechFeature, new_flag_on);
    }
#endif

    return SetSpeechEnhancementMaskToAllModem(mask);
}

void SpeechEnhancementController::SetMagicConferenceCallOn(const bool magic_conference_call_on) {
    ALOGD("%s(), mMagicConferenceCallOn = %d, new magic_conference_call_on = %d",
          __FUNCTION__, mMagicConferenceCallOn, magic_conference_call_on);

    //always set
    mMagicConferenceCallOn = magic_conference_call_on;

    SetDynamicMaskOnToAllModem(SPH_ENH_DYNAMIC_MASK_LSPK_DMNR, mMagicConferenceCallOn);

}

void SpeechEnhancementController::SetHACOn(const bool hac_on) {
    ALOGD("%s(), hac_on = %d, new hac_on = %d",
          __FUNCTION__, mHACOn, hac_on);
    property_set(PROPERTY_KEY_HAC_ON, (hac_on == false) ? "0" : "1");

    mHACOn = hac_on;

}

void SpeechEnhancementController::SetBtHeadsetNrecOnToAllModem(const bool bt_headset_nrec_on) {
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    property_set(PROPERTY_KEY_BT_HEADSET_NREC_ON, (bt_headset_nrec_on == false) ? "0" : "1");
    if (mBtHeadsetNrecOn == bt_headset_nrec_on) {
        ALOGD("%s(), mBtHeadsetNrecOn(%d) status keeps the same, skip.", __FUNCTION__, mBtHeadsetNrecOn);
    } else {
        mBtHeadsetNrecOn = bt_headset_nrec_on;
        for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
            pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
            if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
                pSpeechDriver->SetBtHeadsetNrecOn(mBtHeadsetNrecOn);
            }
        }
    }
}

bool  SpeechEnhancementController::GetBtHeadsetNrecOn(void) {
    ALOGD("%s(), mBtHeadsetNrecOn = %d", __FUNCTION__, mBtHeadsetNrecOn);
    return mBtHeadsetNrecOn;
}

void SpeechEnhancementController::SetSMNROn(void) {
    mSMNROn = true;
    ALOGD("%s(), mSMNROn = %d", __FUNCTION__, mSMNROn);
}

}

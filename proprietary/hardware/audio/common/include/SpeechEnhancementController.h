#ifndef ANDROID_SPEECH_ENHANCEMENT_CONTROLLER_H
#define ANDROID_SPEECH_ENHANCEMENT_CONTROLLER_H

#include "AudioType.h"
#include "SpeechType.h"

#include "AudioCustParamClient.h"

namespace android {
class SpeechEnhancementController {
public:
    static SpeechEnhancementController *GetInstance();

    virtual ~SpeechEnhancementController();

    /**
     * speech enhancement parameters setting
     */
    status_t SetNBSpeechParametersToAllModem(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB);
    status_t SetDualMicSpeechParametersToAllModem(const AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic);
    //#if defined(MTK_WB_SPEECH_SUPPORT)
    status_t SetWBSpeechParametersToAllModem(const AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB);
    //#endif
    status_t SetMagiConSpeechParametersToAllModem(const AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon);
    status_t SetHACSpeechParametersToAllModem(const AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC);

    /**
     * speech enhancement functions on/off
     */
    sph_enh_mask_struct_t GetSpeechEnhancementMask() const { return mSpeechEnhancementMask; }
    status_t SetSpeechEnhancementMaskToAllModem(const sph_enh_mask_struct_t &mask);

    bool GetDynamicMask(const sph_enh_dynamic_mask_t dynamic_mask_type) const { return ((mSpeechEnhancementMask.dynamic_func & dynamic_mask_type) > 0); }
    status_t SetDynamicMaskOnToAllModem(const sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on);


    bool GetMagicConferenceCallOn() const { return mMagicConferenceCallOn; }
    void SetMagicConferenceCallOn(const bool magic_conference_call_on);

    bool GetHACOn() const { return mHACOn; }
    void SetHACOn(const bool hac_on);

    status_t SetBtHeadsetNrecOnMaskOn(sph_enh_mask_struct_t *ori_mask, const sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on);
    void SetBtHeadsetNrecOnToAllModem(const bool bt_headset_nrec_on);
    void SetSMNROn(void);
    status_t SetNBSpeechLpbkParametersToAllModem(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB, AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT *pSphParamNBLpbk);
    bool GetBtHeadsetNrecOn(void);

private:
    SpeechEnhancementController();
    static SpeechEnhancementController *mSpeechEnhancementController; // singleton

    sph_enh_mask_struct_t mSpeechEnhancementMask;

    bool mMagicConferenceCallOn;
    bool mHACOn;

    bool mBtHeadsetNrecOn;
    bool mSMNROn;
};


} // end namespace android

#endif // end of ANDROID_SPEECH_ENHANCEMENT_CONTROLLER_H

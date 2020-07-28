#ifndef AUDIO_SPEECH_ENHANCE_INFO_H
#define AUDIO_SPEECH_ENHANCE_INFO_H

//#include "AudioUtility.h"
#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <utils/TypeHelpers.h>
#include <utils/Vector.h>
#include <utils/String16.h>

#include "SpeechType.h"
#include "AudioType.h"
#include <AudioLock.h>

#include "AudioCustParamClient.h"

namespace android {

enum TOOL_TUNING_MODE {
    TUNING_MODE_NONE            = 0,
    NORMAL_MODE_DMNR            = 1,
    HANDSFREE_MODE_DMNR           = 2
};


class AudioSpeechEnhanceInfo {
public:

    static AudioSpeechEnhanceInfo *getInstance();
    AudioSpeechEnhanceInfo();
    ~AudioSpeechEnhanceInfo();

    //BesRecord Preprocess +++
    void SetBesRecScene(int32_t BesRecScene);
    int32_t GetBesRecScene(void);
    void ResetBesRecScene(void);
    //BesRecord Preprocess ---

    //get the MMI switch info
    void UpdateDynamicSpeechEnhancementMask(const voip_sph_enh_mask_struct_t &mask);
    status_t SetDynamicVoIPSpeechEnhancementMask(const voip_sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on);
    voip_sph_enh_mask_struct_t GetDynamicVoIPSpeechEnhancementMask() const { return mVoIPSpeechEnhancementMask; }

    //Engineer mode enable MagiASR+++
    status_t GetForceMagiASRState();
    bool SetForceMagiASR(bool enable);
    //Engineer mode MagiASR---

    //Engineer mode enable AECRecord+
    bool GetForceAECRecState();
    bool SetForceAECRec(bool enable);
    //Engineer mode enable AECRecord-

    //----------------Audio tunning +++ --------------------------------
    //----------------for AP DMNR tunning --------------------------------
    void SetAPDMNRTuningEnable(bool bEnable);
    bool IsAPDMNRTuningEnable(void);
    bool SetAPTuningMode(const TOOL_TUNING_MODE mode);
    int GetAPTuningMode(void);
    //----------------for HDRec tunning --------------------------------
    void SetBesRecTuningEnable(bool bEnable);
    bool IsBesRecTuningEnable(void);

    status_t SetBesRecVMFileName(const char *fileName);
    void GetBesRecVMFileName(char *VMFileName, size_t string_size);
    //----------------Audio tunning --- --------------------------------
#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
    void PreLoadBesRecordParams(void);
    void UpdateBesRecordParams(void);
    void GetPreLoadBesRecordSceneTable(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara);
    void GetPreLoadBesRecordParam(AUDIO_HD_RECORD_PARAM_STRUCT *pPara);
    void GetPreLoadAudioVoIPParam(AUDIO_VOIP_PARAM_STRUCT *pPara);
    void GetPreLoadDualMicSpeechParam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic);
#endif
    //----------------for Hifi Record --- --------------------------------
    void SetHifiRecord(bool bEnable);
    bool GetHifiRecord(void);

    //----------------for debug --- --------------------------------
    bool GetDebugStatus(void);

private:

    /**
      * singleton pattern
      */
    static AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfo;

    /**
     * AudioSpeechEnhanceInfo lock
     */
    AudioLock mLock;

    //BesRecord Preprocess +++
    int32_t mBesRecScene;

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
    AUDIO_HD_RECORD_SCENE_TABLE_STRUCT mPreLoadBesRecordSceneTable;
    AUDIO_HD_RECORD_PARAM_STRUCT mPreLoadBesRecordParam;
    AUDIO_VOIP_PARAM_STRUCT mPreLoadVOIPParam;
    AUDIO_CUSTOM_EXTRA_PARAM_STRUCT mPreLoadDMNRParam;
#endif
    //BesRecord Preprocess ---

    //for BesRec tuning
    bool mBesRecTuningEnable;
    char mVMFileName[VM_FILE_NAME_LEN_MAX];
    //for AP DMNR tunning
    bool mAPDMNRTuningEnable;
    int mAPTuningMode;

    //Tina todo
    //        KeyedVector<AudioALSAStreamIn *, SPELayer *> mSPELayerVector; // vector to save current recording client
    voip_sph_enh_mask_struct_t mVoIPSpeechEnhancementMask;

    //Engineer mode enable MagiASR+++
    bool mForceMagiASR;
    //Engineer mode MagiASR---

    //Engineer mode enable AECRecord+
    bool mForceAECRec;
    //Engineer mode enable AECRecord-

    //for Normal record with high sample rate (>48K)
    bool mHiFiRecordEnable;

    //for debug purpose, temp using
    bool mDebugflag;

    /**
     * AudioCustParamClient
     */
    AudioCustParamClient *mAudioCustParamClient;
};

}

#endif

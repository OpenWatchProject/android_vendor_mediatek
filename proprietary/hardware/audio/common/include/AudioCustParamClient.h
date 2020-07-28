#ifndef _AUDIO_CUST_PARAM_CLIENT_H_
#define _AUDIO_CUST_PARAM_CLIENT_H_
#include <dlfcn.h>
#include "AudioUtility.h"
#include <AudioLock.h>

#include <utils/Log.h>
#include <utils/String8.h>
#include "AudioCustParam.h"

namespace android {

struct AudioDeviceParam {
    int numMicSupport;
};

class AudioCustParamClient {
public:
    virtual ~AudioCustParamClient();

    static AudioCustParamClient *GetInstance(void);


    uint32_t (*QueryFeatureSupportInfo)(void);
    // NB speech parameters
    int (*GetNBSpeechParamFromNVRam)(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB);
    int (*SetNBSpeechParamToNVRam)(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB);

    // Dual mic speech parameters
    int (*GetDualMicSpeechParamFromNVRam)(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic);
    int (*SetDualMicSpeechParamToNVRam)(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic);

    // WB speech parameters
    int (*GetWBSpeechParamFromNVRam)(AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB);
    int (*SetWBSpeechParamToNVRam)(AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB);

    // Med param parameter
    int (*GetMedParamFromNV)(AUDIO_PARAM_MED_STRUCT *pPara);
    int (*SetMedParamToNV)(AUDIO_PARAM_MED_STRUCT *pPara);

    // VolumeVer1 parameter
    int (*GetVolumeVer1ParamFromNV)(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *pPara);
    int (*SetVolumeVer1ParamToNV)(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *pPara);

    // Audio Custom Paramete
    int (*GetAudioCustomParamFromNV)(AUDIO_VOLUME_CUSTOM_STRUCT *pPara);
    int (*SetAudioCustomParamToNV)(AUDIO_VOLUME_CUSTOM_STRUCT *pPara);

    // AudioGainTable Parameter
    int (*GetAudioGainTableParamFromNV)(AUDIO_GAIN_TABLE_STRUCT *pPara);
    int (*SetAudioGainTableParamToNV)(AUDIO_GAIN_TABLE_STRUCT *pPara);

    // Audio HD record parameters
    int (*GetHdRecordParamFromNV)(AUDIO_HD_RECORD_PARAM_STRUCT *pPara);
    int (*SetHdRecordParamToNV)(AUDIO_HD_RECORD_PARAM_STRUCT *pPara);

    //  Audio HD record scene table
    int (*GetHdRecordSceneTableFromNV)(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara);
    int (*SetHdRecordSceneTableToNV)(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara);

    // Audio VoIP Parameters
    int (*GetAudioVoIPParamFromNV)(AUDIO_VOIP_PARAM_STRUCT *pPara);
    int (*SetAudioVoIPParamToNV)(AUDIO_VOIP_PARAM_STRUCT *pPara);

    // Audio HFP Parameters
    int (*GetAudioHFPParamFromNV)(AUDIO_HFP_PARAM_STRUCT *pPara);
    int (*SetAudioHFPParamToNV)(AUDIO_HFP_PARAM_STRUCT *pPara);

    // MagiConference Call parameters
    int (*GetMagiConSpeechParamFromNVRam)(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon);
    int (*SetMagiConSpeechParamToNVRam)(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *pSphParamMagiCon);

    // HAC parameters
    int (*GetHACSpeechParamFromNVRam)(AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC);
    int (*SetHACSpeechParamToNVRam)(AUDIO_CUSTOM_HAC_PARAM_STRUCT *pSphParamHAC);

    // Speech Loopback parameters
    int (*GetNBSpeechLpbkParamFromNVRam)(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT *pSphParamNBLpbk);
    int (*SetNBSpeechLpbkParamToNVRam)(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT *pSphParamNBLpbk);

    // BT Gain parameter
    int (*GetAudioBTGainParamFromNV)(AUDIO_BT_GAIN_STRUCT *pParaBT);
    int (*SetAudioBTGainParamToNV)(AUDIO_BT_GAIN_STRUCT *pParaBT);

    // param query
    int getNumMicSupport(void);

protected:
    AudioCustParamClient();

    AudioLock mMutex;

private:
    void init(void);
    void deinit(void);

    void initParam(void);
    static AudioCustParamClient *mAudioCustParamClient; // singleton

    void          *handle;
    short acpOpsInited;

    // param
    struct AudioDeviceParam mDeviceParam;

    // param function pointer
    int (*mGetNumMicSupport)(void);
};

}

#endif

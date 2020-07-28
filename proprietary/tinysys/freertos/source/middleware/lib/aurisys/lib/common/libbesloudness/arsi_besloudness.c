#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <arsi_api.h> // implement MTK AURISYS API
#include <wrapped_audio.h>

#include "arsi_besloudness.h"
#include <BesLoudness_HD_exp.h>



static debug_log_fp_t print_log;

static audio_devices_t mIputDeviceOld;
static audio_devices_t mOutputDeviceOld;


//-----------------------------------------------------------
// private function
//-----------------------------------------------------------

static audioloud_lib_handle_t *lib_param_init(char *pbuf, const int32_t mFilterType, uint32_t mPcmFormat);

static int copy_param(audioloud_lib_handle_t *user, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioparam);

status_t audioloud_arsi_set_key_value_pair(
    const string_buf_t *key_value_pair,
    void               *p_handler);
static AudioCompFltType_t convert_flt_by_stream_type(audio_stream_type_t stream_type, bool is_drc, audio_devices_t output_devices);
//-----------------------------------------------------------


audioloud_lib_handle_t *lib_param_init(char *pbuf, const int32_t mFilterType, uint32_t mPcmFormat) {

    char *ptmpbuf = pbuf;
    audioloud_lib_handle_t *pUser = (audioloud_lib_handle_t *)(ptmpbuf);
    ptmpbuf += sizeof(audioloud_lib_handle_t);
    memset(&pUser->mBloudHandle, 0, sizeof(BS_HD_Handle));
    memset(&pUser->mInitParam, 0, sizeof(BLOUD_HD_InitParam));
    memset(&pUser->mParamFormatUse, 0,
           sizeof(BLOUD_HD_IIR_Design_Param));

    BLOUD_HD_InitParam *mInitParam = &pUser->mInitParam;
    mInitParam->pMode_Param = (BLOUD_HD_ModeParam *)ptmpbuf;
    memset(mInitParam->pMode_Param, 0, sizeof(BLOUD_HD_ModeParam));

    ptmpbuf += sizeof(BLOUD_HD_ModeParam);
    mInitParam->pMode_Param->pFilter_Coef_L = (BLOUD_HD_FilterCoef *)ptmpbuf;
    memset(mInitParam->pMode_Param->pFilter_Coef_L, 0, sizeof(BLOUD_HD_FilterCoef));

    ptmpbuf += sizeof(BLOUD_HD_FilterCoef);
    mInitParam->pMode_Param->pFilter_Coef_R = (BLOUD_HD_FilterCoef *)ptmpbuf;
    memset(mInitParam->pMode_Param->pFilter_Coef_R, 0, sizeof(BLOUD_HD_FilterCoef));

    ptmpbuf += sizeof(BLOUD_HD_FilterCoef);
    mInitParam->pMode_Param->pCustom_Param = (BLOUD_HD_CustomParam *)ptmpbuf;
    memset(mInitParam->pMode_Param->pCustom_Param, 0, sizeof(BLOUD_HD_CustomParam));

    ptmpbuf += sizeof(BLOUD_HD_CustomParam);
    /* To get size info*/
    BLOUD_HD_SetHandle(&pUser->mBloudHandle);

    pUser->mBloudHandle.GetBufferSize(&pUser->mLibWorkingBuf.mInternalBufSize, &pUser->mLibWorkingBuf.mTempBufSize, mPcmFormat);

    pUser->mLibWorkingBuf.mpInternalBuf = (char *)ptmpbuf;
    ptmpbuf += pUser->mLibWorkingBuf.mInternalBufSize;
    pUser->mLibWorkingBuf.mpTempBuf = (char *)ptmpbuf;



    pUser->id = (uint32_t)pUser;                  /* SHOULD BE uniq ID */
    pUser->mFilterType = mFilterType;
    pUser->bIsEnhaceOn = true;
    pUser->mState = BLOUD_STATE_INIT;

    return pUser;
}

status_t lib_switch_drcparam(audioloud_lib_handle_t *pUser) {

    int mCheckStatus = 0;

    if (pUser == NULL) {
        return BAD_VALUE;
    }

    AUDIO_ACF_CUSTOM_PARAM_STRUCT acf_temp;
    memset(&acf_temp, 0, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    //copy ACF param
    if (pUser->mFilterType == AUDIO_COMP_FLT_AUDIO) {
        memcpy((void *)&acf_temp.bes_loudness_f_param.V5F,
               (void *)&pUser->mCachedAudioParam.bes_loudness_f_param.V5F, sizeof(AUDIO_ACF_V5F_PARAM));
        acf_temp.bes_loudness_Sep_LR_Filter = pUser->mCachedAudioParam.bes_loudness_Sep_LR_Filter;

        mCheckStatus = copy_param(pUser, &acf_temp);
    } else {
        //ACF+DRC
        mCheckStatus = copy_param(pUser, &pUser->mCachedAudioParam);
    }

    if (mCheckStatus != 0) {
        return BAD_VALUE;
    } else {
        return NO_ERROR;
    }


}

AudioCompFltType_t get_filter_bydevice(const arsi_task_config_t *p_arsi_task_config, uint32_t drc_enable) {
    AudioCompFltType_t filtertype = AUDIO_COMP_FLT_AUDIO;
    AudioCompFltType_t acf_filtertype = filtertype;
    audio_devices_t output_devices = p_arsi_task_config->output_device_info.devices;
    uint32_t hw_info_mask = p_arsi_task_config->output_device_info.hw_info_mask;

    //ALOGD("Liang: get_filter_bydevice : output_devices = %x mAudioMode = %d",output_devices,mAudioMode);
    if ((output_devices & AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
        (output_devices & AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
        filtertype = AUDIO_COMP_FLT_HEADPHONE;
    } else if (output_devices & AUDIO_DEVICE_OUT_USB_DEVICE) {
        filtertype = AUDIO_COMP_FLT_HCF_FOR_USB;
    }

    if (output_devices & AUDIO_DEVICE_OUT_SPEAKER) {
        acf_filtertype = convert_flt_by_stream_type(p_arsi_task_config->stream_type, false, output_devices);
        if (drc_enable) {   // DRC enable
            filtertype = convert_flt_by_stream_type(p_arsi_task_config->stream_type, true, output_devices);
        } else {
            filtertype = acf_filtertype;
        }
    } else if (output_devices & AUDIO_DEVICE_OUT_EARPIECE) {
        if ((hw_info_mask & OUTPUT_DEVICE_HW_INFO_VIBRATION_SPEAKER) &&
            (hw_info_mask & OUTPUT_DEVICE_HW_INFO_2IN1_SPEAKER)) {
            filtertype = AUDIO_COMP_FLT_VIBSPK;
        }
    }
    return filtertype;
}


status_t set_lib_parameter(uint32_t paramID, void *param,
                           audioloud_lib_handle_t *pUser) {
    uint32_t Curparam = (uint32_t)((long)param);
    int32_t ret = 0;
    uint32_t task_flag = pUser->mScenario;
    //print_log("+%s(), paramID %d, param %d\n", __FUNCTION__, paramID, Curparam);

    //Add constraint to limit the use after open.
    switch (paramID) {
    case BLOUD_PAR_SET_FILTER_TYPE: {
        pUser->mFilterType = Curparam;
        break;
    }
    case BLOUD_PAR_SET_WORK_MODE: {
        switch (Curparam) {
        case AUDIO_CMP_FLT_LOUDNESS_BASIC:     // basic Loudness mode
            pUser->mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOUD_FLT;
            pUser->mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_BASIC;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_ENHANCED:     // enhancement(1) Loudness mode
            pUser->mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOUD_FLT;
            pUser->mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_ENHANCED;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_AGGRESSIVE:     // enhancement(2) Loudness mode
            pUser->mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOUD_FLT;
            pUser->mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_AGGRESSIVE;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_LITE:     // Only DRC, no filtering
            pUser->mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_NONE;
            pUser->mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_BASIC;
            if (task_flag & AUDIO_OUTPUT_FLAG_FAST) {
                pUser->mInitParam.Frame_Length = BLOCK_SIZE_LOW_LATENCY;
            }
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP:     // Audio Compensation Filter mode (No DRC)
            pUser->mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_COMP_FLT;
            pUser->mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
            if (task_flag & AUDIO_OUTPUT_FLAG_FAST) {
                pUser->mInitParam.Frame_Length = BLOCK_SIZE_LOW_LATENCY;
            }
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_BASIC:     // Audio Compensation Filter mode + DRC
            pUser->mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_COMP_FLT;
            pUser->mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_BASIC;
            if (task_flag & AUDIO_OUTPUT_FLAG_FAST) {
                pUser->mInitParam.Frame_Length = BLOCK_SIZE_LOW_LATENCY;
            }
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_HEADPHONE:     //HCF
            pUser->mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_COMP_HDP;
            pUser->mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_AUDENH:
            pUser->mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_AUD_ENH;
            pUser->mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_LOW_LATENCY_WITHOUT_LIMITER:
            pUser->mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOW_LATENCY_ACF;
            pUser->mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
            pUser->mInitParam.pMode_Param->pCustom_Param->Disable_Limiter = 1;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_LOW_LATENCY_WITH_LIMITER:
            pUser->mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOW_LATENCY_ACF;
            pUser->mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
            pUser->mInitParam.pMode_Param->pCustom_Param->Disable_Limiter = 0;
            break;
        default:
            if (print_log) {
                print_log("%s() invalide workmode %d\n", __FUNCTION__, Curparam);
            }
            break;
        }
        break;
    }
    case BLOUD_PAR_SET_SAMPLE_RATE: {
        pUser->mInitParam.Sampling_Rate = Curparam;
        break;
    }
    case BLOUD_PAR_SET_PCM_FORMAT: {
        pUser->mInitParam.PCM_Format = Curparam;
        break;
    }
    case BLOUD_PAR_SET_CHANNEL_NUMBER: {
        uint32_t chNum = Curparam;
        if (chNum > 0 && chNum < 3) {
            // chnum should be 1 or 2
            pUser->mInitParam.Channel = chNum;
        } else {
            ret = -1;
        }
        break;
    }
    case BLOUD_PAR_SET_STEREO_TO_MONO_MODE: {
        pUser->mInitParam.pMode_Param->S2M_Mode = Curparam;
        if (pUser->mState == BLOUD_STATE_PROCESS) {
            BLOUD_HD_RuntimeParam runtime_param;
            runtime_param.Command = BLOUD_HD_CHANGE_MODE;
            runtime_param.pMode_Param = pUser->mInitParam.pMode_Param;
            ret = pUser->mBloudHandle.SetParameters(&pUser->mBloudHandle,
                                                    &runtime_param);
        }
        break;
    }
    case BLOUD_PAR_SET_UPDATE_PARAM_TO_SWIP: {
        if (pUser->mState == BLOUD_STATE_OPEN) {
            // reopen lib before processing
            ret = pUser->mBloudHandle.Open(&pUser->mBloudHandle,
                                           pUser->mLibWorkingBuf.mpInternalBuf,
                                           (const void *)&pUser->mInitParam);
        } else if (pUser->mState == BLOUD_STATE_PROCESS) {
            BLOUD_HD_RuntimeParam runtime_param;
            runtime_param.Command = BLOUD_HD_CHANGE_MODE;
            runtime_param.pMode_Param = (BLOUD_HD_ModeParam *)
                                        pUser->mInitParam.pMode_Param;
            ret = pUser->mBloudHandle.SetParameters(&pUser->mBloudHandle,
                                                    &runtime_param);
        } else {
            ret = -1;
        }
        if (print_log) {
            print_log("%s runtime BLOUD_HD_CHANGE_MODE, FLT Mode = %d Loudness Mode = %d", __FUNCTION__,
                      pUser->mInitParam.pMode_Param->Filter_Mode, pUser->mInitParam.pMode_Param->Loudness_Mode);
        }
        break;
    }
    case BLOUD_PAR_SET_RAMP_UP: {
        if (Curparam == 0) {
            pUser->mInitParam.Initial_State = BLOUD_HD_NORMAL_STATE;
        } else {
            pUser->mInitParam.Initial_State = BLOUD_HD_BYPASS_STATE;
        }

        // print_log("%s %d mInitParam.Initial_State %d", __FUNCTION__, __LINE__,
        //       pUser->mInitParam.Initial_State);
        break;
    }
    case BLOUD_PAR_SET_CHANGE_TO_BYPASS: {
        if (pUser->mState >= BLOUD_STATE_OPEN) {
            //BLOUD_HD_RuntimeStatus runtime_status;
            BLOUD_HD_RuntimeParam runtime_param;
            runtime_param.Command = BLOUD_HD_TO_BYPASS_STATE;
            ret = pUser->mBloudHandle.SetParameters(&pUser->mBloudHandle,
                                                    &runtime_param);
            if (print_log) {
                print_log("Change to Bypass! ret = %d", ret);
            }
        }
        break;
    }
    case BLOUD_PAR_SET_CHANGE_TO_NORMAL: {
        if (pUser->mState >= BLOUD_STATE_OPEN) {
            //BLOUD_HD_RuntimeStatus runtime_status;
            BLOUD_HD_RuntimeParam runtime_param;
            runtime_param.Command = BLOUD_HD_TO_NORMAL_STATE;
            ret = pUser->mBloudHandle.SetParameters(&pUser->mBloudHandle,
                                                    &runtime_param);
            if (print_log) {
                print_log("Change to Normal! ret = %d", ret);
            }
        }
        break;
    }
    default:
        //print_log("-%s() Error\n", __FUNCTION__);
        ret = -1;
    }

    if (ret != 0) {
        ret = BAD_VALUE;
    } else {
        ret = NO_ERROR;
    }

    return ret;
}

int convert_filter_type(audioloud_lib_handle_t *pUser) {
    AudioComFltMode_t AudioFltMode = AUDIO_CMP_FLT_LOUDNESS_COMP;
    AudioCompFltType_t filtertype = pUser->mFilterType;
    bool bypass_acf = (pUser->mBypassFlag & FILTER_BYPASS_ACF) ? true : false;

    switch (filtertype) {
    case AUDIO_COMP_FLT_AUDIO :
    case AUDIO_COMP_FLT_AUDIO_FOR_RINGTONE :
        AudioFltMode = AUDIO_CMP_FLT_LOUDNESS_COMP;
        break;
    case AUDIO_COMP_FLT_HEADPHONE :
    case AUDIO_COMP_FLT_HCF_FOR_USB :
        AudioFltMode = AUDIO_CMP_FLT_LOUDNESS_COMP_HEADPHONE;
        break;
    case AUDIO_COMP_FLT_AUDENH :
        AudioFltMode = AUDIO_CMP_FLT_LOUDNESS_COMP_AUDENH;
        break;
    case AUDIO_COMP_FLT_VIBSPK :
        AudioFltMode = AUDIO_CMP_FLT_LOUDNESS_COMP;
        break;
    case AUDIO_COMP_FLT_DRC_FOR_MUSIC :
    case AUDIO_COMP_FLT_DRC_FOR_RINGTONE:
        if (bypass_acf) {
            AudioFltMode = AUDIO_CMP_FLT_LOUDNESS_LITE;    // only Apply DRC
        } else {
            AudioFltMode = AUDIO_CMP_FLT_LOUDNESS_COMP_BASIC;    // Apply ACF+DRC
        }
        break;
    default:
        break;
    }
    return AudioFltMode;
}

AudioCompFltType_t convert_flt_by_stream_type(audio_stream_type_t stream_type, bool is_drc, audio_devices_t output_devices) {
    AudioCompFltType_t AudioFltType = AUDIO_COMP_FLT_HEADPHONE;

    if (output_devices & AUDIO_DEVICE_OUT_SPEAKER) {
        if (stream_type == AUDIO_STREAM_RING) {
            if (is_drc) {
                AudioFltType = AUDIO_COMP_FLT_DRC_FOR_RINGTONE;
            } else {
                AudioFltType = AUDIO_COMP_FLT_AUDIO_FOR_RINGTONE;
            }
        } else {
            if (is_drc) {
                AudioFltType = AUDIO_COMP_FLT_DRC_FOR_MUSIC;
            } else {
                AudioFltType = AUDIO_COMP_FLT_AUDIO;
            }
        }
    }
    if (print_log) {
        print_log("%s() stream_type = %d, AudioFltType = %d\n", __FUNCTION__, stream_type, AudioFltType);
    }
    return AudioFltType;
}
int get_samplingrate_index(unsigned int sampling_rate) {
    int sr_idx;

    switch (sampling_rate) {
    case 48000:
        sr_idx = 0;
        break;
    case 44100:
        sr_idx = 1;
        break;
    case 32000:
        sr_idx = 2;
        break;
    case 24000:
        sr_idx = 3;
        break;
    case 22050:
        sr_idx = 4;
        break;
    case 16000:
        sr_idx = 5;
        break;
    case 12000:
        sr_idx = 6;
        break;
    case 11025:
        sr_idx = 7;
        break;
    case  8000:
        sr_idx = 8;
        break;
    default:
        sr_idx = -1;
        break;
    }

    return sr_idx;
}

// shoud parse content in AP side

static int copy_param(audioloud_lib_handle_t *user, AUDIO_ACF_CUSTOM_PARAM_STRUCT *audioparam) { //ony use for v5
    bool ZeroFlag = true;
    bool bIsZeroCoeff;
    bool mIsSepLR_Filter;
    int mCheckStatus = 0, i;

    BLOUD_HD_InitParam mInitParam = user->mInitParam;
    AUDIO_ACF_CUSTOM_PARAM_STRUCT mCachedAudioParam = *audioparam;//user->mCachedAudioParam;
    uint32_t mFilterType = user->mFilterType;
    BLOUD_HD_IIR_Design_Param mParamFormatUse = user->mParamFormatUse;

    mInitParam.pMode_Param->pCustom_Param->WS_Gain_Max =
        mCachedAudioParam.bes_loudness_WS_Gain_Max;
    mInitParam.pMode_Param->pCustom_Param->WS_Gain_Min =
        mCachedAudioParam.bes_loudness_WS_Gain_Min;
    mInitParam.pMode_Param->pCustom_Param->Filter_First =
        mCachedAudioParam.bes_loudness_Filter_First;
    mInitParam.pMode_Param->pCustom_Param->Num_Bands =
        mCachedAudioParam.bes_loudness_Num_Bands;
    mInitParam.pMode_Param->pCustom_Param->Flt_Bank_Order =
        mCachedAudioParam.bes_loudness_Flt_Bank_Order;
    mInitParam.pMode_Param->pCustom_Param->DRC_Delay = mCachedAudioParam.DRC_Delay;
    mInitParam.pMode_Param->pCustom_Param->Lim_Th = mCachedAudioParam.Lim_Th;
    mInitParam.pMode_Param->pCustom_Param->Lim_Gn = mCachedAudioParam.Lim_Gn;
    mInitParam.pMode_Param->pCustom_Param->Lim_Const = mCachedAudioParam.Lim_Const;
    mInitParam.pMode_Param->pCustom_Param->Lim_Delay = mCachedAudioParam.Lim_Delay;
    mInitParam.pMode_Param->pCustom_Param->Sep_LR_Filter = mIsSepLR_Filter =
                                                               mCachedAudioParam.bes_loudness_Sep_LR_Filter;
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->Att_Time,
           (void *)mCachedAudioParam.Att_Time, 48 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->Rel_Time,
           (void *)mCachedAudioParam.Rel_Time, 48 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->Cross_Freq,
           (void *)mCachedAudioParam.bes_loudness_Cross_Freq, 7 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->SB_Mode,
           (void *)mCachedAudioParam.SB_Mode, 8 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->SB_Gn,
           (void *)mCachedAudioParam.SB_Gn, 8 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->Hyst_Th,
           (void *)mCachedAudioParam.Hyst_Th, 48 * sizeof(int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->DRC_Th,
           (void *)mCachedAudioParam.DRC_Th, 40 * sizeof(int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->DRC_Gn,
           (void *)mCachedAudioParam.DRC_Gn, 40 * sizeof(int));

    if (mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_hpf_fc != 0
        || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_hpf_order != 0
        || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_lpf_fc != 0
        || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_lpf_order != 0
        || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_hpf_fc != 0
        || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_hpf_order != 0
        || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_lpf_fc != 0
        || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_lpf_order != 0
       ) {
        ZeroFlag = false;
    }

    if (ZeroFlag) {
        for (i = 0; i < 8; i++) {
            if (mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_fc[i] != 0
                || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_bw[i] != 0
                || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_gain[i] != 0
                || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_fc[i] != 0
                || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_bw[i] != 0
                || mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_gain[i] != 0
               ) {
                ZeroFlag = false;
                break;
            }
        }
    }

    bIsZeroCoeff = ZeroFlag;

    if (mFilterType == AUDIO_COMP_FLT_VIBSPK) {
        unsigned int bes_loudness_bpf_coeff[8][6][3] = {{{0}}};

        memset(mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF, 0,
               8 * 6 * sizeof(unsigned int));
        memcpy((void *)bes_loudness_bpf_coeff,
               (void *)mCachedAudioParam.bes_loudness_f_param.V5ViVSPK.bes_loudness_bpf_coeff,
               36 * sizeof(unsigned int));

        int sr_idx = get_samplingrate_index(mInitParam.Sampling_Rate);

        if (sr_idx >= 0 && sr_idx < 6) {
            int flt_idx;
            int i = 0;
            for (flt_idx = 0; flt_idx < 8; flt_idx++) {
                memset(mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[flt_idx], 0,
                       sizeof(unsigned int) * 6);
                memcpy(mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[flt_idx],
                       bes_loudness_bpf_coeff[flt_idx][sr_idx], sizeof(unsigned int) * 3);
                memset(mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[flt_idx], 0,
                       sizeof(unsigned int) * 6);
                memcpy(mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[flt_idx],
                       bes_loudness_bpf_coeff[flt_idx][sr_idx], sizeof(unsigned int) * 3);
                i++;
            }
        }
    } else {
        mParamFormatUse.hpf_fc =
            mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_hpf_fc;
        mParamFormatUse.hpf_order =
            mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_hpf_order;
        mParamFormatUse.lpf_fc =
            mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_lpf_fc;
        mParamFormatUse.lpf_order =
            mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_lpf_order;
        memcpy((void *)mParamFormatUse.bpf_fc,
               (void *)mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_fc,
               8 * sizeof(unsigned int));
        if (print_log) {
            print_log("hpf_fc = %d order = %d", mParamFormatUse.hpf_fc, mParamFormatUse.hpf_order);
            print_log("%s Filter_Mode %d, Loudness_Mode %d", __FUNCTION__,
                      mInitParam.pMode_Param->Filter_Mode, mInitParam.pMode_Param->Loudness_Mode);
            int i = 0;
            for (i = 0; i < 8; i++) {
                print_log("mParamFormatUse.bpf_fc : 0x%08X, ", mParamFormatUse.bpf_fc[i]);
            }
        }
        memcpy((void *)mParamFormatUse.bpf_bw,
               (void *)mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_bw,
               8 * sizeof(unsigned int));
        memcpy((void *)mParamFormatUse.bpf_gain,
               (void *)mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_gain,
               8 * sizeof(int));
        mCheckStatus = BLOUD_HD_Filter_Design(mInitParam.pMode_Param->Filter_Mode,
                                              mInitParam.Sampling_Rate, &mParamFormatUse,
                                              mInitParam.pMode_Param->pFilter_Coef_L);

        mParamFormatUse.hpf_fc =
            mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_hpf_fc;
        mParamFormatUse.hpf_order =
            mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_hpf_order;
        mParamFormatUse.lpf_fc =
            mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_lpf_fc;
        mParamFormatUse.lpf_order =
            mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_lpf_order;
        memcpy((void *)mParamFormatUse.bpf_fc,
               (void *)mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_fc,
               8 * sizeof(unsigned int));
        memcpy((void *)mParamFormatUse.bpf_bw,
               (void *)mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_bw,
               8 * sizeof(unsigned int));
        memcpy((void *)mParamFormatUse.bpf_gain,
               (void *)mCachedAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_gain,
               8 * sizeof(int));

        mCheckStatus = BLOUD_HD_Filter_Design(mInitParam.pMode_Param->Filter_Mode,
                                              mInitParam.Sampling_Rate, &mParamFormatUse,
                                              mInitParam.pMode_Param->pFilter_Coef_R);
    }

    if (mIsSepLR_Filter == 0) {
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF, 0,
               10 * sizeof(unsigned int));
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF, 0,
               48 * sizeof(unsigned int));
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_R->LPF_COEF, 0,
               3 * sizeof(unsigned int));
    }

    if (mInitParam.pMode_Param->pCustom_Param->Disable_Limiter) {
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF, 0,
               48 * sizeof(unsigned int));
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_L->LPF_COEF, 0,
               3 * sizeof(unsigned int));
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF, 0,
               48 * sizeof(unsigned int));
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_R->LPF_COEF, 0,
               3 * sizeof(unsigned int));
    }
    if (print_log) {
        print_log("copy_param mIsSepLR_Filter [%d]", mIsSepLR_Filter);
        for (i = 0; i < 5; i++) {
            print_log("DRC_Th[0][%d] = %d", i,
                      mInitParam.pMode_Param->pCustom_Param->DRC_Th[0][i]);
            print_log("DRC_Gn[0][%d] = %d", i,
                      mInitParam.pMode_Param->pCustom_Param->DRC_Gn[0][i]);
        }
    }
    return mCheckStatus;
}



status_t audioloud_arsi_query_working_buf_size(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    uint32_t                 *p_working_buf_size,
    const debug_log_fp_t      debug_log_fp) {
    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_working_buf_size == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }
    uint32_t mPcmFormat;
    uint32_t mTempBufSize;
    uint32_t mInternalBufSize;
    uint32_t mCoefParamBufSize;
    uint32_t mCustomParamBufSize;
    uint32_t mLibHandlerSize;
    uint32_t mModeParamSize;
    /* for callback query buffer API */
    BS_HD_Handle mBloudHandle;
    BLOUD_HD_SetHandle(&mBloudHandle);

    if (p_arsi_lib_config->audio_format ==
        AUDIO_FORMAT_PCM_32_BIT) {
        mPcmFormat = BLOUDHD_IN_Q1P31_OUT_Q1P31;
    } else {
        mPcmFormat = BLOUDHD_IN_Q1P15_OUT_Q1P15;
    }
    /* query mode param size */
    mModeParamSize = sizeof(BLOUD_HD_ModeParam);
    /* query coef buf size */
    mCoefParamBufSize = sizeof(BLOUD_HD_FilterCoef);
    /* query custom param size */
    mCustomParamBufSize = sizeof(BLOUD_HD_CustomParam);
    /* query custom param size */
    mLibHandlerSize = sizeof(audioloud_lib_handle_t);
    /* query working buffer size */
    mBloudHandle.GetBufferSize(&mInternalBufSize, &mTempBufSize, mPcmFormat);
    /* L.R coef buffer */
    *p_working_buf_size = mInternalBufSize + mTempBufSize + mModeParamSize + (2 * mCoefParamBufSize) + mCustomParamBufSize + mLibHandlerSize;

    debug_log_fp("%s(), working_buf_size(%lu) = mInternalBufSize (%u) + mTempBufSize (%u) + mModeParamSize (%u) +  mCoefParamBufSize (%u) + mCustomParamBufSize (%u) + mLibHandlerSize (%u), mPcmFormat(%d)\n",
                 __func__, *p_working_buf_size, mInternalBufSize, mTempBufSize, mModeParamSize, 2 * mCoefParamBufSize, mCustomParamBufSize, mLibHandlerSize, mPcmFormat);
    return NO_ERROR;
}

status_t audioloud_arsi_create_handler(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    data_buf_t               *p_working_buf,
    void                    **pp_handler,
    const debug_log_fp_t      debug_log_fp) {
    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL ||
        p_working_buf == NULL ||
        pp_handler == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    uint32_t mPcmFormat;
    AudioComFltMode_t AudioMode;
    AudioCompFltType_t filtertype;
    audioloud_lib_handle_t *lib_handler = NULL;
    int32_t result;

    audioloud_lib_param_t *lib_param = p_param_buf->p_buffer;
    char *pworking_buf = (char *)p_working_buf->p_buffer;

    if (p_arsi_lib_config->audio_format ==
        AUDIO_FORMAT_PCM_32_BIT) {
        mPcmFormat = BLOUDHD_IN_Q1P31_OUT_Q1P31;
    } else {
        mPcmFormat = BLOUDHD_IN_Q1P15_OUT_Q1P15;
    }

    filtertype = get_filter_bydevice(p_arsi_task_config, lib_param->mDrcSwitch);

    /*init lib handle*/
    lib_handler = lib_param_init(pworking_buf, filtertype, mPcmFormat);

    if (lib_handler == NULL) {
        return BAD_VALUE;
    }

    lib_handler->mScenario = p_arsi_task_config->output_flags;
    lib_handler->mStreamType = p_arsi_task_config->stream_type;
    lib_handler->mOutputDevice = p_arsi_task_config->output_device_info.devices;
    lib_handler->mBypassFlag = lib_param->mBypassFlag;

    set_lib_parameter(BLOUD_PAR_SET_RAMP_UP, (void *)(long)1, lib_handler); //start alway from Ramp up
    AudioMode = convert_filter_type(lib_handler);
    set_lib_parameter(BLOUD_PAR_SET_WORK_MODE, (void *)(long)AudioMode, lib_handler);
    debug_log_fp("%s() : mScenario = %d AudioMode = %d", __func__, lib_handler->mScenario, AudioMode);


    // combine DRC + ACF param
    if (p_arsi_task_config->output_device_info.devices & AUDIO_DEVICE_OUT_SPEAKER) {
        if (p_arsi_task_config->stream_type != AUDIO_STREAM_RING) {
            memcpy((void *)&lib_handler->mCachedAudioParam, (void *)&lib_param->sDrcParam,
                   sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        } else {
            memcpy((void *)&lib_handler->mCachedAudioParam, (void *)&lib_param->sXmlParam,
                   sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        }
    } else {
        //copy param
        memcpy((void *)&lib_handler->mCachedAudioParam, (void *)&lib_param->sXmlParam,
               sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    }

    if (lib_handler->mState == BLOUD_STATE_INIT) {
        /*set lib params*/
        set_lib_parameter(BLOUD_PAR_SET_CHANNEL_NUMBER,
                          (void *)(long)p_arsi_lib_config->p_dl_buf_in->num_channels, lib_handler);
        set_lib_parameter(BLOUD_PAR_SET_SAMPLE_RATE,
                          (void *)(long)p_arsi_lib_config->p_dl_buf_in->sample_rate_content, lib_handler);
        set_lib_parameter(BLOUD_PAR_SET_PCM_FORMAT, (void *)(long)mPcmFormat, lib_handler);

        result = copy_param(lib_handler, &lib_handler->mCachedAudioParam);
        if (result != 0) {
            debug_log_fp("%s : copy_param failed", __func__);
            return BAD_VALUE;
        }
        result = lib_handler->mBloudHandle.Open(&lib_handler->mBloudHandle,
                                                lib_handler->mLibWorkingBuf.mpInternalBuf, (const void *)&lib_handler->mInitParam);
        if (result < 0) {
            return BAD_VALUE;
        }

        lib_handler->mState = BLOUD_STATE_OPEN;
    }
    lib_handler->pLibParamBuf = lib_param;
    debug_log_fp("create handler: %p", lib_handler);

    *pp_handler = lib_handler;

    return NO_ERROR;
}


status_t audioloud_arsi_process_ul_buf(
    audio_buf_t *p_ul_buf_in,
    audio_buf_t *p_ul_buf_out,
    audio_buf_t *p_ul_ref_bufs,
    data_buf_t  *p_debug_dump_buf,
    void        *p_handler) {

    if (p_ul_buf_in == NULL ||
        p_ul_buf_out == NULL ||
        p_handler == NULL) {
        return NO_INIT;
    }

    return NO_ERROR;
}


status_t audioloud_arsi_process_dl_buf(
    audio_buf_t *p_dl_buf_in,
    audio_buf_t *p_dl_buf_out,
    audio_buf_t *p_dl_ref_bufs,
    data_buf_t  *p_debug_dump_buf,
    void        *p_handler) {
    int32_t result = 0;
    audioloud_lib_handle_t *lib_handler = NULL;

    if (p_dl_buf_in == NULL ||
        p_dl_buf_out == NULL ||
        p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (audioloud_lib_handle_t *)p_handler;

    if (lib_handler->mBypassFlag & FILTER_BYPASS_HCF) {
        if ((convert_filter_type(lib_handler))
            == AUDIO_CMP_FLT_LOUDNESS_COMP_HEADPHONE) {
            // bypass the process
            memcpy(p_dl_buf_out->data_buf.p_buffer, p_dl_buf_in->data_buf.p_buffer,
                   p_dl_buf_in->data_buf.data_size);
            p_dl_buf_out->data_buf.data_size = p_dl_buf_in->data_buf.data_size;
            p_dl_buf_in->data_buf.data_size = 0;
            print_log("Bypass HCF process\n");
            return NO_ERROR;
        }
    }
    if (lib_handler->mBypassFlag & FILTER_BYPASS_ACF) {
        if ((convert_filter_type(lib_handler)) == AUDIO_CMP_FLT_LOUDNESS_COMP ||
            (convert_filter_type(lib_handler)) == AUDIO_CMP_FLT_LOUDNESS_COMP_LOW_LATENCY_WITHOUT_LIMITER ||
            (convert_filter_type(lib_handler)) == AUDIO_CMP_FLT_LOUDNESS_COMP_LOW_LATENCY_WITH_LIMITER) {
            // bypass the process
            if (lib_handler->mState == BLOUD_STATE_OPEN) {
                set_lib_parameter(BLOUD_PAR_SET_CHANGE_TO_BYPASS, (void *)(long)1, lib_handler);
                print_log("Bypass ACF process\n");
            }
        }
    }

    uint32_t block_size, block_size_byte, offset_bit, loop_cnt, i;
    uint32_t ConsumedSampleCount = 0, TotalConsumedSample = 0, TotalOuputSample = 0;


    //Simplify handle (BLOCK_SIZE x N) Samples
    if (lib_handler->mInitParam.PCM_Format == BLOUDHD_IN_Q1P15_OUT_Q1P15) { // 16 bits
        // 2-byte, mono
        if (lib_handler->mInitParam.Channel == 1) {
            offset_bit = 1;
        }// 2-byte, L/R
        else {
            offset_bit = 2;
        }
    } else { //32 bits
        if (lib_handler->mInitParam.Channel == 1) {
            // 4-byte, Mono
            offset_bit = 2;
        } else {
            // 4-byte, L/R
            offset_bit = 3;
        }
    }

    if (lib_handler->mScenario & AUDIO_OUTPUT_FLAG_FAST) {
        block_size = BLOCK_SIZE_LOW_LATENCY;    // for fast process
    } else {
        block_size = BLOCK_SIZE;
    }

    block_size_byte = block_size * (1 << offset_bit);
    loop_cnt = (p_dl_buf_in->data_buf.data_size) / block_size_byte;

    print_log("[%d] usr id = %u block_size_byte = %d,PCM = %d, CH = %d filtertype = %d,in data_size = %d",
              lib_handler->mBypassFlag, lib_handler->id, block_size_byte,
              lib_handler->mInitParam.PCM_Format, lib_handler->mInitParam.Channel, lib_handler->mFilterType, p_dl_buf_in->data_buf.data_size);

    for (i = 0; i < loop_cnt; i++) {
        ConsumedSampleCount = block_size_byte;
        p_dl_buf_out->data_buf.data_size = block_size_byte;

        if ((p_dl_buf_in->data_buf.data_size - TotalConsumedSample) <
            ConsumedSampleCount) {
            print_log("Warning for remained input [%d] < output [%d], and skip process",
                      p_dl_buf_in->data_buf.data_size, ConsumedSampleCount);
            break;
        }
        lib_handler->mState = BLOUD_STATE_PROCESS;

        result = lib_handler->mBloudHandle.Process(&lib_handler->mBloudHandle,
                                                   (char *)lib_handler->mLibWorkingBuf.mpTempBuf,
                                                   (int *)((char *)p_dl_buf_in->data_buf.p_buffer + TotalConsumedSample),
                                                   (int *)&ConsumedSampleCount,
                                                   (int *)((char *)p_dl_buf_out->data_buf.p_buffer + TotalOuputSample),
                                                   (int *)&p_dl_buf_out->data_buf.data_size);

        TotalConsumedSample += ConsumedSampleCount;
        TotalOuputSample += p_dl_buf_out->data_buf.data_size;
        print_log("[loop: %d] id = %u ConsumedSampleCount = %d", i,
                  lib_handler->id,
                  ConsumedSampleCount);
    }
    p_dl_buf_out->data_buf.data_size = TotalOuputSample;
    p_dl_buf_in->data_buf.data_size -= TotalConsumedSample;  //remained input
#if 0
    /* Liang: check xml param change here */
    if (getCallbackFilter(lib_handler->mFilterType)) {
        string_buf_t keyValue;
        char updatestr[50];
        keyValue.p_string = updatestr;
        snprintf(keyValue.p_string, sizeof(updatestr), "UpdateACFHCFParameters=%u", lib_handler->mFilterType);
        print_log("%s Param change! %s", __FUNCTION__, keyValue.p_string);
        audioloud_arsi_set_key_value_pair(&keyValue, lib_handler);
        resetCallbackFilter();
    }
#endif
    print_log("- DL raw: result = %d in->data_buf.p_buffer = %p in->data_buf.data_size = %d, out->data_buf.p_buffer = %p, out->data_buf.data_size = %d\n",
              result,
              p_dl_buf_in->data_buf.p_buffer,
              p_dl_buf_in->data_buf.data_size,
              p_dl_buf_out->data_buf.p_buffer,
              p_dl_buf_out->data_buf.data_size
             );
    return result;
}


status_t audioloud_arsi_reset_handler(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler) {

    audioloud_lib_handle_t *lib_handler = NULL;

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL ||
        p_handler == NULL) {
        return BAD_VALUE;
    }


    lib_handler = (audioloud_lib_handle_t *)p_handler;

    BLOUD_HD_RuntimeParam runtime_param;
    runtime_param.Command = BLOUD_HD_RESET;
    lib_handler->mBloudHandle.SetParameters(&lib_handler->mBloudHandle,
                                            &runtime_param);

    print_log("%s(), done\n", __func__);

    return NO_ERROR;
}


status_t audioloud_arsi_destroy_handler(void *p_handler) {
    //audioloud_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    print_log("%s(), p_handler = %p\n", __func__, p_handler);

    return NO_ERROR;
}


status_t audioloud_arsi_update_device(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler) {
    audioloud_lib_handle_t *lib_handler = NULL;

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL ||
        p_handler == NULL) {
        return NO_INIT;
    }
    // should not happened? should reset buffer and re-open again?
    bool needupdate  = false;
    //AudioComFltMode_t AudioMode;
    //AudioCompFltType_t filtertype;
    uint32_t pcm_format;
    //uint32_t num_users;

    lib_handler = (audioloud_lib_handle_t *)p_handler;

    audio_devices_t input_device_new =
        p_arsi_task_config->input_device_info.devices;

    audio_devices_t output_devices_new =
        p_arsi_task_config->output_device_info.devices;

    if (p_arsi_lib_config->p_dl_buf_in->audio_format == AUDIO_FORMAT_PCM_8_24_BIT ||
        p_arsi_lib_config->p_dl_buf_in->audio_format == AUDIO_FORMAT_PCM_32_BIT) {
        pcm_format = BLOUDHD_IN_Q1P31_OUT_Q1P31;
    } else {
        pcm_format = BLOUDHD_IN_Q1P15_OUT_Q1P15;
    }

    print_log("[arsi_update_device] input device: new => 0x%x, old => 0x%x\n",
              input_device_new, mIputDeviceOld);
    print_log("[arsi_update_device] output device: new => 0x%x, old => 0x%x\n",
              output_devices_new, mOutputDeviceOld);

    if (mOutputDeviceOld != output_devices_new) {
        mOutputDeviceOld = output_devices_new;
        needupdate = true;
    }

    return NO_ERROR;
}


status_t audioloud_arsi_update_param(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler) {

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL  ||
        p_handler == NULL) {
        return NO_INIT;
    }
    audioloud_lib_handle_t *lib_handler = NULL;
    audioloud_lib_param_t *lib_param = p_param_buf->p_buffer;

    int32_t result;
    AudioComFltMode_t AudioMode;

    lib_handler = (audioloud_lib_handle_t *)p_handler;
    AudioMode = convert_filter_type(lib_handler);
    set_lib_parameter(BLOUD_PAR_SET_WORK_MODE, (void *)(long)AudioMode, lib_handler);
    print_log("update_param mScenario = %d AudioMode = %d", lib_handler->mScenario, AudioMode);


    // combine DRC + ACF param
    if (p_arsi_task_config->output_device_info.devices & AUDIO_DEVICE_OUT_SPEAKER) {
        if (p_arsi_task_config->stream_type != AUDIO_STREAM_RING) {
            memcpy((void *)&lib_handler->mCachedAudioParam, (void *)&lib_param->sDrcParam,
                   sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        } else {
            memcpy((void *)&lib_handler->mCachedAudioParam, (void *)&lib_param->sXmlParam,
                   sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        }
    } else {
        //copy param
        memcpy((void *)&lib_handler->mCachedAudioParam, (void *)&lib_param->sXmlParam,
               sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));

    }
    result = copy_param(lib_handler, &lib_handler->mCachedAudioParam);

    print_log("%s(), p_param_buf = %p\n",
              __func__, p_param_buf->p_buffer);

    if (result != 0) {
        return BAD_VALUE;
    }

    result = set_lib_parameter(BLOUD_PAR_SET_UPDATE_PARAM_TO_SWIP, (void *)(long)0, lib_handler);

    return (result == 0) ? NO_ERROR : BAD_VALUE;
}


status_t audioloud_arsi_query_param_buf_size(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const string_buf_t       *product_name,
    const string_buf_t       *param_file_path,
    const int32_t             enhancement_mode, //new API
    uint32_t                 *p_param_buf_size,
    const debug_log_fp_t      debug_log_fp) {
    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        product_name == NULL ||
        param_file_path == NULL ||
        p_param_buf_size == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    *p_param_buf_size = sizeof(audioloud_lib_param_t);

    debug_log_fp("%s(), get param buf size %u\n", __func__, *p_param_buf_size);

    return NO_ERROR;
}


status_t audioloud_arsi_parsing_param_file(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const string_buf_t       *product_name,
    const string_buf_t       *param_file_path,
    const int32_t             enhancement_mode, //new API
    data_buf_t               *p_param_buf,
    const debug_log_fp_t      debug_log_fp) {

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        product_name == NULL ||
        param_file_path == NULL ||
        p_param_buf == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    //call xml flow
    if (enhancement_mode >= AUDIO_COMP_FLT_NUM) {
        return BAD_VALUE;
    }

    return NO_ERROR;
}


status_t audioloud_arsi_set_addr_value(
    const uint32_t addr,
    const uint32_t value,
    void          *p_handler) {
    audioloud_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (audioloud_lib_handle_t *)p_handler;

    print_log("%s(), set value 0x%x at addr 0x%x\n",
              __func__, value, addr);


    return NO_ERROR;
}


status_t audioloud_arsi_get_addr_value(
    const uint32_t addr,
    uint32_t      *p_value,
    void          *p_handler) {
    audioloud_lib_handle_t *lib_handler = NULL;

    if (p_value == NULL || p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (audioloud_lib_handle_t *)p_handler;

    if (addr == 0x1234) {
        //  *p_value = lib_handler->value_at_addr_0x1234;
    }

    // print_log("%s(), value 0x%x at addr 0x%x\n",
    //                        __func__, *p_value, addr);
    return NO_ERROR;
}


status_t audioloud_arsi_set_key_value_pair(
    const string_buf_t *key_value_pair,
    void               *p_handler) {
    audioloud_lib_handle_t *lib_handler = NULL;

    if (key_value_pair == NULL || p_handler == NULL) {
        return NO_INIT;
    }

    int ret = NO_INIT;
    //int num_users = 0;
    int set_value = 0;
    char *strval = NULL;
    char *pch = NULL;
    lib_handler = (audioloud_lib_handle_t *)p_handler;
    pch = strchr(key_value_pair->p_string, '=');
    if (pch != NULL) {  // this line contain '=' character
        strval = pch + 1;
        set_value = atoi(strval);
        print_log("%s(), key value pair = %s value = %s\n",
                  __func__, key_value_pair->p_string, strval);
    } else {
        return BAD_VALUE;
    }

    // check ACF <-> ACF+DRC, BYPASS ->
    if (strstr(key_value_pair->p_string, "SetBesLoudnessStatus") != NULL &&
        lib_handler->mFilterType != AUDIO_COMP_FLT_HEADPHONE) {
        AudioComFltMode_t AudioFltMode;
        if (set_value == 0) {   //set process to bypass
            if (lib_handler->mBypassFlag & FILTER_BYPASS_ACF) {
                ret = set_lib_parameter(BLOUD_PAR_SET_CHANGE_TO_BYPASS, (void *)(long)set_value, lib_handler);
                print_log("DRC off, change to bypass mode\n");
            } else {       // only ACF
                lib_handler->mFilterType = convert_flt_by_stream_type(lib_handler->mStreamType, false, AUDIO_DEVICE_OUT_SPEAKER);
                AudioFltMode = convert_filter_type(lib_handler);
                set_lib_parameter(BLOUD_PAR_SET_WORK_MODE, (void *)(long)AudioFltMode, lib_handler);
                lib_switch_drcparam(lib_handler);
                ret = set_lib_parameter(BLOUD_PAR_SET_UPDATE_PARAM_TO_SWIP, (void *)(long)set_value, lib_handler);
            }
            lib_handler->pLibParamBuf->mDrcSwitch = set_value;
        } else if (set_value == 1) {
            lib_handler->mFilterType = convert_flt_by_stream_type(lib_handler->mStreamType, true, AUDIO_DEVICE_OUT_SPEAKER);
            AudioFltMode = convert_filter_type(lib_handler);
            set_lib_parameter(BLOUD_PAR_SET_WORK_MODE, (void *)(long)AudioFltMode, lib_handler);
            lib_switch_drcparam(lib_handler);
            if (lib_handler->mBypassFlag & FILTER_BYPASS_ACF) {
                // re-open & set DRC only
                set_lib_parameter(BLOUD_PAR_SET_RAMP_UP, (void *)(long)1, lib_handler);
                ret = lib_handler->mBloudHandle.Open(&lib_handler->mBloudHandle,
                                                     lib_handler->mLibWorkingBuf.mpInternalBuf,
                                                     (const void *)&lib_handler->mInitParam);
                print_log("DRC on, Re-open to DRC mode\n");
            } else {
                ret = set_lib_parameter(BLOUD_PAR_SET_UPDATE_PARAM_TO_SWIP, (void *)(long)set_value, lib_handler);
            }
            lib_handler->pLibParamBuf->mDrcSwitch = set_value;
        }
    }
    if (strstr(key_value_pair->p_string, "SetAudioCustomScene") != NULL) {
        unsigned int str_len = strlen(strval);
        unsigned int lastscene_len = strlen(lib_handler->mCustScene);
        unsigned int arr_len = sizeof(lib_handler->mCustScene);
        if (str_len > arr_len) {
            str_len = arr_len;
        }
        if ((str_len == lastscene_len) &&
            (strncmp(lib_handler->mCustScene, strval, str_len) == 0)) {
            print_log("SAME Scene!");
            return NO_ERROR;
        }
        memset(lib_handler->mCustScene, 0, arr_len);
        strncpy(lib_handler->mCustScene, strval, str_len);
        ret = 0;
    } else if (strstr(key_value_pair->p_string,
                      "UpdateACFHCFParameters") != NULL) {
        uint32_t cur_filtype = lib_handler->mFilterType;
        if ((set_value == AUDIO_COMP_FLT_AUDIO &&
             (cur_filtype == AUDIO_COMP_FLT_AUDIO || cur_filtype == AUDIO_COMP_FLT_AUDIO_FOR_RINGTONE)) ||
            (set_value == AUDIO_COMP_FLT_HEADPHONE && cur_filtype == AUDIO_COMP_FLT_HEADPHONE) ||
            (set_value == AUDIO_COMP_FLT_HCF_FOR_USB && cur_filtype == AUDIO_COMP_FLT_HCF_FOR_USB) ||
            (set_value == AUDIO_COMP_FLT_DRC_FOR_MUSIC &&
             (cur_filtype == AUDIO_COMP_FLT_DRC_FOR_MUSIC || cur_filtype == AUDIO_COMP_FLT_DRC_FOR_RINGTONE))) {
            ret = NO_ERROR; //use APPLY_PARAM cmd
        } else {
            print_log("UpdateACFHCFParameters not match with current Filter\n");
            return BAD_VALUE;
        }
    } else if (strstr(key_value_pair->p_string,
                      "BLOUD_PAR_SET_STEREO_TO_MONO_MODE") != NULL) {
        ret = set_lib_parameter(BLOUD_PAR_SET_STEREO_TO_MONO_MODE, (void *)(long)set_value, lib_handler);
    } else if (strstr(key_value_pair->p_string,
                      "BLOUD_PAR_SET_UPDATE_PARAM_TO_SWIP") != NULL) {
        ret = set_lib_parameter(BLOUD_PAR_SET_UPDATE_PARAM_TO_SWIP, (void *)(long)set_value, lib_handler);
    } else if (strstr(key_value_pair->p_string,
                      "BLOUD_PAR_SET_CHANGE_TO_BYPASS") != NULL) {
        ret = set_lib_parameter(BLOUD_PAR_SET_CHANGE_TO_BYPASS, (void *)(long)set_value, lib_handler);
    } else if (strstr(key_value_pair->p_string,
                      "BLOUD_PAR_SET_CHANGE_TO_NORMAL") != NULL) {
        ret = set_lib_parameter(BLOUD_PAR_SET_CHANGE_TO_NORMAL, (void *)(long)set_value, lib_handler);
    } else if (strstr(key_value_pair->p_string, "SetStreamType") != NULL) {
        AudioComFltMode_t AudioMode;

        if (lib_handler->mOutputDevice == AUDIO_DEVICE_OUT_SPEAKER) {
            uint32_t newAudioFilter;
            audioloud_lib_param_t *pParamBuf = lib_handler->pLibParamBuf;
            newAudioFilter = convert_flt_by_stream_type(set_value, pParamBuf->mDrcSwitch, AUDIO_DEVICE_OUT_SPEAKER);

            if (newAudioFilter == lib_handler->mFilterType) {
                return NO_ERROR;
            } else {
                switch (newAudioFilter) {
                print_log("old filter = %d new filter = %d", lib_handler->mFilterType, newAudioFilter);
                case AUDIO_COMP_FLT_AUDIO:
                case AUDIO_COMP_FLT_DRC_FOR_MUSIC:
                    memcpy((void *)&lib_handler->mCachedAudioParam, (void *)&pParamBuf->sDrcParam,
                           sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
                    break;
                case AUDIO_COMP_FLT_AUDIO_FOR_RINGTONE:
                case AUDIO_COMP_FLT_DRC_FOR_RINGTONE:
                    memcpy((void *)&lib_handler->mCachedAudioParam, (void *)&pParamBuf->sXmlParam,
                           sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
                    break;
                default:
                    return NO_ERROR;
                }
                lib_handler->mFilterType = newAudioFilter;
            }

            AudioMode = convert_filter_type(lib_handler);
            print_log("update_param mScenario = %d AudioMode = %d", lib_handler->mScenario, AudioMode);
            set_lib_parameter(BLOUD_PAR_SET_WORK_MODE, (void *)(long)AudioMode, lib_handler);
            ret = copy_param(lib_handler, &lib_handler->mCachedAudioParam);

            if (ret != 0) {
                return BAD_VALUE;
            }
            ret = set_lib_parameter(BLOUD_PAR_SET_UPDATE_PARAM_TO_SWIP, (void *)(long)0, lib_handler);
        } else {
            return NO_ERROR;
        }
    }
    if (ret != 0) {
        ret = UNKNOWN_ERROR;
    } else {
        ret = NO_ERROR;
    }

    print_log("- %s(), ret = %d\n", __func__, ret);

    return ret;
}

status_t audioloud_arsi_get_key_value_pair(
    string_buf_t *key_value_pair,
    void         *p_handler) {
    audioloud_lib_handle_t *lib_handler = NULL;

    if (key_value_pair == NULL || p_handler == NULL) {
        return NO_INIT;
    }
    lib_handler = (audioloud_lib_handle_t *)p_handler;
    print_log("+%s(), key = %s\n",
              __func__, key_value_pair->p_string);

    if (strcmp(key_value_pair->p_string, "GetStatus") == 0) {
        BLOUD_HD_RuntimeStatus run_stat;
        memset(&run_stat, 0, sizeof(BLOUD_HD_RuntimeStatus));
        lib_handler->mBloudHandle.GetStatus(&lib_handler->mBloudHandle, &run_stat);
        if (run_stat.State == BLOUD_HD_SWITCHING_STATE) {
            snprintf(key_value_pair->p_string, key_value_pair->memory_size,
                     "GetStatus=%s", "BLOUD_HD_SWITCHING_STATE");
        } else if (run_stat.State == BLOUD_HD_NORMAL_STATE) {
            snprintf(key_value_pair->p_string, key_value_pair->memory_size,
                     "GetStatus=%s", "BLOUD_HD_NORMAL_STATE");
        }
    }
    return NO_ERROR;
}


status_t audioloud_arsi_set_ul_digital_gain(
    const int16_t ul_analog_gain_ref_only,
    const int16_t ul_digital_gain,
    void         *p_handler) {
    audioloud_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (audioloud_lib_handle_t *)p_handler;
    return NO_ERROR;
}


status_t audioloud_arsi_set_dl_digital_gain(
    const int16_t dl_analog_gain_ref_only,
    const int16_t dl_digital_gain,
    void         *p_handler) {
    audioloud_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (audioloud_lib_handle_t *)p_handler;
    return NO_ERROR;
}


status_t audioloud_arsi_set_ul_mute(const uint8_t b_mute_on, void *p_handler) {
    audioloud_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (audioloud_lib_handle_t *)p_handler;
    return NO_ERROR;
}


status_t audioloud_arsi_set_dl_mute(const uint8_t b_mute_on, void *p_handler) {
    audioloud_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (audioloud_lib_handle_t *)p_handler;

    print_log("%s b_mute_on = %d\n", __func__, b_mute_on);
    return NO_ERROR;
}


status_t audioloud_arsi_set_ul_enhance(const uint8_t b_enhance_on,
                                       void *p_handler) {
    if (p_handler == NULL) {
        return NO_INIT;
    }
    return NO_ERROR;
}


status_t audioloud_arsi_set_dl_enhance(const uint8_t b_enhance_on,
                                       void *p_handler) {
    audioloud_lib_handle_t *lib_handler = NULL;
    int ret = 0;
    bool change = false;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (audioloud_lib_handle_t *)p_handler;

    print_log("pUser->id = %u b_enhance_on = %d\n", lib_handler->id,
              b_enhance_on);

    if (lib_handler->bIsEnhaceOn != b_enhance_on) {
        lib_handler->bIsEnhaceOn  = b_enhance_on;
        change = true;
        print_log("Change! pUser->id = %u\n", lib_handler->id);
    }
    if (change) {
        if (!b_enhance_on) {
            ret = set_lib_parameter(BLOUD_PAR_SET_CHANGE_TO_BYPASS, (void *)(long)b_enhance_on, lib_handler);
        } else {
            ret = set_lib_parameter(BLOUD_PAR_SET_CHANGE_TO_NORMAL, (void *)(long)b_enhance_on, lib_handler);
        }

        if (ret != 0) {
            ret = UNKNOWN_ERROR;
        } else {
            ret = NO_ERROR;
        }
    }
    return ret;
}


status_t audioloud_arsi_set_debug_log_fp(const debug_log_fp_t debug_log,
                                         void *p_handler) {
    audioloud_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (audioloud_lib_handle_t *)p_handler;
    print_log = debug_log;

    return NO_ERROR;
}

status_t audioloud_arsi_get_lib_version(string_buf_t *version_buf) {

    /* for callback query buffer API */
    BS_HD_Handle mBloudHandle;
    BLOUD_HD_SetHandle(&mBloudHandle);
    BS_HD_EngineInfo Eng_Info;
    mBloudHandle.GetEngineInfo(&Eng_Info);
    sprintf(version_buf->p_string, "%d", Eng_Info.Version);

    return NO_ERROR;

}


status_t audioloud_arsi_query_process_unit_bytes(uint32_t *p_uplink_bytes,
                                                 uint32_t *p_downlink_bytes,
                                                 void     *p_handler) {
    uint32_t block_size, offset_bit;
    audioloud_lib_handle_t *lib_handler = NULL;
    if (p_handler == NULL ||
        p_uplink_bytes == NULL ||
        p_downlink_bytes == NULL) {
        return BAD_VALUE;
    }

    lib_handler = (audioloud_lib_handle_t *)p_handler;
    *p_uplink_bytes = 0; //no uplink process
    if (lib_handler->mInitParam.PCM_Format == BLOUDHD_IN_Q1P15_OUT_Q1P15) { // 16 bits
        // 2-byte, mono
        if (lib_handler->mInitParam.Channel == 1) {
            offset_bit = 1;
        }// 2-byte, L/R
        else {
            offset_bit = 2;
        }
    } else { //32 bits
        if (lib_handler->mInitParam.Channel == 1) {
            // 4-byte, Mono
            offset_bit = 2;
        } else {
            // 4-byte, L/R
            offset_bit = 3;
        }
    }

    if (lib_handler->mScenario & AUDIO_OUTPUT_FLAG_FAST) {
        block_size = BLOCK_SIZE_LOW_LATENCY;    // for fast proces
    } else {
        block_size = BLOCK_SIZE;
    }

    *p_downlink_bytes = block_size * (1 << offset_bit);
    return NO_ERROR;
}


void besloudness_arsi_assign_lib_fp(AurisysLibInterface *lib) {
    lib->arsi_get_lib_version = audioloud_arsi_get_lib_version;
    lib->arsi_query_working_buf_size = audioloud_arsi_query_working_buf_size;
    lib->arsi_create_handler = audioloud_arsi_create_handler;
    lib->arsi_process_ul_buf = audioloud_arsi_process_ul_buf;
    lib->arsi_process_dl_buf = audioloud_arsi_process_dl_buf;
    lib->arsi_reset_handler = audioloud_arsi_reset_handler;
    lib->arsi_destroy_handler = audioloud_arsi_destroy_handler;
    lib->arsi_update_device = audioloud_arsi_update_device;
    lib->arsi_update_param = audioloud_arsi_update_param;
    lib->arsi_query_param_buf_size = audioloud_arsi_query_param_buf_size;
    lib->arsi_parsing_param_file = audioloud_arsi_parsing_param_file;
    lib->arsi_set_addr_value = audioloud_arsi_set_addr_value;
    lib->arsi_get_addr_value = audioloud_arsi_get_addr_value;
    lib->arsi_set_key_value_pair = audioloud_arsi_set_key_value_pair;
    lib->arsi_get_key_value_pair = audioloud_arsi_get_key_value_pair;
    lib->arsi_set_ul_digital_gain = audioloud_arsi_set_ul_digital_gain;
    lib->arsi_set_dl_digital_gain = audioloud_arsi_set_dl_digital_gain;
    lib->arsi_set_ul_mute = audioloud_arsi_set_ul_mute;
    lib->arsi_set_dl_mute = audioloud_arsi_set_dl_mute;
    lib->arsi_set_ul_enhance = audioloud_arsi_set_ul_enhance;
    lib->arsi_set_dl_enhance = audioloud_arsi_set_dl_enhance;
    lib->arsi_set_debug_log_fp = audioloud_arsi_set_debug_log_fp;
    lib->arsi_query_process_unit_bytes = audioloud_arsi_query_process_unit_bytes;

}


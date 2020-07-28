/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "StereoTuningProvider_DVS_ME"

#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
#include "hw_dpe_dvs_me_tuning.h"

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

HW_DPE_DVS_ME_Tuning::~HW_DPE_DVS_ME_Tuning()
{
    for(auto &t : __tuningMap) {
        if(t.second) {
            delete t.second;
        }
    }
}

bool
HW_DPE_DVS_ME_Tuning::retrieveTuningParams(TuningQuery_T &query)
{
    NSCam::NSIoPipe::DPEConfig *dst = (NSCam::NSIoPipe::DPEConfig *)query.results[STEREO_TUNING_NAME[E_TUNING_HW_DPE]];

    if(NULL == dst) {
        MY_LOGE("Cannot get target");
        return false;
    }

    DVS_ME_CFG *src = NULL;
    if(StereoSettingProvider::isActiveStereo() &&
       __tuningMap.find(QUERY_KEY_ACTIVE) != __tuningMap.end())
    {
        src = __tuningMap[QUERY_KEY_ACTIVE];
    }
    else if(__tuningMap.find(QUERY_KEY_PREVIEW) != __tuningMap.end())
    {
        src = __tuningMap[QUERY_KEY_PREVIEW];
    }
    else
    {
        src = __tuningMap.begin()->second;
    }

    if(NULL == src) {
        MY_LOGE("Cannot get source");
        return false;
    }

    ::memcpy(&(dst->Dpe_DVSSettings.TuningBuf_ME), src, sizeof(DVS_ME_CFG));

    return true;
}

void
HW_DPE_DVS_ME_Tuning::_initBBox(DVS_ME_CFG *tuningBuffer, ENUM_STEREO_SCENARIO scenario)
{
    StereoArea area = StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y, scenario);
    area *= 8;
    MSize szImage = area.size;
    MPoint ptTopLeft = area.startPt;
    if(0 == StereoSettingProvider::getSensorRelativePosition()) {   //Main on left
        tuningBuffer->DVS_ME_33.Bits.c_lp_box_l_left   = ptTopLeft.x;
        tuningBuffer->DVS_ME_33.Bits.c_lp_box_l_right  = szImage.w - 8;
        tuningBuffer->DVS_ME_34.Bits.c_lp_box_l_top    = ptTopLeft.y;
        tuningBuffer->DVS_ME_34.Bits.c_lp_box_l_bottom = szImage.h - ptTopLeft.y - 8;
        tuningBuffer->DVS_ME_35.Bits.c_lp_box_r_left   = 0;
        tuningBuffer->DVS_ME_35.Bits.c_lp_box_r_right  = szImage.w - 8;
        tuningBuffer->DVS_ME_36.Bits.c_lp_box_r_top    = 0;
        tuningBuffer->DVS_ME_36.Bits.c_lp_box_r_bottom = szImage.h - 8;
    } else {
        tuningBuffer->DVS_ME_33.Bits.c_lp_box_l_left   = 0;
        tuningBuffer->DVS_ME_33.Bits.c_lp_box_l_right  = szImage.w - 8;
        tuningBuffer->DVS_ME_34.Bits.c_lp_box_l_top    = 0;
        tuningBuffer->DVS_ME_34.Bits.c_lp_box_l_bottom = szImage.h - 8;
        tuningBuffer->DVS_ME_35.Bits.c_lp_box_r_left   = 0;
        tuningBuffer->DVS_ME_35.Bits.c_lp_box_r_right  = szImage.w - ptTopLeft.x - 8;
        tuningBuffer->DVS_ME_36.Bits.c_lp_box_r_top    = ptTopLeft.y;
        tuningBuffer->DVS_ME_36.Bits.c_lp_box_r_bottom = szImage.h - ptTopLeft.y - 8;
    }
}

void
HW_DPE_DVS_ME_Tuning::_initDefaultValues()
{
    DVS_ME_CFG *tuningBuffer = new DVS_ME_CFG();
    _initBBox(tuningBuffer, eSTEREO_SCENARIO_PREVIEW);

    tuningBuffer->DVS_ME_00.Bits.c_dv_cand_num              = 8;
    tuningBuffer->DVS_ME_00.Bits.c_ord_pnlty_shift_dir      = 1;
    tuningBuffer->DVS_ME_00.Bits.c_ord_pnlty_shift_num      = 1;
    tuningBuffer->DVS_ME_00.Bits.c_ord_th                   = 128;
    tuningBuffer->DVS_ME_00.Bits.c_ord_coring               = 4;
    tuningBuffer->DVS_ME_01.Bits.c_sm_dv_x_pnlty_sel        = 0;
    tuningBuffer->DVS_ME_01.Bits.c_sm_dv_y_pnlty_sel        = 4;
    tuningBuffer->DVS_ME_01.Bits.c_sm_dv_ada_base           = 0;
    tuningBuffer->DVS_ME_01.Bits.c_sm_dv_th                 = 12;
    tuningBuffer->DVS_ME_01.Bits.c_sm_dv_th2                = 0;
    tuningBuffer->DVS_ME_02.Bits.c_sptl_pnlty_sel           = 2;
    tuningBuffer->DVS_ME_02.Bits.c_tmpr_pnlty_sel           = 2;
    tuningBuffer->DVS_ME_02.Bits.c_rand_pnlty_sel           = 2;
    tuningBuffer->DVS_ME_02.Bits.c_nbr_pnlty_sel            = 2;
    tuningBuffer->DVS_ME_02.Bits.c_prev_pnlty_sel           = 2;
    tuningBuffer->DVS_ME_02.Bits.c_gmv_pnlty_sel            = 0;
    tuningBuffer->DVS_ME_02.Bits.c_refine_pnlty_sel         = 2;
    tuningBuffer->DVS_ME_03.Bits.c_sptl_cost                = 1;
    tuningBuffer->DVS_ME_03.Bits.c_tmpr_cost                = 3;
    tuningBuffer->DVS_ME_03.Bits.c_refine_cost              = 2;
    tuningBuffer->DVS_ME_03.Bits.c_nbr_cost                 = 3;
    tuningBuffer->DVS_ME_03.Bits.c_prev_cost                = 5;
    tuningBuffer->DVS_ME_03.Bits.c_gmv_cost                 = 0;
    tuningBuffer->DVS_ME_03.Bits.c_rand_cost                = 16;
    tuningBuffer->DVS_ME_04.Bits.c_bg_mv_th                 = 160;
    tuningBuffer->DVS_ME_04.Bits.c_bg_mv_pnlty_sel          = 0;
    tuningBuffer->DVS_ME_04.Bits.c_bg_mv_cost               = 0;
    tuningBuffer->DVS_ME_04.Bits.c_bg_mv_diff               = 32;
    tuningBuffer->DVS_ME_04.Bits.c_conf_div                 = 2;
    tuningBuffer->DVS_ME_05.Bits.c_seed_l_c04               = 17;
    tuningBuffer->DVS_ME_05.Bits.c_seed_l_c03               = 33;
    tuningBuffer->DVS_ME_05.Bits.c_seed_l_c02               = 32;
    tuningBuffer->DVS_ME_05.Bits.c_seed_l_c01               = 28;
    tuningBuffer->DVS_ME_05.Bits.c_seed_l_c00               = 7;
    tuningBuffer->DVS_ME_06.Bits.c_seed_r_c01               = 28;
    tuningBuffer->DVS_ME_06.Bits.c_seed_r_c00               = 7;
    tuningBuffer->DVS_ME_06.Bits.c_seed_l_c07               = 31;
    tuningBuffer->DVS_ME_06.Bits.c_seed_l_c06               = 11;
    tuningBuffer->DVS_ME_06.Bits.c_seed_l_c05               = 13;
    tuningBuffer->DVS_ME_07.Bits.c_seed_r_c06               = 11;
    tuningBuffer->DVS_ME_07.Bits.c_seed_r_c05               = 13;
    tuningBuffer->DVS_ME_07.Bits.c_seed_r_c04               = 17;
    tuningBuffer->DVS_ME_07.Bits.c_seed_r_c03               = 33;
    tuningBuffer->DVS_ME_07.Bits.c_seed_r_c02               = 32;
    tuningBuffer->DVS_ME_08.Bits.c_dv_ini_x                 = 0;
    tuningBuffer->DVS_ME_08.Bits.c_gmv_y1                   = 0;
    tuningBuffer->DVS_ME_08.Bits.c_gmv_x1                   = 0;
    tuningBuffer->DVS_ME_08.Bits.c_seed_r_c07               = 31;
    tuningBuffer->DVS_ME_09.Bits.c_type_l_c07               = 6;
    tuningBuffer->DVS_ME_09.Bits.c_type_l_c06               = 1;
    tuningBuffer->DVS_ME_09.Bits.c_type_l_c05               = 2;
    tuningBuffer->DVS_ME_09.Bits.c_type_l_c04               = 2;
    tuningBuffer->DVS_ME_09.Bits.c_type_l_c03               = 3;
    tuningBuffer->DVS_ME_09.Bits.c_type_l_c02               = 3;
    tuningBuffer->DVS_ME_09.Bits.c_type_l_c01               = 4;
    tuningBuffer->DVS_ME_09.Bits.c_type_l_c00               = 1;
    tuningBuffer->DVS_ME_10.Bits.c_type_r_c07               = 6;
    tuningBuffer->DVS_ME_10.Bits.c_type_r_c06               = 1;
    tuningBuffer->DVS_ME_10.Bits.c_type_r_c05               = 2;
    tuningBuffer->DVS_ME_10.Bits.c_type_r_c04               = 2;
    tuningBuffer->DVS_ME_10.Bits.c_type_r_c03               = 3;
    tuningBuffer->DVS_ME_10.Bits.c_type_r_c02               = 3;
    tuningBuffer->DVS_ME_10.Bits.c_type_r_c01               = 4;
    tuningBuffer->DVS_ME_10.Bits.c_type_r_c00               = 1;
    // tuningBuffer->DVS_ME_11.Bits.sta_shift_h                = 0;
    // tuningBuffer->DVS_ME_11.Bits.sta_shift_v                = 0;
    tuningBuffer->DVS_ME_12.Bits.c_rand_max_mode            = 7;
    tuningBuffer->DVS_ME_12.Bits.c_rand_round_offset        = 18;
    tuningBuffer->DVS_ME_12.Bits.c_rand_init                = 0;
    tuningBuffer->DVS_ME_12.Bits.c_rand1_vec_base_mv_shift  = 0;
    tuningBuffer->DVS_ME_12.Bits.c_rand2_vec_base_mv_shift  = 1;
    tuningBuffer->DVS_ME_12.Bits.c_refine_vec_base_mv_shift = 0;
    tuningBuffer->DVS_ME_12.Bits.c_rand3_vec_base_mv_shift  = 1;
    tuningBuffer->DVS_ME_13.Bits.c_rand_dv4                 = 29;
    tuningBuffer->DVS_ME_14.Bits.c_rand_dv1                 = 7;
    tuningBuffer->DVS_ME_14.Bits.c_rand_dv2                 = 13;
    tuningBuffer->DVS_ME_14.Bits.c_rand_dv3                 = 23;
    tuningBuffer->DVS_ME_15.Bits.c_ad_shift                 = 8;
    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c07               = 3;
    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c06               = 3;
    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c05               = 2;
    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c04               = 2;
    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c03               = 1;
    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c02               = 1;
    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c01               = 0;
    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c00               = 0;
    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c15               = 7;
    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c14               = 7;
    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c13               = 6;
    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c12               = 6;
    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c11               = 5;
    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c10               = 5;
    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c09               = 4;
    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c08               = 4;
    tuningBuffer->DVS_ME_18.Bits.c_seed_priority_r          = 64;
    tuningBuffer->DVS_ME_18.Bits.c_seed_priority_l          = 1;
    tuningBuffer->DVS_ME_19.Bits.c_sr_h                     = 255;
    tuningBuffer->DVS_ME_19.Bits.c_sr_v                     = 8;
    tuningBuffer->DVS_ME_19.Bits.c_zrv                      = 0;
    // tuningBuffer->DVS_ME_20.Bits.c_var_frm_th               = 0;
    // tuningBuffer->DVS_ME_20.Bits.c_coner_th                 = 0;
    // tuningBuffer->DVS_ME_20.Bits.c_sr_h_warp                = 0;
    tuningBuffer->DVS_ME_21.Bits.c_lp_box_en                = 1;
    tuningBuffer->DVS_ME_21.Bits.c_as_en                    = 1;
    tuningBuffer->DVS_ME_21.Bits.c_conf_mode                = 0;
    tuningBuffer->DVS_ME_21.Bits.c_as_th                    = 0;
    tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c03             = 26;
    tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c02             = 32;
    tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c01             = 40;
    tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c00             = 50;
    tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c07             = 11;
    tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c06             = 13;
    tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c05             = 16;
    tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c04             = 21;
    tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c11             = 4;
    tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c10             = 5;
    tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c09             = 7;
    tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c08             = 8;
    tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c15             = 2;
    tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c14             = 2;
    tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c13             = 3;
    tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c12             = 3;
    tuningBuffer->DVS_ME_26.Bits.c_lut_cmlm_step            = 2;
    tuningBuffer->DVS_ME_26.Bits.c_lut_cmlm_c16             = 1;
    tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c03             = 2;
    tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c02             = 1;
    tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c01             = 1;
    tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c00             = 0;
    tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c07             = 4;
    tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c06             = 3;
    tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c05             = 3;
    tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c04             = 2;
    tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c11             = 6;
    tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c10             = 5;
    tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c09             = 5;
    tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c08             = 4;
    tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c15             = 7;
    tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c14             = 7;
    tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c13             = 6;
    tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c12             = 6;
    tuningBuffer->DVS_ME_31.Bits.c_bgcmmn                   = 1;
    tuningBuffer->DVS_ME_31.Bits.c_bgcpkrn                  = 3;
    tuningBuffer->DVS_ME_31.Bits.c_bgclrd                   = 3;
    tuningBuffer->DVS_ME_31.Bits.c_bgcmlm                   = 50;
    tuningBuffer->DVS_ME_32.Bits.c_wgtcmmn                  = 1;
    tuningBuffer->DVS_ME_32.Bits.c_wgtcpkrn                 = 1;
    tuningBuffer->DVS_ME_32.Bits.c_wgtclrd                  = 1;
    tuningBuffer->DVS_ME_32.Bits.c_wgtcmlm                  = 1;
    tuningBuffer->DVS_ME_32.Bits.c_divwgt                   = 1;
    tuningBuffer->DVS_ME_32.Bits.c_cost_validnum_th         = 7;

    __tuningMap[QUERY_KEY_PREVIEW] = tuningBuffer;
}

void
HW_DPE_DVS_ME_Tuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("===== DVS ME Parameters =====");
    for(auto &tuning : __tuningMap) {
        DVS_ME_CFG *tuningBuffer = tuning.second;
        logger
        .FastLogD("Scenario                   %s", tuning.first.c_str())
        .FastLogD("c_lp_box_l_left            %d", tuningBuffer->DVS_ME_33.Bits.c_lp_box_l_left)
        .FastLogD("c_lp_box_l_top             %d", tuningBuffer->DVS_ME_34.Bits.c_lp_box_l_top)
        .FastLogD("c_lp_box_l_right           %d", tuningBuffer->DVS_ME_33.Bits.c_lp_box_l_right)
        .FastLogD("c_lp_box_l_bottom          %d", tuningBuffer->DVS_ME_34.Bits.c_lp_box_l_bottom)
        .FastLogD("c_lp_box_r_left            %d", tuningBuffer->DVS_ME_35.Bits.c_lp_box_r_left)
        .FastLogD("c_lp_box_r_top             %d", tuningBuffer->DVS_ME_36.Bits.c_lp_box_r_top)
        .FastLogD("c_lp_box_r_right           %d", tuningBuffer->DVS_ME_35.Bits.c_lp_box_r_right)
        .FastLogD("c_lp_box_r_bottom          %d", tuningBuffer->DVS_ME_36.Bits.c_lp_box_r_bottom)
        .FastLogD("c_dv_cand_num              %d", tuningBuffer->DVS_ME_00.Bits.c_dv_cand_num)
        .FastLogD("c_ord_pnlty_shift_dir      %d", tuningBuffer->DVS_ME_00.Bits.c_ord_pnlty_shift_dir)
        .FastLogD("c_ord_pnlty_shift_num      %d", tuningBuffer->DVS_ME_00.Bits.c_ord_pnlty_shift_num)
        .FastLogD("c_ord_th                   %d", tuningBuffer->DVS_ME_00.Bits.c_ord_th)
        .FastLogD("c_ord_coring               %d", tuningBuffer->DVS_ME_00.Bits.c_ord_coring)
        .FastLogD("c_sm_dv_x_pnlty_sel        %d", tuningBuffer->DVS_ME_01.Bits.c_sm_dv_x_pnlty_sel)
        .FastLogD("c_sm_dv_y_pnlty_sel        %d", tuningBuffer->DVS_ME_01.Bits.c_sm_dv_y_pnlty_sel)
        .FastLogD("c_sm_dv_ada_base           %d", tuningBuffer->DVS_ME_01.Bits.c_sm_dv_ada_base)
        .FastLogD("c_sm_dv_th                 %d", tuningBuffer->DVS_ME_01.Bits.c_sm_dv_th)
        .FastLogD("c_sm_dv_th2                %d", tuningBuffer->DVS_ME_01.Bits.c_sm_dv_th2)
        .FastLogD("c_sptl_pnlty_sel           %d", tuningBuffer->DVS_ME_02.Bits.c_sptl_pnlty_sel)
        .FastLogD("c_tmpr_pnlty_sel           %d", tuningBuffer->DVS_ME_02.Bits.c_tmpr_pnlty_sel)
        .FastLogD("c_rand_pnlty_sel           %d", tuningBuffer->DVS_ME_02.Bits.c_rand_pnlty_sel)
        .FastLogD("c_nbr_pnlty_sel            %d", tuningBuffer->DVS_ME_02.Bits.c_nbr_pnlty_sel)
        .FastLogD("c_prev_pnlty_sel           %d", tuningBuffer->DVS_ME_02.Bits.c_prev_pnlty_sel)
        .FastLogD("c_gmv_pnlty_sel            %d", tuningBuffer->DVS_ME_02.Bits.c_gmv_pnlty_sel)
        .FastLogD("c_refine_pnlty_sel         %d", tuningBuffer->DVS_ME_02.Bits.c_refine_pnlty_sel)
        .FastLogD("c_sptl_cost                %d", tuningBuffer->DVS_ME_03.Bits.c_sptl_cost)
        .FastLogD("c_tmpr_cost                %d", tuningBuffer->DVS_ME_03.Bits.c_tmpr_cost)
        .FastLogD("c_refine_cost              %d", tuningBuffer->DVS_ME_03.Bits.c_refine_cost)
        .FastLogD("c_nbr_cost                 %d", tuningBuffer->DVS_ME_03.Bits.c_nbr_cost)
        .FastLogD("c_prev_cost                %d", tuningBuffer->DVS_ME_03.Bits.c_prev_cost)
        .FastLogD("c_gmv_cost                 %d", tuningBuffer->DVS_ME_03.Bits.c_gmv_cost)
        .FastLogD("c_rand_cost                %d", tuningBuffer->DVS_ME_03.Bits.c_rand_cost)
        .FastLogD("c_bg_mv_th                 %d", tuningBuffer->DVS_ME_04.Bits.c_bg_mv_th)
        .FastLogD("c_bg_mv_pnlty_sel          %d", tuningBuffer->DVS_ME_04.Bits.c_bg_mv_pnlty_sel)
        .FastLogD("c_bg_mv_cost               %d", tuningBuffer->DVS_ME_04.Bits.c_bg_mv_cost)
        .FastLogD("c_bg_mv_diff               %d", tuningBuffer->DVS_ME_04.Bits.c_bg_mv_diff)
        .FastLogD("c_conf_div                 %d", tuningBuffer->DVS_ME_04.Bits.c_conf_div)
        .FastLogD("c_seed_l                   % 2d % 2d % 2d % 2d % 2d % 2d % 2d % 2d",
                    tuningBuffer->DVS_ME_05.Bits.c_seed_l_c00,
                    tuningBuffer->DVS_ME_05.Bits.c_seed_l_c01,
                    tuningBuffer->DVS_ME_05.Bits.c_seed_l_c02,
                    tuningBuffer->DVS_ME_05.Bits.c_seed_l_c03,
                    tuningBuffer->DVS_ME_05.Bits.c_seed_l_c04,
                    tuningBuffer->DVS_ME_06.Bits.c_seed_l_c05,
                    tuningBuffer->DVS_ME_06.Bits.c_seed_l_c06,
                    tuningBuffer->DVS_ME_06.Bits.c_seed_l_c07)
        .FastLogD("c_seed_r                   % 2d % 2d % 2d % 2d % 2d % 2d % 2d % 2d",
                    tuningBuffer->DVS_ME_06.Bits.c_seed_r_c00,
                    tuningBuffer->DVS_ME_06.Bits.c_seed_r_c01,
                    tuningBuffer->DVS_ME_07.Bits.c_seed_r_c02,
                    tuningBuffer->DVS_ME_07.Bits.c_seed_r_c03,
                    tuningBuffer->DVS_ME_07.Bits.c_seed_r_c04,
                    tuningBuffer->DVS_ME_07.Bits.c_seed_r_c05,
                    tuningBuffer->DVS_ME_07.Bits.c_seed_r_c06,
                    tuningBuffer->DVS_ME_08.Bits.c_seed_r_c07)
        .FastLogD("c_dv_ini_x                 %d", tuningBuffer->DVS_ME_08.Bits.c_dv_ini_x)
        .FastLogD("c_gmv_y1                   %d", tuningBuffer->DVS_ME_08.Bits.c_gmv_y1)
        .FastLogD("c_gmv_x1                   %d", tuningBuffer->DVS_ME_08.Bits.c_gmv_x1)
        .FastLogD("c_type_l                   %d %d %d %d %d %d %d %d",
                    tuningBuffer->DVS_ME_09.Bits.c_type_l_c00,
                    tuningBuffer->DVS_ME_09.Bits.c_type_l_c01,
                    tuningBuffer->DVS_ME_09.Bits.c_type_l_c02,
                    tuningBuffer->DVS_ME_09.Bits.c_type_l_c03,
                    tuningBuffer->DVS_ME_09.Bits.c_type_l_c04,
                    tuningBuffer->DVS_ME_09.Bits.c_type_l_c05,
                    tuningBuffer->DVS_ME_09.Bits.c_type_l_c06,
                    tuningBuffer->DVS_ME_09.Bits.c_type_l_c07)
        .FastLogD("c_type_r                   %d %d %d %d %d %d %d %d",
                    tuningBuffer->DVS_ME_10.Bits.c_type_r_c00,
                    tuningBuffer->DVS_ME_10.Bits.c_type_r_c01,
                    tuningBuffer->DVS_ME_10.Bits.c_type_r_c02,
                    tuningBuffer->DVS_ME_10.Bits.c_type_r_c03,
                    tuningBuffer->DVS_ME_10.Bits.c_type_r_c04,
                    tuningBuffer->DVS_ME_10.Bits.c_type_r_c05,
                    tuningBuffer->DVS_ME_10.Bits.c_type_r_c06,
                    tuningBuffer->DVS_ME_10.Bits.c_type_r_c07)
        // .FastLogD("sta_shift_h                %d", tuningBuffer->DVS_ME_11.Bits.sta_shift_h)
        // .FastLogD("sta_shift_v                %d", tuningBuffer->DVS_ME_11.Bits.sta_shift_v)
        .FastLogD("c_rand_max_mode            %d", tuningBuffer->DVS_ME_12.Bits.c_rand_max_mode)
        .FastLogD("c_rand_round_offset        %d", tuningBuffer->DVS_ME_12.Bits.c_rand_round_offset)
        .FastLogD("c_rand_init                %d", tuningBuffer->DVS_ME_12.Bits.c_rand_init)
        .FastLogD("c_rand1_vec_base_mv_shift  %d", tuningBuffer->DVS_ME_12.Bits.c_rand1_vec_base_mv_shift)
        .FastLogD("c_rand2_vec_base_mv_shift  %d", tuningBuffer->DVS_ME_12.Bits.c_rand2_vec_base_mv_shift)
        .FastLogD("c_refine_vec_base_mv_shift %d", tuningBuffer->DVS_ME_12.Bits.c_refine_vec_base_mv_shift)
        .FastLogD("c_rand3_vec_base_mv_shift  %d", tuningBuffer->DVS_ME_12.Bits.c_rand3_vec_base_mv_shift)
        .FastLogD("c_rand_dv1-4               %d %d %d %d",
                    tuningBuffer->DVS_ME_14.Bits.c_rand_dv1,
                    tuningBuffer->DVS_ME_14.Bits.c_rand_dv2,
                    tuningBuffer->DVS_ME_14.Bits.c_rand_dv3,
                    tuningBuffer->DVS_ME_13.Bits.c_rand_dv4)
        .FastLogD("c_ad_shift                 %d", tuningBuffer->DVS_ME_15.Bits.c_ad_shift)
        .FastLogD("c_lut_ad[00-07]            % 3d % 3d % 3d % 3d % 3d % 3d % 3d % 3d",
                    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c00,
                    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c01,
                    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c02,
                    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c03,
                    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c04,
                    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c05,
                    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c06,
                    tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c07)
        .FastLogD("c_lut_ad[08-15]            % 3d % 3d % 3d % 3d % 3d % 3d % 3d % 3d",
                    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c08,
                    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c09,
                    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c10,
                    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c11,
                    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c12,
                    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c13,
                    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c14,
                    tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c15)
        .FastLogD("c_seed_priority_r          %d", tuningBuffer->DVS_ME_18.Bits.c_seed_priority_r)
        .FastLogD("c_seed_priority_l          %d", tuningBuffer->DVS_ME_18.Bits.c_seed_priority_l)
        .FastLogD("c_sr_h                     %d", tuningBuffer->DVS_ME_19.Bits.c_sr_h)
        .FastLogD("c_sr_v                     %d", tuningBuffer->DVS_ME_19.Bits.c_sr_v)
        .FastLogD("c_zrv                      %d", tuningBuffer->DVS_ME_19.Bits.c_zrv)
        // .FastLogD("c_var_frm_th               %d", tuningBuffer->DVS_ME_20.Bits.c_var_frm_th)
        // .FastLogD("c_coner_th                 %d", tuningBuffer->DVS_ME_20.Bits.c_coner_th)
        // .FastLogD("c_sr_h_warp                %d", tuningBuffer->DVS_ME_20.Bits.c_sr_h_warp)
        .FastLogD("c_lp_box_en                %d", tuningBuffer->DVS_ME_21.Bits.c_lp_box_en)
        .FastLogD("c_as_en                    %d", tuningBuffer->DVS_ME_21.Bits.c_as_en)
        .FastLogD("c_conf_mode                %d", tuningBuffer->DVS_ME_21.Bits.c_conf_mode)
        .FastLogD("c_as_th                    %d", tuningBuffer->DVS_ME_21.Bits.c_as_th)
        .FastLogD("c_lut_cmlm_step            %d", tuningBuffer->DVS_ME_26.Bits.c_lut_cmlm_step)
        .FastLogD("c_lut_cmlm[00-07]          % 4d % 4d % 4d % 4d % 4d % 4d % 4d % 4d",
                    tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c00,
                    tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c01,
                    tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c02,
                    tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c03,
                    tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c04,
                    tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c05,
                    tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c06,
                    tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c07)
        .FastLogD("c_lut_cmlm[08-16]          % 4d % 4d % 4d % 4d % 4d % 4d % 4d % 4d % 4d",
                    tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c08,
                    tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c09,
                    tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c10,
                    tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c11,
                    tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c12,
                    tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c13,
                    tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c14,
                    tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c15,
                    tuningBuffer->DVS_ME_26.Bits.c_lut_cmlm_c16)
        .FastLogD("c_lut_conf[00-07]          %d %d %d %d %d %d %d %d",
                    tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c00,
                    tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c01,
                    tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c02,
                    tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c03,
                    tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c04,
                    tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c05,
                    tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c06,
                    tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c07)
        .FastLogD("c_lut_conf[08-15]          %d %d %d %d %d %d %d %d",
                    tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c08,
                    tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c09,
                    tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c10,
                    tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c11,
                    tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c12,
                    tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c13,
                    tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c14,
                    tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c15)
        .FastLogD("c_bgcmmn                   %d", tuningBuffer->DVS_ME_31.Bits.c_bgcmmn)
        .FastLogD("c_bgcpkrn                  %d", tuningBuffer->DVS_ME_31.Bits.c_bgcpkrn)
        .FastLogD("c_bgclrd                   %d", tuningBuffer->DVS_ME_31.Bits.c_bgclrd)
        .FastLogD("c_bgcmlm                   %d", tuningBuffer->DVS_ME_31.Bits.c_bgcmlm)
        .FastLogD("c_wgtcmmn                  %d", tuningBuffer->DVS_ME_32.Bits.c_wgtcmmn)
        .FastLogD("c_wgtcpkrn                 %d", tuningBuffer->DVS_ME_32.Bits.c_wgtcpkrn)
        .FastLogD("c_wgtclrd                  %d", tuningBuffer->DVS_ME_32.Bits.c_wgtclrd)
        .FastLogD("c_wgtcmlm                  %d", tuningBuffer->DVS_ME_32.Bits.c_wgtcmlm)
        .FastLogD("c_divwgt                   %d", tuningBuffer->DVS_ME_32.Bits.c_divwgt)
        .FastLogD("c_cost_validnum_th         %d", tuningBuffer->DVS_ME_32.Bits.c_cost_validnum_th)
        .FastLogD("-----------------------------");
    }
    logger.print();
}

void
HW_DPE_DVS_ME_Tuning::_loadValuesFromDocument(const json& dpeValues)
{
    for(auto &tuning : dpeValues) {
        DVS_ME_CFG *tuningBuffer = NULL;
        std::string scenarioKey = tuning[VALUE_KEY_PARAMETERS][QUERY_KEY_SCENARIO].get<std::string>();
        if(__tuningMap.find(scenarioKey) == __tuningMap.end()) {
            tuningBuffer = new DVS_ME_CFG();
            __tuningMap[scenarioKey] = tuningBuffer;
        } else {
            tuningBuffer = __tuningMap[scenarioKey];
        }

        ENUM_STEREO_SCENARIO scenario = eSTEREO_SCENARIO_PREVIEW;
        // std::transform(scenarioKey.begin(), scenarioKey.end(), scenarioKey.begin(), ::tolower);
        // if(std::string::npos != scenarioKey.find("cap")) {
        //     scenario = eSTEREO_SCENARIO_CAPTURE;
        // }

        const json &tuningValues = tuning[VALUE_KEY_VALUES];
        _initBBox(tuningBuffer, scenario);

        #define CUSTOM_VALUE(json, key, value) \
            if(LogicalCamJSONUtil::HasMember(json, key)) { \
                value = _getInt(tuningValues, key); \
            }
        CUSTOM_VALUE(tuningValues, "c_lp_box_l_left",   tuningBuffer->DVS_ME_33.Bits.c_lp_box_l_left);
        CUSTOM_VALUE(tuningValues, "c_lp_box_l_right",  tuningBuffer->DVS_ME_33.Bits.c_lp_box_l_right);
        CUSTOM_VALUE(tuningValues, "c_lp_box_l_top",    tuningBuffer->DVS_ME_34.Bits.c_lp_box_l_top);
        CUSTOM_VALUE(tuningValues, "c_lp_box_l_bottom", tuningBuffer->DVS_ME_34.Bits.c_lp_box_l_bottom);
        CUSTOM_VALUE(tuningValues, "c_lp_box_r_left",   tuningBuffer->DVS_ME_35.Bits.c_lp_box_r_left);
        CUSTOM_VALUE(tuningValues, "c_lp_box_r_right",  tuningBuffer->DVS_ME_35.Bits.c_lp_box_r_right);
        CUSTOM_VALUE(tuningValues, "c_lp_box_r_top",    tuningBuffer->DVS_ME_36.Bits.c_lp_box_r_top);
        CUSTOM_VALUE(tuningValues, "c_lp_box_r_bottom", tuningBuffer->DVS_ME_36.Bits.c_lp_box_r_bottom);

        tuningBuffer->DVS_ME_00.Bits.c_dv_cand_num         = _getInt(tuningValues, "c_dv_cand_num");
        tuningBuffer->DVS_ME_00.Bits.c_ord_pnlty_shift_dir = _getInt(tuningValues, "c_ord_pnlty_shift_dir");
        tuningBuffer->DVS_ME_00.Bits.c_ord_pnlty_shift_num = _getInt(tuningValues, "c_ord_pnlty_shift_num");
        tuningBuffer->DVS_ME_00.Bits.c_ord_th              = _getInt(tuningValues, "c_ord_th");
        tuningBuffer->DVS_ME_00.Bits.c_ord_coring          = _getInt(tuningValues, "c_ord_coring");
        tuningBuffer->DVS_ME_01.Bits.c_sm_dv_x_pnlty_sel   = _getInt(tuningValues, "c_sm_dv_x_pnlty_sel");
        tuningBuffer->DVS_ME_01.Bits.c_sm_dv_y_pnlty_sel   = _getInt(tuningValues, "c_sm_dv_y_pnlty_sel");
        tuningBuffer->DVS_ME_01.Bits.c_sm_dv_ada_base      = _getInt(tuningValues, "c_sm_dv_ada_base");
        tuningBuffer->DVS_ME_01.Bits.c_sm_dv_th            = _getInt(tuningValues, "c_sm_dv_th");
        tuningBuffer->DVS_ME_01.Bits.c_sm_dv_th2           = _getInt(tuningValues, "c_sm_dv_th2");
        tuningBuffer->DVS_ME_02.Bits.c_sptl_pnlty_sel      = _getInt(tuningValues, "c_sptl_pnlty_sel");
        tuningBuffer->DVS_ME_02.Bits.c_tmpr_pnlty_sel      = _getInt(tuningValues, "c_tmpr_pnlty_sel");
        tuningBuffer->DVS_ME_02.Bits.c_rand_pnlty_sel      = _getInt(tuningValues, "c_rand_pnlty_sel");
        tuningBuffer->DVS_ME_02.Bits.c_nbr_pnlty_sel       = _getInt(tuningValues, "c_nbr_pnlty_sel");
        tuningBuffer->DVS_ME_02.Bits.c_prev_pnlty_sel      = _getInt(tuningValues, "c_prev_pnlty_sel");
        tuningBuffer->DVS_ME_02.Bits.c_gmv_pnlty_sel       = _getInt(tuningValues, "c_gmv_pnlty_sel");
        tuningBuffer->DVS_ME_02.Bits.c_refine_pnlty_sel    = _getInt(tuningValues, "c_refine_pnlty_sel");
        tuningBuffer->DVS_ME_03.Bits.c_sptl_cost           = _getInt(tuningValues, "c_sptl_cost");
        tuningBuffer->DVS_ME_03.Bits.c_tmpr_cost           = _getInt(tuningValues, "c_tmpr_cost");
        tuningBuffer->DVS_ME_03.Bits.c_refine_cost         = _getInt(tuningValues, "c_refine_cost");
        tuningBuffer->DVS_ME_03.Bits.c_nbr_cost            = _getInt(tuningValues, "c_nbr_cost");
        tuningBuffer->DVS_ME_03.Bits.c_prev_cost           = _getInt(tuningValues, "c_prev_cost");
        tuningBuffer->DVS_ME_03.Bits.c_gmv_cost            = _getInt(tuningValues, "c_gmv_cost");
        tuningBuffer->DVS_ME_03.Bits.c_rand_cost           = _getInt(tuningValues, "c_rand_cost");
        tuningBuffer->DVS_ME_04.Bits.c_bg_mv_th            = _getInt(tuningValues, "c_bg_mv_th");
        tuningBuffer->DVS_ME_04.Bits.c_bg_mv_pnlty_sel     = _getInt(tuningValues, "c_bg_mv_pnlty_sel");
        tuningBuffer->DVS_ME_04.Bits.c_bg_mv_cost          = _getInt(tuningValues, "c_bg_mv_cost");
        tuningBuffer->DVS_ME_04.Bits.c_bg_mv_diff          = _getInt(tuningValues, "c_bg_mv_diff");
        tuningBuffer->DVS_ME_04.Bits.c_conf_div            = _getInt(tuningValues, "c_conf_div");

        if(_checkArrayAndSize(tuningValues, "c_seed_l", 8)) {
            const json &arrayValue = tuningValues["c_seed_l"];
            tuningBuffer->DVS_ME_05.Bits.c_seed_l_c00 = arrayValue[0];
            tuningBuffer->DVS_ME_05.Bits.c_seed_l_c01 = arrayValue[1];
            tuningBuffer->DVS_ME_05.Bits.c_seed_l_c02 = arrayValue[2];
            tuningBuffer->DVS_ME_05.Bits.c_seed_l_c03 = arrayValue[3];
            tuningBuffer->DVS_ME_05.Bits.c_seed_l_c04 = arrayValue[4];
            tuningBuffer->DVS_ME_06.Bits.c_seed_l_c05 = arrayValue[5];
            tuningBuffer->DVS_ME_06.Bits.c_seed_l_c06 = arrayValue[6];
            tuningBuffer->DVS_ME_06.Bits.c_seed_l_c07 = arrayValue[7];
        } else {
            tuningBuffer->DVS_ME_05.Bits.c_seed_l_c04 = _getInt(tuningValues, "c_seed_l_c04");
            tuningBuffer->DVS_ME_05.Bits.c_seed_l_c03 = _getInt(tuningValues, "c_seed_l_c03");
            tuningBuffer->DVS_ME_05.Bits.c_seed_l_c02 = _getInt(tuningValues, "c_seed_l_c02");
            tuningBuffer->DVS_ME_05.Bits.c_seed_l_c01 = _getInt(tuningValues, "c_seed_l_c01");
            tuningBuffer->DVS_ME_05.Bits.c_seed_l_c00 = _getInt(tuningValues, "c_seed_l_c00");
            tuningBuffer->DVS_ME_06.Bits.c_seed_l_c07 = _getInt(tuningValues, "c_seed_l_c07");
            tuningBuffer->DVS_ME_06.Bits.c_seed_l_c06 = _getInt(tuningValues, "c_seed_l_c06");
            tuningBuffer->DVS_ME_06.Bits.c_seed_l_c05 = _getInt(tuningValues, "c_seed_l_c05");
        }

        if(_checkArrayAndSize(tuningValues, "c_seed_r", 8)) {
            const json &arrayValue = tuningValues["c_seed_r"];
            tuningBuffer->DVS_ME_06.Bits.c_seed_r_c00 = arrayValue[0];
            tuningBuffer->DVS_ME_06.Bits.c_seed_r_c01 = arrayValue[1];
            tuningBuffer->DVS_ME_07.Bits.c_seed_r_c02 = arrayValue[2];
            tuningBuffer->DVS_ME_07.Bits.c_seed_r_c03 = arrayValue[3];
            tuningBuffer->DVS_ME_07.Bits.c_seed_r_c04 = arrayValue[4];
            tuningBuffer->DVS_ME_07.Bits.c_seed_r_c05 = arrayValue[5];
            tuningBuffer->DVS_ME_07.Bits.c_seed_r_c06 = arrayValue[6];
            tuningBuffer->DVS_ME_08.Bits.c_seed_r_c07 = arrayValue[7];
        } else {
            tuningBuffer->DVS_ME_06.Bits.c_seed_r_c01 = _getInt(tuningValues, "c_seed_r_c01");
            tuningBuffer->DVS_ME_06.Bits.c_seed_r_c00 = _getInt(tuningValues, "c_seed_r_c00");
            tuningBuffer->DVS_ME_07.Bits.c_seed_r_c06 = _getInt(tuningValues, "c_seed_r_c06");
            tuningBuffer->DVS_ME_07.Bits.c_seed_r_c05 = _getInt(tuningValues, "c_seed_r_c05");
            tuningBuffer->DVS_ME_07.Bits.c_seed_r_c04 = _getInt(tuningValues, "c_seed_r_c04");
            tuningBuffer->DVS_ME_07.Bits.c_seed_r_c03 = _getInt(tuningValues, "c_seed_r_c03");
            tuningBuffer->DVS_ME_07.Bits.c_seed_r_c02 = _getInt(tuningValues, "c_seed_r_c02");
            tuningBuffer->DVS_ME_08.Bits.c_seed_r_c07 = _getInt(tuningValues, "c_seed_r_c07");
        }
        tuningBuffer->DVS_ME_08.Bits.c_dv_ini_x = _getInt(tuningValues, "c_dv_ini_x");
        tuningBuffer->DVS_ME_08.Bits.c_gmv_y1   = _getInt(tuningValues, "c_gmv_y1");
        tuningBuffer->DVS_ME_08.Bits.c_gmv_x1   = _getInt(tuningValues, "c_gmv_x1");

        if(_checkArrayAndSize(tuningValues, "c_type_l", 8)) {
            const json &arrayValue = tuningValues["c_type_l"];
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c00 = arrayValue[0];
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c01 = arrayValue[1];
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c02 = arrayValue[2];
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c03 = arrayValue[3];
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c04 = arrayValue[4];
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c05 = arrayValue[5];
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c06 = arrayValue[6];
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c07 = arrayValue[7];
        } else {
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c07 = _getInt(tuningValues, "c_type_l_c07");
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c06 = _getInt(tuningValues, "c_type_l_c06");
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c05 = _getInt(tuningValues, "c_type_l_c05");
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c04 = _getInt(tuningValues, "c_type_l_c04");
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c03 = _getInt(tuningValues, "c_type_l_c03");
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c02 = _getInt(tuningValues, "c_type_l_c02");
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c01 = _getInt(tuningValues, "c_type_l_c01");
            tuningBuffer->DVS_ME_09.Bits.c_type_l_c00 = _getInt(tuningValues, "c_type_l_c00");
        }

        if(_checkArrayAndSize(tuningValues, "c_type_r", 8)) {
            const json &arrayValue = tuningValues["c_type_r"];
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c00 = arrayValue[0];
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c01 = arrayValue[1];
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c02 = arrayValue[2];
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c03 = arrayValue[3];
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c04 = arrayValue[4];
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c05 = arrayValue[5];
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c06 = arrayValue[6];
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c07 = arrayValue[7];
        } else {
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c07 = _getInt(tuningValues, "c_type_r_c07");
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c06 = _getInt(tuningValues, "c_type_r_c06");
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c05 = _getInt(tuningValues, "c_type_r_c05");
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c04 = _getInt(tuningValues, "c_type_r_c04");
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c03 = _getInt(tuningValues, "c_type_r_c03");
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c02 = _getInt(tuningValues, "c_type_r_c02");
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c01 = _getInt(tuningValues, "c_type_r_c01");
            tuningBuffer->DVS_ME_10.Bits.c_type_r_c00 = _getInt(tuningValues, "c_type_r_c00");
        }

        // tuningBuffer->DVS_ME_11.Bits.sta_shift_h                = _getInt(tuningValues, "sta_shift_h");
        // tuningBuffer->DVS_ME_11.Bits.sta_shift_v                = _getInt(tuningValues, "sta_shift_v");
        tuningBuffer->DVS_ME_12.Bits.c_rand_max_mode            = _getInt(tuningValues, "c_rand_max_mode");
        tuningBuffer->DVS_ME_12.Bits.c_rand_round_offset        = _getInt(tuningValues, "c_rand_round_offset");
        tuningBuffer->DVS_ME_12.Bits.c_rand_init                = _getInt(tuningValues, "c_rand_init");
        tuningBuffer->DVS_ME_12.Bits.c_rand1_vec_base_mv_shift  = _getInt(tuningValues, "c_rand1_vec_base_mv_shift");
        tuningBuffer->DVS_ME_12.Bits.c_rand2_vec_base_mv_shift  = _getInt(tuningValues, "c_rand2_vec_base_mv_shift");
        tuningBuffer->DVS_ME_12.Bits.c_refine_vec_base_mv_shift = _getInt(tuningValues, "c_refine_vec_base_mv_shift");
        tuningBuffer->DVS_ME_12.Bits.c_rand3_vec_base_mv_shift  = _getInt(tuningValues, "c_rand3_vec_base_mv_shift");

        if(_checkArrayAndSize(tuningValues, "c_rand_dv", 4)) {
            const json &arrayValue = tuningValues["c_rand_dv"];
            tuningBuffer->DVS_ME_14.Bits.c_rand_dv1 = arrayValue[0];
            tuningBuffer->DVS_ME_14.Bits.c_rand_dv2 = arrayValue[1];
            tuningBuffer->DVS_ME_14.Bits.c_rand_dv3 = arrayValue[2];
            tuningBuffer->DVS_ME_13.Bits.c_rand_dv4 = arrayValue[3];
        } else {
            tuningBuffer->DVS_ME_13.Bits.c_rand_dv4 = _getInt(tuningValues, "c_rand_dv4");
            tuningBuffer->DVS_ME_14.Bits.c_rand_dv1 = _getInt(tuningValues, "c_rand_dv1");
            tuningBuffer->DVS_ME_14.Bits.c_rand_dv2 = _getInt(tuningValues, "c_rand_dv2");
            tuningBuffer->DVS_ME_14.Bits.c_rand_dv3 = _getInt(tuningValues, "c_rand_dv3");
        }

        tuningBuffer->DVS_ME_15.Bits.c_ad_shift                 = _getInt(tuningValues, "c_ad_shift");

        if(_checkArrayAndSize(tuningValues, "c_lut_ad", 16)) {
            const json &arrayValue = tuningValues["c_lut_ad"];
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c00 = arrayValue[0];
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c01 = arrayValue[1];
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c02 = arrayValue[2];
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c03 = arrayValue[3];
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c04 = arrayValue[4];
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c05 = arrayValue[5];
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c06 = arrayValue[6];
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c07 = arrayValue[7];
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c08 = arrayValue[8];
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c09 = arrayValue[9];
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c10 = arrayValue[10];
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c11 = arrayValue[11];
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c12 = arrayValue[12];
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c13 = arrayValue[13];
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c14 = arrayValue[14];
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c15 = arrayValue[15];
        } else {
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c07 = _getInt(tuningValues, "c_lut_ad_c07");
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c06 = _getInt(tuningValues, "c_lut_ad_c06");
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c05 = _getInt(tuningValues, "c_lut_ad_c05");
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c04 = _getInt(tuningValues, "c_lut_ad_c04");
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c03 = _getInt(tuningValues, "c_lut_ad_c03");
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c02 = _getInt(tuningValues, "c_lut_ad_c02");
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c01 = _getInt(tuningValues, "c_lut_ad_c01");
            tuningBuffer->DVS_ME_16.Bits.c_lut_ad_c00 = _getInt(tuningValues, "c_lut_ad_c00");
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c15 = _getInt(tuningValues, "c_lut_ad_c15");
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c14 = _getInt(tuningValues, "c_lut_ad_c14");
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c13 = _getInt(tuningValues, "c_lut_ad_c13");
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c12 = _getInt(tuningValues, "c_lut_ad_c12");
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c11 = _getInt(tuningValues, "c_lut_ad_c11");
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c10 = _getInt(tuningValues, "c_lut_ad_c10");
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c09 = _getInt(tuningValues, "c_lut_ad_c09");
            tuningBuffer->DVS_ME_17.Bits.c_lut_ad_c08 = _getInt(tuningValues, "c_lut_ad_c08");
        }

        tuningBuffer->DVS_ME_18.Bits.c_seed_priority_r = _getInt(tuningValues, "c_seed_priority_r");
        tuningBuffer->DVS_ME_18.Bits.c_seed_priority_l = _getInt(tuningValues, "c_seed_priority_l");
        tuningBuffer->DVS_ME_19.Bits.c_sr_h            = _getInt(tuningValues, "c_sr_h");
        tuningBuffer->DVS_ME_19.Bits.c_sr_v            = _getInt(tuningValues, "c_sr_v");
        tuningBuffer->DVS_ME_19.Bits.c_zrv             = _getInt(tuningValues, "c_zrv");
        // tuningBuffer->DVS_ME_20.Bits.c_var_frm_th      = _getInt(tuningValues, "c_var_frm_th");
        // tuningBuffer->DVS_ME_20.Bits.c_coner_th        = _getInt(tuningValues, "c_coner_th");
        // tuningBuffer->DVS_ME_20.Bits.c_sr_h_warp       = _getInt(tuningValues, "c_sr_h_warp");
        tuningBuffer->DVS_ME_21.Bits.c_lp_box_en       = _getInt(tuningValues, "c_lp_box_en");
        tuningBuffer->DVS_ME_21.Bits.c_as_en           = _getInt(tuningValues, "c_as_en");
        tuningBuffer->DVS_ME_21.Bits.c_conf_mode       = _getInt(tuningValues, "c_conf_mode");
        tuningBuffer->DVS_ME_21.Bits.c_as_th           = _getInt(tuningValues, "c_as_th");

        tuningBuffer->DVS_ME_26.Bits.c_lut_cmlm_step   = _getInt(tuningValues, "c_lut_cmlm_step");
        if(_checkArrayAndSize(tuningValues, "c_lut_cmlm", 17)) {
            const json &arrayValue = tuningValues["c_lut_cmlm"];
            tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c00 = arrayValue[0];
            tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c01 = arrayValue[1];
            tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c02 = arrayValue[2];
            tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c03 = arrayValue[3];
            tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c04 = arrayValue[4];
            tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c05 = arrayValue[5];
            tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c06 = arrayValue[6];
            tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c07 = arrayValue[7];
            tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c08 = arrayValue[8];
            tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c09 = arrayValue[9];
            tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c10 = arrayValue[10];
            tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c11 = arrayValue[11];
            tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c12 = arrayValue[12];
            tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c13 = arrayValue[13];
            tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c14 = arrayValue[14];
            tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c15 = arrayValue[15];
            tuningBuffer->DVS_ME_26.Bits.c_lut_cmlm_c16 = arrayValue[16];
        } else {
            tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c03 = _getInt(tuningValues, "c_lut_cmlm_c03");
            tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c02 = _getInt(tuningValues, "c_lut_cmlm_c02");
            tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c01 = _getInt(tuningValues, "c_lut_cmlm_c01");
            tuningBuffer->DVS_ME_22.Bits.c_lut_cmlm_c00 = _getInt(tuningValues, "c_lut_cmlm_c00");
            tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c07 = _getInt(tuningValues, "c_lut_cmlm_c07");
            tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c06 = _getInt(tuningValues, "c_lut_cmlm_c06");
            tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c05 = _getInt(tuningValues, "c_lut_cmlm_c05");
            tuningBuffer->DVS_ME_23.Bits.c_lut_cmlm_c04 = _getInt(tuningValues, "c_lut_cmlm_c04");
            tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c11 = _getInt(tuningValues, "c_lut_cmlm_c11");
            tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c10 = _getInt(tuningValues, "c_lut_cmlm_c10");
            tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c09 = _getInt(tuningValues, "c_lut_cmlm_c09");
            tuningBuffer->DVS_ME_24.Bits.c_lut_cmlm_c08 = _getInt(tuningValues, "c_lut_cmlm_c08");
            tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c15 = _getInt(tuningValues, "c_lut_cmlm_c15");
            tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c14 = _getInt(tuningValues, "c_lut_cmlm_c14");
            tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c13 = _getInt(tuningValues, "c_lut_cmlm_c13");
            tuningBuffer->DVS_ME_25.Bits.c_lut_cmlm_c12 = _getInt(tuningValues, "c_lut_cmlm_c12");
            tuningBuffer->DVS_ME_26.Bits.c_lut_cmlm_c16 = _getInt(tuningValues, "c_lut_cmlm_c16");
        }

        if(_checkArrayAndSize(tuningValues, "c_lut_conf", 16)) {
            const json &arrayValue = tuningValues["c_lut_conf"];
            tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c00 = arrayValue[0];
            tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c01 = arrayValue[1];
            tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c02 = arrayValue[2];
            tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c03 = arrayValue[3];
            tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c04 = arrayValue[4];
            tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c05 = arrayValue[5];
            tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c06 = arrayValue[6];
            tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c07 = arrayValue[7];
            tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c08 = arrayValue[8];
            tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c09 = arrayValue[9];
            tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c10 = arrayValue[10];
            tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c11 = arrayValue[11];
            tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c12 = arrayValue[12];
            tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c13 = arrayValue[13];
            tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c14 = arrayValue[14];
            tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c15 = arrayValue[15];
        } else {
            tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c03 = _getInt(tuningValues, "c_lut_conf_c03");
            tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c02 = _getInt(tuningValues, "c_lut_conf_c02");
            tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c01 = _getInt(tuningValues, "c_lut_conf_c01");
            tuningBuffer->DVS_ME_27.Bits.c_lut_conf_c00 = _getInt(tuningValues, "c_lut_conf_c00");
            tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c07 = _getInt(tuningValues, "c_lut_conf_c07");
            tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c06 = _getInt(tuningValues, "c_lut_conf_c06");
            tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c05 = _getInt(tuningValues, "c_lut_conf_c05");
            tuningBuffer->DVS_ME_28.Bits.c_lut_conf_c04 = _getInt(tuningValues, "c_lut_conf_c04");
            tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c11 = _getInt(tuningValues, "c_lut_conf_c11");
            tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c10 = _getInt(tuningValues, "c_lut_conf_c10");
            tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c09 = _getInt(tuningValues, "c_lut_conf_c09");
            tuningBuffer->DVS_ME_29.Bits.c_lut_conf_c08 = _getInt(tuningValues, "c_lut_conf_c08");
            tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c15 = _getInt(tuningValues, "c_lut_conf_c15");
            tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c14 = _getInt(tuningValues, "c_lut_conf_c14");
            tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c13 = _getInt(tuningValues, "c_lut_conf_c13");
            tuningBuffer->DVS_ME_30.Bits.c_lut_conf_c12 = _getInt(tuningValues, "c_lut_conf_c12");
        }

        tuningBuffer->DVS_ME_31.Bits.c_bgcmmn           = _getInt(tuningValues, "c_bgcmmn");
        tuningBuffer->DVS_ME_31.Bits.c_bgcpkrn          = _getInt(tuningValues, "c_bgcpkrn");
        tuningBuffer->DVS_ME_31.Bits.c_bgclrd           = _getInt(tuningValues, "c_bgclrd");
        tuningBuffer->DVS_ME_31.Bits.c_bgcmlm           = _getInt(tuningValues, "c_bgcmlm");
        tuningBuffer->DVS_ME_32.Bits.c_wgtcmmn          = _getInt(tuningValues, "c_wgtcmmn");
        tuningBuffer->DVS_ME_32.Bits.c_wgtcpkrn         = _getInt(tuningValues, "c_wgtcpkrn");
        tuningBuffer->DVS_ME_32.Bits.c_wgtclrd          = _getInt(tuningValues, "c_wgtclrd");
        tuningBuffer->DVS_ME_32.Bits.c_wgtcmlm          = _getInt(tuningValues, "c_wgtcmlm");
        tuningBuffer->DVS_ME_32.Bits.c_divwgt           = _getInt(tuningValues, "c_divwgt");
        tuningBuffer->DVS_ME_32.Bits.c_cost_validnum_th = _getInt(tuningValues, "c_cost_validnum_th");
    }
}

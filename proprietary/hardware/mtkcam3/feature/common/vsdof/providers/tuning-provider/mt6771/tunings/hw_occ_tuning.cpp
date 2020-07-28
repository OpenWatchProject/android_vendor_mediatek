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
#define LOG_TAG "StereoTuningProvider_HWOCC"

#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
#include "hw_occ_tuning.h"

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

bool
HW_OCCTuning::retrieveTuningParams(TuningQuery_T &query)
{
    OCCConfig *pOccConfig = (OCCConfig *)query.results[STEREO_TUNING_NAME[E_TUNING_HW_OCC]];

    if(NULL == pOccConfig) {
        MY_LOGE("Cannot get target address");
        return false;
    }

    OCCConfig *src = NULL;
    ENUM_STEREO_SCENARIO scenario;
    if(query.intParams[QUERY_KEY_SCENARIO] == eSTEREO_SCENARIO_CAPTURE) {
        src = &__capTuning;
        scenario = eSTEREO_SCENARIO_CAPTURE;
    } else {
        src = &__pvTuning;
        scenario = eSTEREO_SCENARIO_PREVIEW;
    }

    pOccConfig->occ_scan_r2l      = src->occ_scan_r2l;
    pOccConfig->occ_horz_ds4      = src->occ_horz_ds4;
    pOccConfig->occ_vert_ds4      = src->occ_vert_ds4;

    pOccConfig->occ_hsize         = src->occ_hsize;
    pOccConfig->occ_vsize         = src->occ_vsize;
    pOccConfig->occ_v_crop_s      = src->occ_v_crop_s;
    pOccConfig->occ_v_crop_e      = src->occ_v_crop_e;
    pOccConfig->occ_h_crop_s      = src->occ_h_crop_s;
    pOccConfig->occ_h_crop_e      = src->occ_h_crop_e;
    pOccConfig->occ_h_skip_mode   = src->occ_h_skip_mode;

    pOccConfig->occ_th_luma       = src->occ_th_luma;
    pOccConfig->occ_th_h          = src->occ_th_h;
    pOccConfig->occ_th_v          = src->occ_th_v;
    pOccConfig->occ_vec_shift     = src->occ_vec_shift;
    pOccConfig->occ_vec_offset    = src->occ_vec_offset;
    pOccConfig->occ_invalid_value = src->occ_invalid_value;
    pOccConfig->occ_owc_th        = src->occ_owc_th;
    pOccConfig->occ_owc_en        = src->occ_owc_en;
    pOccConfig->occ_depth_clip_en = src->occ_depth_clip_en;
    pOccConfig->occ_spare         = src->occ_spare;

    return true;
}

void
HW_OCCTuning::_initDefaultValues()
{
    //__tuningBuffers[0]: Preview/VR
    //__tuningBuffers[1]: Capture
    for(int i = 0; i < SUPPORT_SCENARIO_COUNT; i++) {
        OCCConfig *tuningBuffer = __tuningBuffers[i];

        tuningBuffer->occ_scan_r2l      = StereoSettingProvider::getSensorRelativePosition();
        tuningBuffer->occ_horz_ds4      = 0;
        tuningBuffer->occ_vert_ds4      = 0;

        __updateROI(*tuningBuffer, (0==i)?eSTEREO_SCENARIO_PREVIEW:eSTEREO_SCENARIO_CAPTURE);

        tuningBuffer->occ_th_luma       = 64;
        tuningBuffer->occ_th_h          = 4;
        tuningBuffer->occ_th_v          = 4;
        tuningBuffer->occ_vec_shift     = 0;
        tuningBuffer->occ_vec_offset    = 128;
        tuningBuffer->occ_invalid_value = DEPTH_MASK_VALUE;
        tuningBuffer->occ_owc_th        = 3;
        tuningBuffer->occ_owc_en        = 1;
        tuningBuffer->occ_depth_clip_en = 1;
        tuningBuffer->occ_spare         = 0;
    }
}

void
HW_OCCTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("===== HW OCC Parameters =====");
    for(int i = 0; i < SUPPORT_SCENARIO_COUNT; i++) {
        NSCam::NSIoPipe::OCCConfig *tuningBuffer = __tuningBuffers[i];

        logger
        .FastLogD("Scenario:             %s", SCENARIO_NAMES[__scenarios[i]])
        .FastLogD("occ_scan_r2l          %d", tuningBuffer->occ_scan_r2l)
        .FastLogD("occ_horz_ds4          %d", tuningBuffer->occ_horz_ds4)
        .FastLogD("occ_vert_ds4          %d", tuningBuffer->occ_vert_ds4)
        .FastLogD("occ_h_skip_mode       %d", tuningBuffer->occ_h_skip_mode)

        .FastLogD("occ_hsize             %d", tuningBuffer->occ_hsize)
        .FastLogD("occ_vsize             %d", tuningBuffer->occ_vsize)
        .FastLogD("occ_v_crop_s          %d", tuningBuffer->occ_v_crop_s)
        .FastLogD("occ_v_crop_e          %d", tuningBuffer->occ_v_crop_e)
        .FastLogD("occ_h_crop_s          %d", tuningBuffer->occ_h_crop_s)
        .FastLogD("occ_h_crop_e          %d", tuningBuffer->occ_h_crop_e)

        .FastLogD("occ_th_luma           %d", tuningBuffer->occ_th_luma)
        .FastLogD("occ_th_h              %d", tuningBuffer->occ_th_h)
        .FastLogD("occ_th_v              %d", tuningBuffer->occ_th_v)
        .FastLogD("occ_vec_shift         %d", tuningBuffer->occ_vec_shift)
        .FastLogD("occ_vec_offset        %d", tuningBuffer->occ_vec_offset)
        .FastLogD("occ_invalid_value     %d", tuningBuffer->occ_invalid_value)
        .FastLogD("occ_owc_th            %d", tuningBuffer->occ_owc_th)
        .FastLogD("occ_owc_en            %d", tuningBuffer->occ_owc_en)
        .FastLogD("occ_depth_clip_en     %d", tuningBuffer->occ_depth_clip_en)
        .FastLogD("occ_spare             %d", tuningBuffer->occ_spare)
        .FastLogD("===========================");
    }
    logger.print();
}

void
HW_OCCTuning::_loadValuesFromDocument(const json &occValues)
{
    OCCConfig *tuningBuffer;
    ENUM_STEREO_SCENARIO scenario;
    for(auto &occValue : occValues) {
        if(occValue[VALUE_KEY_PARAMETERS][QUERY_KEY_SCENARIO] == SCENARIO_NAMES[__scenarios[0]])
        {
            tuningBuffer = &__pvTuning;
            scenario = eSTEREO_SCENARIO_PREVIEW;
            tuningBuffer->occ_h_skip_mode = StereoSettingProvider::isMTKDepthmapMode() ? 0 : 1;
        } else {
            tuningBuffer = &__capTuning;
            scenario = eSTEREO_SCENARIO_CAPTURE;
            tuningBuffer->occ_h_skip_mode = 0;
        }

        const json &tuningValues = occValue[VALUE_KEY_VALUES];

        tuningBuffer->occ_scan_r2l      = StereoSettingProvider::getSensorRelativePosition();

        tuningBuffer->occ_horz_ds4      = _getInt(tuningValues, "occ_horz_ds4");
        tuningBuffer->occ_vert_ds4      = _getInt(tuningValues, "occ_vert_ds4");

        __updateROI(*tuningBuffer, scenario);

        tuningBuffer->occ_th_luma       = _getInt(tuningValues, "occ_th_luma");
        tuningBuffer->occ_th_h          = _getInt(tuningValues, "occ_th_h");
        tuningBuffer->occ_th_v          = _getInt(tuningValues, "occ_th_v");
        tuningBuffer->occ_vec_shift     = _getInt(tuningValues, "occ_vec_shift");
        tuningBuffer->occ_vec_offset    = _getInt(tuningValues, "occ_vec_offset");
        tuningBuffer->occ_invalid_value = _getInt(tuningValues, "occ_invalid_value");
        tuningBuffer->occ_owc_th        = _getInt(tuningValues, "occ_owc_th");
        tuningBuffer->occ_owc_en        = _getInt(tuningValues, "occ_owc_en");
        tuningBuffer->occ_depth_clip_en = _getInt(tuningValues, "occ_depth_clip_en");
        tuningBuffer->occ_spare         = _getInt(tuningValues, "occ_spare");
    }
}

void
HW_OCCTuning::__updateROI(OCCConfig &config, ENUM_STEREO_SCENARIO scenario)
{
    StereoArea area = (eSTEREO_SCENARIO_PREVIEW == scenario)
                      ? StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y, scenario)
                      : StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y_LARGE, scenario);

    config.occ_hsize    = area.size.w;
    config.occ_vsize    = area.size.h;
    config.occ_v_crop_s = area.startPt.y;
    config.occ_v_crop_e = area.size.h - area.startPt.y - 1;
    config.occ_h_crop_s = area.startPt.x;
    config.occ_h_crop_e = area.size.w - area.startPt.x - 1;
}

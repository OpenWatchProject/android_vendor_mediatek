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
#define LOG_TAG "StereoTuningProvider_HWDPE"

#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
#include "hw_dpe_tuning.h"

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
HW_DPETuning::retrieveTuningParams(TuningQuery_T &query)
{
    void *targetAddr = query.results[STEREO_TUNING_NAME[E_TUNING_HW_DPE]];

    if(NULL == targetAddr) {
        MY_LOGE("Cannot get target address");
        return false;
    }

    if(query.intParams[QUERY_KEY_SCENARIO] == eSTEREO_SCENARIO_CAPTURE) {
        ::memcpy(targetAddr, &__capTuning, sizeof(DVEConfig));
    } else {
        ::memcpy(targetAddr, &__pvTuning, sizeof(DVEConfig));
    }

    return true;
}

void
HW_DPETuning::_initDefaultValues()
{
    //__tuningBuffers[0]: Preview/VR
    //__tuningBuffers[1]: Capture
    //__tuningBuffers[2]: Active Stereo
    for(int i = 0; i < E_DPE_SCENARIO_COUNT; i++) {
        NSCam::NSIoPipe::DVEConfig *tuningBuffer = __tuningBuffers[i];
        tuningBuffer->Dve_l_Bbox_En     = 1;
        tuningBuffer->Dve_r_Bbox_En     = 1;
        tuningBuffer->Dve_Mask_En       = 1;

        tuningBuffer->Dve_Org_Horz_Sr_0 = 255;
        tuningBuffer->Dve_Org_Horz_Sr_1 = 128;

        __initBBox(tuningBuffer, STEREO_SCANARIO[i]);

        tuningBuffer->Dve_Horz_Ds_Mode = 0;
        tuningBuffer->Dve_Vert_Ds_Mode = 0;

        tuningBuffer->Dve_Cand_Num              = 7;
        tuningBuffer->Dve_Cand_0.DVE_CAND_SEL   = 0xB;
        tuningBuffer->Dve_Cand_0.DVE_CAND_TYPE  = 0x1;
        tuningBuffer->Dve_Cand_1.DVE_CAND_SEL   = 0x12;
        tuningBuffer->Dve_Cand_1.DVE_CAND_TYPE  = 0x2;
        tuningBuffer->Dve_Cand_2.DVE_CAND_SEL   = 0x7;
        tuningBuffer->Dve_Cand_2.DVE_CAND_TYPE  = 0x1;
        tuningBuffer->Dve_Cand_3.DVE_CAND_SEL   = 0x1F;
        tuningBuffer->Dve_Cand_3.DVE_CAND_TYPE  = 0x6;
        tuningBuffer->Dve_Cand_4.DVE_CAND_SEL   = 0x19;
        tuningBuffer->Dve_Cand_4.DVE_CAND_TYPE  = 0x3;
        tuningBuffer->Dve_Cand_5.DVE_CAND_SEL   = 0x1A;
        tuningBuffer->Dve_Cand_5.DVE_CAND_TYPE  = 0x3;
        tuningBuffer->Dve_Cand_6.DVE_CAND_SEL   = 0x1C;
        tuningBuffer->Dve_Cand_6.DVE_CAND_TYPE  = 0x4;
        tuningBuffer->Dve_Cand_7.DVE_CAND_SEL   = 0x1C;
        tuningBuffer->Dve_Cand_7.DVE_CAND_TYPE  = 0x4;
        tuningBuffer->DVE_VERT_GMV = 0;
        tuningBuffer->DVE_HORZ_GMV = 0;

        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_RAND_COST   = 24;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_GMV_COST    = 3;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_PREV_COST   = 3;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_NBR_COST    = 5;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_REFINE_COST = 3;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_TMPR_COST   = 5;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_SPTL_COST   = 1;

        //Other default values
        tuningBuffer->Dve_Skp_Pre_Dv                              = false;
        tuningBuffer->Dve_Org_Vert_Sr_0                           = 24;
        tuningBuffer->Dve_Org_Start_Vert_Sv                       = 0;
        tuningBuffer->Dve_Org_Start_Horz_Sv                       = 0;
        tuningBuffer->Dve_Rand_Lut_0                              = 11;
        tuningBuffer->Dve_Rand_Lut_1                              = 23;
        tuningBuffer->Dve_Rand_Lut_2                              = 31;
        tuningBuffer->Dve_Rand_Lut_3                              = 43;
        tuningBuffer->Dve_Horz_Dv_Ini                             = 0;
        tuningBuffer->Dve_Coft_Shift                              = 2;
        tuningBuffer->Dve_Corner_Th                               = 16;
        tuningBuffer->Dve_Smth_Luma_Th_1                          = 0;
        tuningBuffer->Dve_Smth_Luma_Th_0                          = 12;
        tuningBuffer->Dve_Smth_Luma_Ada_Base                      = 0;
        tuningBuffer->Dve_Smth_Luma_Horz_Pnlty_Sel                = 0;
        tuningBuffer->Dve_Smth_Dv_Mode                            = false;
        tuningBuffer->Dve_Smth_Dv_Th_1                            = 0;
        tuningBuffer->Dve_Smth_Dv_Th_0                            = 12;
        tuningBuffer->Dve_Smth_Dv_Ada_Base                        = 5;
        tuningBuffer->Dve_Smth_Dv_Vert_Pnlty_Sel                  = 1;
        tuningBuffer->Dve_Smth_Dv_Horz_Pnlty_Sel                  = 2;
        tuningBuffer->Dve_Ord_Pnlty_Sel                           = 1;
        tuningBuffer->Dve_Ord_Coring                              = 4;
        tuningBuffer->Dve_Ord_DownSample_En                       = 0;
        tuningBuffer->Dve_Ord_Th                                  = 256;
        tuningBuffer->Dve_Vert_Sv                                 = 4;
        tuningBuffer->Dve_Horz_Sv                                 = 1;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL = 2;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL    = 0;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL   = 2;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL    = 2;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL   = 2;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL   = 2;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL   = 2;

        tuningBuffer->DVE_RESPO_SEL = 0;
        tuningBuffer->DVE_CONFO_SEL = 0;

        tuningBuffer->Dve_Ord_As_TH      = 0;
        tuningBuffer->Dve_Ord_As_Mask    = {0x1c1c1c, 0x1c1c1c, 0x1c1c1c, 0x1c1c1c};
        tuningBuffer->Dve_Ord_Ref_Mask_A = {0, 0x7e007e, 0x7e007e, 0, 0, 0, 0};
        tuningBuffer->Dve_Ord_Ref_Mask_B = {0, 0x7e007e, 0x7e007e, 0, 0, 0, 0};
        tuningBuffer->Dve_Ord_Ref_Mask_C = {0, 0x7e007e, 0x7e007e, 0, 0, 0, 0};
        tuningBuffer->Dve_Ord_Ref_Mask_D = {0, 0x7e007e, 0x7e007e, 0, 0, 0, 0};
    }
}

void
HW_DPETuning::__initBBox(NSCam::NSIoPipe::DVEConfig *tuningBuffer, ENUM_STEREO_SCENARIO scenario)
{
    StereoArea area = StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y, scenario);
    area *= 8;
    MSize szImage = area.size;
    MPoint ptTopLeft = area.startPt;
    if(0 == StereoSettingProvider::getSensorRelativePosition()) {   //Main on left
        tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_LEFT   = ptTopLeft.x;
        tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_BOTTOM = szImage.h - ptTopLeft.y;
        tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_TOP    = ptTopLeft.y;
        tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_RIGHT  = szImage.w - ptTopLeft.x;
        tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_LEFT   = 0;
        tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_BOTTOM = szImage.h;
        tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_TOP    = 0;
        tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_RIGHT  = szImage.w;
    } else {
        tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_LEFT   = 0;
        tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_BOTTOM = szImage.h;
        tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_TOP    = 0;
        tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_RIGHT  = szImage.w;
        tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_LEFT   = ptTopLeft.x;
        tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_BOTTOM = szImage.h - ptTopLeft.y;
        tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_TOP    = ptTopLeft.y;
        tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_RIGHT  = szImage.w - ptTopLeft.x;
    }
    tuningBuffer->Dve_Org_Width  = szImage.w;
    tuningBuffer->Dve_Org_Height = szImage.h;
}

void
HW_DPETuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("===== DPE Parameters =====");
    for(int i = 0; i < E_DPE_SCENARIO_COUNT; i++) {
        NSCam::NSIoPipe::DVEConfig *tuningBuffer = __tuningBuffers[i];

        logger
        .FastLogD("Scenario:          %s", DPE_SCENARIO_NAMES[i])
        .FastLogD("Dve_l_Bbox_En:     %d", tuningBuffer->Dve_l_Bbox_En)
        .FastLogD("Dve_r_Bbox_En:     %d", tuningBuffer->Dve_r_Bbox_En)
        .FastLogD("Dve_Mask_En:       %d", tuningBuffer->Dve_Mask_En)
        .FastLogD("Dve_Org_Horz_Sr_0: %d", tuningBuffer->Dve_Org_Horz_Sr_0)
        .FastLogD("Dve_Org_Horz_Sr_1: %d", tuningBuffer->Dve_Org_Horz_Sr_1)

        .FastLogD("Dve_Org_l_Bbox.DVE_ORG_BBOX_LEFT:   %d", tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_LEFT)
        .FastLogD("Dve_Org_l_Bbox.DVE_ORG_BBOX_BOTTOM: %d", tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_BOTTOM)
        .FastLogD("Dve_Org_l_Bbox.DVE_ORG_BBOX_TOP:    %d", tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_TOP)
        .FastLogD("Dve_Org_l_Bbox.DVE_ORG_BBOX_RIGHT:  %d", tuningBuffer->Dve_Org_l_Bbox.DVE_ORG_BBOX_RIGHT)
        .FastLogD("Dve_Org_r_Bbox.DVE_ORG_BBOX_LEFT:   %d", tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_LEFT)
        .FastLogD("Dve_Org_r_Bbox.DVE_ORG_BBOX_BOTTOM: %d", tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_BOTTOM)
        .FastLogD("Dve_Org_r_Bbox.DVE_ORG_BBOX_TOP:    %d", tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_TOP)
        .FastLogD("Dve_Org_r_Bbox.DVE_ORG_BBOX_RIGHT:  %d", tuningBuffer->Dve_Org_r_Bbox.DVE_ORG_BBOX_RIGHT)
        .FastLogD("Dve_Org_Width:  %d", tuningBuffer->Dve_Org_Width)
        .FastLogD("Dve_Org_Height: %d", tuningBuffer->Dve_Org_Height)

        .FastLogD("Dve_Horz_Ds_Mode %d", tuningBuffer->Dve_Horz_Ds_Mode)
        .FastLogD("Dve_Vert_Ds_Mode %d", tuningBuffer->Dve_Vert_Ds_Mode)

        .FastLogD("Dve_Cand_Num:             %d", tuningBuffer->Dve_Cand_Num)
        .FastLogD("Dve_Cand_0.DVE_CAND_SEL:  0x%X", tuningBuffer->Dve_Cand_0.DVE_CAND_SEL)
        .FastLogD("Dve_Cand_0.DVE_CAND_TYPE: 0x%X", tuningBuffer->Dve_Cand_0.DVE_CAND_TYPE)
        .FastLogD("Dve_Cand_1.DVE_CAND_SEL:  0x%X", tuningBuffer->Dve_Cand_1.DVE_CAND_SEL)
        .FastLogD("Dve_Cand_1.DVE_CAND_TYPE: 0x%X", tuningBuffer->Dve_Cand_1.DVE_CAND_TYPE)
        .FastLogD("Dve_Cand_2.DVE_CAND_SEL:  0x%X", tuningBuffer->Dve_Cand_2.DVE_CAND_SEL)
        .FastLogD("Dve_Cand_2.DVE_CAND_TYPE: 0x%X", tuningBuffer->Dve_Cand_2.DVE_CAND_TYPE)
        .FastLogD("Dve_Cand_3.DVE_CAND_SEL:  0x%X", tuningBuffer->Dve_Cand_3.DVE_CAND_SEL)
        .FastLogD("Dve_Cand_3.DVE_CAND_TYPE: 0x%X", tuningBuffer->Dve_Cand_3.DVE_CAND_TYPE)
        .FastLogD("Dve_Cand_4.DVE_CAND_SEL:  0x%X", tuningBuffer->Dve_Cand_4.DVE_CAND_SEL)
        .FastLogD("Dve_Cand_4.DVE_CAND_TYPE: 0x%X", tuningBuffer->Dve_Cand_4.DVE_CAND_TYPE)
        .FastLogD("Dve_Cand_5.DVE_CAND_SEL:  0x%X", tuningBuffer->Dve_Cand_5.DVE_CAND_SEL)
        .FastLogD("Dve_Cand_5.DVE_CAND_TYPE: 0x%X", tuningBuffer->Dve_Cand_5.DVE_CAND_TYPE)
        .FastLogD("Dve_Cand_6.DVE_CAND_SEL:  0x%X", tuningBuffer->Dve_Cand_6.DVE_CAND_SEL)
        .FastLogD("Dve_Cand_6.DVE_CAND_TYPE: 0x%X", tuningBuffer->Dve_Cand_6.DVE_CAND_TYPE)
        .FastLogD("Dve_Cand_7.DVE_CAND_SEL:  0x%X", tuningBuffer->Dve_Cand_7.DVE_CAND_SEL)
        .FastLogD("Dve_Cand_7.DVE_CAND_TYPE: 0x%X", tuningBuffer->Dve_Cand_7.DVE_CAND_TYPE)

        .FastLogD("DVE_VERT_GMV: %d", tuningBuffer->DVE_VERT_GMV)
        .FastLogD("DVE_HORZ_GMV: %d", tuningBuffer->DVE_HORZ_GMV)

        .FastLogD("Dve_Type_Penality_Ctrl.DVE_RAND_COST:   %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_RAND_COST)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_GMV_COST:    %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_GMV_COST)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_PREV_COST:   %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_PREV_COST)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_NBR_COST:    %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_NBR_COST)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_REFINE_COST: %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_REFINE_COST)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_TMPR_COST:   %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_TMPR_COST)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_SPTL_COST:   %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_SPTL_COST)

        .FastLogD("Dve_Skp_Pre_Dv: %d", tuningBuffer->Dve_Skp_Pre_Dv)
        .FastLogD("Dve_Org_Vert_Sr_0: %d", tuningBuffer->Dve_Org_Vert_Sr_0)
        .FastLogD("Dve_Org_Start_Vert_Sv: %d", tuningBuffer->Dve_Org_Start_Vert_Sv)
        .FastLogD("Dve_Org_Start_Horz_Sv: %d", tuningBuffer->Dve_Org_Start_Horz_Sv)
        .FastLogD("Dve_Rand_Lut_0: %d", tuningBuffer->Dve_Rand_Lut_0)
        .FastLogD("Dve_Rand_Lut_1: %d", tuningBuffer->Dve_Rand_Lut_1)
        .FastLogD("Dve_Rand_Lut_2: %d", tuningBuffer->Dve_Rand_Lut_2)
        .FastLogD("Dve_Rand_Lut_3: %d", tuningBuffer->Dve_Rand_Lut_3)
        .FastLogD("Dve_Horz_Dv_Ini: %d", tuningBuffer->Dve_Horz_Dv_Ini)
        .FastLogD("Dve_Coft_Shift: %d", tuningBuffer->Dve_Coft_Shift)
        .FastLogD("Dve_Corner_Th: %d", tuningBuffer->Dve_Corner_Th)
        .FastLogD("Dve_Smth_Luma_Th_1: %d", tuningBuffer->Dve_Smth_Luma_Th_1)
        .FastLogD("Dve_Smth_Luma_Th_0: %d", tuningBuffer->Dve_Smth_Luma_Th_0)
        .FastLogD("Dve_Smth_Luma_Ada_Base: %d", tuningBuffer->Dve_Smth_Luma_Ada_Base)
        .FastLogD("Dve_Smth_Luma_Horz_Pnlty_Sel: %d", tuningBuffer->Dve_Smth_Luma_Horz_Pnlty_Sel)
        .FastLogD("Dve_Smth_Dv_Mode: %d", tuningBuffer->Dve_Smth_Dv_Mode)
        .FastLogD("Dve_Smth_Dv_Th_1: %d", tuningBuffer->Dve_Smth_Dv_Th_1)
        .FastLogD("Dve_Smth_Dv_Th_0: %d", tuningBuffer->Dve_Smth_Dv_Th_0)
        .FastLogD("Dve_Smth_Dv_Ada_Base: %d", tuningBuffer->Dve_Smth_Dv_Ada_Base)
        .FastLogD("Dve_Smth_Dv_Vert_Pnlty_Sel: %d", tuningBuffer->Dve_Smth_Dv_Vert_Pnlty_Sel)
        .FastLogD("Dve_Smth_Dv_Horz_Pnlty_Sel: %d", tuningBuffer->Dve_Smth_Dv_Horz_Pnlty_Sel)
        .FastLogD("Dve_Ord_Pnlty_Sel: %d", tuningBuffer->Dve_Ord_Pnlty_Sel)
        .FastLogD("Dve_Ord_Coring: %d", tuningBuffer->Dve_Ord_Coring)
        .FastLogD("Dve_Ord_DownSample_En: %d", tuningBuffer->Dve_Ord_DownSample_En)
        .FastLogD("Dve_Ord_Th: %d", tuningBuffer->Dve_Ord_Th)
        .FastLogD("Dve_Vert_Sv: %d", tuningBuffer->Dve_Vert_Sv)
        .FastLogD("Dve_Horz_Sv: %d", tuningBuffer->Dve_Horz_Sv)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL: %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL: %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL: %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL: %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL: %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL: %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL)
        .FastLogD("Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL: %d", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL)
        .FastLogD("DVE_RESPO_SEL: %d", tuningBuffer->DVE_RESPO_SEL)
        .FastLogD("DVE_CONFO_SEL: %d", tuningBuffer->DVE_CONFO_SEL)
        .FastLogD("Dve_Ord_As_TH:      %d", tuningBuffer->Dve_Ord_As_TH)
        .FastLogD("Dve_Ord_As_Mask:    0x%X 0x%X 0x%X 0x%X", tuningBuffer->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_0,
                                                             tuningBuffer->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_1,
                                                             tuningBuffer->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_2,
                                                             tuningBuffer->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_3)
        .FastLogD("Dve_Ord_Ref_Mask_A: 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X", tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_0,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_1,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_2,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_3,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_4,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_5,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_6)
        .FastLogD("Dve_Ord_Ref_Mask_B: 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X", tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_0,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_1,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_2,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_3,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_4,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_5,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_6)
        .FastLogD("Dve_Ord_Ref_Mask_C: 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X", tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_0,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_1,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_2,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_3,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_4,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_5,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_6)
        .FastLogD("Dve_Ord_Ref_Mask_D: 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X", tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_0,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_1,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_2,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_3,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_4,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_5,
                                                                            tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_6)
        .FastLogD("===========================");
    }
    logger.print();
}

void
HW_DPETuning::_loadValuesFromDocument(const json& dpeValues)
{
    NSCam::NSIoPipe::DVEConfig *tuningBuffer;
    for(auto &tuning : dpeValues) {
        ENUM_STEREO_SCENARIO scenario;
        tuningBuffer = NULL;
        std::string scenarioKey = tuning[VALUE_KEY_PARAMETERS][QUERY_KEY_SCENARIO].get<std::string>();
        for(int s = 0; s < E_DPE_SCENARIO_COUNT; s++) {
            if(0 == scenarioKey.compare(0, strlen(DPE_SCENARIO_NAMES[s]), DPE_SCENARIO_NAMES[s])) {
                tuningBuffer = __tuningBuffers[s];
                scenario = STEREO_SCANARIO[s];
                break;
            }
        }

        if(NULL == tuningBuffer) {
            MY_LOGW("Scenario mismatch: %s", scenarioKey.c_str());
            continue;
        }

        const json &tuningValues = tuning[VALUE_KEY_VALUES];
        tuningBuffer->Dve_l_Bbox_En                               = _getInt(tuningValues, "Dve_l_Bbox_En");
        tuningBuffer->Dve_r_Bbox_En                               = _getInt(tuningValues, "Dve_r_Bbox_En");
        tuningBuffer->Dve_Mask_En                                 = _getInt(tuningValues, "Dve_Mask_En");

        tuningBuffer->Dve_Org_Horz_Sr_0                           = _getInt(tuningValues, "Dve_Org_Horz_Sr_0");
        tuningBuffer->Dve_Org_Horz_Sr_1                           = _getInt(tuningValues, "Dve_Org_Horz_Sr_1");

        tuningBuffer->Dve_Horz_Ds_Mode                            = 0;
        tuningBuffer->Dve_Vert_Ds_Mode                            = 0;

        __initBBox(tuningBuffer, scenario);

        tuningBuffer->Dve_Cand_Num                                = _getInt(tuningValues, "Dve_Cand_Num");
        tuningBuffer->Dve_Cand_0.DVE_CAND_SEL                     = _getInt(tuningValues, "Dve_Cand_0.DVE_CAND_SEL");
        tuningBuffer->Dve_Cand_0.DVE_CAND_TYPE                    = _getInt(tuningValues, "Dve_Cand_0.DVE_CAND_TYPE");
        tuningBuffer->Dve_Cand_1.DVE_CAND_SEL                     = _getInt(tuningValues, "Dve_Cand_1.DVE_CAND_SEL");
        tuningBuffer->Dve_Cand_1.DVE_CAND_TYPE                    = _getInt(tuningValues, "Dve_Cand_1.DVE_CAND_TYPE");
        tuningBuffer->Dve_Cand_2.DVE_CAND_SEL                     = _getInt(tuningValues, "Dve_Cand_2.DVE_CAND_SEL");
        tuningBuffer->Dve_Cand_2.DVE_CAND_TYPE                    = _getInt(tuningValues, "Dve_Cand_2.DVE_CAND_TYPE");
        tuningBuffer->Dve_Cand_3.DVE_CAND_SEL                     = _getInt(tuningValues, "Dve_Cand_3.DVE_CAND_SEL");
        tuningBuffer->Dve_Cand_3.DVE_CAND_TYPE                    = _getInt(tuningValues, "Dve_Cand_3.DVE_CAND_TYPE");
        tuningBuffer->Dve_Cand_4.DVE_CAND_SEL                     = _getInt(tuningValues, "Dve_Cand_4.DVE_CAND_SEL");
        tuningBuffer->Dve_Cand_4.DVE_CAND_TYPE                    = _getInt(tuningValues, "Dve_Cand_4.DVE_CAND_TYPE");
        tuningBuffer->Dve_Cand_5.DVE_CAND_SEL                     = _getInt(tuningValues, "Dve_Cand_5.DVE_CAND_SEL");
        tuningBuffer->Dve_Cand_5.DVE_CAND_TYPE                    = _getInt(tuningValues, "Dve_Cand_5.DVE_CAND_TYPE");
        tuningBuffer->Dve_Cand_6.DVE_CAND_SEL                     = _getInt(tuningValues, "Dve_Cand_6.DVE_CAND_SEL");
        tuningBuffer->Dve_Cand_6.DVE_CAND_TYPE                    = _getInt(tuningValues, "Dve_Cand_6.DVE_CAND_TYPE");
        tuningBuffer->Dve_Cand_7.DVE_CAND_SEL                     = _getInt(tuningValues, "Dve_Cand_7.DVE_CAND_SEL");
        tuningBuffer->Dve_Cand_7.DVE_CAND_TYPE                    = _getInt(tuningValues, "Dve_Cand_7.DVE_CAND_TYPE");

        tuningBuffer->DVE_VERT_GMV                                = _getInt(tuningValues, "DVE_VERT_GMV");
        tuningBuffer->DVE_HORZ_GMV                                = _getInt(tuningValues, "DVE_HORZ_GMV");

        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_RAND_COST        = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_RAND_COST");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_GMV_COST         = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_GMV_COST");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_PREV_COST        = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_PREV_COST");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_NBR_COST         = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_NBR_COST");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_REFINE_COST      = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_REFINE_COST");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_TMPR_COST        = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_TMPR_COST");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_SPTL_COST        = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_SPTL_COST");

        //Other default values
        tuningBuffer->Dve_Skp_Pre_Dv                              = _getInt(tuningValues, "Dve_Skp_Pre_Dv");
        tuningBuffer->Dve_Org_Vert_Sr_0                           = _getInt(tuningValues, "Dve_Org_Vert_Sr_0");
        tuningBuffer->Dve_Org_Start_Vert_Sv                       = _getInt(tuningValues, "Dve_Org_Start_Vert_Sv");
        tuningBuffer->Dve_Org_Start_Horz_Sv                       = _getInt(tuningValues, "Dve_Org_Start_Horz_Sv");
        tuningBuffer->Dve_Rand_Lut_0                              = _getInt(tuningValues, "Dve_Rand_Lut_0");
        tuningBuffer->Dve_Rand_Lut_1                              = _getInt(tuningValues, "Dve_Rand_Lut_1");
        tuningBuffer->Dve_Rand_Lut_2                              = _getInt(tuningValues, "Dve_Rand_Lut_2");
        tuningBuffer->Dve_Rand_Lut_3                              = _getInt(tuningValues, "Dve_Rand_Lut_3");
        tuningBuffer->Dve_Horz_Dv_Ini                             = _getInt(tuningValues, "Dve_Horz_Dv_Ini");
        tuningBuffer->Dve_Coft_Shift                              = _getInt(tuningValues, "Dve_Coft_Shift");
        tuningBuffer->Dve_Corner_Th                               = _getInt(tuningValues, "Dve_Corner_Th");
        tuningBuffer->Dve_Smth_Luma_Th_1                          = _getInt(tuningValues, "Dve_Smth_Luma_Th_1");
        tuningBuffer->Dve_Smth_Luma_Th_0                          = _getInt(tuningValues, "Dve_Smth_Luma_Th_0");
        tuningBuffer->Dve_Smth_Luma_Ada_Base                      = _getInt(tuningValues, "Dve_Smth_Luma_Ada_Base");
        tuningBuffer->Dve_Smth_Luma_Horz_Pnlty_Sel                = _getInt(tuningValues, "Dve_Smth_Luma_Horz_Pnlty_Sel");
        tuningBuffer->Dve_Smth_Dv_Mode                            = _getInt(tuningValues, "Dve_Smth_Dv_Mode");
        tuningBuffer->Dve_Smth_Dv_Th_1                            = _getInt(tuningValues, "Dve_Smth_Dv_Th_1");
        tuningBuffer->Dve_Smth_Dv_Th_0                            = _getInt(tuningValues, "Dve_Smth_Dv_Th_0");
        tuningBuffer->Dve_Smth_Dv_Ada_Base                        = _getInt(tuningValues, "Dve_Smth_Dv_Ada_Base");
        tuningBuffer->Dve_Smth_Dv_Vert_Pnlty_Sel                  = _getInt(tuningValues, "Dve_Smth_Dv_Vert_Pnlty_Sel");
        tuningBuffer->Dve_Smth_Dv_Horz_Pnlty_Sel                  = _getInt(tuningValues, "Dve_Smth_Dv_Horz_Pnlty_Sel");
        tuningBuffer->Dve_Ord_Pnlty_Sel                           = _getInt(tuningValues, "Dve_Ord_Pnlty_Sel");
        tuningBuffer->Dve_Ord_Coring                              = _getInt(tuningValues, "Dve_Ord_Coring");
        tuningBuffer->Dve_Ord_DownSample_En                       = 0;
        tuningBuffer->Dve_Ord_Th                                  = _getInt(tuningValues, "Dve_Ord_Th");
        tuningBuffer->Dve_Vert_Sv                                 = _getInt(tuningValues, "Dve_Vert_Sv");
        tuningBuffer->Dve_Horz_Sv                                 = _getInt(tuningValues, "Dve_Horz_Sv");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL    = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL   = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL    = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL   = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL   = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL");
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL   = _getInt(tuningValues, "Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL");

        tuningBuffer->DVE_RESPO_SEL                               = 0;
        tuningBuffer->DVE_CONFO_SEL                               = 0;

        tuningBuffer->Dve_Ord_As_TH                               = _getInt(tuningValues, "Dve_Ord_As_TH");
        tuningBuffer->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_0           = _getInt(tuningValues, "Dve_Ord_As_Mask.DVE_ORD_AS_MASK_0");
        tuningBuffer->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_1           = _getInt(tuningValues, "Dve_Ord_As_Mask.DVE_ORD_AS_MASK_1");
        tuningBuffer->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_2           = _getInt(tuningValues, "Dve_Ord_As_Mask.DVE_ORD_AS_MASK_2");
        tuningBuffer->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_3           = _getInt(tuningValues, "Dve_Ord_As_Mask.DVE_ORD_AS_MASK_3");

        tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_0     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_0");
        tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_1     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_1");
        tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_2     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_2");
        tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_3     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_3");
        tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_4     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_4");
        tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_5     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_5");
        tuningBuffer->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_6     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_6");

        tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_0     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_0");
        tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_1     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_1");
        tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_2     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_2");
        tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_3     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_3");
        tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_4     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_4");
        tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_5     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_5");
        tuningBuffer->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_6     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_6");

        tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_0     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_0");
        tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_1     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_1");
        tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_2     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_2");
        tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_3     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_3");
        tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_4     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_4");
        tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_5     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_5");
        tuningBuffer->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_6     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_6");

        tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_0     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_0");
        tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_1     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_1");
        tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_2     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_2");
        tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_3     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_3");
        tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_4     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_4");
        tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_5     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_5");
        tuningBuffer->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_6     = _getInt(tuningValues, "Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_6");
    }
}

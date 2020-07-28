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

#include "hw_dpe_tuning.h"

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

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
    for(int i = 0; i < SUPPORT_SCENARIO_COUNT; i++) {
        NSCam::NSIoPipe::DVEConfig *tuningBuffer = __tuningBuffers[i];
        tuningBuffer->Dve_l_Bbox_En     = 1;
        tuningBuffer->Dve_r_Bbox_En     = 1;
        tuningBuffer->Dve_Mask_En       = 1;

        tuningBuffer->Dve_Org_Horz_Sr_0 = 255;
        tuningBuffer->Dve_Org_Horz_Sr_1 = 128;

        __initBBox(tuningBuffer, __scenarios[i]);

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
        tuningBuffer->Dve_Smth_Luma_Horz_Pnlty_Sel                = 1;
        tuningBuffer->Dve_Smth_Dv_Mode                            = true;
        tuningBuffer->Dve_Smth_Dv_Th_1                            = 0;
        tuningBuffer->Dve_Smth_Dv_Th_0                            = 12;
        tuningBuffer->Dve_Smth_Dv_Ada_Base                        = 5;
        tuningBuffer->Dve_Smth_Dv_Vert_Pnlty_Sel                  = 1;
        tuningBuffer->Dve_Smth_Dv_Horz_Pnlty_Sel                  = 2;
        tuningBuffer->Dve_Ord_Pnlty_Sel                           = 1;
        tuningBuffer->Dve_Ord_Coring                              = 4;
        tuningBuffer->Dve_Ord_Th                                  = 256;
        tuningBuffer->Dve_Vert_Sv                                 = 4;
        tuningBuffer->Dve_Horz_Sv                                 = 1;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL = 2;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL    = 2;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL   = 2;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL    = 2;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL   = 2;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL   = 2;
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL   = 2;
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
    for(int i = 0; i < SUPPORT_SCENARIO_COUNT; i++) {
        NSCam::NSIoPipe::DVEConfig *tuningBuffer = __tuningBuffers[i];

        logger
        .FastLogD("Scenario:          %s", SCENARIO_NAMES[__scenarios[i]])
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
        .FastLogD("===========================");
    }
    logger.print();
}

void
HW_DPETuning::_initDocument()
{
    // "HW_DPE": [
    //     {
    //         "Parameters": {
    //             "Scenario": "Capture"
    //         },
    //         "Values": {
    //             "Dve_l_Bbox_En": 0,
    //             "Dve_r_Bbox_En": 0,
    //             "Dve_Mask_En": 0,
    //             ...
    //         }
    //     },
    //     {
    //         "Parameters": {
    //             "Scenario": "Preview"
    //         },
    //         "Values": {
    //             "Dve_l_Bbox_En": 0,
    //             "Dve_r_Bbox_En": 0,
    //             "Dve_Mask_En": 0,
    //             ...
    //         }
    //     }
    // ],

    Value dpeValue(kArrayType);
    Value previewParams(kObjectType);
    Value captureParams(kObjectType);

    Value *values[] = {&previewParams, &captureParams};
    for(int i = 0; i < SUPPORT_SCENARIO_COUNT; i++) {
        Value *value = values[i];
        // "Parameters": {
        //     "Scenario": "Capture"
        // },
        Value parameters(kObjectType);
        parameters.AddMember(QUERY_KEY_SCENARIO, StringRef(SCENARIO_NAMES[__scenarios[i]]), _allocator);
        value->AddMember(VALUE_KEY_PARAMETERS, parameters, _allocator);

        // "Values": {
        //     "Dve_l_Bbox_En": 0,
        //     "Dve_r_Bbox_En": 0,
        //     "Dve_Mask_En": 0,
        //     ...
        // }
        NSCam::NSIoPipe::DVEConfig *tuningBuffer = __tuningBuffers[i];
        Value tuningValue(kObjectType);
        #define AddTuning(key, value) tuningValue.AddMember(key, Value().SetInt(value), _allocator)
        AddTuning("Dve_l_Bbox_En", tuningBuffer->Dve_l_Bbox_En);
        AddTuning("Dve_r_Bbox_En", tuningBuffer->Dve_r_Bbox_En);
        AddTuning("Dve_Mask_En", tuningBuffer->Dve_Mask_En);
        AddTuning("Dve_Org_Horz_Sr_0", tuningBuffer->Dve_Org_Horz_Sr_0);
        AddTuning("Dve_Org_Horz_Sr_1", tuningBuffer->Dve_Org_Horz_Sr_1);
        AddTuning("Dve_Cand_Num", tuningBuffer->Dve_Cand_Num);
        AddTuning("Dve_Cand_0.DVE_CAND_SEL", tuningBuffer->Dve_Cand_0.DVE_CAND_SEL);
        AddTuning("Dve_Cand_0.DVE_CAND_TYPE", tuningBuffer->Dve_Cand_0.DVE_CAND_TYPE);
        AddTuning("Dve_Cand_1.DVE_CAND_SEL", tuningBuffer->Dve_Cand_1.DVE_CAND_SEL);
        AddTuning("Dve_Cand_1.DVE_CAND_TYPE", tuningBuffer->Dve_Cand_1.DVE_CAND_TYPE);
        AddTuning("Dve_Cand_2.DVE_CAND_SEL", tuningBuffer->Dve_Cand_2.DVE_CAND_SEL);
        AddTuning("Dve_Cand_2.DVE_CAND_TYPE", tuningBuffer->Dve_Cand_2.DVE_CAND_TYPE);
        AddTuning("Dve_Cand_3.DVE_CAND_SEL", tuningBuffer->Dve_Cand_3.DVE_CAND_SEL);
        AddTuning("Dve_Cand_3.DVE_CAND_TYPE", tuningBuffer->Dve_Cand_3.DVE_CAND_TYPE);
        AddTuning("Dve_Cand_4.DVE_CAND_SEL", tuningBuffer->Dve_Cand_4.DVE_CAND_SEL);
        AddTuning("Dve_Cand_4.DVE_CAND_TYPE", tuningBuffer->Dve_Cand_4.DVE_CAND_TYPE);
        AddTuning("Dve_Cand_5.DVE_CAND_SEL", tuningBuffer->Dve_Cand_5.DVE_CAND_SEL);
        AddTuning("Dve_Cand_5.DVE_CAND_TYPE", tuningBuffer->Dve_Cand_5.DVE_CAND_TYPE);
        AddTuning("Dve_Cand_6.DVE_CAND_SEL", tuningBuffer->Dve_Cand_6.DVE_CAND_SEL);
        AddTuning("Dve_Cand_6.DVE_CAND_TYPE", tuningBuffer->Dve_Cand_6.DVE_CAND_TYPE);
        AddTuning("Dve_Cand_7.DVE_CAND_SEL", tuningBuffer->Dve_Cand_7.DVE_CAND_SEL);
        AddTuning("Dve_Cand_7.DVE_CAND_TYPE", tuningBuffer->Dve_Cand_7.DVE_CAND_TYPE);
        AddTuning("DVE_VERT_GMV", tuningBuffer->DVE_VERT_GMV);
        AddTuning("DVE_HORZ_GMV", tuningBuffer->DVE_HORZ_GMV);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_RAND_COST", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_RAND_COST);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_GMV_COST", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_GMV_COST);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_PREV_COST", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_PREV_COST);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_NBR_COST", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_NBR_COST);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_REFINE_COST", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_REFINE_COST);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_TMPR_COST", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_TMPR_COST);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_SPTL_COST", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_SPTL_COST);
        AddTuning("Dve_Skp_Pre_Dv", tuningBuffer->Dve_Skp_Pre_Dv);
        AddTuning("Dve_Org_Vert_Sr_0", tuningBuffer->Dve_Org_Vert_Sr_0);
        AddTuning("Dve_Org_Start_Vert_Sv", tuningBuffer->Dve_Org_Start_Vert_Sv);
        AddTuning("Dve_Org_Start_Horz_Sv", tuningBuffer->Dve_Org_Start_Horz_Sv);
        AddTuning("Dve_Rand_Lut_0", tuningBuffer->Dve_Rand_Lut_0);
        AddTuning("Dve_Rand_Lut_1", tuningBuffer->Dve_Rand_Lut_1);
        AddTuning("Dve_Rand_Lut_2", tuningBuffer->Dve_Rand_Lut_2);
        AddTuning("Dve_Rand_Lut_3", tuningBuffer->Dve_Rand_Lut_3);
        AddTuning("Dve_Horz_Dv_Ini", tuningBuffer->Dve_Horz_Dv_Ini);
        AddTuning("Dve_Coft_Shift", tuningBuffer->Dve_Coft_Shift);
        AddTuning("Dve_Corner_Th", tuningBuffer->Dve_Corner_Th);
        AddTuning("Dve_Smth_Luma_Th_1", tuningBuffer->Dve_Smth_Luma_Th_1);
        AddTuning("Dve_Smth_Luma_Th_0", tuningBuffer->Dve_Smth_Luma_Th_0);
        AddTuning("Dve_Smth_Luma_Ada_Base", tuningBuffer->Dve_Smth_Luma_Ada_Base);
        AddTuning("Dve_Smth_Luma_Horz_Pnlty_Sel", tuningBuffer->Dve_Smth_Luma_Horz_Pnlty_Sel);
        AddTuning("Dve_Smth_Dv_Mode", tuningBuffer->Dve_Smth_Dv_Mode);
        AddTuning("Dve_Smth_Dv_Th_1", tuningBuffer->Dve_Smth_Dv_Th_1);
        AddTuning("Dve_Smth_Dv_Th_0", tuningBuffer->Dve_Smth_Dv_Th_0);
        AddTuning("Dve_Smth_Dv_Ada_Base", tuningBuffer->Dve_Smth_Dv_Ada_Base);
        AddTuning("Dve_Smth_Dv_Vert_Pnlty_Sel", tuningBuffer->Dve_Smth_Dv_Vert_Pnlty_Sel);
        AddTuning("Dve_Smth_Dv_Horz_Pnlty_Sel", tuningBuffer->Dve_Smth_Dv_Horz_Pnlty_Sel);
        AddTuning("Dve_Ord_Pnlty_Sel", tuningBuffer->Dve_Ord_Pnlty_Sel);
        AddTuning("Dve_Ord_Coring", tuningBuffer->Dve_Ord_Coring);
        AddTuning("Dve_Ord_Th", tuningBuffer->Dve_Ord_Th);
        AddTuning("Dve_Vert_Sv", tuningBuffer->Dve_Vert_Sv);
        AddTuning("Dve_Horz_Sv", tuningBuffer->Dve_Horz_Sv);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL);
        AddTuning("Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL", tuningBuffer->Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL);
        value->AddMember(VALUE_KEY_VALUES, tuningValue.Move(), _allocator);

        dpeValue.PushBack(value->Move(), _allocator);
    }

    Value key;
    key.SetString(StringRef(getTuningKey(), strlen(getTuningKey())));
    _document.AddMember(key, dpeValue, _allocator);
}

void
HW_DPETuning::_loadValuesFromDocument()
{
    Value& dpeValues = _document[getTuningKey()];
    NSCam::NSIoPipe::DVEConfig *tuningBuffer;
    for(SizeType i = 0; i < dpeValues.Size(); i++) {
        const Value &value = dpeValues[i];
        const Value &tuningValues = value[VALUE_KEY_VALUES];
        ENUM_STEREO_SCENARIO scenario;

        if(!strncmp(SCENARIO_NAMES[__scenarios[0]],
                    value[VALUE_KEY_PARAMETERS][QUERY_KEY_SCENARIO].GetString(),
                    strlen(SCENARIO_NAMES[__scenarios[0]])))
        {
            tuningBuffer = &__pvTuning;
            scenario = eSTEREO_SCENARIO_PREVIEW;
        } else {
            tuningBuffer = &__capTuning;
            scenario = eSTEREO_SCENARIO_CAPTURE;
        }

        tuningBuffer->Dve_l_Bbox_En = tuningValues["Dve_l_Bbox_En"].GetInt();
        tuningBuffer->Dve_r_Bbox_En = tuningValues["Dve_r_Bbox_En"].GetInt();
        tuningBuffer->Dve_Mask_En   = tuningValues["Dve_Mask_En"].GetInt();

        tuningBuffer->Dve_Org_Horz_Sr_0 = tuningValues["Dve_Org_Horz_Sr_0"].GetInt();
        tuningBuffer->Dve_Org_Horz_Sr_1 = tuningValues["Dve_Org_Horz_Sr_1"].GetInt();

        tuningBuffer->Dve_Horz_Ds_Mode = tuningValues["Dve_Horz_Ds_Mode"].GetInt();
        tuningBuffer->Dve_Vert_Ds_Mode = tuningValues["Dve_Vert_Ds_Mode"].GetInt();

        __initBBox(tuningBuffer, scenario);

        tuningBuffer->Dve_Cand_Num             = tuningValues["Dve_Cand_Num"].GetInt();
        tuningBuffer->Dve_Cand_0.DVE_CAND_SEL  = tuningValues["Dve_Cand_0.DVE_CAND_SEL"].GetInt();
        tuningBuffer->Dve_Cand_0.DVE_CAND_TYPE = tuningValues["Dve_Cand_0.DVE_CAND_TYPE"].GetInt();
        tuningBuffer->Dve_Cand_1.DVE_CAND_SEL  = tuningValues["Dve_Cand_1.DVE_CAND_SEL"].GetInt();
        tuningBuffer->Dve_Cand_1.DVE_CAND_TYPE = tuningValues["Dve_Cand_1.DVE_CAND_TYPE"].GetInt();
        tuningBuffer->Dve_Cand_2.DVE_CAND_SEL  = tuningValues["Dve_Cand_2.DVE_CAND_SEL"].GetInt();
        tuningBuffer->Dve_Cand_2.DVE_CAND_TYPE = tuningValues["Dve_Cand_2.DVE_CAND_TYPE"].GetInt();
        tuningBuffer->Dve_Cand_3.DVE_CAND_SEL  = tuningValues["Dve_Cand_3.DVE_CAND_SEL"].GetInt();
        tuningBuffer->Dve_Cand_3.DVE_CAND_TYPE = tuningValues["Dve_Cand_3.DVE_CAND_TYPE"].GetInt();
        tuningBuffer->Dve_Cand_4.DVE_CAND_SEL  = tuningValues["Dve_Cand_4.DVE_CAND_SEL"].GetInt();
        tuningBuffer->Dve_Cand_4.DVE_CAND_TYPE = tuningValues["Dve_Cand_4.DVE_CAND_TYPE"].GetInt();
        tuningBuffer->Dve_Cand_5.DVE_CAND_SEL  = tuningValues["Dve_Cand_5.DVE_CAND_SEL"].GetInt();
        tuningBuffer->Dve_Cand_5.DVE_CAND_TYPE = tuningValues["Dve_Cand_5.DVE_CAND_TYPE"].GetInt();
        tuningBuffer->Dve_Cand_6.DVE_CAND_SEL  = tuningValues["Dve_Cand_6.DVE_CAND_SEL"].GetInt();
        tuningBuffer->Dve_Cand_6.DVE_CAND_TYPE = tuningValues["Dve_Cand_6.DVE_CAND_TYPE"].GetInt();
        tuningBuffer->Dve_Cand_7.DVE_CAND_SEL  = tuningValues["Dve_Cand_7.DVE_CAND_SEL"].GetInt();
        tuningBuffer->Dve_Cand_7.DVE_CAND_TYPE = tuningValues["Dve_Cand_7.DVE_CAND_TYPE"].GetInt();

        tuningBuffer->DVE_VERT_GMV  = tuningValues["DVE_VERT_GMV"].GetInt();
        tuningBuffer->DVE_HORZ_GMV  = tuningValues["DVE_HORZ_GMV"].GetInt();

        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_RAND_COST   = tuningValues["Dve_Type_Penality_Ctrl.DVE_RAND_COST"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_GMV_COST    = tuningValues["Dve_Type_Penality_Ctrl.DVE_GMV_COST"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_PREV_COST   = tuningValues["Dve_Type_Penality_Ctrl.DVE_PREV_COST"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_NBR_COST    = tuningValues["Dve_Type_Penality_Ctrl.DVE_NBR_COST"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_REFINE_COST = tuningValues["Dve_Type_Penality_Ctrl.DVE_REFINE_COST"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_TMPR_COST   = tuningValues["Dve_Type_Penality_Ctrl.DVE_TMPR_COST"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_SPTL_COST   = tuningValues["Dve_Type_Penality_Ctrl.DVE_SPTL_COST"].GetInt();

        //Other default values
        tuningBuffer->Dve_Skp_Pre_Dv = tuningValues["Dve_Skp_Pre_Dv"].GetInt();
        tuningBuffer->Dve_Org_Vert_Sr_0 = tuningValues["Dve_Org_Vert_Sr_0"].GetInt();
        tuningBuffer->Dve_Org_Start_Vert_Sv = tuningValues["Dve_Org_Start_Vert_Sv"].GetInt();
        tuningBuffer->Dve_Org_Start_Horz_Sv = tuningValues["Dve_Org_Start_Horz_Sv"].GetInt();
        tuningBuffer->Dve_Rand_Lut_0 = tuningValues["Dve_Rand_Lut_0"].GetInt();
        tuningBuffer->Dve_Rand_Lut_1 = tuningValues["Dve_Rand_Lut_1"].GetInt();
        tuningBuffer->Dve_Rand_Lut_2 = tuningValues["Dve_Rand_Lut_2"].GetInt();
        tuningBuffer->Dve_Rand_Lut_3 = tuningValues["Dve_Rand_Lut_3"].GetInt();
        tuningBuffer->Dve_Horz_Dv_Ini = tuningValues["Dve_Horz_Dv_Ini"].GetInt();
        tuningBuffer->Dve_Coft_Shift = tuningValues["Dve_Coft_Shift"].GetInt();
        tuningBuffer->Dve_Corner_Th = tuningValues["Dve_Corner_Th"].GetInt();
        tuningBuffer->Dve_Smth_Luma_Th_1 = tuningValues["Dve_Smth_Luma_Th_1"].GetInt();
        tuningBuffer->Dve_Smth_Luma_Th_0 = tuningValues["Dve_Smth_Luma_Th_0"].GetInt();
        tuningBuffer->Dve_Smth_Luma_Ada_Base = tuningValues["Dve_Smth_Luma_Ada_Base"].GetInt();
        tuningBuffer->Dve_Smth_Luma_Horz_Pnlty_Sel = tuningValues["Dve_Smth_Luma_Horz_Pnlty_Sel"].GetInt();
        tuningBuffer->Dve_Smth_Dv_Mode = tuningValues["Dve_Smth_Dv_Mode"].GetInt();
        tuningBuffer->Dve_Smth_Dv_Th_1 = tuningValues["Dve_Smth_Dv_Th_1"].GetInt();
        tuningBuffer->Dve_Smth_Dv_Th_0 = tuningValues["Dve_Smth_Dv_Th_0"].GetInt();
        tuningBuffer->Dve_Smth_Dv_Ada_Base = tuningValues["Dve_Smth_Dv_Ada_Base"].GetInt();
        tuningBuffer->Dve_Smth_Dv_Vert_Pnlty_Sel = tuningValues["Dve_Smth_Dv_Vert_Pnlty_Sel"].GetInt();
        tuningBuffer->Dve_Smth_Dv_Horz_Pnlty_Sel = tuningValues["Dve_Smth_Dv_Horz_Pnlty_Sel"].GetInt();
        tuningBuffer->Dve_Ord_Pnlty_Sel = tuningValues["Dve_Ord_Pnlty_Sel"].GetInt();
        tuningBuffer->Dve_Ord_Coring = tuningValues["Dve_Ord_Coring"].GetInt();
        tuningBuffer->Dve_Ord_Th = tuningValues["Dve_Ord_Th"].GetInt();
        tuningBuffer->Dve_Vert_Sv = tuningValues["Dve_Vert_Sv"].GetInt();
        tuningBuffer->Dve_Horz_Sv = tuningValues["Dve_Horz_Sv"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL = tuningValues["Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL    = tuningValues["Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL   = tuningValues["Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL    = tuningValues["Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL   = tuningValues["Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL   = tuningValues["Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL"].GetInt();
        tuningBuffer->Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL   = tuningValues["Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL"].GetInt();
    }
}

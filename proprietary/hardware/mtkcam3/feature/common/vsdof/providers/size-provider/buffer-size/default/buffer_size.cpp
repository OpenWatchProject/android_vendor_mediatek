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
#define LOG_TAG "StereoSizeProvider"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
CAM_ULOG_DECLARE_MODULE_ID(CAM_ULOG_MODULE_ID);

#include <math.h>
#include <algorithm>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>

#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include "stereo_size_provider_imp.h"
#include <vsdof/hal/ProfileUtil.h>
#include <stereo_tuning_provider.h>
#include "base_size.h"

#if (1==HAS_AIDEPTH)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include <MTKAIDepth.h>
#include <MTKVideoAIDepth.h>
#pragma GCC diagnostic pop
#endif

using namespace android;
using namespace NSCam;

#define STEREO_SIZE_PROVIDER_DEBUG

#ifdef STEREO_SIZE_PROVIDER_DEBUG    // Enable debug log.

#undef __func__
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else   // Disable debug log.
#define MY_LOGD(a,...)
#define MY_LOGI(a,...)
#define MY_LOGW(a,...)
#define MY_LOGE(a,...)
#endif  // STEREO_SIZE_PROVIDER_DEBUG

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

StereoArea
StereoSizeProviderImp::__getBufferSize(ENUM_BUFFER_NAME eName,
                                       ENUM_STEREO_SCENARIO eScenario,
                                       ENUM_ROTATION capOrientation)
{
    StereoArea result = STEREO_AREA_ZERO;
    const StereoSizeConfig CONFIG_WO_PADDING_WO_ROTATE(STEREO_AREA_WO_PADDING, STEREO_AREA_WO_ROTATE);
    const StereoSizeConfig CONFIG_WO_PADDING_W_ROTATE(STEREO_AREA_WO_PADDING, STEREO_AREA_W_ROTATE);
    const StereoSizeConfig CONFIG_W_PADDING_W_ROTATE(STEREO_AREA_W_PADDING, STEREO_AREA_W_ROTATE);

    switch(eName) {
        //P2A output for DL-Depth
        case E_DLD_P2A_M:
            {
                //Rule:
                //1. Max: 1920x1080
                //2. Min: FEFM input size
                //3. Can beCustomized in setting
                //4. Default value is decided by experiences(in thirdPartyDepthmapSize)
                StereoArea area = thirdPartyDepthmapSize(StereoSettingProvider::imageRatio(), eScenario);
                result = area.apply64Align();
            }
            break;
        case E_DLD_P2A_S:
            {
                StereoArea area = getBufferSize(E_DLD_P2A_M);
                //Get FOV Cropped size
                area *= StereoSettingProvider::getStereoCameraFOVRatio();
                result = area.apply64Align();
            }
            break;

        //DL Depth
        case E_DL_DEPTHMAP:
            {
                StereoArea area = thirdPartyDepthmapSize(StereoSettingProvider::imageRatio(), eScenario);
                result = area.rotatedByModule();
            }
            break;

#if (1==HAS_AIDEPTH)
        case E_VAIDEPTH_DEPTHMAP:
            result = StereoArea(__getVideoAIDepthModelSize()).rotatedByModule();
            break;
#endif

        case E_MASK_IN_M:
        case E_RECT_IN_M:
#if (0==HAS_HW_DPE)
            {
                StereoArea area;
                if(eSTEREO_SCENARIO_CAPTURE == eScenario) {
                    area = StereoSize::getStereoArea2x(CONFIG_WO_PADDING_W_ROTATE);
                } else {
                    area = StereoSize::getStereoArea1x(CONFIG_WO_PADDING_W_ROTATE);
                    if(StereoSettingProvider::getStereoBaseline() <= DOUBLE_WIDTH_THRESHOLD) {
                        area.applyDoubleWidth();
                    }
                }

                area.apply32Align(IS_KEEP_CONTENT, !IS_CENTER_CONTENT);    //For GPU
                result = area;
                break;
            }
#else
            if(eSTEREO_SCENARIO_CAPTURE == eScenario) {
#if (1==HAS_AIDEPTH)
                result = StereoArea(__getAIDepthModelSize()).rotatedByModule();
#else
                StereoArea area = StereoSize::getStereoArea2x(CONFIG_WO_PADDING_W_ROTATE);
                area.apply32Align(IS_KEEP_CONTENT, !IS_CENTER_CONTENT);    //For GPU
                result = area;
#endif
            } else {
                result = StereoSize::getStereoArea1x(CONFIG_WO_PADDING_W_ROTATE);
#if (1==HAS_WPE)
                result.apply64Align(IS_KEEP_CONTENT, !IS_CENTER_CONTENT); //For WPE
#endif
            }
#endif
            break;
        case E_MASK_IN_S:
#if (0==HAS_WPE || 0==HAS_HW_DPE)
            result = getBufferSize(E_MASK_IN_M, eScenario);
            break;
#endif
        case E_RECT_IN_S:
            {
                StereoArea area = getBufferSize(E_RECT_IN_M, eScenario);
                if(StereoSettingProvider::isActiveStereo())
                {
                    result = area;
                    break;
                }

                area.removePadding();

                //For quality enhancement
                const float WARPING_IMAGE_RATIO = 1.0f; //For better quality
                const float FOV_RATIO           = StereoSettingProvider::getStereoCameraFOVRatio();
                const float IMAGE_RATIO         = WARPING_IMAGE_RATIO * FOV_RATIO;
                area.enlargeWith2AlignedRounding(IMAGE_RATIO);

                if(eSTEREO_SCENARIO_CAPTURE != eScenario) {
#if (0==HAS_WPE || 0==HAS_HW_DPE)
                    //Work around for GPU: input size must >= output size
                    MSize outputSize = getBufferSize(E_MV_Y, eScenario);
                    if(area.size.w < outputSize.w ||
                       area.size.h < outputSize.h)
                    {
                        MSize contentSize = area.size;
                        area.size.w = std::max(outputSize.w, contentSize.w);
                        area.size.h = std::max(outputSize.h, contentSize.h);
                        area.padding = area.size - contentSize;
                    }

                    area.apply32Align(IS_KEEP_CONTENT, !IS_CENTER_CONTENT); //For GPU
#else
                    area.apply64Align(IS_KEEP_CONTENT, !IS_CENTER_CONTENT); //For WPE
#endif

                } else {
                    area.apply64Align(!IS_KEEP_CONTENT);       //For AIDepth
                }

                result = area;
            }
            break;

        //N3D before MDP for capture
        case E_MV_Y_LARGE:
        case E_MASK_M_Y_LARGE:
        case E_SV_Y_LARGE:
        case E_MASK_S_Y_LARGE:
            {
                StereoArea area = StereoSize::getStereoArea2x(CONFIG_W_PADDING_W_ROTATE);
                area.apply32Align(IS_KEEP_CONTENT);    //For GPU

                result = area;
            }   //no need break here
            break;

        //N3D Output
        case E_LDC:
            if(eScenario != eSTEREO_SCENARIO_CAPTURE) {
                result = StereoSize::getStereoArea1x(CONFIG_W_PADDING_W_ROTATE);
                break;
            }   //no need break here

        case E_MV_Y:
        case E_MASK_M_Y:
#if (1==HAS_AIDEPTH)
            if(eScenario == eSTEREO_SCENARIO_CAPTURE)
            {
                result = getBufferSize(E_RECT_IN_M, eScenario);
                break;
            }   // no need break here
#endif
        case E_SV_Y:
        case E_MASK_S_Y:
            {
#if (1==HAS_AIDEPTH)
                if(eScenario == eSTEREO_SCENARIO_CAPTURE)
                {
                    result = getBufferSize(E_RECT_IN_M, eScenario);
                    break;
                }   // no need break here
#endif
                StereoArea area = StereoSize::getStereoArea1x(CONFIG_W_PADDING_W_ROTATE);
                if(eScenario == eSTEREO_SCENARIO_CAPTURE) {
                    area *= 2;
                    // area.apply32Align();    //For GPU
                }
#if (0==HAS_HW_DPE)
                else if(StereoSettingProvider::getStereoBaseline() <= DOUBLE_WIDTH_THRESHOLD)
                {
                    area.applyDoubleWidth();
                }

                //1. For GPU, must be 32-pixel aligned
                area.apply32Align(IS_KEEP_CONTENT);
#else
                //2. For WPE, image block has better performance if 64 bytes-aligned
                area.apply64Align(IS_KEEP_CONTENT);
#endif

                result = area;
            }
        break;

        //DPE Output
        case E_DMP_H:
        case E_CFM_H:
        case E_RESPO:
#if (0==HAS_HW_DPE)
            result = getBufferSize(E_MV_Y, eScenario);
            break;
#else
        case E_DV_LR:   //In DPE2
            {
                StereoArea area = StereoSize::getStereoArea1x(CONFIG_W_PADDING_W_ROTATE);
                area.apply128AlignToWidth(true, false);

                if(E_DV_LR == eName) {
                    area.applyDoubleHeight();
                }

                result = area;
            }
#endif
            break;

        //For WPE
        case E_WARP_MAP_M:
        case E_WARP_MAP_S:
            {
                StereoArea area = getBufferSize(E_SV_Y, eScenario);
                //Only keep size
                area.padding = MSIZE_ZERO;
                area.startPt = MPOINT_ZERO;
                area.rotatedByModule();
                area /= 2;

                //Check hardware ability
                if(area.size.w > WPE_MAX_SIZE.w ||
                   area.size.h > WPE_MAX_SIZE.h)
                {
                    area = StereoArea(WPE_MAX_SIZE);
                    ENUM_STEREO_RATIO ratio = StereoSettingProvider::imageRatio();
                    int m, n;
                    imageRatioMToN(ratio, m, n);

                    if(m*WPE_MAX_SIZE.h/WPE_MAX_SIZE.w >= n) {
                        area.applyRatio(StereoSettingProvider::imageRatio(), E_KEEP_WIDTH);
                    } else {
                        area.applyRatio(StereoSettingProvider::imageRatio(), E_KEEP_HEIGHT);
                    }
                }

                //1. WPE has better performance if 128 byte aligned, and WARP_MAP is 4 bytes/pixel
                //2. GPU must be 32 pixel aligned
                result = area.rotatedByModule().apply32Align();
            }
            break;

        //OCC Output
        case E_MY_S:
            if(!StereoSettingProvider::isMTKDepthmapMode()) {
                StereoArea area = StereoSize::getStereoArea1x(CONFIG_WO_PADDING_W_ROTATE);
                int factor = 1;
                if(eSTEREO_SCENARIO_CAPTURE == eScenario) {
                    factor *= 2;
                }

                if(E_WMF_Y_CHANNEL != StereoTuningProvider::getWMFInputChannel()) {
                    factor *= 2;
                }

                if(factor > 1) {
                    area *= factor;
                }

                result = area;
                break;
            }   //no need break here for mtk size

        case E_DMH:
        case E_OCC:
        case E_NOC:
        case E_CFM_M:
        case E_ASF_CRM:
        case E_ASF_RD:
        case E_ASF_HF:
        case E_DMW:
            {
                StereoArea area = StereoSize::getStereoArea1x(CONFIG_WO_PADDING_W_ROTATE);
                if(E_ASF_CRM == eName) {
                    area.size.h *= 4;
                }

#if (HAS_HW_DPE < 2)
                if(StereoSettingProvider::isMTKDepthmapMode() &&
                   StereoSettingProvider::getDepthmapRefineLevel() == E_DEPTHMAP_REFINE_NONE)
                {
                    area.applyDoubleWidth();
                }
#else
                area.apply128AlignToWidth(true, false);
#endif

                result = area;
            }
            break;
        case E_BOKEH_PACKED_BUFFER:
            {
                StereoArea area = getBufferSize(E_DMW, eScenario);
                if(capOrientation & 0x2) {  //90 or 270 degree
                    MSize::value_type s = area.size.w;
                    area.size.w = area.size.h;
                    area.size.h = s;
                }

                area.size.h *= 4;
                result = area;
            }
            break;
        //GF Input
        case E_GF_IN_IMG_4X:
            {
                result = StereoSize::getStereoArea4x(CONFIG_WO_PADDING_W_ROTATE);
            }
            break;

        case E_GF_IN_IMG_2X:
            {
                result = StereoSize::getStereoArea2x(CONFIG_WO_PADDING_W_ROTATE);
            }
            break;

        //GF Output
        case E_DMG:
        case E_DMBG:
            {
                StereoArea area = StereoSize::getStereoArea1x(CONFIG_WO_PADDING_WO_ROTATE);
                if(eSTEREO_SCENARIO_CAPTURE == eScenario) {
                    area *= 2;
                }
#if (HAS_HW_DPE==2)
                area /= 2;
#endif

                result = area;
            }
            break;

        case E_DEPTH_MAP:
#if (1==HAS_AIDEPTH)
            if(eSTEREO_SCENARIO_CAPTURE == eScenario) {
                result = getBufferSize(E_DL_DEPTHMAP, eScenario);
                break;
            }
#endif
            if(StereoSettingProvider::is3rdParty(eScenario)){
                result = thirdPartyDepthmapSize(StereoSettingProvider::imageRatio());
            }
            else if(StereoSettingProvider::isMTKDepthmapMode())
            {
#if (0==HAS_HW_DPE)
                result = getBufferSize(E_OCC, eScenario);
#else
                result = StereoSize::getStereoArea1x(CONFIG_WO_PADDING_W_ROTATE);
#endif
            }
            else
            {
                result = getBufferSize(E_DMG, eScenario);
            }
            break;

        case E_INK:
            result = StereoSize::getStereoArea2x(CONFIG_WO_PADDING_WO_ROTATE);
            break;

        //Bokeh Output
        case E_BOKEH_WROT: //Saved image
            switch(eScenario) {
                case eSTEREO_SCENARIO_PREVIEW:
                    result = STEREO_AREA_ZERO;
                    break;
                case eSTEREO_SCENARIO_RECORD:
                    {
                        Pass2SizeInfo pass2SizeInfo;
                        getInstance()->getPass2SizeInfo(PASS2A, eScenario, pass2SizeInfo);
                        result = pass2SizeInfo.areaWDMA;
                    }
                    break;
                case eSTEREO_SCENARIO_CAPTURE:
                    {
                        result = StereoArea(__captureSize);
                    }
                    break;
                default:
                    break;
            }
            break;
        case E_BOKEH_WDMA:
            switch(eScenario) {
                case eSTEREO_SCENARIO_PREVIEW:  //Display
                case eSTEREO_SCENARIO_RECORD:
                    {
                        Pass2SizeInfo pass2SizeInfo;
                        getInstance()->getPass2SizeInfo(PASS2A, eScenario, pass2SizeInfo);
                        result = pass2SizeInfo.areaWDMA;
                    }
                    break;
                case eSTEREO_SCENARIO_CAPTURE:  //Clean image
                    {
                        result = StereoArea(__captureSize);
                    }
                    break;
                default:
                    break;
            }
            break;
        case E_BOKEH_3DNR:
            switch(eScenario) {
                case eSTEREO_SCENARIO_PREVIEW:
                case eSTEREO_SCENARIO_RECORD:
                    {
                        Pass2SizeInfo pass2SizeInfo;
                        getInstance()->getPass2SizeInfo(PASS2A, eScenario, pass2SizeInfo);
                        result = pass2SizeInfo.areaWDMA;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return result;
}

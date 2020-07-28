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
#define LOG_TAG "BOKEH_HAL"

#include "bokeh_hal_imp.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <vsdof/hal/ProfileUtil.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_VSDOF_HAL);

void
BOKEH_HAL_IMP::__logInitData()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("=============== Init ===============");
    FAST_LOGD("  WorkingBuffAddr      %p", __initInfo.WorkingBuffAddr);
    FAST_LOGD("  WorkingBuffSize      %d", __initInfo.WorkingBuffSize);
    FAST_LOGD("  +++++ Tuning params +++++  ");
    FAST_LOGD("  HorzDownSampleRatio: %d", __tuningInfo.HorzDownSampleRatio);
    FAST_LOGD("  VertDownSampleRatio: %d", __tuningInfo.VertDownSampleRatio);
    FAST_LOGD("  IterationTimes:      %d", __tuningInfo.IterationTimes);
    FAST_LOGD("  InterpolationMode:   %d", __tuningInfo.InterpolationMode);
    FAST_LOGD("  CoreNumber:          %d", __tuningInfo.CoreNumber);
    FAST_LOGD("  RFCoreNumber:        %d %d %d", __tuningInfo.RFCoreNumber[0],
                                                 __tuningInfo.RFCoreNumber[1],
                                                 __tuningInfo.RFCoreNumber[2]);
    FAST_LOGD("  NumOfExecution:      %d", __tuningInfo.NumOfExecution);
    FAST_LOGD("  Baseline:            %.2f", __tuningInfo.Baseline);

    FAST_LOGD("  NumOfParam: %d", __tuningInfo.NumOfParam);
    std::for_each(__tuningParams.begin(), __tuningParams.end(), [this](RefocusTuningParam &param) {
        FAST_LOGD("  %s: %d", param.key, param.value);
    });
    FAST_LOGD("  ----- Tuning params -----  ");
    FAST_LOG_PRINT;
}

void
BOKEH_HAL_IMP::__logExtraData(char *extraData)
{
    if(!LOG_ENABLED) {
        return;
    }

    if(NULL == extraData) {
        MY_LOGE("Invalid extra data(NULL)");
    } else if(0 == strlen(extraData))  {
        MY_LOGE("Invalid extra data(lenght = 0)");
    } else {
        FAST_LOGD("ExtraData: %s", extraData);
        FAST_LOG_PRINT;
    }
}

void
BOKEH_HAL_IMP::__logImageData()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("=============== Image ===============");
    FAST_LOGD("  Mode            %d", __imgInfo.Mode);
    FAST_LOGD("  ImgFmt          %d", __imgInfo.ImgFmt);
    FAST_LOGD("  TargetWidth     %d", __imgInfo.TargetWidth);
    FAST_LOGD("  TargetHeight    %d", __imgInfo.TargetHeight);
    FAST_LOGD("  TargetImgAddr   %p", __imgInfo.TargetImgAddr);
    FAST_LOGD("  ViewWidth       %d", __imgInfo.ViewWidth);
    FAST_LOGD("  ViewHeight      %d", __imgInfo.ViewHeight);
    FAST_LOGD("  DepthBufferAddr %p", __imgInfo.DepthBufferAddr);
    FAST_LOGD("  DepthBufferSize %d", __imgInfo.DepthBufferSize);
    FAST_LOGD("  TouchCoordX     %d", __imgInfo.TouchCoordX);
    FAST_LOGD("  TouchCoordY     %d", __imgInfo.TouchCoordY);
    FAST_LOGD("  DepthOfField    %d", __imgInfo.DepthOfField);
    FAST_LOGD("  MainCamPos      %d", __imgInfo.MainCamPos);
    FAST_LOGD("  dacInfo.min     %d", __imgInfo.dacInfo.min);
    FAST_LOGD("  dacInfo.max     %d", __imgInfo.dacInfo.max);
    FAST_LOGD("  dacInfo.cur     %d", __imgInfo.dacInfo.cur);
    FAST_LOGD("  faceInfo.isFd   %d", __imgInfo.faceInfo.isFd);
    FAST_LOGD("  faceInfo.ratio  %f", __imgInfo.faceInfo.ratio);
    FAST_LOGD("  afInfo.afType   %d", __imgInfo.afInfo.afType);
    FAST_LOGD("  afInfo.ROI      (%d, %d) (%d, %d)", __imgInfo.afInfo.x1,
                                                     __imgInfo.afInfo.y1,
                                                     __imgInfo.afInfo.x2,
                                                     __imgInfo.afInfo.y2);
    FAST_LOG_PRINT;
}

void
BOKEH_HAL_IMP::__logResult()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("=============== Result ===============");
    FAST_LOGD("  RefocusImageWidth     %d", __resultInfo.RefocusImageWidth);
    FAST_LOGD("  RefocusImageHeight    %d", __resultInfo.RefocusImageHeight);
    FAST_LOGD("  RefocusedYUVImageAddr %p", __resultInfo.RefocusedYUVImageAddr);
    FAST_LOG_PRINT;
}

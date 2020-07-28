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
#define LOG_TAG "BWDN_HAL"

#include "bwdn_hal_imp.h"
#include <mtkcam/utils/std/Log.h>

#if (1==SINGLE_LINE_LOG)
#define FAST_LOGD(fmt, arg...)  __fastLogger.FastLogD(fmt, ##arg)
#define FAST_LOGI(fmt, arg...)  __fastLogger.FastLogI(fmt, ##arg)
#else
#define FAST_LOGD(fmt, arg...)  __fastLogger.FastLogD("[%s]" fmt, __func__, ##arg)
#define FAST_LOGI(fmt, arg...)  __fastLogger.FastLogI("[%s]" fmt, __func__, ##arg)
#endif
#define FAST_LOGW(fmt, arg...)  __fastLogger.FastLogW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define FAST_LOGE(fmt, arg...)  __fastLogger.FastLogE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define FAST_LOG_PRINT  __fastLogger.print()

using namespace StereoHAL;

void
BWDN_HAL_IMP::__logInitData(bool initResult)
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("===== BWDN Init =====");
    FAST_LOGD("Init result %d", initResult);
    FAST_LOGD("i4CoreNum   %d", __initParams.i4CoreNum);
    FAST_LOGD("i4SplitSize %d", __initParams.i4SplitSize);
    FAST_LOGD("fConvMat    %f %f %f", __initParams.fConvMat[0],__initParams.fConvMat[1], __initParams.fConvMat[2]);
    FAST_LOGD("rAcc        %d", __initParams.rAcc);
    FAST_LOGD("=====================");
    FAST_LOG_PRINT;
}

void
BWDN_HAL_IMP::__logParams(bool setParamsResult)
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("===== BWDN Params =====");
    FAST_LOGD("Set params result  %d", setParamsResult);
    //ISP Info
    FAST_LOGD("rOBOffsetBayer     %d %d %d %d", __runtimeParams.rOBOffsetBayer[0],
                                              __runtimeParams.rOBOffsetBayer[1],
                                              __runtimeParams.rOBOffsetBayer[2],
                                              __runtimeParams.rOBOffsetBayer[3]);
    FAST_LOGD("rOBOffsetMono      %d %d %d %d", __runtimeParams.rOBOffsetMono[0],
                                              __runtimeParams.rOBOffsetMono[1],
                                              __runtimeParams.rOBOffsetMono[2],
                                              __runtimeParams.rOBOffsetMono[3]);
    FAST_LOGD("i4SensorGainBayer  %d", __runtimeParams.i4SensorGainBayer);
    FAST_LOGD("i4SensorGainMono   %d", __runtimeParams.i4SensorGainMono);
    FAST_LOGD("i4IspGainBayer     %d", __runtimeParams.i4IspGainBayer);
    FAST_LOGD("i4IspGainMono      %d", __runtimeParams.i4IspGainMono);
    FAST_LOGD("rPreGainBayer      %d %d %d", __runtimeParams.rPreGainBayer[0],
                                           __runtimeParams.rPreGainBayer[1],
                                           __runtimeParams.rPreGainBayer[2]);
    FAST_LOGD("i4BayerOrder       %d", __runtimeParams.i4BayerOrder);
    FAST_LOGD("i4RA               %d", __runtimeParams.i4RA);
    FAST_LOGD("i4BitMode          %d", __runtimeParams.i4BitMode);

    //ISO dependent
    FAST_LOGD("i4BWSingleRange    %d", __runtimeParams.i4BWSingleRange);
    FAST_LOGD("i4BWOccRange       %d", __runtimeParams.i4BWOccRange);
    FAST_LOGD("i4BWRange          %d", __runtimeParams.i4BWRange);
    FAST_LOGD("i4BWKernel         %d", __runtimeParams.i4BWKernel);
    FAST_LOGD("i4BRange           %d", __runtimeParams.i4BRange);
    FAST_LOGD("i4BKernel          %d", __runtimeParams.i4BKernel);
    FAST_LOGD("i4WRange           %d", __runtimeParams.i4WRange);
    FAST_LOGD("i4WKernel          %d", __runtimeParams.i4WKernel);

    //sensor dependent
    FAST_LOGD("i4VSL              %d", __runtimeParams.i4VSL);
    FAST_LOGD("i4VOFT             %d", __runtimeParams.i4VOFT);
    FAST_LOGD("i4VGAIN            %d", __runtimeParams.i4VGAIN);

    //information from N3D
    FAST_LOGD("rTrans             %f %f %f %f %f %f %f %f %f", __runtimeParams.rTrans[0],
                                                             __runtimeParams.rTrans[1],
                                                             __runtimeParams.rTrans[2],
                                                             __runtimeParams.rTrans[3],
                                                             __runtimeParams.rTrans[4],
                                                             __runtimeParams.rTrans[5],
                                                             __runtimeParams.rTrans[6],
                                                             __runtimeParams.rTrans[7],
                                                             __runtimeParams.rTrans[8]);
    FAST_LOGD("rDPadding          %d %d", __runtimeParams.rDPadding[0], __runtimeParams.rDPadding[1]);

    //Performance tuning
    FAST_LOGD("i4FastPreprocess   %d", __runtimeParams.i4FastPreprocess);
    FAST_LOGD("i4FullSkipStep     %d", __runtimeParams.i4FullSkipStep);
    FAST_LOGD("i4DblkRto          %d", __runtimeParams.i4DblkRto);
    FAST_LOGD("i4DblkTH           %d", __runtimeParams.i4DblkTH);
    FAST_LOGD("i4QSch             %d", __runtimeParams.i4QSch);

    //  For test
    FAST_LOGD("bUnPack            %d", __runtimeParams.bUnPack);
    FAST_LOGD("i4OutPackSize      %d", __runtimeParams.i4OutPackSize);

    FAST_LOGD("======================");
    FAST_LOG_PRINT;
}

void
BWDN_HAL_IMP::__logImageDetails(const char *prefix, BWDNImage *image)
{
    FAST_LOGD("--- %s ---", prefix);
    FAST_LOGD("eMemType   %d", image->eMemType);
    FAST_LOGD("eImgFmt    %d", image->eImgFmt);
    FAST_LOGD("i4Width    %d", image->i4Width);
    FAST_LOGD("i4Height   %d", image->i4Height);
    FAST_LOGD("i4Offset   %d", image->i4Offset);
    FAST_LOGD("i4Pitch    %d", image->i4Pitch);
    FAST_LOGD("i4Size     %d", image->i4Size);
    FAST_LOGD("i4PlaneNum %d", image->i4PlaneNum);
    FAST_LOGD("pvPlane    %p %p %p %p", image->pvPlane[0],
                                      image->pvPlane[1],
                                      image->pvPlane[2],
                                      image->pvPlane[3]);
}

void
BWDN_HAL_IMP::__logImages()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("===== BWDN Images =====");
    FAST_LOGD("i4DepthDSH %d", __runtimeImages.i4DepthDSH);
    FAST_LOGD("i4DepthDSV %d", __runtimeImages.i4DepthDSV);

    // Input Data
    __logImageDetails("prMonoProcessedRaw", __runtimeImages.prMonoProcessedRaw);
    __logImageDetails("prBayerProcessedRaw", __runtimeImages.prBayerProcessedRaw);
    __logImageDetails("prBayerW", __runtimeImages.prBayerW);

    // depth
    __logImageDetails("prDepth", __runtimeImages.prDepth);

    // Full Shading Gain
    __logImageDetails("prBayerGain", __runtimeImages.prBayerGain);
    __logImageDetails("prMonoGain", __runtimeImages.prMonoGain);

    // RGB denoise output after BWDN
    __logImageDetails("prOutput", __runtimeImages.prOutput);
    FAST_LOGD("======================");
    FAST_LOG_PRINT;
}

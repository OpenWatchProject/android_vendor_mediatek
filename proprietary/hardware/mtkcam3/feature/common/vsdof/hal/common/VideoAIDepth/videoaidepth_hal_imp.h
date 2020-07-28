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
#ifndef VIDEO_AIDEPTH_HAL_IMP_H_
#define VIDEO_AIDEPTH_HAL_IMP_H_

#include <videoaidepth_hal.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include <MTKVideoAIDepth.h>
#pragma GCC diagnostic pop
#include <mtkcam3/feature/stereo/hal/FastLogger.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>

using namespace android;
using namespace StereoHAL;
using namespace NSCam;
using namespace std;

#define DUMP_PATH_SIZE (128)

class VIDEO_AIDEPTH_HAL_IMP : public VIDEO_AIDEPTH_HAL
{
public:
    VIDEO_AIDEPTH_HAL_IMP();
    virtual ~VIDEO_AIDEPTH_HAL_IMP();

    virtual bool VideoAIDepthHALRun(VIDEO_AIDEPTH_HAL_PARAMS &param, VIDEO_AIDEPTH_HAL_OUTPUT &output);
protected:

private:
    void __init();
    bool __setAIDepthParams(VIDEO_AIDEPTH_HAL_PARAMS &param);
    bool __runAIDepth(VIDEO_AIDEPTH_HAL_OUTPUT &output);

    void __logInitData();
    void __logSetProcData();
    void __logAIDepthResult();

    void __dumpInitData();
    void __dumpSetProcData();

private:
    MTKVideoAIDepth         *__pAIDepth;

    int                     __mainCamPos = 0;
    MSize                   __imgSize;
    MSize                   __outputSize;   //after resizing, real output
    sp<IImageBuffer>        __aidepthOutputImg;
    VideoAIDepthTuningInfo  __tuningInfo;
    VideoAIDepthInitInfo    __initInfo;
    VideoAIDepthParam       __paramInfo;
    VideoAIDepthImageInfo   __imgInfo;
    VideoAIDepthResultInfo  __resultInfo;
    std::vector<sp<IImageBuffer>> __workingBuffers;

    const bool              LOG_ENABLED;
    char                    __dumpPrefix[DUMP_PATH_SIZE];
    TuningUtils::FILE_DUMP_NAMING_HINT *__dumpHint = NULL;
    int                     __dumpLevel = 1;

    FastLogger              __fastLogger;
};

#endif